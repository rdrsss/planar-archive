# X02-C01 author evidence

Docs-only correction candidate; independent review is still required. P1/P2 are
unselected. This evidence does not implement external loading or close its gate.

Baseline: shared `dev/planar-v2-vernacular` at
`3171c49e7b92d3abda01bfbb8bfb0c6b29d4c67a`. `baseline-head.txt`, complete
`baseline-status.txt` and `baseline-diff.txt` preserve the pre-existing intentional
rewrite. `baseline.json` freezes 14,740 regular files outside `.git`, `build`,
this new evidence root and symlinks. AGENTS.md is checked separately as a symlink
to CLAUDE.md. `baseline-owned/` holds exact pre-correction document bytes.

Owned candidate documents: external-profile-contract.md and
external-profile-implementation.md under docs/vernacular. No status-only files
needed changes. The coordinator owns the correction round. Original X02 frozen
candidate/review/evidence, exact current contracts, product/source/tests/CMake and
all other baseline regular files retain baseline bytes. `protection.json`
identifies the two changed files, zero additions outside this evidence root and
zero unexpected changes. `source-basis.json` pins the cited existing ID/omission
implementation and review; these are source bases, not new runtime proof.

Changes: distinct native capability-bound history_id descriptor and Lua string
ABI through INT64_MAX; phased initial overlap checks and retained/rechecked
receipts, with the consequence for pending P2; actual removal-order omitted IDs
and notice, original section-order metadata, exact multiple-omission bytes.
X04/X05-C/X07 and E02/E03/E08/E09/E13 carry corresponding future test obligations.
No new product choice was introduced.

Verification executed: baseline capture using Python hashlib/pathlib/subprocess;
`python3 tests/native/v5-02-x02-c01-evidence/check.py`; read-only source/doc searches
and diff inspection. The first checker invocation had an overly narrow textual
coverage needle for E08 (looked for “omission” but row said “omitted”); corrected
the checker to accept “omitted” and reran successfully. Final result is 31 valid
local links/anchors, 57 textual section/schema/coverage assertions, eight UTF-8
byte-length arithmetic checks, and complete baseline hash comparison. All pass.
No executable schema, parser, Lua or product test is claimed. `omission-vector.json`
records the independently counted 87→94→59-byte assembly vector. The 405 test
inventory and previous R14 gates remain prior evidence, with no new count.

`candidate.json` uses repository-relative paths for two candidate document hashes;
`candidate-owned/` freezes those bytes. `owned.diff` is relative to baseline-owned,
not Git HEAD (the rewrite is intentionally uncommitted). `report.json` records
candidate/diff SHA256s. `evidence-sha256.json` uses evidence-relative paths and
excludes itself; do not treat its hash as a candidate-document identity.

No build, CTest, runtime/schema execution, lint, Planar tools/skills/queue,
installed binary, operator data, commit, installation, publication or merge was
used. Debug/release build directories were untouched and remain idle. Linux,
execution, RA01 and external runtime acceptance remain open. Author releases
ownership of both documents and this evidence for fresh independent review;
coordinator changes to the round have separate ownership.
