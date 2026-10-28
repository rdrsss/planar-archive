# V5-02-X06-D-C04 evidence index

Round record: `docs/vernacular/rounds/V5-02-X06-D-C04.md` (its "Final candidate record (C04 coordinator sync)" is authoritative and is the final record of the lineage
X06-D, C01, C02, C03, C04; its "Margin record" section is the measurement). Earlier evidence: `../v5-02-x06-d-c03-evidence/` (C03 candidate), `../v5-02-x06-d-c02-evidence/`
(C02 candidate), `../v5-02-x06-d-c01-evidence/` (C01 candidate) and `../v5-02-x06-d-evidence/` (X06-D round candidate). This folder is gitignored except for this README;
nothing here may be deleted.

Final candidate: HEAD `c875e08acdddf7b255ee0ff4c9533a59c872f81c`, uncommitted. Pathspec `src`: sha256 of `git diff --binary HEAD -- src` (tracked only)
`ca2d1ba5b1d714914d326f0b0f1c5c40918526cc27d57d2cb71b637c2e4853dd`, combined with the untracked `src/engine/budget/budget.cppm`
`d8a2cab0b7a2a699158189aa1592c9c134d14b862de7e59dbd0968a7ee548aca`. Pathspec `src docs/architecture.md docs/vernacular/profile-preparation-architecture.md docs/vernacular/orchestration.md`:
`55dee86ae40e8c83433dd666cc9d0293e4facecfc4d5b665beedac69e2224cdb` (tracked), `81dc303acdf5cc8f87c880d0e538f86cfdf183e3267860f309785c9977570f47` (combined).
The pass 2 measurement, gate run 2 and probe run 2 all ran on exactly these `src` bytes (per-file hashes equal; the full per-file table is in the round record).
Sources changed since the C03 candidate: `src/engine/CMakeLists.txt` (`4f263d74...`), `src/engine/runtime/runtime.t.cpp` (`1d7ecf32...`),
`src/engine/runtime/external_registration.t.cpp` (`281ed935...`).

| path | what it is | status |
|---|---|---|
| `REPORT.md` | gate runner report, run 2 (1114 of 1114, Debug and RelWithDebInfo; 165 X06, 36 budget) | authoritative for gate results; see "Corrections" |
| `REPORT-run1.md` | gate run 1 (same outcome; its first full build ran a dyndep step) | superseded by run 2, kept |
| `inventory.txt` | sorted `ctest -N` names of the final candidate (1114, identical to the C03 names) | final |
| `plan/INVARIANTS.md`, `plan/DECISIONS.md` | the search plan (revision 2): invariant table and decisions D1 to D33; `superseded-1/` holds revision 1 | final plan; both are copied in full into the round record, the table with outcomes |
| `sizing/SIZING.md` | measured pass 2 on the final candidate (Margin record in the round record is a shell copy) | final; supersedes `../v5-02-x06-d-c03-evidence/sizing/SIZING.md` |
| `sizing/pass1/` | pass 1: items a to h, margins of the shipped, raised and trial ceilings, `RECOMMENDATION.md` that chose the joint ceilings; `raw/` with superseded runs | final for pass 1 |
| `sizing/pass2/` | pass 2: `check_margins.py` and its outputs (exit 0 in both configurations), identity before and after, margins tables, `raw/` | final |
| `probes/PROBES.md` | probe run 2 (8 re-run rows plus text rows), carrying run 1 | final |
| `probes/superseded-1/` | probe run 1 (55 raw records, the C4-5r red check, row notes) | superseded for the rows run 2 re-ran; carried rows and the red check still cite it |
| `probes/raw/`, `probes/baseline/`, `probes/tools/` | run 2 records, unmutated baselines, mutant definitions and runners | final |
| `raw/run2/` | gate run 2 logs, `.exit` files, identity, name lists | final |
| `raw/` files outside `run2/` | gate run 1 | superseded, kept |

There is no `red/` folder: no test-first wave ran (`testFirst` false). The one behavioural red check (C4-5r) is in `probes/superseded-1/PROBES.md` and its raw files.

## Corrections to `REPORT.md` (written before the final record sync; the sync may not edit that file)

- Its table and Limitations say line numbers marked "per run 1" were not re-read, and that diagnostics were not re-diffed: the round record's invariant table row C4-15 records the
  hash comparison (only three `src` files differ from C03, none is product source) re-done at the sync.
- It says the equality pins `budget_instruction_max - pass.instructions == 100` and `% 100` stay as ceiling-derived equalities and cites `runtime.t.cpp:3892, 4015`
  "as recorded in run 1, not re-read now"; the sync did not re-read them either. The facts the record relies on are the derived bound at `runtime.t.cpp:3859` and the kept
  registration pin at `:3863`.
- Its Limitations say the margin measurement and mutation probes were not re-run by the gate; they are recorded in `sizing/` and `probes/` and summarised in the round record.

## Hash conventions

Every hash in the round record names its pathspec. Record and README edits are outside the `src` pathspec and do not change it. An unrestricted `git diff HEAD` (tracked,
`61c4688c...`) also covers `docs/vernacular/roadmap.md` and changes when the roadmap is edited; the untracked round records and the gitignored evidence are not in it.
