# R19-ACCEPT-A-C02 independent review

Disposition: **request-changes**. The C02 candidate closes several C01 findings well:

- The mini-fault run is real, complete and byte-checkable.
- The five host-path files named in C01-R2 are redacted.
- The F, L and Q rows now have probe outcomes with named killing tests.
- The harness fixes have behavioural red-first evidence.
- A3, A4 and A5 are closed.

The round still does not meet its first and most important objective. The heavy faulted CMake run that the record
marks "MET for the run" did not run the faulted scenario. It stopped at the first check after takeover, because both
real CMake checks failed. It never reached a rejecting verdict, repair, a `max_rounds` blocker, final approval or
replay, and it still exited 0. `runs/cmake-rerun/index.json`, `limits`, `packet`, `index`, `scan` and `verify-redaction`
were not executed, as the record itself states. The cause of the check failure is a test-input dependency on a
gitignored, untracked file. Any export of tracked sources will reproduce it. The round should have stopped and
reported it under the stop rule. Records alone cannot fix this: it needs a coordinator decision, a corrected run
and new evidence.

The reviewer was not the coder, coordinator or prober. Writes were limited to this file. No checkout file was mutated.
No build, ctest, ledger command, installed binary, legacy binary or script under `tests/native/` or `scripts/` was
run. The only tool executed was the workflow identity tool, copied to the scratchpad and run from the repository root.
All other checks were read-only: `ls`, `grep`, `git diff --numstat`, `git cat-file`, and inline Python with `-I` that
parsed JSON and AST and compared bytes over existing files.

## Candidate identity

Re-run by this reviewer with pathspec `src tests/native/r19-accept-support`. The output matches the record's "Final
candidate identity" table and the identity table of `probes/PROBES.md` byte for byte:

| identity | value |
|---|---|
| HEAD | `91e0df24f2167471e49ec057c7159bce5dff3608` |
| sha256(git diff --binary HEAD -- src tests/native/r19-accept-support), tracked only | `83a16fdee033fce5a1a792d8d65f4090fba8dacffc2d38013684cacb4daeba19` |
| combined sha256, pathspec src tests/native/r19-accept-support, evidence excluded | `988e085f2272f80c8422dc99ed2098b0f8ab9eaa0d3c82f79988569e8907e1d9` |

`src` is unchanged against C01. The tracked diff hash equals the reviewed C01 value `83a16fde...`, and the per-file
hashes of `storage.cpp` (`e4cf41dc...`) and `execution.t.cpp` (`250867b8...`) equal the C01 table.
`git diff --numstat HEAD -- src` gives 177/0 and 8/4, the same as C01. No C++ test was removed, and the gate's
`ctest -N` list equals the C01 inventory. The product-unchanged shortcut, which cites C01 run 4 for ctest, Release and
Linux, is therefore valid.

## What was verified and holds

- **Mini-fault run (R1a, R1s).** `runs/mini-fault/mini-fault.exit` and `rerun-index.exit` are 0, and both stderr files
  are empty. `raw/mini-fault-c02/raw/` holds argv, stdout, stderr and exit for each of 145 actions (585 entries).
  `index.json` is redacted (`<path>` argv) and shows the following:
  - interrupt: CLI return code -9, child group dead, reservation `in_progress`.
  - `check-recover`: exit 0 with outcome `interrupted`.
  - takeover: generation 1 to 2.
  - fenced old token: exit 3 `fenced`, ledger digest unchanged.
  - verdicts: `reject1` with repair admitted, then `reject2` capped with `max_rounds` open, then `load` raising
    `max_rounds` 1 to 2 and clearing the blocker, then a final `approve` accepted with targets equal to the latest
    output.

  The reviewer compared the raw replay bytes directly, grouping the raw files by `--request-id`. All seven pairs
  are byte-identical with exit 0:
  - `activate` (298 B)
  - `claim` (6297 B)
  - `status` (749 B)
  - `check-2` (881 B)
  - `verdict-reject1` (518 B)
  - `check-4` (881 B)
  - `verdict-approve` (416 B)

  The scratch `check-counter.log` has 4 lines across 6 `check` invocations, so the two check replays did not execute.
  `ledger_before_replays` equals `ledger_after_replays`, and `lua_executions_during_history` is 0. The run used the
  recorded candidate: `774bcd69...`, step 1 of `COMMANDS.txt`.
