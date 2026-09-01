"""Authenticated, bounded local HTTP receiver for ChinesePoint vocabulary."""

from __future__ import annotations

import hmac
import json
import secrets
import threading
from http.server import BaseHTTPRequestHandler, ThreadingHTTPServer
from typing import Callable

from .protocol import MAX_PAYLOAD_BYTES, ProtocolError, VocabularyRecord, parse_vocabulary_ndjson, valid_batch_id

ENDPOINT = "/v1/cjk/vocabulary"
DECK_NAME = "ChinesePoint"
MODEL_NAME = "ChinesePoint Vocabulary"
MAX_PROCESSED_BATCHES = 128


class BridgeStartError(RuntimeError):
    """The optional listener could not start without risking Anki startup."""


class _BridgeHttpServer(ThreadingHTTPServer):
    allow_reuse_address = True


class BridgeServer:
    def __init__(self, config_getter: Callable[[], dict], config_saver: Callable[[dict], None],
                 run_on_main: Callable[[Callable[[], None]], None], collection_getter: Callable[[], object | None]):
        self._config_getter = config_getter
        self._config_saver = config_saver
        self._run_on_main = run_on_main
        self._collection_getter = collection_getter
        self._httpd: ThreadingHTTPServer | None = None
        self._thread: threading.Thread | None = None

    def start(self) -> None:
        if self._httpd is not None:
            return
        config = self._config()
        try:
            httpd = _BridgeHttpServer(("0.0.0.0", int(config["port"])), self._handler_type())
        except OSError as exc:
            # A busy port must not prevent Anki Desktop from opening. The user
            # can free the port and select the details menu item to retry.
            raise BridgeStartError(f"port {config['port']} is unavailable") from exc
        self._httpd = httpd
        self._httpd.daemon_threads = True
        self._thread = threading.Thread(target=self._httpd.serve_forever, name="ChinesePointBridge", daemon=True)
        self._thread.start()

    def details(self) -> tuple[int, str]:
        config = self._config()
        return int(config["port"]), str(config["token"])

    def _config(self) -> dict:
        config = dict(self._config_getter() or {})
        config["port"] = int(config.get("port", 5051))
        if not 1024 <= config["port"] <= 65535:
            config["port"] = 5051
        token = config.get("token")
        if not isinstance(token, str) or len(token) < 32:
            config["token"] = secrets.token_urlsafe(32)
            self._config_saver(config)
        return config

    def _handler_type(self):
        bridge = self

        class Handler(BaseHTTPRequestHandler):
            server_version = "ChinesePointBridge/1"

            def log_message(self, _format: str, *_args: object) -> None:
                # Do not leak bearer credentials or reader vocabulary to Anki's console.
                return

            def do_POST(self) -> None:  # noqa: N802 (HTTP method required by stdlib)
                if self.path != ENDPOINT:
                    self._reply(404, {"error": "not found"})
                    return
                token = self.headers.get("Authorization", "")
                expected = "Bearer " + str(bridge._config()["token"])
                if not hmac.compare_digest(token, expected):
                    self._reply(401, {"error": "unauthorized"})
                    return
                batch_id = self.headers.get("X-ChinesePoint-Batch", "")
                client_id = self.headers.get("X-ChinesePoint-Client", "")
                if not valid_batch_id(batch_id) or len(client_id) != 32 or not all(c in "0123456789abcdef" for c in client_id):
                    self._reply(400, {"error": "invalid batch"})
                    return
                try:
                    length = int(self.headers.get("Content-Length", "-1"))
                except ValueError:
                    length = -1
                if length <= 0 or length > MAX_PAYLOAD_BYTES:
                    self._reply(413, {"error": "payload too large"})
                    return
                try:
                    records = parse_vocabulary_ndjson(self.rfile.read(length))
                    added, updated = bridge._apply(batch_id, records)
                except ProtocolError as exc:
                    self._reply(400, {"error": str(exc)})
                    return
                except TimeoutError:
                    self._reply(503, {"error": "Anki collection busy"})
                    return
                except Exception:
                    self._reply(500, {"error": "Anki import failed"})
                    return
                self._reply(200, {"batch_id": batch_id, "added": added, "updated": updated}, batch_id)

            def _reply(self, status: int, payload: dict, batch_id: str | None = None) -> None:
                body = json.dumps(payload, separators=(",", ":")).encode("utf-8")
                self.send_response(status)
                self.send_header("Content-Type", "application/json")
                self.send_header("Content-Length", str(len(body)))
                if batch_id is not None:
                    self.send_header("X-ChinesePoint-Batch", batch_id)
                self.end_headers()
                self.wfile.write(body)

        return Handler

    def _apply(self, batch_id: str, records: list[VocabularyRecord]) -> tuple[int, int]:
        config = self._config()
        if batch_id in config.get("processed_batches", []):
            return 0, 0
        done = threading.Event()
        result: list[tuple[int, int] | BaseException] = []

        def on_main() -> None:
            try:
                collection = self._collection_getter()
                if collection is None:
                    raise RuntimeError("no Anki profile is open")
                result.append(_upsert_records(collection, records))
                config_now = self._config()
                processed = list(config_now.get("processed_batches", []))
                processed.append(batch_id)
                config_now["processed_batches"] = processed[-MAX_PROCESSED_BATCHES:]
                self._config_saver(config_now)
            except BaseException as exc:  # return error to the HTTP worker, never crash Anki's UI loop
                result.append(exc)
            finally:
                done.set()

        self._run_on_main(on_main)
        if not done.wait(timeout=30):
            raise TimeoutError()
        if not result:
            raise RuntimeError("missing import result")
        if isinstance(result[0], BaseException):
            raise result[0]
        return result[0]


def _ensure_model(collection: object) -> object:
    models = collection.models
    model = models.by_name(MODEL_NAME)
    if model:
        return model
    model = models.new(MODEL_NAME)
    for field_name in ("Word", "Sentence", "Source", "Status", "ChinesePointId"):
        models.add_field(model, models.new_field(field_name))
    template = models.new_template("Recognition")
    template["qfmt"] = "{{Word}}<br><br>{{Sentence}}"
    template["afmt"] = "{{FrontSide}}<hr id=answer>{{Source}}<br>{{Status}}"
    model["tmpls"].append(template)
    model["css"] = ".card { font-family: arial; font-size: 28px; text-align: center; }"
    models.add(model)
    return model


def _upsert_records(collection: object, records: list[VocabularyRecord]) -> tuple[int, int]:
    model = _ensure_model(collection)
    deck_id = collection.decks.id(DECK_NAME)
    added = updated = 0
    for record in records:
        note_ids = collection.find_notes(f'"ChinesePointId:{record.word_id}"')
        if note_ids:
            note = collection.get_note(note_ids[0])
            updated += 1
        else:
            note = collection.new_note(model)
            added += 1
        note["Word"] = record.headword
        note["Sentence"] = record.sentence
        note["Source"] = record.book_path
        note["Status"] = record.status
        note["ChinesePointId"] = record.word_id
        if note_ids:
            collection.update_note(note)
        else:
            collection.add_note(note, deck_id)
    return added, updated
