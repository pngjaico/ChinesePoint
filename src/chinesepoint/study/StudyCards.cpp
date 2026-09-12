#include "chinesepoint/study/StudyCards.h"

#include <cmath>
#include <cstring>

namespace ChinesePoint::Study {
namespace {

constexpr uint8_t kMagic[] = {'C', 'P', 'C', 'A', 'R', 'D', 'S', '1'};

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

bool validState(const CardState state) {
  return static_cast<uint8_t>(state) <= static_cast<uint8_t>(CardState::Relearning);
}

CardsDecodeStatus validateCard(const CardSnapshot& card) {
  if (card.cardId <= 0 || card.noteId <= 0) return CardsDecodeStatus::BadCardId;
  if (!validState(card.state)) return CardsDecodeStatus::BadState;
  if (card.flags != 0) return CardsDecodeStatus::UnsupportedFlags;
  if (!std::isfinite(card.stability) || !std::isfinite(card.difficulty)) return CardsDecodeStatus::NonFiniteValue;
  return CardsDecodeStatus::Ok;
}

}  // namespace

bool encodeCardsHeader(const uint32_t recordCount, std::array<uint8_t, kCardsHeaderBytes>& output) {
  auto* bytes = output.data();
  std::memcpy(bytes, kMagic, sizeof(kMagic));
  writeU16(bytes + 8, kCardsSchemaVersion);
  writeU16(bytes + 10, kCardSnapshotBytes);
  writeU32(bytes + 12, recordCount);
  return true;
}

CardsDecodeStatus decodeCardsHeader(const uint8_t* input, const size_t size, CardsHeader& output) {
  if (input == nullptr || size != kCardsHeaderBytes) return CardsDecodeStatus::BadArgument;
  if (std::memcmp(input, kMagic, sizeof(kMagic)) != 0) return CardsDecodeStatus::BadMagic;
  const uint16_t version = readU16(input + 8);
  if (version != kCardsSchemaVersion) return CardsDecodeStatus::UnsupportedVersion;
  if (readU16(input + 10) != kCardSnapshotBytes) return CardsDecodeStatus::BadRecordSize;
  output.version = version;
  output.recordCount = readU32(input + 12);
  return CardsDecodeStatus::Ok;
}

bool encodeCardSnapshot(const CardSnapshot& card, std::array<uint8_t, kCardSnapshotBytes>& output) {
  if (validateCard(card) != CardsDecodeStatus::Ok) return false;

  auto* bytes = output.data();
  writeU64(bytes, static_cast<uint64_t>(card.cardId));
  writeU64(bytes + 8, static_cast<uint64_t>(card.noteId));
  bytes[16] = static_cast<uint8_t>(card.state);
  bytes[17] = card.step;
  writeU16(bytes + 18, card.flags);
  writeU32(bytes + 20, static_cast<uint32_t>(card.dueDay));
  writeU16(bytes + 24, card.dueMinute);
  writeU32(bytes + 26, static_cast<uint32_t>(card.lastReviewDay));
  writeF32(bytes + 30, card.stability);
  writeF32(bytes + 34, card.difficulty);
  writeU16(bytes + 38, card.elapsedDays);
  writeU16(bytes + 40, card.scheduledDays);
  writeU16(bytes + 42, card.reps);
  writeU16(bytes + 44, card.lapses);
  writeU64(bytes + 46, static_cast<uint64_t>(card.lastReviewAtMs));
  writeU16(bytes + 54, 0);
  return true;
}

CardsDecodeStatus decodeCardSnapshot(const uint8_t* input, const size_t size, CardSnapshot& output) {
  if (input == nullptr || size != kCardSnapshotBytes) return CardsDecodeStatus::BadArgument;
  if (readU16(input + 54) != 0) return CardsDecodeStatus::NonZeroReserved;

  CardSnapshot candidate{};
  candidate.cardId = static_cast<int64_t>(readU64(input));
  candidate.noteId = static_cast<int64_t>(readU64(input + 8));
  candidate.state = static_cast<CardState>(input[16]);
  candidate.step = input[17];
  candidate.flags = readU16(input + 18);
  candidate.dueDay = static_cast<int32_t>(readU32(input + 20));
  candidate.dueMinute = readU16(input + 24);
  candidate.lastReviewDay = static_cast<int32_t>(readU32(input + 26));
  candidate.stability = readF32(input + 30);
  candidate.difficulty = readF32(input + 34);
  candidate.elapsedDays = readU16(input + 38);
  candidate.scheduledDays = readU16(input + 40);
  candidate.reps = readU16(input + 42);
  candidate.lapses = readU16(input + 44);
  candidate.lastReviewAtMs = static_cast<int64_t>(readU64(input + 46));

  const CardsDecodeStatus status = validateCard(candidate);
  if (status != CardsDecodeStatus::Ok) return status;
  output = candidate;
  return CardsDecodeStatus::Ok;
}

}  // namespace ChinesePoint::Study
