/*
 * Copyright (C) 2017 The Android Open Source Project
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include "chre/platform/slpi/see/see_helper.h"

#include "pb_decode.h"
#include "pb_encode.h"
#include "sns_cal.pb.h"
#include "sns_client.pb.h"
#include "sns_client_api_v01.h"
#include "sns_std.pb.h"
#include "sns_std_sensor.pb.h"
#include "stringl.h"
#include "timer.h"

#include <cfloat>
#include <cinttypes>

#include "chre/platform/assert.h"
#include "chre/platform/log.h"
#include "chre/platform/slpi/power_control_util.h"
#include "chre/platform/slpi/system_time.h"
#include "chre/util/lock_guard.h"

namespace chre {
namespace {

//! Interval between SUID request retry.
constexpr Milliseconds kSuidReqIntervalMsec = Milliseconds(500);

//! Maximum dwell time to try a data type's SUID request.
constexpr Seconds kSuidReqMaxDwellSec = Seconds(10);

//! The SUID of the look up sensor.
const sns_std_suid kSuidLookup = sns_suid_sensor_init_default;

//! A struct to facilitate pb encode/decode
struct SeeBufArg {
  const void *buf;
  size_t bufLen;
};

//! A struct to facilitate pb decode of sync calls.
struct SeeSyncArg {
  sns_std_suid syncSuid;
  void *syncData;
  const char *syncDataType;
  bool syncIndFound;
};

//! SeeFloatArg can be used to decode a vectorized 3x3 array.
constexpr size_t kSeeFloatArgValLen = 9;

//! A struct to facilitate decoding a float array.
struct SeeFloatArg {
  size_t index;
  float val[kSeeFloatArgValLen];
};

//! A struct to facilitate pb decode of sensor data event.
struct SeeDataArg {
  uint64_t prevTimeNs;
  uint64_t timeNs;
  size_t sampleIndex;
  size_t totalSamples;
  UniquePtr<uint8_t> event;
  SensorType sensorType;
};

//! A struct to facilitate pb decode
struct SeeInfoArg {
  sns_std_suid suid;
  uint32_t msgId;
  SeeSyncArg *sync;
  SeeDataArg *data;
  bool decodeMsgIdOnly;
};

//! A struct to facilitate decoding sensor attributes.
struct SeeAttrArg {
  union {
    char strVal[kSeeAttrStrValLen];
    bool boolVal;
    struct {
      float fltMin;
      float fltMax;
    };
    int64_t int64;
  };
  bool initialized;
};

bool suidsMatch(const sns_std_suid& suid0, const sns_std_suid& suid1) {
  return (suid0.suid_high == suid1.suid_high
          && suid0.suid_low == suid1.suid_low);
}

/**
 * Copy an encoded pb message to a wrapper proto's field.
 */
bool copyPayload(pb_ostream_t *stream, const pb_field_t *field,
                 void *const *arg) {
  bool success = false;

  auto *data = static_cast<const SeeBufArg *>(*arg);
  if (!pb_encode_tag_for_field(stream, field)) {
    LOGE("Failed encoding pb tag");
  } else if (!pb_encode_string(
      stream, static_cast<const pb_byte_t *>(data->buf), data->bufLen)) {
    LOGE("Failed encoding pb string");
  } else {
    success = true;
  }
  return success;
}

/**
 * Encodes sns_std_attr_req pb message.
 *
 * @param msg A non-null pointer to the pb message unique pointer whose object
 *            will be assigned here.
 * @param msgLen A non-null pointer to the size of the encoded pb message.
 *
 * @return true if the pb message and length were obtained.
 */
bool encodeSnsStdAttrReq(UniquePtr<pb_byte_t> *msg, size_t *msgLen) {
  CHRE_ASSERT(msg);
  CHRE_ASSERT(msgLen);

  // Initialize the pb message
  sns_std_attr_req req = {};

  bool success = pb_get_encoded_size(msgLen, sns_std_attr_req_fields, &req);
  if (!success) {
    LOGE("pb_get_encoded_size failed for sns_str_attr_req");
  } else {
    UniquePtr<pb_byte_t> buf(static_cast<pb_byte_t *>(memoryAlloc(*msgLen)));
    *msg = std::move(buf);

    // The encoded size can be 0 as there's only one optional field.
    if (msg->isNull() && *msgLen > 0) {
      LOGE("No memory allocated to encode sns_std_attr_req");
    } else {
      pb_ostream_t stream = pb_ostream_from_buffer(msg->get(), *msgLen);

      success = pb_encode(&stream, sns_std_attr_req_fields, &req);
      if (!success) {
        LOGE("Error encoding sns_std_attr_req: %s", PB_GET_ERROR(&stream));
      }
    }
  }
  return success;
}

/**
 * Encodes sns_suid_req pb message.
 *
 * @param dataType Sensor data type, "accel" for example.
 * @param msg A non-null pointer to the pb message unique pointer whose object
 *            will be assigned here.
 * @param msgLen A non-null pointer to the size of the encoded pb message.
 *
 * @return true if the pb message and length were obtained.
 */
bool encodeSnsSuidReq(const char *dataType,
                      UniquePtr<pb_byte_t> *msg, size_t *msgLen) {
  CHRE_ASSERT(msg);
  CHRE_ASSERT(msgLen);

  // Initialize the pb message
  SeeBufArg data = {
    .buf = dataType,
    .bufLen = strlen(dataType),
  };
  sns_suid_req req = {
    .data_type.funcs.encode = copyPayload,
    .data_type.arg = &data,
  };

  bool success = pb_get_encoded_size(msgLen, sns_suid_req_fields, &req);
  if (!success) {
    LOGE("pb_get_encoded_size failed for sns_suid_req: %s", dataType);
  } else if (*msgLen == 0) {
    LOGE("Invalid pb encoded size for sns_suid_req");
  } else {
    UniquePtr<pb_byte_t> buf(static_cast<pb_byte_t *>(memoryAlloc(*msgLen)));
    *msg = std::move(buf);
    if (msg->isNull()) {
      LOGE("No memory allocated to encode sns_suid_req");
    } else {
      pb_ostream_t stream = pb_ostream_from_buffer(msg->get(), *msgLen);

      success = pb_encode(&stream, sns_suid_req_fields, &req);
      if (!success) {
        LOGE("Error encoding sns_suid_req: %s", PB_GET_ERROR(&stream));
      }
    }
  }
  return success;
}

