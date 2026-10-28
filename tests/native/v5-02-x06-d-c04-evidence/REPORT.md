# V5-02-X06-D-C04 gate report (gate runner, run 2, 2026-10-27)

No source file was edited by the gate runner. Raw logs and exit-code files for this run are in `raw/run2/` (run 1 logs remain in `raw/`, run 1 report in `REPORT-run1.md`). Nothing was piped.

## Candidate identity

- HEAD `c875e08acdddf7b255ee0ff4c9533a59c872f81c`, uncommitted X06-D + C01 to C04 tree.
- sha256 of `git diff HEAD` (tracked, whole tree): `61c4688c7f99cae85b2868597e56e955a1b2ba3773ed960651f5e336db266422` (`raw/run2/diff-sha.txt`), the same as run 1.
- `identity.py` output verbatim in `raw/run2/identity.txt` (pathspec `src docs/architecture.md docs/vernacular/profile-preparation-architecture.md docs/vernacular/orchestration.md`): tracked-only sha256 `55dee86ae40e8c83433dd666cc9d0293e4facecfc4d5b665beedac69e2224cdb`, combined sha256 `81dc303acdf5cc8f87c880d0e538f86cfdf183e3267860f309785c9977570f47`. Per-file hashes match the probe run (`probes/PROBES.md`): CMakeLists.txt `4f263d74...`, runtime.t.cpp `1d7ecf32...`, external_registration.t.cpp `281ed935...`.

## Commands (serialized, exit codes in `raw/run2/*.exit`)

| step | command | exit | result |
|---|---|---|---|
| G3 | `cmake --build build/rewrite-debug --target planar_tests` | 0 | success |
| G1 | `ctest --test-dir build/rewrite-debug -R X06 --no-tests=error --output-on-failure -j4` | 0 | 165 matched, 165 passed, 0 failed, 0 skipped |
| G1b | `ctest --test-dir build/rewrite-debug -R '[Bb]udget' --no-tests=error -j4` (ctest labels are only `acceptance` and `unit`; there is no budget label, so the name regex was used) | 0 | 36 matched, 36 passed, 0 skipped |
| G4 | full Debug `ctest --test-dir build/rewrite-debug --no-tests=error --output-on-failure -j4` | 0 | 1114 of 1114 passed, 0 failed, 0 skipped (125.03 s) |
| G5a | `cmake --preset release` | 0 | success |
| G5b | `cmake --build build/rewrite-release --target planar_tests` | 0 | success |
| G5c | full RelWithDebInfo `ctest --test-dir build/rewrite-release --no-tests=error --output-on-failure -j4` | 0 | 1114 of 1114 passed, 0 failed, 0 skipped (53.32 s) |
| G6 | `ctest -N` in both build dirs | 0 / 0 | 1114 names each, Debug and Release identical (`cmp` clean) |
| G7 | `python3 scripts/rewrite-check.py lint --build-dir build/rewrite-debug` | 0 | "Formatting and Doxygen checked 143 active source files" |
| G8a | `cmake --build build/rewrite-debug` (first) | 0 | `ninja: no work to do.` (the preceding G3 build had already built everything, so this was not a producing build) |
| G8b | `cmake --build build/rewrite-debug` (second) | 0 | `ninja: no work to do.` (zero steps) after the successful G3 and G8a builds |
| G9a | `python3 scripts/redact-local-paths.py check --repo-path "$PWD" --home "$HOME"` with no files | 1 | scans ignored local files (`.claude/settings.local.json`, `build/install-release/*`); not meaningful |
| G9b | same with explicit files: the 37 modified or untracked paths from `git status` excluding ignored output and evidence `raw/` (`raw/run2/redact-files.txt`) | 0 | clean |

The "skipped" grep hits in the Debug and Release logs are only the test name `replacement.command load refuses forged family order skipped duplicate and malformed arguments` (Passed). No ctest test was skipped. passed=true: every criterion is met; G9a exit 1 is the known not-meaningful variant and G9b is the criterion.

## Test inventory

Compared with the C03 run 3 list `tests/native/v5-02-x06-d-c03-evidence/raw/inv-debug.names` (the most recent per-run inventory in earlier evidence; 1114 names): `comm` gives 0 new and 0 removed (`raw/run2/new.txt`, `raw/run2/removed.txt`, both empty). The C03 `inventory.txt` differs only in omitting the `replacement.` prefix on 23 names; it is not a different set. `inventory.txt` here is the sorted Debug `ctest -N` names (1114), identical to the Release list.

## (a) Acceptance criteria to tests and locations

C04 adds no test case. Its criteria are met by records, comments and ceiling constants; ceiling-dependent tests re-ran green at the new ceilings in both configurations.

