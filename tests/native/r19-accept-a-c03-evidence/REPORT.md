# R19-ACCEPT-A-C03 FINAL GATE REPORT

Result: passed=true (with one scoring note on the Python suites candidate path, below). Final run is `raw/run3/` (driver `driver.sh`; every command has `.log` and `.exit`; `ALLDONE` present). No source file was edited. Earlier attempts (`raw/run1`, `raw/run2` aborted, `raw/prelim-*`) are kept; run1's Linux gate failed from a driver HOME error and is superseded by run3, which ran the Linux gate to a pass.

## Candidate identity (identity.py, `raw/run3/identity-before.txt` and `identity-after.txt`, byte-identical)
| identity | value |
|---|---|
| HEAD | `91e0df24f2167471e49ec057c7159bce5dff3608` |
| sha256(git diff --binary HEAD -- src tests/native), tracked only | `d9ca75a13fcf1c3aa11b79ee38fdb5d24d733501deb4caf24c42d1830a6726c0` |
| combined sha256 (diff bytes plus untracked paths, pathspec src tests/native, evidence excluded) | `d90e991fb73d03d43e8be801bfc7271ba5cee31a6a9904d573f405a2917ed0c9` |
| sha256 of whole-tree `git diff --binary HEAD` (`whole-diff-sha.txt`) | `358db35d4e7bb9f1e4bfc33fe25d914e9ec73dc2128d852a4561a3fd7b596814` |

## Commands (exit codes read from `.exit` files, nothing piped)
| command | exit | result |
|---|---|---|
| `cmake --preset debug` | 0 | `configure-debug.log` |
| `python3 scripts/rewrite-check.py gate --build-dir build/rewrite-debug` | 0 | 100% passed out of 1854, `Executed 1854 registered tests; zero skips`, `Formatting and Doxygen checked 251 active source files`, final `ninja: no work to do.` |
| `cmake --build build/rewrite-debug --target planar_tests` | 0 | |
| `ctest --test-dir build/rewrite-debug --output-on-failure` | 0 | `100% tests passed out of 1854`, 0 failed |
| no-change build, debug (`--target all planar_tests`, after the successful build) | 0 | `ninja: no work to do.` (zero steps) |
| `cmake --preset release` | 0 | |
| `python3 scripts/rewrite-check.py gate --build-dir build/rewrite-release` | 0 | 100% passed out of 1854, zero skips, no work to do |
| `cmake --build build/rewrite-release --target planar_tests` | 0 | |
| `ctest --test-dir build/rewrite-release --output-on-failure` | 0 | `100% tests passed out of 1854` |
| no-change build, release | 0 | `ninja: no work to do.` |
| `python3 -B scripts/redact-local-paths.py --repo-path <repo> --home <home> check <files>` | 0 | empty log. `COMMIT-LIST.txt` does not exist in this evidence directory, so the explicit list (`redact-files.txt`: all tracked changes plus untracked non-evidence, non-build files) was used; the script requires `--repo-path` and `--home` |
| Python suites with `PLANAR_CANDIDATE=<repo>/build/rewrite-debug/src/cmd/planar/planar_authoring_stack` (`suites.log`) | 1 | REFUSED by the runner: `candidate must not live in a shared build/rewrite-* directory`; no test ran. The brief's candidate path is unusable by design; this is the one non-zero exit and is not a candidate defect |
| Python suites with the recorded out-of-tree candidate (`tests/native/r19-accept-evidence/gate/candidate/PATH`, `--record .../candidate.json`), -B, PYTHONDONTWRITEBYTECODE=1, scratch HOME and PLANAR_DB (`suites-recorded.log`) | 0 | accept_test 61, acceptance_cli_test 27, quota_live_test 28, fault_live_test 33: 149 run, 0 failures, 0 errors, 0 skipped |
| `python3 -B -m unittest discover -s . -p '*_test.py'` in `tests/native/r19-accept-support` | 0 | `Ran 149 tests ... OK`, no skips |
| clean export (cached + untracked non-ignored files copied to a scratch dir; no gitignored file; evidence and build excluded): configure debug, build `planar_ledger_format_tests`, ctest `ledger format initializes exact metadata schema` | 0 | `Test #111: replacement.ledger format initializes exact metadata schema and Glaze event ... Passed`, `100% tests passed out of 1` (`cleanexport.log`) |
| `scripts/linux-gate.sh <scratchpad>/lg-r19-accept-run7` (real HOME, 142 GiB free at start) | 0 | Linux debug and release both `100% tests passed out of 1854`, both no-change builds `ninja: no work to do.`, host manifest before/after and versus container context equal (`linux/manifest-compare.txt`). Container logs copied to `raw/run3/linux/` |
| `docker builder prune -af --filter 'description~=home/planar'` x2 | 0, 0 | `Total: 0B` each (`prune1.log`, `prune2.log`) |

