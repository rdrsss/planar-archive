# X03-B3 verification candidate

Verification-only candidate for independent review; no self-approval. Repository
path base `<repo>`, shared branch
`dev/planar-v2-vernacular`, HEAD `3171c49e7b92d3abda01bfbb8bfb0c6b29d4c67a`.
The coordinator owns `docs/vernacular/rounds/V5-02-X03-B3.md`. B3 author owns only
its new support/evidence directories, `docs/vernacular/x03-format-evidence.md`
and the eight status documents listed in `owned-status-paths.json`.
No product, existing test, CMake, contract or existing script was edited.
No Planar tools/skills/queue, installed binary, operator data, commits,
installation, publication, merge or subagents were used.

`../v5-02-x03-b3-support/verify.py` captures complete filesystem identity and runs
the documented helper-equivalent gate, exactly once in each configuration:
configure preset; all + planar_tests; exact inventory; full CTest with absolute
JUnit output; immediate LastTest/CTestCostData copy; assert all actual cases ran,
nonzero count equals inventory and zero failures/errors/skips; pinned active
formatting/Doxygen; final no-work build. Debug runs before release. Exact argv,
cwd, UTC start/end, exit status and wall duration appear in each result JSON;
matching `.log` files preserve combined subprocess output. `debug-full.xml` and
`release-full.xml` retain actual full runs, not replacement focused selections.
`*-test-summary.json` retains all 430 exact names; both match the independent C01
430-name inventory. `results-summary.json` verifies preservation of all prior405
and extracts the real per-format fault counts and unique process boundaries from
both copied LastTest logs. No gate-helper run or duplicate full suite was used.

`../v5-02-x03-b3-support/audits.py` runs the independent 10 + 2 byte/hash oracles,
checks the five current storage/export generation maps forward and reverse, saves
the exact generated files/maps, and regenerates the seven inherited native/CLI
families plus fixed-export harness, D6 six variants, D6-C01 and D7-B CLI graphs in
scratch. Every regenerated file matches current generated source. Fifteen compiled
native support C++ files receive pinned formatting checks. Debug supplemental
audits were repeated after adding generated-source artifact capture; no full test
suite was repeated and no executable source changed. `*-generated-source/` paths
are evidence copies; their original path base is the corresponding build tree.

The existing B1 `old-binary-results.json` already contains eight real executable
history/load refusals against native-created format2 (clean/journal/WAL/SHM), all
exit3 unsupported_ledger_version and identical complete directory snapshots.
`retained-old-binary-proof.json` rechecks all eight results and the archived binary
hash `be1c4cee1b358208831faa919707ce5fd01de3ea31bc63db333a027f8844464a`, with
exact hashes of its baseline no-work/dependency/provenance and independent-review
sources. B3 reuses this reviewed existing proof; it does not claim a new old-binary
execution or a pristine historical source rebuild. B1 evidence was not overwritten.

`results-summary.json` also binds the approved B2-B+C01 source members and verifies
all 62 protected record-v1 codec/stdlib/fixture hashes against the pre-X03-A
manifest. This establishes unchanged shipped W selection and old goldens as well
as current full-suite execution; no profile/custom W implementation is inferred.

The contemporaneous `baseline-complete-filesystem.json` covers 15,424 regular
files and two symlink targets. Exclusions apply only to root `.git/`, root
`build/`, and these new B3 support/evidence directories; historical nested build
files and prior evidence are included. It was captured before builds/status edits,
never reconstructed. Each `*-source-stability.json` confirms the complete baseline
still matched after that configuration's gate. `initial-capture-note.md` discloses
a subsequent raw-git-diff display decoding error and delayed baseline build/cache
identity capture. The error did not alter or invalidate the original filesystem
manifest or saved raw diff. `*-build-identities.json` binds the actual tested
binaries/cache/compilation database/build graph and is rechecked at final freeze.

The eight status documents and new coverage report are finalized after both
verified configurations. `final-complete-filesystem.json` therefore accurately
records those allowed prose changes rather than claiming the whole filesystem
remained unchanged afterward. `ownership-audit.json` checks no missing baseline
files, exactly those eight changed files and one new report outside B3 support/
evidence, and unchanged symlink targets. `baseline-owned/` preserves the exact
original bytes of all eight status documents. `candidate-source/` snapshots the
owned documents and four new support scripts; `candidate-manifest.json` records
repository-relative identities, and `owned.diff` reconstructs their exact change.
`identity.json` binds the baseline, final filesystem, candidate and diff. The
artifact manifest uses this evidence directory as path base and excludes only
itself. All artifacts/reports/manifests are frozen before ownership release;
there are no edits after handoff.

See `docs/vernacular/x03-format-evidence.md` from the repository root for the
requirement-to-actual-test table and final results. This is macOS ARM64 Debug and
RelWithDebInfo verification of current native support. Independent B3 review is
required before the coordinator closes X03-B. X04/X06, public format2 cutover,
profile acceptance/history, Linux, hardware power loss and the mandatory external
runtime gate remain outside this evidence. Non-hot journal remnants continue to
refuse unchanged; recognized hot journals require explicit quiescent recovery.
