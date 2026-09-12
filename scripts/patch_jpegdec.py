"""
PlatformIO pre-build script: apply CrossPoint's JPEGDEC patches via `git apply`.

The upstream JPEGDEC pin still has the wild-pointer + DC-write bugs in
JPEGDecodeMCU_P that surface when EIGHT_BIT_GRAYSCALE decodes a 3-component
progressive JPEG (each Y MCU drags two MCU_SKIP calls behind it for Cb/Cr).
The patches in `scripts/jpegdec_patches/` carry the fix; this script applies
each one against the libdep working tree.

The patch series is checked as a series, not one patch at a time. Later
patches deliberately touch lines changed by earlier ones, so testing an early
patch in reverse after its successor is already present gives a false
"does not apply" result. Git checks the complete series in forward order or
the complete reverse series in reverse order:
  * reverse series succeeds -> all patches already applied, skip
  * forward series succeeds -> apply all patches
  * neither succeeds        -> abort the build

Patches live in `scripts/jpegdec_patches/` as one-commit-per-fix files
(see the file headers for context). Applied in lexical order.
"""

Import("env")  # noqa: F821 (SCons-injected global)
import os
import subprocess
import sys


PATCH_DIR = os.path.join(env["PROJECT_DIR"], "scripts", "jpegdec_patches")  # noqa: F821


def patch_jpegdec(env):
    libdeps_dir = os.path.join(env["PROJECT_DIR"], ".pio", "libdeps")
    if not os.path.isdir(libdeps_dir):
        return
    patches = _patch_files()
    for env_dir in os.listdir(libdeps_dir):
        jpeg_dir = os.path.join(libdeps_dir, env_dir, "JPEGDEC")
        if not os.path.isdir(os.path.join(jpeg_dir, ".git")):
            continue
        _apply_series(jpeg_dir, patches)


def _patch_files():
    if not os.path.isdir(PATCH_DIR):
        raise RuntimeError(
            "JPEGDEC patches missing -- aborting build (expected directory %s)"
            % PATCH_DIR
        )
    patches = sorted(
        os.path.join(PATCH_DIR, name)
        for name in os.listdir(PATCH_DIR)
        if name.endswith(".patch")
    )
    if not patches:
        raise RuntimeError(
            "JPEGDEC patches missing -- aborting build (no .patch files in %s)"
            % PATCH_DIR
        )
    return patches


def _apply_series(jpeg_dir, patches):
    if _git_apply_succeeds(jpeg_dir, patches, reverse=True):
        print("JPEGDEC patches already applied")
        return
    if not _git_apply_succeeds(jpeg_dir, patches, reverse=False):
        # The source has diverged or contains only part of the series. Do not
        # write a half-patched library dependency.
        result = subprocess.run(
            ["git", "apply", "--check", *patches],
            cwd=jpeg_dir,
            capture_output=True,
            text=True,
        )
        sys.stderr.write(
            "ERROR: JPEGDEC patch series does not apply cleanly:\n%s%s\n"
            % (result.stdout, result.stderr)
        )
        raise SystemExit(1)
    subprocess.run(["git", "apply", *patches], cwd=jpeg_dir, check=True)
    for patch_path in patches:
        print("Applied JPEGDEC patch: %s" % os.path.basename(patch_path))


def _git_apply_succeeds(jpeg_dir, patches, *, reverse):
    cmd = ["git", "apply", "--check"]
    if reverse:
        cmd.append("--reverse")
    cmd.extend(reversed(patches) if reverse else patches)
    return subprocess.run(
        cmd, cwd=jpeg_dir, capture_output=True, text=True
    ).returncode == 0


patch_jpegdec(env)  # noqa: F821
