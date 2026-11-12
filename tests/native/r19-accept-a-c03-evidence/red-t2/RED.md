# red-t2: R2 commit-list tests against the stub (PKG-T2)

Precondition: `runs/run_acceptance.at-l21.txt` did NOT exist (no `r19-accept-a-c03-evidence/runs/` directory) when this package ran; proceeded anyway. The pre-edit copies were kept only in the session scratchpad (`ra_before.py`, `cli_before.py`); `diff ra_before.py run_acceptance.py | grep -c '^<'` printed 0 (only added lines).

Stub: `run_acceptance.py` gains `cmd_commit_list` (raises `lib.AcceptError("commit-list: not implemented")`) and the `commit-list` subparser (`--list`, `--repo-root`, both required). Nothing else changed.

Command per test (cwd tests/native/r19-accept-support): `python3 -B -m unittest -v acceptance_cli_test.AcceptanceCliTest.<id>`; raw logs `<id>.log` here. Every failure is an AssertionError against the stub's "commit-list: not implemented" stderr, exit 1 (no import errors):

- test_commit_list_check_accepts_a_list_of_existing_sorted_files: `1 != 0` (exit code).
- test_commit_list_check_fails_naming_a_missing_path: 'b/gone.txt' not in stderr.
- test_commit_list_check_refuses_unsorted_duplicate_raw_and_bytecode_paths: every refusal subtest fails on the missing rule word; both PASS guards (rawdata, draw) fail on exit 1 != 0 (stub exits 1; they pass once implemented).
- test_commit_list_check_refuses_snapshot_original_and_unlisted_superseded_paths: four refusal subtests fail on the word; the allowance run fails on exit.
- test_commit_list_check_refuses_files_that_pin_report_hashes: 'a/pins.txt' not in stderr.

Full-suite runs (`acceptance_cli_test.log`, `accept_test.log`):
- acceptance_cli_test: Ran 27, failures 22 = the 5 new tests (21 failing subtests+parents) + test_live_commands_abort_before_first_action_on_candidate_mismatch (fails because PLANAR_CANDIDATE is unset in this shell, by design).
- accept_test: Ran 56, failures 4 = the wave-1 red HeavyDriverTest tests awaiting PKG-SUP1 impl (not touched).

Post-edit identity: identity-post.txt.