Disk (`df -h /System/Volumes/Data`, Avail): 90 GiB before (`df-before.txt`), 90 GiB before Linux (`df-before-linux.txt`), 73 GiB after the gate (`df-after-gate.txt`), 74 GiB after prune (`df-after-prune.txt`). Linux script own df: 142 GiB before, 188 GiB after (units differ, 1K blocks).

## Test inventory
`ctest -N` in build/rewrite-debug: Total Tests 1854 (`raw/run3/ctestN-debug.log`; sorted name list saved as `inventory.txt`). Compared against the previous `inventory.txt` in this directory (written by the run1 gate, which had matched `tests/native/r19-accept-a-c02-evidence/inventory.txt`; evidence dirs are gitignored, not committed): 1854 names, identical sets by `comm -3`. New names: none. Removed: none. Release and both Linux configurations also register 1854. `replacement.ledger format initializes exact metadata schema and Glaze event` is #612 (#111 in the clean export).

Zero-work result: `ninja: no work to do.` after a successful build in Debug, Release and both Linux configurations, as separate checks and as the last step of each gate.

## (a) Acceptance criteria to tests (round record `plan/INVARIANTS.md` revision 4, 67 rows; file:line resolved against the working tree)
Rows whose observer is RECORD/RUN have no unittest or Catch2 test; their observer is the named shell check or live run in the earlier evidence (`r1/`, `red/`, `gate/`). Corrective-round findings closed by C03 are the same C3-* rows (the C02 review findings map to C3-R1x fixture/Linux-context rows and C3-R2x commit-list rows).

