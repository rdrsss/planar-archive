# X05-C3-B1 bounded metadata parent traversal candidate

Candidate awaits fresh independent review; no self-approval. Shared branch
`dev/planar-v2-vernacular`, starting HEAD3171c49e7b92d3abda01bfbb8bfb0c6b29d4c67a.
Only six owned existing files changed; new support/evidence are local to this round.
No runtime wrapper, handler/provider, public CLI or profile product implementation.

Native admission now opens physical parent components using retained no-follow
directory descriptors. Upward edges are retained for verification while the
active final ancestry pops. Exited directories therefore never fabricate final
containment but still invalidate the original owner if moved/replaced. Every
preceding directory must exist, have the correct type and not be a symlink before
its parent step executes. Root parents, repeated parents, dots and repeated/trailing
slashes preserve directory typing. Existing role/alias/poison/move/absent-target
policy and no-content-read boundary remain. Storage accounting is described in
[accounting](../v5-02-x05-c3-b1-support/accounting.md).

`preedit/` contains the six owned source/prose bytes captured before edits.
`protected-before.json` and `release-before.json` record full protected membership,
mode, links and file SHA256. Historical evidence is included; round records are
excluded because the coordinator owns them. Baseline snapshot hashing continued
while the test-only regression was added/built. Consequently the actual raw CTest
capture is explicitly named `inventory-captured-after-red-build.json` (717 names).
`inventory-before.json` is a derived716-name baseline, verified exactly equal to
immutable C3-A final-gate inventory after removing only the single new red name;
it is not represented as a preedit CTest capture. Owned preedit bytes were copied
before any changes. `git-status-before.txt` likewise is a post-red-build status
capture of the intentional shared dirty tree.

`red-build.log` and `red-test.log` preserve the real ordinary-native failing
regression before the production fix: trusted/../workspace was refused. The red
CTest invocation ran exactly one test with one failure. `development-parent.log`
is explicitly an invalid/stale exploratory run made before the asynchronous
production rebuild completed; it is not candidate verification. The subsequent
serialized `development-parent-after-build.log` verifies six parent-named cases;
the seventh new integration case is included by the final gate's binary-based
selection.

Seven new cases cover parent spelling/equivalence, trusted-root parent spelling,
root/repeated parents and slash/type behavior, refusal of symlink/missing/file
before a parent across root/file/target, physical overlaps bothways, registry vs
observation/declaration policy, selected aliases, absent export target appearance,
exited-directory replacement, and the same original profile_capture owner with
retained original bytes and late receipts. Existing read interposition cases
verify no content reads before binding.

Final gate results are recorded in final-gate/summary.json and commands.json.
The frozen support runner checks all716 baseline names survive and exactly seven
new names exist; selects113 native cases across complete admission/profile/
profile_codec/host_path/native/capture binaries; verifies JUnit run status and
zero failures/errors/skips/disabled cases; runs pinned formatting/Doxygen and a
no-work build. Product/test/support/prose is frozen before that gate; only evidence
is finalized afterward. `frozen-candidate.json`, `candidate.diff` and
`candidate-source/` identify exact candidate bytes. Final artifact manifest
records schema/version, modes, sizes, link targets and SHA256. No full suite,
release suite, private probe, Linux gate, commit/install/publish/merge or operator
data access is claimed. Release identity must match before and after.

The frozen gate passed113/113 in46.20s with zero failures, errors, skips or
disabled cases. Inventory723 preserves all716 baseline names and adds seven.
Pinned formatter/Doxygen passed over118 active sources. Both frozen build checks
reported no work. Final identity verification and ownership release are recorded
in protected-final.json and ownership-release.json; no sources, tests, support or
product prose changed after freeze.
