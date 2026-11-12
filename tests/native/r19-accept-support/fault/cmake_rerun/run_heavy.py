#!/usr/bin/env python3
# R19-ACCEPT-A heavy CMake rerun entry: builds the cmake-rerun configuration
# (composed base, max_rounds 1, scripted comment repairs in the scratch export
# only, kill target cli) and the counting registry naming cmake_check.py for both
# presets, requires the recorded candidate before any live action, then calls
# fault_driver.run_scenario unchanged.
#
# Usage: run_heavy.py --scratch DIR --candidate EXE --record candidate.json [--repo REPO]
# Exit status: 0 when the result carries trajectory.valid true, 3 when the scenario stopped (result.json is
# written first either way). The export overlays the working-tree ledger CMakeLists.txt and schema fixture.
# The run takes hours; it is not started by the test suite (F24 aborts it before any action).
import sys

sys.dont_write_bytecode = True

import argparse
import json
from pathlib import Path

HERE = Path(__file__).resolve().parent
SUPPORT = HERE.parents[1]
if str(SUPPORT) not in sys.path:
    sys.path.insert(0, str(SUPPORT))

import accept_lib  # noqa: E402
import fault_driver  # noqa: E402
import run_acceptance  # noqa: E402

REPO = SUPPORT.parents[2]
# The scenario's check rounds (check-1 to check-4 in fault_driver.run); each runs every registered check.
CHECK_REQUESTS = 4
OLD_REVISION = "58128234"
BASE_LABEL = "baseline_head_cmake58128234"
OWNED_CMAKE_PATHS = (
    "CMakeLists.txt", "acceptance/CMakeLists.txt", "budget/CMakeLists.txt",
    "canonical/CMakeLists.txt", "capture/CMakeLists.txt", "execution/CMakeLists.txt",
    "export/CMakeLists.txt", "hash/CMakeLists.txt", "history/CMakeLists.txt",
    "json_codec/CMakeLists.txt", "ledger/CMakeLists.txt", "lua/CMakeLists.txt",
    "process/CMakeLists.txt", "runtime/CMakeLists.txt",
    "runtime/closure_guard/CMakeLists.txt", "value/CMakeLists.txt",
)


def wait_budget_s(config):
    """Return the wait budget in seconds: the registry timeout_ms values summed over its checks, times the check requests.

    The scenario issues CHECK_REQUESTS check rounds, and each round runs every registered check (D31 H5).
    """
    registry = build_registry(config)
    total_ms = sum(int(check["timeout_ms"]) for check in registry["checks"])
    return total_ms / 1000 * CHECK_REQUESTS


def build_config(scratch_root, candidate):
    """Return the heavy configuration: composed base, max_rounds 1, owned scratch-only repairs, kill target cli.

    Every path lives under scratch_root (the export, evidence, raw logs, ledger, counter and HOME); no
    checkout file is named as a destination. The export is created by fault_driver.run_scenario. The wait keys
    marker_timeout_s and check_timeout_s carry the registry budget (wait_budget_s), so the recorded R19-CMAKE
    durations cannot time out the heavy run.
    """
    scratch = Path(scratch_root)
    repairs = [{"path": "value/CMakeLists.txt", "action": "append_comment", "scratch_only": True}
               for _ in range(2)]
    config = {
        "candidate": str(candidate),
        "repo": str(REPO),
        "mode": "cmake",
        "check_mode": "real",
        "base": "composed",
        "base_label": BASE_LABEL,
        "old_revision": OLD_REVISION,
        "max_rounds": 1,
        "kill_target": "cli",
        "owned": list(OWNED_CMAKE_PATHS),
        "overlay": ["src/engine/ledger/CMakeLists.txt", "fixtures/ledger/expected-schema.sql"],
        "repairs": repairs,
        "tamper": {},
        "scratch": str(scratch),
        "export": str(scratch / "export"),
        "evidence": str(scratch / "evidence"),
        "raw_root": str(scratch / "raw"),
        "home": str(scratch / "home"),
        "ledger": str(scratch / "ledger.db"),
        "counter": str(scratch / "check-counter.log"),
    }
    config["marker_timeout_s"] = wait_budget_s(config)
    config["check_timeout_s"] = wait_budget_s(config)
    return config


def build_registry(config):
    """Return the counting registry naming cmake_check.py for cmake_debug and cmake_release.

    The export and evidence paths are the scratch directories of config; cmake_check.py runs the real
    configure, build, inventory, rendezvous, ctest, lint and no-work rebuild for each preset.
    """
    return fault_driver.cmake_registry(config["export"], config["evidence"], config["home"])


def main(argv=None):
    """Verify the recorded candidate, then run the cmake-rerun scenario; raise AcceptError before any action on a mismatch.

    Options: --scratch DIR, --candidate EXE, --record candidate.json, --repo REPO. The recorded candidate
    is verified (accept_lib.require_recorded_candidate) before any live action; a mismatch raises AcceptError.
    """
    parser = argparse.ArgumentParser(description="R19-ACCEPT-A heavy CMake rerun.")
    parser.add_argument("--scratch", required=True, type=Path)
    parser.add_argument("--candidate", required=True, type=Path)
    parser.add_argument("--record", required=True, type=Path)
    parser.add_argument("--repo", type=Path, default=REPO)
    args = parser.parse_args(argv)
    accept_lib.require_recorded_candidate(args.candidate, args.record, args.repo)
    config = build_config(args.scratch, args.candidate)
    config["repo"] = str(args.repo)
    result = fault_driver.run_scenario(config)
    (args.scratch / "result.json").write_text(json.dumps(result, indent=1, sort_keys=True) + "\n", encoding="utf-8")
    trajectory = result.get("trajectory")
    stopped = not (isinstance(trajectory, dict) and trajectory.get("valid") is True)
    if stopped:
        traced = len(result.get("trace") or [])
        print(f"run_heavy: the scenario stopped before completion ({traced} traced steps); "
              f"see result.json and the scratch evidence", file=sys.stderr)
        return 3
    try:
        run_acceptance.validate_rerun_result(result)
    except accept_lib.AcceptError as error:
        print(f"run_heavy: the scenario completed but rerun validation refused its result: {error}", file=sys.stderr)
        return 4
    return 0


if __name__ == "__main__":
    sys.exit(main())
