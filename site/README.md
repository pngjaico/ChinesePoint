# ChinesePoint release dashboard

This static dashboard is the user-facing companion of every ChinesePoint release. It is intentionally Xteink X4 Pro-only, never accepts a local firmware upload, and uses ESP Web Tools only for a release whose evidence gate has passed.

`release-manifest.json` is the site-facing status record. The browser-install component is created only when `artifact.installable` is `true`, all simulator and physical panel rows plus the recovery drill are passed, and `web_install.state` is `ready`. A release workflow must then package the approved image as `web-install-manifest.json` and a merged ESP32-S3 image. The dashboard never accepts an arbitrary `.bin` file.

The visual assets and base stylesheet originated in the supplied site reference ZIP. Its Flash Center, custom binary flow, stale v0.5 checkpoint and unsupported-device material were not imported.