| criterion | evidence | location |
|---|---|---|
| C03-R1 measured table, no UNMEASURED, 25 percent margins | round record Margin record sections 1 to 9; `sizing/SIZING.md`; `sizing/pass2/check_margins.py` exit files; `grep -c UNMEASURED` is 0 (PROBES.md row notes) | ceilings `src/engine/CMakeLists.txt:128-131` (39845888 / 19900000 / 1000000 / 25000), comment `:122-125` |
| R1 ceiling-dependent tests green at new ceilings | `X06 D native ceiling spans registration and frame at a small ceiling` | runtime.t.cpp:3293 |
| | `X06 D conversion node ceiling refuses with nodes at a small ceiling` | runtime.t.cpp:3396 |
| | `X06 D C03 ordinary fixtures keep a quarter of every observable ceiling free` (derived instruction bound, stale 24,963,619 comment replaced) | runtime.t.cpp:3833 (comment above it) |
| R1 equality pins replaced by derivation checks | `budget_instruction_max - pass.instructions == 100` and `% 100` stay as ceiling-derived equalities | runtime.t.cpp:3892, 4015 (as recorded in run 1, not re-read now) |
| C03-R2 roadmap CL-1 to CL-6 | Next work row `Carried limits CL-1 to CL-6` with reachability verdicts and clauses `:396`, `:455/:459`; round row C04 | docs/vernacular/roadmap.md:44, :42 |
| C03-R3 full plan copies | C04 record `INVARIANTS.md` section and `DECISIONS.md` section | docs/vernacular/rounds/V5-02-X06-D-C04.md:572, :726 (C03 record per run 1: :538, :707) |
| C03-R4 architecture citations | `runtime.cpp:32` 512 MiB, separate 64 MiB `loader.cpp:12`; historical qualification near line 1514 per run 1 | docs/architecture.md:88-91 |
| C03-R5 role-test comment | comment states `c03_entries (= 1)`, constant at :4567 | external_registration.t.cpp:4580-4582 |
| Standing: 1114 names preserved, diagnostics identical, surface unchanged | inventory comparison above; PROBES.md shows `git diff --stat` of `runtime.cppm`, `src/cmd`, `src/engine/budget`, `src/engine/ledger` checked by the prober; diagnostics were not re-diffed by the gate runner | `raw/run2/removed.txt` empty |

## (b) Red-first

One behavioural red exists (prober, `probes/PROBES.md` section "C4-5r", raw files `probes/raw/C4-5r*`, script `probes/tools/red.sh`):

| test | command (verbatim from PROBES.md) | red snapshot hash | failing line | real red or guard |
|---|---|---|---|---|
| `X06 D C03 ordinary fixtures keep a quarter of every observable ceiling free` | `cmake --preset debug`; `cmake --build build/rewrite-debug --target planar_tests`; `ctest --test-dir build/rewrite-debug -R 'X06 D C03 ordinary fixtures keep a quarter of every observable ceiling free' --no-tests=error --output-on-failure` | `d421789b99d78f25d7b0eea1a40fcb14872ce8d973a7b1f1f391ab651e0bb1dd` (C03 ceilings 41943040 / 18000000 restored, fallback `find src -type f | LC_ALL=C sort | xargs shasum -a 256 | shasum -a 256`; final src hash by the same command `4709f0c7...`) | ctest exit 8, `runtime.t.cpp:3859: FAILED: CHECK( budget_instruction_max * 3 >= 4 * extrapolated )`, extrapolated 16812370 | real red (behavioural, builds and configures succeed), green after restore |

No other test was added or changed with its own red snapshot. The other tests listed under PROBES.md "Red-first" are existing tests whose ceilings or constants were recalibrated; PROBES.md lists mutants killing them but no pre-fix red run, so none is claimed. The prober's mutation results (killed named tests per mutant) are in `probes/PROBES.md`; the gate runner did not rerun them.

## (c) Deliberate behaviour differences

Test-variant ceilings only (`src/engine/CMakeLists.txt`, not product runtime): native 41943040 to 39845888, invocation instructions 18000000 to 19900000, nodes 25500 to 25000; registration 1000000 unchanged. Product ceilings (512 MiB, 100,000,000 instructions, 4,000,000 nodes) are unchanged. Permission: the C04 round brief ("CMake ceilings and calibration constants changed only as required") supported by the Margin record. Conformance to a clause of `profile-preparation-architecture.md` was not reviewed here.

## Limitations

- G9a without files exits 1 by design; only G9b is meaningful.
- G8a was a no-work build because G3 had already built the tree; the no-work property is therefore demonstrated twice, after a successful build.
- Margin measurement and mutation probes were run by earlier workers; this gate did not rerun them. Line numbers marked "per run 1" were not re-read in run 2.
- `-j4` runs; no per-test timing inspection. Independent review is not part of this gate.
