# R14-D5 independent review

Disposition: **approve** for the frozen bounded native load capture/preparation
candidate. No blocking findings. This approval does not admit public load or close
R14. Reviewed in the shared `dev/planar-v2-vernacular` checkout at HEAD
`3171c49e7b92d3abda01bfbb8bfb0c6b29d4c67a` on 2026-10-21.

## Exact candidate and preservation

The reviewed wrapped repository-relative candidate manifest contains 12 files:
`tests/native/r14-d5-evidence/candidate.json`, SHA256
`7c00052ec4a171f2575184a37194ff88193b3749fd495d4da237cf64e3bcb312`.
Its `owned.diff` SHA256 is
`df03ab634a8094deb5ffc40d01dd13547facceb89c9f1631ec37c9e5f0c54509`.
The wrapped evidence-relative `artifacts.json` contains 67 artifacts, SHA256
`3d3f4043b4959ded4b4c7d05515bbaf0be2710b9c340062de35c76995d7a559d`.

Independent [input audit](../../../tests/native/r14-d5-review-evidence/input-audit.json)
verified every current candidate path, frozen snapshot and evidence artifact,
then rechecked candidate and artifact hashes after verification. All match.
All 13,968 pre-existing protected paths outside permitted candidate/coordinator
round scope retain their hashes. The approved load contract body beginning at
`## Fixed catalog` is byte-identical to baseline; v5, fixtures/goldens, existing
runtime/stdlib/CLI/codec/storage algorithms and historical records/evidence remain
protected. `AGENTS.md` remains the symlink to `CLAUDE.md`.

## Boundary and implementation assessment

- `load_capture` is noncopyable and moves its sole native owner. Capture and
  preparation move state out before validation, so typed failure/consumption is
  terminal; workspace retrieval is one-shot and repeated retrieval releases
  ownership. Moved-from owners refuse, replacement assignment releases prior
  ownership, and self-move preserves it (`load.cppm`, `load.cpp:94–140,232–236`).
- `open_workspace` validates both explicit absolute lexical intent paths before
  filesystem work. It uses the normalized target directly, with no cwd,
  environment or workspace-name lookup. The workspace parent becomes the confined
  root; only its declaration/references are opened initially (`load.cpp:103–127`).
- Capture validates fixed route containers, original workspace/root/registry
  identities, portable paths, sorted/unique job routes and agreement among raw,
  normalized and routed job maps before spec/job reads. It loads every job and
  every declared text reference, separately captures explicit registry bytes,
  and seals exactly once with `finish_for_acceptance` (`load.cpp:137–205`).
  There is no recursive tree call, source/base mapping, observation or selected
  job/role input. Unrelated siblings cannot become source members.
- Preparation first uses pure verified stdlib and interpretation codecs; subsequent
  typed checks compare retained declaration headers/files and normalized text
  references/bytes. B/M structural job correspondence is checked by the existing
  interpretation codec. Sorted source membership comes from **all** workspace
  files in the retained native snapshot, with no caller source-list parameter.
  The original sealed capture is transferred into `prepared_acceptance::create`,
  retaining its native receipt verifier (`load.cpp:232–302`). No Lua, callback,
  ambient embedded-source lookup, native schema normalizer/default evaluator or
  replacement semantic dispatcher is introduced. The new production target links
  capture/prepared/pure helpers; runtime/embedded-provenance linkage is test-only.
- Semantic rejection remains trusted Lua's responsibility. Actual Lua regressions
  exercise every job, invalid unselected jobs, duplicate job/work/role IDs, wrong
  constructors and mismatched job headers, checking logical file/field diagnostics.
  Malformed route or successful-protocol B/M disagreement gets a native typed
  refusal. Native structural correspondence does not certify semantic execution.
- The explicit trusted-native provenance seam is consistent with the scoped D5
  handoff: verification proves exact provenance integrity and retained ownership,
  not that these files came from the same retained command VM. Tests use real
  embedded sources and actual Lua routing/interpretation. D6 must bind the exact
  original VM bundle and demonstrate actual retained-handler execution/order;
  D7 must not bypass that obligation. This limitation is explicit in the interface,
  architecture, round and coder report and is not treated as delivered proof.

## Evidence sequencing and independent verification

The coder's first focused/build overlap is disclosed and superseded; it is not
accepted as final verification. The later strictly serialized completed run has
123 passing cases with zero skips, archived JUnit and LastTest. Their retained
`attempt-01` and `focused-final` copies are byte-identical. Final Doxygen changes
are confined to private implementation comments/whitespace in the displayed diff.
Independent token comparison confirms equality against the reconstructed snapshot,
but that reconstructed file cannot establish contemporaneous pre-comment bytes.
The aborted redundant rerun is retained separately and supplies no passing result.
These limitations are stated accurately enough for this review.

To establish current-source confidence independently, the reviewer ran, strictly
serialized in exclusively assigned debug:

1. `cmake --build build/rewrite-debug --target all planar_tests`: exit 0, no work.
2. `ctest --test-dir build/rewrite-debug -R 'replacement\.(capture |acceptance prepared |prepared |load capture |interpretation |atomic |command |runtime |.*cli$)' --output-on-failure --output-junit <review-evidence>/focused.xml`:
   **123/123 passed**, zero failures/skips: 117 unit cases and all six current
   actual CLI suites, including all seven new load cases.
3. `ctest --test-dir build/rewrite-debug --show-only=json-v1`: exit 0, 380 names;
   all 373 baseline names preserved.
4. `cmake --build build/rewrite-debug --target all planar_tests`: exit 0, no work.

Exact argv, UTC start/end times, exits and logs are in independent
[commands](../../../tests/native/r14-d5-review-evidence/commands.json),
[JUnit](../../../tests/native/r14-d5-review-evidence/focused.xml),
[LastTest](../../../tests/native/r14-d5-review-evidence/LastTest.log), and
[summary](../../../tests/native/r14-d5-review-evidence/summary.json).
The final-bytes rerun establishes behavior without relying on reconstruction.
The hash-verified coder final lint covers 92 active C++ files; unchanged vectors
pass 12/20/41; exact audits cover seven generated variant families and the fixed
runtime harness. Those passing artifacts are reused, not presented as fresh
reviewer executions.

The seven new tests cover exact six-file native closure and provenance payloads,
malformed route refusal before missing routed reads, B/M/header/file/reference
mismatch, selected byte/mode/root/ancestor/link/registry changes, unrelated sibling
tolerance, missing/oversized selected inputs, provenance corruption/bounds and
terminal/move ownership. Existing focused cases independently cover capture and
prepared receipt integrity, pure interpretation bounds, atomic acceptance and
unchanged CLI output contracts.

Debug verification is complete and its build directory is released idle. Release
was untouched and remains idle. Evidence is macOS ARM64 debug with the pinned
LLVM23.1.4/CMake4.4.4 toolchain; no full milestone, release/Linux runtime, hardware
power-loss, retained same-VM load or public load result is claimed. Reviewer wrote
only this review and `tests/native/r14-d5-review-evidence/`; no implementation,
prior evidence, Planar tools, installed binary, operator data, commit, installation,
publication or merge was used. The coordinator may advance to D6 after confirming
the exact reviewed candidate is still current.
