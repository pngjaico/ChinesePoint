# Anki flashcard import check — 2026-09-12

## Change

The firmware's learner export already emits the bounded `answer` field. The
desktop bridge previously ignored it, so imported notes were vocabulary-only.
The bridge now retains that field, escapes it as text, and upgrades the
project-owned `ChinesePoint Vocabulary` model with an `Answer` field and a
`Recognition` template. Anki itself creates a card object even for an empty
conditional front, so the bridge does not create a note for an entry without a
local dictionary answer. A later export that contains an answer creates the
card for the first time.

The card front contains the saved word and source sentence. Its back contains
the saved dictionary answer, source path, and learner status. Anki Desktop is
the only scheduler after import. The device neither imports Anki due dates nor
applies Anki reviews to its local scheduler.

## Automated evidence

- `python -m unittest discover -s tests -v` in `tools/anki_bridge` passed
  **9/9** tests. Coverage includes strict answer parsing, empty-answer
  rejection before note creation, idempotent update, legacy model upgrade,
  and HTML escaping.
- `python real_collection_smoke.py --anki-app-packages
  C:\\Users\\Usuario-pc\\AppData\\Local\\Programs\\Anki\\app_packages` passed against
  a temporary real Anki collection. It verified first import, idempotent
  retry, update of the existing note, `Answer` persistence, and that a
  no-answer export creates neither a note nor a card while a later answered
  export creates one card.
- `tools/anki_bridge/dist/chinesepoint-anki-bridge-v0.6.2.ankiaddon` was
  rebuilt from this source. SHA-256:
  `9d16b4272380f9f56d772f6fbea3cc02adbb4627b8c7ea0ac1ecea31538a335a`.

## Limits

The smoke creates a temporary collection through Anki's installed collection
API. It does not launch Anki Desktop's GUI, exercise the add-on startup in a
real profile, transfer over X4 Pro Wi-Fi, or prove physical card rendering.
No release or device-installability claim follows from this record.
