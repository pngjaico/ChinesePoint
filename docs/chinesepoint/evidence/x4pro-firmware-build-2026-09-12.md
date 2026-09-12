# X4 Pro firmware build check — 2026-09-12

## Input and scope

The first clean builds of source revision `b166b10` had equal loadable
segments but different application hashes. The difference came from the ELF
SHA recorded in the ESP-IDF app descriptor: ordinary linked ELF files include
debug paths, and their paths differ between worktrees. That made the initial
`firmware.bin` hashes unsuitable as a reproducibility gate even though the
program bytes were equal.

The feature environment now strips non-loadable debug and symbol data into a
temporary ELF, regenerates the ESP32-S3 application image from it, then copies
that normalized image to `update.bin`. A complete integrated
`pio run -e chinesepoint_x4pro` succeeded in 95.47 seconds after the cached
link. The source continues to isolate the general X4 Pro CrossPoint environment
from ChinesePoint; only `chinesepoint_x4pro` compiles the learner, dictionary,
and Anki paths.

This is a local compiler and artifact record. It is not a flash, a device test,
or a release approval.

## Artifact result

| File | Bytes | SHA-256 |
| --- | ---: | --- |
| `firmware.bin` | 5,390,320 | `60457af20c3d9e16f96c4853af08f8e11866dfd6d7bed7c1446401aef2a1062b` |
| `update.bin` | 5,390,320 | `60457af20c3d9e16f96c4853af08f8e11866dfd6d7bed7c1446401aef2a1062b` |

The normalized build exactly matches the independently regenerated normalized
image from the first clean linked ELF. `esptool image-info` reports an ESP32-S3
image with seven segments, 16 MB/DIO/80 MHz flash settings, valid checksum
`0x95`, and a valid validation hash. Its app descriptor records the canonical
stripped-ELF SHA-256
`3b6c2097ce633b2921ab351863eadbfb76ffb0e36bd2a7f4f61e6318c2a063bc`.
This proves deterministic local packaging for the observed clean inputs. It
does not prove a device boot, display output, or safe flash.

A fresh WSL/GCC 13.3 host build of the current source also completed and
`ctest` passed 222/222 cases. The Python build/release checks passed 13/13.
The host build emitted pre-existing warnings for partial test font initializers,
MiniBidi signedness, and Windows/WSL clock skew; none failed compilation or a
test. Host checks do not exercise the ESP32-S3, a physical display controller,
or recovery hardware.

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

The zero applies to dedicated IRAM. ESP32-S3 also uses shared D/IRAM, whose
static remainder is 184,682 bytes in this build; see
[`x4pro-memory-map-audit-2026-09-12.md`](x4pro-memory-map-audit-2026-09-12.md).
This still does not prove runtime internal heap or safe operation during Wi-Fi,
display refresh, sleep/wake, or recovery on a locked X4 Pro.

## Compiler warnings

The build deliberately redefines `__DATE__` and `__TIME__` to make the image
reproducible. The remaining warning is upstream WebSockets calling deprecated
`NetworkClient::flush()`. Neither is a build failure; the latter remains an
upstream maintenance item.
