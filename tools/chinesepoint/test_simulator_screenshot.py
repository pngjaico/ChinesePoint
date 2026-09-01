"""Tests for the dependency-free simulator screenshot verifier."""

from __future__ import annotations

import importlib.util
import struct
import tempfile
import unittest
from pathlib import Path


MODULE_PATH = Path(__file__).with_name("verify_simulator_screenshot.py")
SPEC = importlib.util.spec_from_file_location("verify_simulator_screenshot", MODULE_PATH)
assert SPEC and SPEC.loader
MODULE = importlib.util.module_from_spec(SPEC)
SPEC.loader.exec_module(MODULE)


def make_bmp(path: Path, pixels: list[bytes]) -> None:
    width = height = 2
    payload = b"".join(pixels)
    offset = 54
    path.write_bytes(
        struct.pack("<2sIHHI", b"BM", offset + len(payload), 0, 0, offset)
        + struct.pack("<IiiHHIIiiII", 40, width, height, 1, 32, 0, len(payload), 0, 0, 0, 0)
        + payload
    )


class SimulatorScreenshotTest(unittest.TestCase):
    def test_nonuniform_frame_is_accepted(self) -> None:
        with tempfile.TemporaryDirectory() as directory:
            path = Path(directory) / "frame.bmp"
            make_bmp(path, [b"\xff\xff\xff\xff", b"\x00\x00\x00\xff"] * 2)
            self.assertEqual(MODULE.inspect_bmp(path, 2, 2)["foreground_pixels"], 2)

    def test_uniform_frame_is_rejected(self) -> None:
        with tempfile.TemporaryDirectory() as directory:
            path = Path(directory) / "frame.bmp"
            make_bmp(path, [b"\xff\xff\xff\xff"] * 4)
            with self.assertRaises(MODULE.ScreenshotError):
                MODULE.inspect_bmp(path, 2, 2)


if __name__ == "__main__":
    unittest.main()
