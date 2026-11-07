# V5-02-R15-A1-C01 evidence

Evidence for the corrective round V5-02-R15-A1-C01, which closes findings R1 to R12 of the R15-A1 review on the uncommitted
R15-A1 candidate (HEAD `d536922b`). The round is documentation only: no product build, no test binary, no Docker gate.

This folder is gitignored (`tests/native/*-evidence/`). Files are staged only by explicit path from the record's "Staging list (C02)" with `git add -f`. The round record,
`docs/vernacular/rounds/V5-02-R15-A1.md`, cites these files by pointer, and its section "V5-02-R15-A1-C01 outcome and
closure table" is the summary.

## Path table

| Path | Contents |
| --- | --- |
| `plan/` | `DECISIONS.md` (decisions C.1 to C.26, D97 to D104 and the amended and relabelled rows), `INVARIANTS.md` (checks C1 to C61 and guards G1 to G8), and `superseded-1/` to `superseded-3/` (earlier plan revisions) |
| `red/` | `RED.md` (rewritten in C02), `ex_checks.py` (evidence copy of the repaired checker), the real red: `red-run.txt` (default-mode run on the pre-C01 snapshot), `red-audit-run.txt` (audit-mode run on the same snapshot) and `raw/red-checks` (72 files: 71 logs, one log per C01 invariant row with two for C59b, and SUMMARY.txt; 34 of the logs record a FAIL of their own check, the rest a PASS; under `raw/`); the C02 red on the pre-fix tree: `red-c02-run.txt` and `red-c02-audit-run.txt`; `red/raw/*.log` are the ctest logs of an irrelevant first attempt (kept, not red evidence) |
| `green/` | `checks.txt` (core run on the final candidate), `sql-final.txt` and `sql_final.sh` (SQL results and script), `ex_checks-c02.py` (the C02 checker), `mask_log.py`, `*-c02.txt` (identity, redaction, link, scope, frozen and cmp outputs), `audit-final.txt`, `redaction-final-c04.txt` and `record-asserts-final-c04.txt` (written by the FIN phase of C04; `redaction-final-c02.txt` and `audit-verify-c02.txt` were never written), and the earlier-stage logs `checks-final.txt`, `checks-closing.txt`, `sql-c01.txt` and the `*-final`, `*-closing` outputs |
| `probes/` | `PROBES.md` (verdicts and survivors), `mutants.tsv` (one row per mutant id), `raw/<id>.log` (one raw log per mutant) and the three baselines `baseline.txt`, `baseline-closing.txt` and `baseline-final.txt` |
| `raw/` | the original checker copy and its run (`orig-checker-run.txt`), the baseline identity, the pre-C01 snapshot and its comparison, `raw/pre-c02-*` (the C01 candidate contract, record and manifest as they stood before C02) and other raw logs |

Raw logs are never deleted. A later stage replaces only its own placeholder logs in `probes/raw/`.

## Wave order

1. T1: repaired checker, red runs on the pre-C01 candidate, frozen copies.
2. PA (contract text) and PB (`v5-alignment.md`), in parallel.
3. R1: this record's relabel and C01 section, the two old plan files, and this README.
4. V1: green core run (`green/checks.txt`), SQL results, link, redaction, identity, scope and frozen-file checks.
5. PROBER: mutation probes in a scratch clone outside the checkout.
6. R2: D81 resolution text, the `probes/PROBES.md` cite, and REPORT.md.
7. PROBER2: closing probes for C31-m1, C31-m2, C61-m1 and C61-m2.
8. FINAL: audit run (`green/checks-final.txt`) and the final-stage outputs.
9. SYNC: final results in the record and REPORT.md.
10. PROBER3: closing probes for C32-m3, C32-m4 and C61-m3.
11. CLOSE: last audit run on the final record (`green/checks-closing.txt`).

The green core run follows R1 because the relabel checks pass only on the relabelled record and old plan files.

## C02 wave order

Superseded by 'State of this folder (C03 coordinator sync, 2026-11-06)' at the end of this file: the paragraphs below state the plan and the folder at the C02 sync, and the files they call absent now exist.

