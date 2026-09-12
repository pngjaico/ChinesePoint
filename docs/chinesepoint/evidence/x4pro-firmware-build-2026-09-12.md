# X4 Pro firmware build check — 2026-09-12

## Input and scope

`pio run -e chinesepoint_x4pro` completed successfully in 1,315.83 seconds.
It built the application source at `d24e15f`. `git diff --name-only
d24e15f..58171bc` contains only documentation and the desktop Anki bridge, so
the current branch has the same firmware application inputs as this build.

This is a local compiler and artifact record. It is not a flash, a device test,
or a release approval.

## Artifact result

| File | Bytes | SHA-256 |
| --- | ---: | --- |
| `firmware.bin` | 5,390,320 | `ea0cb4aaf74002d7f668cf6ec1f2154c392e0a9c93c1667a663cb684b1c4cd83` |
| `update.bin` | 5,390,320 | `ea0cb4aaf74002d7f668cf6ec1f2154c392e0a9c93c1667a663cb684b1c4cd83` |

`verify_artifact.py` accepted the application as target `xteink-x4-pro`, with
the sole board tag `CROSSPOINT-BOARD-V1:x4pro;` and version
`ChinesePoint-v0.6-x4pro`. Its generated manifest keeps `installable: false`.

## Memory observation

`esp-idf-size` 2.3.1 reported:

| Region | Used | Total | Free |
| --- | ---: | ---: | ---: |
| DIRAM | 157,078 | 341,760 | 184,682 |
| IRAM | 16,384 | 16,384 | **0** |
| RTC slow | 5,524 | 7,680 | 2,156 |
| RTC fast | 92 | 8,192 | 8,100 |

The zero-byte IRAM margin is unchanged and blocks an installable release. A
successful link does not prove this image is safe during Wi-Fi, display refresh,
sleep/wake, or recovery on a locked X4 Pro.

## Compiler warnings

The build deliberately redefines `__DATE__` and `__TIME__` to make the image
reproducible. The remaining warning is upstream WebSockets calling deprecated
`NetworkClient::flush()`. Neither is a build failure; the latter remains an
upstream maintenance item.
