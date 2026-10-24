# X02 author evidence

Docs-only candidate; independent review required. `report.json` records exact
basis, checks and limits. `baseline.json` covers all 14,714 files outside .git,
build and this evidence directory; full pre-existing git diff/status/HEAD and
owned-document baseline bytes are retained. `candidate.json` hashes eight owned
documents; `owned.diff` compares against these exact baseline bytes (including
new untracked docs), not HEAD's intentionally obsolete product tree.

`check.py` verifies baseline protection, candidate new-document links/anchors,
explicit contract sections/requirement IDs, selected textual schema invariants,
command ABI-body equality, and the AGENTS.md symlink. `protection.json`,
`links.json` and `schema-coverage.json` retain results. This is prose consistency
verification, not executable schema validation or runtime acceptance. Earlier
text matcher authoring failure is disclosed in report.json.

No builds/tests ran. Existing 405 names and historical 402/402 debug/release D8
results are prior evidence, preserved by unchanged source/test/evidence hashes.
Both build directories remain idle. P1/P2 recommendations await review then
operator disposition. New profile, shape/projection API and v2 codecs are targets.

`evidence-sha256.json` hashes all files beneath this evidence root except itself;
paths are evidence-relative. Candidate hashes are repository-relative. Coordinator
round/reviewer reports have separate ownership and are not candidate entries.
