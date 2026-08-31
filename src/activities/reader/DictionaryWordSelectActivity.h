#pragma once

#include <Epub/Page.h>
#include <I18n.h>

#include <memory>
#include <array>
#include <string>
#include <vector>

#include "activities/Activity.h"
#include "chinesepoint/cjk/CjkSentenceSelection.h"
#include "util/Dictionary.h"

// Word selection over the current reader page: Left/Right step through words
// in reading order, Up/Down jump rows, Confirm opens the configured local
// dictionary and then the DictionaryDefinitionActivity. Without a dictionary,
// it still opens that activity so the selected word and sentence can be saved;
// it never substitutes an online lookup. Back returns to the reader. On touch
// devices a touch-down moves the highlight and a tap opens the selected word.
class DictionaryWordSelectActivity final : public Activity {
 public:
  explicit DictionaryWordSelectActivity(GfxRenderer& renderer, MappedInputManager& mappedInput,
                                        std::unique_ptr<Page> page, int marginLeft, int marginTop,
                                        uint16_t spineIndex, bool startsAtSectionBoundary,
                                        bool endsAtSectionBoundary, std::string bookPath)
      : Activity("DictionaryWordSelect", renderer, mappedInput),
        page(std::move(page)),
        marginLeft(marginLeft),
        marginTop(marginTop),
        spineIndex(spineIndex),
        startsAtSectionBoundary(startsAtSectionBoundary),
        endsAtSectionBoundary(endsAtSectionBoundary),
        bookPath(std::move(bookPath)) {}

  void onEnter() override;
  void loop() override;
  void render(RenderLock&&) override;

 private:
  // Screen box of one selectable word. `text` points into the owned Page's
  // TextBlock arena (NUL-terminated), valid for this activity's lifetime.
  struct WordBox {
    int16_t x;
    int16_t y;
    int16_t width;
    uint16_t row;
    const char* text;
    EpdFontFamily::Style style;
  };

  enum class Popup : uint8_t { None, Busy, NotFound, Error };

  void extractWords();
  int closestInRow(uint16_t row, int centerX) const;
  int wordAt(int x, int y) const;
  void moveVertical(int direction);
  void performLookup();
  bool drawHighlightWithSnapshot();
  void drawHints() const;

  std::unique_ptr<Page> page;
  const int marginLeft;
  const int marginTop;
  int fontId = 0;
  int lineHeight = 0;

  std::vector<WordBox> words;
  std::vector<ChinesePoint::Cjk::SelectableToken> learnerTokens;
  std::array<char, ChinesePoint::Cjk::kMaxSentenceBytes + 1> learnerSentence{};
  const uint16_t spineIndex;
  const bool startsAtSectionBoundary;
  const bool endsAtSectionBoundary;
  const std::string bookPath;
  int selected = 0;
  uint16_t rowCount = 0;

  Dictionary dict;
  bool dictOpenAttempted = false;
  bool dictOpenOk = false;
  bool dictNeedsIndex = false;

  Popup popup = Popup::None;
  StrId popupMsg = StrId::STR_DICT_NOT_FOUND;
  unsigned long popupTime = 0;

  // Differential highlight repaint: the pixels under the current highlight
  // box, so a cursor move restores them and repaints only the two affected
  // boxes instead of re-running the full two-pass page render (which also
  // reloads every SD-font glyph on the page). snapshotIdx is the word whose
  // under-pixels are saved; -1 means the framebuffer no longer holds a clean
  // page (popup drawn, sub-activity shown) and the next render must be full.
  static constexpr size_t SNAPSHOT_CAPACITY = 4096;
  std::unique_ptr<uint8_t[]> snapshot;
  int16_t snapshotX = 0;
  int16_t snapshotY = 0;
  int16_t snapshotW = 0;
  int16_t snapshotH = 0;
  int snapshotIdx = -1;
};
