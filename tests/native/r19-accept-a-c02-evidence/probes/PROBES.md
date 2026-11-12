# R19-ACCEPT-A-C02 mutation probes (focused re-probe after survivor repair)

Run 2026-11-12 by the mutation prober. The previous PROBES.md, raw/ and the earlier results were moved to `superseded-3/` (`superseded-3/PROBES.md`, `superseded-3/raw/`); nothing was deleted. Mutants were applied only in the persistent scratch copy `<scratch-root>/probe-warm` (synced from the checkout with `rsync -a --delete --exclude build/ --exclude .git/ --exclude 'tests/native/*-evidence/'`; its build directory kept). Each mutant is the verbatim diff of the earlier run (diff sha256 equal to the superseded record), applied with `patch -p1` after a dry run, the run chained on a successful apply, then the file restored by byte copy and its sha256 verified (all 22 restores EQUAL). Every mutated file is Python, so no C++ target was rebuilt (the repair changed only `acceptance_cli_test.py`; no `src` change). Runner: `env`-scrubbed (PATH, LANG, PYTHONDONTWRITEBYTECODE=1, PLANAR_CANDIDATE = the recorded C01 candidate, sha256 `774bcd69809ead9e4cd7b6e06c74aa045ea9272a2321cd36f91bc9c76a99ab0c`, scratch HOME and PLANAR_DB), `perl -e 'alarm 1500; exec @ARGV' python3 -B -m unittest -v <named tests>` from `<scratch-root>/probe-warm/tests/native/r19-accept-support`. KILLED = a named test appears in a FAIL or ERROR header; a run without a failing test name is not a kill. Control: `raw/BASELINE.log` (unmutated; whole `acceptance_cli_test` module plus the two F11 fault tests: Ran 24, OK, exit 0).

## Scope of this re-probe

- Requested rows: F11 (alt b), L17, R1j, R1m, and the shell/record/guard group. Also re-run: every earlier mutant whose observer lives in `acceptance_cli_test.py`, the only test file the repair changed (identity: acceptance_cli_test.py `1b71cf8c...`, previously `9a4d5082...`; fault_driver.py, fault_live_test.py, run_acceptance.py, accept_lib.py unchanged). Rows with mutants observed only by other files are carried forward unchanged.
- Two new tests were added by the repair: `test_rerun_index_names_the_missing_top_level_key_itself` (R1j-a) and `test_index_lists_only_host_lines_and_refuses_out_dir_other_than_work_cwd` (R1m-a, R1m-b). The earlier F11-b/L17 negative fixture (`test_trace_is_derived_from_executed_steps_in_plain_cmake_run`) is unchanged.

## Identity (identity.py output, verbatim, run before the first mutant; src tracked hash equals the recorded value, G2)

| identity | value |
|---|---|
| HEAD | `91e0df24f2167471e49ec057c7159bce5dff3608` |
| sha256(git diff --binary HEAD -- src tests/native/r19-accept-support), tracked only | `83a16fdee033fce5a1a792d8d65f4090fba8dacffc2d38013684cacb4daeba19` |
| combined sha256 (diff bytes, then each untracked path NUL bytes, sorted), pathspec src tests/native/r19-accept-support, evidence excluded | `988e085f2272f80c8422dc99ed2098b0f8ab9eaa0d3c82f79988569e8907e1d9` |

| file | state | sha256 |
|---|---|---|
| `src/engine/ledger/execution.t.cpp` | modified | `250867b8bd74b86d718cbe17e5fa9983fda6a60a2b5e7c127829a3fec6f98950` |
| `src/engine/ledger/storage.cpp` | modified | `e4cf41dcb8cf44cc8004d4ba011543a557c742c623e20ede631eb975f237d822` |
| `tests/native/r19-accept-support/accept_lib.py` | untracked | `6ef4e069c2da203d508f3695482b5ea9883b71fc47ee8519a6f910f99e223e68` |
| `tests/native/r19-accept-support/accept_test.py` | untracked | `c6ae7b28851063836cc57ae1a77640976caecfa8c5c741672b81e6a797a93d8e` |
| `tests/native/r19-accept-support/acceptance_cli_test.py` | untracked | `1b71cf8cf9be5d2ba217120da0cab4b66fd1654a1919247643f9fb63cf58978a` |
| `tests/native/r19-accept-support/check_runner.py` | untracked | `b71e939a226415c0aeb43083dffcf263618d2c88804500300e585bfafec1d5f1` |
| `tests/native/r19-accept-support/cmake_check.py` | untracked | `ba8f20625bda0f3c8fc3c613529e16f6a1f8e0a38b0b261f8341b2fdcbb3f360` |
| `tests/native/r19-accept-support/fault/cmake_rerun/run_heavy.py` | untracked | `316fa2abd3d86614a79f952d2eb4819c71c836d266e2124916b1bbb639e78b47` |
| `tests/native/r19-accept-support/fault/delayed_marker_check.py` | untracked | `6ca4c0b4283083a8241a5edbe71c1912d0d8fa908ac6d602de29f453ed728141` |
| `tests/native/r19-accept-support/fault/mini/base/src/unit.txt` | untracked | `d73d5ccc52b8747e2bc253b7267b6110d41021eacd54ff33695d96c0a47fe56a` |
| `tests/native/r19-accept-support/fault/mini/jobs/combined-max1.lua` | untracked | `c7d8b3a33fd7940b6ef56e4ac4923836f527644d8b6952aaa8ae5d22f54e8e39` |
| `tests/native/r19-accept-support/fault/mini/jobs/combined-max2.lua` | untracked | `6d0df6cf79f148664587dcf43e774243cad211e4d26420ce8fe4f955c1281e0e` |
| `tests/native/r19-accept-support/fault/mini/registry.json` | untracked | `eca5ac671b6929f042a839591ff56da72d0afef8f4e9aca5433df7b1119490ca` |
| `tests/native/r19-accept-support/fault/mini/source/src/unit.txt` | untracked | `753f8fe079abf1160eaa98cc40257a00da60a033b2d3730ab679fa0f62a0778c` |
| `tests/native/r19-accept-support/fault/mini/workspace/spec.lua` | untracked | `d35b07e34032202e830cedb66b4d45b5774621f087acc9f8012b1d8edacf07da` |
| `tests/native/r19-accept-support/fault/mini/workspace/workspace.lua` | untracked | `21ac5c6da0e2d27bb4911202d0a364acacc49d39b052fa71d8ca44b68e2d453d` |
| `tests/native/r19-accept-support/fault/sleep_check.py` | untracked | `751414911570978faf4f82d0ee79631ceb41c76dfd2bc4e66d35c41faa51ce57` |
| `tests/native/r19-accept-support/fault_driver.py` | untracked | `ced606f417f4a9098e362d3e49e65d383c08dcbd1e9b34826978f979f1f62197` |
| `tests/native/r19-accept-support/fault_live_test.py` | untracked | `d675691ff97c30a1bdfe3763533c8d85fd4228750aac3ecbc9ee4e334908b44b` |
| `tests/native/r19-accept-support/quota_driver.py` | untracked | `3c45a76571439a528e3cf1aee3803308964349da6edb247235cc656e8a4a3420` |
| `tests/native/r19-accept-support/quota_live_test.py` | untracked | `bedad9a306b98b535c33f2a27ea4224575bc6f2a81120019815d3d119117f861` |
| `tests/native/r19-accept-support/registry_gen.py` | untracked | `26752307e7257a99722b713342c790ae3eb146bd99c93ea410b6b4312109d172` |
| `tests/native/r19-accept-support/run_acceptance.py` | untracked | `3c04f00ecf48c127e83b9f5c0a8436c0673c3ca178810b808bec3839dec40c94` |

