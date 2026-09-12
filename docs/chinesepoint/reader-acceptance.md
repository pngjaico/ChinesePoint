# ChinesePoint 1.0 reader acceptance

This is the release gate for the reader. It applies to the exact X4 Pro
application artifact named in `site/release-manifest.json`; a simulator image,
an upstream CrossPoint release, or a previous ChinesePoint build is not a
substitute.

The person running a physical pass records the artifact SHA-256, X4 Pro panel
controller, SD card model/capacity, reader font, dictionary package checksum,
and a short serial log. Any reset, corrupted page, missing glyph, lost reading
position, or lookup failure is a failure until reproduced and resolved.

## Required reading corpus

Use clean EPUB fixtures and ordinary user books. The fixture names and book
titles must be recorded with the test result; copyrighted books are not stored
in this repository.

| Case | Pass condition |
| --- | --- |
| Latin EPUB | Open, turn at least 100 pages, change size and margins, create/reopen bookmark, close and resume the same location. |
| Simplified Chinese EPUB without spaces | CJK glyphs render without tofu; line breaks do not split UTF-8; selection moves among visible Chinese tokens. |
| Traditional Chinese EPUB | Same as simplified, including a configured fallback font. |
| Mixed Chinese and Latin | Punctuation does not become a selectable word; Latin selection remains unchanged. |
| Ruby, tables and images | Render, page-turn, back navigation and orientation changes do not crash or lose the chapter. |
| Long chapter | Open, navigate to the end, sleep/wake, reopen and preserve the saved progress. |
| 8 KiB unspaced CJK TXT | Delete its cache, open it, wait for indexing, reopen it, and record elapsed time plus serial `MEM` lines. There must be no watchdog reset, reboot, corrupt UTF-8, or lost progress. |

Run every case in portrait, portrait inverted, landscape clockwise and
landscape counter-clockwise. A screenshot alone cannot establish correct touch
selection or power-resume behavior.

## Dictionary and learner gate

1. Install the pinned CC-CEDICT package through the device flow and confirm
   that a cancel, network failure, checksum failure and interrupted SD write
   leave no selected partial dictionary.
2. From the simplified-Chinese corpus, check a direct headword, a word split
   into adjacent CJK tokens, punctuation adjacent to a word, a missing word,
   and a long definition. The direct StarDict match must win; the CJK phrase
   fallback is used only after a genuine local miss.
3. Repeat lookups after index creation and after sleep/wake. Record free heap
   before the first lookup and after the final lookup. A low-memory,
   decompression, or SD read error is a failure, not a valid `Not found`.
4. Save a found word and a miss with sentence context. Reopen Vocabulary,
   export the journal, and confirm that headword, original sentence, book path
   and anchor are retained.
5. Complete the Anki Desktop bridge round trip with an existing deck: first
   sync, second idempotent sync, intentional conflicting edit, and an offline
   device retry. Anki Desktop remains the scheduling authority.

## Stability probes from external reports

CrossPoint and CrossInk issue reports are test targets, not reproductions on
ChinesePoint. On every available physical panel, run a Wi-Fi scan and connect,
open a reader, turn pages, sleep/wake, and repeat the scan while collecting a
serial log and `MEM` values. Repeat the long CJK TXT case after a clean cache.
A panic, watchdog reset, freeze, mirrored output, stack-overflow message, or
materially falling free heap blocks release until investigated.

## Panel and recovery gate

Repeat the reader and dictionary smoke paths on SSD1677, UC8179 and UC8279.
For each panel, exercise normal boot, reader launch, sleep/wake, and the
physical **DOWN+POWER** recovery route. The recovery drill is mandatory even
when the normal reader test succeeds.

## Evidence required before 1.0

The manifest must link one physical evidence record per panel and recovery
state, as enforced by `verify_release_manifest.py`. The record additionally
names this acceptance document, the corpus cases exercised, the dictionary
package checksum and any known visual limitations. Until all records exist,
the artifact remains diagnostic and must not be described as safe to install.
