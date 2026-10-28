# V5-02-X06-D gate report (gate runner, run 3, 2026-10-26)

Result: passed = true. Every command meets its criterion. The candidate is byte-identical to run 2 (same diff hash), so run 3
is a clean re-run of the full gate sequence. Run 2's report is kept unedited at raw/REPORT-run2-superseded.md; run 1 at
raw/REPORT-run1-superseded.md.

## Candidate identity

- HEAD: a0a8b3fb7b038cba98e77f1f80e0008353279cf3 (raw/r3-head.txt)
- sha256(`git diff`) = 125785336bdb7891d0600149f28728f4dd484cf7dd0aedf4e73c60eeadf18ca6 (raw/r3-candidate.sha256), equal to the
  run 2 and mutation-probe candidate.
- Modified: src/engine/CMakeLists.txt, src/engine/ledger/acceptance.t.cpp, src/engine/runtime/runtime.cpp,
  src/engine/runtime/runtime.t.cpp; untracked docs/vernacular/rounds/V5-02-X06-D.md (raw/r3-status.txt).
- The gate runner edited no source file.

## Commands (serial, unpiped, exit codes in raw/r3-*.exit)

| # | command | exit | result |
|---|---|---|---|
| 1 | `cmake --build build/rewrite-debug --target planar_tests` | 0 | ok |
| 2 | `ctest --test-dir build/rewrite-debug --no-tests=error --output-on-failure` | 0 | 1056 matched, 1056 passed, 0 failed, 0 skipped, no timeouts, 379.53 s |
| 3 | `ctest --test-dir build/rewrite-debug -N` | 0 | 1056 names |
| 4 | `python3 scripts/rewrite-check.py lint --build-dir build/rewrite-debug` | 0 | clang-format --Werror and Doxygen: "checked 142 active source files" |
| 5 | `cmake --build build/rewrite-debug` | 0 | success |
| 6 | `cmake --build build/rewrite-debug` (second run) | 0 | "ninja: no work to do." zero steps, pass |
| 7 | `cmake --preset release` (RelWithDebInfo, build/rewrite-release) | 0 | configured |
| 8 | `cmake --build build/rewrite-release --target planar_tests` | 0 | ok |
| 9 | `ctest --test-dir build/rewrite-release --no-tests=error --output-on-failure` | 0 | 1056 matched, 1056 passed, 0 failed, 0 skipped, no timeouts, 153.01 s |
| 10 | `ctest --test-dir build/rewrite-release -N` | 0 | 1056 names, identical to Debug |
| 11 | `scripts/redact-local-paths.py check <5 edited files>` without --repo-path/--home | 2 | usage error (raw/r3-11) |
| 11b | same with `--repo-path $PWD --home $HOME` on the 5 edited files | 0 | clean (raw/r3-11b) |

Skips: neither ctest log has a "Skipped" or "Not Run" summary line; the only grep hits for "skipped" are the name of test #573.
Test #531 took 15.47 s on Debug (limit 30 s; earlier runs 15.77 and 16.45 s) and 4.54 s on RelWithDebInfo. Command 5 did no
compile work of its own beyond what command 1 built; command 6, a separate invocation after the successful command 5, reported no work.

## Inventory

