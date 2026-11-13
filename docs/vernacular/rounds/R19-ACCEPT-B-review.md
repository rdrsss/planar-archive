# R19-ACCEPT-B review (uncommitted tree on top of 080f1666)

Disposition: **request-changes** (not records only: one blocking finding needs a fresh evidence run, one needs the mutation prober, and one needs an operator decision or an evidence run).

The substance of the A34 provenance work holds. The driver writes an honest `driver_provenance_v1` document. The candidate accepts it on `claim` and `takeover`. `history --event` reads it back with authority `driver_assertion_unverified` next to the native derived block, and the no-file takeover reads back the all-unknown `none` shape. No product defect was found, and `src/` is unchanged. A24, the confirmations and the carried items are recorded with cites that hold when opened.

The round does not meet its own close criteria, and the coordinator's sync section already says so:

- The A34 matrix cell still reads `**pending the run**`.
- The committed evidence the plan requires does not exist: `a34-readback.json`, `README.md`, and the REPORT sections "A34 readback manifest", "Red-first evidence", "Candidate" and "Evidence id index".
- No mutation probe ran.
- A03 is open.
- The REPORT that would be committed pins hashes of records.

R19 is correctly **not** declared complete. The roadmap names A03 and A34 as what remains, which is consistent with the matrix.

The reviewer was not the coder or the coordinator. Writes were limited to this file and the new local folder `tests/native/r19-accept-b-evidence/review/`, which is gitignored and not for commit. Nothing was staged or committed. No build, ctest, Docker or Linux gate ran, and no installed or legacy binary ran. In the checkout the reviewer ran only:

- read-only commands: `git diff`, `git status`, `git show`, `shasum`, `grep`, `sed` and `awk`;
- the workflow `identity.py src`;
- `scripts/redact-local-paths.py check`;
- a link and anchor checker kept in the scratchpad.

The five `r19-accept-support` suites ran from `tests/native/r19-accept-support` with `python3 -B`, `PYTHONDONTWRITEBYTECODE=1`, scratch `HOME` and scratch `PLANAR_DB`. `PLANAR_CANDIDATE` was the recorded A candidate. The reviewer also made one `run_acceptance.py provenance` run (`--raw-root` in the session scratchpad, scratch `HOME` and `PLANAR_DB`) to read the stored records directly. No `.pyc` or `__pycache__` appeared in the checkout. No mutant was applied, because only the prober applies mutants. The mutant analysis below reads the code and does not run it.

## What was verified and holds

- **Source unchanged.** `git diff HEAD --stat -- src` is empty (exit 0). `identity.py src`, run by the reviewer, prints exactly:

  | identity | value |
  |---|---|
  | HEAD | `080f1666ca5c095b7c02fb4526dca10e95937262` |
  | sha256(git diff --binary HEAD -- src), tracked only | `e3b0c44298fc1c149afbf4c8996fb92427ae41e4649b934ca495991b7852b855` |
  | combined sha256 (diff bytes, then each untracked path NUL bytes, sorted), pathspec src, evidence excluded | `e3b0c44298fc1c149afbf4c8996fb92427ae41e4649b934ca495991b7852b855` |

  This equals `identity/identity-src-start.txt` and the round record's paste.
- **Support-tree scope.**
  - `git diff -U0 HEAD -- tests/native/r19-accept-support/run_acceptance.py` has 13 added lines and no removed line: the `cmd_provenance` handler and its subparser. Every existing message is byte-identical.
  - No other tracked support file changed.
  - The only new files are `provenance_driver.py` and `provenance_test.py`. No existing test name was removed or renamed.
- **Honest document (`execution-contract.md` P2-4, lines 1106-1139).** `provenance_driver.build_document` emits every key: `version`, `driver`, `trial`, `view`, `model`, `session`, `settings`. The values are:
  - `driver.name`: the real file name.
  - `driver.version`: the first 12 hex digits of the driver file's sha256.
  - `trial.id` `r19-accept-b-a34-minimal`.
  - `view.id` `source`. This is the real root id of the R19-CMAKE workspace (`tests/native/r19-cmake-support/workspace/workspace.lua:5`), not an invented value.
  - `model` and `session`: all four leaves are exactly `{"status":"unknown"}`, so nothing about a model or session is invented.
  - `settings`: six real facts as string leaves, sorted by unsigned name bytes: `accepted_revision`, `baseline_ref` `58128234:src/engine`, `baseline_sha256` (a tree digest), `candidate_sha256`, `candidate_size` and `driver_phase`.

  `validate_document` applies the leaf rule (1 to 1024 bytes, no NUL, the exact unknown object), at most 32 settings and unique names before any file is written.