## Summary

- 22 mutant runs, one raw log each under `raw/`, plus `raw/BASELINE.log` (control, passes). 20 KILLED, 2 SURVIVOR.
- Survivors: **F11 (alt b) and L17** (one diff, `d5cab53c0ffc3c00`: hard-coded trace list in `finish()`). Unchanged from `superseded-3`: the repair did not address them; they remain survivors and still need a negative fixture (a run that completes through `finish()` with an executed verb sequence different from the 15-entry list, or a direct `finish()` unit call on a driver whose `self.trace` is custom; the plain cmake run used by the existing fixture returns through `stopped_result` and never reaches the mutated line).
- Resolved survivors: **R1j-a** (now killed by `test_rerun_index_names_the_missing_top_level_key_itself`; the existing test still passes under it), **R1m-a** and **R1m-b** (now killed by `test_index_lists_only_host_lines_and_refuses_out_dir_other_than_work_cwd`).

## Table A. Re-run rows (this run)

| row | probe | mutant | diff sha256 | outcome | killing test | note | run |
|---|---|---|---|---|---|---|---|
| F11 (alt b) | F11-b | hard-code the trace list in fault_driver.finish() | `d5cab53c0ffc3c0089448886cdc0e9d924595d32fe45c3a89c5d5fec8bdc700a` | SURVIVOR | none | existing `test_scenario_driver_is_config_parameterized` and new `test_trace_is_derived_from_executed_steps_in_plain_cmake_run` both pass (both named, no failure) | run (`raw/F11-b.log`) |
| L17 | L17 | same diff as F11-b | `d5cab53c0ffc3c0089448886cdc0e9d924595d32fe45c3a89c5d5fec8bdc700a` | SURVIVOR | none | same as F11-b; the repair did not touch fault_driver.py or fault_live_test.py | run (`raw/L17.log`) |
| R1j | R1j-a | drop the RERUN_REQUIRED_KEYS presence loop in validate_rerun_result | `c5c28b442e87f5145756e7cdd0c12fb881a1c12f35fa5cbccd49ed7dccbbe9bb` | KILLED | test_rerun_index_names_the_missing_top_level_key_itself (FAIL, 6 subtests) | new test only; the existing missing-field test passes under it (the earlier survivor is resolved by the new test) | run (`raw/R1j-a.log`) |
| R1j | R1j-b | skip the replay/ledger consistency predicate | `9fdcd6d08c57299e3355e9791cf7cd50a18af753613f613c746ff781140cf874` | KILLED | test_rerun_index_fails_when_any_required_field_is_missing (FAIL, 4 subtests) | existing test; new test passes under it | run (`raw/R1j-b.log`) |
| R1j (H3) | H3 | host-path guard skipped | `1a5caab58044ed71825ad226bec368af60d4374a10acf3e1017ecb2feb9ab3e6` | KILLED | test_rerun_index_still_refuses_host_path_outside_argv (FAIL) |  | run (`raw/H3.log`) |
| R1j (H1) | H1 | copy faults.recover.argv whole | `b0d1ceb8f55521a458e5319a600194b38262dc07179cc5fc68271beaaf403a22` | KILLED | test_rerun_index_normalizes_real_shaped_absolute_argv (FAIL) |  | run (`raw/H1.log`) |
| R1m | R1m-a | compact_index also appends the raw text | `e4c7a0c367e7e2d6d30f765d6759d3371af372b9f5e748d5407894a96a26d3de` | KILLED | test_index_lists_only_host_lines_and_refuses_out_dir_other_than_work_cwd (FAIL) | new test only; the earlier survivor is resolved | run (`raw/R1m-a.log`) |
| R1m | R1m-b | accept --out-dir different from --work-cwd | `960552956ffeb4d783e35e26c921c8af413c3152ae84d1eac4b31c4d802d0262` | KILLED | test_index_lists_only_host_lines_and_refuses_out_dir_other_than_work_cwd (FAIL) | new test only; the earlier survivor is resolved | run (`raw/R1m-b.log`) |
| R1m (H8) | H8 | NUL/control bytes not escaped | `f9b81c341a8f1cb1c1c5a4ab22bde18c1c74dbe699112e632d116e6ee6328421` | KILLED | test_index_escapes_nul_next_to_host_path_in_binary_raw_file (FAIL) |  | run (`raw/H8.log`) |
| R1m (H9) | H9 | dry run skips residual host-prefix scan | `d87ccc6b05977dc4e60cd07d1b7ac082dd992f4ec66ac2b17e212c553ef0229c` | KILLED | test_index_dry_run_refuses_unredactable_raw_file_and_writes_nothing (FAIL) |  | run (`raw/H9.log`) |
| R1m (H10) | H10 | dry run count wrong or writes | `a626705a56f38544d6295b5dad6c0e57f46a553b276c1a1aa9656532e54809c9` | KILLED | test_index_dry_run_reports_expected_count_and_writes_nothing (FAIL) |  | run (`raw/H10.log`) |
| R1m / dry run | R1w-a | dry-run branch skipped | `e6f19f763a5b06582fb1f870d7e4fe462595b7acc95c5196771767621b7b684d` | KILLED | both named dry-run tests (FAIL) |  | run (`raw/R1w-a.log`) |
| R1a, R1p | R1a-a | skip candidate check | `01de32f8412bed86a6bc230d216aeee4f9b8ad1e37d0f3528ef66f1e670f9880` | KILLED | test_live_commands_abort_before_first_action_on_candidate_mismatch (FAIL) |  | run (`raw/R1a-a.log`) |
| R1a, R1p | R1a-b | reuse non-fresh ledger root | `13fdf37a9e424ac9c7f83707ed1b4b3a7e2c16edee29c9bab383e4e46a72bccf` | KILLED | test_retained_dir_never_overwrites_existing_runs (FAIL) |  | run (`raw/R1a-b.log`) |
| R1k | R1k | record nonzero exit without closed ok | `1f56ea2c7242eb46e53b9d4dcf5e557de183cbf0c94a7325d178c7b9deadeb0f` | KILLED | test_limits_command_requires_exact_closed_diagnostic (FAIL) |  | run (`raw/R1k.log`) |
| R1k (H4) | H4 | limits rejects real error envelope | `ed0f3d5c79aaf9cd958fc6968234cd39911c1bc8b4700a8c538f5f5915a3d1f3` | KILLED | test_limits_command_accepts_real_error_envelope (FAIL) |  | run (`raw/H4.log`) |
| R1k (H2) | H2-a | capture-limit predicate accepts/rejects wrongly | `72653e61b34f1b4acc6c469c3e0fc35a927fc612e6bcdded7596381376cba5c9` | KILLED | test_capture_limit_accepts_exact_real_error_envelope_only and test_limits_command_accepts_real_error_envelope (FAIL) | accept_lib mutant observed through acceptance_cli_test too | run (`raw/H2-a.log`) |
| R1l | R1l | record packet lines not read from source | `51dd74e2ff4135528fc7a1eab0d1a8999a2dcd462f1fbc32bb83520699f6a73d` | KILLED | test_packet_command_records_literal_source_lines (FAIL) |  | run (`raw/R1l.log`) |
| R1n, R2a, R2e | R1n | skip the scan root walk | `7ca0217a634b3ef8069f081b5c70583ad4bdf65693a89e89a679c3ad40393aed` | KILLED | test_scan_command_fails_on_host_path_in_committed_evidence (FAIL) |  | run (`raw/R1n.log`) |
| R1o | R1o | skip the standard verify-redaction tool | `6b642db84b339a6759bc3f75ed7cc9acc8f90d25ac42120daf1d26c89952290e` | KILLED | test_verify_redaction_runs_standard_tool_and_rejects_scratch_tokens_in_manifest (FAIL) |  | run (`raw/R1o.log`) |
| A3 | A3-a | cmd_suites accepts skips/failures/zero matched | `ac395c1547770ac2b0c264c43e83a4a59b56795d7f6172f6b58670599c347474` | KILLED | test_suites_command_refuses_skips_failures_and_zero_matched (FAIL) |  | run (`raw/A3-a.log`) |
| A3 | A3-b | cmd_suites skips live-candidate gate | `79604703083dbaf0c9f00cdc4b8e231ea25e4bfa3f895a03c9ace5394a5a2da4` | KILLED | test_suites_command_live_modules_require_the_recorded_candidate (FAIL) |  | run (`raw/A3-b.log`) |

