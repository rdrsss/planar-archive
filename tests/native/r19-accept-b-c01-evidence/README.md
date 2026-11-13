# R19-ACCEPT-B-C01 evidence folder

Evidence for the A03 checkpoint acceptance and the C01 probes. The round record is `docs/vernacular/rounds/R19-ACCEPT-B.md`. Placeholders: `$PLANAR_CANDIDATE` the recorded candidate binary, `<scratch-root>` a scratch directory, `<repo>` the repository root.

## Committed files

Each at most 100 KB, added with `git add -f`:

- [REPORT.md](REPORT.md): the single evidence report.
- [a03-readback.json](a03-readback.json): the path-free A03 readback manifest.
- [plan/DECISIONS.md](plan/DECISIONS.md) and [plan/INVARIANTS.md](plan/INVARIANTS.md): the planning records of the C01 round.
- [probes/PROBES.md](probes/PROBES.md): the mutation probe record.
- [red/RED.md](red/RED.md): the C01 red record file of this folder.
- This README. The B evidence folder commits the candidate record, the A34 manifest, the red records and the commit list.

## Local files

These stay on this machine and are kept out of git: `raw/` (run and gate logs, including the raw COMMIT-LIST copies), `red/raw`, `red/snapshot`, `probes/raw`, `probes/superseded-1`, `plan/superseded-N`, `inventory.txt`, `outcomes.tsv` and the C02 evidence folder `tests/native/r19-accept-b-c02-evidence`.

## Reproduce the A03 run

Fresh scratch root distinct from the A34 and suites roots, scratch `HOME` and `PLANAR_DB`:

    mkdir <scratch-root> && mkdir <scratch-root>/home
    cd <repo>/tests/native/r19-accept-support
    HOME=<scratch-root>/home PLANAR_DB=<scratch-root>/planar.db PLANAR_CANDIDATE=$PLANAR_CANDIDATE \
      python3 -B run_acceptance.py checkpoint --candidate $PLANAR_CANDIDATE \
      --record <repo>/tests/native/r19-accept-b-evidence/candidate/candidate.json --repo-path <repo> \
      --raw-root <repo>/tests/native/r19-accept-b-c01-evidence/raw/c02-a03 --run-name a03-checkpoint

The raw root `raw/c02-a03` holds `env.txt`, `run.log`, `a03.exit`, `DONE` and the run directory `a03-checkpoint` (with `result.json` and the readback).

## Superseded drafts

The four COMMIT-LIST copies under `tests/native/r19-accept-b-c01-evidence/raw` (`raw/prelim-1/COMMIT-LIST-draft.txt`, `raw/prelim-1/COMMIT-LIST-final.txt`, `raw/prelim-2/COMMIT-LIST-final.txt` and `raw/final/COMMIT-LIST.txt`) are superseded drafts. They are left in place and unedited; the only commit list is `tests/native/r19-accept-b-evidence/COMMIT-LIST.txt`.
