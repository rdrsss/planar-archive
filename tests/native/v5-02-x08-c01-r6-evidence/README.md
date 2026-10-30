# V5-02-X08-C01-R6 evidence

Final gate and probe evidence for the X08 with C01 candidate after the R6 test-only repair (2026-10-29).

- `REPORT.md`: gate report of run 2 (fresh full re-run at the same identity as run 1). Run 1's report is `raw/REPORT-previous-gate-run1.md`.
- `raw/run2/`: macOS Debug and RelWithDebInfo ctest logs (1260 of 1260), lint, no-op builds, redaction, identity, validator log and exit; `raw/linux/` and
  `raw/run2/linux/`: Linux gate (Debug and RelWithDebInfo, 1260 of 1260). Run 1 logs sit directly in `raw/`.
- `probes/PROBES.md`: focused re-probe (run 2) with carried rows; `probes/superseded-1/` holds the earlier probe run, `probes/tools/` the drivers.
- `inventory.txt`, `inventory-sorted.txt`: the 1260-name inventory.

Candidate identity is recorded in `raw/run2/identity.txt` and in the round record `docs/vernacular/rounds/V5-02-X08.md` ("Final candidate: V5-02-X08-C01-R7"; the earlier "R6 candidate" section there is superseded).
Superseded limit (R6 stage): `validate.py` then exited 0 only over the older 1258-name tree. C01-R7 rebuilt `../v5-02-x08-evidence/` from `raw/run2/` here (1260 names); the validator covers it, see `../v5-02-x08-c01-r7-evidence/`.
The R6 `I16b` equivalent-survivor verdict in `REPORT.md` and `probes/PROBES.md` is superseded: the R7 prober killed the mutant (`../v5-02-x08-c01-r7-evidence/probes/superseded-1/PROBES.md`).
