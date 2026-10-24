# X05-A3-C05 — Sticky completed Lua memory failure

Candidate pending fresh independent review. Work used shared
`dev/planar-v2-vernacular`, HEAD
`3171c49e7b92d3abda01bfbb8bfb0c6b29d4c67a`, preserving the intentional rewrite
dirty tree and all prior evidence. Coder ownership is runtime.cpp,
external_registration.t.cpp, new C05 support/evidence and the six authorized
current reference files; coordinator owns the C05 round. No legacy Planar tools,
roles, skills, queue, installed binary, operator data, commit, installation,
publication or merge was used. Sole Debug build ownership is retained until
final release identity verification.

## R7 correction

The restricted native `pcall` wrapper now sets the trivial `lua_memory_failed`
bit when the actual completed `lua_pcall` returns `LUA_ERRMEM`. It sets no flag
for an initial allocator null: Lua's emergency GC retry can still recover.
The common exhaustion predicate and nested wrapper include the bit; enclosing
restricted calls rethrow, and all existing command host adapters retain resource
classification. The standalone trusted-export adapter recognizes the bit after
a nested wrapper rethrows as an ordinary Lua error. Native diagnostic priority,
resource text/limits, allocator accounting and existing quota flags remain.

No registration callback change is necessary: the original entry token already
precedes descriptor/function reference retention, and native ownership precedes
Lua allocation. The immediately enclosing wrapper now retains failure before
Lua resumes. The host reports resource before protocol classification and renders
the original token outside Lua; subsequent callbacks and module tails cannot
clear it. Reference cleanup and terminal owner behavior remain intact.
[Bounded related-class audit](../v5-02-x05-a3-c05-support/caught-memory-class-audit.json)
traces the allocator, nested wrapper, raw references, standalone export, native
anchor, setup, native enrollment, sealing, profile calls and invocation adapters.

## Actual allocation evidence

[Red generator](../v5-02-x05-a3-c05-support/red-reference5.py) is an output-path-only
copy of the immutable C04 review reference5 generator. Its [exact reversible
substitutions](red-reference5/delta.json) preserve the pre-C05 product copy.
Both expected-resource detectors reproduce exit1: [normal tail](red-reference5/run-one.log)
reports `internal / Invalid external profile protocol / catalog.projections[2]`;
[later ordinary Lua error](red-reference5/run-one-later.log) reports the same
internal message at `profile.modules[1]`. Both refuse two actual 24-byte growth
requests at projection2's descriptor reference, observe exactly one real
`LUA_ERRMEM` with quota/conversion flags false, and then reach projection64's
function reference. The no-fault twin succeeds; the function-reference-only
window observes no growth. All owner cleanup checks report zero live Lua bytes.
Historical C04 evidence is unchanged.

The [green generator](../v5-02-x05-a3-c05-support/memory.py) uses reversible private
production copies and actual null returns at real growth requests. It never
injects a Lua status, sets an exhaustion flag, fabricates an allocation or throws
a C++ exception. Ten modes cover no-fault, the zero-growth function-reference
window, caught memory with normal/later-error tails, three catch layers with
both tails, one-null emergency retry recovery, direct host memory failure,
caught ordinary error success, and later ordinary error refusal. Actual request
bytes, positions, raw nested statuses and zero-live-byte destruction are logged.
This is controlled allocator refusal, not natural host exhaustion.

Caught failures now report exactly `resource_limit / Lua allocation exceeded /
catalog.projections[2]`, limit `lua_bytes`, maximum 67108864. They stop at the
failed descriptor reference: last index 2/phase 1 and no later callback. Three
nested frames observe the initial real memory status and two subsequent ordinary
Lua statuses with the sticky bit true. The direct host memory control keeps its
existing resource result. Recovery logs the same actual 24-byte request first
refused then permitted, returns a valid catalog, and observes no `LUA_ERRMEM`.
Ordinary caught Lua errors permit later registration; a later ordinary unhandled
error remains `internal` at module1. No actual function-reference allocation
failure is claimed.

