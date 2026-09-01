#!/usr/bin/env python3
"""Create a deterministic, non-flashing ChinesePoint simulator evidence bundle."""

from __future__ import annotations

import argparse
import hashlib
import json
import re
import zipfile
from pathlib import Path


PANELS = ("ssd1677", "uc8179", "uc8279")
REPOSITORY = "https://github.com/pngjaico/ChinesePoint.git"
COMMIT_RE = re.compile(r"^[0-9a-f]{40}$")


def sha256(path: Path) -> str:
    return hashlib.sha256(path.read_bytes()).hexdigest()


def runner_shell(source_commit: str) -> str:
    return f'''#!/usr/bin/env bash
set -euo pipefail
panel="${{1:-ssd1677}}"
case "$panel" in
  ssd1677|uc8179|uc8279) ;;
  *) echo "panel must be ssd1677, uc8179 or uc8279" >&2; exit 2 ;;
esac
workdir="${{CHINESEPOINT_SIM_WORKDIR:-$PWD/ChinesePoint-simulator}}"
git clone --recurse-submodules {REPOSITORY} "$workdir"
git -C "$workdir" checkout --detach {source_commit}
cd "$workdir"
python3 -m pip install --user https://github.com/pioarduino/platformio-core/archive/refs/tags/v6.1.19.zip
python3 -m platformio run -e "chinesepoint_simulator_x4pro_${{panel}}"
echo "Simulator built. This bundle never flashes firmware."
'''


def runner_powershell(source_commit: str) -> str:
    return f'''param(
  [ValidateSet("ssd1677", "uc8179", "uc8279")]
  [string]$Panel = "ssd1677",
  [string]$Workdir = (Join-Path $PWD "ChinesePoint-simulator")
)
$ErrorActionPreference = "Stop"
git clone --recurse-submodules {REPOSITORY} $Workdir
git -C $Workdir checkout --detach {source_commit}
Push-Location $Workdir
try {{
  python -m pip install --user https://github.com/pioarduino/platformio-core/archive/refs/tags/v6.1.19.zip
  python -m platformio run -e "chinesepoint_simulator_x4pro_$Panel"
  Write-Host "Simulator built. This bundle never flashes firmware."
}} finally {{
  Pop-Location
}}
'''


def write_entry(archive: zipfile.ZipFile, name: str, content: bytes, executable: bool = False) -> None:
    info = zipfile.ZipInfo(name, date_time=(1980, 1, 1, 0, 0, 0))
    info.compress_type = zipfile.ZIP_DEFLATED
    info.external_attr = ((0o755 if executable else 0o644) & 0xFFFF) << 16
    archive.writestr(info, content)


def create_bundle(source_commit: str, captures_dir: Path, output: Path) -> None:
    if not COMMIT_RE.fullmatch(source_commit):
        raise ValueError("source commit must be a 40-character lowercase SHA-1")

    screenshots: dict[str, Path] = {}
    for panel in PANELS:
        screenshot = captures_dir / panel / "boot.bmp"
        if not screenshot.is_file():
            raise FileNotFoundError(f"missing simulator capture: {screenshot}")
        screenshots[panel] = screenshot

    manifest = {
        "schema": 1,
        "project": "ChinesePoint",
        "source_commit": source_commit,
        "panels": list(PANELS),
        "flashing": "not-supported",
        "screenshots": {panel: {"path": f"screenshots/{panel}/boot.bmp", "sha256": sha256(path)}
                        for panel, path in screenshots.items()},
    }
    output.parent.mkdir(parents=True, exist_ok=True)
    with zipfile.ZipFile(output, "w") as archive:
        write_entry(archive, "manifest.json", (json.dumps(manifest, indent=2, sort_keys=True) + "\n").encode())
        write_entry(archive, "README.md", (
            "# ChinesePoint simulator evidence\n\n"
            "This is a Linux/WSL re-execution bundle for a pinned source commit. "
            "It contains captured panel evidence and has no flashing command.\n"
        ).encode())
        write_entry(archive, "run-simulator.sh", runner_shell(source_commit).encode(), executable=True)
        write_entry(archive, "run-simulator.ps1", runner_powershell(source_commit).encode())
        for panel, screenshot in screenshots.items():
            write_entry(archive, f"screenshots/{panel}/boot.bmp", screenshot.read_bytes())


def main() -> int:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--source-commit", required=True)
    parser.add_argument("--captures-dir", type=Path, required=True)
    parser.add_argument("--output", type=Path, required=True)
    args = parser.parse_args()
    try:
        create_bundle(args.source_commit, args.captures_dir, args.output)
    except (OSError, ValueError) as error:
        parser.error(str(error))
    print(f"simulator bundle: {args.output}")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
