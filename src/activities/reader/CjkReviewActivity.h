#pragma once

#include <cstdint>
#include <string>
#include <vector>

#include "activities/UiListActivity.h"
#include "chinesepoint/cjk/CjkStudyClock.h"

// Bounded local-review surface. It intentionally selects only cards whose
// definition was saved from a successful local dictionary lookup; a bare word
// or an Anki-authoritative entry cannot be rated here.
class CjkReviewActivity final : public UiListActivity {
 public:
  CjkReviewActivity(GfxRenderer& renderer, MappedInputManager& mappedInput);
  void onEnter() override;
  void onExit() override;

 private:
  int listCount() const override { return static_cast<int>(rowItems.size()); }
  void buildScreen(UiScreen& screen) override;
  void activateIndex(int index) override;
  const char* headerTitle() const override;
  void drawFooter() override;

  bool loadNextDueCard();
  void rebuildRows();
  void rate(ChinesePoint::Cjk::Rating rating);

  bool dataAvailable = false;
  bool hasDueCard = false;
  uint64_t wordId = 0;
  std::string headword;
  std::string sourceSentence;
  std::string answer;
  std::string status;
  ChinesePoint::Cjk::StudyClock clock{};
  std::vector<std::string> labels;
  std::vector<freeink::ui::ListItem> rowItems;
};
