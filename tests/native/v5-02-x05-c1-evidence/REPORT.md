# X05-C1 pure section assembly candidate

Status: implementation and focused Debug verification complete; **independent
review pending**. No self-approval, full C milestone gate or release rerun.
Shared checkout `dev/planar-v2-vernacular`, starting HEAD
`3171c49e7b92d3abda01bfbb8bfb0c6b29d4c67a`; inherited dirty rewrite preserved.
No Planar tools/skills/roles/queues, subagents, commits, installation, publication,
merge or operator data were used. Ownership is the nine files in the scoped diff
plus new C1 support/evidence; coordinator planning rounds remain separately owned.
AGENTS.md remains the symlink to CLAUDE.md.

## Changes and boundaries

`src/stdlib/planar/render.lua` extracts private `assemble_sections(ctx,base,custom,budget)`
in the existing embedded source. It copies base/custom inclusion bookkeeping,
retains fully prepared base bytes and mandatory flags, appends custom heading plus
supplied body bytes, and recomputes exact joins and notice after each removal.
Custom optionals drop in reverse result order, before checkpoint/job_notes/
workspace_notes/rationale. Mandatory records never drop. Metadata remains in
original section order; omitted IDs retain actual removal order. No truncation,
trailing-LF normalization, new source module, inventory or native source-count
reservation is introduced.

Ordinary `render_material` passes an empty custom array and keeps its existing
manifest fields. The helper returns the omitted array already tagged once; the
manifest reuses it. The narrow private `assemble_projection_material` bootstrap
export adapts exact projection metadata to {id,bytes,included}. This internal
ABI requires authentic native-prepared base, already admitted custom records and
a validated budget. It does not authenticate caller tables, validate native F
structure/limits/reserved collisions, dispatch retained F or expose a command.
Native projection admission/identity/dispatch, reads, capabilities, receipts,
whole-invocation orchestration, profile CLI, X06–X08, execution/RA01 and Linux
remain open. Public init creates format1.

Six colocated C1 tests cover the exact 87→94→59 vector at budget60; omitted[b,a]
and metadata bytes11/37/37/48; mandatory/base retention and exact-budget/one-byte-
under refusal; mixed custom/base priorities; optional exhaustion and notice growth;
UTF-8 byte lengths, supplied zero/one/multiple trailing LFs; empty/singleton and
exact extension metadata; repeatability and input snapshots. Isolated native
freezer probes add actual readonly root/base/custom refusal and two invocations
against the same frozen Lua input. These isolated fixtures test the algorithm,
not replacement mandatory packet sections or native admission.

## Baseline and preserved attempts

The actual pre-edit baseline records 29,923 filesystem entries with membership,
SHA256, modes and symlink targets in baseline-filesystem.json. original-source/
contains actual copies of all nine existing owned files. The archived render.lua
SHA256 is `803287f7bd091c2bea270021c4bbda94204f62648bc884f0691bb320455fbe51`. Both build configurations'
starting object/PCM/archive hashes were captured before edits. No git clean-tree
assumption was made.

Development assembly attempt01 exposed an incorrect test expectation (the mandatory
packet is25 bytes, not26); the corrected exact/one-byte-under values are25/24.
Development old-body differential attempt01 then exposed a real integration
error: the extracted helper's omitted array was retagged by the old manifest
builder, causing native already_typed refusal. The manifest now reuses the typed
array. Raw failed JUnit/logs and development-build/development-build02/development-build03 remain preserved.
The successful development-probes02 and final-probes rerun the unchanged final
algorithm. These failures are not represented as successful verification.

## Frozen focused Debug gate

All product/test/support/current-prose files were frozen before the final gate.
The manifest checks before/after each command; final-filesystem.json matches the
freeze exactly. Exact argv/cwd/exit/duration for all 21 outer commands
are in [final-gate/commands.json](final-gate/commands.json). Private compile/link/run
argument arrays and statuses are in [final-probes/commands.json](final-probes/commands.json)
(8 commands). Every final command exits zero.

- `cmake --build --preset debug --target all planar_tests` passes.
- `ctest --preset debug --show-only=json-v1` registers **682** unique names,
  preserving all676 approved B4-C names and adding six C1 names.
- CTest with final-gate/selected-tests.txt executes **416/416** (prior410+six).
  The separately selected history/codec supplement executes **11/11**.
  Both have zero failures/errors/skips/disabled and all JUnit statuses `run`;
  immediate LastTest.log copies are preserved.
- Direct `[projection-assembly]` executes **6/6**; direct `[render],[profile-render]`
  executes **46/46**, including shipped/custom goldens and policy/definition/W
  expectations. No unexplained skips.
- Six common vectors pass: selection/W, shape definition, canonical/hash,
  record-v1, request-v2 and effective provenance. Six exact source/runtime audits
  pass: historical B2, D6, D6-C01, D7-B, B1 and B2-A.
- Pinned formatting/Doxygen passes over118 active C++ sources.
- Final `all planar_tests` build reports `ninja: no work to do.`

## Actual old-body differential and frozen-input evidence

The private adapter embeds the actual archived full pre-edit render.lua body,
renaming only five function identifiers at definitions/calls. The transformation
checks exact reverse reconstruction and records original/private assembly hashes.
No expected hash oracle is fabricated. Both before/after functions run with the
same current native boundary and authentic selected material; command VM capture,
semantic/selection services and effective-source identity remain the current
approved implementation. Thus the comparison isolates rendering byte behavior,
not a claim that old and current complete source identities match.

All46 existing `[render],[profile-render]` tests pass with **242 exact complete
Glaze-encoded response comparisons**, covering envelopes/manifests/errors,
worker/reviewer/recovery, current/base/no capture, observations, shipped/custom
shapes, exact budgets and omission metadata. The tests retain existing goldens.
Private setup then applies the actual unmodified production `freeze_graph` to
C1 input tables, installs private backings/memo, and verifies mutation refusal
plus identical repeated results on the same Lua input: **6/6** pass.

Private compile drivers copy current native/test sources only into this evidence
root. Every response file expands recursively. Object, PCM and dependency output
paths are isolated. The complete shared object/PCM/archive membership and hashes
are identical before/after private compilation. Original historical adapters and
artifacts are immutable. Generated variant/differential source is verification
only and introduces no product native edits.

## Identity and release

candidate.diff is the exact scoped before/after patch including new support.
candidate-manifest.json and candidate-source/ bind all frozen candidate source,
fixtures, vectors, support and current prose. protected-final.json confirms
baseline historical/source membership/bytes/modes/symlinks except the explicit
owned edits; no product native/CMake/vendor/codec/hash/schema/fixture/golden edits occurred.
The protected release object/PCM/archive membership and hashes remain identical
to their actual pre-edit baseline. No release build or test was invoked.

Artifact/candidate/diff/report hashes are bound by finalization.json. Debug and
all author edits are released after final hashing; no subsequent author edits.
The coordinator must check this exact candidate before fresh independent review.
