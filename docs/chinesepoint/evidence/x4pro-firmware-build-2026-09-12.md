# X4 Pro firmware build check — 2026-09-12

## Input and scope

`pio run -e chinesepoint_x4pro` completed successfully in 1,100.49 seconds.
It built the current application source at `b166b10`. That revision contains
the explicit X4 Pro isolation: the general CrossPoint environment excludes
ChinesePoint sources, while this feature environment compiles the learner,
dictionary and Anki paths.

This is a local compiler and artifact record. It is not a flash, a device test,
or a release approval.

## Artifact result

| File | Bytes | SHA-256 |
| --- | ---: | --- |
| `firmware.bin` | 5,390,320 | `5eb1f9ccc7b828d061f15acc12502fb1686dffbc2976898ea5a6a459f52aff56` |
| `update.bin` | 5,390,320 | `5eb1f9ccc7b828d061f15acc12502fb1686dffbc2976898ea5a6a459f52aff56` |

`verify_artifact.py` accepted the application as target `xteink-x4-pro`, with
the sole board tag `CROSSPOINT-BOARD-V1:x4pro;` and version
`ChinesePoint-v0.6-x4pro`. Its generated manifest keeps `installable: false`.
The IRAM attribution check also passed: no ChinesePoint symbols appeared in
the dedicated IRAM sections.

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
