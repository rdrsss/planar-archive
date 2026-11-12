# R19-ACCEPT-A-C01 final gate report (run4)

Final gate for the corrective round. No source file was edited. Raw logs are under `raw/run4/` (driver `driver.sh`; each step has `.cmd`, `.log` and `.exit`; Linux output under `raw/run4/linux/`). Earlier runs `raw/prelim-1`, `raw/prelim-2`, `raw/run1` to `run3` and `REPORT-run2.md` are kept untouched. A copy of the previous preliminary report is `raw/REPORT-prelim2.md.bak`.

Verdict: every command met its criterion. passed = true.

## 1. Candidate identity

`raw/run4/identity-before.txt` and `identity-after.txt` are byte-identical (workflow identity tool, run from the repository root).

| identity | value |
|---|---|
| HEAD | `91e0df24f2167471e49ec057c7159bce5dff3608` |
| sha256(git diff --binary HEAD -- src tests/native), tracked only | `83a16fdee033fce5a1a792d8d65f4090fba8dacffc2d38013684cacb4daeba19` |
| combined sha256 (diff bytes, then each untracked path NUL bytes, sorted), pathspec src tests/native, evidence excluded | `6612206823f3350ea5e9ce36bf9f39750a553894ca6624941379f2b200dd9891` |

The per-file table (two modified src files and 21 untracked support files) is in `identity-before.txt`. This identity equals prelim-1, prelim-2 and run3. The candidate binary for the Python suites is `build/rewrite-debug/src/cmd/planar/planar_authoring_stack`, sha256 `d85ee4c3aa7078b357dac2223e2ad9c4be9ecb2c84497b5db337450189a81184` (`12-candidate-sha256.txt`).

## 2. Commands

| # | command | exit | result |
|---|---|---|---|
| 1 | `cmake --preset debug` | 0 | configured |
| 2 | `cmake --build --preset debug --target planar_tests` | 0 | built |
| 3 | `ctest --test-dir build/rewrite-debug --output-on-failure` | 0 | 100% passed out of 1854, no failures, no Timeout (1151.07 s) |
| 4 | `python3 scripts/rewrite-check.py gate --build-dir build/rewrite-debug` | 0 | "Executed 1854 registered tests; zero skips"; no rerun needed |
| 5 | `cmake --preset release` | 0 | configured |
| 6 | `cmake --build --preset release --target planar_tests` | 0 | built |
| 7 | `ctest --test-dir build/rewrite-release --output-on-failure` | 0 | 100% passed out of 1854 (418.73 s) |
| 8 | `python3 scripts/rewrite-check.py gate --build-dir build/rewrite-release` | 0 | "Executed 1854 registered tests; zero skips" |
| 9 | `cmake --build --preset debug --target all planar_tests` (after successful build) | 0 | `ninja: no work to do.` (zero steps) |
| 10 | `cmake --build --preset release --target all planar_tests` (after successful build) | 0 | `ninja: no work to do.` (zero steps) |
| 11 | `ctest --test-dir build/rewrite-debug -N` | 0 | Total Tests: 1854 |
| 12 | `cmake --build build/rewrite-debug --target planar_authoring_stack` | 0 | built |
| 13 | Python support suites (`python3 -m unittest discover -s tests/native/r19-accept-support -p '*_test.py' -v`), `PLANAR_CANDIDATE` set to the absolute authoring-stack path, scratch HOME and PLANAR_DB, run once | 0 | Ran 108 tests in 495.137 s, OK, zero skipped, zero failures (the word "skipped" appears only in three test docstrings) |
| 14 | `scripts/redact-local-paths.py check` with explicit files (`13-redact-files.txt`, 23 paths: src and the support tree plus the round record) | 0 | passed; no argument-less run |
| 15 | `scripts/linux-gate.sh <scratchpad>/lg-r19-accept-run5` | 0 | linux/arm64 Debug and Release: configure, build, ctest all exit 0, "100% tests passed out of 1854" each, inventory 1854 each, no-work builds `ninja: no work to do.`; host manifest equal before and after build and equal to the container context manifest |
| 16 | `docker builder prune -af --filter 'description~=home/planar'` twice | 0, 0 | both reported Total 0B (the gate script had already pruned) |

