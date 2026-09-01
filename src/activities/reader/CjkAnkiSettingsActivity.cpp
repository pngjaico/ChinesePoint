#include "CjkAnkiSettingsActivity.h"

#include <GfxRenderer.h>
#include <I18n.h>

#include "CjkAnkiSyncActivity.h"
#include "MappedInputManager.h"
#include "activities/util/ConfirmationActivity.h"
#include "activities/util/KeyboardEntryActivity.h"
#include "chinesepoint/cjk/CjkAnkiConfig.h"
#include "components/UITheme.h"

namespace fui = freeink::ui;

CjkAnkiSettingsActivity::CjkAnkiSettingsActivity(GfxRenderer& renderer, MappedInputManager& mappedInput)
    : UiListActivity("CjkAnkiSettings", renderer, mappedInput) {
  const StrId labels[kItems] = {StrId::STR_ANKI_SERVER, StrId::STR_ANKI_API_TOKEN, StrId::STR_ANKI_SYNC_NOW};
  for (int index = 0; index < kItems; ++index) {
    items_[index].label = I18N.get(labels[index]);
    items_[index].actionValue = static_cast<int16_t>(index);
  }
}

void CjkAnkiSettingsActivity::onEnter() {
  UiListActivity::onEnter();
  ChinesePoint::Cjk::ankiBridgeConfigStore().load();
}

void CjkAnkiSettingsActivity::activateIndex(const int index) {
  app.clearTapFlash();
  auto& config = ChinesePoint::Cjk::ankiBridgeConfigStore();
  const auto& current = config.config();
  if (index == 0) {
    // The bridge deliberately does not advertise a hostname. A manually
    // entered private LAN address is more predictable on the X4 Pro than
    // assuming mDNS works on every router.
    const std::string prefill = current.serverUrl.empty() ? "http://192.168.1.2:5051/v1/cjk/vocabulary"
                                                           : current.serverUrl;
    startActivityForResult(std::make_unique<KeyboardEntryActivity>(renderer, mappedInput, tr(STR_ANKI_SERVER), prefill,
                                                                   192, InputType::Url),
                           [this](const ActivityResult& result) {
                             if (!result.isCancelled) {
                               const auto& keyboard = std::get<KeyboardResult>(result.data);
                               auto& config = ChinesePoint::Cjk::ankiBridgeConfigStore();
                               if (config.setServerUrl(keyboard.text)) config.save();
                             }
                             requestUpdate();
                           });
  } else if (index == 1) {
    startActivityForResult(std::make_unique<KeyboardEntryActivity>(renderer, mappedInput, tr(STR_ANKI_API_TOKEN),
                                                                   current.apiToken, 128, InputType::Password),
                           [this](const ActivityResult& result) {
                             if (!result.isCancelled) {
                               const auto& keyboard = std::get<KeyboardResult>(result.data);
                               auto& config = ChinesePoint::Cjk::ankiBridgeConfigStore();
                               if (config.setApiToken(keyboard.text)) config.save();
                             }
                             requestUpdate();
                           });
  } else if (index == 2) {
    if (!current.configured()) {
      requestUpdate();
      return;
    }
    startActivityForResult(
        std::make_unique<ConfirmationActivity>(renderer, mappedInput, tr(STR_LEARNER_ANKI_SYNC), tr(STR_ANKI_LAN_ONLY)),
        [this](const ActivityResult& result) {
          if (result.isCancelled) return;
          startActivityForResult(std::make_unique<CjkAnkiSyncActivity>(renderer, mappedInput),
                                 [this](const ActivityResult&) { requestUpdate(); });
        });
  }
}

void CjkAnkiSettingsActivity::buildScreen(UiScreen& screen) {
  const auto& config = ChinesePoint::Cjk::ankiBridgeConfigStore().config();
  values_[0] = config.serverUrl.empty() ? tr(STR_NOT_SET) : config.serverUrl;
  values_[1] = config.apiToken.empty() ? tr(STR_NOT_SET) : "******";
  values_[2] = config.configured() ? "" : std::string("[") + tr(STR_ANKI_CONFIGURE) + "]";
  for (int index = 0; index < kItems; ++index) items_[index].value = values_[index].empty() ? nullptr : values_[index].c_str();
  const auto& metrics = UITheme::getInstance().getMetrics();
  screen.setContentMargin(fui::Insets{static_cast<int16_t>(metrics.topPadding + metrics.headerHeight), 0,
                                      static_cast<int16_t>(metrics.buttonHintsHeight), 0});
  screen.spacer(static_cast<int16_t>(metrics.verticalSpacing));
  fui::ListProps props;
  props.items = items_;
  props.count = kItems;
  props.action = ACTION_ROW;
  props.inputMask = fui::InputTouch;
  props.valueInset = 8;
  props.labelText = screen.theme().smallText;
  props.labelText.maxLines = 2;
  syncListViewport(screen, props);
  screen.list(props);
}

const char* CjkAnkiSettingsActivity::headerTitle() const { return tr(STR_LEARNER_ANKI_SYNC); }
