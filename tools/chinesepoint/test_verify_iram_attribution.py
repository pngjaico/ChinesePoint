#!/usr/bin/env python3
"""Tests for the IRAM attribution guard's parser."""

import importlib.util
import unittest
from pathlib import Path


TOOLS = Path(__file__).resolve().parent
SPEC = importlib.util.spec_from_file_location("verify_iram_attribution", TOOLS / "verify_iram_attribution.py")
assert SPEC and SPEC.loader
MODULE = importlib.util.module_from_spec(SPEC)
SPEC.loader.exec_module(MODULE)


class IramAttributionTest(unittest.TestCase):
    def test_accepts_platform_iram_symbols(self):
        symbol_table = "40375590 l F .iram0.text 00000020 _ZN7freeinkL10epdBusyIsrEv\n"
        self.assertEqual(MODULE.forbidden_iram_symbols(symbol_table), [])

    def test_rejects_chinesepoint_iram_symbols(self):
        symbol_table = "40375590 g F .iram0.text 00000020 _ZN12ChinesePoint3Cjk4syncEv\n"
        self.assertEqual(len(MODULE.forbidden_iram_symbols(symbol_table)), 1)


if __name__ == "__main__":
    unittest.main()