/**
 * Encodes sns_std_sensor_config pb message.
 *
 * @param request The request to be encoded.
 * @param msg A non-null pointer to the pb message unique pointer whose object
 *            will be assigned here.
 * @param msgLen A non-null pointer to the size of the encoded pb message.
 *
 * @return true if the pb message and length were obtained.
 */
bool encodeSnsStdSensorConfig(const SeeSensorRequest& request,
                              UniquePtr<pb_byte_t> *msg, size_t *msgLen) {
  CHRE_ASSERT(msg);
  CHRE_ASSERT(msgLen);

  // Initialize the pb message
  sns_std_sensor_config req = {
    .sample_rate = request.samplingRateHz,
  };

  bool success = pb_get_encoded_size(msgLen, sns_std_sensor_config_fields,
                                     &req);
  if (!success) {
    LOGE("pb_get_encoded_size failed for sns_std_sensor_config");
  } else if (*msgLen == 0) {
    LOGE("Invalid pb encoded size for sns_std_sensor_config");
  } else {
    UniquePtr<pb_byte_t> buf(static_cast<pb_byte_t *>(memoryAlloc(*msgLen)));
    *msg = std::move(buf);
    if (msg->isNull()) {
      LOGE("No memory allocated to encode sns_std_sensor_config");
    } else {
      pb_ostream_t stream = pb_ostream_from_buffer(msg->get(), *msgLen);

      success = pb_encode(&stream, sns_std_sensor_config_fields, &req);
      if (!success) {
        LOGE("Error encoding sns_std_sensor_config: %s", PB_GET_ERROR(&stream));
      }
    }
  }
  return success;
}

/**
 * Sends a sync request to QMI.
 */
bool sendQmiReq(qmi_client_type qmiHandle, const sns_client_req_msg_v01& reqMsg,
                Nanoseconds timeoutResp) {
  bool success = false;

  sns_client_resp_msg_v01 resp;
  qmi_client_error_type status = qmi_client_send_msg_sync(
      qmiHandle, SNS_CLIENT_REQ_V01,
      const_cast<sns_client_req_msg_v01 *>(&reqMsg), sizeof(reqMsg),
      &resp, sizeof(resp), Milliseconds(timeoutResp).getMilliseconds());

  if (status != QMI_NO_ERR) {
    LOGE("Error sending QMI message %d", status);
  } else if (resp.resp.result != QMI_RESULT_SUCCESS_V01) {
    // TODO: Remove bypass and uncomment logging when b/68825825 is resolved.
    // LOGE("Client request failed with error %d", resp.resp.error);
    success = true;
  } else {
    success = true;
  }
  return success;
}

/**
 * Sends a sns_client_req message with provided payload.
 */
bool sendSnsClientReq(qmi_client_type qmiHandle, sns_std_suid suid,
                      uint32_t msgId, void *payload, size_t payloadLen,
                      bool batchValid, uint32_t batchPeriodUs,
                      Nanoseconds timeoutResp) {
  CHRE_ASSERT(payload || payloadLen == 0);
  bool success = false;

  // Initialize pb message to be encoded
  SeeBufArg data = {
    .buf = payload,
    .bufLen = payloadLen,
  };
  sns_client_request_msg req = {
    .suid = suid,
    .msg_id = msgId,
    .susp_config.client_proc_type = SNS_STD_CLIENT_PROCESSOR_SSC,
    .susp_config.delivery_type = SNS_CLIENT_DELIVERY_WAKEUP,
    .request.has_batching = batchValid,
    .request.batching.batch_period = batchPeriodUs,
    .request.payload.funcs.encode = copyPayload,
    .request.payload.arg = &data,
  };

  auto qmiReq = MakeUnique<sns_client_req_msg_v01>();
  if (qmiReq.isNull()) {
    LOGE("Failed to allocate memory for sns_client_req_msg_v01");
  } else {
    // Create pb stream
    pb_ostream_t stream = pb_ostream_from_buffer(
        qmiReq->payload, SNS_CLIENT_REQ_LEN_MAX_V01);

    // Encode pb message
    if (!pb_encode(&stream, sns_client_request_msg_fields, &req)) {
      LOGE("Error Encoding request: %s", PB_GET_ERROR(&stream));
    } else {
      qmiReq->payload_len = stream.bytes_written;
      success = sendQmiReq(qmiHandle, *qmiReq, timeoutResp);
    }
  }
  return success;
}

/**
 * Helps decode a pb string field and passes the string to the calling function.
 */
bool decodeStringField(pb_istream_t *stream, const pb_field_t *field,
                       void **arg) {
  auto *data = static_cast<SeeBufArg *>(*arg);
  data->bufLen = stream->bytes_left;
  data->buf = stream->state;

  return pb_read(stream, nullptr /* buf */, stream->bytes_left);
}

/**
 * Decodes each SUID.
 */
bool decodeSnsSuidEventSuid(pb_istream_t *stream, const pb_field_t *field,
                            void **arg) {
  sns_std_suid suid = {};
  bool success = pb_decode(stream, sns_std_suid_fields, &suid);
  if (!success) {
    LOGE("Error decoding sns_std_suid: %s", PB_GET_ERROR(stream));
  } else {
    auto *suids = static_cast<DynamicVector<sns_std_suid> *>(*arg);
    suids->push_back(suid);
    LOGD("Received SUID 0x%" PRIx64 " %" PRIx64, suid.suid_high, suid.suid_low);
  }
  return success;
}

