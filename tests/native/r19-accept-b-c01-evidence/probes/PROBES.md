# R19-ACCEPT-B-C01 mutation probes (focused re-probe after the survivor repair)

Prober: package PROBE, 2026-11-12. The repair changed only `tests/native/r19-accept-support/checkpoint_test.py` (new empty-path and NUL-path cases in the C2 test, nine per-check negative cases in the C9 manifest test, extended `Kills:` text; no test renamed or removed, no driver or product file changed). Mutants were applied only in the persistent scratch copy `probe-warm` (outside the checkout, synced from the checkout with `rsync -a --delete --exclude build/ --exclude .git/ --exclude 'tests/native/*-evidence/'`; the scratch support tree equals the checkout support tree byte for byte, verified with `diff -r`). The checkout and `build/` were not edited. Scratch `HOME` and `PLANAR_DB` were set for every run; candidate `PLANAR_CANDIDATE` is the recorded candidate (`tests/native/r19-accept-evidence/gate/candidate/PATH`, sha256 774bcd69809ead9e4cd7b6e06c74aa045ea9272a2321cd36f91bc9c76a99ab0c, size 33982376, verified).

The previous probe record is kept whole under `probes/superseded-1/` (`PROBES.md` and `raw/`, 87 logs).

## Identity (identity.py output, verbatim, run at the start of this re-probe)

| identity | value |
|---|---|
| HEAD | `080f1666ca5c095b7c02fb4526dca10e95937262` |
| sha256(git diff --binary HEAD -- tests/native/r19-accept-support), tracked only | `2636b446e572892fb3f08087d56376f342579b04a262fd605499fc19bb1e5420` |
| combined sha256 (diff bytes, then each untracked path NUL bytes, sorted), pathspec tests/native/r19-accept-support, evidence excluded | `85fffe4b63e99f27ca2b0fc93c3e21b04e2f794d9e33b22aadb8ee4a44b45cbe` |

| file | state | sha256 |
|---|---|---|
| `tests/native/r19-accept-support/run_acceptance.py` | modified | `2bb38280e24005684fc67fcbbeff48d858471a169323053bc5cd8fc59336946c` |
| `tests/native/r19-accept-support/checkpoint_driver.py` | untracked | `be85f5eff833a1c3a5eab6b815c2f99507342d8caad988c22fa5156acd6cfc62` |
| `tests/native/r19-accept-support/checkpoint_test.py` | untracked | `ef6e7ef047e96bdd8d14d61cea903c2d365acdda3f39e52576e7f4e2779c2bdd` |
| `tests/native/r19-accept-support/provenance_driver.py` | untracked | `9470b069e5480d3065514ce31514a6ec4094460abe77c9d285886b48456d443e` |
| `tests/native/r19-accept-support/provenance_test.py` | untracked | `c6147318550b76c5daf950ec3dc88f9c56237557d6390dd1403b287dbb11bfa1` |

`identity.py src` (verbatim, empty diff equals HEAD):

| identity | value |
|---|---|
| HEAD | `080f1666ca5c095b7c02fb4526dca10e95937262` |
| sha256(git diff --binary HEAD -- src), tracked only | `e3b0c44298fc1c149afbf4c8996fb92427ae41e4649b934ca495991b7852b855` |
| combined sha256 (diff bytes, then each untracked path NUL bytes, sorted), pathspec src, evidence excluded | `e3b0c44298fc1c149afbf4c8996fb92427ae41e4649b934ca495991b7852b855` |

Differences from the superseded record: only `checkpoint_test.py` changed (superseded sha256 `0c7036d6e91d449dd9da0a3191834e512d08815c59bae44479b8a4e46f5fea2e`); the other four support files are byte-identical to it.

## Method

- Python support tree, no compiled target consumes it: no per-mutant build is applicable and the warm CMake build directory was not touched. Baseline in scratch before probing: `provenance_test` 27 of 27 OK, `checkpoint_test` 18 of 18 OK.
- Command per mutant (cwd `tests/native/r19-accept-support` in the scratch copy, scratch HOME/PLANAR_DB, PLANAR_CANDIDATE recorded): `perl -e 'alarm 900; exec @ARGV' python3 -B -m unittest <module> -v` (provenance_test for provenance mutants, checkpoint_test for checkpoint mutants, both for `run_acceptance.py` mutants). `timeout` is not installed; perl alarm is used.
- Each mutant is the unified diff of the superseded raw log applied by exact-text hunk replace that must match exactly once (a miss is recorded as not applied, never as a kill); R1-R5 are new exact-text edits. The mutated file is restored by byte copy and its sha256 compared to the pristine bytes (every raw log records `restored byte copy sha256 equals pristine: True`).
- Raw logs: `probes/raw/<id>.log` (unified diff, diff/pristine/mutated sha256, exact command and exit code, full unittest output, failing-test SUMMARY, named-test verdict). A row is KILLED only when the NAMED test failed by assertion (`FAIL:` line). A `setUpClass` abort does not count.
- Re-run set: the twelve focused rows I13, E1-E5, E8, E9, E15-E18, plus every row whose named test lives in the changed `checkpoint_test.py` (C1-C21b, C23, C24, E6, E7, E14, E19, E20), plus X3 (the named-assertion mutant of I13). 56 rows run (55 in one pass plus X3). R1-R5 are repair-verification mutants beyond the 20-exploratory cap: they target the checks T-1 left unprobed (`state_equal`, `render_moved_by_checkpoint1/2`, `since_material_empty`, `token_absent`).
- Carried forward from `probes/superseded-1/PROBES.md` unchanged (their mutated file and named test are unchanged by the repair): X1, X2, X4-X7, M1-M6, V1 and the provenance original rows I1-I12, I16-I19, I32, I41a, I41b, plus exploratory E10-E13. These ran `provenance_test.py`, which is byte-identical.
- Hash column: first 12 hex of the sha256 of the unified diff (full value in the raw log).