| id | observer (test names with file:line where a registered test exists) | expectRed |
|---|---|---|
| C3-R1a | RECORD: shell `cmp -s fixtures/ledger/expected-schema.sql tests/native/r12b-evidence/expected-schema.sql`, the two `shasum -a 256` outputs and `git check-ignore -v`, recorded in `r19-accept-a-c03-evidence/r1/` | true (the fixture file does not exist be |
| C3-R1a2 | SCHEMA612 = ctest #612 in build/rewrite-debug (#111 in the clean export), src/engine/ledger/format.t.cpp:115 REQUIRE_FALSE(expected_sql.empty()) | false (proof is the mutant kill; passes  |
| C3-R1b | SCHEMA612 = ctest #612 in build/rewrite-debug (#111 in the clean export), src/engine/ledger/format.t.cpp:115 REQUIRE_FALSE(expected_sql.empty()) | true (pre-fix clean-export run fails beh |
| C3-R1c | RECORD: `r19-accept-a-c03-evidence/r1/RED.md` plus raw logs under `r19-accept-a-c03-evidence/r1/raw/`; observer: the ctest log of SCHEMA612 (fail then pass) ; SCHEMA612 = ctest #612 in build/rewrite-debug (#111 in the clean export), src/engine/ledger/format.t.cpp:115 REQUIRE_FALSE(expected_sql.empty()) | true |
| C3-R1d | RECORD: `identity.py src` output before and after, `git diff --numstat HEAD -- src` | false (guard) |
| C3-R1e | RECORD: before and after `shasum -a 256` outputs in `r19-accept-a-c03-evidence/r1/` | false (guard) |
| C3-R1f | x08.linux_recipe_pins (tests/native/v5-02-x08-support/x08.cmake:107; tests/acceptance/x08_recipe.py) | true (pre-fix runner has all of them) |
| C3-R1g | x08.linux_recipe_pins (tests/native/v5-02-x08-support/x08.cmake:107; tests/acceptance/x08_recipe.py) | true |
| C3-R1h | x08.linux_recipe_pins (tests/native/v5-02-x08-support/x08.cmake:107; tests/acceptance/x08_recipe.py) | true |
| C3-R1i | x08.linux_recipe_pins (tests/native/v5-02-x08-support/x08.cmake:107; tests/acceptance/x08_recipe.py) | true |
| C3-R1j | RECORD: FOUR separate greps over the C03 result section of `docs/vernacular/rounds/R19-ACCEPT-A.md`, each count at least 1 (Q7): `write_manifest` (with `linux-gate.sh` lines 125-129), the `is_excluded` case (lines 101-104), the dockerignore re-include (`!tests | false |
| C3-R1k | RECORD: `grep -n "r12b-evidence/expected-schema" docs/testing.md` exits 1 and `grep -n "src/engine/CMakeLists.txt. points" docs/testing.md` exits 1 (recorded) | true (current file has the hits) |
| C3-R1q | RECORD: `bash tests/native/r19-accept-support/ignored_context_check.sh <runner> <repo>` (owned driver under `r19-accept-support`, Q2; the verbatim one-line equivalent is in `r1/RESULT.md`) prints the set and the count; output files `r1/raw/ignored-context-befo | true (before-fix run prints the old path |
| C3-R1l | HeavyDriverTest.test_run_heavy_exits_nonzero_when_the_scenario_stopped (tests/native/r19-accept-support/accept_test.py:1108) | true (current `main` returns 0) |
| C3-R1m | HeavyDriverTest.test_run_heavy_exits_zero_and_writes_result_for_a_completed_scenario (tests/native/r19-accept-support/accept_test.py:1119) | false (passes on current code; proof is  |
| C3-R1n | HeavyDriverTest.test_run_heavy_config_overlays_the_fixture_and_ledger_cmake (tests/native/r19-accept-support/accept_test.py:1128) | true (current config has no `overlay`) |
| C3-R1o | HeavyDriverTest.test_export_overlay_copies_working_tree_bytes_of_the_named_paths (tests/native/r19-accept-support/accept_test.py:1164) | true (current code ignores the key; the  |
| C3-R1p | HeavyDriverTest.test_export_overlay_refuses_a_path_missing_from_the_working_tree (tests/native/r19-accept-support/accept_test.py:1179) | true (current code raises nothing) |
| C3-H0 | AcceptanceCliTest.test_live_commands_abort_before_first_action_on_candidate_mismatch (tests/native/r19-accept-support/acceptance_cli_test.py:266); FaultLiveTest.test_run_heavy_aborts_on_unrecorded_candidate (tests/native/r19-accept-support/fault_live_test.py:447) | false |
| C3-H1 | HeavyDriverTest.test_run_heavy_exits_nonzero_when_the_scenario_stopped (tests/native/r19-accept-support/accept_test.py:1108) | false (live; proof is the run and the R1 |
| C3-H2 | FaultLiveTest.test_interrupted_check_is_recovered_by_check_recover (tests/native/r19-accept-support/fault_live_test.py:88); FaultLiveTest.test_state_survives_process_restart_between_phases (tests/native/r19-accept-support/fault_live_test.py:106) | false |
| C3-H3 | FaultLiveTest.test_takeover_fences_old_run_token (tests/native/r19-accept-support/fault_live_test.py:127) | false |
| C3-H4 | FaultLiveTest.test_rejecting_verdict_admits_repair_and_worker_returns (tests/native/r19-accept-support/fault_live_test.py:140) | false |
| C3-H5 | FaultLiveTest.test_load_raising_max_rounds_clears_blocker (tests/native/r19-accept-support/fault_live_test.py:170); FaultLiveTest.test_second_rejection_is_capped_and_opens_max_rounds_blocker (tests/native/r19-accept-support/fault_live_test.py:155) | false |
| C3-H6 | FaultLiveTest.test_final_approval_after_faults_is_exact_target (tests/native/r19-accept-support/fault_live_test.py:180) | false |
| C3-H7 | FaultLiveTest.test_replays_and_inspections_do_not_reexecute_checks_or_write (tests/native/r19-accept-support/fault_live_test.py:201); FaultLiveTest.test_run_scenario_aborts_on_tampered_replay (tests/native/r19-accept-support/fault_live_test.py:314); FaultLiveTest.test_run_scenario_aborts_when_counter_increases_on_replay (tests/native/r19-accept-support/fault_live_test.py:321); FaultLiveTest.test_run_scenario_aborts_when_inspection_changes_ledger (tests/native/r19-accept-support/fault_live_test.py:307) | false |
| C3-H8 | FaultLiveTest.test_history_references_through_faults (tests/native/r19-accept-support/fault_live_test.py:210) | false |
| C3-H9 | FaultLiveTest.test_every_check_attempt_folder_keeps_its_command_logs (tests/native/r19-accept-support/fault_live_test.py:670); FaultLiveTest.test_first_completed_check_passes_boundary_rows (tests/native/r19-accept-support/fault_live_test.py:279) | false |
| C3-H10 | RECORD: file count per action kind equals the action count in `result.json` `steps`; `shasum -a 256` list at copy time and again at the end (equal) | false |
| C3-L20 | AcceptanceCliTest.test_rerun_index_records_every_required_fault_field (tests/native/r19-accept-support/acceptance_cli_test.py:515); AcceptanceCliTest.test_rerun_index_fails_when_any_required_field_is_missing (tests/native/r19-accept-support/acceptance_cli_test.py:530); AcceptanceCliTest.test_rerun_index_still_refuses_host_path_outside_argv (tests/native/r19-accept-support/acceptance_cli_test.py:652) | false |
| C3-L21a | AcceptanceCliTest.test_limits_command_requires_exact_closed_diagnostic (tests/native/r19-accept-support/acceptance_cli_test.py:192); AcceptanceCliTest.test_limits_command_accepts_real_error_envelope (tests/native/r19-accept-support/acceptance_cli_test.py:667) | false |
| C3-L21b | AcceptanceCliTest.test_packet_command_records_literal_source_lines (tests/native/r19-accept-support/acceptance_cli_test.py:227) | false |
| C3-L21c | AcceptanceCliTest.test_index_dry_run_reports_expected_count_and_writes_nothing (tests/native/r19-accept-support/acceptance_cli_test.py:766); AcceptanceCliTest.test_index_escapes_nul_next_to_host_path_in_binary_raw_file (tests/native/r19-accept-support/acceptance_cli_test.py:693); AcceptanceCliTest.test_index_is_redacted_with_recorded_hashes_and_excludes_raw (tests/native/r19-accept-support/acceptance_cli_test.py:156); AcceptanceCliTest.test_index_lists_only_host_lines_and_refuses_out_dir_other_than_work_cwd (tests/native/r19-accept-support/acceptance_cli_test.py:722) | false |
| C3-L21d | AcceptanceCliTest.test_scan_command_fails_on_host_path_in_committed_evidence (tests/native/r19-accept-support/acceptance_cli_test.py:130) | false |
| C3-L21e | AcceptanceCliTest.test_verify_redaction_runs_standard_tool_and_rejects_scratch_tokens_in_manifest (tests/native/r19-accept-support/acceptance_cli_test.py:291); AcceptanceCliTest.test_verify_redaction_checks_original_hash_and_real_prefixes (tests/native/r19-accept-support/acceptance_cli_test.py:336) | false |
| C3-L21f | RECORD: `test -s` of the manifest and of the count file exit 0 after L21; `cmp` of the count file number with the `index` count; `shasum -a 256` of `runs/pre-index/report-shas.txt` equal before and after | true (the manifest and the count file do |
| C3-S1 | HeavyDriverTest.test_run_heavy_exits_nonzero_when_the_scenario_stopped (tests/native/r19-accept-support/accept_test.py:1108) | false |
| C3-D1 | RECORD: `df` output files in `r19-accept-a-c03-evidence/gate/free-space/` and the sizes of the removed trees | false |
| C3-H11 | RECORD: `grep -n "run_acceptance" run_heavy.py fault_driver.py` exit 1 and identity outputs before the run and at PKG-GATE | false |
| C3-L0 | RECORD: the two identity outputs and `cmp -s` of the saved copy with the file at that moment (exit 0) | false |
| C3-R2a | AcceptanceCliTest.test_commit_list_check_accepts_a_list_of_existing_sorted_files (tests/native/r19-accept-support/acceptance_cli_test.py:805); AcceptanceCliTest.test_commit_list_check_fails_naming_a_missing_path (tests/native/r19-accept-support/acceptance_cli_test.py:811) | true (stub exits 1 with a "not implement |
| C3-R2b | AcceptanceCliTest.test_commit_list_check_refuses_snapshot_original_and_unlisted_superseded_paths (tests/native/r19-accept-support/acceptance_cli_test.py:853); AcceptanceCliTest.test_commit_list_check_refuses_unsorted_duplicate_raw_and_bytecode_paths (tests/native/r19-accept-support/acceptance_cli_test.py:821) | true |
| C3-R2c | AcceptanceCliTest.test_commit_list_check_refuses_files_that_pin_report_hashes (tests/native/r19-accept-support/acceptance_cli_test.py:876) | true |
| C3-R2d | RECORD: `commit-list` exit 0 on the final list (command, exit file, stderr empty), `grep -c -x -F` of each of the six excluded FULL repository paths (`tests/native/r19-accept-a-c02-evidence/COMMIT-LIST.txt`, `tests/native/r19-accept-a-c02-evidence/probes/PROBE | false |
| C3-R2e | RECORD: `redaction/COMMANDS.txt`, `.exit`, `.count` files; `commit-list` exit 0 precedes in the same record | false |
| C3-R2f | AcceptanceCliTest.test_commit_list_check_refuses_files_that_pin_report_hashes (tests/native/r19-accept-support/acceptance_cli_test.py:876) | false |
| C3-R2g | RECORD: `grep -n "r19-accept-a-c02-evidence/COMMIT-LIST.txt"` over the record and README shows only "superseded" mentions (reviewed), `grep -n "check hashes are in" docs/vernacular/rounds/R19-ACCEPT-A.md` exits 1 | true (current text names the C02 list as |
| C3-R2h | RECORD: `commit-list` exit 0 plus before and after `shasum -a 256` of the four files | false |
| C3-R2i | a rewrite or reformat of an existing subcommand; a changed existing message | RECORD: the `diff` output and the `grep  |
| C3-R2j | RECORD: `test -s` of both files exit 0 recorded at PKG-LIST start, then `commit-list` exit 0 on a list naming both | true (neither file exists now) |
| C3-R3a | FaultLiveTest.test_trace_is_derived_from_executed_steps_in_plain_cmake_run (tests/native/r19-accept-support/fault_live_test.py:579) | false (passes before the mutant; proof i |
| C3-R3b | RECORD: shell `grep -c "Kills F11-b" tests/native/r19-accept-support/fault_live_test.py` is 1 and sits in the docstring of `PLAIN17`; `ast` before and after lists equal for `fault_live_test.py` | true (the current docstring claim is not |
| C3-R3c | the prober's raw logs `probes/raw/F11-b.log` and `probes/raw/L17.log` under `tests/native/r19-accept-a-c03-evidence/` | false |
| C3-A1 | RECORD: (a) `test -e tests/native/r19-accept-a-evidence/red-c01/history/RED.md` and `test -d tests/native/r19-accept-evidence/gate/suites/summary.txt.raw` exit 0 (Q3: `summary.txt.raw` is a DIRECTORY of per-module raw files and no `summary.txt` exists, so `tes | true (the line 772 and A3 row carry the  |
| C3-A2 | RECORD: the `comm` output files and the line counts (before 134, after 134 plus the number of new tests) | false |
| C3-A3 | RECORD: every F-row in the record's C02 table has a "fires through" entry (shell count equals the number of F rows killed by `AcceptError`) | false |
| C3-G1 | RECORD: `comm -23 before after` empty and exit 0 for C++ and Python lists | false |
| C3-G2 | RECORD: identity outputs | false |
| C3-G3 | RECORD: `grep` of the new record text for 64-hex strings next to the words REPORT, review or roadmap exits 1; `commit-list` exit 0 | false |
| C3-G4 | AcceptanceCliTest.test_limits_command_requires_exact_closed_diagnostic (tests/native/r19-accept-support/acceptance_cli_test.py:192) | false |
| C3-G5 | RECORD: `find tests/native/r19-accept-support -name "*.pyc" -o -name __pycache__` prints nothing (last step of PKG-GATE) | false |
| C3-G6 | AcceptanceCliTest.test_suites_command_refuses_skips_failures_and_zero_matched (tests/native/r19-accept-support/acceptance_cli_test.py:459) | false |
| C3-G7 | RECORD: `ctest --test-dir build/rewrite-debug --no-tests=error` summary lines and the matched list for the two named tests | false |
| C3-G8 | x08.linux_recipe_pins (tests/native/v5-02-x08-support/x08.cmake:107; tests/acceptance/x08_recipe.py) | false |
| C3-G9 | the stale statements left | RECORD: row line (`sed -n 809p`, or its  |
| C3-G10 | RECORD: `redact-local-paths.py check` exit 0 and the D26 greps exit 1 over `plan/DECISIONS.md` and `plan/INVARIANTS.md`; they are members of `COMMIT-LIST.txt` | false |
| C3-G11 | RECORD: stage-0 and final `find` of the regular files, bytewise sorted, each hashed with `shasum -a 256`, lists and `comm -23` empty | false |

## (b) Red-first
Source: the coders' evidence only; I did not re-run any red check. Method used by the coders: Python rows `python3 -B -m unittest <module>.<Class>.<test>` from `tests/native/r19-accept-support` against a pre-fix snapshot; C++/x08 rows `ctest -R <exact name>`; the fixture row used a clean export (`r1/RED.md`).
- Snapshot 1: `red/snapshot/` (candidate.diff sha256 83a16fdee033fce5a1a792d8d65f4090fba8dacffc2d38013684cacb4daeba19, SHA256SUMS 88a39e35b6d5d0c9f61b8741f3b2bec9effc8d99568c6fabf03a6b05c7764de8), results `red/RED-1.md`, raw logs `red/raw/`.
- Snapshot 2: `red/snapshot-2/` (candidate.diff sha256 83a16fde...eba19, SHA256SUMS 6061761a9d40d44e0bd60c3dc362d22222aeaf8b9fbf0803d0e27425d28bef20), results `red/RED.md`, raw logs `red/raw-2/`.
- SCHEMA612 (ctest `replacement.ledger format initializes exact metadata schema and Glaze event`, format.t.cpp:115): in the checkout it passed at red time (NOT RED in the checkout, a guard, because the ignored old fixture existed). Real behavioural red only in the clean export before the fix: ctest exit 8, `format.t.cpp:115: FAILED: REQUIRE_FALSE( expected_sql.empty() )`; passed after the fix. Command: `cmake --preset debug`, build `planar_ledger_format_tests`, `ctest --test-dir build/rewrite-debug -R "ledger format initializes exact metadata schema" --no-tests=error` inside the export (`r1/RED.md`; pre-fix identity for src fixtures scripts tests/acceptance docs/testing.md sha256 2cb15eb83efa6c00d393d1e339deac84d30378231c9bc8c304909e32cec6f418; export lists before/after sha256 3f8d205b...dd04 / 68ff3c11...eff4bf9). Today's clean-export run above is the green side again.
- x08.linux_recipe_pins: real red, `no RUN asserts test -s fixtures/ledger/expected-schema.sql` (only the first failing check is reported; the sub-checks C3-R1f..i are not individually distinguishable).
- HeavyDriverTest.test_run_heavy_exits_nonzero_when_the_scenario_stopped (accept_test.py:1112 in snapshot): real red, `0 == 0 : a stopped scenario must exit nonzero`.
- HeavyDriverTest.test_run_heavy_exits_zero_and_writes_result_for_a_completed_scenario: guard that already passed.
- HeavyDriverTest.test_run_heavy_config_overlays_the_fixture_and_ledger_cmake: real red (`None != [overlay list]`).
- HeavyDriverTest.test_export_overlay_copies_working_tree_bytes_of_the_named_paths: real red (head bytes != working tree bytes).
- HeavyDriverTest.test_export_overlay_refuses_a_path_missing_from_the_working_tree: real red (AcceptError not raised).
- AcceptanceCliTest commit_list_check tests (refuses/accepts, sorted/duplicate/raw/bytecode/snapshot/original/pins-report-hashes): real reds, `commit-list: not implemented` style failures; run 2 table in `red/RED.md` has the per-test lines.
- RECORD rows and live rows (H0-H9, R3a, L20/L21): no red-first test; not-registered at red time per `red/RED.md`. Missing from the evidence: the exact per-test command lines for the three Python rows beyond the shared method above, and any red run for the later PKG-SUP2/PKG-T3 rows (R2d-j, R3b,c); I did not invent them.

## (c) Deliberate behaviour differences
None found in this preliminary run. The only product-tree delta is the one CMake line pointing the schema fixture at `fixtures/ledger/expected-schema.sql` (`src/engine/ledger/CMakeLists.txt`); `storage.cpp` and `execution.t.cpp` are the C02 files. No diagnostic text changed. I did not locate a contract clause permitting a behaviour difference because none is claimed.

## Limitations
- The brief's PLANAR_CANDIDATE path (inside build/rewrite-debug) is refused by `run_acceptance.py` by design, so the suites were scored against the recorded out-of-tree candidate (sha256 equal to `candidate.json`), not a fresh build of the current tree. Current Debug, Release and Linux binaries are covered by ctest.
- Gitignored `tests/native/r19-accept-support/__pycache__/accept_test.cpython-315.pyc` exists (`pyc.txt`). It predates this run and is not mine; every run used -B and PYTHONDONTWRITEBYTECODE=1. Left in place; it blocks the C3-G5 "no pycache" RECORD check until its owner removes it.
- Linux prune freed 0B because the Linux script already pruned its own cache (`linux/builder-prune.log`).
- Sections (a) to (c) are carried from the earlier reading of the coders' evidence; no red check was re-run. Missing from the evidence: exact per-test command lines for the Python red rows beyond the shared method, and red runs for PKG-SUP2/PKG-T3 rows (R2d-j, R3b, c).
- Files I edited: `REPORT.md`, `inventory.txt` and `raw/run3/*` in this evidence directory only (`raw/run3/REPORT-prelim-3-run1.md` is the preserved previous report).
