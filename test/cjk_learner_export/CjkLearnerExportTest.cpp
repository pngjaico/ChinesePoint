#include <gtest/gtest.h>

#include <limits>
#include <string>

#include "chinesepoint/cjk/CjkLearnerExport.h"

namespace {
using ChinesePoint::Cjk::ExportSink;
using ChinesePoint::Cjk::LearnerEntry;
using ChinesePoint::Cjk::ScheduleAuthority;
using ChinesePoint::Cjk::WordStatus;

bool appendToString(void* context, const char* bytes, const size_t size) {
  static_cast<std::string*>(context)->append(bytes, size);
  return true;
}

bool rejectWrites(void*, const char*, size_t) { return false; }

LearnerEntry exampleEntry() {
  LearnerEntry entry;
  entry.wordId = 0x0123456789abcdefULL;
  entry.headword = "学习";
  entry.status = WordStatus::Saved;
  entry.encounterCount = 7;
  entry.firstSeenStudyMs = 100;
  entry.lastSeenStudyMs = 200;
  entry.bookPath = "/books/中文.epub";
  entry.sourceAnchor = {3, 44, 2, 99};
  entry.sourceSentence = "他说：\"你好\"\\n";
  entry.review.authority = ScheduleAuthority::Anki;
  entry.review.ankiCardId = 987654321012345ULL;
  return entry;
}

TEST(CjkLearnerExport, WritesDeterministicVersionedJsonlWithEscapes) {
  const auto entry = exampleEntry();
  std::string first;
  std::string second;
  ASSERT_TRUE(ChinesePoint::Cjk::writeLearnerExportJsonl(&entry, 1, {&first, appendToString}));
  ASSERT_TRUE(ChinesePoint::Cjk::writeLearnerExportJsonl(&entry, 1, {&second, appendToString}));
  EXPECT_EQ(first, second);
  EXPECT_NE(first.find("{\"schema\":\"chinesepoint-learner-export\",\"version\":1,\"format\":\"ndjson\"}\n"),
            std::string::npos);
  EXPECT_NE(first.find("\"word_id\":\"0123456789abcdef\""), std::string::npos);
  EXPECT_NE(first.find("\"headword\":\"学习\""), std::string::npos);
  EXPECT_NE(first.find("他说：\\\"你好\\\"\\\\n"), std::string::npos);
  EXPECT_NE(first.find("\"anki_card_id\":\"987654321012345\""), std::string::npos);
}

TEST(CjkLearnerExport, FailsClosedForInvalidValuesOrSinkFailure) {
  auto entry = exampleEntry();
  std::string output;
  entry.review.difficulty = std::numeric_limits<float>::quiet_NaN();
  EXPECT_FALSE(ChinesePoint::Cjk::writeLearnerExportJsonl(&entry, 1, {&output, appendToString}));

  entry = exampleEntry();
  entry.headword.assign("\xFF", 1);
  output.clear();
  EXPECT_FALSE(ChinesePoint::Cjk::writeLearnerExportJsonl(&entry, 1, {&output, appendToString}));
  EXPECT_FALSE(ChinesePoint::Cjk::writeLearnerExportJsonl(&entry, 1, {nullptr, rejectWrites}));
}

}  // namespace
