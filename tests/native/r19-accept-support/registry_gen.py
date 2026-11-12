#!/usr/bin/env python3
# R19-ACCEPT-A counting registry generator. Only toolchain command prefixes
# change; check argv, timeouts, output bounds, selection, inputs, environment,
# ids and revision are preserved.
#
# generate(registry, runner=, counter=, mode=, python=) returns a deep copy in
# which every toolchain command is
#   [python, runner, "--counter", counter, "--mode", mode, "--", *original]
# so that every check execution goes through check_runner.py (D26).
import sys

sys.dont_write_bytecode = True

import argparse
import copy
import json
from pathlib import Path

MODES = ("unittest", "passthrough")


def generate(registry, *, runner, counter, mode, python, transcript=None):
    """Return the counting registry; only each toolchain command prefix differs from the input.

    transcript, when given, adds "--transcript DIR" to each prefix so check_runner retains every
    execution's output (quota driver evidence); without it the prefix is unchanged.
    """
    if mode not in MODES:
        raise ValueError(f"unknown runner mode {mode!r}")
    generated = copy.deepcopy(registry)
    toolchains = generated.get("toolchains")
    if not isinstance(toolchains, list) or not toolchains:
        raise ValueError("registry has no toolchains to wrap")
    retain = [] if transcript is None else ["--transcript", str(transcript)]
    for toolchain in toolchains:
        original = list(toolchain["command"])
        toolchain["command"] = [str(python), str(runner), "--counter", str(counter), *retain,
                                "--mode", mode, "--", *original]
    return generated


def main(argv=None):
    """Write the counting registry for a source registry file; return the exit status."""
    parser = argparse.ArgumentParser(description="Write a counting registry (D26).")
    parser.add_argument("source", type=Path)
    parser.add_argument("output", type=Path)
    parser.add_argument("--runner", required=True, type=Path)
    parser.add_argument("--counter", required=True, type=Path)
    parser.add_argument("--mode", required=True, choices=MODES)
    parser.add_argument("--python", default=sys.executable)
    args = parser.parse_args(argv)
    registry = json.loads(args.source.read_text(encoding="utf-8"))
    generated = generate(registry, runner=args.runner, counter=args.counter,
                         mode=args.mode, python=args.python)
    args.output.write_text(json.dumps(generated, indent=2) + "\n", encoding="utf-8")
    return 0


if __name__ == "__main__":
    sys.exit(main())
