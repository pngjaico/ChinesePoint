#include "CcCedictDownloadActivity.h"

#include <CrossPointSettings.h>
#include <GfxRenderer.h>
#include <I18n.h>
#include <WiFi.h>

#include <cstring>

#include "MappedInputManager.h"
#include "activities/network/WifiSelectionActivity.h"
#include "activities/util/ConfirmationActivity.h"
#include "chinesepoint/CjkSafetyGuard.h"
#include "components/UITheme.h"
#include "fontIds.h"

CcCedictDownloadActivity::CcCedictDownloadActivity(GfxRenderer& renderer, MappedInputManager& mappedInput)
    : Activity("CcCedictDownload", renderer, mappedInput) {}

void CcCedictDownloadActivity::onEnter() {
  Activity::onEnter();
  // The size warning includes generous staging headroom. Storage capacity is
  // not exposed by the HAL, so this is an honest warning rather than a false
  // preflight guarantee.
  startActivityForResult(std::make_unique<ConfirmationActivity>(renderer, mappedInput, tr(STR_CJK_DICTIONARY),
                                                                 tr(STR_CJK_DICTIONARY_CONFIRM)),
                         [this](const ActivityResult& result) { onConfirmationComplete(result); });
}

bool CcCedictDownloadActivity::preventAutoSleep() {
  return state == State::Downloading || state == State::Complete || state == State::Error || state == State::Cancelled;
}

void CcCedictDownloadActivity::onConfirmationComplete(const ActivityResult& result) {
  if (result.isCancelled) {
    finish();
    return;
  }
  if (ChinesePoint::Cjk::CcCedictInstaller::isInstalled()) {
    selectDictionary();
    state = State::Complete;
    requestUpdate(true);
    return;
  }
  state = State::Wifi;
  WiFi.mode(WIFI_STA);
  startActivityForResult(std::make_unique<WifiSelectionActivity>(renderer, mappedInput),
                         [this](const ActivityResult& wifiResult) { onWifiSelectionComplete(wifiResult); });
}

void CcCedictDownloadActivity::onWifiSelectionComplete(const ActivityResult& result) {
  if (result.isCancelled) {
    finish();
    return;
  }
  install();
}

void CcCedictDownloadActivity::selectDictionary() {
  strncpy(SETTINGS.dictionaryName, ChinesePoint::Cjk::CcCedictInstaller::dictionaryFolder(),
          sizeof(SETTINGS.dictionaryName) - 1);
  SETTINGS.dictionaryName[sizeof(SETTINGS.dictionaryName) - 1] = '\0';
  SETTINGS.saveToFile();
}

void CcCedictDownloadActivity::install() {
  state = State::Downloading;
  completed = total = 0;
  cancelRequested = false;
  goHomeRequested = false;
  requestUpdateAndWait();

  ChinesePoint::Cjk::CcCedictInstaller::Result result = ChinesePoint::Cjk::CcCedictInstaller::Result::StorageFailed;
  if (ChinesePoint::CjkSafetyGuard::startLearnerSession()) {
    ChinesePoint::Cjk::CcCedictInstaller installer;
    result = installer.install(
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

  if (goHomeRequested && result == ChinesePoint::Cjk::CcCedictInstaller::Result::Cancelled) {
    onGoHome();
    return;
  }
  setResult(result);
  requestUpdate(true);
}

void CcCedictDownloadActivity::setResult(const ChinesePoint::Cjk::CcCedictInstaller::Result result) {
  using Result = ChinesePoint::Cjk::CcCedictInstaller::Result;
  if (result == Result::Installed || result == Result::AlreadyInstalled) {
    selectDictionary();
    state = State::Complete;
    return;
  }
  if (result == Result::Cancelled) {
    state = State::Cancelled;
    return;
  }
  state = State::Error;
  switch (result) {
    case Result::DownloadFailed: status = tr(STR_DOWNLOAD_FAILED); break;
    case Result::ChecksumFailed: status = tr(STR_CJK_DICTIONARY_CHECKSUM_FAILED); break;
    case Result::ArchiveInvalid: status = tr(STR_CJK_DICTIONARY_ARCHIVE_INVALID); break;
    case Result::StorageFailed: status = tr(STR_CJK_DICTIONARY_STORAGE_FAILED); break;
    default: status = tr(STR_ERROR_GENERAL_FAILURE); break;
  }
}

void CcCedictDownloadActivity::loop() {
  if (state == State::Downloading || state == State::Confirming || state == State::Wifi) return;
  if (mappedInput.wasPressed(MappedInputManager::Button::Back)) {
    finish();
    return;
  }
  if (state == State::Error && mappedInput.wasPressed(MappedInputManager::Button::Confirm)) {
    onEnter();
  }
}

void CcCedictDownloadActivity::render(RenderLock&&) {
  const auto& metrics = UITheme::getInstance().getMetrics();
  const int width = renderer.getScreenWidth();
  const int height = renderer.getScreenHeight();
  const int lineHeight = renderer.getLineHeight(UI_10_FONT_ID);
  const int centerY = (height - lineHeight) / 2;
  renderer.clearScreen();
  GUI.drawHeader(renderer, Rect{0, metrics.topPadding, width, metrics.headerHeight}, tr(STR_CJK_DICTIONARY));

  if (state == State::Downloading) {
    renderer.drawCenteredText(UI_10_FONT_ID, centerY - lineHeight, tr(STR_DOWNLOADING));
    const int percent = total == 0 ? 0 : static_cast<int>((completed * 100U) / total);
    GUI.drawProgressBar(renderer,
                        Rect{metrics.contentSidePadding, centerY + metrics.verticalSpacing,
                             width - metrics.contentSidePadding * 2, metrics.progressBarHeight},
                        percent, 100);
    const auto labels = mappedInput.mapLabels(tr(STR_CANCEL), "", "", "");
    GUI.drawButtonHints(renderer, labels.btn1, labels.btn2, labels.btn3, labels.btn4);
  } else if (state == State::Complete) {
    renderer.drawCenteredText(UI_10_FONT_ID, centerY, tr(STR_CJK_DICTIONARY_INSTALLED), true, EpdFontFamily::BOLD);
    const auto labels = mappedInput.mapLabels(tr(STR_BACK), "", "", "");
    GUI.drawButtonHints(renderer, labels.btn1, labels.btn2, labels.btn3, labels.btn4);
  } else if (state == State::Cancelled) {
    renderer.drawCenteredText(UI_10_FONT_ID, centerY, tr(STR_CJK_DICTIONARY_CANCELLED));
    const auto labels = mappedInput.mapLabels(tr(STR_BACK), "", "", "");
    GUI.drawButtonHints(renderer, labels.btn1, labels.btn2, labels.btn3, labels.btn4);
  } else if (state == State::Error) {
    renderer.drawCenteredText(UI_10_FONT_ID, centerY - lineHeight, tr(STR_CJK_DICTIONARY_INSTALL_FAILED), true,
                              EpdFontFamily::BOLD);
    renderer.drawCenteredText(UI_10_FONT_ID, centerY + metrics.verticalSpacing, status.c_str());
    const auto labels = mappedInput.mapLabels(tr(STR_BACK), tr(STR_RETRY), "", "");
    GUI.drawButtonHints(renderer, labels.btn1, labels.btn2, labels.btn3, labels.btn4);
  }
  renderer.displayBuffer();
}