- **C01-R2 host-path files.** The identity-script path in `red-c01/history/RED.md:9` and `red-c01/support/RED.md:10`
  is now `<workflows-scripts>`. The driver `RED.md:8` shows `<temp-root>`. The first line of `inventory.txt` and line 3
  of `red/run_red.sh` show `<repo>`. `COMMIT-LIST.txt` holds no `/raw/` path. A `grep` for `/Users/`, `/private/`,
  `/var/folders`, `claude-501` and `.claude/projects` over every listed file, plus the C02 `REPORT.md` and
  `probes/PROBES.md`, finds only the following, so no real host path was found:
  - the six allowlisted support literals;
  - generic meta-mentions;
  - three pre-existing `/private/...` string literals in `execution.t.cpp` test data.

  The P6 README now defers to `COMMIT-LIST.txt` as the single authority, so the two commit lists no longer conflict.
- **R3 probe coverage.** Every F1 to F25, L2 to L19 and Q1 to Q19 row has a probe outcome in `probes/PROBES.md`
  (Table A) or in the carried `probes/superseded-3/PROBES.md`, each with a named test and its own raw log. The carried
  set has 129 logs and the final re-probe 22 logs plus a baseline. Spot checks:
  - F4, F5, F7 and F8 each show the applied diff, the before, mutated and restored file hashes, the scratch working
    directory, `Ran 1` and a named ERROR. In each case the ERROR is a driver `AcceptError` from the scenario's own
    validation, not an unrelated crash.
  - F11-b shows the mutant applied, both named tests passing, and `outcome: SURVIVOR`.

  Survivors were not relabelled. The INVARIANTS.md row ids equal the record's table, with four added repair rows
  (`comm` difference is exactly R1j-a, R1m-a, R1m-b and F11-b/L17-nf).
- **Harness red-first (H1 to H10).** The pre-fix snapshot `2d808518...` is verbatim identity output.
  `harness/red-run.out` shows `AssertionError` failures for H1, H2, H4 to H8, and for H9 and H10 on exit code, as the
  record states. It shows no compile or import failure. H3 is honestly a guard. `green-run.out` reports
  `Ran 69 / 3 / 28 ... OK`.
- **Gate (final-1).** Every `.exit` is 0. Both builds print no work. `05-suites` reports `accept_test` 51 and
  `acceptance_cli_test` 22, with zero skipped, failures and errors. `06-live` reports `Ran 61 tests ... OK`. Python is
  reported separately from ctest.
- **Test names (G1).** This reviewer independently parsed the four Python modules with `ast`: 134 test methods now,
  against the 108-id before-list, with none removed and 26 added. This matches the record. The C++ `comm` outputs are
  empty with exit 0.
- **No bytecode (A4).** No `__pycache__` or `.pyc` exists under the support tree.

## Findings

### R19-ACCEPT-A-C02-R1 (blocking): the heavy faulted run stopped at its first check and the record reports it as met

The record's acceptance table says "Heavy faulted CMake run executed with exit 0 and no abort | MET for the run". The
evidence README A1 note says "The heavy faulted run finished with exit 0". The retained heavy result in the session
scratch (`c02-heavy-scratch/result.json`, control `heavy.exit` 0, DONE `2026-11-12T04:37:31Z`) shows a stopped run:

- `trace` is `["activate", "claim", "check", "check-recover", "takeover", "check"]` and there is no `verdicts`,
  `replays`, `load`, `trajectory` or `ledger_*_replays` key. `counter_lines` is 3 and `intended_executions` is 4.
- The post-takeover `check-2` response (raw `036-check.stdout`) has `withheld: ["check_failed"]`. `cmake_debug` and
  `cmake_release` both report `outcome: fail`, `exit_status: 1`, matched 1851. The scenario therefore took the
  `fault_driver.stopped_result` branch (the F13 stop: "stopped at a withheld first completed check").
  `run_heavy.main` writes `result.json` and returns 0 whatever the result holds, so no `STOPPED*.md` or `ABORTED.json`
  was written.
