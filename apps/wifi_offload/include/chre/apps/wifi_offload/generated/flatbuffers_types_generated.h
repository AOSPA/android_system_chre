// automatically generated by the FlatBuffers compiler, do not modify


#ifndef FLATBUFFERS_GENERATED_FLATBUFFERSTYPES_WIFI_OFFLOAD_FBS_H_
#define FLATBUFFERS_GENERATED_FLATBUFFERSTYPES_WIFI_OFFLOAD_FBS_H_

#include "flatbuffers/flatbuffers.h"

namespace wifi_offload {
namespace fbs {

struct Ssid;
struct SsidBuilder;

struct PreferredNetwork;
struct PreferredNetworkBuilder;

struct ScanResult;
struct ScanResultBuilder;

struct ScanResultMessage;
struct ScanResultMessageBuilder;

struct ScanParams;
struct ScanParamsBuilder;

struct ScanFilter;
struct ScanFilterBuilder;

struct ScanConfig;
struct ScanConfigBuilder;

struct ScanRecord;
struct ScanRecordBuilder;

struct RpcLogRecord;
struct RpcLogRecordBuilder;

struct ScanStats;
struct ScanStatsBuilder;

struct Ssid FLATBUFFERS_FINAL_CLASS : private flatbuffers::Table {
  typedef SsidBuilder Builder;
  enum FlatBuffersVTableOffset FLATBUFFERS_VTABLE_UNDERLYING_TYPE {
    VT_SSID = 4
  };
  const flatbuffers::Vector<uint8_t> *ssid() const {
    return GetPointer<const flatbuffers::Vector<uint8_t> *>(VT_SSID);
  }
  bool Verify(flatbuffers::Verifier &verifier) const {
    return VerifyTableStart(verifier) &&
           VerifyOffset(verifier, VT_SSID) &&
           verifier.VerifyVector(ssid()) &&
           verifier.EndTable();
  }
};

struct SsidBuilder {
  typedef Ssid Table;
  flatbuffers::FlatBufferBuilder &fbb_;
  flatbuffers::uoffset_t start_;
  void add_ssid(flatbuffers::Offset<flatbuffers::Vector<uint8_t>> ssid) {
    fbb_.AddOffset(Ssid::VT_SSID, ssid);
  }
  explicit SsidBuilder(flatbuffers::FlatBufferBuilder &_fbb)
        : fbb_(_fbb) {
    start_ = fbb_.StartTable();
  }
  SsidBuilder &operator=(const SsidBuilder &);
  flatbuffers::Offset<Ssid> Finish() {
    const auto end = fbb_.EndTable(start_);
    auto o = flatbuffers::Offset<Ssid>(end);
    return o;
  }
};

inline flatbuffers::Offset<Ssid> CreateSsid(
    flatbuffers::FlatBufferBuilder &_fbb,
    flatbuffers::Offset<flatbuffers::Vector<uint8_t>> ssid = 0) {
  SsidBuilder builder_(_fbb);
  builder_.add_ssid(ssid);
  return builder_.Finish();
}

inline flatbuffers::Offset<Ssid> CreateSsidDirect(
    flatbuffers::FlatBufferBuilder &_fbb,
    const std::vector<uint8_t> *ssid = nullptr) {
  auto ssid__ = ssid ? _fbb.CreateVector<uint8_t>(*ssid) : 0;
  return wifi_offload::fbs::CreateSsid(
      _fbb,
      ssid__);
}

struct PreferredNetwork FLATBUFFERS_FINAL_CLASS : private flatbuffers::Table {
  typedef PreferredNetworkBuilder Builder;
  enum FlatBuffersVTableOffset FLATBUFFERS_VTABLE_UNDERLYING_TYPE {
    VT_SSID = 4,
    VT_SECURITY_MODES = 6
  };
  const wifi_offload::fbs::Ssid *ssid() const {
    return GetPointer<const wifi_offload::fbs::Ssid *>(VT_SSID);
  }
  uint8_t security_modes() const {
    return GetField<uint8_t>(VT_SECURITY_MODES, 0);
  }
  bool Verify(flatbuffers::Verifier &verifier) const {
    return VerifyTableStart(verifier) &&
           VerifyOffset(verifier, VT_SSID) &&
           verifier.VerifyTable(ssid()) &&
           VerifyField<uint8_t>(verifier, VT_SECURITY_MODES) &&
           verifier.EndTable();
  }
};

struct PreferredNetworkBuilder {
  typedef PreferredNetwork Table;
  flatbuffers::FlatBufferBuilder &fbb_;
  flatbuffers::uoffset_t start_;
  void add_ssid(flatbuffers::Offset<wifi_offload::fbs::Ssid> ssid) {
    fbb_.AddOffset(PreferredNetwork::VT_SSID, ssid);
  }
  void add_security_modes(uint8_t security_modes) {
    fbb_.AddElement<uint8_t>(PreferredNetwork::VT_SECURITY_MODES, security_modes, 0);
  }
  explicit PreferredNetworkBuilder(flatbuffers::FlatBufferBuilder &_fbb)
        : fbb_(_fbb) {
    start_ = fbb_.StartTable();
  }
  PreferredNetworkBuilder &operator=(const PreferredNetworkBuilder &);
  flatbuffers::Offset<PreferredNetwork> Finish() {
    const auto end = fbb_.EndTable(start_);
    auto o = flatbuffers::Offset<PreferredNetwork>(end);
    return o;
  }
};

inline flatbuffers::Offset<PreferredNetwork> CreatePreferredNetwork(
    flatbuffers::FlatBufferBuilder &_fbb,
    flatbuffers::Offset<wifi_offload::fbs::Ssid> ssid = 0,
    uint8_t security_modes = 0) {
  PreferredNetworkBuilder builder_(_fbb);
  builder_.add_ssid(ssid);
  builder_.add_security_modes(security_modes);
  return builder_.Finish();
}

struct ScanResult FLATBUFFERS_FINAL_CLASS : private flatbuffers::Table {
  typedef ScanResultBuilder Builder;
  enum FlatBuffersVTableOffset FLATBUFFERS_VTABLE_UNDERLYING_TYPE {
    VT_SSID = 4,
    VT_SECURITY_MODES = 6,
    VT_BSSID = 8,
    VT_CAPABILITY = 10,
    VT_FREQUENCY_SCANNED_MHZ = 12,
    VT_RSSI_DBM = 14,
    VT_TSF = 16
  };
  const wifi_offload::fbs::Ssid *ssid() const {
    return GetPointer<const wifi_offload::fbs::Ssid *>(VT_SSID);
  }
  uint8_t security_modes() const {
    return GetField<uint8_t>(VT_SECURITY_MODES, 0);
  }
  const flatbuffers::Vector<uint8_t> *bssid() const {
    return GetPointer<const flatbuffers::Vector<uint8_t> *>(VT_BSSID);
  }
  uint16_t capability() const {
    return GetField<uint16_t>(VT_CAPABILITY, 0);
  }
  uint32_t frequency_scanned_mhz() const {
    return GetField<uint32_t>(VT_FREQUENCY_SCANNED_MHZ, 0);
  }
  int8_t rssi_dbm() const {
    return GetField<int8_t>(VT_RSSI_DBM, 0);
  }
  uint64_t tsf() const {
    return GetField<uint64_t>(VT_TSF, 0);
  }
  bool Verify(flatbuffers::Verifier &verifier) const {
    return VerifyTableStart(verifier) &&
           VerifyOffset(verifier, VT_SSID) &&
           verifier.VerifyTable(ssid()) &&
           VerifyField<uint8_t>(verifier, VT_SECURITY_MODES) &&
           VerifyOffset(verifier, VT_BSSID) &&
           verifier.VerifyVector(bssid()) &&
           VerifyField<uint16_t>(verifier, VT_CAPABILITY) &&
           VerifyField<uint32_t>(verifier, VT_FREQUENCY_SCANNED_MHZ) &&
           VerifyField<int8_t>(verifier, VT_RSSI_DBM) &&
           VerifyField<uint64_t>(verifier, VT_TSF) &&
           verifier.EndTable();
  }
};

struct ScanResultBuilder {
  typedef ScanResult Table;
  flatbuffers::FlatBufferBuilder &fbb_;
  flatbuffers::uoffset_t start_;
  void add_ssid(flatbuffers::Offset<wifi_offload::fbs::Ssid> ssid) {
    fbb_.AddOffset(ScanResult::VT_SSID, ssid);
  }
  void add_security_modes(uint8_t security_modes) {
    fbb_.AddElement<uint8_t>(ScanResult::VT_SECURITY_MODES, security_modes, 0);
  }
  void add_bssid(flatbuffers::Offset<flatbuffers::Vector<uint8_t>> bssid) {
    fbb_.AddOffset(ScanResult::VT_BSSID, bssid);
  }
  void add_capability(uint16_t capability) {
    fbb_.AddElement<uint16_t>(ScanResult::VT_CAPABILITY, capability, 0);
  }
  void add_frequency_scanned_mhz(uint32_t frequency_scanned_mhz) {
    fbb_.AddElement<uint32_t>(ScanResult::VT_FREQUENCY_SCANNED_MHZ, frequency_scanned_mhz, 0);
  }
  void add_rssi_dbm(int8_t rssi_dbm) {
    fbb_.AddElement<int8_t>(ScanResult::VT_RSSI_DBM, rssi_dbm, 0);
  }
  void add_tsf(uint64_t tsf) {
    fbb_.AddElement<uint64_t>(ScanResult::VT_TSF, tsf, 0);
  }
  explicit ScanResultBuilder(flatbuffers::FlatBufferBuilder &_fbb)
        : fbb_(_fbb) {
    start_ = fbb_.StartTable();
  }
  ScanResultBuilder &operator=(const ScanResultBuilder &);
  flatbuffers::Offset<ScanResult> Finish() {
    const auto end = fbb_.EndTable(start_);
    auto o = flatbuffers::Offset<ScanResult>(end);
    return o;
  }
};

inline flatbuffers::Offset<ScanResult> CreateScanResult(
    flatbuffers::FlatBufferBuilder &_fbb,
    flatbuffers::Offset<wifi_offload::fbs::Ssid> ssid = 0,
    uint8_t security_modes = 0,
    flatbuffers::Offset<flatbuffers::Vector<uint8_t>> bssid = 0,
    uint16_t capability = 0,
    uint32_t frequency_scanned_mhz = 0,
    int8_t rssi_dbm = 0,
    uint64_t tsf = 0) {
  ScanResultBuilder builder_(_fbb);
  builder_.add_tsf(tsf);
  builder_.add_frequency_scanned_mhz(frequency_scanned_mhz);
  builder_.add_bssid(bssid);
  builder_.add_ssid(ssid);
  builder_.add_capability(capability);
  builder_.add_rssi_dbm(rssi_dbm);
  builder_.add_security_modes(security_modes);
  return builder_.Finish();
}

inline flatbuffers::Offset<ScanResult> CreateScanResultDirect(
    flatbuffers::FlatBufferBuilder &_fbb,
    flatbuffers::Offset<wifi_offload::fbs::Ssid> ssid = 0,
    uint8_t security_modes = 0,
    const std::vector<uint8_t> *bssid = nullptr,
    uint16_t capability = 0,
    uint32_t frequency_scanned_mhz = 0,
    int8_t rssi_dbm = 0,
    uint64_t tsf = 0) {
  auto bssid__ = bssid ? _fbb.CreateVector<uint8_t>(*bssid) : 0;
  return wifi_offload::fbs::CreateScanResult(
      _fbb,
      ssid,
      security_modes,
      bssid__,
      capability,
      frequency_scanned_mhz,
      rssi_dbm,
      tsf);
}

struct ScanResultMessage FLATBUFFERS_FINAL_CLASS : private flatbuffers::Table {
  typedef ScanResultMessageBuilder Builder;
  enum FlatBuffersVTableOffset FLATBUFFERS_VTABLE_UNDERLYING_TYPE {
    VT_SCAN_RESULTS = 4
  };
  const flatbuffers::Vector<flatbuffers::Offset<wifi_offload::fbs::ScanResult>> *scan_results() const {
    return GetPointer<const flatbuffers::Vector<flatbuffers::Offset<wifi_offload::fbs::ScanResult>> *>(VT_SCAN_RESULTS);
  }
  bool Verify(flatbuffers::Verifier &verifier) const {
    return VerifyTableStart(verifier) &&
           VerifyOffset(verifier, VT_SCAN_RESULTS) &&
           verifier.VerifyVector(scan_results()) &&
           verifier.VerifyVectorOfTables(scan_results()) &&
           verifier.EndTable();
  }
};

struct ScanResultMessageBuilder {
  typedef ScanResultMessage Table;
  flatbuffers::FlatBufferBuilder &fbb_;
  flatbuffers::uoffset_t start_;
  void add_scan_results(flatbuffers::Offset<flatbuffers::Vector<flatbuffers::Offset<wifi_offload::fbs::ScanResult>>> scan_results) {
    fbb_.AddOffset(ScanResultMessage::VT_SCAN_RESULTS, scan_results);
  }
  explicit ScanResultMessageBuilder(flatbuffers::FlatBufferBuilder &_fbb)
        : fbb_(_fbb) {
    start_ = fbb_.StartTable();
  }
  ScanResultMessageBuilder &operator=(const ScanResultMessageBuilder &);
  flatbuffers::Offset<ScanResultMessage> Finish() {
    const auto end = fbb_.EndTable(start_);
    auto o = flatbuffers::Offset<ScanResultMessage>(end);
    return o;
  }
};

inline flatbuffers::Offset<ScanResultMessage> CreateScanResultMessage(
    flatbuffers::FlatBufferBuilder &_fbb,
    flatbuffers::Offset<flatbuffers::Vector<flatbuffers::Offset<wifi_offload::fbs::ScanResult>>> scan_results = 0) {
  ScanResultMessageBuilder builder_(_fbb);
  builder_.add_scan_results(scan_results);
  return builder_.Finish();
}

inline flatbuffers::Offset<ScanResultMessage> CreateScanResultMessageDirect(
    flatbuffers::FlatBufferBuilder &_fbb,
    const std::vector<flatbuffers::Offset<wifi_offload::fbs::ScanResult>> *scan_results = nullptr) {
  auto scan_results__ = scan_results ? _fbb.CreateVector<flatbuffers::Offset<wifi_offload::fbs::ScanResult>>(*scan_results) : 0;
  return wifi_offload::fbs::CreateScanResultMessage(
      _fbb,
      scan_results__);
}

struct ScanParams FLATBUFFERS_FINAL_CLASS : private flatbuffers::Table {
  typedef ScanParamsBuilder Builder;
  enum FlatBuffersVTableOffset FLATBUFFERS_VTABLE_UNDERLYING_TYPE {
    VT_SSIDS_TO_SCAN = 4,
    VT_FREQUENCIES_TO_SCAN_MHZ = 6,
    VT_DISCONNECTED_MODE_SCAN_INTERVAL_MS = 8
  };
  const flatbuffers::Vector<flatbuffers::Offset<wifi_offload::fbs::Ssid>> *ssids_to_scan() const {
    return GetPointer<const flatbuffers::Vector<flatbuffers::Offset<wifi_offload::fbs::Ssid>> *>(VT_SSIDS_TO_SCAN);
  }
  const flatbuffers::Vector<uint32_t> *frequencies_to_scan_mhz() const {
    return GetPointer<const flatbuffers::Vector<uint32_t> *>(VT_FREQUENCIES_TO_SCAN_MHZ);
  }
  uint32_t disconnected_mode_scan_interval_ms() const {
    return GetField<uint32_t>(VT_DISCONNECTED_MODE_SCAN_INTERVAL_MS, 0);
  }
  bool Verify(flatbuffers::Verifier &verifier) const {
    return VerifyTableStart(verifier) &&
           VerifyOffset(verifier, VT_SSIDS_TO_SCAN) &&
           verifier.VerifyVector(ssids_to_scan()) &&
           verifier.VerifyVectorOfTables(ssids_to_scan()) &&
           VerifyOffset(verifier, VT_FREQUENCIES_TO_SCAN_MHZ) &&
           verifier.VerifyVector(frequencies_to_scan_mhz()) &&
           VerifyField<uint32_t>(verifier, VT_DISCONNECTED_MODE_SCAN_INTERVAL_MS) &&
           verifier.EndTable();
  }
};

struct ScanParamsBuilder {
  typedef ScanParams Table;
  flatbuffers::FlatBufferBuilder &fbb_;
  flatbuffers::uoffset_t start_;
  void add_ssids_to_scan(flatbuffers::Offset<flatbuffers::Vector<flatbuffers::Offset<wifi_offload::fbs::Ssid>>> ssids_to_scan) {
    fbb_.AddOffset(ScanParams::VT_SSIDS_TO_SCAN, ssids_to_scan);
  }
  void add_frequencies_to_scan_mhz(flatbuffers::Offset<flatbuffers::Vector<uint32_t>> frequencies_to_scan_mhz) {
    fbb_.AddOffset(ScanParams::VT_FREQUENCIES_TO_SCAN_MHZ, frequencies_to_scan_mhz);
  }
  void add_disconnected_mode_scan_interval_ms(uint32_t disconnected_mode_scan_interval_ms) {
    fbb_.AddElement<uint32_t>(ScanParams::VT_DISCONNECTED_MODE_SCAN_INTERVAL_MS, disconnected_mode_scan_interval_ms, 0);
  }
  explicit ScanParamsBuilder(flatbuffers::FlatBufferBuilder &_fbb)
        : fbb_(_fbb) {
    start_ = fbb_.StartTable();
  }
  ScanParamsBuilder &operator=(const ScanParamsBuilder &);
  flatbuffers::Offset<ScanParams> Finish() {
    const auto end = fbb_.EndTable(start_);
    auto o = flatbuffers::Offset<ScanParams>(end);
    return o;
  }
};

inline flatbuffers::Offset<ScanParams> CreateScanParams(
    flatbuffers::FlatBufferBuilder &_fbb,
    flatbuffers::Offset<flatbuffers::Vector<flatbuffers::Offset<wifi_offload::fbs::Ssid>>> ssids_to_scan = 0,
    flatbuffers::Offset<flatbuffers::Vector<uint32_t>> frequencies_to_scan_mhz = 0,
    uint32_t disconnected_mode_scan_interval_ms = 0) {
  ScanParamsBuilder builder_(_fbb);
  builder_.add_disconnected_mode_scan_interval_ms(disconnected_mode_scan_interval_ms);
  builder_.add_frequencies_to_scan_mhz(frequencies_to_scan_mhz);
  builder_.add_ssids_to_scan(ssids_to_scan);
  return builder_.Finish();
}

inline flatbuffers::Offset<ScanParams> CreateScanParamsDirect(
    flatbuffers::FlatBufferBuilder &_fbb,
    const std::vector<flatbuffers::Offset<wifi_offload::fbs::Ssid>> *ssids_to_scan = nullptr,
    const std::vector<uint32_t> *frequencies_to_scan_mhz = nullptr,
    uint32_t disconnected_mode_scan_interval_ms = 0) {
  auto ssids_to_scan__ = ssids_to_scan ? _fbb.CreateVector<flatbuffers::Offset<wifi_offload::fbs::Ssid>>(*ssids_to_scan) : 0;
  auto frequencies_to_scan_mhz__ = frequencies_to_scan_mhz ? _fbb.CreateVector<uint32_t>(*frequencies_to_scan_mhz) : 0;
  return wifi_offload::fbs::CreateScanParams(
      _fbb,
      ssids_to_scan__,
      frequencies_to_scan_mhz__,
      disconnected_mode_scan_interval_ms);
}

struct ScanFilter FLATBUFFERS_FINAL_CLASS : private flatbuffers::Table {
  typedef ScanFilterBuilder Builder;
  enum FlatBuffersVTableOffset FLATBUFFERS_VTABLE_UNDERLYING_TYPE {
    VT_NETWORKS_TO_MATCH = 4,
    VT_MIN_RSSI_THRESHOLD_DBM = 6
  };
  const flatbuffers::Vector<flatbuffers::Offset<wifi_offload::fbs::PreferredNetwork>> *networks_to_match() const {
    return GetPointer<const flatbuffers::Vector<flatbuffers::Offset<wifi_offload::fbs::PreferredNetwork>> *>(VT_NETWORKS_TO_MATCH);
  }
  int8_t min_rssi_threshold_dbm() const {
    return GetField<int8_t>(VT_MIN_RSSI_THRESHOLD_DBM, 0);
  }
  bool Verify(flatbuffers::Verifier &verifier) const {
    return VerifyTableStart(verifier) &&
           VerifyOffset(verifier, VT_NETWORKS_TO_MATCH) &&
           verifier.VerifyVector(networks_to_match()) &&
           verifier.VerifyVectorOfTables(networks_to_match()) &&
           VerifyField<int8_t>(verifier, VT_MIN_RSSI_THRESHOLD_DBM) &&
           verifier.EndTable();
  }
};

struct ScanFilterBuilder {
  typedef ScanFilter Table;
  flatbuffers::FlatBufferBuilder &fbb_;
  flatbuffers::uoffset_t start_;
  void add_networks_to_match(flatbuffers::Offset<flatbuffers::Vector<flatbuffers::Offset<wifi_offload::fbs::PreferredNetwork>>> networks_to_match) {
    fbb_.AddOffset(ScanFilter::VT_NETWORKS_TO_MATCH, networks_to_match);
  }
  void add_min_rssi_threshold_dbm(int8_t min_rssi_threshold_dbm) {
    fbb_.AddElement<int8_t>(ScanFilter::VT_MIN_RSSI_THRESHOLD_DBM, min_rssi_threshold_dbm, 0);
  }
  explicit ScanFilterBuilder(flatbuffers::FlatBufferBuilder &_fbb)
        : fbb_(_fbb) {
    start_ = fbb_.StartTable();
  }
  ScanFilterBuilder &operator=(const ScanFilterBuilder &);
  flatbuffers::Offset<ScanFilter> Finish() {
    const auto end = fbb_.EndTable(start_);
    auto o = flatbuffers::Offset<ScanFilter>(end);
    return o;
  }
};

inline flatbuffers::Offset<ScanFilter> CreateScanFilter(
    flatbuffers::FlatBufferBuilder &_fbb,
    flatbuffers::Offset<flatbuffers::Vector<flatbuffers::Offset<wifi_offload::fbs::PreferredNetwork>>> networks_to_match = 0,
    int8_t min_rssi_threshold_dbm = 0) {
  ScanFilterBuilder builder_(_fbb);
  builder_.add_networks_to_match(networks_to_match);
  builder_.add_min_rssi_threshold_dbm(min_rssi_threshold_dbm);
  return builder_.Finish();
}

inline flatbuffers::Offset<ScanFilter> CreateScanFilterDirect(
    flatbuffers::FlatBufferBuilder &_fbb,
    const std::vector<flatbuffers::Offset<wifi_offload::fbs::PreferredNetwork>> *networks_to_match = nullptr,
    int8_t min_rssi_threshold_dbm = 0) {
  auto networks_to_match__ = networks_to_match ? _fbb.CreateVector<flatbuffers::Offset<wifi_offload::fbs::PreferredNetwork>>(*networks_to_match) : 0;
  return wifi_offload::fbs::CreateScanFilter(
      _fbb,
      networks_to_match__,
      min_rssi_threshold_dbm);
}

struct ScanConfig FLATBUFFERS_FINAL_CLASS : private flatbuffers::Table {
  typedef ScanConfigBuilder Builder;
  enum FlatBuffersVTableOffset FLATBUFFERS_VTABLE_UNDERLYING_TYPE {
    VT_SCAN_PARAMS = 4,
    VT_SCAN_FILTER = 6
  };
  const wifi_offload::fbs::ScanParams *scan_params() const {
    return GetPointer<const wifi_offload::fbs::ScanParams *>(VT_SCAN_PARAMS);
  }
  const wifi_offload::fbs::ScanFilter *scan_filter() const {
    return GetPointer<const wifi_offload::fbs::ScanFilter *>(VT_SCAN_FILTER);
  }
  bool Verify(flatbuffers::Verifier &verifier) const {
    return VerifyTableStart(verifier) &&
           VerifyOffset(verifier, VT_SCAN_PARAMS) &&
           verifier.VerifyTable(scan_params()) &&
           VerifyOffset(verifier, VT_SCAN_FILTER) &&
           verifier.VerifyTable(scan_filter()) &&
           verifier.EndTable();
  }
};

struct ScanConfigBuilder {
  typedef ScanConfig Table;
  flatbuffers::FlatBufferBuilder &fbb_;
  flatbuffers::uoffset_t start_;
  void add_scan_params(flatbuffers::Offset<wifi_offload::fbs::ScanParams> scan_params) {
    fbb_.AddOffset(ScanConfig::VT_SCAN_PARAMS, scan_params);
  }
  void add_scan_filter(flatbuffers::Offset<wifi_offload::fbs::ScanFilter> scan_filter) {
    fbb_.AddOffset(ScanConfig::VT_SCAN_FILTER, scan_filter);
  }
  explicit ScanConfigBuilder(flatbuffers::FlatBufferBuilder &_fbb)
        : fbb_(_fbb) {
    start_ = fbb_.StartTable();
  }
  ScanConfigBuilder &operator=(const ScanConfigBuilder &);
  flatbuffers::Offset<ScanConfig> Finish() {
    const auto end = fbb_.EndTable(start_);
    auto o = flatbuffers::Offset<ScanConfig>(end);
    return o;
  }
};

inline flatbuffers::Offset<ScanConfig> CreateScanConfig(
    flatbuffers::FlatBufferBuilder &_fbb,
    flatbuffers::Offset<wifi_offload::fbs::ScanParams> scan_params = 0,
    flatbuffers::Offset<wifi_offload::fbs::ScanFilter> scan_filter = 0) {
  ScanConfigBuilder builder_(_fbb);
  builder_.add_scan_filter(scan_filter);
  builder_.add_scan_params(scan_params);
  return builder_.Finish();
}

struct ScanRecord FLATBUFFERS_FINAL_CLASS : private flatbuffers::Table {
  typedef ScanRecordBuilder Builder;
  enum FlatBuffersVTableOffset FLATBUFFERS_VTABLE_UNDERLYING_TYPE {
    VT_TIME_SPENT_SCANNING_MS = 4,
    VT_NUM_CHANNELS_SCANNED = 6,
    VT_NUM_ENTRIES_AGGREGATED = 8
  };
  uint32_t time_spent_scanning_ms() const {
    return GetField<uint32_t>(VT_TIME_SPENT_SCANNING_MS, 0);
  }
  uint32_t num_channels_scanned() const {
    return GetField<uint32_t>(VT_NUM_CHANNELS_SCANNED, 0);
  }
  uint32_t num_entries_aggregated() const {
    return GetField<uint32_t>(VT_NUM_ENTRIES_AGGREGATED, 0);
  }
  bool Verify(flatbuffers::Verifier &verifier) const {
    return VerifyTableStart(verifier) &&
           VerifyField<uint32_t>(verifier, VT_TIME_SPENT_SCANNING_MS) &&
           VerifyField<uint32_t>(verifier, VT_NUM_CHANNELS_SCANNED) &&
           VerifyField<uint32_t>(verifier, VT_NUM_ENTRIES_AGGREGATED) &&
           verifier.EndTable();
  }
};

struct ScanRecordBuilder {
  typedef ScanRecord Table;
  flatbuffers::FlatBufferBuilder &fbb_;
  flatbuffers::uoffset_t start_;
  void add_time_spent_scanning_ms(uint32_t time_spent_scanning_ms) {
    fbb_.AddElement<uint32_t>(ScanRecord::VT_TIME_SPENT_SCANNING_MS, time_spent_scanning_ms, 0);
  }
  void add_num_channels_scanned(uint32_t num_channels_scanned) {
    fbb_.AddElement<uint32_t>(ScanRecord::VT_NUM_CHANNELS_SCANNED, num_channels_scanned, 0);
  }
  void add_num_entries_aggregated(uint32_t num_entries_aggregated) {
    fbb_.AddElement<uint32_t>(ScanRecord::VT_NUM_ENTRIES_AGGREGATED, num_entries_aggregated, 0);
  }
  explicit ScanRecordBuilder(flatbuffers::FlatBufferBuilder &_fbb)
        : fbb_(_fbb) {
    start_ = fbb_.StartTable();
  }
  ScanRecordBuilder &operator=(const ScanRecordBuilder &);
  flatbuffers::Offset<ScanRecord> Finish() {
    const auto end = fbb_.EndTable(start_);
    auto o = flatbuffers::Offset<ScanRecord>(end);
    return o;
  }
};

inline flatbuffers::Offset<ScanRecord> CreateScanRecord(
    flatbuffers::FlatBufferBuilder &_fbb,
    uint32_t time_spent_scanning_ms = 0,
    uint32_t num_channels_scanned = 0,
    uint32_t num_entries_aggregated = 0) {
  ScanRecordBuilder builder_(_fbb);
  builder_.add_num_entries_aggregated(num_entries_aggregated);
  builder_.add_num_channels_scanned(num_channels_scanned);
  builder_.add_time_spent_scanning_ms(time_spent_scanning_ms);
  return builder_.Finish();
}

struct RpcLogRecord FLATBUFFERS_FINAL_CLASS : private flatbuffers::Table {
  typedef RpcLogRecordBuilder Builder;
  enum FlatBuffersVTableOffset FLATBUFFERS_VTABLE_UNDERLYING_TYPE {
    VT_RECORD_TYPE = 4,
    VT_TIMESTAMP_CHRE_MS = 6
  };
  uint8_t record_type() const {
    return GetField<uint8_t>(VT_RECORD_TYPE, 0);
  }
  uint32_t timestamp_chre_ms() const {
    return GetField<uint32_t>(VT_TIMESTAMP_CHRE_MS, 0);
  }
  bool Verify(flatbuffers::Verifier &verifier) const {
    return VerifyTableStart(verifier) &&
           VerifyField<uint8_t>(verifier, VT_RECORD_TYPE) &&
           VerifyField<uint32_t>(verifier, VT_TIMESTAMP_CHRE_MS) &&
           verifier.EndTable();
  }
};

struct RpcLogRecordBuilder {
  typedef RpcLogRecord Table;
  flatbuffers::FlatBufferBuilder &fbb_;
  flatbuffers::uoffset_t start_;
  void add_record_type(uint8_t record_type) {
    fbb_.AddElement<uint8_t>(RpcLogRecord::VT_RECORD_TYPE, record_type, 0);
  }
  void add_timestamp_chre_ms(uint32_t timestamp_chre_ms) {
    fbb_.AddElement<uint32_t>(RpcLogRecord::VT_TIMESTAMP_CHRE_MS, timestamp_chre_ms, 0);
  }
  explicit RpcLogRecordBuilder(flatbuffers::FlatBufferBuilder &_fbb)
        : fbb_(_fbb) {
    start_ = fbb_.StartTable();
  }
  RpcLogRecordBuilder &operator=(const RpcLogRecordBuilder &);
  flatbuffers::Offset<RpcLogRecord> Finish() {
    const auto end = fbb_.EndTable(start_);
    auto o = flatbuffers::Offset<RpcLogRecord>(end);
    return o;
  }
};

inline flatbuffers::Offset<RpcLogRecord> CreateRpcLogRecord(
    flatbuffers::FlatBufferBuilder &_fbb,
    uint8_t record_type = 0,
    uint32_t timestamp_chre_ms = 0) {
  RpcLogRecordBuilder builder_(_fbb);
  builder_.add_timestamp_chre_ms(timestamp_chre_ms);
  builder_.add_record_type(record_type);
  return builder_.Finish();
}

struct ScanStats FLATBUFFERS_FINAL_CLASS : private flatbuffers::Table {
  typedef ScanStatsBuilder Builder;
  enum FlatBuffersVTableOffset FLATBUFFERS_VTABLE_UNDERLYING_TYPE {
    VT_NUM_SCANS_REQUESTED_BY_NANOAPP = 4,
    VT_NUM_SCANS_SERVICED_BY_HARDWARE = 6,
    VT_NUM_SCANS_SERVICED_BY_CACHE = 8,
    VT_UPDATED_AT_CHRE_MS = 10,
    VT_SENT_AT_CHRE_MS = 12,
    VT_LAST_SUBSCRIPTION_DURATION_MS = 14,
    VT_CHANNEL_SCAN_COUNT = 16,
    VT_SCAN_RECORDS = 18,
    VT_RPC_LOG_RECORDS = 20
  };
  uint32_t num_scans_requested_by_nanoapp() const {
    return GetField<uint32_t>(VT_NUM_SCANS_REQUESTED_BY_NANOAPP, 0);
  }
  uint32_t num_scans_serviced_by_hardware() const {
    return GetField<uint32_t>(VT_NUM_SCANS_SERVICED_BY_HARDWARE, 0);
  }
  uint32_t num_scans_serviced_by_cache() const {
    return GetField<uint32_t>(VT_NUM_SCANS_SERVICED_BY_CACHE, 0);
  }
  uint32_t updated_at_chre_ms() const {
    return GetField<uint32_t>(VT_UPDATED_AT_CHRE_MS, 0);
  }
  uint32_t sent_at_chre_ms() const {
    return GetField<uint32_t>(VT_SENT_AT_CHRE_MS, 0);
  }
  uint32_t last_subscription_duration_ms() const {
    return GetField<uint32_t>(VT_LAST_SUBSCRIPTION_DURATION_MS, 0);
  }
  const flatbuffers::Vector<uint8_t> *channel_scan_count() const {
    return GetPointer<const flatbuffers::Vector<uint8_t> *>(VT_CHANNEL_SCAN_COUNT);
  }
  const flatbuffers::Vector<flatbuffers::Offset<wifi_offload::fbs::ScanRecord>> *scan_records() const {
    return GetPointer<const flatbuffers::Vector<flatbuffers::Offset<wifi_offload::fbs::ScanRecord>> *>(VT_SCAN_RECORDS);
  }
  const flatbuffers::Vector<flatbuffers::Offset<wifi_offload::fbs::RpcLogRecord>> *rpc_log_records() const {
    return GetPointer<const flatbuffers::Vector<flatbuffers::Offset<wifi_offload::fbs::RpcLogRecord>> *>(VT_RPC_LOG_RECORDS);
  }
  bool Verify(flatbuffers::Verifier &verifier) const {
    return VerifyTableStart(verifier) &&
           VerifyField<uint32_t>(verifier, VT_NUM_SCANS_REQUESTED_BY_NANOAPP) &&
           VerifyField<uint32_t>(verifier, VT_NUM_SCANS_SERVICED_BY_HARDWARE) &&
           VerifyField<uint32_t>(verifier, VT_NUM_SCANS_SERVICED_BY_CACHE) &&
           VerifyField<uint32_t>(verifier, VT_UPDATED_AT_CHRE_MS) &&
           VerifyField<uint32_t>(verifier, VT_SENT_AT_CHRE_MS) &&
           VerifyField<uint32_t>(verifier, VT_LAST_SUBSCRIPTION_DURATION_MS) &&
           VerifyOffset(verifier, VT_CHANNEL_SCAN_COUNT) &&
           verifier.VerifyVector(channel_scan_count()) &&
           VerifyOffset(verifier, VT_SCAN_RECORDS) &&
           verifier.VerifyVector(scan_records()) &&
           verifier.VerifyVectorOfTables(scan_records()) &&
           VerifyOffset(verifier, VT_RPC_LOG_RECORDS) &&
           verifier.VerifyVector(rpc_log_records()) &&
           verifier.VerifyVectorOfTables(rpc_log_records()) &&
           verifier.EndTable();
  }
};

struct ScanStatsBuilder {
  typedef ScanStats Table;
  flatbuffers::FlatBufferBuilder &fbb_;
  flatbuffers::uoffset_t start_;
  void add_num_scans_requested_by_nanoapp(uint32_t num_scans_requested_by_nanoapp) {
    fbb_.AddElement<uint32_t>(ScanStats::VT_NUM_SCANS_REQUESTED_BY_NANOAPP, num_scans_requested_by_nanoapp, 0);
  }
  void add_num_scans_serviced_by_hardware(uint32_t num_scans_serviced_by_hardware) {
    fbb_.AddElement<uint32_t>(ScanStats::VT_NUM_SCANS_SERVICED_BY_HARDWARE, num_scans_serviced_by_hardware, 0);
  }
  void add_num_scans_serviced_by_cache(uint32_t num_scans_serviced_by_cache) {
    fbb_.AddElement<uint32_t>(ScanStats::VT_NUM_SCANS_SERVICED_BY_CACHE, num_scans_serviced_by_cache, 0);
  }
  void add_updated_at_chre_ms(uint32_t updated_at_chre_ms) {
    fbb_.AddElement<uint32_t>(ScanStats::VT_UPDATED_AT_CHRE_MS, updated_at_chre_ms, 0);
  }
  void add_sent_at_chre_ms(uint32_t sent_at_chre_ms) {
    fbb_.AddElement<uint32_t>(ScanStats::VT_SENT_AT_CHRE_MS, sent_at_chre_ms, 0);
  }
  void add_last_subscription_duration_ms(uint32_t last_subscription_duration_ms) {
    fbb_.AddElement<uint32_t>(ScanStats::VT_LAST_SUBSCRIPTION_DURATION_MS, last_subscription_duration_ms, 0);
  }
  void add_channel_scan_count(flatbuffers::Offset<flatbuffers::Vector<uint8_t>> channel_scan_count) {
    fbb_.AddOffset(ScanStats::VT_CHANNEL_SCAN_COUNT, channel_scan_count);
  }
  void add_scan_records(flatbuffers::Offset<flatbuffers::Vector<flatbuffers::Offset<wifi_offload::fbs::ScanRecord>>> scan_records) {
    fbb_.AddOffset(ScanStats::VT_SCAN_RECORDS, scan_records);
  }
  void add_rpc_log_records(flatbuffers::Offset<flatbuffers::Vector<flatbuffers::Offset<wifi_offload::fbs::RpcLogRecord>>> rpc_log_records) {
    fbb_.AddOffset(ScanStats::VT_RPC_LOG_RECORDS, rpc_log_records);
  }
  explicit ScanStatsBuilder(flatbuffers::FlatBufferBuilder &_fbb)
        : fbb_(_fbb) {
    start_ = fbb_.StartTable();
  }
  ScanStatsBuilder &operator=(const ScanStatsBuilder &);
  flatbuffers::Offset<ScanStats> Finish() {
    const auto end = fbb_.EndTable(start_);
    auto o = flatbuffers::Offset<ScanStats>(end);
    return o;
  }
};

inline flatbuffers::Offset<ScanStats> CreateScanStats(
    flatbuffers::FlatBufferBuilder &_fbb,
    uint32_t num_scans_requested_by_nanoapp = 0,
    uint32_t num_scans_serviced_by_hardware = 0,
    uint32_t num_scans_serviced_by_cache = 0,
    uint32_t updated_at_chre_ms = 0,
    uint32_t sent_at_chre_ms = 0,
    uint32_t last_subscription_duration_ms = 0,
    flatbuffers::Offset<flatbuffers::Vector<uint8_t>> channel_scan_count = 0,
    flatbuffers::Offset<flatbuffers::Vector<flatbuffers::Offset<wifi_offload::fbs::ScanRecord>>> scan_records = 0,
    flatbuffers::Offset<flatbuffers::Vector<flatbuffers::Offset<wifi_offload::fbs::RpcLogRecord>>> rpc_log_records = 0) {
  ScanStatsBuilder builder_(_fbb);
  builder_.add_rpc_log_records(rpc_log_records);
  builder_.add_scan_records(scan_records);
  builder_.add_channel_scan_count(channel_scan_count);
  builder_.add_last_subscription_duration_ms(last_subscription_duration_ms);
  builder_.add_sent_at_chre_ms(sent_at_chre_ms);
  builder_.add_updated_at_chre_ms(updated_at_chre_ms);
  builder_.add_num_scans_serviced_by_cache(num_scans_serviced_by_cache);
  builder_.add_num_scans_serviced_by_hardware(num_scans_serviced_by_hardware);
  builder_.add_num_scans_requested_by_nanoapp(num_scans_requested_by_nanoapp);
  return builder_.Finish();
}

inline flatbuffers::Offset<ScanStats> CreateScanStatsDirect(
    flatbuffers::FlatBufferBuilder &_fbb,
    uint32_t num_scans_requested_by_nanoapp = 0,
    uint32_t num_scans_serviced_by_hardware = 0,
    uint32_t num_scans_serviced_by_cache = 0,
    uint32_t updated_at_chre_ms = 0,
    uint32_t sent_at_chre_ms = 0,
    uint32_t last_subscription_duration_ms = 0,
    const std::vector<uint8_t> *channel_scan_count = nullptr,
    const std::vector<flatbuffers::Offset<wifi_offload::fbs::ScanRecord>> *scan_records = nullptr,
    const std::vector<flatbuffers::Offset<wifi_offload::fbs::RpcLogRecord>> *rpc_log_records = nullptr) {
  auto channel_scan_count__ = channel_scan_count ? _fbb.CreateVector<uint8_t>(*channel_scan_count) : 0;
  auto scan_records__ = scan_records ? _fbb.CreateVector<flatbuffers::Offset<wifi_offload::fbs::ScanRecord>>(*scan_records) : 0;
  auto rpc_log_records__ = rpc_log_records ? _fbb.CreateVector<flatbuffers::Offset<wifi_offload::fbs::RpcLogRecord>>(*rpc_log_records) : 0;
  return wifi_offload::fbs::CreateScanStats(
      _fbb,
      num_scans_requested_by_nanoapp,
      num_scans_serviced_by_hardware,
      num_scans_serviced_by_cache,
      updated_at_chre_ms,
      sent_at_chre_ms,
      last_subscription_duration_ms,
      channel_scan_count__,
      scan_records__,
      rpc_log_records__);
}

}  // namespace fbs
}  // namespace wifi_offload

#endif  // FLATBUFFERS_GENERATED_FLATBUFFERSTYPES_WIFI_OFFLOAD_FBS_H_
