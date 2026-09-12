#pragma once

#include <string>
#include <vector>

// One StarDict dictionary found under /dictionaries or /.dictionaries: a
// subfolder holding <stem>.idx plus <stem>.dict or <stem>.dict.dz.  A
// language-organized dictionary uses one additional directory level, such as
// /dictionaries/zh/cc-cedict/.
struct DictionaryEntry {
  std::string name;  // folder below the dictionary root (shown/stored in settings)
  std::string stem;  // index basename without .idx
};

namespace DictionaryRegistry {

// Scan /dictionaries/*/ and /.dictionaries/*/ for dictionaries. One nested
// language directory is accepted. Folders with multiple index stems are
// ambiguous and skipped. Result is sorted case-insensitively by name.
void discover(std::vector<DictionaryEntry>& out);

// Resolve a folder name to its extensionless base path
// ("/dictionaries/<folder>/<stem>" or "/.dictionaries/<folder>/<stem>").
// Returns false if the folder holds no usable dictionary in either root.
bool resolveBasePath(const char* folderName, std::string& basePathOut);

// Select a dictionary under the first two letters of an EPUB language tag.
// For example, "zh-Hans" resolves a dictionary below "zh/". The selection is
// intentionally best-effort: malformed or absent metadata has no effect.
bool folderForLanguage(const std::string& language, std::string& folderNameOut);

// Prefer the language-matched dictionary and otherwise leave the established
// global selection in control. This makes a book's metadata a routing hint,
// never a requirement for opening the reader or looking up a saved word.
bool folderForLanguageOrFallback(const std::string& language, const char* fallbackFolder,
                                 std::string& folderNameOut);

}  // namespace DictionaryRegistry
