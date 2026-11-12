# R19-ACCEPT-A-C01 red-first record: repair-pass tests (RF1, RF2) and H6 citation

Owner: PKG-G (role test). Stage: wave 3 record, per DECISIONS D18 (corrected baseline; `red/snapshot-4` is NOT used).
Plan: `tests/native/r19-accept-a-c01-evidence/plan/INVARIANTS.md` Part 3 rows and RF1/RF2; `DECISIONS.md` D18, D26, D33, D36.

## Statement (honest; no red is fabricated)

For each of the 14 repair-pass tests below, **no red possible against the unmutated pre-repair library**: repair pass 1 changed
tests only (D18), so the pre-repair library is the current library and each test passes by construction. Each test passes
here by name, green, with zero skips. The only meaningful red for each test is the prober's kill of the survivor mutant(s)
named in its row. That kill is produced only by the prober, in a scratch copy, through PROBE-1 (`probes-c01/PROBES.md` and
its raw log under `probes-c01/raw/`). PROBE-1 has not run at the time of this record, so no kill is cited as present.

Lineage evidence of the survivors (read-only, from the prior R19-ACCEPT-A probe pass, before repair pass 1): the raw
logs under `tests/native/r19-accept-a-evidence/probes/raw/` for I3b, I10, I10c, I12, I12c, I12d, I13b, I17c, I24, I24b,
I29b, I30c, I35b, E6, E8, E10, E11, E13, E14, E15, E17 and E18 are recorded as SURVIVED in `probes/PROBES.md`. I10d was
KILLED there by a different test (`test_trajectory_requires_interrupted_check_recovery`). For E6 and E8 the prior raw logs
show only `FAIL: test_live_commands_abort_before_first_action_on_candidate_mismatch` (environment-caused: no candidate),
which is an invalid kill under D26, not a kill of the mutant.

## Scratch tree and identity (RF2)

Construction: `git clone --local --no-hardlinks <repo> <scratch-root>/clone` (HEAD `91e0df24f2167471e49ec057c7159bce5dff3608`,
never `git worktree add`), then the untracked `tests/native/r19-accept-support/**` copied into it from the checkout. The
repair-pass tests do not import anything under `src/`, so no D26 modified-tracked overlay is needed. No checkout file was
stashed, reverted, mutated or built; `git -C <repo> status` is unchanged by this stage.

Identity output, verbatim from `python3 identity.py --repo <scratch-root>/clone tests/native/r19-accept-support` (run from
the scratch clone; kept as `raw/identity-scratch-clone.txt`):

| identity | value |
|---|---|
| HEAD | `91e0df24f2167471e49ec057c7159bce5dff3608` |
| sha256(git diff --binary HEAD -- tests/native/r19-accept-support), tracked only | `e3b0c44298fc1c149afbf4c8996fb92427ae41e4649b934ca495991b7852b855` |
| combined sha256 (diff bytes, then each untracked path NUL bytes, sorted), pathspec tests/native/r19-accept-support, evidence excluded | `91825501bd7c85bae8ddc6d7f5154ba8a25725f01313d8d14311c65852eb0c1f` |

| file | state | sha256 |
|---|---|---|
| `tests/native/r19-accept-support/accept_lib.py` | untracked | `9809217494dc27ebcfdf4728df83076a303aefb473de79ba6488b8d0099f6f16` |
| `tests/native/r19-accept-support/accept_test.py` | untracked | `c5a236611785a84bd2ecd44db8d802e7e3e19a2191904dbf5f563f05a38275a9` |
| `tests/native/r19-accept-support/acceptance_cli_test.py` | untracked | `57c15f37e813dacad89b0b6892f6b1e2269df96107d789c54c2b22cd59976336` |
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
| `tests/native/r19-accept-support/run_acceptance.py` | untracked | `ff3877cb3da0603676debab0cdcab716fc7a093c7214d1db3ca357f8a30d6fa7` |

The same command run with `--repo <repo>` on the checkout printed byte-identical output (`raw/identity-checkout.txt`), so the
scratch tree matches the checkout for this pathspec. The combined sha256 row above is the snapshot hash for every record
below.

## Command (RF1, by name)

