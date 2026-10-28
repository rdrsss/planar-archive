# V5-02-X06-D-C02 evidence index

Round record: `docs/vernacular/rounds/V5-02-X06-D-C02.md` (its "C02 candidate record (C02 coordinator sync)" is authoritative for the C02 candidate only; the final
candidate and final figures of the lineage are in `docs/vernacular/rounds/V5-02-X06-D-C04.md` and `../v5-02-x06-d-c04-evidence/README.md`; the C03 candidate is in `docs/vernacular/rounds/V5-02-X06-D-C03.md` and `../v5-02-x06-d-c03-evidence/README.md`). Earlier evidence: `../v5-02-x06-d-c01-evidence/` (C01 candidate) and `../v5-02-x06-d-evidence/` (X06-D round candidate). This folder is
gitignored except for this README; nothing here may be deleted.

C02 candidate (superseded by the C03 candidate, `../v5-02-x06-d-c03-evidence/`, and finally by the C04 candidate, `../v5-02-x06-d-c04-evidence/`): HEAD `c875e08acdddf7b255ee0ff4c9533a59c872f81c`, uncommitted. Pathspec `src`: sha256 of `git diff --binary HEAD -- src` (tracked only)
`f5961de92f66cfd686fb4f78dadd8355e7cfd98f466540edc10d714c8d4e1451`, combined with the untracked `src/engine/budget/budget.cppm`
`c45946d4f117610e31f7af1509698aa073ebbcab8b58af7f3e858aa72c68c2c9`. Pathspec `src docs/architecture.md docs/vernacular/profile-preparation-architecture.md`:
`ae2edabaf70beeb4d25b68304a6a98d31505889b3396e9406ec7aee59e8bb18e` (tracked), `401c718b7f4e02f21b25e3b034b60776ef5ad32da9d0416d5e4b5f22fd95706e` (combined).
Gate run 3 and probe run 2 both ran on exactly these bytes (per-file hashes equal).

| path | what it is | status |
|---|---|---|
| `REPORT.md` | gate runner report, run 3 (1110 of 1110, Debug and RelWithDebInfo) | authoritative for gate results; see "Corrections" |
| `REPORT-pre-repair.md`, `REPORT-run2.md` | gate runs 1 (failed) and 2 (passed before the I1b, T9b and L7c repair) | superseded, kept |
| `inventory.txt` | sorted `ctest -N` names of the C02 candidate (1110) | final for the C02 candidate; C03 inventory has 1114 |
| `inventory-run2.txt` | run 2 names (1109) | superseded, kept |
| `margins.md` | R5 margins, measured on a PRE-raise snapshot, Debug only | not a post-raise table; R5 not met |
| `raw/run3-*` | gate run 3 logs, `.exit` files, identity before and after | final |
| `raw/` files without the `run3-` prefix (and `raw/pre-repair/`) | gate runs 1 and 2 | superseded, kept |
| `red/RED.md`, `red/raw/`, `red/snapshot/`, `red/snapshot-2/`, `red/p3-red.txt` | red check (snapshot-2 is the one `RED.md` cites) | final for its scope |
| `probes/PROBES.md`, `probes/raw/`, `probes/baseline/`, `probes/tools/` | probe run 2 (23 re-run rows, 14 killed, survivors X11, X2, X3, X4, T8) | final |
| `probes/superseded-1/` | probe run 1 | superseded for the rows run 2 re-ran; carried rows still cite it |

## Corrections to `REPORT.md` (written before probe run 2 and before the final record sync; the sync may not edit that file)

- Its Limitations say mutation probes were not run and a prober run on this candidate is still required. Probe run 2 (13:10) ran on the same bytes as gate run 3
  (12:31); it is that run. It also says the table row for I1b "has not been re-probed": it was (X11 survives, X12 and X13 kill it by name).
- Its section (a) says the round record carries no invariant table. The record now has one (rows I1 to I14 as delivered, I15 to I18 and I23 from `RED.md` and `PROBES.md`;
  I19 to I22 and I24 to I28 are not recoverable).
- Its statements that inventory rows, R6 and R7 record edits are "not verified by the gate runner" are resolved in the round record's acceptance table
  (R6 met, R7 met, inventory rows 7a to 7c written; R5 not met).

## Hash conventions

Every hash in the round record names its pathspec. Record and README edits are outside the `src` pathspec and do not change it; an unrestricted `git diff HEAD`
also covers the roadmap and changes with every record edit.
