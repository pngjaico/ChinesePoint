#include "CjkReviewActivity.h"

#include <Arduino.h>
#include <GfxRenderer.h>
#include <I18n.h>

#include <limits>

#include "MappedInputManager.h"
#include "chinesepoint/CjkSafetyGuard.h"
#include "chinesepoint/cjk/CjkLearnerStore.h"
#include "chinesepoint/cjk/CjkReviewScheduler.h"
#include "components/UIScale.h"
#include "components/UITheme.h"

namespace fui = freeink::ui;

namespace {
constexpr int kCardRows = 3;
constexpr int kAgainRow = kCardRows;
constexpr int kHardRow = kAgainRow + 1;
constexpr int kGoodRow = kHardRow + 1;
constexpr int kEasyRow = kGoodRow + 1;
}  // namespace

CjkReviewActivity::CjkReviewActivity(GfxRenderer& renderer, MappedInputManager& mappedInput)
    : UiListActivity("CjkReview", renderer, mappedInput) {}

void CjkReviewActivity::onEnter() {
  UiListActivity::onEnter();
  loadNextDueCard();
}

void CjkReviewActivity::onExit() {
  rowItems.clear();
  labels.clear();
  headword.clear();
  sourceSentence.clear();
  answer.clear();
  Activity::onExit();
}

bool CjkReviewActivity::loadNextDueCard() {
  dataAvailable = false;
  hasDueCard = false;
  wordId = 0;
  headword.clear();
  sourceSentence.clear();
  answer.clear();
  status.clear();

  if (!ChinesePoint::CjkSafetyGuard::startLearnerSession()) {
    rebuildRows();
    return false;
  }
  auto& store = ChinesePoint::Cjk::learnerStore();
  dataAvailable = store.load();
  if (dataAvailable) {
    clock = ChinesePoint::Cjk::StudyClock(store.repository().studyClock(), static_cast<int64_t>(millis()));
    const auto reading = clock.observe(static_cast<int64_t>(millis()));
    ChinesePoint::Cjk::ReviewScheduler scheduler;
    const ChinesePoint::Cjk::LearnerEntry* selected = nullptr;
    for (const auto& entry : store.repository().entries()) {
      if (entry.review.authority != ChinesePoint::Cjk::ScheduleAuthority::Local ||
          !ChinesePoint::Cjk::validCardAnswer(entry.cardAnswer) || !scheduler.isDue(entry.review, reading.nowMs)) {
        continue;
      }
      if (selected == nullptr || entry.review.dueAtMs < selected->review.dueAtMs) selected = &entry;
    }
    if (selected != nullptr) {
      hasDueCard = true;
      wordId = selected->wordId;
      headword = selected->headword;
      sourceSentence = selected->sourceSentence;
      answer = selected->cardAnswer;
    }
  }
  ChinesePoint::CjkSafetyGuard::finishLearnerSession();
  rebuildRows();
  return hasDueCard;
}

void CjkReviewActivity::rebuildRows() {
  labels.clear();
  rowItems.clear();
  if (!dataAvailable) {
    labels.emplace_back(tr(STR_LEARNER_DATA_UNAVAILABLE));
  } else if (!hasDueCard) {
    labels.emplace_back(tr(STR_LEARNER_NO_DUE));
  } else {
    labels.reserve(kEasyRow + 1);
    labels.emplace_back(headword);
    labels.emplace_back(std::string(tr(STR_LEARNER_CONTEXT)) + ": " + sourceSentence);
    labels.emplace_back(std::string(tr(STR_LEARNER_ANSWER)) + ": " + answer);
    labels.emplace_back(tr(STR_LEARNER_AGAIN));
    labels.emplace_back(tr(STR_LEARNER_HARD));
    labels.emplace_back(tr(STR_LEARNER_GOOD));
    labels.emplace_back(tr(STR_LEARNER_EASY));
  }
  rowItems.reserve(labels.size());
  for (size_t index = 0; index < labels.size(); ++index) {
    fui::ListItem item;
    item.label = labels[index].c_str();
    item.actionValue = static_cast<int16_t>(index);
    rowItems.push_back(item);
  }
  renderer.prewarmFallbackText(
      uiScaleSpec().smallFontId,
      [](const void* context, const uint32_t index) -> const char* {
        const auto& rows = *static_cast<const std::vector<std::string>*>(context);
        return index < rows.size() ? rows[index].c_str() : nullptr;
      },
      &labels, static_cast<uint32_t>(labels.size()));
}

void CjkReviewActivity::rate(const ChinesePoint::Cjk::Rating rating) {
  if (!hasDueCard || !ChinesePoint::CjkSafetyGuard::startLearnerSession()) return;
  const auto reading = clock.observe(static_cast<int64_t>(millis()));
  const bool rated = ChinesePoint::Cjk::learnerStore().rateLocalReview(wordId, headword, rating, reading.nowMs,
                                                                         reading.state);
  ChinesePoint::CjkSafetyGuard::finishLearnerSession();
  if (!rated) {
    status = tr(STR_LEARNER_REVIEW_FAILED);
    hasDueCard = false;
    rebuildRows();
    requestUpdate();
    return;
  }
  loadNextDueCard();
  requestUpdate();
}

void CjkReviewActivity::activateIndex(const int index) {
  if (!hasDueCard) return;
  switch (index) {
    case kAgainRow:
      rate(ChinesePoint::Cjk::Rating::Again);
      break;
    case kHardRow:
      rate(ChinesePoint::Cjk::Rating::Hard);
      break;
    case kGoodRow:
      rate(ChinesePoint::Cjk::Rating::Good);
      break;
    case kEasyRow:
      rate(ChinesePoint::Cjk::Rating::Easy);
      break;
    default:
      break;
  }
}

void CjkReviewActivity::buildScreen(UiScreen& screen) {
  const auto& metrics = UITheme::getInstance().getMetrics();
  screen.setContentMargin(fui::Insets{static_cast<int16_t>(metrics.topPadding + metrics.headerHeight), 0,
                                      static_cast<int16_t>(metrics.buttonHintsHeight), 0});
  screen.spacer(static_cast<int16_t>(metrics.verticalSpacing));
  if (!dataAvailable) {
    screen.centeredText(tr(STR_LEARNER_DATA_UNAVAILABLE), screen.theme().bodyText);
    return;
  }
  if (!hasDueCard) {
    screen.centeredText(status.empty() ? tr(STR_LEARNER_NO_DUE) : status.c_str(), screen.theme().bodyText);
    return;
  }
  fui::ListProps props;
  props.items = rowItems.data();
  props.count = static_cast<uint16_t>(rowItems.size());
  props.action = ACTION_ROW;
  props.inputMask = fui::InputTouch;
  props.labelText = screen.theme().smallText;
  props.labelText.maxLines = 2;
  syncListViewport(screen, props);
  screen.list(props);
}

const char* CjkReviewActivity::headerTitle() const { return tr(STR_LEARNER_REVIEW); }

void CjkReviewActivity::drawFooter() {
  const auto hints = mappedInput.mapLabels(tr(STR_BACK), hasDueCard ? tr(STR_SELECT) : "",
                                           hasDueCard ? tr(STR_DIR_UP) : "", hasDueCard ? tr(STR_DIR_DOWN) : "");
  GUI.drawButtonHints(renderer, hints.btn1, hints.btn2, hints.btn3, hints.btn4);
}
