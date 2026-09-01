"""ChinesePoint bridge add-on entrypoint; network work stays off Anki's UI loop."""

from __future__ import annotations

import socket

from aqt import mw
from aqt.qt import QAction
from aqt.utils import showInfo

from .server import BridgeServer, BridgeStartError, ENDPOINT


def _config() -> dict:
    return mw.addonManager.getConfig(__name__) or {}


def _save_config(config: dict) -> None:
    mw.addonManager.writeConfig(__name__, config)


bridge = BridgeServer(_config, _save_config, mw.taskman.run_on_main, lambda: mw.col)
bridge_error = ""


def _ensure_running() -> bool:
    global bridge_error
    try:
        bridge.start()
    except BridgeStartError as exc:
        bridge_error = str(exc)
        return False
    bridge_error = ""
    return True


# This receiver is optional. A busy local port is a recoverable configuration
# issue, never a reason to break Anki's startup.
_ensure_running()


def _details() -> None:
    if not _ensure_running():
        showInfo(
            "ChinesePoint Bridge is not running because " + bridge_error + ".\n\n"
            "Free that port or change the add-on configuration, then select this menu item again."
        )
        return
    port, token = bridge.details()
    addresses = sorted({addr[4][0] for addr in socket.getaddrinfo(socket.gethostname(), None, family=socket.AF_INET)})
    addresses_text = "\n".join(f"http://{address}:{port}{ENDPOINT}" for address in addresses) or "Use this computer's private IPv4 address."
    showInfo(
        "ChinesePoint bridge is running. Enter one of these private-LAN URLs on the reader:\n"
        f"{addresses_text}\n\nBridge token (keep it private):\n{token}\n\n"
        "The reader must be on the same trusted Wi-Fi network."
    )


action = QAction("ChinesePoint Bridge details", mw)
action.triggered.connect(_details)
mw.form.menuTools.addAction(action)
