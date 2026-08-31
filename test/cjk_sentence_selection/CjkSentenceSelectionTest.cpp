#include <gtest/gtest.h>

#include <array>

#include "chinesepoint/cjk/CjkSentenceSelection.h"

namespace {
using ChinesePoint::Cjk::SelectableToken;
using ChinesePoint::Cjk::SentenceCompleteness;
using ChinesePoint::Cjk::SentenceSelection;

TEST(CjkSentenceSelection, KeepsChineseTokensTogetherAndAnchorsTouchedWord) {
  const std::array<SelectableToken, 5> tokens = {{{"他", 10, 1, false}, {"喜欢", 11, 2, true},
                                                     {"读书", 13, 2, true}, {"。", 15, 1, true},
                                                     {"下一句", 16, 3, false}}};
  std::array<char, 64> sentence{};
  SentenceSelection selection;

  ASSERT_TRUE(ChinesePoint::Cjk::buildSentenceSelection(tokens.data(), tokens.size(), 1, 4, true, false,
                                                         sentence.data(), sentence.size(), selection));
  EXPECT_STREQ(sentence.data(), "他喜欢读书。");
  EXPECT_EQ(selection.anchor.spineIndex, 4u);
  EXPECT_EQ(selection.anchor.visibleCodepointOffset, 11u);
  EXPECT_EQ(selection.anchor.codepointLength, 2u);
  EXPECT_EQ(selection.selectedSentenceCodepoint, 1u);
  EXPECT_EQ(selection.completeness, SentenceCompleteness::Complete);
}

TEST(CjkSentenceSelection, ReportsAnIncompletePageWindowInsteadOfPretendingItIsComplete) {
  const std::array<SelectableToken, 2> tokens = {{{"continua", 0, 7, false}, {"aqui", 8, 4, false}}};
  std::array<char, 64> sentence{};
  SentenceSelection selection;

  ASSERT_TRUE(ChinesePoint::Cjk::buildSentenceSelection(tokens.data(), tokens.size(), 0, 0, false, false,
                                                         sentence.data(), sentence.size(), selection));
  EXPECT_STREQ(sentence.data(), "continua aqui");
  EXPECT_EQ(selection.completeness, SentenceCompleteness::TruncatedBoth);
}

TEST(CjkSentenceSelection, RefusesToTruncateSavedSentenceBuffer) {
  const std::array<SelectableToken, 2> tokens = {{{"frase", 0, 5, false}, {" longa.", 6, 7, false}}};
  std::array<char, 4> sentence{};
  SentenceSelection selection;

  EXPECT_FALSE(ChinesePoint::Cjk::buildSentenceSelection(tokens.data(), tokens.size(), 0, 0, true, true,
                                                          sentence.data(), sentence.size(), selection));
  EXPECT_STREQ(sentence.data(), "");
}

}  // namespace