## Red-first table (tests added in this round lineage, repair additions first)

| test added | mutant that makes it fail | outcome | source |
|---|---|---|---|
| AcceptanceCliTest.test_rerun_index_names_the_missing_top_level_key_itself | `R1j-a` | KILLED | this run (raw/R1j-a.log) |
| AcceptanceCliTest.test_index_lists_only_host_lines_and_refuses_out_dir_other_than_work_cwd | `R1m-a`, `R1m-b` | KILLED | this run (raw/R1m-a.log, raw/R1m-b.log) |
| FaultLiveTest.test_trace_is_derived_from_executed_steps_in_plain_cmake_run | `F11-b`, `L17` | NOT KILLED (survivors) | this run (raw/F11-b.log, raw/L17.log) |
| AcceptanceCliTest.test_rerun_index_normalizes_real_shaped_absolute_argv (H1) | `H1` | KILLED | this run (raw/H1.log) |
| AcceptLibTest.test_capture_limit_accepts_exact_real_error_envelope_only (H2) | `H2-a` | KILLED | this run (raw/H2-a.log) |
| AcceptanceCliTest.test_limits_command_accepts_real_error_envelope (H2/H4) | `H4`, `H2-a` | KILLED | this run |
| AcceptanceCliTest.test_rerun_index_still_refuses_host_path_outside_argv (H3, guard) | `H3` | KILLED | this run |
| AcceptanceCliTest.test_index_escapes_nul_next_to_host_path_in_binary_raw_file (H8) | `H8` | KILLED | this run |
| AcceptanceCliTest.test_index_dry_run_refuses_unredactable_raw_file_and_writes_nothing (H9) | `H9`, `R1w-a` | KILLED | this run |
| AcceptanceCliTest.test_index_dry_run_reports_expected_count_and_writes_nothing (H10) | `H10`, `R1w-a` | KILLED | this run |
| H5, H6, H7 tests and the other fix-wave negative fixtures (F20-nf, F21-a-nf, F22-a-nf, L15-nf, Q2-a-nf, Q7-nf, Q11-a-nf, Q12-b-nf, Q13-b-nf, Q14-b-nf, Q15-nf, L6-nf, L7-a-nf) | unchanged rows | KILLED | carried from `superseded-3/PROBES.md` (raw in `superseded-3/raw/`) |

## Shell, record and guard rows

- R0, R1s, R1q, R1r, R1u (argv equality), R1x, R1y, R2a-R2i, A1, A2, A5, G1-G3: NOT PROBED by mutation (shell, record or guard rows with no automatable test, or the argv is operator-supplied so there is no code to mutate; carried from `superseded-3/PROBES.md` Skipped). Checked here: G2, the tracked-src hash in the identity block equals `83a16fdee033fce5a1a792d8d65f4090fba8dacffc2d38013684cacb4daeba19`; G1, no test removed or renamed (the baseline run and the 22 mutant runs matched every named test id, none missing). The remaining shell checks belong to the gate and record waves.
- R1u observer `AcceptanceCliTest.test_limits_command_requires_exact_closed_diagnostic` is covered by `R1k` above; R1k/H4/H2-a all KILLED.

## Skipped

- No exploratory mutants beyond the table rows were added in this re-probe (0 of 20 used).
- C++ (`execution.t.cpp`, `storage.cpp`): unchanged, no C02 row, no mutant, no ctest gate claimed.

