# R19-ACCEPT-A-C02 PKG-HARNESS red-first record (wave 2)

Scope: support files under `tests/native/r19-accept-support` only; `src/` unchanged.
Pre-fix snapshot: `harness/pre-fix-identity.txt` (identity tool, pathspec `tests/native/r19-accept-support`, taken before any edit of the support tree). Combined sha256 of the pre-fix snapshot: `2d8085186ebff43d125415fec54e7c1282773aed0b8063b05cc5afaa00cc01cb`.
Identity command, run from the repository root: `python3 <workflows-scripts>/identity.py tests/native/r19-accept-support`.
Candidate for the live tests: the recorded C01 candidate `planar_authoring_stack` (sha256 `774bcd69809ead9e4cd7b6e06c74aa045ea9272a2321cd36f91bc9c76a99ab0c`, checked with `shasum -a 256` before the run; the binary was not executed by the harness outside the tests).

## Commands (run from `tests/native/r19-accept-support`, scratch HOME and PLANAR_DB)

Non-live rows (unfixed tree, before any production-support edit):

    PYTHONDONTWRITEBYTECODE=1 HOME=<scratch-root>/c02/home PLANAR_DB=<scratch-root>/c02/home/planar.db PLANAR_CANDIDATE=<scratch-root>/r19-candidate-c01-rev2/export/build/rewrite-debug/src/cmd/planar/planar_authoring_stack python3 -B -m unittest accept_test.AcceptLibTest.test_capture_limit_accepts_exact_real_error_envelope_only acceptance_cli_test.AcceptanceCliTest.test_rerun_index_normalizes_real_shaped_absolute_argv acceptance_cli_test.AcceptanceCliTest.test_rerun_index_still_refuses_host_path_outside_argv acceptance_cli_test.AcceptanceCliTest.test_limits_command_accepts_real_error_envelope acceptance_cli_test.AcceptanceCliTest.test_index_escapes_nul_next_to_host_path_in_binary_raw_file acceptance_cli_test.AcceptanceCliTest.test_index_dry_run_refuses_unredactable_raw_file_and_writes_nothing acceptance_cli_test.AcceptanceCliTest.test_index_dry_run_reports_expected_count_and_writes_nothing

Live rows (unfixed tree):

    PYTHONDONTWRITEBYTECODE=1 HOME=<scratch-root>/c02/home PLANAR_DB=<scratch-root>/c02/home/planar.db PLANAR_CANDIDATE=<scratch-root>/r19-candidate-c01-rev2/export/build/rewrite-debug/src/cmd/planar/planar_authoring_stack python3 -B -m unittest fault_live_test.FaultLiveTest.test_run_heavy_config_wait_timeouts_cover_registry_budget fault_live_test.FaultLiveTest.test_marker_wait_honors_configured_timeout fault_live_test.FaultLiveTest.test_check_completion_timeout_aborts_with_accept_error_and_kills_cli

Raw output (unredacted) stays in the scratch root; the tokenized copies are `harness/red-run.out`.

## Outcomes on the unfixed tree

| row | test | unfixed result (tokenized, from `red-run.out`) | red? |
|---|---|---|---|
| H2 | AcceptLibTest.test_capture_limit_accepts_exact_real_error_envelope_only | FAIL: `AssertionError: False is not true : the exact real envelope is accepted` | yes (behavioural) |
| H1 | AcceptanceCliTest.test_rerun_index_normalizes_real_shaped_absolute_argv | FAIL: `1 != 0 : a real-shaped absolute argv must index: ... rerun index would hold a host path` | yes |
| H3 | AcceptanceCliTest.test_rerun_index_still_refuses_host_path_outside_argv | PASS (guard, expectRed false, exempt from GREEN-NOFIX) | n/a |
| H4 | AcceptanceCliTest.test_limits_command_accepts_real_error_envelope | FAIL: `1 != 0 : the real error envelope is the closed reproduction: ... capture limit is not the closed diagnostic (exit 3)` | yes |
| H8 | AcceptanceCliTest.test_index_escapes_nul_next_to_host_path_in_binary_raw_file | FAIL: `1 != 0 : a binary raw file with a NUL next to a host path must index: ... host prefix remains after redaction in: ledger.db.index` | yes |
| H9 | AcceptanceCliTest.test_index_dry_run_refuses_unredactable_raw_file_and_writes_nothing | FAIL: `2 != 1 : ...` (the unfixed parser rejects the unknown `--dry-run` with exit 2, usage text) | yes (exit-code assertion; the unknown flag is the expected unfixed behaviour, per the brief) |
| H10 | AcceptanceCliTest.test_index_dry_run_reports_expected_count_and_writes_nothing | FAIL: `2 != 0 : a clean raw tree must pass the dry run: ...` (usage text, unknown flag) | yes (same as H9) |
| H5 | FaultLiveTest.test_run_heavy_config_wait_timeouts_cover_registry_budget | FAIL: `'marker_timeout_s' not found in {...}` and the same for `check_timeout_s` (the config carries neither key) | yes |
| H6 | FaultLiveTest.test_marker_wait_honors_configured_timeout | FAIL: `None is not an instance of AcceptError : a marker that appears after the configured wait must abort` (the unfixed driver waits 120 s and completes) | yes |
| H7 | FaultLiveTest.test_check_completion_timeout_aborts_with_accept_error_and_kills_cli | FAIL: `None is not an instance of AcceptError : a completion wait that expires must raise AcceptError, not None` (the unfixed driver waits 900 s and completes) | yes |

Counts: non-live 7 run, 6 failures, 1 pass (the guard H3); live 3 tests run, FAILED with 4 failure entries (H5 reports one per subtest for marker_timeout_s and check_timeout_s; H6 and H7 one each). No row with `expectRed` true passed on the unfixed tree, so `harness/GREEN-NOFIX.md` is not needed. Every failure is an assertion; none is an ImportError, SyntaxError, KeyError or ERROR. The only non-assertion text is the argparse usage of the unknown `--dry-run` flag (H9, H10), which the brief names as the expected unfixed behaviour.
