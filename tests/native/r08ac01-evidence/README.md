# R08-A-C01 corrective coder evidence

Shared checkout, HEAD 3171c49e7b92d3abda01bfbb8bfb0c6b29d4c67a.
Only observations.lua, schema.t.cpp, stdlib README and this evidence directory
were edited by this coder. Prior R08-A manifests and review evidence are preserved.
No Planar tools, legacy binaries, installation, commits or publication were used.

## Red before green

Before any observation validator edit, added three production-export Catch2 cases:
portable checkpoint aliases (six sections), valid missing sorted paths, and
original semantic diagnostic indexes (five sections). Built with
`cmake --build --preset debug --target planar_runtime_tests` (red-build.log).
Ran:

```
build/rewrite-debug/src/kernel/planar_runtime_tests '[r08ac01]' --reporter console --reporter JUnit::out=tests/native/r08ac01-evidence/red.xml
```

red.log/red.xml preserve exit 42: 3 cases, 279 assertions, 11 failed assertions,
2 failed cases, zero skips. Five accepted aliases incorrectly succeeded; the
already-refused boundary case returned only one ownership path. All five semantic
index cases pointed into sorted order instead of raw JSON. Missing paths passed.

After the validator fix, the same command targeting green.xml passed 3 cases /
309 assertions (green-build.log, green.log, green.xml). The additional assertions
come from successfully reaching refusal detail checks. Then tightened alias
field expectations from prefix matches to exact raw indexes and formatted C++.
The final source is frozen below; final-regressions.log/.xml show the final
`[r08ac01]` run passing 3 cases / 309 assertions, zero skips.

## Final checks

Executed directly, serially per build directory, exit 0 unless noted above:

```
/opt/homebrew/opt/llvm/bin/clang-format -i src/kernel/runtime/schema.t.cpp
cmake --build --preset debug --target planar_runtime_tests
build/rewrite-debug/src/kernel/planar_runtime_tests '[render]' --reporter console --reporter JUnit::out=tests/native/r08ac01-evidence/final-focused.xml
ctest --test-dir build/rewrite-debug -R 'replacement.(render|runtime|schema|semantic|selection)' --output-on-failure --output-junit tests/native/r08ac01-evidence/ctest.xml
python3 scripts/rewrite-check.py lint --build-dir build/rewrite-debug
cmake --build --preset debug --target planar_runtime_tests
git diff --check
```

- final-build.log: rebuild successful.
- final-focused.log/.xml: 15 render cases, 27,176 assertions, zero skips/failures.
- ctest.log/.xml: 76 matched cases, zero skips/failures. CTest writes relative
  JUnit paths under its build directory; XML was copied here afterwards.
- lint.log: formatter and Doxygen passed for 22 active first-party sources.
- no-change-build.log: `ninja: no work to do.`
- diff-check.log: empty, exit 0.

Scope: debug macOS native production Lua renderer. No Linux, release, native CLI
transport or final integrated R09 gate was exercised. Fresh independent review
is required. The original review probes runner was not run or modified.

candidate.sha256 covers the three changed source/reference files and all raw
corrective evidence except itself. Coordinator round records are outside coder
ownership. Earlier candidate manifests naturally no longer match changed source
files; they remain intact as historical evidence.