bool decodeSnsSuidEvent(pb_istream_t *stream, const pb_field_t *field,
                        void **arg) {
  auto *info = static_cast<SeeInfoArg *>(*arg);
  if (!suidsMatch(info->suid, kSuidLookup)) {
    LOGE("SNS_SUID_MSGID_SNS_SUID_EVENT with incorrect SUID: 0x%" PRIx64
         " %" PRIx64, info->suid.suid_high, info->suid.suid_low);
  }

  SeeBufArg data;
  DynamicVector<sns_std_suid> suids;
  sns_suid_event event = {
    .data_type.funcs.decode = decodeStringField,
    .data_type.arg = &data,
    .suid.funcs.decode = decodeSnsSuidEventSuid,
    .suid.arg = &suids,
  };

  bool success = pb_decode(stream, sns_suid_event_fields, &event);
  if (!success) {
    LOGE("Error decoding sns_suid_event: %s", PB_GET_ERROR(stream));
  } else {
    // TODO: remove dataType once initial development is complete.
    char dataType[data.bufLen + 1];
    memcpy(dataType, data.buf, data.bufLen);
    dataType[data.bufLen] = '\0';

    // If syncData == nullptr, this indication is received outside of a sync
    // call. If the decoded data type doesn't match the one we are waiting
    // for, this indication is from a previous call (may be findSuidSync)
    // and happens to arrive between another sync req/ind pair.
    // Note that req/ind misalignment can still happen if findSuidSync is
    // called again with the same data type.
    // Note that there's no need to compare the SUIDs as no other calls
    // but findSuidSync populate mWaitingDataType and can lead to a data
    // type match.
    if (info->sync->syncData == nullptr
        || strncmp(info->sync->syncDataType, dataType,
                   std::min(data.bufLen, kSeeAttrStrValLen)) != 0) {
      LOGW("Received late SNS_SUID_MSGID_SNS_SUID_EVENT indication");
    } else {
      info->sync->syncIndFound = true;
      auto *outputSuids = static_cast<DynamicVector<sns_std_suid> *>(
          info->sync->syncData);
      for (const auto& suid : suids) {
        outputSuids->push_back(suid);
      }
    }
    LOGD("Finished sns_suid_event of data type '%s'", dataType);
  }
  return success;
}

/**
 * Decode messages defined in sns_suid.proto
 */
bool decodeSnsSuidProtoEvent(pb_istream_t *stream, const pb_field_t *field,
                             void **arg) {
  bool success = false;

  auto *info = static_cast<SeeInfoArg *>(*arg);
  switch (info->msgId) {
    case SNS_SUID_MSGID_SNS_SUID_EVENT:
      success = decodeSnsSuidEvent(stream, field, arg);
      break;

    default:
      LOGW("Unhandled sns_suid.proto msg ID: %" PRIu32, info->msgId);
      break;
  }
  return success;
}

/**
 * Defined in sns_std_sensor.pb.h
 */
const char *getAttrNameFromAttrId(int32_t id) {
  switch (id) {
    case SNS_STD_SENSOR_ATTRID_NAME:
      return "NAME";
    case SNS_STD_SENSOR_ATTRID_VENDOR:
      return "VENDOR";
    case SNS_STD_SENSOR_ATTRID_TYPE:
      return "TYPE";
    case SNS_STD_SENSOR_ATTRID_AVAILABLE:
      return "AVAILABLE";
    case SNS_STD_SENSOR_ATTRID_VERSION:
      return "VERSION";
    case SNS_STD_SENSOR_ATTRID_API:
      return "API";
    case SNS_STD_SENSOR_ATTRID_RATES:
      return "RATES";
    case SNS_STD_SENSOR_ATTRID_RESOLUTIONS:
      return "RESOLUTIONS";
    case SNS_STD_SENSOR_ATTRID_FIFO_SIZE:
      return "FIFO_SIZE";
    case SNS_STD_SENSOR_ATTRID_ACTIVE_CURRENT:
      return "ACTIVE_CURRENT";
    case SNS_STD_SENSOR_ATTRID_SLEEP_CURRENT:
      return "SLEEP_CURRENT";
    case SNS_STD_SENSOR_ATTRID_RANGES:
      return "RANGES";
    case SNS_STD_SENSOR_ATTRID_OP_MODES:
      return "OP_MODES";
    case SNS_STD_SENSOR_ATTRID_DRI:
      return "DRI";
    case SNS_STD_SENSOR_ATTRID_STREAM_SYNC:
      return "STREAM_SYNC";
    case SNS_STD_SENSOR_ATTRID_EVENT_SIZE:
      return "EVENT_SIZE";
    case SNS_STD_SENSOR_ATTRID_STREAM_TYPE:
      return "STREAM_TYPE";
    case SNS_STD_SENSOR_ATTRID_DYNAMIC:
      return "DYNAMIC";
    case SNS_STD_SENSOR_ATTRID_HW_ID:
      return "HW_ID";
    case SNS_STD_SENSOR_ATTRID_RIGID_BODY:
      return "RIGID_BODY";
    case SNS_STD_SENSOR_ATTRID_PLACEMENT:
      return "PLACEMENT";
    case SNS_STD_SENSOR_ATTRID_PHYSICAL_SENSOR:
      return "PHYSICAL_SENSOR";
    case SNS_STD_SENSOR_ATTRID_PHYSICAL_SENSOR_TESTS:
      return "PHYSICAL_SENSOR_TESTS";
    case SNS_STD_SENSOR_ATTRID_SELECTED_RESOLUTION:
      return "SELECTED_RESOLUTION";
    case SNS_STD_SENSOR_ATTRID_SELECTED_RANGE:
      return "SELECTED_RANGE";
    default:
      return "UNKNOWN ATTRIBUTE";
  }
}

/**
 * Decodes each attribute field and passes the value to the calling function.
 * For repeated fields of float or integers, only store the maximum and
 * minimum values for the calling function.
 */
