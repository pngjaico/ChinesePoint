#!/usr/bin/env python3
"""Reject blank or malformed X4 Pro simulator screenshots without Pillow."""

from __future__ import annotations

import argparse
import struct
from pathlib import Path


class ScreenshotError(ValueError):
    """The screenshot cannot demonstrate a meaningful simulator boot frame."""


def inspect_bmp(path: Path, expected_width: int = 480, expected_height: int = 800) -> dict[str, int]:
    data = path.read_bytes()
    if len(data) < 54 or data[:2] != b"BM":
        raise ScreenshotError("not a BMP file")
    offset = struct.unpack_from("<I", data, 10)[0]
    dib_size = struct.unpack_from("<I", data, 14)[0]
    if dib_size < 40:
        raise ScreenshotError("unsupported BMP header")
    width, height = struct.unpack_from("<ii", data, 18)
    planes, bits_per_pixel = struct.unpack_from("<HH", data, 26)
    absolute_height = abs(height)
    if (width, absolute_height) != (expected_width, expected_height):
        raise ScreenshotError(f"unexpected screenshot size {width}x{absolute_height}")
    if planes != 1 or bits_per_pixel != 32:
        raise ScreenshotError("expected a 32-bit BMP screenshot")
    pixel_bytes = width * absolute_height * 4
    if offset > len(data) or len(data) - offset < pixel_bytes:
        raise ScreenshotError("truncated BMP pixel data")

    colors: dict[bytes, int] = {}
    for index in range(offset, offset + pixel_bytes, 4):
        rgb = data[index:index + 3]
        colors[rgb] = colors.get(rgb, 0) + 1
    if len(colors) < 2:
        raise ScreenshotError("uniform screenshot: boot content is absent")
    total = width * absolute_height
    foreground = total - max(colors.values())
    if foreground < max(1, total // 1000):
        raise ScreenshotError("near-uniform screenshot: boot content is insufficient")
    return {"width": width, "height": absolute_height, "colors": len(colors), "foreground_pixels": foreground}


def main() -> int:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--screenshot", type=Path, required=True)
    args = parser.parse_args()
    try:
        details = inspect_bmp(args.screenshot)
    except (OSError, ScreenshotError) as error:
        parser.error(str(error))
    print("simulator screenshot: valid " + ", ".join(f"{key}={value}" for key, value in details.items()))
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
