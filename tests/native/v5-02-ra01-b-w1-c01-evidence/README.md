# V5-02-RA01-B-W1-C01 evidence

Round record: [V5-02-RA01-B-W1-C01.md](../../../docs/vernacular/rounds/V5-02-RA01-B-W1-C01.md); section "C01 results (final candidate)" holds the
final candidate identity, the complete invariant table (with red-check and probe outcomes) and the acceptance table. The corrective builds on the wave 1
candidate described in [V5-02-RA01-B-W1.md](../../../docs/vernacular/rounds/V5-02-RA01-B-W1.md) (evidence in `tests/native/v5-02-ra01-b-w1-evidence/`).

Final candidate identity (identity.py, pathspec `src tests/native`): HEAD `307026288615e317d145ba6846c0692929c50a17`, tracked-only diff sha256
`eb11d7a210bfb7036f08ed98b900ef3953fffd6e11c5e4e4b399f0f4602d48eb`, combined sha256 `de65ea1fe7bab915d77bf3e9329b9e2a226f64ac04e3ddad5bdd409e2dabb3d5`.
The gate (run 4) and the probe pass (`probes/PROBES.md`) both ran on exactly this identity.

| path | content |
| --- | --- |
| `plan/DECISIONS.md`, `plan/INVARIANTS.md` | C01 plan, revision 2 (revision 1 under `plan/superseded-1/`); `INVARIANTS.md` rows C-E17b and C-E17c say "still seals", which the test does not assert (it asserts equality with the `mine` control); the record's table uses the corrected wording |
| `red/RED.md`, `red/raw/`, `red/snapshot/` | red-first run against the pre-fix candidate (snapshot `candidate.diff` sha256 `7b880280948d74e70409b4b72e7a75dd319dfe45d05f99d564a308775d3da324`) |
| `probes/PROBES.md`, `probes/results.jsonl`, `probes/raw/` | focused re-probe of the final candidate (22 rows run, the rest carried forward with byte-identity proof) |
| `probes/superseded-1/` | the earlier full probe pass of the previous C01 tree (`authoring.t.cpp` `50e9333b...`, `external_registration.t.cpp` `23b7e4fa...`); not final, kept as raw evidence and as the source of the carried rows |
| `REPORT.md` | gate run 4 report (final candidate; its Limitations section about `PROBES.md` predating two test edits is superseded by `probes/PROBES.md`, which ran on the final identity); runs 1 to 3 are `raw/run1-REPORT.md` to `raw/run3-REPORT.md` and belong to earlier trees |
| `raw/run4/` | raw gate logs of run 4, including `linux/` |
| `inventory.txt`, `new-names.txt`, `removed-names.txt` | final inventory (1344 names), 5 new names against the wave 1 inventory, 0 removed |

Raw logs are never deleted. Figures from `raw/run1` to `raw/run3`, `probes/superseded-1/` and `plan/superseded-1/` belong to earlier candidates or plan revisions.
