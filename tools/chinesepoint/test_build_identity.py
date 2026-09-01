"""Regression checks for deterministic ChinesePoint firmware identity."""

from pathlib import Path
import unittest


PROJECT_ROOT = Path(__file__).resolve().parents[2]
UPDATER_SOURCE = PROJECT_ROOT / "src" / "activities" / "settings" / "SdFirmwareUpdateActivity.cpp"
PLATFORMIO_INI = PROJECT_ROOT / "platformio.ini"


class BuildIdentityTest(unittest.TestCase):
    def test_sd_updater_uses_stable_firmware_identity(self) -> None:
        """A rebuild must not embed the compiler clock in the X4 Pro image."""
        source = UPDATER_SOURCE.read_text(encoding="utf-8")

        self.assertNotIn("__DATE__", source)
        self.assertNotIn("__TIME__", source)
        self.assertIn("CROSSPOINT_VERSION", source)

    def test_x4pro_build_pins_compiler_clock_for_framework_metadata(self) -> None:
        """The ESP-IDF application descriptor must not receive wall-clock values."""
        config = PLATFORMIO_INI.read_text(encoding="utf-8")
        environment = config.split("[env:chinesepoint_x4pro]", 1)[1].split("[chinesepoint_simulator_base]", 1)[0]

        self.assertIn('-D__DATE__=\\"Jan 01 1970\\"', environment)
        self.assertIn('-D__TIME__=\\"00:00:00\\"', environment)


if __name__ == "__main__":
    unittest.main()
