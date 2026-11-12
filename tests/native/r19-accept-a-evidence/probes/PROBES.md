# R19-ACCEPT-A mutation probes (prober pass 1)

Run 2026-11-11. Mutants were applied only in the warm scratch copy (probe-warm, synced from the checkout with the prescribed rsync; its build/ kept). Every mutant: exact-string patch (each old string occurred once), PYTHONDONTWRITEBYTECODE=1, PLANAR_CANDIDATE unset, whole `accept_test.AcceptLibTest` (35 tests) run, plus `acceptance_cli_test.AcceptanceCliTest` (7 tests) for run_acceptance.py mutants; file restored by byte copy and sha256 verified (all restored=true, recorded in each log). Scratch files were byte-identical to the checkout copies afterwards; the checkout and build/ were not touched. No product C++ is involved, so no CMake targets were rebuilt (all probed code is Python).

## Candidate identity (identity.py, pathspec tests/native/r19-accept-support)

| identity | value |
|---|---|
| HEAD | `91e0df24f2167471e49ec057c7159bce5dff3608` |
| sha256(git diff --binary HEAD -- tests/native/r19-accept-support), tracked only | `e3b0c44298fc1c149afbf4c8996fb92427ae41e4649b934ca495991b7852b855` |
| combined sha256 (diff bytes, then each untracked path NUL bytes, sorted), pathspec tests/native/r19-accept-support, evidence excluded | `9ee1c9579677aec7dcbeebe5d81ed74dfb4ee66e362a23f821ea7639b5f317db` |

| file | state | sha256 |
|---|---|---|
| `tests/native/r19-accept-support/accept_lib.py` | untracked | `9809217494dc27ebcfdf4728df83076a303aefb473de79ba6488b8d0099f6f16` |
| `tests/native/r19-accept-support/accept_test.py` | untracked | `ed4d8dd0b4675d8471d6871a5570b0cee340ec89c383150b51f260d765c63459` |
| `tests/native/r19-accept-support/acceptance_cli_test.py` | untracked | `5eb98e86d53bfe3a3bbb6545021db180bc0ccd61eddbf4102588e4c84509505a` |
| `tests/native/r19-accept-support/check_runner.py` | untracked | `1c05d3473f4951d660cd7c1033ba4c0dacf6c12871502c4d066a6cba4b7b9b65` |
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
| `tests/native/r19-accept-support/fault_live_test.py` | untracked | `5f43287b412be23b8036318c0b85588d436eca1e8b87d38be12e4ff7e0c3b016` |
| `tests/native/r19-accept-support/quota_driver.py` | untracked | `87eef12106c5b3cdd46d16174fd8f0e58b7f25f817267ee14530ac8e388294f0` |
| `tests/native/r19-accept-support/quota_live_test.py` | untracked | `1bb70239dfe4968fc780e775c472b6cefa59873551d30b79389ac5acf07ccecb` |
| `tests/native/r19-accept-support/registry_gen.py` | untracked | `15b79f6704c72e5f7342e45c5185ba042588b03972af590d3580b613314c37ca` |
| `tests/native/r19-accept-support/run_acceptance.py` | untracked | `17de0ed1223fb75efab68598e25634afbdcb0ba543c3124ce899e4b67b26e148` |


## Scope and limits

