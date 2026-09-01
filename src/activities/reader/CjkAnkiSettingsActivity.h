#pragma once

#include <string>

#include "activities/UiListActivity.h"

class CjkAnkiSettingsActivity final : public UiListActivity {
 public:
  CjkAnkiSettingsActivity(GfxRenderer& renderer, MappedInputManager& mappedInput);
  void onEnter() override;

 private:
  static constexpr int kItems = 3;
  int listCount() const override { return kItems; }
  void buildScreen(UiScreen& screen) override;
  void activateIndex(int index) override;
  const char* headerTitle() const override;

  std::string values_[kItems];
  freeink::ui::ListItem items_[kItems]{};
};