## Carried forward from superseded-3 (rows not re-run; the Table A rows above supersede any row with the same probe id)

## Table 1. Run rows, H rows and acceptance rows (invariant to mutant to killing test)

| invariant row | mutant ids | diff sha256 (first 16) | outcome | killing test(s) | run or carried-from |
|---|---|---|---|---|---|
| R1a | R1a-a | `01de32f8412bed86` | KILLED | test_live_commands_abort_before_first_action_on_candidate_mismatch (FAIL) | run (`raw/R1a-a.log`) |
| R1a | R1a-b | `13fdf37a9e424ac9` | KILLED | test_retained_dir_never_overwrites_existing_runs (FAIL) | run (`raw/R1a-b.log`) |
| R1b | F1-a | `5b5d84c879dafdb5` | KILLED | test_interrupted_check_is_recovered_by_check_recover (ERROR) | run (`raw/F1-a.log`) |
| R1b | F1-b | `64485a30a2649a1d` | KILLED | test_interrupted_check_is_recovered_by_check_recover (ERROR) | run (`raw/F1-b.log`) |
| R1b | F1-c | `520e7dd29a9c7b0f` | KILLED | test_interrupted_check_is_recovered_by_check_recover (ERROR) | run (`raw/F1-c.log`) |
| R1b | L8 | `6a4746f7397340ea` | KILLED | test_interrupted_check_is_recovered_by_check_recover (ERROR) | run (`raw/L8.log`) |
| R1c | F2 | `7fdd88fb404dacb1` | KILLED | test_state_survives_process_restart_between_phases (FAIL) | run (`raw/F2.log`) |
| R1c | L9 | `dbd68afccf3379b8` | KILLED | test_state_survives_process_restart_between_phases (FAIL) | run (`raw/L9.log`) |
| R1d | F3 | `750c76cc2ebe15c1` | KILLED | test_takeover_fences_old_run_token (ERROR) | run (`raw/F3.log`) |
| R1d | L10 | `270c2e33b822ce5f` | KILLED | test_takeover_fences_old_run_token (ERROR) | run (`raw/L10.log`) |
| R1e | F4 | `4d3ea2fe07a27287` | KILLED | test_rejecting_verdict_admits_repair_and_worker_returns (ERROR) | run (`raw/F4.log`) |
| R1e | F21-a-nf | `ef3a55b0297e84cf` | KILLED | test_run_heavy_config_names_the_composed_base (FAIL) | run (`raw/F21-a-nf.log`) |
| R1e | F21-b | `f7ef9e017573b90c` | KILLED | test_run_heavy_config_is_composed_base_and_owned_repairs (FAIL) | run (`raw/F21-b.log`) |
| R1e | F21-c | `32782d1754db231a` | KILLED | test_run_heavy_config_is_composed_base_and_owned_repairs (FAIL) | run (`raw/F21-c.log`) |
| R1f | F5 | `bb1607fc0ef73c38` | KILLED | test_second_rejection_is_capped_and_opens_max_rounds_blocker (ERROR) | run (`raw/F5.log`) |
| R1f | F6 | `a955e1a4e91c23f6` | KILLED | test_load_raising_max_rounds_clears_blocker (ERROR) | run (`raw/F6.log`) |
| R1f | L12 | `914b94cf9c0d2257` | KILLED | test_load_raising_max_rounds_clears_blocker (ERROR), test_second_rejection_is_capped_and_opens_max_rounds_blocker (ERROR) | run (`raw/L12.log`) |
| R1g | F7 | `a7b601933d18419b` | KILLED | test_final_approval_after_faults_is_exact_target (ERROR) | run (`raw/F7.log`) |
| R1g | L13 | `c97bdc0a8691203c` | KILLED | test_final_approval_after_faults_is_exact_target (ERROR) | run (`raw/L13.log`) |
| R1h | F8 | `cd3b8020aedde660` | KILLED | test_replays_and_inspections_do_not_reexecute_checks_or_write (ERROR) | run (`raw/F8.log`) |
| R1h | F16 | `7b4af53812cd8b94` | KILLED | test_run_scenario_aborts_on_tampered_replay (FAIL) | run (`raw/F16.log`) |
| R1h | F17 | `83f54f895abfd082` | KILLED | test_run_scenario_aborts_when_counter_increases_on_replay (FAIL) | run (`raw/F17.log`) |
| R1i | F15 | `b71afce5f5b43b8b` | KILLED | test_run_scenario_aborts_when_inspection_changes_ledger (FAIL) | run (`raw/F15.log`) |
| R1j | R1j-a | `c5c28b442e87f514` | SURVIVOR | none | run (`raw/R1j-a.log`) |
| R1j | R1j-b | `9fdcd6d08c57299e` | KILLED | test_rerun_index_fails_when_any_required_field_is_missing (FAIL) | run (`raw/R1j-b.log`) |
| R1j | H3 | `1a5caab58044ed71` | KILLED | test_rerun_index_still_refuses_host_path_outside_argv (FAIL) | run (`raw/H3.log`) |
| R1k | R1k | `1f56ea2c7242eb46` | KILLED | test_limits_command_requires_exact_closed_diagnostic (FAIL) | run (`raw/R1k.log`) |
| R1k | H4 | `ed0f3d5c79aaf9cd` | KILLED | test_limits_command_accepts_real_error_envelope (FAIL) | run (`raw/H4.log`) |
| R1l | R1l | `51dd74e2ff413552` | KILLED | test_packet_command_records_literal_source_lines (FAIL) | run (`raw/R1l.log`) |
| R1m | R1m-a | `e4c7a0c367e7e2d6` | SURVIVOR | none | run (`raw/R1m-a.log`) |
| R1m | R1m-b | `960552956ffeb4d7` | SURVIVOR | none | run (`raw/R1m-b.log`) |
| R1m | H8 | `f9b81c341a8f1cb1` | KILLED | test_index_escapes_nul_next_to_host_path_in_binary_raw_file (FAIL) | run (`raw/H8.log`) |
| R1n | R1n | `7ca0217a634b3ef8` | KILLED | test_scan_command_fails_on_host_path_in_committed_evidence (FAIL) | run (`raw/R1n.log`) |
| R1o | R1o | `6b642db84b339a67` | KILLED | test_verify_redaction_runs_standard_tool_and_rejects_scratch_tokens_in_manifest (FAIL) | run (`raw/R1o.log`) |
| R1p | R1a-a | `01de32f8412bed86` | KILLED | test_live_commands_abort_before_first_action_on_candidate_mismatch (FAIL) | run (`raw/R1a-a.log`) |
| R1p | R1a-b | `13fdf37a9e424ac9` | KILLED | test_retained_dir_never_overwrites_existing_runs (FAIL) | run (`raw/R1a-b.log`) |
| R1p | R1p-b | `9be686982092c70e` | KILLED | test_recorded_candidate_verification_rejects_mismatch (FAIL) | run (`raw/R1p-b.log`) |
| R1p | F24 | `eefdef2e4dd761c7` | KILLED | test_run_heavy_aborts_on_unrecorded_candidate (ERROR) | run (`raw/F24.log`) |
| R1u | R1k | `1f56ea2c7242eb46` | KILLED | test_limits_command_requires_exact_closed_diagnostic (FAIL) | run (`raw/R1k.log`) |
| R1v | H5 | `1e56986c2687e9c5` | KILLED | test_run_heavy_config_wait_timeouts_cover_registry_budget (FAIL) | run (`raw/H5.log`) |
| R1v | H6 | `e6a1fe33420d5de3` | KILLED | test_marker_wait_honors_configured_timeout (FAIL) | run (`raw/H6.log`) |
| R1v | H7-a | `d30d1152de164e66` | KILLED | test_check_completion_timeout_aborts_with_accept_error_and_kills_cli (FAIL) | run (`raw/H7-a.log`) |
| R1v | H7-b | `f3468c45eb596aa0` | KILLED | test_check_completion_timeout_aborts_with_accept_error_and_kills_cli (FAIL) | run (`raw/H7-b.log`) |
| R1w | R1w-a | `e6f19f763a5b0658` | KILLED | test_index_dry_run_refuses_unredactable_raw_file_and_writes_nothing (FAIL), test_index_dry_run_reports_expected_count_and_writes_nothing (FAIL) | run (`raw/R1w-a.log`) |
| R1w | H9 | `d87ccc6b05977dc4` | KILLED | test_index_dry_run_refuses_unredactable_raw_file_and_writes_nothing (FAIL) | run (`raw/H9.log`) |
| R1w | H10 | `a626705a56f38544` | KILLED | test_index_dry_run_reports_expected_count_and_writes_nothing (FAIL) | run (`raw/H10.log`) |
| R2a | R1n | `7ca0217a634b3ef8` | KILLED | test_scan_command_fails_on_host_path_in_committed_evidence (FAIL) | run (`raw/R1n.log`) |
| R2e | R1n | `7ca0217a634b3ef8` | KILLED | test_scan_command_fails_on_host_path_in_committed_evidence (FAIL) | run (`raw/R1n.log`) |
| A3 | A3-a | `ac395c1547770ac2` | KILLED | test_suites_command_refuses_skips_failures_and_zero_matched (FAIL) | run (`raw/A3-a.log`) |
| A3 | A3-b | `79604703083dbaf0` | KILLED | test_suites_command_live_modules_require_the_recorded_candidate (FAIL) | run (`raw/A3-b.log`) |
| A4 | A4 | `68ac34f647a3788b` | KILLED | test_child_env_sets_bytecode_flag_even_when_the_parent_lacks_it (ERROR) | run (`raw/A4.log`) |
| G4 | R1k | `1f56ea2c7242eb46` | KILLED | test_limits_command_requires_exact_closed_diagnostic (FAIL) | run (`raw/R1k.log`) |
| H1 | H1 | `b0d1ceb8f55521a4` | KILLED | test_rerun_index_normalizes_real_shaped_absolute_argv (FAIL) | run (`raw/H1.log`) |
| H2 | H2-a | `72653e61b34f1b4a` | KILLED | test_capture_limit_accepts_exact_real_error_envelope_only (FAIL), test_limits_command_accepts_real_error_envelope (FAIL) | run (`raw/H2-a.log`) |
| H2 | H2-b | `0b7d736bd86457a7` | KILLED | test_capture_limit_accepts_exact_real_error_envelope_only (FAIL) | run (`raw/H2-b.log`) |
| H3 | H3 | `1a5caab58044ed71` | KILLED | test_rerun_index_still_refuses_host_path_outside_argv (FAIL) | run (`raw/H3.log`) |
| H4 | H4 | `ed0f3d5c79aaf9cd` | KILLED | test_limits_command_accepts_real_error_envelope (FAIL) | run (`raw/H4.log`) |
| H5 | H5 | `1e56986c2687e9c5` | KILLED | test_run_heavy_config_wait_timeouts_cover_registry_budget (FAIL) | run (`raw/H5.log`) |
| H6 | H6 | `e6a1fe33420d5de3` | KILLED | test_marker_wait_honors_configured_timeout (FAIL) | run (`raw/H6.log`) |
| H7 | H7-a | `d30d1152de164e66` | KILLED | test_check_completion_timeout_aborts_with_accept_error_and_kills_cli (FAIL) | run (`raw/H7-a.log`) |
| H7 | H7-b | `f3468c45eb596aa0` | KILLED | test_check_completion_timeout_aborts_with_accept_error_and_kills_cli (FAIL) | run (`raw/H7-b.log`) |
| H8 | H8 | `f9b81c341a8f1cb1` | KILLED | test_index_escapes_nul_next_to_host_path_in_binary_raw_file (FAIL) | run (`raw/H8.log`) |
| H9 | H9 | `d87ccc6b05977dc4` | KILLED | test_index_dry_run_refuses_unredactable_raw_file_and_writes_nothing (FAIL) | run (`raw/H9.log`) |
| H10 | H10 | `a626705a56f38544` | KILLED | test_index_dry_run_reports_expected_count_and_writes_nothing (FAIL) | run (`raw/H10.log`) |
| R0 | n/a | - | NOT PROBED | - | shell checks (before-lists, free space, identity before-output, candidate sha): no automatable test; verified by the recorded gate files, not mutated |
| R1s | n/a | - | NOT PROBED | - | run-level ordering (mini rerun-index exit before the heavy run): shell evidence, no unit test; not mutated |
| R1q | n/a | - | NOT PROBED | - | stop rule; verified by the src identity hash below (`83a16fde...` tracked-diff value unchanged before and after this probe) |
| R1r | n/a | - | NOT PROBED | - | disk and retention (COPY-VERIFY, df poll): shell evidence, no TEST_CASE; not mutated |
| R1u | n/a | - | NOT PROBED | - | the argv equality with the recorded R19-CMAKE argv is operator-run shell evidence; the code observer (closed result) is probed as R1k |
| R1x, R1y | n/a | - | NOT PROBED | - | shell evidence (export retention, canonical redaction); no automatable test; not mutated |
| R2a-R2i | n/a | - | NOT PROBED | - | list, redaction and record rows: shell/script verifiers; the code observer `cmd_scan` is probed as R1n; the allowlist/line-number variants are not code in the harness |
| A1, A2, A5, G1-G3 | n/a | - | NOT PROBED | - | record, before/after list and guard rows: shell checks; not mutated |

