# V5-02-X07-C02 evidence

Round record: `docs/vernacular/rounds/V5-02-X07-C02.md` (its "Results" sections hold the final candidate identity, gate, red-first, probe and acceptance figures and the complete invariant table).

Final candidate: HEAD `f78f8eb47267ab9c72efacc07708e7c0c2669840`; `git diff HEAD -- src` sha256 `5d4a1139cbb22809e7faa34780c85887735db0ed4f274a07cf13a1f7aae46c71`; identity.py `src tests/acceptance` sha256 `b4a0bf1878d52940a43d27b5b37216cf71aedd51d33324055d8a6ac3d38d590e`.

| path | what it is | status |
|---|---|---|
| `REPORT.md`, `raw/gate2/` | gate run 2 (1273 tests, macOS Debug and RelWithDebInfo, Linux Debug and RelWithDebInfo) | final. Its limitation bullet that says the E1 and E7 repair tests were not re-probed is out of date; `probes/PROBES.md` records both killed |
| `superseded-gate1/`, `raw/0*` to `raw/09*` (gate-1 logs) | gate run 1 (1271 tests) | superseded, kept |
| `inventory.txt`, `new-names.txt`, `removed-names.txt` | 1273 names, 13 new, 0 removed | final |
| `plan/INVARIANTS.md`, `plan/DECISIONS.md` | plan revision 4 | final; `plan/superseded-1/` to `superseded-3/` are earlier revisions |
| `probes/PROBES.md`, `probes/raw/` | focused re-probe on the final candidate | final; `probes/superseded-1/` is the first pass (E1, E5, E7 survivors belong to it) |
| `red/RED.md`, `red/raw/`, `red/snapshot/` | red check against the test-package plus stub snapshot | final red evidence; identity of that snapshot is not the candidate identity |
