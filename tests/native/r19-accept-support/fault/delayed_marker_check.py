#!/usr/bin/env python3
# R19-ACCEPT-A delayed-marker check (fault fixture for the H6 and H7 tests): like
# sleep_check.py it writes RENDEZVOUS through accept_lib.write_rendezvous and, when
# the --hold file exists, consumes it. Unlike sleep_check.py it first sleeps
# DELAY_SECONDS, so the marker appears late: a configured marker wait shorter than
# the delay expires first, and a configured completion wait shorter than the delay
# expires before a non-held check returns. A held run lingers at most
# LINGER_SECONDS after its marker, so an aborted test cannot leave it running long.
# It writes PLANAR_CHECK_RESULT only after reading its own rendezvous back, as
# sleep_check.py does.
#
# Usage: delayed_marker_check.py --evidence DIR [--hold FILE]
# Exit 0 on success, 1 when the self-check fails.
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

DELAY_SECONDS = 4
LINGER_SECONDS = 30


def main(argv=None):
    """Sleep, write the rendezvous, optionally linger when held, write the self-check result; return the exit status."""
    parser = argparse.ArgumentParser(description="R19-ACCEPT-A delayed-marker check.")
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
    time.sleep(DELAY_SECONDS)
    pid = os.getpid()
    pgid = os.getpgid(0)
    marker = accept_lib.write_rendezvous(evidence, "ctest", pid, pgid)
    if holding:
        time.sleep(LINGER_SECONDS)
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
    print(json.dumps({"delayed_marker_check": "passed" if passed else "failed", "pgid": pgid}, sort_keys=True))
    return 0 if passed else 1


if __name__ == "__main__":
    sys.exit(main())
