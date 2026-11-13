# R19-ACCEPT-B-C01 review (uncommitted tree on top of 080f1666)

Disposition: **request-changes** (not records only). Two blocking findings need recorded live runs (A34 and A03), and one needs an operator decision on A28 before R19 completeness can be decided.

The test and probe half of C01 holds:

- `src` is unchanged.
- The A03 checkpoint scenario exists and was written test first, with its red capture kept.
- The I9, I13, I14, I15 and I31 invariants are now killed by their named assertions.
- The mutation probes are real. I re-ran 11 of them in my own scratch copy and got the recorded outcome every time.
- A15, A20 and A32 name C03 test cases that exist and do what the cells say. A28 is honestly downgraded to `open`.
- Advisories A1 to A4 are applied, A5 is confirmed and A6 is recorded.

The evidence half was not delivered, and the C01 coordinator sync says so itself ("What it did NOT deliver", and acceptance criteria 1, 3 and 6 marked "not met"):

- There is no recorded A34 run, and no `a34-readback.json`, `README.md` or final REPORT.
- There is no recorded A03 run and no `a03-readback.json`.
- No final commit list exists.
- The B-round REPORT still pins hashes over the records.
- The roadmap still carries the B-round outcome text.
- A28 is `open` with no operator question raised.

So findings R1, R3 and R5 of [R19-ACCEPT-B-review](R19-ACCEPT-B-review.md) are not closed. R19 is correctly **not** declared complete in the matrix and the round record. The roadmap's reasons are stale.

The reviewer was not the coder, the coordinator or the prober. Writes were limited to:

- this file;
- the gitignored local folder `tests/native/r19-accept-b-evidence/review/c01/` (`~/.planar` name lists, and the `commit-list` and redaction outputs);
- scratch files in the session scratchpad.

Nothing was staged or committed. No build, ctest, Docker or Linux gate ran, and no installed or legacy binary ran. The checkout's support tree and `build/rewrite-debug` were not touched. Mutants were applied only in my own scratch clone, outside the checkout:

- The clone was made with `git clone --shared`, checked out at 080f1666, and had the five candidate support files copied in. `diff -r` against the checkout exited 0 before and after.
- Every run set scratch `HOME` and `PLANAR_DB`, `PYTHONDONTWRITEBYTECODE=1` and `python3 -B`.
- `PLANAR_CANDIDATE` was the recorded A candidate, whose sha256 I re-hashed to the value in `candidate/candidate.json`.
- Every run used the `perl -e 'alarm N; exec @ARGV'` runner.
- Every mutated file was restored by byte copy and re-hashed equal to the checkout file.

The top-level entry names of `~/.planar` were identical before and after all reviewer runs.

## What was verified and holds

- **Source unchanged.** `identity.py src`, run by the reviewer, prints exactly:

  | identity | value |
  |---|---|
  | HEAD | `080f1666ca5c095b7c02fb4526dca10e95937262` |
  | sha256(git diff --binary HEAD -- src), tracked only | `e3b0c44298fc1c149afbf4c8996fb92427ae41e4649b934ca495991b7852b855` |
  | combined sha256 (diff bytes, then each untracked path NUL bytes, sorted), pathspec src, evidence excluded | `e3b0c44298fc1c149afbf4c8996fb92427ae41e4649b934ca495991b7852b855` |

  `identity.py tests/native/r19-accept-support` gives the following. All values equal the C01 sync and `probes/PROBES.md`.
  - tracked diff `2636b446e572892fb3f08087d56376f342579b04a262fd605499fc19bb1e5420`
  - combined `85fffe4b63e99f27ca2b0fc93c3e21b04e2f794d9e33b22aadb8ee4a44b45cbe`
  - five per-file hashes:
    - `run_acceptance.py` `2bb38280e24005684fc67fcbbeff48d858471a169323053bc5cd8fc59336946c`
    - `checkpoint_driver.py` `be85f5eff833a1c3a5eab6b815c2f99507342d8caad988c22fa5156acd6cfc62`
    - `checkpoint_test.py` `ef6e7ef047e96bdd8d14d61cea903c2d365acdda3f39e52576e7f4e2779c2bdd`
    - `provenance_driver.py` `9470b069e5480d3065514ce31514a6ec4094460abe77c9d285886b48456d443e`
    - `provenance_test.py` `c6147318550b76c5daf950ec3dc88f9c56237557d6390dd1403b287dbb11bfa1`
