# X4 Pro internal-memory map audit — `8e92861`

This is a local, non-flashing analysis of the X4 Pro application built from
source commit `8e92861` on 2026-09-12. It corrects a misleading interpretation
of the PlatformIO size summary: its full 16 KiB `IRAM` row is the
**dedicated** portion, not the complete executable-memory capacity of ESP32-S3.

## Evidence

The linker map declares:

| Region | Address | Length |
| --- | --- | ---: |
| `iram0_0_seg` | `0x40374000` | 358,144 bytes |
| `dram0_0_seg` | `0x3fc88000` | 341,760 bytes |

The application occupies 1,028 bytes of vectors plus 82,207 bytes of
`.iram0.text`. The build's ESP-IDF size report accounts for the overlap as:

| Memory type | Static used | Static remainder |
| --- | ---: | ---: |
| DIRAM | 157,078 bytes | 184,682 bytes |
| dedicated IRAM | 16,384 bytes | 0 bytes |

ESP-IDF labels memory reachable from both instruction and data buses as
**DIRAM**; the ESP32-S3 documentation explains that internal SRAM unused for
IRAM becomes DRAM and that runtime heap is smaller than the static remainder
because of SDK startup allocations. [ESP-IDF size guide](https://docs.espressif.com/projects/esp-idf/en/v5.5/esp32s3/api-guides/tools/idf-size.html)
and [ESP32-S3 memory guide](https://docs.espressif.com/projects/esp-idf/en/v5.4.2/esp32s3/api-guides/memory-types.html)
provide the underlying model.

The artifact was a valid X4 Pro application and its `firmware.bin` and
`update.bin` aliases were byte-identical:

- SHA-256: `7c24667ae972f37e035984679e2cfd0400697565c47f10ff229ac46bd5d06c27`
- Size: 5,391,024 bytes
- Version: `ChinesePoint-v0.6-x4pro`

`verify_iram_attribution.py` passed: no ChinesePoint symbol was placed in an
IRAM text or vector section.

## What this does and does not establish

It removes the false claim that there are zero bytes of usable executable
memory. It does **not** prove runtime headroom, because USB-MSC, Wi-Fi,
FreeInk display work, reader allocation, task stacks, and fragmentation occur
after boot. The physical matrix must capture internal heap before and after
reading, Wi-Fi/Anki use, USB transfer, sleep/wake, and a panel refresh. Until
then this remains a development build, not an installable release.
