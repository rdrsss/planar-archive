# V5-02-R15-A1 evidence (documentation round)

Round record: `docs/vernacular/rounds/V5-02-R15-A1.md`. This folder is gitignored. It holds no round-record, review, REPORT or
roadmap hash (record rule, acceptance criterion 10). The pre-C01 candidate identity is in the round record, section (g); the final identity is in its C01 section.

| path | content |
|---|---|
| `plan/DECISIONS.md`, `plan/INVARIANTS.md` | search stage, revision 4: decisions D1 to D96 and the 70-row invariant table |
| `plan/superseded-1`, `-2`, `-3` | earlier revisions, kept, not authoritative |
| `red/ex_checks.py` | evidence copy of the scratch checks CHK-I1 to CHK-I70 (canonical copy is in the session scratchpad) |
| `red/red-run.txt` | T1 red run against the pre-round clone (section "Pre-round snapshot", the evidence of record) and an informational live run |
| `green/` | V1 run (checks, per-document links check, identity, redaction, scope), taken before the last gate run |
| `raw/` | gate-runner logs: redaction exits and control, link check script and output, identity, diffstat, final live check output |
| `REPORT.md`, `inventory.txt` | gate report and read-only ctest inventory (1439 names, no change) |

Not present: `probes/PROBES.md` and `red/RED.md`. No mutation prober ran for this round, so no kill result exists.
The green and final runs of this folder belong to the R15-A1 candidate before the corrective round C01 and are not all-pass: 13 of 70 checks failed there (round record, sections (g) and (j)). C01 repaired the checker and the text; the final candidate's results are in the round record's C01 sections and in `tests/native/v5-02-r15-a1-c01-evidence/`.