bool decodeSnsStdAttrValue(pb_istream_t *stream, const pb_field_t *field,
                           void **arg) {
  SeeBufArg strData = {};
  SeeAttrArg subtypeAttrArg;
  sns_std_attr_value_data value = {
    .str.funcs.decode = decodeStringField,
    .str.arg = &strData,
    .subtype.values.funcs.decode = decodeSnsStdAttrValue,
    .subtype.values.arg = &subtypeAttrArg,
  };

  bool success = pb_decode(stream, sns_std_attr_value_data_fields, &value);
  if (!success) {
    LOGE("Error decoding sns_std_attr_value_data: %s", PB_GET_ERROR(stream));
  } else {
    auto *attrVal = static_cast<SeeAttrArg *>(*arg);
    if (value.has_flt) {
      // If this is a float (repeated) field, initialize the union as floats
      // to store the maximum and minmum values of the repeated fields.
      if (!attrVal->initialized) {
        attrVal->initialized = true;
        attrVal->fltMin = FLT_MAX;
        attrVal->fltMax = FLT_MIN;
      }
      if (value.flt < attrVal->fltMin) {
        attrVal->fltMin = value.flt;
      }
      if (value.flt > attrVal->fltMax) {
        attrVal->fltMax = value.flt;
      }
    } else if (value.has_sint) {
      attrVal->int64 = value.sint;
    } else if (value.has_boolean) {
      attrVal->boolVal = value.boolean;
    } else if (strData.buf != nullptr) {
      strlcpy(attrVal->strVal, static_cast<const char *>(strData.buf),
              sizeof(attrVal->strVal));
    } else if (!value.has_subtype) {
      LOGW("Unknown attr type");
    }
  }
  return success;
}

bool decodeSnsStrAttr(pb_istream_t *stream, const pb_field_t *field,
                      void **arg) {
  SeeAttrArg attrArg = {};
  sns_std_attr attr = {
    .value.values.funcs.decode = decodeSnsStdAttrValue,
    .value.values.arg = &attrArg,
  };

  bool success = pb_decode(stream, sns_std_attr_fields, &attr);
  if (!success) {
    LOGE("Error decoding sns_std_attr: %s", PB_GET_ERROR(stream));
  } else {
    auto *attrData = static_cast<SeeAttributes *>(*arg);
    if (attr.attr_id == SNS_STD_SENSOR_ATTRID_NAME) {
      strlcpy(attrData->name, attrArg.strVal, sizeof(attrData->name));
    } else if (attr.attr_id == SNS_STD_SENSOR_ATTRID_VENDOR) {
      strlcpy(attrData->vendor, attrArg.strVal, sizeof(attrData->vendor));
    } else if (attr.attr_id == SNS_STD_SENSOR_ATTRID_TYPE) {
      LOGI("%s: '%s'", getAttrNameFromAttrId(attr.attr_id), attrArg.strVal);
      strlcpy(attrData->type, attrArg.strVal, sizeof(attrData->type));
    } else if (attr.attr_id == SNS_STD_SENSOR_ATTRID_RATES) {
      attrData->maxSampleRate = attrArg.fltMax;
    } else if (attr.attr_id == SNS_STD_SENSOR_ATTRID_STREAM_SYNC) {
      attrData->streamType = attrArg.int64;
    }
  }
  return success;
}

bool decodeSnsStdAttrEvent(pb_istream_t *stream, const pb_field_t *field,
                           void **arg) {
  SeeAttributes attr;
  sns_std_attr_event event = {
    .attributes.funcs.decode = decodeSnsStrAttr,
    .attributes.arg = &attr,
  };

  bool success = pb_decode(stream, sns_std_attr_event_fields, &event);
  if (!success) {
    LOGE("Error decoding sns_std_attr_event: %s", PB_GET_ERROR(stream));
  } else {
    auto *info = static_cast<SeeInfoArg *>(*arg);

    // If syncData == nullptr, this indication is received outside of a sync
    // call. If the decoded SUID doesn't match the one we are waiting for,
    // this indication is from a previous getAttributes call and happens to
    // arrive between a later findAttributesSync req/ind pair.
    // Note that req/ind misalignment can still happen if getAttributesSync is
    // called again with the same SUID.
    if (info->sync->syncData == nullptr
        || !suidsMatch(info->suid, info->sync->syncSuid)) {
      LOGW("Received late SNS_STD_MSGID_SNS_STD_ATTR_EVENT indication");
    } else {
      info->sync->syncIndFound = true;
      memcpy(info->sync->syncData, &attr, sizeof(attr));
    }
  }
  return success;
}

/**
 * Decode messages defined in sns_std.proto
 */
bool decodeSnsStdProtoEvent(pb_istream_t *stream, const pb_field_t *field,
                            void **arg) {
  bool success = false;

  auto *info = static_cast<SeeInfoArg *>(*arg);
  switch (info->msgId) {
    case SNS_STD_MSGID_SNS_STD_ATTR_EVENT:
      success = decodeSnsStdAttrEvent(stream, field, arg);
      break;

    case SNS_STD_MSGID_SNS_STD_FLUSH_EVENT:
      // An empty message.
      success = true;
      break;

    case SNS_STD_MSGID_SNS_STD_ERROR_EVENT: {
      sns_std_error_event event = {};
      success = pb_decode(stream, sns_std_error_event_fields, &event);
      if (!success) {
        LOGE("Error decoding sns_std_error_event: %s", PB_GET_ERROR(stream));
      } else {
        LOGW("SNS_STD_MSGID_SNS_STD_ERROR_EVENT: %d", event.error);
      }
      break;
    }

    default:
      LOGW("Unhandled sns_std.proto msg ID %" PRIu32, info->msgId);
  }
  return success;
}

