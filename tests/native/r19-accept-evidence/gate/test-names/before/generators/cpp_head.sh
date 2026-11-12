#!/bin/bash
# usage: cpp_head.sh <repo-root>  -> "file:TEST_CASE(\"name\"" lines for HEAD sources, sorted with LC_ALL=C
cd "$1" || exit 2
git ls-tree -r --name-only HEAD src | grep '\.t\.cpp$' | LC_ALL=C sort | while IFS= read -r f; do
  git show "HEAD:$f" | grep -oE 'TEST_CASE\("[^"]*"' | sed "s|^|$f:|"
done | LC_ALL=C sort
