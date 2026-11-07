# V5-02-R15-A1-C02 independent review (execution contract, part 1, second corrective)

Disposition: **request-changes, not records only**. The one contract edit is right. Line 520 of
`execution-contract.md` now carries the exact OP2 tag, and it is the only line that differs from the C01 candidate.
Record section (c) is reworded, the 15 versus 13 FAIL count is explained, and the C01 `red/RED.md` now describes the
real behavioural red. No `src` or `tests` byte changed, no check name was removed or renamed, and no raw evidence log
was deleted.

C02 is not finished, though, and its two blocking findings are still open. The mutation-prober waves (PROBE, PROBE2)
and the final green waves (GREEN, GREENB, GREEN2) never ran. The round's own coordinator sync says so ("before the PROBE
and GREEN waves"). `probes/PROBES.md`, `probes/mutants.tsv`, `probes/raw/`, `green/checks.txt`,
`green/sql-final.txt`, `green/audit-final.txt`, `green/linkcheck-c02.txt` and `green/redaction-c02.txt` do not exist.
Despite that, the status line, three "Superseded in C02" notes, the closure table and the new post-C01 status table all
say or imply that the prober has run and that the final logs exist. That repeats the C01-R4 defect. Fixing it properly
needs the missing evidence data, so a records-only fix cannot close this round.

