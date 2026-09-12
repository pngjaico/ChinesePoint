#!/usr/bin/env python3
"""Reject ChinesePoint symbols that have entered the saturated X4 Pro IRAM.

The USB-MSC X4 Pro SDK prebuild currently consumes the whole dedicated IRAM
region. This guard cannot manufacture headroom, but it prevents a learner
feature from silently making the fixed platform constraint worse.
"""

from __future__ import annotations

import argparse
import shutil
import subprocess
import sys
from pathlib import Path


FORBIDDEN_MARKERS = ("Cjk", "CJK", "ChinesePoint")


def forbidden_iram_symbols(symbol_table: str) -> list[str]:
    """Return project symbols placed in an IRAM text/vector section."""
    matches: list[str] = []
    for line in symbol_table.splitlines():
        if ".iram0.text" not in line and ".iram0.vectors" not in line:
            continue
        if any(marker in line for marker in FORBIDDEN_MARKERS):
            matches.append(line.strip())
    return matches


def main() -> int:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--elf", type=Path, required=True)
    parser.add_argument("--objdump", help="Xtensa objdump executable; defaults to PATH")
    args = parser.parse_args()
    if not args.elf.is_file():
        parser.error(f"ELF does not exist: {args.elf}")
    objdump = args.objdump or shutil.which("xtensa-esp32s3-elf-objdump")
    if not objdump:
        parser.error("xtensa-esp32s3-elf-objdump is not on PATH; pass --objdump")
    result = subprocess.run([objdump, "-t", str(args.elf)], check=False, capture_output=True, text=True)
    if result.returncode:
        sys.stderr.write(result.stderr)
        return result.returncode
    forbidden = forbidden_iram_symbols(result.stdout)
    if forbidden:
        sys.stderr.write("ChinesePoint symbols in saturated IRAM:\n" + "\n".join(forbidden) + "\n")
        return 1
    print("IRAM attribution check passed: no ChinesePoint symbols found")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