- Live rows (Q1-Q19, F1-F25, A6) need PLANAR_CANDIDATE, the D28 scratch-built candidate binary. None exists (scratchpad r19-candidate holds only build scripts and logs), so their mutants (quota_driver.py / fault_driver.py / run_acceptance.py live call sites, check_runner counts through the live registry) were NOT run. They are listed under skipped.
- Baseline in the unmodified checkout copy: 2 failures in AcceptLibTest, `test_cmake_adapter_rendezvous_after_inventory_before_ctest` (I25) and `test_cmake_adapter_never_writes_bytecode_into_support_tree` (I35), and 1 in the CLI class (A6, needs the candidate). I25/I35 fail because `support_copy()` copies tests/native/r19-cmake-support, which contains no cmake_check.py or accept_lib.py, then executes `support/cmake_check.py` (exit 2 'can't open file'). FINDING (test defect): these two rows are red on the real tree and cannot kill anything as written. To measure them anyway, the harness used a scratch-adjusted layout (copies of cmake_check.py and accept_lib.py placed in the scratch r19-cmake-support, re-mirrored after each mutation); with that layout the baseline is 35/35 green. The kills of I25, I25b, I30 (secondary) and I35 below are valid only in that adjusted layout. The same adjustment makes mutants of cmake_check.py/accept_lib.py reach the copied adapter.
- A kill requires a failing test other than the baseline-red A6 test; a mutant failing only A6 counts as a survivor.

## Mutant results

Kind: table = the INVARIANTS.md mutant as written (first variant), table-variant = another reading of the same row's mutant text, extra = exploratory (18 of the 20 allowed). Hash = sha256 of the unified diff (full diff in the raw log).

| id | kind | file | mutant | diff sha256 | killed | failing tests (excluding baseline A6) | row's named test |
|---|---|---|---|---|---|---|---|
| I1 | table | accept_lib.py | compare only len() | `e494270acb3fcdb19760385759b329749603d079a0540159ea6840cb59dbeda9` | KILLED | test_replay_requires_identical_bytes_and_unchanged_ledger | test_replay_requires_identical_bytes_and_unchanged_ledger |
| I1b | table-variant | accept_lib.py | skip ledger digest compare | `fd1852887e299714fd1b8526ee944d27cfe231332275c801b023e86423c909d1` | KILLED | test_replay_requires_identical_bytes_and_unchanged_ledger | test_replay_requires_identical_bytes_and_unchanged_ledger |
| I2 | table | accept_lib.py | ignore -wal/-shm siblings | `b40bd53a111d00099852b610030283a8243309a94796fe8180e5dddf88d78b97` | KILLED | test_ledger_digest_detects_single_byte_write_and_sidecars | test_ledger_digest_detects_single_byte_write_and_sidecars |
| I2b | table-variant | accept_lib.py | ignore tail byte | `8e765550fff8537dd2172e7e698e22fac3ef61b5756e8b4de927f720bb803c6c` | KILLED | test_ledger_digest_detects_single_byte_write_and_sidecars | test_ledger_digest_detects_single_byte_write_and_sidecars |
| I3 | table | accept_lib.py | skip subdirectories | `83130f53002c534c17549243f46f42105560e6e6001fd47b4a7cdc25935525fa` | KILLED | test_host_path_scan_finds_repo_home_scratch_in_nested_files | test_host_path_scan_finds_repo_home_scratch_in_nested_files |
| I3b | table-variant | accept_lib.py | skip JSON decoded value search | `a79bbd53cc85ef3940a2080d11d04bc1e57e10a90bbb488b6ab2c6c9295957ab` | SURVIVED | none | test_host_path_scan_finds_repo_home_scratch_in_nested_files |
| I4 | table | accept_lib.py | omit scratch-root substitution | `71e8dd5759177fcdc49e0af420e30376f06917e7ad9e98f88cc8665c96b2f107` | KILLED | test_redaction_round_trips_with_recorded_hash | test_redaction_round_trips_with_recorded_hash |
| I4b | table-variant | accept_lib.py | record redacted bytes as original hash | `eae2ae92630bf5064457871c6b69c561c50e8e5655d3f09cf76bc327d28519e4` | KILLED | test_redaction_round_trips_with_recorded_hash | test_redaction_round_trips_with_recorded_hash |
| I4c | table-variant | accept_lib.py | write real prefix into rules | `d61be9306d52fe249336e329dd19e2f3e14cfc84aacf9d85c5b9ffdbd1303cc2` | KILLED | test_redaction_round_trips_with_recorded_hash | test_redaction_round_trips_with_recorded_hash |
| I5 | table | accept_lib.py | admit docs/** | `cb0f41c17fa20e2a031728d913d8c095e9d10046b4ed83f42d4082a6a74f1d35` | KILLED | test_pin_scope_admits_only_src_and_data | test_pin_scope_admits_only_src_and_data |
| I5b | table-variant | accept_lib.py | drop fixtures/render/quota admission | `732a2743ee7f093baba2ba73c89f76382ec2ff195fe3b612e2097847cef3dc14` | KILLED | test_pin_scope_admits_only_src_and_data | test_pin_scope_admits_only_src_and_data |
| I5c | table-variant | accept_lib.py | drop r19-cmake-support/workspace/*.lua admission | `2aaf4b78bff25289297190f2e012bfb3847f17fa4baa4739453df9e24ac00b10` | KILLED | test_pin_scope_admits_only_src_and_data | test_pin_scope_admits_only_src_and_data |
| I6 | table | accept_lib.py | skip no-token check | `278e231bc4cd510dcde4b90611856bb4e82116d38a41a727be1e2573ccdbf365` | KILLED | test_event_expansion_binds_declaration_and_dispatch_references | test_event_expansion_binds_declaration_and_dispatch_references |
| I6b | table-variant | accept_lib.py | skip dispatch/manifest id comparison | `f8642a86dcffb2bfd33de283519e0afda34280157a33a8ccd736dfe5a62ffac4` | KILLED | test_event_expansion_binds_declaration_and_dispatch_references | test_event_expansion_binds_declaration_and_dispatch_references |
| I7 | table | accept_lib.py | ignore reservation id and outcome | `ce3e9769f3843ae62f2e379ade101702731a421895daf7458d128252fddd5f91` | KILLED | test_event_expansion_binds_check_state_references | test_event_expansion_binds_check_state_references |
| I8 | table | accept_lib.py | ignore targets/policy | `f66e0467390ed7ed8cae75c298ed780232e9cf2337d379f37839608c34f02d59` | KILLED | test_event_expansion_binds_verdict_targets_policy_blockers | test_event_expansion_binds_verdict_targets_policy_blockers |
| I8b | table-variant | accept_lib.py | ignore blockers | `fab091a166afaf06abe73e5d3f3e86cfc4b3404a83db875a6f48ab4ececbaa58` | KILLED | test_event_expansion_binds_verdict_targets_policy_blockers | test_event_expansion_binds_verdict_targets_policy_blockers |
| I9 | table | accept_lib.py | skip per-role compare | `dec69b993d8388ed4b165a4893551b3f49826d966bea6c716e8d2b9c4a3599fe` | KILLED | test_at_event_state_checks_roles_and_runs | test_at_event_state_checks_roles_and_runs |
| I9b | table-variant | accept_lib.py | compare only active_job (drop activation) | `17dd6e987a63a87418f7858f00e8fb24309744c23586124d0d717b0a497ec140` | KILLED | test_at_event_state_checks_roles_and_runs | test_at_event_state_checks_roles_and_runs |
| I9c | table-variant | accept_lib.py | compare only role state | `d027c6e69fb95d921b76ca003b59d2c9316860bd6196de7ac50a93724ff92a4e` | KILLED | test_at_event_state_checks_roles_and_runs | test_at_event_state_checks_roles_and_runs |
| I10 | table | accept_lib.py | drop check-recover requirement | `337f8a65e9bb136881616ac5ae8526acdbebbc48aa22972d48904d275154f02c` | SURVIVED | none | test_trajectory_requires_interrupted_check_recovery |
| I10b | table-variant | accept_lib.py | allow completion for interrupted reservation | `784d6eb38508195aee3fc452e19060207ae3360dddb45aa070b3c80d929b50b7` | KILLED | test_trajectory_requires_interrupted_check_recovery | test_trajectory_requires_interrupted_check_recovery |
| I10c | table-variant | accept_lib.py | drop check_interrupted requirement | `29a4dbcf7f12ca2cbc53bb79ed165d386d899b0ebd5a4e2e4996dd69459dbcc6` | SURVIVED | none | test_trajectory_requires_interrupted_check_recovery |
| I11 | table | accept_lib.py | drop fenced requirement | `c4141f9ef3db5348a1c2b6b698001feb6282d2f9fe6c333225d1de757248d4d3` | KILLED | test_trajectory_requires_takeover_and_fencing | test_trajectory_requires_takeover_and_fencing |
| I11b | table-variant | accept_lib.py | drop takeover requirement | `fb290383045b4fb34d6cafe8857dce2817a4230f237e060d66750dae5e8c1c28` | KILLED | test_trajectory_requires_takeover_and_fencing | test_trajectory_requires_takeover_and_fencing |
| I12 | table | accept_lib.py | skip repair_admitted | `5649c8713051312f895ed8f704beeb9a9c3f4c880e13fc2e83fb464d69901435` | SURVIVED | none | test_trajectory_requires_rejection_then_repair |
| I12b | table-variant | accept_lib.py | worker need not return | `c398ddeea79fc162d0fa2aeac8828104eb9ef2f4ca9885ee38c9aaa77cfbef9b` | KILLED | test_trajectory_requires_rejection_then_repair | test_trajectory_requires_rejection_then_repair |
| I12c | table-variant | accept_lib.py | admit rejection regardless of count<max_rounds | `d88c469a7d4315630f30f073a409fef8e573e35053deb498d7f0b88b14250753` | SURVIVED | none | test_trajectory_requires_rejection_then_repair |
| I13 | table | accept_lib.py | ignore blockers_opened | `b2e97042c5baea2314f9f3ba8d1e8eea2ee7f14ee3302dc121609ca5e1e86f17` | KILLED | test_trajectory_requires_capped_rejection_and_max_rounds_blocker | test_trajectory_requires_capped_rejection_and_max_rounds_blocker |
| I13b | table-variant | accept_lib.py | capped treated as admitted-or-capped | `cd6d41f530b21a604cca7c89511e1f6fa86f3152014539a4305d7b941d1d4437` | SURVIVED | none | test_trajectory_requires_capped_rejection_and_max_rounds_blocker |
| I14 | table | accept_lib.py | accept missing blockers_cleared | `4ca9555eb5fa4b36184dc8b1455689e1afcd1f6edd89ec83cf10c5961ffa51bc` | KILLED | test_trajectory_requires_blocker_cleared_by_load | test_trajectory_requires_blocker_cleared_by_load |
| I14b | table-variant | accept_lib.py | clear without declaration_accepted | `d503bb695ae51ec892c56285ad555ff8978c15256abdfeaf608c894a4de88b0a` | KILLED | test_trajectory_requires_blocker_cleared_by_load | test_trajectory_requires_blocker_cleared_by_load |
| I15 | table | accept_lib.py | accept approval of older output | `1756eb7e06e2851d6c3ddad37b06d0b88d78b7538428b72d7246676128d6f9c4` | KILLED | test_trajectory_requires_exact_target_final_approval | test_trajectory_requires_exact_target_final_approval |
| I15b | table-variant | accept_lib.py | accept open blockers | `bee326c39fae6c7fd8cbedd61814a471bfc89d4fc4365d7479e72e9c55701b97` | KILLED | test_trajectory_requires_exact_target_final_approval | test_trajectory_requires_exact_target_final_approval |
| I15c | table-variant | accept_lib.py | allow check after final approval | `a82a62c6e4e52707c0d30c96f612b92dab916818dcea4d99d9a1dbe517733c93` | KILLED | test_trajectory_requires_exact_target_final_approval | test_trajectory_requires_exact_target_final_approval |
| I16 | table | accept_lib.py | count only debug preset | `c81da6c5cfd905638f418b9b47fab3297377121a3b585c0abc34a780027232cb` | KILLED | test_reexecution_counter_counts_all_attempt_folders | test_reexecution_counter_counts_all_attempt_folders |
| I16b | table-variant | accept_lib.py | ignore counter file | `7af7e10bc4aa25fb808178890ae0e23f34296ff35b4cd707ace4669fb3552b5e` | KILLED | test_reexecution_counter_counts_all_attempt_folders | test_reexecution_counter_counts_all_attempt_folders |
| I16c | table-variant | accept_lib.py | count only one attempt per preset | `20e28319fe131292b2a71d1579d062245847d60f5585d41430790efb2cabc8d7` | KILLED | test_reexecution_counter_counts_all_attempt_folders | test_reexecution_counter_counts_all_attempt_folders |
| I17 | table | cmake_check.py | tolerate skipped/notrun | `1b5fa2c26331ba9aa98eb4bc9b103cca0e3a193faea5bb388f8812df0955672a` | KILLED | test_cmake_adapter_inventory_rejects_skips_missing_and_duplicates | test_cmake_adapter_inventory_rejects_skips_missing_and_duplicates |
| I17b | table-variant | cmake_check.py | tolerate missing names | `2cd1fed89ac9ef1c18d1b0500bcfe911c2bc006f5e3d90ebfd6e05cc0ec28f6d` | KILLED | test_cmake_adapter_inventory_rejects_skips_missing_and_duplicates | test_cmake_adapter_inventory_rejects_skips_missing_and_duplicates |
| I17c | table-variant | cmake_check.py | tolerate duplicate inventory | `f5ec2536f4764bcd51d3e663458ed3ee2ae9528f3cb887b5cd55d2ecfff9be49` | SURVIVED | none | test_cmake_adapter_inventory_rejects_skips_missing_and_duplicates |
| I18 | table | cmake_check.py | drop lint | `0917096a4fd48b2c2dd0b2403a89054823a4d84a2ddbaa7c515bafb55fe43229` | KILLED | test_cmake_adapter_command_plan_drops_equivalence_keeps_gates | test_cmake_adapter_command_plan_drops_equivalence_keeps_gates |
| I18b | table-variant | cmake_check.py | drop repeat-build | `4cddd8b7b16a4b6be61f1166bb7b8bf488b588c8ba8da68d90806fbdcb7f1135` | KILLED | test_cmake_adapter_command_plan_drops_equivalence_keeps_gates | test_cmake_adapter_command_plan_drops_equivalence_keeps_gates |
| I18c | table-variant | cmake_check.py | drop rendezvous | `a8fe5cad0e145592e43ad507542ee1a9b3449aa675b06b05a6a0a0a4009e8e1c` | KILLED | test_cmake_adapter_command_plan_drops_equivalence_keeps_gates | test_cmake_adapter_command_plan_drops_equivalence_keeps_gates |
| I18d | table-variant | cmake_check.py | re-add verify.py equivalence step | `781aef34821ee308990dc9a7d4d8a51a63504e325a882d0a82433352df5b5412` | KILLED | test_cmake_adapter_command_plan_drops_equivalence_keeps_gates | test_cmake_adapter_command_plan_drops_equivalence_keeps_gates |
| I19 | table | accept_lib.py | policy from worker claim | `7dbe76cea520fb9b9dfd092df6237c2c7ef1065160803b432d1746dbbfcde52e` | KILLED | test_verdict_document_binds_exact_targets_and_reviewer_policy | test_verdict_document_binds_exact_targets_and_reviewer_policy |
| I19b | table-variant | accept_lib.py | targets from claim | `6e57e74185fe49679be559132b353a940fe371ae1fcbebf0cc837a4aebe3f5e5` | KILLED | test_verdict_document_binds_exact_targets_and_reviewer_policy | test_verdict_document_binds_exact_targets_and_reviewer_policy |
| I20 | table | accept_lib.py | True for any nonempty text | `0f969c6c05f94e2f6266a9e0b0dd88d58a070b8c6e0f1edb406dcca6e1bee8af` | KILLED | test_guidance_usable_predicate_rejects_empty_and_generic | test_guidance_usable_predicate_rejects_empty_and_generic |
| I20b | table-variant | accept_lib.py | ignore job name | `b14717f203787a2260fe316b01a479dc2bf57f6a42a900c769399f231cd9a561` | KILLED | test_guidance_usable_predicate_rejects_empty_and_generic | test_guidance_usable_predicate_rejects_empty_and_generic |
| I20c | table-variant | accept_lib.py | ignore role states | `dd48568e92d397ac74474f8689041d8aadd22e82ed5efd28919f281fb41243c3` | KILLED | test_guidance_usable_predicate_rejects_empty_and_generic | test_guidance_usable_predicate_rejects_empty_and_generic |
| I21 | table | accept_lib.py | accept any resource_limit | `2cc1bb03e6684cfc61615259270ae6eb858e8af60e5358b5847bcf4ae5ddd473` | KILLED | test_carried_capture_limit_requires_exact_closed_diagnostic | test_carried_capture_limit_requires_exact_closed_diagnostic |
| I21b | table-variant | accept_lib.py | accept any maximum/limit details | `5795f933d5b2743f4abb026626afd7e6341c830a2c4fdc4d9402a986d18246ad` | KILLED | test_carried_capture_limit_requires_exact_closed_diagnostic | test_carried_capture_limit_requires_exact_closed_diagnostic |
| I21c | table-variant | accept_lib.py | accept nonempty stdout | `deeefd0f3276f9bc6d4b373796ab3f4304cc2b14360a5007be032a40c6c72983` | KILLED | test_carried_capture_limit_requires_exact_closed_diagnostic | test_carried_capture_limit_requires_exact_closed_diagnostic |
| I22 | table | accept_lib.py | ignore untracked | `7530052570d13060b1144aa925ed345e9816e8ec937baefede1c19e827c567b0` | KILLED | test_product_change_guard_sees_tracked_and_untracked_src_edits | test_product_change_guard_sees_tracked_and_untracked_src_edits |
| I23 | table | accept_lib.py | omit stdlib lua pattern | `bbdd5eb2ab9d2093fb4b58b4aa876f81f22bcd9f1481a943f94113c4e227883f` | KILLED | test_repin_rule_flags_runtime_cpp_and_embedded_lua | test_repin_rule_flags_runtime_cpp_and_embedded_lua |
| I23b | table-variant | accept_lib.py | omit runtime.cpp | `e17e0d9e928e402af89dddd7590c7d29769bc4980806cdcd76562fe8bc14b695` | KILLED | test_repin_rule_flags_runtime_cpp_and_embedded_lua | test_repin_rule_flags_runtime_cpp_and_embedded_lua |
| I24 | table | accept_lib.py | accept nonzero without assertion marker | `73b5e65841a7c77415e18ca0044a9b2c9519c1ee0667185096e3789f493d130d` | SURVIVED | none | test_red_record_must_be_behavioural_and_hashed |
| I24b | table-variant | accept_lib.py | accept compile/import error | `506aed4757328eb0390ca86fdcc63528e2acccc4056cde95199a9d1c2b41fe74` | SURVIVED | none | test_red_record_must_be_behavioural_and_hashed |
| I24c | table-variant | accept_lib.py | accept missing snapshot hash | `055e0e58cc589483ac7d65d72c5616e2ce98148c28fd28ddae1d6ea503bd8923` | KILLED | test_red_record_must_be_behavioural_and_hashed | test_red_record_must_be_behavioural_and_hashed |
| I25 | table | cmake_check.py | marker written before build (scratch-adjusted layout) | `009bf6820abfe3b58fb4a03c6666850f1d2569cf54d3f6750035925449a1d322` | KILLED | test_cmake_adapter_rendezvous_after_inventory_before_ctest | test_cmake_adapter_rendezvous_after_inventory_before_ctest |
| I25b | table-variant | cmake_check.py | pid recorded as pgid | `92a5a33d2e302bb566c8d67be641a313618bea87b6db32ee44f6b2d74e58ced8` | KILLED | test_cmake_adapter_rendezvous_after_inventory_before_ctest | test_cmake_adapter_rendezvous_after_inventory_before_ctest |
| I26 | table | cmake_check.py | True for any build output | `45492e629b1756e9a3c62b081a3e142bab47b23bcf353a249933067f9fba0aad` | KILLED | test_cmake_adapter_post_kill_integrity_requires_noop_build | test_cmake_adapter_post_kill_integrity_requires_noop_build |
| I27 | table | accept_lib.py | owned files absent in old tree not removed | `fce41430a220a5e883e1045904d07e7d0506326521a28b26bbfa911df3150e3f` | KILLED | test_compose_base_diff_touches_only_owned_cmake_paths | test_compose_base_diff_touches_only_owned_cmake_paths |
| I27b | table-variant | accept_lib.py | boundary_paths_ok accepts any path | `c1644d96ac2a79acd01bd38aad9e08c108128d10029512219f47586d6a8c760f` | KILLED | test_compose_base_diff_touches_only_owned_cmake_paths | test_compose_base_diff_touches_only_owned_cmake_paths |
| I28 | table | check_runner.py | passed=matched | `4295d46dff264a4652e116ecb8a8d0abc8ea2bbc03bbc46b72d3a44587bf7632` | KILLED | test_check_runner_derives_honest_result_from_unittest_child | test_check_runner_derives_honest_result_from_unittest_child |
| I28b | table-variant | check_runner.py | skips counted as passed | `1335dad54df42f47c07ccb83b118317bdc84072d9d663175be8027b58dfe4bde` | KILLED | test_check_runner_derives_honest_result_from_unittest_child | test_check_runner_derives_honest_result_from_unittest_child |
| I29 | table | check_runner.py | exit 0 regardless | `2fdf38024b623da538f200e26cc2fb097e8445682131f78d607498d2d0c784ff` | KILLED | test_check_runner_propagates_exit_counts_once_and_passthrough_keeps_result | test_check_runner_propagates_exit_counts_once_and_passthrough_keeps_result |
| I29b | table-variant | check_runner.py | passthrough writes result too | `17e3ce255f4c96b4622bbbd591aaf5fc68a0aee8eddd2cd52a56b7e1aee61889` | SURVIVED | none | test_check_runner_propagates_exit_counts_once_and_passthrough_keeps_result |
| I30 | table | accept_lib.py | pgid under other key | `03f77e5de2cebf8ddb0fe1c3420006092d1b3c17308f82a5cf996ca48ee93b0e` | KILLED | test_cmake_adapter_rendezvous_after_inventory_before_ctest, test_rendezvous_pair_round_trips_atomically_and_rejects_partial | test_rendezvous_pair_round_trips_atomically_and_rejects_partial |
| I30b | table-variant | accept_lib.py | read accepts missing keys | `1ada42a8c397bf7c9671858033bf0e3010d3a41debade21a2ab29d5891088525` | KILLED | test_rendezvous_pair_round_trips_atomically_and_rejects_partial | test_rendezvous_pair_round_trips_atomically_and_rejects_partial |
| I30c | table-variant | accept_lib.py | non-atomic in-place write | `c388fccd0435cdf31a4ef52119914aeb0506785149a4e9b7b0e9c3ac233ab49a` | SURVIVED | none | test_rendezvous_pair_round_trips_atomically_and_rejects_partial |
| I31 | table | registry_gen.py | drop toolchain environment | `91cd4d57fdd8392979a7463e4f9171124c389c5a344e671d82f570a4756d2db5` | KILLED | test_registry_gen_changes_only_toolchain_command_prefix | test_registry_gen_changes_only_toolchain_command_prefix |
| I31b | table-variant | registry_gen.py | alter a check timeout | `e7e41e42c696eeed8f0d2fd12383730931f38398b7d7450acb052fb9faec7519` | KILLED | test_registry_gen_changes_only_toolchain_command_prefix | test_registry_gen_changes_only_toolchain_command_prefix |
| I32 | table | accept_lib.py | drop shared-build-dir refusal | `fd7e479405761ad07c3c893d829ae6492694d4a625f362601e6bcfc803eea2c5` | KILLED | test_candidate_record_hashes_bytes_and_refuses_shared_build_dirs | test_candidate_record_hashes_bytes_and_refuses_shared_build_dirs |
| I32b | table-variant | accept_lib.py | hash a fixed string | `ddbfc8df44309db73ceb75306c4bf0323c82545fc11c24877061e80bae4ff254` | KILLED | test_candidate_record_hashes_bytes_and_refuses_shared_build_dirs, test_recorded_candidate_verification_rejects_mismatch | test_candidate_record_hashes_bytes_and_refuses_shared_build_dirs |
| I33 | table | accept_lib.py | compare only head | `1e68b27a1a7dffb1730de66a5b43a9055ab6949f5a9c2bcfe1d9145ce9a0d230` | KILLED | test_recorded_candidate_verification_rejects_mismatch | test_recorded_candidate_verification_rejects_mismatch |
| I34 | table | accept_lib.py | accept verb absent from guidance | `35c584408fb672c094570518a5eb694ce01cfe8ffc4cda936c9854f5b0efcfe5` | KILLED | test_require_legal_action_rejects_verb_absent_from_guidance | test_require_legal_action_rejects_verb_absent_from_guidance |
| I35 | table | cmake_check.py | cmake_check enables bytecode writing before import of support (scratch-adjusted layout) | `2e71f146b0c878b8db5dedf7dff96c318ab22762ff85e8d58e6ea97ac69bb75d` | KILLED | test_cmake_adapter_never_writes_bytecode_into_support_tree | test_cmake_adapter_never_writes_bytecode_into_support_tree |
| I35b | table-variant | accept_lib.py | child env drops PYTHONDONTWRITEBYTECODE | `0c68fdf077f6546d356440bab1f14234b7ae839e77cff6a57c5a5a109bb0edde` | SURVIVED | none | test_cmake_adapter_never_writes_bytecode_into_support_tree |
| I10d | table-variant | accept_lib.py | table I10 combined: drop check_interrupted requirement AND via check-recover | `3823af53af7b052e67eb82a7ad030c62b51a36120f37f53e1c9e0f57a96d7f02` | KILLED | test_trajectory_requires_interrupted_check_recovery | test_trajectory_requires_interrupted_check_recovery |
| I12d | table-variant | accept_lib.py | accept an approve as the first verdict | `c9c94aeeefebf7ce446d2c480facfd42638e5b8f7bfdfb2f3387d0106dfb7eb9` | SURVIVED | none | test_trajectory_requires_rejection_then_repair |
| E1 | extra | run_acceptance.py | extra: scan exits 0 despite hits (A1) | `593a11cc41f5be0418a90a413ea40326ddd252d8cdefbbf540cfa74b0869769a` | KILLED | test_scan_command_fails_on_host_path_in_committed_evidence | test_scan_command_fails_on_host_path_in_committed_evidence |
| E2 | extra | run_acceptance.py | extra: scan omits HOME prefix (A1) | `38e57e851e61fe6c2df7a9034bfd13f4e6a878e62e4e58e445fccbe536a27b9a` | KILLED | test_scan_command_fails_on_host_path_in_committed_evidence | test_scan_command_fails_on_host_path_in_committed_evidence |
| E3 | extra | run_acceptance.py | extra: retained dir reuses an existing run (A5) | `35b2d4be82deeba614e8cae135066ca1c293ae108d353a58ea0bffb3b2142349` | KILLED | test_retained_dir_never_overwrites_existing_runs | test_retained_dir_never_overwrites_existing_runs |
| E4 | extra | run_acceptance.py | extra: limits records any outcome (A3) | `2ec5b16514ff2b6c3fbe7f60eb8493f88cac9c89029ab3207070449a4d5bb250` | KILLED | test_limits_command_requires_exact_closed_diagnostic | test_limits_command_requires_exact_closed_diagnostic |
| E5 | extra | run_acceptance.py | extra: packet records without the marker (A4) | `977e3df394e257f7d5aec31321264abaa0a6414a1c19a14da5e01e4d2b45d3ca` | KILLED | test_packet_command_records_literal_source_lines | test_packet_command_records_literal_source_lines |
| E6 | extra | run_acceptance.py | extra: index skips residual host-prefix scan (A2) | `031b7c533c94ec1e4d6c16572398b6f04a9f14a38cf6300403880dbef1188efd` | SURVIVED | none | test_index_is_redacted_with_recorded_hashes_and_excludes_raw |
| E7 | extra | run_acceptance.py | extra: index skips standard redaction tool (A2) | `66d2e1f4d266a226341ab9c84c74dd84ce3b035d390e836411e8c4392ec7904f` | KILLED | test_index_is_redacted_with_recorded_hashes_and_excludes_raw, test_verify_redaction_runs_standard_tool_and_rejects_scratch_tokens_in_manifest | test_index_is_redacted_with_recorded_hashes_and_excludes_raw |
| E8 | extra | run_acceptance.py | extra: verify-redaction skips original-hash check (A7) | `e06cd0ce4721efe4fe052db4a53a82bd775a4c076c5199a535b393d77d6d7441` | SURVIVED | none | test_verify_redaction_runs_standard_tool_and_rejects_scratch_tokens_in_manifest |
| E9 | extra | run_acceptance.py | extra: verify-redaction ignores standard verify exit (A7) | `8950613d1988ff768f5e95a4d100d9a425f69f6f0203cbb1f4da556f38defd73` | KILLED | test_verify_redaction_runs_standard_tool_and_rejects_scratch_tokens_in_manifest | test_verify_redaction_runs_standard_tool_and_rejects_scratch_tokens_in_manifest |
| E10 | extra | run_acceptance.py | extra: manifest real-prefix check removed (A7) | `33e32b81232802f6baf263cb3c0d20ee8572aaa4d3e8d67e698d87b7e6926e72` | SURVIVED | none | test_verify_redaction_runs_standard_tool_and_rejects_scratch_tokens_in_manifest |
| E11 | extra | run_acceptance.py | extra: TEMP rules real-prefix check removed (A7) | `9e1ec6b14c7ebdffe9169c109a222bc42daecf94198e6579e4f10b8aa9d292c6` | SURVIVED | none | test_verify_redaction_runs_standard_tool_and_rejects_scratch_tokens_in_manifest |
| E12 | extra | run_acceptance.py | extra: index writes unredacted text (A2) | `f8b7aba28354f903ba98340674fda0617e807994a5dc7df4cb8ea58c65b492cd` | KILLED | test_index_is_redacted_with_recorded_hashes_and_excludes_raw, test_verify_redaction_runs_standard_tool_and_rejects_scratch_tokens_in_manifest | test_index_is_redacted_with_recorded_hashes_and_excludes_raw |
| E13 | extra | accept_lib.py | extra: duplicate target unit accepted | `fbeedc5ffbdc174c42ceb508beb3dda7cef8c67c6f3cf5a395a1d1c78becb56f` | SURVIVED | none | test_verdict_document_binds_exact_targets_and_reviewer_policy |
| E14 | extra | accept_lib.py | extra: empty expected references accepted (vacuous compare) | `2fd2c5a4fe5a1709b2a6b93a66572922b28d0e79338c8171717487d8a2b95965` | SURVIVED | none | - |
| E15 | extra | accept_lib.py | extra: empty scan prefix accepted | `d8faec719a980803ed05a131d0a7b55d4aeea4f3766700c46227990685e1f678` | SURVIVED | none | - |
| E16 | extra | accept_lib.py | extra: redaction shortest prefix first (nested prefixes) | `8aebf8ea7444d65baf156dcba6baeee3358cea4bf3d2f3ee9baf2b6195ba77bc` | KILLED | test_redaction_round_trips_with_recorded_hash | - |
| E17 | extra | accept_lib.py | extra: missing roles mapping accepted | `cedfaeab06304368c9ffbc637aad62ee385661a11e93c7e9dc8775c47fdb201e` | SURVIVED | none | - |
| E18 | extra | accept_lib.py | extra: role set mismatch accepted | `62d6789b87a6664108aaf89a9df2ac377c1579c44d879e40365b0596ad6977d4` | SURVIVED | none | - |

All rows are 'run' (none carried from an earlier round; no probes existed for this round, so no superseded-<n> directory was needed).

## Survivors (stay survivors)

| id | file | mutant | row test |
|---|---|---|---|
| I3b | accept_lib.py | skip JSON decoded value search | test_host_path_scan_finds_repo_home_scratch_in_nested_files |
| I10 | accept_lib.py | drop check-recover requirement | test_trajectory_requires_interrupted_check_recovery |
| I10c | accept_lib.py | drop check_interrupted requirement | test_trajectory_requires_interrupted_check_recovery |
| I12 | accept_lib.py | skip repair_admitted | test_trajectory_requires_rejection_then_repair |
| I12c | accept_lib.py | admit rejection regardless of count<max_rounds | test_trajectory_requires_rejection_then_repair |
| I13b | accept_lib.py | capped treated as admitted-or-capped | test_trajectory_requires_capped_rejection_and_max_rounds_blocker |
| I17c | cmake_check.py | tolerate duplicate inventory | test_cmake_adapter_inventory_rejects_skips_missing_and_duplicates |
| I24 | accept_lib.py | accept nonzero without assertion marker | test_red_record_must_be_behavioural_and_hashed |
| I24b | accept_lib.py | accept compile/import error | test_red_record_must_be_behavioural_and_hashed |
| I29b | check_runner.py | passthrough writes result too | test_check_runner_propagates_exit_counts_once_and_passthrough_keeps_result |
| I30c | accept_lib.py | non-atomic in-place write | test_rendezvous_pair_round_trips_atomically_and_rejects_partial |
| I35b | accept_lib.py | child env drops PYTHONDONTWRITEBYTECODE | test_cmake_adapter_never_writes_bytecode_into_support_tree |
| I12d | accept_lib.py | accept an approve as the first verdict | test_trajectory_requires_rejection_then_repair |
| E6 | run_acceptance.py | extra: index skips residual host-prefix scan (A2) | test_index_is_redacted_with_recorded_hashes_and_excludes_raw |
| E8 | run_acceptance.py | extra: verify-redaction skips original-hash check (A7) | test_verify_redaction_runs_standard_tool_and_rejects_scratch_tokens_in_manifest |
| E10 | run_acceptance.py | extra: manifest real-prefix check removed (A7) | test_verify_redaction_runs_standard_tool_and_rejects_scratch_tokens_in_manifest |
| E11 | run_acceptance.py | extra: TEMP rules real-prefix check removed (A7) | test_verify_redaction_runs_standard_tool_and_rejects_scratch_tokens_in_manifest |
| E13 | accept_lib.py | extra: duplicate target unit accepted | test_verdict_document_binds_exact_targets_and_reviewer_policy |
| E14 | accept_lib.py | extra: empty expected references accepted (vacuous compare) | - |
| E15 | accept_lib.py | extra: empty scan prefix accepted | - |
| E17 | accept_lib.py | extra: missing roles mapping accepted | - |
| E18 | accept_lib.py | extra: role set mismatch accepted | - |

Survivor notes:
- I3b: no JSON-escaped-only host path fixture; the decoded-JSON-string search is untested.
- I10 / I10c singly survive because the check-recover `via` check and the interrupted-existence check mask each other; the combined I10d (drop both) is killed. Not an acceptance gap beyond a missing `via != check-recover` negative fixture.
- I12 (skip repair_admitted), I12c (drop count < max_rounds), I12d (accept a first approve as the admitted rejection): the fixtures never give a rejection with a nonempty repair list and repair_admitted false or count >= max_rounds, nor an approve in that slot, so the admission predicate is only partly pinned. I13b: no capped-rejection fixture with repair_admitted true.
- I17c: duplicate names in the live inventory (with matching duplicate cases) are tolerated; the docstring says duplicate-free.
- I24 / I24b: the assertion-marker requirement and the compile/import refusal mask each other (every negative fixture lacks the marker); neither is individually pinned. The row's mutant ('accept a nonzero exit without an assertion marker') survives.
- I29b: passthrough mode also writing a result is not detected.
- I30c: non-atomic in-place rendezvous write is undetectable by a single-process test (the test has no concurrent reader).
- I35b: dropping PYTHONDONTWRITEBYTECODE in child_env is equivalent under the harness because the test environment already exports it.
- E6, E8, E10, E11 (cmd_index residual scan, verify-redaction original-hash check, manifest real-prefix check, TEMP-rules real-prefix check) fail only the baseline-red A6 test: no runnable test pins them.
- E13 (duplicate unit in an observation), E14 (empty expected references), E15 (empty scan prefix), E17 (missing roles mapping), E18 (role set mismatch): unpinned by any test.

## Red-first table (every test of this lineage)

Each AcceptLibTest row's mutant is the row above (I1-I35). Acceptance CLI rows:

| row | test | mutant that fails it | result |
|---|---|---|---|
| A1 | test_scan_command_fails_on_host_path_in_committed_evidence | E1, E2 (also I3) | killed |
| A2 | test_index_is_redacted_with_recorded_hashes_and_excludes_raw | E7, E12 | killed (E6 survived) |
| A3 | test_limits_command_requires_exact_closed_diagnostic | E4 (also I21) | killed |
| A4 | test_packet_command_records_literal_source_lines | E5 | killed |
| A5 | test_retained_dir_never_overwrites_existing_runs | E3 | killed |
| A6 | test_live_commands_abort_before_first_action_on_candidate_mismatch | removing require_candidate / require_recorded_candidate call | NOT RUN (red at baseline without candidate; needs live candidate) |
| A7 | test_verify_redaction_runs_standard_tool_and_rejects_scratch_tokens_in_manifest | E7, E9 | killed (E8, E10, E11 survived) |
Live rows Q1-Q19 and F1-F25: red-first mutants are the table mutants of INVARIANTS.md; not run (no candidate).

## Skipped

- Q1-Q19, F1-F25 table mutants (quota_driver.py, fault_driver.py, live call sites, check_runner through live registry): need the D28 candidate binary, absent. The heavy CMake rerun is not probeable by design.
- Not applied from the exploratory budget: 2 of 20 unused. Ideas not run: check_runner counter-line-before-child (I29 third variant), accept_lib.write_rendezvous extra-key refusal, require_legal_action empty-list refusal.
- Raw logs contain scratch and checkout absolute paths in tracebacks; they are raw evidence (retained, excluded from the committed index) and must not be committed unredacted.
