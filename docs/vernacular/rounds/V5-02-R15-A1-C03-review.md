# V5-02-R15-A1-C03 independent review (execution contract, part 1, third corrective)

Disposition: **request-changes, not records only**. Most of what C02 lacked now exists and holds up. The D81 prober
ran: `probes/PROBES.md`, `probes/mutants.tsv` and one raw log per mutant are present. I re-derived all 161 KILLED
verdicts and the 7 controls from their raw logs, and every one is supported. The guard rows C5, C24 to C28, C34 to
C38, C44 to C47, C49 to C56, C58 to C60, C59b and G1 to G8 each have kills that name their check. The C55-m1 note
is present. The runner cannot score a missing command as a kill. The seven candidate documents are byte-identical to
the C02 review, no `src` or `tests` byte changed, links and redaction pass on my own runs, and no record or evidence
file pins a hash of a round record, review, REPORT or the roadmap. C02-R4 and C02-R5 are closed.

C03 is still not finished. The round's own record says so: rows 9 and 10 of 'C03 acceptance' read **not met**. The
last FIN wave (FIN2) stopped when its audit run failed on clause 9 of `CHK-C02-evidence-logs`. The checker was then
repaired and two logs were rewritten, and the coordinator edited the record and the C01 README again. The result:

- Three GREEN2 logs do not exist. Two of them are files the staging list says will be staged.
- Every saved FIN log is older than the record and the README that would be committed with it.
- The round's redaction log leaves out `CLAUDE.md`, and the checker cannot see that gap.

These are gaps in the evidence data. Editing the records alone cannot close them, so a FIN re-run is needed before
commit.

Scope: the uncommitted R15-A1 plus C01 plus C02 plus C03 candidate on HEAD `d536922b`. That covers the seven
candidate documents and the round record [V5-02-R15-A1](V5-02-R15-A1.md): all of it, with weight on the C02 outcome,
the C03 dispatch, the 'V5-02-R15-A1-C03 outcome' section, the staging list and the C03 invariant table. It also
covers the [C02 review](V5-02-R15-A1-C02-review.md), the C01 evidence folder `tests/native/v5-02-r15-a1-c01-evidence/`
(`probes/`, `green/`, `README.md`, `red/RED.md`), `tests/native/v5-02-r15-a1-c02-evidence/red/RED.md`, and the C03
plan and gate folder `tests/native/v5-02-r15-a1-c03-evidence/`.

I ran no script from `tests/native/`. I ran the checker only as a `cmp`-identical scratch copy against a scratch clone
outside the checkout. I ran `scripts/redact-local-paths.py` from the repository. I ran a `cmp`-identical scratch copy
of `record_asserts_c03.py` read-only against the checkout (see Limits).

## Exact candidate

Workflow `identity.py`, run from the repository root on 2026-11-06 and pasted verbatim (also in
`tests/native/v5-02-r15-a1-c03-review-evidence/identity-c03-review.txt`). Seven candidate documents:

| identity | value |
|---|---|
| HEAD | `d536922b59bf9118b91e4188a26d770f9745d600` |
| sha256(git diff --binary HEAD -- docs/vernacular/execution-contract.md docs/vernacular/v5-alignment.md docs/vernacular/ledger-contract.md docs/vernacular/external-profile-contract.md docs/vernacular/acceptance-matrix.md docs/vernacular/implementation.md CLAUDE.md), tracked only | `f48088154adefb428bdfc3187df4e9d55558c5a5955bc951201e3785aaec8b1f` |
| combined sha256 (diff bytes, then each untracked path NUL bytes, sorted), pathspec docs/vernacular/execution-contract.md docs/vernacular/v5-alignment.md docs/vernacular/ledger-contract.md docs/vernacular/external-profile-contract.md docs/vernacular/acceptance-matrix.md docs/vernacular/implementation.md CLAUDE.md, evidence excluded | `499d95b7cd77a6476c4c50390d2b500c9f782dfa01676c2808b2d6bb00b756c7` |

