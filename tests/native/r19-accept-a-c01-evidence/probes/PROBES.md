# R19-ACCEPT-A-C01 mutation probes (prober, re-probe after the survivor repairs)

Run 2026-11-11. Mutants applied only in the warm scratch copy `probe-warm` (outside the checkout; synced with `rsync -a --delete --exclude build/ --exclude .git/ --exclude 'tests/native/*-evidence/'`, its `build/` and `.git` kept), restored by byte copy with the sha256 verified after every mutant (`restored=True` in each log). A mutant is applied with `patch` and the run is chained on a successful apply (a failed apply would be logged as not a kill). Python mutants ran as `PYTHONDONTWRITEBYTECODE=1 PLANAR_CANDIDATE=<recorded candidate> python3 -B -m unittest -v <modules>` from `tests/native/r19-accept-support` in the scratch copy. No C++ file changed since the superseded probe run (src hashes below equal `superseded-1/PROBES.md`), so no C++ target was rebuilt and the H rows are carried forward. After the run the scratch tree equals the checkout (`rsync -an` shows no difference). The checkout and its `build/` were not touched; the previous record is kept whole under `probes/superseded-1/` (PROBES.md and raw/).

## Candidate identity (identity.py src tests/native/r19-accept-support, run from the repository root)

| identity | value |
|---|---|
| HEAD | `91e0df24f2167471e49ec057c7159bce5dff3608` |
| sha256(git diff --binary HEAD -- src tests/native/r19-accept-support), tracked only | `83a16fdee033fce5a1a792d8d65f4090fba8dacffc2d38013684cacb4daeba19` |
| combined sha256 (diff bytes, then each untracked path NUL bytes, sorted), pathspec src tests/native/r19-accept-support, evidence excluded | `6612206823f3350ea5e9ce36bf9f39750a553894ca6624941379f2b200dd9891` |

| file | state | sha256 |
|---|---|---|
| `src/engine/ledger/execution.t.cpp` | modified | `250867b8bd74b86d718cbe17e5fa9983fda6a60a2b5e7c127829a3fec6f98950` |
| `src/engine/ledger/storage.cpp` | modified | `e4cf41dcb8cf44cc8004d4ba011543a557c742c623e20ede631eb975f237d822` |
| `tests/native/r19-accept-support/accept_lib.py` | untracked | `9809217494dc27ebcfdf4728df83076a303aefb473de79ba6488b8d0099f6f16` |
| `tests/native/r19-accept-support/accept_test.py` | untracked | `6c3d6605e599a2b98a5fa047d0761afec5a9307a0d088ea03889fe6eeb73d863` |
| `tests/native/r19-accept-support/acceptance_cli_test.py` | untracked | `57c15f37e813dacad89b0b6892f6b1e2269df96107d789c54c2b22cd59976336` |
| `tests/native/r19-accept-support/check_runner.py` | untracked | `b71e939a226415c0aeb43083dffcf263618d2c88804500300e585bfafec1d5f1` |
| `tests/native/r19-accept-support/cmake_check.py` | untracked | `ba8f20625bda0f3c8fc3c613529e16f6a1f8e0a38b0b261f8341b2fdcbb3f360` |
| `tests/native/r19-accept-support/fault/cmake_rerun/run_heavy.py` | untracked | `d30d1faa5f0c82d1a57eee69b7727dcb6d5bb8e352607506dab2fd729ef5a9ed` |
| `tests/native/r19-accept-support/fault/mini/base/src/unit.txt` | untracked | `d73d5ccc52b8747e2bc253b7267b6110d41021eacd54ff33695d96c0a47fe56a` |
| `tests/native/r19-accept-support/fault/mini/jobs/combined-max1.lua` | untracked | `c7d8b3a33fd7940b6ef56e4ac4923836f527644d8b6952aaa8ae5d22f54e8e39` |
| `tests/native/r19-accept-support/fault/mini/jobs/combined-max2.lua` | untracked | `6d0df6cf79f148664587dcf43e774243cad211e4d26420ce8fe4f955c1281e0e` |
| `tests/native/r19-accept-support/fault/mini/registry.json` | untracked | `eca5ac671b6929f042a839591ff56da72d0afef8f4e9aca5433df7b1119490ca` |
| `tests/native/r19-accept-support/fault/mini/source/src/unit.txt` | untracked | `753f8fe079abf1160eaa98cc40257a00da60a033b2d3730ab679fa0f62a0778c` |
| `tests/native/r19-accept-support/fault/mini/workspace/spec.lua` | untracked | `d35b07e34032202e830cedb66b4d45b5774621f087acc9f8012b1d8edacf07da` |
| `tests/native/r19-accept-support/fault/mini/workspace/workspace.lua` | untracked | `21ac5c6da0e2d27bb4911202d0a364acacc49d39b052fa71d8ca44b68e2d453d` |
| `tests/native/r19-accept-support/fault/sleep_check.py` | untracked | `751414911570978faf4f82d0ee79631ceb41c76dfd2bc4e66d35c41faa51ce57` |
| `tests/native/r19-accept-support/fault_driver.py` | untracked | `4e12a07086ddbf773d8a88c45727d5a89b425bdf0874de7eaed76dd058c8509f` |
| `tests/native/r19-accept-support/fault_live_test.py` | untracked | `a7f7c4761246e4f06ac519092431346f47ac32961e1cbc56c2491f200ba51f78` |
| `tests/native/r19-accept-support/quota_driver.py` | untracked | `3c45a76571439a528e3cf1aee3803308964349da6edb247235cc656e8a4a3420` |
| `tests/native/r19-accept-support/quota_live_test.py` | untracked | `038f9c25f0f9de6db7ecec7f091ae508698196c52851870c397aa1522ef8873a` |
| `tests/native/r19-accept-support/registry_gen.py` | untracked | `26752307e7257a99722b713342c790ae3eb146bd99c93ea410b6b4312109d172` |
| `tests/native/r19-accept-support/run_acceptance.py` | untracked | `7e83069b78b82b8f58568c66fe133a1560c014527050a74158a1c7fbf49d1c87` |


