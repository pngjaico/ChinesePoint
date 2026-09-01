import json
import sys
import unittest
from pathlib import Path

sys.path.insert(0, str(Path(__file__).parents[1] / "chinesepoint_anki_bridge"))
from protocol import ProtocolError, parse_vocabulary_ndjson, valid_batch_id  # noqa: E402


def payload(record: dict) -> bytes:
    return (json.dumps({"schema": "chinesepoint-learner-export", "version": 1, "format": "ndjson"}) + "\n" +
            json.dumps(record) + "\n").encode()


class ProtocolTest(unittest.TestCase):
    def test_accepts_bounded_export_record(self):
        records = parse_vocabulary_ndjson(payload({"type": "vocabulary", "word_id": "0123456789abcdef", "headword": "你好",
                                                   "status": "saved", "sentence": "你好，世界。",
                                                   "source": {"book_path": "/books/example.epub"}}))
        self.assertEqual(records[0].headword, "你好")

    def test_rejects_unknown_schema_and_duplicate_ids(self):
        with self.assertRaises(ProtocolError):
            parse_vocabulary_ndjson(b'{"schema":"wrong"}\n{}\n')
        line = {"type": "vocabulary", "word_id": "0123456789abcdef", "headword": "你好", "status": "saved",
                "sentence": "你好", "source": {"book_path": "/a.epub"}}
        with self.assertRaises(ProtocolError):
            parse_vocabulary_ndjson(payload(line) + json.dumps(line).encode() + b"\n")

    def test_batch_id_is_strict(self):
        self.assertTrue(valid_batch_id("cp-v1-0123456789abcdef0123456789abcdef-42-3"))
        self.assertFalse(valid_batch_id("cp-v1-not-a-valid-id"))


if __name__ == "__main__":
    unittest.main()
