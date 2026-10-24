# V5-01 documentation candidate evidence

The baseline was frozen before this coder's edits in the shared checkout at
HEAD 3171c49e7b92d3abda01bfbb8bfb0c6b29d4c67a. The existing rewrite was preserved.
Owned paths are in owned-paths.json. Baseline copies/hashes are in baseline/ and
baseline.json; new authored documents have a null baseline. Candidate copies and
hashes are in candidate/ and candidate.json. candidate.diff compares the actual
pre-round files with this candidate, independently of pre-existing git changes.

protected.json freezes 11,711 existing files outside owned documents, build/cache/
git directories, this new evidence directory and coordinator-owned round records.
It includes user v5, historical v4, product/build tooling/vendor and prior evidence.
The checker also detects new files outside those exclusions. AGENTS.md is checked
as a symlink with target CLAUDE.md. The v5 hash remains
c3d37e0e28407a9066eb51c515982ef6c93d82795eb56f97dea4a780b66fe08e.

Run `python3 tests/native/v5-01-evidence/verify.py` from the repository root.
checks.json, link-check.json and verification.txt record results. Every authored
local Markdown link and anchor is checked. The matrix must contain exactly one
each of A01–A36, in order, with A35/A36 explicitly locally assigned. All three
exact contract bodies from their first level-two heading are byte-identical to
the baseline; only status/authority introductions changed. Whitespace checking
uses git diff --check on owned paths; candidate.diff is the exact scoped change.

No product build, CTest invocation, model call, legacy tool, installed binary or
operator-data access was performed. Historical test counts are cited evidence,
not rerun results. This package is coder evidence, not independent approval.
The coordinator owns V5-01/next-round records and their changing status.
