#!/usr/bin/env python3
"""Refuse incorrectly-targeted ChinesePoint application artifacts.

This is a release gate, not a flasher. It validates simple, durable facts from
the app image and writes a manifest that the site/simulator release flow can
consume. Espressif's full image validation remains a separate build command.
"""

from __future__ import annotations

import argparse
import hashlib
import json
from pathlib import Path


ESP_IMAGE_MAGIC = 0xE9
ESP32_S3_CHIP_ID = 9
X4PRO_BOARD_TAG = b"CROSSPOINT-BOARD-V1:x4pro;"
X4PRO_APPLICATION_MAX_BYTES = 0x640000


class ArtifactError(ValueError):
    """The file cannot be presented as a ChinesePoint X4 Pro app image."""


def verify_application(path: Path, environment: str, expected_version: str) -> dict[str, object]:
    if environment != "chinesepoint_x4pro":
        raise ArtifactError("expected PlatformIO environment 'chinesepoint_x4pro'")

    payload = path.read_bytes()
    if not payload or payload[0] != ESP_IMAGE_MAGIC:
        raise ArtifactError("not an ESP application image (missing ESP image magic)")
    if len(payload) <= 12 or payload[12] != ESP32_S3_CHIP_ID:
        raise ArtifactError("application image is not tagged for ESP32-S3")
    if len(payload) > X4PRO_APPLICATION_MAX_BYTES:
        raise ArtifactError("refusing a full/oversized image; provide firmware.bin only")
    if payload.count(X4PRO_BOARD_TAG) != 1:
        raise ArtifactError("expected exactly one X4 Pro board tag")
    if expected_version.encode("utf-8") not in payload:
        raise ArtifactError(f"expected version marker not found: {expected_version}")

    return {
        "schema": 1,
        "kind": "application",
        "environment": environment,
        "target": "xteink-x4-pro",
        "board_tag": X4PRO_BOARD_TAG.decode("ascii"),
        "filename": path.name,
        "bytes": len(payload),
        "sha256": hashlib.sha256(payload).hexdigest(),
        "version": expected_version,
        "installable": False,
        "installable_reason": "Hardware validation and release sign-off are still required.",
    }


def main() -> int:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--firmware", type=Path, required=True)
    parser.add_argument("--environment", required=True)
    parser.add_argument("--expected-version", required=True)
    parser.add_argument("--manifest", type=Path, required=True)
    args = parser.parse_args()

    try:
        manifest = verify_application(args.firmware, args.environment, args.expected_version)
    except (ArtifactError, OSError) as error:
        parser.error(str(error))

    args.manifest.parent.mkdir(parents=True, exist_ok=True)
    args.manifest.write_text(json.dumps(manifest, indent=2) + "\n", encoding="utf-8")
    print(json.dumps(manifest, indent=2))
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
