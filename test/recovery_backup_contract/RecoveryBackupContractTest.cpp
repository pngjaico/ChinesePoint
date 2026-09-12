#include <gtest/gtest.h>

#include <array>
#include <string>

#include "chinesepoint/RecoveryBackupContract.h"

namespace {

using ChinesePoint::RecoveryBackup::kSha256Bytes;
using ChinesePoint::RecoveryBackup::parseExactLowercaseSha256;

TEST(RecoveryBackupContract, AcceptsOnlyAnExactLowercaseDigest) {
  std::array<uint8_t, kSha256Bytes> digest{};
  ASSERT_TRUE(parseExactLowercaseSha256("0123456789abcdef0123456789abcdef0123456789abcdef0123456789abcdef", digest));
  EXPECT_EQ(digest.front(), 0x01);
  EXPECT_EQ(digest.back(), 0xef);
}

TEST(RecoveryBackupContract, RejectsAlternativeChecksumFileFormats) {
  constexpr auto valid = "0123456789abcdef0123456789abcdef0123456789abcdef0123456789abcdef";
  std::array<uint8_t, kSha256Bytes> digest{};
  EXPECT_FALSE(parseExactLowercaseSha256(std::string(valid) + "\n", digest));
  EXPECT_FALSE(parseExactLowercaseSha256(std::string(valid) + "  crosspoint-x4pro.bin", digest));
  EXPECT_FALSE(parseExactLowercaseSha256("0123456789ABCDEF0123456789abcdef0123456789abcdef0123456789abcdef", digest));
  EXPECT_FALSE(parseExactLowercaseSha256("0123456789abcdef0123456789abcdef0123456789abcdef0123456789abcdeg", digest));
  EXPECT_FALSE(parseExactLowercaseSha256("0123456789abcdef", digest));
}

}  // namespace
