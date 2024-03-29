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

#include "chre/util/nanoapp/app_id.h"
#include "chre/util/system/napp_header_utils.h"
#include "chre_host/file_stream.h"
#include "chre_host/host_protocol_host.h"
#include "chre_host/log.h"
#include "chre_host/napp_header.h"
#include "chre_host/socket_client.h"

#include <inttypes.h>
#include <sys/socket.h>
#include <sys/types.h>

#include <fstream>
#include <future>
#include <sstream>
#include <thread>

#include <cutils/sockets.h>
#include <utils/StrongPointer.h>

/**
 * @file
 * A test utility that connects to the CHRE daemon that runs on the apps
 * processor of MSM chipsets, which is used to help test basic functionality.
 *
 * Usage:
 *  chre_test_client load <nanoapp-id> <nanoapp-so-path> \
 *      [app-version] [api-version] [tcm-capable] [nanoapp-header-path]
 *  chre_test_client load_with_header <nanoapp-header-path> <nanoapp-so-path>
 *  chre_test_client unload <nanoapp-id>
 */

using android::sp;
using android::chre::FragmentedLoadTransaction;
using android::chre::getStringFromByteVector;
using android::chre::HostProtocolHost;
using android::chre::IChreMessageHandlers;
using android::chre::NanoAppBinaryHeader;
using android::chre::readFileContents;
using android::chre::SocketClient;
using flatbuffers::FlatBufferBuilder;

// Aliased for consistency with the way these symbols are referenced in
// CHRE-side code
namespace fbs = ::chre::fbs;

