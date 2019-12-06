/*
 * Copyright (C) 2019 The Android Open Source Project
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

#include "chre/platform/platform_sensor_type_helpers.h"

#include "chre/platform/assert.h"

#ifdef CHREX_SENSOR_SUPPORT
#include "chre/extensions/platform/vendor_sensor_types.h"
#endif  // CHREX_SENSOR_SUPPORT

namespace chre {

ReportingMode PlatformSensorTypeHelpers::getVendorSensorReportingMode(
    uint8_t sensorType) {
#ifdef CHREX_SENSOR_SUPPORT
  if (extension::vendorSensorTypeIsOneShot(sensorType)) {
    return ReportingMode::OneShot;
  }
  if (extension::vendorSensorTypeIsOnChange(sensorType)) {
    return ReportingMode::OnChange;
  }
#endif
  return ReportingMode::Continuous;
}

bool PlatformSensorTypeHelpers::getVendorSensorIsCalibrated(
    uint8_t /* sensorType */) {
  return false;
}

bool PlatformSensorTypeHelpers::getVendorSensorBiasEventType(
    uint8_t sensorType, uint16_t *eventType) {
#ifdef CHREX_SENSOR_SUPPORT
  return extension::vendorGetSensorBiasEventType(sensorType, eventType);
#else
  return false;
#endif
}

const char *PlatformSensorTypeHelpersBase::getSensorTypeName(
    uint8_t sensorType) {
  switch (sensorType) {
    case CHRE_SENSOR_TYPE_INVALID:
      return "Unknown";
    case CHRE_SENSOR_TYPE_ACCELEROMETER:
      return "Accelerometer";
    case CHRE_SENSOR_TYPE_INSTANT_MOTION_DETECT:
      return "Instant Motion";
    case CHRE_SENSOR_TYPE_STATIONARY_DETECT:
      return "Stationary Detect";
    case CHRE_SENSOR_TYPE_GYROSCOPE:
      return "Gyroscope";
    case CHRE_SENSOR_TYPE_GEOMAGNETIC_FIELD:
      return "Geomagnetic Field";
    case CHRE_SENSOR_TYPE_PRESSURE:
      return "Pressure";
    case CHRE_SENSOR_TYPE_LIGHT:
      return "Light";
    case CHRE_SENSOR_TYPE_PROXIMITY:
      return "Proximity";
    case CHRE_SENSOR_TYPE_STEP_DETECT:
      return "Step Detect";
    case CHRE_SENSOR_TYPE_ACCELEROMETER_TEMPERATURE:
      return "Accelerometer Temp";
    case CHRE_SENSOR_TYPE_GYROSCOPE_TEMPERATURE:
      return "Gyroscope Temp";
    case CHRE_SENSOR_TYPE_GEOMAGNETIC_FIELD_TEMPERATURE:
      return "Geomagnetic Field Temp";
    case CHRE_SENSOR_TYPE_UNCALIBRATED_ACCELEROMETER:
      return "Uncal Accelerometer";
    case CHRE_SENSOR_TYPE_UNCALIBRATED_GYROSCOPE:
      return "Uncal Gyroscope";
    case CHRE_SENSOR_TYPE_UNCALIBRATED_GEOMAGNETIC_FIELD:
      return "Uncal Geomagnetic Field";
    // TODO: Map this to a vendor provided by an extension similar to
    // vendorGetSensorBiasEventType.
    case CHRE_VENDOR_SENSOR_TYPE(0):
      return "Vendor Type 0";
    case CHRE_VENDOR_SENSOR_TYPE(1):
      return "Vendor Type 1";
    case CHRE_VENDOR_SENSOR_TYPE(2):
      return "Vendor Type 2";
    case CHRE_VENDOR_SENSOR_TYPE(3):
      return "Vendor Type 3";
    case CHRE_VENDOR_SENSOR_TYPE(4):
      return "Vendor Type 4";
    case CHRE_VENDOR_SENSOR_TYPE(5):
      return "Vendor Type 5";
    case CHRE_VENDOR_SENSOR_TYPE(6):
      return "Vendor Type 6";
    case CHRE_VENDOR_SENSOR_TYPE(7):
      return "Vendor Type 7";
    case CHRE_VENDOR_SENSOR_TYPE(8):
      return "Vendor Type 8";
    default:
      CHRE_ASSERT(false);
      return "";
  }
}

uint8_t PlatformSensorTypeHelpersBase::getTempSensorType(uint8_t sensorType) {
  switch (sensorType) {
    case CHRE_SENSOR_TYPE_ACCELEROMETER:
    case CHRE_SENSOR_TYPE_UNCALIBRATED_ACCELEROMETER:
      return CHRE_SENSOR_TYPE_ACCELEROMETER_TEMPERATURE;
    case CHRE_SENSOR_TYPE_GYROSCOPE:
    case CHRE_SENSOR_TYPE_UNCALIBRATED_GYROSCOPE:
      return CHRE_SENSOR_TYPE_GYROSCOPE_TEMPERATURE;
    case CHRE_SENSOR_TYPE_GEOMAGNETIC_FIELD:
    case CHRE_SENSOR_TYPE_UNCALIBRATED_GEOMAGNETIC_FIELD:
      return CHRE_SENSOR_TYPE_GEOMAGNETIC_FIELD_TEMPERATURE;
    default:
      return CHRE_SENSOR_TYPE_INVALID;
  }
}

