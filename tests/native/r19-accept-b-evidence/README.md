# R19-ACCEPT-B evidence folder

Evidence for the A34 provenance acceptance (R19-ACCEPT-B) and the C01 and C02 rounds. The round record is `docs/vernacular/rounds/R19-ACCEPT-B.md`. Placeholders: `$PLANAR_CANDIDATE` the recorded candidate binary, `<scratch-root>` a scratch directory, `<repo>` the repository root.

## Committed files

Each at most 100 KB, added with `git add -f`:

- [REPORT.md](REPORT.md): the single evidence report.
- [a34-readback.json](a34-readback.json): the path-free A34 readback manifest.
- [candidate/candidate.json](candidate/candidate.json): the recorded candidate.
- [plan/DECISIONS.md](plan/DECISIONS.md) and [plan/INVARIANTS.md](plan/INVARIANTS.md): the planning records of the B round.
- [red/RED.md](red/RED.md) and [red-c01/RED-C01.md](red-c01/RED-C01.md): the red-first records.
- `COMMIT-LIST.txt`: the single commit list for B, C01 and C02.
- This README. The C01 folder commits its own report, README, `a03-readback.json`, `plan/DECISIONS.md`, `plan/INVARIANTS.md`, `probes/PROBES.md` and `red/RED.md`.

## Local files

These stay on this machine and are kept out of git: `raw/` (run, suite, identity and earlier-run logs, including the raw COMMIT-LIST copies), `snapshot` and red raw logs, `superseded-N` folders, `review/`, `identity/`, `pin-check/`, `green-c01/`, `inventory.txt`, and the whole C02 evidence folder `tests/native/r19-accept-b-c02-evidence`.

## Reproduce the A34 run

Fresh scratch root, scratch `HOME` and `PLANAR_DB`, the candidate from the recorded `PATH` file of the R19-ACCEPT-A gate:

    mkdir <scratch-root> && mkdir <scratch-root>/home
    cd <repo>/tests/native/r19-accept-support
    HOME=<scratch-root>/home PLANAR_DB=<scratch-root>/planar.db PLANAR_CANDIDATE=$PLANAR_CANDIDATE \
      python3 -B run_acceptance.py provenance --candidate $PLANAR_CANDIDATE \
      --record <repo>/tests/native/r19-accept-b-evidence/candidate/candidate.json --repo-path <repo> \
      --raw-root <repo>/tests/native/r19-accept-b-evidence/raw/c02-a34 --run-name a34-provenance

The raw root `raw/c02-a34` holds `env.txt`, `run.log`, `a34.exit`, `DONE` and the run directory `a34-provenance` (with `result.json` and the readback). The suites ran from a different scratch root with the environment recorded in `raw/c02-suites/env.txt`.

## Superseded drafts

The four COMMIT-LIST copies under `tests/native/r19-accept-b-c01-evidence/raw` (`raw/prelim-1/COMMIT-LIST-draft.txt`, `raw/prelim-1/COMMIT-LIST-final.txt`, `raw/prelim-2/COMMIT-LIST-final.txt` and `raw/final/COMMIT-LIST.txt`) are superseded drafts. They are left in place and unedited; the only commit list is `tests/native/r19-accept-b-evidence/COMMIT-LIST.txt`.
