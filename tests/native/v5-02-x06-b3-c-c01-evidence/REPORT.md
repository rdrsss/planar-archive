# V5-02-X06-B3-C-C01 gate report (second gate run, after the repair round)

Result: PASS (passed=true). No source file was edited by the gate runner. This run supersedes the first gate run (1002 tests, before the repair test was added); the first report is kept at raw/REPORT-r1-superseded.md and its logs are untouched. Logs of this run carry the suffix -r2 in raw/.

## Candidate identity

- HEAD: 7110196d87c473439d33c9174a8c90218477a805 (raw/head-r2.txt)
- git diff HEAD -- src, sha256: ea5f45376e9cd9473a57060f99a1b7adc41c38ddb165265693ba30a6c9c5daae (raw/hash-src-r2.txt). It differs from the first gate run (e5dfafab...) because the repair round added a test to runtime.t.cpp.
- unrestricted git diff HEAD, sha256: 33cff6430a030050c9e3ffe32088b0fbc04dbb6565df17ae134091da390abfde (changes with every record edit)
- src/engine/runtime/runtime.t.cpp, sha256: 8fedecbc2874f3280dbac17c8c6609a8e0b4e668fb5b9ec7f4b621834416433e

## Commands

| # | Command | Exit | Result |
| --- | --- | --- | --- |
| 1 | cmake --build build/rewrite-debug --target planar_tests | 0 | "ninja: no work to do." The tree was already built from the repaired runtime.t.cpp (repair coder built it); raw/build-r2.log |
| 2 | ctest --test-dir build/rewrite-debug --no-tests=error --output-on-failure | 0 | 100% tests passed, 0 failed out of 1003; Total Test time (real) 357.59 s; no Skipped or Not Run entries; raw/ctest-r2.log |
| 2a | test #531 wall time | n/a | #531 (the stride sweep test) Passed 15.40 s (limit 30 s; first gate run 16.35 s). New test #533 (in-place edit sweep) Passed 10.27 s. |
| 3 | ctest -N inventory comparison | 0 | Total Tests: 1003; 1 new, 0 removed against the 1002 names of the B3-C candidate; raw/ctest-N-r2.log |
| 4 | python3 scripts/rewrite-check.py lint --build-dir build/rewrite-debug | 0 | "Formatting and Doxygen checked 136 active source files". Doxygen succeeded on attempt 2 of 3; attempt 1 died from a signal with no diagnostic (the known upstream doxygen 1.18.0 SIGBUS flake); raw/lint-r2.log |
| 5a | cmake --build build/rewrite-debug (first) | 0 | "ninja: no work to do." raw/all1-r2.log |
| 5b | cmake --build build/rewrite-debug (second) | 0 | "ninja: no work to do." zero steps; raw/all2-r2.log |
| 6 | python3 scripts/redact-local-paths.py check --repo-path $PWD --home $HOME on the 16 edited or created files (raw/files-r2.txt), README.md, REPORT.md, inventory.txt and the B3-C PROBES.md | 0 | clean; raw/redact-files-r2.log. The no-argument form is an argparse usage error (exit 2, first run, raw/redact-nofiles.log) |

Caveat on 1 and 5: the tree was already fully built, so no build in this run compiled anything. The no-work checks hold but were not preceded by a build that did work. Test counts: 1003 matched, 1003 passed, 0 skipped, 0 failed.

## Inventory

inventory.txt holds the 1003 sorted names from raw/ctest-N-r2.log. Compared against the most recent committed inventory, tests/native/v5-02-x06-b3-c-evidence/inventory.txt (1002 names). New (raw/new-r2.txt): "replacement.X06 B3 C an in place same size edit at any allocation inside prepare_load is never published as verified". Removed (raw/removed-r2.txt): none. All 1002 prior names preserved.

## (a) Acceptance criteria and findings to tests

Line numbers are in src/engine/runtime/runtime.t.cpp.

| Criterion or finding | Test or evidence | Location |
| --- | --- | --- |
| 1. Stride total/40, tail step 40, name unchanged, test fails on receipt-skip or publish-before-verify | "X06 B3 C a file replaced at any allocation inside prepare_load is never published as verified"; stride loop line 2685, tail step 40 at line 2709. Prober kills (raw probe logs): P03, P03s, PB1, PB1s, PB2, V3b, V3bs | runtime.t.cpp:2665 (test), :2685, :2709; probes/PROBES.md |
| 2. Totals, positions, phases, #531 time | Total allocations 18863, stride 471, 41 whole-call plus 16 tail plus one beyond-end = 58 positions per target (121 for closure-member targets); phases not instrumented (stated limit); #531 15.40 s here | probes/PROBES.md "Measurements for R1" |
| 3. R2, N1 to N4 recorded, no diagnostic change, no product source change | Records in V5-02-X06-B3-C.md, V5-02-X06-B3-C-C01.md, B3-C PROBES.md (checked by records coder and prober, not re-verified line by line by the gate). N4: confirmed by the prober (no receipt-skip mutant lists the "replacement before publication" test, runtime.t.cpp:2522) | round records |
| 4. 1002 names preserved, zero skips, zero failures | 1003 of 1003 passed; new.txt has the one repair test, removed.txt empty | raw/ctest-r2.log |
| Repair finding P01 (source reverification skip unreachable by rename sweep) | New test "X06 B3 C an in place same size edit at any allocation inside prepare_load is never published as verified" | runtime.t.cpp:2768 (stride total/20 at :2790, tail at :2825) |
| N4 (by construction, unchanged) | "X06 B3 C replacement before publication refuses terminally and publishes nothing" | runtime.t.cpp:2522 |
| N1 codec reserved shapes | Prober kills C1, C2 | probes/PROBES.md |
| N2 moved-from message | Open item, no change; verify tests V3b killed by "X06 B3 C verify_native_receipts ..." | runtime.t.cpp:2462, :2488 |

## (b) Red-first

This is a test-tightening round with no product change, so there is no behavioural red snapshot for the stride change (R1): a total/24 versus total/40 stride has no product mutant whose verdict depends on it (prober: R1-stride-only passes), so it is a coverage restoration and not a guard that fails. Not run and not available in the evidence: a red-first command with a pre-fix snapshot hash for the new test #533. The repair record states its expected kill (mutant P01) and says the coder did not apply it; I found no prober log that applies P01 to the repaired candidate with test #533 present (the prober ran before the repair and P01 survived there). That kill is therefore unconfirmed by evidence and needs a prober re-run of P01 against the new test. The remaining kills listed in (a) are from the prober on the pre-repair candidate.

## (c) Deliberate behaviour differences

None. No product source changed and no diagnostic text changed.

## Limitations

- macOS Debug only; no Linux run.
- Builds were no-ops (tree pre-built).
- Probe results predate the repair test; P01 against test #533 is unverified.
- Wall time of #531 is 15 to 16 s; under heavy load it once hit the 30 s ctest timeout in scratch (flake risk, PROBES.md).
- Allocation counts per phase are not measured.
