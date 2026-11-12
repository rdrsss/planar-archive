# R19-ACCEPT-A-C02 PKG-HARNESS fix notes (wave 2, D31 H1-H10)

Scope: support files only. `src/` unchanged (identity `83a16fdee033fce5a1a792d8d65f4090fba8dacffc2d38013684cacb4daeba19`, re-checked after the fixes). No public field, method or hook was added to a product type. No existing test was renamed or removed; every existing diagnostic message is byte-identical (new messages only).

## Files edited or created (owned-file list for the round record)

Edited:
- `tests/native/r19-accept-support/accept_test.py`: one new test, `AcceptLibTest.test_capture_limit_accepts_exact_real_error_envelope_only` (H2).
- `tests/native/r19-accept-support/acceptance_cli_test.py`: `import run_acceptance`; six new tests `test_rerun_index_normalizes_real_shaped_absolute_argv` (H1), `test_rerun_index_still_refuses_host_path_outside_argv` (H3 guard), `test_limits_command_accepts_real_error_envelope` (H4), `test_index_escapes_nul_next_to_host_path_in_binary_raw_file` (H8), `test_index_dry_run_refuses_unredactable_raw_file_and_writes_nothing` (H9), `test_index_dry_run_reports_expected_count_and_writes_nothing` (H10); helpers `real_shaped_recover_argv` and `tree_snapshot`.
- `tests/native/r19-accept-support/fault_live_test.py`: imports `re` and `unittest.mock`; helpers `capture_error`, `recording_popen`, `kill_leftover_clis`, `live_lines_naming`; three new tests `test_run_heavy_config_wait_timeouts_cover_registry_budget` (H5), `test_marker_wait_honors_configured_timeout` (H6), `test_check_completion_timeout_aborts_with_accept_error_and_kills_cli` (H7).
- `tests/native/r19-accept-support/accept_lib.py`: `closed_capture_limit_ok` accepts the exact engine error envelope (H2 fix); new private `_is_error_envelope`.
- `tests/native/r19-accept-support/run_acceptance.py`: `normalize_argv` and its use in `build_rerun_index` (H1); `escape_control` and its use in `compact_index` (H8); `cmd_index_dry_run` and the `--dry-run` flag of `index` (H9, H10); module header line for the flag.
- `tests/native/r19-accept-support/fault_driver.py`: constants `MARKER_TIMEOUT_S`, `CHECK_TIMEOUT_S`, `VERB_TIMEOUT_S`; public helpers `descendant_pids`, `_signal_group`, `kill_session` (H7); `_run` and `check_round` start every CLI in its own session, read the wait keys (H5, H6) and convert completion timeouts into `AcceptError` after `kill_session` (H7).
- `tests/native/r19-accept-support/fault/cmake_rerun/run_heavy.py`: `CHECK_REQUESTS = 4`, `wait_budget_s`, and the keys `marker_timeout_s` and `check_timeout_s` set by `build_config` (H5).

Created (new helper fixture, no product code):
- `tests/native/r19-accept-support/fault/delayed_marker_check.py`: a check that sleeps 4 s before its rendezvous marker and lingers at most 30 s when held (used by H6 and H7 through `mock.patch` of `fault_driver.SLEEP_CHECK`).

