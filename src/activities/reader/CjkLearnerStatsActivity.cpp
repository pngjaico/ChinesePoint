#include "CjkLearnerStatsActivity.h"

#include <GfxRenderer.h>
#include <I18n.h>

#include <utility>

#include "MappedInputManager.h"
#include "CjkVocabularyActivity.h"
#include "activities/util/ConfirmationActivity.h"
#include "chinesepoint/CjkSafetyGuard.h"
#include "chinesepoint/cjk/CjkLearnerStore.h"
#include "components/UITheme.h"

namespace fui = freeink::ui;

CjkLearnerStatsActivity::CjkLearnerStatsActivity(GfxRenderer& renderer, MappedInputManager& mappedInput)
    : UiListActivity("CjkLearnerStats", renderer, mappedInput) {}

void CjkLearnerStatsActivity::onEnter() {
  UiListActivity::onEnter();
  // Loading learner data is an optional CJK operation. The guard lets a
  // previous learner-only crash degrade to this harmless unavailable screen;
  // it never prevents returning to the normal reader.
  if (ChinesePoint::CjkSafetyGuard::startLearnerSession()) {
    dataAvailable = ChinesePoint::Cjk::learnerStore().load();
    if (dataAvailable) stats = ChinesePoint::Cjk::learnerStore().stats();
    ChinesePoint::CjkSafetyGuard::finishLearnerSession();
  }
  rebuildRows();
}

void CjkLearnerStatsActivity::rebuildRows() {
  labels.clear();
  values.clear();
  rowItems.clear();

  if (!dataAvailable) {
    labels.emplace_back(tr(STR_LEARNER_DATA_UNAVAILABLE));
  } else if (stats.vocabularyCount == 0) {
    labels.emplace_back(tr(STR_LEARNER_NO_ENTRIES));
  } else {
    labels = {tr(STR_LEARNER_VOCABULARY), tr(STR_LEARNER_EXPORT), tr(STR_LEARNER_SAVED), tr(STR_LEARNER_LEARNING),
              tr(STR_LEARNER_KNOWN), tr(STR_LEARNER_ENCOUNTERS), tr(STR_LEARNER_BOOKS)};
    values = {std::to_string(stats.vocabularyCount), exportStatus, std::to_string(stats.savedCount),
              std::to_string(stats.learningCount), std::to_string(stats.knownCount),
              std::to_string(stats.encounterTotal), std::to_string(stats.sourceBookCount)};
  }

  rowItems.reserve(labels.size());
  for (size_t index = 0; index < labels.size(); ++index) {
    fui::ListItem item;
    item.label = labels[index].c_str();
    item.value = index < values.size() ? values[index].c_str() : nullptr;
    item.actionValue = static_cast<int16_t>(index);
    rowItems.push_back(item);
  }
}

void CjkLearnerStatsActivity::activateIndex(const int index) {
  // The first summary row is intentionally an affordance, not a separate
  // menu entry: it keeps the reader menu compact on X4 Pro while giving the
  // user a direct path from the vocabulary total to its local entries.
  if (!dataAvailable || stats.vocabularyCount == 0) return;
  if (index == 0) {
    app.clearTapFlash();
    startActivityForResult(std::make_unique<CjkVocabularyActivity>(renderer, mappedInput),
                           [this](const ActivityResult&) { requestUpdate(); });
    return;
  }
  if (index == 1) exportEntries();
}

void CjkLearnerStatsActivity::exportEntries() {
  // Export is opt-in even though it is non-destructive. The confirmation makes
  // the SD destination visible and prevents an accidental refresh while users
  // are only browsing their statistics.
  const std::string destination = ChinesePoint::Cjk::learnerStore().exportPath();
  app.clearTapFlash();
  startActivityForResult(
      std::make_unique<ConfirmationActivity>(renderer, mappedInput, tr(STR_LEARNER_EXPORT), destination),
      [this](const ActivityResult& result) {
        if (result.isCancelled) return;
        bool exported = false;
        if (ChinesePoint::CjkSafetyGuard::startLearnerSession()) {
          exported = ChinesePoint::Cjk::learnerStore().exportJsonl();
          ChinesePoint::CjkSafetyGuard::finishLearnerSession();
        }
        exportStatus = exported ? tr(STR_LEARNER_EXPORT_READY) : tr(STR_LEARNER_EXPORT_FAILED);
        rebuildRows();
        requestUpdate();
      });
}

void CjkLearnerStatsActivity::buildScreen(UiScreen& screen) {
  const auto& metrics = UITheme::getInstance().getMetrics();
  screen.setContentMargin(fui::Insets{static_cast<int16_t>(metrics.topPadding + metrics.headerHeight), 0,
                                      static_cast<int16_t>(metrics.buttonHintsHeight), 0});
  screen.spacer(static_cast<int16_t>(metrics.verticalSpacing));
  if (rowItems.empty()) return;

  fui::ListProps props;
  props.items = rowItems.data();
  props.count = static_cast<uint16_t>(rowItems.size());
  props.action = ACTION_ROW;
  props.inputMask = fui::InputTouch;
  props.valueInset = 8;
  props.labelText = screen.theme().smallText;
  props.labelText.maxLines = 2;
  syncListViewport(screen, props);
  screen.list(props);
}

const char* CjkLearnerStatsActivity::headerTitle() const { return tr(STR_CHINESEPOINT_LEARNER); }

void CjkLearnerStatsActivity::drawFooter() {
  const auto hints = mappedInput.mapLabels(tr(STR_BACK),
                                           dataAvailable && stats.vocabularyCount > 0 ? tr(STR_SELECT) : "", "", "");
  GUI.drawButtonHints(renderer, hints.btn1, hints.btn2, hints.btn3, hints.btn4);
}
