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

#include "chre/platform/platform_audio.h"

namespace chre {

PlatformAudio::PlatformAudio() {
  // TODO: Init the connection to the audio subsystem.
}

PlatformAudio::~PlatformAudio() {
  // TODO: Deinit the connection to the audio subsystem.
}

size_t PlatformAudio::getSourceCount() {
  // TODO(P1-f3f9a0): Implement this.
  return 0;
}

bool PlatformAudio::getAudioSource(uint32_t handle,
                                   chreAudioSource *source) {
  // TODO(P1-f3f9a0): Implement this.
  return false;
}

}  // namespace chre