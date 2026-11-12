#!/bin/bash
# usage: cpp_names.sh <repo-root>  -> prints "file:TEST_CASE(\"name\"" lines sorted with LC_ALL=C
cd "$1" || exit 2
find src -name '*.t.cpp' -print0 | sort -z | xargs -0 grep -HoE 'TEST_CASE\("[^"]*"' | LC_ALL=C sort