void populateEventSample(SeeDataArg *data, const float *val) {
  size_t index = data->sampleIndex;
  if (!data->event.isNull() && index < data->totalSamples) {
    SensorSampleType sampleType = getSensorSampleTypeFromSensorType(
        data->sensorType);

    uint32_t *timestampDelta = nullptr;
    switch (sampleType) {
      case SensorSampleType::ThreeAxis: {
        auto *event = reinterpret_cast<chreSensorThreeAxisData *>(
            data->event.get());
        event->readings[index].x = val[0];
        event->readings[index].y = val[1];
        event->readings[index].z = val[2];
        timestampDelta = &event->readings[index].timestampDelta;
        break;
      }

      case SensorSampleType::Float: {
        auto *event = reinterpret_cast<chreSensorFloatData *>(
            data->event.get());
        event->readings[index].value = val[0];
        timestampDelta = &event->readings[index].timestampDelta;
        break;
      }

      case SensorSampleType::Byte: {
        auto *event = reinterpret_cast<chreSensorByteData *>(data->event.get());
        event->readings[index].value = 0;
        event->readings[index].isNear = (val[0] > 0.5f);
        timestampDelta = &event->readings[index].timestampDelta;
        break;
      }

      case SensorSampleType::Occurrence: {
        auto *event = reinterpret_cast<chreSensorOccurrenceData *>(
            data->event.get());
        timestampDelta = &event->readings[index].timestampDelta;
        break;
      }

      default:
        LOGE("Invalid sample type %" PRIu8, static_cast<uint8_t>(sampleType));
    }

    if (data->sampleIndex == 0) {
      auto *header = reinterpret_cast<chreSensorDataHeader *>(
          data->event.get());
      header->baseTimestamp = data->timeNs;
      *timestampDelta = 0;
    } else {
      *timestampDelta = static_cast<uint32_t>(data->timeNs - data->prevTimeNs);
    }
    data->prevTimeNs = data->timeNs;
  }
}

/**
 * Decodes a float array and ensures that the data doesn't go out of bound.
 */
bool decodeFloatData(pb_istream_t *stream, const pb_field_t *field,
                     void **arg) {
  auto *data = static_cast<SeeFloatArg *>(*arg);

  float value;
  float *fltPtr = &value;
  if (data->index >= ARRAY_SIZE(data->val)) {
    LOGE("Float array length exceeds %zu", ARRAY_SIZE(data->val));
  } else {
    // Decode to the provided array only if it doesn't go out of bound.
    fltPtr = &(data->val[data->index]);
  }
  // Increment index whether it's gone out of bounds or not.
  (data->index)++;

  bool success = pb_decode_fixed32(stream, fltPtr);
  if (!success) {
    LOGE("Error decoding float data: %s", PB_GET_ERROR(stream));
  }
  return success;
}

bool decodeSnsStdSensorPhysicalConfigEvent(
    pb_istream_t *stream, const pb_field_t *field, void **arg) {
  sns_std_sensor_physical_config_event event = {};

  bool success = pb_decode(stream, sns_std_sensor_physical_config_event_fields,
                           &event);
  if (!success) {
    LOGE("Error decoding sns_std_sensor_physical_config_event: %s",
         PB_GET_ERROR(stream));
  } else {
    // TODO: handle the sample rate update.
    LOGD("Sample rate: %f", event.sample_rate);
  }
  return success;
}

bool decodeSnsStdSensorEvent(pb_istream_t *stream, const pb_field_t *field,
                             void **arg) {
  SeeFloatArg sample = {};
  sns_std_sensor_event event = {
    .data.funcs.decode = decodeFloatData,
    .data.arg = &sample,
  };

  bool success = pb_decode(stream, sns_std_sensor_event_fields, &event);
  if (!success) {
    LOGE("Error decoding sns_std_sensor_event: %s", PB_GET_ERROR(stream));
  } else {
    auto *info = static_cast<SeeInfoArg *>(*arg);
    populateEventSample(info->data, sample.val);
  }
  return success;
}

/**
 * Decode messages defined in sns_std_sensor.proto
 */
bool decodeSnsStdSensorProtoEvent(pb_istream_t *stream, const pb_field_t *field,
                                  void **arg) {
  bool success = false;

  auto *info = static_cast<SeeInfoArg *>(*arg);
  switch (info->msgId) {
    case SNS_STD_SENSOR_MSGID_SNS_STD_SENSOR_PHYSICAL_CONFIG_EVENT:
      success = decodeSnsStdSensorPhysicalConfigEvent(stream, field, arg);
      break;

    case SNS_STD_SENSOR_MSGID_SNS_STD_SENSOR_EVENT:
      success = decodeSnsStdSensorEvent(stream, field, arg);
      break;

    default:
      LOGW("Unhandled sns_std_sensor.proto msg ID %" PRIu32, info->msgId);
  }
  return success;
}

bool decodeSnsCalEvent(pb_istream_t *stream, const pb_field_t *field,
                       void **arg) {
  bool success = false;

  SeeFloatArg offset = {};
  SeeFloatArg scale = {};
  SeeFloatArg matrix = {};
  sns_cal_event event = {
    .bias.funcs.decode = decodeFloatData,
    .bias.arg = &offset,
    .scale_factor.funcs.decode = decodeFloatData,
    .scale_factor.arg = &scale,
    .comp_matrix.funcs.decode = decodeFloatData,
    .comp_matrix.arg = &matrix,
  };

  success = pb_decode(stream, sns_cal_event_fields, &event);
  if (!success) {
    LOGE("Error decoding sns_cal_event: %s", PB_GET_ERROR(stream));
  } else {
    // TODO: handle cal data.
    LOGD("Bias L=%zu: %f %f %f",
         offset.index, offset.val[0], offset.val[1], offset.val[2]);
    LOGD("Scale L=%zu: %f %f %f",
         scale.index, scale.val[0], scale.val[1], scale.val[2]);
    LOGD("Matrix L=%zu: %f %f %f, %f %f %f, %f %f %f",
         matrix.index, matrix.val[0], matrix.val[1], matrix.val[2],
         matrix.val[3], matrix.val[4], matrix.val[5],
         matrix.val[6], matrix.val[7], matrix.val[8]);
  }
  return success;
}

/**
 * Decode messages defined in sns_cal.proto
 */