| file | state | sha256 |
|---|---|---|
| `CLAUDE.md` | modified | `7cdf4eadca490e6d3f6c6c697cd373c2c8a683f41612de02f98dce8b69b72163` |
| `docs/vernacular/acceptance-matrix.md` | modified | `6370cb09d99eaa33bf068f787e6ed040c3a81d160d6d2cbba47b3944e7178be5` |
| `docs/vernacular/external-profile-contract.md` | modified | `e190e2d1d88ad0a40cab3bd4bb31bbe826774a58f729224d083abcac877577da` |
| `docs/vernacular/implementation.md` | modified | `486a80a78d247e45badd73b2b96184bb952317d1229dbdba94d93cdd734e35fb` |
| `docs/vernacular/ledger-contract.md` | modified | `3e595653feeaa63f824f1f0bbf929a4923f10854dbcdd3e35725b8ff6223ceaf` |
| `docs/vernacular/v5-alignment.md` | modified | `58f5d441281c9b6d0dbf5049b604ba9a7f8129610e6a084a55959d4282449bd6` |
| `docs/vernacular/execution-contract.md` | untracked | `2d909b8adea509d7140e2ba9f719891b549838949535a5f0de52c20f71e572d3` |

Source and tests (`src tests`):

| identity | value |
|---|---|
| HEAD | `d536922b59bf9118b91e4188a26d770f9745d600` |
| sha256(git diff --binary HEAD -- src tests), tracked only | `e3b0c44298fc1c149afbf4c8996fb92427ae41e4649b934ca495991b7852b855` |
| combined sha256 (diff bytes, then each untracked path NUL bytes, sorted), pathspec src tests, evidence excluded | `e3b0c44298fc1c149afbf4c8996fb92427ae41e4649b934ca495991b7852b855` |

| file | state | sha256 |
|---|---|---|

These values equal the C02 review's block and the record's 'C03 candidate identity' block value for value.
`readlink AGENTS.md` prints `CLAUDE.md`. The seven documents keep their identity, and the `src tests` identity is the
empty input.

## Closure of C02-R1 to C02-R7

| Finding | Verdict | What I read or ran |
| --- | --- | --- |
| C02-R1 prober | closed | `mutants.tsv` has 168 rows: 161 KILLED and 7 control-ok. Each row has exactly one `probes/raw/<id>.log`, and no log lacks a row. For every KILLED row I took the last `runner counts` line and the `exit code of the command` line of its log. Each one has a non-zero exit, a `FAIL <row test>` line, F >= 1 and P + F = R. Each control exits 0 with no FAIL line. The PROBES.md totals (main 95, closing 66) equal the tsv. All 35 guard rows have at least one KILLED mutant naming their check. The three real survivors (C58-m1, C60-m1, C31-m12) keep their first logs as `*.survived-1.log` and were re-probed KILLED. The C55-m1 note (killed by the `M table row count != 41` pin) is in PROBES.md and in the D81 resolution. The scratch runner (`probe_lib.py`) returns KILLED only under that same rule, and returns `not run` on exit 127 or empty output. See C03-R4 for six overwritten first-attempt logs. |
| C02-R2 final logs | **partly closed** | `green/checks.txt` (113 PASS, exit 0), `green/sql-final.txt` (25 of 25, foreign keys on, current contract identity), `green/audit-final.txt` (122 PASS, exit 0), `green/linkcheck-c02.txt` (11 files, 0 broken, negative control) and `green/redaction-c02.txt` exist. But all of them predate the final record and README, the GREEN2 logs are missing, and the redaction log omits `CLAUDE.md`. See C03-R1 and C03-R2. |
| C02-R3 record claims | **partly closed** | Line 3, the three `Superseded in C02:` notes (lines 329, 762, 819), the D81 resolution and section (l2) now point at files that exist. 'Owned files (C03)' still lists three files as written by FIN2 that were never written. See C03-R3. |
| C02-R4 C02 red wave label | closed | `tests/native/v5-02-r15-a1-c02-evidence/red/RED.md` has the section 'Label (C03): irrelevant first attempt', which names the build, the 58 ctest runs and the checker run from `tests/native`. The record's closure row says the same. (That RED.md is unstaged because of host paths, so the committed form of the label is the closure row.) |
| C02-R5 red column | closed | In the C02 invariant table, PR-C27, PR-G8 and N8 read `PASS (guard)`. All three are PASS lines in both `red/red-c02-run.txt` and `red/red-c02-audit-run.txt`. |
| C02-R6 red-checks wording | closed, with a residual inexactness | Both files now say 72 logs, one per C01 invariant row. The exact count differs slightly (C03-R6). |
| C02-R7 message wording | closed, with a residual inexactness | 'Checker amendments (C02)' now gives counts and cites `green/messages-diff-c03.txt`. I confirm 21 removed. The 36 added includes one C03 message (C03-R7). |

