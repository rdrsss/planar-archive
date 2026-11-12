# R19-ACCEPT-A-C02 final gate report (macOS Debug, product-unchanged shortcut)

Gate runner: final gate, 2026-11-12. No source file was edited. Raw logs: `raw/final-1/` (driver `driver.sh`, one `.cmd`, `.log`, `.exit` per step, `ALLDONE`). The earlier preliminary report is kept unchanged as `REPORT-prelim.md`; `raw/prelim-1` and `raw/prelim-2` are kept.

## Candidate identity

Workflow identity tool, pathspec `src cmake CMakeLists.txt CMakePresets.json scripts`, before and after the run, identical (`raw/final-1/identity-before.txt`, `identity-after.txt`, and equal to `raw/prelim-2/identity-before.txt`):

- HEAD `91e0df24f2167471e49ec057c7159bce5dff3608`
- sha256 of `git diff --binary HEAD -- src cmake CMakeLists.txt CMakePresets.json scripts`, tracked only: `83a16fdee033fce5a1a792d8d65f4090fba8dacffc2d38013684cacb4daeba19`; combined with untracked (none in this pathspec): the same value.
- `src/engine/ledger/execution.t.cpp` `250867b8bd74b86d718cbe17e5fa9983fda6a60a2b5e7c127829a3fec6f98950`
- `src/engine/ledger/storage.cpp` `e4cf41dcb8cf44cc8004d4ba011543a557c742c623e20ede631eb975f237d822`
- Debug candidate `build/rewrite-debug/src/cmd/planar/planar_authoring_stack` sha256 `d85ee4c3aa7078b357dac2223e2ad9c4be9ecb2c84497b5db337450189a81184` (`raw/final-1/03-candidate-sha256.txt`), equal to the C01 run4 candidate hash.

Support tree identity, pathspec `src tests/native/r19-accept-support`, run after the gate (`raw/final-1/identity-support-after.txt`): combined sha256 `988e085f2272f80c8422dc99ed2098b0f8ab9eaa0d3c82f79988569e8907e1d9`, the same value the mutation prober recorded in `probes/PROBES.md`. The newest support file is `acceptance_cli_test.py` (modified 01:36), older than the gate start (01:53), so the gated support tree is the probed tree.

## Product-unchanged shortcut (binding)

Per-file sha256 of both changed product files, the tracked diff hash and the set of changed paths (the two files, no untracked file under the pathspec) equal the values recorded for the C01 reviewed candidate (C01 `raw/run4/identity-after.txt`, and the C01 record sync in `docs/vernacular/rounds/R19-ACCEPT-A.md`). Therefore ctest, Release and the Linux Docker gate were NOT re-run. The full gate result is cited from `tests/native/r19-accept-a-c01-evidence/raw/run4`: macOS Debug and Release 1854/1854, Linux arm64 Debug and Release 1854/1854. Docker was not used, so no prune was needed (disk free on /System/Volumes/Data at start: 28 GiB).

## Commands (all from the repository root unless stated)

| # | command | exit | result |
|---|---|---|---|
| 1 | `cmake --build build/rewrite-debug --target planar_authoring_stack` | 0 | `ninja: no work to do.` |
| 2 | same again, no-change check after the successful build | 0 | `ninja: no work to do.`, zero steps: PASS |
| 3 | `ctest --test-dir build/rewrite-debug -N` (listing, not a run) | 0 | Total Tests: 1854 |
| 4 | `PYTHONDONTWRITEBYTECODE=1 python3 -B tests/native/r19-accept-support/run_acceptance.py suites --support-dir tests/native/r19-accept-support --module accept_test --module acceptance_cli_test --out <scratch>` with scratch HOME, PLANAR_DB and PLANAR_CANDIDATE=<Debug candidate> | 0 | accept_test ran 51, acceptance_cli_test ran 22, total 73, failures 0, errors 0, skipped 0 |
| 5 | `PYTHONDONTWRITEBYTECODE=1 PLANAR_CANDIDATE=<Debug candidate> python3 -B -m unittest -v fault_live_test quota_live_test` from `tests/native/r19-accept-support`, scratch HOME and PLANAR_DB | 0 | Ran 61 tests in 577.161s, OK; 61 `ok` lines, zero skipped lines (eight "skip" text hits are docstring words) |
| 5b | `python3 -B -m unittest discover -v -s . -p 'accept*_test.py'` from the support dir, same environment | 0 | Ran 73 tests in 9.301s, OK, zero skipped lines (matches command 4) |
| 6 | `python3 scripts/redact-local-paths.py check --repo-path "$PWD" --home "$HOME"` on the 186 paths of `COMMIT-LIST.txt` as it stood when the driver ran | 0 | no output |
| 6b | same on the current list (`raw/final-1/08-redact-list.txt`, 186 paths, with the renamed `probes/PROBES-reprobe.md` replaced by `probes/PROBES.md`; every path exists) | 0 | no output |

