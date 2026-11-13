# R19-ACCEPT-B-C02 review (uncommitted tree on top of 080f1666)

Disposition: **approve**. The five C01 findings (C01-R1 to C01-R5) are closed by what is on disk, not only by what the records say.

- The A34 and A03 runs happened. Each ran on its own fresh scratch ledger, with the recorded candidate. Both readbacks match the raw output of the candidate.
- A28 is deferred by the operator, with a Later phases entry and a Next work owner row.
- No committed file pins a hash of a record, report, matrix, history or roadmap.
- The single commit list passes `commit-list`, the redaction check and a link check.
- `src` and the support tree are byte-identical to the C01 candidate.

The findings below are non-blocking. Two are process gaps of this round's local verifier (no probes, and the D1 file-name defect). Three are wording errors in committed records. None changes a disposition, a run result or a committed data file. No edit is required before the flip.

What this approval releases: the post-approval flip (pairs P1 to P15 in the round record), the insertion of this file into `COMMIT-LIST.txt` at its sorted position, the post-flip re-runs, and the commit of R19-ACCEPT-B, C01 and C02 from the explicit list. After the flip, R19 is complete with these open limits:

- the E13 A deviation: scripted drivers, no live Claude Code run;
- A21 composition, deferred;
- controlled A34 experiments, deferred;
- A28 declaration history, deferred by the operator on 2026-11-13;
- x86_64 Linux, never run.

The reviewer was not the coder, the coordinator or the prober. My writes were limited to:

- this file;
- the gitignored local folder `tests/native/r19-accept-b-c01-evidence/review-c02/`, which holds the commit-list, redaction, identity and verifier outputs;
- scratch files in the session scratchpad. The support suites ran from a fresh scratch root there.

I applied no mutant anywhere. Nothing was staged or committed. No build, ctest run, Release build, Docker run or Linux gate ran (`ctest -N` was used as an inventory only), and no installed or legacy binary ran. The real `~/.planar/planar.db` still has its 2026-11-07 modification time.

## What was verified and holds

### Candidate identity

I ran `identity.py src` from the repository root. It prints exactly:

| identity | value |
|---|---|
| HEAD | `080f1666ca5c095b7c02fb4526dca10e95937262` |
| sha256(git diff --binary HEAD -- src), tracked only | `e3b0c44298fc1c149afbf4c8996fb92427ae41e4649b934ca495991b7852b855` |
| combined sha256 (diff bytes, then each untracked path NUL bytes, sorted), pathspec src, evidence excluded | `e3b0c44298fc1c149afbf4c8996fb92427ae41e4649b934ca495991b7852b855` |

Both values are the empty-diff hash, so `src` equals HEAD.

`identity.py src tests/native/r19-accept-support` prints:

- tracked `2636b446e572892fb3f08087d56376f342579b04a262fd605499fc19bb1e5420`;
- combined `85fffe4b63e99f27ca2b0fc93c3e21b04e2f794d9e33b22aadb8ee4a44b45cbe`;
- per-file hashes:
  - `run_acceptance.py` `2bb38280e24005684fc67fcbbeff48d858471a169323053bc5cd8fc59336946c`
  - `checkpoint_driver.py` `be85f5eff833a1c3a5eab6b815c2f99507342d8caad988c22fa5156acd6cfc62`
  - `checkpoint_test.py` `ef6e7ef047e96bdd8d14d61cea903c2d365acdda3f39e52576e7f4e2779c2bdd`
  - `provenance_driver.py` `9470b069e5480d3065514ce31514a6ec4094460abe77c9d285886b48456d443e`
  - `provenance_test.py` `c6147318550b76c5daf950ec3dc88f9c56237557d6390dd1403b287dbb11bfa1`

Every value equals the C01 review and the C02 final sync.

I also checked for edits outside the support tree:

- No support file has a modification time on 2026-11-13.
- `git status --porcelain` for `src`, `scripts`, `cmake` and the support tree lists only the five C01-era support files.

Since the C01 review approved these bytes, no coder or repair coder has changed them in the checkout.

### Tests and diagnostics

No test was renamed or removed, and every diagnostic is byte-identical:

- `git diff --numstat HEAD -- tests/native/r19-accept-support` shows 55 added and 0 removed lines, all in `run_acceptance.py`, as at C01.
- The support tree is byte-identical to C01, so every test name and message string is the C01 one.
- `ctest -N` in `build/rewrite-debug` lists 1854 tests. Sorted, they are name-for-name equal to the C03 `inventory.txt`, which is local (see R19-ACCEPT-B-C02-R3).

### The recorded candidate

The binary named by the R19-ACCEPT-A gate `PATH` file has the sha256 `774bcd69809ead9e4cd7b6e06c74aa045ea9272a2321cd36f91bc9c76a99ab0c` and a size of 33982376 bytes. Both equal `candidate/candidate.json`.

