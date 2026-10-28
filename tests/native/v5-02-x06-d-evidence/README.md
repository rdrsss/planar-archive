# V5-02-X06-D evidence index

Round record: `docs/vernacular/rounds/V5-02-X06-D.md`; review: `docs/vernacular/rounds/V5-02-X06-D-review.md` (request-changes, findings R1 to R5);
corrective round: `docs/vernacular/rounds/V5-02-X06-D-C01.md` with evidence in `../v5-02-x06-d-c01-evidence/`. This folder is gitignored except for
this README; nothing here may be deleted.

Everything here describes the X06-D round candidate, `git diff HEAD -- src` sha256
`125785336bdb7891d0600149f28728f4dd484cf7dd0aedf4e73c60eeadf18ca6` (4 files, 1056 tests), at HEAD `a0a8b3fb7b038cba98e77f1f80e0008353279cf3`. That candidate
was reviewed and is no longer the working candidate; the final figures of the lineage are in the C04 evidence (`../v5-02-x06-d-c04-evidence/README.md`); C03 candidate figures are in `../v5-02-x06-d-c03-evidence/README.md`; C02 candidate figures are in the C02 evidence (`../v5-02-x06-d-c02-evidence/README.md`) and C01 candidate figures in the C01 evidence.

## Contents

| path | what it is | status |
|---|---|---|
| `REPORT.md` | gate runner report, run 3 (1056 of 1056, Debug and RelWithDebInfo) | valid for the X06-D round candidate; see "Corrections" |
| `inventory.txt` | sorted `ctest -N` names of the X06-D round candidate (1056) | superseded by the C01 inventory (1098) |
| `raw/g*`, `raw/r2-*`, `raw/r3-*` | gate run 1, 2 and 3 logs and exit codes; `raw/*-superseded.md` hold the run 1 and run 2 reports | run 3 is the X06-D gate; earlier runs superseded, kept |
| `raw/red/` | the coder's first-run focused logs (`p1-*.log` with `.rc`) | kept |
| `red/RED.md`, `red/raw/`, `red/snapshot/` | red check (one expectRed test, 16 guards) and the tracked-only red snapshot | final for the X06-D round |
| `probes/PROBES.md`, `probes/raw/`, `probes/mutants.py`, `probes/run.py` | mutation probes, generation 2 (73 result rows, 144 selected tests, 17 survivors) | X06-D round's last; C01 probe run 2 supersedes the dispositions |
| `probes/superseded-1/` | generation 1 (wrong selection and build target) | superseded, kept |
| `review/` | the independent review's evidence: identity, red-snapshot-versus-final diff, added and removed names against the C04 baseline, focused X06 run (107 of 107), `accounting-owner-audit.txt` (the greps behind R1 and R2) | final |

## Corrections to `REPORT.md` (the C01 sync may not edit that file)

- It says the survivors "I4b to I4h" are limits. In probe generation 2, I4b, I4c, I4g and I4h are KILLED by the two capture-module tests
  (`profile capture module ceiling checked before observed module read`, `profile capture aggregate remaining module budget precedes final read`); only I4d,
  I4e and I4f were survivors. In C01 probe run 2, I4d is also killed (by an existing loader test); I4e and I4f remain masked by the capture layer.
- It says the probes ran on a filter that excludes capture and profile tests. That was generation 1; generation 2 added them (144 tests).
  C01 probe run 2 ran the full unit suite per mutant.
- It says G1 and the focused runs were not separate; C01 performed G1 on its own candidate (149 of 149) before its full runs.
- Its candidate hash and test count (1056) are those of the X06-D round candidate, not of the C01 candidate (`3b45bac8...`, 1098) nor of the C02, C03 or final C04 candidate.