Scope: the uncommitted R15-A1 plus C01 plus C02 candidate on HEAD `d536922b`. That covers the seven candidate
documents, the round record [V5-02-R15-A1](V5-02-R15-A1.md) (all of it, with weight on the C01 review outcome, the C02
dispatch, 'Checker amendments (C02)', 'Owned files (C02)', 'Redaction (C02)', 'C02 acceptance status', 'Staging list
(C02)', the C02 coordinator sync and the C02 invariant table), the [C01 review](V5-02-R15-A1-C01-review.md), the C01
evidence folder `tests/native/v5-02-r15-a1-c01-evidence/` and the C02 plan folder
`tests/native/v5-02-r15-a1-c02-evidence/`. I ran no script from `tests/native/`. I ran the checker only as a
`cmp`-identical scratch copy against a scratch clone outside the checkout.

## Exact candidate

Workflow `identity.py`, run from the repository root on 2026-11-06 and pasted verbatim (also in
`tests/native/v5-02-r15-a1-c02-review-evidence/identity-c02-review.txt`). Seven candidate documents:

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

These values match the record's block 'Current identity after the C02 line-520 edit' exactly. `readlink AGENTS.md`
prints `CLAUDE.md`. The `src tests` identity is the empty input.

## Closure of C01-R1 to C01-R8

| Finding | Verdict | What I read or ran |
| --- | --- | --- |
| C01-R1 prober | **open** | `probes/PROBES.md`, `probes/mutants.tsv` and `probes/raw/` do not exist. The 'D81 resolution' section is unchanged from C01 ("not RUN, open", "This record does not choose"). Every probe cell of the C02 invariant table reads "not run". See C02-R1. |
| C01-R2 final logs | **open** | `green/checks.txt`, `green/sql-final.txt` and `green/audit-final.txt` do not exist. The only `sql-final.txt` copies are in `green/superseded-1/` and `green/superseded-2/`, and both carry the C01 contract hash. See C02-R2. |
| C01-R3 RED.md | closed (one wording point, C02-R6) | The C01 `red/RED.md` quotes the command and the pre-fix identity lines verbatim from `red/red-run.txt`. The execution-contract row in those lines equals the sha256 of `raw/pre-c01-snapshot/docs/vernacular/execution-contract.md`. Its 29 failing check names equal the FAIL set of `red/red-run.txt` (70 PASS, 29 FAIL, exit 1), compared with `diff`. It labels the ctest logs an irrelevant first attempt. The record's 'Evidence by pointer' block and the README `red/` row say the same. |
| C01-R4 status line | **not closed** | Line 3 now says the C01 review findings "are closed per the C01 and C02 sections". C01-R1 and C01-R2 are not closed. See C02-R3. |
| C01-R5 post-C01 status | partly closed | Section (l2) exists. It has rows 1 to 10 and states the HALT-1 amendment in row 3. But rows 1, 2, 7, 8 and 9 rest on files that do not exist and on a 'C02 Final results' section that does not exist. See C02-R3. |
| C01-R6 redaction and staging | partly closed | 'Redaction (C02)' cites the reviewer's control `review-c01/redaction-c01-review.txt`. The staging list leaves out `inventory.txt`, every `plan/superseded-*/INVARIANTS.md` and every `raw/`, `red/raw/` and `probes/raw/` log. The round's own positive control `green/redaction-c02.txt` does not exist. 7 of the 33 listed paths do not exist. My run is below. |
| C01-R7 OP2 tag | closed | `execution-contract.md:520` reads "Decision OP2, tagged `operator-decided (delegated 2026-10-31, option A)`, applies." Section (c) (record line 209) now says each choice "is recorded as delegated, not as live answers; the operator may overrule them later" and no longer says "does not record an operator decision". |
| C01-R8 FAIL count | closed | 'Repairs and amendments in this lineage' explains the 15 against 13. I counted 15 FAIL lines in `raw/orig-checker-run.txt`: the 13 of 2026-11-01 plus `CHK-I45-diff-scope` and `CHK-I59-record-additions`. C33 and the R15-A1 review keep the historical 13. |

## Findings

### V5-02-R15-A1-C02-R1 (blocking, evidence): the D81 prober still has not run

Dispatch work item 1 and acceptance criterion 1 require one mutant, one `probes/raw/<id>.log` and one verdict for each
guard row: C5, C24 to C28, C34 to C38, C44 to C47, C49 to C56, C58 to C60, C59b and G1 to G8. Survivors must be
fixed in the check and probed again, and `PROBES.md` must carry the killed and survived counts. None of this exists. The
D81 resolution section still holds the C01 text, which says no prober ran and makes no choice. The C02 invariant table
has "not run" in the probe column of all 66 rows. So the brief's requirement that "each listed test is killed by it"
is not shown for any row. The scratch folder of the C02 waves holds partial `probe/` and `probe2/` folders, but nothing
reached the evidence tree.

To scope how much work remains, I applied 14 mutants in a scratch clone (HEAD plus the working-tree documents and the
three evidence trees, outside the checkout) and ran the scratch copy of `green/ex_checks-c02.py` on each. Before
mutating, the clone's baseline was 113 PASS and 0 FAIL in default mode. Every mutant was killed. In each case the exit
was non-zero and a FAIL line named the row's check:

- C5-m1 (`CHECK(ordinal>0)` to `>=0` in `transitions`) and a permissive C5 variant (the CHECK dropped): both killed by
  `CHK-C01-R2-ordinal-check` ("transitions accepts ordinal 0").
- C25-m1 (`running` requires `run_id IS NULL`): killed by `CHK-C01-sql-legal-rows`.
- C28-m1 ("may be complete" to "may be completed"): killed by `CHK-C01-message-bytes`.
- C46-m1 (drop `records_digest`): killed by `CHK-I47-sql-executes`.
- C55-m2 (M8 reason to `prerequisite_available`): killed by both `CHK-I60-role-rows` and `CHK-I69-stale-done`.
- A permissive extra `stale` to `ready` row: killed by both of those checks.
- G7-m1 (`fenced` exit 3 to 4): killed by `CHK-I22-code-table`.
- N1-m1 (old OP2 wording restored): killed by both `CHK-C02-op2-history-tag` and `CHK-C02-contract-one-line`.
- C27-m1 (sentence appended to `ledger-contract.md`): killed by `CHK-C01-frozen-files`.
- G2-m1 (untracked file under `src/`): killed by `CHK-I44-identity-src-tests`.
- G8-m2 (one byte appended to the C01 review): killed by `CHK-I45-diff-scope`.
- I46-m1 (the roadmap's sha256 pasted into the C01 README): killed by `CHK-I46-record-no-hash`.
- N3-m1 (the old D81 sentence restored on line 3): killed by `CHK-C02-record-status`.

These runs are my evidence, not the round's (`probe-sample-review.txt`). They suggest the prober will mostly report
kills. One thing the prober should record: a mutant that adds a row to the M table, such as planned C55-m1, is
killed in `CHK-I60-role-rows` by the `M table row count != 41` pin before the new-generation rule runs. Also,
`stale_ready_rows` names rows by position, so the row I inserted was reported as `M41`. C55-m1 therefore does not
show that I60's stale-to-ready rule can fail. C55-m2 and C55-m3 do, and the prober's log should name the assertion that
fired.

Fix: run waves PROBE and PROBE2 as planned. Write `mutants.tsv`, one raw log per mutant and `PROBES.md` with the
`main section totals` line. Then replace the D81 resolution with the counts.

### V5-02-R15-A1-C02-R2 (blocking, evidence): no final-candidate log exists

Acceptance criterion 2 and gate bullets G1 to G4 require these logs on the final candidate: `green/checks.txt` (zero
FAIL, count recorded), `green/sql-final.txt` (foreign keys on, 25 of 25), a resolved audit run (`green/audit-final.txt`),
`green/linkcheck-c02.txt` and `green/redaction-c02.txt`. None of them exists. The two `sql-final.txt` files in
`green/superseded-1/` and `green/superseded-2/` print the C01 contract hash for `execution-contract.md` in their
identity blocks, so they do not describe this candidate. They are correctly set aside. The record states no explanation
for the audit run that exits 1. In my clone the audit mode exits 1 with seven FAIL lines, all caused by these missing
files: `CHK-C01-sql-evidence-recorded`, `-green-evidence`, `-prober-evidence`, `-record-closure`, `-record-d81`,
`CHK-C02-evidence-logs` and `-logs-masked`.

Fix: run GREEN, GREENB and GREEN2 after the prober. Write the logs with their commands and identity blocks, bring the
audit run to zero FAIL, and finish the staging list.

### V5-02-R15-A1-C02-R3 (blocking, records): the record claims work that has not been done

The record contradicts itself in the same way C01-R4 found, now in more places:

- Line 3 says the C01 review findings R1 to R8 "are closed per the C01 and C02 sections". The C02 coordinator sync
  (lines 1149 to 1206) and the C02 acceptance tables mark every criterion "not met".
- Lines 329, 756 and 811 each add "Superseded in C02: the prober has since run; see probes/PROBES.md and Final results
  (C02)". The prober has not run, `probes/PROBES.md` does not exist, and no 'Final results (C02)' section exists.
- The closure-table R6 row (line 610) cites `green/checks.txt` and `probes/PROBES.md` "(C02; see Final results (C02))".
  Neither file exists.
- Section (l2) marks R15-A1 criteria 1, 2, 7, 8 and 9 "met". The cited bases are `green/checks.txt`,
  `green/sql-final.txt`, `green/linkcheck-c02.txt` and `green/redaction-c02.txt`, which do not exist, and "C02 Final
  results", which is not written. (`green/identity-c02.txt` exists. Criterion 9 is true on my own identity run.)

The core checker passes all of these lines (113 of 113 PASS in my clone), because its record checks match text, not
the existence of what the text names. The audit set catches part of this. A reader of the record would conclude that
the prober ran and that the final logs exist.

Fix: once C02-R1 and C02-R2 produce the evidence, write 'Final results (C02)' and the D81 resolution from it, then
check every sentence above against the files. If any wave is dropped, reword those sentences to the true state.

### V5-02-R15-A1-C02-R4 (medium, process): the C02 red wave repeated the ctest attempt and ran the checker from `tests/native/`

`tests/native/v5-02-r15-a1-c02-evidence/red/RED.md` records `cmake --build build/rewrite-debug --target planar_tests`
(`raw/build.log`, "ninja: no work to do") and one `ctest -R '^name$' --no-tests=error` run per check. That is 58
`*.ctest.log` files, each "No tests were found", and ctest writes `build/rewrite-debug/Testing/Temporary/LastTest.log`.
The same file says the checks "were run with: python3 -I tests/native/v5-02-r15-a1-c01-evidence/green/ex_checks-c02.py".
This breaks three rules: the dispatch ("No product build, no test run"), the binding rule ("never run any ... script
under `tests/native/`"), and the plan's own rule that the checker runs only from the scratch copy (INVARIANTS.md header,
D5 and D36). C01-R3 found exactly this attempt in C01. The real C02 red, `red/red-c02-run.txt` in the C01 folder, used
the scratch copy correctly. Its FAIL lines also contain unmasked host paths. This file is not on the staging list, but
it is cited from the record's invariant table.

Fix: label the C02 plan-tree `red/RED.md` an irrelevant first attempt, as the C01 one now is. Note it in the C02
section of the record and keep the logs.

### V5-02-R15-A1-C02-R5 (medium, records): three red-check cells come from that misdirected run

The C02 invariant table takes its red-check column from the plan-tree `red/RED.md`. For PR-C27
(`CHK-C01-frozen-files`), PR-G8 (`CHK-I45-diff-scope`) and N8 (`CHK-C02-raw-preserved`), all with expectRed false, the
column says "red (FAIL on an assertion)". In that run those checks failed with `file missing:
.../green/frozen/roadmap.md` and `.../green/frozen/reports/...REPORT.md`, because no `--frozen-dir` was given. That is
an environment failure, not a behavioural red. In the real red log `red/red-c02-run.txt`, which was run with
`--frozen-dir`, all three PASS. So the column contradicts both expectRed and the real red evidence.

Fix: take the column from `red/red-c02-run.txt` and `red/red-c02-audit-run.txt`, which give "passed" for these three
rows.

### V5-02-R15-A1-C02-R6 (advisory): `raw/red-checks` is described as one log per failing check

The C01 `red/RED.md` ("holds one log per failing check of `red/red-run.txt`") and the README `red/` row ("one log per
failing check") are wrong on this point. `raw/red-checks/` holds 72 logs, one for each C01 invariant row (C1 to C61,
C59b and G1 to G8), PASS and FAIL alike. For example, `C24-CHK-C01-sql-counts.log` is a PASS with exit 0. Say "one log
per C01 invariant row (29 FAIL, the rest PASS)".

### V5-02-R15-A1-C02-R7 (advisory): "no message changed" for the checker amendments is not exact

'Checker amendments (C02)' says that no check name was removed or renamed "and no message changed". The names hold: the
105 ids of `red/ex_checks.py` are the first 105 of the C02 registry in order, the 70 original ids are unchanged, and 17
`CHK-C02-*` ids are appended. But 17 assertion messages of the amended bodies are no longer in the C02 checker. For
example, "the D81 sentence lacks killed and survived counts" became "the D81 resolution lacks killed and survived
counts", and "the two-index count is not recorded" is gone. These are checker messages, not product or contract
diagnostics, so no rule is broken. The C01 record worded the same situation accurately ("Checker-side diagnostic text
changed for I32 and I59 only"). Use similar wording here.

## Checks that passed

- **One contract line.** `diff raw/pre-c02-contract.md docs/vernacular/execution-contract.md` shows exactly one hunk,
  `520c520`, and both files have 1089 lines. The snapshot's sha256 equals the C01 value that the record prints
  (`28635996...`), so the snapshot is the C01 candidate. No message line and no check name occurs in the hunk.
  `CLAUDE.md`, `acceptance-matrix.md`, `external-profile-contract.md`, `implementation.md` and `ledger-contract.md`
  compare identical to the pre-C01 snapshot. `v5-alignment.md` keeps its C01 hash.
- **Source untouched.** `identity.py` over `src tests` gives the empty-input hash and no file rows (block above).
- **No raw log deleted or edited.** All 320 paths of `raw/pre-c02-manifest.txt` exist with their recorded size. The
  only two that differ are the allowlisted rewrites, `red/RED.md` and `README.md`. Files that C02 moved into
  `green/superseded-*` were C02's own early outputs and are not in the manifest.
- **Red-first evidence of C02.** `red/red-c02-run.txt` holds the scratch-copy command, the verbatim identity of the
  pre-fix tree (execution-contract row `28635996...`), exit 1, 101 PASS and FAIL lines for exactly the 12 expected ids
  (`CHK-C02-red-logs` has two lines), and the mask line. The first attempt is kept in `green/superseded-2/`, and the
  rerun's header says why it was rerun.
- **Links and anchors.** My own checker (`links_review.py`) covered the seven candidate documents, the round record,
  the C01 README and `red/RED.md`, and the C02 plan `INVARIANTS.md` and `DECISIONS.md`: 12 files, 422 links, 0 broken.
  The negative control (one missing anchor, one missing file) exits 1 with 2 broken.
- **Redaction.** I ran one `check` per path of the record's staging list, plus the roadmap. All 26 listed files that
  exist exit 0, and so does the roadmap. Seven listed paths are absent (C02-R2). The positive control, a file holding
  the repository root, exits 1. The staging list leaves out `tests/native/v5-02-r15-a1-evidence/inventory.txt`, every
  `plan/superseded-*/INVARIANTS.md` and every raw log, as C01-R6 asked.
- **Record rule.** I computed the sha256 of every round record, the roadmap, every evidence REPORT and
  `raw/pre-c02-record.md` in memory only and compared them with every 64-hex token in the R15-A1 lineage. That covers
  the record, its three reviews and the three evidence trees. There were no hits. Hashes in the record and evidence are
  `identity.py` outputs for candidate documents, the empty `src tests` input, the salted manifest column and the
  `SHA256SUMS` of evidence data files.
- **Invariant table against the plan.** All 58 rows of `plan/INVARIANTS.md` revision 4 (35 guard, 6 audit, 17 new) are
  in the record's C02 table, with the same mutant, test and expectRed values. The record adds eight finding rows. The
  only text differences are two quotations in N6 and N7, which the record shortens so that the checks do not fire on
  the record itself. Every dispatch criterion and every C01 finding has a row. The hollow part is the probe column
  (C02-R1).

## Evidence

New files under `tests/native/v5-02-r15-a1-c02-review-evidence/`, written by this review only (gitignored). The scripts
are copies of the scratchpad files I ran, and I ran them only from the scratchpad.

- `identity-c02-review.txt`: workflow `identity.py` for both pathspecs, and `readlink AGENTS.md`.
- `links_review.py`, `linkcheck-c02-review.txt`: link and anchor check and the negative control.
- `redaction-c02-review.txt`: one `check` per staging-list path, absences marked, the roadmap and the positive control.
- `probe_sample_review.py`, `probe-sample-review.txt`: the clone baseline (default and audit) and the 14 sample
  mutants with their verdicts and FAIL lines.
- `record-rule-c02-review.txt`: the record-rule search summary. It writes no hash value.

## Limits

- My 14 mutants are a sample. They do not replace the prober's run over every guard row, its control runs or its
  closing rows.
- I ran the checker's audit mode only to see what it reports on this candidate, because the round's audit log does not
  exist. I did not read all 122 check bodies. I read I60, part of the C01 to C02 diff, and the bodies the sample
  mutants hit.
- Several expectRed-true FAIL lines in `red/red-c02-run.txt` report that a record subsection is absent ("the record has
  no ... subsection"). In a records corrective, that absence is the behaviour under test, and plan D34 allows it. I
  accepted these lines on that basis.
- I did not re-derive `plan/DECISIONS.md` beyond D5, D12, D34 and D36.
- No product build or test ran, because this is a documentation round.
