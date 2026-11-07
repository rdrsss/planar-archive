# Probes of the C04 checker edits (R15-A1-C04)

Two edits of `tests/native/v5-02-r15-a1-c01-evidence/green/ex_checks-c02.py` need a probe, because the C03 review (R2, R5) found that no mutant covered them:

- C03-R2: `staging_paths()` also returns a bullet whose whole content is one slashless backticked token (`CLAUDE.md`), so the existence clause and the redaction clause of `CHK-C02-evidence-logs` cover it.
- C03-R5: clause 9 of `CHK-C02-evidence-logs` no longer skips lines that start with a pipe or whitespace.

Method. A scratch clone of HEAD (outside the checkout), overlaid with the working-tree `docs/` and the R15-A1 evidence trees, whose `green/redaction-c02.txt` is completed so that it holds one `exit 0` line for every bulleted staging path (this is the state the FIN phase writes). The checker is run as `python3 -I -B <scratchpad>/c04/<checker> --repo <clone> --frozen-dir <scratchpad>/c04/frozen --audit CHK-C02-evidence-logs` from the clone. Every probe runs twice: on the C04 checker, and on the C03 final checker reconstructed by reverting the three C04 edits (its sha256 equals the C03 record, 8745c3bf...). The control must exit 0 on both checkers. A mutant is KILLED when the C04 checker exits non-zero and a line `FAIL CHK-C02-evidence-logs` names the mutation. The run on the C03 checker is the red-first reference: a mutant that survives there is a surviving mutant of C03.

The record in the clone is the live record at the time of the run (after its last edit before the FIN phase). Nothing in the checkout is changed by a probe. Logs are in this folder.

| id | mutation | C04 checker | C03 checker (red-first) | log |
|---|---|---|---|---|
| C4-c1 | control: clean clone | control-ok (exit 0) | control-ok (exit 0) | `C4-c1.log`, `C4-c1-on-c03-checker.log` |
| C4-m1 | drop the line `exit 0 CLAUDE.md` from the scratch `green/redaction-c02.txt` | KILLED: `green/redaction-c02.txt lacks "exit 0 CLAUDE.md"` | survives (exit 0) | `C4-m1.log`, `C4-m1-on-c03-checker.log` |
| C4-m2 | add the table row `| audit-final.txt: 122 PASS |` to the scratch record | KILLED: `the record quotes a result of audit-final` | survives (exit 0) | `C4-m2.log`, `C4-m2-on-c03-checker.log` |
| C4-m3 | add the indented line `  audit-final.txt: 122 PASS` to the scratch record | KILLED: `the record quotes a result of audit-final` | survives (exit 0) | `C4-m3.log`, `C4-m3-on-c03-checker.log` |

Totals (C4-m1 to C4-m3, controls excluded): killed 3, survived 0, not run 0. C4-m2 and C4-m3 were requested by the C03 review (R5, the table row and the indented line); C4-m1 is the mutant of R2. C4-m3 is added by C04.

The other file here, `record-asserts-c03-unmodified-final-phase.txt`, is the output of the unmodified C03 record assertion script (final phase) on the C04 record; it shows why `../record_asserts_c04.py` exists.
