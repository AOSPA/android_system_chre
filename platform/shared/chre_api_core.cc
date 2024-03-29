/*
 * Copyright (C) 2016 The Android Open Source Project
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

#include <cstdarg>
#include <cstdio>
#include <cstring>

#include "chre/core/event_loop_manager.h"
#include "chre/core/host_comms_manager.h"
#include "chre/core/host_endpoint_manager.h"
#include "chre/platform/fatal_error.h"
#include "chre/platform/log.h"
#include "chre/util/macros.h"
#include "chre/util/system/napp_permissions.h"
#include "chre_api/chre/event.h"
#include "chre_api/chre/re.h"

using chre::EventLoop;
using chre::EventLoopManager;
using chre::EventLoopManagerSingleton;
using chre::Nanoapp;

DLL_EXPORT void chreAbort(uint32_t /* abortCode */) {
  Nanoapp *nanoapp = EventLoopManager::validateChreApiCall(__func__);

  // TODO: we should cleanly unload the nanoapp, release all of its resources,
  // and send an abort notification to the host so as to localize the impact to
  // the calling nanoapp
  if (nanoapp == nullptr) {
    FATAL_ERROR("chreAbort called in unknown context");
  } else {
    FATAL_ERROR("chreAbort called by app 0x%016" PRIx64, nanoapp->getAppId());
  }
}

DLL_EXPORT bool chreSendEvent(uint16_t eventType, void *eventData,
                              chreEventCompleteFunction *freeCallback,
                              uint32_t targetInstanceId) {
  Nanoapp *nanoapp = EventLoopManager::validateChreApiCall(__func__);

  // Prevent an app that is in the process of being unloaded from generating new
  // events
  bool success = false;
  EventLoop &eventLoop = EventLoopManagerSingleton::get()->getEventLoop();
  CHRE_ASSERT_LOG(targetInstanceId <= UINT16_MAX,
                  "Invalid instance ID %" PRIu32 " provided", targetInstanceId);
  if (eventLoop.currentNanoappIsStopping()) {
    LOGW("Rejecting event from app instance %" PRIu16 " because it's stopping",
         nanoapp->getInstanceId());
  } else if (targetInstanceId <= UINT16_MAX) {
    success = eventLoop.postLowPriorityEventOrFree(
        eventType, eventData, freeCallback, nanoapp->getInstanceId(),
        static_cast<uint16_t>(targetInstanceId));
  }
  return success;
}

DLL_EXPORT bool chreSendMessageToHost(void *message, uint32_t messageSize,
                                      uint32_t messageType,
                                      chreMessageFreeFunction *freeCallback) {
  return chreSendMessageToHostEndpoint(
      message, static_cast<size_t>(messageSize), messageType,
      CHRE_HOST_ENDPOINT_BROADCAST, freeCallback);
}

DLL_EXPORT bool chreSendMessageWithPermissions(
    void *message, size_t messageSize, uint32_t messageType,
    uint16_t hostEndpoint, uint32_t messagePermissions,
    chreMessageFreeFunction *freeCallback) {
  Nanoapp *nanoapp = EventLoopManager::validateChreApiCall(__func__);

  bool success = false;
  const EventLoop &eventLoop = EventLoopManagerSingleton::get()->getEventLoop();
  if (eventLoop.currentNanoappIsStopping()) {
    LOGW("Rejecting message to host from app instance %" PRIu16
         " because it's stopping",
         nanoapp->getInstanceId());
  } else {
    auto &hostCommsManager =
        EventLoopManagerSingleton::get()->getHostCommsManager();
    success = hostCommsManager.sendMessageToHostFromNanoapp(
        nanoapp, message, messageSize, messageType, hostEndpoint,
        messagePermissions, freeCallback);
  }

  if (!success && freeCallback != nullptr) {
    freeCallback(message, messageSize);
  }

  return success;
}

DLL_EXPORT bool chreSendMessageToHostEndpoint(
    void *message, size_t messageSize, uint32_t messageType,
    uint16_t hostEndpoint, chreMessageFreeFunction *freeCallback) {
  return chreSendMessageWithPermissions(
      message, messageSize, messageType, hostEndpoint,
      static_cast<uint32_t>(chre::NanoappPermissions::CHRE_PERMS_NONE),
      freeCallback);
}

DLL_EXPORT bool chreGetNanoappInfoByAppId(uint64_t appId,
                                          struct chreNanoappInfo *info) {
  return EventLoopManagerSingleton::get()
      ->getEventLoop()
      .populateNanoappInfoForAppId(appId, info);
}

DLL_EXPORT bool chreGetNanoappInfoByInstanceId(uint32_t instanceId,
                                               struct chreNanoappInfo *info) {
  CHRE_ASSERT(instanceId <= UINT16_MAX);
  if (instanceId <= UINT16_MAX) {
    return EventLoopManagerSingleton::get()
        ->getEventLoop()
        .populateNanoappInfoForInstanceId(static_cast<uint16_t>(instanceId),
                                          info);
  }
  return false;
}

DLL_EXPORT void chreConfigureNanoappInfoEvents(bool enable) {
  chre::Nanoapp *nanoapp = EventLoopManager::validateChreApiCall(__func__);
  nanoapp->configureNanoappInfoEvents(enable);
}

DLL_EXPORT void chreConfigureHostSleepStateEvents(bool enable) {
  chre::Nanoapp *nanoapp = EventLoopManager::validateChreApiCall(__func__);
  nanoapp->configureHostSleepEvents(enable);
}

DLL_EXPORT bool chreIsHostAwake() {
  return EventLoopManagerSingleton::get()
      ->getEventLoop()
      .getPowerControlManager()
      .hostIsAwake();
}

DLL_EXPORT void chreConfigureDebugDumpEvent(bool enable) {
  chre::Nanoapp *nanoapp = EventLoopManager::validateChreApiCall(__func__);
  nanoapp->configureDebugDumpEvent(enable);
}

DLL_EXPORT bool chreConfigureHostEndpointNotifications(uint16_t hostEndpointId,
                                                       bool enable) {
  chre::Nanoapp *nanoapp = EventLoopManager::validateChreApiCall(__func__);
  return nanoapp->configureHostEndpointNotifications(hostEndpointId, enable);
}

DLL_EXPORT bool chrePublishRpcServices(struct chreNanoappRpcService *services,
                                       size_t numServices) {
  chre::Nanoapp *nanoapp = EventLoopManager::validateChreApiCall(__func__);
  return nanoapp->publishRpcServices(services, numServices);
}

DLL_EXPORT bool chreGetHostEndpointInfo(uint16_t hostEndpointId,
                                        struct chreHostEndpointInfo *info) {
  return EventLoopManagerSingleton::get()
      ->getHostEndpointManager()
      .getHostEndpointInfo(hostEndpointId, info);
}
