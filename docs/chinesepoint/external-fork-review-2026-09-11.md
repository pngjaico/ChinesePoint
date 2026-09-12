# External fork review — 2026-09-11

This is a design review of external projects, not an endorsement or a release
claim.  Their source was inspected at the commits below; no firmware code was
copied into ChinesePoint from this review.

## Follow-up status — 2026-09-12

The controlled FreeInk update was completed at
`7f6bd0f47a766eea18206dd19f723f3707b6c9d3`. An isolated-cache X4 Pro build,
artifact identity check, boot-order test, and all three simulator profiles
(SSD1677, UC8179, UC8279) passed. The recovery route was moved ahead of normal
reader state, settings, optional services, and frontlight initialization. None
of this is a physical-panel, PSRAM, deep-sleep, touch, or recovery-drill result;
the release manifest remains blocked.

| Project | Reviewed commit | Useful finding | Decision |
| --- | --- | --- | --- |
| Papyrix | `4a8afdbe660e72c4a60565ed02b147a1797d2558` | Clear SD update and crash-recovery documentation; explicit image validation | Do not port. Its X4 Pro recovery helper currently returns an SSD1677 fixed panel although its own support matrix lists UC8179 and UC8279. It cannot be the source of ChinesePoint display or recovery code. |
| CrossPlay (`xteink`) | `f29f0ab682114000e93d77629704609caadb01c3` | The X4 Pro input budget is genuinely small: touch, two side keys, power and the capacitive Home key | Keep ChinesePoint controls discoverable and avoid features that require legacy bottom buttons. Do not import its app/game stack into a reader build. Its reported mirrored-display incident is a dependency-age warning. |
| Matcha Reader | `61ca61ba86e3c5709a24d1b9c4f3cf2d41488012` | Dictionary data is split by purpose, and a book-level language override can correct bad automatic detection | Retain this as a product reference. Its deinflector, furigana and vertical layout are Japanese-specific and must not be relabelled as Chinese segmentation. ChinesePoint needs a bounded Chinese tokenizer and CEDICT-based lookup, not a Japanese code port. |

## Immediate decisions

1. Keep FreeInk as the only X4 Pro display-controller source. Its pin is now
   `7f6bd0f47a766eea18206dd19f723f3707b6c9d3`; any later update needs the same
   controlled review, isolated build, and three simulator profiles. This still
   does not dismiss the CrossPlay mirrored/inverted-display report without
   physical coverage of all three controller paths.
2. The minimal DOWN+POWER route now reaches the verified SD updater before
   optional services. A deliberate 2.5-second hold may restore only
   `/backup/crosspoint-x4pro.bin` with its separately verified SHA-256. It is
   intentionally not a generic `force_update.bin` route, and has no physical
   recovery-drill evidence yet.
3. Do not add Wi-Fi-first features while the PSRAM, display and recovery
   gates are unresolved. The learning flow remains local-first: a bounded
   journal, manual export, then an opt-in token-authenticated Anki Desktop
   sync.

## Path to the first installable 1.0 candidate

1. **Emulator baseline:** build and capture boot screens for SSD1677, UC8179
   and UC8279 from this exact source revision. This checks code paths only.
2. **Dependency safety update:** on every new FreeInk revision, review and pin
   it; rebuild all simulator profiles and retain an X4 Pro boot diagnostic for
   usable PSRAM.
3. **Recovery isolation:** physically exercise the minimal DOWN+POWER route
   and the deliberate backup restore. Demonstrate rejection of missing,
   wrong-digest, wrong-target, oversized, and corrupt images before a valid
   restore is attempted.
4. **Reading and learning acceptance:** verify CJK rendering, dictionary
   misses with preserved context, journal persistence, CC-CEDICT integrity,
   export and idempotent Anki Desktop sync in the emulator where applicable.
5. **Physical matrix:** test separate SSD1677, UC8179 and UC8279 X4 Pro
   units, including cold boot, orientation, full/partial refresh, sleep/wake,
   frontlight, touch and recovery drill. One device is one row, not evidence
   for the other rows.
6. **Release:** publish an app-only `firmware.bin`, its SHA-256 and the
   observed evidence only after every preceding gate passes. Until then,
   `site/release-manifest.json` remains blocked.
