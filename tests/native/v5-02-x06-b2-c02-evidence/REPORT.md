# V5-02-X06-B2-C02 gate report

Gate runner only; no source file was edited. Debug preset, build directory build/rewrite-debug. Raw logs are under raw/.

## Candidate identity

HEAD bcfe1ad7ed94a85985f1189deba3b12cc851e7bd. sha256 of `git diff` (tracked changes only): 7dc155c0ac55e0c9b88bdb70341246d63c78e6413835b70d0618f12ddc404fb6. The untracked files (source_capture.*) are covered by the per-file hashes in raw/sha256.log:

    db0e6dfdfd7e6f03adfce25c87eaf15e71f8c5ddf15cc7de4dbadc0396545cc9  src/engine/acceptance/source_capture.cpp
    58865a5a328b0d157cc9bd0f6621a0601487e10feea2b3126cd8f353b2e2791e  src/engine/acceptance/source_capture.cppm
    3924ecc0c0ff9aed37a858d7d8d78908dbba961c2accc0c59aedfb2a1934e182  src/engine/acceptance/source_capture.t.cpp
    caf038172725875e88e26800733467aa26cc1e06bd1286e11579c41bc25412be  src/engine/acceptance/load.cpp
    74721d41994951236a70388730e7e30dcca424559f382b6765c653f2fc1adfca  src/engine/CMakeLists.txt
    7886d7407df2679a90b234da46b01e7f7f711fa6c3e3c418eb208b26c77bd109  src/engine/capture/admission.cpp
    b289112661a17394d083f3b853c4f5bf4c96efac3bc0dc46ec7897acfb34cf2f  src/engine/capture/admission.cppm
    75ffa7bed2bebc3f1c1d8f1141cfb812971447509f03d55b08805e1ef52b9ef1  src/engine/capture/admission.t.cpp

## Commands

1. sha256sum of the eight listed files: exit 0.
2. cmake --preset debug: exit 0 (raw/configure.log).
3. cmake --build build/rewrite-debug --target planar_tests: exit 0, 532 of 533 steps shown, all linked (raw/build-planar_tests.log).
4. ctest --test-dir build/rewrite-debug --no-tests=error --output-on-failure: exit 0, "100% tests passed out of 940". No failed, not-run or disabled entries. The only "skip" matches in the log are two test names containing the word skipped/skip; no test was skipped (raw/ctest.log).
5. ctest -N: exit 0, Total Tests: 940 (raw/ctest-N.log). Matched 940 equals registered 940.
6. cmake --build build/rewrite-debug (first no-change check, after the successful build): exit 0 but it was NOT zero work. It printed "Re-checking globbed directories" and "[1/4] Generating CXX dyndep file src/cmd/planar/CMakeFiles/planar.dir/CXX.dd", i.e. ninja ran steps because the default `all` target (planar binary) had not been built by the planar_tests build (raw/build-nochange.log). By the stated criterion this is a FAILURE.
7. cmake --build build/rewrite-debug (second): exit 0, "ninja: no work to do." Zero steps (raw/build-nochange2.log). This is informational; it follows the first, so it does not satisfy the "separate successful build then check" criterion on its own.

## Inventory

inventory.txt holds the 940 sorted test names. Compared with the most recent committed-or-present earlier list, tests/native/v5-02-x06-b2-c01-evidence/inventory.txt (940 names): identical. New names: none. Removed names: none.

## Limitations

Overall passed=false only because of command 6. Tests, build and inventory are green. Format/Doxygen lint (make test-all / cpp-lint) and RelWithDebInfo were not part of the assigned commands and were not run. Brief listed only sha256sum explicitly; the build/ctest commands mirror the C01 evidence sequence. The git diff hash excludes untracked files.

## Repair addendum (2026-10-24)

Cause of command 6 failing: the gate built only `planar_tests`, which does not include the default `all` target, so the first `cmake --build build/rewrite-debug` legitimately generated the planar binary dyndep. No source, test or documentation under the candidate changed; the eight sha256 values above and the `git diff` sha256 were re-verified identical.

Corrected sequence run by the repair coder, all exit 0:

1. `cmake --build build/rewrite-debug --target planar_tests`: "ninja: no work to do." (everything already built).
2. `cmake --build build/rewrite-debug` (default all, now built): "ninja: no work to do."
3. `cmake --build build/rewrite-debug` again: "ninja: no work to do." Zero steps; the no-change criterion holds.
4. `python3 scripts/rewrite-check.py lint --build-dir build/rewrite-debug`: exit 0, "Formatting and Doxygen checked 136 active source files" (pinned clang-format --dry-run --Werror plus Doxygen; raw/lint.log), including source_capture.* and the modified admission/load files.

The ctest result (940/940, zero failures/skips) from command 4 above stands; candidate bytes are unchanged since that run. Future gates should run `cmake --build build/rewrite-debug` (all) before the no-change check. RelWithDebInfo was not run.

## Final confirmation run (gate runner, 2026-10-24)

The eight sha256 values were recomputed and are identical to raw/sha256.log. `cmake --build build/rewrite-debug` exit 0 (raw/gate2-build.log), then a separate no-change build exit 0 reporting only "Re-checking globbed directories" and "ninja: no work to do." with zero steps (raw/gate2-nochange.log). `ctest -N` exit 0, Total Tests: 940 (raw/gate2-ctest-N.log), unchanged from inventory.txt. The full ctest run (940/940, zero failures/skips) was not repeated because candidate bytes are unchanged since it ran; that result stands from raw/ctest.log. Lint (136 files) is from the earlier repair run, not repeated.
