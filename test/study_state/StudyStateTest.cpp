#include <array>
#include <cstdint>
#include <limits>

#include <gtest/gtest.h>

#include "chinesepoint/study/StudyState.h"

namespace {

ChinesePoint::Study::StateEntry sampleEntry() {
  using namespace ChinesePoint::Study;
  StateEntry entry{};
  entry.cardId = 1700000000001;
  entry.state = CardState::Review;
  entry.step = 2;
  entry.dueAtMs = 1720000000123;
  entry.lastReviewAtMs = 1719000000123;
  entry.stability = 4.25f;
  entry.difficulty = 5.5f;
  entry.reps = 42;
  entry.lapses = 4;
  entry.lastRevlogOffset = 128;
  return entry;
}

TEST(StudyState, HeaderPreservesGenerationAndBodyChecksum) {
  using namespace ChinesePoint::Study;
  StateHeader expected{};
  expected.recordCount = 3;
  expected.generation = 9;
  expected.recordsCrc32 = 0xAABBCCDDu;
  std::array<uint8_t, kStateHeaderBytes> encoded{};
  ASSERT_TRUE(encodeStateHeader(expected, encoded));
  StateHeader decoded{};
  ASSERT_EQ(decodeStateHeader(encoded.data(), encoded.size(), decoded), StateDecodeStatus::Ok);
  EXPECT_EQ(decoded.recordCount, expected.recordCount);
  EXPECT_EQ(decoded.generation, expected.generation);
  EXPECT_EQ(decoded.recordsCrc32, expected.recordsCrc32);
  encoded[10] = 0;
  EXPECT_EQ(decodeStateHeader(encoded.data(), encoded.size(), decoded), StateDecodeStatus::BadRecordSize);
}

TEST(StudyState, EntryRoundTripIsChecksummed) {
  using namespace ChinesePoint::Study;
  const StateEntry expected = sampleEntry();
  std::array<uint8_t, kStateRecordBytes> encoded{};
  ASSERT_TRUE(encodeStateEntry(expected, encoded));
  StateEntry decoded{};
  ASSERT_EQ(decodeStateEntry(encoded.data(), encoded.size(), decoded), StateDecodeStatus::Ok);
  EXPECT_EQ(decoded.cardId, expected.cardId);
  EXPECT_EQ(decoded.state, expected.state);
  EXPECT_EQ(decoded.dueAtMs, expected.dueAtMs);
  EXPECT_EQ(decoded.lastReviewAtMs, expected.lastReviewAtMs);
  EXPECT_FLOAT_EQ(decoded.stability, expected.stability);
  EXPECT_FLOAT_EQ(decoded.difficulty, expected.difficulty);
  EXPECT_EQ(decoded.reps, expected.reps);
  EXPECT_EQ(decoded.lapses, expected.lapses);
  EXPECT_EQ(decoded.lastRevlogOffset, expected.lastRevlogOffset);
  encoded[31] ^= 0x80u;
  EXPECT_EQ(decodeStateEntry(encoded.data(), encoded.size(), decoded), StateDecodeStatus::BadChecksum);
}

TEST(StudyState, RejectsInvalidValuesBeforeWrite) {
  using namespace ChinesePoint::Study;
  StateEntry invalid = sampleEntry();
  std::array<uint8_t, kStateRecordBytes> encoded{};
  invalid.cardId = 0;
  EXPECT_FALSE(encodeStateEntry(invalid, encoded));
  invalid = sampleEntry();
  invalid.flags = 1;
  EXPECT_FALSE(encodeStateEntry(invalid, encoded));
  invalid = sampleEntry();
  invalid.difficulty = std::numeric_limits<float>::infinity();
  EXPECT_FALSE(encodeStateEntry(invalid, encoded));
}

}  // namespace
