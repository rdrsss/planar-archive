# V5-02-R15-A1-C01 independent review (execution contract, part 1, corrective)

Disposition: **request-changes, not records only**. The contract text is now right. Every finding R1 to R12 of the
[R15-A1 review](V5-02-R15-A1-review.md) that needed a contract or alignment edit is closed at the line the closure
table cites. I re-read each of those lines and its surrounding text. The new CHECKs behave as stated when the format3
block runs in `sqlite3`. No diagnostic message, approved clause or check name changed. No `src` or `tests` byte changed.

The corrective is not finished as verification, and the round record says so itself ("not ready to be called complete by
its own acceptance table"). Two items block approval, and both need new evidence data, so a records-only fix cannot
close them:

- The D81 mutation prober never applied a mutant, and D81 has not been withdrawn either. This is the second half of
  review finding R6 and acceptance criterion 3.
- No committed-evidence log describes the final candidate. `green/checks.txt` does not exist,
  `green/sql-c01.txt` is the pre-C01 run, and the audit run exits 1.

Four smaller record defects should be fixed in the same pass (C01-R3 to C01-R6). Two wording points are advisory
(C01-R7, C01-R8).

Scope: the uncommitted R15-A1 candidate plus the C01 edits on HEAD `d536922b`. That is the seven candidate documents,
the round record [V5-02-R15-A1](V5-02-R15-A1.md) (all of it, including the C01 dispatch, the closure table, Final
results, D81 resolution, Acceptance (C01) and the C01 invariant table), the evidence folders
`tests/native/v5-02-r15-a1-evidence/` and `tests/native/v5-02-r15-a1-c01-evidence/`, and the frozen
copies of the roadmap and the R15-A1 review. I diffed the pre-C01 snapshot (`raw/pre-c01-snapshot/`) against the live
files to get the exact C01 change set. I read the sources behind the C01 edits: execution-design.md EX-1 (line 102, the
`evidence` row), lines 410 to 418 (R16 and R17 verb table and the `--confirm-stopped` note) and R17-3 (lines 879 to 883),
plus external-profile-contract.md lines 242 to 268 and ledger-contract.md lines 476 to 481. I ran no script under
`tests/native/`.

## Exact candidate

Workflow `identity.py`, run from the repository root on 2026-11-06 and pasted verbatim (also in
`tests/native/v5-02-r15-a1-evidence/review-c01/identity-c01-review.txt`):

| identity | value |
|---|---|
| HEAD | `d536922b59bf9118b91e4188a26d770f9745d600` |
| sha256(git diff --binary HEAD -- docs/vernacular/execution-contract.md docs/vernacular/v5-alignment.md docs/vernacular/ledger-contract.md docs/vernacular/external-profile-contract.md docs/vernacular/acceptance-matrix.md docs/vernacular/implementation.md CLAUDE.md), tracked only | `f48088154adefb428bdfc3187df4e9d55558c5a5955bc951201e3785aaec8b1f` |
| combined sha256 (diff bytes, then each untracked path NUL bytes, sorted), pathspec docs/vernacular/execution-contract.md docs/vernacular/v5-alignment.md docs/vernacular/ledger-contract.md docs/vernacular/external-profile-contract.md docs/vernacular/acceptance-matrix.md docs/vernacular/implementation.md CLAUDE.md, evidence excluded | `87aeabd8da8c5095501104a1f6d958aff629feab7cec02208dd26111fca52438` |

| file | state | sha256 |
|---|---|---|
| `CLAUDE.md` | modified | `7cdf4eadca490e6d3f6c6c697cd373c2c8a683f41612de02f98dce8b69b72163` |
| `docs/vernacular/acceptance-matrix.md` | modified | `6370cb09d99eaa33bf068f787e6ed040c3a81d160d6d2cbba47b3944e7178be5` |
| `docs/vernacular/external-profile-contract.md` | modified | `e190e2d1d88ad0a40cab3bd4bb31bbe826774a58f729224d083abcac877577da` |
| `docs/vernacular/implementation.md` | modified | `486a80a78d247e45badd73b2b96184bb952317d1229dbdba94d93cdd734e35fb` |
| `docs/vernacular/ledger-contract.md` | modified | `3e595653feeaa63f824f1f0bbf929a4923f10854dbcdd3e35725b8ff6223ceaf` |
| `docs/vernacular/v5-alignment.md` | modified | `58f5d441281c9b6d0dbf5049b604ba9a7f8129610e6a084a55959d4282449bd6` |
| `docs/vernacular/execution-contract.md` | untracked | `28635996d17a17f81af671c96c0bde33b5f4fd4b20be826755d27daa8267e323` |

| identity | value |
|---|---|
| HEAD | `d536922b59bf9118b91e4188a26d770f9745d600` |
| sha256(git diff --binary HEAD -- src tests), tracked only | `e3b0c44298fc1c149afbf4c8996fb92427ae41e4649b934ca495991b7852b855` |
| combined sha256 (diff bytes, then each untracked path NUL bytes, sorted), pathspec src tests, evidence excluded | `e3b0c44298fc1c149afbf4c8996fb92427ae41e4649b934ca495991b7852b855` |

| file | state | sha256 |
|---|---|---|

These match the record's "Final candidate identity" block value for value. The `src tests` identity is the empty input,
and `readlink AGENTS.md` prints `CLAUDE.md`. Against the pre-C01 snapshot, C01 changed only `execution-contract.md` (14
hunks) and `v5-alignment.md` (the E2 and E5 pointers). The other five documents compare identical with `cmp`. The
roadmap and the R15-A1 review are byte-identical to their frozen copies.

## Closure of R1 to R12

Each row was checked by reading the cited line and its context in the final text.

| Finding | Verdict | What I read |
| --- | --- | --- |
| R1 token exposure | closed | `execution-contract.md:537-539`: the token is shown in the claim and takeover results, in `status --request-id` of those requests and in format3 `history --request-id`, and "never shown in `status` other than its request lookup". `:518` says the saved `status --request-id` result carries the token. A09 at `:1017` recovers through `status --request-id` or `history --request-id`. D102 at `:1085`. This now matches R15-9, R15-7 and OP2. C39 corrects I54. |
| R2 event reason | closed | `:679`: the lowest ordinal, "which is ordinal 1 (the SQL has `CHECK(ordinal>0)`)". C5 at `:68` adds contiguity from 1 (writer-enforced, reader-verified). D43 `:1044` and D101 `:1084` agree. C40 corrects I57 and C48 corrects I67. `sqlite3` refuses ordinal 0 and accepts ordinal 1. |
| R3 OP1 wording | closed | OP1 `:1087` and D103 `:1086`: a required scalar option, so the rule "Flags cannot be required" is unchanged. `external-profile-contract.md:266-267` is untouched; its diff is the one inserted format3 paragraph. No "changes that rule" text remains in any document. C41 aligns I33. |
| R4 non-running role state | closed | `role_state` table CHECK at `:272`. The `transitions` CHECKs at `:309-310` sit beside the existing NULL-coupling CHECK at `:311`. The replay derivation of `run_id` is invariant 6 at `:436`, and the coupling is invariant 12 at `:445`. The EX-4 States text is at `:651`. D97 to D99 are at `:1080-1082`. I checked the derivation against rows M25 to M28 and N1 to N3. A refresh keeps its run (M28), a takeover writes the new run on the target (M27/N2), and claims come only from ready or stale (M26). So "the previous row is also running" holds on every path that keeps a role running. |
| R5 `evidence.reason` | closed (fallback) | C6 `:69` and D100 `:1083` say the column is native-writer enforced and reader-verified only, with no SQL CHECK. The basis is accurate. Design EX-1 (`execution-design.md:102`) says "closed enum, R17". R17-3 (`:879-881`) lists `fail` and `inconclusive` reasons but none for `pass`, while the column is `TEXT NOT NULL`. So the set is not fully listed, and the dispatch's else branch applies. The decision and its basis are recorded. |
| R6 checker and prober | **partly closed** | Checker repair: `strip_comments` and `sql_statements` in the evidence copy `red/ex_checks.py` drop leading comment lines before a statement. The old version kept `-- Reserved for R17` attached to the next `CREATE TABLE`, so `TABLE_RE` missed it. All 70 old check ids are still in the registry (`comm` of the id sets is empty). I60 and I69 (C55, C56) accept the M8 and M10 stale-to-ready rows through the "new generation" rule; M8 is "same as M7" at `:827` and M10 is "new empty generation" at `:829`. The sentence "No `stale` to `ready` row exists within one generation" is at `:810`. Zero FAIL and the prober are not evidenced: see C01-R1 and C01-R2. |
| R7 header | closed | `:5-8` lists EX-1 to EX-8, the state machines, the placeholders, messages, pointer amendments, traceability, checklist and decisions. |
| R8 M5/M7/M8 note | closed | `:864`. |
| R9 D55/D56 | closed | D55 `:1050` reads "I/O, corruption, busy and disk-full", matching `ledger-contract.md:481`. D56 `:1051` is new-engineering with basis "EX-8; the E15 row is D78". |
| R10 v5-alignment | closed | `v5-alignment.md:99-101`: E2 points to EX-4 (activation and re-activation) and EX-5. `:105` points E5 to EX-5 only. Both anchors resolve. |
| R11 cutover | closed | `:470` "at a new explicit target", matching `execution-design.md:80`. |
| R12 EX-8 check row | closed | `:801`: the "Existing site" cell is "None yet", and the `output_limit` outcome is labelled an R17 fact. |

Delegated decisions. D54 `:1049`, D67 `:1055`, D68 `:1056`, OP1 to OP3 `:1087-1089`, M41 `:860` and its note at
`:869` carry the exact tag `operator-decided (delegated 2026-10-31, option A)`. The Decisions preamble at `:1029` and the
closed message set at `:754` say they are recorded as delegated, not as live answers, and that the operator may overrule
them. No document, record section or plan file still uses the old pending wording, apart from the C01 dispatch paragraph
that quotes the phrase it replaces. One spelling variant remains (C01-R7).

## Findings

### V5-02-R15-A1-C01-R1 (blocking, evidence): D81 is neither run nor withdrawn

The C01 dispatch (record, Objective R6 and acceptance criterion 3) requires one of two things. Either the D81 mutation
prober runs over the final contract in a scratch copy, with one raw log per mutant and survivors listed, or the record
withdraws D81 explicitly with a reason. Neither happened:

- `probes/PROBES.md`, `probes/mutants.tsv` and `probes/raw/` do not exist.
- The three prober stages stopped at baseline audits (`probes/baseline*.txt`, 71 PASS and 34 FAIL each) on a scratch
  tree that held pre-C01 text.
- The record's "D81 resolution" says "not RUN, open" and "This record does not choose". It adds that withdrawal needs "an
  explicit operator decision". The dispatch does not require that: it allows a reasoned withdrawal in the record.

As a result, the "Probe outcome" column of the C01 invariant table is `not run` on every row. The 29 expectRed-true core
rows and the six audit rows are shown able to fail by the red runs. No row has shown that its listed mutant breaks the
invariant and is killed by the named check. This matters most for the rows whose check passed on the pre-C01 candidate.
`plan/INVARIANTS.md` lists them as C5, C24 to C28, C34 to C38, C44 to C47, C49 to C56 and C58 to C60, plus C59b and the
guard rows G1 to G8. Their ability to fail rests on nothing recorded, which standing lesson 1 forbids. Among them are C5
(the ordinal CHECK) and C55 and C56 (the repaired I60 and I69, which exist because the old checks could not tell a
correct row from a wrong one).

Fix, one of:

- Run the prober (prober only, scratch overlay of the final candidate outside the checkout). Write `mutants.tsv`, one
  raw log per mutant id, and `PROBES.md` with verdicts and survivors. Then write the D81 sentence with killed and
  survived counts.
- Withdraw D81 in the record with a stated reason, and record the residual limit: the guard rows above are unproven.

### V5-02-R15-A1-C01-R2 (blocking, evidence): no evidence log describes the final candidate

Acceptance criteria 2 and 3 require recorded results. The record's "Final results" puts the coordinator's re-runs (core
checker 99 PASS and 0 FAIL, SQL 23 of 23 as expected, 417 links) in record prose only. It explains that the outputs went
to the session scratchpad. In the evidence folder:

- `green/checks.txt` (the V1 core run) does not exist.
- `green/sql-c01.txt` is the pre-C01 run, with 7 of 23 statements differing from the expected result.
- Every `green/*-final.txt`, `green/*-closing.txt` and `green/identity*.txt` file describes pre-C01 text.
- The repaired checker's audit mode exits 1 with five FAIL lines (`CHK-C01-sql-evidence-recorded`,
  `-green-evidence`, `-prober-evidence`, `-record-closure`, `-record-d81`). The round's own closure-table R6 row cites
  the missing `green/checks.txt`.