## Findings

### V5-02-R15-A1-C03-R1 (blocking, evidence): the FIN evidence is incomplete and older than what it describes

The C03 dispatch requires that every staging-list path exists and that redaction passes for every staged file. The
plan's own order rule (`ASSERT-C03-order`) requires the FIN logs to be newer than the record. Neither holds at review
time.

- **Missing files.** `green/audit-verify-c02.txt`, `green/redaction-final-c02.txt` and
  `green/record-asserts-final-c03.txt` do not exist. The staging list says that `green/audit-final.txt`,
  `green/audit-verify-c02.txt` and `green/redaction-final-c02.txt` "are staged after the last wave writes them" and
  that the redaction lines of these files live in `redaction-final-c02.txt` (D46). So two planned staged files are
  absent, and `audit-final.txt` has no redaction evidence in the round: `redaction-c02.txt` skips it "by rule".
- **Stale files.** The record (18:54:39) and the C01 README (18:54:56) were edited after:
  - every FIN log: `record-asserts-c03.txt` and `reprobe-c03.txt` (18:35), `linkcheck-c02.txt` (18:37),
    `identity-compare-c03.txt` (18:37), `redaction-c02.txt` (18:38), `checks.txt` (18:42) and `audit-final.txt`
    (18:42);
  - the gate report (18:45).

  `checks.txt` calls itself an "unconditional post-record run", and `audit-final.txt` says "after the final record
  edit". Neither is true of the bytes that would be committed. The core checks read the record, so these logs do not
  show the committed record passing.
- **The record agrees.** 'C03 acceptance' rows 9 and 10 are "not met", 'C03 remaining limits' says the FIN steps must
  run again, and my run of `record_asserts_c03.py --phase all` gives 18 PASS and 1 FAIL (`ASSERT-C03-order`).
- **Why FIN2 stopped.** The scratch FIN2 audit output shows the cause: `FAIL CHK-C02-evidence-logs the record quotes a
  result of audit-final: ...` on an indented plan quote. That failure led to the clause 9 repair (C03-R5).

My own runs over the current bytes all pass:

- the checker, default mode, 113 PASS, 0 FAIL;
- the checker, audit mode, 122 PASS, 0 FAIL;
- redaction, 60 of 60 files exit 0, with the positive control exiting 1;
- links, 519 links in 15 files, 0 broken.

So I expect a correct FIN re-run to come out green. But the round's evidence does not show it, and the commit would
stage logs that describe an earlier record.

Fix: make every remaining record and README edit first. Then run FIN1 and FIN2 completely: `record-asserts`, re-probe,
`identity-compare`, `checks`, `audit-final`, `linkcheck`, `redaction`, `audit-verify`, `redaction-final`, and
`record-asserts-final` last. Do not edit the record or README afterwards. If the coordinator must write a sync
section, it goes before FIN, or FIN runs again after it. Rows 9 and 10 can then read met from logs.

### V5-02-R15-A1-C03-R2 (medium, evidence and checker): `CLAUDE.md` is missing from the redaction log, and the checker cannot notice