Python totals: 73 non-live plus 61 live = 134 tests, zero skips, zero failures. No `.pyc` or `__pycache__` exists under `tests/native/r19-accept-support` afterwards (`find` printed nothing). The `suites` command was run with the two non-live modules only (the recorded candidate record names the C01 export build, not the checkout Debug binary); live modules ran through command 5 with the explicit Debug candidate. All pass criteria are met: every exit code 0, nonzero counts, zero skips, no-change build zero steps.

Note for the committer: `COMMIT-LIST.txt` still names `probes/PROBES-reprobe.md`, which no longer exists (the prober renamed it to `PROBES.md` at 01:53, after the driver started; command 6 listed it and exited 0 regardless). The list should be updated before commit; I did not edit it. Superseded plan or probe copies (`probes/superseded-*`) and `raw/` are not on the list.

## Inventory

`inventory.txt` is the sorted `ctest -N` name list (1854 names, from `raw/final-1/04-ctest-list.log`); byte-identical to the file already in place. Compared with the most recent committed inventory, `tests/native/r19-accept-a-c01-evidence/inventory.txt` (1857 lines: header, 1854 `Test #n:` rows, footer; names after stripping numbering and sorting): identical, no new names, no removed names (`cmp` of the two sorted lists, exit 0). The C++ names are unchanged because `src/` is unchanged. Python tests are not ctest-registered.

## (a) Acceptance criteria to tests (current line numbers, re-derived with grep)

C++ history regression (carried from C01, in the product candidate): `src/engine/ledger/execution.t.cpp:4086` "R19-ACCEPT-A-C01 history event of a recorded check keeps evidence integer nodes free of the boolean flag", `:4111` "... boolean nodes free of the integer value", `:4132` "... preserves evidence integer and boolean values exactly". They were not re-run here (shortcut); they passed in C01 run4.