Harness records (this directory): `pre-fix-identity.txt`, `post-fix-identity.txt`, `RED.md`, `red-run.out`, `green-run.out`, `FIX-NOTES.md`.
Redaction records (this round's evidence tree): `redaction/support-allowlist.txt` (committed) and `redaction/original/support-users-grep.txt` (never committed).

## Identity

- Pre-fix combined sha256 of `tests/native/r19-accept-support` (pre-fix-identity.txt): `2d8085186ebff43d125415fec54e7c1282773aed0b8063b05cc5afaa00cc01cb`.
- Post-fix combined sha256 (post-fix-identity.txt): `831d0ace78c3d09baa549e1e0444bee0b4735033b879d960a2b92af1bbd818da`.

## Fix per row and the mutant each new test should kill

| row | fix | new test | mutant it must kill (the prober applies it) |
|---|---|---|---|
| H1 | `build_rerun_index` rewrites each absolute element of `faults.recover.argv` to `<path>/<basename>`; the marker refusal is unchanged | `AcceptanceCliTest.test_rerun_index_normalizes_real_shaped_absolute_argv` | build_rerun_index copies the argv whole (the unfixed behaviour): exit 1, host path refusal |
| H2 | `closed_capture_limit_ok` also accepts the exact envelope {error: closed object, kind: "error", version: 1}; everything else refused; diagnostic text untouched | `AcceptLibTest.test_capture_limit_accepts_exact_real_error_envelope_only` | H2a: accept only the bare object (fails at the envelope case); H2b: accept any envelope (fails at the envelope other maximum/code/message/field/limit/extra key/kind/version cases) |
| H3 | none (guard) | `AcceptanceCliTest.test_rerun_index_still_refuses_host_path_outside_argv` | drop the HOST_PATH_MARKERS refusal in cmd_rerun_index (expectRed false; passes unfixed by design) |
| H4 | none in cmd_limits: the envelope is accepted through the H2 fix in accept_lib | `AcceptanceCliTest.test_limits_command_accepts_real_error_envelope` | H4: cmd_limits compares stderr with the bare closed object only (fails at the good envelope; the other-maximum refusal still holds) |
| H5 | `build_config` sets `marker_timeout_s` and `check_timeout_s` to the registry budget: sum of `timeout_ms` (7200 s for the two presets) times `CHECK_REQUESTS` (4), i.e. 28800 s; `fault_driver.check_round` reads both keys with the 120 s and 900 s defaults for mini configs | `FaultLiveTest.test_run_heavy_config_wait_timeouts_cover_registry_budget` | H5: build_config omits the keys, or sets 120 and 900 (fails the budget assertion; the recorded 1458551 ms and 819557 ms durations are asserted below both values) |
| H6 | `check_round` uses `marker_timeout_s` for the held-check marker wait and for `kill_check` | `FaultLiveTest.test_marker_wait_honors_configured_timeout` | H6: check_round hard-codes 120 for the marker wait (the marker_timeout_s 1 run completes instead of raising the marker AcceptError) |
| H7 | completion wait uses `check_timeout_s`; expiry calls `kill_session` (SIGKILL the CLI, its descendants and their groups, reap, confirm groups gone) and raises `AcceptError`, and `run_scenario` writes ABORTED.json | `FaultLiveTest.test_check_completion_timeout_aborts_with_accept_error_and_kills_cli` | H7a: remove the TimeoutExpired conversion (TimeoutExpired escapes: fails the AcceptError type assertion); H7b: convert without the SIGKILL (the CLI is not reaped: fails the returncode assertion; the leftover check child is caught by the `ps` check) |
| H8 | `compact_index` escapes NUL and every control byte (< 0x20, 0x7f) of each hit line as `\xNN`; digest, size and hit count unchanged | `AcceptanceCliTest.test_index_escapes_nul_next_to_host_path_in_binary_raw_file` | H8: hit lines emitted unescaped (fails the no-control-byte assertion and the exit 0 assertion) |
| H9 | `index --dry-run` computes the indexes in a staging copy outside the checkout, runs the standard redaction there, applies the residual scan, names each offending raw file and exits 1; writes nothing under the out directory | `AcceptanceCliTest.test_index_dry_run_refuses_unredactable_raw_file_and_writes_nothing` | H9a: skip the residual scan in the dry run (exit 0: fails the exit-1 assertion); H9b: the dry run writes the index or the TEMP record (fails the snapshot and the absent-index assertions) |
| H10 | dry run prints `expected-index-files: N` with N the regular files under the raw root, exits 0, writes nothing; the real index then writes exactly N files | `AcceptanceCliTest.test_index_dry_run_reports_expected_count_and_writes_nothing` | H10a: miscount (for example N-1: fails the printed-count assertion); H10b: the dry run writes under the out directory (fails the snapshot assertion) |

## Red-first

The new tests were run unfixed before any production-support edit (`RED.md`). Six non-live tests and three live tests failed by assertion; the H3 guard passed as designed. No expectRed-true row passed unfixed, so there is no GREEN-NOFIX item.

## Green

- `accept_test` and `acceptance_cli_test` (final tree): 69 tests, OK, zero failures, zero errors, zero skips (`green-run.out` part 1).
- The three new live tests (final tree): 3 tests, OK (`green-run.out` part 2).
- Full `fault_live_test` regression (final tree): 28 tests, OK, zero skips; this includes the 25 existing F-tests, which exercise the changed spawn and wait code (`green-run.out` part 3).
- No stray processes remained after the live runs (checked with ps after each run, including the marker-test leak that the descendant fix removed).
- Support allowlist (D26): the users-prefix literal occurs at exactly six sites (`run_acceptance.py` 2, `accept_test.py` 3, `acceptance_cli_test.py` 1), with the same content hashes as `redaction/support-allowlist.txt`. No new site was added.
- No bytecode was written (`-B`, `PYTHONDONTWRITEBYTECODE=1`).

## Limits and changes the coordinator should know

1. Product check children are spawned in their own process group (`src/engine/process/runner.cpp`, `POSIX_SPAWN_SETPGROUP`). Killing only the CLI group leaves the check child running (observed: the check runner and its helper survived the marker test before this fix). `kill_session` therefore signals the CLI's descendant groups found from a `ps -axo pid=,ppid=` snapshot taken before the kill. If `ps` cannot run, `descendant_pids` returns an empty list and a check child could survive; the `ps` check in the H7 test would then fail.
2. Behaviour change in the driver: every CLI verb (`_run`) and every non-held check (`check_round`) now starts in its own session (`start_new_session=True`; previously only held checks did). This is needed so the CLI group can be signalled. The driver is not interactive, so no terminal signal path depends on the old shared group.
3. `_run` communicate conversion (TimeoutExpired at 900 s into AcceptError after `kill_session`): implemented, but no cheap observer exists without a hook. Covered by review only; no hook was added (D31, standing lesson 7). Reported as a limit.
4. Not changed (outside H5-H7): the held-check marker timeout still raises AcceptError without killing the held CLI, so a held CLI and its check child keep running after that abort until their check ends. The heavy run's own abort handling (D33(f), PKG-HEAVY kills its process group) is the existing path; the coordinator should decide whether the marker timeout must also kill the CLI.
5. The H5 budget is 28800 s per wait (4 requests times the 7200 s registry sum). This is the literal D31 formula; the recorded R19-CMAKE durations (1458551 ms, 819557 ms) sit far below it.
6. `ps` is used by `descendant_pids` and by the H7 test (`live_lines_naming`); both work on macOS and Linux.
