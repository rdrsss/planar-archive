# R19-ACCEPT-B red check
Red-time identity (covers tracked and untracked bytes): HEAD 080f1666ca5c095b7c02fb4526dca10e95937262; sha256(snapshot-2/candidate.diff) e3b0c44298fc1c149afbf4c8996fb92427ae41e4649b934ca495991b7852b855 (empty: no tracked src/tests/native changes); sha256(snapshot-2/SHA256SUMS) 9a47d150b154fb81636462ed7d4097c3b3899258c49abfd2245ca80640e5aeba (lists the two untracked provenance_driver.py and provenance_test.py bytes). snapshot/ pre-existed from another actor and was not overwritten.

Build: cmake --build build/rewrite-debug --target planar_tests exit 0 (raw/build.log).
Registration: the 22 tests are Python unittest, not ctest-registered; the ctest -R run reports no tests (ctest exit nonzero, logged in each raw log under '## ctest'). Outcomes are from: cd tests/native/r19-accept-support && python3 -B -m unittest provenance_test.<Class>.<test> with HOME and PLANAR_DB in scratch and PLANAR_CANDIDATE=build/rewrite-debug/bin/planar. No ERROR, import or syntax markers in any log.

| test | expectRed | outcome | failing line |
|---|---|---|---|
| ProvenanceCliTest.test_provenance_command_aborts_before_first_action_on_candidate_mismatch | expectRed | failed-assertion | provenance_test.py line 950: provenance did not report the recorded-candidate mismatch (exit 2) |
| ProvenanceCliTest.test_provenance_command_writes_result_json_with_both_readbacks | expectRed | failed-assertion | provenance_test.py line 963: the candidate record could not be made from the checkout HEAD |
| ProvenanceDocTest.test_build_document_has_every_key_and_leaves_model_and_session_unknown | expectRed | failed-assertion | provenance_test.py line 313: Lists differ: [] != ['driver', 'model', 'session', 'settings', 'trial', 'version', 'view'] |
| ProvenanceDocTest.test_normalize_document_sorts_settings_by_unsigned_name_bytes | expectRed | failed-assertion | provenance_test.py line 391: Lists differ: ['é', 'B', 'a'] != ['B', 'a', 'é'] |
| ProvenanceDocTest.test_refused_claim_is_reported_as_product_defect | expectRed | failed-assertion | provenance_test.py line 511: claim refused with a JSON code: refusal lacks an expected clause; claim refused with unparsed stderr: refusal  |
| ProvenanceDocTest.test_validate_document_accepts_one_and_1024_byte_leaves_and_unknown_object | expectRed | failed-assertion | provenance_test.py line 150: 1-byte leaf was refused: not implemented |
| ProvenanceDocTest.test_validate_document_refuses_each_leaf_rule_violation_alone | expectRed | failed-assertion | provenance_test.py line 364: empty leaf: refusal lacks an expected clause; 1025-byte leaf: refusal lacks an expected clause; NUL in leaf: r |
| ProvenanceDocTest.test_verify_absent_readback_accepts_none_driver_and_refuses_stored_record_or_known_leaf | expectRed | failed-assertion | provenance_test.py line 150: control no-file read-back was refused: not implemented |
| ProvenanceDocTest.test_verify_readback_accepts_exact_expansion | expectRed | failed-assertion | provenance_test.py line 150: real-shaped expansion (settings stored in reverse order) was refused: not implemented |
| ProvenanceDocTest.test_verify_readback_refuses_bearer_token_anywhere | expectRed | failed-assertion | provenance_test.py line 150: control expansion without the token was refused: not implemented |
| ProvenanceDocTest.test_verify_readback_refuses_document_differing_in_one_leaf | expectRed | failed-assertion | provenance_test.py line 150: control expansion was refused: not implemented |
| ProvenanceDocTest.test_verify_readback_refuses_missing_native_key_and_known_preferences | expectRed | failed-assertion | provenance_test.py line 150: control expansion was refused: not implemented |
| ProvenanceDocTest.test_verify_readback_refuses_wrong_authority | expectRed | failed-assertion | provenance_test.py line 150: control expansion was refused: not implemented |
| ProvenanceLiveRefusalTest.test_run_provenance_aborts_as_product_defect_when_claim_is_refused | expectRed | failed-assertion | provenance_test.py line 137: the refused claim is not reported as a product defect naming claim |
| ProvenanceLiveTest.test_claim_argv_passes_provenance_file_holding_the_honest_document | expectRed | failed-assertion | provenance_test.py line 705: provenance run did not complete: not implemented |
| ProvenanceLiveTest.test_documents_carry_recomputed_driver_version_and_candidate_size | expectRed | failed-assertion | provenance_test.py line 860: provenance run did not complete: not implemented |
| ProvenanceLiveTest.test_history_readback_shows_native_derived_block | expectRed | failed-assertion | provenance_test.py line 764: provenance run did not complete: not implemented |
| ProvenanceLiveTest.test_history_readback_shows_supplied_leaves_and_unknown_leaves | expectRed | failed-assertion | provenance_test.py line 738: provenance run did not complete: not implemented |
| ProvenanceLiveTest.test_run_uses_scratch_home_and_leaves_real_home_untouched | expectRed | failed-assertion | provenance_test.py line 818: provenance run did not complete: not implemented |
| ProvenanceLiveTest.test_takeover_stores_its_own_provenance_record | expectRed | failed-assertion | provenance_test.py line 774: provenance run did not complete: not implemented |
| ProvenanceLiveTest.test_takeover_without_provenance_reads_back_all_unknown_driver | expectRed | failed-assertion | provenance_test.py line 840: provenance run did not complete: not implemented |
| ProvenanceLiveTest.test_token_never_appears_in_provenance_files_or_readbacks | expectRed | failed-assertion | provenance_test.py line 800: provenance run did not complete: not implemented |

## C01 addendum (A1)

Written by the RED package of R19-ACCEPT-B-C01 (wave 2). Append-only: the original red record above is unchanged. The C01 red capture is in `red-c01/RED-C01.md` (committed) with raw logs in `red-c01/raw/` (local).

- `accept_lib.validate_red_record` on the ORIGINAL red logs (read-only; the 22 `unittest` sections of `red/raw/*.log`, exit 1 each, `snapshot_sha256` = the combined snapshot hash recorded in `red/snapshot-hashes.txt`): accepted 22 of 22.
- `COMPILE_OR_IMPORT` matches in the ORIGINAL red logs: 0 (the accept_lib regex, run over the same 22 `unittest` sections).
- The I19 red failed partly because the candidate sat under `build/rewrite-*`: the recorded run used `PLANAR_CANDIDATE=build/rewrite-debug/bin/planar`, and the log of `ProvenanceCliTest.test_provenance_command_writes_result_json_with_both_readbacks` shows `accept_lib.py:599` refusing it (`candidate must not live in a shared build/rewrite-* directory`). That test's red is therefore not purely the missing behaviour. The log of `ProvenanceCliTest.test_provenance_command_aborts_before_first_action_on_candidate_mismatch` ends with the assertion `provenance did not report the recorded-candidate mismatch (exit 2)`.
- The shared-build ninja call was a no-op: `red/raw/build.log` ends with `ninja: no work to do.`, so the `cmake --build build/rewrite-debug --target planar_tests` call of that red did not rebuild anything.
