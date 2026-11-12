#!/usr/bin/env python3
# R19-ACCEPT-A CMake check adapter: derives the executed CTest inventory from
# live ctest json, writes RENDEZVOUS through accept_lib after inventory, runs the
# gates (lint, no-work repeat build) and never calls verify.py.
#
# Steps, in order: configure, build, inventory, rendezvous, ctest, lint,
# repeat-build. The idle recognizer and source snapshot come from
# r19-cmake-support/check.py, loaded from --support-dir without executing its
# verify.py equivalence step. Each run gets a new attempt folder under
# <evidence>/checks/<preset>/ and keeps every raw log.
#
# Argument contract: --repo, --evidence, --preset debug|release,
# --support-dir (default the real tests/native/r19-cmake-support) and
# --stub-commands JSON (step name to argv; placeholders {attempt}, {evidence},
# {repo}, {build}, {preset}). Exit 0 only when every step passes.
import sys

sys.dont_write_bytecode = True

import argparse
import importlib.util
import json
import os
from pathlib import Path
import shutil
import subprocess
import time
import xml.etree.ElementTree as ET
from collections import Counter

HERE = Path(__file__).resolve().parent
if str(HERE) not in sys.path:
    sys.path.insert(0, str(HERE))

import accept_lib  # noqa: E402

REPO = HERE.parents[2]
DEFAULT_SUPPORT = REPO / "tests/native/r19-cmake-support"
PRESETS = ("debug", "release")
STEPS = ("configure", "build", "inventory", "rendezvous", "ctest", "lint", "repeat-build")
STUBBABLE = tuple(step for step in STEPS if step != "rendezvous")


def command_plan(preset):
    """Return the adapter step names in order for preset."""
    if preset not in PRESETS:
        raise ValueError(f"unknown preset {preset!r}")
    return list(STEPS)


def inventory_verdict(inventory, cases):
    """Return True only when executed cases equal the live inventory with zero skipped or notrun.

    inventory is the list of registered test names; cases are {name, status} records derived from
    the CTest JUnit report. True only when the inventory is nonempty and duplicate-free, the case
    names equal it as multisets (no missing, extra or duplicate names), and every status is passed.
    """
    if not inventory or len(set(inventory)) != len(inventory):
        return False
    names = [case.get("name") for case in cases]
    if Counter(names) != Counter(inventory):
        return False
    return all(case.get("status") == "passed" for case in cases)


def _case_status(case):
    status = case.get("status")
    if case.find("skipped") is not None or status == "skipped":
        return "skipped"
    if status in ("notrun", "disabled"):
        return "notrun"
    if case.find("failure") is not None or case.find("error") is not None:
        return "failed"
    if status in (None, "run"):
        return "passed"
    return "failed"


def cases_from_junit(path):
    """Return [{name, status}] for every testcase of a CTest JUnit report at path."""
    root = ET.parse(path).getroot()
    return [{"name": case.get("name"), "status": _case_status(case)} for case in root.iter("testcase")]


def _load_support(support_dir):
    """Load check.py from support_dir without executing verify.py; return the module."""
    support = Path(support_dir).resolve()
    if str(support) not in sys.path:
        sys.path.insert(0, str(support))
    spec = importlib.util.spec_from_file_location("r19_cmake_check_support", support / "check.py")
    if spec is None or spec.loader is None:
        raise accept_lib.AcceptError(f"cannot load check.py from {support}")
    module = importlib.util.module_from_spec(spec)
    spec.loader.exec_module(module)
    return module


def integrity_verdict(output):
    """Return True only for a no-work repeat build output."""
    data = output.encode() if isinstance(output, str) else bytes(output)
    return bool(_load_support(DEFAULT_SUPPORT).is_no_work_output(data))


def _substitute(argv, values):
    rendered = []
    for item in argv:
        text = str(item)
        for key, value in values.items():
            text = text.replace("{" + key + "}", value)
        rendered.append(text)
    return rendered


def _load_stubs(path):
    if path is None:
        return {}
    data = json.loads(Path(path).read_text(encoding="utf-8"))
    if not isinstance(data, dict):
        raise accept_lib.AcceptError("stub commands must be a JSON object")
    for step, argv in data.items():
        if step not in STUBBABLE:
            raise accept_lib.AcceptError(f"unknown stub step {step!r}")
        if not isinstance(argv, list) or not argv or not all(isinstance(x, str) for x in argv):
            raise accept_lib.AcceptError(f"stub {step!r} must be a nonempty argv list")
    return data


def _tool(name):
    found = shutil.which(name)
    if not found:
        raise accept_lib.AcceptError(f"{name} is not on PATH")
    return found


def _real_argv(step, *, preset, repo, build, attempt):
    if step == "configure":
        return [_tool("cmake"), "--preset", preset]
    if step == "build":
        return [_tool("cmake"), "--build", build, "--parallel", "4", "--target", "all", "planar_tests"]
    if step == "inventory":
        return [_tool("ctest"), "--test-dir", build, "--show-only=json-v1"]
    if step == "ctest":
        return [_tool("ctest"), "--test-dir", build, "--output-on-failure",
                "--output-junit", attempt / "ctest.xml"]
    if step == "lint":
        return [sys.executable, repo / "scripts/rewrite-check.py", "lint", "--build-dir", build]
    if step == "repeat-build":
        return [_tool("cmake"), "--build", build, "--parallel", "4", "--target", "all", "planar_tests"]
    raise accept_lib.AcceptError(f"no command for step {step!r}")