bool decodeSnsCalProtoEvent(pb_istream_t *stream, const pb_field_t *field,
                            void **arg) {
  bool success = false;

  auto *info = static_cast<SeeInfoArg *>(*arg);
  switch (info->msgId) {
    case SNS_CAL_MSGID_SNS_CAL_EVENT:
      success = decodeSnsCalEvent(stream, field, arg);
      break;

    default:
      LOGW("Unhandled sns_cal.proto msg ID %" PRIu32, info->msgId);
  }
  return success;
}

bool assignPayloadCallback(const SeeInfoArg *info, pb_callback_t *payload) {
  bool success = true;

  payload->arg = const_cast<SeeInfoArg *>(info);

  switch (info->msgId) {
    case SNS_SUID_MSGID_SNS_SUID_EVENT:
      // TODO: remove the SUID comparison once b/69456964 is resolved.
      if (suidsMatch(info->suid, kSuidLookup)) {
        payload->funcs.decode = decodeSnsSuidProtoEvent;
      } else {
        payload->funcs.decode = decodeSnsStdSensorProtoEvent;
      }
      break;

    case SNS_STD_MSGID_SNS_STD_ATTR_EVENT:
    case SNS_STD_MSGID_SNS_STD_FLUSH_EVENT:
    case SNS_STD_MSGID_SNS_STD_ERROR_EVENT:
      payload->funcs.decode = decodeSnsStdProtoEvent;
      break;

    // TODO: uncomment this case once b/69456964 is resolved.
    //case SNS_STD_SENSOR_MSGID_SNS_STD_SENSOR_PHYSICAL_CONFIG_EVENT:
    case SNS_STD_SENSOR_MSGID_SNS_STD_SENSOR_EVENT:
      payload->funcs.decode = decodeSnsStdSensorProtoEvent;
      break;

    case SNS_CAL_MSGID_SNS_CAL_EVENT:
      payload->funcs.decode = decodeSnsCalProtoEvent;
      break;

    default:
      success = false;
      LOGW("Unhandled msg ID %" PRIu32, info->msgId);
  }
  return success;
}

/**
 * Decodes only msg_id and timestamp defined in sns_client_event and converts
 * the timestamp to nanoseconds.
 */
bool decodeMsgIdAndTime(pb_istream_t *stream, uint32_t *msgId,
                        uint64_t *timeNs) {
  sns_client_event_msg_sns_client_event event = {};

  bool success = pb_decode(
      stream, sns_client_event_msg_sns_client_event_fields, &event);
  if (!success) {
    LOGE("Error decoding msg ID and timestamp: %s", PB_GET_ERROR(stream));
  } else {
    *msgId = event.msg_id;
    *timeNs = getNanosecondsFromQTimerTicks(event.timestamp);
  }
  return success;
}

/**
 * Decodes pb-encoded message
 */
bool decodeSnsClientEventMsg(pb_istream_t *stream, const pb_field_t *field,
                             void **arg) {
  // Make a copy for data decoding.
  pb_istream_t streamCpy = *stream;

  auto *info = static_cast<SeeInfoArg *>(*arg);
  bool success = decodeMsgIdAndTime(stream, &info->msgId, &info->data->timeNs);

  if (!info->decodeMsgIdOnly) {
    sns_client_event_msg_sns_client_event event = {};

    // Payload callback must be assigned if and only if we want to decode beyond
    // msg ID.
    success = assignPayloadCallback(info, &event.payload);
    if (!success) {
      LOGE("No pb callback assigned");
    } else {
      success = pb_decode(&streamCpy,
                          sns_client_event_msg_sns_client_event_fields, &event);
      if (!success) {
        LOGE("Error decoding sns_client_event_msg_sns_client_event: %s",
             PB_GET_ERROR(&streamCpy));
      }
    }
  }

  // Increment sample count only after sensor event decoding.
  if (success && info->msgId == SNS_STD_SENSOR_MSGID_SNS_STD_SENSOR_EVENT) {
    info->data->sampleIndex++;
  }
  return success;
}

/**
 * Obtain the SensorType from the list of registered SUIDs.
 */
SensorType getSensorTypeFromSuid(
    const sns_std_suid& suid,
    const DynamicVector<SeeHelper::SuidSensorType>& suids) {
  for (size_t i = 0; i < suids.size(); i++) {
    if (suidsMatch(suid, suids[i].suid)) {
        return suids[i].sensorType;
    }
  }
  return SensorType::Unknown;
}

/**
 * Allocate event memory according to SensorType and the number of samples.
 */
void *allocateEvent(SensorType sensorType, size_t numSamples) {
  SensorSampleType sampleType = getSensorSampleTypeFromSensorType(sensorType);
  size_t sampleSize = 0;
  switch (sampleType) {
    case SensorSampleType::ThreeAxis:
      sampleSize = sizeof(
          chreSensorThreeAxisData::chreSensorThreeAxisSampleData);
      break;

    case SensorSampleType::Float:
      sampleSize = sizeof(
          chreSensorFloatData::chreSensorFloatSampleData);
      break;

    case SensorSampleType::Byte:
      sampleSize = sizeof(
          chreSensorByteData::chreSensorByteSampleData);
      break;

    case SensorSampleType::Occurrence:
      sampleSize = sizeof(
          chreSensorOccurrenceData::chreSensorOccurrenceSampleData);
      break;

    default:
      LOGW("Unhandled SensorSampleType %" PRIu8,
           static_cast<uint8_t>(sampleType));
  }

  size_t memorySize = (sampleType == SensorSampleType::Unknown)
      ? 0 : (sizeof(chreSensorDataHeader) + numSamples * sampleSize);
  return memoryAlloc(memorySize);
}

