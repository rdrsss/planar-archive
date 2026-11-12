# R19-ACCEPT-A-C03 red check

Red-time identity (covers tracked diff bytes of src and tests/native plus untracked files under src and tests/native/r19-accept-support):
- HEAD: 91e0df24f2167471e49ec057c7159bce5dff3608
- sha256 candidate.diff: 83a16fdee033fce5a1a792d8d65f4090fba8dacffc2d38013684cacb4daeba19
- sha256 SHA256SUMS: 88a39e35b6d5d0c9f61b8741f3b2bec9effc8d99568c6fabf03a6b05c7764de8
- Snapshot: snapshot/ (candidate.diff, untracked/ with 21 files, SHA256SUMS).

Build: cmake --build build/rewrite-debug --target planar_tests exit 0 (raw/build.log).
Python rows run as: python3 -B -m unittest <module>.<Class>.<test> from tests/native/r19-accept-support (accept_test, acceptance_cli_test, fault_live_test). Logs: raw/<Class>.<test>.log.
C++ and x08 rows run through ctest -R exact name (raw/ledger_format_init.log, raw/x08_linux_recipe_pins.log).

## Results
| test | invariant | expectRed | outcome | failing line |
|---|---|---|---|---|
| replacement.ledger format initializes exact metadata schema and Glaze event | C3-R1a2 | no | passed | ctest passed |
| same test | C3-R1b | yes | passed (NOT RED) | passes at red time; no new failing assertion |
| x08.linux_recipe_pins | C3-R1f,R1g,R1h,R1i | yes | failed-assertion (single ctest; only first check reported) | "no RUN asserts `test -s fixtures/ledger/expected-schema.sql`" (check_runner); schema_assertion and dockerignore sub-checks not individually distinguishable in this single test |
| HeavyDriverTest.test_run_heavy_exits_nonzero_when_the_scenario_stopped | C3-R1l | yes | failed-assertion | accept_test.py:1112 `0 == 0 : a stopped scenario must exit nonzero` |
| HeavyDriverTest.test_run_heavy_exits_zero_and_writes_result_for_a_completed_scenario | C3-R1m | no | passed | |
| HeavyDriverTest.test_run_heavy_config_overlays_the_fixture_and_ledger_cmake | C3-R1n | yes | failed-assertion | accept_test.py:1131 `None != [overlay list]` |
| HeavyDriverTest.test_export_overlay_copies_working_tree_bytes_of_the_named_paths | C3-R1o | yes | failed-assertion | accept_test.py:1173 head bytes != working tree bytes |
| HeavyDriverTest.test_export_overlay_refuses_a_path_missing_from_the_working_tree | C3-R1p | yes | failed-assertion | accept_test.py:1184 AcceptError not raised |
| AcceptanceCliTest.test_commit_list_check_fails_naming_a_missing_path | C3-R2a | yes | failed-assertion | acceptance_cli_test.py:819 'commit-list: not implemented' |
| AcceptanceCliTest.test_commit_list_check_accepts_a_list_of_existing_sorted_files | C3-R2a | yes | failed-assertion | acceptance_cli_test.py:808 |
| ..._refuses_unsorted_duplicate_raw_and_bytecode_paths | C3-R2b | yes | failed-assertion | acceptance_cli_test.py:845 |
| ..._refuses_snapshot_original_and_unlisted_superseded_paths | C3-R2b | yes | failed-assertion | acceptance_cli_test.py:867 |
| ..._refuses_files_that_pin_report_hashes | C3-R2c, R2f | yes / no | failed-assertion | acceptance_cli_test.py:882 |
| AcceptanceCliTest rerun-index, limits, packet, index, scan, verify-redaction, suites tests (15 names incl. L20, L21a-e, G4, G6) | guards | no | passed | |
| FaultLiveTest.* (16 named guards H0,H2-H9,R3a) and AcceptanceCliTest.test_live_commands_abort_before_first_action_on_candidate_mismatch | guards | no | failed-assertion at setUp (environmental) | "PLANAR_CANDIDATE must name the D28 candidate binary"; live rows need the D28 candidate, not run here |

## Rows not runnable as tests (RECORD / prober / shell rows)
These are evidence-record or prober rows with no executable test at red time; outcome not-registered:
C3-R1a, R1c, R1k (expectRed), R1q (ignored_context_check.sh absent from support dir listing), R1d, R1e, R1j, H1 (record part), H10, L0, L21f (expectRed), S1, D1, H11, R2d, R2e, R2g (expectRed), R2h, R2i, R2j (expectRed), R3b (expectRed), R3c, A1 (expectRed), A2, A3, G1-G3, G5, G7, G8 (partly x08.linux_recipe_pins), G9 (expectRed), G10, G11.
G4 and G6 test parts passed.

allRed = false: C3-R1b passed at red time, and the expectRed RECORD rows are not-registered.
