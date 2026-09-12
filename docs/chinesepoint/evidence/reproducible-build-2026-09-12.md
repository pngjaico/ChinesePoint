# Clean X4 Pro build evidence — 2026-09-12

This is a local build record, not device or release evidence.

The previous canonical PlatformIO object cache could retain stale library
objects across source changes. A cached build then failed only at link time with
missing `Section` and `OpdsServerStore` symbols. The canonical
`platformio.ini` now leaves `build_cache_dir` unset; its ordinary `.pio` build
directory remains incremental when inputs are unchanged.

After `pio run -e chinesepoint_x4pro -t clean`, a full no-object-cache build
completed successfully. The post-build updater alias was generated and both
files matched exactly:

| File | Bytes | SHA-256 |
| --- | ---: | --- |
| `firmware.bin` | 5,379,216 | `761324065c89cc08d40577af300fc0694e712a8d80e97df15a3dc36cb473e9fe` |
| `update.bin` | 5,379,216 | `761324065c89cc08d40577af300fc0694e712a8d80e97df15a3dc36cb473e9fe` |

`verify_artifact.py` confirmed ESP32-S3, the single X4 Pro board tag, the
expected version marker, application-size limit, and `installable: false`.

The same build reports 90,204 bytes of ordinary RAM (27.5%), 5,378,702 bytes
of flash (82.1%), and the fixed 16,384-byte IRAM region fully allocated. The
zero-byte dedicated-IRAM margin remains a release risk; this build does not
prove watchdog safety, panel behavior, sleep/wake, frontlight, Anki over Wi-Fi,
or the recovery drill on a physical X4 Pro.