The `bin/planar` binary was not used as the Python candidate, per the operator correction. No Debug rerun was needed (no Timeouts).

Disk: before the Linux gate `df -h /System/Volumes/Data` showed 28 GiB available (above the 15 GiB threshold); after the gate and both prunes it showed 12 GiB (`linux-df-after.txt`). At the end of the run it showed 28 GiB again as the checkout's scratch cleared. The 12 GiB reading is a limitation worth the operator knowing: the gate temporarily consumed about 16 GiB.

## 3. Test inventory

`inventory.txt` (ctest -N, 1854 tests) is byte-identical in names to this run's `11-ctest-list-debug.log` and to the Linux container inventories. Comparison base: the 1851-test run-1 list in `tests/native/r19-accept-a-evidence/raw/run1/04-debug-ctest-N.log`; no names removed, 3 added, all in `src/engine/ledger/execution.t.cpp` (the "R19-ACCEPT-A-C01 history event of a recorded check ..." integer, boolean and exact-values tests). No committed inventory after R15 exists (newest: `tests/native/v5-02-x08-c01-r7-evidence/inventory.txt`). New names versus HEAD's committed state are therefore only those three plus the Python suites, which are not registered with ctest.

Zero-work result: commands 9, 10 and the Linux no-work builds all reported `ninja: no work to do.`

## 4. Acceptance criteria and findings mapped to tests

Round record criteria (docs/vernacular/rounds/R19-ACCEPT-A.md, R19-ACCEPT-A-C01) to the plan rows in `plan/INVARIANTS.md`:

| criterion | rows |
|---|---|
| History defect: red-first behavioural regression, minimal fix, diagnostics unchanged | H1, H2, H3, H5, H6 |
| Quota combined and split complete, raw per action, byte-identical replay after restart, exact-target independent approval | L1a, L1b, L2, L3, L24, Q1 to Q19 |
| Faulted CMake rerun: faults, rejection then repair, max_rounds blocker, final approval, no re-execution, no inspection writes | L8 to L20, F1 to F25 |
| 22 mutants killed, A6 green | RF1, RF2 and the `AcceptLibGapTest` plus extra `AcceptanceCliTest` tests (killing is the prober's job, not part of this gate) |
| Python suite counts reported, ctest nonzero with zero skips | L22, S1 to S3 and section 2 |
| Redaction passes, no host paths | section 2 (fails on one test fixture) |

Per row (file:line is the current checkout; Python files are under `tests/native/r19-accept-support/`):

| row | invariant (abridged) | test file:line |
|---|---|---|
| H1 | history --event integer evidence nodes carry no boolean flag | execution.t.cpp:4086 |
| H2 | the boolean evidence node `output_truncated` is `kind == boolean`, `boolean == true`, `int | execution.t.cpp:4111 |
| H3 | integer evidence nodes keep exact values (`duration_ms == 42`, `output_bytes == 7`) and a  | execution.t.cpp:4132 |
| H4 | End to end through the candidate: `QuotaLiveTest.test_history_event_and_at_event_reproduce | quota_live_test.py:214 |
| H5 | closed diagnostics byte-identical; runtime.cpp and production Lua untouched | external_registration.t.cpp:4591 (X08 C01 role preparation outputs-off boundary; passes) |
| H6 | The new C++ regression (H1, H2) is red on the unfixed source with a behavioural assertion  | accept_test.py:539 |
| S1 | per-module counts reported separately and totals summed: two synthetic modules (3 and 2 pa | acceptance_cli_test.py:429 |
| S2 | a skipped test, a failing test, an erroring import and a zero-test module each make the co | acceptance_cli_test.py:458 |
| S3 | a `--live-module` refuses to run (nonzero exit, marker file absent) when `PLANAR_CANDIDATE | acceptance_cli_test.py:481 |
| R1 | `run_acceptance.py rerun-index --result FILE --out FILE`: given a synthetic `result.json`  | acceptance_cli_test.py:514 |
| R2 | `rerun-index` exits nonzero naming the field when ANY one required field is missing or con | acceptance_cli_test.py:529 |
| G1 | no existing test removed or renamed | section 3 (ctest -N diff: 0 removed, 3 added) |
| RF1 | each of the 14 repair-pass tests (12 in `AcceptLibGapTest` accept_test.py:869-998: scan d | accept_test.py:539 |
| RF2 | repair-pass tests, scratch-clone record | tests/native/r19-accept-a-evidence/red-c01/rf/RED.md (no red possible, section 5) |
| L1a | quota combined: raw argv/stdout/stderr/exit retained for every action, one `.exit` and one | quota_live_test.py:161 |
| L1b | quota combined: exact-target independent approval through the CLI by `review_session`, a n | quota_live_test.py:103 |
| L2 | quota split completes with dependency order | quota_live_test.py:127 |
| L3 | restart replay byte-identical and executes no check (counter file unchanged) | fault_live_test.py:199, quota_live_test.py:178 |
| L4 | inspection (`history`, `status`, `orchestrator`) never writes the ledger | quota_live_test.py:232 |
| L5 | wrong-target and worker-token verdicts are refused with the closed diagnostics and leave t | quota_live_test.py:192 |
| L6 | regroup combined to split from the same spec bytes fences old tokens | quota_live_test.py:265 |
| L7 | reference orchestrator guidance is usable at every phase (quota and fault phases) | fault_live_test.py:251, quota_live_test.py:245 |
| L8 | fault: an interrupted check is recovered by `check-recover` | fault_live_test.py:86 |
| L9 | fault: state survives a process restart between phases | fault_live_test.py:104 |
| L10 | fault: takeover fences the old run token | fault_live_test.py:125 |
| L11 | fault: a rejecting verdict with repair is admitted (count 1 of max_rounds) and the worker  | fault_live_test.py:138 |
| L12 | fault: the second rejection is capped and opens a `max_rounds` blocker; a load raising `ma | fault_live_test.py:153, fault_live_test.py:168 |
| L13 | fault: final approval after all faults targets the exact latest check output | fault_live_test.py:178 |
| L14 | fault: history references through faults reproduce (event and at-event) | fault_live_test.py:208 |
| L15 | fault: failed and interrupted attempts remain retained | fault_live_test.py:222 |
| L16 | the first completed check passes the boundary rows (observed `recorded`, test expects `wit | fault_live_test.py:277 |
| L17 | scenario driver is configuration parameterized (verb list differs by extra claim and verdi | fault_live_test.py:236 |
| L18 | fault call-site aborts: `run_scenario` aborts on tampered replay, counter increase, inspec | fault_live_test.py:312 |
| L19 | quota call-site aborts: tamper replay, unusable guidance, inspection change, mismatched hi | quota_live_test.py:403 |
| L20 | heavy faulted CMake rerun shows each injected fault with observed recovery, a rejecting ve | acceptance_cli_test.py:514, acceptance_cli_test.py:529, fault_live_test.py:346, fault_live_test.py:376, fault_live_test.py:445 |
| L21 | `limits`, `packet`, `index`, `scan`, `verify-redaction` recorded against the committed lay | acceptance_cli_test.py:129, acceptance_cli_test.py:155, acceptance_cli_test.py:191, acceptance_cli_test.py:226, acceptance_cli_test.py:290 |
| L22 | Python suite counts reported separately from ctest | S1 to S3 rows (acceptance_cli_test.py) and raw/run2/12-py-suites.log, 12b-py-suites-authoring-stack.log |
| L23 | candidate rebuilt with the fix and re-recorded: executable sha256 from `shasum -a 256` equ | accept_test.py:509, accept_test.py:772 |
| L24 | quota SPLIT grouping, end to end: raw argv/stdout/stderr/exit retained for every action (g | quota_live_test.py:420 |
| Q1 | Quota fixture copied from fixtures/render/quota: combined and split load from the byte-ide | quota_live_test.py:90 |
| Q2 | Combined grouping completes: one builder (library+command), one root view with all edits p | quota_live_test.py:103 |
| Q3 | Split grouping per D30 root topology: cli_worker is claimable at phase start; a cli_worker | quota_live_test.py:127 |
| Q4 | Every driver action retains raw argv, stdout, stderr and exit files; numbering gapless; fi | quota_live_test.py:161 |
| Q5 | After a process restart every mutation (activate, claim, check, verdict) replays byte-iden | quota_live_test.py:178 |
| Q6 | Wrong-target and non-independent verdicts are refused with a typed error and leave ledger  | quota_live_test.py:192 |
| Q7 | history --event/--at-event after restart reproduce accepted declarations, dispatch, check  | quota_live_test.py:214 |
| Q8 | Inspection reads (status, history, orchestrator, export) never write: ledger digest identi | quota_live_test.py:232 |
| Q9 | Reference orchestrator renders usable guidance (I20 predicate, next verb as input) on the  | quota_live_test.py:245 |
| Q10 | Regrouping A22/A33: after a completed combined run, load split.lua from the same spec byte | quota_live_test.py:265 |
| Q11 | The fixtures/render/quota tree is copied into scratch and left unmodified (tree digest of  | quota_live_test.py:283 |
| Q12 | Contract delivery (D2/D30): the driver places text/quota-api.md, byte-equal to the loaded  | quota_live_test.py:314 |
| Q13 | The recorded check evidence of each executed quota check shows matched, passed and failed  | quota_live_test.py:336 |
| Q14 | Call site: run_grouping aborts with AcceptError when config tamper.replay alters the repla | quota_live_test.py:359 |
| Q15 | Call site: run_grouping aborts with AcceptError when config tamper.guidance replaces the o | quota_live_test.py:365 |
| Q16 | Call site: run_grouping aborts with AcceptError when config tamper.inspection changes ledg | quota_live_test.py:371 |
| Q17 | Call site: run_grouping aborts with AcceptError when config tamper.expansion or tamper.at_ | quota_live_test.py:377 |
| Q18 | Detached inspection (D33): after load, with the scratch workspace .lua files and registry  | quota_live_test.py:385 |
| F1 | Interrupted check: the driver SIGKILLs the planar check CLI process (the session crash) af | fault_live_test.py:86 |
| F2 | Process restart: status/history/orchestrator reads before and after the kill are served by | fault_live_test.py:104 |
| F3 | Takeover after the interrupted check yields a new generation; the old token is fenced (typ | fault_live_test.py:125 |
| F4 | A rejecting verdict with repair naming the worker is admitted (count 1 of max_rounds); wor | fault_live_test.py:138 |
| F5 | With max_rounds 1 the second rejection is capped, no repair written, a max_rounds blocker  | fault_live_test.py:153 |
| F6 | A load that raises max_rounds clears the blocker (blockers.cleared lists it) and re-binds  | fault_live_test.py:168 |
| F7 | Final approval after all faults accepts the exact latest check output (the repair edits ar | fault_live_test.py:178 |
| F8 | Check execution counter (counting wrapper in the mini registry; attempt folders in the cma | fault_live_test.py:199 |
| F9 | history --event/--at-event at each fault event (interrupt, recover, takeover, reject, capp | fault_live_test.py:208 |
| F10 | Failed, interrupted and refused attempts keep raw logs; the driver never deletes or overwr | fault_live_test.py:222 |
| F11 | One run_scenario code path serves the mini fixture and the cmake-rerun configuration: the  | fault_live_test.py:236 |
| F12 | Orchestrator guidance passes the I20 predicate (with the next scripted verb as input) at t | fault_live_test.py:251 |
| F13 | With the composed base (HEAD engine tree plus the 16 owned CMake files from 58128234) the  | fault_live_test.py:277 |
| F14 | Call site: run_scenario aborts with AcceptError when tamper.omit_reject makes the trajecto | fault_live_test.py:297 |
| F15 | Call site: run_scenario aborts with AcceptError when tamper.inspection changes ledger byte | fault_live_test.py:305 |
| F16 | Call site: run_scenario aborts with AcceptError when tamper.replay alters the replayed res | fault_live_test.py:312 |
| F17 | Call site: run_scenario aborts with AcceptError when tamper.counter appends a counter line | fault_live_test.py:319 |
| F18 | Call site: run_scenario aborts with AcceptError when tamper.expansion corrupts a recorded  | fault_live_test.py:326 |
| F19 | Call site: run_scenario aborts with AcceptError when tamper.at_event corrupts a recorded r | fault_live_test.py:333 |
| F20 | Call site: run_scenario aborts with AcceptError when tamper.guidance blanks the orchestrat | fault_live_test.py:340 |
| F21 | run_heavy.build_config returns the heavy configuration: mode cmake, check_mode real, base  | fault_live_test.py:346 |
| F22 | Kill path against the real writer: fault_driver.kill_check, given a fake CLI parent that s | fault_live_test.py:376 |
| F23 | Detached inspection (D33) through the fault run: after the load and takeover, with scratch | fault_live_test.py:426 |
| Q19 | Call site (legal-action binding, E13 A / R19 section 4): run_grouping performs every live  | quota_live_test.py:403 |
| F24 | Call site (candidate binding, D28): `run_heavy.main` calls `accept_lib.require_recorded_ca | fault_live_test.py:445 |
| F25 | Call site (legal-action binding): run_scenario performs every live mutating action only af | fault_live_test.py:462 |

Rows with a shorthand name in the plan (L3, L7, L12, L18, L19) list every test whose name the table could resolve; the siblings are the `..._` abbreviated test names in `plan/INVARIANTS.md`.

Contract-level criteria that these tests do not alone prove: the live acceptance runs themselves (quota combined and split, mini faults, heavy CMake rerun) are kept as evidence under `tests/native/r19-accept-evidence/` and were not re-executed by this gate; the gate re-ran only the unittest suites that exercise them.

## 5. Red-first records

Every record below comes from the coders' evidence; I ran none of these commands, and no mutant was applied by this gate.

### C++ history regression (`src/engine/ledger/execution.t.cpp`)

Record: `tests/native/r19-accept-a-evidence/red-c01/history/RED.md`. Pre-fix snapshot (`identity.py src`, combined row): `b5b9a3fb357c2ded89b7deb8ff5ba5550dc5d1fd359c22ba2e28099ca1bf4ec4`. Command: `ctest --test-dir build/rewrite-debug -R "R19-ACCEPT-A-C01 history event of a recorded check" --no-tests=error --output-on-failure`, exit 8, "33% tests passed, 2 tests failed out of 3", raw `red-c01/history/raw/history-red-combined.log`; `validate_red_record` ACCEPTED.

| test | outcome | failing line |
|---|---|---|
| H1 integer nodes free of the boolean flag (execution.t.cpp:4086) | real red | `:4101`, `:4105` (also `:4002`, `:4004` from the shared walk) |
| H2 boolean nodes free of the integer value (:4111) | real red | `:4125` |
| H3 preserves values exactly (:4132) | guard, already passed on the unfixed source (red only via the prober's clamp and always-true mutants) | none |

Fix: `src/engine/ledger/storage.cpp` `history_rows` (`kind 'n'` sets only `integer`, `'b'` sets only `boolean`). The fix is green in command 3, 7 and 14. `runtime.cpp` and production Lua are unchanged, so `c02_off_boundary` needed no re-pin; its guard `X08 C01 role preparation outputs-off boundary pins the request-record conversion charge` passes in all four runs.

### Support code (suites and rerun-index)

Record: `tests/native/r19-accept-a-evidence/red-c01/support/RED.md`. Snapshot (combined, support tree): `91825501bd7c85bae8ddc6d7f5154ba8a25725f01313d8d14311c65852eb0c1f`. Command (abridged in the record; the full text is in `red-c01/support/raw/support-red-five-record.log`): `cd tests/native/r19-accept-support && PYTHONDONTWRITEBYTECODE=1 python3.15 -B -m unittest -v acceptance_cli_test.AcceptanceCliTest.<five tests>`, exit 1, `Ran 5 tests`, `FAILED (failures=45)` (stub returns exit 2). All five are real reds: S1 (`:443`), S2 (`:475`, `:478`), S3 (`:510`), R1 (`:520`), R2 (`:591`).

### Driver tests (D39)

Records `tests/native/r19-accept-a-evidence/red-c01/driver/<Class.test>/RED.md`, each run against the candidate built from the fixed source, snapshot (combined, support tree before the driver fix) `7cbadf5e190627978a86ac4610030977d66b5d4f91a228e2a6ddd1bc2524b16f`, exit 1, `validate_red_record` accepted. Each command is in that directory's `raw/command.txt`. Real reds: `QuotaLiveTest.test_quota_check_results_match_actual_unittest_counts` (no `Ran` line), `...test_run_grouping_aborts_before_action_not_listed_as_legal` (`'orchestrator' != 'check'`), `...test_split_grouping_orders_dependency_and_completes` (`2 != 1`), `...test_regroup_combined_to_split_fences_old_tokens` (`1 not greater than 1`), `...test_wrong_target_verdict_is_refused_and_ledger_unchanged` (`0 == 0`), `FaultLiveTest.test_final_approval_after_faults_is_exact_target` (`0 == 0`), `...test_first_completed_check_passes_boundary_rows` (`'recorded' != 'withheld'`), `...test_scenario_driver_is_config_parameterized` (list differs), `...test_orchestrator_guidance_usable_through_fault_phases` (`False is not true`), `...test_run_scenario_aborts_before_action_not_listed_as_legal`, `...aborts_on_mismatched_at_event_state`, `...aborts_on_mismatched_event_expansion`, `...aborts_on_tampered_replay`, `...aborts_when_counter_increases_on_replay`, `...aborts_when_inspection_changes_ledger`, `...aborts_when_trajectory_omits_rejection` (each `AcceptError not raised`). I did not re-open every raw record; the failing lines are those printed by the `RED.md` summaries.

Not red-first (missing or impossible):
- `QuotaLiveTest.test_split_grouping_retains_raw_exact_targets_and_non_builder_actor` (L24): in `red/RED.md` it is "not registered (AttributeError, method absent)" before the edit; the test is new and its red is only the AttributeError, which the plan treats as not behavioural. It passes now.
- The earlier `red/RED.md` pass (before the driver fix, debug binary as candidate) records L13, L16, L17, L19 as crash and L18 as passing; the later D39 records above supersede it with the correct candidate.
- 14 repair-pass tests (12 `AcceptLibGapTest` plus 2 in `AcceptanceCliTest`): no red possible, because repair pass 1 changed tests only (`red-c01/rf/RED.md`). Their only meaningful red is the prober's mutant kill; this gate did not run mutants.
- H3, H5 and the Quota/Fault rows listed `false` in the plan are guards or live runs; the plan's D33 treatment applies.
- G1 "gate stage comm -23" has no registered script; this report's section 3 is the manual equivalent.

## 6. Deliberate behaviour differences

- Driver expectations were corrected where the contract, not the candidate, was right (class (b) test-expectation defects in the D39 records): a verdict built from an earlier output or the wrong target is recorded historical with exit 0 and is not a refusal (execution-contract-r18.md:144, R18-3; execution-design.md A06); the first completed check on a plain base is `recorded` with `withheld ['boundary']` (execution-contract-r17.md:408); after regroup each (job, role) lineage starts again at generation 1 (execution-contract.md:820, M1); each registered check, not each root, yields an output entry (execution-contract-r17.md:408). I verified these citations only through the records, not by reading the contract files.
- Product: `history --event` no longer sets both integer and boolean on a stored scalar; the old output violated the runtime's structural-value rule (`runtime.cpp:210`), which raised the closed `internal` diagnostic. No diagnostic text changed.

## 7. Limitations

- The Python suites were scored only with the source-built `planar_authoring_stack`; the `bin/planar` candidate lacks the orchestrator verb by design and was not run.
- The live acceptance runs themselves (quota, faults, heavy CMake rerun) are kept in `tests/native/r19-accept-evidence/` and were not re-executed beyond what the unittest suites drive.
- The Linux gate covers arm64 only. The filtered prunes removed 0B because the gate script had already pruned. Free disk dipped to 12 GiB after the Linux gate.
- Mutant kills are the prober's job; this gate applied no mutants and ran none of the red-first commands.
- Ctest and Python test names were compared only against run 1 and the previous round's inventory; no committed list exists after R15.
- I verified the contract citations in section 6 only through the coders' records.
