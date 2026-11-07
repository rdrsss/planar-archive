# V5-02-R15-A2-C02 review (checker-strength and records corrective for R15-A2-C01)

Decision: **request-changes**. The checker work is sound. C01-R1 and C01-R2 are closed: CHK-A2-016 now compares the
exact exempt-read list and rejects `init`, `preview` and extra members. CHK-A2-011 pins the uppercase `NONE` sentence
and rejects other none-like spellings. CHK-A2-022 sees the "then `X`" form and Part 2 Message-column cells, and its
remaining limit is stated. The first attempts of I21, I60 and I69 are killed. I63 has four meaning-changing mutants,
all killed by CHK-A2-092. Every pre-fix survivor is reported as a survivor. All 123 check names are unchanged, and the
seven contract documents are byte-identical to the C01 Identity hashes.

The round still cannot be committed. A redaction finisher ran at 00:42, after the record sync of 00:41, and rewrote
`green/staging-list.txt` and `green/redaction.txt`. The new list stages the unstaged full-hash inventory under `work/`
and three `red/superseded/` files, both of which plan rule C02-D23 excludes. It also drops the round's authoritative
`plan/DECISIONS.md` instead of redacting it, so the committed C02 record would link to a file that is not committed.
The records and README still describe the earlier 191-line list and the earlier redaction result. Two pointers send
readers to the 32-row rerun table as if it were the 80-row two-checker table. Fixing the staging list means changing
evidence files (`staging-list.txt`, `plan/DECISIONS.md`, `redaction.txt`, `hash-audit-final.txt`), so this is not a
records-only change.

Scope: the uncommitted candidate on HEAD `cdcfe829c466bf6338815ff6f5b6ea90ffddc570`. That is the seven contract
documents, the records [V5-02-R15-A2](V5-02-R15-A2.md), [V5-02-R15-A2-C01](V5-02-R15-A2-C01.md) and
[V5-02-R15-A2-C02](V5-02-R15-A2-C02.md), and the evidence folders `tests/native/v5-02-r15-a2-evidence/`,
`tests/native/v5-02-r15-a2-c01-evidence/` and `tests/native/v5-02-r15-a2-c02-evidence/`. I read the checker diff, the
plan, the probe tables, every C02 P3 probe log, the red attempt-3 logs, the green outputs, the staging list and the
records. I applied no mutant and edited nothing except this file. I ran no script in place under `tests/native/`.

## Candidate identity (workflow `identity.py`, run by me, verbatim)

Command, from the repository root: `python3 <identity.py> docs/vernacular/command-contract.md docs/vernacular/execution-contract.md docs/vernacular/execution-design.md docs/vernacular/external-profile-contract.md docs/vernacular/ledger-contract.md docs/vernacular/load-command-contract.md docs/vernacular/render-contract.md` (exit 0).

| identity | value |
|---|---|
| HEAD | `cdcfe829c466bf6338815ff6f5b6ea90ffddc570` |
| sha256(git diff --binary HEAD -- docs/vernacular/command-contract.md docs/vernacular/execution-contract.md docs/vernacular/execution-design.md docs/vernacular/external-profile-contract.md docs/vernacular/ledger-contract.md docs/vernacular/load-command-contract.md docs/vernacular/render-contract.md), tracked only | `312fabfc4520d7670aa085a70508672b0232a514b32d3d8ceaed426c47b7c08f` |
| combined sha256 (diff bytes, then each untracked path NUL bytes, sorted), pathspec docs/vernacular/command-contract.md docs/vernacular/execution-contract.md docs/vernacular/execution-design.md docs/vernacular/external-profile-contract.md docs/vernacular/ledger-contract.md docs/vernacular/load-command-contract.md docs/vernacular/render-contract.md, evidence excluded | `312fabfc4520d7670aa085a70508672b0232a514b32d3d8ceaed426c47b7c08f` |

