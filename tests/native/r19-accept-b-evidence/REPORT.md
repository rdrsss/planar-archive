# R19-ACCEPT-B evidence report

One report for the R19-ACCEPT-B and R19-ACCEPT-B-C01 evidence, written at the C02 evidence step. The recorded A34 run is the single run named below. Placeholders: `$PLANAR_CANDIDATE` is the recorded candidate binary, `<scratch-root>` a scratch directory, `<repo>` the repository root, `<home>` the real user home, `<workflow-scripts>` the workflow scripts directory.

## A34 readback manifest

Source: `a34-readback.json` in this folder (committed, path-free, under 100 KB). sha256 of the file: `1eda7db11e566e8eba9b7bd77865ba635f4d5ddc3aada456e672eb30caf17bd7`.

The manifest has the keys `version`, `facts`, `claim`, `takeover`, `nofile` and `checks`. Every boolean in `checks` is true:

| check | value |
|---|---|
| verified | true |
| token_absent_in_run_dir | true |
| claim.document_matches_supplied | true |
| claim.generation_expected | true |
| claim.native_ten_keys | true |
| claim.preferences_unknown | true |
| takeover.document_matches_supplied | true |
| takeover.generation_expected | true |
| takeover.native_ten_keys | true |
| takeover.preferences_unknown | true |
| nofile.generation_expected | true |
| nofile.native_ten_keys | true |
| nofile.preferences_unknown | true |

Facts as pasted from the manifest:

| fact | value |
|---|---|
| accepted_revision | 1 |
| baseline_ref | 58128234:src/engine |
| baseline_sha256 | recorded in the manifest (the digest of the baseline engine tree) |
| candidate_sha256 | `774bcd69809ead9e4cd7b6e06c74aa045ea9272a2321cd36f91bc9c76a99ab0c` |
| candidate_size | 33982376 |
| driver_version | 9470b069e548 |

`facts.accepted_revision` is the ledger load revision of the scratch ledger (a positive decimal equal to the revision in `result.json`). It is not a git commit.

The A34 run, command with placeholders (scratch `HOME` and `PLANAR_DB` under `<scratch-root>`, a fresh ledger, run in the background with a DONE marker):

    cd <repo>/tests/native/r19-accept-support
    HOME=<scratch-root>/home PLANAR_DB=<scratch-root>/planar.db PLANAR_CANDIDATE=$PLANAR_CANDIDATE \
      python3 -B run_acceptance.py provenance --candidate $PLANAR_CANDIDATE \
      --record <repo>/tests/native/r19-accept-b-evidence/candidate/candidate.json --repo-path <repo> \
      --raw-root <repo>/tests/native/r19-accept-b-evidence/raw/c02-a34 --run-name a34-provenance

Result: exit 0 (`a34.exit`), `DONE` present, `result.json` parsed, with the run directory `raw/c02-a34/a34-provenance`. The environment record `raw/c02-a34/env.txt` shows scratch `HOME` and `PLANAR_DB` outside the real home. Raw logs stay on this machine.

## Red-first evidence

Two behavioural red captures precede the implementation. Both ran one `python3 -B -m unittest <module>.<Class>.<test>` process per test, from the support directory, with scratch `HOME` and `PLANAR_DB` and no compile step.

- Provenance (22 tests of R19-ACCEPT-B): `red/RED.md`. Command as recorded there: `cd tests/native/r19-accept-support && python3 -B -m unittest provenance_test.<Class>.<test>` with scratch `HOME` and `PLANAR_DB`. The pre-fix snapshot is described by the files `red/snapshot-2` (the empty tracked diff and the checksums of the two untracked provenance files) and `red/snapshot-hashes.txt`; the hash values are in those files. Every one of the 22 tests failed by assertion on the stubs.
- C01 (45 tests: `provenance_test` 27 and `checkpoint_test` 18): `red-c01/RED-C01.md`. Command as recorded there: `HOME=<scratch>/home PLANAR_DB=<scratch>/db/planar.db PLANAR_CANDIDATE=<recorded candidate> PYTHONDONTWRITEBYTECODE=1 python3 -B -m unittest -v <module>.<Class>.<test>`. The pre-fix snapshot is the file `red-c01/snapshot-hashes.txt` and the copy `red-c01/snapshot`. 24 expectRed tests failed by assertion; 20 guards passed; the one guard that failed in the red run (`ProvenanceLiveTest.test_token_never_appears_in_provenance_files_or_readbacks`, on the new `token_absent` result key) passes in the suites below.

