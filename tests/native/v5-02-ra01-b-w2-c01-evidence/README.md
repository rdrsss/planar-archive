# V5-02-RA01-B-W2-C01 evidence

Round record: [V5-02-RA01-B-W2-C01.md](../../../docs/vernacular/rounds/V5-02-RA01-B-W2-C01.md). The dispatch
section there holds the objective, the operator decisions and the acceptance criteria. The section "C01 candidate record (coordinator record sync, 2026-10-31, before review)" at the end of that file describes the C01 candidate (identity `1f30a26e...` for pathspec `src tests/native`, 1437 tests, gate run 3, red-first, probes, repairs, delegated decision, acceptance table, complete invariant table, open items). Superseded by V5-02-RA01-B-W2-C02: the [C02 record](../../../docs/vernacular/rounds/V5-02-RA01-B-W2-C02.md) and its [evidence README](../v5-02-ra01-b-w2-c02-evidence/README.md) hold the current identity and the final figures. The section "Record of package I-R" (site table, pins, checks, limits) is the record of that package at its time; its identity block and "not yet run" statements are superseded.

Plan tables (authoritative together): [plan/DECISIONS.md](plan/DECISIONS.md) and [plan/INVARIANTS.md](plan/INVARIANTS.md).
Earlier plan revisions are kept under `plan/superseded-1/`, `plan/superseded-2/` and `plan/superseded-3/`.

| path | content | package and status |
| --- | --- | --- |
| `plan/DECISIONS.md`, `plan/INVARIANTS.md` | 40 decision rows (D-C01-01 to D-C01-40; D-C01-25 is the delegated residual post-gate decision), the invariant table (C01-01 to C01-45) and the R1 failure-site table | plan revision in force; superseded revisions kept; the round record adds C01-46, GR-01 and the exploratory rows |
| `red/RED.md`, `red/raw/` | red-first run of the 23 named C01 tests against the pre-fix snapshot: 11 expectRed failures and 12 guards (the 1 MiB prompt guard timed out under ctest), exact ctest commands, raw logs. The 21st added test (`observe maps a failed entry inspection after listing`) post-dates it and has no red-first record | R-0; kept unchanged |
| `red/snapshot/` | pre-fix copies of `capture.cpp`, `capture.cppm`, `directory.cpp`, `directory.cppm`, `prompts.lua`, `observe.lua`, `invocation.cpp` (sources only) | R-0; baseline of `c01-only.patch` |
| `red/snapshot-2/` | `candidate.diff` (tracked diff of `src` and `tests/native` at red time), the untracked files it covers, and `SHA256SUMS` | R-0; identity at red time |
| `c01-only.patch` | `diff -u` of `red/snapshot/` against the C01 product files (seven files, stable labels). Used for C01-17 literal review | I-R; 194 lines |
| `probes/PROBES.md`, `probes/results.jsonl`, `probes/raw/` | focused re-probe on the C01 candidate (combined `1f30a26e...`; the C02 re-probe is in the C02 evidence folder): E01, E02, E02b, E02c, E03, E05, E11 and the 10 rows whose test file changed (all killed except the fault-only E01, E02, E02b, E03, E05, E11, which survive as limits) | prober; present |
| `probes/superseded-1/` | first full probe pass (47 mutants plus build-clean reruns, candidate `afc66baa...`); source of the carried rows | prober; present, kept |
| `REPORT.md` (run 3, passed), `REPORT-run1.md` (failed, one Debug timeout), `REPORT-run2.md` (passed, 1436 tests), `raw/` (runs 1 to 3 and `raw/run2/`, `raw/run3/`, `raw/run3/linux/`) | gate reports and raw logs; nothing deleted | gate stage; present (there is no `gate/` folder) |
| `inventory.txt`, `new-names.txt`, `removed-names.txt` | 1437 names, 21 added against the 1416 of the W2 baseline, 0 removed; identical in macOS Debug and RelWithDebInfo and in both Linux configurations | gate stage; present |

Evidence outside this folder that C01 cites:

| path | content |
| --- | --- |
| `tests/native/v5-02-ra01-b-w2-evidence/c02-pin.md` | acceptance-5 pin values (macOS Debug and RelWithDebInfo measured by I-R; Linux not re-measured: the gate-3 container runs passed the pin test at the unchanged 227301, within the +/-8 window, and the C01 Linux rows of this file were filled by C02 from `raw/run3/linux`, with window wording), the R8 `orchestrator.lua` hash and the identity output |
| `tests/native/v5-02-ra01-b-w2-evidence/inventory.txt`, `new-names.txt`, `removed-names.txt` | the W2 baseline (1416 names), kept unchanged |
| `docs/vernacular/rounds/V5-02-RA01-B-W2-review.md` | the wave 2 review whose findings R1 to R8 this round closes |

Raw logs are never deleted. Logs and snapshots that carry local absolute paths follow the redaction rules: only files
that pass `scripts/redact-local-paths.py check` may be staged.
