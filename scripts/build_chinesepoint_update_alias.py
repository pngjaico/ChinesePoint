"""Create the X4 Pro SD-update alias after the application image is built.

The ChinesePoint updater verifies image contents rather than trusting a file
name. `update.bin` is therefore an exact, convenient alias for a verified
`firmware.bin`, never a merged USB-rescue image.
"""

from __future__ import annotations

import hashlib
import shutil
from pathlib import Path


def copy_update_alias(firmware: Path, update_alias: Path) -> str:
    """Copy a completed application image and return its SHA-256.

    Read both files back so a post-build filesystem failure cannot silently
    leave a partial `update.bin` beside an otherwise good firmware image.
    """

    shutil.copyfile(firmware, update_alias)
    source_hash = hashlib.sha256(firmware.read_bytes()).hexdigest()
    alias_hash = hashlib.sha256(update_alias.read_bytes()).hexdigest()
    if source_hash != alias_hash:
        raise RuntimeError("update.bin does not match firmware.bin")
    return source_hash


try:
    Import("env")  # type: ignore[name-defined]  # PlatformIO/SCons builtin
except NameError:
    env = None


if env is not None and env["PIOENV"] == "chinesepoint_x4pro":
    # SCons passes target, source, env by keyword. The target of this post
    # action is firmware.bin; its source is the linked firmware.elf.
    def create_alias(target, source, env):
        del source
        firmware = Path(str(target[0]))
        alias = Path(env.subst("$BUILD_DIR")) / "update.bin"
        digest = copy_update_alias(firmware, alias)
        print(f"ChinesePoint X4 Pro update alias: {alias} sha256={digest}")


    env.AddPostAction("$BUILD_DIR/firmware.bin", create_alias)
