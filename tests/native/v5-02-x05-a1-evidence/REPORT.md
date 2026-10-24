# X05-A1 production sealing foundation candidate

Status: candidate pending fresh independent review; no self-approval. Shared checkout
`dev/planar-v2-vernacular`, starting HEAD `3171c49e7b92d3abda01bfbb8bfb0c6b29d4c67a`.
The coordinator owns the round. Existing uncommitted rewrite/history was preserved.
No Planar operational tools/skills/queues, installed binaries, operator data,
commit, installation, publication or merge were used.

## Production change

The new `src/engine/runtime/closure_guard/` module promotes the independently
approved A0+C01 pinned-Lua metadata algorithm into the embedded five-command VM.
It rejects writes to retained upvalue cells, including inherited descendant
prototype writes and independently reachable helper writers. Invocation-local
scratch remains allowed. Graph keys, values, aliases and actual closure upvalues
are inspected without executing inspected code. Pinned LOADKX/NEWTABLE/SETLIST
payload rules remain isolated in this module; a pinned Lua upgrade must revalidate
these private metadata and active-frame assumptions. No vendor code changed.

The guard uses caller-supplied PMR storage. The runtime adapter charges every
allocation request before allocation through the existing checked cumulative
512 MiB native conversion accounting; deallocation does not refund it. There is
no fresh 4 MiB arena or new public edge/prototype/instruction quota. Checked
inspection counters accumulate in native state separately from executed Lua
instructions and converted nodes. Prototype memo keys contain both `Proto*` and
all inherited sealed-mask bits, preventing repeated shared-prototype rescans
without confusing retained cells and invocation-local cells. Compiled scan input
remains bounded by the existing registration Lua heap. Embedded invocation code,
including its instruction reset, is byte-identical to the starting candidate
(see contract-preservation.json).

One multi-root audit shares identity/prototype memo across original globals and
the five callable roots, before any proxy mutation. This preserves the existing
freezer's depth256 behavior: primitive and previously visited alias leaves do not
consume a new retained-object frame. Callable C roots refuse even if previously
seen through globals. Tests cover a boundary global tree, newly reachable helper
writers after the first root, shared prototypes and explicit native callable roots.
The existing alias-preserving table freezer remains unchanged.

The actual protected sealing C frame reserves 2048 relative slots before root
pushes or native RAII. Audit entry checks current CallInfo capacity without
allocation; every traversal push/raw-next/upvalue operation checks actual
headroom. The guard performs no Lua allocation/metamethod/call. All native owners
unwind before protected Lua error/allocating paths. Failure restores the original
Lua top. Native accounting diagnostic exceptions unwind through the guard and
are caught by the runtime outside audit RAII; bad_alloc is typed resource refusal.
Existing depth and stack resource diagnostics retain their limits.

A strong private native-identity catalog is captured before module evaluation.
Only the five native-owned digest/JSON callbacks enroll after their explicit
native installation, in an immediate protected frame with no intervening module
execution. Enrollment uses fixed native-owned field names, never module-supplied
identities. A0's separate test catalog slot remains independent. Known native
contexts remain an explicit trusted opaque policy. Proxy helpers are installed
only after audit; this does not claim they pass a subsequent raw audit.

Owned edits are the new guard module/tests, narrow runtime implementation/API
comments, engine build wiring, six current documentation files and new A1
support/evidence. The coordinator additionally authorized exactly the guard
DEPENDS additions for copied-runtime targets in `src/cmd/planar/CMakeLists.txt`;
its original bytes were copied and matched against baseline before editing.
Historical generators/support/evidence and shipped Lua are unchanged.

## Frozen verification

Frozen attempt01 completed all 15 commands, all exit zero. Serialized Debug
`all` plus `planar_tests` builds pass. The registry contains **552** unique names,
preserving all **535** starting names. The broad focused selector matched and
executed **137/137**, zero failures/errors/skips/disabled cases: **17** new
production guard cases, **15** historical A0 cases, **32** command VM cases and
**73** runtime/interpretation/load/acceptance/CLI/workspace cases. Exact matched
names and statuses are in attempt01/test-summary.json and focused.xml. The
existing command packet/golden and live service tests pass unchanged.