## Table 2. Probe rows F, L, Q (wave-6 mutant set, full re-run)

For every alternative the row is KILLED only if all alternatives are killed by the named test; the final column gives the row verdict. A row whose original test survived is judged on its `-nf` negative fixture.

| row | mutant id | diff sha256 (first 16) | original test outcome | negative fixture outcome | killing test(s) of the judged run | verdict basis |
|---|---|---|---|---|---|---|
| F1 | F1-a | `5b5d84c879dafdb5` | KILLED | - | test_interrupted_check_is_recovered_by_check_recover (ERROR) | run |
| F1 | F1-b | `64485a30a2649a1d` | KILLED | - | test_interrupted_check_is_recovered_by_check_recover (ERROR) | run |
| F1 | F1-c | `520e7dd29a9c7b0f` | KILLED | - | test_interrupted_check_is_recovered_by_check_recover (ERROR) | run |
| F2 | F2 | `7fdd88fb404dacb1` | KILLED | - | test_state_survives_process_restart_between_phases (FAIL) | run |
| F3 | F3 | `750c76cc2ebe15c1` | KILLED | - | test_takeover_fences_old_run_token (ERROR) | run |
| F4 | F4 | `4d3ea2fe07a27287` | KILLED | - | test_rejecting_verdict_admits_repair_and_worker_returns (ERROR) | run |
| F5 | F5 | `bb1607fc0ef73c38` | KILLED | - | test_second_rejection_is_capped_and_opens_max_rounds_blocker (ERROR) | run |
| F6 | F6 | `a955e1a4e91c23f6` | KILLED | - | test_load_raising_max_rounds_clears_blocker (ERROR) | run |
| F7 | F7 | `a7b601933d18419b` | KILLED | - | test_final_approval_after_faults_is_exact_target (ERROR) | run |
| F8 | F8 | `cd3b8020aedde660` | KILLED | - | test_replays_and_inspections_do_not_reexecute_checks_or_write (ERROR) | run |
| F9 | F9 | `587dd5c7df75b690` | KILLED | - | test_history_references_through_faults (FAIL) | run |
| F10 | F10 | `350ae359f635440a` | KILLED | - | test_failed_and_interrupted_attempts_remain_retained (ERROR) | run |
| F11 | F11-a | `19166ebeea448c35` | KILLED | - | test_scenario_driver_is_config_parameterized (FAIL) | run |
| F11 | F11-b | `d5cab53c0ffc3c00` | SURVIVOR | SURVIVOR | none | negative fixture (`F11-b-nf`) |
| F12 | F12-a | `b95280d9e000123e` | KILLED | - | test_orchestrator_guidance_usable_through_fault_phases (ERROR) | run |
| F12 | F12-b | `1a20b7e197b5b334` | KILLED | - | test_orchestrator_guidance_usable_through_fault_phases (ERROR) | run |
| F12 | F12-c | `c0eb0b2881b45ed3` | KILLED | - | test_orchestrator_guidance_usable_through_fault_phases (ERROR) | run |
| F13 | F13 | `08b8ec94cfac1dda` | KILLED | - | test_first_completed_check_passes_boundary_rows (FAIL) | run |
| F14 | F14 | `2edd973f2b65c479` | KILLED | - | test_run_scenario_aborts_when_trajectory_omits_rejection (FAIL) | run |
| F15 | F15 | `b71afce5f5b43b8b` | KILLED | - | test_run_scenario_aborts_when_inspection_changes_ledger (FAIL) | run |
| F16 | F16 | `7b4af53812cd8b94` | KILLED | - | test_run_scenario_aborts_on_tampered_replay (FAIL) | run |
| F17 | F17 | `83f54f895abfd082` | KILLED | - | test_run_scenario_aborts_when_counter_increases_on_replay (FAIL) | run |
| F18 | F18 | `da66f6f0d28920fd` | KILLED | - | test_run_scenario_aborts_on_mismatched_event_expansion (FAIL) | run |
| F19 | F19 | `4d00a018996d1ead` | KILLED | - | test_run_scenario_aborts_on_mismatched_at_event_state (FAIL) | run |
| F20 | F20 | `99da3c048ad1ce97` | SURVIVOR | KILLED | test_blank_guidance_is_refused_by_the_usability_predicate (FAIL) | negative fixture (`F20-nf`) |
| F21 | F21-a | `ef3a55b0297e84cf` | SURVIVOR | KILLED | test_run_heavy_config_names_the_composed_base (FAIL) | negative fixture (`F21-a-nf`) |
| F21 | F21-b | `f7ef9e017573b90c` | KILLED | - | test_run_heavy_config_is_composed_base_and_owned_repairs (FAIL) | run |
| F21 | F21-c | `32782d1754db231a` | KILLED | - | test_run_heavy_config_is_composed_base_and_owned_repairs (FAIL) | run |
| F22 | F22-a | `23ee6421bbbdd707` | SURVIVOR | KILLED | test_kill_check_signals_the_marker_pgid_when_it_differs_from_the_marker_pid (FAIL) | negative fixture (`F22-a-nf`) |
| F22 | F22-b | `5fbe05a734556b91` | KILLED | - | test_kill_check_consumes_marker_written_by_cmake_check (ERROR) | run |
| F22 | F22-c | `b2611af9f2b2331e` | KILLED | - | test_kill_check_consumes_marker_written_by_cmake_check (ERROR) | run |
| F23 | F23-a | `a7fcbdbb66da4a0e` | KILLED | - | test_fault_history_inspection_is_independent_of_scratch_files (ERROR) | run |
| F23 | F23-b | `971ae61d0479bfc7` | KILLED | - | test_fault_history_inspection_is_independent_of_scratch_files (ERROR) | run |
| F24 | F24 | `eefdef2e4dd761c7` | KILLED | - | test_run_heavy_aborts_on_unrecorded_candidate (ERROR) | run |
| F25 | F25 | `10a46a53164393cd` | KILLED | - | test_run_scenario_aborts_before_action_not_listed_as_legal (FAIL) | run |
| H1 | H1 | `b0d1ceb8f55521a4` | KILLED | - | test_rerun_index_normalizes_real_shaped_absolute_argv (FAIL) | run |
| H2 | H2-a | `72653e61b34f1b4a` | KILLED | - | test_capture_limit_accepts_exact_real_error_envelope_only (FAIL), test_limits_command_accepts_real_error_envelope (FAIL) | run |
| H2 | H2-b | `0b7d736bd86457a7` | KILLED | - | test_capture_limit_accepts_exact_real_error_envelope_only (FAIL) | run |
| H3 | H3 | `1a5caab58044ed71` | KILLED | - | test_rerun_index_still_refuses_host_path_outside_argv (FAIL) | run |
| H4 | H4 | `ed0f3d5c79aaf9cd` | KILLED | - | test_limits_command_accepts_real_error_envelope (FAIL) | run |
| H5 | H5 | `1e56986c2687e9c5` | KILLED | - | test_run_heavy_config_wait_timeouts_cover_registry_budget (FAIL) | run |
| H6 | H6 | `e6a1fe33420d5de3` | KILLED | - | test_marker_wait_honors_configured_timeout (FAIL) | run |
| H7 | H7-a | `d30d1152de164e66` | KILLED | - | test_check_completion_timeout_aborts_with_accept_error_and_kills_cli (FAIL) | run |
| H7 | H7-b | `f3468c45eb596aa0` | KILLED | - | test_check_completion_timeout_aborts_with_accept_error_and_kills_cli (FAIL) | run |
| H8 | H8 | `f9b81c341a8f1cb1` | KILLED | - | test_index_escapes_nul_next_to_host_path_in_binary_raw_file (FAIL) | run |
| H9 | H9 | `d87ccc6b05977dc4` | KILLED | - | test_index_dry_run_refuses_unredactable_raw_file_and_writes_nothing (FAIL) | run |
| H10 | H10 | `a626705a56f38544` | KILLED | - | test_index_dry_run_reports_expected_count_and_writes_nothing (FAIL) | run |
| L2 | L2 | `ab96826712aaaed2` | KILLED | - | test_split_grouping_orders_dependency_and_completes (FAIL) | run |
| L6 | L6 | `8bec53585e84290d` | SURVIVOR | KILLED | test_regroup_fenced_probe_presents_the_retired_builder_token (FAIL) | negative fixture (`L6-nf`) |
| L7 | L7-a | `d1bcb0cf2e2da824` | SURVIVOR | KILLED | test_guidance_predicate_refuses_text_without_job_role_line_or_verb (FAIL) | negative fixture (`L7-a-nf`) |
| L7 | L7-b | `52e1108bacbd8669` | KILLED | - | test_orchestrator_guidance_usable_through_fault_phases (ERROR), test_orchestrator_guidance_is_usable_at_every_phase (ERROR) | run |
| L8 | L8 | `6a4746f7397340ea` | KILLED | - | test_interrupted_check_is_recovered_by_check_recover (ERROR) | run |
| L9 | L9 | `dbd68afccf3379b8` | KILLED | - | test_state_survives_process_restart_between_phases (FAIL) | run |
| L10 | L10 | `270c2e33b822ce5f` | KILLED | - | test_takeover_fences_old_run_token (ERROR) | run |
| L12 | L12 | `914b94cf9c0d2257` | KILLED | - | test_load_raising_max_rounds_clears_blocker (ERROR), test_second_rejection_is_capped_and_opens_max_rounds_blocker (ERROR) | run |
| L13 | L13 | `c97bdc0a8691203c` | KILLED | - | test_final_approval_after_faults_is_exact_target (ERROR) | run |
| L14 | L14 | `a3fc3fe8c56d195a` | KILLED | - | test_history_references_through_faults (ERROR) | run |
| L15 | L15 | `17c63b81a7cad503` | SURVIVOR | KILLED | test_every_check_attempt_folder_keeps_its_command_logs (FAIL) | negative fixture (`L15-nf`) |
| L16 | L16 | `ce1c32259025ec6a` | KILLED | - | test_first_completed_check_passes_boundary_rows (FAIL) | run |
| L17 | L17 | `d5cab53c0ffc3c00` | SURVIVOR | SURVIVOR | none | negative fixture (`L17-nf`) |
| L19 | Q14-a | `d956d15c2dc1b6a5` | KILLED | - | test_run_grouping_aborts_on_tampered_replay (FAIL) | run |
| L19 | Q15 | `7a7b0c7d63f3149a` | SURVIVOR | KILLED | test_unusable_guidance_is_refused_by_the_usability_predicate (FAIL) | negative fixture (`Q15-nf`) |
| L19 | Q19 | `6e8dc34796e69622` | KILLED | - | test_run_grouping_aborts_before_action_not_listed_as_legal (FAIL) | run |
| Q1 | Q1 | `e6d0a1d039489677` | KILLED | - | test_combined_and_split_share_spec_bytes_and_load_accepted (FAIL) | run |
| Q2 | Q2-a | `9af994f964a61ca8` | SURVIVOR | KILLED | test_combined_builder_root_carries_the_current_overlay_not_the_base_copy (FAIL) | negative fixture (`Q2-a-nf`) |
| Q2 | Q2-b | `4988470d96d866c4` | KILLED | - | test_combined_grouping_completes_with_exact_target_approval (ERROR) | run |
| Q3 | Q3-a | `6dc2e725af0749b9` | KILLED | - | test_split_grouping_orders_dependency_and_completes (ERROR) | run |
| Q3 | Q3-b | `de62fb79eaac89eb` | KILLED | - | test_split_grouping_orders_dependency_and_completes (ERROR) | run |
| Q4 | Q4 | `61653076bd2f9c1b` | KILLED | - | test_every_action_retains_raw_argv_stdout_stderr_exit (FAIL) | run |
| Q5 | Q5 | `f7bc2d9c3752bd7e` | KILLED | - | test_restart_replay_is_byte_identical_without_reexecution (ERROR) | run |
| Q6 | Q6 | `20cd1b4c7539e77d` | KILLED | - | test_wrong_target_verdict_is_refused_and_ledger_unchanged (ERROR) | run |
| Q7 | Q7 | `d2fe2b0f291a7857` | SURVIVOR | KILLED | test_every_history_event_has_a_replayed_state_equal_to_its_recorded_state (FAIL) | negative fixture (`Q7-nf`) |
| Q7-ii | Q7-ii | `b4d968212ae60b56` | KILLED | - | test_history_event_and_at_event_reproduce_accepted_records (FAIL) | run |
| Q8 | Q8 | `0361c78e83a1b549` | KILLED | - | test_inspection_reads_do_not_write_ledger (ERROR) | run |
| Q9 | Q9-a | `370edc96fad5ab0e` | KILLED | - | test_orchestrator_guidance_is_usable_at_every_phase (ERROR) | run |
| Q9 | Q9-b | `2ee3955812a3811a` | KILLED | - | test_orchestrator_guidance_is_usable_at_every_phase (ERROR) | run |
| Q9 | Q9-c | `948ba012e652eb2c` | KILLED | - | test_orchestrator_guidance_is_usable_at_every_phase (ERROR) | run |
| Q10 | Q10-a | `086c81a1d2cb3dc7` | KILLED | - | test_regroup_combined_to_split_fences_old_tokens (ERROR) | run |
| Q10 | Q10-b | `5f1e1b945e60cd79` | KILLED | - | test_regroup_combined_to_split_fences_old_tokens (FAIL) | run |
| Q11 | Q11-a | `612ca5810fde545d` | SURVIVOR | KILLED | test_session_argv_names_scratch_paths_and_never_the_checkout_fixture (FAIL) | negative fixture (`Q11-a-nf`) |
| Q11 | Q11-b | `ccd668204a80550d` | KILLED | - | test_fixture_copied_unmodified_and_counter_counts_checks (FAIL) | run |
| Q11 | Q11-c | `ed7281186d6ec7bc` | KILLED | - | test_fixture_copied_unmodified_and_counter_counts_checks (FAIL) | run |
| Q12 | Q12-a | `a4c60d2db44a98c6` | KILLED | - | test_contract_delivered_to_base_and_every_root (ERROR) | run |
| Q12 | Q12-b | `7d882c15f9909425` | SURVIVOR | KILLED | test_root_view_places_the_contract_even_when_its_source_lacks_it (FAIL) | negative fixture (`Q12-b-nf`) |
| Q13 | Q13-a | `c6a441a8bf4b1568` | KILLED | - | test_quota_check_results_match_actual_unittest_counts (FAIL) | run |
| Q13 | Q13-b | `423be5f8adda77d0` | SURVIVOR | KILLED | test_check_runner_passed_excludes_failures_and_skips (FAIL) | negative fixture (`Q13-b-nf`) |
| Q13 | Q13-b-inj | `822286dfc583e533` | INVALID (control red) | - | test_quota_check_results_match_actual_unittest_counts (ERROR) | run |
| Q14 | Q14-a | `d956d15c2dc1b6a5` | KILLED | - | test_run_grouping_aborts_on_tampered_replay (FAIL) | run |
| Q14 | Q14-b | `254be9fcd1494aa6` | SURVIVOR | KILLED | test_counter_only_replay_tamper_leaves_an_aborted_record_naming_the_replay (FAIL) | negative fixture (`Q14-b-nf`) |
| Q15 | Q15 | `7a7b0c7d63f3149a` | SURVIVOR | KILLED | test_unusable_guidance_is_refused_by_the_usability_predicate (FAIL) | negative fixture (`Q15-nf`) |
| Q16 | Q16 | `e3450d3d75a17f69` | KILLED | - | test_run_grouping_aborts_when_inspection_changes_ledger (FAIL) | run |
| Q17 | Q17-a | `052abd2850edd8ca` | KILLED | - | test_run_grouping_aborts_on_mismatched_history_reference (FAIL) | run |
| Q17 | Q17-b | `71be683a6aadd89d` | KILLED | - | test_run_grouping_aborts_on_mismatched_history_reference (FAIL) | run |
| Q18 | Q18-a | `e5aedf26821937a5` | KILLED | - | test_history_inspection_is_independent_of_scratch_workspace_and_registry (FAIL) | run |
| Q18 | Q18-b | `57b49cd9e431aa15` | KILLED | - | test_history_inspection_is_independent_of_scratch_workspace_and_registry (ERROR) | run |
| Q19 | Q19 | `6e8dc34796e69622` | KILLED | - | test_run_grouping_aborts_before_action_not_listed_as_legal (FAIL) | run |

