#!/usr/bin/env python3
# R19-ACCEPT-A mini sleep check: a passthrough registry check that writes
# RENDEZVOUS through accept_lib.write_rendezvous (phase ctest, its own pid and
# pgid) and, when the --hold file exists, consumes it and sleeps so that a fault
# can interrupt it. Otherwise it returns at once. It writes PLANAR_CHECK_RESULT
# only after it reads its own rendezvous back, so the document is derived from
# that self-check: matched 1, passed 1 on success, failed 1 otherwise.
#
# Usage: sleep_check.py --evidence DIR [--hold FILE]
# Exit 0 on success, 1 when the self-check fails. Sleeping is capped at
# HOLD_CAP_SECONDS so an orphaned hold cannot run unbounded.
import sys

sys.dont_write_bytecode = True

import argparse
import json
import os
from pathlib import Path
import time

HERE = Path(__file__).resolve().parent
sys.path.insert(0, str(HERE.parent))

import accept_lib  # noqa: E402

HOLD_CAP_SECONDS = 600


def main(argv=None):
    """Write the rendezvous, optionally hold it, write the self-check result; return the exit status."""
    parser = argparse.ArgumentParser(description="R19-ACCEPT-A mini sleep check.")
    parser.add_argument("--evidence", required=True, type=Path)
    parser.add_argument("--hold", type=Path)
    args = parser.parse_args(argv)
    evidence = args.evidence
    evidence.mkdir(parents=True, exist_ok=True)
    holding = False
    if args.hold is not None:
        try:
            args.hold.unlink()
            holding = True
        except FileNotFoundError:
            holding = False
    pid = os.getpid()
    pgid = os.getpgid(0)
    marker = accept_lib.write_rendezvous(evidence, "ctest", pid, pgid)
    if holding:
        time.sleep(HOLD_CAP_SECONDS)
    try:
        echoed = accept_lib.read_rendezvous(marker)
        passed = echoed["pid"] == pid and echoed["pgid"] == pgid
    except accept_lib.AcceptError:
        passed = False
    result = {"version": 1, "matched": 1, "passed": 1 if passed else 0, "failed": 0 if passed else 1}
    reserved = os.environ.get("PLANAR_CHECK_RESULT")
    if reserved:
        with Path(reserved).open("x", encoding="utf-8") as stream:
            json.dump(result, stream, sort_keys=True, separators=(",", ":"))
    print(json.dumps({"sleep_check": "passed" if passed else "failed", "pgid": pgid}, sort_keys=True))
    return 0 if passed else 1


if __name__ == "__main__":
    sys.exit(main())
