#include <array>
#include <cstdint>
#include <limits>

#include <gtest/gtest.h>

#include "chinesepoint/study/StudyCards.h"

namespace {

ChinesePoint::Study::CardSnapshot sampleCard() {
  using namespace ChinesePoint::Study;
  CardSnapshot card{};
  card.cardId = 1700000000001;
  card.noteId = 1700000000000;
  card.state = CardState::Review;
  card.step = 2;
  card.dueDay = 19880;
  card.dueMinute = 540;
  card.lastReviewDay = 19877;
  card.stability = 4.25f;
  card.difficulty = 5.5f;
  card.elapsedDays = 3;
  card.scheduledDays = 5;
  card.reps = 42;
  card.lapses = 4;
  card.lastReviewAtMs = 1720000000123;
  return card;
}

TEST(StudyCards, HeaderRoundTripFixesSchemaAndRecordWidth) {
  using namespace ChinesePoint::Study;
  std::array<uint8_t, kCardsHeaderBytes> encoded{};
  ASSERT_TRUE(encodeCardsHeader(123, encoded));
  CardsHeader decoded{};
  ASSERT_EQ(decodeCardsHeader(encoded.data(), encoded.size(), decoded), CardsDecodeStatus::Ok);
  EXPECT_EQ(decoded.version, kCardsSchemaVersion);
  EXPECT_EQ(decoded.recordCount, 123u);
  encoded[10] = 0;
  EXPECT_EQ(decodeCardsHeader(encoded.data(), encoded.size(), decoded), CardsDecodeStatus::BadRecordSize);
}

TEST(StudyCards, SnapshotRoundTripUsesSpecifiedOffsets) {
  using namespace ChinesePoint::Study;
  const CardSnapshot expected = sampleCard();
  std::array<uint8_t, kCardSnapshotBytes> encoded{};
  ASSERT_TRUE(encodeCardSnapshot(expected, encoded));
  CardSnapshot decoded{};
  ASSERT_EQ(decodeCardSnapshot(encoded.data(), encoded.size(), decoded), CardsDecodeStatus::Ok);
  EXPECT_EQ(decoded.cardId, expected.cardId);
  EXPECT_EQ(decoded.noteId, expected.noteId);
  EXPECT_EQ(decoded.state, expected.state);
  EXPECT_EQ(decoded.step, expected.step);
  EXPECT_EQ(decoded.dueDay, expected.dueDay);
  EXPECT_EQ(decoded.dueMinute, expected.dueMinute);
  EXPECT_EQ(decoded.lastReviewDay, expected.lastReviewDay);
  EXPECT_FLOAT_EQ(decoded.stability, expected.stability);
  EXPECT_FLOAT_EQ(decoded.difficulty, expected.difficulty);
  EXPECT_EQ(decoded.elapsedDays, expected.elapsedDays);
  EXPECT_EQ(decoded.scheduledDays, expected.scheduledDays);
  EXPECT_EQ(decoded.reps, expected.reps);
  EXPECT_EQ(decoded.lapses, expected.lapses);
  EXPECT_EQ(decoded.lastReviewAtMs, expected.lastReviewAtMs);
}

TEST(StudyCards, RejectsUnsupportedAndCorruptFields) {
  using namespace ChinesePoint::Study;
  CardSnapshot invalid = sampleCard();
  std::array<uint8_t, kCardSnapshotBytes> encoded{};
  invalid.cardId = 0;
  EXPECT_FALSE(encodeCardSnapshot(invalid, encoded));
  invalid = sampleCard();
  invalid.flags = 1;
  EXPECT_FALSE(encodeCardSnapshot(invalid, encoded));
  invalid = sampleCard();
  invalid.stability = std::numeric_limits<float>::infinity();
  EXPECT_FALSE(encodeCardSnapshot(invalid, encoded));

  ASSERT_TRUE(encodeCardSnapshot(sampleCard(), encoded));
  encoded[54] = 1;
  CardSnapshot decoded{};
  EXPECT_EQ(decodeCardSnapshot(encoded.data(), encoded.size(), decoded), CardsDecodeStatus::NonZeroReserved);
}

}  // namespace