- **Read-back, reviewer's own run** (local copy: `tests/native/r19-accept-b-evidence/review/a34-review-run-result.json`):
  - The claim (event `run_claimed`, request `claim-builder-1`) and the takeover (`run_taken_over`, `takeover-1`) each read back with `driver.authority` `driver_assertion_unverified`. Their record ids differ (39 and 43).
  - Each document equals the supplied one: known leaves as supplied, model and session leaves `{"status":"unknown"}`, `driver_phase` `claim` or `takeover`.
  - Each has the ten-key `native` block: ledger, revision `1`, source `{id, xxh3_128 digest}`, job `combined`, role `builder`, generation `1` or `2`, base_label `baseline_head_cmake58128234`, `xxh3_128:` work and render digests, and preferences `{model, session}` unknown.
  - The no-file takeover (`takeover-2`) reads back `id` `none`, `authority` `none`, the all-unknown document with `settings: []`, and generation `3`.
  - The `--provenance` argument appears on the claim and the first takeover and not on the no-file takeover.
  - None of the three run tokens (36 bytes each) appears in any of the 4 retained history stdouts, in the two provenance files or in `result.json`.
  - `candidate_sha256` in the document equals `shasum -a 256` of the candidate binary.
- **Scratch isolation.**
  - The scenario's child environment sets `HOME` and `PLANAR_DB` under the run's scratch root (`fault_driver.py:322`), and `result.env` is copied from that environment.
  - The reviewer's runs used scratch `HOME` and `PLANAR_DB` at the outer level as well.
  - The top-level entry names of `~/.planar` were identical before and after all reviewer runs (`review/planar-names-before.txt` and `review/planar-names-after.txt`).
- **Suites, reviewer runs** (`review/*.log`, `review/suites-summary.txt`), all exit 0 with zero failures and zero skips:

  | suite | tests run |
  |---|---|
  | `provenance_test` | 22 |
  | `accept_test` | 63 |
  | `acceptance_cli_test` | 29 |
  | `quota_live_test` | 28 |
  | `fault_live_test` | 33 |

  The total is 175, which matches `raw/final/`.
- **Candidate equivalence.** The candidate is the R19-ACCEPT-A C01 binary: sha256 equal to its `candidate.json`, built from 91e0df24 plus the two overlay files. The overlay bytes equal HEAD's `storage.cpp` and `execution.t.cpp`. `git diff --stat 91e0df24 080f1666 -- src cmake CMakeLists.txt CMakePresets.json vendor` lists only those two files and `src/engine/ledger/CMakeLists.txt`. That third change is the compile definition `PLANAR_LEDGER_SCHEMA` of the test-only target `planar_ledger_format_tests`, so the `planar_authoring_stack` product bytes are unaffected. The reused candidate is therefore equivalent to a fresh HEAD export for this run, though the dispatch expected a fresh one (see R1).
- **Red-first.**
  - `red/snapshot/` and `red/snapshot-2/` hash to the recorded values. The test file is byte-identical before and after (`18abff01…dcd3`), so the green tests are the red tests.
  - The pre-fix driver was a stub. Its refusing functions raise `AcceptError("not implemented")`, and its other functions return wrong values.
  - Every one of the 22 red logs ends in `AssertionError` with fixed text and has no import or compile marker. The exact command is in `RED.md`.
  - Twenty reds are behavioural, failing on missing behaviour. Two qualify: see A1.
- **A24.** Every cite was opened and holds:
  - `verdict.t.cpp`: TEST_CASE at line 290; `unsupported_policy` checks at lines 297, 305 and 312; line 307 is a comment.
  - `review_lua.t.cpp`: TEST_CASE at line 534 with `unsupported_policy` at line 566; TEST_CASE at line 646 with `external_approval=not_configured` at lines 654 and 659.
  - `execution-contract-r18.md`: R18-7 at lines 245-250 and the table row at line 361.
  - `execution-contract.md` line 750.

  All three TEST_CASE names are in the C03 inventory, and the row claims no human approval.