- **No test renamed or removed; diagnostics byte-identical.**
  - Every `Class.test_` pair of the B-round `provenance_test.py` (pre-fix snapshot, sha256 `18abff01…dcd3`, 22 pairs) is present in the current file (27 pairs). The five additions are the four `ProvenanceManifestTest` tests and `test_live_fixture_runs_with_verification_off_and_reports_it`.
  - `checkpoint_test.py` is new, with 18 tests.
  - `git diff -U0 HEAD -- run_acceptance.py` has 55 added lines and 0 removed lines: the `provenance` and `checkpoint` subcommands, two readback wrappers and header comments. No other tracked support file changed.
- **Suites, reviewer runs in the scratch clone.** All exit 0, with zero failures and zero skips (the word "skipped" occurs only inside `Kills:` docstrings): `provenance_test` 27, `checkpoint_test` 18, `accept_test` 63, `acceptance_cli_test` 29. The two live suites are in the Limits section below; neither suite's files changed (`quota_live_test.py`, `fault_live_test.py` and their drivers are byte-identical to HEAD).
- **Mutation probes are real** (`tests/native/r19-accept-b-c01-evidence/probes/PROBES.md`).
  - The prober's scratch copy `probe-warm` is a separate git checkout in the session scratchpad, outside the checkout.
  - All 56 raw logs of pass 2 do the following:
    - name a runner that exists (`perl` alarm);
    - show `Ran N` with N > 0;
    - record a byte-identical restore;
    - contain no "not applied" line.
  - Only `I13.log` contains `did not complete`, as P22 states. The carried rows' files (`provenance_driver.py`, `provenance_test.py`, `run_acceptance.py`) have the same hashes in `superseded-1/PROBES.md` as now, so carrying them is valid.
- **Reviewer re-runs of recorded mutants.** Each diff was extracted from the prober's raw log and applied with `patch`. The run completed in every case, and a kill is the named test failing by assertion:

  | mutant | invariant | named test failed | assertion text |
  |---|---|---|---|
  | X1 | I9 | `test_verify_readback_accepts_exact_expansion` | supplied document with settings out of name order (stored sorted) was refused |
  | X3 | I13 | `test_history_readback_shows_native_derived_block` | result claim readback differs from the independent history read |
  | X4 | I14 | `test_takeover_stores_its_own_provenance_record` | takeover argv lacks --provenance |
  | X6 | I15 | `test_token_never_appears_in_provenance_files_or_readbacks` | takeover provenance file holds the claim token |
  | X7 | I31 | `test_takeover_without_provenance_reads_back_all_unknown_driver` | the no-file takeover argv carries --provenance |
  | I16 | I16 | `test_run_uses_scratch_home_and_leaves_real_home_untouched` | HOME does not resolve under the run scratch root |
  | C6 | C6 (A03 core) | `test_verify_effect_refuses_each_clause_alone` | render digest unchanged: accepted, expected a refusal |
  | C12 | C12 | `test_checkpoint_moves_only_the_render_digest` | the before packet already has a checkpoint section |
  | C14 | C14 | `test_preview_since_reports_a_render_only_change` | result since differs from a fresh preview --since read of the claim digest |
  | E8 | survivor | none (18 of 18 OK) | survives as recorded |
  | E16 | survivor | none (18 of 18 OK) | survives as recorded |

  So acceptance criterion 2 is met: I9, I13, I14, I15 and I31 are killed by their named assertions. The six survivors E1, E2, E8, E15, E16 and E17 each have a stated reason that I find sound.
- **A03 test first.**
  - `tests/native/r19-accept-b-evidence/red-c01/RED-C01.md` lists all 18 `checkpoint_test` tests as `failed-assertion` with `validate_red_record` ACCEPT.
  - The raw logs (46) are kept, and none contains an import, syntax or name error.
  - The nine live rows fail in the shared fixture with `checkpoint run did not complete: not implemented`. That is a real red against the stubs, and the probes prove the named assertions. The repaired assertions (T-1 and T-2) have probe evidence only, which the record states.