// Allocates the sensor event memory and partially populates the header.
bool prepareSensorEvent(SeeInfoArg& info,
                        const DynamicVector<SeeHelper::SuidSensorType>& suids) {
  bool success = false;

  info.data->sensorType = getSensorTypeFromSuid(info.suid, suids);
  UniquePtr<uint8_t> buf(static_cast<uint8 *>(
      allocateEvent(info.data->sensorType, info.data->sampleIndex)));
  info.data->event = std::move(buf);

  if (info.data->event.isNull()) {
    LOGE("Failed to allocate sensor event memory");
  } else {
    success = true;

    info.data->prevTimeNs = 0;

    auto *header = reinterpret_cast<chreSensorDataHeader *>(
        info.data->event.get());
    memset(header->reserved, 0, sizeof(header->reserved));
    header->sensorHandle = getSensorHandleFromSensorType(
        info.data->sensorType);
    header->readingCount = info.data->sampleIndex;

    // Protect against out of bounds access in data decoding.
    info.data->totalSamples = info.data->sampleIndex;

    // Reset sampleIndex only after memory has been allocated and header
    // populated.
    info.data->sampleIndex = 0;
  }
  return success;
}

}  // anonymous

void SeeHelper::handleSnsClientEventMsg(const void *payload,
                                        size_t payloadLen) {
  CHRE_ASSERT(payload);

  pb_istream_t stream = pb_istream_from_buffer(
      static_cast<const pb_byte_t *>(payload), payloadLen);

  // Make a copy of the stream for sensor data decoding.
  pb_istream_t streamCpy = stream;

  // Only initialize fields that are not accessed in the main CHRE thread.
  SeeSyncArg syncArg = {};
  SeeDataArg dataArg = {};
  SeeInfoArg info = {
    .sync = &syncArg,
    .data = &dataArg,
    .decodeMsgIdOnly = true,
  };
  sns_client_event_msg event = {
    .events.funcs.decode = decodeSnsClientEventMsg,
    .events.arg = &info,
  };

  // Decode SUID and MSG ID only to help further decode.
  bool success = pb_decode(&stream, sns_client_event_msg_fields, &event);
  if (success) {
    info.suid = event.suid;
    info.decodeMsgIdOnly = false;

    mMutex.lock();
    bool synchronizedDecode = mWaiting;
    if (!synchronizedDecode) {
      // Early unlock, we're not going to use anything from the main thread.
      mMutex.unlock();
    } else {
      // Populate fields set by the main thread.
      info.sync->syncData = mSyncData;
      info.sync->syncDataType = mSyncDataType;
      info.sync->syncSuid = mSyncSuid;
    }

    if (info.data->sampleIndex > 0) {
      if (!prepareSensorEvent(info, mSuids)) {
        LOGE("Failed to prepare sensor event");
      }
    }

    success = pb_decode(&streamCpy, sns_client_event_msg_fields, &event);
    if (!success) {
      LOGE("Error decoding sns_client_event_msg: %s", PB_GET_ERROR(&streamCpy));
    } else if (synchronizedDecode && info.sync->syncIndFound) {
      mWaiting = false;
      mCond.notify_one();
    } else if (info.data->sampleIndex > 0 && mIndCb != nullptr) {
      mIndCb(info.data->sensorType, std::move(info.data->event));
    }

    if (synchronizedDecode) {
      mMutex.unlock();
    }
  }
}

bool SeeHelper::findSuidSync(const char *dataType,
                             DynamicVector<sns_std_suid> *suids) {
  CHRE_ASSERT(suids);
  bool success = false;

  if (mQmiHandle == nullptr) {
    LOGE("Sensor client service QMI client wasn't initialized");
  } else {
    suids->clear();

    UniquePtr<pb_byte_t> msg;
    size_t msgLen;
    success = encodeSnsSuidReq(dataType, &msg, &msgLen);
    if (success) {
      // TODO: modify retry implementation  when b/69066253 is resolved.
      // Sensor client QMI service may come up before SEE sensors are
      // enumerated. A max dwell time is set and retries are performed as
      // currently there's no message indicating that SEE intialization is
      // complete.
      constexpr time_timetick_type kSuidReqIntervalUsec =
          kSuidReqIntervalMsec.getMicroseconds();
      constexpr uint32_t kSuidReqMaxTrialCount =
          kSuidReqMaxDwellSec.toRawNanoseconds()
          / kSuidReqIntervalMsec.toRawNanoseconds();

      uint32_t trialCount = 0;
      do {
        if (++trialCount > 1) {
          timer_sleep(kSuidReqIntervalUsec, T_USEC, true /* non_deferrable */);
        }
        success = sendReq(sns_suid_sensor_init_default, suids, dataType,
                          SNS_SUID_MSGID_SNS_SUID_REQ, msg.get(), msgLen,
                          false /* batchValid */, 0 /* batchPeriodUs */,
                          true /* waitForIndication */);
      } while (suids->empty() && trialCount < kSuidReqMaxTrialCount);
      if (trialCount > 1) {
        LOGD("%" PRIu32 " trials took %" PRIu32 " msec", trialCount,
             static_cast<uint32_t>(
                 trialCount * kSuidReqIntervalMsec.getMilliseconds()));
      }
    }
  }
  return success;
}

bool SeeHelper::getAttributesSync(const sns_std_suid& suid,
                                  SeeAttributes *attr) {
  CHRE_ASSERT(attr);
  bool success = false;

  if (mQmiHandle == nullptr) {
    LOGE("Sensor client service QMI client wasn't initialized");
  } else {
    UniquePtr<pb_byte_t> msg;
    size_t msgLen;
    success = encodeSnsStdAttrReq(&msg, &msgLen);

    if (success) {
      success = sendReq(suid, attr, nullptr /* syncDataType */,
                        SNS_STD_MSGID_SNS_STD_ATTR_REQ, msg.get(), msgLen,
                        false /* batchValid */, 0 /* batchPeriodUs */,
                        true /* waitForIndication */);
    }
  }
  return success;
}

