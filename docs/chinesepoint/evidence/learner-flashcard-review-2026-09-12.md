# Learner flashcard answer and review check — 2026-09-12

## Source

- `e35f48e`: a successful local dictionary lookup saves a bounded, plain-text
  answer in a separate checksummed learner-journal record.
- `eee3837` and `6823348`: add the local review activity and its required
  rating type include.

The separate answer record avoids changing the existing entry-snapshot wire
format. Existing journals remain readable. A subsequent entry snapshot or
review mutation preserves the already replayed answer rather than erasing it.

## Review behavior

The learner menu now opens **Review**. It selects only local-authority cards
that are due and have a non-empty saved answer. It renders the word, source
context, answer, and Again/Hard/Good/Easy rows. Ratings use the existing atomic
review-mutation path with the durable study clock. Vocabulary entries saved
without a successful dictionary definition remain browseable but cannot be
rated as flashcards.

This does **not** make Anki the schedule authority: the current LAN bridge is
vocabulary export only. There is no Anki pull/schedule conflict policy yet.

## Automated evidence

- Host suite: `ctest --test-dir build-host --output-on-failure` passed
  **222/222** tests. New tests cover answer-record codec validity, replay after
  a later snapshot, and rejection of review without an answer.
- X4 Pro simulator SSD1677: `pio run -e
  chinesepoint_simulator_x4pro_ssd1677` at `6823348` completed the final link
  successfully in 181.003 seconds.

## Limits

The host suite does not drive the physical four buttons or inspect layout, and
the simulator build does not prove that the review screen fits or refreshes
correctly on an individual X4 Pro panel. UC8179 and UC8279 were successfully
built before this UI change, but must be rebuilt with this exact revision
before any display-profile claim. Physical device validation, recovery drill,
and IRAM headroom are still release gates.
