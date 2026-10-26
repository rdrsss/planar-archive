# V5-02-X06-B3-C gate report (fourth gate run)

Result: PASS (passed=true). Every command met its criterion. No source file was edited by the gate runner. The formerly timing-out test, "X06 B3 C a file replaced at any allocation inside prepare_load is never published as verified" (ctest #531), passed in 13.24 s (limit 30 s; previous gate 12.94 s). Raw logs for this run are in raw/g3/. Earlier runs are untouched in raw/ and raw/prev-run-*; the previous REPORT.md is copied to raw/g3/prev-REPORT.md.

## Candidate identity

- HEAD: 7110196d87c473439d33c9174a8c90218477a805 (dev/planar-v2-vernacular), raw/g3/head.txt.
- sha256 of git diff (tracked changes): 78b1deb4ea855d6bbfca6648048c941af59c09db58f68f6b616714249251517f (raw/g3/diff-sha.txt), identical to the previous gate and re-checked after the run, so the candidate did not change.
- Edited files (raw/g3/status.txt): src/cmd/planar/CMakeLists.txt, src/engine/CMakeLists.txt, src/engine/acceptance/load.cpp, source_capture.cpp, source_capture.cppm, source_capture.t.cpp, src/engine/history/interpretation_v2_codec.cpp, interpretation_v2_codec.t.cpp, src/engine/runtime/runtime.cpp, runtime.cppm, runtime.t.cpp; untracked docs/vernacular/rounds/V5-02-X06-B3-C.md and this evidence folder.

## Commands (serial, nothing piped, exit codes read from raw/g3/*.exit)

| # | Command | Exit | Result |
|---|---|---|---|
| 1 | cmake --build build/rewrite-debug --target planar_tests | 0 | ninja: no work to do (binaries already current from the earlier builds; raw/g3/01-build.log) |
| 2 | ctest --test-dir build/rewrite-debug --no-tests=error --output-on-failure | 0 | 100% tests passed, 0 failed out of 1002 matched; 355.61 s real. No skipped or disabled tests: the only "skipped" text is the name of test #559 ("...forged family order skipped duplicate..."), which passed. #531 passed in 13.24 s (raw/g3/02-ctest.log) |
| 3 | ctest --test-dir build/rewrite-debug -N | 0 | Total Tests: 1002; 987 baseline names kept, 0 removed, 15 new (raw/g3/03-ctest-N.log, new.txt, removed.txt) |
| 4 | python3 scripts/rewrite-check.py lint --build-dir build/rewrite-debug | 0 | Formatting and Doxygen checked 136 active source files. Doxygen died with signal 138 on attempt 1 (known upstream 1.18.0 SIGBUS flake with no diagnostic) and succeeded on attempt 2 of 3; the script's own retry, final exit 0 (raw/g3/04-lint.log) |
| 5a | cmake --build build/rewrite-debug | 0 | ninja: no work to do |
| 5b | cmake --build build/rewrite-debug (second) | 0 | ninja: no work to do, zero steps. PASS |
| 6 | scripts/redact-local-paths.py --repo-path <repo> --home <home> check <11 edited files + round record> | 0 | clean (raw/g3/06-redact.log). Run with no arguments it is a usage error, exit 2 (raw/g3/06b-redact-noargs.log); the file-argument run is the meaningful one |

Limitation on the no-change check: because the tree was already built, build 5a also did no work, so no build in this gate actually compiled anything. The zero-work result holds after the earlier successful builds recorded in raw/prev-run-gate2b/ and the repair build, not after a fresh compile in this gate.

## Inventory

Baseline: p4-inventory-base-sorted.txt (987 names, the B3-B C04 inventory, the most recent committed-lineage inventory list in this evidence). Current: inventory.txt (1002 sorted names from ctest -N; identical to the previous gate's inventory.txt). Removed: none. New, 15 (raw/g3/new.txt):

- replacement.source capture correspondence refuses an unsealed or released source as mismatch
- replacement.source capture correspondence refuses every single bundle tamper as mismatch
- replacement.source capture correspondence returns the exact sorted workspace paths for a good bundle
- replacement.X06 B3 C a file replaced at any allocation inside prepare_load is never published as verified
- replacement.X06 B3 C allocation failure after validation maps to a resource refusal and leaves nothing retryable
- replacement.X06 B3 C derivation is charged to the one frame and counters never reset
- replacement.X06 B3 C derived interpretation decodes against the raw effective tree with equal separate versions
- replacement.X06 B3 C each derivation charge scales exactly with the bytes it covers
- replacement.X06 B3 C published data is immutable shared and independent of the prepared object
- replacement.X06 B3 C published workspace id follows the declaration and source modes are normalised
- replacement.X06 B3 C replacement before publication refuses terminally and publishes nothing
- replacement.X06 B3 C success consumes the owner, refuses retry and moved-from prepared objects refuse
- replacement.X06 B3 C success publishes source registry effective tree and bound intent for each fixture shape
- replacement.X06 B3 C verify_native_receipts fails with capture_changed after any captured input changes post publication
- replacement.X06 B3 C verify_native_receipts is repeatable on an untouched prepared profile and reads without consuming

The full registered inventory is in inventory.txt next to this report.

## (a) Acceptance criteria to tests

Line numbers are TEST_CASE lines in the current checkout.

| Criterion | Tests (file:line) |
|---|---|
| 1 real profile, built-in and custom jobs, data(), v2 interpretation, separate V equals embedded | src/engine/runtime/runtime.t.cpp:2245 (success shapes), :2312 (decodes against raw tree, equal versions), :2859 (workspace id, source modes) |
| 2 no caller construction, data() copies are not authority, move-only, failed and consumed publish nothing | runtime.t.cpp:2339 (immutable, shared, independent), :2398 (consumes, retry refused, moved-from refuse); existing B3-B "opaque consumed failed and moved-from owners refuse prepare_load". Absence of caller-bundle APIs is a compile-time property, untested |
| 3 failure order, verify_native_receipts | runtime.t.cpp:2508 (replacement before publication), :2448 (repeatable, no consume), :2474 (capture_changed after post-publication change), :2648 (replacement at any allocation, now 12.94 s) |
| 4 consumed before fallible copy, bad_alloc, one frame no reset, final catch | runtime.t.cpp:2751 (allocation failure), :2552 (one frame, no reset), :2825 (charges scale with bytes), :2398. Final catch arm unprobed (limit N5); consumed-mark ordering has no observer (P04/P05 unkilled) |
| 5a falsifiable zero-declaration-read check | runtime.t.cpp:1548 "X06 B3 A into_extension adopts the same VM and receipts without recapture" (name kept, positive control added) |
| 5d N1 | runtime.t.cpp:1678 (owner allocation charge observable) |
| 5d N3 | cited test at src/engine/capture/native.t.cpp:782 exists; probe citation is the prober's (probes/PROBES.md) |
| 6 987 names preserved, lint, diagnostics, cycle | 0 removed (above); lint exit 0; this gate did not diff diagnostic messages or run a dependency-cycle audit |
| shared correspondence | src/engine/acceptance/source_capture.t.cpp:781, :806, :938 |
| codec shipped shape without catalog entry (repair), C2 | src/engine/history/interpretation_v2_codec.t.cpp:277 TEST_CASE, added block at lines 309 to 315 |
| repair: P08b, P13c | runtime.t.cpp:2859 |
| repair: P11a to P11d, P10c | runtime.t.cpp:2825 |
| repair: S10, S14, S15, S16 | source_capture.t.cpp:806 |
| repair: T1 | runtime.t.cpp:1548 |
| repair: P01 dense tail sampling and the 531 timeout | runtime.t.cpp:2648 |
| 7 mutation probes | owned by the prober, probes/PROBES.md (its results predate the latest repair, so they do not cover the repaired tests). Not run by this gate |

The round record names no corrective findings beyond the two repair sections, which are mapped above.

## (b) Red-first

Only one red-first record exists in the evidence: the P4 run of the full suite against the pre-repair runtime.cpp (24 failures, p4-ctest.log, raw/prev-run-1049/). Command: ctest --test-dir build/rewrite-debug --no-tests=error --output-on-failure. Pre-fix snapshot raw/redfirst-prefix-runtime.cpp.snapshot, sha256 b0af42c718eaa7c85493dc29319dbcf309084c23eee21e4ef3fbecbb0e78b8c3 (verified again by this gate). Failing line: the first REQUIRE on a successful prepare_load in "X06 B3 C success consumes the owner..." plus 23 others. These were real reds for the ten original X06 B3 C runtime tests and the B3-A and B3-B tests that need a successful prepare_load.

Missing, not invented: no red command or pre-fix hash exists for the three source capture correspondence tests, the codec shipped-shape test block, the two survivor-repair tests (runtime.t.cpp:2825 and :2859), the positive control at :1548, the extra correspondence tampers, or the sampling-constant change. Those tests are, as far as the evidence shows, guards that were written against already-correct product code; whether each can fail rests on the prober's mutants (several of which were survivors before the repair), and no prober re-run after the repair is present. Also note that runtime.cpp changed after the recorded pre-fix hash although the survivor-repair record says no product code changed; the reviewer should reconcile.

## (c) Deliberate behaviour differences

- prepared_profile::observe() returns the unchanged profile_load_stage_observation. Permitted by the round record's Allowance (amends criterion 5(b) for this one method).
- interpretation_v2_codec verify_correspondence treats a job naming a reserved shipped shape (pipeline, fanout_review) with no catalog entry as a shipped job without a selection; unknown shapes still refuse. The permitting contract clause is not named in the evidence; reviewer to confirm.

## Limitations

macOS Debug only; no RelWithDebInfo, Linux, RSS or whole-invocation budget evidence; no mutation probes by this gate and none re-run after the latest repair; no dependency-cycle audit; diagnostic byte-identity not diffed; N4 unreachable branches and the N5 final catch have no observers; the no-change build check 5a did no work either because everything was already built, so the second build proves zero-work only after the earlier successful builds recorded in raw/prev-run-gate2b/.
