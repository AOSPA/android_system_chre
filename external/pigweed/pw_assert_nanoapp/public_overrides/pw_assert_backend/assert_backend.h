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

#ifndef _PW_ASSERT_NANOAPP_PW_ASSERT_BACKEND_H_
#define _PW_ASSERT_NANOAPP_PW_ASSERT_BACKEND_H_

#include "chre/re.h"
#include "pw_log/log.h"

#define PW_ASSERT_HANDLE_FAILURE(condition_string)  \
  do {                                              \
    PW_HANDLE_LOG(                                  \
        PW_LOG_LEVEL_FATAL,                         \
        PW_LOG_MODULE_NAME,                         \
        PW_LOG_FLAGS,                               \
        "Assert failed: " condition_string);        \
    chreAbort(UINT32_MAX);                          \
  } while (0)

#endif // _PW_ASSERT_NANOAPP_PW_ASSERT_BACKEND_H_
