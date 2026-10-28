# V5-02-X06-D-C02 gate report (run 3, after the post-probe repair)

Result: every gate command passes (zero failures, zero skips, zero removed names, no-change build does no work). Mutation probes were NOT re-run by the gate runner (see Limitations). Earlier gate reports are kept unchanged as `REPORT-pre-repair.md` (run 1, failed) and `REPORT-run2.md` (run 2, passed before the I1b, T9b and L7c repair); their raw logs stay under `raw/` (`raw/pre-repair/`, files without the `run3-` prefix). This run's raw logs and `.exit` files are `raw/run3-*`. No source file was edited by the gate runner; the only files written are under this evidence folder.

## Candidate identity

From `identity.py` (pathspec `src docs/architecture.md docs/vernacular/profile-preparation-architecture.md`), taken before the gates (`raw/run3-identity-before.txt`) and after them (`raw/run3-identity-after.txt`; byte-identical, the tree did not change during the run):

- HEAD `c875e08acdddf7b255ee0ff4c9533a59c872f81c`
- sha256(git diff --binary HEAD -- src docs/architecture.md docs/vernacular/profile-preparation-architecture.md), tracked only: `ae2edabaf70beeb4d25b68304a6a98d31505889b3396e9406ec7aee59e8bb18e`
- combined sha256 (diff bytes plus untracked paths, evidence excluded): `401c718b7f4e02f21b25e3b034b60776ef5ad32da9d0416d5e4b5f22fd95706e`
- whole-repo `git diff --binary HEAD` sha256 (includes roadmap and other docs, tracked only): `81faf2de9016ba1927ded78abbd909e6d5267b775a2a4fc7cf9918aee300994a` (`raw/run3-identity-gitdiff.txt`)
- Per-file hashes for every changed file and the untracked `src/engine/budget/budget.cppm` are in `raw/run3-identity-before.txt`. Compared with run 2, `runtime.t.cpp`, `external_registration.t.cpp`, `loader.t.cpp` and the two docs changed (the post-probe repair); `runtime.cpp` and `CMakeLists.txt` hashes did not.

## Commands (raw logs and `.exit` files: `raw/run3-<id>.log|.exit`)

| id | command | exit | result |
|---|---|---|---|
| G0 | `cmake --build build/rewrite-debug --target planar_tests` | 0 | `ninja: no work to do.` (candidate already built by the repair coder) |
| G1 | `ctest --test-dir build/rewrite-debug -R 'X06' --no-tests=error --output-on-failure` | 0 | 161 matched, 100 percent passed, 0 failed, 0 skipped (91.97 s) |
| G1b | `ctest --test-dir build/rewrite-debug -N -R 'budget\|Budget\|BUDGET'` (there is no ctest label `budget`; budget tests are selected by name) | 0 | 35 budget-named tests listed (`raw/run3-g1b-budget-names.log`); all also match X06 and ran in G1 |
| G2 | `cmake --build build/rewrite-debug --target planar_tests` | 0 | no work to do |
| G3 | full Debug `ctest --test-dir build/rewrite-debug --no-tests=error --output-on-failure` | 0 | 1110 matched, 100 percent passed, 0 failed, 0 skipped (409.92 s) |
| G4 | `cmake --preset release` | 0 | configured |
| G5 | `cmake --build build/rewrite-release --target planar_tests` | 0 | built (544 steps) |
| G6 | full RelWithDebInfo `ctest --test-dir build/rewrite-release --no-tests=error --output-on-failure` | 0 | 1110 matched, 100 percent passed, 0 failed, 0 skipped (162.58 s) |
| G7 | `ctest -N` in both configs (`raw/run3-g7a-N-debug.log`, `run3-g7b-N-release.log`) | 0, 0 | 1110 names each; sorted name lists identical (`diff` empty) |
| G8 | `python3 scripts/rewrite-check.py lint --build-dir build/rewrite-debug` | 0 | clang-format `--Werror` clean; "Formatting and Doxygen checked 143 active source files" |
| G9a | `cmake --build build/rewrite-debug` (first) | 0 | `ninja: no work to do.` (the full build, including `bin/planar`, was already current from run 2 and the repair; no source changed since) |
| G9b | `cmake --build build/rewrite-debug` (second) | 0 | `ninja: no work to do.` Zero steps: pass |
| G11 | `python3 scripts/redact-local-paths.py check --repo-path $PWD --home $HOME <32 explicit files>` (every modified and untracked non-evidence path from `git status`; list in `raw/run3-g11-files.txt`) | 0 | pass, empty output |
| G11b | same command with no file arguments | 1 | fails only on ignored build output (`build/install-release/...`) and `.claude/settings.local.json`; not meaningful as a gate, reported because the dispatch asked for it |

Zero skips in every run (no skipped result lines; ctest summaries "100 percent passed").

### Timing of the guarded test