## Invariant rows

| id | invariant | mutant | diff | killed | killing test (named) | also failed | source |
|---|---|---|---|---|---|---|---|
| X1 | I9 | verify_readback compares the supplied document un-normalized | 49ed8bce7b1f | yes | `ProvenanceDocTest.test_verify_readback_accepts_exact_expansion` | - | run | carried from superseded-1 |
| X2 | I9 | verify_readback compares the stored document without normalize_document | f70883ee2189 | yes | `ProvenanceDocTest.test_verify_readback_accepts_exact_expansion` | P.DocTest.test_verify_readback_refuses_bearer_token_anywhere, P.DocTest.test_verify_readback_refuses_document_differing_in_one_leaf, P.DocTest.test_verify_readback_refuses_missing_native_key_and_known_preferences, P.D... | run | carried from superseded-1 |
| X3 | I13 | claim read-back taken from the takeover event | b786c0f223fd | yes | `ProvenanceLiveTest.test_history_readback_shows_native_derived_block` | test_provenance_command_writes_result_json_with_both_readbacks | run (this re-probe) |
| X4 | I14 | takeover argv drops --provenance | bf174e829aa1 | yes | `ProvenanceLiveTest.test_takeover_stores_its_own_provenance_record` | P.CliTest.test_provenance_command_writes_result_json_with_both_readbacks, P.LiveTest.test_claim_argv_passes_provenance_file_holding_the_honest_document, P.LiveTest.test_documents_carry_recomputed_driver_version_and_ca... | run | carried from superseded-1 |
| X5 | I14 | takeover passes the claim file | a8441bc5f4c2 | yes | `ProvenanceLiveTest.test_takeover_stores_its_own_provenance_record` | P.CliTest.test_provenance_command_writes_result_json_with_both_readbacks, P.LiveTest.test_claim_argv_passes_provenance_file_holding_the_honest_document | run | carried from superseded-1 |
| X6 | I15 | takeover document carries the claim run token as a setting | 8c2117670403 | yes | `ProvenanceLiveTest.test_token_never_appears_in_provenance_files_or_readbacks` | P.CliTest.test_provenance_command_writes_result_json_with_both_readbacks | run | carried from superseded-1 |
| X7 | I31 | no-file takeover argv gains --provenance | c9c1bb08c4cf | yes | `ProvenanceLiveTest.test_takeover_without_provenance_reads_back_all_unknown_driver` | P.CliTest.test_provenance_command_writes_result_json_with_both_readbacks, P.LiveTest.test_claim_argv_passes_provenance_file_holding_the_honest_document | run | carried from superseded-1 |
| M1 | M1 | cmd_provenance forces verify False | 4dfd2864f714 | yes | `ProvenanceCliTest.test_provenance_command_writes_result_json_with_both_readbacks` | - | run | carried from superseded-1 |
| M2 | M2 | cmd_provenance skips writing a34-readback.json | b5f5d2e12a38 | yes | `ProvenanceCliTest.test_provenance_command_writes_result_json_with_both_readbacks` | - | run | carried from superseded-1 |
| M3 | M3 | readback_manifest omits the nofile block | 4399d6e50e15 | yes | `ProvenanceManifestTest.test_readback_manifest_has_exact_keys_and_values_from_result` | P.ManifestTest.test_readback_manifest_holds_no_path_argv_or_token | run | carried from superseded-1 |
| M4 | M4 | readback_manifest copies result env into checks | 82b5cee13a81 | yes | `ProvenanceManifestTest.test_readback_manifest_holds_no_path_argv_or_token` | P.CliTest.test_provenance_command_writes_result_json_with_both_readbacks, P.ManifestTest.test_readback_manifest_recomputes_each_check_from_the_blocks | run | carried from superseded-1 |
| M5 | M5 | generation_expected constant True | 2964409beada | yes | `ProvenanceManifestTest.test_readback_manifest_recomputes_each_check_from_the_blocks` | - | run | carried from superseded-1 |
| M6 | M6 | scan_for_tokens returns True without reading files | 5f24928456f3 | yes | `ProvenanceManifestTest.test_scan_for_tokens_finds_a_token_in_any_listed_file` | - | run | carried from superseded-1 |
| V1 | V1 | run result reports verified True constantly | 5198edba37e2 | yes | `ProvenanceLiveTest.test_live_fixture_runs_with_verification_off_and_reports_it` | - | run | carried from superseded-1 |
| C1 | C1 | build_checkpoint swaps progress and next | c939da5d7a40 | yes | `CheckpointDocTest.test_build_checkpoint_and_observations_have_exact_shape` | test_checkpoint_argv_passes_observations_file_holding_the_honest_checkpoint, test_checkpoint_event_is_recorded_and_read_back_by_history, test_second_checkpoint_with_different_text_moves_render_again | run (this re-probe) |
| C2a | C2 | delete the 2048-byte upper bound | 68c21b08517e | yes | `CheckpointDocTest.test_validate_checkpoint_refuses_each_violation_alone_and_accepts_bounds` | - | run (this re-probe) |
| C2b | C2 | refuse empty strings | 398b7f1fc64f | yes | `CheckpointDocTest.test_validate_checkpoint_refuses_each_violation_alone_and_accepts_bounds` | - | run (this re-probe) |
| C2c | C2 | refuse an empty files list | e395df0c9f19 | yes | `CheckpointDocTest.test_validate_checkpoint_refuses_each_violation_alone_and_accepts_bounds` | - | run (this re-probe) |
| C3 | C3 | verify_effect always raises | 993fd121124d | yes | `CheckpointDocTest.test_verify_effect_accepts_a_render_only_change` | test_checkpoint_command_writes_result_and_path_free_readback, test_verify_effect_classifies_budget_omission_as_fixture_limit, test_verify_effect_refuses_each_clause_alone | run (this re-probe) |
| C4 | C4 | delete work_fp comparison | 7716781f077a | yes | `CheckpointDocTest.test_verify_effect_refuses_each_clause_alone` | - | run (this re-probe) |
| C5 | C5 | delete state comparison | 66bdcaba639b | yes | `CheckpointDocTest.test_verify_effect_refuses_each_clause_alone` | - | run (this re-probe) |
| C6 | C6 | accept equal digests | eed0c61f6982 | yes | `CheckpointDocTest.test_verify_effect_refuses_each_clause_alone` | - | run (this re-probe) |
| C7 | C7 | delete the checkpoint-section check | 3cd1d35dda8e | yes | `CheckpointDocTest.test_verify_effect_refuses_each_clause_alone` | test_verify_effect_classifies_budget_omission_as_fixture_limit | run (this re-probe) |
| C8a | C8 | delete the since material check | bf68073c10ba | yes | `CheckpointDocTest.test_verify_effect_refuses_each_clause_alone` | - | run (this re-probe) |
| C8b | C8 | delete the checkpoint-row check | 0195b04212a1 | yes | `CheckpointDocTest.test_verify_effect_refuses_each_clause_alone` | - | run (this re-probe) |
| C8c | C8 | delete the since-digest check | 33cee03969be | yes | `CheckpointDocTest.test_verify_effect_refuses_each_clause_alone` | - | run (this re-probe) |
| C8d | C8 | render_changed as sufficient evidence in place of the row | 0602ddcf6530 | yes | `CheckpointDocTest.test_verify_effect_refuses_each_clause_alone` | - | run (this re-probe) |
| C9a | C9 | copy result env into checks | f1a4cc5537db | yes | `CheckpointDocTest.test_checkpoint_manifest_is_path_free_and_recomputes_its_checks` | test_checkpoint_command_writes_result_and_path_free_readback | run (this re-probe) |
| C9b | C9 | work_fp_equal constant True | a6d5b4025ce1 | yes | `CheckpointDocTest.test_checkpoint_manifest_is_path_free_and_recomputes_its_checks` | - | run (this re-probe) |
| C9c | C9 | add render_changed to checks | 9e9ac6ea0017 | yes | `CheckpointDocTest.test_checkpoint_manifest_is_path_free_and_recomputes_its_checks` | test_checkpoint_command_writes_result_and_path_free_readback | run (this re-probe) |
| C9d | C9 | move the budget flag into checks | 4ca1be5e8f10 | yes | `CheckpointDocTest.test_checkpoint_manifest_is_path_free_and_recomputes_its_checks` | test_checkpoint_command_writes_result_and_path_free_readback | run (this re-probe) |
| C10a | C10 | checkpoint #1 file holds different valid text T2 | ab08b039f0dd | yes | `CheckpointLiveTest.test_checkpoint_argv_passes_observations_file_holding_the_honest_checkpoint` | test_checkpoint_command_writes_result_and_path_free_readback, test_checkpoint_event_is_recorded_and_read_back_by_history, test_second_checkpoint_with_different_text_moves_render_again | run (this re-probe) |
| C10b | C10 | checkpoint #1 file holds an empty files list | 374dbdb32885 | yes | `CheckpointLiveTest.test_checkpoint_argv_passes_observations_file_holding_the_honest_checkpoint` | test_checkpoint_command_writes_result_and_path_free_readback | run (this re-probe) |
| C11 | C11 | event_id from checkpoint.id | 2a1197b43f57 | yes | `CheckpointLiveTest.test_checkpoint_event_is_recorded_and_read_back_by_history` | test_checkpoint_command_writes_result_and_path_free_readback, test_second_checkpoint_with_different_text_moves_render_again | run (this re-probe) |
| C12 | C12 | before snapshot taken after checkpoint #1 | 63b5efe47bd1 | yes | `CheckpointLiveTest.test_checkpoint_moves_only_the_render_digest` | test_checkpoint_command_writes_result_and_path_free_readback | run (this re-probe) |
| C13 | C13 | after1 preview taken before checkpoint #1 | f845359b4df7 | yes | `CheckpointLiveTest.test_checkpoint_moves_only_the_render_digest` | test_checkpoint_command_writes_result_and_path_free_readback, test_second_checkpoint_with_different_text_moves_render_again | run (this re-probe) |
| C14 | C14 | since read before checkpoint #2 | 368da48ce83b | yes | `CheckpointLiveTest.test_preview_since_reports_a_render_only_change` | test_checkpoint_command_writes_result_and_path_free_readback | run (this re-probe) |
| C15 | C15 | checkpoint #2 reuses text one | 7859eb023c14 | yes | `CheckpointLiveTest.test_second_checkpoint_with_different_text_moves_render_again` | test_checkpoint_event_is_recorded_and_read_back_by_history | run (this re-probe) |
| C16 | C16 | control taken after a third checkpoint | 5b2255cd0003 | yes | `CheckpointLiveTest.test_unchanged_state_gives_byte_equal_previews` | test_checkpoint_command_writes_result_and_path_free_readback, test_checkpoint_argv_passes_observations_file_holding_the_honest_checkpoint, test_preview_since_reports_a_render_only_change | run (this re-probe) |
| C17 | C17 | scenario.env = dict(os.environ) after prepare | 08cb3f5e447d | yes | `CheckpointLiveTest.test_run_uses_scratch_home_and_leaves_real_home_untouched` | - | run (this re-probe) |
| C18 | C18 | unredacted argv stored in checkpoint1 | 13eb8abaffae | yes | `CheckpointLiveTest.test_run_token_never_appears_in_result_or_manifest_files` | test_checkpoint_command_writes_result_and_path_free_readback | run (this re-probe) |
| C19 | C19 | delete require_accepted for checkpoint | 11e07f280f5b | yes | `CheckpointLiveRefusalTest.test_run_checkpoint_aborts_as_product_defect_when_checkpoint_is_refused` | - | run (this re-probe) |
| C20 | C20 | run_live skips the candidate check (the path cmd_checkpoint takes) | 722c6d9b2fd4 | yes | `CheckpointCliTest.test_checkpoint_command_aborts_before_first_action_on_candidate_mismatch` | test_provenance_command_aborts_before_first_action_on_candidate_mismatch | run (this re-probe) |
| C21a | C21 | cmd_checkpoint skips a03-readback.json | 16ce86f48676 | yes | `CheckpointCliTest.test_checkpoint_command_writes_result_and_path_free_readback` | - | run (this re-probe) |
| C21b | C21 | cmd_checkpoint forces verify False | a4f5c9572ba8 | yes | `CheckpointCliTest.test_checkpoint_command_writes_result_and_path_free_readback` | - | run (this re-probe) |
| C23 | C23 | checkpoint result reports verified True constantly | e83ae8be9c8f | yes | `CheckpointLiveTest.test_live_fixture_reports_verification_off` | - | run (this re-probe) |
| C24 | C24 | delete budget-omission branch | e3226c6596d0 | yes | `CheckpointDocTest.test_verify_effect_classifies_budget_omission_as_fixture_limit` | - | run (this re-probe) |
| I1 | I1 | build_document model.actual is the string unknown | 3249134b101b | yes | `ProvenanceDocTest.test_build_document_has_every_key_and_leaves_model_and_session_unknown` | P.LiveTest.test_history_readback_shows_supplied_leaves_and_unknown_leaves | run | carried from superseded-1 |
| I2a | I2 | validate_document deletes the NUL check | bf18e50f363f | yes | `ProvenanceDocTest.test_validate_document_refuses_each_leaf_rule_violation_alone` | - | run | carried from superseded-1 |
| I2b | I2 | 1024 limit becomes 2048 | 5a8f4f971fe2 | yes | `ProvenanceDocTest.test_validate_document_refuses_each_leaf_rule_violation_alone` | - | run | carried from superseded-1 |
| I3 | I3 | > 1024 becomes >= 1024 | 8cdf27d26c0d | yes | `ProvenanceDocTest.test_validate_document_accepts_one_and_1024_byte_leaves_and_unknown_object` | - | run | carried from superseded-1 |
| I4 | I4 | drop sorted in normalize_document and build_document | 5d088b5aa68f | yes | `ProvenanceDocTest.test_normalize_document_sorts_settings_by_unsigned_name_bytes` | P.DocTest.test_build_document_has_every_key_and_leaves_model_and_session_unknown, P.DocTest.test_verify_readback_accepts_exact_expansion, P.DocTest.test_verify_readback_refuses_bearer_token_anywhere, P.DocTest.test_ve... | run | carried from superseded-1 |
| I5 | I5 | delete the authority comparison | 454f83ba8018 | yes | `ProvenanceDocTest.test_verify_readback_refuses_wrong_authority` | - | run | carried from superseded-1 |
| I6 | I6 | compare only document.driver and version | 7ba37f59997a | yes | `ProvenanceDocTest.test_verify_readback_refuses_document_differing_in_one_leaf` | - | run | carried from superseded-1 |
| I7 | I7 | skip key presence loop and preferences check | 523cb90e691c | yes | `ProvenanceDocTest.test_verify_readback_refuses_missing_native_key_and_known_preferences` | - | run | carried from superseded-1 |
| I8 | I8 | delete the serialized-token scan | 531b664c3d8e | yes | `ProvenanceDocTest.test_verify_readback_refuses_bearer_token_anywhere` | P.DocTest.test_verify_absent_readback_accepts_none_driver_and_refuses_stored_record_or_known_leaf | run | carried from superseded-1 |
| I10 | I10 | require_accepted returns None regardless of exit | 0c56ed998bc5 | yes | `ProvenanceDocTest.test_refused_claim_is_reported_as_product_defect` | P.LiveRefusalTest.test_run_provenance_aborts_as_product_defect_when_claim_is_refused | run | carried from superseded-1 |
| I11 | I11 | claim argv drops --provenance | eac312833c2c | yes | `ProvenanceLiveTest.test_claim_argv_passes_provenance_file_holding_the_honest_document` | P.CliTest.test_provenance_command_writes_result_json_with_both_readbacks, P.LiveRefusalTest.test_run_provenance_aborts_as_product_defect_when_claim_is_refused, P.LiveTest.test_documents_carry_recomputed_driver_version... | run | carried from superseded-1 |
| I12 | I12 | claim document with every leaf unknown | e578117842c5 | yes | `ProvenanceLiveTest.test_history_readback_shows_supplied_leaves_and_unknown_leaves` | P.LiveTest.test_claim_argv_passes_provenance_file_holding_the_honest_document, P.LiveTest.test_documents_carry_recomputed_driver_version_and_candidate_size, P.LiveTest.test_history_readback_shows_native_derived_block,... | run | carried from superseded-1 |
| I13 | I13 | read job_activated instead of run_claimed (plan: kill lands in setUpClass, does not count; X3 is the named-assertion mutant) | 7870232f3bb1 | NO (original mutant aborts every live test with 'provenance run did not complete: history --event for claim lacks data.execution.provenance'; the plan says this does not count; X3 is the named-assertion kill) | `ProvenanceLiveTest.test_history_readback_shows_native_derived_block` | test_provenance_command_writes_result_json_with_both_readbacks, test_claim_argv_passes_provenance_file_holding_the_honest_document, test_documents_carry_recomputed_driver_version_and_candidate_size, test_history_readback_shows_supplied_leaves_and_unknown_leaves, test_live_fixture_runs_with_verification_off_and_reports_it, test_run_uses_scratch_home_and_leaves_real_home_untouched, test_takeover_stores_its_own_provenance_record, test_takeover_without_provenance_reads_back_all_unknown_driver, test_token_never_appears_in_provenance_files_or_readbacks | run (this re-probe) |
| I16 | I16 | scenario.env = dict(os.environ) after prepare (scratch HOME and PLANAR_DB in prober env) | 43cebdf580dc | yes | `ProvenanceLiveTest.test_run_uses_scratch_home_and_leaves_real_home_untouched` | - | run | carried from superseded-1 |
| I17 | I17 | delete require_accepted for claim | 410e2a4c36e5 | yes | `ProvenanceLiveRefusalTest.test_run_provenance_aborts_as_product_defect_when_claim_is_refused` | - | run | carried from superseded-1 |
| I18 | I18 | run_live skips the candidate check (the path cmd_provenance takes) | 36c65ba961b9 | yes | `ProvenanceCliTest.test_provenance_command_aborts_before_first_action_on_candidate_mismatch` | C.CliTest.test_checkpoint_command_aborts_before_first_action_on_candidate_mismatch | run | carried from superseded-1 |
| I19 | I19 | result written without the takeover section | b73b3bdc57ee | yes | `ProvenanceCliTest.test_provenance_command_writes_result_json_with_both_readbacks` | - | run | carried from superseded-1 |
| I32 | I32 | delete the authority none comparison | 281ac5295851 | yes | `ProvenanceDocTest.test_verify_absent_readback_accepts_none_driver_and_refuses_stored_record_or_known_leaf` | - | run | carried from superseded-1 |
| I41a | I41 | constant v1 driver version | b96e73ca4b0b | yes | `ProvenanceLiveTest.test_documents_carry_recomputed_driver_version_and_candidate_size` | - | run | carried from superseded-1 |
| I41b | I41 | candidate_size from len(candidate path) | 141b2dd13748 | yes | `ProvenanceLiveTest.test_documents_carry_recomputed_driver_version_and_candidate_size` | - | run | carried from superseded-1 |

