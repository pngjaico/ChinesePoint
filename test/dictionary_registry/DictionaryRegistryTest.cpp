#include <gtest/gtest.h>

#include <filesystem>
#include <fstream>
#include <string>
#include <vector>

#include "HalStorage.h"
#include "util/DictionaryRegistry.h"

namespace {

class DictionaryRegistryTest : public ::testing::Test {
 protected:
  void SetUp() override {
    // Keep fixtures outside the CMake build directory and give each discovered
    // CTest process a separate path: the full suite runs cases in parallel.
    const auto* testInfo = ::testing::UnitTest::GetInstance()->current_test_info();
    root = std::filesystem::temp_directory_path() /
           (std::string("chinesepoint-dictionary-registry-") + testInfo->name());
    std::error_code error;
    std::filesystem::remove_all(root, error);
    std::filesystem::create_directories(root);
    Storage.setTestRoot(root);
  }

  void TearDown() override {
    std::error_code error;
    std::filesystem::remove_all(root, error);
  }

  void addDictionary(const std::string& folder, const std::string& stem) {
    const auto path = root / "dictionaries" / folder;
    std::filesystem::create_directories(path);
    std::ofstream(path / (stem + ".idx")) << "index";
    std::ofstream(path / (stem + ".dict")) << "definition";
  }

  std::filesystem::path root;
};

TEST_F(DictionaryRegistryTest, DiscoversFlatAndOneNestedLanguageDictionary) {
  addDictionary("cc-cedict", "cedict");
  addDictionary("zh/hanzi", "hanzi");
  addDictionary("ja/japanese", "japanese");

  std::vector<DictionaryEntry> entries;
  DictionaryRegistry::discover(entries);

  ASSERT_EQ(entries.size(), 3u);
  EXPECT_EQ(entries[0].name, "cc-cedict");
  EXPECT_EQ(entries[1].name, "ja/japanese");
  EXPECT_EQ(entries[2].name, "zh/hanzi");
}

TEST_F(DictionaryRegistryTest, PrefersLanguageFolderAndFallsBackForMissingOrInvalidMetadata) {
  addDictionary("cc-cedict", "cedict");
  addDictionary("zh/hanzi", "hanzi");
  std::string folder;

  EXPECT_TRUE(DictionaryRegistry::folderForLanguageOrFallback("ZH-Hans", "cc-cedict", folder));
  EXPECT_EQ(folder, "zh/hanzi");
  EXPECT_TRUE(DictionaryRegistry::folderForLanguageOrFallback("ko", "cc-cedict", folder));
  EXPECT_EQ(folder, "cc-cedict");
  EXPECT_TRUE(DictionaryRegistry::folderForLanguageOrFallback("../zh", "cc-cedict", folder));
  EXPECT_EQ(folder, "cc-cedict");
}

TEST_F(DictionaryRegistryTest, ResolvesOnlyAValidatedSingleNestedFolder) {
  addDictionary("zh/hanzi", "hanzi");
  std::string basePath;

  EXPECT_TRUE(DictionaryRegistry::resolveBasePath("zh/hanzi", basePath));
  EXPECT_EQ(basePath, "/dictionaries/zh/hanzi/hanzi");
  EXPECT_FALSE(DictionaryRegistry::resolveBasePath("zh/hanzi/extra", basePath));
  EXPECT_FALSE(DictionaryRegistry::resolveBasePath("zh/../hanzi", basePath));
  EXPECT_FALSE(DictionaryRegistry::resolveBasePath("/zh/hanzi", basePath));
  EXPECT_FALSE(DictionaryRegistry::resolveBasePath(".hidden", basePath));
}

}  // namespace
