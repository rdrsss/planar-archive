#!/usr/bin/env python3
# R19-ACCEPT-A honest check runner. It appends one counter line per real
# execution before running its child, forwards child output, exits with the
# child status, and derives PLANAR_CHECK_RESULT honestly in unittest mode.
#
# Usage: check_runner.py --counter FILE --mode unittest|passthrough [--transcript DIR] -- COMMAND...
#   --transcript DIR retains each counted execution's child stdout then stderr as DIR/NNN.txt,
#   where NNN is the execution's line number in the counter file (driver evidence, R19 Q13).
#   unittest mode writes PLANAR_CHECK_RESULT as {"version":1,"matched":N,"passed":P,"failed":F}
#   where N is the "Ran N tests" count, F = failures + errors + unexpected successes,
#   P = N - F - skipped (skips and errors are never passed). No unittest summary
#   means no result document is written. An existing PLANAR_CHECK_RESULT is refused.
#   passthrough mode leaves a child-written result untouched and only counts.
# The child runs as a subprocess (not exec) with PYTHONDONTWRITEBYTECODE=1 and
# the exit status is the child's (a signal exit maps to 128 + signal number).
import sys

sys.dont_write_bytecode = True

import argparse
import json
import os
from pathlib import Path
import re
import subprocess
import time

RESULT_VERSION = 1


def unittest_result(text):
    """Return the honest result document derived from unittest output text, or None without a summary."""
    ran = re.findall(r"^Ran (\d+) tests? in ", text, re.MULTILINE)
    if not ran:
        return None
    matched = int(ran[-1])
    if matched == 0:
        return {"version": RESULT_VERSION, "matched": 0, "passed": 0, "failed": 0}
    summaries = re.findall(r"^(?:OK|FAILED)(?: \((.*)\))?\s*$", text, re.MULTILINE)
    if not summaries:
        return None
    counts = {}
    for key, value in re.findall(r"([A-Za-z ]+?)=(\d+)", summaries[-1] or ""):
        counts[key.strip()] = int(value)
    failed = counts.get("failures", 0) + counts.get("errors", 0) + counts.get("unexpected successes", 0)
    passed = matched - failed - counts.get("skipped", 0)
    if passed < 0:
        return None
    return {"version": RESULT_VERSION, "matched": matched, "passed": passed, "failed": failed}


def _write_result(reserved, result):
    with Path(reserved).open("x", encoding="utf-8") as stream:
        stream.write(json.dumps(result, sort_keys=True, separators=(",", ":")) + "\n")


def main(argv=None):
    """Run the child under the counter and result rules; return the child's exit status."""
    argv = list(sys.argv[1:] if argv is None else argv)
    if "--" not in argv:
        print("check_runner: expected --counter FILE --mode MODE -- COMMAND...", file=sys.stderr)
        return 2
    split = argv.index("--")
    options, command = argv[:split], argv[split + 1:]
    parser = argparse.ArgumentParser(prog="check_runner.py")
    parser.add_argument("--counter", required=True, type=Path)
    parser.add_argument("--mode", required=True, choices=("unittest", "passthrough"))
    parser.add_argument("--transcript", type=Path, default=None)
    args = parser.parse_args(options)
    if not command:
        print("check_runner: no command after --", file=sys.stderr)
        return 2
    try:
        with args.counter.open("a", encoding="utf-8") as stream:
            stream.write(json.dumps({"argv": command, "mono_ns": time.monotonic_ns()}, sort_keys=True) + "\n")
        execution = sum(1 for line in args.counter.read_text(encoding="utf-8").splitlines() if line.strip())
    except OSError as error:
        print(f"check_runner: cannot append counter line: {error}", file=sys.stderr)
        return 2
    env = dict(os.environ)
    env["PYTHONDONTWRITEBYTECODE"] = "1"
    try:
        child = subprocess.run(command, env=env, capture_output=True)
    except OSError as error:
        print(f"check_runner: cannot start child: {error}", file=sys.stderr)
        return 127
    sys.stdout.buffer.write(child.stdout)
    sys.stdout.flush()
    sys.stderr.buffer.write(child.stderr)
    sys.stderr.flush()
    status = child.returncode if child.returncode >= 0 else 128 - child.returncode
    if args.transcript is not None:
        try:
            args.transcript.mkdir(parents=True, exist_ok=True)
            with (args.transcript / f"{execution:03d}.txt").open("xb") as stream:
                stream.write(child.stdout + child.stderr)
        except OSError as error:
            print(f"check_runner: cannot retain execution {execution} output: {error}", file=sys.stderr)
            return 2
    reserved = os.environ.get("PLANAR_CHECK_RESULT")
    if args.mode == "unittest" and reserved:
        text = (child.stdout + child.stderr).decode("utf-8", errors="replace")
        result = unittest_result(text)
        if result is None:
            print("check_runner: no unittest summary; no result document written", file=sys.stderr)
        else:
            try:
                _write_result(reserved, result)
            except FileExistsError:
                print("check_runner: PLANAR_CHECK_RESULT already exists; refusing to overwrite", file=sys.stderr)
                return 1
    return status


if __name__ == "__main__":
    sys.exit(main())