- **R4 (A15, A20, A28 and A32).** Each case was opened at the cited `file:line` and found by `grep -Fx replacement.<name>` at the cited line of the C03 `inventory.txt`:
  - `R17-C child mutation makes all evidence inconclusive` (`check.t.cpp:83`, inventory 889) asserts `recapture_matches` false and reason `input_changed` on every entry, as the A15 cell says.
  - `command load malformed bundle semantic and captured result fail before completion` (`commands.t.cpp:850`, inventory 293) asserts `load_may_be_complete()` false and nothing prepared. The A20 cell correctly says that it does not re-read ledger bytes.
  - `semantic phase cycles report actual deterministic closed traces` (`schema.t.cpp:955`, inventory 1280), `X05 B1 accepted dependency base cycles precede every custom callback` (`shapes.t.cpp:423`, inventory 1452) and `X05 B1 added full graph detects cycles for grouped units` (`shapes.t.cpp:298`, inventory 1453) are cycle refusals, as the A32 cell says.
  - A28's two nearest cases exist at the cited lines (inventory 191 and 1611), and the cell honestly says they do not assert declaration history.
  - The retired cells of all four rows are appended to `history.md`. The history diff has 0 removed lines.
- **Advisories.**
  - A1: `red/RED.md` gained an append-only "C01 addendum" with the `validate_red_record` result (22 of 22), zero `COMPILE_OR_IMPORT`, the `build/rewrite-*` note and the ninja no-op.
  - A2: the B-round I24 row now quotes the matrix-relative link as code.
  - A3: `C03-H2` is defined as row 927 of `R19-ACCEPT-A.md`. That row is `H2` of the C02 table, and the test names match it.
  - A4: the `LOCAL-ONLY.txt` pointer is in 9 matrix rows, and `tests/native/r19-accept-a-c03-evidence/closing/LOCAL-ONLY.txt` is tracked.
  - A5: the E13 A deviation is in the roadmap open limits.
  - A6: recorded as a limit.
- **Round-record line rule (B-R5, last bullet).** No line of `R19-ACCEPT-B.md`, `acceptance-matrix.md` or `roadmap.md`, nor any added line of `history.md`, holds both `REPORT` and a 64-hex value. The 64-hex values in the round record are `src` or support-tree identities, file hashes of support files and the candidate sha256. None is the hash of a record.
- **Candidate record.** `tests/native/r19-accept-b-evidence/candidate/candidate.json` (1,745 bytes) records the reuse of the R19-ACCEPT-A candidate as deviation E3, with the reviewer's equivalence argument. Its sha256 and size equal the binary, and its binary path is redacted to `<scratch-root>`.
- **Checks on the draft list.** I ran these unpiped, from the repository root:
  - `run_acceptance.py commit-list --list tests/native/r19-accept-b-c01-evidence/raw/final/COMMIT-LIST.txt --repo-root .` exits 0 (`commit-list: 11 entries`).
  - `scripts/redact-local-paths.py --repo-path <repo> --home <home> check` on those 11 files plus the B-round REPORT, the C01 README, `PROBES.md` and `candidate.json` exits 0 with no output.
  - A link and anchor check over the round record, the matrix, the roadmap and the C01 README finds 0 broken links in the working tree. See R5 for the committed tree.

## Findings

### R19-ACCEPT-B-C01-R1 (blocking, needs a run): A34 still has no recorded run or committed evidence

This is acceptance criterion 1 and closes B-R1. None of its deliverables exists:

- `tests/native/r19-accept-b-evidence/a34-readback.json`;
- `tests/native/r19-accept-b-evidence/README.md`;
- the final REPORT sections: A34 readback manifest, Red-first evidence, Candidate, Evidence id index, Prober isolation, and Commit list and redaction;
- a C01 `COMMIT-LIST.txt`.

`find` over both evidence folders returns no `a34-readback.json`. The matrix A34 cell still reads `**pending the run**`. The only commit-list and redaction runs were on the draft 11-entry list, which contains neither the evidence nor the candidate record.

The tooling to make the run now exists and is probed: `run_acceptance.py provenance` writes `a34-readback.json` (M2), with verification on (M1). What remains is to do the following:

1. Run it for the record, on a fresh scratch ledger with scratch `HOME` and `PLANAR_DB`.
2. Commit the path-free manifest and write the README and the final REPORT.
3. Write the A34 cell as `**met pending close review**`, citing both halves by reference (P5).
4. Build the final list, then run `commit-list` and the redaction check on it, unpiped.

### R19-ACCEPT-B-C01-R2 (blocking, needs a run): A03 has a tested scenario but no recorded run, and its cell still reads `deferred`