- **Refusal rows A04, A27 and A31.** Each named TEST_CASE exists at the cited `src` line and is in the C03 inventory. So are the A35 and A36 names.
- **Matrix and history.**
  - All 36 rows have a disposition token, and the matrix rows equal the round record's dispositions table.
  - Every cited R19-ACCEPT-A row line (390, 426-490, 892) is the row the id names, and both table anchors resolve.
  - The history diff is additions only (182 lines, no removed line). All 36 retired last cells and the retired roadmap opening are found verbatim in `history.md`.
  - The link and anchor check over the matrix, the roadmap and the round record finds one broken link (A2).
- **Confirmations.** The cites for (a) to (h) were opened:
  - (b): `tests/native/r19-accept-evidence/plan/DECISIONS.md:11` and `README.md:132-135`, both committed.
  - (d): `execution-design.md:1305-1308` and `:1388`.
  - (e): `R19-ACCEPT-A.md:644-645`.
  - (f): `execution-design.md:1088-1090`, quoted verbatim.
  - (g): `R19-ACCEPT-A.md:1285` and `:1539`.

  C03-A1 is recorded as delegated, as the dispatch asked. (d) correctly says E13 A's Claude Code wording is not met.
- **Carried items.** Every listed item has a row: reviewer-packet wording, the captured-preview limit, the accounting audit, the configure timestamp, CL-1 to CL-6 (six rows), x86_64 Linux, #1321 and #1030 (separate rows), and the authoring-loaded activation mismatch. Each `carried` target names a roadmap Next work row that exists. The cited source lines hold:
  - `R19-ACCEPT-A.md:29-31`
  - `V5-02-R15-B.md:91`
  - `V5-02-R15-B-C01.md:44`
- **Redaction.** `redact-local-paths.py --repo-path <repo> --home <home> check` on the 7 edited files exits 0, and on `REPORT.md` exits 0 (both read unpiped). `inventory.txt` (175,751 bytes) is over 100 KB and is correctly absent from the commit list.

## Findings

### R19-ACCEPT-B-R1 (blocking, needs a run): A34 is not closed and its committed evidence does not exist

The run passes, both in `raw/final/provenance_test.log` and in the reviewer's run, but the round never recorded it as a result:

- `acceptance-matrix.md` A34 still reads `**pending the run**`. The round record's section "A34 minimal provenance run" says "no run record exists". Per I24, I25 and I38 the cell should now read `**met pending close review**`, stating both halves (supplied file and absent file) by reference.
- These files are absent: `tests/native/r19-accept-b-evidence/a34-readback.json` (I43), `README.md`, and `candidate/candidate.json` (I36). `REPORT.md` lacks the sections "A34 readback manifest", "Red-first evidence" (verbatim, I22 and I44), "Candidate" (I36 and I44), "Evidence id index" (I47), "Prober isolation" (I45) and "Commit list and redaction" (I30). `run_acceptance.py commit-list` was not run.
- Without these, A34 cannot be marked met from committed evidence: the only committed proof would be the test source. The raw logs that show the read-back are local.

Required:

1. Produce `a34-readback.json` from a recorded run, in path-free form per D35/I43.
2. Write the missing REPORT sections and `README.md`.
3. Record the candidate decision. Either build the fresh export the dispatch expected, or record the reuse of the A candidate as a deviation, with the equivalence proof above: the only `src` difference is the test-target compile definition in `src/engine/ledger/CMakeLists.txt`.
4. Write the A34 cell and the round record's A34 section from that evidence.
5. Run `commit-list` and the redaction check on the final commit list.

### R19-ACCEPT-B-R2 (blocking, needs the prober): no mutation probe ran, and one stated mutant cannot be killed

