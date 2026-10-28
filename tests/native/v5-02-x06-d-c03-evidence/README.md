# V5-02-X06-D-C03 evidence index

Round record: `docs/vernacular/rounds/V5-02-X06-D-C03.md` (its "C03 candidate record (C03 coordinator sync)" is authoritative for the C03 candidate only; the final record of the lineage
X06-D, C01, C02, C03, C04 is the "Final candidate record (C04 coordinator sync)" of `docs/vernacular/rounds/V5-02-X06-D-C04.md`, evidence in `../v5-02-x06-d-c04-evidence/README.md`). Earlier evidence: `../v5-02-x06-d-c02-evidence/` (C02 candidate), `../v5-02-x06-d-c01-evidence/` (C01 candidate) and
`../v5-02-x06-d-evidence/` (X06-D round candidate). This folder is gitignored except for this README; nothing here may be deleted.

C03 candidate (superseded by the C04 candidate, `../v5-02-x06-d-c04-evidence/`): HEAD `c875e08acdddf7b255ee0ff4c9533a59c872f81c`, uncommitted. Pathspec `src`: sha256 of `git diff --binary HEAD -- src` (tracked only)
`c740bd2bac8bd94eb5eb216c10a46f7c59a63b446a5e137441928879582ad276`, combined with the untracked `src/engine/budget/budget.cppm`
`81972b34782baf628f2a63029ac5987a0605e92946246a5a138995456b485d49`. Pathspec `src docs/architecture.md docs/vernacular/profile-preparation-architecture.md docs/vernacular/orchestration.md`:
`76092574c5795d16b91f04086938556c4ac8d52be85478d6d116cd8621a53f25` (tracked), `4a4f1ecb16a0dfad68fb566f73fc5a2f5ac4bea30bbdb122b07c8ef547f76a53` (combined).
Gate run 3 and probe run 2 both ran on exactly these bytes (per-file hashes equal; the full per-file table is in the round record).

| path | what it is | status |
|---|---|---|
| `REPORT.md` | gate runner report, run 3 (1114 of 1114, Debug and RelWithDebInfo) | authoritative for gate results; see "Corrections" |
| `REPORT-run1.md`, `REPORT-run2.md` | gate run 1 (failed lint) and run 2 (passed, before the quarter-free instruction pins) | superseded, kept |
| `inventory.txt` | sorted `ctest -N` names of the C03 candidate (1114; the C04 candidate has the same 1114 names) | final for the C03 candidate |
| `plan/INVARIANTS.md`, `plan/DECISIONS.md` | the search plan (revision 3): invariant table and decisions; `superseded-1/`, `superseded-2/` hold revisions 1 and 2 | final plan; the table is copied into the round record with outcomes |
| `sizing/SIZING.md` | P0 sizing evidence: states NOT MEASURED (planning values only) | NOT a margin table; C03 criterion R1 not met; superseded by `../v5-02-x06-d-c04-evidence/sizing/SIZING.md` (measured) |
| `red/RED.md`, `red/raw/`, `red/snapshot/` | red check (two expectRed tests, 21 unique guards) | final for its scope; the role test's red run used the N=40 version |
| `probes/PROBES.md`, `probes/raw/`, `probes/baseline/`, `probes/tools/` | probe run 2 (14 re-run rows after the repair) | final |
| `probes/superseded-1/` | probe run 1 (54 rows plus exploratory) | superseded for the rows run 2 re-ran; carried rows still cite it |
| `raw/run3/*` | gate run 3 logs, `.exit` files, identity, name lists | final |
| `raw/run2/`, `raw/` files without the `run3/` prefix | gate runs 1 and 2 | superseded, kept |

## Corrections to `REPORT.md` (written before the final record sync; the sync may not edit that file)

- Its section (b) says the red-first rows of the two direct shape tests cannot be identified; this stays true (RED.md does not map `t3` to `t23` to names).
- Its section (a) says the round record copy of the invariant table and decisions was "the coordinator's step; not verified here": the record now carries the full table
  with red and probe outcomes and links `plan/DECISIONS.md`. Its row for the R5 documents ("text not verified here") is resolved in the record's acceptance table (met, checked by grep).
- Its statement that the R1 margin table "was NOT produced" is accurate and remains true: no margin table exists anywhere in this folder (`sizing/SIZING.md` says NOT MEASURED). The C04 round produced it: `../v5-02-x06-d-c04-evidence/sizing/SIZING.md`.
- Its Limitations say mutation probes are a later stage: probe run 2 is that stage (same bytes).

## Hash conventions

Every hash in the round record names its pathspec. Record and README edits are outside both recorded pathspecs and do not change them; an unrestricted `git diff HEAD` also covers
the roadmap and the tracked records and changes with every edit.
