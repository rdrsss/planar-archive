# R19-ACCEPT-A-C01 RED-1 record: suites and rerun-index (S1, S2, S3, R1, R2)

Stage RED-1 (D40), run after wave 1 and before wave 2. `tests/native/r19-accept-support/run_acceptance.py` holds the
PKG-B stubs: `cmd_suites` and `cmd_rerun_index` print `run_acceptance: suites not implemented` or
`run_acceptance: rerun-index not implemented` on stderr, return 2 and write nothing. The subcommands are registered,
so argparse never errors on them.

## Pre-implementation snapshot

Verbatim output of `python3 <workflows-scripts>/identity.py tests/native/r19-accept-support`,
run from the repository root after the run (also kept as `raw/identity-support.txt`). The support tree is untracked, so
the combined row covers its bytes. The `find -newer` check showed no support file changed after the run.

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

## Command and result

Command (D40), run from the repository root. Raw output is `raw/support-red-five.log` (unchanged); the composed record
with the command line and exit line is `raw/support-red-five-record.log`:

    cd tests/native/r19-accept-support && PYTHONDONTWRITEBYTECODE=1 python3.15 -B -m unittest -v acceptance_cli_test.AcceptanceCliTest.test_suites_command_reports_per_module_and_total_counts acceptance_cli_test.AcceptanceCliTest.test_suites_command_refuses_skips_failures_and_zero_matched acceptance_cli_test.AcceptanceCliTest.test_suites_command_live_modules_require_the_recorded_candidate acceptance_cli_test.AcceptanceCliTest.test_rerun_index_records_every_required_fault_field acceptance_cli_test.AcceptanceCliTest.test_rerun_index_fails_when_any_required_field_is_missing

Result: `Ran 5 tests`, `FAILED (failures=45)`, exit 1. The matched count is 5. Each failure is an `AssertionError`.
The output has no `ImportError`, `ModuleNotFoundError`, `SyntaxError`, or `run_acceptance.py: error:` line. Python is
`/opt/homebrew/bin/python3.15` (3.15.2).

| row | test | failing assertion (acceptance_cli_test.py) | failures |
|---|---|---|---|
| S1 | `test_suites_command_reports_per_module_and_total_counts` | line 443: `two passing modules must exit 0` (stub returns 2) | 1 |
| S2 | `test_suites_command_refuses_skips_failures_and_zero_matched` | line 475 (`s2_passing` must exit 0) and line 478 (`must name the module`, stub stderr has no module name) for `s2_skipped`, `s2_failing`, `s2_import_error`, `s2_zero_matched` | 5 (subTests) |
| S3 | `test_suites_command_live_modules_require_the_recorded_candidate` | line 510: refusal must name the candidate or the record (exit 2 and marker absent, so only the wording check fails) | 3 (subTests `candidate_unset`, `hash_mismatch`, `record_missing`) |
| R1 | `test_rerun_index_records_every_required_fault_field` | line 520: a complete result must index (exit 2). The sentinel subtests after it are not reached under the stub | 1 |
| R2 | `test_rerun_index_fails_when_any_required_field_is_missing` | line 591: the refusal must name the field (stub stderr names no field), per subtest | 35 (14 missing-key subTests and 21 predicate subTests) |

## Validator (D36 and D40)

`raw/validate_red_record.txt` runs `accept_lib.validate_red_record` through `python3 -I -B -c` on the composed record
`{exit: 1, output: raw/support-red-five-record.log, snapshot_sha256: 91825501bd7c85bae8ddc6d7f5154ba8a25725f01313d8d14311c65852eb0c1f, command}`,
where the snapshot is the `combined sha256` row of `raw/identity-support.txt`. Result: `ACCEPTED`.

## Mutants each new test should kill (applied only by the prober, PROBE-1 and PROBE-2, in scratch)

- S1: `cmd_suites` writes only the total, reports the same `ran` for every module, sums wrong, or starts children
  with a bare `python3`. The `python3` shim on PATH exits 99, so a bare child fails.
- S2: `cmd_suites` ignores `skipped`, treats `ran=0` as success, or ignores a nonzero child exit (the import error
  module is the one that exits nonzero with a traceback).
- S3: `cmd_suites` skips `require_recorded_candidate` for live modules. The live child writes the marker file only when
  it starts, so a skipped check shows as `live-marker.txt` existing.
- R1: `cmd_rerun_index` omits a D38 section (`fenced`, `load` or `trajectory`) or copies fields without reading them.
  Each sentinel of `RERUN_SECTIONS` must appear in the index text.
- R2: `cmd_rerun_index` treats a missing field as success, skips a predicate, or writes the index before validating.
  Each subtest asserts nonzero exit, that the stderr names the field, and that no index file exists.

## Limits

- The stubs print no field or module names, so every S2 and R2 subtest and the S3 wording check fail on the message
  assertion. This is the expected red. Each test still needs its real implementation (PKG-D) to go green, and the
  prober must confirm that each mutant is killed by the named test.
- R1 stops after its exit assertion under the stub, so its sentinel checks are not yet exercised by this red. Their
  behaviour is proven by the green run after PKG-D and by the prober.
- Nothing was mutated, stashed or reverted. Test files were only run, not edited.
