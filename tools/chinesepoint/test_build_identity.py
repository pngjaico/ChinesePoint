"""Regression checks for deterministic ChinesePoint firmware identity."""

from pathlib import Path
import unittest


PROJECT_ROOT = Path(__file__).resolve().parents[2]
UPDATER_SOURCE = PROJECT_ROOT / "src" / "activities" / "settings" / "SdFirmwareUpdateActivity.cpp"


class BuildIdentityTest(unittest.TestCase):
    def test_sd_updater_uses_stable_firmware_identity(self) -> None:
        """A rebuild must not embed the compiler clock in the X4 Pro image."""
        source = UPDATER_SOURCE.read_text(encoding="utf-8")

        self.assertNotIn("__DATE__", source)
        self.assertNotIn("__TIME__", source)
        self.assertIn("CROSSPOINT_VERSION", source)


if __name__ == "__main__":
    unittest.main()
