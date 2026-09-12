# Upstream watch — 2026-09-12

This review records upstream state after the ChinesePoint X4 Pro candidate was
built. It is not a request to update dependencies and it does not change the
blocked release status.

## Observed heads

| Base | ChinesePoint pin | Observed upstream head | Decision |
| --- | --- | --- | --- |
| FreeInk | `7f6bd0f47a766eea18206dd19f723f3707b6c9d3` | `c881d219b05520d42ff93c20c2bf3a4c401c64fe` | Hold pending physical X4 Pro panel and recovery coverage. |
| CrossPoint `develop` | initial base `e7a3bb48817f1cb951b521ca958562723159c2f6` plus ChinesePoint changes | `472b5e485f9f55864133a2f64dd25256749041d8` | Review individual fixes in a dedicated integration branch; do not rebase the diagnostic candidate. |

## FreeInk: high display risk

The FreeInk range changes runtime display GPIO configuration, Xteink detection,
SSD1677, UC8179, UC8279, UC8279-X4 refresh paths, grayscale planes, SPI buffer
handling, input gestures and content decrypt/inflate allocation order. It adds
useful X4 support and host driver tests, but it is a broad panel-driver change.

ChinesePoint must not move this pin until all of the following are available:

1. the known-good CrossPoint backup has been restored through the physical
   DOWN+POWER route at least once;
2. the new pin has a clean X4 Pro build and the three simulator profiles pass;
3. separate physical SSD1677, UC8179 and UC8279 units pass cold boot,
   orientation, full/partial refresh, touch, sleep/wake and frontlight tests;
4. an inverted, mirrored, blank or non-responsive panel stops the rollout and
   uses the proven backup route.

The hold is deliberate. A simulated BMP cannot validate controller waveforms,
power sequencing or panel-specific lookup tables.

## New X4 Pro risk reports — reviewed 2026-09-12

These are upstream user reports, not reproduced ChinesePoint defects. They
raise the physical acceptance bar and are reasons not to merge a newer SDK or
call a simulator result device-safe.

| Report | Observed scope | ChinesePoint decision |
| --- | --- | --- |
| [CrossPoint #3371](https://github.com/crosspoint-reader/crosspoint-reader/issues/3371) | An X4 Pro user on v1.6.0 reported that the cold frontlight channel starts then disappears, leaving only warm light. | Require sustained cold-only, warm-only, mixed, off/on, sleep/wake, and Wi-Fi-use frontlight observations on every physical panel row. Do not infer frontlight health from boot or a simulator. |
| [CrossPoint #3392](https://github.com/crosspoint-reader/crosspoint-reader/issues/3392) | An X4 Pro v1.6.0 report described a memory error when downloading a font immediately after reset. | Keep font download outside the first recovery candidate. In the later reader matrix, capture free heap and maximum allocation before/after download and stop on an allocation failure or falling memory floor. |
| [CrossPoint v1.6.0 RC discussion #3099](https://github.com/crosspoint-reader/crosspoint-reader/discussions/3099) | Users reported substantial idle battery loss; the discussion linked it to display-variant behavior. | Run an overnight sleep/battery observation only after baseline recovery works. It is a release gate for the tested device, never evidence for untested controller variants. |
| [CrossPoint simulator README](https://github.com/crosspoint-reader/crosspoint-simulator) | The UC8279 X4 Pro simulator path explicitly remains pending physical validation. | Keep UC8279 `pending` in the release manifest until a separate real unit passes. The Home screenshot is only a code-path check. |

FreeInk's current README describes the X4 Pro as auto-detecting SSD1677 and
UC8179, while its build-composition table also describes a UC8279 path. That
documentation discrepancy is itself a reason to retain the three-path matrix
and the pinned SDK until hardware logs resolve the actual controller.

## FreeInk change since the previous observation

The observed FreeInk head advanced from `76e66b1` to `c881d21`. Relative to
the ChinesePoint pin, the display, Xteink-detection and BoardConfig range now
changes 43 files (1,787 additions and 203 deletions). It includes runtime GPIO
configuration, Xteink probe changes, SSD1677/UC8179/UC8279/UC8279-X4 waveform
and grayscale work, and new upstream host tests. In particular, it changes
both the driver-selection probe and the refresh implementations for the three
X4 Pro controller paths.

That is evidence for a separate integration candidate, not a safe cherry-pick
set. Pulling only a detector or only a waveform would couple it to an older
driver contract and make a display failure harder to attribute. ChinesePoint
keeps `7f6bd0f` until the physical recovery baseline exists.

## CrossPoint fixes worth staging later

| Upstream change | Benefit | Why it is not merged now |
| --- | --- | --- |
| `46d9125` OTA recognizes the new format | Could improve compatibility with future update metadata. | It changes board-tag and OTA paths that protect recovery. It needs corrupt/wrong-board/rollback device drills. |
| `9ba1608` wake idle poll on raw contact | Could prevent missed short button presses. | It changes `HalGPIO` and boot input ordering; the X4 Pro recovery gesture must be tested first. |
| `3e60e13` PSRAM monitor data | Better serial diagnostics. | Monitor-only; ChinesePoint already has the early PSRAM guard. |
| `b7bceb5`, `93b6fe1` memory/glyph fixes | Potentially lower reader and font pressure. | They cross Epub, KOReader Sync, inflate and font lifetime code, so require corpus and sleep/wake acceptance. |
| `03c4847` webserver path normalization | Security improvement for the optional webserver. | It spans filesystem helper and HTML changes; integrate as a focused security patch after the physical recovery baseline. |
| `e5dcc64` grayscale consolidation | Useful capability model. | It deliberately changes reader, sleep and display semantics and depends on the newer FreeInk range. |

## Next integration order

1. Complete the physical backup/recovery baseline on a real X4 Pro.
2. Stage CrossPoint OTA and short-button fixes independently, with targeted
   host tests and a hardware recovery drill after each change.
3. Stage FreeInk in a separate branch, then repeat the entire three-panel
   physical matrix before merging it to the diagnostic candidate.

Until those records exist, `site/release-manifest.json` remains blocked and no
new upstream head may be described as safe for X4 Pro installation.
