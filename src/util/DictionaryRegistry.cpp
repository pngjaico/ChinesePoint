#include "DictionaryRegistry.h"

#include <HalStorage.h>
#include <Logging.h>

#include <algorithm>
#include <cctype>
#include <cstring>

#include "StringUtils.h"

namespace DictionaryRegistry {
namespace {

// Dictionaries are looked up in both roots, in order. The hidden variant
// lets users keep the folder out of the file browser (hidden by default,
// see FileBrowserActivity's showHiddenFiles check).
constexpr const char* DICT_ROOTS[] = {"/dictionaries", "/.dictionaries"};

std::string languageFolder(const std::string& language) {
  // EPUB language is metadata supplied by the book. Constrain it to an ASCII
  // ISO-639 primary tag before it influences dictionary routing.
  if (language.size() < 2 || !std::isalpha(static_cast<unsigned char>(language[0])) ||
      !std::isalpha(static_cast<unsigned char>(language[1]))) {
    return {};
  }
  std::string folder;
  folder.push_back(static_cast<char>(std::tolower(static_cast<unsigned char>(language[0]))));
  folder.push_back(static_cast<char>(std::tolower(static_cast<unsigned char>(language[1]))));
  return folder;
}

// Find the single .idx stem inside one dictionary folder. Returns false when
// the folder holds no .idx or more than one distinct stem (ambiguous).
bool findStem(const char* folderPath, std::string& stemOut) {
  auto dir = Storage.open(folderPath);
  if (!dir || !dir.isDirectory()) return false;

  dir.rewindDirectory();
  char name[128];
  char foundStem[128];
  foundStem[0] = '\0';
  for (auto entry = dir.openNextFile(); entry; entry = dir.openNextFile()) {
    entry.getName(name, sizeof(name));
    // Skip macOS metadata files (AppleDouble resource forks)
    if (entry.isDirectory() || strncmp(name, "._", 2) == 0) continue;

    const size_t len = strlen(name);
    if (len <= 4 || strcmp(name + len - 4, ".idx") != 0) continue;

    name[len - 4] = '\0';
    if (foundStem[0] != '\0' && strcmp(foundStem, name) != 0) {
      LOG_DBG("DREG", "Skipping %s: multiple index stems found", folderPath);
      return false;
    }
    strncpy(foundStem, name, sizeof(foundStem) - 1);
    foundStem[sizeof(foundStem) - 1] = '\0';
  }

  if (foundStem[0] == '\0') return false;

  // Require dictionary data next to the index, so folders holding only an
  // .idx never surface as selectable dictionaries that fail at lookup time.
  const std::string base = std::string(folderPath) + "/" + foundStem;
  if (!Storage.exists((base + ".dict").c_str()) && !Storage.exists((base + ".dict.dz").c_str())) {
    LOG_DBG("DREG", "Skipping %s: no .dict or .dict.dz", folderPath);
    return false;
  }

  stemOut = foundStem;
  return true;
}

}  // namespace

void discover(std::vector<DictionaryEntry>& out) {
  out.clear();
  out.reserve(8);

  for (const char* dictRoot : DICT_ROOTS) {
    auto rootDir = Storage.open(dictRoot);
    if (!rootDir || !rootDir.isDirectory()) {
      LOG_DBG("DREG", "No %s directory on SD card", dictRoot);
      continue;
    }

    rootDir.rewindDirectory();
    char name[128];
    for (auto entry = rootDir.openNextFile(); entry; entry = rootDir.openNextFile()) {
      entry.getName(name, sizeof(name));
      if (!entry.isDirectory() || name[0] == '.') continue;

      std::string folderPath = std::string(dictRoot) + "/" + name;
      std::string stem;
      if (findStem(folderPath.c_str(), stem)) {
        out.push_back({name, std::move(stem)});
        LOG_DBG("DREG", "Found dictionary: %s", name);
        continue;
      }

      // A language directory is not a dictionary on its own. Inspect exactly
      // one child level so metadata routing can use zh/<dictionary> without
      // recursively walking user-controlled SD-card paths.
      auto languageDir = Storage.open(folderPath.c_str());
      if (!languageDir || !languageDir.isDirectory()) continue;
      languageDir.rewindDirectory();
      char child[128];
      for (auto nested = languageDir.openNextFile(); nested; nested = languageDir.openNextFile()) {
        nested.getName(child, sizeof(child));
        if (!nested.isDirectory() || child[0] == '.') continue;
        if (!findStem((folderPath + "/" + child).c_str(), stem)) continue;
        out.push_back({std::string(name) + "/" + child, stem});
        LOG_DBG("DREG", "Found dictionary: %s/%s", name, child);
      }
    }
  }

  // Case-insensitive sort by folder name (matches FileBrowserActivity ordering).
  std::sort(out.begin(), out.end(), [](const DictionaryEntry& a, const DictionaryEntry& b) {
    return StringUtils::asciiCaseCmp(a.name.c_str(), b.name.c_str()) < 0;
  });
}

bool resolveBasePath(const char* folderName, std::string& basePathOut) {
  if (!folderName || folderName[0] == '\0') return false;
  // folderName is persisted in the settings JSON. Permit one nested language
  // level but reject absolute paths, dot components, backslashes, and a second
  // separator before it can be joined to an SD-card root.
  if (folderName[0] == '.' || strpbrk(folderName, "\\") != nullptr || strstr(folderName, "..") != nullptr) return false;
  const char* slash = strchr(folderName, '/');
  if (slash && (slash == folderName || slash[1] == '\0' || strchr(slash + 1, '/'))) return false;

  for (const char* dictRoot : DICT_ROOTS) {
    std::string folderPath = std::string(dictRoot) + "/" + folderName;
    std::string stem;
    if (!findStem(folderPath.c_str(), stem)) continue;
    basePathOut = folderPath + "/" + stem;
    return true;
  }
  return false;
}

bool folderForLanguage(const std::string& language, std::string& folderNameOut) {
  const std::string languagePrefix = languageFolder(language);
  if (languagePrefix.empty()) return false;

  std::vector<DictionaryEntry> entries;
  discover(entries);
  const std::string prefix = languagePrefix + "/";
  const auto match = std::find_if(entries.begin(), entries.end(), [&prefix](const DictionaryEntry& entry) {
    return entry.name.compare(0, prefix.size(), prefix) == 0;
  });
  if (match == entries.end()) return false;
  folderNameOut = match->name;
  return true;
}

bool folderForLanguageOrFallback(const std::string& language, const char* fallbackFolder,
                                 std::string& folderNameOut) {
  if (folderForLanguage(language, folderNameOut)) return true;
  if (!fallbackFolder || fallbackFolder[0] == '\0') {
    folderNameOut.clear();
    return false;
  }
  folderNameOut = fallbackFolder;
  return true;
}

}  // namespace DictionaryRegistry
