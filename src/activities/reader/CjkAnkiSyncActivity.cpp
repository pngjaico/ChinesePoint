#include "CjkAnkiSyncActivity.h"

#include <GfxRenderer.h>
#include <I18n.h>
#include <WiFi.h>

#include <memory>

#include "MappedInputManager.h"
#include "activities/network/WifiSelectionActivity.h"
#include "chinesepoint/CjkSafetyGuard.h"
#include "chinesepoint/cjk/CjkAnkiClient.h"
#include "chinesepoint/cjk/CjkAnkiConfig.h"
#include "chinesepoint/cjk/CjkLearnerStore.h"
#include "components/UITheme.h"
#include "fontIds.h"

CjkAnkiSyncActivity::CjkAnkiSyncActivity(GfxRenderer& renderer, MappedInputManager& mappedInput)
    : Activity("CjkAnkiSync", renderer, mappedInput) {}

bool CjkAnkiSyncActivity::preventAutoSleep() {
  return state == State::Syncing || state == State::Complete || state == State::Error || state == State::Cancelled;
}

void CjkAnkiSyncActivity::onEnter() {
  Activity::onEnter();
  auto& config = ChinesePoint::Cjk::ankiBridgeConfigStore();
  if (!config.load() || !config.config().configured()) {
    state = State::Error;
    message = tr(STR_ANKI_CONFIGURE);
    requestUpdate(true);
    return;
  }
  if (WiFi.status() == WL_CONNECTED) {
    synchronize();
    return;
  }
  WiFi.mode(WIFI_STA);
  startActivityForResult(std::make_unique<WifiSelectionActivity>(renderer, mappedInput),
                         [this](const ActivityResult& result) { onWifiSelectionComplete(result); });
}

void CjkAnkiSyncActivity::onWifiSelectionComplete(const ActivityResult& result) {
  if (result.isCancelled) {
    finish();
    return;
  }
  synchronize();
}

void CjkAnkiSyncActivity::synchronize() {
  state = State::Syncing;
  completed = total = 0;
  cancelRequested = false;
  goHomeRequested = false;
  requestUpdateAndWait();

  ChinesePoint::Cjk::CjkAnkiClient::Result result = ChinesePoint::Cjk::CjkAnkiClient::Result::NotConfigured;
  bool learnerAvailable = false;
  if (ChinesePoint::CjkSafetyGuard::startLearnerSession()) {
    learnerAvailable = true;
    ChinesePoint::Cjk::CjkAnkiClient client;
    result = client.pushVocabulary(
        ChinesePoint::Cjk::learnerStore(), ChinesePoint::Cjk::ankiBridgeConfigStore().config(),
        [this](const size_t progress, const size_t progressTotal) {
          completed = progress;
          total = progressTotal;
          mappedInput.update();
          if (mappedInput.isPressed(MappedInputManager::Button::Back) ||
              mappedInput.wasPressed(MappedInputManager::Button::Back)) {
            cancelRequested = true;
          }
          if (mappedInput.wasHomeGesture()) {
            cancelRequested = true;
            goHomeRequested = true;
          }
          requestUpdate(true);
        },
        &cancelRequested);
    ChinesePoint::CjkSafetyGuard::finishLearnerSession();
  }

  if (goHomeRequested && result == ChinesePoint::Cjk::CjkAnkiClient::Result::Cancelled) {
    onGoHome();
    return;
  }
  using Result = ChinesePoint::Cjk::CjkAnkiClient::Result;
  if (result == Result::Ok) {
    state = State::Complete;
  } else if (result == Result::Cancelled) {
    state = State::Cancelled;
  } else {
    state = State::Error;
    message = !learnerAvailable ? tr(STR_LEARNER_DATA_UNAVAILABLE)
                                : (result == Result::NotConfigured ? tr(STR_ANKI_CONFIGURE) : tr(STR_ANKI_SYNC_FAILED));
  }
  requestUpdate(true);
}

void CjkAnkiSyncActivity::loop() {
  if (state == State::Syncing || state == State::Wifi) return;
  if (mappedInput.wasPressed(MappedInputManager::Button::Back) ||
      mappedInput.wasPressed(MappedInputManager::Button::Confirm)) {
    finish();
  }
}

void CjkAnkiSyncActivity::render(RenderLock&&) {
  const auto& metrics = UITheme::getInstance().getMetrics();
  const int width = renderer.getScreenWidth();
  const int height = renderer.getScreenHeight();
  const int lineHeight = renderer.getLineHeight(UI_10_FONT_ID);
  const int centerY = (height - lineHeight) / 2;
  renderer.clearScreen();
  GUI.drawHeader(renderer, Rect{0, metrics.topPadding, width, metrics.headerHeight}, tr(STR_LEARNER_ANKI_SYNC));
  if (state == State::Syncing) {
    renderer.drawCenteredText(UI_10_FONT_ID, centerY - lineHeight, tr(STR_ANKI_SYNCING));
    const int percent = total == 0 ? 0 : static_cast<int>((completed * 100U) / total);
    GUI.drawProgressBar(renderer,
                        Rect{metrics.contentSidePadding, centerY + metrics.verticalSpacing,
                             width - metrics.contentSidePadding * 2, metrics.progressBarHeight},
                        percent, 100);
    const auto labels = mappedInput.mapLabels(tr(STR_CANCEL), "", "", "");
    GUI.drawButtonHints(renderer, labels.btn1, labels.btn2, labels.btn3, labels.btn4);
  } else if (state == State::Complete) {
    renderer.drawCenteredText(UI_10_FONT_ID, centerY, tr(STR_ANKI_SYNCED), true, EpdFontFamily::BOLD);
    const auto labels = mappedInput.mapLabels(tr(STR_BACK), "", "", "");
    GUI.drawButtonHints(renderer, labels.btn1, labels.btn2, labels.btn3, labels.btn4);
  } else if (state == State::Cancelled) {
    renderer.drawCenteredText(UI_10_FONT_ID, centerY, tr(STR_ANKI_SYNC_CANCELLED));
    const auto labels = mappedInput.mapLabels(tr(STR_BACK), "", "", "");
    GUI.drawButtonHints(renderer, labels.btn1, labels.btn2, labels.btn3, labels.btn4);
  } else if (state == State::Error) {
    renderer.drawCenteredText(UI_10_FONT_ID, centerY - lineHeight, tr(STR_ANKI_SYNC_FAILED), true, EpdFontFamily::BOLD);
    renderer.drawCenteredText(UI_10_FONT_ID, centerY + metrics.verticalSpacing, message.c_str());
    const auto labels = mappedInput.mapLabels(tr(STR_BACK), "", "", "");
    GUI.drawButtonHints(renderer, labels.btn1, labels.btn2, labels.btn3, labels.btn4);
  }
  renderer.displayBuffer();
}
