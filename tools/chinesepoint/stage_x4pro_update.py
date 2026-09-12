#!/usr/bin/env python3
"""Stage a verified ChinesePoint X4 Pro application as an SD-card update.bin.

This tool copies only an application image to an explicitly selected SD-card
root. It never talks to a USB serial port, never flashes a device, and refuses
to overwrite an existing update unless --force is supplied.
"""

from __future__ import annotations

import argparse
import hashlib
import os
import shutil
import sys
from pathlib import Path

# Direct execution already puts this directory on sys.path. Insert it explicitly
# as well so the script remains importable by the repository's isolated tests.
SCRIPT_DIR = Path(__file__).resolve().parent
if str(SCRIPT_DIR) not in sys.path:
    sys.path.insert(0, str(SCRIPT_DIR))

from verify_artifact import ArtifactError, verify_application


ENVIRONMENT = "chinesepoint_x4pro"
VERSION = "ChinesePoint-v0.6-x4pro"
UPDATE_NAME = "update.bin"
DIGEST_NAME = "update.bin.sha256"


def stage_update(firmware: Path, sd_root: Path, force: bool = False) -> dict[str, object]:
    """Validate and atomically stage firmware plus a lowercase SHA-256 file."""

    source = firmware.resolve(strict=True)
    root = sd_root.resolve(strict=True)
    if not root.is_dir():
        raise NotADirectoryError(root)
    manifest = verify_application(source, ENVIRONMENT, VERSION)
    target = root / UPDATE_NAME
    digest = root / DIGEST_NAME
    if (target.exists() or digest.exists()) and not force:
        raise FileExistsError(f"refusing to overwrite existing {UPDATE_NAME} or {DIGEST_NAME}; use --force after inspection")

    temporary = root / ".chinesepoint-update.bin.tmp"
    if temporary.exists():
        raise FileExistsError(f"remove stale staging file after inspection: {temporary}")
    try:
        with source.open("rb") as input_file, temporary.open("xb") as output_file:
            shutil.copyfileobj(input_file, output_file, length=1024 * 1024)
            output_file.flush()
            os.fsync(output_file.fileno())
        copied = verify_application(temporary, ENVIRONMENT, VERSION)
        if copied["sha256"] != manifest["sha256"] or copied["bytes"] != manifest["bytes"]:
            raise ArtifactError("staged update does not match the validated source image")
        os.replace(temporary, target)
        digest.write_text(str(manifest["sha256"]) + "\n", encoding="ascii", newline="\n")
    finally:
        if temporary.exists():
            temporary.unlink()
    return {**manifest, "filename": UPDATE_NAME, "digest_filename": DIGEST_NAME, "sd_root": str(root)}


def main() -> int:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--firmware", type=Path, required=True)
    parser.add_argument("--sd-root", type=Path, required=True)
    parser.add_argument("--force", action="store_true")
    args = parser.parse_args()
    try:
        staged = stage_update(args.firmware, args.sd_root, args.force)
    except (ArtifactError, OSError, ValueError) as error:
        parser.error(str(error))
    print("staged X4 Pro SD update: " + str(staged))
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
