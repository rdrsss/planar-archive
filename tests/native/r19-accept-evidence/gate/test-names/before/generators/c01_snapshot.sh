#!/bin/bash
# usage: c01_snapshot.sh <repo-root> -> "file:def test_name" lines of the C01 snapshot support tests, sorted
cd "$1" || exit 2
D=tests/native/r19-accept-a-c01-evidence/red/snapshot/untracked/tests/native/r19-accept-support
for f in "$D"/*.py; do
  grep -noE '^[[:space:]]*def test_[A-Za-z0-9_]*' "$f" | sed -E "s|^|$(basename "$f"):|"
done | LC_ALL=C sort
