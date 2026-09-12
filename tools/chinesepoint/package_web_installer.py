#!/usr/bin/env python3
"""Create the immutable ESP Web Tools bundle for an approved X4 Pro release.

This tool deliberately refuses blocked releases. It merges the X4 Pro bootloader,
partition table, OTA selector and application image at their explicit offsets so
ESP Web Tools can flash it from an HTTPS page. The generated bundle is a release
artifact, never an input to the on-device SD updater.
"""

from __future__ import annotations

import argparse
import hashlib
import importlib.util
import json
import re
import subprocess
import sys
from pathlib import Path
from typing import Any

ROOT = Path(__file__).resolve().parents[2]
VERIFY_PATH = ROOT / "tools" / "chinesepoint" / "verify_release_manifest.py"
SPEC = importlib.util.spec_from_file_location("verify_release_manifest", VERIFY_PATH)
assert SPEC and SPEC.loader
VERIFY = importlib.util.module_from_spec(SPEC)
SPEC.loader.exec_module(VERIFY)


def sha256(path: Path) -> str:
    digest = hashlib.sha256()
    with path.open("rb") as stream:
        for block in iter(lambda: stream.read(1024 * 1024), b""):
            digest.update(block)
    return digest.hexdigest()


def safe_release_filename(version: str) -> str:
    safe = re.sub(r"[^A-Za-z0-9._-]+", "-", version).strip(".-")
    if not safe:
        raise ValueError("release version cannot produce a file name")
    return f"{safe}-web-install.bin"


def web_manifest(version: str, firmware_path: str) -> dict[str, Any]:
    return {
        "name": "ChinesePoint X4 Pro",
        "version": version,
        "new_install_prompt_erase": True,
        "new_install_improv_wait_time": 0,
        "builds": [{
            "chipFamily": "ESP32-S3",
            "parts": [{"path": firmware_path, "offset": 0}],
        }],
    }


def main() -> int:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--release-manifest", type=Path, required=True)
    parser.add_argument("--firmware", type=Path, required=True)
    parser.add_argument("--bootloader", type=Path, required=True)
    parser.add_argument("--partitions", type=Path, required=True)
    parser.add_argument("--boot-app0", type=Path, required=True)
    parser.add_argument("--output-dir", type=Path, required=True)
    parser.add_argument("--esptool", default="esptool")
    args = parser.parse_args()

    release = json.loads(args.release_manifest.read_text(encoding="utf-8"))
    try:
        VERIFY.validate(release, require_installable=True)
    except VERIFY.ReleaseEvidenceError as error:
        parser.error(str(error))
    web_install = release["web_install"]
    if web_install["state"] != "ready":
        parser.error("approved release requires web_install.state=ready")

    inputs = (args.firmware, args.bootloader, args.partitions, args.boot_app0)
    for source in inputs:
        if not source.is_file():
            parser.error(f"missing web-install input: {source}")
    if sha256(args.firmware) != release["artifact"]["sha256"]:
        parser.error("firmware SHA-256 does not match the approved release manifest")

    firmware_dir = args.output_dir / "firmware"
    firmware_dir.mkdir(parents=True, exist_ok=True)
    output = firmware_dir / safe_release_filename(release["version"])
    esptool_command = [str(args.esptool)]
    if str(args.esptool).endswith(".py"):
        esptool_command.insert(0, sys.executable)
    command = esptool_command + [
        "--chip", "esp32s3", "merge_bin",
        "-o", str(output),
        "--flash_mode", "dio", "--flash_freq", "80m", "--flash_size", "16MB",
        "0x1000", str(args.bootloader),
        "0x8000", str(args.partitions),
        "0xe000", str(args.boot_app0),
        "0x10000", str(args.firmware),
    ]
    subprocess.run(command, check=True)
    if not output.is_file() or output.stat().st_size == 0:
        parser.error("esptool did not produce a merged web-install image")

    manifest_path = args.output_dir / "web-install-manifest.json"
    manifest_path.write_text(
        json.dumps(web_manifest(release["version"], f"firmware/{output.name}"), indent=2) + "\n",
        encoding="utf-8",
    )
    print(f"web-install bundle: {output}")
    print(f"web-install manifest: {manifest_path}")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