Compared against tests/native/v5-02-x06-c-c04-evidence/inventory.txt, the most recent committed list (1041 names). The current
sorted list is tests/native/v5-02-x06-d-evidence/inventory.txt (1056 names; Debug equals Release; equal to run 2's list).
Removed or renamed: none. Added: 15 (1041 + 15 = 1056), all named "replacement.X06 D ..." (raw/added.txt): a failure caught by
Lua stays terminal with the first diagnostic; capture stage charges every sealed byte to the one native counter; composed
ceiling constants keep their contract values; conversion node ceiling refuses with nodes at a small ceiling; destroying the
consumed owner wrapper cannot erase prepared counters; each custom shape dispatch adds to the one budget and nothing is
refunded; frame start inherits the factory counters and the prepared object keeps them; invocation instruction ceiling is
shared by every stage at a small ceiling; lower inherited limits still win over the composed ceilings; moving the prepared
profile transfers counters and move assignment is safe; native ceiling spans registration and frame at a small ceiling;
native exhaustion is resource_limit terminal and cleans nested roots; prepared_profile has no new construction path;
registration instruction ceiling is separate and refuses at open with no owner; runtime public surface and resource messages
stay closed.

## (a) Acceptance criteria to tests

The round record has no criterion-to-test table; red/RED.md keys by invariant I1 to I18. This mapping is the runner's
reading of RED.md and titles, and the reviewer must confirm it. No corrective findings apply.

| criterion | test (file:line) | result |
|---|---|---|
| B1 one owner | X06 D capture stage charges every sealed byte to the one native counter (runtime.t.cpp:3175); X06 D frame start inherits the factory counters and the prepared object keeps them (3199); X06 B3 C derivation is charged to the one frame and counters never reset (2567, existing) | pass |
| B2 lower limits win, composed ceilings cumulative | X06 D lower inherited limits still win over the composed ceilings (3476); X06 D native ceiling spans registration and frame at a small ceiling (3233); X06 D invocation instruction ceiling is shared by every stage at a small ceiling (3251); X06 D registration instruction ceiling is separate and refuses at open with no owner (3304); X06 D conversion node ceiling refuses with nodes at a small ceiling (3335); X06 D each custom shape dispatch adds to the one budget and nothing is refunded (3216); X06 D composed ceiling constants keep their contract values (acceptance.t.cpp:1147) | pass |
| B3 exhaustion is resource_limit, terminal, cleans nested roots | X06 D native exhaustion is resource_limit terminal and cleans nested roots (3365) plus the ceiling tests above | pass |
| B4 caught failure keeps first diagnostic | X06 D a failure caught by Lua stays terminal with the first diagnostic (3398) | pass |
| B5 moves | X06 D moving the prepared profile transfers counters and move assignment is safe (3418); X06 D destroying the consumed owner wrapper cannot erase prepared counters (3453) | pass |
| B6 no reset, no refund | X06 D each custom shape dispatch... (3216); X06 B3 C derivation... (2567); X06 D frame start inherits... (3199) | pass |
| B7 boundary | small-ceiling tests (3233, 3251, 3304, 3335). the unobservable counters are listed in the round record's survivor disposition | partial; unobservable counters are reported as limits (see Mutation probes) |
| B8 public surface unchanged | X06 D runtime public surface and resource messages stay closed (acceptance.t.cpp:1161); X06 D prepared_profile has no new construction path (runtime.t.cpp:3523); X06 C C03 seam is a non-exported partition of the acceptance module (acceptance.t.cpp:826, existing) | pass |
| G1 focused X06 label runs | not run separately (this runner's list was full runs only); the full runs include every X06 test | not run separately |
| G2 full Debug | 1056/1056, 0 skipped, no timeouts | pass |
| G3 full RelWithDebInfo | 1056/1056, 0 skipped, no timeouts | pass |
| G4 lint, format, Doxygen, no-work build | all pass | pass |
| G5 prior evidence and carried limits | no prior evidence edited or deleted; P01 masked, N1 phase coverage, Linux not re-measured | recorded only |

Not mapped to a distinct test: one-over-the-ceiling for the 512 MiB live Lua including retained registration, and the
checked-addition-before-allocation clause of B2.



G1 note: no separate focused label run was requested of this runner; the full runs execute every X06 test with zero skips.

## (b) Red-first

Sources: red/RED.md and red/raw/tN.log. Red snapshot HEAD a0a8b3fb, sha256(red/snapshot/candidate.diff) =
07414a330904baa0f02077733d60adf4a1b8363f08a738fbf0a29373ceba9b99 (SHA256SUMS hash
d1b0ab8c883184f9d68be6ce756386fb86b521111cec8828f8b97e6a90bc5b58). Command per test, as recorded in RED.md:
`ctest --test-dir build/rewrite-debug -R '^replacement\.<escaped name>$' --no-tests=error --output-on-failure`.

- Real red (assertion failure): "X06 D capture stage charges every sealed byte to the one native counter", failing at
  runtime.t.cpp:3189 (1952 >= 5851) and 3195, 3196 of the red snapshot. Fixed by the prepare_load charge loop in runtime.cpp.
- Guards that already passed at red time (no demonstrated red, per RED.md expectRed=false): the other 14 new X06 D tests, plus the
  existing X06 B3 C derivation test and X06 C C03 seam test. Their ability to fail is evidenced by the mutation probes, not by red.
- Missing: no red run exists for the post-red candidate (it differs from the red snapshot hash because the candidate moved after
  the red check, including the C03 vocabulary rewording after run 1).

## Mutation probes

probes/PROBES.md (same candidate hash) records per-mutant logs. Survivors and their disposition are in the round record section
"Mutation-probe survivor disposition": I1b, I3b, I4b to I4h, E2, E3, E6b, E11, E12, E22 and the Lua-memory ceilings are limits
(unobservable without a new public field, fault hook or real-ceiling allocation); I10 and E25 variants are observably redundant;
I17 variants are compile-time kills. The runner did not re-run probes.

## (c) Deliberate behaviour differences

One: prepare_load charges the sealed capture's retained bytes (bytes, path, root, mode and sizeof(captured_file) per file) to the
frame's native allocation counter. Permitting clause: round record B1/B2 (one budget carried through capture; 512 MiB charged
native allocation) with exhaustion mapped to the existing resource_limit code. No message changed (messages-closed test passes).
I did not re-read the external-profile contract text in this run.

## Limitations

- Baseline is the C04 list; no later committed inventory exists.
- No Linux run (X08), no N1 phase-coverage measurement, P01 still masked, test #531 timing carried (15.47 s of 30 s).
- B7: unobservable counters and the 512 MiB Lua and capture-alone ceilings are limits, not tests.
- The mutation probes ran in a scratch copy on a ctest filter that excludes #531 and capture/profile tests; the surviving mutants
  are dispositioned above, not killed.
- redact-local-paths.py needs --repo-path and --home; the no-argument-style invocation exits 2.