| file | state | sha256 |
|---|---|---|
| `docs/vernacular/command-contract.md` | modified | `34ae9d4010f8e13a4183ce452d18395be9eb7f0f95d54eb3bc7aad34d48d4e8a` |
| `docs/vernacular/execution-contract.md` | modified | `38c0d3f70afe6412dc7b7ffe90270bb956562752493c690f0b740714f9176d75` |
| `docs/vernacular/execution-design.md` | modified | `1a5134a0815bb6a18bcb069b96eed94bd28ce247db88b5e4cf54c798c612cb87` |
| `docs/vernacular/external-profile-contract.md` | modified | `86fcbefbf070293b4da7f7efbab1dd7b6065b6d02abc0df7e4bcfa5199f0c493` |
| `docs/vernacular/ledger-contract.md` | modified | `46d8a0eef9f5a65822efbeea1a6e5b61dbd14f3b0adf3e0c01ade94112f4a84f` |
| `docs/vernacular/load-command-contract.md` | modified | `af99144c596184c3ec90a60f3f773fff8509401e14f76502f49b59684e70ee22` |
| `docs/vernacular/render-contract.md` | modified | `22a702eb57177f0c4a3d9b52ca3eb43cf9533a09d63c8586b3c3292d1e2ca874` |

All seven per-file values equal the C01 Identity table and the C01 review table byte for byte (acceptance 7). With
pathspec `src`, `identity.py` gives the empty-input value `e3b0c442...b855` for both the diff and the combined hash,
so `src` is unchanged.

## Closure of the C01 findings

| Finding | Verdict | Basis |
| --- | --- | --- |
| C01-R1, CHK-A2-016 exact list | closed | `prim_exempt_reads` (`contract_checks.py:737-753`, with `exempt_read_list` at `:724`) compares the P2-1 list, read from `Exempt reads (D108, D142):` up to ` skip step (4) only.`, with the exact expected string. It also requires the full D108 exempt and bound sentences and rejects `init`, `preview` or `activate` in either exempt list. Logs: I12-a1 (`preview` added), K01 (`init` added), K02 (`activate`), K03 and K04 (D108 `preview` and `init`) and K05 (`preview` removed from the bound list) each give `pass=123 fail=0` on the pre-fix checker and fail only CHK-A2-016 on the tightened checker. |
| C01-R1, CHK-A2-011 none-like spellings | closed | `prim_expect_job_spelling` with `_no_active_job_tokens` (`:756-777`) pins the sentence "The uppercase value `NONE` is the no-active-job expectation; any other value is a job ID." and the D104 sentence. It rejects any `--expect-job X` token other than `NONE`, `JOB` or `JOB_OR_NONE`, and any token before "is the no-active-job expectation" or "means "no active job"" other than `` `NONE` ``. I07-a1 (lowercase `none`), K10 (`None`), K11 (`null`) and K12 (D104 cell) survive the pre-fix checker and are killed by CHK-A2-011. In K13, CHK-A2-011 fails with "--expect-job value token none is not NONE, JOB or JOB_OR_NONE". |
| C01-R1, CHK-A2-022 extraction | closed, limit stated | `then_messages` and `table_message_cells` (`:534-549`) feed `message_strings`. K20 (a "then `X`" message alone) and K21 (a Message-column cell) are KILLED-OTHER on the pre-fix checker and fail CHK-A2-022 on the tightened checker. K22 (unmutated control) passes CHK-A2-022 and CHK-A2-160 on both checkers. The record section "CHK-A2-022 limit" and the README state what is still not seen: prose without backticks, lowercase-initial backtick messages, and Part 1 tables. |
| C01-R1, I21, I60, I69 | closed | CHK-A2-025 now pins the whole D152 retired-role sentence, CHK-A2-084 the RA-7 context sentence and the "database-free form only" sentence, and CHK-A2-098 the two ledger-pointer clauses. The replayed first attempts I21-a1, I60-a1 and I69-a1 are `pass=123 fail=0` on the pre-fix checker. On the tightened checker each fails only its named check. Their diff lines are identical to the C01 `raw/<id>.attempt-1.log` diffs (checked by `grep` and `cmp`), so they are true replays. K30, K31, K40, K50 and K51 kill the neighbouring clauses. |
| C01-R1, first attempts reported honestly | closed | `probes/mutants.prober-p3.tsv` and `PROBES.prober-p3.md` list I07-a1, I12-a1, I21-a1, I60-a1 and I69-a1 as SURVIVED and I18-a1 as KILLED-OTHER (CHK-A2-017 only) on the pre-fix checker. I63-a1 is EMPTY, with the empty C01 diff quoted. The C01 `PROBES.md` and the C01 record carry dated correction blocks that quote and withdraw the "mutant-fidelity defects" sentence and the "156 KILLED, 1 SURVIVED" headline. No first attempt is relabelled. |
| C01-R1, attempt 3 and names | closed | `red/red-run.attempt-3.txt` is `pass=8 fail=115` (exit 1) and `red/stub-run.attempt-3.txt` is `pass=10 fail=113` (exit 1), both with 0 Traceback lines. Attempts 1 and 2 are present. The unsuffixed files are `cmp`-equal to the `.attempt-2.` copies. The 123 `def CHK_A2_*` names of the tightened checker equal those of `probes/checker-attempt-2/contract_checks.py` (`diff` empty). |
| C01-R2, I63 | closed | I63-m1 (`read_execution({})` changed to `read_ledger({})`), I63-m2 (selector `event` changed to `request`), I63-m3 (the five-families clause deleted) and I63-m4 (limit 50 changed to 1) are meaning-changing. Their exact texts are in plan `INVARIANTS.md` section C. Each log shows a non-empty diff of `command-contract.md:57` and a CHK-A2-092 FAIL on both checkers; I63-m4 also fails CHK-A2-131. The C01 reword is described as a reword, not as a kill. |
| C01-R3, staging list and redaction | **not closed** | See C02-R1. |
| C01-R4, records | mostly closed; new stale claims | The C01-R4 items (pending, "none exists", `baseline.log`, `FIN.md`, `redact-exits`, wrong folder) are gone: my grep over the three records and the README finds them only in the dispatch text and the plan-row quotations. Every backticked `tests/native/...-evidence/` path in those four files exists, except `green/redaction-final-coordinator.txt`, which the record itself says does not exist. New stale claims appeared after the sync; see C02-R2. |

