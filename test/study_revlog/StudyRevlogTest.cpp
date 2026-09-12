#include <array>
#include <cmath>
#include <cstdint>
#include <limits>

#include <gtest/gtest.h>

#include "chinesepoint/study/StudyRevlog.h"

namespace {

ChinesePoint::Study::ReviewEntry sampleEntry() {
  using namespace ChinesePoint::Study;
  ReviewEntry entry{};
  for (size_t index = 0; index < entry.id.size(); ++index) entry.id[index] = static_cast<uint8_t>(index + 1);
  entry.cardId = 1700000000001;
  entry.reviewedAtMs = 1720000000123;
  entry.rating = 3;
  entry.previousState = CardState::Learning;
  entry.nextState = CardState::Review;
  entry.previousStability = 1.25f;
  entry.nextStability = 3.5f;
  entry.previousDifficulty = 6.0f;
  entry.nextDifficulty = 5.2f;
  entry.intervalDays = 3;
  entry.elapsedDays = 1;
  entry.durationMs = 4800;
  return entry;
}

TEST(StudyRevlog, FixedWidthRoundTripPreservesAllFields) {
  using namespace ChinesePoint::Study;
  const ReviewEntry expected = sampleEntry();
  std::array<uint8_t, kReviewEntryBytes> encoded{};
  ASSERT_TRUE(encodeReviewEntry(expected, encoded));

  ReviewEntry decoded{};
  ASSERT_EQ(decodeReviewEntry(encoded.data(), encoded.size(), decoded), ReviewDecodeStatus::Ok);
  EXPECT_EQ(decoded.id, expected.id);
  EXPECT_EQ(decoded.cardId, expected.cardId);
  EXPECT_EQ(decoded.reviewedAtMs, expected.reviewedAtMs);
  EXPECT_EQ(decoded.rating, expected.rating);
  EXPECT_EQ(decoded.previousState, expected.previousState);
  EXPECT_EQ(decoded.nextState, expected.nextState);
  EXPECT_FLOAT_EQ(decoded.previousStability, expected.previousStability);
  EXPECT_FLOAT_EQ(decoded.nextStability, expected.nextStability);
  EXPECT_FLOAT_EQ(decoded.previousDifficulty, expected.previousDifficulty);
  EXPECT_FLOAT_EQ(decoded.nextDifficulty, expected.nextDifficulty);
  EXPECT_EQ(decoded.intervalDays, expected.intervalDays);
  EXPECT_EQ(decoded.elapsedDays, expected.elapsedDays);
  EXPECT_EQ(decoded.durationMs, expected.durationMs);
}

TEST(StudyRevlog, RejectsTornAndTamperedRecords) {
  using namespace ChinesePoint::Study;
  std::array<uint8_t, kReviewEntryBytes> encoded{};
  ASSERT_TRUE(encodeReviewEntry(sampleEntry(), encoded));
  ReviewEntry decoded{};
  EXPECT_EQ(decodeReviewEntry(encoded.data(), encoded.size() - 1, decoded), ReviewDecodeStatus::BadArgument);
  encoded[42] ^= 0x80u;
  EXPECT_EQ(decodeReviewEntry(encoded.data(), encoded.size(), decoded), ReviewDecodeStatus::BadChecksum);
}

TEST(StudyRevlog, RejectsInvalidValuesBeforeOrAfterEncoding) {
  using namespace ChinesePoint::Study;
  ReviewEntry invalid = sampleEntry();
  std::array<uint8_t, kReviewEntryBytes> encoded{};
  invalid.rating = 5;
  EXPECT_FALSE(encodeReviewEntry(invalid, encoded));
  invalid = sampleEntry();
  invalid.flags = 1;
  EXPECT_FALSE(encodeReviewEntry(invalid, encoded));
  invalid = sampleEntry();
  invalid.nextStability = std::numeric_limits<float>::infinity();
  EXPECT_FALSE(encodeReviewEntry(invalid, encoded));

  ASSERT_TRUE(encodeReviewEntry(sampleEntry(), encoded));
  encoded[32] = 0;
  // Rewriting just the payload makes the existing checksum fail first.
  ReviewEntry decoded{};
  EXPECT_EQ(decodeReviewEntry(encoded.data(), encoded.size(), decoded), ReviewDecodeStatus::BadChecksum);
}

}  // namespace
