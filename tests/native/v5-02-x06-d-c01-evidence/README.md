# V5-02-X06-D-C01 evidence index

Round record: `docs/vernacular/rounds/V5-02-X06-D-C01.md` (its "C01 candidate record" is authoritative for the C01 candidate only; the final candidate and final figures of the lineage are in `docs/vernacular/rounds/V5-02-X06-D-C04.md` and `../v5-02-x06-d-c04-evidence/README.md`; the C03 candidate is in `docs/vernacular/rounds/V5-02-X06-D-C03.md`; the C02 candidate is in `docs/vernacular/rounds/V5-02-X06-D-C02.md`). Parent evidence:
`../v5-02-x06-d-evidence/`. This folder is gitignored except for this README; nothing here may be deleted.

C01 candidate (superseded by the C02 candidate, `../v5-02-x06-d-c02-evidence/`, by the C03 candidate, `../v5-02-x06-d-c03-evidence/`, and finally by the C04 candidate, `../v5-02-x06-d-c04-evidence/`): HEAD `a0a8b3fb7b038cba98e77f1f80e0008353279cf3`, uncommitted; `git diff HEAD -- src` sha256
`3b45bac8e00f835a1ceb2249d3beab166100e7e13da423fd3bfcf379c4b03e4a`; `git diff HEAD` (no pathspec) sha256
`1424dc80e9e44bcf357f6c930e8fe60896179728602cffc1db9d1ec70ff01e9f`. Gate run 3 and probe run 2 both ran on this candidate.

## Contents

| path | what it is | status |
|---|---|---|
| `REPORT.md` | gate runner report, run 3 (1098 of 1098, Debug and RelWithDebInfo) | authoritative for gate results; see "Corrections" below |
| `inventory.txt` | sorted `ctest -N` names of the C01 candidate (1098) | final for the C01 candidate |
| `raw/r3-*` | gate run 3 logs and exit codes (`r3-g1-focused` is G1, run first), `r3-candidate*.sha256`, `r3-candidate.diff`, name lists, `r3-added.txt` (42 new names), `r3-removed.txt` (empty) | final |
| `raw/r2-*`, `raw/report-run2-superseded.md` | gate run 2 (passed, 1086 tests, candidate `ba700adc...` src) | superseded, kept |
| `raw/c01-*`, `raw/report-run1-superseded.md` | gate run 1 (failed: four runtime tests, clang-format, Doxygen) | superseded, kept |
| `red/RED.md`, `red/results.tsv`, `red/summary.tsv`, `red/tests.txt`, `red/raw/` | red check of the first test wave, one log per test | final for its scope (first wave only) |
| `red/snapshot/`, `red/snapshot-2/` | pre-fix snapshots (`snapshot-2` is the one RED.md cites; `candidate.diff` sha256 `dbec1665...`) | `snapshot` superseded, kept |
| `probes/PROBES.md` | mutation probes, run 2 (171 mutants, 123 killed, 33 survivors, 15 build failures) | final |
| `probes/raw/<id>.{log,diff,json}` | one record per mutant | final |
| `probes/baseline/` | unmutated build and ctest before and after run 2 (1091 of 1091 unit tests) | final |
| `probes/tools/` | mutant definitions (`mutants2.py` to `mutants10.py`) and the runner `run2.py` | final |
| `probes/superseded-1/` | probe run 1 (165 mutants, 105 killed, 45 survivors, pre-repair candidate) with its full-suite reruns | superseded, kept |

## Corrections to `REPORT.md` (written before probe run 2 and before the final record sync; the sync may not edit that file)

- It says the mutation probes ran against `ba700adc...` and that the twelve survivor-repair tests are unprobed. Probe run 2 probed the final
  candidate, including those twelve tests; all twelve claims reproduced except the T3 family (T3, T3c, T3d, T16 still survive).
- It says the invariant table is truncated after T3 and that the record's outcome is Pending. The record now has the complete table and an outcome.
  Rows T8 and T9 are still not written (as `REPORT.md` says).
- It says an evidence README index was not found. This file is that index.
- It says test #531 timing was not measured separately. The gate log shows the same test (renumbered #556, `X06 B3 C a file replaced at any allocation
  inside prepare_load is never published as verified`) at 15.29 s Debug and 4.51 s RelWithDebInfo.
- Its R4 row (not met) and R5 row (not checked) are updated in the record's acceptance table: R4 not met in part (T8, T9), R5 met.
- Its "section (a2)" statement that there is no probe evidence for the twelve tests is superseded by `probes/PROBES.md`.

## Hash conventions

Every hash in the round record names its pathspec. `r3-candidate.sha256` is `git diff` of tracked files without a pathspec, equal to `git diff HEAD`;
`r3-candidate-src.sha256` is `git diff HEAD -- src`. The unrestricted hash also covers `docs/vernacular/roadmap.md`, so it changes when the roadmap
is edited; the `src` hash does not.
