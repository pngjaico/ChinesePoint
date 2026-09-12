# External fork review — 2026-09-11

This is a design review of external projects, not an endorsement or a release
claim.  Their source was inspected at the commits below; no firmware code was
copied into ChinesePoint from this review.

| Project | Reviewed commit | Useful finding | Decision |
| --- | --- | --- | --- |
| Papyrix | `4a8afdbe660e72c4a60565ed02b147a1797d2558` | Clear SD update and crash-recovery documentation; explicit image validation | Do not port. Its X4 Pro recovery helper currently returns an SSD1677 fixed panel although its own support matrix lists UC8179 and UC8279. It cannot be the source of ChinesePoint display or recovery code. |
| CrossPlay (`xteink`) | `f29f0ab682114000e93d77629704609caadb01c3` | The X4 Pro input budget is genuinely small: touch, two side keys, power and the capacitive Home key | Keep ChinesePoint controls discoverable and avoid features that require legacy bottom buttons. Do not import its app/game stack into a reader build. Its reported mirrored-display incident is a dependency-age warning. |
| Matcha Reader | `61ca61ba86e3c5709a24d1b9c4f3cf2d41488012` | Dictionary data is split by purpose, and a book-level language override can correct bad automatic detection | Retain this as a product reference. Its deinflector, furigana and vertical layout are Japanese-specific and must not be relabelled as Chinese segmentation. ChinesePoint needs a bounded Chinese tokenizer and CEDICT-based lookup, not a Japanese code port. |

## Immediate decisions

1. Keep FreeInk as the only X4 Pro display-controller source. Before any
   candidate is installed, update its pinned revision in a controlled change,
   inspect the X4 Pro driver and deep-sleep diffs, and rebuild all three
   simulator profiles. The current pin is not current enough to dismiss the
   CrossPlay mirrored/inverted-display report.
2. Do not add a pre-UI automatic `force_update.bin` route. A stale file that
   flashes automatically is a worse failure mode for a USB-locked device. The
   existing DOWN+POWER route must instead be made minimal enough to reach the
   verified SD updater even when optional reader and learner state fail.
3. Do not add Wi-Fi-first features while the PSRAM, display and recovery
   gates are unresolved. The learning flow remains local-first: a bounded
   journal, manual export, then an opt-in token-authenticated Anki Desktop
   sync.

## Path to the first installable 1.0 candidate

1. **Emulator baseline:** build and capture boot screens for SSD1677, UC8179
   and UC8279 from this exact source revision. This checks code paths only.
2. **Dependency safety update:** review and pin current FreeInk; add a test
   that requires controller resolution before `display.begin()` and an X4 Pro
   boot diagnostic for usable PSRAM.
3. **Recovery isolation:** route DOWN+POWER to the SD updater before reader
   settings, frontlight, network and learner initialization. Exercise valid,
   wrong-target and corrupt-image rejection.
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
