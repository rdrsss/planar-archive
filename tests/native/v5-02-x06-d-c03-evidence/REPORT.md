# V5-02-X06-D-C03 gate report (run 3, frozen candidate after the last runtime.t.cpp edit)

Gate runner result: **PASS for every listed gate command** (passed=true). One item is not a listed command and was NOT produced: the R1 margin table (Debug and
RelWithDebInfo, per fixture, measured in a scratch copy on the frozen candidate); see Limitations. No source file was edited. Run 2 report: `REPORT-run2.md`;
run 1: `REPORT-run1.md`. Run 3 raw logs: `raw/run3/` (exit codes in `*.exit`, nothing piped).

## Candidate identity (run 3)

- HEAD: `c875e08acdddf7b255ee0ff4c9533a59c872f81c`
- sha256 of `git diff HEAD` (tracked, whole tree, includes roadmap.md): `83f49b06618c33d9eacf9dd23bc9e5b860ab918092210293252f8a7835d4555a`
- `identity.py` output verbatim: `raw/run3/identity.txt` (pathspec `src docs/architecture.md docs/vernacular/profile-preparation-architecture.md docs/vernacular/orchestration.md`):
  tracked-only diff sha256 `76092574c5795d16b91f04086938556c4ac8d52be85478d6d116cd8621a53f25`, combined sha256 `4a4f1ecb16a0dfad68fb566f73fc5a2f5ac4bea30bbdb122b07c8ef547f76a53`.
  Changed since run 2: `runtime.t.cpp` (now `a444106a...`, edited 18:07), `external_registration.t.cpp` (`feb5b215...`), `src/engine/CMakeLists.txt` (`d0952563...`),
  the two architecture documents. Ceilings now: native 41943040, instructions 18000000, registration instructions 1000000, nodes 25500, tight instructions 500000.

## Commands (serialized, exit codes in files, never piped)

