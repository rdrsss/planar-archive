# V5-02-X05-A3+C01+C02+C03+C04+C05 independent review

Disposition: **approve**. C05 closes A3-C04-R7: a completed Lua memory failure
caught by the restricted native `pcall` remains terminal, retains resource
classification and preserves the first original entry location. The combined
A3 through C05 registration candidate is independently approved. R1–R6 remain
closed. This approval permits the integrated X05-A verification gate; it is not
itself a full Debug/RelWithDebInfo or external-profile acceptance gate.

Review used shared `dev/planar-v2-vernacular`, HEAD
`3171c49e7b92d3abda01bfbb8bfb0c6b29d4c67a`, preserving the intentional rewrite
dirty tree. Writes are confined to this report and
`tests/native/v5-02-x05-a3-c05-review-evidence/`. Product, current references,
coordinator records and all author/historical evidence remained read-only;
others' work was preserved. No legacy Planar tools/roles/skills/queue, installed
binary, operator data, commit, installation, publication or merge was used.
Review held sole serialized Debug build ownership and read active CLAUDE.md,
implementation/roadmap/v5 direction, exact X-REG/X-LIMIT/X-STORE, the bounded
implementation path, C05 authorization and all previous A3/C01–C04 reviews.

## R7 closure and bounded related-class audit

[Restricted pcall](../../../src/engine/runtime/runtime.cpp):79 now records
`lua_memory_failed` only after the actual completed `lua_pcall` returns
`LUA_ERRMEM`, before Lua can resume. Its existing exhaustion rethrow and the
common [exhaustion predicate](../../../src/engine/runtime/runtime.cpp):767
include this bit. Each enclosing restricted wrapper therefore rethrows even
when its immediate result is an ordinary Lua error. There is no owner-local
reset. Only trivial locals survive the Lua longjmp boundary.

The allocator is unchanged: actual `realloc` null does not set the logical quota
flag. Pinned Lua's emergency GC can retry that allocation successfully. C05
therefore marks the completed failure rather than the first null return, and
introduces no quota/accounting change. Ordinary caught Lua errors remain
recoverable. Existing native diagnostic and instruction/stack/conversion
priority remain intact; embedded invocation still gives its existing retained
native service error precedence.

[Registration](../../../src/engine/runtime/runtime.cpp):832 already selects the
trivial original-entry token and installs native vector ownership before the two
raw descriptor/function `luaL_ref` boundaries. A failing longjmp retains both.
The now-sticky memory status blocks later callback selection/clearing and makes
the common module host adapter report resource before incomplete descriptor or
later Lua protocol failures. It renders the token outside Lua, retaining C03/C04's
owned module fallback if rendering allocation refuses. Failed constructors
destroy the owner; later collection/sealing/providers do not run.

The independent [class audit](../../../tests/native/v5-02-x05-a3-c05-review-evidence/class-audit.json)
covers actual versus logical allocation refusal, both raw references, all runtime
`lua_pcall` sites, constructor setup/native enrollment/initial sealing, external
execution/collection/sealing, standalone trusted exports and embedded invocation.
The standalone adapter recognizes the sticky bit when the nested wrapper
rethrows as ordinary Lua status. Native-only anchoring already throws resource
on a failed protected return. All command host adapters use the shared predicate
or direct `LUA_ERRMEM`; no alternate user `xpcall`, coroutine or loading API is
available. No further missing propagation site was found in this bounded class.

## Actual refusal, retry and related API evidence

The independently recompiled [ten-mode memory runner](../../../tests/native/v5-02-x05-a3-c05-review-evidence/memory-probes/commands.json)
passes against reversible private copies of the exact fixed production source.
The runner changes only its output root; [adaptations](../../../tests/native/v5-02-x05-a3-c05-review-evidence/runner-adaptations.json)
reverse exactly. Faults return null only at real observed Lua growth requests;
no quota flag, Lua status or C++ exception is injected.

At original projection2's descriptor reference, initial and emergency retry each
request **24 bytes** and are both refused. The first real nested status is
`LUA_ERRMEM`, with quota/conversion flags false. Normal and later ordinary-error
tails both report exactly `resource_limit / Lua allocation exceeded /
catalog.projections[2]`, `lua_bytes`, maximum67108864. No later callback runs.
Three catch layers observe the real memory status and then two ordinary statuses
with the sticky bit set, preserving the same first category/location.
[Normal](../../../tests/native/v5-02-x05-a3-c05-review-evidence/memory-probes/run-one.log),
[nested later-error](../../../tests/native/v5-02-x05-a3-c05-review-evidence/memory-probes/run-nested3-later.log).

The [retry control](../../../tests/native/v5-02-x05-a3-c05-review-evidence/memory-probes/run-recovery.log)
refuses the first actual 24-byte request, permits the emergency retry of that
same request, completes valid registration and observes no completed memory
error. Direct host failure retains the resource result; caught ordinary error
permits later registration; an unhandled later ordinary error retains `internal`
at module1. The function-reference-only fixture observes **zero actual growth
requests** and succeeds. No function-reference allocation failure is fabricated.
Every destruction checks zero tracked live Lua allocation.

An additional independent [API adaptation](../../../tests/native/v5-02-x05-a3-c05-review-evidence/api-memory-adaptation.json)
extends this exact actual-reference experiment to the other two descriptor APIs:
**nine modes each**, all **18/18** pass. Shape faults retain original
`catalog.shapes[2]`; command faults retain `catalog.commands[7]`, including the
five shipped-command offset. Normal/later-error tails, three catches with both
tails, direct host refusal, real one-null emergency recovery and ordinary-error
controls all pass with zero-live-byte cleanup. Real failing growth requests
remain24 bytes; no later callback replaces the first token.
[Shape commands](../../../tests/native/v5-02-x05-a3-c05-review-evidence/api-memory-shape/commands.json),
[command commands](../../../tests/native/v5-02-x05-a3-c05-review-evidence/api-memory-command/commands.json).
These are controlled allocation refusals, not natural host exhaustion.

