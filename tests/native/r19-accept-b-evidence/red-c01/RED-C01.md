# R19-ACCEPT-B-C01 red capture (RED package, wave 2)

Committed record (E18, E19). Taken on the live support tree before any wave-3 implementation. Raw logs are local in `red-c01/raw/` (not committed); the pre-run snapshot is `red-c01/snapshot/` (local) and `red-c01/snapshot-hashes.txt` (local).

## Snapshot

- Identity of the live tree (pathspec `tests/native/r19-accept-support`), pasted verbatim in `red-c01/snapshot-hashes.txt`: combined sha256 `d8858b2f55abfd7b1ccd1376f93c087462548003e075bc9424e43aadcbe099c9`. This is the `snapshot_sha256` given to `accept_lib.validate_red_record`.
- Copy then proof: `diff -r tests/native/r19-accept-support tests/native/r19-accept-b-evidence/red-c01/snapshot` exit 0 before the runs, and exit 0 again after all 45 runs (the live tree did not change during the capture).
- identity.py was never run on an evidence path.

## Command

One process per test id, 45 ids (`provenance_test` 27, `checkpoint_test` 18), each from `tests/native/r19-accept-support` (host paths shown as placeholders):

    HOME=<scratch>/home PLANAR_DB=<scratch>/db/planar.db PLANAR_CANDIDATE=<recorded candidate> PYTHONDONTWRITEBYTECODE=1 python3 -B -m unittest -v <module>.<Class>.<test>

- Recorded candidate: the R19-ACCEPT-A C01 binary reused under the E3 deviation (`tests/native/r19-accept-evidence/gate/candidate/candidate.json`, head `91e0df24`, sha256 `774bcd69809ead9e4cd7b6e06c74aa045ea9272a2321cd36f91bc9c76a99ab0c`, size 33982376). It is not under `build/rewrite-*`.
- Driver and checks (local, `red-c01/raw/tools/`): `run_c01.py` (one process per id), `validate_c01.py` (validator on the logs), `validate_original.py` (read-only validator run on the original red logs), `render_red_c01.py` (this table).
- The runs were not piped; each log's first line is the command and the second line the exit code.

## Counts

- Tests run: 45 (one per process; `Ran 1 test` in every log). Skipped: 0. ERROR markers: 0. `COMPILE_OR_IMPORT` matches (accept_lib regex): 0 across all 45 logs.
- Outcomes: failed-assertion 25, passed 20, other 0.
- The 24 expectRed tests (18 of 18 `checkpoint_test`, the strengthened `ProvenanceCliTest.test_provenance_command_writes_result_json_with_both_readbacks`, `ProvenanceLiveTest.test_live_fixture_runs_with_verification_off_and_reports_it` (V1), 4 `ProvenanceManifestTest`): all 24 failed by assertion; `accept_lib.validate_red_record` ACCEPTS 24 of 24.
- Guard tests (expectRed false): 21 `provenance_test` tests; 20 pass; 1 fails unexpectedly (X6, below).

## Unexpected result for the coordinator

`ProvenanceLiveTest.test_token_never_appears_in_provenance_files_or_readbacks` (row X6, expectRed false) FAILS on its final assertion `the live result does not report token_absent true`. The token checks before it (provenance files, retained history stdout, result) pass on the current code. The failure comes from the new `token_absent` result key that I1 provides, so the row's `expectRed false` claim does not hold for this red. It is an assertion failure, not an ERROR or an import failure, so the capture did not stop. Decision needed: mark X6 expectRed true with this reason, or move the `token_absent` assertion out of the red-phase test. No test file was edited by this package.

## Limits recorded for the prober

- The strengthened CLI test (M1, M2) stops at its first failing assertion (`verified` is not true). The M2 readback assertions are not reached in this red and are for the prober.
- Live checkpoint and live provenance tests that fail in this red fail on the setUpClass message `run did not complete: not implemented`. That is an assertion failure against the stubs, not a mutant kill; the prober must confirm each named kill (E26).
- C22 and M7 are limit rows with no test.

## Outcome per test

Excerpts are the first `AssertionError` message of the log, truncated to 300 bytes. `row` is the INVARIANTS.md row id(s); `expectRed` is the INVARIANTS.md value (X6 is shown as `false (X6)`, see above).

