# R14-E1 implementation evidence

`report.json` records the bounded candidate, commands, counts, initial oracle
corrections and limits. It is ready for fresh independent review, not approved.
`candidate/` freezes all twelve owned files, and `candidate-files.json` records
SHA256 content hashes. The candidate identity hashes the canonical compact file
map; the report separately gives the SHA256 of the formatted manifest file.

`before.json` and `preexisting-status.txt` record the shared checkout baseline.
`before-owned.json` separates preexisting owned digests from new files; exact
recoverable prior file copies live under `baseline/`. See `baseline-recovery.json`
for the explicit hash-only baseline limitations. `audit.py` verifies 4,836 protected
files unchanged, only permitted additions, every prior registered test retained,
and exact configured/reversible generated ledger variants.

`gate.log` and `full-LastTest.log` preserve the complete 312-case debug gate.
`command-final.log/.xml` cover all seven command unit cases plus final init CLI.
`init-cli.log` records 60 actual init processes; `cli-complete.log` records the
unchanged 247 render processes. Native publication faults remain in the complete
gate; no CLI publication faults are simulated. `vectors.log`, `record-vectors.log`,
`helper-lint.log` and `no-work.log` preserve the other verification results.

`evidence-sha256.json` hashes evidence files except itself. Candidate identity and
manifest SHA256 are distinct by recipe. This round claims macOS debug coverage;
Linux and the integrated R14 release gate remain outside this slice.