```
cd <scratch-root>/clone/tests/native/r19-accept-support && PYTHONDONTWRITEBYTECODE=1 python3.15 -B -m unittest -v <module>.<Class>.<test>
```

Interpreter: `python3.15` (Python 3.15.2, Homebrew). Each run is a single named test, executed serially with the exit code
captured. Raw output: `raw/<Class>.<test>.log`. Exit codes and last line: `raw/results.tsv`. Test list: `raw/tests.txt`.

## Results (15 runs: the 14 repair-pass tests plus H6)

Every run printed `Ran 1 test in ...` followed by `OK`, exit 0, zero skips (no `skipped` suffix in any log; the word
"skip" appears only in docstrings).

| # | test (by name) | survivor mutant(s), INVARIANTS Part 3 | exit | raw log | status |
|---|---|---|---|---|---|
| 1 | `accept_test.AcceptLibGapTest.test_scan_finds_prefix_only_present_as_decoded_json_value` | M-I3b | 0 | `raw/AcceptLibGapTest.test_scan_finds_prefix_only_present_as_decoded_json_value.log` | green by construction; red only via prober kill of M-I3b |
| 2 | `accept_test.AcceptLibGapTest.test_scan_and_event_expansion_refuse_vacuous_inputs` | M-E14, M-E15 | 0 | `raw/AcceptLibGapTest.test_scan_and_event_expansion_refuse_vacuous_inputs.log` | green by construction; red only via prober kill of M-E14 / M-E15 |
| 3 | `accept_test.AcceptLibGapTest.test_at_event_state_refuses_missing_roles_and_extra_roles` | M-E17, M-E18 | 0 | `raw/AcceptLibGapTest.test_at_event_state_refuses_missing_roles_and_extra_roles.log` | green by construction; red only via prober kill of M-E17 / M-E18 |
| 4 | `accept_test.AcceptLibGapTest.test_trajectory_interruption_must_close_by_check_recover` | M-I10, M-I10c, M-I10d | 0 | `raw/AcceptLibGapTest.test_trajectory_interruption_must_close_by_check_recover.log` | green by construction; red only via prober kill of M-I10 / M-I10c / M-I10d |
| 5 | `accept_test.AcceptLibGapTest.test_trajectory_rejection_admission_predicate_is_exact` | M-I12, M-I12c, M-I12d | 0 | `raw/AcceptLibGapTest.test_trajectory_rejection_admission_predicate_is_exact.log` | green by construction; red only via prober kill of M-I12 / M-I12c / M-I12d |
| 6 | `accept_test.AcceptLibGapTest.test_trajectory_capped_rejection_requires_repair_not_admitted` | M-I13b | 0 | `raw/AcceptLibGapTest.test_trajectory_capped_rejection_requires_repair_not_admitted.log` | green by construction; red only via prober kill of M-I13b |
| 7 | `accept_test.AcceptLibGapTest.test_inventory_verdict_refuses_duplicate_inventory_names` | M-I17c | 0 | `raw/AcceptLibGapTest.test_inventory_verdict_refuses_duplicate_inventory_names.log` | green by construction; red only via prober kill of M-I17c |
| 8 | `accept_test.AcceptLibGapTest.test_red_record_each_marker_rule_stands_alone` | M-I24, M-I24b | 0 | `raw/AcceptLibGapTest.test_red_record_each_marker_rule_stands_alone.log` | green by construction; red only via prober kill of M-I24 / M-I24b |
| 9 | `accept_test.AcceptLibGapTest.test_check_runner_passthrough_never_writes_a_result_document` | M-I29b | 0 | `raw/AcceptLibGapTest.test_check_runner_passthrough_never_writes_a_result_document.log` | green by construction; red only via prober kill of M-I29b |
| 10 | `accept_test.AcceptLibGapTest.test_rendezvous_write_replaces_the_file_instead_of_rewriting_it_in_place` | M-I30c | 0 | `raw/AcceptLibGapTest.test_rendezvous_write_replaces_the_file_instead_of_rewriting_it_in_place.log` | green by construction; red only via prober kill of M-I30c |
| 11 | `accept_test.AcceptLibGapTest.test_child_env_sets_bytecode_flag_even_when_the_parent_lacks_it` | M-I35b | 0 | `raw/AcceptLibGapTest.test_child_env_sets_bytecode_flag_even_when_the_parent_lacks_it.log` | green by construction; red only via prober kill of M-I35b |
| 12 | `accept_test.AcceptLibGapTest.test_build_verdict_refuses_a_unit_named_twice` | M-E13 | 0 | `raw/AcceptLibGapTest.test_build_verdict_refuses_a_unit_named_twice.log` | green by construction; red only via prober kill of M-E13 |
| 13 | `acceptance_cli_test.AcceptanceCliTest.test_index_fails_when_a_host_prefix_survives_redaction` | M-E6 | 0 | `raw/AcceptanceCliTest.test_index_fails_when_a_host_prefix_survives_redaction.log` | green by construction; red only via prober kill of M-E6 (run_acceptance.py fix by PKG-F/PKG-R if survivor, D43) |
| 14 | `acceptance_cli_test.AcceptanceCliTest.test_verify_redaction_checks_original_hash_and_real_prefixes` | M-E8, M-E10, M-E11 | 0 | `raw/AcceptanceCliTest.test_verify_redaction_checks_original_hash_and_real_prefixes.log` | green by construction; red only via prober kill of M-E8 / M-E10 / M-E11 |
| H6 | `accept_test.AcceptLibTest.test_red_record_must_be_behavioural_and_hashed` (existing, accept_test.py:539) | none (record row; its clauses are the existing guards M-I24 / M-I24b) | 0 | `raw/AcceptLibTest.test_red_record_must_be_behavioural_and_hashed.log` | green; the composed RED-1 record validation is in `red-c01/history/RED.md`, which is blocked (see below) |