`CLAUDE.md` is the seventh path of the staging list. `green/redaction-c02.txt` has 56 `exit 0` lines and none names
`CLAUDE.md` (`coverage-c03-review.txt`). The FIN2 extraction script keeps only backticked tokens that contain `/`, and
so does the checker's `staging_paths()` (`green/ex_checks-c02.py`, `def staging_paths`). The redaction clause and the
"Staging list path exists" clause of `CHK-C02-evidence-logs` therefore skip every root-level staged file. The current
log shows this hole directly: the log lacks a staged file, and `CHK-C02-evidence-logs` still passes in both modes. No
prober mutant covered it. `CLAUDE.md` itself is clean (exit 0 in my run and in the gate's `raw/redaction2.log`).

Fix: include `CLAUDE.md` in the FIN redaction run. Treat the extraction as a surviving mutant: let `staging_paths()`
accept a slashless backticked token that names an existing staged file, then probe it with "delete the `CLAUDE.md`
line from a scratch `redaction-c02.txt`" (new message only, no id change). If the round does not want a checker
repair, record the limit instead.

### V5-02-R15-A1-C03-R3 (medium, records): 'Owned files (C03)' and the acceptance rows still claim files that were not written

'Owned files (C03)' introduces its list as "the files edited or written by the C03 waves". It ends with
`green/audit-verify-c02.txt`, `green/redaction-final-c02.txt` and `green/record-asserts-final-c03.txt`, each labelled
"wave 8 FIN2". None of them exists. 'C03 acceptance' row 3 reads **met** for "every staging-list path exists". The
staging list's own closing sentence names two of those absent files as staged, and row 3 calls them exceptions to
itself. This is the C02-R3 pattern again, although 'C03 remaining limits' and line 3 do state the truth. It goes away
if C03-R1 produces the files. If any of them is dropped instead, reword these lines to the true state.

### V5-02-R15-A1-C03-R4 (medium, process): six first-attempt probe logs were overwritten

The runner writes straight into `probes/raw/` with `open(path, 'w')`. PROBES.md says that the first runs of N15-m2,
N9-m1, N9-m2 and C3-A3c-m1 survived because of defects in the mutant text, and that their logs "are the runs of the
corrected mutants". C31-m2 and C31-m8 also "survived once" before their mutants were changed. Unlike C58-m1, C60-m1
and C31-m12, none of these six has a `*.survived-1.log` or an attempt log. So the claim that each was a defect of the
mutant, not of the check, rests on PROBES.md prose alone, and the raw logs of those runs are gone. That goes against
standing lesson 6 (never delete raw evidence logs) and against the round's own convention. The explanations are
plausible. For example, `staging_paths()` reads only the `### Staging list (C02)` subsection, so a path appended
under an earlier mention of that heading cannot count. But I cannot verify them from evidence.

Fix (records): name the six overwritten first attempts in the record as lost evidence, with the reason each was
judged a mutant-text defect. In later rounds, the runner writes `<id>.attempt-<n>.log` before it re-runs a mutant.

### V5-02-R15-A1-C03-R5 (advisory): the clause 9 repair is permissive and has no mutant

After the final record edit, clause 9 of `CHK-C02-evidence-logs` skips every line that starts with `|` or whitespace.
In my scratch clone, with the final checker in audit mode:

- the plan mutant N9-m14 (a prose line `audit-final.txt: 122 PASS`) is still KILLED;
- the table row `` | x | `green/audit-final.txt`: 122 PASS, exit 0 | `` passes, exit 0 and 122 PASS;
- the indented line `  audit-final.txt: 122 PASS` passes, exit 0 and 122 PASS.

All three are in `probe-sample-c03-review.txt`. The record states the missing mutant as a limit, and the reason for
the repair (an indented plan quote describing D46) is real. Still, the check was relaxed so that the record would
pass. A narrower exemption would keep the rule, for example invariant-table rows whose first cell is a plan id plus
lines inside a quoted plan block. Otherwise, add the two mutants above to the record as known survivors.

### V5-02-R15-A1-C03-R6 (advisory): the `raw/red-checks` description is still not exact

`raw/red-checks/` holds 71 `.log` files plus `SUMMARY.txt` (72 files). There are 70 C01 rows, and C59b has two logs.
34 logs record a FAIL of their own check, not 29: the 29 FAIL ids of `red/red-run.txt` plus the 5 audit-only ids from
`red/red-audit-run.txt` (`CHK-C01-green-evidence`, `-prober-evidence`, `-record-closure`, `-record-d81` and
`-sql-evidence-recorded`). The C02 review proposed the "29 FAIL, the rest PASS" wording itself, so this is my
lineage's error carried forward. `ASSERT-C03-R6` passes because it counts directory entries. A correct wording: "71
logs (C59b has two) and SUMMARY.txt; 34 record a FAIL, 29 in default mode and 5 audit-only".

### V5-02-R15-A1-C03-R7 (advisory): the C02 "36 added" count includes a C03 message

Part 1 of `green/messages-diff-c03.txt` compares the C01 checker with an amended checker whose sha256 begins `9a0e3d27`.
That checker is neither the C02 checker (`raw/ex_checks-c02-preedit-c03.py`) nor the final one. My AST comparison of
`red/ex_checks.py` against the C02 pre-edit checker, over the eight amended checks, gives 21 removed and **35** added
(`messages-c03-review.txt`, Part A). The 36th is `probes/raw/%s has no row in mutants.tsv`, the C03 survivor repair for
C31-m12 (Part B shows exactly the three C03 additions and 0 removed). 'Checker amendments (C02)' should say 35, or say
that the count includes the C03 C31-m12 message.

### V5-02-R15-A1-C03-R8 (advisory): two small record accuracy points

- 'C03 candidate identity' pins the sha256 of `raw/ex_checks-c02-preedit-c03.py`. That file is under `raw/` and is not
  staged. Acceptance criterion 10 allows hash pins only of `src` and committed evidence data files. This is not a
  record-rule breach (it is not a record, review, REPORT or the roadmap), but the pin points at a file the commit will
  not carry. Say so, or rely on the copy of the same value inside `green/freeze-red-c03.txt`.
- The C03 invariant-table intro says that only the `E/`, `P2/`, `P3/` and `A1/` notation differs from the plan. In
  fact 19 cells also differ in other ways: `Final results (C02)` is written as `Final results labelled C02`, and some
  backticks are dropped, so that the record's own checks do not fire on it. Every row id and every mutant, test and
  expectRed value otherwise matches revision 4 of `tests/native/v5-02-r15-a1-c03-evidence/plan/INVARIANTS.md`. State
  the extra rewording.

## Checks that passed

- **Invariant table against the plan.** All 91 rows of the C03 plan are in 'Complete invariant table (C03)': 33 in
  section 0, 35 guard, 6 audit and 17 C02 checks. The record adds F-C02-R1 to F-C02-R7 and C3-CL9, 99 rows in all.
  After normalizing the evidence-path notation, the mutant, test and expectRed cells equal the plan, apart from the
  rewording in C03-R8. Every C03 acceptance criterion and every C02 finding has a row. Probe cells agree with
  `mutants.tsv` (`ASSERT-C03-probe-column` passes, and I spot-recomputed the guard rows).
- **Red-first.** `green/freeze-red-c03.txt` holds the command, the pre-edit checker path and its sha256 (equal to
  the file), the seven-document identity, exit 1, 112 PASS and one assertion FAIL (`CHK-I45-diff-scope` naming the C02
  review). `green/record-asserts-red-c03.txt` shows 12 assertion FAILs against `raw/pre-rec-record-c03.md`. Both are
  behavioural.
- **Checker messages.** C02 pre-edit to final C03: 0 messages removed or changed and 3 added (the three survivor
  repairs). No check id was removed or renamed: 122 ids register, and `CHK-C01-names-preserved` passes.
- **SQL.** `green/sql-final.txt` runs with `PRAGMA foreign_keys` 1 and reports 24 STRICT tables, 43 triggers, the
  explicit indexes `records_digest` and `run_state_one_active`, a clean `foreign_key_check` and 25 of 25 probe
  statements as expected. Its identity block carries the current `execution-contract.md` hash `2d909b8a...`.
- **No raw log deleted from the pre-C02 manifest.** All 320 paths of `raw/pre-c02-manifest.txt` exist. Only the
  allowlisted rewrites (`README.md`, `red/RED.md`) differ in size. C03-R4 concerns logs written after the manifest.
- **Record rule.** I took the sha256 of every round record and review, every evidence `REPORT.md`, the roadmap (live
  and at HEAD), the frozen copies and the pre-record snapshots, and compared them in memory with every 64-hex token
  in the record, the reviews, the roadmap and the four R15-A1 evidence trees: 819 files, 0 hits.
- **Every staging-list bullet exists.** All 58 bulleted paths exist. My redaction run passes all of them, plus
  `audit-final.txt` and the roadmap, and the positive control exits 1.
- **Links.** My own checker covered the seven documents, the record, the roadmap, the C01 README, both RED.md files,
  the C03 plan files and PROBES.md: 15 files, 519 links, 0 broken. The negative control reports 2 broken.
- **Paths named by the record.** Every backticked path in the record resolves, apart from the three GREEN2 files
  (C03-R1, C03-R3), mutant texts that name nonexistent files on purpose, and the historical sentence that a
  `v5-02-r15-a1-evidence/probes/PROBES.md` never existed.
- **Roadmap and contract untouched by C03.** `CHK-C01-frozen-files` and `CHK-I45-diff-scope` pass against the
  refreshed frozen directory, and no contract byte changed.

## Evidence

New files under `tests/native/v5-02-r15-a1-c03-review-evidence/`, written by this review only (gitignored, host paths
masked, each passes `scripts/redact-local-paths.py check`). The scripts are copies of the scratch files I ran, and I
ran them only from the scratchpad.

- `identity-c03-review.txt`: workflow `identity.py` for both pathspecs, and `readlink AGENTS.md`.
- `checker-c03-review.txt`: default and audit runs of the scratch checker copy on the scratch clone.
- `probe-sample-c03-review.txt`: the three clause 9 mutants and the recomputation of all 168 tsv verdicts from their
  raw logs.
- `coverage-c03-review.txt`: the `CLAUDE.md` redaction gap, file times of the record against the FIN logs, the three
  absent GREEN2 files, and the `raw/red-checks` counts.
- `messages-c03-review.txt`, `msgs_c03_review.py`: AST message comparison C01 to C02 to C03.
- `links_c03_review.py`, `linkcheck-c03-review.txt`: the link and anchor check and its negative control.
- `redaction-c03-review.txt`: one `check` per staging-list path, plus `audit-final.txt`, the roadmap and the positive
  control.
- `record-asserts-c03-review.txt`: the scratch copy of `record_asserts_c03.py --phase all` (18 PASS, 1 FAIL
  `ASSERT-C03-order`).

## Limits

- I ran the `cmp`-identical scratch copy of `record_asserts_c03.py` with `--repo` set to the checkout, not to the
  clone, because `ASSERT-C03-order` reads file times. The script has no write call. I compared the four evidence trees
  and `docs/` with the pre-run clone afterwards, and `git status` was unchanged.
- My three clause 9 mutants are a sample. I did not re-run the prober's 161 mutants. I checked their verdicts against
  their raw logs only.
- I did not read all 122 check bodies. I read clause 9 and the redaction and staging clauses of
  `CHK-C02-evidence-logs`, the GREEN2-only exemption, and the runner's verdict function.
- I did not re-derive `plan/DECISIONS.md` of C03.
- No product build or test ran, because this is a documentation and evidence round.
- On re-review after FIN, the remaining checks are: the order assertion passes, the three GREEN2 files exist,
  `redaction-final-c02.txt` covers `audit-final.txt` and `audit-verify-c02.txt`, and `redaction-c02.txt` covers
  `CLAUDE.md`.
