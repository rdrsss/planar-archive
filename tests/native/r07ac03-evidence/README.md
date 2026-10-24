# R07-A-C03 native string pattern correction evidence

Candidate implementation; independent review pending. Changes are limited to
src/kernel/runtime/runtime.cpp, runtime.t.cpp, tests/stdlib/bridge.lua,
stdlib/planar/README.md and this evidence directory. The checkout is shared;
existing edits are preserved. No Planar tools, installed binary, legacy queue,
commits, installation or publication were used.

## Failing-before-fix evidence

before-runtime.cpp preserves the exact effective C02 runtime before this fix.
With new bridge probes and subprocess regressions but this unchanged runtime,
red-build.log records the test build and red-regressions.log records exit 42:
two cases failed, 17 failed assertions. All eight pattern paths timed out and
were killed with SIGKILL after the 400 x 10 ms polling bound. The capability
absence check failed as expected. This is a watchdog bound, not a native runtime
deadline; scheduling can extend its elapsed duration.

The expressions retain the C02 review's pattern `a*a*a*a*a*b`: find uses 100
`a` bytes; match/gmatch/gsub use 50,000. Both direct calls and string methods
are tested independently. gmatch is iterated, so the regression enters its
matcher rather than merely constructing the iterator. No loop was weakened.
Normal string operations already passed in this red run.

## Correction and boundary

The curated string table removes find/match/gmatch/gsub together with the
previously removed dump/format. Lua's string metatable indexes this same table,
so methods lose access too. There is no alternate plain-find capability.
Capability assertions explicitly check all eight accesses are nil. Each
pathological attempted call must now return a native internal refusal within
the subprocess watchdog bound. Positive checks retain byte/char, method sub,
rep with separators, lower/upper, reverse, NUL byte length, pack/unpack/packsize.

The reference documents that native C calls do not consume the Lua instruction
meter. The remaining string operations are bounded by input/output/binary-format
sizes and allocation ceilings; no wall-clock or universal native instruction
meter is claimed. This does not add schemas or render behavior.

## Executed verification

All commands ran sequentially in the shared repository, against the debug
replacement build only:

```sh
cmake --build build/rewrite-debug --target planar_runtime_tests
build/rewrite-debug/src/kernel/planar_runtime_tests '[r07ac03]'
/opt/homebrew/opt/llvm/bin/clang-format -i src/kernel/runtime/runtime.cpp src/kernel/runtime/runtime.t.cpp
cmake --build build/rewrite-debug --target all planar_tests
build/rewrite-debug/src/kernel/planar_runtime_tests '[r07ac03]'
build/rewrite-debug/src/kernel/planar_runtime_tests '[runtime]'
ctest --test-dir build/rewrite-debug -R 'replacement.(runtime|loader|capture|canonical|hash)' --repeat until-fail:2 --output-on-failure --output-junit <repo>/tests/native/r07ac03-evidence/native.xml
python3 scripts/rewrite-check.py lint --build-dir build/rewrite-debug
cmake --build build/rewrite-debug --target all planar_tests
```

Raw logs are named red-build, red-regressions, green-build, green-regressions,
runtime, native-ctest, lint and no-change-build. Successful command logs retain
exit codes and elapsed seconds. Final focused regressions: 2 cases / 43
assertions, 0.117 seconds total. Runtime: 15 cases / 404 assertions. Native CTest:
39 selected cases passed twice in fresh processes, zero failures/skips. native.xml
contains 39 cases with no skipped/notrun/disabled entries; native-ctest-lasttest.log
retains the raw CTest transcript. Pinned formatting/Doxygen passed for 21 active
sources. The final build reports `ninja: no work to do.`

candidate.sha256 freezes the four owned source/document files. Its SHA256 is
1d7431ed57a11f87f084c872bfa1a7a632bbb2da9da0094598681a4130bdf21e.
Hashes were reverified after all checks. Host: macOS arm64 debug. Linux, release,
sanitizers and complete CLI gate were not exercised. No self-approval is claimed.