The corrective V5-02-R15-A1-C02 is planned in these waves: CHK, DOC+REC1, PROBE, REC2, GREEN, REC3, GREENB, PROBE2, CLOSE, GREEN2. State at the coordinator sync of 2026-11-06: CHK and DOC+REC1 ran; the later waves have not run, so `probes/PROBES.md`, `probes/mutants.tsv`, `probes/raw/`, `green/checks.txt`, `green/sql-final.txt` and `green/audit-final.txt` do not exist yet and the rows of the path table above that name them describe files still to be written. The plan of C02 is in `tests/native/v5-02-r15-a1-c02-evidence/plan/`.

GREEN2, the last wave, writes `green/audit-final.txt`, `green/audit-verify-c02.txt` and `green/redaction-final-c02.txt`; the record cites them by path only. The closing baselines are `probes/baseline-closing.txt` and `probes/baseline-final.txt`; the audit counts quoted from them are the C01 figures. Files that a later wave has not yet written (`green/checks.txt`, `green/sql-final.txt`, `probes/PROBES.md`, `probes/mutants.tsv`, `probes/raw/`, `green/linkcheck-c02.txt`, `green/redaction-c02.txt`) are absent from this folder until that wave runs.

## Conventions

- Commands in these files use the placeholders `<scratchpad>` (the session scratch folder, which holds the canonical
  checker and the SQL and link scripts) and `<repo root>`. No host path appears here or in any evidence file.
- Scripts run only from `<scratchpad>`. The copies under this folder are byte-identical to the scratch files (compared with
  `cmp`). No script is executed from `tests/native/`.
- No sha256 of any round record, review file, REPORT or roadmap is written in this folder. Identity hashes come only from
  the workflow `identity.py` output, run with pathspecs that exclude `docs/vernacular/rounds/` and `roadmap.md`, and are
  pasted verbatim.

## Checks covered by R1

- C22 and C43: no pending wording in the record (except the quoted dispatch paragraph) or the old plan files; correction
  notes present; HALT-1 and OP1 to OP3 carry the exact delegated tag. R1 wrote these.
- C32: the record's closure table has one row for each of R1 to R12, each citing a file:line of the final text. The R6 row
  cites `red/red-run.txt` and `green/checks.txt` only.
- C61: the record's D81 line is the placeholder `PLACEHOLDER: prober pending (waves 5, 7 and 10), replaced by R2` until R2
  writes the prober counts from `probes/PROBES.md`.

## State of this folder (coordinator sync, 2026-11-06)

Superseded (C01 and C02 stage): this section describes the folder at the C01 and C02 syncs. The state after C03 is in the last section of this file.

This section supersedes the path table and wave order above wherever they say a file "is written by" a later wave. Checked against the files on 2026-11-06.

- Present: `plan/`, `red/` (`ex_checks.py`, `red-run.txt`, `red-audit-run.txt`, `RED.md`, `table.txt`, `raw/`, snapshots), `green/` (`checks-final.txt`, `checks-closing.txt`, `sql-c01.txt`, `sql_c01.py`, `linkcheck*.txt`, `linkcheck.py`, `identity*.txt`, `redaction*.txt`, `scope*.txt`, `frozen*.txt`), `probes/baseline.txt`, `probes/baseline-closing.txt`, `probes/baseline-final.txt`, `raw/`.
- Absent: `green/checks.txt`, `probes/PROBES.md`, `probes/mutants.tsv`, `probes/raw/` and `REPORT.md`. The mutation prober never applied a mutant.
- Earlier-stage logs: every `green/*-final.txt`, `green/*-closing.txt`, `green/sql-c01.txt`, `green/identity*.txt` and `probes/baseline*.txt` was written before the last edits to `execution-contract.md` and `v5-alignment.md` and reports pre-C01 text. They do not describe the final candidate. Final figures (core checker 99 PASS and 0 FAIL, audit 5 FAIL, SQL 23 of 23 as expected, 417 links ok, final candidate identity) are in the round record, section "V5-02-R15-A1-C01 outcome and closure table".
- Open: write `green/checks.txt` and a fresh `green/sql-c01.txt`, then run the prober on a fresh overlay of the final candidate, or withdraw D81 by operator decision.