| step | command | exit | result |
|---|---|---|---|
| G3 | `cmake --build build/rewrite-debug --target planar_tests` (run first so G1 uses current binaries) | 0 | `ninja: no work to do.` (binaries were already current for the frozen sources) |
| G1 | `ctest --test-dir build/rewrite-debug -R X06 --no-tests=error --output-on-failure -j4` | 0 | 165 matched, 165 passed, 0 failed |
| G1b | `ctest --test-dir build/rewrite-debug -R '[Bb]udget' --no-tests=error -j4` (the label matches nothing, so the name regex) | 0 | 36 matched, 36 passed |
| G4 | full Debug `ctest --test-dir build/rewrite-debug --no-tests=error --output-on-failure -j4` | 0 | 1114 of 1114 passed, 0 failed (the only "skipped" text in the log is the name of test #626, which passed) |
| G5 | `cmake --preset release` / `cmake --build build/rewrite-release --target planar_tests` / full ctest `-j4` | 0 / 0 / 0 | 1114 of 1114 passed, 0 failed, 0 skipped |
| G6 | `ctest -N` in both trees, compared with C02 inventory | 0 / 0 | pass: 0 removed, 4 new, Debug and Release name sets identical (`cmp` clean) |
| G7 | `python3 scripts/rewrite-check.py lint --build-dir build/rewrite-debug` | 0 | "Formatting and Doxygen checked 143 active source files" |
| G8 | `cmake --build build/rewrite-debug`, twice | 0, 0 | both `ninja: no work to do.` (zero steps) after the successful G3 build |
| G9a | `python3 scripts/redact-local-paths.py check --repo-path "$PWD" --home "$HOME"`, no file arguments | 1 | scans ignored build output and local settings; not meaningful |
| G9b | same with explicit files (36 modified or untracked files under `src` and `docs`, plus `inventory.txt`; `raw/run3/redact-files.txt`) | 0 | clean |

Timing: test #559 took 16.28 s in the Debug `-j4` full run and 5.70 s in Release (run 2: 17.27 s and 6.13 s). Total test time 127.22 s Debug, 53.34 s Release.

## Inventory

- Debug and Release `ctest -N` name sets are identical, 1114 names each (`raw/run3/inv-debug.names`, `inv-release.names`).
- Compared against the most recent committed-evidence inventory, `tests/native/v5-02-x06-d-c02-evidence/inventory.txt` (1110 names). Removed: 0 (`raw/run3/removed.txt` empty). New: 4 (`raw/run3/new.txt`):
  1. `X06 D C03 a caught illegal registration followed by exhaustion keeps the first failure in a direct shape call`
  2. `X06 D C03 a recovered allocation refusal followed by an illegal registration keeps the resource failure in a direct shape call`
  3. `X06 D C03 ordinary fixtures keep a quarter of every observable ceiling free`
  4. `X06 D C03 role preparation charges tree enumeration and registry and observation reads to the factory budget`
- Full list (Debug, `replacement.` prefix stripped, sorted): `inventory.txt` next to this file (1114 lines).

## (a) Acceptance criteria to tests

Source of rows: round record `docs/vernacular/rounds/V5-02-X06-D-C03.md` criteria, ids from `plan/INVARIANTS.md`. Lines are in the current tree.
"Record/doc" rows have no test; they are reviewer checks and were not verified by the gate runner.

| criterion | tests (file:line) |
|---|---|
| R1 ordinary margin 25 percent | `X06 D C03 ordinary fixtures keep a quarter of every observable ceiling free` runtime.t.cpp:3827 |
| R1 node headroom, non-empty trees | `X06 D C03 role preparation charges tree enumeration and registry and observation reads to the factory budget` external_registration.t.cpp:4643 |
| R1 refusal tests still refuse | `X06 D conversion node ceiling refuses with nodes at a small ceiling` runtime.t.cpp:3396; `X06 D C01 native exhaustion after all-job validation refuses in preparation and cleans nested roots` runtime.t.cpp:4121; `X06 D native ceiling spans registration and frame at a small ceiling` runtime.t.cpp:3293; `X06 D native exhaustion is resource_limit terminal and cleans nested roots` runtime.t.cpp:3426; `X06 D C01 route stage exhaustion refuses before any declaration is captured` runtime.t.cpp:4067; `X06 D C01 retained declaration values exhaust the composed native ceiling in the declaration stage` runtime.t.cpp:3730; `X06 D C01 declaration chunks exhaust the composed invocation instruction ceiling across stages` runtime.t.cpp:3687 (all pass in G4 and G5, run 3) |
| R1 margin table in both configs on the frozen candidate | NOT PRODUCED by the gate runner. `sizing/SIZING.md` states "NOT MEASURED" (brief conflict, planning values only). No post-final-ceiling Debug or RelWithDebInfo table exists in the evidence. Open. |
| R1 I1b comment true | comment above `c02_outputs_gap` / `c02_padded`, external_registration.t.cpp near 4494-4534 (record review; not verified here) |
| R2 role test killed by X11, X11b, registry-read and observation-read charge mutants | external_registration.t.cpp:4643 (kills proved only by the mutation prober, not by this gate) |
| R3 site 2 `call_retained_value` | `X06 D C02 a protocol failure caught by pcall in a known-load prepare frame stays terminal` runtime.t.cpp:4210 (mutant M_O2) |
| R3 site 4 shape path of `invoke_shape_impl` | `X06 D C03 a caught illegal registration followed by exhaustion keeps the first failure in a direct shape call` external_registration.t.cpp:4680 |
| R3 site 1 / X4 `command_failure` guard | `X06 D C03 a recovered allocation refusal followed by an illegal registration keeps the resource failure in a direct shape call` external_registration.t.cpp:4700 |
| R3 sites `call_retained_assembly`, route phase, X2, X3 | recorded equivalent in `plan/INVARIANTS.md` (E2, E3, E3b, E5); no test; record review |
| R3 comment correction, clauses 396 and 455 cited | record/doc review |
| R4 invariant table and decisions copied | `plan/INVARIANTS.md`, `plan/DECISIONS.md`; copy into the round record is the coordinator's step; not verified here |
| R5 docs | `docs/architecture.md`, `docs/vernacular/profile-preparation-architecture.md` modified; text not verified here |
| R6 records | record review |
| All 1110 names preserved, name sets identical, zero failures and skips | G4, G5, G6 above: met |
| #559 not slower than about 16 s | 16.28 s Debug under `-j4` full run (run 3; 5.70 s RelWithDebInfo); not re-timed serially. Marginally above "about 16 s" but inside the load-dependent spread (run 2 was 17.27 s); not a failure of any listed gate. |
| Existing C02 tests (R2 baseline) | `X06 D C02 role preparation charges enumeration and registry reads to the factory budget` external_registration.t.cpp:4538; `... charges declaration chunks to the factory budget` :4434; `... extension owners created without a factory keep role declarations outside the budget` :4477 |

## (b) Red-first

From `red/RED.md` (the only red evidence found). Snapshot: `red/snapshot/` with `candidate.diff` sha256
`ed58f909e4c07d227499f11c6f451b0af1a938163480864d7a7a2efa663f3124` and `SHA256SUMS` sha256
`87bf6a83db795df1e115a426045f1fcdd99d25e2cdb6e2ae8b773dad42d05aa2` (HEAD `c875e08a`). Build: `cmake --build build/rewrite-debug --target planar_tests`.
Per-test command: `ctest --test-dir build/rewrite-debug -R '^replacement\.<escaped name>$' --no-tests=error --output-on-failure`; raw logs `red/raw/t1.log`..`t23.log`.

| test | real red or guard | failing line |
|---|---|---|
| C03 role preparation charges tree enumeration and registry and observation reads (:4638) | real red (assertion) | external_registration.t.cpp:4636 `REQUIRE(both_trees.ok)` at the old ceilings (red-time line numbers; the test is now at :4643) |
| C03 ordinary fixtures keep a quarter of every observable ceiling free (:3827) | real red (assertion) | runtime.t.cpp:3824 `CHECK(p.native_bytes*4 <= budget_native_max*3)` (shifted) |
| the two C03 direct shape tests (:4675, :4695) | not in the red table by name; RED.md lists 21 unique guards that passed, "all other listed tests". Whether these two were among them is not identifiable from RED.md. They are described in INVARIANTS as expectRed=no (guards proved by probes M_O4 and X4). | none |

Missing from the evidence: per-test mapping of t3..t23 logs to test names (RED.md gives only the order of the unique-test list), and any red evidence for the
rows that depend on a mutation (those are the prober's).

## (c) Deliberate behaviour differences

None found by the gate runner. The round is test, ceiling and documentation scope (`runtime.cppm` unmodified per the record). The ceilings in
`src/engine/CMakeLists.txt` changed (test-variant budgets); whether they are within clause permission was not reviewed here.

## Limitations

- R1 margin table (per fixture, Debug and RelWithDebInfo, measured after the last ceiling change) was NOT produced by this gate run: it needs a scratch-copy instrumented
  build (about 15 minutes cold per configuration) and `sizing/SIZING.md` still says NOT MEASURED, with planning values that differ from the ceilings now in
  `CMakeLists.txt`. Only the in-suite check `X06 D C03 ordinary fixtures keep a quarter of every observable ceiling free` covers the 25 percent margin for ordinary
  fixtures (passes in both configurations). Live-Lua, role-flow native and node peaks have no public observer (limit under lesson 7).
- The red-first rows for the two direct shape tests cannot be identified in RED.md; the red logs `red/raw/t1..t23.log` are not mapped to names. The red evidence
  predates the current ceilings and runtime.t.cpp; line numbers above are current.
- Test #559 took 16.28 s Debug under `-j4` load (about 16 s criterion; not timed serially).
- Linux not run (X08). Mutation probes and review are later stages.
