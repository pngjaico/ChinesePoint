#include "chinesepoint/cjk/CjkLearnerStats.h"

#include <algorithm>
#include <limits>
#include <string_view>
#include <vector>

namespace ChinesePoint::Cjk {

LearnerStats computeLearnerStats(const LearnerEntry* const entries, const size_t entryCount) {
  LearnerStats stats;
  if (entries == nullptr || entryCount == 0) return stats;

  std::vector<std::string_view> books;
  books.reserve(entryCount);
  int64_t firstSeen = std::numeric_limits<int64_t>::max();
  for (size_t index = 0; index < entryCount; ++index) {
    const LearnerEntry& entry = entries[index];
    ++stats.vocabularyCount;
    stats.encounterTotal += entry.encounterCount;
    switch (entry.status) {
      case WordStatus::Encountered:
        ++stats.encounteredCount;
        break;
      case WordStatus::Saved:
        ++stats.savedCount;
        break;
      case WordStatus::Learning:
        ++stats.learningCount;
        break;
      case WordStatus::Known:
        ++stats.knownCount;
        break;
    }
    if (!entry.bookPath.empty() && std::find(books.begin(), books.end(), entry.bookPath) == books.end()) {
      books.push_back(entry.bookPath);
    }
    if (entry.firstSeenStudyMs > 0) firstSeen = std::min(firstSeen, entry.firstSeenStudyMs);
    stats.lastSeenStudyMs = std::max(stats.lastSeenStudyMs, entry.lastSeenStudyMs);
  }
  stats.sourceBookCount = static_cast<uint32_t>(books.size());
  stats.firstSeenStudyMs = firstSeen == std::numeric_limits<int64_t>::max() ? 0 : firstSeen;
  return stats;
}

}  // namespace ChinesePoint::Cjk