Limits of the red record are stated in the two RED files (the first capture used a candidate path that `accept_lib` refuses, so two candidate-dependent CLI tests were red partly for environment). Raw red logs stay on this machine.

## Candidate

Source identity (`identity.py src`, `raw/c02-identity/src.txt`, pasted verbatim):

| identity | value |
|---|---|
| HEAD | `080f1666ca5c095b7c02fb4526dca10e95937262` |
| sha256(git diff --binary HEAD -- src), tracked only | `e3b0c44298fc1c149afbf4c8996fb92427ae41e4649b934ca495991b7852b855` |
| combined sha256 (diff bytes, then each untracked path NUL bytes, sorted), pathspec src, evidence excluded | `e3b0c44298fc1c149afbf4c8996fb92427ae41e4649b934ca495991b7852b855` |

| file | state | sha256 |
|---|---|---|

Support-tree identity (`identity.py tests/native/r19-accept-support`, `raw/c02-identity/support.txt`, pasted verbatim; the support tree is unchanged since the C01 gate):

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

Candidate binary: the R19-ACCEPT-A C01 candidate, recorded in `candidate/candidate.json` (sha256 of that file `7a9d85033b508f7934cbff0fee14d470b66528b68420e5e72c54cea48d49ce20`). Its `sha256` field is `774bcd69809ead9e4cd7b6e06c74aa045ea9272a2321cd36f91bc9c76a99ab0c`, size 33982376, built from commit 91e0df24f2167471e49ec057c7159bce5dff3608 with two overlay files whose blobs equal HEAD. This reuse is the recorded deviation E3; the record carries the equivalence argument. `src` is unchanged since, so the C03 full gate (macOS Debug and Release and Linux arm64, 1854 of 1854 each) is cited as the R19 gate.

## Evidence id index

Identifiers of this round and where each is recorded. Test names are `Class.test` in `tests/native/r19-accept-support/provenance_test.py`.

| id | test |
|---|---|
| I1 | `ProvenanceDocTest.test_build_document_has_every_key_and_leaves_model_and_session_unknown` |
| I2 | `ProvenanceDocTest.test_validate_document_refuses_each_leaf_rule_violation_alone` |
| I3 | `ProvenanceDocTest.test_validate_document_accepts_one_and_1024_byte_leaves_and_unknown_object` |
| I4 | `ProvenanceDocTest.test_normalize_document_sorts_settings_by_unsigned_name_bytes` |
| I5 | `ProvenanceDocTest.test_verify_readback_refuses_wrong_authority` |
| I6 | `ProvenanceDocTest.test_verify_readback_refuses_document_differing_in_one_leaf` |
| I7 | `ProvenanceDocTest.test_verify_readback_refuses_missing_native_key_and_known_preferences` |
| I8 | `ProvenanceDocTest.test_verify_readback_refuses_bearer_token_anywhere` |
| I9 | `ProvenanceDocTest.test_verify_readback_accepts_exact_expansion` |
| I10 | `ProvenanceDocTest.test_refused_claim_is_reported_as_product_defect` |
| I11 | `ProvenanceLiveTest.test_claim_argv_passes_provenance_file_holding_the_honest_document` |
| I12 | `ProvenanceLiveTest.test_history_readback_shows_supplied_leaves_and_unknown_leaves` |
| I13 | `ProvenanceLiveTest.test_history_readback_shows_native_derived_block` |
| I14 | `ProvenanceLiveTest.test_takeover_stores_its_own_provenance_record` |
| I15 | `ProvenanceLiveTest.test_token_never_appears_in_provenance_files_or_readbacks` |
| I16 | `ProvenanceLiveTest.test_run_uses_scratch_home_and_leaves_real_home_untouched` |
| I17 | `ProvenanceLiveRefusalTest.test_run_provenance_aborts_as_product_defect_when_claim_is_refused` |
| I18 | `ProvenanceCliTest.test_provenance_command_aborts_before_first_action_on_candidate_mismatch` |
| I19 | `ProvenanceCliTest.test_provenance_command_writes_result_json_with_both_readbacks` |
| I31 | `ProvenanceLiveTest.test_takeover_without_provenance_reads_back_all_unknown_driver` |
| I32 | `ProvenanceDocTest.test_verify_absent_readback_accepts_none_driver_and_refuses_stored_record_or_known_leaf` |
| I41 | `ProvenanceLiveTest.test_documents_carry_recomputed_driver_version_and_candidate_size` |

