# CrossPoint X4 Pro isolation build — 2026-09-12

## Purpose

The general `x4pro` environment previously compiled ChinesePoint learner
activities even though `CHINESEPOINT` was not defined. Their safety guard has
no non-ChinesePoint implementation, so the full image failed to link. This
record validates the correction that excludes ChinesePoint sources from the
general environment and gates the reader and dictionary integration behind
`CHINESEPOINT`.

This is a compiler result only. It is neither a ChinesePoint firmware release
nor physical X4 Pro evidence.

## Result

An isolated worktree at source `3209bb4`, with this patch copied in, ran:

```text
pio run -c platformio.isolation.ini -e x4pro
```

The full clean build succeeded in **1,057.47 seconds**.

| Item | Result |
| --- | --- |
| ChinesePoint object files in `src/chinesepoint` | 0 |
| `firmware.bin` bytes | 5,337,088 |
| `firmware.bin` SHA-256 | `49ad8853029381a6c092154b24203183bdeba6d08dd51135085f0382d5de031a` |
| Flash reported by PlatformIO | 5,336,582 / 6,553,600 bytes (81.4%) |
| RAM reported by PlatformIO | 89,836 / 327,680 bytes (27.4%) |

`esp-idf-size` reported 156,686 / 341,760 bytes of DIRAM and 16,384 / 16,384
bytes of dedicated IRAM. `verify_iram_attribution.py` found no ChinesePoint
symbols in IRAM. The SDK still leaves no dedicated-IRAM headroom, so this does
not establish device safety or make any image installable.

The only compiler warning was the existing upstream WebSockets use of the
deprecated `NetworkClient::flush()` API.