## Findings

### V5-02-R15-A2-C02-R1 (blocking, evidence): the final staging list breaks the plan's staging rule and drops the round's own decisions file

`green/staging-list.txt` and `green/redaction.txt` are dated 00:42:11. That is after the record sync edits (00:41) and
after P6's hash audit (00:32:22) and final link check (00:31:02). The 00:42 run replaced P6's list. The bytes on disk
now differ from both the plan and the record:

- **`work/` is staged.** Lines 89 and 90 are `tests/native/v5-02-r15-a2-c02-evidence/work/before-files.full.txt` and
  `work/evidence-diff.full.txt`. C02-D23 excludes "the whole `c02-evidence/work/` folder (full hash inventory, unstaged
  by C02-D28)", and A09 and A26 require `grep -c '/work/' green/staging-list.txt` = 0. `before-files.full.txt` has
  1217 lines with a 64-hex field. They include sha256 values of `README.md`, of the C01 `probes/PROBES.md` and of the
  plan `DECISIONS.md` files and their `superseded-*` copies. `evidence-diff.full.txt` adds 1485 more. Staging these
  files pins hashes of READMEs, plan files and probe reports, which acceptance 6 and C02-D15 forbid. The
  `hash-audit-final.txt` run at 00:32 did not cover them, because they were not on the list then.
- **`red/superseded/` is staged.** `RED-ctest-table.md`, `RED-ctest.md` and `tests-ctest.txt` are on the list.
  C02-D23 excludes `red/superseded/**`, and the A09 observer requires 0 `superseded` matches. They pass redaction, so
  acceptance 5 alone would allow them. But the plan rule says otherwise, and no decision records the change.
- **The C02 decisions file is dropped instead of redacted.** The trailer of `redaction.txt` reads `pass=197 fail=2`,
  then "removed from staging after failing check (evidence files): `tests/native/v5-02-r15-a2-c02-evidence/plan/DECISIONS.md`,
  `.../red/superseded/RED-pre-redcheck3.md`". `plan/DECISIONS.md` is the authoritative decisions table of this round.
  C02-D23 includes "the plan files ... of the evidence folders", and the C02 record links it four times, for example
  `[plan/DECISIONS.md](../../../tests/native/v5-02-r15-a2-c02-evidence/plan/DECISIONS.md)` in the Invariant table
  lead. Committed as listed, the record would point to a file that is not in the commit. The failure is one host path
  on line 96 (the absolute path of the workflow `identity.py`). Open item 7 of the record already says it is to be
  redacted, not excluded.
- **No markers.** The 197 lines carry no `<TAB>marker` field (`awk -F'\t' '{print $2}'` is empty for every line).
  C02-D23 and A09 require TRACKED-MODIFIED, NEW, FORCE-ADD or CHECKED-BY-COORDINATOR on every line, and the commit
  step relies on them to choose `git add -f`.
