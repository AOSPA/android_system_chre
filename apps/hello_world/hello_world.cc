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

#include <inttypes.h>

#include "chre/util/nanoapp/log.h"
#include "chre_api/chre.h"

#define LOG_TAG "[HelloWorld]"

/**
 * @file
 * A very basic example nanoapp that just logs activity in its entry points.
 * Note that code wrapped in #ifdef CHRE_NANOAPP_INTERNAL is only relevant when
 * a nanoapp is to be statically built into the CHRE system binary, which would
 * make it a "system nanoapp" rather than a true dynamically loadable nanoapp.
 */

#ifdef CHRE_NANOAPP_INTERNAL
namespace chre {
namespace {
#endif  // CHRE_NANOAPP_INTERNAL

bool nanoappStart() {
  LOGI("Hello, world from version 0x%08" PRIx32, chreGetVersion());
  return true;
}

void nanoappHandleEvent(uint32_t senderInstanceId, uint16_t eventType,
                        const void * /*eventData*/) {
  LOGI("Received event 0x%" PRIx16 " from 0x%" PRIx32 " at time %" PRIu64,
       eventType, senderInstanceId, chreGetTime());
}

void nanoappEnd() {
  LOGI("Goodbye, world!");
}

#ifdef CHRE_NANOAPP_INTERNAL
}  // anonymous namespace
}  // namespace chre

#include "chre/platform/static_nanoapp_init.h"
#include "chre/util/nanoapp/app_id.h"
#include "chre/util/system/napp_permissions.h"

CHRE_STATIC_NANOAPP_INIT(HelloWorld, chre::kHelloWorldAppId, 0,
                         chre::NanoappPermissions::CHRE_PERMS_NONE);
#endif  // CHRE_NANOAPP_INTERNAL
