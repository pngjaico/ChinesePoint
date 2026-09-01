"""Build the distributable ChinesePoint Anki add-on without external tooling."""

from __future__ import annotations

import argparse
from pathlib import Path
from zipfile import ZIP_DEFLATED, ZipFile


ROOT = Path(__file__).resolve().parent
ADDON = ROOT / "chinesepoint_anki_bridge"
DEFAULT_OUTPUT = ROOT / "dist" / "chinesepoint-anki-bridge-v0.6.ankiaddon"


def is_package_file(path: Path) -> bool:
    return "__pycache__" not in path.parts and path.suffix != ".pyc"


def build(output: Path) -> list[str]:
    output.parent.mkdir(parents=True, exist_ok=True)
    files = sorted(path for path in ADDON.rglob("*") if path.is_file() and is_package_file(path))
    required = {"__init__.py", "config.json", "manifest.json", "protocol.py", "server.py"}
    present = {path.relative_to(ADDON).as_posix() for path in files}
    missing = required - present
    if missing:
        raise RuntimeError("missing add-on files: " + ", ".join(sorted(missing)))
    with ZipFile(output, "w", compression=ZIP_DEFLATED) as archive:
        for path in files:
            archive.write(path, path.relative_to(ADDON).as_posix())
    return sorted(present)


def main() -> None:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--output", type=Path, default=DEFAULT_OUTPUT)
    args = parser.parse_args()
    files = build(args.output.resolve())
    print(f"built {args.output.resolve()} ({len(files)} files)")


if __name__ == "__main__":
    main()
