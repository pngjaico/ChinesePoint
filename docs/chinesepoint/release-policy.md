# ChinesePoint X4 Pro release policy

ChinesePoint publishes no firmware for a device other than the Xteink X4 Pro.
The release workflow emits only an ESP32-S3 application `firmware.bin` built
from `chinesepoint_x4pro`. It must never emit a binary named for X3, original
X4, X4 Classic, Sticky, PaperMono, or another CrossPoint board.

## Evidence gate

`tools/chinesepoint/verify_release_manifest.py` validates
`site/release-manifest.json`. A candidate may be blocked, but a version tag
matching `chinesepoint-v*` requires all of the following before its artifact
can be called installable:

1. A valid X4 Pro application header, one X4 Pro board tag, exact SHA-256,
   byte count, and ChinesePoint version marker.
2. A matching-source simulator pass for SSD1677, UC8179, and UC8279.
3. A physical pass recorded for each of those panel paths. A result from one
   X4 Pro must not be extrapolated to every panel variant.
4. A physical recovery drill using **DOWN+POWER**. UP is GPIO0 and does not
   replace this route. Every passed panel and recovery state must reference an
   evidence record in the manifest; a bare `passed` value is rejected.

The script cannot manufacture this evidence. It only prevents an unchecked
manifest from enabling a download.

The observable procedure and evidence-file format are in
[`physical-validation.md`](physical-validation.md).

## Artifact separation

The device updater receives only the application image. A full USB rescue
image, if ever produced after separate review, must be labelled as rescue-only
and must never be offered through the on-device updater or website flasher.

## Current state

The current manifest remains blocked. The checked-in build is useful for
simulator and physical validation only; it is not a release or installation
instruction.