`probes/` and `probes/PROBES.md` do not exist. All 22 `Kills:` lines and the INVARIANTS mutants are untested claims (standing lesson 1; the round's own acceptance table marks this "not met"). The code reading below gives the prober specific targets.

- **I9 is hollow as written.** The mutant "`verify_readback` compares against the un-normalized supplied document" is equivalent on the test's fixture. `_doc()` builds `settings` already sorted (`sorted(values)`, all-ASCII names), so `document` equals `normalize_document(document)`. The test `test_verify_readback_accepts_exact_expansion` therefore still passes under the mutant. The real run is the same: `build_document` sorts too. The named mutant does not break the invariant on this fixture. Either:
  - change the mutant to "compare against the un-normalized stored document", which the reversed stored order kills; or
  - give the control a supplied document whose settings are out of order.
- **Indirect kills.** The mutants of I13 (read `job_activated`), I14 (omit takeover `--provenance` or reuse the claim file), I15 (token in the takeover document) and I31 (file on the final takeover) are caught by the driver's own `verify_readback` and `verify_absent_readback` inside `run()`. That abort lands in `setUpClass` and fails all eight live tests through `cls.error`. The named test's own assertion never runs. That still counts as a kill, but PROBES must record the failing test and message for each mutant, so it is clear which of these rows is a real second check (memory lesson: a test named for a clause often cannot detect it).
- I16's mutant must run with scratch `HOME` and `PLANAR_DB` in the prober's own environment (D37 and I45), as planned.

### R19-ACCEPT-B-R3 (blocking, operator decision or run): A03 is open, and "deferred" names no deferring authority

The downgrade of A03 from the plan's target `met` to `deferred` is correct: F6, F7 and L13 show fingerprint re-binding and approval, not that a checkpoint or rationale change moves only the render digest (`execution-design.md:754` and `:794`). No R19-ACCEPT-A id or support driver touches `checkpoint`.

The governing source in the cell, however, is the roadmap R19 item that requires the row ("close every applicable Render matrix portion"). That source requires the row; it does not defer it. Unlike A21, which Later phases item 1 defers, A03 has no owner or next step in the roadmap. R19 therefore cannot complete, and nothing scheduled will close the row.

This is an operator question; per CLAUDE.md, unresolved choices go to the operator. Options:

- **Recommended:** an evidence-only A03 scenario, like the A34 option A. It would checkpoint a running builder, then use `preview` or `preview --since` to show the next packet's render digest moving while `work_fp` and state stay equal. It would be test-first under `tests/native/r19-accept-support/`.
- An explicit operator deferral of A03 to a named later phase. With this option, R19 is declared complete with A03 deferred, which the current completeness rule (D18) does not allow without that decision.

Until the operator decides, the A03 cell should say "open, awaiting operator decision" rather than `deferred`.

### R19-ACCEPT-B-R4 (blocking, records only): some `met` rows rest on partial scope that the round's rule does not allow

The round's rule (D18, and the record's "Rule applied" paragraph) says a row is `met` only when a named R19 id exercises the row's principal mechanism, and that a refusal obligation needs the exact refusal ctest cases that ran green in C03. Four rows fall short:

- **A15** ("check input/output races are inconclusive"). The cited ids (L8, F1, F8) exercise only the interrupted-check path. The principal mechanism is race detection with the inconclusive outcome, which is left to the Current evidence column.
- **A32** ("cycles refuse"). The refusal half is left to R07 and R09 with no named ctest case.
- **A20** ("invalid draft leaves active revision"). The refusal half is left to R14-D8 with no named case.
- **A28** (exact optional-byte, deletion, rename and mtime history). This is the row's principal content, and no R19 id or named case covers it.

A12 and A13 state their partial scope honestly, and their principal mechanism is exercised, so they may stand.

Fix, the same way as A04, A27 and A31: name exact TEST_CASEs that ran green in the C03 gate on the unchanged `src`, grepped in `src/**/*.t.cpp` and the C03 inventory. Candidates found in `src` include `R17-C child mutation makes all evidence inconclusive` (A15) and `semantic phase cycles report actual deterministic closed traces` (A32). Otherwise downgrade the row and report it. Records only.

### R19-ACCEPT-B-R5 (blocking, records only): the REPORT pins record hashes, and one round-record line breaks the REPORT-line rule