bool SeeHelper::init(SeeIndCallback *indCb, Microseconds timeout) {
  bool success = false;

  mIndCb = indCb;
  if (indCb == nullptr) {
    LOGW("SeeHelper indication callback not provided");
  }

  qmi_idl_service_object_type snsSvcObj =
      SNS_CLIENT_SVC_get_service_object_v01();
  if (snsSvcObj == nullptr) {
    LOGE("Failed to obtain the sensor client service instance");
  } else {
    qmi_client_os_params sensorContextOsParams;
    qmi_client_error_type status = qmi_client_init_instance(
        snsSvcObj, QMI_CLIENT_INSTANCE_ANY, SeeHelper::qmiIndCb, this,
        &sensorContextOsParams, timeout.getMicroseconds(), &mQmiHandle);
    if (status != QMI_NO_ERR) {
      LOGE("Failed to initialize the sensor client service QMI client: %d",
           status);
    }
  }
  return success;
}

bool SeeHelper::makeRequest(const SeeSensorRequest& request) {
  bool success = false;

  if (mQmiHandle == nullptr) {
    LOGE("Sensor client service QMI client wasn't initialized");
  } else {
    uint32_t msgId;
    UniquePtr<pb_byte_t> msg;
    size_t msgLen = 0;

    if (!request.enable) {
      // An empty message
      msgId = SNS_CLIENT_MSGID_SNS_CLIENT_DISABLE_REQ;
      success = true;
    } else if (request.continuous) {
      msgId = SNS_STD_SENSOR_MSGID_SNS_STD_SENSOR_CONFIG;
      success = encodeSnsStdSensorConfig(request, &msg, &msgLen);
    } else {
      msgId = SNS_STD_SENSOR_MSGID_SNS_STD_ON_CHANGE_CONFIG;
      // TODO: Implement non-continuous sensors.
      success = false;
    }

    if (success) {
      success = sendReq(request.suid,
                        nullptr /* syncData */, nullptr /* syncDatType */,
                        msgId, msg.get(), msgLen,
                        true /* batchValid */, request.batchPeriodUs,
                        false /* waitForIndication */);
    }
  }
  return success;
}

bool SeeHelper::deinit() {
  qmi_client_error_type status = qmi_client_release(mQmiHandle);
  if (status != QMI_NO_ERR) {
    LOGE("Failed to release sensor client service QMI client: %d", status);
  }

  mQmiHandle = nullptr;
  mSuids.clear();
  return (status == QMI_NO_ERR);
}

bool SeeHelper::sendReq(
    const sns_std_suid& suid, void *syncData, const char *syncDataType,
    uint32_t msgId, void *payload, size_t payloadLen,
    bool batchValid, uint32_t batchPeriodUs,
    bool waitForIndication, Nanoseconds timeoutResp, Nanoseconds timeoutInd) {
  CHRE_ASSERT(payload || payloadLen == 0);
  bool success = false;

  // Force big image as the future QMI-replacement interface may not be
  // supported in micro-image.
  slpiForceBigImage();

  if (!waitForIndication) {
    success = sendSnsClientReq(mQmiHandle, suid, msgId, payload, payloadLen,
                               batchValid, batchPeriodUs, timeoutResp);
  } else {
    LockGuard<Mutex> lock(mMutex);
    CHRE_ASSERT(!mWaiting);

    // Specify members needed for a sync call.
    mSyncSuid = suid;
    mSyncData = syncData;
    mSyncDataType = syncDataType;

    success = sendSnsClientReq(mQmiHandle, suid, msgId, payload, payloadLen,
                               batchValid, batchPeriodUs, timeoutResp);

    if (success) {
      bool waitSuccess = true;
      mWaiting = true;

      while (mWaiting && waitSuccess) {
        waitSuccess = mCond.wait_for(mMutex, timeoutInd);
      }

      if (!waitSuccess) {
        LOGE("QMI indication timed out after %" PRIu64 " ms",
             Milliseconds(timeoutInd).getMilliseconds());
        success = false;
        mWaiting = false;
      }
    }

    // Reset members needed for a sync call.
    mSyncSuid = sns_suid_sensor_init_zero;
    mSyncData = nullptr;
    mSyncDataType = nullptr;
  }
  return success;
}

void SeeHelper::handleInd(qmi_client_type clientHandle, unsigned int msgId,
                          const void *indBuf, unsigned int indBufLen) {
  CHRE_ASSERT(indBuf || indBufLen == 0);

  switch (msgId) {
    case SNS_CLIENT_REPORT_IND_V01: {
      // Decode sns_client_report_ind_msg_v01 to extract pb-encoded payload.
      auto ind = MakeUnique<sns_client_report_ind_msg_v01>();

      if (ind.isNull()) {
        LOGE("Failed to allocate memory for sns_client_report_ind_msg_v01");
      } else {
        int status = qmi_client_message_decode(
            clientHandle, QMI_IDL_INDICATION, SNS_CLIENT_REPORT_IND_V01,
            indBuf, indBufLen, ind.get(), sizeof(*ind));
        if (status != QMI_NO_ERR) {
          LOGE("Error parsing SNS_CLIENT_REPORT_IND_V01: %d", status);
        } else {
          handleSnsClientEventMsg(ind->payload, ind->payload_len);
        }
      }
      break;
    }

    default:
      // TODO: handle sns_client_jumbo_report_ind_msg_v01.
      LOGW("Unhandled sns_client_api_v01 msg ID %u", msgId);
  }
}

void SeeHelper::qmiIndCb(qmi_client_type clientHandle, unsigned int msgId,
                         void *indBuf, unsigned int indBufLen,
                         void *indCbData) {
  auto *obj = static_cast<SeeHelper *>(indCbData);
  obj->handleInd(clientHandle, msgId, indBuf, indBufLen);
}

bool SeeHelper::registerSuid(const sns_std_suid& suid, SensorType sensorType) {
  bool success = true;

  bool found = false;
  for (SuidSensorType suidSensorType : mSuids) {
    if (suidsMatch(suid, suidSensorType.suid)) {
      found = true;
      break;
    }
  }

  // Add a new entry only if this SUID hasn't been registered.
  if (!found) {
    SuidSensorType suidSensorType = {
      .suid = suid,
      .sensorType = sensorType,
    };
    success = mSuids.push_back(suidSensorType);
  }
  return success;
}

}  // namespace chre
