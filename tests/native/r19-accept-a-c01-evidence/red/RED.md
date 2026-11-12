# R19-ACCEPT-A-C01 red check

Red-time identity (covers tracked and untracked bytes):
- HEAD: 91e0df24f2167471e49ec057c7159bce5dff3608
- sha256 candidate.diff (git diff HEAD -- src tests/native): b5b9a3fb357c2ded89b7deb8ff5ba5550dc5d1fd359c22ba2e28099ca1bf4ec4
- sha256 SHA256SUMS (candidate.diff + 20 untracked files under tests/native/r19-accept-support): 8a5cdd7be032c1fccc92c9bc94f30601bb7f1af3fad9cbfd4c3171a4856c9e0e
- Snapshot: red/snapshot/ (candidate.diff, untracked/, SHA256SUMS). No untracked file under src existed.

Build: `cmake --build build/rewrite-debug --target planar_tests` exit 0 (raw/build.log).

Commands. C++ tests: `ctest --test-dir build/rewrite-debug -R '<escaped name>' --no-tests=error --output-on-failure`.
Python tests are not ctest-registered; run as `cd tests/native/r19-accept-support && python3 -m unittest -v <module>.<Class>.<test>`.
Pass 1 (raw/, outcomes.tsv) had PLANAR_CANDIDATE unset. Pass 2 (raw2/, outcomes2.tsv) set it to
build/rewrite-debug/bin/planar for live tests only. That binary is the debug build, not the recorded D28 candidate,
and its `orchestrator` command answers usage error exit 2, so live rows cannot reach their behaviour here.

## expectRed rows
| test | inv | outcome | failing line |
|---|---|---|---|
| history event ... integer nodes free of the boolean flag | H1 | failed-assertion | execution.t.cpp:4002, 4004, 4101, 4105 |
| history event ... boolean nodes free of the integer value | H2 | failed-assertion | execution.t.cpp:4125 |
| AcceptanceCliTest suites_reports_per_module_and_total_counts | S1 | failed-assertion | acceptance_cli_test.py:443 "suites not implemented" |
| ...suites_refuses_skips_failures_and_zero_matched | S2 | failed-assertion | exit 2 "suites not implemented" |
| ...suites_live_modules_require_the_recorded_candidate | S3 | failed-assertion | exit 2 "suites not implemented" |
| ...rerun_index_records_every_required_fault_field | R1 | failed-assertion | acceptance_cli_test.py:520 "rerun-index not implemented" |
| ...rerun_index_fails_when_any_required_field_is_missing | R2 | failed-assertion | exit 2 "rerun-index not implemented" |
| FaultLiveTest final_approval_after_faults_is_exact_target | L13 | pass1 failed-assertion (setUp: PLANAR_CANDIDATE unset, vacuous); pass2 failed-crash | fault_driver.py:445 AcceptError orchestrator capture exited nonzero |
| FaultLiveTest first_completed_check_passes_boundary_rows | L16 | same as L13 | same |
| FaultLiveTest scenario_driver_is_config_parameterized | L17 | same as L13 | same |
| FaultLiveTest run_scenario_aborts_on_tampered_replay | L18 | pass1 failed-assertion (setUp, vacuous); pass2 PASSED | none |
| QuotaLiveTest run_grouping_aborts_before_action_not_listed_as_legal | L19 | pass1 failed-assertion (setUp, vacuous); pass2 failed-crash | quota_driver CandidateFault: orchestrator usage error exit 2 |

allRed = false: L13, L16, L17, L19 have no behavioural red (setUp env or candidate crash), L18 passed in pass 2.

## Guard rows (expectRed=false), pass 1
Passed (exit 0): H3, H5 (X08 C01), H6/RF1/RF2 (test_red_record_must_be_behavioural_and_hashed), L23, all AcceptLibGapTest rows
(M-I3b, I10*, I12*, I13b, I17c, I24*, I29b, I30c, I35b, E13, E14, E15, E17, E18), M-E6, M-E8/E10/E11 (verify_redaction), L21 (limits).
M-A6 (test_live_commands_abort_before_first_action_on_candidate_mismatch): pass1 failed (PLANAR_CANDIDATE unset), pass2 passed.
H4 / Q7 (QuotaLive history_event_and_at_event) and all other Quota/Fault live rows (L1-L12, L14, L15, L24, Q1-Q18, F1-F25 except below):
pass1 failed at setUp (env); pass2 failed with candidate crash (orchestrator usage error / orchestrator capture exited nonzero),
except these, which PASSED in pass 2: QuotaLive run_grouping_aborts_on_tampered_replay,
run_grouping_aborts_when_inspection_changes_ledger, and FaultLive run_scenario_aborts_{on_tampered_replay,when_trajectory_omits_rejection,
when_inspection_changes_ledger,when_counter_increases_on_replay,on_mismatched_event_expansion,on_mismatched_at_event_state,on_unusable_guidance},
run_heavy_config_is_composed_base_and_owned_repairs, kill_check_consumes_marker_written_by_cmake_check, run_heavy_aborts_on_unrecorded_candidate.
Per-test exit codes: outcomes.tsv/outcomes2.tsv with raw logs.

## Not registered
- QuotaLiveTest.test_split_grouping_retains_raw_exact_targets_and_non_builder_actor (L24): AttributeError, method absent (raw/ and raw2/).
- G1 "gate stage comm -23 ...": no registered test or script found; not-registered.
- Names in the brief that duplicate others (L20, L22, RF2) map to the tests above; the FaultLiveTest run_heavy tests are covered by the rows above.

## Limits
Live rows need the real D28 candidate with the orchestrator command; the debug `planar` binary is not it. Logs are retained, none deleted.
run_red.sh and tests.csv in this folder are the runner and test list.