def _require(condition, message):
    if not condition:
        raise accept_lib.AcceptError(message)


def _run(args):
    repo = args.repo.resolve()
    evidence = args.evidence.resolve()
    preset = args.preset
    build = repo / "build" / ("rewrite-" + preset)
    stubs = _load_stubs(args.stub_commands)
    support = _load_support(args.support_dir)
    attempts = evidence / "checks" / preset
    attempts.mkdir(parents=True, exist_ok=True)
    attempt = attempts / (str(time.time_ns()) + "-" + str(os.getpid()))
    attempt.mkdir()  # Never overwrite earlier attempts, including failures.
    values = {"attempt": str(attempt), "evidence": str(evidence), "repo": str(repo),
              "build": str(build), "preset": preset}
    before = support.snapshot(repo)
    (attempt / "source-before.json").write_text(json.dumps(before, indent=2) + "\n")
    manifest = {"preset": preset, "commands": [], "started_ns": time.time_ns(),
                "matched": 0, "passed": 0, "failed": 0, "errors": []}

    def write_manifest():
        (attempt / "MANIFEST.json").write_text(json.dumps(manifest, indent=2) + "\n")

    def run(step):
        argv = stubs.get(step) or _real_argv(step, preset=preset, repo=repo, build=build, attempt=attempt)
        argv = _substitute(argv, values)
        start = time.time_ns()
        with (attempt / (step + ".stdout")).open("wb") as out, (attempt / (step + ".stderr")).open("wb") as err:
            try:
                code = subprocess.run(argv, cwd=repo, stdout=out, stderr=err,
                                      env=accept_lib.child_env()).returncode
            except OSError as error:
                err.write(str(error).encode() + b"\n")
                code = 127
        manifest["commands"].append({"name": step, "argv": argv, "exit": code,
                                     "duration_ns": time.time_ns() - start})
        write_manifest()
        return code

    status = 1
    try:
        query = build / ".cmake/api/v1/query/client-r19-cmake"
        query.mkdir(parents=True, exist_ok=True)
        (query / "codemodel-v2").touch()
        _require(run("configure") == 0, "configure failed; see retained raw logs")
        _require(run("build") == 0, "build failed; see retained raw logs")
        _require(run("inventory") == 0, "inventory failed; see retained raw logs")
        try:
            listing = json.loads((attempt / "inventory.stdout").read_bytes())
            names = [test["name"] for test in listing["tests"]]
        except (ValueError, KeyError, TypeError) as error:
            raise accept_lib.AcceptError("inventory output is not a CTest test listing") from error
        # Rendezvous after inventory and before ctest (D27, D31): a kill during ctest cannot hit a Ninja writer.
        accept_lib.write_rendezvous(evidence, "ctest", os.getpid(), os.getpgid(0))
        ctest_exit = run("ctest")
        junit = attempt / "ctest.xml"
        _require(junit.is_file(), "CTest did not produce its JUnit report")
        cases = cases_from_junit(junit)
        manifest["matched"] = len(cases)
        manifest["passed"] = sum(case["status"] == "passed" for case in cases)
        manifest["failed"] = len(cases) - manifest["passed"]
        _require(inventory_verdict(names, cases),
                 "executed CTest cases differ from the live inventory, or some were skipped or failed")
        _require(ctest_exit == 0, "ctest exited nonzero; see retained raw logs")
        _require(run("lint") == 0, "lint failed; see retained raw logs")
        _require(run("repeat-build") == 0, "no-change repeat build failed")
        output = (attempt / "repeat-build.stdout").read_bytes()
        _require(support.is_no_work_output(output),
                 "no-change build performed work or produced unexplained output")
        status = 0
    except (accept_lib.AcceptError, OSError, ValueError, ET.ParseError) as error:
        manifest["errors"].append(str(error))

    after = support.snapshot(repo)
    (attempt / "source-after.json").write_text(json.dumps(after, indent=2) + "\n")
    if before != after:
        manifest["errors"].append("Source files changed while the actual check was running")
        status = 1
    manifest["finished_ns"] = time.time_ns()
    manifest["exit"] = status
    write_manifest()
    result = {"version": 1, "matched": manifest["matched"], "passed": manifest["passed"],
              "failed": manifest["failed"]}
    reserved = os.environ.get("PLANAR_CHECK_RESULT")
    if reserved:
        with Path(reserved).open("x", encoding="utf-8") as stream:
            json.dump(result, stream, separators=(",", ":"))
    print(json.dumps({"preset": preset, "matched": result["matched"], "passed": result["passed"],
                      "failed": result["failed"], "exit": status, "errors": manifest["errors"]}))
    return status


def main(argv=None):
    """Run the adapter; return 0 only when every step passes and the source tree is unchanged."""
    parser = argparse.ArgumentParser(description="R19-ACCEPT-A CMake check adapter.")
    parser.add_argument("--repo", required=True, type=Path)
    parser.add_argument("--evidence", required=True, type=Path)
    parser.add_argument("--preset", required=True, choices=PRESETS)
    parser.add_argument("--support-dir", type=Path, default=DEFAULT_SUPPORT)
    parser.add_argument("--stub-commands", type=Path)
    args = parser.parse_args(argv)
    try:
        return _run(args)
    except accept_lib.AcceptError as error:
        print(f"cmake_check: {error}", file=sys.stderr)
        return 1


if __name__ == "__main__":
    sys.exit(main())
