#!/usr/bin/env python3
"""Validate the ChinesePoint release-evidence contract before publication.

This guard has a deliberately narrow purpose: it does not create a release,
flash a device, or turn an artifact installable. It refuses a claim that an
X4 Pro application image is safe to install unless the simulator, the three
known panel paths, and the DOWN+POWER recovery drill have recorded evidence.
"""

from __future__ import annotations

import argparse
import json
import re
from pathlib import Path
from typing import Any


PANEL_IDS = ("ssd1677", "uc8179", "uc8279")
SHA256_RE = re.compile(r"^[0-9a-f]{64}$")
COMMIT_RE = re.compile(r"^[0-9a-f]{40}$")
STATES = {"pending", "passed", "failed"}


class ReleaseEvidenceError(ValueError):
    """The manifest cannot support the requested release claim."""


def require(condition: bool, message: str) -> None:
    if not condition:
        raise ReleaseEvidenceError(message)


def require_state(value: Any, name: str) -> str:
    require(isinstance(value, str) and value in STATES, f"{name} must be pending, passed, or failed")
    return value


def require_panels(value: Any, name: str) -> dict[str, str]:
    require(isinstance(value, dict), f"{name} must be an object")
    require(set(value) == set(PANEL_IDS), f"{name} must list exactly: {', '.join(PANEL_IDS)}")
    return {panel: require_state(value[panel], f"{name}.{panel}") for panel in PANEL_IDS}


def validate(manifest: dict[str, Any], require_installable: bool = False) -> None:
    require(manifest.get("schema") == 2, "manifest schema must be 2")
    require(manifest.get("project") == "ChinesePoint", "project must be ChinesePoint")
    require(manifest.get("target") == "xteink-x4-pro", "target must be xteink-x4-pro")
    commit = manifest.get("source_commit")
    require(isinstance(commit, str) and COMMIT_RE.fullmatch(commit) is not None, "source_commit must be a full SHA-1")
    require(isinstance(manifest.get("version"), str) and manifest["version"].startswith("ChinesePoint-v"),
            "version must be a ChinesePoint version")

    artifact = manifest.get("artifact")
    require(isinstance(artifact, dict), "artifact must be an object")
    require(artifact.get("filename") == "firmware.bin", "only firmware.bin may be a ChinesePoint app artifact")
    require(artifact.get("environment") == "chinesepoint_x4pro", "artifact environment must be chinesepoint_x4pro")
    require(artifact.get("board_tag") == "CROSSPOINT-BOARD-V1:x4pro;", "artifact must carry the X4 Pro board tag")
    require(isinstance(artifact.get("bytes"), int) and artifact["bytes"] > 0, "artifact bytes must be positive")
    require(isinstance(artifact.get("sha256"), str) and SHA256_RE.fullmatch(artifact["sha256"]) is not None,
            "artifact sha256 must be lowercase hexadecimal")
    require(isinstance(artifact.get("installable"), bool), "artifact installable must be boolean")
    if not artifact["installable"]:
        require(isinstance(artifact.get("installable_reason"), str) and artifact["installable_reason"].strip(),
                "blocked artifact needs an installable_reason")

    evidence = manifest.get("evidence")
    require(isinstance(evidence, dict), "evidence must be an object")
    require(isinstance(evidence.get("native_tests"), int) and evidence["native_tests"] > 0,
            "native_tests must be a positive integer")
    require(isinstance(evidence.get("artifact_tests"), int) and evidence["artifact_tests"] > 0,
            "artifact_tests must be a positive integer")

    simulator = evidence.get("simulator")
    require(isinstance(simulator, dict), "simulator evidence must be an object")
    require_state(simulator.get("state"), "simulator.state")
    require(simulator.get("source_commit") == commit, "simulator source_commit must match the artifact source")
    simulator_panels = require_panels(simulator.get("panels"), "simulator.panels")

    physical = evidence.get("physical")
    require(isinstance(physical, dict), "physical evidence must be an object")
    require_state(physical.get("state"), "physical.state")
    physical_panels = require_panels(physical.get("panels"), "physical.panels")

    recovery = evidence.get("recovery")
    require(isinstance(recovery, dict), "recovery evidence must be an object")
    require_state(recovery.get("state"), "recovery.state")
    require(recovery.get("method") == "DOWN+POWER", "recovery method must be DOWN+POWER")

    if require_installable:
        require(artifact["installable"], "release tags require artifact.installable=true")
        require(simulator["state"] == "passed" and all(value == "passed" for value in simulator_panels.values()),
                "release tags require all three simulator panel paths to pass")
        require(physical["state"] == "passed" and all(value == "passed" for value in physical_panels.values()),
                "release tags require physical evidence for all three X4 Pro panel paths")
        require(recovery["state"] == "passed", "release tags require a passed DOWN+POWER recovery drill")


def main() -> int:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--manifest", type=Path, required=True)
    parser.add_argument("--require-installable", action="store_true")
    args = parser.parse_args()
    try:
        manifest = json.loads(args.manifest.read_text(encoding="utf-8"))
        validate(manifest, require_installable=args.require_installable)
    except (OSError, json.JSONDecodeError, ReleaseEvidenceError) as error:
        parser.error(str(error))
    print("release evidence: valid" + (" and installable" if args.require_installable else ""))
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
