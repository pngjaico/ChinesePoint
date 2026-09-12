#!/usr/bin/env python3
"""Exercise the ChinesePoint bridge against Anki's real Collection API.

This does not launch Anki Desktop or touch its profile directory. It is a
manual workstation smoke test for the persistence API that the add-on uses.
"""

from __future__ import annotations

import argparse
import http.client
import importlib
import json
import socket
import sys
import tempfile
import types
from pathlib import Path


ROOT = Path(__file__).resolve().parent
ADDON = ROOT / "chinesepoint_anki_bridge"
CLIENT_ID = "0123456789abcdef0123456789abcdef"
TOKEN = "test-token-not-for-production-0123456789abcdef"


def free_port() -> int:
    with socket.socket() as probe:
        probe.bind(("127.0.0.1", 0))
        return int(probe.getsockname()[1])


def load_server():
    package_name = "chinesepoint_anki_bridge_runtime_smoke"
    package = types.ModuleType(package_name)
    package.__path__ = [str(ADDON)]
    sys.modules[package_name] = package
    return importlib.import_module(f"{package_name}.server")


def body(sentence: str) -> bytes:
    header = {"schema": "chinesepoint-learner-export", "version": 1, "format": "ndjson"}
    record = {
        "type": "vocabulary",
        "word_id": "0123456789abcdef",
        "headword": "你好",
        "status": "saved",
        "sentence": sentence,
        "source": {"book_path": "/books/anki-runtime-smoke.epub"},
    }
    return (json.dumps(header) + "\n" + json.dumps(record) + "\n").encode("utf-8")


def post(server, port: int, batch: str, sentence: str) -> dict:
    request = http.client.HTTPConnection("127.0.0.1", port, timeout=5)
    request.request(
        "POST",
        server.ENDPOINT,
        body=body(sentence),
        headers={
            "Authorization": f"Bearer {TOKEN}",
            "Content-Type": "application/x-ndjson",
            "X-ChinesePoint-Client": CLIENT_ID,
            "X-ChinesePoint-Batch": batch,
        },
    )
    response = request.getresponse()
    result = {"status": response.status, "payload": json.loads(response.read())}
    request.close()
    return result


def main() -> int:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument(
        "--anki-app-packages",
        type=Path,
        required=True,
        help="Anki Desktop app_packages directory; it is used only to import anki.collection.",
    )
    args = parser.parse_args()
    if not (args.anki_app_packages / "anki" / "collection.pyc").is_file():
        parser.error("--anki-app-packages does not contain Anki's collection runtime")

    sys.path.insert(0, str(args.anki_app_packages))
    from anki.collection import Collection

    server = load_server()
    config = {"port": free_port(), "token": TOKEN, "processed_batches": []}
    with tempfile.TemporaryDirectory(prefix="chinesepoint-anki-smoke-") as temporary:
        collection = Collection(str(Path(temporary) / "collection.anki2"))
        bridge = server.BridgeServer(
            lambda: config,
            lambda value: config.update(value),
            lambda callback: callback(),
            lambda: collection,
        )
        bridge.start()
        try:
            first = post(server, config["port"], f"cp-v1-{CLIENT_ID}-1-1", "Frase inicial.")
            retry = post(server, config["port"], f"cp-v1-{CLIENT_ID}-1-1", "Frase inicial.")
            updated = post(server, config["port"], f"cp-v1-{CLIENT_ID}-2-1", "Frase atualizada.")
            note_ids = collection.find_notes('"ChinesePointId:0123456789abcdef"')
            if (first, retry, updated) != (
                {"status": 200, "payload": {"batch_id": f"cp-v1-{CLIENT_ID}-1-1", "added": 1, "updated": 0}},
                {"status": 200, "payload": {"batch_id": f"cp-v1-{CLIENT_ID}-1-1", "added": 0, "updated": 0}},
                {"status": 200, "payload": {"batch_id": f"cp-v1-{CLIENT_ID}-2-1", "added": 0, "updated": 1}},
            ) or len(note_ids) != 1 or collection.get_note(note_ids[0])["Sentence"] != "Frase atualizada.":
                raise RuntimeError("unexpected bridge result against real Anki collection")
            print("real Anki collection bridge smoke passed")
        finally:
            if bridge._httpd is not None:
                bridge._httpd.shutdown()
                bridge._httpd.server_close()
            if bridge._thread is not None:
                bridge._thread.join(timeout=2)
            collection.close()
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
