#pragma once

#include <array>
#include <cstddef>
#include <cstdint>

#include "chinesepoint/cjk/CjkLearnerModel.h"
#include "chinesepoint/cjk/CjkStudyClock.h"

namespace ChinesePoint::Cjk::Journal {

constexpr uint8_t kVersion = 1;
constexpr size_t kHeaderBytes = 16;
constexpr size_t kMaxPayloadBytes = 1200;

enum class RecordType : uint8_t {
  EntrySnapshot = 1,
  ReviewMutation = 2,
  StudyClock = 3,
  FlashcardAnswer = 4,
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
bool encodeStudyClock(const StudyClockState& state, PayloadBuffer& output);
bool decodeStudyClock(const uint8_t* data, size_t size, StudyClockState& output);
// A review rating and the clock used to calculate its due date share one
// checksummed record. Replay never observes one without the other.
bool encodeReviewMutation(const LearnerEntry& entry, const StudyClockState& clock, PayloadBuffer& output);
bool decodeReviewMutation(const uint8_t* data, size_t size, LearnerEntry& entry, StudyClockState& clock);
// Definitions are separate from entry snapshots so the append-only v1 journal
// stays backward-readable and its fixed payload cap remains enforceable.
bool encodeFlashcardAnswer(uint64_t wordId, std::string_view headword, std::string_view answer, PayloadBuffer& output);
bool decodeFlashcardAnswer(const uint8_t* data, size_t size, uint64_t& wordId, std::string& headword,
                           std::string& answer);

}  // namespace ChinesePoint::Cjk::Journal
