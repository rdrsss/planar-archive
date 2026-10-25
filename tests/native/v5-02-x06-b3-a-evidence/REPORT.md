# V5-02-X06-B3-A gate report: profile invocation owner

Result: the gate PASSED. All five agreed commands met their criteria. No source file was edited by the gate runner. This report supersedes an earlier failed gate run (its logs are preserved under `raw/prev/`); the tree has since changed (the `src/cmd/planar/CMakeLists.txt` generated-variant dependency edges and the formatting of `runtime.t.cpp` were corrected by the coders), so the earlier failures no longer apply.

## Candidate identity

- HEAD: `d783c0b9514c50c8a2ad060281c4e75400d5b2dd` (V5-02-X06-B2+C01+C02), branch dev/planar-v2-vernacular.
- sha256 of `git diff` (10 modified tracked files, 922 insertions, 59 deletions; untracked round record `docs/vernacular/rounds/V5-02-X06-B3.md` not in the diff): `e21917490f51841de6b687388f02a2da0e5a8d1185534994521890bbaa231e64` (`raw/diff.sha256`), taken before the gate. Platform: macOS Debug only.

## Commands (serial, unpiped; raw logs in `raw/`)

| # | Command | Exit | Result |
| --- | --- | --- | --- |
| 1 | `cmake --build build/rewrite-debug --target planar_tests` (01-build-tests.log) | 0 | `ninja: no work to do.` The tests were already built from this exact tree by the preceding coder/gate work; the diff hash above is that tree, so this is a valid up-to-date build, not a fresh compile. |
| 2 | `ctest --test-dir build/rewrite-debug --no-tests=error` (02-ctest.log) | 0 | `100% tests passed out of 957`; no failed, skipped, not-run or disabled lines; 321.79 s real |
| 3 | `ctest -N` inventory (03-inventory-after.txt) | 0 | 957 registered; compared below |
| 4 | `python3 scripts/rewrite-check.py lint --build-dir build/rewrite-debug` (04-lint.log) | 0 | `Formatting and Doxygen checked 136 active source files` |
| 5a | `cmake --build build/rewrite-debug` (05-build-all-1.log) | 0 | linked/regenerated (4 steps, dyndep and planar link) |
| 5b | `cmake --build build/rewrite-debug` again (06-build-all-2.log) | 0 | `ninja: no work to do.` (zero steps) |

## Inventory

Compared against `tests/native/v5-02-x06-b2-c01-evidence/inventory.txt` (940 names, the most recent committed plain-name list). After: 957 registered, all preserved, none removed, 17 added (`inventory.txt`). The coders' `logs/inventory-before.txt` has 944 names, taken mid-work; the pre-gate `raw/inventory-before.txt` already shows 957 because the tests had been built.

New names: 4 in source_capture.t.cpp (N1, N2, N3, receipt accessors) and 13 `X06 B3 A ...` cases in runtime.t.cpp (the 13-case count matches the coder's filtered `[x06b3a]` run: 221 assertions).

## (a) Acceptance criteria to tests

| Criterion | Tests (file:line) |
| --- | --- |
| 1. Metadata admission of root, declaration, registry, ledger before any chunk; hard-link refusal; no content read | runtime.t.cpp:1086 (reads no declaration or registry content, metadata before registration); :1099 (hard-link aliasing a trusted module in each of declaration, registry and ledger slots refuses unsafe_path, no partial owner, healthy retry); :1118 (declaration hard-linked to an invalid module refuses unsafe_path before registration would execute it; unaliased it is a registration error); :1073 (opens once with original receipts) |
| 2. Opaque cannot reach built-in load | runtime.t.cpp:1070 (static surface offers no prepare, vm or upgrade access); :1168 (opaque open never known load) |
| 3. Owner semantics | :1189 (moves preserve address and counters, moved-from refuses); :1070 (copy deleted, via static surface); :1137 (failed owner publishes nothing); :1148 (non-codec2 intent refuses); :1242 (into_extension same VM and receipts, no recapture); :1265 (opaque adoption works) |
| 4. One active frame; public validation unchanged; names kept | :1222 (begin_frame once-only, blocks adoption); :1278 (public validate_profile cumulative semantics); inventory preserves all 940 |
| 5. Receipt roles, N1, N3 | source_capture.t.cpp:543 (N2 roles); :505 (N1); :579 (N3); :627 area accessors test "receipt accessors report moved_from and role"; runtime.t.cpp:1137 (declaration adapter) |
| 6. Fresh profile acceptance refusal remains; no CLI change; no cycle or runtime-to-ledger-acceptance edge | No new test; existing refusal tests are part of the 957-case pass. No `src/cmd` source changed; `src/cmd/planar/CMakeLists.txt` only adds `planar_acceptance_source_capture planar_acceptance_intent` to two generated-variant DEPENDS lists. A configure and full build succeeded, which Ninja would refuse on a target cycle. No explicit dependency audit script was run; the edges added in both CMakeLists diffs are source_capture and intent only, no ledger acceptance. |
| 7. No runtime assert, typed failures, docs | Lint (format plus Doxygen, 136 files) passes. No separate grep audit was run by the gate. |

Carried advisories: N1 (source_capture.t.cpp:505), N2 (:543), N3 (:579). The admission diff adds `moved_from()` and `role()` to the retained receipts. The gate did not verify how macOS `O_EVTONLY` is handled beyond the N1 test passing.

## (b) Red-first

Evidence present (`logs/`): `p1-red.log` (header "red: new tests against unmodified product"), `p1-green.log`, `p3-red.log`, `p3-gate.log`, `p2-build.log`.

- Source capture regressions: the red is a compile failure of target `planar_acceptance_source_capture_tests` at source_capture.t.cpp:594 and :625 (`no member named 'moved_from'` in `retained_root_receipt` and `retained_file_receipt`). This is a missing-API compile red, not a runtime assertion red. The exact command is not recorded in the log (target inferable from the ninja output, not stated). p1-green.log shows the target rebuilding and linking afterward. The red snapshot hash is NOT recorded in the coder evidence. N1 and N3 behavioural reds are not separately demonstrated.
- `X06 B3 A` runtime cases: no red run is present; these are new-API tests with no pre-fix behavior to fail. Missing: command, snapshot hash, failing lines.
- `p3-red.log` records the earlier `planar_tests` `intent not found` build failure, which the dependency edge fix resolved. It is a build defect record, not a regression-test red.

Red-first is incomplete: red command lines and snapshot hashes are missing.

## (c) Deliberate behaviour differences

None identified by the gate runner. No statement in the coder evidence documents a macOS N1 difference.

## Limitations

macOS Debug only; no RelWithDebInfo or Linux; no RSS or whole-invocation budget claim. Command 1 was an up-to-date no-op, so the compile of this tree is evidenced by the coders' build rather than by this run's log.
