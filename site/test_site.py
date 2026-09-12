import json
from pathlib import Path


ROOT = Path(__file__).parent


def test_site_is_x4pro_only_and_only_prepares_a_gated_usb_installer():
    html = (ROOT / "index.html").read_text(encoding="utf-8").lower()
    javascript = (ROOT / "script.js").read_text(encoding="utf-8").lower()

    assert "xteink x4 pro" in html
    assert "xteink x3" not in html
    assert "esp-web-tools@10.1.0" in html
    assert "esp-web-install-button" in javascript
    assert "mayofferwebinstaller" in javascript
    assert "navigator.serial" not in javascript
    assert 'type="file"' not in html


def test_site_does_not_present_stale_simulator_or_anki_evidence_as_complete():
    html = (ROOT / "index.html").read_text(encoding="utf-8")

    assert "Build, boot e captura: passou" not in html
    assert "run #22" not in html
    assert html.count("consulte o manifesto do artefato") == 3
    assert "transferência real X4 Pro → Anki Desktop ainda é pendente" in html


def test_public_docs_do_not_promise_an_unimplemented_browser_simulator():
    readme = (ROOT.parent / "README.md").read_text(encoding="utf-8").lower()
    architecture = (ROOT.parent / "docs" / "chinesepoint" / "v1-architecture.md").read_text(encoding="utf-8").lower()

    assert "browser simulator" not in readme
    assert "browser simulator" not in architecture


def test_release_manifest_blocks_installation_until_physical_evidence_exists():
    manifest = json.loads((ROOT / "release-manifest.json").read_text(encoding="utf-8"))

    assert manifest["schema"] == 3
    assert manifest["target"] == "xteink-x4-pro"
    assert manifest["artifact"]["installable"] is False
    assert manifest["web_install"]["state"] == "blocked"
    assert manifest["evidence"]["simulator"]["state"] == "passed"
    assert manifest["evidence"]["physical"]["state"] == "pending"
    assert manifest["evidence"]["recovery"]["state"] == "pending"
