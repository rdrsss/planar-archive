# V5-02-RA01-B wave 1 evidence

Round record: [V5-02-RA01-B-W1.md](../../../docs/vernacular/rounds/V5-02-RA01-B-W1.md) (section "Coordinator sync before review" holds the
wave 1 candidate identity at the 2026-10-30 sync, the wave 1 invariant table and acceptance table). The C01 corrective changed that candidate;
the final candidate, its 1344-test inventory and its probe results are in [V5-02-RA01-B-W1-C01.md](../../../docs/vernacular/rounds/V5-02-RA01-B-W1-C01.md)
and `tests/native/v5-02-ra01-b-w1-c01-evidence/`. "Final" below means the wave 1 candidate (gate run 6), not the lineage's final candidate (the C01 candidate was in turn superseded by the wave 2 candidate: see [V5-02-RA01-B-W2.md](../../../docs/vernacular/rounds/V5-02-RA01-B-W2.md)).

| path | content |
| --- | --- |
| `plan/DECISIONS.md`, `plan/INVARIANTS.md` | wave 1 plan, revision 3 (earlier revisions under `plan/superseded-1/` and `plan/superseded-2/`) |
| `red/RED.md`, `red/summary.txt`, `red/names.txt`, `red/raw/`, `red/snapshot*/` | red-first run against the stub-era candidate (`snapshot-2`) |
| `probes/results.jsonl`, `probes/raw/` | mutation probes of the wave 1 candidate (run in progress when the record was synced; no `PROBES.md` was written here; the completed probes are `tests/native/v5-02-ra01-b-w1-c01-evidence/probes/PROBES.md`) |
| `probes/superseded-1/` | probes of the PRE-REPAIR candidate (combined sha256 `0f276a2b...2adaa837`); superseded, kept as raw evidence |
| `REPORT.md` | gate run 6 report (wave 1 candidate; the C01 gate report is `tests/native/v5-02-ra01-b-w1-c01-evidence/REPORT.md`); earlier gate runs are under `raw/run2/` to `raw/run5/` |
| `raw/` | raw gate logs, including `raw/run6/` and the Linux logs |
| `inventory.txt`, `new-names.txt`, `removed-names.txt` | run 6 test inventory (1339 names), 66 new names, 0 removed |

Raw logs are never deleted. Figures from `probes/superseded-1/` and `raw/run2/` to `raw/run5/` belong to their earlier candidates.

Not committed: `red/summary.txt` contains machine-local checkout paths and fails the redaction check; it stays on disk, unstaged (review RA01-B-W1-C01-R3).
