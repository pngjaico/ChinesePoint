import importlib.util
import tempfile
import unittest
from pathlib import Path


SCRIPT = Path(__file__).parents[2] / "tools" / "chinesepoint" / "verify_artifact.py"
SPEC = importlib.util.spec_from_file_location("verify_artifact", SCRIPT)
VERIFY = importlib.util.module_from_spec(SPEC)
assert SPEC.loader is not None
SPEC.loader.exec_module(VERIFY)


def x4pro_image(version: str = "ChinesePoint-v0.6-x4pro") -> bytes:
    image = bytearray(64)
    image[0] = VERIFY.ESP_IMAGE_MAGIC
    image[12] = VERIFY.ESP32_S3_CHIP_ID
    image.extend(VERIFY.X4PRO_BOARD_TAG)
    image.extend(version.encode("utf-8"))
    return bytes(image)


class VerifyArtifactTest(unittest.TestCase):
    def write_image(self, payload: bytes) -> Path:
        path = Path(self.tempdir.name) / "firmware.bin"
        path.write_bytes(payload)
        return path

    def setUp(self):
        self.tempdir = tempfile.TemporaryDirectory()

    def tearDown(self):
        self.tempdir.cleanup()

    def test_accepts_a_tightly_matched_x4pro_application(self):
        manifest = VERIFY.verify_application(
            self.write_image(x4pro_image()), "chinesepoint_x4pro", "ChinesePoint-v0.6-x4pro"
        )
        self.assertEqual(manifest["target"], "xteink-x4-pro")
        self.assertFalse(manifest["installable"])

    def test_rejects_another_platformio_environment(self):
        with self.assertRaisesRegex(VERIFY.ArtifactError, "chinesepoint_x4pro"):
            VERIFY.verify_application(self.write_image(x4pro_image()), "x4pro", "ChinesePoint-v0.6-x4pro")

    def test_rejects_a_missing_x4pro_tag(self):
        with self.assertRaisesRegex(VERIFY.ArtifactError, "board tag"):
            VERIFY.verify_application(self.write_image(x4pro_image().replace(VERIFY.X4PRO_BOARD_TAG, b"")),
                                      "chinesepoint_x4pro", "ChinesePoint-v0.6-x4pro")

    def test_rejects_an_oversized_full_image(self):
        image = x4pro_image() + b"\0" * VERIFY.X4PRO_APPLICATION_MAX_BYTES
        with self.assertRaisesRegex(VERIFY.ArtifactError, "full/oversized"):
            VERIFY.verify_application(self.write_image(image), "chinesepoint_x4pro", "ChinesePoint-v0.6-x4pro")


if __name__ == "__main__":
    unittest.main()
