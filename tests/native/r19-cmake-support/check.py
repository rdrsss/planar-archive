#!/usr/bin/env python3
"""Trusted real R19 check adapter; results come from executed CTest JUnit cases."""
import argparse
import hashlib
import json
import os
from pathlib import Path
import shutil
import subprocess
import sys
import time
import xml.etree.ElementTree as ET

import verify


def is_no_work_output(output):
    """Accept only Ninja's idle result, optionally after its exact glob probe."""
    return output.removesuffix(b"\n") in (
        b"ninja: no work to do.",
        b"[0/2] Re-checking globbed directories...\nninja: no work to do.",
    )


def snapshot(repo):
    return {str(p.relative_to(repo)): hashlib.sha256(p.read_bytes()).hexdigest()
            for p in sorted((repo / "src").rglob("*")) if p.is_file()}


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--repo", required=True, type=Path)
    parser.add_argument("--evidence", required=True, type=Path)
    parser.add_argument("--preset", required=True, choices=("debug", "release"))
    args = parser.parse_args()
    repo = args.repo.resolve()
    evidence = args.evidence.resolve()
    build = repo / "build" / ("rewrite-" + args.preset)
    attempts = evidence / "checks" / args.preset
    attempts.mkdir(parents=True, exist_ok=True)
    attempt = attempts / (str(time.time_ns()) + "-" + str(os.getpid()))
    attempt.mkdir()  # Never overwrite earlier attempts, including failures.
    before = snapshot(repo)
    (attempt / "source-before.json").write_text(json.dumps(before, indent=2) + "\n")
    manifest = {"preset": args.preset, "commands": [], "started_ns": time.time_ns(),
                "matched": 0, "passed": 0, "failed": 0, "errors": []}
    cmake = shutil.which("cmake")
    ctest = shutil.which("ctest")
    if not cmake or not ctest:
        raise RuntimeError("Explicit registry PATH does not resolve CMake/CTest")
    def run(name, command):
        with (attempt / (name + ".stdout")).open("wb") as out, (attempt / (name + ".stderr")).open("wb") as err:
            start = time.time_ns()
            result = subprocess.run(list(map(str, command)), cwd=repo, stdout=out, stderr=err)
        manifest["commands"].append({"name": name, "argv": list(map(str, command)),
                                     "exit": result.returncode, "duration_ns": time.time_ns()-start})
        (attempt / "MANIFEST.json").write_text(json.dumps(manifest, indent=2) + "\n")
        if result.returncode:
            raise RuntimeError(f"{name} failed with exit {result.returncode}; see retained raw logs")
        return (attempt / (name + ".stdout")).read_bytes()
    status = 1
    try:
        query = build / ".cmake/api/v1/query/client-r19-cmake"
        query.mkdir(parents=True, exist_ok=True)
        (query / "codemodel-v2").touch()
        run("configure", [cmake, "--preset", args.preset])
        run("build", [cmake, "--build", build, "--parallel", "4", "--target", "all", "planar_tests"])
        run("equivalence", [sys.executable, Path(__file__).with_name("verify.py"), "--repo", repo,
                            "--evidence", evidence, "--preset", args.preset,
                            "--output", attempt / "EQUIVALENCE.json"])
        listing = run("inventory", [ctest, "--test-dir", build, "--show-only=json-v1"])
        names = [t["name"] for t in json.loads(listing)["tests"]]
        if not names or len(names) != len(set(names)):
            raise RuntimeError("Empty or duplicate registered tests")
        junit = attempt / "ctest.xml"
        # Record and parse an actual report even when CTest returns a failure.
        ctest_failure = None
        try:
            run("ctest", [ctest, "--test-dir", build, "--output-on-failure", "--output-junit", junit])
        except RuntimeError as error:
            ctest_failure = error
        if not junit.is_file():
            raise RuntimeError("CTest did not produce its actual JUnit report")
        cases = ET.parse(junit).findall(".//testcase")
        manifest["matched"] = len(cases)
        manifest["passed"] = sum(c.find("failure") is None and c.find("error") is None and
                                 c.find("skipped") is None and c.get("status") not in {"notrun", "disabled"}
                                 for c in cases)
        manifest["failed"] = len(cases) - manifest["passed"]
        actual_names = [c.get("name") for c in cases]
        if sorted(actual_names) != sorted(names) or manifest["passed"] != len(names):
            raise RuntimeError("Executed CTest cases differ from inventory, failed, or skipped")
        if ctest_failure:
            raise ctest_failure
        run("lint", [sys.executable, repo / "scripts/rewrite-check.py", "lint", "--build-dir", build])
        repeat = run("repeat-build", [cmake, "--build", build, "--parallel", "4", "--target", "all", "planar_tests"])
        if not is_no_work_output(repeat):
            raise RuntimeError("No-change build performed work or produced unexplained output")
        status = 0
    except Exception as error:
        manifest["errors"].append(str(error))
    after = snapshot(repo)
    (attempt / "source-after.json").write_text(json.dumps(after, indent=2) + "\n")
    if before != after:
        manifest["errors"].append("Source files changed while the actual check was running")
        status = 1
    manifest["finished_ns"] = time.time_ns()
    manifest["exit"] = status
    (attempt / "MANIFEST.json").write_text(json.dumps(manifest, indent=2) + "\n")
    result = {"version": 1, "matched": manifest["matched"], "passed": manifest["passed"],
              "failed": manifest["failed"]}
    reserved = os.environ.get("PLANAR_CHECK_RESULT")
    if reserved:
        with Path(reserved).open("x") as stream:
            json.dump(result, stream, separators=(",", ":"))
    print(json.dumps({"preset": args.preset, "matched": result["matched"], "passed": result["passed"],
                      "failed": result["failed"], "exit": status, "errors": manifest["errors"]}))
    return status


if __name__ == "__main__":
    sys.exit(main())
