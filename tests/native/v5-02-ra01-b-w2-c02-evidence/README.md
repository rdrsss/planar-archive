# V5-02-RA01-B-W2-C02 evidence

Round record: [V5-02-RA01-B-W2-C02.md](../../../docs/vernacular/rounds/V5-02-RA01-B-W2-C02.md). Its sections "Candidate identity (final)", "Acceptance", "Complete invariant table" and "Remaining limits" describe the final candidate of the RA01-B wave 2 lineage (HEAD `3343ad38304db57388bc7ac2438a40043eba1b7d`, uncommitted; `src` pathspec combined `68097c45...`, `src tests/native` combined `d56c7fbb...`, 1437 tests, same names as the C01 candidate). The C01 candidate (combined `1f30a26e...`) is described in [V5-02-RA01-B-W2-C01.md](../../../docs/vernacular/rounds/V5-02-RA01-B-W2-C01.md) and its [evidence README](../v5-02-ra01-b-w2-c01-evidence/README.md); the W2 candidate in [V5-02-RA01-B-W2.md](../../../docs/vernacular/rounds/V5-02-RA01-B-W2.md).

Plan tables (authoritative together): [plan/DECISIONS.md](plan/DECISIONS.md) and [plan/INVARIANTS.md](plan/INVARIANTS.md); earlier revisions in `plan/superseded-1/` to `plan/superseded-3/`.

| path | content | stage and status |
| --- | --- | --- |
| `plan/` | decisions D-C02-01 to D-C02-27 and the invariant table C02-01 to C02-41 (revision 4) | plan stage; present |
| `base/`, `base/SHA256SUMS` | copies for `diff -U0`. Product files and `authoring-contract.md` are pre-C02 bytes; the five T1 test files are post-T1 bytes. `base/BASE.md` was never written | S0; present, with that deviation recorded in the round record |
| `red/RED.md`, `red/identity.txt`, `red/snapshot/`, `red/*.log` | red-first run of T37 and T44 against the pre-fix product (both failed on assertions) and three guards that passed | RF; present. `red/guard1.log` and `red/guard2.log` hold local paths: do not stage |
| `probes/PROBES.md`, `probes/results.jsonl`, `probes/raw/` | focused re-probe after the survivor repair, with every earlier row re-run on this candidate (193 mutants) | prober; present. `probes/superseded-1/` is the first pass, kept |
| `REPORT.md` | gate report, second run (passed) | gate; present |
| `raw/`, `raw/gate2/` | first gate run logs (`raw/01-*` to `raw/14-*`, `raw/linux/`, `raw/gate1-REPORT.md`) and the second run (`raw/gate2/`, including `identity-*.txt`, `redact-files-clean.txt` and the Linux container logs); nothing deleted | gate; present (there is no `gate/` folder) |
| `inventory.txt` | 1437 test names, identical to the C01 inventory | gate; present |
| `review/` | the reviewer's identity files, review ctest log and inventory check | reviewer stage; not started |

Raw logs are never deleted. Only files that pass `scripts/redact-local-paths.py check` may be staged.
