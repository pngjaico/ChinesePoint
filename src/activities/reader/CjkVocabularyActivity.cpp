#include "CjkVocabularyActivity.h"

#include <GfxRenderer.h>
#include <I18n.h>

#include <utility>

#include "chinesepoint/CjkSafetyGuard.h"
#include "DictionaryDefinitionActivity.h"
#include "MappedInputManager.h"
#include "chinesepoint/cjk/CjkLearnerStore.h"
#include "components/UIScale.h"
#include "components/UITheme.h"

namespace fui = freeink::ui;

CjkVocabularyActivity::CjkVocabularyActivity(GfxRenderer& renderer, MappedInputManager& mappedInput)
    : UiListActivity("CjkVocabulary", renderer, mappedInput) {}

void CjkVocabularyActivity::onEnter() {
  UiListActivity::onEnter();
  dataAvailable = false;
  entries.clear();
  if (ChinesePoint::CjkSafetyGuard::startLearnerSession()) {
    dataAvailable = ChinesePoint::Cjk::learnerStore().load();
    if (dataAvailable) {
      const auto& storedEntries = ChinesePoint::Cjk::learnerStore().repository().entries();
      entries.reserve(storedEntries.size());
      for (const auto& entry : storedEntries) entries.push_back(&entry);
    }
    ChinesePoint::CjkSafetyGuard::finishLearnerSession();
  }
  rebuildRows();
}

void CjkVocabularyActivity::onExit() {
  // ListItem text aliases learner entry strings. Clearing the views before the
  // activity stack unwinds makes that lifetime explicit and avoids retaining
  // a potentially large journal view underneath a re-opened reader menu.
  rowItems.clear();
  subtitles.clear();
  entries.clear();
  Activity::onExit();
}

const char* CjkVocabularyActivity::statusLabel(const ChinesePoint::Cjk::WordStatus status) {
  switch (status) {
    case ChinesePoint::Cjk::WordStatus::Encountered:
      return tr(STR_LEARNER_ENCOUNTERED);
    case ChinesePoint::Cjk::WordStatus::Saved:
      return tr(STR_LEARNER_SAVED);
    case ChinesePoint::Cjk::WordStatus::Learning:
      return tr(STR_LEARNER_LEARNING);
    case ChinesePoint::Cjk::WordStatus::Known:
      return tr(STR_LEARNER_KNOWN);
  }
  return tr(STR_LEARNER_ENCOUNTERED);
}

void CjkVocabularyActivity::rebuildRows() {
  subtitles.clear();
  rowItems.clear();
  if (!dataAvailable) return;

  subtitles.reserve(entries.size());
  rowItems.reserve(entries.size());
  for (size_t index = 0; index < entries.size(); ++index) {
    const auto& entry = *entries[index];
    subtitles.emplace_back(std::string(statusLabel(entry.status)) + " · " + std::to_string(entry.encounterCount) + " " +
                           tr(STR_LEARNER_ENCOUNTERS));
    fui::ListItem item;
    item.label = entry.headword.c_str();
    item.subtitle = subtitles.back().c_str();
    item.actionValue = static_cast<int16_t>(index);
    rowItems.push_back(item);
  }

  // Headwords can require an SD-backed CJK fallback font. Prewarm once when
  // the journal view changes so list repaints never stream a glyph per row.
  renderer.prewarmFallbackText(
      uiScaleSpec().smallFontId,
      [](const void* context, const uint32_t index) -> const char* {
        const auto& view = *static_cast<const std::vector<const ChinesePoint::Cjk::LearnerEntry*>*>(context);
        return index < view.size() ? view[index]->headword.c_str() : nullptr;
      },
      &entries, static_cast<uint32_t>(entries.size()), EpdFontFamily::BOLD);
}

void CjkVocabularyActivity::activateIndex(const int index) {
  if (index < 0 || index >= listCount()) return;
  const auto& entry = *entries[static_cast<size_t>(index)];
  std::string detail = std::string(statusLabel(entry.status)) + "\n" + tr(STR_LEARNER_ENCOUNTERS) + ": " +
                       std::to_string(entry.encounterCount);
  if (!entry.bookPath.empty()) detail += "\n" + std::string(tr(STR_LEARNER_BOOKS)) + ": " + entry.bookPath;
  if (!entry.sourceSentence.empty()) detail += "\n\n" + entry.sourceSentence;

  app.clearTapFlash();
  startActivityForResult(std::make_unique<DictionaryDefinitionActivity>(renderer, mappedInput, entry.headword,
                                                                         std::move(detail)),
                         [this](const ActivityResult&) { requestUpdate(); });
}

void CjkVocabularyActivity::buildScreen(UiScreen& screen) {
  const auto& metrics = UITheme::getInstance().getMetrics();
  screen.setContentMargin(fui::Insets{static_cast<int16_t>(metrics.topPadding + metrics.headerHeight), 0,
                                      static_cast<int16_t>(metrics.buttonHintsHeight), 0});
  screen.spacer(static_cast<int16_t>(metrics.verticalSpacing));
  if (!dataAvailable) {
    screen.centeredText(tr(STR_LEARNER_DATA_UNAVAILABLE), screen.theme().bodyText);
    return;
  }
  if (rowItems.empty()) {
    screen.centeredText(tr(STR_LEARNER_NO_ENTRIES), screen.theme().bodyText);
    return;
  }

  fui::ListProps props;
  props.items = rowItems.data();
  props.count = static_cast<uint16_t>(rowItems.size());
  props.action = ACTION_ROW;
  props.inputMask = fui::InputTouch;
  fui::TextStyle label = screen.theme().smallText;
  label.bold = true;
  props.labelText = label;
  syncListViewport(screen, props, /*hasSubtitle=*/true);
  screen.list(props);
}

const char* CjkVocabularyActivity::headerTitle() const { return tr(STR_LEARNER_VOCABULARY); }

void CjkVocabularyActivity::drawFooter() {
  const auto hints = mappedInput.mapLabels(tr(STR_BACK), rowItems.empty() ? "" : tr(STR_SELECT),
                                           rowItems.empty() ? "" : tr(STR_DIR_UP),
                                           rowItems.empty() ? "" : tr(STR_DIR_DOWN));
  GUI.drawButtonHints(renderer, hints.btn1, hints.btn2, hints.btn3, hints.btn4);
}