### Row verdicts (F, L, Q)

| row | verdict |
|---|---|
| F1 | KILLED |
| F2 | KILLED |
| F3 | KILLED |
| F4 | KILLED |
| F5 | KILLED |
| F6 | KILLED |
| F7 | KILLED |
| F8 | KILLED |
| F9 | KILLED |
| F10 | KILLED |
| F11 | SURVIVOR/INVALID present: KILLED, SURVIVOR |
| F12 | KILLED |
| F13 | KILLED |
| F14 | KILLED |
| F15 | KILLED |
| F16 | KILLED |
| F17 | KILLED |
| F18 | KILLED |
| F19 | KILLED |
| F20 | KILLED |
| F21 | KILLED |
| F22 | KILLED |
| F23 | KILLED |
| F24 | KILLED |
| F25 | KILLED |
| H1 | KILLED |
| H2 | KILLED |
| H3 | KILLED |
| H4 | KILLED |
| H5 | KILLED |
| H6 | KILLED |
| H7 | KILLED |
| H8 | KILLED |
| H9 | KILLED |
| H10 | KILLED |
| L2 | KILLED |
| L6 | KILLED |
| L7 | KILLED |
| L8 | KILLED |
| L9 | KILLED |
| L10 | KILLED |
| L12 | KILLED |
| L13 | KILLED |
| L14 | KILLED |
| L15 | KILLED |
| L16 | KILLED |
| L17 | SURVIVOR/INVALID present: SURVIVOR |
| L19 | KILLED |
| Q1 | KILLED |
| Q2 | KILLED |
| Q3 | KILLED |
| Q4 | KILLED |
| Q5 | KILLED |
| Q6 | KILLED |
| Q7 | KILLED |
| Q7-ii | KILLED |
| Q8 | KILLED |
| Q9 | KILLED |
| Q10 | KILLED |
| Q11 | KILLED |
| Q12 | KILLED |
| Q13 | KILLED |
| Q14 | KILLED |
| Q15 | KILLED |
| Q16 | KILLED |
| Q17 | KILLED |
| Q18 | KILLED |
| Q19 | KILLED |


## Scratch artefacts

`<scratch-root>/probe-warm` kept (never deleted); per-mutant backups `rp2-bak-<id>` and diffs `rp2-diff-<id>` and the runner `c02-probe-reprobe2.py` and `rp2-summary.txt` live in the scratch directory, outside the checkout. No stray processes remained. The checkout was written only under `tests/native/r19-accept-a-c02-evidence/probes/`.