The author's unchanged pre-C05 red reference5 reproduction is preserved and was
inspected: normal tail loses resource category at projection2, later ordinary
error loses the field to module1, and both proceed to projection64. Those are
expected failing product detectors. Author development harness compile failure
and its correction remain preserved. The two new native noninjection controls
also pass in the focused suite: nested ordinary errors permit later registration,
while nested quota failure retains original shape65 despite a later error tail.

## Preserved combined behavior and earlier corrections

R1's pre-sort original key checks and canonical-to-original remapping remain;
R2's initial pure provenance typed resource mapping remains. R3's six separate
canonical/payload buffers, distinct framing/digest space, independently derived
13 embedded metadata byte passes/four graph passes, and eleven embedded/three
external source passes remain. The high catalog still measures **1,041,459
catalog bytes** and **1,041,840 manifest bytes**; metadata410 and exact/one-byte-short
remaining-budget probes pass. These are conservative logical charge recipes,
not allocator/RSS accounting.

R4/R5 retain the guard's one shared all-root identity/prototype audit and original
D/F root mapping, nonallocating location token, independently staged module
fallbacks, unchanged root order/depth and publication only after complete
success. Failed sealing owners remain terminal. R6 retains nonallocating
callback/collection/remap selection, sticky first failures, unmatched canonical
boundary fallback, owned global-field moves and staged final catalog swap.
All **69** retained class scenarios, **ten** C03 allocation scenarios and
repeated-root/location runners pass. Deliberate red harness controls retain their
expected nonzero status and are not counted as passing product checks.

The profile VM still structurally verifies and privately copies bounded captured
bytes/modes/order, executes zero-result source-only chunks with native owners,
validates/copies final descriptors and seals D/F before the next module. External
`_ENV` is created from pristine curated state before shipped evaluation, with
constructor-only engine, self `_G` and exactly three registration functions.
Exact native identities enroll before module code; original embedded handlers
retain their engine. Catalog/effective provenance bind original captured inputs.
Private retained-callable dispatch remains verification; profile-owner public
invocation refuses before provider work. No shipped Lua or B/C semantic adapter
change is included.

## Independent verification, identity and release

The [independent command record](../../../tests/native/v5-02-x05-a3-c05-review-evidence/commands.json)
serializes Debug all+planar_tests, inventory, focused execution, all nine private
runners, high catalog, pinned formatting/Doxygen and no-work. Both product builds
report `ninja: no work to do.`. Inventory is **582 unique names**, preserving all
**580** prior names. The exact focused selector executes **209/209**, with zero
failures/errors/skips/disabled cases.
[JUnit](../../../tests/native/v5-02-x05-a3-c05-review-evidence/focused.xml),
[counts and names](../../../tests/native/v5-02-x05-a3-c05-review-evidence/test-summary.json),
[raw log](../../../tests/native/v5-02-x05-a3-c05-review-evidence/focused.log).
Pinned formatting/Doxygen checks **117 active sources**. Retained production,
reachable-native, R1/R2, budget, sealing and attribution runners pass their
explicit expectations. Additional command/shape compilations are serialized
after this scope gate and leave product build identities unchanged.

Author final24 commands, vectors, B2/D6/D6-C01/D7-B historical exact-source audits
and A0 reversible generated-source audit were inspected, not independently rerun
claims. [Scope audit](../../../tests/native/v5-02-x05-a3-c05-review-evidence/scope-audit.json)
reconstructs the exact C05 owned diff. Its only product/test changes are the
small runtime sticky-state correction and two native tests; shipped Lua,
guard/API/build/codec/vendor and all historical evidence remain unchanged.
AGENTS.md remains the CLAUDE.md symlink.

[Before](../../../tests/native/v5-02-x05-a3-c05-review-evidence/before.json),
[after scope tests](../../../tests/native/v5-02-x05-a3-c05-review-evidence/after-tests.json)
and [final release](../../../tests/native/v5-02-x05-a3-c05-review-evidence/final-source.json)
independently verify **23,267 source entries**, complete membership/modes/symlinks,
**400 author artifacts** and **931 build identities**, with zero mismatches.
Source exclusions are root .git/build, C05 author/reviewer evidence and this
reviewer-owned report. Candidate and author evidence remain frozen.

Author identities:

- Source freeze/release: `452c5c5875966054441dd01451fa688e084a3181a66a42b520f436b9c4507eb4`.
- Owned diff: `de4a3c0111d428548b88efa81dd0df23976b977121cab68b0587aba7a3ee5260`.
- Report: `23f61287f88369a1c2efff84ea1e97ee539e318ee7d0a3398371703bcf1f15ef`.
- Artifact manifest: `d4ed84e91568631ffe50d5b85f39b6b5adda0c240cba34918d885ba1040b8ec7`.
- Build identities: `944529f5a3ee3fcc147a4795f41764931247d73aeb82626298fc54f64744b325`.

Review [finalization](../../../tests/native/v5-02-x05-a3-c05-review-evidence/finalization.json)
binds this report and complete review artifact manifest. Serialized build
ownership is released after final hashing; no subsequent reviewer edits follow.

No full582 suite, RelWithDebInfo/release or Linux gate was run. Latest full gate
remains520 per configuration; integrated X05-A verification is next. Public
profile CLI, B/C semantics/adapters, X06 storage/history/interpretation/acceptance,
caller receipt/budget orchestration, mandatory external-profile gate, public
cutover, execution/RA01 and Linux remain open. Public init creates format1.