| criterion (C02 record) | test names (file:line) | status |
|---|---|---|
| R1 rerun-index normalizes real absolute argv (H1) | `AcceptanceCliTest.test_rerun_index_normalizes_real_shaped_absolute_argv` acceptance_cli_test.py:624 | passes |
| host path outside argv still refused (H3, guard) | `AcceptanceCliTest.test_rerun_index_still_refuses_host_path_outside_argv` acceptance_cli_test.py:652 | passes |
| exact real error envelope accepted only (H2) | `AcceptLibTest.test_capture_limit_accepts_exact_real_error_envelope_only` accept_test.py:510 | passes |
| `limits` accepts the real envelope (H4) | `AcceptanceCliTest.test_limits_command_accepts_real_error_envelope` acceptance_cli_test.py:667 | passes |
| NUL next to host path is indexed (H8) | `AcceptanceCliTest.test_index_escapes_nul_next_to_host_path_in_binary_raw_file` acceptance_cli_test.py:693 | passes |
| index dry run refuses unredactable raw, writes nothing (H9) | `AcceptanceCliTest.test_index_dry_run_refuses_unredactable_raw_file_and_writes_nothing` acceptance_cli_test.py:749 | passes |
| dry run reports expected count, writes nothing (H10) | `AcceptanceCliTest.test_index_dry_run_reports_expected_count_and_writes_nothing` acceptance_cli_test.py:766 | passes |
| heavy wait budget keys (H5) | `FaultLiveTest.test_run_heavy_config_wait_timeouts_cover_registry_budget` fault_live_test.py:517 | passes |
| marker wait honors config (H6) | `FaultLiveTest.test_marker_wait_honors_configured_timeout` fault_live_test.py:535 | passes |
| completion timeout aborts and kills CLI (H7) | `FaultLiveTest.test_check_completion_timeout_aborts_with_accept_error_and_kills_cli` fault_live_test.py:552 | passes |
| R3 survivor F11-b, L17 | `FaultLiveTest.test_trace_is_derived_from_executed_steps_in_plain_cmake_run` fault_live_test.py:579 | passes; mutant outcome per `probes/PROBES.md` (the prober's table, not re-derived here) |
| R3 F20 | `FaultLiveTest.test_blank_guidance_is_refused_by_the_usability_predicate` fault_live_test.py:598 | passes |
| R3 F21-a | `FaultLiveTest.test_run_heavy_config_names_the_composed_base` fault_live_test.py:605 | passes |
| R3 F22-a | `FaultLiveTest.test_kill_check_signals_the_marker_pgid_when_it_differs_from_the_marker_pid` fault_live_test.py:610 | passes |
| R3 L15 | `FaultLiveTest.test_every_check_attempt_folder_keeps_its_command_logs` fault_live_test.py:646 | passes |
| R3 Q2-a | `QuotaLiveTest.test_combined_builder_root_carries_the_current_overlay_not_the_base_copy` quota_live_test.py:471 | passes |
| R3 Q7 | `QuotaLiveTest.test_every_history_event_has_a_replayed_state_equal_to_its_recorded_state` quota_live_test.py:481 | passes |
| R3 Q11-a | `QuotaLiveTest.test_session_argv_names_scratch_paths_and_never_the_checkout_fixture` quota_live_test.py:495 | passes |
| R3 Q12-b | `QuotaLiveTest.test_root_view_places_the_contract_even_when_its_source_lacks_it` quota_live_test.py:512 | passes |
| R3 Q13-b | `AcceptLibGapTest.test_check_runner_passed_excludes_failures_and_skips` accept_test.py:1056 | passes |
| R3 L7-a | `AcceptLibGapTest.test_guidance_predicate_refuses_text_without_job_role_line_or_verb` accept_test.py:1065 | passes |
| R3 Q14-b | `QuotaLiveTest.test_counter_only_replay_tamper_leaves_an_aborted_record_naming_the_replay` quota_live_test.py:523 | passes |
| R3 Q15 (L19) | `QuotaLiveTest.test_unusable_guidance_is_refused_by_the_usability_predicate` quota_live_test.py:532 | passes |
| R3 L6 | `QuotaLiveTest.test_regroup_fenced_probe_presents_the_retired_builder_token` quota_live_test.py:541 | passes |
| R3 R1j-a, R1m-a, R1m-b (added by the latest repair, named in `probes/PROBES.md`) | `test_rerun_index_names_the_missing_top_level_key_itself`, `test_index_lists_only_host_lines_and_refuses_out_dir_other_than_work_cwd` in acceptance_cli_test.py (line numbers not re-derived here) | pass (inside the 73 non-live tests); kill outcomes per `probes/PROBES.md` |
| A3 Python suites reported separately from ctest | commands 4, 5, 5b | met |
| A4 no `.pyc` in the support tree | `find` printed nothing | met |
| R2 redaction check on the exact list | commands 6 and 6b, exit 0 | met (list needs the PROBES.md rename fixed, see note) |
| R1 heavy faulted run, `rerun-index`, `limits`, `packet`, `index`, `scan`, `verify-redaction` live evidence | not exercised by this gate | outside this gate; status is the round record's, not re-claimed here |
| A1, A2, A5 (records fixes) | A2 name-list checks live in `tests/native/r19-accept-evidence/gate/test-names/`; A1 and A5 are record edits | not gate results |

Mutation-kill rows for the full F1 to F25, L, Q set: the authority is `probes/PROBES.md` and `probes/superseded-3/PROBES.md` plus `raw/`; this gate did not apply mutants.

## (b) Red-first

Source: `harness/RED.md`, `harness/red-run.out`, `harness/green-run.out`, `harness/pre-fix-identity.txt`. Pre-fix snapshot combined sha256 of `tests/native/r19-accept-support`: `2d8085186ebff43d125415fec54e7c1282773aed0b8063b05cc5afaa00cc01cb`. Command form recorded in `harness/RED.md`, from `tests/native/r19-accept-support` with scratch HOME and PLANAR_DB: `PYTHONDONTWRITEBYTECODE=1 ... python3 -B -m unittest <module>.<Class>.<test> ...` (non-live rows in one invocation, the three live rows in a second; the exact full command text is in that file, not repeated here).

| test | failing line on the unfixed tree | real red or guard |
|---|---|---|
| H2 `test_capture_limit_accepts_exact_real_error_envelope_only` | `AssertionError: False is not true` (exact real envelope rejected) | real red |
| H1 `test_rerun_index_normalizes_real_shaped_absolute_argv` | `1 != 0` (rerun index would hold a host path) | real red |
| H3 `test_rerun_index_still_refuses_host_path_outside_argv` | passed | guard |
| H4 `test_limits_command_accepts_real_error_envelope` | `1 != 0` (envelope not recognized as the closed diagnostic) | real red |
| H8 `test_index_escapes_nul_next_to_host_path_in_binary_raw_file` | `1 != 0` (host prefix remains after redaction) | real red |
| H9 `...dry_run_refuses_unredactable_raw_file_and_writes_nothing` | `2 != 1` (unknown `--dry-run` flag) | red on exit code only |
| H10 `...dry_run_reports_expected_count_and_writes_nothing` | `2 != 0` (unknown flag) | red on exit code only |
| H5 `test_run_heavy_config_wait_timeouts_cover_registry_budget` | `marker_timeout_s` / `check_timeout_s` not found | real red |
| H6 `test_marker_wait_honors_configured_timeout` | `None is not an instance of AcceptError` | real red |
| H7 `test_check_completion_timeout_aborts_with_accept_error_and_kills_cli` | `None is not an instance of AcceptError` | real red |

Survivor negative fixtures and later repair tests (R1j-a, R1m-a, R1m-b) are negative controls added after mutation probing; they pass on the unfixed tree by design, and their red evidence is the mutant run recorded in `probes/PROBES.md`. The C01 history regression red evidence is `tests/native/r19-accept-a-c01-evidence/red/RED.md` (failing lines execution.t.cpp 4002, 4004, 4101, 4105, 4125 at red time; the tests have since moved to 4086/4111/4132 by file edits). Missing from committed evidence: per-mutant raw logs for the earlier KILLED rows beyond what `probes/raw` holds, and a RED snapshot hash for the repair-only tests; I did not invent either.

## (c) Deliberate behaviour differences

No product behaviour changed in C02 (`src/` identical to the C01 candidate). Support-driver changes (`harness/FIX-NOTES.md`): every CLI verb and every non-held check starts in its own session so `kill_session` can signal it; new config keys `marker_timeout_s` and `check_timeout_s`; new `index --dry-run`. These are test-harness behaviour only; no closed diagnostic message was altered (new harness messages only). No contract clause is required beyond the round record's D31 rows H1 to H10.

## Limitations

- Release, Linux Docker and the full ctest run were not re-run (product-unchanged shortcut); cited from C01 run4.
- `suites` ran only the non-live modules; the live modules ran through plain unittest with the explicit Debug candidate.
- H9 and H10 are red only through the unknown-flag exit code; the `_run` 900 s communicate-timeout conversion has no observer; held-check marker timeout does not kill the held CLI (harness limit 4); survivor status of any mutant is whatever `probes/PROBES.md` states.
- Heavy-run post-processing (rerun-index, limits, packet, index, scan, verify-redaction on the canonical tree) is not a result of this gate.
- COMMIT-LIST.txt names a path that no longer exists (see above).
