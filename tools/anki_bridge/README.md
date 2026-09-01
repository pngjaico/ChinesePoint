# ChinesePoint Anki Desktop bridge

This add-on is the only supported network receiver for ChinesePoint v0.6.
It imports the device's saved vocabulary into Anki Desktop; it does not touch
the device journal, act as a review scheduler, or expose AnkiConnect.

## Install

For a packaged build, download `chinesepoint-anki-bridge-v0.6.ankiaddon`
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
later export updates the imported fields for the same `ChinesePointId`; do not
put personal notes in those generated fields.

## Test the protocol parser

From this directory, run:

```powershell
python -m unittest discover -s tests -v
```

These tests exercise only the untrusted HTTP payload parser. They are not a
substitute for running the add-on inside a real Anki Desktop profile.

## Build the add-on archive

From this directory, run:

```powershell
python package_addon.py
```

It writes `dist/chinesepoint-anki-bridge-v0.6.ankiaddon`. Do not distribute a
package until its source commit, package hash, real Anki Desktop import, and
X4 Pro transfer test are recorded in the release evidence.
