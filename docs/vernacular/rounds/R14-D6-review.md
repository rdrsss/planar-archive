# R14-D6 independent review

Disposition: **request-changes**. One scoped contract-ordering defect must be
corrected before D7. The finite retained protocol, same-VM bundle handoff and
completion receipt otherwise satisfy this round's source and focused evidence
checks. This is not public load or full R14 acceptance. Reviewed on 2026-10-21
in the shared `dev/planar-v2-vernacular` checkout, HEAD
`3171c49e7b92d3abda01bfbb8bfb0c6b29d4c67a`.

## Finding D6-R1 — Defer acceptance provenance assembly until a fresh load

**P2, blocking this candidate.** [runtime.cpp:869](../../../src/engine/runtime/runtime.cpp#L869)
calls `history::prepare_stdlib` unconditionally from `seal_catalog`; the call is
reached by `command_vm::create` at line 1648. Lines 862–872 charge and assemble
the original/assembly/manifest provenance and retain the owned tree before the
VM is returned, even for the four existing families or a VM that is never invoked.
This is acceptance provenance construction, beyond the existing original-byte
bundle seal. `prepare_stdlib` actually creates and canonical-encodes the manifest,
verifies assembly/tree structure, hashes the tree and copies all provenance
payloads (`provenance_codec.cpp:89–127`, including `own` at lines 56–66).

The approved [early replay contract](../load-command-contract.md#native-prelude-and-early-replay)
lines 99–105 expressly excludes provenance assembly while requiring registration
before request lookup. The fresh accept service at line 123 owns that assembly.
With this implementation, D7 cannot perform its required registration-before-
lookup sequence without first doing the excluded acceptance preparation. A current
provenance preparation refusal or allocation failure can therefore precede a
saved request lookup that must bypass that work. The defect is present in D6's
runtime seam; it does not require public D7 implementation to establish it.

Keep exact original/bundle identity binding in pure registration. Retain the
same-bundle immutable originals/separator/assembly identity there, then charge
and construct the owned verified provenance only on the fresh load acceptance
path, before handing its const span to the provider. Preserve cumulative charging,
single-VM ownership and no Lua under native transaction locks. Add a focused
regression establishing that creation/registration does not construct acceptance
provenance and that actual fresh acceptance still receives its own exact files.
The future D7 early-hit integration tests remain D7 work. No contract amendment
or additional public command is requested.

## Exact candidate and protected inputs

The wrapped repository-relative [candidate manifest](../../../tests/native/r14-d6-evidence/candidate.json)
contains 17 files, SHA256
`d037d95c7b5c060cde3de8d8e5a2965287be9a376c69932cb7b450078f293721`.
Its `owned.diff` SHA256 is
`3190a4bca0dc74f2ac2db488dd7ea92fbf202c3c9896911dbd519d5d2ca0049f`.
The wrapped evidence-relative `artifacts.json` contains 67 artifacts, SHA256
`94c6e1d6825752fc47aa7eeb804a07cab060945e6ea327fc342519f808d04c6d`;
its paths resolve under `tests/native/r14-d6-evidence/`, not the repository root.

The independent [audit](../../../tests/native/r14-d6-review-evidence/audit.json)
checks every current candidate file, frozen snapshot and evidence artifact,
reproduces the owned diff from baseline snapshots, and verifies all protected
paths outside candidate/coordinator ownership. All 4,687 contemporaneous baseline
paths and an additional 9,366 paths from D5's explicitly historical protection
manifest are preserved. Historical hashes are not represented as a reconstructed
contemporaneous baseline. No unexplained new path appears outside scope. The
approved contract body beginning at `## Fixed catalog` retains SHA256
`a842901180c15691cda24a0ddafcc708dcb88add85717b0080b2a1313e6f4ab8`;
`AGENTS.md` remains the symlink to `CLAUDE.md`.

The dependency-free `ledger/completion.cppm` and dependency-list-only command
CMake change match the coordinator's explicit scope refinements. Existing product
Lua, CLI algorithms, D5 capture/preparation, storage/codecs, generators, fixtures,
goldens and historical evidence are preserved.

## Remaining implementation assessment

- Production requires exactly four entries. Private generated runtimes change
  only module/source/namespace identities and the fixed admission constant to
  five; missing/duplicate/capability/signature variants refuse. There is no public
  load parser, runtime flag, optional production fifth entry or sixth capability.
- Typed options retain canonical token/numeric-head agreement, request ID and
  bounded UTF-8 actor. Workspace/ledger/registry slots carry invocation-local
  userdata identities. Only open/capture/accept methods are offered to load,
  exact keys and once/in-order state are enforced, and cross-family/forged calls
  refuse. D5 owns full route/B/M correspondence before native completion.
- The actual retained callable uses real `route_workspace`, `interpret_bundle`
  and D5 preparation. The altered callable changes resulting goal bytes and
  carries its own exact original `commands.lua` and provenance digest. The native
  immutable span comes from this runtime's imported bundle, with no ambient
  embedded helper, Lua-supplied bundle ID or caller source list. Finding D6-R1
  concerns when those files are assembled, rather than their demonstrated origin.
- Native errors poison further services and win over Lua replacement errors.
  Registration/execution conversion accounting remains cumulative; caught Lua
  allocation, instruction and native-byte exhaustion is terminal. Argument and
  result C++ owners unwind before allocating Lua callbacks can longjmp.
- The shared receipt has exactly four phases and owns saved bytes independently
  of Lua. It is passed by reference before acceptance, survives provider exceptions,
  result validation/conversion, handler failure, terminal cleanup and VM moves.
  Native phase assignment and owned-string moves do not allocate. Success requires
  committed/replayed and exact equality to the retained canonical saved bytes;
  uncertain completion refuses success while preserving its receipt. Test
  completion is explicitly simulated. Real ledger receipt production, identity
  recheck, replay, preallocated fallbacks and stdout transport remain D7 work.

## Evidence and independent verification

The final coder evidence is hash-verified: 133/133 cases passed with zero skips,
including 127 unit cases and all six current CLI suites. Final JUnit and LastTest
are preserved before inventory. Inventory 390 retains all 380 prior names.
The final lint covers 93 active C++ files; vectors pass 12/20/41; the seven
historical variant families, fixed harness and six private D6 variants pass their
exact-source audits. Final build-05 and the passing focused run use the frozen
candidate, with no later product/test edits. The initial missing dependency and
test compilation failures, first 8-case success, and 132/133 failed cumulative
fixture attempt remain honestly archived. They are superseded by final evidence,
not presented as passing checks; initial precise timestamps were not captured.

Fresh reviewer verification used the exclusively assigned debug directory,
strictly serialized:

1. `cmake --build build/rewrite-debug --target all planar_tests`: exit 0, no work.
2. `ctest --test-dir build/rewrite-debug -R 'replacement\.(command |runtime |load capture )' --output-on-failure --output-junit <review-evidence>/focused.xml`:
   **58/58 passed**, zero failures/skips, including all 10 D6 cases and 7 D5 load
   capture cases. JUnit and LastTest were archived before inventory.
3. `ctest --test-dir build/rewrite-debug --show-only=json-v1`: 390 names; all 380
   baseline names preserved.
4. `python3 tests/native/r14-d6-support/audit.py build/rewrite-debug <review-evidence>/private-variants.json`:
   all six exact native/interface/logical-original/assembly variants passed.
5. `cmake --build build/rewrite-debug --target all planar_tests`: exit 0, no work.

Exact argv, UTC times, exits and raw logs are in independent
[commands](../../../tests/native/r14-d6-review-evidence/commands.json),
[JUnit](../../../tests/native/r14-d6-review-evidence/focused.xml),
[LastTest](../../../tests/native/r14-d6-review-evidence/LastTest.log) and
[summary](../../../tests/native/r14-d6-review-evidence/summary.json).
The coder's broader 133-case result, lint, vectors and historical audits are reused
as verified evidence, not claimed as fresh reviewer executions. No redundant
full milestone was run.

Debug is released idle; release was untouched and remains idle. Verification is
macOS ARM64 debug on pinned LLVM 23.1.4/CMake 4.4.4. No release/Linux, full R14,
hardware power loss, real completion or public load result is claimed. Reviewer
wrote only this review and `tests/native/r14-d6-review-evidence/`; no product fix,
Planar tools, old queue, installed binary, operator data, commit, installation,
publication or merge was used. Start a fresh corrective round preserving this
finding and candidate; D7 must wait for renewed independent approval.
