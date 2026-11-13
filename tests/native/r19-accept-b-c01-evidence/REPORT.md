# R19-ACCEPT-B-C01 evidence report

One report for the A03 checkpoint scenario and its tests, written at the C02 evidence step. Placeholders: `$PLANAR_CANDIDATE` is the recorded candidate binary, `<scratch-root>` a scratch directory, `<repo>` the repository root.

## A03 readback manifest

Source: `a03-readback.json` in this folder (committed, path-free, under 100 KB). sha256 of the file: `823fb826125e8302e6565ae3fa30c42e41fbf8adc7ea60aa22b46a8f2264889b`.

The `checks` object has exactly these ten checks, all true:

| check | value |
|---|---|
| control_equal | true |
| render_moved_by_checkpoint1 | true |
| render_moved_by_checkpoint2 | true |
| since_has_checkpoint_added_section | true |
| since_matches_after2 | true |
| since_material_empty | true |
| state_equal | true |
| token_absent | true |
| verified | true |
| work_fp_equal | true |

Informational: `checkpoint_section_omitted_by_budget` is false, so the checkpoint section was present in every preview. Facts: `accepted_revision` 1 (the ledger load revision, not a git commit), `candidate_sha256` `774bcd69809ead9e4cd7b6e06c74aa045ea9272a2321cd36f91bc9c76a99ab0c`, `candidate_size` 33982376. The candidate is described in `tests/native/r19-accept-b-evidence/candidate/candidate.json`.

## A03 run

Command with placeholders (scratch `HOME` and `PLANAR_DB` under `<scratch-root>`, a fresh ledger distinct from the A34 ledger and the suites root):

    cd <repo>/tests/native/r19-accept-support
    HOME=<scratch-root>/home PLANAR_DB=<scratch-root>/planar.db PLANAR_CANDIDATE=$PLANAR_CANDIDATE \
      python3 -B run_acceptance.py checkpoint --candidate $PLANAR_CANDIDATE \
      --record <repo>/tests/native/r19-accept-b-evidence/candidate/candidate.json --repo-path <repo> \
      --raw-root <repo>/tests/native/r19-accept-b-c01-evidence/raw/c02-a03 --run-name a03-checkpoint

Result: exit 0 (`a03.exit`), `DONE` present, `result.json` parsed, run directory `raw/c02-a03/a03-checkpoint`. The environment record `raw/c02-a03/env.txt` shows scratch `HOME` and `PLANAR_DB` outside the real home. Raw logs stay on this machine.

## Probes C10 to C18

Source: `probes/PROBES.md` (committed). Each row names a mutant and the test that killed it by assertion:

| row | mutant | killing test |
|---|---|---|
| C10 | checkpoint file holds different text or an empty files list (rows C10a, C10b) | `CheckpointLiveTest.test_checkpoint_argv_passes_observations_file_holding_the_honest_checkpoint` |
| C11 | event id taken from the checkpoint id | `CheckpointLiveTest.test_checkpoint_event_is_recorded_and_read_back_by_history` |
| C12 | before snapshot taken after checkpoint 1 | `CheckpointLiveTest.test_checkpoint_moves_only_the_render_digest` |
| C13 | after1 preview taken before checkpoint 1 | `CheckpointLiveTest.test_checkpoint_moves_only_the_render_digest` |
| C14 | since read before checkpoint 2 | `CheckpointLiveTest.test_preview_since_reports_a_render_only_change` |
| C15 | checkpoint 2 reuses the first text | `CheckpointLiveTest.test_second_checkpoint_with_different_text_moves_render_again` |
| C16 | control preview taken after a third checkpoint | `CheckpointLiveTest.test_unchanged_state_gives_byte_equal_previews` |
| C17 | scenario environment copies the process environment | `CheckpointLiveTest.test_run_uses_scratch_home_and_leaves_real_home_untouched` |
| C18 | unredacted argv stored in checkpoint 1 | `CheckpointLiveTest.test_run_token_never_appears_in_result_or_manifest_files` |

All nine were killed (the record has no unexplained survivor). The prober ran every mutant in a scratch copy outside the checkout.

## Red-first

Source: `tests/native/r19-accept-b-evidence/red-c01/RED-C01.md`. Command as recorded there: one process per test, `HOME=<scratch>/home PLANAR_DB=<scratch>/db/planar.db PLANAR_CANDIDATE=<recorded candidate> PYTHONDONTWRITEBYTECODE=1 python3 -B -m unittest -v <module>.<Class>.<test>`, from the support directory. The pre-fix snapshot is described by naming the files `red-c01/snapshot-hashes.txt` and `red-c01/snapshot`. All 18 `checkpoint_test` tests failed by assertion on the stubs; with the strengthened and manifest tests of `provenance_test` the capture has 24 expectRed tests, all failed by assertion. The failures are behavioural (not implemented, wrong shape, missing keys), not compile or import errors.

## Suites

Isolated per `tests/native/r19-accept-b-evidence/raw/c02-suites/env.txt` (scratch `HOME` and `PLANAR_DB`, recorded candidate, `python3 -B`). Counts from the suites summary of the B evidence folder:

| suite | ran | failures | errors | skipped |
|---|---|---|---|---|
| provenance_test | 27 | 0 | 0 | 0 |
| checkpoint_test | 18 | 0 | 0 | 0 |
| accept_test | 63 | 0 | 0 | 0 |
| acceptance_cli_test | 29 | 0 | 0 | 0 |
| quota_live_test | 28 | 0 | 0 | 0 |
| fault_live_test | 33 | 0 | 0 | 0 |

Total 198 tests, 0 failures, 0 errors, 0 skips. The A03 module is `checkpoint_test` with 18 tests.

## Limits

- Evidence is for the macOS Debug candidate only. x86_64 Linux is uncovered.
- E3 deviation (platform handling): the recorded R19-ACCEPT-A candidate is reused (deviation A) and the C03 gate is cited for Release and Linux arm64 because `src` is unchanged; no build, ctest or Docker run was made in this round.
- The A03 checkpoint scenario verifies the render digest and the since preview against the recorded candidate only; it makes no claim about other candidates.
- Raw logs and snapshots stay on this machine and are not part of the committed set; the round reviews are committed.