`X06 B3 C a file replaced at any allocation inside prepare_load is never published as verified` (#556 in the dispatch, #558 in run 2, now #559 because the inventory grew by one): Debug **15.35 s** (limit about 15.3 s; history: C01 gate 15.39, pre-repair 15.97, repair coder 15.46, run 2 15.52). That is 0.3 percent over the stated figure, within the spread of earlier runs; I record it as at the limit, not clearly faster. Release 4.50 s. Single run.

## Test inventory

`inventory.txt` holds the 1110 sorted names (`ctest -N`, Debug; Release identical, prefix `replacement.`). It was compared with the most recent committed-or-recorded earlier inventory, `tests/native/v5-02-x06-d-c01-evidence/inventory.txt` (1098 names, the C01 gate list; it is uncommitted like this one, there is no committed inventory of this lineage newer than the C01 file). The run 2 list is kept as `inventory-run2.txt` (1109 names). Removed names against C01: none (`comm -23` empty). New names against C01: 12, so 1098 + 12 = 1110. Against run 2 exactly one name is new (no. 8).

1. X06 D C01 native exhaustion after all-job validation refuses in preparation and cleans nested roots
2. X06 D C01 route stage exhaustion refuses before any declaration is captured
3. X06 D C02 a protocol failure caught by pcall in a known-load prepare frame stays terminal
4. X06 D C02 a protocol or semantic failure caught by pcall in a profile-owned frame stays terminal
5. X06 D C02 extension owners created without a factory keep role declarations outside the budget
6. X06 D C02 loader module graph does not depend on capture admission
7. X06 D C02 loader reports the owner diagnostic when the composed owner refuses a declaration chunk
8. X06 D C02 loader reports the owner diagnostic when the composed owner refuses Lua state creation (new since run 2; L7c)
9. X06 D C02 role preparation charges declaration chunks to the factory budget
10. X06 D C02 role preparation charges enumeration and registry reads to the factory budget
11. X06 D C02 the composed sink refusal is the first failure on a declaration chunk that crosses the invocation ceiling
12. X06 D C02 the composed sink refuses a pre-frame declaration chunk at the carried ceiling

## (a) Acceptance criteria and findings to tests

The round record carries no invariant table (the search output with rows I1 to I28 is not in the repository), so rows are the objective items C01-R1 to C01-R7, the acceptance criteria, and the invariant numbers used in `red/RED.md` and `probes/PROBES.md`. T8 and T9 file:line values are inferred from the C01 table names. "Passes" means the test passes in Debug and RelWithDebInfo at this candidate. Whether a test kills its mutant is only what `probes/PROBES.md` established, on the run 2 candidate (see Limitations).

| criterion / finding | tests (file:line) | state at this candidate |
|---|---|---|
| C01-R1 role budget charges declarations (I1) | `external_registration.t.cpp:4434` | passes; probes: killed |
| C01-R1 enumeration and registry reads charged (I1b) | `external_registration.t.cpp:4526` | passes; the run 2 version was NOT killed by the I1 mutant nor X11 to X13 (probe headline 2); the test was strengthened afterward (outputs-off vs outputs-on boundary gap, at least 4525 bytes) and has not been re-probed |
| C01-R1 `extension_invocation::create` keeps a null budget (I2) | `external_registration.t.cpp:4477` | passes; probes: killed |
| C01-R1 inventory rows with source citations (I3, I4) | record edits | not a test; not verified by the gate runner (probe review noted the extension-stage row was not yet in the record at probe time) |
| C01-R2 sink refusal is the first failure; mutants T3, T3c, T3d, T16 (I5 to I7) | `runtime.t.cpp:3939` | passes; probes: T3, T3d, T16 killed by it |
| C01-R2 pre-frame chunk refused at the carried ceiling (I8) | `runtime.t.cpp:4014` | passes; probes: T3c, X7 killed |
| C01-R2 L7c owner diagnostic when the composed ceiling refuses Lua state creation (I9) | `loader.t.cpp:251` (declaration chunk), `loader.t.cpp:284` (Lua state creation) | both pass; the L7c test at 284 is new since run 2 and has not been probed (L7c survived in the run 2 probes) |
| C01-R3 T8 exhaustion between stages (I10) | `runtime.t.cpp:4041` (inferred) | passes; probes: row T8 killed; the C01 pre-attach T8 stays a recorded dead-path survivor |
| C01-R3 T9 exhaustion after phase 8 and after all-job phases (I11, I12) | `runtime.t.cpp:4093` (inferred) | passes; rowT9a killed; rowT9b and X6 killed only by `X06 B3 C each derivation charge scales exactly with the bytes it covers`; the post-probe slope pin (at least 7 per padded byte) is not re-probed |
| C01-R4 semantic or protocol failure caught by `pcall` stays terminal (I13, I14, I23) | `runtime.t.cpp:4143`, `runtime.t.cpp:4178` | both pass; 4143 kills the R4 repair mutants M_R4a and M_R4b; 4178 failed before the repair (see (b)) |
| C01-R5 variant ceilings (I15, I16, I17) | `acceptance.t.cpp:1238` (mechanism); budget-variant runtime tests; `margins.md` | ceilings now native 31457280 and instructions 8000000 (`src/engine/CMakeLists.txt`); node ceiling 18000 unchanged. All budget-variant tests pass. **No post-raise peak and margin table exists** (`margins.md` is pre-raise, and pre-raise it showed node margins as low as 2.0 percent on the C02 role fixtures, so the 18000 ceiling probably still has under 25 percent margin there). R5 is not demonstrated. |
| C01-R6 no loader-to-admission edge; `budget_sink` in a lower module (I18) | `loader.t.cpp:297`; module `src/engine/budget/budget.cppm` (`planar_budget`); `src/engine/CMakeLists.txt` | passes; doc edits are in the identity but not checked by the gate |
| C01-R7 record corrections | record edits | not a test; not verified by the gate runner |
| 1098 names preserved, identical across configs, zero failures and skips | inventory above | met (1110 names) |
| #556 not slower than about 15.3 s | `X06 B3 C a file replaced ...` | 15.35 s Debug, at the limit |

## (b) Red-first (from `red/RED.md`, `red/raw/t*.log`, the C02 record)

Snapshot: `red/snapshot-2/`, sha256 of `candidate.diff` `40d7ce2404b12345e6d6b18890525dba5fedb9d8245cc1530627cdde160133d8`, sha256 of `SHA256SUMS` `95588d212d214ddbdab0ddec5afd39273101a2ff29f0be2879d86c7b6d9d9b78`, HEAD `c875e08a`. Command as recorded in `RED.md`: `ctest --test-dir build/rewrite-debug -R '^replacement\.<escaped name>$' --no-tests=error --output-on-failure`, after `cmake --build build/rewrite-debug --target planar_tests` (`red/raw/build.log`).

| log | test | outcome at the snapshot | failing line | classification |
|---|---|---|---|---|
| t1 | role preparation charges declaration chunks | failed assertion | `external_registration.t.cpp:4462` `REQUIRE_FALSE(subject.ok)` | real red; green now |
| t2 | role preparation charges enumeration and registry reads | passed | none | not red (coder expected red); `allRed=false`; red for I1b never obtained |
| t3 | extension owners created without a factory | passed | none | guard |
| t4 | composed sink refusal first failure | passed | none | guard; proven by probes only |
| t5 | composed sink refuses pre-frame chunk | passed | none | guard |
| t6 | loader owner diagnostic (declaration chunk) | passed | none | guard |
| t7 | route stage exhaustion | passed | none | guard |
| t8 | native exhaustion after all-job validation | passed | none | guard |
| t9 | pcall in profile-owned frame stays terminal | passed | none | guard |
| t10 | variant ceiling mechanism | passed | none | guard |
| t11 | declaration Lua memory composed ceiling | passed | none | guard |
| t12 | loader module graph | failed assertion | `loader.t.cpp:266` `REQUIRE_FALSE(budget.empty())` | real red; green now |
| t13 | pcall in known-load prepare frame | failed assertion | `runtime.t.cpp:4156-4157` | failing guard at the snapshot, failed again at the run 1 gate; green after the product change in `runtime.cpp` |

Missing from the evidence (not invented here): no red command or snapshot hash for the Lua-state-creation L7c test (new since run 2), for the I1b strengthening, or for the T9b slope pin; no pre-fix hash for the repair-time `runtime.cpp` change; no post-fix green rerun keyed to a snapshot hash for t1, t12, t13 other than this report's candidate identity; the final text of t1 (constants) and t13 differs from the red snapshot.

## (c) Deliberate behaviour differences

Recorded in the C02 record's repair section: in `runtime.cpp`, a protocol failure already recorded in `bridge_error` is now the first terminal failure even if an instruction exhaustion follows (`call_retained_value`, the shape execution path of `invoke_shape_impl`, and the route phase of `prepare_load` check `bridge_error` before `exhausted`), and `command_failure` records nothing once a ceiling is exhausted. The record cites objective R4 ("the first terminal failure survives") but no external-profile contract clause, and the evidence names none; reviewers must check it against the contract's first-failure wording. No diagnostic message was changed (lint and the full suites in both configurations show no message regression).

## Limitations

- Mutation probes were not run by the gate runner in this run. `probes/PROBES.md` records the earlier prober run against the run 2 candidate (56 mutants, 42 killed, 13 survivors, 1 patch failure, plus 33 C01 rows carried forward), whose dispatch table was truncated after row I16. Since then `runtime.t.cpp`, `external_registration.t.cpp` and `loader.t.cpp` changed (I1b strengthening, T9b pin, L7c test), so those rows are changed or new and a prober run on this candidate is still required. There is no invariant table in the repository to build the full dispatch from; it must be supplied.
- R5 post-raise margin table is missing (see (a)).
- G1b: no ctest label `budget`; by-name selection used.
- Timing is a single run; the guarded test is 15.35 s against about 15.3 s.
- The no-change check G9b follows a first build (G9a) that itself had no work, because the full build was current; no separate successful build with steps ran in this run (G5 built Release with 544 steps).
- `bin/planar` is not built by `planar_tests`; the full `cmake --build build/rewrite-debug` covers it.
