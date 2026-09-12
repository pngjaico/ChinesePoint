import unittest
from pathlib import Path


MAIN = Path(__file__).parents[2] / "src" / "main.cpp"


class BootSafetyOrderTest(unittest.TestCase):
    @classmethod
    def setUpClass(cls):
        cls.source = MAIN.read_text(encoding="utf-8")

    def test_recovery_picker_precedes_normal_persistent_boot(self):
        marker = "  if (recoveryFirmwareMode) {\n    // This is the last possible point"
        start = self.source.index(marker)
        end = self.source.index("\n  APP_STATE.loadFromFile();", start)
        route = self.source[start:end]

        self.assertLess(self.source.index("  HalSystem::checkPanic();"), start)
        self.assertIn("ButtonNavigator::setMappedInputManager(mappedInputManager);", route)
        self.assertIn("setupDisplayAndFonts(/*seamless=*/false);", route)
        self.assertIn("SdFirmwareUpdateActivity", route)
        self.assertIn("    return;", route)

        for ordinary_boot_operation in (
            "APP_STATE.loadFromFile();",
            "SETTINGS.loadFromFile();",
            "RECENT_BOOKS.loadFromFile();",
            "KOREADER_STORE.loadFromFile();",
            "OPDS_STORE.loadFromFile();",
            "Frontlight.begin(",
        ):
            self.assertGreater(self.source.index(ordinary_boot_operation), end)

    def test_x4pro_psram_fault_is_visible_before_normal_boot(self):
        recovery_end = self.source.index("\n  APP_STATE.loadFromFile();")
        psram_gate = self.source.index("if (!hasUsableX4ProPsram())")
        self.assertLess(psram_gate, recovery_end)
        self.assertIn("X4PRO_MINIMUM_PSRAM_BYTES = 6U * 1024U * 1024U", self.source)
        self.assertIn("PSRAM unavailable. Hold DOWN + POWER for recovery.", self.source)
        self.assertIn("setupDisplayAndFonts(/*seamless=*/false);", self.source[psram_gate:recovery_end])

    def test_automatic_restore_requires_a_deliberate_hold_and_fixed_backup_contract(self):
        updater = (MAIN.parent / "activities" / "settings" / "SdFirmwareUpdateActivity.cpp").read_text(encoding="utf-8")
        self.assertIn("X4PRO_AUTORESTORE_HOLD_MS = 2500", self.source)
        self.assertIn("isX4ProAutomaticRestoreGesture(recoveryFirmwareMode)", self.source)
        self.assertIn('RECOVERY_BACKUP_PATH = "/backup/crosspoint-x4pro.bin"', updater)
        self.assertIn('RECOVERY_BACKUP_SHA256_PATH = "/backup/crosspoint-x4pro.bin.sha256"', updater)
        self.assertIn("validateFirmware() || !validateBackupChecksum()", updater)
        self.assertIn("automatic recovery backup changed before flash", updater)

    def test_controller_resolution_precedes_display_initialization(self):
        setup_start = self.source.index("void setupDisplayAndFonts(bool seamless = false)")
        setup_end = self.source.index("\nvoid setup()", setup_start)
        setup = self.source[setup_start:setup_end]
        self.assertLess(setup.index("freeink::applyXteinkDisplayController()"), setup.index("display.begin(seamless);"))


if __name__ == "__main__":
    unittest.main()