Recorded candidate binary: sha256 `774bcd69809ead9e4cd7b6e06c74aa045ea9272a2321cd36f91bc9c76a99ab0c` (shasum of the scratchpad `r19-candidate-c01-rev2` export, equal to `tests/native/r19-accept-evidence/gate/candidate/candidate.json`; the src files it was built from are unchanged).

## Carry-forward decision

Previous record: `probes/superseded-1/PROBES.md` (identity combined `561356934db6...`). Byte comparison with the identity above: changed files are only `accept_test.py` (f1c58dc8... -> 6c3d6605...), `quota_driver.py` (baced5e03... -> 3c45a765...) and `quota_live_test.py` (b32d934f... -> 038f9c25...). Every other src and support file is byte-identical (execution.t.cpp, storage.cpp, accept_lib.py, acceptance_cli_test.py, check_runner.py, cmake_check.py, fault_driver.py, fault_live_test.py, registry_gen.py, run_acceptance.py, the fault fixtures). Rule applied: a row is carried only when its mutated file AND its killing test file are byte-identical.
- RE-RUN: every row killed by `accept_test.py` (M-I*, M-E13/14/15/17/18, X-cand), the three survivors' rows S2b, S2c, S2bc (now also under accept_test), M-I13b2, and every quota row (L1a, L1a2, L1b, L24a, L24b, L3, L3b, L4, L5), plus new row X-Q14a.
- CARRIED (cited as `superseded-1/raw/<id>.log`): H-prefix, H1, H2, H3a, H3b, H3c, H5 (C++ files and runtime.cpp unchanged), M-A6, M-E6, M-E8, M-E10, M-E11, S1a-S1d, S2a, S3, R1a-R1c, R2a-R2k, X-R1d, X-R2l-X-R2p, X-S1e (run_acceptance.py and acceptance_cli_test.py unchanged), L11 (fault_driver.py and fault_live_test.py unchanged).

## Baselines (unmutated, raw logs kept)

| run | command | result | log |
|---|---|---|---|
| unit + CLI suites without candidate | `python3 -B -m unittest accept_test acceptance_cli_test` | Ran 62, FAILED (failures=1): only `test_live_commands_abort_before_first_action_on_candidate_mismatch` (A6 refuses, never skips) | raw/baseline-nocandidate.log |
| unit + CLI suites with `PLANAR_CANDIDATE` | `python3 -B -m unittest -v accept_test acceptance_cli_test` | Ran 62, OK, 0 skipped: A6 green (one test more than before: `test_suite_refusals_each_clause_stands_alone`) | raw/baseline-candidate.log |
| the two live tests touched by the new fixture, with candidate | `python3 -B -m unittest -v quota_live_test.QuotaLiveTest.test_run_grouping_aborts_when_replay_reexecutes_a_check quota_live_test.QuotaLiveTest.test_restart_replay_is_byte_identical_without_reexecution` | Ran 2, OK, 0 skipped (21 s) | raw/baseline-live-q14.log |
| full live modules | previous run `superseded-1/raw/baseline-live.log` (45 tests then); NOT re-run here, quota_live_test now has one more test (46 expected); the GATE stage owns the full live count | not re-run | - |
| C++ H tests | carried: 4 of 4 passed on the unchanged C++ (superseded-1) | carried | superseded-1/PROBES.md |

