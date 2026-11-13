# RED.md R19-ACCEPT-B-C01 red check

Red-time identity (covers tracked diff and untracked bytes under src and tests/native/*-support):
- HEAD: 080f1666ca5c095b7c02fb4526dca10e95937262
- sha256 candidate.diff: 6e18e0b859f9fe338ccf906667ac61abe1bed7cda4eba5dfc00324af4cc9f9f9
- sha256 SHA256SUMS: a616d283aa7e37927a5d00b53fcd2d456b7a883684086da02aac0b4fe0c02d7b
- snapshot: tests/native/r19-accept-b-c01-evidence/red/snapshot/ (SHA256SUMS lists candidate.diff and each untracked file; the checkpoint/provenance support files are tracked-modified or untracked per git status, untracked ones copied under untracked/)

Build: cmake --build build/rewrite-debug --target planar_tests exit 0 (raw/build.log).

Commands per test: ctest --test-dir build/rewrite-debug -R '<name>' --no-tests=error --output-on-failure (exit 8, No tests were found: the rows are Python unittest, not ctest-registered; raw/<name>.ctest.log), and
cd tests/native/r19-accept-support && HOME=<scratch> PLANAR_DB=<scratch>/p.db PLANAR_CANDIDATE=build/rewrite-debug/bin/planar python3 -B -m unittest <module>.<Class>.<test> (raw/<name>.log). The unittest run is the outcome of record.

| test | expectRed | ctest | outcome | failing line |
|---|---|---|---|---|
| ProvenanceDocTest.test_verify_readback_accepts_exact_expansion | False | not-registered | passed |   |
| ProvenanceLiveTest.test_history_readback_shows_native_derived_block | False | not-registered | failed-assertion | provenance_test.py:793 provenance run did not complete: orchestrator capture exited nonzero |
| ProvenanceLiveTest.test_takeover_stores_its_own_provenance_record | False | not-registered | failed-assertion | provenance_test.py:805 provenance run did not complete: orchestrator capture exited nonzero |
| ProvenanceLiveTest.test_token_never_appears_in_provenance_files_or_readbacks | False | not-registered | failed-assertion | provenance_test.py:833 provenance run did not complete: orchestrator capture exited nonzero |
| ProvenanceLiveTest.test_takeover_without_provenance_reads_back_all_unknown_driver | False | not-registered | failed-assertion | provenance_test.py:875 provenance run did not complete: orchestrator capture exited nonzero |
| ProvenanceCliTest.test_provenance_command_writes_result_json_with_both_readbacks | True | not-registered | failed-assertion | provenance_test.py:1007 the candidate record could not be made from the checkout HEAD |
| ProvenanceManifestTest.test_readback_manifest_has_exact_keys_and_values_from_result | True | not-registered | failed-assertion | provenance_test.py:1128 readback_manifest does not carry exactly the keys version, facts, claim, takeover, nofile, |
| ProvenanceManifestTest.test_readback_manifest_holds_no_path_argv_or_token | True | not-registered | failed-assertion | provenance_test.py:1156 readback_manifest does not carry exactly the keys version, facts, claim, takeover, nofile, |
| ProvenanceManifestTest.test_readback_manifest_recomputes_each_check_from_the_blocks | True | not-registered | failed-assertion | provenance_test.py:1188 readback_manifest checks differ from the recomputed per-block checks |
| ProvenanceManifestTest.test_scan_for_tokens_finds_a_token_in_any_listed_file | True | not-registered | failed-assertion | provenance_test.py:1203 scan_for_tokens did not report the token in a listed file |
| ProvenanceLiveTest.test_live_fixture_runs_with_verification_off_and_reports_it | True | not-registered | failed-assertion | provenance_test.py:912 provenance run did not complete: orchestrator capture exited nonzero |
| CheckpointDocTest.test_build_checkpoint_and_observations_have_exact_shape | True | not-registered | failed-assertion | checkpoint_test.py:371 build_checkpoint does not return exactly progress, next, unresolved and files, with next_s |
| CheckpointDocTest.test_validate_checkpoint_refuses_each_violation_alone_and_accepts_bounds | True | not-registered | failed-assertion | checkpoint_test.py:417 progress 2049 bytes: refusal lacks an expected clause; next with NUL: refusal lacks an exp |
| CheckpointDocTest.test_verify_effect_accepts_a_render_only_change | True | not-registered | failed-assertion | checkpoint_test.py:435 render-only change with since was refused: not implemented; render-only change without sin |
| CheckpointDocTest.test_verify_effect_refuses_each_clause_alone | True | not-registered | failed-assertion | checkpoint_test.py:472 work_fp moved: refusal lacks an expected clause; state changed: refusal lacks an expected  |
| CheckpointDocTest.test_checkpoint_manifest_is_path_free_and_recomputes_its_checks | True | not-registered | failed-assertion | checkpoint_test.py:495 the manifest does not carry exactly the keys version, facts, claim, checkpoint1, checkpoin |
| CheckpointLiveTest.test_checkpoint_argv_passes_observations_file_holding_the_honest_checkpoint | True | not-registered | failed-assertion | checkpoint_test.py:600 checkpoint run did not complete: not implemented |
| CheckpointLiveTest.test_checkpoint_event_is_recorded_and_read_back_by_history | True | not-registered | failed-assertion | checkpoint_test.py:629 checkpoint run did not complete: not implemented |
| CheckpointLiveTest.test_checkpoint_moves_only_the_render_digest | True | not-registered | failed-assertion | checkpoint_test.py:655 checkpoint run did not complete: not implemented |
| CheckpointLiveTest.test_preview_since_reports_a_render_only_change | True | not-registered | failed-assertion | checkpoint_test.py:680 checkpoint run did not complete: not implemented |
| CheckpointLiveTest.test_second_checkpoint_with_different_text_moves_render_again | True | not-registered | failed-assertion | checkpoint_test.py:712 checkpoint run did not complete: not implemented |
| CheckpointLiveTest.test_unchanged_state_gives_byte_equal_previews | True | not-registered | failed-assertion | checkpoint_test.py:736 checkpoint run did not complete: not implemented |
| CheckpointLiveTest.test_run_uses_scratch_home_and_leaves_real_home_untouched | True | not-registered | failed-assertion | checkpoint_test.py:752 checkpoint run did not complete: not implemented |
| CheckpointLiveTest.test_run_token_never_appears_in_result_or_manifest_files | True | not-registered | failed-assertion | checkpoint_test.py:774 checkpoint run did not complete: not implemented |
| CheckpointLiveRefusalTest.test_run_checkpoint_aborts_as_product_defect_when_checkpoint_is_refused | True | not-registered | failed-assertion | checkpoint_test.py:825 the refused checkpoint is not reported as a product defect naming checkpoint |
| CheckpointCliTest.test_checkpoint_command_aborts_before_first_action_on_candidate_mismatch | True | not-registered | failed-assertion | checkpoint_test.py:870 checkpoint did not report the recorded-candidate mismatch (exit 2) |
| CheckpointCliTest.test_checkpoint_command_writes_result_and_path_free_readback | True | not-registered | failed-assertion | checkpoint_test.py:881 the candidate record could not be made from the checkout HEAD |
| CheckpointLiveTest.test_live_fixture_reports_verification_off | True | not-registered | failed-assertion | checkpoint_test.py:798 checkpoint run did not complete: not implemented |
| CheckpointDocTest.test_verify_effect_classifies_budget_omission_as_fixture_limit | True | not-registered | failed-assertion | checkpoint_test.py:489 budget omission: refusal lacks an expected clause; absence without explanation: refusal la |
| ProvenanceDocTest.test_build_document_has_every_key_and_leaves_model_and_session_unknown | False | not-registered | passed |   |
| ProvenanceDocTest.test_validate_document_refuses_each_leaf_rule_violation_alone | False | not-registered | passed |   |
| ProvenanceDocTest.test_validate_document_accepts_one_and_1024_byte_leaves_and_unknown_object | False | not-registered | passed |   |
| ProvenanceDocTest.test_normalize_document_sorts_settings_by_unsigned_name_bytes | False | not-registered | passed |   |
| ProvenanceDocTest.test_verify_readback_refuses_wrong_authority | False | not-registered | passed |   |
| ProvenanceDocTest.test_verify_readback_refuses_document_differing_in_one_leaf | False | not-registered | passed |   |
| ProvenanceDocTest.test_verify_readback_refuses_missing_native_key_and_known_preferences | False | not-registered | passed |   |
| ProvenanceDocTest.test_verify_readback_refuses_bearer_token_anywhere | False | not-registered | passed |   |
| ProvenanceDocTest.test_refused_claim_is_reported_as_product_defect | False | not-registered | passed |   |
| ProvenanceLiveTest.test_claim_argv_passes_provenance_file_holding_the_honest_document | False | not-registered | failed-assertion | provenance_test.py:731 provenance run did not complete: orchestrator capture exited nonzero |
| ProvenanceLiveTest.test_history_readback_shows_supplied_leaves_and_unknown_leaves | False | not-registered | failed-assertion | provenance_test.py:764 provenance run did not complete: orchestrator capture exited nonzero |
| ProvenanceLiveTest.test_run_uses_scratch_home_and_leaves_real_home_untouched | False | not-registered | failed-assertion | provenance_test.py:853 provenance run did not complete: orchestrator capture exited nonzero |
| ProvenanceLiveRefusalTest.test_run_provenance_aborts_as_product_defect_when_claim_is_refused | False | not-registered | failed-assertion | provenance_test.py:948 the refused claim is not reported as a product defect naming claim |
| ProvenanceCliTest.test_provenance_command_aborts_before_first_action_on_candidate_mismatch | False | not-registered | passed |   |
| ProvenanceDocTest.test_verify_absent_readback_accepts_none_driver_and_refuses_stored_record_or_known_leaf | False | not-registered | passed |   |
| ProvenanceLiveTest.test_documents_carry_recomputed_driver_version_and_candidate_size | False | not-registered | failed-assertion | provenance_test.py:895 provenance run did not complete: orchestrator capture exited nonzero |

Limits:
- Provenance live guards (expectRed=false) fail in this run with 'orchestrator capture exited nonzero' and the two ProvenanceCliTest/CheckpointCliTest writes rows fail at 'the candidate record could not be made from the checkout HEAD'; the candidate named is build/rewrite-debug/bin/planar, which accept_lib refuses (same limit as R19-ACCEPT-B red). So ProvenanceCliTest.test_provenance_command_writes_result_json_with_both_readbacks and ProvenanceLiveTest.test_live_fixture_runs_with_verification_off_and_reports_it are red partly for environment, not purely missing behaviour. Checkpoint doc/live rows fail with 'not implemented' or exact-shape assertions (genuine).
- ProvenanceCliTest.test_provenance_command_aborts_before_first_action_on_candidate_mismatch passes (guard).
- Rows tagged n/a or process checks (X8, M7, C22, P1-P25) have no test to run.
