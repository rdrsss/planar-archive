# Mutation prober, main section (R15-A1-C03 PRA)

Command: `perl -e "alarm 600; exec @ARGV" python3 -I <scratchpad>/c03/ex_checks-c02.py --repo <scratchpad>/c03/work --frozen-dir <scratchpad>/c03/frozen` (default mode; `<frozen-dir>` is a scratch copy `frozen-mut` for the mutants that change a frozen input). `timeout(1)` is not installed; exit 127 would read "not run". The exit code is read directly.
Environment: macOS (Darwin), python3 stdlib only, scratch clone `<scratchpad>/c03/pristine` (git clone of HEAD overlaid with the working tree and the evidence trees), working clone `work` restored from pristine by `rsync --delete` before every mutant.
Baseline BD: `probes/raw/baseline-c02-default.log` (exit 0, PASS 113, FAIL 0, registered 113). Runner counts line `runner counts: PASS count: P; FAIL count: F; registered count: R` precedes the mask line of every raw log.
Verdict rule (D6, D32): KILLED only if exit != 0, a line starting `FAIL <test>` names the row test, the test is a PASS line of BD (or of the row control), F >= 1 and P + F = R; otherwise SURVIVED or `not run`.

## Per-mutant results

- C5-m1: KILLED by CHK-C01-R2-ordinal-check; FAIL message: transitions accepts ordinal 0
- C24-m1: KILLED by CHK-C01-sql-counts; FAIL message: SQL trigger count 42 differs from 43
- C25-m1: KILLED by CHK-C01-sql-legal-rows; FAIL message: legal running role_state row refused: {'state': 'running', 'run_id': 5, 'drifted': 0}
- C26-m1: KILLED by CHK-C01-names-preserved; FAIL message: registered old check ids differ from the preserved original (names or order) Runner note: the mutated checker registers 112 ids, so its runner counts line states registered count 112 (P 111 + F 1); with the fixed 113 the P + F = R rule could never hold for a mutant that deletes a registration.
- C27-m1: KILLED by CHK-C01-frozen-files; FAIL message: docs/vernacular/ledger-contract.md differs from the baseline candidate
- C27-m2: KILLED by CHK-C01-frozen-files; FAIL message: roadmap.md differs from its frozen copy
- C28-m1: KILLED by CHK-C01-message-bytes; FAIL message: message not byte-identical in the contract: Activate may be complete; retry the same request ID or inspect request history
- C34-m1: KILLED by CHK-I2-tables-strict; FAIL message: table set or creation order differs: ['ledger_meta', 'records', 'tree_edges', 'revisions', 'events', 'current_state', 'reque
- C35-m1: KILLED by CHK-C01-extractor-comments; FAIL message: the CREATE TABLE evidence line is not unique in the SQL block
- C35-c1 (control, source row C35): control-ok; CHK-C01-extractor-comments runs and PASSes (rc=0 P=1 F=0). Excluded from every total. Run with the single id CHK-C01-extractor-comments (P = R = 1): any extra line in the contract breaks CHK-C02-contract-one-line (line count and the line-520-only rule), so a whole default run cannot exit 0 with a longer SQL block.
- C36-m1: KILLED by CHK-I50-no-tbd; FAIL message: closed-text hygiene: TBD present
- C37-m1: KILLED by CHK-I6-reserved-labels; FAIL message: table evidence must be preceded by "Reserved for R17" (found [])
- C38-m1: KILLED by CHK-I1-skeleton; FAIL message: H2 headings differ from the 16 required in order: ['Common rules', 'EX-1 Format3', 'EX-2 Request framing and replay', 'EX-3 Execu
- C44-m1: KILLED by CHK-I2-tables-strict; FAIL message: table set or creation order differs: ['ledger_meta', 'records', 'tree_edges', 'revisions', 'events', 'current_state', 'reque
- C45-m1: KILLED by CHK-I5-fk-resolve; FAIL message: reservations.request_id has REFERENCES but no design arrow
- C46-m1: KILLED by CHK-I47-sql-executes; FAIL message: explicit indexes differ: ['run_state_one_active']
- C47-m1: KILLED by CHK-I53-ex1-columns; FAIL message: runs.secret lacks CHECK length(secret)=16
- C49-m1: KILLED by CHK-I8-cutover; FAIL message: old binary refusal missing
- C50-m1: KILLED by CHK-I39-v5-alignment; FAIL message: E5 narrowing of v5 :241 entry missing
- C51-m1: KILLED by CHK-I42-links-anchors; FAIL message: docs/vernacular/execution-contract.md links missing anchor #ex-9-nothing in docs/vernacular/execution-contract.md
- C52-m1: KILLED by CHK-I46-record-no-hash; FAIL message: record contains a sha256 of a forbidden file: ['<scratchpad>
- C52-m2: KILLED by CHK-I46-record-no-hash; FAIL message: record contains a sha256 of a forbidden file: ['<scratchpad>
- C53-m1: KILLED by CHK-I64-limit-names; FAIL message: observe limit observation_entries not kept with Capture limit exceeded
- C54-m1: KILLED by CHK-I66-reused-messages; FAIL message: reused message not found in any owner: Capture limit exceed
- C55-m1: KILLED by CHK-I60-role-rows; FAIL message: M table row count != 41
- C55-m2: KILLED by CHK-I60-role-rows; FAIL message: stale -> ready rows without a new-generation cause and an allowed reason: ['M8']
- C55-m3: KILLED by CHK-I60-role-rows; FAIL message: stale -> ready rows without a new-generation cause and an allowed reason: ['M8']
- C56-m1: KILLED by CHK-I69-stale-done; FAIL message: no stale -> ready statement missing
- C56-m2: KILLED by CHK-I69-stale-done; FAIL message: OP3 options A, B, C missing
- C58-m1: KILLED by CHK-I10-history-lookup; FAIL message: history --request-id is said to add a disposition field First verdict SURVIVED, repaired and re-probed, see Survivors.
- C59-m1: KILLED by CHK-I27-run-transitions; FAIL message: run table must have 11 rows N1-N11, found 10
- C59b-m1: KILLED by CHK-I28-run-role-coupling; FAIL message: N1 paired role reason must name claimed (cell: (M26))
- C59b-m2: KILLED by CHK-I70-absent-active-job; FAIL message: M14 must name both causes (role removed and active job absent)
- C60-m1: KILLED by CHK-I51-header; FAIL message: header lacks the statement that it narrows no clause of any approved contract First verdict SURVIVED, repaired and re-probed, see Survivors.
- G1-m1: KILLED by CHK-I43-redaction; FAIL message: redaction check failed for the contract: local path in <scratchpad>
- G2-m1: KILLED by CHK-I44-identity-src-tests; FAIL message: src or tests changed: [] ['src/zz_probe_untracked.txt']
- G2-m2: KILLED by CHK-I44-identity-src-tests; FAIL message: src or tests changed: ['src/cmd/planar/authoring.cpp'] []
- G3-m1: KILLED by CHK-I35-claude-md-symlink; FAIL message: git diff --numstat CLAUDE.md is ['3', '0'], want 1 0
- G4-m1: KILLED by CHK-I24-no-removed-diagnostics; FAIL message: docs/vernacular/external-profile-contract.md has removed lines (| Module portable spelling | `path` / `Invalid ext
- G5-m1: KILLED by CHK-I36-ledger-pointers; FAIL message: ledger-contract.md has 1 removed lines
- G6-m1: KILLED by CHK-I41-citation-range; FAIL message: citation ledger-contract.md:9999 points past end of file
- G7-m1: KILLED by CHK-I22-code-table; FAIL message: code fenced exit is not 3
- G8-m1: KILLED by CHK-I45-diff-scope; FAIL message: changes outside owned files: ['docs/vernacular/render-contract.md']
- G8-m2: KILLED by CHK-I45-diff-scope; FAIL message: docs/vernacular/rounds/V5-02-R15-A1-C01-review.md differs from its frozen copy
- G8-m3: KILLED by CHK-I45-diff-scope; FAIL message: docs/vernacular/rounds/V5-02-R15-A1-C02-review.md differs from its frozen copy
- G8-m4: KILLED by CHK-I45-diff-scope; FAIL message: file missing: <scratchpad>
- C3-F1-m1: KILLED by CHK-I45-diff-scope; FAIL message: file missing: <scratchpad>
- C3-F1-m2: KILLED by CHK-I45-diff-scope; FAIL message: docs/vernacular/rounds/V5-02-R15-A1-C02-review.md differs from its frozen copy
- C3-F1-m3: KILLED by CHK-I45-diff-scope; FAIL message: changes outside owned files: ['docs/vernacular/rounds/V5-02-R15-A1-C03-extra.md']
- C3-A6-m1: KILLED by CHK-I46-record-no-hash; FAIL message: record contains a sha256 of a forbidden file: ['<scratchpad>
- N1-m1: KILLED by CHK-C02-op2-history-tag; FAIL message: the OP2 line lacks the exact tag operator-decided (delegated 2026-10-31, option A)
- N2-m1: KILLED by CHK-C02-contract-one-line; FAIL message: contract lines differing from the snapshot are [520, 521], expected [520]
- N2-m2: KILLED by CHK-C02-contract-one-line; FAIL message: contract line count 1089 differs from the snapshot 1090
- N3-m1: KILLED by CHK-C02-record-status; FAIL message: record line 3 still says 'D81 is neither run nor withdrawn'
- N3-m2: KILLED by CHK-C02-record-status; FAIL message: no "Superseded in C02:" note naming probes/PROBES.md and Final results (C02) within three lines after the bullet "- Mutatio
- N3-m3: KILLED by CHK-C02-record-status; FAIL message: no "Superseded in C02:" note naming probes/PROBES.md and Final results (C02) within three lines before the invariant table 
- N3-m4: KILLED by CHK-C02-record-status; FAIL message: no "Superseded in C02:" note naming probes/PROBES.md and Final results (C02) within three lines after the bullet "- **Mutat
- N3-m5: KILLED by CHK-C02-record-status; FAIL message: the original text "not run, no verdict" was rewritten
- N4-m1: KILLED by CHK-C02-record-section-c; FAIL message: section (c) still says 'does not record an operator decision'
- N5-m1: KILLED by CHK-C02-record-post-c01-ac; FAIL message: Post-C01 status lacks the row for 7
- N5-m2: KILLED by CHK-C02-record-post-c01-ac; FAIL message: Post-C01 status row 3 does not state the HALT-1 amendment
- N5-m3: KILLED by CHK-C02-record-post-c01-ac; FAIL message: Post-C01 status row 5 status '' is not one of met, partly met, not met
- N6-m1: KILLED by CHK-C02-record-r8; FAIL message: the record still says "are not reconciled here"
- N6-m2: KILLED by CHK-C02-record-r8; FAIL message: no paragraph or bullet explains raw/orig-checker-run.txt (15 FAIL against 13, I45 and I59 ran on a grown tree)
- N7-m1: KILLED by CHK-C02-red-md; FAIL message: red/RED.md lacks 'irrelevant first attempt'
- N7-m2: KILLED by CHK-C02-red-md; FAIL message: red/RED.md lacks the failing check name CHK-C01-R1-a09-lookup
- N7-m3: KILLED by CHK-C02-red-md; FAIL message: the record block Evidence by pointer lacks 'irrelevant first attempt'
- N7-m4: KILLED by CHK-C02-red-md; FAIL message: the record block Evidence by pointer lacks 'raw/red-checks'
- N7-m5: KILLED by CHK-C02-red-md; FAIL message: README.md red/ row lacks 'red-audit-run.txt'
- N7-m6: KILLED by CHK-C02-red-md; FAIL message: README.md red/ row lacks 'irrelevant first attempt'
- N7-m7: KILLED by CHK-C02-red-md; FAIL message: README.md still says "Nothing here is committed"
- N7-m8: KILLED by CHK-C02-red-md; FAIL message: README.md red/ row lacks 'raw/red-checks'
- N8-m1: KILLED by CHK-C02-raw-preserved; FAIL message: raw evidence file deleted: tests/native/v5-02-r15-a1-c01-evidence/raw/orig-checker-run.txt
- N8-m2: KILLED by CHK-C02-raw-preserved; FAIL message: raw evidence file changed size: tests/native/v5-02-r15-a1-c01-evidence/red/red-run.txt
- N8-m3: KILLED by CHK-C02-raw-preserved; FAIL message: raw evidence file edited: tests/native/v5-02-r15-a1-evidence/review-c01/redaction-c01-review.txt
- N8-m4: KILLED by CHK-C02-raw-preserved; FAIL message: raw evidence file deleted: tests/native/v5-02-r15-a1-c01-evidence/red/snapshot-2/SHA256SUMS
- N8-m5: KILLED by CHK-C02-raw-preserved; FAIL message: raw evidence file deleted: tests/native/v5-02-r15-a1-evidence/raw/diffstat.txt
- N8-m6: KILLED by CHK-C02-raw-preserved; FAIL message: raw evidence file deleted: tests/native/v5-02-r15-a1-c01-evidence/plan/superseded-1/INVARIANTS.md
- N8-m7: KILLED by CHK-C02-raw-preserved; FAIL message: REPORT file differs from its frozen copy: tests/native/v5-02-r15-a1-evidence/REPORT.md
- N10-m1: KILLED by CHK-C02-snapshot-baseline; FAIL message: no line of the Final candidate identity block names execution-contract.md with the snapshot sha256
- N10-m2: KILLED by CHK-C02-snapshot-baseline; FAIL message: no line of the Final candidate identity block names execution-contract.md with the snapshot sha256
- N11-m1: KILLED by CHK-C02-record-c02-ac; FAIL message: C02 acceptance status lacks the row for 4
- N11-m2: KILLED by CHK-C02-record-c02-ac; FAIL message: C02 acceptance status lacks the row for G3
- N11-m3: KILLED by CHK-C02-record-c02-ac; FAIL message: Acceptance (C01) row 3 still says "D81 neither run nor withdrawn"
- N11-m4: KILLED by CHK-C02-record-c02-ac; FAIL message: C02 acceptance status row 2 status 'ok' is not one of met, partly met, not met
- N12-m1: KILLED by CHK-C02-record-owned-files; FAIL message: Owned files (C02) does not name tests/native/v5-02-r15-a1-c01-evidence/green/mask_log.py
- N12-m2: KILLED by CHK-C02-record-owned-files; FAIL message: Owned files (C02) does not name tests/native/v5-02-r15-a1-c01-evidence/probes/mutants.tsv
- N13-m1: KILLED by CHK-C02-record-reviewer-control; FAIL message: the record does not cite the reviewer redaction positive control outside the Staging list
- N13-m2: KILLED by CHK-C02-record-reviewer-control; FAIL message: the reviewer file holds no non-zero "exit" line for the positive control
- N14-m1: KILLED by CHK-C02-record-d6-labels; FAIL message: Checker amendments (C02) does not name CHK-I45-diff-scope
- N14-m2: KILLED by CHK-C02-record-d6-labels; FAIL message: Checker amendments (C02) does not name CHK-C01-record-d81
- N15-m1: KILLED by CHK-C02-red-logs; FAIL message: red/red-c02-run.txt FAIL set differs: extra [], missing ['CHK-C02-record-status']
- N15-m2: KILLED by CHK-C02-red-logs; FAIL message: red/red-c02-run.txt contract row differs from the sha256 of raw/pre-c02-contract.md The first attempt altered the hash inside the sha256(git diff ...) row instead of the contract file row and survived; that was a defect of the mutant text, not of the check, so its log was replaced by the run of the corrected mutant (the contract file row of red-c02-run.txt).
- N15-m3: KILLED by CHK-C02-red-logs; FAIL message: red/red-c02-run.txt lacks "Command exit code: 1"
- N15-m4: KILLED by CHK-C02-red-logs; FAIL message: red/red-c02-run.txt FAIL set differs: extra ['CHK-I1-skeleton'], missing []
- N15-m5: KILLED by CHK-C02-red-logs; FAIL message: red/red-c02-run.txt holds a host path

main section totals: killed 95, survived 0, not run 0

## Notes

- C55-m1 note: C55-m1 (add a row M42 `stale` to `ready` with reason `prerequisite_available`) is killed by the row-count pin of CHK-I60-role-rows, not by its stale-to-ready rule. The real FAIL message read from probes/raw/C55-m1.log is `FAIL CHK-I60-role-rows M table row count != 41`; the stale-to-ready rule is exercised by C55-m2 and C55-m3, which fail with `stale -> ready rows without a new-generation cause and an allowed reason: ['M8']`.
- Mutants whose edit changes the number of lines of execution-contract.md are also reported by CHK-C02-contract-one-line; the verdict names only the row's own test.
- Source row spelling: guard rows use the C01 id (C35, G8, ...), never PR-C35.
- C3-A8-m1: after the main wave `cmp` of CLAUDE.md and the six other candidate documents (execution-contract.md, v5-alignment.md, ledger-contract.md, external-profile-contract.md, acceptance-matrix.md, implementation.md) between the checkout and the scratch pristine copy reported no difference, `AGENTS.md` still links to CLAUDE.md and `git diff HEAD -- src tests` is empty: C3-A8-m1 identical, the probe waves changed none of them.
- Survivor repairs of this wave (both in the bodies of E/green/ex_checks-c02.py, assertions added, no existing message changed): CHK-I10-history-lookup (C58-m1) and CHK-I51-header (C60-m1); BD retaken after the repair (the previous copy is baseline-c02-default.pre-repair-1.log), and the mutants of both checks re-probed.

## Survivors

- C58-m1: first verdict SURVIVED (log `probes/raw/C58-m1.survived-1.log`); CHK-I10-history-lookup only required that some unit states "no disposition field", so a sentence adding the field passed. Repair (assertion added, no message changed): CHK-I10-history-lookup now also fails when any unit says the history --request-id response adds a disposition field (new message "history --request-id is said to add a disposition field"). Re-probed: KILLED (final verdict above).
- C60-m1: first verdict SURVIVED (log `probes/raw/C60-m1.survived-1.log`); CHK-I51-header never read the words "narrows no clause of any approved contract", so deleting them passed. Repair (assertion added, no message changed): CHK-I51-header now requires that sentence in the header (new message "header lacks the statement that it narrows no clause of any approved contract"). Re-probed: KILLED (final verdict above).
- C31-m12: first verdict SURVIVED (log `probes/raw/C31-m12.survived-1.log`); CHK-C01-prober-evidence only required one tsv row per closing group (any N16-m row satisfied it), so deleting the N16-m1 row passed. Repair (assertion added, no message changed): when closing rows are not pending, every raw log whose first line is `mutant id: <id>` must have a row for <id> in mutants.tsv (new message "probes/raw/<log> has no row in mutants.tsv"). Re-probed against the retaken BA and BC: KILLED (final verdict in Closing probes).

## Closing probes

Closing fixture: `<scratchpad>/c03/fixture` (the checkout plus the green logs generated by the existing scripts, the main-section probe files and the scratch record edits of INVARIANTS 6 rule 7; the checkout record is untouched). Mutants run in `<scratchpad>/c03/cwork`, restored from the fixture before every mutant. Baseline BA: `probes/raw/baseline-c02-audit.log` (`--audit --pending closing --pending acceptance`, exit 0, PASS 122, FAIL 0, registered 122). Baseline BC: `probes/raw/baseline-c02-audit-closed.log` (`--audit --pending acceptance`, taken after every other closing mutant and this section existed). N9-c2 and N9-m8 run without `--pending acceptance` (`--audit --pending closing`) so that clause 7 of CHK-C02-evidence-logs runs. C3-F2 and C3-F3: clause 8 of CHK-C02-evidence-logs and CHK-C02-record-status do not depend on the pending flags, so their controls and mutants use the BA command. AU-C30 mutants were probed before any other closing row and no repair was needed. Verdict rule as in the main section (D6, D32).

- N9-c1 (control, source row N9): control-ok; CHK-C02-evidence-logs runs and PASSes (rc=0 P=122 F=0). Excluded from every total.
- N9-c2 (control, source row N9): control-ok; CHK-C02-evidence-logs runs and PASSes (rc=0 P=122 F=0). Excluded from every total.
- N9-c3 (control, source row N9): control-ok; CHK-C02-evidence-logs runs and PASSes (rc=0 P=122 F=0). Excluded from every total.
- N17-c1 (control, source row N17): control-ok; CHK-C02-logs-masked runs and PASSes (rc=0 P=122 F=0). Excluded from every total.
- C3-F2-c1 (control, source row AU-C3-F2): control-ok; CHK-C02-evidence-logs runs and PASSes (rc=0 P=122 F=0). Excluded from every total.
- C3-F3-c1 (control, source row AU-C3-F3): control-ok; CHK-C02-record-status runs and PASSes (rc=0 P=122 F=0). Excluded from every total.
- C30-m1: KILLED by CHK-C01-green-evidence; FAIL message: green run PASS count 112 differs from 113 core checks
- C30-m2: KILLED by CHK-C01-green-evidence; FAIL message: green run exit code 0 is not recorded
- C30-m3: KILLED by CHK-C01-green-evidence; FAIL message: checker sha256 in green/checks.txt differs from green/ex_checks-c02.py
- C30-m4: KILLED by CHK-C01-green-evidence; FAIL message: green/checks.txt lacks the "checker sha256:" line
- C30-m5: KILLED by CHK-C01-green-evidence; FAIL message: green/checks.txt identity block differs from green/identity-c02.txt
- C30-m6: KILLED by CHK-C01-green-evidence; FAIL message: green/checks.txt identity row for docs/vernacular/v5-alignment.md differs from the live file
- C29-m1: KILLED by CHK-C01-sql-evidence-recorded; FAIL message: statement [3] expected refuse but the log records accepted
- C29-m2: KILLED by CHK-C01-sql-evidence-recorded; FAIL message: a clean foreign_key_check is not recorded
- C29-m3: KILLED by CHK-C01-sql-evidence-recorded; FAIL message: green/sql-final.txt identity block differs from green/identity-c02.txt
- C29-m4: KILLED by CHK-C01-sql-evidence-recorded; FAIL message: green/sql-final.txt lacks the identity markers (identity begin (seven documents) / identity end)
- C31-m1: KILLED by CHK-C01-prober-evidence; FAIL message: probes/raw/<mutant id>.log is missing or empty
- C31-m2: KILLED by CHK-C01-prober-evidence; FAIL message: PROBES.md lacks a line for <mutant id>
- C31-m3: KILLED by CHK-C01-prober-evidence; FAIL message: killed mutant <mutant id> has no FAIL line naming its test
- C31-m4: KILLED by CHK-C01-prober-evidence; FAIL message: PROBES.md totals (94, 0, 0) differ from mutants.tsv (95, 0, 0)
- C31-m5: KILLED by CHK-C01-prober-evidence; FAIL message: probes/raw/<mutant id>.log lacks the line "scratch differs from pristine: yes"
- C31-m6: KILLED by CHK-C01-prober-evidence; FAIL message: probes/raw/baseline-c02-default.log holds a FAIL line
- C31-m7: KILLED by CHK-C01-prober-evidence; FAIL message: killed mutant <mutant id>: test CHK-C01-message-bytes is not a PASS line of its baseline
- C31-m8: KILLED by CHK-C01-prober-evidence; FAIL message: the Survivors section does not name <mutant id>
- C31-m13: KILLED by CHK-C01-prober-evidence; FAIL message: control <mutant id> has verdict 'KILLED', not control-ok
- C32-m1: KILLED by CHK-C01-record-closure; FAIL message: closure table lacks one row each for R1 to R12: [1, 2, 3, 4, 5, 6, 8, 9, 10, 11, 12]
- C32-m2: KILLED by CHK-C01-record-closure; FAIL message: closure table lacks one row each for R1 to R12: [1, 2, 3, 4, 5, 7, 8, 9, 10, 11, 12]
- C33-m1: KILLED by CHK-C01-red-evidence; FAIL message: identity.py output is not pasted verbatim in red-run.txt: ['| sha256(git diff --binary HEAD -- docs/vernacular/execution-con
- C33-m2: KILLED by CHK-C01-red-evidence; FAIL message: red-audit-run.txt does not FAIL every audit id: ['CHK-C01-sql-evidence-recorded']
- C61-m1: KILLED by CHK-C01-record-d81; FAIL message: D81 counts (96 killed, 0 survived) differ from PROBES.md totals
- C61-m2: KILLED by CHK-C01-record-d81; FAIL message: PLACEHOLDER marker remains in the D81 resolution
- C61-m3: KILLED by CHK-C01-record-d81; FAIL message: the D81 resolution still says "not RUN, open"
- C61-m4: KILLED by CHK-C01-record-d81; FAIL message: the D81 resolution still says "This record does not choose"
- N9-m1: KILLED by CHK-C02-evidence-logs; FAIL message: the Staging list names inventory.txt
- N9-m2: KILLED by CHK-C02-evidence-logs; FAIL message: the Staging list names a superseded INVARIANTS.md: tests/native/v5-02-r15-a1-evidence/plan/superseded-1/INVARIANTS.md
- N9-m3: KILLED by CHK-C02-evidence-logs; FAIL message: green/redaction-c02.txt lacks "exit 0 docs/vernacular/execution-contract.md"
- N9-m4: KILLED by CHK-C02-evidence-logs; FAIL message: green/redaction-c02.txt lacks the positive control line (non-zero exit)
- N9-m5: KILLED by CHK-C02-evidence-logs; FAIL message: green/linkcheck-c02.txt lacks "links checked: X; ok: X; broken: 0" with X above zero
- N9-m6: KILLED by CHK-C02-evidence-logs; FAIL message: green/cmp-c02.txt lacks "cmp checker exit 0"
- N9-m7: KILLED by CHK-C02-evidence-logs; FAIL message: green/cmp-c02.txt lacks "cmp link checker exit 0"
- N9-m8: KILLED by CHK-C02-evidence-logs; FAIL message: C02 acceptance row 1 basis still says pending
- N9-m9: KILLED by CHK-C02-evidence-logs; FAIL message: Final results (C02) PASS count 114 differs from green/checks.txt (113)
- N9-m10: KILLED by CHK-C02-evidence-logs; FAIL message: green/audit-final.txt identity block differs from green/identity-c02.txt
- N9-m11: KILLED by CHK-C02-evidence-logs; FAIL message: green/audit-final.txt command carries --pending
- N9-m12: KILLED by CHK-C02-evidence-logs; FAIL message: green/linkcheck-c02.txt lacks "files checked: 11"
- N9-m13: KILLED by CHK-C02-evidence-logs; FAIL message: green/linkcheck-c02.txt command does not name docs/vernacular/rounds/V5-02-R15-A1.md
- N9-m14: KILLED by CHK-C02-evidence-logs; FAIL message: the record quotes a result of audit-final: audit-final.txt: 122 PASS
- N9-m15: KILLED by CHK-C02-evidence-logs; FAIL message: a Staging list path does not exist: tests/native/v5-02-r15-a1-c01-evidence/green/frozen-c02.txt
- N9-m16: KILLED by CHK-C02-evidence-logs; FAIL message: green/linkcheck-c02.txt lacks the negative control
- N9-m17: KILLED by CHK-C02-evidence-logs; FAIL message: green/identity-c02.txt identity row for CLAUDE.md differs from the live file
- N9-m18: KILLED by CHK-C02-evidence-logs; FAIL message: green/redaction-final-c02.txt lacks "exit 0 green/audit-final.txt" or audit-final.txt is absent
- N9-m19: KILLED by CHK-C02-evidence-logs; FAIL message: green/redaction-final-c02.txt lacks "exit 0 green/audit-final.txt" or audit-final.txt is absent
- N16-m1: KILLED by CHK-C02-identity-block; FAIL message: identity block row for CLAUDE.md differs from the live file
- N16-m2: KILLED by CHK-C02-identity-block; FAIL message: the identity block lacks a current row for: ['docs/vernacular/execution-contract.md']
- N17-m1: KILLED by CHK-C02-logs-masked; FAIL message: probes/raw/<mutant id>.log does not end with the mask line
- N17-m2: KILLED by CHK-C02-logs-masked; FAIL message: probes/raw/<mutant id>.log holds a host path
- N17-m3: KILLED by CHK-C02-logs-masked; FAIL message: probes/raw/<mutant id>.log holds the sha256 of a forbidden file (roadmap.md)
- N17-m4: KILLED by CHK-C02-logs-masked; FAIL message: probes/raw/<mutant id>.log holds the sha256 of a forbidden file (REPORT.md)
- N17-m5: KILLED by CHK-C02-logs-masked; FAIL message: green/redaction-final-c02.txt does not end with the mask line
- N17-m6: KILLED by CHK-C02-logs-masked; FAIL message: green/redaction-c02.txt does not end with the mask line
- C3-F2-m1: KILLED by CHK-C02-evidence-logs; FAIL message: Final results (C02) PASS count 114 differs from green/checks.txt (113)
- C3-F2-m2: KILLED by CHK-C02-evidence-logs; FAIL message: the record has no Final results (C02) subsection
- C3-F3-m1: KILLED by CHK-C02-record-status; FAIL message: no "Superseded in C02:" note naming probes/PROBES.md and Final results (C02) within three lines after the bullet "- **Mutat
- C3-F3-m2: KILLED by CHK-C02-record-status; FAIL message: no "Superseded in C02:" note naming probes/PROBES.md and Final results (C02) within three lines after the bullet "- **Mutat
- C3-A2-m1: KILLED by CHK-C01-prober-evidence; FAIL message: PROBES.md totals (95, 0, 0) differ from mutants.tsv (94, 1, 0)
- C3-A3a-m1: KILLED by CHK-C01-sql-evidence-recorded; FAIL message: the 25 probe statements are not counted
- C3-A3a-m2: KILLED by CHK-C01-sql-evidence-recorded; FAIL message: green/sql-final.txt identity block differs from green/identity-c02.txt
- C3-A3c-m1: KILLED by CHK-C02-evidence-logs; FAIL message: a Staging list path does not exist: tests/native/v5-02-r15-a1-c01-evidence/green/nonexistent-c03.txt
- C31-m9: KILLED by CHK-C01-prober-evidence; FAIL message: probes/raw/<mutant id>.log is missing or empty
- C31-m10: KILLED by CHK-C01-prober-evidence; FAIL message: PROBES.md has no closing probes section
- C31-m11: KILLED by CHK-C01-prober-evidence; FAIL message: probes/raw/<mutant id>.log is a PENDING log
- C31-m12: KILLED by CHK-C01-prober-evidence; FAIL message: probes/raw/<mutant id>.log has no row in mutants.tsv

- Mutant-text note: the first runs of N9-m1, N9-m2 and C3-A3c-m1 survived because the probe appended the staging path under the first textual mention of the heading (the C02 coordinator sync section), not under the Staging list subsection; that was a defect of the mutant text, not of CHK-C02-evidence-logs. The mutant text was corrected (the path is appended inside the `### Staging list (C02)` subsection), the three mutants were re-run, and their logs are the runs of the corrected mutants.
- Survivor repair of this wave: C31-m12 survived (log `probes/raw/C31-m12.survived-1.log`); the body of CHK-C01-prober-evidence in `green/ex_checks-c02.py` gained one assertion (every `probes/raw/*.log` whose first line is `mutant id: <id>` needs a row for that id in `mutants.tsv`; new message only, no existing message changed). The scratch checker and `green/ex_checks-c02.py` are cmp-identical again. BA and BC were retaken (the previous copies are `baseline-c02-audit.pre-repair-1.log` and `baseline-c02-audit-closed.pre-repair-1.log`), and every mutant of the touched check (C31-m1 to C31-m13, C3-A2-m1) was re-probed; BD is unchanged because CHK-C01-prober-evidence is an audit check and BD is a default-mode run. In the re-probe C31-m2 survived once only because the PROBES.md line of another closing row quoted the deleted id, and C31-m8 once because the probe chose a closing survivor; the quoting was removed (closing lines show a placeholder for other ids) and C31-m8 now deletes a main survivor line, so both final verdicts come from corrected mutants.
- C3-A8-m1 for this wave: the closing probes mutate only `<scratchpad>/c03/cwork`, restored from the fixture; the seven candidate documents, CLAUDE.md, `src` and `tests` of the checkout were not touched by them.

closing probes totals: killed 66, survived 0, not run 0
