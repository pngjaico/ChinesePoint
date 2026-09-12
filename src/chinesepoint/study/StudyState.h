#pragma once

#include <array>
#include <cstddef>
#include <cstdint>

#include "chinesepoint/study/StudyTypes.h"

namespace ChinesePoint::Study {

constexpr uint16_t kStateSchemaVersion = 1;
constexpr size_t kStateHeaderBytes = 28;
constexpr size_t kStateRecordBytes = 56;

struct StateHeader {
  uint16_t version = kStateSchemaVersion;
  uint32_t recordCount = 0;
  uint64_t generation = 0;
  uint32_t recordsCrc32 = 0;
};

enum class StateDecodeStatus : uint8_t {
  Ok,
  BadArgument,
  BadMagic,
  UnsupportedVersion,
  BadRecordSize,
  BadCardId,
  BadState,
  UnsupportedFlags,
  NonFiniteValue,
  BadChecksum,
};

struct StateEntry {
  int64_t cardId = 0;
  CardState state = CardState::New;
  uint8_t step = 0;
  uint16_t flags = 0;
  int64_t dueAtMs = 0;
  int64_t lastReviewAtMs = 0;
  float stability = 0.0f;
  float difficulty = 0.0f;
  uint32_t reps = 0;
  uint32_t lapses = 0;
  uint64_t lastRevlogOffset = 0;
};

uint32_t stateCrc32(const uint8_t* data, size_t size);
bool encodeStateHeader(const StateHeader& header, std::array<uint8_t, kStateHeaderBytes>& output);
StateDecodeStatus decodeStateHeader(const uint8_t* input, size_t size, StateHeader& output);
bool encodeStateEntry(const StateEntry& entry, std::array<uint8_t, kStateRecordBytes>& output);
StateDecodeStatus decodeStateEntry(const uint8_t* input, size_t size, StateEntry& output);

}  // namespace ChinesePoint::Study
