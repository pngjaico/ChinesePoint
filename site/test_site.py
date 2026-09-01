import json
from pathlib import Path


ROOT = Path(__file__).parent


def test_site_is_x4pro_only_and_has_no_flash_engine():
    html = (ROOT / "index.html").read_text(encoding="utf-8").lower()
    javascript = (ROOT / "script.js").read_text(encoding="utf-8").lower()

    assert "xteink x4 pro" in html
    assert "xteink x3" not in html
    assert "webserial" not in javascript
    assert "navigator.serial" not in javascript
    assert "flasher.js" not in html
    assert "esptool" not in javascript
    assert 'type="file"' not in html
    assert not (ROOT / "flasher.js").exists()


def test_release_manifest_blocks_installation_until_physical_evidence_exists():
    manifest = json.loads((ROOT / "release-manifest.json").read_text(encoding="utf-8"))

    assert manifest["schema"] == 2
    assert manifest["target"] == "xteink-x4-pro"
    assert manifest["artifact"]["installable"] is False
    assert manifest["evidence"]["simulator"]["state"] == "pending"
    assert manifest["evidence"]["physical"]["state"] == "pending"
    assert manifest["evidence"]["recovery"]["state"] == "pending"