## State of this folder (C03 coordinator sync, 2026-11-06)

This section supersedes the path table, the wave orders and the two earlier state sections wherever they say that a file is absent, pending or written by a later wave. Checked against the files on 2026-11-06.

- Present now: `probes/PROBES.md`, `probes/mutants.tsv` and `probes/raw/` (one log per mutant, controls, baselines and the first logs of the repaired survivors, `*.survived-1.log`), `probes/reprobe/`, and in `green/` the final-candidate logs `checks.txt`, `sql-final.txt`, `linkcheck-c02.txt`, `redaction-c02.txt` and `audit-final.txt`, the C03 logs (`freeze-red-c03.txt`, `freeze-c03.txt`, `messages-before-c03.txt`, `messages-diff-c03.txt`, `record-asserts-red-c03.txt`, `record-asserts-c03.txt`, `reprobe-c03.txt`, `identity-compare-c03.txt`, `audit-pre-rec-c02.txt`, `staging-candidates-c03.txt`), the scripts `record_asserts_c03.py` and `rec_preflight_c03.py`, the refreshed `frozen/` copies and `superseded-1/` to `superseded-4/` (earlier copies of rewritten logs). The C03 plan is in `tests/native/v5-02-r15-a1-c03-evidence/plan/` and the gate report in `tests/native/v5-02-r15-a1-c03-evidence/REPORT.md`.
- Absent now: `green/audit-verify-c02.txt`, `green/redaction-final-c02.txt` and `green/record-asserts-final-c03.txt` (GREEN2-only paths). Nothing else this README called absent is absent.
- Final figures: the default checker run in `green/checks.txt` and the audit run in `green/audit-final.txt` were written with the final checker (`green/ex_checks-c02.py`); the mutation prober killed 161 of 161 mutants (survivors repaired and probed again); the figures and the file hashes are in the round record, sections 'Final results (C03)' and 'C03 coordinator sync'.
- Older logs: `green/checks-final.txt`, `green/checks-closing.txt`, `green/sql-c01.txt`, the other `green/*-final.txt` and `green/*-closing.txt` files and `probes/baseline*.txt` are C01-stage logs; they do not describe the final candidate and are kept.
- Open: the FIN steps run again after the last edit of the round record, because several FIN logs are older than the record (see 'C03 remaining limits' in the record).

## State of this folder (C04, 2026-11-06)

This section supersedes the 'Absent now' and 'Open' bullets of the C03 section above and the statements in the earlier sections that a log is written by a later wave. Checked against the files on 2026-11-06.

- Written by the FIN phase of C04, in this order and after the last edit of the round record, this README, `red/RED.md` and the checker: `green/checks.txt`, `green/linkcheck-c02.txt`, `green/redaction-c02.txt`, `green/audit-final.txt`, `green/redaction-final-c04.txt` and `green/record-asserts-final-c04.txt`. Their results are in the logs, not in this file.
- Never written, and not staged: `green/audit-verify-c02.txt`, `green/redaction-final-c02.txt` and `green/record-asserts-final-c03.txt`. The C04 names replace them.
- The C03 FIN1 logs `green/record-asserts-c03.txt`, `green/reprobe-c03.txt` and `green/identity-compare-c03.txt` are not rewritten in C04 and describe the C03 record state.
- The C04 evidence folder is `tests/native/v5-02-r15-a1-c04-evidence/`: `probes/PROBES.md` with the logs of the mutants of the two checker edits (C4-m1 to C4-m3, with controls, on the C04 and on the C03 checker), `record_asserts_c04.py` with `record_asserts_c04.diff`, and the output of the unmodified C03 assertion script.
- `green/frozen/` holds refreshed copies of the roadmap and of the C03 review.
- The six first-attempt probe logs that were overwritten in C03 are lost; see the round record, 'Lost first-attempt probe logs (C03-R4)'.