This is acceptance criterion 3 and closes B-R3. The scenario, its 18 tests, the red capture and the probes hold, as shown above. However, P7 was not done: no `run_acceptance.py checkpoint` run for the record, no `a03-readback.json`. A03 therefore cannot be met yet. No product defect was observed: the CLI test runs the subcommand against the recorded candidate and passes.

The cell also keeps the wording that B-R3 objected to. It says `**deferred**` and names the roadmap R19 section as its "Governing source". That source requires the row; it does not defer it. Until the run exists, the cell should say open and pending the A03 run, not `deferred`.

Required:

1. Make the recorded checkpoint run.
2. Commit the path-free `a03-readback.json`.
3. Write the A03 cell from it, with all ten checks true and the informational flag false (P8).
4. State the rationale half per P21: either cite verified existing evidence or name it as uncovered scope.

### R19-ACCEPT-B-C01-R3 (blocking): B-R5 is not closed, and the C01 report adds a new pin over the records

This is acceptance criterion 5, first clause.

- `tests/native/r19-accept-b-evidence/REPORT.md` is unchanged from the B round:
  - Lines 3, 12 and 13 still carry the full identity hashes `f7a86fe5…e449` and `954c7340…2c60`. Their pathspec includes `docs/vernacular`.
  - Line 15 still lists truncated sha256 values of `acceptance-matrix.md`, `history.md`, `roadmap.md` and `R19-ACCEPT-B.md`.
  - It still mixes "FINAL RUN" with "This is a PRELIMINARY report" and `prelim-2` tables.

  The round record admits this as "partly met" (5c) and lists P10 as not met. The REPORT is a planned committed file.
- `tests/native/r19-accept-b-c01-evidence/REPORT.md` line 37 pins `6c6ba511…b398`, described as "`sha256(git diff --binary HEAD)` over the whole checkout, tracked files only (docs and support)". That is a hash over the matrix, history and roadmap, which this round's own rule forbids. The file is on the draft commit list. `commit-list` does not catch it because the line lacks the word `REPORT`, so the pin scan must be run separately, as P10 requires.

Fix: rewrite the B-round REPORT as one final report with the R1 sections, citing the candidate and the baseline by reference. Then remove the line-37 hash, or leave the C01 report off the commit list.

### R19-ACCEPT-B-C01-R4 (blocking, operator decision and records): R19 completeness is not re-decided, the roadmap is stale, and A28 needs the operator

This is acceptance criterion 6.

The roadmap opening and open limits still say the following, all from before C01:

- "no mutation probes";
- "A03 has no owner and no R19 id exercises it";
- "A15, A20, A28 and A32 name no ran-green cases";
- "the matrix rows still open are A03, A34 …, A15, A20, A28 and A32 (evidence naming)".

The matrix and the C01 sync contradict this: A15, A20 and A32 are now met, and A28 is open on substance, not on naming. The roadmap must be brought in line with the matrix in all three places: the opening, the R19 row and Next work.

The C01 downgrade of A28 to `open` is honest. By the round's own rule E27 and P16, however, R19 cannot be complete while A28 is open, and no existing source defers it. The C01 sync records that "the operator question ... is not yet raised". Under CLAUDE.md, unresolved product choices go to the operator. That decision cannot be made inside a corrective round. Options:

- **Recommended:** an explicit operator deferral of the optional-byte, deletion, rename and mtime declaration-history obligation to a named later phase, with an owner row in Next work.
- A further evidence round, either an R19 scenario or exact product cases.

Until the operator answers, the roadmap should name A28 as awaiting the operator, alongside A03 and A34, together with the open limits criterion 6 names: the E13 A deviation, A21 deferred, controlled A34 deferred, and x86_64 Linux.

### R19-ACCEPT-B-C01-R5 (blocking, records): the committed evidence set is undecided, and committed links would break

The round record links to the following plan files:

- `../../../tests/native/r19-accept-b-evidence/plan/DECISIONS.md` (B-round table);
- `../../../tests/native/r19-accept-b-c01-evidence/plan/DECISIONS.md` and `plan/INVARIANTS.md` (C01 sync and C01 table).

The A1 addendum calls `red-c01/RED-C01.md` "committed". All of these files are gitignored and absent from the draft commit list, so those links would break once committed. The C01 evidence lives in a second folder, `r19-accept-b-c01-evidence/`, which the dispatch did not name. The record carries the placement as "not yet decided" (Amendments, Evidence placement).

