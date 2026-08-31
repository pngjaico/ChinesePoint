#pragma once

#include <string>
#include <vector>

#include "activities/UiListActivity.h"
#include "chinesepoint/cjk/CjkLearnerModel.h"

// Read-only browser for the local learner journal. It aliases the store's
// in-memory entries while the activity is open; no book, dictionary, or
// firmware state is changed from this screen.
class CjkVocabularyActivity final : public UiListActivity {
 public:
  explicit CjkVocabularyActivity(GfxRenderer& renderer, MappedInputManager& mappedInput);
  void onEnter() override;
  void onExit() override;

 private:
  int listCount() const override { return static_cast<int>(entries.size()); }
  void buildScreen(UiScreen& screen) override;
  void activateIndex(int index) override;
  const char* headerTitle() const override;
  void drawFooter() override;

  void rebuildRows();
  static const char* statusLabel(ChinesePoint::Cjk::WordStatus status);

  bool dataAvailable = false;
  std::vector<const ChinesePoint::Cjk::LearnerEntry*> entries;
  std::vector<std::string> subtitles;
  std::vector<freeink::ui::ListItem> rowItems;
};