Python counts observed here: accept_test + acceptance_cli_test 62 tests, 0 skipped (candidate set). Full ctest (Debug, Release), the Linux container gate, G1 comm -23 and the per-module Python totals belong to the gate stage and were not run here.

## Mutant results

Hash = sha256 of the unified diff (full diff at the top of each raw log). "named" = a test the row names failed; "other test" = a different test killed. Failing-test names are unique names (a log lists FAIL/ERROR once per header and traceback).

### Re-run rows

| id | file | mutant | diff sha256 | killed | failing tests | named? | how |
|---|---|---|---|---|---|---|---|
| M-I3b | accept_lib.py | scan_host_paths: skip decoded JSON string value search | `4b5fad2af08a49afe6c1981980631a6e5308029025e9c1bf9548ef879298a33a` | KILLED | test_scan_finds_prefix_only_present_as_decoded_json_value | named | re-run, raw/M-I3b.log (restored=True) |
| M-I10 | accept_lib.py | validate_trajectory: drop the check-recover (via) requirement | `88cd0eebb3c889b8b7c6c4a7226c4c287f3ae4a8d9d2451110c477439fff963a` | KILLED | test_trajectory_interruption_must_close_by_check_recover | named | re-run, raw/M-I10.log (restored=True) |
| M-I10c | accept_lib.py | validate_trajectory: drop the check_interrupted requirement (a missing event falls back to event 0) | `51dc9014075a9aa02a74f1790484d70147e4f218dbce42a85aeb829e4268bfc6` | KILLED | test_trajectory_interruption_must_close_by_check_recover | named | re-run, raw/M-I10c.log (restored=True) |
| M-I10d | accept_lib.py | validate_trajectory: drop BOTH check-recover and check_interrupted requirements | `3d6304e3625420120138b6a8f8b24a1b3f4212b1419e75d3329d97fccbe67366` | KILLED | test_trajectory_interruption_must_close_by_check_recover, test_trajectory_requires_interrupted_check_recovery | named | re-run, raw/M-I10d.log (restored=True) |
| M-I12 | accept_lib.py | _admitted_rejection: skip repair_admitted | `8f533e82d7995dd678e174f3bf48f2050ecced15e53643975e37ee2dfcb6bfa8` | KILLED | test_trajectory_rejection_admission_predicate_is_exact | named | re-run, raw/M-I12.log (restored=True) |
| M-I12c | accept_lib.py | _admitted_rejection: admit regardless of count < max_rounds | `73cbc22c0600eda72836f9f717cf99623b13c5722016d653f2ba4e3f3fff2fd4` | KILLED | test_trajectory_rejection_admission_predicate_is_exact | named | re-run, raw/M-I12c.log (restored=True) |
| M-I12d | accept_lib.py | _admitted_rejection: accept an approve as the admitted rejection | `44953e487c22efa1c73140f349daf5f68e8b593d076f8a085df3c78615e43c25` | KILLED | test_trajectory_rejection_admission_predicate_is_exact | named | re-run, raw/M-I12d.log (restored=True) |
| M-I13b | accept_lib.py | _capped_rejection: repair_admitted may be True or False (admitted-or-capped) | `13e9af434e6a2771bcf33324c55583a87bc0a2abc68b8a233d1cc1211449083d` | KILLED | test_trajectory_capped_rejection_requires_repair_not_admitted | named | re-run, raw/M-I13b.log (restored=True) |
| M-I13b2 | accept_lib.py | _capped_rejection: repair roles allowed on a capped rejection | `2fe75178511fe2a4f771f80bd4138a34255b1d7cd40eac183893cf6e7f99e7a7` | KILLED | test_trajectory_capped_rejection_requires_repair_not_admitted | named | re-run, raw/M-I13b2.log (restored=True) |
| M-I17c | cmake_check.py | inventory_verdict: tolerate a duplicate inventory name | `e18319f17aae81dd170abe52660adcabe16a86cad5269b17ba4869ee03f866f7` | KILLED | test_inventory_verdict_refuses_duplicate_inventory_names | named | re-run, raw/M-I17c.log (restored=True) |
| M-I24 | accept_lib.py | validate_red_record: accept nonzero exit without an assertion marker | `6983793a64a6a863407bdb40a83a18042a03c25de25a02ee5f0fc519f5a12f67` | KILLED | test_red_record_each_marker_rule_stands_alone | named | re-run, raw/M-I24.log (restored=True) |
| M-I24b | accept_lib.py | validate_red_record: accept a compile/import error | `5941afbb963721f99effbc11b949527631b5921b994f09155117fa4302ed997e` | KILLED | test_red_record_each_marker_rule_stands_alone | named | re-run, raw/M-I24b.log (restored=True) |
| M-I29b | check_runner.py | check_runner: passthrough mode also writes the result | `40eb153e5042823eb373b0af9c25594d6970b4ea356f0caa76201bac6231b71e` | KILLED | test_check_runner_passthrough_never_writes_a_result_document | named | re-run, raw/M-I29b.log (restored=True) |
| M-I30c | accept_lib.py | write_rendezvous: in-place write instead of atomic replace | `4dadc2d9df1e1e3e63bf43bdb1d0bcd7734cab8cb3b57b6b80156fcc7339295e` | KILLED | test_rendezvous_write_replaces_the_file_instead_of_rewriting_it_in_place | named | re-run, raw/M-I30c.log (restored=True) |
| M-I35b | accept_lib.py | child_env: drop the PYTHONDONTWRITEBYTECODE key | `99cd680510cf1b4e07628d274e1cfab22cf48d21800526f888996b8386ba66d0` | KILLED | test_child_env_sets_bytecode_flag_even_when_the_parent_lacks_it | named | re-run, raw/M-I35b.log (restored=True) |
| M-E13 | accept_lib.py | build_verdict: accept a duplicate target unit | `0b9b84eac317ea990029d78c51957d30e5e79dd020e616f14c9277b1f519d802` | KILLED | test_build_verdict_refuses_a_unit_named_twice | named | re-run, raw/M-E13.log (restored=True) |
| M-E14 | accept_lib.py | verify_event_expansion: accept empty expected references | `70a1dcf15a0fc6be9af5bc03bd07e30bdc6ec01aea77a251d65de983d17973f4` | KILLED | test_scan_and_event_expansion_refuse_vacuous_inputs | named | re-run, raw/M-E14.log (restored=True) |
| M-E15 | accept_lib.py | scan_host_paths: accept an empty scan prefix | `838be58c77dea8ee2c1f588251bc8f61b8c9e6a88eec077ce8b787bc8f858f2f` | KILLED | test_scan_and_event_expansion_refuse_vacuous_inputs | named | re-run, raw/M-E15.log (restored=True) |
| M-E17 | accept_lib.py | verify_at_event_state: accept a missing roles mapping | `b10536ef880df4add00b5306179725ddc80cf03e787de348db2a88d114a823f3` | KILLED | test_at_event_state_refuses_missing_roles_and_extra_roles | named | re-run, raw/M-E17.log (restored=True) |
| M-E18 | accept_lib.py | verify_at_event_state: accept a role set mismatch | `a8a76ab75a2d8728c2c3588e34ab27755eb00c2b7b704a6b8b73ba3c2d5b736c` | KILLED | test_at_event_state_refuses_missing_roles_and_extra_roles | named | re-run, raw/M-E18.log (restored=True) |
| X-cand | accept_lib.py | require_recorded_candidate: skip sha256/size comparison (head only) | `63ca1c1cf3f2311126ae80847ea8432a5f7fe36ee9489246d439e7a44ae87a07` | KILLED | test_recorded_candidate_verification_rejects_mismatch | named | re-run, raw/X-cand.log (restored=True) |
| S2b | run_acceptance.py | suite_refusals: zero matched tests is success | `10a922822e1c87d7d20dd29cab1cabd1118cfa361587a22513fea72afd1ddc0c` | KILLED | test_suite_refusals_each_clause_stands_alone | named | re-run, raw/S2b.log (restored=True) |
| S2c | run_acceptance.py | suite_refusals: ignore child exit | `3cc615e469482654e04658c3e50858450aa5b92160dda41c5d519bea72fcd6b5` | KILLED | test_suite_refusals_each_clause_stands_alone | named | re-run, raw/S2c.log (restored=True) |
| S2bc | run_acceptance.py | suite_refusals: zero matched tests is success AND child exit ignored (the two masking rules together) | `a8f5cd64a1b4a9a465b1c5f4fbfb567a50d93ccffbe87e8d81be928eacd29531` | KILLED | test_suite_refusals_each_clause_stands_alone, test_suites_command_refuses_skips_failures_and_zero_matched | named | re-run, raw/S2bc.log (restored=True) |
| L1a | quota_driver.py | quota_driver _exec: skip the .stdout retention write for the check verb | `a8b19c6964d4900fc3cadcd3b6cb44e7b85d5e422af2931087e146d7343dfc1a` | KILLED | test_every_action_retains_raw_argv_stdout_stderr_exit | named | re-run, raw/L1a.log (restored=True) |
| L1a2 | quota_driver.py | quota_driver _exec: skip the .exit retention write for the verdict verb | `45bd362161f6d894792ade256e6e230380e029aa06999020fe107ff2aac95cb1` | KILLED | test_every_action_retains_raw_argv_stdout_stderr_exit | named | re-run, raw/L1a2.log (restored=True) |
| L1b | quota_driver.py | quota_driver submit_verdict: approve with the builder actor instead of the reviewer | `bc3351ed13492765d4224591c72b465c377c4ae31ba7eb5ec86906b681ab049b` | KILLED | test_split_grouping_retains_raw_exact_targets_and_non_builder_actor | named | re-run, raw/L1b.log (restored=True) |
| L24a | quota_driver.py | quota_driver split: approve with only the library unit's targets (another/partial unit targets) | `a74d43addcf2098bf3cab8e3cac6ff49e4064023fb81567da1f5e7e4430737ef` | KILLED | test_split_grouping_retains_raw_exact_targets_and_non_builder_actor, test_split_grouping_orders_dependency_and_completes | named | re-run, raw/L24a.log (restored=True) |
| L24b | quota_driver.py | quota_driver _exec: skip the .argv.json write for the claim verb (split run) | `af12b596fa8fc1c78d8a09838643dd8c88f65000da48fec33ea4c9011a6e34bf` | KILLED | test_split_grouping_retains_raw_exact_targets_and_non_builder_actor | named | re-run, raw/L24b.log (restored=True) |
| L3 | quota_driver.py | quota_driver replay: send a new request id instead of the saved request | `eedae0321493c7291d62b73c9f0882b7aed9617b28a2620d2c053a4cae36bb94` | KILLED | test_restart_replay_is_byte_identical_without_reexecution | named | re-run, raw/L3.log (restored=True) |
| L3b | quota_driver.py | quota_driver replay: ignore the check-execution counter | `e8207d9d82827dc0835095dfaa7d873efb3d611a2504e482d7fb78d1f16b5345` | KILLED | test_run_grouping_aborts_when_replay_reexecutes_a_check | named | re-run, raw/L3b.log (restored=True) |
| L4 | quota_driver.py | quota_driver inspect: drop the ledger digest comparison after inspection | `5b12a228540f8b331b9dcfd9dcb0d9cd269b7e0afa89c4482e3a21218004b72a` | KILLED | test_run_grouping_aborts_when_inspection_changes_ledger | named | re-run, raw/L4.log (restored=True) |
| L5 | quota_driver.py | quota_driver wrong_target_control: keep the correct output (accept the wrong-target document) | `80d14922d9200e95ce17b3b52558a2123d0c1ea42f2970754ab8e2ac71dc6f42` | KILLED | test_wrong_target_verdict_is_refused_and_ledger_unchanged | named | re-run, raw/L5.log (restored=True) |
| X-Q14a | quota_driver.py | quota_driver replay tamper: counter tamper never applied (the new Q14 fixture's injected fault vanishes) | `ce2fd141967e43db1aee1d4a050ee2099a486ccb6194794985b487cb994bbf6a` | KILLED | test_run_grouping_aborts_when_replay_reexecutes_a_check | named | re-run, raw/X-Q14a.log (restored=True) |

Notes on killers: M-I10d is also killed by `test_trajectory_requires_interrupted_check_recovery` (existing test). M-I35b is killed as an ERROR in the named test. S2bc is killed by both the new `test_suite_refusals_each_clause_stands_alone` and the existing CLI test. L1b, L24a also fail a second quota test as before.

### Carried-forward rows (not re-run; source row in superseded-1)

| id | file | mutant | killed | killing test | source |
|---|---|---|---|---|---|
| H-prefix | storage.cpp (C++) | the original unfixed history_rows block restored (both defects at once) | KILLED | R19-ACCEPT-A-C01 history event of a recorded check keeps evidence boolean nodes free of the integer value; R19-ACCEPT-A-C01 history event of a recorded check keeps evidence integer nodes free of the boolean flag | superseded-1/raw/H-prefix.log |
| H1 | storage.cpp (C++) | integer kind also sets node.boolean = **raw != 0 (pre-fix defect 1) | KILLED | R19-ACCEPT-A-C01 history event of a recorded check keeps evidence integer nodes free of the boolean flag | superseded-1/raw/H1.log |
| H2 | storage.cpp (C++) | boolean kind also keeps node.integer = **raw (pre-fix defect 2) | KILLED | R19-ACCEPT-A-C01 history event of a recorded check keeps evidence boolean nodes free of the integer value | superseded-1/raw/H2.log |
| H3a | storage.cpp (C++) | clamp: node.integer = **raw != 0 (over-correction) | KILLED | R19-ACCEPT-A-C01 history event of a recorded check preserves evidence integer and boolean values exactly | superseded-1/raw/H3a.log |
| H3b | storage.cpp (C++) | always-true: node.boolean = true (over-correction) | KILLED | R19-ACCEPT-A-C01 history event of a recorded check preserves evidence integer and boolean values exactly | superseded-1/raw/H3b.log |
| H3c | storage.cpp (C++) | extra: always-false: node.boolean = false | KILLED | R19-ACCEPT-A-C01 history event of a recorded check keeps evidence boolean nodes free of the integer value; R19-ACCEPT-A-C01 history event of a recorded check preserves evidence integer and boolean values exactly | superseded-1/raw/H3c.log |
| H5 | runtime.cpp (C++) | prepare_role: delete charge(s, 6 * 320 + sizeof(value)) (runtime.cpp:7812) | KILLED | X08 C01 role preparation outputs-off boundary pins the request-record conversion charge | superseded-1/raw/H5.log |
| M-A6 | run_acceptance.py | run_live: remove the require_candidate call before the first action | KILLED | test_live_commands_abort_before_first_action_on_candidate_mismatch | superseded-1/raw/M-A6.log |
| M-E6 | run_acceptance.py | cmd_index: skip the residual host-prefix scan | KILLED | test_index_fails_when_a_host_prefix_survives_redaction | superseded-1/raw/M-E6.log |
| M-E8 | run_acceptance.py | cmd_verify_redaction: skip the original-hash check | KILLED | test_verify_redaction_checks_original_hash_and_real_prefixes | superseded-1/raw/M-E8.log |
| M-E10 | run_acceptance.py | cmd_verify_redaction: remove the manifest real-prefix check | KILLED | test_verify_redaction_checks_original_hash_and_real_prefixes | superseded-1/raw/M-E10.log |
| M-E11 | run_acceptance.py | cmd_verify_redaction: remove the TEMP rules record real-prefix check | KILLED | test_verify_redaction_checks_original_hash_and_real_prefixes | superseded-1/raw/M-E11.log |
| S1a | run_acceptance.py | suite_report: only the total line | KILLED | test_suites_command_reports_per_module_and_total_counts | superseded-1/raw/S1a.log |
| S1b | run_acceptance.py | suite_report: total sums wrong (max instead of sum) | KILLED | test_suites_command_reports_per_module_and_total_counts | superseded-1/raw/S1b.log |
| S1c | run_acceptance.py | suite_report: same ran for every module | KILLED | test_suites_command_reports_per_module_and_total_counts | superseded-1/raw/S1c.log |
| S1d | run_acceptance.py | run_suite_module: bare python3 child | KILLED | test_suites_command_reports_per_module_and_total_counts | superseded-1/raw/S1d.log |
| S2a | run_acceptance.py | suite_refusals: ignore skipped | KILLED | test_suites_command_refuses_skips_failures_and_zero_matched | superseded-1/raw/S2a.log |
| S3 | run_acceptance.py | cmd_suites: skip require_live_candidate for live modules | KILLED | test_suites_command_live_modules_require_the_recorded_candidate | superseded-1/raw/S3.log |
| R1a | run_acceptance.py | build_rerun_index: omit the fenced section | KILLED | test_rerun_index_records_every_required_fault_field | superseded-1/raw/R1a.log |
| R1b | run_acceptance.py | build_rerun_index: omit the load section | KILLED | test_rerun_index_records_every_required_fault_field | superseded-1/raw/R1b.log |
| R1c | run_acceptance.py | build_rerun_index: omit the trajectory section | KILLED | test_rerun_index_records_every_required_fault_field | superseded-1/raw/R1c.log |
| R2a | run_acceptance.py | rerun_field: a missing path returns None instead of failing | KILLED | test_rerun_index_fails_when_any_required_field_is_missing | superseded-1/raw/R2a.log |
| R2b | run_acceptance.py | check_verdicts: skip reject1.repair_admitted predicate | KILLED | test_rerun_index_fails_when_any_required_field_is_missing | superseded-1/raw/R2b.log |
| R2c | run_acceptance.py | check_verdicts: skip reject2.capped predicate | KILLED | test_rerun_index_fails_when_any_required_field_is_missing | superseded-1/raw/R2c.log |
| R2d | run_acceptance.py | check_verdicts: skip max_rounds blocker predicate | KILLED | test_rerun_index_fails_when_any_required_field_is_missing | superseded-1/raw/R2d.log |
| R2e | run_acceptance.py | check_load: load need not raise max_rounds | KILLED | test_rerun_index_fails_when_any_required_field_is_missing | superseded-1/raw/R2e.log |
| R2f | run_acceptance.py | check_verdicts: skip targets_equal_latest_output predicate | KILLED | test_rerun_index_fails_when_any_required_field_is_missing | superseded-1/raw/R2f.log |
| R2g | run_acceptance.py | check_replays_and_ledger: replay response_identical ignored | KILLED | test_rerun_index_fails_when_any_required_field_is_missing | superseded-1/raw/R2g.log |
| R2h | run_acceptance.py | check_replays_and_ledger: counter lines need not equal intended executions | KILLED | test_rerun_index_fails_when_any_required_field_is_missing | superseded-1/raw/R2h.log |
| R2i | run_acceptance.py | check_replays_and_ledger: ledger digests need not match | KILLED | test_rerun_index_fails_when_any_required_field_is_missing | superseded-1/raw/R2i.log |
| R2j | run_acceptance.py | check_history_and_trajectory: empty trajectory events accepted | KILLED | test_rerun_index_fails_when_any_required_field_is_missing | superseded-1/raw/R2j.log |
| R2k | run_acceptance.py | cmd_rerun_index: write the index before validating | KILLED | test_rerun_index_fails_when_any_required_field_is_missing, test_rerun_index_records_every_required_fault_field | superseded-1/raw/R2k.log |
| X-R1d | run_acceptance.py | build_rerun_index: omit the counters section | KILLED | test_rerun_index_records_every_required_fault_field | superseded-1/raw/X-R1d.log |
| X-R2l | run_acceptance.py | check_takeover_and_fence: fenced ledger_before==ledger_after not required | KILLED | test_rerun_index_fails_when_any_required_field_is_missing | superseded-1/raw/X-R2l.log |
| X-R2m | run_acceptance.py | check_takeover_and_fence: fenced.exit may be zero | KILLED | test_rerun_index_fails_when_any_required_field_is_missing | superseded-1/raw/X-R2m.log |
| X-R2n | run_acceptance.py | check_takeover_and_fence: takeover generation need not advance | KILLED | test_rerun_index_fails_when_any_required_field_is_missing | superseded-1/raw/X-R2n.log |
| X-R2o | run_acceptance.py | check_faults_and_restart: recover exit need not be 0 | KILLED | test_rerun_index_fails_when_any_required_field_is_missing | superseded-1/raw/X-R2o.log |
| X-R2p | run_acceptance.py | check_faults_and_restart: interrupted child need not be dead | KILLED | test_rerun_index_fails_when_any_required_field_is_missing | superseded-1/raw/X-R2p.log |
| X-S1e | run_acceptance.py | cmd_suites: --out file differs from stdout (extra newline) | KILLED | test_suites_command_reports_per_module_and_total_counts | superseded-1/raw/X-S1e.log |
| L11 | fault_driver.py | fault_driver: approve instead of reject for the first verdict | KILLED | test_rejecting_verdict_admits_repair_and_worker_returns | superseded-1/raw/L11.log |

H rows from the invariant table: H1 (killed by its named test), H2 (named test), H3 (H3a clamp, H3b always-true, both killed by the named H3 test; the test passes on the unfixed source), H5 (killed by the named X08 test), all carried. H4 (live history --event, D33) is not a probe mutant; protection is H1/H2 and H-prefix. H6 and G1 are record/gate rows (see below).

## Survivors

None remain among the rows run or carried. Earlier survivors (superseded-1) and their closing fixtures, all re-run here and now killed by the named new fixture:
1. M-I13b2 (capped rejection with repair roles): killed by `test_trajectory_capped_rejection_requires_repair_not_admitted` (new `repair=["builder"]` assertion).
2. S2b (ran=0 accepted) and S2c (child exit ignored), each survived alone before: now killed by the new `AcceptLibGapTest.test_suite_refusals_each_clause_stands_alone`; S2bc stays killed.
3. L3b (replay counter comparison ignored): killed by the new `QuotaLiveTest.test_run_grouping_aborts_when_replay_reexecutes_a_check` (Q14, uses the new `tamper.counter` of `quota_driver`). X-Q14a (the tamper never applied) also kills it, proving the fixture can fail for the reason given.

## Red-first table (every test added in this lineage and the mutant that makes it fail)

| new test | mutant that makes it fail | where recorded |
|---|---|---|
| R19-ACCEPT-A-C01 history event ... keeps evidence integer nodes free of the boolean flag | H1, H-prefix | superseded-1/raw/H1.log, H-prefix.log (carried; behavioural red of the pre-fix source in `tests/native/r19-accept-a-evidence/red-c01/history/`) |
| ... keeps evidence boolean nodes free of the integer value | H2, H-prefix | superseded-1/raw/H2.log |
| ... preserves evidence integer and boolean values exactly | H3a, H3b, H3c (passes unfixed, expectRed false) | superseded-1/raw/H3a.log, H3b.log, H3c.log |
| AcceptanceCliTest suites / rerun-index tests (S1, S2, S3, R1, R2) | S1a-S1d, X-S1e; S2a, S2bc; S3; R1a-R1c, X-R1d; R2a-R2k, X-R2l..p | carried raw logs |
| AcceptLibGapTest.test_suite_refusals_each_clause_stands_alone | S2b, S2c, S2bc | raw/S2b.log, S2c.log, S2bc.log |
| AcceptLibGapTest.test_trajectory_capped_rejection_requires_repair_not_admitted | M-I13b, M-I13b2 | raw/ |
| other AcceptLibGapTest tests (I3b, E13-E18, I10, I12, I17c, I24, I29b, I30c, I35b) | M-I3b, M-E13..E18, M-I10/c/d, M-I12/c/d, M-I17c, M-I24/b, M-I29b, M-I30c, M-I35b | raw/ |
| QuotaLiveTest.test_run_grouping_aborts_when_replay_reexecutes_a_check | L3b, X-Q14a | raw/L3b.log, raw/X-Q14a.log |
| QuotaLiveTest L1/L24/L3/L4/L5 tests | L1a, L1a2, L1b, L24a, L24b, L3, L4, L5 | raw/ |
| AcceptLibTest.test_red_record_must_be_behavioural_and_hashed (H6) | M-I24, M-I24b (stand-alone rules) | raw/ |

## Acceptance items observed by the prober

- H6: unchanged from the superseded record (carried): red record has `FAILED:` lines, no compile error, validator ACCEPTED, snapshot hash of the pre-fix src; this round did not re-verify it.
- A6 green with `PLANAR_CANDIDATE` (62 tests, 0 skipped) and red-never-skip without it (exactly one failure, the A6 test).
- No existing test removed or renamed: names in the new `accept_test.py` are a strict superset of the old by diff (the only edits are an import, one added assertion inside the existing capped-rejection test, and one new test); `quota_live_test.py` adds one test (Q14); `quota_driver.py` adds a tamper key only. No product public surface was added or touched. G1 comm -23 over the full lists is a gate-stage check, not run here.
- Quota combined/split raw responses, byte-identical replay after restart and exact-target approval: probed by L1a, L1a2, L1b, L24a, L24b, L3, L3b, L4, L5, X-Q14a (all killed with named tests, re-run this round). The faulted CMake rerun (heavy run) was NOT executed by the prober and only L11 (carried) probes the fault driver.
- A24 applicability and matrix closure: not assessed here; they remain open for R19-ACCEPT-B.
- Raw logs contain scratch host paths (tracebacks); they are kept unredacted as raw evidence and must pass the record's redaction pass before commit. This PROBES.md names no host path.

## Skipped

Extra exploratory budget: 1 new X-row (X-Q14a) this round; the 8 earlier X rows are carried (superseded-1). Not probed: fault/quota driver rows L2, L6, L7, L9, L10, L12-L19 (each needs a live scenario), Q1-Q19 and F1-F25 beyond the L rows, L20 heavy CMake rerun, L21-L23 record rows, G1 gate row, full-suite live baseline re-run, full ctest/Release/Linux.

