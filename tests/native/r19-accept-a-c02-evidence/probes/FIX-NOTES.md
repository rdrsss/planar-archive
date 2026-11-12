# R19-ACCEPT-A-C02 PKG-FIX notes (wave 7, survivors only)

Scope: negative fixtures for the survivors of the wave-6 mutation probe. Each fixture is a NEW test in the module of the survivor's row. No existing test was renamed or removed. No product source, no `fault_driver.py`, no `quota_driver.py`, no `check_runner.py` and no `accept_lib.py` was edited, and no mutant was applied by this package (only the prober applies mutants; the probe itself is wave 8).

## Survivor source

- The wave-6 probe `PROBES.md` is not in the checkout (`probes/` did not exist). The survivor list below is taken from the probe's own result records (92 probe records; 15 records marked SURVIVOR, covering 14 distinct mutants because F11-b and L17 carry an identical diff). Each `diff sha256` is the probe's recorded value.
- Records (not committed, raw): the prober's `results.jsonl` in the prober scratch directory. Wave 8 must confirm against the final `PROBES.md`.

| probe id | rows | alt | diff sha256 | outcome | named tests |
|---|---|---|---|---|---|
| F11-b | F11 | b | `d5cab53c0ffc3c0089448886cdc0e9d924595d32fe45c3a89c5d5fec8bdc700a` | SURVIVOR | test_scenario_driver_is_config_parameterized |
| L17 | L17 | - | `d5cab53c0ffc3c0089448886cdc0e9d924595d32fe45c3a89c5d5fec8bdc700a` | SURVIVOR | test_scenario_driver_is_config_parameterized |
| F20 | F20 | - | `99da3c048ad1ce971704569ae0790fc1f60936ab81e061c04e942e4c7a9cd8a2` | SURVIVOR | test_run_scenario_aborts_on_unusable_guidance |
| F21-a | F21 | a | `ef3a55b0297e84cf683ef94f2241b49a5a90e3d31aabb44e445d62a7f7c78067` | SURVIVOR | test_run_heavy_config_is_composed_base_and_owned_repairs |
| F22-a | F22 | a | `23ee6421bbbdd70717d7d8c6b88060e0bb29024efed404aac7db6668e10e9ece` | SURVIVOR | test_kill_check_consumes_marker_written_by_cmake_check |
| Q2-a | Q2 | a | `9af994f964a61ca886eaa277834a2bc2ddfb688d5e6dc900f3290acbe1d24bbc` | SURVIVOR | test_combined_grouping_completes_with_exact_target_approval |
| Q7 | Q7 | - | `d2fe2b0f291a7857a8fcf2cc97c70ca0cdb889e8ee3badb65354452a4c91f054` | SURVIVOR | test_history_event_and_at_event_reproduce_accepted_records |
| Q11-a | Q11 | a | `612ca5810fde545d57da14f02533ce860152a7708b9e1733a718b9b491f8a616` | SURVIVOR | test_fixture_copied_unmodified_and_counter_counts_checks |
| Q12-b | Q12 | b | `7d882c15f99094251eb23c773b9b1617ff430142a6589e3c37ca38a1fb634ea5` | SURVIVOR | test_contract_delivered_to_base_and_every_root |
| Q13-b | Q13 | b | `423be5f8adda77d05777d821cd8215bbb5ff127d3e4e367d3286e945efb7eafc` | SURVIVOR | test_quota_check_results_match_actual_unittest_counts |
| Q14-b | Q14 | b | `254be9fcd1494aa60a3dbb98850fe9a536a31a8c3f6a61800dc178c4847109b9` | SURVIVOR | test_run_grouping_aborts_on_tampered_replay, test_run_grouping_aborts_when_replay_reexecutes_a_check |
| Q15 | Q15,L19 | - | `7a7b0c7d63f3149acf85657135da720173fbb727512f77b5e3874ee0965b2417` | SURVIVOR | test_run_grouping_aborts_on_unusable_guidance |
| L6 | L6 | - | `8bec53585e84290d35ba380f4f24b3571b551c1109abfc561c71ad16686d2b37` | SURVIVOR | test_regroup_combined_to_split_fences_old_tokens |
| L7-a | L7 | a | `d1bcb0cf2e2da8249026ceea043b61279ce2f3dbf115b63593ff4be8faac0071` | SURVIVOR | test_orchestrator_guidance_is_usable_at_every_phase, test_orchestrator_guidance_usable_through_fault_phases |
| L15 | L15 | - | `17c63b81a7cad50399d91d5e593fdd5b6c864ff2d128c32dfa9a119c5159bdac` | SURVIVOR | test_failed_and_interrupted_attempts_remain_retained |