### Support suites (reviewer run)

I ran the suites from `tests/native/r19-accept-support` with `python3 -B -m unittest -v provenance_test checkpoint_test accept_test acceptance_cli_test quota_live_test fault_live_test`, using a fresh scratch `HOME` and `PLANAR_DB` and `PLANAR_CANDIDATE` set to the recorded candidate. I read the exit code unpiped. The run exited 0 with `Ran 198 tests in 707.363s` and `OK`: 198 `ok` lines and no skipped, failed or errored test. Per module: `provenance_test` 27, `checkpoint_test` 18, `accept_test` 63, `acceptance_cli_test` 29, `quota_live_test` 28 and `fault_live_test` 33. These equal the gate counts.

### A34 run (closes C01-R1)

The run script `raw/c02-a34/run-a34.sh` starts with `mkdir "$ROOT" || exit 3`, so the scratch root cannot already exist.

- It exports scratch `HOME` and `PLANAR_DB` under its own root and reads the candidate from the `PATH` file.
- It writes `a34.exit` (`0`) and `DONE`.
- `env.txt` names a scratch root that differs from the A03 root and the suites root.
- The scratch root holds only an empty `home`, so no `.planar` was created.
- The run's own ledger is `a34-provenance/scratch/ledger.db`.

`tests/native/r19-accept-b-evidence/a34-readback.json` is byte-identical (`cmp`) to the run copy. I recomputed its content from the raw candidate output instead of trusting the driver's own checks:

- The `claim` and `takeover` driver documents are equal, as parsed JSON, to the supplied `scratch/provenance-claim.json` and `scratch/provenance-takeover.json`.
- The full `provenance` object of each of `claim`, `takeover` and `nofile` occurs verbatim in the candidate's `history --event 4`, `--event 5` and `--event 6` stdout (steps 014 to 016, each exit 0).
- The generations are 1, 2 and 3. Model and session are `{"status":"unknown"}`. The no-file takeover reads back authority `none` and empty settings.
- `facts` equal `result.json`. The candidate digest and size equal the candidate record. `accepted_revision` is the ledger load revision `1`, not a git commit.

The file is 5840 bytes. A grep for `/Users/`, `/private/`, `/var/folders` and the scratch strings finds nothing.

### A03 run (closes C01-R2)

The run has the same shape and isolation, with exit `0`, `DONE` and a distinct scratch root. `a03-readback.json` is byte-identical to the run copy. The digests come from the candidate's output:

- `before` is the step-008 preview and `after1` is the step-013 preview.
- `after2` and `control` equal the step-018 to 020 previews.
- The `since` render digest equals `after2`.
- All four `work_fp` values are equal.
- `since.material` is empty, and its sections show `checkpoint` `added`.
- All 19 recorded steps exit 0.

The ten checks are true and `checkpoint_section_omitted_by_budget` is false.

### Matrix cells (acceptance criteria 1 and 2)

