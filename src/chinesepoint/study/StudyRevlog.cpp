#include "chinesepoint/study/StudyRevlog.h"

#include <cmath>
#include <cstring>

namespace ChinesePoint::Study {
namespace {

constexpr size_t kChecksumOffset = 60;

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
  for (size_t index = 0; index < sizeof(value); ++index) {
    value |= static_cast<uint32_t>(input[index]) << (index * 8);
  }
  return value;
}

uint64_t readU64(const uint8_t* input) {
  uint64_t value = 0;
  for (size_t index = 0; index < sizeof(value); ++index) {
    value |= static_cast<uint64_t>(input[index]) << (index * 8);
  }
  return value;
}

void writeF32(uint8_t* output, const float value) {
  static_assert(sizeof(float) == sizeof(uint32_t));
  uint32_t raw = 0;
  std::memcpy(&raw, &value, sizeof(raw));
  writeU32(output, raw);
}

float readF32(const uint8_t* input) {
  static_assert(sizeof(float) == sizeof(uint32_t));
  const uint32_t raw = readU32(input);
  float value = 0.0f;
  std::memcpy(&value, &raw, sizeof(value));
  return value;
}

uint32_t crc32(const uint8_t* data, const size_t size) {
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

bool validState(const CardState state) {
  return static_cast<uint8_t>(state) <= static_cast<uint8_t>(CardState::Relearning);
}

bool finiteFields(const ReviewEntry& entry) {
  return std::isfinite(entry.previousStability) && std::isfinite(entry.nextStability) &&
         std::isfinite(entry.previousDifficulty) && std::isfinite(entry.nextDifficulty);
}

ReviewDecodeStatus validate(const ReviewEntry& entry) {
  if (entry.rating < 1 || entry.rating > 4) return ReviewDecodeStatus::BadRating;
  if (!validState(entry.previousState) || !validState(entry.nextState)) return ReviewDecodeStatus::BadState;
  if (entry.flags != 0) return ReviewDecodeStatus::UnsupportedFlags;
  if (!finiteFields(entry)) return ReviewDecodeStatus::NonFiniteValue;
  return ReviewDecodeStatus::Ok;
}

}  // namespace

bool encodeReviewEntry(const ReviewEntry& entry, std::array<uint8_t, kReviewEntryBytes>& output) {
  if (validate(entry) != ReviewDecodeStatus::Ok) return false;

  auto* bytes = output.data();
  std::memcpy(bytes, entry.id.data(), entry.id.size());
  writeU64(bytes + 16, static_cast<uint64_t>(entry.cardId));
  writeU64(bytes + 24, static_cast<uint64_t>(entry.reviewedAtMs));
  bytes[32] = entry.rating;
  bytes[33] = static_cast<uint8_t>(entry.previousState);
  bytes[34] = static_cast<uint8_t>(entry.nextState);
  bytes[35] = entry.flags;
  writeF32(bytes + 36, entry.previousStability);
  writeF32(bytes + 40, entry.nextStability);
  writeF32(bytes + 44, entry.previousDifficulty);
  writeF32(bytes + 48, entry.nextDifficulty);
  writeU32(bytes + 52, static_cast<uint32_t>(entry.intervalDays));
  writeU16(bytes + 56, entry.elapsedDays);
  writeU16(bytes + 58, entry.durationMs);
  writeU32(bytes + kChecksumOffset, crc32(bytes, kChecksumOffset));
  return true;
}

ReviewDecodeStatus decodeReviewEntry(const uint8_t* input, const size_t size, ReviewEntry& output) {
  if (input == nullptr || size != kReviewEntryBytes) return ReviewDecodeStatus::BadArgument;
  if (readU32(input + kChecksumOffset) != crc32(input, kChecksumOffset)) return ReviewDecodeStatus::BadChecksum;

  ReviewEntry candidate{};
  std::memcpy(candidate.id.data(), input, candidate.id.size());
  candidate.cardId = static_cast<int64_t>(readU64(input + 16));
  candidate.reviewedAtMs = static_cast<int64_t>(readU64(input + 24));
  candidate.rating = input[32];
  candidate.previousState = static_cast<CardState>(input[33]);
  candidate.nextState = static_cast<CardState>(input[34]);
  candidate.flags = input[35];
  candidate.previousStability = readF32(input + 36);
  candidate.nextStability = readF32(input + 40);
  candidate.previousDifficulty = readF32(input + 44);
  candidate.nextDifficulty = readF32(input + 48);
  candidate.intervalDays = static_cast<int32_t>(readU32(input + 52));
  candidate.elapsedDays = readU16(input + 56);
  candidate.durationMs = readU16(input + 58);

  const ReviewDecodeStatus status = validate(candidate);
  if (status != ReviewDecodeStatus::Ok) return status;
  output = candidate;
  return ReviewDecodeStatus::Ok;
}

}  // namespace ChinesePoint::Study
