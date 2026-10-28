# V5-02-X06-D-C01 gate report (run 3, after the survivor-repair tests)

Verdict: every command met its criterion (passed=true). The gate runner edited no source file. Earlier reports are kept at
raw/report-run1-superseded.md and raw/report-run2-superseded.md; their logs (prefixes `c01-` and `r2-`) are untouched. This run's
logs use the prefix `r3-` under raw/. The task text named the parent evidence directory for the first log; per the evidence rule
all logs are under tests/native/v5-02-x06-d-c01-evidence/raw/ only.

## Candidate identity

- HEAD: `a0a8b3fb7b038cba98e77f1f80e0008353279cf3` (raw/r3-head.txt). The tree is uncommitted.
- sha256 of `git diff HEAD -- src`: `3b45bac8e00f835a1ceb2249d3beab166100e7e13da423fd3bfcf379c4b03e4a` (raw/r3-candidate-src.sha256).
- sha256 of `git diff` (tracked, unstaged): `1424dc80e9e44bcf357f6c930e8fe60896179728602cffc1db9d1ec70ff01e9f` (raw/r3-candidate.sha256);
  full diff raw/r3-candidate.diff.
- The run-2 source hash was `ba700adc...` and the mutation probes (probes/PROBES.md) ran against `ba700adc...`. The candidate has since
  changed (tests only, the "repair of the mutation-probe survivors" in the round record: 12 more tests, 1086 to 1098). The new tests
  are not yet re-probed; the prober must re-run them.
- Round files (git diff names plus untracked docs/src/scripts/cmake files): 26 paths in raw/r3-files.txt.

## Commands, in order (exit codes read from files, never piped)

| step | command | exit | result |
|---|---|---|---|
| G1 | `ctest --test-dir build/rewrite-debug -R 'X06' --no-tests=error --output-on-failure` (raw/r3-g1-focused.log) | 0 | 149 matched, 149 passed, 0 failed, 0 skipped, 65.9 s. Ran first, before any build or full run |
| build | `cmake --build build/rewrite-debug --target planar_tests` (raw/r3-g2-build.log) | 0 | `ninja: no work to do` (binaries already current, so G1 ran on the final candidate) |
| G1b | same focused ctest after the build (raw/r3-g1b-focused-postbuild.log) | 0 | 149 of 149 passed, 0 skipped |
| Debug full | `ctest --test-dir build/rewrite-debug --no-tests=error --output-on-failure` (raw/r3-g3-ctest-debug.log) | 0 | 1098 matched, 1098 passed, 0 failed, 0 skipped, 381.6 s |
| release configure | `cmake --preset release` (raw/r3-g4-cfg.log) | 0 | |
| release build | `cmake --build build/rewrite-release --target planar_tests` (raw/r3-g5-build.log) | 0 | 552 steps, linked |
| Release full | `ctest --test-dir build/rewrite-release --no-tests=error --output-on-failure` (raw/r3-g6-ctest-release.log) | 0 | 1098 matched, 1098 passed, 0 failed, 0 skipped, 154.3 s |
| inventory | `ctest -N` for both configs (raw/r3-inv-debug.txt, raw/r3-inv-release.txt) | 0, 0 | 1098 each, name sets identical (raw/r3-names-*.txt) |
| lint | `python3 scripts/rewrite-check.py lint --build-dir build/rewrite-debug` (raw/r3-g8-lint.log) | 0 | clang-format `--Werror` clean, Doxygen checked 142 active source files |
| full build | `cmake --build build/rewrite-debug` (raw/r3-g9-build.log) | 0 | `ninja: no work to do` |
| no-change build | `cmake --build build/rewrite-debug` again (raw/r3-g10-nochange.log) | 0 | `ninja: no work to do`, zero steps. PASS |
| redact | `python3 scripts/redact-local-paths.py --repo-path "$PWD" --home "$HOME" check <26 files of raw/r3-files.txt>` (raw/r3-g11-redact.log) | 0 | empty output, PASS |

The words "skipped" in the ctest logs occur only inside the test name #610 (`command load refuses forged family order skipped ...`),
which passed. The no-argument redaction form was not run (run 1 showed it only flags ignored local files, raw/c01-g11b-redact-noargs.log).
The full-build step found no work because the planar_tests build had already produced the binaries, so the second build is a no-change
check after a successful build.

## Inventory comparison