- **A34** reads `**met pending close review**`. It is scoped to minimal `driver_provenance_v1` and cites the committed readback and the README. Controlled experiments stay deferred, with a link to Later phases item 2.
- **A03** reads `**met pending close review**`. It cites `a03-readback.json` and probe rows C10 to C18 in `PROBES.md`, and it states that the rationale half rests on R09. It does not contain the word "deferred". I re-read rows C10 to C18 of `probes/PROBES.md`: each names its killing test, and the C01 reviewer re-ran C12 and C14.
- **A28** begins with `**deferred (operator 2026-11-13)**` and links [Later phases](../roadmap.md#later-phases-deferred). It keeps the honest note that no R19 or C03 case covers optional-byte, deletion, rename or mtime history.

`met pending close review` is the amended pre-approval form (C02 Amendments, "A03 flip strings"). I accept the amendment: the cells become `met` only through pairs P1 and P2, which this approval releases.

I counted the dispositions from the matrix: met 31, met pending close review 2, deferred 2, not applicable 1, open 0. These equal the dispositions section of the round record.

### Roadmap (acceptance criterion 5)

- Later phases item 5, "Declaration history acceptance", names optional-byte, deletion, rename and mtime history.
- Next work has the row "Declaration history acceptance (A28)" with an owner: a later scoped round, no id assigned.
- The roadmap opening, the Open limits bullet, the `### R19` Status line and the Next work `R19 acceptance` row all carry the same pre-approval sentence. Each lists the five open limits, and they agree with the matrix.
- The `### R19` scope sentence now names the A28 deferral.
- The stale B-round texts named in C01-R4 are gone from the roadmap and the matrix.
- `history.md` keeps every line of HEAD and of the pre-C02 snapshot in order (0 removed lines against HEAD). It has the new heading `Status retired at R19-ACCEPT-B-C02 (2026-11-13)`.

### Flip pairs

I applied the listed pairs P1 to P15 to in-memory copies of the matrix, the roadmap and the round record:

- Every old string occurs exactly the stated number of times.
- Afterwards neither the matrix nor the roadmap contains `pending close review`, `not yet declared complete` or `awaits`.
- The round record's status line reads "R19 is complete (R19-ACCEPT-B-C02 approved on <date>, ...)".

The flip can be executed as written.

### Hash pins (acceptance criterion 3, closes C01-R3)

I scanned every file on the 27-entry list for hashes of documents, without using the round's verifier:

- I hashed every file under `docs/`, every `.md` and `.txt` under `tests/native`, every HEAD blob under `docs`, and the `git diff --binary HEAD` of `docs`, `docs/vernacular` and the whole checkout. I then searched every list file for any hex run of 8 or more characters that is a prefix of one of those hashes. The only hits are the empty-file hash (`e3b0c442…`), which is the `src` empty-diff identity, not a document pin.
- I classified every 64-hex value in the listed non-`.py` files by context. Each one is one of:
  - a `src` identity, a support-tree identity or a support-file hash, current or historic (allowed by the criterion 3 amendment);
  - the candidate sha256;
  - the sha256 of a committed evidence data file (`a34-readback.json`, `a03-readback.json`, `candidate.json`);
  - the baseline engine digest inside the readbacks;
  - the C03 `src` identity, in a historic section of the round record;
  - a red-snapshot hash of support code in the RED files.
- The removed docs pins are absent from every list file except the two immutable reviews. These are the two `docs/vernacular` pathspec identities, the four truncated matrix, history, roadmap and record file hashes, and the C01 whole-checkout diff hash, checked by value against the local baseline list. The two reviews quote them as findings, which the scoped review reading permits.
- Neither REPORT contains `PRELIMINARY`, `prelim-` or `FINAL RUN`. Each names one run.

This review quotes no hash of any document.

### Commit list, redaction and links (acceptance criterion 4, closes C01-R5)

I ran these from the repository root after the last C02 edit, on exactly the 27 entries:

- `run_acceptance.py commit-list --list tests/native/r19-accept-b-evidence/COMMIT-LIST.txt --repo-root <repo>` exits 0 (`commit-list: 27 entries`).
- `scripts/redact-local-paths.py --repo-path <repo> --home <home> check <the 27 entries>` exits 0 with no output. `<home>` is the account-database home, not a scratch `HOME`.

The list itself:

- It is sorted bytewise and unique.
- No evidence entry exceeds 100 KB. Only `history.md` and the round record are larger, and they are records, not evidence.
- It has no `raw/`, `snapshot`, `superseded-N` or `.log` entry.
- It names every evidence file the records link to: both `plan/DECISIONS.md` and `plan/INVARIANTS.md` files, `red/RED.md` in both folders, `red-c01/RED-C01.md`, `probes/PROBES.md`, `candidate/candidate.json`, both readbacks, both READMEs and both REPORTs.

My own link check covered every Markdown entry. It skipped code spans and fences, resolved each target against the list or HEAD, and checked anchors against the headings. It found 0 missing files and 0 missing anchors.

Two further greps found no stray paths:

- A grep for `/private/`, `/var/folders`, `/Users/` and the scratch directory name finds only pattern mentions: the invariant rows that describe the grep, and `HOST_PATH_MARKERS` in `run_acceptance.py`.
- Exactly one `COMMIT-LIST*.txt` exists outside `raw/`. The four raw copies are left in place and described as superseded drafts.

`COMMIT_LIST_SUPERSEDED_ALLOWED` did not need extending, because no `superseded-N` file is listed.

### Local verifier

I ran `verify_c02.py --phase pre` read-only, with `python3 -B`. It reports 53 PASS, 1 SKIP (I50, post only) and 1 FAIL (`check_red_record_lists_all_expectred_checks`, which is D1). This matches the C02 final sync.

## Findings

### R19-ACCEPT-B-C02-R1 (non-blocking, process): D1 leaves I38 without a valid red and the verifier red

I38 (`check_red_record_lists_all_expectred_checks`) reads `red/RED-C02.md`. The record on disk is `red/RED.md`.

- In red run 1, I38 passed because a `RED-C02.md` existed at that moment.
- In run 2 it failed only because the file was missing.

Neither run tested the invariant, which is that the red record lists every check, so I38 has no behavioural red. The verifier is not green in phase pre.

What I38 is meant to guard does hold. I compared `red/RED.md` with the check functions of `verify_c02.py`:

- Every one of the 55 invariant checks is listed in it.
- Each expectRed check's `red/raw-2/` log begins with `FAIL`.
- Only the five guards (I15, I20, I25, I28, I29) pass.

No committed file depends on the verifier, which is gitignored. I record D1 as an accepted limit of the local tooling, closed by this review without a fix. The round record's "D1 (open, evidence owner)" bullet is to be read with this disposition.

### R19-ACCEPT-B-C02-R2 (non-blocking, process): no mutation probes for the 55 C02 checks

`tests/native/r19-accept-b-c02-evidence/probes/PROBES.md` does not exist. Every row of the C02 invariant table says "not run", and the round record's acceptance table honestly marks this "not met".

Under the instructions for this review, a row without a probe outcome is a finding. It does not block, for three reasons:

- C02 adds no committed test and changes no support or product byte.
- 49 of the 50 expectRed checks failed behaviourally on the pre-edit state, so none of them is a constant.
- I verified every acceptance criterion directly from the bytes and the candidate's raw output (sections above), without relying on the verifier.

The invariant table is otherwise complete: 55 rows, one per check, plus the C01-R1 to R5 closure rows. For the rows I checked against the bytes (I1 to I14, I20, I21 to I24, I30 to I35, I39, I53), each mutant would break the stated invariant.

### R19-ACCEPT-B-C02-R3 (non-blocking, records): the C03 inventory is called committed, and the gate report misstates the inventory comparison

The C02 final sync ("Tests and counts") says the `ctest -N` names equal "the committed C03 inventory". `tests/native/r19-accept-a-c03-evidence/inventory.txt` is not tracked. The A24 matrix cell correctly calls it local and uncommitted.

The local gate report says the names equal `tests/native/v5-02-x08-c01-r7-evidence/inventory.txt` with "zero differing names". That file lists 1260 names, not 1854, so the statement is false. The dispatched gate summary repeated it.

The substance holds: the 1854 names equal the local C03 inventory exactly. A later records edit should say "the local C03 inventory".

### R19-ACCEPT-B-C02-R4 (non-blocking, wording): three REPORT sentences mislead

- `tests/native/r19-accept-b-c01-evidence/REPORT.md` Limits says "E13 deviation (platform handling): the recorded R19-ACCEPT-A candidate is reused (deviation A)". E13 there is the C01 plan's local decision id, and the candidate reuse is E3. The roadmap's "E13 A deviation" is the execution-design decision about scripted drivers. The sentence conflates the two.
- Both REPORTs end with "Raw logs, snapshots and reviews stay on this machine". The round reviews are committed. The sentence means the local `review/` folders.
- The B-round REPORT says the commit-list and redaction results are recorded in "section C02 sync". That section marks them superseded; they are in the C02 final sync.

### R19-ACCEPT-B-C02-R5 (non-blocking, records): flip pairs P9 to P15 rewrite their own bullets

For the round record, the stated counts include the flip bullet itself. Replacing every occurrence therefore turns those bullets into "old <new text> => new <new text>". The old strings survive in the C02 sync sections and the local pre-flip snapshot, so nothing is lost. The flip section will, however, no longer show what it replaced. Future rounds should exclude the listing bullet from the replacement.

## Invariant table check

`tests/native/r19-accept-b-c02-evidence/plan/INVARIANTS.md` (revision 4) has 55 rows, I1 to I55. The round record's "Complete invariant table (C02)" carries every row verbatim, with host-path markers and removed pins replaced by placeholders, plus the closure rows for C01-R1 to C01-R5 and D1.

Coverage:

- Criterion 1: I1 to I11 and I39.
- Criterion 2: I12 to I14 and I49.
- Criterion 3: I21 to I24.
- Criterion 4: I30 to I35, I44, I45 and I51.
- Criterion 5: I16 to I19, I47, I48, I52 and I53.
- Criterion 6: I25, I27 to I29, I43, I54 and I55.
- Nothing left pending: I26, I36, I37, I42 and I46.

Status of the rows:

- Red: every expectRed-true row except I38 has a behavioural FAIL in both red runs, and the five guards pass (R1).
- Probes: no row has a probe outcome (R2).
- I found no row whose mutant would leave its invariant intact.

## Limits of this review

- I applied no mutant and did not re-run any red check. My confidence in the checks rests on direct verification of the outcomes, not on the verifier.
- No ctest run, Release build, Docker run or Linux gate. `src` equals HEAD, and the C03 gate (macOS Debug and Release and Linux arm64, 1854 of 1854 each) is cited. x86_64 Linux is uncovered.
- The A34 and A03 readbacks were recomputed from the candidate's recorded stdout. I did not repeat the two runs.
- The flip, the insertion of this file into the commit list, and the post-flip `commit-list`, redaction and link re-runs happen after this review. They are the flip package's duty (I50) and were not observed here.
- My local outputs under `tests/native/r19-accept-b-c01-evidence/review-c02/` are gitignored and not for commit.
