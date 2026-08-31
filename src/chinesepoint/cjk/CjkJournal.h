#pragma once

#include <array>
#include <cstddef>
#include <cstdint>

#include "chinesepoint/cjk/CjkLearnerModel.h"

namespace ChinesePoint::Cjk::Journal {

constexpr uint8_t kVersion = 1;
constexpr size_t kHeaderBytes = 16;
constexpr size_t kMaxPayloadBytes = 1200;

enum class RecordType : uint8_t {
  EntrySnapshot = 1,
  ReviewMutation = 2,
  StudyClock = 3,
};

enum class DecodeStatus : uint8_t {
  Ok,
  NeedMoreData,
  BadMagic,
  UnsupportedVersion,
  UnknownRecordType,
  PayloadTooLarge,
  BadChecksum,
};

struct EncodedRecord {
  std::array<uint8_t, kHeaderBytes + kMaxPayloadBytes> bytes{};
  size_t size = 0;
};

struct PayloadBuffer {
  std::array<uint8_t, kMaxPayloadBytes> bytes{};
  size_t size = 0;
};

struct RecordView {
  RecordType type = RecordType::EntrySnapshot;
  uint32_t sequence = 0;
  const uint8_t* payload = nullptr;
  size_t payloadSize = 0;
  size_t totalSize = 0;
};

uint32_t crc32(const uint8_t* data, size_t size, uint32_t seed = 0xFFFFFFFFu);

bool encodeRecord(RecordType type, uint32_t sequence, const uint8_t* payload,
                  size_t payloadSize, EncodedRecord& output);
DecodeStatus decodeRecord(const uint8_t* data, size_t size, RecordView& output);

bool encodeEntry(const LearnerEntry& entry, PayloadBuffer& output);
bool decodeEntry(const uint8_t* data, size_t size, LearnerEntry& output);

}  // namespace ChinesePoint::Cjk::Journal
