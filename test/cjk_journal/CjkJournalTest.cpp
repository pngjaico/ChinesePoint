#include <gtest/gtest.h>

#include <array>

#include "chinesepoint/cjk/CjkJournal.h"

namespace {
using ChinesePoint::Cjk::Journal::DecodeStatus;
using ChinesePoint::Cjk::Journal::EncodedRecord;
using ChinesePoint::Cjk::Journal::RecordType;
using ChinesePoint::Cjk::Journal::RecordView;

TEST(CjkJournal, RoundTripPreservesFramingAndPayload) {
  const std::array<uint8_t, 3> payload = {0xA1, 0xB2, 0xC3};
  EncodedRecord encoded;
  ASSERT_TRUE(ChinesePoint::Cjk::Journal::encodeRecord(
      RecordType::EntrySnapshot, 42, payload.data(), payload.size(), encoded));

  RecordView decoded;
  ASSERT_EQ(ChinesePoint::Cjk::Journal::decodeRecord(encoded.bytes.data(), encoded.size, decoded),
            DecodeStatus::Ok);
  EXPECT_EQ(decoded.type, RecordType::EntrySnapshot);
  EXPECT_EQ(decoded.sequence, 42u);
  EXPECT_EQ(decoded.payloadSize, payload.size());
  EXPECT_EQ(decoded.totalSize, encoded.size);
  EXPECT_EQ(decoded.payload[0], payload[0]);
  EXPECT_EQ(decoded.payload[2], payload[2]);
}

TEST(CjkJournal, RejectsTornWritesAndTampering) {
  const std::array<uint8_t, 1> payload = {0xA1};
  EncodedRecord encoded;
  ASSERT_TRUE(ChinesePoint::Cjk::Journal::encodeRecord(
      RecordType::ReviewMutation, 7, payload.data(), payload.size(), encoded));

  RecordView decoded;
  EXPECT_EQ(ChinesePoint::Cjk::Journal::decodeRecord(encoded.bytes.data(), encoded.size - 1, decoded),
            DecodeStatus::NeedMoreData);

  encoded.bytes[ChinesePoint::Cjk::Journal::kHeaderBytes] ^= 0x01u;
  EXPECT_EQ(ChinesePoint::Cjk::Journal::decodeRecord(encoded.bytes.data(), encoded.size, decoded),
            DecodeStatus::BadChecksum);
}

TEST(CjkJournal, BoundsInvalidArgumentsBeforeWriting) {
  EncodedRecord encoded;
  EXPECT_FALSE(ChinesePoint::Cjk::Journal::encodeRecord(
      RecordType::StudyClock, 1, nullptr, 1, encoded));
  EXPECT_FALSE(ChinesePoint::Cjk::Journal::encodeRecord(
      static_cast<RecordType>(99), 1, nullptr, 0, encoded));
}

TEST(CjkJournal, EntryCodecRoundTripsAllLearnerState) {
  ChinesePoint::Cjk::LearnerEntry entry;
  entry.wordId = ChinesePoint::Cjk::stableWordId("学习");
  entry.headword = "学习";
  entry.status = ChinesePoint::Cjk::WordStatus::Learning;
  entry.encounterCount = 3;
  entry.firstSeenStudyMs = 100;
  entry.lastSeenStudyMs = 200;
  entry.bookPath = "/books/mandarin.epub";
  entry.sourceAnchor = {2, 301, 2, 0x12345678u};
  entry.sourceSentence = "我每天学习中文。";
  entry.review.phase = ChinesePoint::Cjk::ReviewPhase::Review;
  entry.review.dueAtMs = 300;
  entry.review.lastReviewAtMs = 200;
  entry.review.difficulty = 4.5f;
  entry.review.stabilityDays = 9.5f;
  entry.review.reps = 5;
  entry.review.lapses = 1;

  ChinesePoint::Cjk::Journal::PayloadBuffer payload;
  ASSERT_TRUE(ChinesePoint::Cjk::Journal::encodeEntry(entry, payload));
  ChinesePoint::Cjk::LearnerEntry decoded;
  ASSERT_TRUE(ChinesePoint::Cjk::Journal::decodeEntry(payload.bytes.data(), payload.size, decoded));
  EXPECT_EQ(decoded.wordId, entry.wordId);
  EXPECT_EQ(decoded.headword, entry.headword);
  EXPECT_EQ(decoded.status, entry.status);
  EXPECT_EQ(decoded.bookPath, entry.bookPath);
  EXPECT_EQ(decoded.sourceAnchor.visibleCodepointOffset, entry.sourceAnchor.visibleCodepointOffset);
  EXPECT_EQ(decoded.sourceSentence, entry.sourceSentence);
  EXPECT_EQ(decoded.review.phase, entry.review.phase);
  EXPECT_FLOAT_EQ(decoded.review.stabilityDays, entry.review.stabilityDays);
}

TEST(CjkJournal, EntryCodecRejectsTrailingOrInvalidData) {
  ChinesePoint::Cjk::LearnerEntry entry;
  entry.wordId = 1;
  entry.headword = "字";
  ChinesePoint::Cjk::Journal::PayloadBuffer payload;
  ASSERT_TRUE(ChinesePoint::Cjk::Journal::encodeEntry(entry, payload));
  payload.bytes[payload.size++] = 0;
  ChinesePoint::Cjk::LearnerEntry decoded;
  EXPECT_FALSE(ChinesePoint::Cjk::Journal::decodeEntry(payload.bytes.data(), payload.size, decoded));
}

}  // namespace
