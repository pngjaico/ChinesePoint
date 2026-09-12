# Anki flashcard import check — 2026-09-12

## Change

The firmware's learner export already emits the bounded `answer` field. The
desktop bridge previously ignored it, so imported notes were vocabulary-only.
The bridge now retains that field, escapes it as text, and upgrades the
project-owned `ChinesePoint Vocabulary` model with an `Answer` field and a
`Recognition` template. The template has an `{{#Answer}}` front conditional:
an entry without a local dictionary answer creates no empty Anki card; a later
export that contains an answer activates the same note as a card.

The card front contains the saved word and source sentence. Its back contains
the saved dictionary answer, source path, and learner status. Anki Desktop is
the only scheduler after import. The device neither imports Anki due dates nor
applies Anki reviews to its local scheduler.

## Automated evidence

- `python -m unittest discover -s tests -v` in `tools/anki_bridge` passed
  **8/8** tests. Coverage includes strict answer parsing, empty-answer
  handling, idempotent update, legacy model upgrade, and HTML escaping.
- `python real_collection_smoke.py --anki-app-packages
  C:\\Users\\Usuario-pc\\AppData\\Local\\Programs\\Anki\\app_packages` passed against
  a temporary real Anki collection. It verified first import, idempotent
  retry, update of the existing note, `Answer` persistence, and the card
  template conditional.
- `tools/anki_bridge/dist/chinesepoint-anki-bridge-v0.6.1.ankiaddon` was
  rebuilt from this source. SHA-256:
  `a59019f801b51466e96d1ff35878c628283a845aed8e76e4eabf2557eeb23469`.

## Limits

The smoke creates a temporary collection through Anki's installed collection
API. It does not launch Anki Desktop's GUI, exercise the add-on startup in a
real profile, transfer over X4 Pro Wi-Fi, or prove physical card rendering.
No release or device-installability claim follows from this record.
