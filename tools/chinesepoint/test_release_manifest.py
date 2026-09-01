"""Unit tests for the release-evidence guard."""

from __future__ import annotations

import copy
import importlib.util
import json
import unittest
from pathlib import Path


ROOT = Path(__file__).resolve().parents[2]
MODULE_PATH = ROOT / "tools" / "chinesepoint" / "verify_release_manifest.py"
SPEC = importlib.util.spec_from_file_location("verify_release_manifest", MODULE_PATH)
assert SPEC and SPEC.loader
MODULE = importlib.util.module_from_spec(SPEC)
SPEC.loader.exec_module(MODULE)


class ReleaseManifestTest(unittest.TestCase):
    def setUp(self) -> None:
        self.manifest = json.loads((ROOT / "site" / "release-manifest.json").read_text(encoding="utf-8"))

    def test_current_prephysical_manifest_is_valid_but_not_installable(self) -> None:
        MODULE.validate(self.manifest)
        with self.assertRaises(MODULE.ReleaseEvidenceError):
            MODULE.validate(self.manifest, require_installable=True)

    def test_installable_claim_requires_every_panel_and_recovery(self) -> None:
        candidate = copy.deepcopy(self.manifest)
        candidate["artifact"]["installable"] = True
        candidate["evidence"]["simulator"]["state"] = "passed"
        candidate["evidence"]["simulator"]["panels"] = {panel: "passed" for panel in MODULE.PANEL_IDS}
        candidate["evidence"]["physical"]["state"] = "passed"
        candidate["evidence"]["physical"]["panels"] = {panel: "passed" for panel in MODULE.PANEL_IDS}
        candidate["evidence"]["recovery"]["state"] = "passed"
        MODULE.validate(candidate, require_installable=True)
        candidate["evidence"]["physical"]["panels"]["uc8279"] = "pending"
        with self.assertRaises(MODULE.ReleaseEvidenceError):
            MODULE.validate(candidate, require_installable=True)


if __name__ == "__main__":
    unittest.main()
