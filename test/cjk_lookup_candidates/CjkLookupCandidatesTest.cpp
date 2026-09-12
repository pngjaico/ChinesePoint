#include <gtest/gtest.h>

#include <array>

#include "chinesepoint/cjk/CjkLookupCandidates.h"

namespace {
using ChinesePoint::Cjk::LookupCandidate;
using ChinesePoint::Cjk::SelectableToken;

TEST(CjkLookupCandidates, TriesLongestContiguousPhraseBeforeShorterPhrases) {
  const std::array<SelectableToken, 5> tokens = {{{"他", 0, 1, false}, {"喜欢", 1, 2, true},
                                                     {"读书", 3, 2, true}, {"。", 5, 1, true},
                                                     {"然后", 6, 2, false}}};
  std::array<LookupCandidate, 12> candidates{};

  const size_t count = ChinesePoint::Cjk::buildCjkLookupCandidates(tokens.data(), tokens.size(), 1,
                                                                     candidates.data(), candidates.size());

  ASSERT_EQ(count, 3u);
  EXPECT_STREQ(candidates[0].text, "他喜欢读书");
  EXPECT_STREQ(candidates[1].text, "喜欢读书");
  EXPECT_STREQ(candidates[2].text, "他喜欢");
}

TEST(CjkLookupCandidates, NeverJoinsAcrossPunctuationOrLatinText) {
  const std::array<SelectableToken, 5> tokens = {{{"我", 0, 1, false}, {"爱", 1, 1, true},
                                                     {"，", 2, 1, true}, {"you", 3, 3, false},
                                                     {"好", 6, 1, false}}};
  std::array<LookupCandidate, 12> candidates{};

  const size_t count = ChinesePoint::Cjk::buildCjkLookupCandidates(tokens.data(), tokens.size(), 0,
                                                                     candidates.data(), candidates.size());

  ASSERT_EQ(count, 1u);
  EXPECT_STREQ(candidates[0].text, "我爱");
}

TEST(CjkLookupCandidates, RejectsMalformedAndOversizeCjkTokens) {
  const std::array<SelectableToken, 1> malformed = {{{"\xE4\xB8", 0, 1, false}}};
  const std::array<SelectableToken, 2> oversized = {{{"这是一个非常非常非常非常非常非常非常非常长的中文词条", 0, 26, false},
                                                       {"继续", 26, 2, true}}};
  std::array<LookupCandidate, 12> candidates{};

  EXPECT_EQ(ChinesePoint::Cjk::buildCjkLookupCandidates(malformed.data(), malformed.size(), 0, candidates.data(),
                                                          candidates.size()),
            0u);
  EXPECT_EQ(ChinesePoint::Cjk::buildCjkLookupCandidates(oversized.data(), oversized.size(), 0, candidates.data(),
                                                          candidates.size()),
            0u);
}

TEST(CjkLookupCandidates, BoundsTheResultWithoutDiscardingTheLongestPhrase) {
  const std::array<SelectableToken, 3> tokens = {{{"他", 0, 1, false}, {"喜欢", 1, 2, true},
                                                     {"读书", 3, 2, true}}};
  std::array<LookupCandidate, 1> candidate{};

  const size_t count = ChinesePoint::Cjk::buildCjkLookupCandidates(tokens.data(), tokens.size(), 1,
                                                                     candidate.data(), candidate.size());

  ASSERT_EQ(count, 1u);
  EXPECT_STREQ(candidate[0].text, "他喜欢读书");
}

}  // namespace
