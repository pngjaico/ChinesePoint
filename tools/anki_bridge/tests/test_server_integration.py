import http.client
import importlib
import json
import socket
import sys
import types
import unittest
from pathlib import Path


ADDON_ROOT = Path(__file__).parents[1] / "chinesepoint_anki_bridge"
# Import the receiver without executing the add-on entrypoint, which correctly
# requires Anki Desktop's `aqt` module and must not be faked by the test.
PACKAGE_NAME = "chinesepoint_anki_bridge"
package = types.ModuleType(PACKAGE_NAME)
package.__path__ = [str(ADDON_ROOT)]
sys.modules.setdefault(PACKAGE_NAME, package)
SERVER = importlib.import_module(f"{PACKAGE_NAME}.server")


CLIENT_ID = "0123456789abcdef0123456789abcdef"
TOKEN = "t" * 32


class FakeModels:
    def __init__(self):
        self.by_name_result = {}

    def by_name(self, name):
        return self.by_name_result.get(name)

    @staticmethod
    def new(name):
        return {"name": name, "flds": [], "tmpls": []}

    @staticmethod
    def new_field(name):
        return {"name": name}

    @staticmethod
    def add_field(model, field):
        model["flds"].append(field)

    @staticmethod
    def new_template(name):
        return {"name": name}

    def add(self, model):
        self.by_name_result[model["name"]] = model


class FakeDecks:
    @staticmethod
    def id(_name):
        return 1


class FakeCollection:
    def __init__(self):
        self.models = FakeModels()
        self.decks = FakeDecks()
        self.notes = {}
        self.next_id = 1

    def find_notes(self, query):
        word_id = query.split("ChinesePointId:", 1)[1].split('"', 1)[0]
        return [note_id for note_id, note in self.notes.items() if note.get("ChinesePointId") == word_id]

    def get_note(self, note_id):
        return self.notes[note_id]

    @staticmethod
    def new_note(_model):
        return {}

    def add_note(self, note, _deck_id):
        self.notes[self.next_id] = note
        self.next_id += 1

    @staticmethod
    def update_note(_note):
        return None


def free_port():
    with socket.socket(socket.AF_INET, socket.SOCK_STREAM) as sock:
        sock.bind(("127.0.0.1", 0))
        return sock.getsockname()[1]


def export(sentence):
    header = {"schema": "chinesepoint-learner-export", "version": 1, "format": "ndjson"}
    vocabulary = {
        "type": "vocabulary",
        "word_id": "0123456789abcdef",
        "headword": "你好",
        "status": "saved",
        "sentence": sentence,
        "source": {"book_path": "/books/example.epub"},
    }
    return (json.dumps(header) + "\n" + json.dumps(vocabulary) + "\n").encode()


class BridgeServerIntegrationTest(unittest.TestCase):
    def setUp(self):
        self.config = {"port": free_port(), "token": TOKEN}
        self.collection = FakeCollection()
        self.server = SERVER.BridgeServer(
            lambda: self.config,
            lambda value: self.config.update(value),
            lambda callback: callback(),
            lambda: self.collection,
        )
        self.server.start()

    def tearDown(self):
        if self.server._httpd is not None:
            self.server._httpd.shutdown()
            self.server._httpd.server_close()
        if self.server._thread is not None:
            self.server._thread.join(timeout=2)

    def post(self, body, batch, token=TOKEN):
        connection = http.client.HTTPConnection("127.0.0.1", self.config["port"], timeout=2)
        connection.request(
            "POST",
            SERVER.ENDPOINT,
            body=body,
            headers={
                "Authorization": f"Bearer {token}",
                "Content-Type": "application/x-ndjson",
                "X-ChinesePoint-Client": CLIENT_ID,
                "X-ChinesePoint-Batch": batch,
            },
        )
        response = connection.getresponse()
        payload = json.loads(response.read())
        headers = dict(response.getheaders())
        connection.close()
        return response.status, headers, payload

    def test_authenticated_import_is_idempotent_and_later_batch_updates_note(self):
        first_batch = f"cp-v1-{CLIENT_ID}-1-1"
        status, headers, payload = self.post(export("Primeira frase."), first_batch)
        self.assertEqual(status, 200)
        self.assertEqual(headers["X-ChinesePoint-Batch"], first_batch)
        self.assertEqual(payload, {"batch_id": first_batch, "added": 1, "updated": 0})
        self.assertEqual(len(self.collection.notes), 1)

        status, _headers, payload = self.post(export("Primeira frase."), first_batch)
        self.assertEqual(status, 200)
        self.assertEqual(payload, {"batch_id": first_batch, "added": 0, "updated": 0})
        self.assertEqual(len(self.collection.notes), 1)

        next_batch = f"cp-v1-{CLIENT_ID}-2-1"
        status, _headers, payload = self.post(export("Frase atualizada."), next_batch)
        self.assertEqual(status, 200)
        self.assertEqual(payload, {"batch_id": next_batch, "added": 0, "updated": 1})
        self.assertEqual(next(iter(self.collection.notes.values()))["Sentence"], "Frase atualizada.")

    def test_rejects_bad_bearer_token_before_touching_collection(self):
        status, _headers, payload = self.post(export("Não importar."), f"cp-v1-{CLIENT_ID}-1-1", token="wrong")
        self.assertEqual(status, 401)
        self.assertEqual(payload, {"error": "unauthorized"})
        self.assertEqual(self.collection.notes, {})


if __name__ == "__main__":
    unittest.main()