- `tests/native/r19-accept-b-evidence/REPORT.md`, a planned committed file, pins hashes of other records:
  - Lines 3, 12 and 13 carry the full identity hashes `f7a86fe5…e449` and `954c7340…2c60`. Their pathspec includes `docs/vernacular`, so they hash the records and the roadmap.
  - Line 15 lists truncated sha256 values of `acceptance-matrix.md`, `history.md`, `roadmap.md` and `R19-ACCEPT-B.md`.

  The acceptance criterion forbids pins of other records or the roadmap, and these values are already stale.
- The REPORT also mixes a "FINAL RUN" paragraph with "This is a PRELIMINARY report" text and with tables "from prelim-2". Rewrite it as one final report with the R1 sections, citing the candidate and the baseline by reference.
- `R19-ACCEPT-B.md` line 428 ("Red-first evidence") contains the word `REPORT` and several 64-hex values on one line. That breaks the D29 and I29 line rule, which `run_acceptance.py commit-list` enforces as `COMMIT_LIST_REPORT_PIN`. Split the line, or drop the REPORT mention from it.

## Advisory

- **A1 (red quality).** Two reds are not purely behavioural, as REPORT caveat 1 admits:
  - `ProvenanceCliTest.test_provenance_command_writes_result_json_with_both_readbacks` failed because `accept_lib.py:598-599` refuses a candidate under `build/rewrite-*`, not because the behaviour was missing.
  - `test_provenance_command_aborts_before_first_action_on_candidate_mismatch` failed on argparse exit 2 for the missing subcommand. That is a fair behavioural red for an absent subcommand.

  `RED.md` does not record the `accept_lib.validate_red_record` result or the zero `COMPILE_OR_IMPORT` count that I22 requires. The red phase also ran `cmake --build build/rewrite-debug --target planar_tests` (`red/raw/build.log`: `ninja: no work to do`). That is harmless, but it touched the shared build directory, which the dispatch said would not be used.
- **A2 (link).** `R19-ACCEPT-B.md` line 500 (row I24) contains the matrix-relative link `[R19-ACCEPT-B](rounds/R19-ACCEPT-B.md)`, which does not resolve from `rounds/`. Quote it as code.
- **A3 (id label).** "C03-H2" is defined as row 892, which sits in the C02 table of `R19-ACCEPT-A.md`. The test names given are those of row 927. Neither label appears in `R19-ACCEPT-A.md`. Name the row's real table, or cite row 927.
- **A4 (local proof).** The proof that the named refusal, A24, A35 and A36 cases ran green rests on the C03 `inventory.txt`, which is local only, and on the committed C03 REPORT counts. Say so in the matrix or the record; the TEST_CASEs are in `src` at HEAD and can be re-run.
- **A5 (operator visibility).** Confirmation (d) accepts, under the standing delegation, a deviation from the operator-binding E13 A ("one recorded live model-driven run (Claude Code)"). Add this to the roadmap's open limits so the operator sees it, since it qualifies the claim that R19 is complete.
- **A6 (repair trace).** The prelim-1 driver bytes (`3d2130f8…3c`) were not kept, so the X1 repair diff cannot be reproduced. The prelim-1 logs are kept. Keep future pre-repair bytes.

## Invariant table check

`plan/INVARIANTS.md` (revision 4) has 47 I-rows. The round record copies all of them plus X1 and X2. Each test row (I1-I19, I31, I32, I41) has a red-check outcome that matches `red/RED.md`. Every probe outcome reads "not run" (R2). The non-test rows I20 to I47 are process or docs checks. The ones this review re-ran hold: I21, I23, I26, I27, I33, I34, I35, I39, I40 and I46. These remain unmet: I22 (validator result), I24, I25 and I38 (A34 cell), I29 (REPORT pins, line 428), I30, I36, I43, I44, I45 and I47.

## Limits of this review

- No mutant was applied. R2's analysis reads the code; it is not a probe.
- The reviewer's live runs used the same reused A candidate; no fresh HEAD export was built.
- No ctest, Release, Docker or Linux gate ran. The C03 gate is cited because `src` is unchanged and the only build-input difference since the candidate's source is a test-target compile definition.
- The reviewer's evidence under `tests/native/r19-accept-b-evidence/review/` is local, gitignored and contains host paths. It is not for commit.