- Cause, from the check evidence (`evidence/checks/{debug,release}/*/ctest.stdout`): in both presets
  `replacement.ledger format initializes exact metadata schema and Glaze event` fails at
  `src/engine/ledger/format.t.cpp:115`, `REQUIRE_FALSE( expected_sql.empty() )`. `src/engine/ledger/CMakeLists.txt:41`
  sets `PLANAR_LEDGER_SCHEMA` to `tests/native/r12b-evidence/expected-schema.sql`. That file exists in the checkout but
  is gitignored (`.gitignore:37`, `/tests/native/*-evidence/`) and is in neither `HEAD` nor `58128234`. `git cat-file`
  refuses it in both. The heavy run's export does not contain it. Any build of tracked sources, including a public
  clone, will fail that test. An earlier round already noted the file as "otherwise ignored required"
  (`V5-02-R15-R18-C11-experiments.md`).

The heavy run did observe the first half of the scenario: the interrupt with CLI return code -9 and the child group
dead, `check-recover` returning `interrupted`, takeover from generation 1 to 2, and the fenced old token with exit 3
and the ledger digest unchanged. The rejecting verdict, repair, `max_rounds` blocker, final approval, replay identity
and inspection-write checks were never reached on the heavy run. As the record states, the following are also absent:
`runs/cmake-rerun/index.json` (L20), `limits/limits.json`, `packet`, the index dry run, `index`, `scan`,
`verify-redaction` (L21), `raw/cmake-rerun-heavy/` and `export-retention.txt`. `runs/cmake-rerun/` holds only
`config-timeouts.txt`. C01-R1 is therefore **not closed**. The mini-fault part of it is closed.

This is a run-exposed defect. The dispatch says such a defect stops the round with a red-first regression proposal
(R1q). Instead the run was recorded as met and the cause was not identified.

Fix:
1. The coordinator decides whether the defect is a repository defect or a harness composition defect, and records
   the decision. A repository defect means a CMake test input that is not tracked; tracking the schema fixture or
   moving it out of the ignored evidence tree is a product or test change that needs its own red-first evidence. A
   harness composition defect means the composed base omits a required ignored input.
2. Make `run_heavy.py` exit nonzero, or write `runs/STOPPED-HARNESS-HEAVY.md`, when the result is a stopped result.
   Add a negative fixture for that change.
3. Re-run the heavy scenario under a new run name (C02-D8) until it reaches final approval.
4. Then run `rerun-index`, `limits`, `packet`, the index dry run, `index`, `scan` and `verify-redaction` as planned.
5. Correct the record's acceptance row and the A1 note in the README so that neither says the heavy run was met or
   finished.

### R19-ACCEPT-A-C02-R2 (blocking): the commit list is stale, never checked as final, and pins REPORT hashes

- `COMMIT-LIST.txt` (186 paths) names `tests/native/r19-accept-a-c02-evidence/probes/PROBES-reprobe.md`, which does
  not exist. It omits the C02 `REPORT.md` and `probes/PROBES.md`. The record says so (R2a and R2d NOT MET).
- The gate's redaction check (`raw/final-1/07-redact-check.*`) ran on that list and exited 0 with empty output even
  though one listed path does not exist. So exit 0 from that tool does not prove that every listed path was read. The
  final list must be existence-checked before the redaction run, which the D17 existence loop requires.
- The committable list contains `tests/native/r19-accept-evidence/runs/pre-index/report-shas.txt` and
  `tests/native/r19-accept-a-c02-evidence/redaction/w5-report-shas-now.out`. Both hold sha256 values of
  `r19-accept-a-evidence/REPORT.md` and `r19-accept-a-c01-evidence/REPORT-run2.md`. The record's Commit list section
  also cites "their check hashes" for the excluded REPORTs. This breaks the hash-pin rule (never REPORTs), and G3 is
  marked MET only because the record text itself holds no such hash.

Fix: drop the REPORT-hash files from the list, or justify them through an explicit coordinator amendment of the rule.
Correct the list. Run the existence loop and `redact-local-paths.py check` on exactly the final list, and record the
command, exit status and list.

### R19-ACCEPT-A-C02-R3 (blocking): the F11-b/L17 survivor's negative fixture cannot detect it

