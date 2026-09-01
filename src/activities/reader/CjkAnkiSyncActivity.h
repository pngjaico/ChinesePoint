#pragma once

#include <string>

#include "activities/Activity.h"

class CjkAnkiSyncActivity final : public Activity {
 public:
  CjkAnkiSyncActivity(GfxRenderer& renderer, MappedInputManager& mappedInput);
  void onEnter() override;
  void loop() override;
  void render(RenderLock&&) override;
  bool preventAutoSleep() override;
  bool skipLoopDelay() override { return true; }

 private:
  enum class State { Wifi, Syncing, Complete, Error, Cancelled };
  State state = State::Wifi;
  size_t completed = 0;
  size_t total = 0;
  bool cancelRequested = false;
  bool goHomeRequested = false;
  std::string message;

  void onWifiSelectionComplete(const ActivityResult& result);
  void synchronize();
};
