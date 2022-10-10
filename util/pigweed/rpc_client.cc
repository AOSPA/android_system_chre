/*
 * Copyright (C) 2022 The Android Open Source Project
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

#include "chre/util/pigweed/rpc_client.h"

#include <cinttypes>
#include <cstdint>

#include "chre/event.h"
#include "chre/re.h"
#include "chre/util/macros.h"
#include "chre/util/nanoapp/log.h"

#ifndef LOG_TAG
#define LOG_TAG "[RpcClient]"
#endif  // LOG_TAG

namespace chre {
namespace {

// Mask to extract the host ID / nanoapp ID from a channel ID.
constexpr uint32_t kClientIdMask = 0xffff;

constexpr uint32_t kNanoappMaxId = 0xffff;

// Returns whether the host / nanoapp IDs match.
bool endpointsMatch(uint32_t expectedId, uint32_t actualId) {
  if ((expectedId & kClientIdMask) != (actualId & kClientIdMask)) {
    LOGE("Invalid endpoint 0x%04" PRIx32 " expected 0x%04" PRIx32, actualId,
         expectedId);
    return false;
  }

  return true;
}

}  // namespace

bool RpcClient::handleEvent(uint32_t senderInstanceId, uint16_t eventType,
                            const void *eventData) {
  switch (eventType) {
    case chre::ChreChannelOutputBase::PW_RPC_CHRE_NAPP_RESPONSE_EVENT_TYPE:
      return handleMessageFromServer(senderInstanceId, eventData);
    case CHRE_EVENT_NANOAPP_STOPPED:
      handleNanoappStopped(eventData);
      return true;
  }

  return true;
}

bool RpcClient::handleMessageFromServer(uint32_t senderInstanceId,
                                        const void *eventData) {
  auto data = static_cast<const chre::ChrePigweedNanoappMessage *>(eventData);
  std::span packet(static_cast<const std::byte *>(data->msg), data->msgSize);

  pw::Result result = pw::rpc::ExtractChannelId(packet);
  if (result.status() != PW_STATUS_OK) {
    LOGE("Unable to extract channel ID from packet");
    return false;
  }

  if (!validateNanoappChannelId(senderInstanceId, result.value())) {
    return false;
  }

  pw::Status status = mRpcClient.ProcessPacket(packet);

  if (status != pw::OkStatus()) {
    LOGE("Failed to process the packet");
    return false;
  }

  return true;
}

void RpcClient::handleNanoappStopped(const void *eventData) {
  auto info = static_cast<const struct chreNanoappInfo *>(eventData);

  if (info->instanceId > 0xffff) {
    LOGE("Invalid nanoapp Id 0x%08" PRIx32, info->instanceId);
    return;
  }

  if (info->instanceId == mChannelId) {
    mRpcClient.CloseChannel(mChannelId);
    mChannelId = 0;
  }
}

bool RpcClient::validateNanoappChannelId(uint32_t nappId, uint32_t channelId) {
  if (nappId > kNanoappMaxId) {
    LOGE("Invalid nanoapp Id 0x%08" PRIx32, nappId);
    return false;
  }

  return endpointsMatch(channelId, nappId);
}

}  // namespace chre