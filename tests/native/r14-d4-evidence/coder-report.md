# R14-D4 coder handoff

Candidate for independent review; no self-approval. Shared checkout remains
dev/planar-v2-vernacular at 3171c49e7b92d3abda01bfbb8bfb0c6b29d4c67a.
Existing rewrite changes were preserved. No Planar tool/skill, installed binary,
operator data, release build, commit, installation or publication was used.

The native error owns an optional finite variant: optional signed-64 expected/
actual revision heads, or record kind/optional digest. The locked comparison after
request relookup and the unequal interner bucket are the only population changes.
Request-kind collision detail contains no digest. Existing tree/error copies and
rollback preserve complete ownership, proved by the selectively forced atomic
cases. No new module dependency, generator, runtime, codec, schema, storage/tree
algorithm, parser or public command was changed.

The shared response mapper validates detail/code, positive heads and exact
kind/digest pairing. It emits approved canonical expected/actual and kind/digest
fields, omits request digest, ignores native messages and refuses malformed
payloads as the fixed internal protocol error, status 4. Exact tests pin all 17
existing no-detail mappings, valid envelopes, malformed envelopes, empty stdout,
LF and statuses.

Owned scope is in owned.json: 17 paths including nine C++ paths and eight docs.
format.t.cpp remains byte-identical. The coordinator explicitly approved one
additional support path, tests/native/r14c3b2-support/atomic.t.cpp, before its edit;
its original bytes/hash were frozen first. Existing test names, rollback checks
and all generators remain. New evidence is confined to this directory. Status
edits acknowledge D3 approval and D4 candidate status; the approved load-contract
body and user v5 are unchanged, AGENTS remains the CLAUDE.md symlink.

The old API fails the added diagnostic test with two missing-detail assertions.
Typed regressions then show heads retained after rollback and later head advance,
all four collision kinds through writers and acceptance/tree callbacks, request
digest omission, request conflict redaction and byte-identical generic mappings.
No private request body, key, host path or payload is admitted to typed detail.

Final verification on macOS26.5.2 ARM64 / Clang23.1.4:
- Direct incremental all/planar_tests succeeds. One intermediate immutable-value
  compile failure was corrected; its raw build-01.log is retained.
- Final focused CTest: 153/153, zero failures/skips (147 unit plus six CLI).
  CLI process counts remain 247 render, 60 init, 115 history, 55 export, 99
  retained composition, 41 workspace selector; render also retains 60 host-path checks.
- The initial focused 153 pass preceded final formatting. Final formatting-only
  edits affected response.cpp, bootstrap.t.cpp and atomic.t.cpp; the final rebuild
  and second bounded 153 pass test the final formatted bytes.
- JUnit and LastTest are retained for red, initial focused and final focused runs.
  LastTest was copied before inventory. CTest wrote relative JUnit paths under the
  build directory; those unchanged actual files were copied into this directory.
  The red wrapper's original CTest exit was not separately recorded; its failing
  log/XML and two failed assertions are retained.
- Current inventory has 373 entries; every one of the prior 370 names is retained.
  This is not a full 373-test run. The approved C full debug/release370 baseline
  is reused; D4 does not rerun the full milestone.
- Pinned formatting/Doxygen passes all 89 active sources; all nine owned C++ paths
  separately pass pinned formatting, including both test support files.
- Unchanged vectors pass 12/20/41. All seven exact generated variant families and
  the fixed runtime implementation/interface harness match final build bytes.
- No-change all/planar_tests reports ninja: no work to do.
- Protection audit finds no changed protected files outside owned scope and no
  new out-of-scope files. It verifies all 17 baseline snapshots and the exact
  approved load-contract body.

candidate.json maps repository-relative paths to SHA256, with byte snapshots
under candidate/. owned.diff is the exact baseline-relative unified diff.
baseline.json maps repository-relative paths, with snapshots under baseline/.
artifacts.json maps evidence-relative paths. handoff-identities.json identifies
the candidate map, exact diff and artifact map. freeze.py is reproducible and
owns only this evidence directory.

Limitations: no Linux execution, hardware power-loss claim, actual allocator
exhaustion, public load service/CLI or complete R14 acceptance. No full debug or
release milestone gate was rerun. Build directories are idle; exclusive debug
ownership is released for the independent reviewer. Release remained idle.
All candidate/evidence writes finish with the final freeze before handoff.