- Previous list: `tests/native/v5-02-x06-d-evidence/inventory.txt` (1056 names, the most recent inventory found; raw/r3-prev-sorted.txt).
- New list: `tests/native/v5-02-x06-d-c01-evidence/inventory.txt` (1098 sorted names, `replacement.` prefix); Debug and
  RelWithDebInfo name sets are byte-identical.
- Removed or renamed: 0 (raw/r3-removed.txt empty). Added: 42 (raw/r3-added.txt), all prefixed `replacement.X06 D C01 `. Locations:

  - runtime/runtime.t.cpp:3758 `X06 D C01 a caught Lua error still leaves the budget counters monotone`
  - capture/native.t.cpp:1054 `X06 D C01 a single discovery entry refuses at the composed ceiling`
  - capture/admission.t.cpp:991 `X06 D C01 admission charges the factory budget exactly once per charge`
  - runtime/runtime.t.cpp:3729 `X06 D C01 admission metadata charges are part of the factory counter at open`
  - capture/admission.t.cpp:927 `X06 D C01 admission metadata charges reach the factory budget`
  - capture/admission.t.cpp:947 `X06 D C01 admission refuses at the composed ceiling with the existing resource message`
  - capture/native.t.cpp:993 `X06 D C01 ancestor staging is charged with each retained file`
  - capture/native.t.cpp:915 `X06 D C01 capture charges the composed ceiling before reading or allocating a file`
  - capture/capture.t.cpp:772 `X06 D C01 capture session threads the factory budget through declarations and reads`
  - runtime/runtime.t.cpp:3650 `X06 D C01 declaration chunks exhaust the composed invocation instruction ceiling across stages`
  - runtime/runtime.t.cpp:3820 `X06 D C01 declaration live Lua memory is released between chunks`
  - runtime/runtime.t.cpp:3709 `X06 D C01 declaration Lua memory counts against the composed live Lua ceiling`
  - capture/native.t.cpp:944 `X06 D C01 discovery staging is charged to the factory budget and never refunded`
  - capture/native.t.cpp:959 `X06 D C01 discovery staging refuses at the composed ceiling`
  - lua/loader.t.cpp:124 `X06 D C01 loader charges instructions to the factory budget`
  - lua/loader.t.cpp:152 `X06 D C01 loader charges retained declaration values to the factory budget byte for byte`
  - lua/loader.t.cpp:193 `X06 D C01 loader charges the final retained declaration and references to the factory budget`
  - lua/loader.t.cpp:171 `X06 D C01 loader declaration Lua memory is live against the factory budget and released on close`
  - lua/loader.t.cpp:141 `X06 D C01 loader inherited chunk limits still win over the composed budget`
  - lua/loader.t.cpp:131 `X06 D C01 loader refuses at the composed instruction ceiling with the owner diagnostic`
  - lua/loader.t.cpp:182 `X06 D C01 loader refuses at the composed live Lua ceiling with the owner diagnostic`
  - lua/loader.t.cpp:160 `X06 D C01 loader refuses at the composed native ceiling and reports the owner diagnostic`
  - lua/loader.t.cpp:210 `X06 D C01 loader refuses one oversized value even when every later charge fits`
  - runtime/runtime.t.cpp:3749 `X06 D C01 native boundary is exercised by the post-validation derivation charges`
  - capture/native.t.cpp:930 `X06 D C01 native read charges each retained file exactly once`
  - runtime/runtime.t.cpp:3831 `X06 D C01 opaque factory charges module reads like the known-load factory`
  - capture/profile.t.cpp:982 `X06 D C01 profile capture charges the profile read to the factory budget`
  - capture/profile.t.cpp:955 `X06 D C01 profile capture threads the factory budget through admission and supplied reads`
  - capture/native.t.cpp:1025 `X06 D C01 refused capture allocates nothing near the file size`
  - runtime/runtime.t.cpp:3848 `X06 D C01 repeated direct shape calls never refund native bytes`
  - capture/admission.t.cpp:962 `X06 D C01 retained declaration handle charges accumulate on every issue without refund`
  - runtime/runtime.t.cpp:3690 `X06 D C01 retained declaration values exhaust the composed native ceiling in the declaration stage`
  - acceptance/source_capture.t.cpp:1099 `X06 D C01 source capture bundle charges refuse at their own ceiling`
  - acceptance/source_capture.t.cpp:1042 `X06 D C01 source capture charges the bundle copy of every captured file`
  - acceptance/source_capture.t.cpp:1056 `X06 D C01 source capture refuses at the composed ceiling as resource_limit`
  - acceptance/source_capture.t.cpp:1016 `X06 D C01 source capture threads one budget through workspace spec and job declarations`
  - runtime/runtime.t.cpp:3629 `X06 D C01 spec and job declaration instructions join the invocation counter`
  - runtime/runtime.t.cpp:3786 `X06 D C01 the composed sink alone enforces the invocation instruction ceiling on declaration chunks`
  - ledger/acceptance.t.cpp:1238 `X06 D C01 variant ceiling mechanism does not depend on runtime source text`
  - capture/native.t.cpp:1070 `X06 D C01 verification charges only the enumeration staging`
  - capture/native.t.cpp:974 `X06 D C01 verification staging is charged on every verify and never refunded`
  - runtime/runtime.t.cpp:3615 `X06 D C01 workspace declaration instructions are carried into the invocation counter`

