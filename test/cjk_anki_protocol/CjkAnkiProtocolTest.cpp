#include <gtest/gtest.h>

#include "chinesepoint/cjk/CjkAnkiProtocol.h"

namespace {

TEST(CjkAnkiProtocol, AcceptsOnlyPrivateLanOrMdnsHttpEndpoints) {
  ChinesePoint::Cjk::AnkiBridgeUrl parsed;
  ASSERT_TRUE(ChinesePoint::Cjk::parseAnkiBridgeUrl("http://192.168.1.44:5051/v1/cjk/vocabulary", parsed));
  EXPECT_EQ(parsed.host, "192.168.1.44");
  EXPECT_EQ(parsed.port, 5051);
  EXPECT_EQ(parsed.path, "/v1/cjk/vocabulary");
  ASSERT_TRUE(ChinesePoint::Cjk::parseAnkiBridgeUrl("http://chinesepoint-anki.local", parsed));
  EXPECT_EQ(parsed.port, 5051);
  EXPECT_EQ(parsed.path, "/v1/cjk/vocabulary");
  EXPECT_FALSE(ChinesePoint::Cjk::parseAnkiBridgeUrl("https://192.168.1.44", parsed));
  EXPECT_FALSE(ChinesePoint::Cjk::parseAnkiBridgeUrl("http://8.8.8.8", parsed));
  EXPECT_FALSE(ChinesePoint::Cjk::parseAnkiBridgeUrl("http://token@192.168.1.44", parsed));
}

TEST(CjkAnkiProtocol, ValidatesCredentialMaterialAndMakesStableRetryBatchIds) {
  constexpr auto clientId = "0123456789abcdef0123456789abcdef";
  EXPECT_TRUE(ChinesePoint::Cjk::validAnkiBridgeClientId(clientId));
  EXPECT_TRUE(ChinesePoint::Cjk::validAnkiBridgeToken("cjk_token-ABC_123"));
  EXPECT_FALSE(ChinesePoint::Cjk::validAnkiBridgeToken("bad token"));
  EXPECT_FALSE(ChinesePoint::Cjk::validAnkiBridgeClientId("short"));
  EXPECT_EQ(ChinesePoint::Cjk::ankiBridgeBatchId(clientId, 42, 3),
            "cp-v1-0123456789abcdef0123456789abcdef-42-3");
  EXPECT_EQ(ChinesePoint::Cjk::ankiBridgeBatchId(clientId, 42, 3),
            ChinesePoint::Cjk::ankiBridgeBatchId(clientId, 42, 3));
  EXPECT_TRUE(ChinesePoint::Cjk::ankiBridgeBatchId("invalid", 42, 3).empty());
}

}  // namespace