namespace {

//! The host endpoint we use when sending; set to CHRE_HOST_ENDPOINT_UNSPECIFIED
//! Other clients below the HAL may use a value above 0x8000 to enable unicast
//! messaging (currently requires internal coordination to avoid conflict;
//! in the future these should be assigned by the daemon).
constexpr uint16_t kHostEndpoint = 0xfffe;

constexpr uint32_t kDefaultAppVersion = 1;
constexpr uint32_t kDefaultApiVersion = 0x01000000;

class SocketCallbacks : public SocketClient::ICallbacks,
                        public IChreMessageHandlers {
 public:
  void onMessageReceived(const void *data, size_t length) override {
    if (!HostProtocolHost::decodeMessageFromChre(data, length, *this)) {
      LOGE("Failed to decode message");
    }
  }

  void onConnected() override {
    LOGI("Socket (re)connected");
  }

  void onConnectionAborted() override {
    LOGI("Socket (re)connection aborted");
  }

  void onDisconnected() override {
    LOGI("Socket disconnected");
  }

  void handleNanoappMessage(const fbs::NanoappMessageT &message) override {
    LOGI("Got message from nanoapp 0x%" PRIx64 " to endpoint 0x%" PRIx16
         " with type 0x%" PRIx32 " and length %zu",
         message.app_id, message.host_endpoint, message.message_type,
         message.message.size());
  }

  void handleHubInfoResponse(const fbs::HubInfoResponseT &rsp) override {
    LOGI("Got hub info response:");
    LOGI("  Name: '%s'", getStringFromByteVector(rsp.name));
    LOGI("  Vendor: '%s'", getStringFromByteVector(rsp.vendor));
    LOGI("  Toolchain: '%s'", getStringFromByteVector(rsp.toolchain));
    LOGI("  Legacy versions: platform 0x%08" PRIx32 " toolchain 0x%08" PRIx32,
         rsp.platform_version, rsp.toolchain_version);
    LOGI("  MIPS %.2f Power (mW): stopped %.2f sleep %.2f peak %.2f",
         rsp.peak_mips, rsp.stopped_power, rsp.sleep_power, rsp.peak_power);
    LOGI("  Max message len: %" PRIu32, rsp.max_msg_len);
    LOGI("  Platform ID: 0x%016" PRIx64 " Version: 0x%08" PRIx32,
         rsp.platform_id, rsp.chre_platform_version);
  }

  void handleNanoappListResponse(
      const fbs::NanoappListResponseT &response) override {
    LOGI("Got nanoapp list response with %zu apps:", response.nanoapps.size());
    for (const std::unique_ptr<fbs::NanoappListEntryT> &nanoapp :
         response.nanoapps) {
      LOGI("  App ID 0x%016" PRIx64 " version 0x%" PRIx32
           " permissions 0x%" PRIx32 " enabled %d system %d",
           nanoapp->app_id, nanoapp->version, nanoapp->permissions,
           nanoapp->enabled, nanoapp->is_system);
    }
  }

  void handleLoadNanoappResponse(
      const fbs::LoadNanoappResponseT &response) override {
    LOGI("Got load nanoapp response, transaction ID 0x%" PRIx32 " result %d",
         response.transaction_id, response.success);
  }

  void handleUnloadNanoappResponse(
      const fbs::UnloadNanoappResponseT &response) override {
    LOGI("Got unload nanoapp response, transaction ID 0x%" PRIx32 " result %d",
         response.transaction_id, response.success);
  }

  void handleSelfTestResponse(const ::chre::fbs::SelfTestResponseT &response) {
    LOGI("Got self test response with success %d", response.success);
    mResultPromise.set_value(response.success);
  }

  std::future<bool> getResultFuture() {
    return mResultPromise.get_future();
  }

 private:
  std::promise<bool> mResultPromise;
};

void requestHubInfo(SocketClient &client) {
  FlatBufferBuilder builder(64);
  HostProtocolHost::encodeHubInfoRequest(builder);

  LOGI("Sending hub info request (%" PRIu32 " bytes)", builder.GetSize());
  if (!client.sendMessage(builder.GetBufferPointer(), builder.GetSize())) {
    LOGE("Failed to send message");
  }
}

void requestNanoappList(SocketClient &client) {
  FlatBufferBuilder builder(64);
  HostProtocolHost::encodeNanoappListRequest(builder);

  LOGI("Sending app list request (%" PRIu32 " bytes)", builder.GetSize());
  if (!client.sendMessage(builder.GetBufferPointer(), builder.GetSize())) {
    LOGE("Failed to send message");
  }
}

void sendMessageToNanoapp(SocketClient &client) {
  FlatBufferBuilder builder(64);
  uint8_t messageData[] = {1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11};
  HostProtocolHost::encodeNanoappMessage(builder, chre::kMessageWorldAppId,
                                         1234 /* messageType */, kHostEndpoint,
                                         messageData, sizeof(messageData));

  LOGI("Sending message to nanoapp (%" PRIu32 " bytes w/%zu bytes of payload)",
       builder.GetSize(), sizeof(messageData));
  if (!client.sendMessage(builder.GetBufferPointer(), builder.GetSize())) {
    LOGE("Failed to send message");
  }
}

void sendNanoappLoad(SocketClient &client, uint64_t appId, uint32_t appVersion,
                     uint32_t apiVersion, uint32_t appFlags,
                     const std::vector<uint8_t> &binary) {
  // Perform loading with 1 fragment for simplicity
  FlatBufferBuilder builder(binary.size() + 128);
  FragmentedLoadTransaction transaction = FragmentedLoadTransaction(
      1 /* transactionId */, appId, appVersion, appFlags, apiVersion, binary,
      binary.size() /* fragmentSize */);
  HostProtocolHost::encodeFragmentedLoadNanoappRequest(
      builder, transaction.getNextRequest());

  LOGI("Sending load nanoapp request (%" PRIu32
       " bytes total w/%zu bytes of "
       "payload)",
       builder.GetSize(), binary.size());
  if (!client.sendMessage(builder.GetBufferPointer(), builder.GetSize())) {
    LOGE("Failed to send message");
  }
}

void sendLoadNanoappRequest(SocketClient &client, const char *headerPath,
                            const char *binaryPath) {
  std::vector<uint8_t> headerBuffer;
  std::vector<uint8_t> binaryBuffer;
  if (readFileContents(headerPath, headerBuffer) &&
      readFileContents(binaryPath, binaryBuffer)) {
    if (headerBuffer.size() != sizeof(NanoAppBinaryHeader)) {
      LOGE("Header size mismatch");
    } else {
      // The header blob contains the struct above.
      const auto *appHeader =
          reinterpret_cast<const NanoAppBinaryHeader *>(headerBuffer.data());

      // Build the target API version from major and minor.
      uint32_t targetApiVersion = (appHeader->targetChreApiMajorVersion << 24) |
                                  (appHeader->targetChreApiMinorVersion << 16);

      sendNanoappLoad(client, appHeader->appId, appHeader->appVersion,
                      targetApiVersion, appHeader->flags, binaryBuffer);
    }
  }
}

void sendLoadNanoappRequest(SocketClient &client, const char *filename,
                            uint64_t appId, uint32_t appVersion,
                            uint32_t apiVersion, bool tcmApp) {
  std::vector<uint8_t> buffer;
  if (readFileContents(filename, buffer)) {
    // All loaded nanoapps must be signed currently.
    uint32_t appFlags = CHRE_NAPP_HEADER_SIGNED;
    if (tcmApp) {
      appFlags |= CHRE_NAPP_HEADER_TCM_CAPABLE;
    }

    sendNanoappLoad(client, appId, appVersion, apiVersion, appFlags, buffer);
  }
}

void sendUnloadNanoappRequest(SocketClient &client, uint64_t appId) {
  FlatBufferBuilder builder(48);
  constexpr uint32_t kTransactionId = 4321;
  HostProtocolHost::encodeUnloadNanoappRequest(
      builder, kTransactionId, appId, true /* allowSystemNanoappUnload */);

  LOGI("Sending unload request for nanoapp 0x%016" PRIx64 " (size %" PRIu32 ")",
       appId, builder.GetSize());
  if (!client.sendMessage(builder.GetBufferPointer(), builder.GetSize())) {
    LOGE("Failed to send message");
  }
}

void sendSelfTestRequest(SocketClient &client) {
  FlatBufferBuilder builder(48);
  HostProtocolHost::encodeSelfTestRequest(builder);

  LOGI("Sending self test");
  if (!client.sendMessage(builder.GetBufferPointer(), builder.GetSize())) {
    LOGE("Failed to send message");
  }
}

}  // anonymous namespace

