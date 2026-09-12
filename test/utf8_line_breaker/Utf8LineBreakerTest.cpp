#include <cstring>
#include <string>

#include <gtest/gtest.h>

#include "util/Utf8LineBreaker.h"

namespace {

int asciiWidth(const char* text) { return static_cast<int>(std::strlen(text)); }

int codepointWidth(const char* text) {
  int count = 0;
  for (const auto* byte = reinterpret_cast<const unsigned char*>(text); *byte != 0; ++byte) {
    if ((*byte & 0xC0U) != 0x80U) ++count;
  }
  return count;
}

}  // namespace

TEST(Utf8LineBreaker, FindsLargestAsciiPrefix) {
  std::string text = "unbroken-text";
  EXPECT_EQ(Utf8LineBreaker::longestPrefixWithinWidth(text, 8, asciiWidth), 8U);
  EXPECT_EQ(text, "unbroken-text");
}

TEST(Utf8LineBreaker, UsesLogarithmicMeasurementsForALargeCjkLine) {
  std::string text;
  text.reserve(8190);
  for (int i = 0; i < 2730; ++i) text += "汉";

  int measurements = 0;
  const auto measure = [&measurements](const char* prefix) {
    ++measurements;
    return codepointWidth(prefix);
  };

  EXPECT_EQ(Utf8LineBreaker::longestPrefixWithinWidth(text, 40, measure), 120U);
  EXPECT_LE(measurements, 14);
  EXPECT_EQ(text.size(), 8190U);
}

TEST(Utf8LineBreaker, NeverSplitsCjkUtf8Codepoints) {
  std::string text = "汉字学习";
  EXPECT_EQ(Utf8LineBreaker::longestPrefixWithinWidth(text, 2, codepointWidth), 6U);
  EXPECT_EQ(text.substr(0, 6), "汉字");
  EXPECT_EQ(text, "汉字学习");
}

TEST(Utf8LineBreaker, LetsCallerHandleAnOverwideFirstCodepoint) {
  std::string text = "汉字";
  EXPECT_EQ(Utf8LineBreaker::longestPrefixWithinWidth(text, 0, codepointWidth), 0U);
  EXPECT_EQ(Utf8LineBreaker::nextBoundary(text, 0), 3U);
}