- **P6's outputs were overwritten.** P6's 191-line list and its 190-file `redaction.txt`, which the records describe,
  no longer exist. No `.attempt-<n>` copy was kept.

What is right: every one of the 197 listed paths exists, and my per-file diagnostic run of
`scripts/redact-local-paths.py --repo-path <repo> --home <home> check` exits 0 on all 197. `PROBES.prober-p3.md` and
`mutants.prober-p3.tsv` are now listed, which closes the record's Open item 9. Both review files and all three
records are listed. No `raw/` path is listed.

Fix: redact the host path on line 96 of the C02 `plan/DECISIONS.md` (the plan file is evidence, not a record) and put
it back on the list. Rebuild `green/staging-list.txt` under C02-D23, with markers and without `work/`. For
`red/superseded/`, either remove it or record a dated decision that changes C02-D23. Keep the overwritten P6
list and redaction result as `.attempt-<n>` copies if they can be recovered, or say plainly that they were overwritten.
Then re-run the per-file redaction, the hash audit and the final link check over the final list, as the last steps.

### V5-02-R15-A2-C02-R2 (blocking, records): the records describe files that have since changed

The record sync ran at 00:41, before the 00:42 staging run. The following claims are now false:

- C02 record: Files edited (the `staging-list.txt` row, "191 lines (177 FORCE-ADD, 8 TRACKED-MODIFIED, 5 NEW, 1
  CHECKED-BY-COORDINATOR); it does not list `PROBES.prober-p3.md` ..."), rows A09 and A10 of the Invariant table,
  "Final-state finisher (P6)", Acceptance row 5, Open items 7 and 9, and "Redaction and staging" ("191 lines", "189 of
  190 exit 0, plan/DECISIONS.md exit 1"). The file on disk has 197 lines and no markers, lists the P3 tables, and its
  redaction log shows 199 checks with two files removed.
- R15-A2 record line 484 ("189 of 190 files exit 0") and README line 29 ("The C02 staging list (191 lines) ...
  189 of 190 exit 0").
- R15-A2 record line 512 says "C02 (`tests/native/v5-02-r15-a2-c02-evidence/probes/PROBES.md`, `mutants.tsv`): 80
  rows", and the C01 `probes/PROBES.md` line 27 says that the C02 table "with the true verdicts of every first attempt
  on both checkers" is `.../c02-evidence/probes/PROBES.md` (with `mutants.tsv`). Since the 00:35 rerun, those two files
  hold the 32-row attempt-3-only table, whose headline is "There are no survivors". The table that has both checkers
  and the survivors is `PROBES.prober-p3.md` with `mutants.prober-p3.tsv`. A reader who follows these pointers sees
  no survivor. The README, the C02 record and the R15-A2 Invariant-table cells already point to the `prober-p3` files
  correctly.
- README layout row 18 and the C02 Owned-files amendment call `work/` UNSTAGED, which the staging list contradicts
  (this resolves once C02-R1 is fixed).

Fix: after C02-R1, sync the four files from the final outputs. State the final list size, the markers and the
redaction result. Re-point the two pointers to `PROBES.prober-p3.md` and `mutants.prober-p3.tsv`. Leave the
coordinator's final-bytes step (C02-D29) as the last check over every file edited after the redaction run.

## Advisories (not blocking)

- **A1, plan table syntax.** Plan `INVARIANTS.md` row A09 writes `` `grep -cE 'raw/|superseded|/work/'` `` with
  unescaped pipes inside a table cell. This splits the row, and its expectRed column renders as `superseded`. The
  record copy escapes the pipes correctly. All other 59 rows equal the record copy in Invariant, Mutant, Test and
  expectRed (compared by script).
- **A2, red attempt 3 does not reach the new branches.** On base and stub, the tightened checks fail on a missing
  heading or an earlier literal (for example "heading ### P2-1 Verbs and services occurs 0 times"). The record says
  so, and the behavioural red-first evidence is the pre-fix checker column of the probes (16 SURVIVED, 6
  KILLED-OTHER, logs `pass=123 fail=0` or another check only). I accept this. The pre-fix checker is pinned by the
  `shasum` pasted in the record (`6283fdd1...`), which equals `work/before-files.full.txt` and the copy on disk.
- **A3, the 18 hash-audit findings.** I accept them. The five in `V5-02-R15-A2-review.md` are `identity.py` blocks of
  an earlier contract state. The other 13 are snapshot hashes of `base/` copies under the C01-D14 exception. None
  pins a record, review, REPORT or the roadmap.
- **A4, Owned-files amendment timing (Open item 6).** It was entered at the sync rather than before P1. The files
  actually edited match the amendment and the Files edited table, so I do not hold this against the round.
- **A5, minor code note.** In `prim_exempt_reads`, the `d108_bound` slice ends at `cell.find('`init` is classified')`.
  If that sentence disappears, `find` returns -1 and the slice silently drops the last character. The preceding
  exact-sentence check of `BOUND_D108` already covers the bound list, so this has no effect today.
- **A6, deviations.** K13 (CHK-A2-019 does not fail), I100 (CHK-A2-160 no longer fails with CHK-A2-133) and the
  NOT RUN replays I129 and I132 are reported as deviations, not relabelled. I accept them.

## What I verified

- **Identity.** My `identity.py` output (above) equals the C01 Identity table, `green/identity.txt` and the record's
  pasted table. With pathspec `src`, both hashes are the empty value.
- **Checker diff.** `diff -u` of `probes/checker-attempt-2/contract_checks.py` against the checkout checker shows only
  the new helpers, the extra calls in CHK-A2-011 and 016, the new literals in 025, 084 and 098, the widened
  `message_strings` and four self-test families. The fixtures differ only by the eight new files. No existing
  diagnostic string changed, and no check was added, removed or renamed.
- **Green (my diagnostic, not round evidence).** A `diff -r`-identical scratch copy of `checks/`, run as
  `python3 -I -B <scratch>/checks/contract_checks.py --repo <repo> --base <repo>/tests/native/v5-02-r15-a2-evidence/base
  --git-repo <repo> --base-commit cdcfe829c466bf6338815ff6f5b6ea90ffddc570` from the repository root, exits 0 with 123
  PASS lines and `SUMMARY pass=123 fail=0`. The PASS/FAIL name set equals `red-run.attempt-2.txt` (`comm -3` empty).
  `--selftest` exits 0 with `SUMMARY pass=1 fail=0`. A first run where I passed the short SHA failed CHK-A2-133 with
  "--base-commit must be a full commit SHA". That was my mistake, and the check behaved correctly.
- **Probe logs.** I summarised all 82 P3 logs (`*.attempt2.log`, `*.attempt3.log`). Exit, `SUMMARY` and FAIL names
  agree with every row of `mutants.prober-p3.tsv`. Every applied mutant has one `RESTORE ... match=True` line, and
  both baselines are `pass=123 fail=0`. The tally by `awk` matches `green/probes-tally.txt`: pre-fix KILLED 6,
  KILLED-OTHER 6, SURVIVED 16, EMPTY 1, PASS 1; tightened KILLED 46, NOT RUN 2, EMPTY 1, PASS 1. The 00:35 rerun
  `mutants.tsv` has 32 rows, all KILLED, and agrees with the P3 attempt-3 rows.
- **Invariant table.** All 60 plan ids appear in the record (`comm` empty). Each row has a mutant that breaks the
  stated invariant (checker-strength rows), or a stated reversal plus an observable file (workflow rows). Each
  acceptance criterion and each of C01-R1 to C01-R4 has a row. A09, A10, A15 and A25 are honestly marked not met by
  the record. A09 and A10 are the subject of C02-R1.
- **Records.** The stale-statement grep and the existence loop are described under C01-R4 above. The C01 correction
  blocks quote the withdrawn sentences and restate the counts.
- **Staging and redaction.** These are described under C02-R1. My diagnostic redaction run covers the 197 listed
  files (all exit 0) and the C02 `plan/DECISIONS.md` (exit 1, a host path on line 96).

## Limits

- I applied no mutant. The verdicts above rest on the saved probe logs and the checker source.
- My checker and redaction runs are diagnostics, not round evidence. Their outputs are in my scratchpad only. No
  files were added under `tests/native/v5-02-r15-a2-c02-review-evidence/`, and every command is stated inline.
- The evidence folders are gitignored, so `identity.py` gives no hash for the checker. I compared the copies with
  `diff -r` and `shasum` only.
- I did not judge whether `red/superseded/` should be committed. I report only that the final list contradicts the
  plan rule without a recorded decision.
