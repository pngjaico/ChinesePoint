# ChinesePoint Anki Desktop bridge

This add-on is the only supported network receiver for ChinesePoint v0.6.
It imports the device's saved vocabulary into Anki Desktop. When the reader
has a saved dictionary answer, the same imported note becomes an Anki card:
the front shows the word and source sentence; the back shows that answer.
Entries without an answer remain notes but deliberately generate no blank card.
Anki Desktop owns all card scheduling after import. The bridge never touches
the device journal or exposes AnkiConnect.

## Install

For a packaged build, download `chinesepoint-anki-bridge-v0.6.1.ankiaddon`
from the matching ChinesePoint source release and open it with Anki Desktop.
Restart Anki Desktop, then continue from step 3 below. The package contains no
top-level folder, as required by Anki add-on archives.

For a development build:

1. In Anki Desktop, choose **Tools → Add-ons → View Files**.
2. Create a `chinesepoint_anki_bridge` folder there and copy the contents of
   this directory's `chinesepoint_anki_bridge` folder into it.
3. Restart Anki Desktop, then use **Tools → ChinesePoint Bridge details**.
4. Copy the displayed token and the computer's private IPv4 address.
5. On the X4 Pro, save at least one word, open **Learning stats → Anki sync**,
   set `http://YOUR-PC-IP:5051/v1/cjk/vocabulary` and the token, then choose
   **Sync now** on a trusted Wi-Fi network.

The add-on binds to all local interfaces so the reader can reach it. It
requires a random bearer token, accepts only the documented endpoint, limits
one payload to 2 MiB/2,000 records, and returns the batch ID the device sent.
It does **not** use mDNS, TLS, a cloud relay, AnkiConnect, or any automatic
background sync. Those omissions are deliberate: the X4 Pro transfer is a
user-triggered, trusted-LAN operation that can be cancelled safely.

The bridge creates and owns the `ChinesePoint` deck and `ChinesePoint
Vocabulary` note type. Re-sending the same device batch is idempotent. A
later export updates the imported fields for the same `ChinesePointId`. The
bridge may upgrade that project-owned note type with its `Answer` field and
card template; do not put personal content in its generated fields or modify
its template.

## Test the protocol parser

From this directory, run:

```powershell
python -m unittest discover -s tests -v
```

These tests exercise the untrusted HTTP payload parser and a synthetic local
HTTP bridge with a fake collection. The latter verifies bearer authentication,
first import, idempotent retry, and a later note update. They are not a
substitute for running the add-on inside a real Anki Desktop profile.

To also exercise the real Anki collection API without opening or modifying a
profile, run this workstation-only smoke test with the local Anki installation:

```powershell
python real_collection_smoke.py `
  --anki-app-packages 'C:\Users\Usuario-pc\AppData\Local\Programs\Anki\app_packages'
```

It creates a temporary collection, verifies first import, idempotent retry and
update, then deletes that temporary collection. It does not prove add-on GUI
startup or X4 Pro Wi-Fi transfer.

## Build the add-on archive

From this directory, run:

```powershell
python package_addon.py
```

It writes `dist/chinesepoint-anki-bridge-v0.6.1.ankiaddon`. Do not distribute a
package until its source commit, package hash, real Anki Desktop import, and
X4 Pro transfer test are recorded in the release evidence.
