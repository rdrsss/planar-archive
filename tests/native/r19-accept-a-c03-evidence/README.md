# R19-ACCEPT-A-C03 evidence

Round record: `docs/vernacular/rounds/R19-ACCEPT-A.md` (section "R19-ACCEPT-A-C03 result and record sync, FINAL candidate"); that section
holds the candidate identity, gate counts, red-first and probe results and the met/not-met tables. Plan: `plan/DECISIONS.md` and `plan/INVARIANTS.md` (revision 4).

Which evidence is final and which is superseded:

- `REPORT.md` and `raw/run3/` are the FINAL gate (Debug, Release, clean export, Linux, Python suites). `raw/run1/`, `raw/run2/`, `raw/prelim-1/`, `raw/prelim-2/`
  and `raw/run3/REPORT-prelim-3-run1.md` are earlier attempts, kept and superseded.
- `gate/` is the earlier Debug-only PKG-GATE, taken before the repair test `HeavyDriverTest.test_run_heavy_exits_nonzero_when_the_trajectory_is_present_but_invalid` was added.
  Its counts and `gate/test-names/after/py-worktree-c03.txt` (148 ids) are superseded by the final gate (149 Python tests, 1854 ctest names). Its `gate/raw-tree/comm-23.txt`,
  `gate/test-names/ctest-comm-23.txt`, `py-comm-23.txt` and `gate/linux/` static checks (all empty or as recorded) still stand.
- `probes/PROBES.md` is revision 2. Rows it marks as carried from revision 1 have their raw logs under `probes/superseded-1/raw/` (including `BASELINE.log` and `C3-F11-b.log`);
  `probes/raw/` holds only the 14 revision 2 mutants plus one malformed first attempt that is not a kill.
- `red/RED-1.md` and `red/snapshot/` are the first red check; `red/RED.md` and `red/snapshot-2/` the second. `red-t/`, `red-t2/`, `red-t3/` are the coders' own red logs.
- `stage0/` identity files were taken after the PKG-T edits; `stage0/NOTE.md` records the reconstructed pre-edit hashes.
- `runs/STOPPED-HARNESS-HEAVY.md` is the stop record of the heavy run (counter check NOT MET); `runs/cmake-rerun/index.json` was not written. Superseded by the closing heavy run (`closing/`, and `tests/native/r19-accept-evidence/runs/cmake-rerun/index.json`).
- `closing/` is the coordinator's closing fix of C03 (2026-11-12): red-first and probe logs (`closing/red/`, redacted, tree-local manifest `tests/native/path-redaction.json`; originals under `closing/redaction/original/`, local only), identity (`closing/identity/`), Python suites (`closing/suites/`, 152 run) and commit-list checks (`closing/list/`).
- `COMMIT-LIST.txt` is the single commit authority for R19-ACCEPT-A (written by the closing fix, trimmed at commit by the operator's evidence-size decision).
- `closing/LOCAL-ONLY.txt` records the counts and sha256 of the evidence kept local at commit (the `index/` trees and files over 100 KB).
- `gate/test-names/after/py-worktree-closing.txt` (152 ids) supersedes `py-worktree-c03.txt` (148).