`FaultLiveTest.test_trace_is_derived_from_executed_steps_in_plain_cmake_run` has the docstring "Kills F11-b and L17".
Its own docstring also says the plain-base run "stops after its first withheld check", so the run returns through
`stopped_result` and never executes `finish()`, which is the mutated line. The final probe (`raw/F11-b.log`,
`raw/L17.log`) confirms that the test passes under the mutant. The survivor is reported honestly, but the fixture
added for it is a test that cannot fail on its stated mutant (standing lesson 1). Its docstring claims a kill that does
not happen. Row L17 (the trace observes the executed steps) has no detecting test.

Fix: add a fixture that completes through `finish()` with an executed verb sequence different from the 15-entry list.
For example, use the mini scenario with the `combined-max2` job, where the second rejection is not capped. Another
option is a direct `finish()` call on a driver with a non-default trace. Correct the existing docstring. Then
re-probe F11-b/L17 in a scratch copy. If neither fixture is possible, the coordinator must record the reason and keep
the rows as survivors.

### R19-ACCEPT-A-C02-A1 (advisory): two record citations name paths that do not exist

- Record line 772 cites `tests/native/r19-accept-a-c01-evidence/red-c01/history/RED.md`. The file is
  `tests/native/r19-accept-a-evidence/red-c01/history/RED.md`.
- Invariant A3 cites `gate/suites/summary.txt`. The directory holds `summary.txt.raw`.

### R19-ACCEPT-A-C02-A2 (advisory): the regenerated Python after-list is not retained

The record says the wave-9 after-list (`gate/test-names/after/py-worktree.txt`, 132 ids) is stale and that a
regenerated list of 134 ids was produced. No file holds it. The reviewer's independent parse confirms 134 ids with 0
removed and 26 added. Save the regenerated list beside the stale one so that G1 is reproducible from committed
evidence.

### R19-ACCEPT-A-C02-A3 (advisory): some fault-path kills are indirect

Several F-row kills are driver `AcceptError`s raised by a different self-check than the row's own assertion. For
example, F4, which rejects with an empty repair list, is killed by "orchestrator guidance is not usable for claim at
claim_builder_2" rather than by the repair assertion. These are genuine detections. The row text should still say
which check fired, so that a later refactor of that check does not silently uncover the row.

## Closure of the C01 findings

| C01 finding | status | basis |
|---|---|---|
| R1 faulted rerun, mini run and live steps | **not closed** | mini-fault run closed and verified; the heavy run stopped at its first check (C02-R1); `index.json`, `limits`, `packet`, `index`, `scan` and `verify-redaction` were not run |
| R2 committable evidence with host paths | **partly closed** | the five named files are redacted, raw trees are excluded and the lists are reconciled; the final list is stale and unchecked and carries REPORT hashes (C02-R2) |
| R3 fault-path rows with no probe outcome | **closed except F11-b/L17** | every in-scope row has an outcome with a named test and its own raw log; the F11-b/L17 fixture is hollow (C02-R3) |
| A1 false REPORT statements | closed in form | the README note supersedes the C01 REPORT; its own "heavy run finished" wording must change under C02-R1 |
| A2 G1 capture | closed with a gap | before and after directories exist and the C++ `comm` is empty; the regenerated Python after-list is not saved (C02-A2) |
| A3 suites command | closed | `run_acceptance.py suites` ran the non-live modules (51 + 22, zero skips); the live modules ran as a separate counted unittest step |
| A4 bytecode | closed | no `__pycache__` or `.pyc` in the support tree |
| A5 objective amendment, D4, owned-file amendment | closed | inline marker at objective item 1, a "C02 AMENDMENT D2" section and a "Statements for R19-ACCEPT-B" section |

## Limits of this review

- This reviewer ran no build, ctest, Python suite, mutant, redaction tool or scan. Gate, probe and red results were
  read from the retained raw logs. The host-path check was a `grep` over the listed files, not
  `redact-local-paths.py`.
- The heavy-run findings come from the session-scratch result, control and check-evidence files, which are outside
  the checkout and are not committed evidence. The next round should retain them, or their redacted index, under the
  evidence tree.
- The probe raw logs were spot-checked (F4, F5, F7, F8 and F11-b), not read in full.
- Whether the missing schema input is a product-repository defect or a harness composition defect is a coordinator
  call. This review states only the observed cause.