| id | recorded in |
|---|---|
| I20 | section Suites |
| I21 | section Candidate (source identity) |
| I22 | section Red-first evidence |
| I30 | section Commit list and redaction |
| I36 | section Candidate |
| I40 | section Limits (additions-only change to `run_acceptance.py`; no existing diagnostic changed) |
| I42 | not applicable: no product defect was found, so no defect file exists |
| I43 | section A34 readback manifest |
| I44 | sections Red-first evidence and Candidate |
| I45 | section Prober isolation |

Raw logs are on this machine under `raw/` (run logs under `raw/c02-a34`, suite logs under `raw/c02-suites`, identity records under `raw/c02-identity`). The committed hashes are in this report; `README.md` lists committed and local files.

## Prober isolation

Mutants are applied only by the mutation prober, in a scratch copy of the support tree outside the checkout, with scratch `HOME` and `PLANAR_DB` and `PLANAR_CANDIDATE` set to the recorded candidate. Coders and repair coders never mutate checkout files. After each mutant the scratch file is restored by byte copy and compared with the pristine bytes. The probe record is `tests/native/r19-accept-b-c01-evidence/probes/PROBES.md`.

## Commit list and redaction

The single final commit list is `tests/native/r19-accept-b-evidence/COMMIT-LIST.txt`, covering B, C01 and C02 (sorted bytewise, unique, repo-relative). Exact commands:

    python3 tests/native/r19-accept-support/run_acceptance.py commit-list --list tests/native/r19-accept-b-evidence/COMMIT-LIST.txt --repo-root <repo>
    python3 scripts/redact-local-paths.py --repo-path <repo> --home <home> check <the entries of that list>

`<home>` is the real user home from the account database, not the scratch `HOME` of the runs. The redaction tool's apply mode is not used and no `redaction/original` copy exists; the one manual redaction was the workflow script path in `tests/native/r19-accept-b-c01-evidence/plan/DECISIONS.md`, replaced by `<workflow-scripts>/identity.py`. The results of both commands are recorded in the round record, section C02 sync, and are re-run by the reviewer.

## Suites

Six support suites ran isolated from a fresh scratch root (`raw/c02-suites/env.txt`: scratch `HOME` and `PLANAR_DB`, `PLANAR_CANDIDATE` equal to the recorded candidate, `python3 -B`), distinct from the A34 and A03 roots. Counts from `raw/c02-suites/suites-summary.txt`:

| suite | ran | failures | errors | skipped |
|---|---|---|---|---|
| provenance_test | 27 | 0 | 0 | 0 |
| checkpoint_test | 18 | 0 | 0 | 0 |
| accept_test | 63 | 0 | 0 | 0 |
| acceptance_cli_test | 29 | 0 | 0 | 0 |
| quota_live_test | 28 | 0 | 0 | 0 |
| fault_live_test | 33 | 0 | 0 | 0 |

Total 198 tests, 0 failures, 0 errors, 0 skips.

## Limits

- Evidence is for the macOS Debug candidate only. x86_64 Linux is uncovered; Release and Linux arm64 are cited to the C03 gate because `src` is unchanged.
- The candidate is the R19-ACCEPT-A C01 binary under deviation E3, not a fresh export of HEAD.
- The A34 manifest records driver assertions: authority `driver_assertion_unverified`; model and session leaves the driver does not know read back as `{"status":"unknown"}`. Nothing is verified or fabricated.
- Each suite ran once; the live suites are slow and a single pass is the record.
- Raw logs and snapshots stay on this machine and are not part of the committed set; the round reviews are committed.
