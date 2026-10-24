# R14-E2 implementation evidence

Candidate awaits fresh independent review. `report.json` records scope, commands,
counts, fixture provenance and limitations. `candidate/` freezes seventeen owned
members (two shared response files unchanged); `candidate.patch` is the exact
incremental diff from starting owned files. Candidate identity hashes the compact
sorted path-to-SHA256 map in `candidate-files.json`.

`before.json`, `before-owned.json`, `baseline/`, `baseline-inventory.json` and
`preexisting-status.txt` preserve the shared baseline. `audit.py` verifies 4,897
protected files, all 312 prior tests, only owned additions and exact reversible
configured storage variants. Coordinator planning files are excluded.

`gate.log` and `full-LastTest.log` retain 317 passing debug tests, zero skips,
81-source formatting/Doxygen and no-work evidence. `focused-candidate.log` and
`focused-final.xml` rerun the final strengthened oracle and four new unit cases:
seven registered cases, 115 history + 247 render + 60 init CLI processes.
`wire-probes.json`, paired stderr files and empty stdout files preserve actual
unsupported-interpretation and oversized-response refusals.

`read-test-link.txt` records a native query link excluding Lua/runtime/live-policy
wrappers. Vectors, helper formatting, protection and generated variant manifests
are separate raw evidence. `evidence-sha256.json` hashes everything except itself.
This is macOS debug coverage; release/Linux remain separate gates.
