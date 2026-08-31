#pragma once

#include <string>
#include <vector>

#include "activities/UiListActivity.h"
#include "chinesepoint/cjk/CjkLearnerStats.h"

// Read-only learner summary reachable from the EPUB reader. It is deliberately
// separate from the reader and from firmware/settings storage: a bad learner
// journal is reported as unavailable and leaves ordinary reading untouched.
class CjkLearnerStatsActivity final : public UiListActivity {
 public:
  explicit CjkLearnerStatsActivity(GfxRenderer& renderer, MappedInputManager& mappedInput);
  void onEnter() override;

 private:
  int listCount() const override { return static_cast<int>(rowItems.size()); }
  void buildScreen(UiScreen& screen) override;
  void activateIndex(int index) override;
  const char* headerTitle() const override;
  void drawFooter() override;

  void rebuildRows();
  void exportEntries();

  bool dataAvailable = false;
  ChinesePoint::Cjk::LearnerStats stats{};
  std::string exportStatus;
  std::vector<std::string> labels;
  std::vector<std::string> values;
  std::vector<freeink::ui::ListItem> rowItems;
};