SensorSampleType
PlatformSensorTypeHelpersBase::getSensorSampleTypeFromSensorType(
    uint8_t sensorType) {
  switch (sensorType) {
    case CHRE_SENSOR_TYPE_ACCELEROMETER:
    case CHRE_SENSOR_TYPE_GYROSCOPE:
    case CHRE_SENSOR_TYPE_GEOMAGNETIC_FIELD:
    case CHRE_SENSOR_TYPE_UNCALIBRATED_ACCELEROMETER:
    case CHRE_SENSOR_TYPE_UNCALIBRATED_GYROSCOPE:
    case CHRE_SENSOR_TYPE_UNCALIBRATED_GEOMAGNETIC_FIELD:
      return SensorSampleType::ThreeAxis;
    case CHRE_SENSOR_TYPE_PRESSURE:
    case CHRE_SENSOR_TYPE_LIGHT:
    case CHRE_SENSOR_TYPE_ACCELEROMETER_TEMPERATURE:
    case CHRE_SENSOR_TYPE_GYROSCOPE_TEMPERATURE:
    case CHRE_SENSOR_TYPE_GEOMAGNETIC_FIELD_TEMPERATURE:
      return SensorSampleType::Float;
    case CHRE_SENSOR_TYPE_INSTANT_MOTION_DETECT:
    case CHRE_SENSOR_TYPE_STATIONARY_DETECT:
    case CHRE_SENSOR_TYPE_STEP_DETECT:
      return SensorSampleType::Occurrence;
    case CHRE_SENSOR_TYPE_PROXIMITY:
      return SensorSampleType::Byte;
#ifdef CHREX_SENSOR_SUPPORT
    case CHRE_VENDOR_SENSOR_TYPE(0):
      return SensorSampleType::Vendor0;
    case CHRE_VENDOR_SENSOR_TYPE(1):
      return SensorSampleType::Vendor1;
    case CHRE_VENDOR_SENSOR_TYPE(2):
      return SensorSampleType::Vendor2;
    case CHRE_SLPI_SENSOR_TYPE_BIG_IMAGE_ACCEL:
      return SensorSampleType::Vendor3;
    case CHRE_VENDOR_SENSOR_TYPE(4):
      return SensorSampleType::Vendor4;
    case CHRE_VENDOR_SENSOR_TYPE(5):
      return SensorSampleType::Vendor5;
    case CHRE_SLPI_SENSOR_TYPE_BIG_IMAGE_UNCAL_ACCEL:
      return SensorSampleType::Vendor6;
    case CHRE_SLPI_SENSOR_TYPE_BIG_IMAGE_UNCAL_GYRO:
      return SensorSampleType::Vendor7;
    case CHRE_SLPI_SENSOR_TYPE_BIG_IMAGE_UNCAL_MAG:
      return SensorSampleType::Vendor8;
#endif  // CHREX_SENSOR_SUPPORT
    default:
      // Update implementation to prevent undefined from being used.
      CHRE_ASSERT(false);
      return SensorSampleType::Unknown;
  }
}

uint8_t PlatformSensorTypeHelpersBase::toCalibratedSensorType(
    uint8_t sensorType) {
  switch (sensorType) {
    case CHRE_SENSOR_TYPE_UNCALIBRATED_ACCELEROMETER:
      return CHRE_SENSOR_TYPE_ACCELEROMETER;
    case CHRE_SENSOR_TYPE_UNCALIBRATED_GYROSCOPE:
      return CHRE_SENSOR_TYPE_GYROSCOPE;
    case CHRE_SENSOR_TYPE_UNCALIBRATED_GEOMAGNETIC_FIELD:
      return CHRE_SENSOR_TYPE_GEOMAGNETIC_FIELD;
    default:
      /* empty */
      break;
  }

  return sensorType;
}

uint8_t PlatformSensorTypeHelpersBase::toUncalibratedSensorType(
    uint8_t sensorType) {
  switch (sensorType) {
    case CHRE_SENSOR_TYPE_ACCELEROMETER:
      return CHRE_SENSOR_TYPE_UNCALIBRATED_ACCELEROMETER;
    case CHRE_SENSOR_TYPE_GYROSCOPE:
      return CHRE_SENSOR_TYPE_UNCALIBRATED_GYROSCOPE;
    case CHRE_SENSOR_TYPE_GEOMAGNETIC_FIELD:
      return CHRE_SENSOR_TYPE_UNCALIBRATED_GEOMAGNETIC_FIELD;
    default:
      /* empty */
      break;
  }

  return sensorType;
}

bool PlatformSensorTypeHelpersBase::reportsBias(uint8_t sensorType) {
  switch (sensorType) {
    case CHRE_SENSOR_TYPE_ACCELEROMETER:
    case CHRE_SENSOR_TYPE_GYROSCOPE:
    case CHRE_SENSOR_TYPE_GEOMAGNETIC_FIELD:
    case CHRE_SENSOR_TYPE_UNCALIBRATED_ACCELEROMETER:
    case CHRE_SENSOR_TYPE_UNCALIBRATED_GYROSCOPE:
    case CHRE_SENSOR_TYPE_UNCALIBRATED_GEOMAGNETIC_FIELD:
      return true;
    default:
#ifdef CHREX_SENSOR_SUPPORT
      return extension::vendorSensorReportsBias(sensorType);
#else
      return false;
#endif
  }
}

}  // namespace chre