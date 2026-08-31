#pragma once

#include <string>

#include "activities/Activity.h"
#include "chinesepoint/cjk/CcCedictInstaller.h"

// One-purpose, opt-in installer for the reviewed CC-CEDICT release. The
// reader never downloads it on its own and the normal CrossPoint dictionary
// route remains available if this optional feature fails.
class CcCedictDownloadActivity final : public Activity {
 public:
  CcCedictDownloadActivity(GfxRenderer& renderer, MappedInputManager& mappedInput);
  void onEnter() override;
  void loop() override;
  void render(RenderLock&&) override;
  bool preventAutoSleep() override;
  bool skipLoopDelay() override { return true; }

 private:
  enum class State { Confirming, Wifi, Downloading, Complete, Error, Cancelled };

  State state = State::Confirming;
  size_t completed = 0;
  size_t total = 0;
  bool cancelRequested = false;
  bool goHomeRequested = false;
  std::string status;

  void onConfirmationComplete(const ActivityResult& result);
  void onWifiSelectionComplete(const ActivityResult& result);
  void install();
  void setResult(ChinesePoint::Cjk::CcCedictInstaller::Result result);
  void selectDictionary();
};
