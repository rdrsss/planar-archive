# V5-02-A exact contract candidate evidence

Docs-only coder candidate; not self-approved. Coordinator owns the current round
record. No product code, builds, product tests, installed binaries, Planar tools,
operator data or vendor changes. Runtime guarantees below are implementation
acceptance requirements, not measurements from this round.

Eight owned documents are frozen in `candidate/`, with their pre-edit copies in
`baseline/` (command-contract.md is new). `owned.diff` compares exactly these
snapshots, independent of the checkout's pre-existing intentional rewrite changes.
`baseline.json` records HEAD/branch/ownership/AGENTS symlink and `status-before.txt`
records pre-edit working state. Starting HEAD is
3171c49e7b92d3abda01bfbb8bfb0c6b29d4c67a.

The first protected inventory covered 2,025 source/vendor/historical/proposal
files. It was expanded to all tests, fixtures, cmake and scripts before the first
document edit, producing 5,601 entries. `protected-before.json` pins all those
files, including prior evidence and every historical round except coordinator-
owned V5-02-A.md. The unchanged user v5 and frozen V5-02 proposal are included.
No prior evidence was overwritten. Current AGENTS.md remains a symlink to CLAUDE.md.

Executed verification:

- `python3 tests/native/v5-02-a-evidence/verify.py`: pass; 5,601 protected entries
  unchanged, 175 local links/anchors resolving, all eight owned snapshots hashed.
- `git diff --check --` followed by the eight owned paths: pass, no whitespace
  errors. The verifier also executes and checks this command.
- No build/test counts are claimed; this is the authorized documentation round.

`verification.json` contains the exact hashes and results; `links.json` lists every
checked reference. `manifest.json` hashes every evidence file except itself.
The audit script is a round-local reproducibility artifact, not product tooling.
Before independent review, coordinator should verify all manifest entries and
that current owned document hashes still equal `verification.json`/`candidate/`.

The contract pins actual retained-VM Lua semantic dispatch and finite native
services, fixed CLI11 signatures/error precedence, decimal-string database IDs,
export postpublication failure behavior, workspace naming/lexical intent versus
physical render, and provenance without changing request_v1/W codecs. Named
implementation slices are B1 runtime/registry, B2 CLI family composition, C naming,
remaining R14, required external X, then RA01/execution. External v2 codec design
is explicitly a later reviewed gate. No unresolved new operator choice is raised.
