# R20-CARRY-A-C01 evidence folder

Index of `tests/native/r20-carry-a-c01-evidence/` (C), the evidence of the R20-CARRY-A-C01 corrective round: the timeout limits 1680 and 840, the per-test guard minima, and the records. The folder is gitignored. Committed files are added with `git add -f` by explicit path, under the commit set of D27 in `plan/DECISIONS.md`.

Read `REPORT.md` first. It states which results were obtained and which are "not run".

| path | holds | committed |
|---|---|---|
| `README.md` | this index | yes |
| `REPORT.md` | the C01 gate and evidence report (row C22): identity, gate, counts, timings and load, red-first, probes, redaction status | yes |
| `plan/DECISIONS.md`, `plan/INVARIANTS.md` | the revision 4 plan: decisions D1 to D34 and invariant rows C1 to C22 (24 rows) | yes |
| `plan/superseded-1/`, `plan/superseded-2/`, `plan/superseded-3/` | earlier plan revisions | no (local) |
| `red/RED.md` | red-first record on the 120-limit tree (row C17) | yes |
| `red/raw/*.log` | per-test red-first logs | no |
| `red/snapshot/` | code snapshot used for the red run (candidate diff, untracked copies, sha256 listing of code) | no |
| `probes/PROBES.md` | probe record (rows C16 and C18): mutant, killing test, failed count, scratch-copy code hash | yes, once written |
| `probes/` logs | prober logs and scratch-copy transcripts | no |
| `raw/` | every other C01 log: `timings-c01.txt`, `ctest-r20.txt`, `ctest-names-after.txt`, `json-v1-timeouts.txt`, `identity-*.txt`, `identity-i16-after.txt`, gate logs, `pkg-i1-brief.txt`, `pre-c01-drivers/` and `pre-c01-files/` snapshots, `redaction-check.txt` | no |
| `raw/commit-list.txt` | the commit list, regenerated from `git diff --cached --name-only` at the pre-commit check (D29) | no |

Rules that apply to every file here:

- Never delete or rewrite a raw log.
- Write the home directory as `<home>`. Never write the home directory itself (D9).
- Record hashes only of code (identity-tool output), never of records, REPORTs or the round record (D21).
- The earlier run1 evidence (the R20-CARRY-A candidate, limit 120) is in `tests/native/r20-carry-a-evidence/` and is indexed in its `README.md`.