## (a) Acceptance criteria and findings mapped to tests

Limit (unchanged in run 3): the round record's invariant table is still truncated after row T3 ("T4 onward, G1 to G5: not delivered") and the record
says "Outcome: Pending". Rows are therefore mapped from the delivered rows, the parent record B1 to B8 and G1 to G5, the review
findings R1 to R5 and the red names. Paths are under src/engine/. All listed tests passed in both configurations.

| criterion | tests (file:line) |
|---|---|
| B1 one owner through every stage | capture/capture.t.cpp:772 `capture session threads the factory budget through declarations and reads`; capture/profile.t.cpp:955 `profile capture threads the factory budget through admission and supplied reads`; acceptance/source_capture.t.cpp:1014 `source capture threads one budget through workspace spec and job declarations`; lua/loader.t.cpp:124 `loader charges instructions to the factory budget`; lua/loader.t.cpp:152 `loader charges retained declaration values to the factory budget byte for byte`; lua/loader.t.cpp:171 `loader declaration Lua memory is live against the factory budget and released on close`; capture/admission.t.cpp:927 `admission metadata charges reach the factory budget`; capture/native.t.cpp:944 `discovery staging is charged to the factory budget and never refunded`; runtime/runtime.t.cpp:3729 `admission metadata charges are part of the factory counter at open`; runtime/runtime.t.cpp:3615 `workspace declaration instructions are carried into the invocation counter`; runtime/runtime.t.cpp:3629 `spec and job declaration instructions join the invocation counter` |
| B2 stricter limits win, ceilings charged before allocation | lua/loader.t.cpp:141 `loader inherited chunk limits still win over the composed budget`; capture/native.t.cpp:915 `capture charges the composed ceiling before reading or allocating a file`; capture/native.t.cpp:930 `native read charges each retained file exactly once`; acceptance/source_capture.t.cpp:1040 `source capture charges the bundle copy of every captured file`; capture/admission.t.cpp:947 `admission refuses at the composed ceiling with the existing resource message` |
| B3 exhaustion is resource_limit, terminal, nested roots cleaned | lua/loader.t.cpp:131, 160, 182 (composed instruction, native and live Lua refusals with owner diagnostic); capture/native.t.cpp:959 `discovery staging refuses at the composed ceiling`; acceptance/source_capture.t.cpp:1054 `source capture refuses at the composed ceiling as resource_limit`; runtime/runtime.t.cpp:3650 `declaration chunks exhaust the composed invocation instruction ceiling across stages`; runtime/runtime.t.cpp:3690 `retained declaration values exhaust the composed native ceiling in the declaration stage`; runtime/runtime.t.cpp:3709 `declaration Lua memory counts against the composed live Lua ceiling`. No registered test is named for "between stages" or "after the all-job phases" separately (record rows T8 and T9, never registered); the three runtime tests above are the only stage-level exhaustion coverage. |
| B4 caught failure keeps first diagnostic (semantic half, I10) | runtime/runtime.t.cpp:3758 `a caught Lua error still leaves the budget counters monotone` (repaired; see section c). Survivors I10 and E25a to c were recorded observably redundant in the parent record |
| B5 moves, wrapper destruction | existing tests, unchanged: `X06 D moving the prepared profile transfers counters and move assignment is safe`, `X06 D destroying the consumed owner wrapper cannot erase prepared counters` |
| B6 no reset, no refund (handle, comparison, discovery staging) | capture/admission.t.cpp:962 `retained declaration handle charges accumulate on every issue without refund`; capture/native.t.cpp:974 `verification staging is charged on every verify and never refunded`; capture/native.t.cpp:993 `ancestor staging is charged with each retained file`; runtime/runtime.t.cpp:3242 `X06 D each custom shape dispatch adds to the one budget and nothing is refunded` (repaired) |
| B7 boundary exact and one-over | the exhaustion tests under B3 plus existing X06 D boundary tests; runtime/runtime.t.cpp:3749 `native boundary is exercised by the post-validation derivation charges` (this added name has no row in the record's table) |
| B8 public surface unchanged | ledger/acceptance.t.cpp:1238 `variant ceiling mechanism does not depend on runtime source text`; existing `X06 D runtime public surface and resource messages stay closed`, `X06 D prepared_profile has no new construction path`, `X06 C C03 seam is a non-exported partition of the acceptance module`, `X06 D composed ceiling constants keep their contract values` |
| G1 focused before full | raw/r3-g1-focused.exit 0, 149 passed, run first (binaries already current) |
| G2 Debug | 1098 of 1098, 0 skipped, 1056 names preserved |
| G3 RelWithDebInfo | 1098 of 1098, 0 skipped, identical name set |
| G4 lint and no-work build | lint exit 0; second build reports no work |
| G5 prior evidence preserved | no raw log deleted; run-1 logs and report kept |
| R1 thread one owner through every stage | B1 row |
| R2 charge before allocation | capture/native.t.cpp:915 |
| R3 G1 before full runs, order recorded | table above (G1 ran first) |
| R4 complete invariant table (rows G1 to G5, B3 stage clauses, B4 half, B6 rows, fix I3 and I6) | NOT MET in the record text (still truncated after T3); test side partly covered as above; I3 and I6 fixes are not verifiable from the gate |
| R5 README index, stale REPORT statements, FATAL_ERROR guard | ledger/acceptance.t.cpp:1238 passes; README index and stale-statement corrections were not checked by this gate (this runner found no README in tests/native/v5-02-x06-d-evidence/ in run 1) |

## (b) Red-first

Source: red/RED.md, red/results.tsv and red/raw/<id>.log. Red snapshot `red/snapshot-2`: HEAD `a0a8b3fb...`, `candidate.diff`
sha256 `dbec1665f94737b2522c8224fad6fd9396b21f4fa8d4f5208b70e888841cf35d`, SHA256SUMS sha256
`4c28ead11056fe022e6edf25fe423d50722b56dea9a76157fcc246823041a92c`; no untracked files existed then. Build: `cmake --build
build/rewrite-debug --target planar_tests` exit 0. Command per test, as written in RED.md:
`ctest --test-dir build/rewrite-debug -R '^replacement\.<escaped name>$' --no-tests=error --output-on-failure`.
Failing lines are behavioural assertion failures (rc 8), not compile errors. Line numbers are those of the red snapshot and have since
shifted (current lines in section a).

| id | test | outcome | failing line in red |
|---|---|---|---|
| L1 | loader charges instructions | real red | lua/loader.t.cpp:127 |
| L2 | loader refuses at composed instruction ceiling | real red | loader.t.cpp:134 |
| L3 | loader inherited chunk limits still win | guard, passed | none |
| L4 | loader charges retained values byte for byte | real red | loader.t.cpp:156 |
| L5 | loader refuses at composed native ceiling | real red | loader.t.cpp:164 |
| L6 | loader declaration Lua memory live and released | real red | loader.t.cpp:175 |
| L7 | loader refuses at composed live Lua ceiling | real red | loader.t.cpp:189 |
| L8a to c | three existing loader tests | guards, passed | none |
| A1, A2, A3 | admission charges, refusal, handle accumulation | real red | capture/admission.t.cpp:932, 955, 983 |
| N1 to N6 | native pre-allocation charge, read once, discovery, refusal, verify, ancestor | real red | capture/native.t.cpp:924, 938, 952, 967, 988, 998 |
| C1 | capture session threads budget | real red | capture/capture.t.cpp:783 |
| P1 | profile capture threads budget | real red | capture/profile.t.cpp:963 |
| S1, S2, S3 | source capture budget, bundle copy, resource_limit mapping | real red | acceptance/source_capture.t.cpp:1020, 1051, 1060 |
| T1, T2 | pre-frame and spec/job instructions carried | real red | runtime/runtime.t.cpp:3608, 3624 |
| T3 | declaration chunks exhaust invocation ceiling | real red | runtime.t.cpp:3649 |
| T4 | retained declaration values exhaust native ceiling | real red | runtime.t.cpp:3106 |
| T5 | declaration Lua memory against live ceiling | real red | runtime.t.cpp:3703 |
| T6 | admission metadata part of factory counter | real red | runtime.t.cpp:3727 |
| T7 | existing `X06 D capture stage charges every sealed byte to the one native counter` | real red | runtime.t.cpp:3205 |
| T8, T9 | between-stage and after-all-job exhaustion | NOT REGISTERED in the red run; no test registered since | none |
| T10 | caught Lua error counters monotone | registered but not run in red (RED.md "NOT REGISTERED" row); it failed in gate run 1 and was repaired | none recorded |
| T11, T13 to T18, B8a to d | existing X06 D guards | guards, passed | none |
| T12 | custom shape dispatch adds to the budget | guard that failed in red at runtime.t.cpp:3263 (never green); repaired | 3263 |
| V1 | variant ceiling independent of runtime source text | real red | ledger/acceptance.t.cpp:1244 |

Missing from the coders' evidence: a red row or command for `native boundary is exercised by the post-validation derivation
charges`; any red evidence for the repaired tests after the repair (the repair record says only that they now pass and names
mutants I10 and I3e without a probe log); T8 and T9 tests. No command was invented.

## (c) Deliberate behaviour differences

- R2: an over-ceiling file that is also invalid UTF-8 refuses with `resource_limit` rather than `schema`, because the charge is
  made in `native.cpp` `read_file` before the read (record, "R2 timing"). The record cites no contract clause permitting it; the
  dispatch names the X-LIMIT check-before-allocation rule. Reviewer to confirm.
- Pre-frame declaration instructions are carried into the invocation counter, not reset (record row 6b); registration (10,000,000)
  and invocation (100,000,000) ceilings stay separate X-LIMIT rows. The record lists this as halt candidate (a) with no operator
  decision recorded.
- Declaration live Lua memory is live acquire and release, not cumulative (record row 1c), halt candidate (b), no operator decision.
- Test expectation changes in the repair, no product change claimed: `X06 B3 A factory registration counters are deterministic
  across factories` (runtime/runtime.t.cpp:1315) and `X06 B3 B owner implementation allocation is charged identically by both
  factories` (:1675) now assert known-load native bytes greater than opaque, because the known-load factory charges admission
  metadata (B1). The 6885-byte difference is therefore explained in the repair record as intended B1 behaviour, though it changes
  the old equality; reviewer to confirm. Names are unchanged.

## (a2) Rows added by the survivor repair (not in the section (a) table above)

Loader: `loader charges the final retained declaration and references` (loader.t.cpp:193), `loader refuses one oversized value even
when every later charge fits` (:210). Admission: `admission charges the factory budget exactly once per charge` (admission.t.cpp:991).
Native: `refused capture allocates nothing near the file size` (native.t.cpp:1025; R2 before-allocation test), `a single discovery
entry refuses at the composed ceiling` (:1054), `verification charges only the enumeration staging` (:1070). Profile: `profile
capture charges the profile read to the factory budget` (profile.t.cpp:982). Source capture: `bundle charges refuse at their own
ceiling` (source_capture.t.cpp:1099). Runtime: `the composed sink alone enforces the invocation instruction ceiling on declaration
chunks` (runtime.t.cpp:3786), `declaration live Lua memory is released between chunks` (:3820), `opaque factory charges module reads
like the known-load factory` (:3831), `repeated direct shape calls never refund native bytes` (:3848). The round record names the
mutant each should kill (section "Repair of the mutation-probe survivors"); no red-first or probe evidence exists yet for these
twelve tests, since they were added after the probes ran.

## Limitations

- The record's invariant table is still truncated (R4), so row-level mapping beyond T3 is inferred.
- T8 and T9 have no registered tests. Mutation probes were not run by the gate.
- G1 ran before the build step as ordered; the build was a no-op so G1 covered the final binaries, and G1b repeats it.
- Test #531 timing was not measured separately; no timeouts occurred (Debug full 381.6 s, Release 154.3 s).
- The no-argument redaction run was not repeated in runs 2 or 3.
- The twelve survivor-repair tests (section a2) are unprobed and have no red evidence; the candidate hash differs from the probed one.
