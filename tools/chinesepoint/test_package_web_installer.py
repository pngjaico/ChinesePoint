"""Tests for the approved-release ESP Web Tools manifest builder."""

from __future__ import annotations

import importlib.util
import unittest
from pathlib import Path


ROOT = Path(__file__).resolve().parents[2]
MODULE_PATH = ROOT / "tools" / "chinesepoint" / "package_web_installer.py"
SPEC = importlib.util.spec_from_file_location("package_web_installer", MODULE_PATH)
assert SPEC and SPEC.loader
MODULE = importlib.util.module_from_spec(SPEC)
SPEC.loader.exec_module(MODULE)


class WebInstallerPackageTest(unittest.TestCase):
    def test_manifest_is_x4pro_s3_single_merged_image(self) -> None:
        manifest = MODULE.web_manifest("ChinesePoint-v1.0-x4pro", "firmware/candidate.bin")
        self.assertEqual(manifest["name"], "ChinesePoint X4 Pro")
        self.assertTrue(manifest["new_install_prompt_erase"])
        self.assertEqual(manifest["builds"], [{
            "chipFamily": "ESP32-S3",
            "parts": [{"path": "firmware/candidate.bin", "offset": 0}],
        }])

    def test_release_filename_cannot_escape_firmware_directory(self) -> None:
        self.assertEqual(MODULE.safe_release_filename("ChinesePoint-v1.0/x4pro"), "ChinesePoint-v1.0-x4pro-web-install.bin")


if __name__ == "__main__":
    unittest.main()
