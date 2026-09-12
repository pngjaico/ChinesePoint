"""Tests for safe `update.bin` staging."""

from __future__ import annotations

import importlib.util
import tempfile
import unittest
from pathlib import Path


ROOT = Path(__file__).resolve().parents[2]
TOOLS = ROOT / "tools" / "chinesepoint"
VERIFY_SPEC = importlib.util.spec_from_file_location("verify_artifact", TOOLS / "verify_artifact.py")
assert VERIFY_SPEC and VERIFY_SPEC.loader
VERIFY = importlib.util.module_from_spec(VERIFY_SPEC)
VERIFY_SPEC.loader.exec_module(VERIFY)

STAGE_SPEC = importlib.util.spec_from_file_location("stage_x4pro_update", TOOLS / "stage_x4pro_update.py")
assert STAGE_SPEC and STAGE_SPEC.loader
STAGE = importlib.util.module_from_spec(STAGE_SPEC)
STAGE_SPEC.loader.exec_module(STAGE)

ALIAS_SPEC = importlib.util.spec_from_file_location("build_chinesepoint_update_alias", ROOT / "scripts" / "build_chinesepoint_update_alias.py")
assert ALIAS_SPEC and ALIAS_SPEC.loader
ALIAS = importlib.util.module_from_spec(ALIAS_SPEC)
ALIAS_SPEC.loader.exec_module(ALIAS)


def x4pro_image() -> bytes:
    image = bytearray(64)
    image[0] = VERIFY.ESP_IMAGE_MAGIC
    image[12] = VERIFY.ESP32_S3_CHIP_ID
    image.extend(VERIFY.X4PRO_BOARD_TAG)
    image.extend(b"ChinesePoint-v0.6-x4pro")
    return bytes(image)


class StageUpdateTest(unittest.TestCase):
    def setUp(self) -> None:
        self.tempdir = tempfile.TemporaryDirectory()
        self.root = Path(self.tempdir.name)
        self.firmware = self.root / "firmware.bin"
        self.firmware.write_bytes(x4pro_image())
        self.sd_root = self.root / "sd"
        self.sd_root.mkdir()

    def tearDown(self) -> None:
        self.tempdir.cleanup()

    def test_stages_exact_application_and_digest(self) -> None:
        record = STAGE.stage_update(self.firmware, self.sd_root)
        target = self.sd_root / "update.bin"
        self.assertEqual(target.read_bytes(), self.firmware.read_bytes())
        self.assertEqual((self.sd_root / "update.bin.sha256").read_text(encoding="ascii"), record["sha256"] + "\n")
        self.assertEqual(record["filename"], "update.bin")

    def test_post_build_alias_is_byte_identical(self) -> None:
        alias = self.root / "update.bin"
        self.assertEqual(ALIAS.copy_update_alias(self.firmware, alias), VERIFY.verify_application(
            self.firmware, "chinesepoint_x4pro", "ChinesePoint-v0.6-x4pro")["sha256"])
        self.assertEqual(alias.read_bytes(), self.firmware.read_bytes())

    def test_refuses_overwrite_without_force(self) -> None:
        STAGE.stage_update(self.firmware, self.sd_root)
        with self.assertRaises(FileExistsError):
            STAGE.stage_update(self.firmware, self.sd_root)

    def test_rejects_wrong_target_without_creating_update(self) -> None:
        self.firmware.write_bytes(b"not an esp image")
        with self.assertRaises(STAGE.ArtifactError):
            STAGE.stage_update(self.firmware, self.sd_root)
        self.assertFalse((self.sd_root / "update.bin").exists())


if __name__ == "__main__":
    unittest.main()