Eight reversible production probes compile/link against exact frozen candidate
module metadata. Their saved source copies, transformed assemblies, commands,
objects, binaries and transformations.json are under attempt01-production-probes/:

- The original production runtime accepts the retained-scalar-writer source
  (red control); the guarded runtime rejects that same exact source as internal.
- A production source variant with invocation-local nested scalar scratch creates;
  the guard unit test also executes local nested counters successfully.
- The exact shipped VM with an independent current-frame graph-entry invariant
  creates with no detected overflow. Guard pre-operation checks remain active.
- A same-native-pointer closure with a new opaque context installed after the
  pristine snapshot refuses; it is not enrolled as a trusted engine callback.
- Exhausting existing native accounting immediately before the actual shared
  audit refuses resource_limit through the real adapter. A separate injected
  upstream bad_alloc also produces resource_limit after ordinary native unwinding.
- The controlled production stack-reservation fixture lowers only the earlier
  backing reserve to20, performs supported full GC, and denies real stack growth
  **816 -> 32880 bytes**. The active protected reserve remains2048; no audit-entry
  sentinel runs, and actual create returns the existing stack resource_limit.

The guarded retained-writer and unknown-native probes exercise real production
creation, not historical test-only runtime guard variants. Source assembly variants
change only commands.lua and module identity; exact reverse equality is asserted.
The initial probes preserve their earlier source identities independently.

Canonical, record-v1, request-v2 and effective-provenance vector checks pass.
Historical B2, D6, D6-C01 and D7-B exact source audits pass. A0 runtime/interface
forward/reverse audits still pass against the current production source, and the
A0 shipped-source assembly remains exact. Pinned formatting/Doxygen pass on all
**116** active C++/module/test sources. Final all+planar_tests no-change build
reports `ninja: no work to do.`. No full552 suite, release/RelWithDebInfo or Linux
gate was run.

## Preserved development results

Initial configure failed the historical generator's exact JSON-tail assertion;
the production enrollment was moved to its immediate separate protected native
frame, preserving the immutable historical pattern. The next build exposed
missing new-module dependencies in CLI copied-runtime targets; the coordinator
authorized the narrow wiring extension. Initial-focused and second-focused each
retain the existing global-depth boundary failure. The fix shares root memo and
checks depth only for newly visited retained objects; third-focused passes18/18.
Initial lint retains the undocumented test helper warning, fixed by placing the
helper in an anonymous namespace; second lint and final lint pass. An intermediate
edit script reported `ValueError: substring not found` after adding the multi-root
API but before changing the formatted runtime helper; a subsequent exact helper
replacement completed the integration. No failing output was overwritten and no
production detector was weakened or old test changed.

## Identity and limits

Baseline contains **18994** regular/symlink entries; frozen candidate contains
**18999**. Only root .git/build and this A1 evidence root are excluded; nested
names remain. Full-tree checks before/after every final gate and finalization
verify bytes, modes and symlinks. Baseline diff is confined to the scoped paths;
all historical support/evidence, shipped Lua, vendor and old tests are identical.
AGENTS.md remains the CLAUDE.md symlink. Build-identities.json retains **747**
executable/object/PCM/cache/build-graph hashes.

- Source-freeze SHA256: `959facccc82fa81be0b3e0acfaaf5c1e9bb77a5da8f51e2062129de792a48f5e`.
- Candidate owned.diff SHA256: `c85838281aca49c545706e4dc865b24f4d856c84fd21a0ea72d573906ec614a4`.
- Source manifests map repository-relative paths to `{sha256,mode}` or
  `{symlink,mode}`; artifact manifest maps A1-evidence-relative paths to SHA256.
  Artifact manifest excludes itself and finalization.json to avoid circular hashes.

This foundation adds no external module execution, public profile flags,
source-owner/three-descriptor admission, module-boundary enforcement,
whole-registration external budgets, live shape/projection semantics, profile
storage/history dispatch or public cutover. Later A rounds retain module-end
sealing before later chunks, frozen globals/API, persistent frozen-identity memo
and add-time descriptor snapshot versus retained mutation semantics. The mandatory
external-profile gate, execution/RA01 and Linux remain open; public init creates
format1. Independent review must assess this production candidate before it
advances. Serialized build ownership is released after final artifact hashing.
