# V5-02-RA01-B-W2 evidence

Round record: [V5-02-RA01-B-W2.md](../../../docs/vernacular/rounds/V5-02-RA01-B-W2.md); its section "Results (W2 candidate, pre-C01)" holds the W2 candidate identity,
the complete invariant table (with red-check and probe outcomes), the acceptance table, the repairs, the pin and the remaining limits. This was the final-figure
record of the RA01-B lineage until the C01 corrective (the C01 candidate figures are in the [C01 record](../../../docs/vernacular/rounds/V5-02-RA01-B-W2-C01.md), section "C01 candidate record (coordinator record sync, 2026-10-31, before review)"; its evidence README is [here](../v5-02-ra01-b-w2-c01-evidence/README.md); the final figures and the current identity are in the [C02 record](../../../docs/vernacular/rounds/V5-02-RA01-B-W2-C02.md), evidence [here](../v5-02-ra01-b-w2-c02-evidence/README.md)); earlier candidates: [wave 1](../../../docs/vernacular/rounds/V5-02-RA01-B-W1.md) and [C01](../../../docs/vernacular/rounds/V5-02-RA01-B-W1-C01.md)
describe earlier candidates (1344 tests, pin 227525 and 227551).

W2 candidate (pre-C01; superseded by the C01 candidate, 1437 tests, combined `1f30a26e...`): HEAD `3343ad38304db57388bc7ac2438a40043eba1b7d`; per-file hashes and the `src`-scoped diff hash are in the round record. The gate (third run) and the
probe pass ran on exactly this candidate (per-file tables equal).

| path | content |
| --- | --- |
| `plan/DECISIONS.md`, `plan/INVARIANTS.md` | plan revision 2 (D-18 option A); revision 1 under `plan/superseded-1/` |
| `red/RED.md`, `red/rows.tsv`, `red/raw/`, `red/snapshot/` | red-first run against the wave 1 stubs (69 reds, 10 guards); predates the probe repair |
| `raw/run3/`, `REPORT.md` | third gate run (W2 candidate): lint, builds, ctest 1416 of 1416 in both macOS configurations, no-work builds, redaction, Linux Debug and RelWithDebInfo |
| `raw/run2/` (`REPORT-superseded-gate1.md`, `REPORT-superseded-gate2.md`), top-level `raw/` | earlier gate runs on earlier trees; superseded, kept |
| `probes/PROBES.md`, `probes/results.jsonl`, `probes/raw/` | focused re-probe of the W2 candidate (14 mutants) after the test-only probe repair. Its first-pass tables for W2-31, X09, X19, X20 and the "Extra exploratory mutants" section still show those rows as survivors; the re-run table at the top supersedes them C01 pointer (D-C01-39): the X03, X04, X05, X07, X16, X17 and W2-51 survivor text above is closed by killing tests added in C01 (`tests/native/v5-02-ra01-b-w2-c01-evidence/probes/PROBES.md`, result in C01 PROBES.md). |
| `probes/superseded-1/` | first full probe pass (127 mutants) on the tree before the probe repair; product bytes identical to the W2 candidate, differs in two test files and `path-redaction.json`; source of the carried rows |
| `c02-pin.md` | boundary pin measurement (macOS 227275, Linux 227301). Two statements are stale: the `orchestrator.lua` hash (not production-embedded, pin unaffected) and the open `planar_hash` blocker (fixed by the gate repair) C01 pointer: the `orchestrator.lua` hash and the `planar_hash` blocker are corrected in `c02-pin.md` (D-C01-20). |
| `inventory.txt`, `new-names.txt`, `removed-names.txt` | 1416 names, 72 added against the 1344 of 3343ad38, 0 removed |

Raw logs are never deleted. Figures from `raw/run2` (gate runs 1 and 2), `probes/superseded-1/` and `plan/superseded-1/` belong to earlier trees or plan revisions. Raw logs, snapshots
and `red/rows.tsv` follow the redaction rules: only files that pass `scripts/redact-local-paths.py check` may be staged.
