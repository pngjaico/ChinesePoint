"""Produce a reproducible X4 Pro application image from its linked ELF.

Espressif's ``--elf-sha256-offset`` records a hash of the complete ELF inside
the app descriptor. A normal PlatformIO ELF includes DWARF paths, so two clean
worktrees can have identical loadable segments but different application image
hashes. Strip non-loadable debug and symbol data first, then regenerate the
same ESP32-S3 application image. This does not alter the linked ELF used for
size analysis or the executable segments supplied to esptool.
"""

from __future__ import annotations

import os
from pathlib import Path
import subprocess


def executable(package_dir: Path, stem: str) -> Path:
    """Return a PlatformIO package executable, including the Windows suffix."""

    name = stem + (".exe" if os.name == "nt" else "")
    path = package_dir / name
    if not path.is_file():
        raise RuntimeError(f"required build tool is missing: {path}")
    return path


def existing_file(path: Path, label: str) -> Path:
    """Reject a PlatformIO tool variable that does not name an executable."""

    if not path.is_file():
        raise RuntimeError(f"required {label} is missing: {path}")
    return path


def normalize_image(
    *, elf: Path, firmware: Path, objcopy: Path, esptool: Path, flash_mode: str, flash_freq: str, flash_size: str
) -> None:
    """Atomically replace ``firmware`` with its canonical ESP32-S3 image."""

    if not elf.is_file():
        raise RuntimeError(f"linked ELF is missing: {elf}")

    stripped = firmware.with_suffix(".stripped.elf")
    replacement = firmware.with_suffix(".normalized.bin")
    try:
        subprocess.run([str(objcopy), "--strip-all", str(elf), str(stripped)], check=True)
        subprocess.run(
            [
                str(esptool),
                "--chip",
                "esp32s3",
                "elf2image",
                "--flash-mode",
                flash_mode,
                "--flash-freq",
                flash_freq,
                "--flash-size",
                flash_size,
                "--elf-sha256-offset",
                "0xb0",
                "-o",
                str(replacement),
                str(stripped),
            ],
            check=True,
        )
        if not replacement.is_file() or replacement.stat().st_size == 0:
            raise RuntimeError("esptool did not create a normalized application image")
        replacement.replace(firmware)
    finally:
        stripped.unlink(missing_ok=True)
        replacement.unlink(missing_ok=True)


try:
    Import("env")  # type: ignore[name-defined]  # PlatformIO/SCons builtin
except NameError:
    env = None


if env is not None and env["PIOENV"] == "chinesepoint_x4pro":
    def canonicalize_firmware(target, source, env):
        firmware = Path(str(target[0]))
        elf = Path(str(source[0]))
        platform = env.PioPlatform()
        toolchain = Path(platform.get_package_dir("toolchain-xtensa-esp-elf")) / "bin"
        normalize_image(
            elf=elf,
            firmware=firmware,
            objcopy=executable(toolchain, "xtensa-esp32s3-elf-objcopy"),
            # PlatformIO resolves this to the penv executable; the package
            # itself only carries the Python module and is not directly
            # executable on every host.
            esptool=existing_file(Path(env.subst("$ERASETOOL")), "esptool"),
            flash_mode="dio",
            flash_freq="80m",
            flash_size="16MB",
        )
        print(f"ChinesePoint X4 Pro normalized application image: {firmware}")


    # Register before the update alias action: update.bin must copy the
    # normalized application, never PlatformIO's debug-path-dependent image.
    env.AddPostAction("$BUILD_DIR/firmware.bin", canonicalize_firmware)