## Negative fixture per survivor

Each docstring names the mutant it kills. "Unfixed" means the current tree (this package adds only tests). Every new test PASSED on the unfixed tree (see Runs).

| survivor | new test (file) | observer that the mutant changes |
|---|---|---|
| F11-b, L17 (constant trace) | `FaultLiveTest.test_trace_is_derived_from_executed_steps_in_plain_cmake_run` (fault_live_test.py) | `trace` must equal the traced verbs derived from the raw executed steps (from the first activation; exit 0, or a SIGKILLed check) in a plain-base cmake run. The plain run stops after its first withheld check, so its real trace (6 entries) differs from the hard-coded 15-entry mini sequence. |
| F20 (usability predicate removed in fault guided) | `FaultLiveTest.test_blank_guidance_is_refused_by_the_usability_predicate` (fault_live_test.py) | The refusal message must be the predicate refusal ("orchestrator guidance is not usable for ..."), not the legal-list refusal that blank text also triggers. |
| F21-a (base plain) | `FaultLiveTest.test_run_heavy_config_names_the_composed_base` (fault_live_test.py) | `config["base"]` must be `composed` (the existing test checks only `base_label`). |
| F22-a (pgid taken from the marker pid) | `FaultLiveTest.test_kill_check_signals_the_marker_pgid_when_it_differs_from_the_marker_pid` (fault_live_test.py) | The marker names the CLI as pid and a separate group leader as pgid (written with `accept_lib.write_rendezvous`); `child_pgid` must be the pgid and that group must be dead. The grandchild is started by a short-lived parent so no zombie of this process keeps its group "alive". |
| L15 (attempt retention dropped) | `FaultLiveTest.test_every_check_attempt_folder_keeps_its_command_logs` (fault_live_test.py) | Every attempt folder must hold `*.argv.json`, `*.stdout`, `*.stderr` and `*.exit`. |
| Q2-a (combined root omits current/src/core) | `QuotaLiveTest.test_combined_builder_root_carries_the_current_overlay_not_the_base_copy` (quota_live_test.py) | The builder root's `src/core/quota.py` (and `src/cli/main.py`) bytes equal the fixture's `current/` bytes. The base copy differs. |
| Q7 (--at-event verified for the latest event only) | `QuotaLiveTest.test_every_history_event_has_a_replayed_state_equal_to_its_recorded_state` (quota_live_test.py) | For every history event, the at-event record has a non-null `replayed` state equal to `recorded`. Keyed on history sequences: the at-event list has two extra records outside history (their `replayed` is null in the unmutated run). |
| Q11-a (checkout fixture bound as the session root) | `QuotaLiveTest.test_session_argv_names_scratch_paths_and_never_the_checkout_fixture` (quota_live_test.py) | No argv token starts with the checkout fixture path; the load argv names the scratch workspace. |
| Q12-b (contract omitted from a root view) | `QuotaLiveTest.test_root_view_places_the_contract_even_when_its_source_lacks_it` (quota_live_test.py) | `Session._root` built from a source without the contract must place `text/quota-api.md` byte-equal to the workspace contract. (Test-only access to the session's helper after `setup()`.) |
| Q13-b (passed = matched) | `AcceptLibGapTest.test_check_runner_passed_excludes_failures_and_skips` (accept_test.py) | `check_runner.unittest_result` on `FAILED (failures=1)` gives passed 2, and on `OK (skipped=1)` gives passed 2 (of 3 matched). |
| Q14-b (counter comparison removed after replay) | `QuotaLiveTest.test_counter_only_replay_tamper_leaves_an_aborted_record_naming_the_replay` (quota_live_test.py) | The counter-only replay tamper must abort and write `ABORTED.json` whose error names "executed a check". |
| Q15 (usability predicate removed after orchestrator capture; also credited to L19) | `QuotaLiveTest.test_unusable_guidance_is_refused_by_the_usability_predicate` (quota_live_test.py) | For tamper `empty` and `generic`, the refusal must be the predicate message ("not usable for next verb"). |
| L6 (fenced probe presents the reviewer token) | `QuotaLiveTest.test_regroup_fenced_probe_presents_the_retired_builder_token` (quota_live_test.py) | The `--run` token of the fenced-probe release must be a builder claim's token (the claim stdout `run.token`) and not a reviewer claim's token. |
| L7-a (predicate always True) | `AcceptLibGapTest.test_guidance_predicate_refuses_text_without_job_role_line_or_verb` (accept_test.py) | `accept_lib.guidance_is_usable` must refuse text without the live job, without a live role line, without the next verb, and empty text. |

Standing notes per fixture:

- F20 and Q15: blank or generic guidance is also refused by `require_legal_action`, which is why the original tests passed under the mutant. The new tests assert the predicate's own message.
- Q14-b: the existing `test_run_grouping_aborts_when_replay_reexecutes_a_check` already FAILS under this mutant in the probe (the row names only `test_run_grouping_aborts_on_tampered_replay`, which cannot see a counter-only change). The new fixture is added as the row's own observer; the existing test was not changed.
- F11-b and L17: one fixture credited to both (identical diff sha256).
- L6: discrimination rests on the recorded claim tokens (reviewer and builder token sets are disjoint in the unmutated run). Not yet observed under the mutant; wave 8 must confirm.
- F22-a: discrimination is by construction (pid differs from pgid); not yet observed under the mutant; wave 8 must confirm.

## Runs on the unfixed tree

Environment: `PYTHONDONTWRITEBYTECODE=1`, `PLANAR_CANDIDATE` set to the recorded C01 candidate (sha256 774bcd69809ead9e4cd7b6e06c74aa045ea9272a2321cd36f91bc9c76a99ab0c, verified before the runs), run from `tests/native/r19-accept-support` with `python3 -B -m unittest`. Serial: no other build, ctest or live run was active.

- New unit tests (accept_test.py, two tests): `python3 -B -m unittest accept_test.AcceptLibGapTest.test_check_runner_passed_excludes_failures_and_skips accept_test.AcceptLibGapTest.test_guidance_predicate_refuses_text_without_job_role_line_or_verb`: Ran 2, OK.
- Full `python3 -B -m unittest accept_test`: Ran 51 tests, OK, zero failures, zero errors, zero skips (includes the two new tests).
- Full `python3 -B -m unittest fault_live_test`: Ran 33 tests, OK, zero skips (28 existing plus five new: trace, F20, F21-a, F22-a, L15).
- Full `python3 -B -m unittest quota_live_test`: Ran 28 tests, OK, zero skips (21 existing plus seven new: Q2-a, Q7, Q11-a, Q12-b, Q14-b, Q15, L6).
- The five new fault tests were run once before the final edit of the trace derivation; the trace test was corrected (the derivation starts at the first activation, since the setup load is not traced) and re-run to pass. The F22 fixture closes its spawner pipe (no ResourceWarning) and then passed in the full module run.
- Probe-side check: `timeout` is not installed on this host; no test was run through it.

## Support allowlist (D26)

Unchanged. `redaction/support-allowlist.txt` was not edited. The users-prefix literal occurs at the same six sites (`run_acceptance.py` 2, `accept_test.py` 3, `acceptance_cli_test.py` 1); the new test lines contain no host path, users-prefix or temp-root literal.

## Identity (verbatim output of identity.py src tests/native/r19-accept-support, run after the edits)

| identity | value |
|---|---|
| HEAD | `91e0df24f2167471e49ec057c7159bce5dff3608` |
| sha256(git diff --binary HEAD -- src tests/native/r19-accept-support), tracked only | `83a16fdee033fce5a1a792d8d65f4090fba8dacffc2d38013684cacb4daeba19` |
| combined sha256 (diff bytes, then each untracked path NUL bytes, sorted), pathspec src tests/native/r19-accept-support, evidence excluded | `c1610eccfe77205557d73926bf323ecf5afe2f94c47237b6e486c19cf3fbc59b` |

| file | state | sha256 |
|---|---|---|
| `src/engine/ledger/execution.t.cpp` | modified | `250867b8bd74b86d718cbe17e5fa9983fda6a60a2b5e7c127829a3fec6f98950` |
| `src/engine/ledger/storage.cpp` | modified | `e4cf41dcb8cf44cc8004d4ba011543a557c742c623e20ede631eb975f237d822` |
| `tests/native/r19-accept-support/accept_lib.py` | untracked | `6ef4e069c2da203d508f3695482b5ea9883b71fc47ee8519a6f910f99e223e68` |
| `tests/native/r19-accept-support/accept_test.py` | untracked | `c6ae7b28851063836cc57ae1a77640976caecfa8c5c741672b81e6a797a93d8e` |
| `tests/native/r19-accept-support/acceptance_cli_test.py` | untracked | `9a4d50821696d9f843764f063c04b5b416a54eca552593e793bf10bc37468edd` |
| `tests/native/r19-accept-support/check_runner.py` | untracked | `b71e939a226415c0aeb43083dffcf263618d2c88804500300e585bfafec1d5f1` |
| `tests/native/r19-accept-support/cmake_check.py` | untracked | `ba8f20625bda0f3c8fc3c613529e16f6a1f8e0a38b0b261f8341b2fdcbb3f360` |
| `tests/native/r19-accept-support/fault/cmake_rerun/run_heavy.py` | untracked | `316fa2abd3d86614a79f952d2eb4819c71c836d266e2124916b1bbb639e78b47` |
| `tests/native/r19-accept-support/fault/delayed_marker_check.py` | untracked | `6ca4c0b4283083a8241a5edbe71c1912d0d8fa908ac6d602de29f453ed728141` |
| `tests/native/r19-accept-support/fault/mini/base/src/unit.txt` | untracked | `d73d5ccc52b8747e2bc253b7267b6110d41021eacd54ff33695d96c0a47fe56a` |
| `tests/native/r19-accept-support/fault/mini/jobs/combined-max1.lua` | untracked | `c7d8b3a33fd7940b6ef56e4ac4923836f527644d8b6952aaa8ae5d22f54e8e39` |
| `tests/native/r19-accept-support/fault/mini/jobs/combined-max2.lua` | untracked | `6d0df6cf79f148664587dcf43e774243cad211e4d26420ce8fe4f955c1281e0e` |
| `tests/native/r19-accept-support/fault/mini/registry.json` | untracked | `eca5ac671b6929f042a839591ff56da72d0afef8f4e9aca5433df7b1119490ca` |
| `tests/native/r19-accept-support/fault/mini/source/src/unit.txt` | untracked | `753f8fe079abf1160eaa98cc40257a00da60a033b2d3730ab679fa0f62a0778c` |
| `tests/native/r19-accept-support/fault/mini/workspace/spec.lua` | untracked | `d35b07e34032202e830cedb66b4d45b5774621f087acc9f8012b1d8edacf07da` |
| `tests/native/r19-accept-support/fault/mini/workspace/workspace.lua` | untracked | `21ac5c6da0e2d27bb4911202d0a364acacc49d39b052fa71d8ca44b68e2d453d` |
| `tests/native/r19-accept-support/fault/sleep_check.py` | untracked | `751414911570978faf4f82d0ee79631ceb41c76dfd2bc4e66d35c41faa51ce57` |
| `tests/native/r19-accept-support/fault_driver.py` | untracked | `ced606f417f4a9098e362d3e49e65d383c08dcbd1e9b34826978f979f1f62197` |
| `tests/native/r19-accept-support/fault_live_test.py` | untracked | `d675691ff97c30a1bdfe3763533c8d85fd4228750aac3ecbc9ee4e334908b44b` |
| `tests/native/r19-accept-support/quota_driver.py` | untracked | `3c45a76571439a528e3cf1aee3803308964349da6edb247235cc656e8a4a3420` |
| `tests/native/r19-accept-support/quota_live_test.py` | untracked | `bedad9a306b98b535c33f2a27ea4224575bc6f2a81120019815d3d119117f861` |
| `tests/native/r19-accept-support/registry_gen.py` | untracked | `26752307e7257a99722b713342c790ae3eb146bd99c93ea410b6b4312109d172` |
| `tests/native/r19-accept-support/run_acceptance.py` | untracked | `3c04f00ecf48c127e83b9f5c0a8436c0673c3ca178810b808bec3839dec40c94` |

## Files edited or created by PKG-FIX (owned-file list)

Edited (test files only):
- `tests/native/r19-accept-support/accept_test.py`: import `check_runner`; two new tests at the end of `AcceptLibGapTest`.
- `tests/native/r19-accept-support/fault_live_test.py`: five new tests at the end of `FaultLiveTest`.
- `tests/native/r19-accept-support/quota_live_test.py`: seven new tests at the end of `QuotaLiveTest`.

Created:
- `tests/native/r19-accept-a-c02-evidence/probes/FIX-NOTES.md` (this file).

Not created: `plan/FIX-NOOP.txt` (survivors exist). Not edited: `redaction/support-allowlist.txt`.
