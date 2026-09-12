#include "chinesepoint/study/StudyState.h"

#include <cmath>
#include <cstring>

namespace ChinesePoint::Study {
namespace {

constexpr uint8_t kMagic[] = {'C', 'P', 'S', 'T', 'A', 'T', 'E', '1'};
constexpr size_t kRecordChecksumOffset = 52;

void writeU16(uint8_t* output, const uint16_t value) {
  output[0] = static_cast<uint8_t>(value & 0xFFu);
  output[1] = static_cast<uint8_t>((value >> 8) & 0xFFu);
}
void writeU32(uint8_t* output, const uint32_t value) {
  for (size_t index = 0; index < sizeof(value); ++index) {
    output[index] = static_cast<uint8_t>((value >> (index * 8)) & 0xFFu);
  }
}
void writeU64(uint8_t* output, const uint64_t value) {
  for (size_t index = 0; index < sizeof(value); ++index) {
    output[index] = static_cast<uint8_t>((value >> (index * 8)) & 0xFFu);
  }
}
uint16_t readU16(const uint8_t* input) {
  return static_cast<uint16_t>(input[0]) | (static_cast<uint16_t>(input[1]) << 8);
}
uint32_t readU32(const uint8_t* input) {
  uint32_t value = 0;
  for (size_t index = 0; index < sizeof(value); ++index) value |= static_cast<uint32_t>(input[index]) << (index * 8);
  return value;
}
uint64_t readU64(const uint8_t* input) {
  uint64_t value = 0;
  for (size_t index = 0; index < sizeof(value); ++index) value |= static_cast<uint64_t>(input[index]) << (index * 8);
  return value;
}
void writeF32(uint8_t* output, const float value) {
  uint32_t raw = 0;
  static_assert(sizeof(raw) == sizeof(value));
  std::memcpy(&raw, &value, sizeof(raw));
  writeU32(output, raw);
}
float readF32(const uint8_t* input) {
  const uint32_t raw = readU32(input);
  float value = 0.0f;
  static_assert(sizeof(raw) == sizeof(value));
  std::memcpy(&value, &raw, sizeof(value));
  return value;
}
bool validState(const CardState state) {
  return static_cast<uint8_t>(state) <= static_cast<uint8_t>(CardState::Relearning);
}
StateDecodeStatus validate(const StateEntry& entry) {
  if (entry.cardId <= 0) return StateDecodeStatus::BadCardId;
  if (!validState(entry.state)) return StateDecodeStatus::BadState;
  if (entry.flags != 0) return StateDecodeStatus::UnsupportedFlags;
  if (!std::isfinite(entry.stability) || !std::isfinite(entry.difficulty)) return StateDecodeStatus::NonFiniteValue;
  return StateDecodeStatus::Ok;
}

}  // namespace

uint32_t stateCrc32(const uint8_t* data, const size_t size) {
  uint32_t crc = 0xFFFFFFFFu;
  for (size_t index = 0; index < size; ++index) {
    crc ^= data[index];
    for (int bit = 0; bit < 8; ++bit) {
      const uint32_t mask = -(crc & 1u);
      crc = (crc >> 1) ^ (0xEDB88320u & mask);
    }
  }
  return crc ^ 0xFFFFFFFFu;
}

bool encodeStateHeader(const StateHeader& header, std::array<uint8_t, kStateHeaderBytes>& output) {
  auto* bytes = output.data();
  std::memcpy(bytes, kMagic, sizeof(kMagic));
  writeU16(bytes + 8, kStateSchemaVersion);
  writeU16(bytes + 10, kStateRecordBytes);
  writeU32(bytes + 12, header.recordCount);
  writeU64(bytes + 16, header.generation);
  writeU32(bytes + 24, header.recordsCrc32);
  return true;
}

StateDecodeStatus decodeStateHeader(const uint8_t* input, const size_t size, StateHeader& output) {
  if (input == nullptr || size != kStateHeaderBytes) return StateDecodeStatus::BadArgument;
  if (std::memcmp(input, kMagic, sizeof(kMagic)) != 0) return StateDecodeStatus::BadMagic;
  const uint16_t version = readU16(input + 8);
  if (version != kStateSchemaVersion) return StateDecodeStatus::UnsupportedVersion;
  if (readU16(input + 10) != kStateRecordBytes) return StateDecodeStatus::BadRecordSize;
  output.version = version;
  output.recordCount = readU32(input + 12);
  output.generation = readU64(input + 16);
  output.recordsCrc32 = readU32(input + 24);
  return StateDecodeStatus::Ok;
}

bool encodeStateEntry(const StateEntry& entry, std::array<uint8_t, kStateRecordBytes>& output) {
  if (validate(entry) != StateDecodeStatus::Ok) return false;
  auto* bytes = output.data();
  writeU64(bytes, static_cast<uint64_t>(entry.cardId));
  bytes[8] = static_cast<uint8_t>(entry.state);
  bytes[9] = entry.step;
  writeU16(bytes + 10, entry.flags);
  writeU64(bytes + 12, static_cast<uint64_t>(entry.dueAtMs));
  writeU64(bytes + 20, static_cast<uint64_t>(entry.lastReviewAtMs));
  writeF32(bytes + 28, entry.stability);
  writeF32(bytes + 32, entry.difficulty);
  writeU32(bytes + 36, entry.reps);
  writeU32(bytes + 40, entry.lapses);
  writeU64(bytes + 44, entry.lastRevlogOffset);
  writeU32(bytes + kRecordChecksumOffset, stateCrc32(bytes, kRecordChecksumOffset));
  return true;
}

StateDecodeStatus decodeStateEntry(const uint8_t* input, const size_t size, StateEntry& output) {
  if (input == nullptr || size != kStateRecordBytes) return StateDecodeStatus::BadArgument;
  if (readU32(input + kRecordChecksumOffset) != stateCrc32(input, kRecordChecksumOffset)) {
    return StateDecodeStatus::BadChecksum;
  }
  StateEntry candidate{};
  candidate.cardId = static_cast<int64_t>(readU64(input));
  candidate.state = static_cast<CardState>(input[8]);
  candidate.step = input[9];
  candidate.flags = readU16(input + 10);
  candidate.dueAtMs = static_cast<int64_t>(readU64(input + 12));
  candidate.lastReviewAtMs = static_cast<int64_t>(readU64(input + 20));
  candidate.stability = readF32(input + 28);
  candidate.difficulty = readF32(input + 32);
  candidate.reps = readU32(input + 36);
  candidate.lapses = readU32(input + 40);
  candidate.lastRevlogOffset = readU64(input + 44);
  const StateDecodeStatus status = validate(candidate);
  if (status != StateDecodeStatus::Ok) return status;
  output = candidate;
  return StateDecodeStatus::Ok;
}

}  // namespace ChinesePoint::Study
