"""Regression test for reproducible embedded web-asset compression."""

from __future__ import annotations

import ast
import gzip
from pathlib import Path
import unittest


SCRIPT = Path(__file__).with_name("build_html.py")


class BuildHtmlDeterminismTest(unittest.TestCase):
    def setUp(self) -> None:
        self.source = SCRIPT.read_text(encoding="utf-8")
        tree = ast.parse(self.source)
        function = next(node for node in tree.body if isinstance(node, ast.FunctionDef) and node.name == "deterministic_gzip")
        module = ast.Module(body=[function], type_ignores=[])
        namespace = {"gzip": gzip}
        exec(compile(module, str(SCRIPT), "exec"), namespace)
        self.compress = namespace["deterministic_gzip"]

    def test_gzip_bytes_are_reproducible_and_have_zero_mtime(self) -> None:
        payload = b"ChinesePoint embedded UI asset"
        first = self.compress(payload)
        second = self.compress(payload)
        self.assertEqual(first, second)
        self.assertEqual(first[4:8], b"\0\0\0\0")
        self.assertEqual(gzip.decompress(first), payload)


if __name__ == "__main__":
    unittest.main()
