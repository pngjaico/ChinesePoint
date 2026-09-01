"""Tests for the versioned, non-flashing simulator evidence bundle."""

from __future__ import annotations

import importlib.util
import json
import tempfile
import unittest
import zipfile
from pathlib import Path


MODULE_PATH = Path(__file__).with_name("package_simulator_bundle.py")


class SimulatorBundleTest(unittest.TestCase):
    def test_bundle_contains_the_three_panel_captures_and_safe_runner(self) -> None:
        self.assertTrue(MODULE_PATH.exists(), "the simulator bundle packager must exist")
        spec = importlib.util.spec_from_file_location("package_simulator_bundle", MODULE_PATH)
        assert spec and spec.loader
        module = importlib.util.module_from_spec(spec)
        spec.loader.exec_module(module)

        with tempfile.TemporaryDirectory() as directory:
            root = Path(directory)
            captures = root / "captures"
            for panel in module.PANELS:
                target = captures / panel
                target.mkdir(parents=True)
                (target / "boot.bmp").write_bytes(b"BM-evidence")

            bundle = root / "chinesepoint-simulator.zip"
            module.create_bundle("a" * 40, captures, bundle)

            with zipfile.ZipFile(bundle) as archive:
                names = set(archive.namelist())
                self.assertTrue({f"screenshots/{panel}/boot.bmp" for panel in module.PANELS} <= names)
                self.assertIn("manifest.json", names)
                self.assertIn("run-simulator.sh", names)
                self.assertIn("run-simulator.ps1", names)
                self.assertNotIn("flash-firmware.sh", names)
                self.assertNotIn("flash-firmware.ps1", names)
                manifest = json.loads(archive.read("manifest.json"))
                self.assertEqual(manifest["source_commit"], "a" * 40)
                self.assertEqual(manifest["panels"], list(module.PANELS))


if __name__ == "__main__":
    unittest.main()
