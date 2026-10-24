# X03-B2-A independent review evidence

Read-only review of the released shared-checkout candidate on macOS ARM64,
HEAD `3171c49e7b92d3abda01bfbb8bfb0c6b29d4c67a`, branch
`dev/planar-v2-vernacular`. Only this directory and the independent review report
are reviewer-owned. No product, source test, author evidence or status document
was edited. No Planar tool, installed binary or operator database was used.

`identity-before.json` and `identity-after.json` independently hash every candidate
file and author artifact against their manifests. Candidate snapshot copies also
match. `final-source-snapshot-audit.json` verifies the 13 pre-verification source
snapshot copies against the frozen manifest. All identities remain unchanged.

Each executed build/test/vector/lint/inventory command has a result JSON containing
exact argv, cwd, UTC start/end and exit code, plus unabridged merged output.
`focused.xml` and `focused-LastTest.log` were copied before inventory. The first
review build used an invalid preset name and failed before configuring or building;
`build.log` retains it. The corrected explicit build directory command passed
and reported no work. All later verification commands passed.

`test-summary.json` parses independent JUnit: 188 cases, no failures or skips.
`inventory-preservation.json` independently compares current names to the retained
416-case B1 final inventory: 421 current, no removals and five additions. Independent
record-v1/request-v2 oracles report 10/2 vectors. Pinned formatting and Doxygen
check 98 active sources. A final all+planar_tests build reports no work.

`baseline-audit.json` hashes all 14,980 regular files in the author's larger
pre-edit manifest, checks symlink targets, identifies only 13 authorized changed
files, and finds no missing or unexpected changed baseline files.
`baseline-omission-audit.json` walks the actual filesystem with only exact root
`.git`/`build` and this round's evidence/support roots excluded, without following
symlinks. The larger author manifest had also omitted four historical nested-build
files, totaling 946,649 bytes. Their paths/hashes are retained in this audit; all
four match the author's earlier 7,273-entry pre-edit manifest. There are no other
current omitted regular files or additional symlinks. The union of both retained
pre-edit manifests covers 14,984 regular files plus two separately checked symlinks.
The 14,980-file manifest alone must not be described as complete under root-only
build exclusion. This audit does not reconstruct a pre-edit baseline.

The review report states the retained initial-regression source-freeze limitation
and the scope of corrected assertions. This is focused debug evidence, not a full
release/Linux or format2 native acceptance gate. B2-B and X06 obligations remain.

`artifact-manifest.json` uses this directory as path base, hashes every other
review evidence file and excludes itself. The report records its exact SHA256.
