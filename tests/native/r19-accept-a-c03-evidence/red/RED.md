# R19-ACCEPT-A-C03 red check (run 2)

Red-time identity (covers tracked diff bytes of src and tests/native plus untracked files under src and tests/native/r19-accept-support):
- HEAD: 91e0df24f2167471e49ec057c7159bce5dff3608
- sha256 candidate.diff: 83a16fdee033fce5a1a792d8d65f4090fba8dacffc2d38013684cacb4daeba19
- sha256 SHA256SUMS: 6061761a9d40d44e0bd60c3dc362d22222aeaf8b9fbf0803d0e27425d28bef20
- Snapshot: snapshot-2/ (candidate.diff, untracked/ 21 files, SHA256SUMS). Earlier run kept as snapshot/ and RED-1.md. Raw logs: raw-2/.

Build: cmake --build build/rewrite-debug --target planar_tests exit 0 (raw-2/build.log).
Python rows: python3 -B -m unittest <module>.<Class>.<test> from tests/native/r19-accept-support. C++/x08 rows: ctest -R exact name.

| test | invariant | expectRed | outcome | failing line |
|---|---|---|---|---|
| replacement.ledger format initializes exact metadata schema and Glaze event | C3-R1a2 | no | passed | |
| same test | C3-R1b | yes | passed (NOT RED) | ctest #612 passes, no failing assertion |
| x08.linux_recipe_pins | C3-R1f,g,h,i | yes | failed-assertion (only first check reported) | "no RUN asserts `test -s fixtures/ledger/expected-schema.sql`" |
| HeavyDriverTest.test_run_heavy_exits_nonzero_when_the_scenario_stopped | C3-R1l | yes | failed-assertion | 0 == 0 : a stopped scenario must exit nonzero |
| HeavyDriverTest.test_run_heavy_exits_zero_and_writes_result_for_a_completed_scenario | C3-R1m | no | passed | |
| HeavyDriverTest.test_run_heavy_config_overlays_the_fixture_and_ledger_cmake | C3-R1n | yes | failed-assertion | None != [overlay list] |
| HeavyDriverTest.test_export_overlay_copies_working_tree_bytes_of_the_named_paths | C3-R1o | yes | failed-assertion | head bytes != working tree bytes |
| HeavyDriverTest.test_export_overlay_refuses_a_path_missing_from_the_working_tree | C3-R1p | yes | failed-assertion | AcceptError not raised |
| AcceptanceCliTest commit_list_check fails_naming_a_missing_path | C3-R2a | yes | failed-assertion | 'b/gone.txt' not found in 'commit-list: not implemented' |
| AcceptanceCliTest commit_list_check accepts_a_list_of_existing_sorted_files | C3-R2a | yes | failed-assertion | 1 != 0 (not implemented) |
| AcceptanceCliTest commit_list_check refuses_unsorted_duplicate_raw_and_bytecode_paths | C3-R2b | yes | failed-assertion | 'unsorted' / 'duplicate' not in stderr |
| AcceptanceCliTest commit_list_check refuses_snapshot_original_and_unlisted_superseded_paths | C3-R2b | yes | failed-assertion | 'snapshot' / 'original' not in stderr |
| AcceptanceCliTest commit_list_check refuses_files_that_pin_report_hashes | C3-R2c | yes | failed-assertion | 'a/pins.txt' not in stderr |
| RECORD rows (R1a,c,d,e,j,k,q; R2d-j; R3b,c; A1-3; G*, L0, L21f, S1, D1, H10, H11), live H0-H9 and R3a, L20/L21 guards | various | mixed | not-registered (RECORD rows are post-fix evidence steps; live rows need the D28 candidate; L guards not rerun) | |