My independent runs agree with the record's figures. The block executes with foreign keys on (24 tables, all STRICT; 43
triggers; the indexes `records_digest` and `run_state_one_active`; `foreign_key_check` empty). All 25 of my accept and
refuse statements give the outcome the contract states: role_state `run_id` and `drifted` for each non-running state,
the UPDATE paths, `after_drifted`, `before_drifted`, the NULL-first coupling, ordinal 0 and 1, `evidence.reason` with no
CHECK but NOT NULL, the outcome enum and a foreign key (`review-c01/sql-c01-review.txt`). The links also check out
(`review-c01/linkcheck-c01-review.txt`). The coordinator's scratch logs (`coord-default3.txt` 99 PASS;
`coord-audit3.txt` the five FAIL lines above) also match. Still, a reviewer's log is not the round's evidence, and the
audit checks the round wrote for itself fail.

Fix: write `green/checks.txt` (core run), a fresh `green/sql-c01.txt` and a fresh audit log from the final candidate.
Bring the audit run to zero FAIL. If an audit check is retired along with D81, the record must say which one and why.

### V5-02-R15-A1-C01-R3 (records): `red/RED.md` is hollow and contradicts the red evidence

The behavioural red is good. `red/red-run.txt` carries the verbatim identity of the pre-C01 candidate and the exact
command, and shows 70 PASS and 29 FAIL. The FAIL set equals the 29 expectRed-true core rows, and every message is an
assertion about text or SQL. `raw/red-checks/` holds one log per row.