| # | test (module.Class.method) | row | expectRed | exit | outcome | validate_red_record | excerpt |
|---|---|---|---|---|---|---|---|
| 1 | `checkpoint_test.CheckpointCliTest.test_checkpoint_command_aborts_before_first_action_on_candidate_mismatch` | C20 | true | 1 | failed-assertion | ACCEPT | checkpoint did not report the recorded-candidate mismatch (exit 2) |
| 2 | `checkpoint_test.CheckpointCliTest.test_checkpoint_command_writes_result_and_path_free_readback` | C21 | true | 1 | failed-assertion | ACCEPT | checkpoint exited 2, expected 0 |
| 3 | `checkpoint_test.CheckpointDocTest.test_build_checkpoint_and_observations_have_exact_shape` | C1 | true | 1 | failed-assertion | ACCEPT | build_checkpoint does not return exactly progress, next, unresolved and files, with next_step mapped to next |
| 4 | `checkpoint_test.CheckpointDocTest.test_checkpoint_manifest_is_path_free_and_recomputes_its_checks` | C9 | true | 1 | failed-assertion | ACCEPT | the manifest does not carry exactly the keys version, facts, claim, checkpoint1, checkpoint2, previews, since, informational and checks |
| 5 | `checkpoint_test.CheckpointDocTest.test_validate_checkpoint_refuses_each_violation_alone_and_accepts_bounds` | C2 | true | 1 | failed-assertion | ACCEPT | progress 2049 bytes: refusal lacks an expected clause; next with NUL: refusal lacks an expected clause; unresolved missing: refusal lacks an expected clause; next missing: refusal lacks an expected clause; unknown key: refusal lacks an expected clause; progress not a string: refusal lacks an expecte ... |
| 6 | `checkpoint_test.CheckpointDocTest.test_verify_effect_accepts_a_render_only_change` | C3 | true | 1 | failed-assertion | ACCEPT | render-only change with since was refused: not implemented; render-only change without since was refused: not implemented |
| 7 | `checkpoint_test.CheckpointDocTest.test_verify_effect_classifies_budget_omission_as_fixture_limit` | C24 | true | 1 | failed-assertion | ACCEPT | budget omission: refusal lacks an expected clause; absence without explanation: refusal lacks an expected clause |
| 8 | `checkpoint_test.CheckpointDocTest.test_verify_effect_refuses_each_clause_alone` | C4, C5, C6, C7, C8 | true | 1 | failed-assertion | ACCEPT | work_fp moved: refusal lacks an expected clause; state changed: refusal lacks an expected clause; render digest unchanged: refusal lacks an expected clause; no checkpoint section: refusal lacks an expected clause; since reports material changes: refusal lacks an expected clause; since without a chec ... |
| 9 | `checkpoint_test.CheckpointLiveRefusalTest.test_run_checkpoint_aborts_as_product_defect_when_checkpoint_is_refused` | C19 | true | 1 | failed-assertion | ACCEPT | the refused checkpoint is not reported as a product defect naming checkpoint |
| 10 | `checkpoint_test.CheckpointLiveTest.test_checkpoint_argv_passes_observations_file_holding_the_honest_checkpoint` | C10 | true | 1 | failed-assertion | ACCEPT | checkpoint run did not complete: not implemented |
| 11 | `checkpoint_test.CheckpointLiveTest.test_checkpoint_event_is_recorded_and_read_back_by_history` | C11 | true | 1 | failed-assertion | ACCEPT | checkpoint run did not complete: not implemented |
| 12 | `checkpoint_test.CheckpointLiveTest.test_checkpoint_moves_only_the_render_digest` | C12, C13 | true | 1 | failed-assertion | ACCEPT | checkpoint run did not complete: not implemented |
| 13 | `checkpoint_test.CheckpointLiveTest.test_live_fixture_reports_verification_off` | C23 | true | 1 | failed-assertion | ACCEPT | checkpoint run did not complete: not implemented |
| 14 | `checkpoint_test.CheckpointLiveTest.test_preview_since_reports_a_render_only_change` | C14 | true | 1 | failed-assertion | ACCEPT | checkpoint run did not complete: not implemented |
| 15 | `checkpoint_test.CheckpointLiveTest.test_run_token_never_appears_in_result_or_manifest_files` | C18 | true | 1 | failed-assertion | ACCEPT | checkpoint run did not complete: not implemented |
| 16 | `checkpoint_test.CheckpointLiveTest.test_run_uses_scratch_home_and_leaves_real_home_untouched` | C17 | true | 1 | failed-assertion | ACCEPT | checkpoint run did not complete: not implemented |
| 17 | `checkpoint_test.CheckpointLiveTest.test_second_checkpoint_with_different_text_moves_render_again` | C15 | true | 1 | failed-assertion | ACCEPT | checkpoint run did not complete: not implemented |
| 18 | `checkpoint_test.CheckpointLiveTest.test_unchanged_state_gives_byte_equal_previews` | C16 | true | 1 | failed-assertion | ACCEPT | checkpoint run did not complete: not implemented |
| 19 | `provenance_test.ProvenanceCliTest.test_provenance_command_aborts_before_first_action_on_candidate_mismatch` | I18 | false | 0 | passed | - | - |
| 20 | `provenance_test.ProvenanceCliTest.test_provenance_command_writes_result_json_with_both_readbacks` | M1, M2, I19 | true | 1 | failed-assertion | ACCEPT | the result does not report in-run verification on (verified is not true) |
| 21 | `provenance_test.ProvenanceDocTest.test_build_document_has_every_key_and_leaves_model_and_session_unknown` | I1 | false | 0 | passed | - | - |
| 22 | `provenance_test.ProvenanceDocTest.test_normalize_document_sorts_settings_by_unsigned_name_bytes` | I4 | false | 0 | passed | - | - |
| 23 | `provenance_test.ProvenanceDocTest.test_refused_claim_is_reported_as_product_defect` | I10 | false | 0 | passed | - | - |
| 24 | `provenance_test.ProvenanceDocTest.test_validate_document_accepts_one_and_1024_byte_leaves_and_unknown_object` | I3 | false | 0 | passed | - | - |
| 25 | `provenance_test.ProvenanceDocTest.test_validate_document_refuses_each_leaf_rule_violation_alone` | I2 | false | 0 | passed | - | - |
| 26 | `provenance_test.ProvenanceDocTest.test_verify_absent_readback_accepts_none_driver_and_refuses_stored_record_or_known_leaf` | I32 | false | 0 | passed | - | - |
| 27 | `provenance_test.ProvenanceDocTest.test_verify_readback_accepts_exact_expansion` | I9, X1, X2 | false | 0 | passed | - | - |
| 28 | `provenance_test.ProvenanceDocTest.test_verify_readback_refuses_bearer_token_anywhere` | I8 | false | 0 | passed | - | - |
| 29 | `provenance_test.ProvenanceDocTest.test_verify_readback_refuses_document_differing_in_one_leaf` | I6 | false | 0 | passed | - | - |
| 30 | `provenance_test.ProvenanceDocTest.test_verify_readback_refuses_missing_native_key_and_known_preferences` | I7 | false | 0 | passed | - | - |
| 31 | `provenance_test.ProvenanceDocTest.test_verify_readback_refuses_wrong_authority` | I5 | false | 0 | passed | - | - |
| 32 | `provenance_test.ProvenanceLiveRefusalTest.test_run_provenance_aborts_as_product_defect_when_claim_is_refused` | I17 | false | 0 | passed | - | - |
| 33 | `provenance_test.ProvenanceLiveTest.test_claim_argv_passes_provenance_file_holding_the_honest_document` | I11 | false | 0 | passed | - | - |
| 34 | `provenance_test.ProvenanceLiveTest.test_documents_carry_recomputed_driver_version_and_candidate_size` | I41 | false | 0 | passed | - | - |
| 35 | `provenance_test.ProvenanceLiveTest.test_history_readback_shows_native_derived_block` | I13, X3 | false | 0 | passed | - | - |
| 36 | `provenance_test.ProvenanceLiveTest.test_history_readback_shows_supplied_leaves_and_unknown_leaves` | I12 | false | 0 | passed | - | - |
| 37 | `provenance_test.ProvenanceLiveTest.test_live_fixture_runs_with_verification_off_and_reports_it` | V1 | true | 1 | failed-assertion | ACCEPT | the live fixture result does not report verification off (verified is not false) |
| 38 | `provenance_test.ProvenanceLiveTest.test_run_uses_scratch_home_and_leaves_real_home_untouched` | I16 | false | 0 | passed | - | - |
| 39 | `provenance_test.ProvenanceLiveTest.test_takeover_stores_its_own_provenance_record` | I14, X4, X5 | false | 0 | passed | - | - |
| 40 | `provenance_test.ProvenanceLiveTest.test_takeover_without_provenance_reads_back_all_unknown_driver` | I31, X7 | false | 0 | passed | - | - |
| 41 | `provenance_test.ProvenanceLiveTest.test_token_never_appears_in_provenance_files_or_readbacks` | I15, X6 | false (X6, fails today) | 1 | failed-assertion | - | the live result does not report token_absent true |
| 42 | `provenance_test.ProvenanceManifestTest.test_readback_manifest_has_exact_keys_and_values_from_result` | M3 | true | 1 | failed-assertion | ACCEPT | readback_manifest does not carry exactly the keys version, facts, claim, takeover, nofile, checks |
| 43 | `provenance_test.ProvenanceManifestTest.test_readback_manifest_holds_no_path_argv_or_token` | M4 | true | 1 | failed-assertion | ACCEPT | readback_manifest does not carry exactly the keys version, facts, claim, takeover, nofile, checks |
| 44 | `provenance_test.ProvenanceManifestTest.test_readback_manifest_recomputes_each_check_from_the_blocks` | M5 | true | 1 | failed-assertion | ACCEPT | readback_manifest checks differ from the recomputed per-block checks |
| 45 | `provenance_test.ProvenanceManifestTest.test_scan_for_tokens_finds_a_token_in_any_listed_file` | M6 | true | 1 | failed-assertion | ACCEPT | scan_for_tokens did not report the token in a listed file |

## X rows (E19: strengthened existing tests)

X1 and X2 (`test_verify_readback_accepts_exact_expansion`), X3 (`test_history_readback_shows_native_derived_block`), X4 and X5 (`test_takeover_stores_its_own_provenance_record`) and X7 (`test_takeover_without_provenance_reads_back_all_unknown_driver`) pass on the current code, as expectRed false requires. X6 fails as described above. Their kill strength is the prober's job (named-assertion kills, RUN and PROBE packages).

