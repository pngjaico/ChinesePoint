#pragma once

#include <array>
#include <cstddef>
#include <cstdint>

#include "chinesepoint/study/StudyTypes.h"

namespace ChinesePoint::Study {

constexpr uint16_t kCardsSchemaVersion = 1;
constexpr size_t kCardsHeaderBytes = 16;
constexpr size_t kCardSnapshotBytes = 56;

struct CardsHeader {
  uint16_t version = kCardsSchemaVersion;
  uint32_t recordCount = 0;
};

enum class CardsDecodeStatus : uint8_t {
  Ok,
  BadArgument,
  BadMagic,
  UnsupportedVersion,
  BadRecordSize,
  BadCardId,
  BadState,
  UnsupportedFlags,
  NonFiniteValue,
  NonZeroReserved,
};

// This is the immutable build-time card snapshot. Scheduling mutations belong
// in state.dat and revlog.dat, never in cards.dat in place.
struct CardSnapshot {
  int64_t cardId = 0;
  int64_t noteId = 0;
  CardState state = CardState::New;
  uint8_t step = 0;
  uint16_t flags = 0;
  int32_t dueDay = 0;
  uint16_t dueMinute = 0;
  int32_t lastReviewDay = 0;
  float stability = 0.0f;
  float difficulty = 0.0f;
  uint16_t elapsedDays = 0;
  uint16_t scheduledDays = 0;
  uint16_t reps = 0;
  uint16_t lapses = 0;
  int64_t lastReviewAtMs = 0;
};

bool encodeCardsHeader(uint32_t recordCount, std::array<uint8_t, kCardsHeaderBytes>& output);
CardsDecodeStatus decodeCardsHeader(const uint8_t* input, size_t size, CardsHeader& output);

bool encodeCardSnapshot(const CardSnapshot& card, std::array<uint8_t, kCardSnapshotBytes>& output);
CardsDecodeStatus decodeCardSnapshot(const uint8_t* input, size_t size, CardSnapshot& output);

}  // namespace ChinesePoint::Study
