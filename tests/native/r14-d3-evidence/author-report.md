# R14-D3 author handoff

Status: documentation candidate pending fresh independent review; no self-approval.
Shared checkout `dev/planar-v2-vernacular`, starting HEAD
`3171c49e7b92d3abda01bfbb8bfb0c6b29d4c67a`; pre-existing uncommitted rewrite preserved.
Coordinator owns `docs/vernacular/rounds/R14-D3.md`; this author did not edit it.

## Candidate

Nine repository-relative authored files are listed in `owned.json` and frozen
under `candidate/`: one new exact load amendment and eight scoped contract/status/
architecture updates. Eight pre-existing author-owned files were captured before
edits under `baseline/`; absence of the new file is represented by `/dev/null` in
`owned.diff`. Baseline/candidate manifests explicitly declare their path bases.
Diff uses Python difflib unified_diff with repository-relative a/b labels and
original file ordering from owned.json; it is relative to this round's baseline,
not HEAD's intentional larger rewrite.

The amendment pins the mandatory fifth descriptor, six required text options
including `--expect-revision`, native prelude/replay, three finite fresh-load
services, all-job interpretation without source topology, native closure membership/
provenance, typed locked CAS/interner details with request privacy, exact saved-byte
output and native completion state covering known commit, replay and uncertainty.
Native final receipt reads under lock remain mandatory. Follow-on D4–D8 slice
ownership/matrices prevent partial public catalog admission. Current commands stay
render/init/history/export; proposed load, full R14, external profiles and Linux
remain pending. Existing v1 formats, user v5, render/W bytes and old evidence are
preserved; status references acknowledge independently approved C.

## Verification and limits

`python3 tests/native/r14-d3-evidence/audit.py` passes:

- 212 local file links/anchors across the nine owned documents, zero failures.
- 11/11 documentation traceability assertions; these are not runtime tests.
- 6,124 protected ordinary-file byte hashes unchanged, plus exact AGENTS.md symlink
  target `CLAUDE.md`. The original protected-before map has 6,125 entries because
  initial hashing followed that symlink. Its target bytes intentionally change as
  an owned file; verification checks the symlink itself instead of falsely
  treating the allowed target change as a protected mutation.
- Baseline/candidate snapshots, manifests and deterministic baseline-relative diff.

`git diff --check -- [eight existing owned documents]` exits 0; all nine owned
files also pass an explicit trailing-whitespace scan. No product builds/tests,
installed binaries, operator data, Planar tools/skills/queues, commits, installation,
publication or merge. Existing approved 370 debug/release tests on macOS ARM64/
LLVM23.1.4 are cited baseline evidence, not rerun or new load acceptance.

An initial audit correctly noticed AGENTS.md's followed target hash changed with
CLAUDE.md. The audit was corrected to verify the symlink identity and hash the owned
target through the candidate manifest; no product or protected input changed to
make that check pass. No unresolved product choice was identified. Proposed native
load overload/typed details/completion receipt still require implementation review.

`candidate.json` keys are repository-relative and map current files; candidate
snapshot prefix is `tests/native/r14-d3-evidence/candidate/`. `baseline.json` keys
are repository-relative under its baseline prefix. `artifacts.json` keys are
relative to this evidence directory, excluding itself to avoid recursive identity.
All hashes are SHA256. Re-run audit.py to verify current protection/links and
regenerate identical candidate/diff; compare manifests before review. No build
directory was claimed or used.
