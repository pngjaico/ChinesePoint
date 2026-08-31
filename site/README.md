# ChinesePoint release dashboard

This static dashboard is the user-facing companion of every ChinesePoint release. It is intentionally Xteink X4 Pro-only and never provides a browser flasher, WebSerial action, or local firmware upload.

`release-manifest.json` is the site-facing status record. A binary button may only be enabled when `artifact.installable` is `true` and the physical panel matrix plus recovery drill are recorded as passed.

The visual assets and base stylesheet originated in the supplied site reference ZIP. Its Flash Center, custom binary flow, stale v0.5 checkpoint and unsupported-device material were not imported.