Two repository-native controls use the public production constructor without
injection: nested ordinary errors permit a later valid projection; nested quota
failure followed by a would-be Lua error retains original shape 65 resource
attribution. Existing actual memory failure requires the separate private
allocator probe. Focused C05 development execution passed 2/2.

## Verification and boundary

The final gate freezes complete source membership, modes, symlinks and bytes
once, then serializes Debug configuration, all+planar_tests, inventory, focused
execution, actual-memory probes, eight retained runners, high catalog,
four vectors, four historical exact-source audits, A0 reversible source audit,
pinned formatting/Doxygen and a no-work build. Source and build identities are
checked again before release. Final evidence links and counts are appended by
finalization after the gate completes.

Development evidence is retained, including the first expanded-main compile
failure caused by assigning an empty initializer to move-only `expected`; the
harness assignment was removed before successful development runs. Product
source was unaffected by that harness error. The final source freeze occurs
after formatting and all implementation/reference/support edits.

No full 582 suite, release/RelWithDebInfo or Linux gate is claimed. Latest full
gate remains 520 per configuration. Combined A3 through C05 requires fresh
independent review before an integrated gate. Public profile CLI, B/C adapters
and semantics, X06 storage/history/interpretation/acceptance, caller receipt and
budget orchestration, mandatory external-profile gate, public cutover,
execution/RA01 and Linux remain open. Public init creates format1.


The support adaptation manifest records the initial output-routing pass from
C04 runners; `verify.py` then received C05 inventory/count checks and the new
actual-memory runner. Its final exact bytes and complete owned diff are bound
by the final freeze, rather than the intermediate adaptation hash.


The final gate completed all **24 recorded commands** successfully.
Inventory is **582 unique names**, preserving all **580** prior names.
The exact focused selector executes **209/209**, with zero failures, errors,
skips or disabled cases. [Commands](final/commands.json),
[JUnit](final/focused.xml), [summary and names](final/test-summary.json),
[inventory preservation](final/inventory-preservation.json),
[raw focused output](final/focused.log).

All **10 actual-memory modes** pass with zero-live-byte cleanup;
[commands](final-memory-probes/commands.json),
[normal caught failure](final-memory-probes/run-one.log),
[later-error tail](final-memory-probes/run-one-later.log),
[three nested catches](final-memory-probes/run-nested3.log),
[nested later-error tail](final-memory-probes/run-nested3-later.log),
[successful actual emergency retry](final-memory-probes/run-recovery.log),
[direct host failure](final-memory-probes/run-host.log),
[zero-growth function reference](final-memory-probes/run-function.log),
[ordinary caught error](final-memory-probes/run-ordinary-caught.log),
[ordinary later error](final-memory-probes/run-ordinary-later.log).
[Reversible source substitutions](final-memory-probes/delta.json) bind the exact
fixed production source and private copy. Retained **69 class scenarios**, ten
C03 allocation scenarios, repeated-root, location, production, reachable-native,
R1/R2 correction and exact/one-byte-short budget probes pass. The high catalog,
four vector sets, B2/D6/D6-C01/D7-B exact-source audits and A0 reversible runtime
source audit pass. Pinned formatting/Doxygen checks **117 active sources**;
the final all+planar_tests build reports no work.

[Release audit](release-audit.json) verifies 23267 source entries and
931 build identities with zero mismatches. Complete source membership,
modes, symlinks and bytes match the one final freeze through release. AGENTS.md
remains the CLAUDE.md symlink. [Owned diff](final/owned.diff),
[scope audit](final/scope-audit.json) and copied candidate sources bind the change.
Shipped Lua, guard/API/build/codec/vendor and historical evidence remain unchanged.
[Finalization](finalization.json) binds this report, complete artifact manifest,
source freeze/release, owned diff and build identities. Serialized Debug build
ownership is released after this verification; no subsequent edits follow.