`red/RED.md` describes something else. It records a `cmake --build build/rewrite-debug --target planar_tests` (ninja: no
work) and 69 `ctest -R '^<check name>$'` runs, all "No tests were found". Its outcome for every test is
"not-registered", and `red/raw/*.log` are those ctest outputs, with a host path in their first line. That is not red
evidence for this round. It also touched the build directory, which a docs-only dispatch and standing lesson 1 exclude.
The record's Evidence section cites "`red/RED.md` exists" as part of the red evidence and does not say what the file
says. The C01 README path table describes `red/` the same way.

Fix: relabel RED.md (or add a superseding note at its top) as a misdirected ctest attempt that is not red evidence.
Point it at `red/red-run.txt`, `red/red-audit-run.txt` and `raw/red-checks/`. Say the same in the record. Keep the logs.

### V5-02-R15-A1-C01-R4 (records): the record's status line contradicts its C01 section

Line 3 ends "C01 complete, candidate re-gated, pending review". The C01 section says the lineage "is therefore not
ready to be called complete". Its acceptance table marks criterion 3 not met and criterion 4 partly met. Fix: state the
true status on line 3.

### V5-02-R15-A1-C01-R5 (records): R15-A1's own acceptance criteria have no post-C01 status

R15-A1 and C01 are to be committed together. Section (l) is the only status table for R15-A1's criteria 1 to 10, and it
is labelled pre-C01, with criteria 1, 2, 3, 6 and 7 "not met as evidenced". Their bases are the failures that C01
repaired (I2, I5, I47, I53, I60, I64, I66, I67, I69 and I39). The C01 acceptance table covers only C01's eight criteria.
Fix: add a post-C01 status for each R15-A1 criterion, with its basis, or state row by row which C01 result discharges it.
Criterion 3's HALT-1 amendment (19 messages instead of "the single new message") should be stated there as amended.

