"""Strict, dependency-free parser for the ChinesePoint v1 NDJSON export."""

from __future__ import annotations

import json
import re
from dataclasses import dataclass
from typing import Any

MAX_PAYLOAD_BYTES = 2 * 1024 * 1024
MAX_RECORDS = 2000
WORD_ID_RE = re.compile(r"^[0-9a-f]{16}$")
BATCH_RE = re.compile(r"^cp-v1-[0-9a-f]{32}-[0-9]+-[0-9]+$")


class ProtocolError(ValueError):
    """The untrusted request cannot change an Anki collection."""


@dataclass(frozen=True)
class VocabularyRecord:
    word_id: str
    headword: str
    sentence: str
    book_path: str
    status: str


def valid_batch_id(value: str) -> bool:
    return bool(BATCH_RE.fullmatch(value))


def _expect_string(value: Any, field: str, maximum: int) -> str:
    if not isinstance(value, str) or not value or len(value.encode("utf-8")) > maximum:
        raise ProtocolError(f"invalid {field}")
    return value


def parse_vocabulary_ndjson(body: bytes) -> list[VocabularyRecord]:
    if not body or len(body) > MAX_PAYLOAD_BYTES:
        raise ProtocolError("payload size is invalid")
    try:
        lines = body.decode("utf-8").splitlines()
    except UnicodeDecodeError as exc:
        raise ProtocolError("payload is not utf-8") from exc
    if len(lines) < 2 or len(lines) > MAX_RECORDS + 1:
        raise ProtocolError("record count is invalid")
    try:
        header = json.loads(lines[0])
    except json.JSONDecodeError as exc:
        raise ProtocolError("invalid header") from exc
    if header != {"schema": "chinesepoint-learner-export", "version": 1, "format": "ndjson"}:
        raise ProtocolError("unsupported export schema")

    records: list[VocabularyRecord] = []
    seen: set[str] = set()
    for line in lines[1:]:
        try:
            value = json.loads(line)
        except json.JSONDecodeError as exc:
            raise ProtocolError("invalid vocabulary record") from exc
        if not isinstance(value, dict) or value.get("type") != "vocabulary":
            raise ProtocolError("record type is invalid")
        word_id = _expect_string(value.get("word_id"), "word_id", 16)
        if not WORD_ID_RE.fullmatch(word_id) or word_id in seen:
            raise ProtocolError("word_id is invalid or duplicated")
        source = value.get("source")
        if not isinstance(source, dict):
            raise ProtocolError("source is invalid")
        status = _expect_string(value.get("status"), "status", 16)
        if status not in {"encountered", "saved", "learning", "known"}:
            raise ProtocolError("status is invalid")
        records.append(
            VocabularyRecord(
                word_id=word_id,
                headword=_expect_string(value.get("headword"), "headword", 256),
                sentence=_expect_string(value.get("sentence"), "sentence", 4096),
                book_path=_expect_string(source.get("book_path"), "book_path", 1024),
                status=status,
            )
        )
        seen.add(word_id)
    return records
