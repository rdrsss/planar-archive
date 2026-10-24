# R14-C2 independent reviewer evidence

Verdict and source assessment: `docs/vernacular/rounds/R14-C2-review.md`.
`check.py` independently validates the exact candidate, coder evidence manifest,
protected files, original-test prefix, raw counts and generation outputs without
rewriting coder evidence. `run.py` runs the serialized debug checks. `probe.py`
compiles/links/runs the independent `probe.cpp` against current candidate capture
artifacts; it interposes fdopendir and scalar allocation for deterministic cleanup
probes. `final-checks.py` runs vectors/helper lint/final audit and validates counts.
Run those scripts in that order, with `check.py` first. Commands and raw outputs
remain recorded separately. `probe-artifacts.json` records build artifact hashes;
the compiled probe and object were removed after the successful run.

`evidence.json` freezes all retained reviewer files except itself, and records the exact
report hash together with the candidate identity and coder manifest file hash.
This folder is reviewer evidence, not product implementation or approval of the
future acceptance writer. macOS arm64 debug only; no Linux/release claim.
