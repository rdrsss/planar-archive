#!/usr/bin/env python3
# R20-CARRY-A evidence tool (I17/I25): prints one line per file under a build directory as
# `<relative path> <st_mtime_ns> <size>`, sorted, excluding only CMakeFiles/*.log and Testing/. The output of two
# snapshots taken around a reconfigure is compared with `diff` to prove configure rewrote no generated file.
#
# Usage: mtime_snapshot.py <build-dir>
import os
import sys


def main():
    if len(sys.argv) != 2 or not os.path.isdir(sys.argv[1]):
        print("usage: mtime_snapshot.py <build-dir>", file=sys.stderr)
        return 2
    root = sys.argv[1]
    lines = []
    for dirpath, dirnames, filenames in os.walk(root):
        rel_dir = os.path.relpath(dirpath, root)
        parts = [] if rel_dir == "." else rel_dir.split(os.sep)
        if parts[:1] == ["Testing"]:
            dirnames[:] = []
            continue
        dirnames.sort()
        for name in filenames:
            if parts[:1] == ["CMakeFiles"] and name.endswith(".log"):
                continue
            path = os.path.join(dirpath, name)
            try:
                st = os.lstat(path)
            except OSError:
                continue
            lines.append((os.path.join(*parts, name), st.st_mtime_ns, st.st_size))
    for rel, mtime, size in sorted(lines):
        print(f"{rel} {mtime} {size}")
    return 0


if __name__ == "__main__":
    sys.exit(main())