Test-count note: the repair-pass set is 14 tests (12 in `AcceptLibGapTest`, 2 in `AcceptanceCliTest`), as D18 corrects.
Line references: the two `AcceptanceCliTest` tests are at acceptance_cli_test.py:319 and :335 in the current file. Part 3
cites :264 and :280 (stale). The 12 `AcceptLibGapTest` tests are at accept_test.py:869-998 inside the class at :853.
The `test_live_commands_abort...` test that Part 3 cites as M-A6 is at acceptance_cli_test.py:265, not :210.

## Limits

- No red exists for any of the 14 tests on the unmutated library, by construction (D18). Their meaningful red is only the
  prober kill, which is pending PROBE-1 (`probes-c01/PROBES.md`) and was not produced by this stage.
- The H6 record (history RED-1) and the S1-S3, R1, R2 support record are not composed here (see the blocker below).
- Nothing in this record was mutated, stashed or reverted. No checkout file was edited by this stage.

## Blocker: RED-1 raw records are absent (history/RED.md and support/RED.md not written)

The brief says RED-1 already wrote `red-c01/history/raw/` (C++ raw log, `validate_red_record.txt`) and
`red-c01/support/raw/` (S1-S3, R1, R2 run, `validate_red_record.txt`). Neither directory exists anywhere under
`tests/native/` or the scratchpad. The repository-wide search for `red-c01` found only mentions in workflow transcripts.
Without those raw files there is no assertion output to quote and no `validate_red_record.txt` result, so
`red-c01/history/RED.md` and `red-c01/support/RED.md` were NOT written: composing them would be fabrication. Per the brief,
the stage stops and PKG-G reports it. RED-1 must be re-run (it may write only under `red-c01/**/raw/`), then these two
records composed from its output.

## Files created by this stage

- `tests/native/r19-accept-a-evidence/red-c01/rf/RED.md` (this file)
- `tests/native/r19-accept-a-evidence/red-c01/rf/raw/<Class>.<test>.log` (15 logs, one per test run)
- `tests/native/r19-accept-a-evidence/red-c01/rf/raw/results.tsv`, `raw/tests.txt`
- `tests/native/r19-accept-a-evidence/red-c01/rf/raw/identity-scratch-clone.txt`, `raw/identity-checkout.txt`
- Scratch only (outside the checkout): `<scratch-root>/clone` (the `git clone --local --no-hardlinks` tree) and the
  scratch notes used to build this record.
- No checkout file, no product file, no support file and no existing evidence file was edited, created or removed.