Rows without a mutant: X8 (fixture mode; covered by M1, V1, X3-X7, all killed), M7 (limit, see E1), C22 (limit: cannot be built without a candidate fault hook; C19 covers the abort).
Table-source note: the "carried" rows end with the source column text of the superseded record; their `source` there was `run`.

## Named-assertion kills for I9, I13, I14, I15, I31

I9 (X1, X2), I14 (X4, X5), I15 (X6), I31 (X7): carried from `superseded-1/PROBES.md` (named-assertion messages recorded there; provenance tests unchanged). I13: X3 re-run in this re-probe, failing named test `ProvenanceLiveTest.test_history_readback_shows_native_derived_block`: AssertionError: result claim readback differs from the independent history read. The I13 original mutant (read `job_activated`) re-run: the run aborts (not a kill by the plan's rule).

## Exploratory mutants (cap 20 respected; E1-E20 identical to the superseded record, re-run where listed)

| id | breaks | mutant | diff | result | named test | note | source |
|---|---|---|---|---|---|---|---|
| E1 | M7/C18 | provenance result reports token_absent True constantly (plan limit M7) | 08245d1d9018 | SURVIVED (plan limit M7: a constant token_absent True is not separable live; the real scan is proven by M6 and X6) | `ProvenanceLiveTest.test_token_never_appears_in_provenance_files_or_readbacks` | - | run (this re-probe) |
| E2 | C18 | checkpoint result token_absent constant True | 06fc020d092a | SURVIVED (same limit for the checkpoint result; scan proven by C18) | `CheckpointLiveTest.test_run_token_never_appears_in_result_or_manifest_files` | - | run (this re-probe) |
| E3 | C9 | manifest control_equal constant True | 6d0f81717bb0 | KILLED (survivor of the superseded record, T-1/T-2 repair) | `CheckpointDocTest.test_checkpoint_manifest_is_path_free_and_recomputes_its_checks` | - | run (this re-probe) |
| E4 | C9 | manifest since_matches_after2 constant True | df9ffa2c1b4d | KILLED (survivor of the superseded record, T-1/T-2 repair) | `CheckpointDocTest.test_checkpoint_manifest_is_path_free_and_recomputes_its_checks` | - | run (this re-probe) |
| E5 | C9 | manifest since_has_checkpoint_added_section constant True | ac1cfa6bbf7a | KILLED (survivor of the superseded record, T-1/T-2 repair) | `CheckpointDocTest.test_checkpoint_manifest_is_path_free_and_recomputes_its_checks` | - | run (this re-probe) |
| E6 | C9 | informational budget flag constant False | e343c27be98c | KILLED | `CheckpointDocTest.test_checkpoint_manifest_is_path_free_and_recomputes_its_checks` | - | run (this re-probe) |
| E7 | C8 | checkpoint row accepted with after_bytes 0 | cd34aa32c0c3 | KILLED | `CheckpointDocTest.test_verify_effect_refuses_each_clause_alone` | - | run (this re-probe) |
| E8 | C8 | checkpoint row accepted with any status (changed) | 5f5644dea8cd | SURVIVED (redundant clause: a contract-valid `changed` row never has before_bytes 0; before_bytes 0 is already refused; recorded as a limit) | `CheckpointDocTest.test_verify_effect_refuses_each_clause_alone` | - | run (this re-probe) |
| E9 | C9 | manifest verified constant True | 584c4e3d808c | KILLED (survivor of the superseded record, T-1/T-2 repair) | `CheckpointDocTest.test_checkpoint_manifest_is_path_free_and_recomputes_its_checks` | - | run (this re-probe) |
| E10 | M5 | manifest native_ten_keys constant True | 17d3da4305c1 | KILLED | `ProvenanceManifestTest.test_readback_manifest_recomputes_each_check_from_the_blocks` | AssertionError: readback_manifest checks differ from the recomputed per-block checks | carried from superseded-1 |
| E11 | M5 | manifest preferences_unknown constant True | c6f2f4d831a0 | KILLED | `ProvenanceManifestTest.test_readback_manifest_recomputes_each_check_from_the_blocks` | AssertionError: readback_manifest checks differ from the recomputed per-block checks | carried from superseded-1 |
| E12 | M5 | manifest document_matches_supplied constant True | ef223e8679c4 | KILLED | `ProvenanceManifestTest.test_readback_manifest_recomputes_each_check_from_the_blocks` | AssertionError: readback_manifest checks differ from the recomputed per-block checks | carried from superseded-1 |
| E13 | M1 | run_provenance default verify False | 655cc607c7d4 | KILLED | `ProvenanceCliTest.test_provenance_command_writes_result_json_with_both_readbacks` | AssertionError: the result does not report in-run verification on (verified is not true) | carried from superseded-1 |
| E14 | C21 | run_checkpoint default verify False | 7cc49ebd61fb | KILLED | `CheckpointCliTest.test_checkpoint_command_writes_result_and_path_free_readback` | - | run (this re-probe) |
| E15 | C19/A03 | _checkpoint skips validate_checkpoint (honest input always valid) | 50bbd48d6f49 | SURVIVED (equivalent under honest input: `_checkpoint` always builds a valid checkpoint; validate_checkpoint is proven by C2) | `CheckpointLiveTest.test_checkpoint_argv_passes_observations_file_holding_the_honest_checkpoint` | - | run (this re-probe) |
| E16 | C16 | in-run control comparison deleted (verify ON path) | cab1d6c56c9e | SURVIVED (in-run clause unreachable without a candidate fault; manifest `control_equal` check, now with a negative case, and C16 cover the property) | `CheckpointCliTest.test_checkpoint_command_writes_result_and_path_free_readback` | - | run (this re-probe) |
| E17 | C11 | in-run checkpoint event listing check deleted (verify ON path) | 1d165fabc415 | SURVIVED (in-run clause unreachable without a candidate fault; C11 covers the property via an independent read) | `CheckpointCliTest.test_checkpoint_command_writes_result_and_path_free_readback` | - | run (this re-probe) |
| E18 | C2 | files entry empty/NUL check deleted | a6aaa3be68aa | KILLED (survivor of the superseded record, T-1/T-2 repair) | `CheckpointDocTest.test_validate_checkpoint_refuses_each_violation_alone_and_accepts_bounds` | - | run (this re-probe) |
| E19 | C2 | duplicate path check deleted | 88b33509991d | KILLED | `CheckpointDocTest.test_validate_checkpoint_refuses_each_violation_alone_and_accepts_bounds` | - | run (this re-probe) |
| E20 | C2 | canonical size check deleted | 4647738ad238 | KILLED | `CheckpointDocTest.test_validate_checkpoint_refuses_each_violation_alone_and_accepts_bounds` | - | run (this re-probe) |

## Repair-verification mutants (beyond the exploratory cap, new in this re-probe)

| id | mutant | diff | killed | killing test (named) | source |
|---|---|---|---|---|---|
| R1 | manifest state_equal constant True | ac815606a897 | yes | `CheckpointDocTest.test_checkpoint_manifest_is_path_free_and_recomputes_its_checks` | run |
| R2 | manifest render_moved_by_checkpoint1 constant True | 7b0f385e352d | yes | `CheckpointDocTest.test_checkpoint_manifest_is_path_free_and_recomputes_its_checks` | run |
| R3 | manifest render_moved_by_checkpoint2 constant True | 3bb8e4aed8d4 | yes | `CheckpointDocTest.test_checkpoint_manifest_is_path_free_and_recomputes_its_checks` | run |
| R4 | manifest since_material_empty constant True | 2819ccd61c22 | yes | `CheckpointDocTest.test_checkpoint_manifest_is_path_free_and_recomputes_its_checks` | run |
| R5 | manifest token_absent constant True | ff0f2471a90a | yes | `CheckpointDocTest.test_checkpoint_manifest_is_path_free_and_recomputes_its_checks` | run |

## Red-first (mutant that makes each test of this round lineage fail)

| test | mutant row(s) | status |
|---|---|---|
| `ProvenanceManifestTest.test_readback_manifest_has_exact_keys_and_values_from_result` | M3 | each killed by this test (carried or re-run, see tables) |
| `ProvenanceManifestTest.test_readback_manifest_holds_no_path_argv_or_token` | M4 | each killed by this test (carried or re-run, see tables) |
| `ProvenanceManifestTest.test_readback_manifest_recomputes_each_check_from_the_blocks` | M5 | each killed by this test (carried or re-run, see tables) |
| `ProvenanceManifestTest.test_scan_for_tokens_finds_a_token_in_any_listed_file` | M6 | each killed by this test (carried or re-run, see tables) |
| `ProvenanceLiveTest.test_live_fixture_runs_with_verification_off_and_reports_it` | V1 | each killed by this test (carried or re-run, see tables) |
| `ProvenanceCliTest.test_provenance_command_writes_result_json_with_both_readbacks (strengthened)` | M1 and M2 | each killed by this test (carried or re-run, see tables) |
| `ProvenanceDocTest.test_verify_readback_accepts_exact_expansion (strengthened)` | X1 and X2 | each killed by this test (carried or re-run, see tables) |
| `ProvenanceLiveTest.test_history_readback_shows_native_derived_block (strengthened)` | X3 | each killed by this test (carried or re-run, see tables) |
| `ProvenanceLiveTest.test_takeover_stores_its_own_provenance_record (strengthened)` | X4 and X5 | each killed by this test (carried or re-run, see tables) |
| `ProvenanceLiveTest.test_token_never_appears_in_provenance_files_or_readbacks (strengthened)` | X6 | each killed by this test (carried or re-run, see tables) |
| `ProvenanceLiveTest.test_takeover_without_provenance_reads_back_all_unknown_driver (strengthened)` | X7 | each killed by this test (carried or re-run, see tables) |
| `CheckpointDocTest.test_build_checkpoint_and_observations_have_exact_shape` | C1 | each killed by this test (carried or re-run, see tables) |
| `CheckpointDocTest.test_validate_checkpoint_refuses_each_violation_alone_and_accepts_bounds` | C2a, C2b, C2c | each killed by this test (carried or re-run, see tables) |
| `CheckpointDocTest.test_verify_effect_accepts_a_render_only_change` | C3 | each killed by this test (carried or re-run, see tables) |
| `CheckpointDocTest.test_verify_effect_refuses_each_clause_alone` | C4, C5, C6, C7, C8a, C8b, C8c, C8d | each killed by this test (carried or re-run, see tables) |
| `CheckpointDocTest.test_verify_effect_classifies_budget_omission_as_fixture_limit` | C24 | each killed by this test (carried or re-run, see tables) |
| `CheckpointDocTest.test_checkpoint_manifest_is_path_free_and_recomputes_its_checks` | C9a, C9b, C9c, C9d | each killed by this test (carried or re-run, see tables) |
| `CheckpointLiveTest.test_checkpoint_argv_passes_observations_file_holding_the_honest_checkpoint` | C10a, C10b | each killed by this test (carried or re-run, see tables) |
| `CheckpointLiveTest.test_checkpoint_event_is_recorded_and_read_back_by_history` | C11 | each killed by this test (carried or re-run, see tables) |
| `CheckpointLiveTest.test_checkpoint_moves_only_the_render_digest` | C12 and C13 | each killed by this test (carried or re-run, see tables) |
| `CheckpointLiveTest.test_preview_since_reports_a_render_only_change` | C14 | each killed by this test (carried or re-run, see tables) |
| `CheckpointLiveTest.test_second_checkpoint_with_different_text_moves_render_again` | C15 | each killed by this test (carried or re-run, see tables) |
| `CheckpointLiveTest.test_unchanged_state_gives_byte_equal_previews` | C16 | each killed by this test (carried or re-run, see tables) |
| `CheckpointLiveTest.test_run_uses_scratch_home_and_leaves_real_home_untouched` | C17 | each killed by this test (carried or re-run, see tables) |
| `CheckpointLiveTest.test_run_token_never_appears_in_result_or_manifest_files` | C18 | each killed by this test (carried or re-run, see tables) |
| `CheckpointLiveTest.test_live_fixture_reports_verification_off` | C23 | each killed by this test (carried or re-run, see tables) |
| `CheckpointLiveRefusalTest.test_run_checkpoint_aborts_as_product_defect_when_checkpoint_is_refused` | C19 | each killed by this test (carried or re-run, see tables) |
| `CheckpointCliTest.test_checkpoint_command_aborts_before_first_action_on_candidate_mismatch` | C20 | each killed by this test (carried or re-run, see tables) |
| `CheckpointCliTest.test_checkpoint_command_writes_result_and_path_free_readback` | C21a and C21b | each killed by this test (carried or re-run, see tables) |
| `CheckpointDocTest.test_validate_checkpoint_refuses_each_violation_alone_and_accepts_bounds` (repair: empty-path and NUL-path cases) | E18 | killed in this re-probe (survivor T-2 of the superseded record) |
| `CheckpointDocTest.test_checkpoint_manifest_is_path_free_and_recomputes_its_checks` (repair: nine per-check negative cases) | E3, E4, E5, E9, R1, R2, R3, R4, R5 | each killed by this test in this re-probe (survivors T-1 of the superseded record) |
Existing tests that gained a `Kills:` claim (I1-I19, I31, I32, I41): probed with their original mutants; each killed by its named test except the I13 original mutant, whose kill lands in every live test through the run abort (does not count); X3 is the named-assertion kill.

## Survivors (no unexplained survivor)

- E1, E2: plan limit M7 (a constant `token_absent: True` is not separable live; the scan is proven by M6, X6 and C18).
- E8: redundant clause (a contract-valid `changed` row never has before_bytes 0).
- E15: equivalent under the honest input (validate_checkpoint is proven by C2).
- E16, E17: in-run clauses an honest candidate never trips; the properties are covered by the manifest `control_equal` negative case, C16 and C11. Reaching them needs a candidate fault hook.
- Superseded survivors E3, E4, E5, E9 (T-1) and E18 (T-2) are now KILLED by the strengthened tests; no test-fix request remains open.

## Skipped

- Exploratory mutants beyond the cap of 20 (only R1-R5 added, as repair verification): the `_check_text` NUL deletion, the files absolute-path and `..` checks and the `len(files) > MAX_FILES` / 33-files boundary (same family as C2 and E18; not probed), `tree_sha256` mutants and the `candidate_sha256` fact (covered by I12 and I41 recomputation), `token_absent_in_run_dir` and `verified` in `readback_manifest`.
- Process rows P1-P5, P7-P16, P18-P21, P23-P25 are owned by other packages; P17: this prober edited only `probes/` in the checkout (moved `PROBES.md`/`raw/` to `superseded-1/`, wrote `raw/` and `PROBES.md`), not `build/`, the support tree or `src/`.

## Totals

- Re-run this pass: 56 mutants (I13, X3, E1-E9, E14-E20, C1-C21b, C23, C24, R1-R5): all patches applied, all files restored byte-identical; killed by the named test: 49; not killed: 7 (the I13 original abort, which does not count because X3 kills, and the explained survivors E1, E2, E8, E15, E16, E17).
- Carried: X1, X2, X4-X7, M1-M6, V1, I1-I12, I16-I19, I32, I41a, I41b, E10-E13 (all killed by their named tests in the superseded record).
- Every mutated file was restored by byte copy and re-hashed equal; baseline in scratch after the pass not re-run separately (restore verified per row); `__pycache__` absent (PYTHONDONTWRITEBYTECODE, `-B`).