Also note: `run_acceptance.py`'s `COMMIT_LIST_SUPERSEDED_ALLOWED` does not cover `plan/superseded-N/` under either new folder. Committing the superseded plan copies, as the R19-ACCEPT-A precedent did, would make `commit-list` fail.

Required: decide the committed set in the final `COMMIT-LIST.txt` (P23). It should contain at least:

- the two REPORTs, if kept;
- the C01 README and the new A34/A03 README;
- `a34-readback.json` and `a03-readback.json`;
- `candidate/candidate.json`;
- `probes/PROBES.md`;
- the plan `DECISIONS.md` and `INVARIANTS.md` that the record links to;
- `RED-C01.md`.

All must be 100 KB or less, and redaction-clean. Otherwise reword the links to plain local paths and drop the "committed" claim.

## Advisory

- **A1 (gate-runner summary).** The dispatched gate summary reports `commit-list: 11 entries` with exit 0 as if it were the final check. It was run on a draft list that omits every evidence file the round must commit. Future summaries should say "draft list".
- **A2 (live checkpoint reds).** The nine live checkpoint rows were red only through the shared-fixture abort. That is acceptable here because each named assertion has a probe kill. For the A03 flip, cite the probe rows (C10 to C18), not RED-C01, as the proof that those assertions can fail.
- **A3 (C03 src identity).** The round record (C01 sync) and the C01 REPORT cite the C03 `src` tracked identity `d9ca75a1…26c0`. It is a `src`-only hash, not a record hash, so it is allowed. It is not in the I29 allowed-set files of this round, though, so the pin scan in R3 should whitelist it explicitly.

## Invariant table check

`tests/native/r19-accept-b-c01-evidence/plan/INVARIANTS.md` (revision 4) has the following rows: X1-X8, M1-M7, V1, C1-C24, the originals I1-I19, I31, I32 and I41, P1-P25, and the matrix target rows A03, A15, A20, A28, A32 and A34. The round record's C01 table carries every X, M, V, C, I and P row. It condenses the matrix target rows into one status line, and it adds the probe-only rows E1-E20, R1-R5, T-1 and T-2.

- Every acceptance criterion and every closed finding of the dispatch has a row:
  - R1: P1-P5 and P23-P25.
  - R2: X1-X7, the C rows and P6.
  - R3: P7, P8 and P21.
  - R4: P9.
  - R5: P10 and P11.
  - A1-A6: P15, P19 and P20.
  - Completeness: P16.
- Each test row's mutant breaks its invariant. I confirmed this for X1, X3, X4, X6, X7, I16, C6, C12 and C14 by re-running them.
- The I13 original mutant aborts in `setUpClass`, and the record correctly does not count it.
- For I9, I14, I15 and I31, the original-text mutants were not re-run. Under the verify-off fixture (X8), X4, X6 and X7 are the same edits as those originals, and I9's original mutant was shown hollow in the B review, so X1 and X2 replace it. I accept P6 with the record's own qualification.
- Every expectRed-true test failed by assertion in RED-C01. X6's expectRed of false did not hold, and that deviation is recorded.
- The process rows not met are P1, P3, P4, P5, P7, P8, P10, P16, P21, P23 and P24. P25 is met in content only. These are findings R1 to R5 above. No row is hollow, but those rows are unfulfilled.

## Limits of this review

- I re-ran 11 mutants, not all of them: 6 of the 56 pass-2 mutants (X3, C6, C12, C14, E8 and E16) and 5 carried mutants (X1, X4, X6, X7 and I16).
- The `HOME=scratch` label in the prober's logs is a label, not the value. Prober isolation rests on that label, the separate checkout and the unchanged `~/.planar` names.
- I started `quota_live_test` and `fault_live_test` in the scratch clone but did not wait for them. When this review was written, `quota_live_test` had passed its first 18 tests and none had failed. Their result here rests on the C01 gate logs (28 and 33, exit 0) and on their files being unchanged since the B review, which ran them green.
- No ctest, Release, Docker or Linux gate ran. `src` is unchanged, and the C03 gate is cited.
- I re-ran `accept_lib.validate_red_record` only on the 18 checkpoint red logs of RED-C01, with the recorded snapshot hash and an assumed exit of 1. It accepted 18 of 18. I did not re-validate the provenance rows.
- My local evidence under `tests/native/r19-accept-b-evidence/review/c01/` is gitignored, contains no host paths beyond the redaction outputs, and is not for commit.