static void usage(const std::string &name) {
  std::string output;

  output =
      "\n"
      "Usage:\n  " +
      name +
      " load <nanoapp-id> <nanoapp-so-path> [app-version] [api-version]\n  " +
      name + " load_with_header <nanoapp-header-path> <nanoapp-so-path>\n  " +
      name + " unload <nanoapp-id>\n " + name + " self_test\n";

  LOGI("%s", output.c_str());
}

int main(int argc, char *argv[]) {
  int argi = 0;
  const std::string name{argv[argi++]};
  const std::string cmd{argi < argc ? argv[argi++] : ""};

  SocketClient client;
  sp<SocketCallbacks> callbacks = new SocketCallbacks();

  bool success = true;
  if (!client.connect("chre", callbacks)) {
    LOGE("Couldn't connect to socket");
    success = false;
  } else if (cmd.empty()) {
    requestHubInfo(client);
    requestNanoappList(client);
    sendMessageToNanoapp(client);
    sendLoadNanoappRequest(client, "/data/activity.so",
                           0x476f6f676c00100b /* appId */, 0 /* appVersion */,
                           0x01000000 /* targetApiVersion */,
                           false /* tcmCapable */);
    sendUnloadNanoappRequest(client, 0x476f6f676c00100b /* appId */);

    LOGI("Sleeping, waiting on responses");
    std::this_thread::sleep_for(std::chrono::seconds(5));
  } else if (cmd == "load_with_header") {
    const std::string headerPath{argi < argc ? argv[argi++] : ""};
    const std::string binaryPath{argi < argc ? argv[argi++] : ""};

    if (headerPath.empty() || binaryPath.empty()) {
      LOGE("Arguments not provided!");
      usage(name);
      success = false;
    } else {
      sendLoadNanoappRequest(client, headerPath.c_str(), binaryPath.c_str());
    }
  } else if (cmd == "load") {
    const std::string idstr{argi < argc ? argv[argi++] : ""};
    const std::string path{argi < argc ? argv[argi++] : ""};
    const std::string appVerStr{argi < argc ? argv[argi++] : ""};
    const std::string apiVerStr{argi < argc ? argv[argi++] : ""};
    const std::string tcmCapStr{argi < argc ? argv[argi++] : ""};

    uint64_t id = 0;
    uint32_t appVersion = kDefaultAppVersion;
    uint32_t apiVersion = kDefaultApiVersion;
    bool tcmApp = false;

    if (idstr.empty() || path.empty()) {
      LOGE("Arguments not provided!");
      usage(name);
      success = false;
    } else {
      std::istringstream(idstr) >> std::setbase(0) >> id;
      if (!appVerStr.empty()) {
        std::istringstream(appVerStr) >> std::setbase(0) >> appVersion;
      }
      if (!apiVerStr.empty()) {
        std::istringstream(apiVerStr) >> std::setbase(0) >> apiVersion;
      }
      if (!tcmCapStr.empty()) {
        std::istringstream(tcmCapStr) >> tcmApp;
      }
      sendLoadNanoappRequest(client, path.c_str(), id, appVersion, apiVersion,
                             tcmApp);
    }
  } else if (cmd == "unload") {
    const std::string idstr{argi < argc ? argv[argi++] : ""};
    uint64_t id = 0;

    if (idstr.empty()) {
      LOGE("Arguments not provided!");
      usage(name);
      success = false;
    } else {
      std::istringstream(idstr) >> std::setbase(0) >> id;
      sendUnloadNanoappRequest(client, id);
    }
  } else if (cmd == "self_test") {
    sendSelfTestRequest(client);

    std::future<bool> future = callbacks->getResultFuture();
    std::future_status status = future.wait_for(std::chrono::seconds(5));

    if (status != std::future_status::ready) {
      LOGE("Self test timed out");
    } else {
      success = future.get();
    }
  } else {
    LOGE("Invalid command provided!");
    usage(name);
  }

  return success ? 0 : -1;
}