### V5-02-R15-A1-C01-R6 (records and commit staging): redaction positive control and staged evidence

Acceptance criterion 4 is "partly met" in the record because the positive control was not re-run. I ran it: exit 1. All
seven candidate documents, the record, both reviews, the roadmap and the evidence READMEs, plan files and REPORT exit 0,
one file per invocation (`review-c01/redaction-c01-review.txt`). The record can cite that file.

Separately, the commit is to stage evidence "like f78f8eb4", which committed `inventory.txt`, plan files, PROBES and
REPORT files. Two kinds of file fail the check and must not be staged as they are:

- `tests/native/v5-02-r15-a1-evidence/inventory.txt`, which starts with the `ctest -N` "Test project <host path>" line.
  The committed X08 inventory has no such line.
- The R15-A1 `plan/superseded-1` to `-3` `INVARIANTS.md` files.

`raw/orig-checker-run.txt` and the `red/raw/*.log` files also fail. They are raw logs that stay gitignored. Fix: the
record's staging list should name which evidence files are committed and confirm each passes `check`. If an excluded
file is needed in the commit, it should be redacted under an explicit decision, not deleted.

### V5-02-R15-A1-C01-R7 (advisory): tag spelling and one sentence about the delegation

`execution-contract.md:520` says "Decision OP2 (operator-decided, delegated 2026-10-31, option A) applies". Every other
site uses the exact tag `operator-decided (delegated 2026-10-31, option A)`, so align this one.

Record section (c) says the standing delegation "does not record an operator decision, so the operator confirms each one
at commit review". Read next to the tag "operator-decided", that is confusing. The C01 dispatch wording ("recorded as
delegated, not as live answers; the operator may overrule them later") is the clearer statement.

### V5-02-R15-A1-C01-R8 (advisory): the original checker's FAIL count

`raw/orig-checker-run.txt` has 15 FAIL lines. The R15-A1 review and the C33 row text say 13, and V1 recorded 14. The
record notes the discrepancy and leaves it open. The extra ids I45 (diff scope) and I59 (record additions) are what
one expects from running on a tree whose record and diff had grown since 2026-11-01. One sentence saying so, or changing
C33's "lists its 13 FAIL lines" to the observed count, would close it. `CHK-C01-red-evidence` itself checks only the exit
code of that log.

## Checks that passed

- **No narrowing and no diagnostic change.** The C01 diff touches only the new, unapproved `execution-contract.md` and
  two pointer sentences in `v5-alignment.md`. No hunk falls in the Messages section (lines 924 onward) or the EX-7 code
  table, and the 19 new messages and the reused-message table are unchanged from the reviewed candidate. The approved
  contracts (`ledger-contract.md`, `external-profile-contract.md`) compare identical to the pre-C01 snapshot. Their
  R15-A1 diffs are insertions only, as the R15-A1 review found.
- **Names.** The 70 old check ids are a subset of the repaired registry. No product test name changed, because no `src`
  or `tests` byte changed.
- **Links and anchors.** 8 files, 417 links, 0 broken. The negative control (a missing anchor and a missing file) exits 1.
- **Record rule.** I computed the sha256 values of this record, the R15-A1 review, the roadmap and the R15-A1 REPORT
  only to search for them, and wrote none of them down. None occurs in `docs/vernacular` or either evidence folder. The
  record's hashes are `identity.py` outputs for the candidate documents and the empty `src tests` input. They include the
  pre-round snapshot line in section (a), which the R15-A1 review accepted under lesson 5. `red/RED.md` pins a hash of
  its own `SHA256SUMS` evidence data file, which is allowed.
- **AGENTS.md** is still a symlink to `CLAUDE.md`. The `CLAUDE.md` diff is the single authority entry.

## Evidence

New files under `tests/native/v5-02-r15-a1-evidence/review-c01/`, written by this review only. The scripts are copies of
the scratchpad files I ran. They were run only from the scratchpad, never from `tests/native/`.

- `sql_c01_review.sh` and `sql-c01-review.txt`: extraction of the format3 block (lines 82 to 355), its execution with
  foreign keys on, object counts, and 25 accept or refuse statements, each run on a fresh copy of the schema.
- `links_review.py` and `linkcheck-c01-review.txt`: the link and anchor check over the eight changed documents, plus
  the negative control.
- `redaction-c01-review.txt`: one `check` per file, the positive control, and the four failing evidence files named in
  C01-R6.
- `identity-c01-review.txt`: the workflow `identity.py` output for both pathspecs, and `readlink AGENTS.md`.

## Limits

- I did not run `ex_checks.py` (the canonical copy is in the session scratchpad; the evidence copy is byte-identical by
  `cmp`). My statement that the core checks pass on the final candidate rests on the coordinator's scratch logs and on
  my own reads of the clauses those checks target. I read the repaired SQL extraction and the C39, C40, C55 and C56 row
  definitions, not all 105 check bodies.
- I applied no mutant. That is the prober's job (C01-R1).
- My `sqlite3` is 3.51.0. The coordinator's probe reported 3.54.1. The counts and outcomes agree.
- I re-derived no part of `plan/DECISIONS.md` beyond D97 to D103 and the relabelled rows.
- No product build or test ran, because this is a documentation round.
