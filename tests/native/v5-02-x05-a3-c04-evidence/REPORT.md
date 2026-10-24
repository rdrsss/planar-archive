# X05-A3-C04 — Registration diagnostic attribution candidate

Status: implementation candidate, pending fresh independent combined review.
This corrects the residual A3-C03-R6 diagnostic class. It does not approve itself
or close the external-profile gate. Shared `dev/planar-v2-vernacular` at starting
HEAD `3171c49e7b92d3abda01bfbb8bfb0c6b29d4c67a`; intentional dirty work and all
historical evidence are preserved. The coordinator owns the round; this coder
owns runtime.cpp, external_registration.t.cpp, C04 support/evidence and the six
current reference files recorded in the scope audit. No guard/API/build/codec/
vendor/shipped Lua/historical source changes, Planar tools/skills/queue, installed
binary, operator data, commit, installation, publication or merge were used.

## Preserved red evidence

Before product edits, the immutable C03 reviewer executables were rerun with the
same five modes. [Commands/status](red/commands.json) and raw [register](red/register.log),
[collection](red/collect.log), [remapping](red/remap.log), [valid twin](red/control.log)
and [semantic twin](red/remap-control.log) preserve all three expected exit-1
product detectors and both passing controls. Each actual refusal was a real
32-byte global new request. Valid projection10 in module2 was attributed to
projection9 at callback admission and collection; invalid-ABI projection10 among
11 was attributed to projection11 when canonical remapping allocation failed.
The semantic twin returned original projection10. Cleanup reported zero tracked
Lua allocation on every path. Copies of the immutable main/runtime/delta and
original command records are retained under red/. This was controlled allocation
refusal, not natural host exhaustion or a capability escape.

## Complete location-flow correction

The existing trivial `seal_location`/`pending_seal_location` token now also serves
callback admission, reference retention, final collection and pure-error remapping.
The private names are retained to minimize unrelated change. Index zero means the
already-owned caller boundary; positive indices are original one-based per-kind
positions, counting the five shipped commands and prior external modules.

- Callback admission first checks sticky bridge/resource state before changing
  the first location. Argument validation uses the module boundary. Once arguments
  identify a new attempt, native kind/index selection precedes count charging,
  vector ownership and raw Lua reference retention. Selection does not allocate.
  Successful reference retention clears the token before later Lua/module work.
  A caught callback refusal or raw reference-allocation longjmp keeps the first
  token; later callback retries cannot replace it.
- Final descriptor collection selects the original token before conversion,
  unchecked-key validation, normalization or native descriptor ownership. Stack
  preflight begins with the cleared module token. Native C++ owners still unwind
  before any callback raises a Lua error.
- Catalog validation clears completed entry ownership before path-vector work,
  catalog assembly, sorting, charging, conversion and pure validation. The pure
  error remains in owned storage and is read by const reference. Literal prefix
  views, checked integer parsing, map iteration and descriptor-pointer matching
  select original ownership without copying fields, constructing prefix strings,
  implicitly allocating string lookup keys or rendering remapped text. Unmatched
  canonical entries keep the caller boundary. Only already-truthful non-entry
  pure global fields move their owned storage into the result path.
- All protected registration-call failure adapters render the token outside Lua.
  Formatting publishes once by swap. If the actual field allocation refuses, the
  existing outer bad_alloc adapter returns the external resource diagnostic with
  the preinstalled module fallback. Outer constructor adapters continue moving
  owned field strings into the returned error. Ordinary diagnostic categories
  and exact messages are unchanged.
- C03's independent staged module strings, fallback swaps, shared guard traversal,
  seal ordering, publication and first-failure terminal tokens remain intact.
  A separately staged global `catalog` string installs by nonallocating swap
  before final provenance work. Its object and conservative 32-byte payload are
  charged before construction under the existing cumulative native metric. This
  final short-string assignment change is preventative consistency; no allocating
  failure at that old assignment is claimed. It adds no quota or RSS claim.

The only remaining public field writes are initial `profile` ownership, move of
an owned pure global field, outer render swap, staged module/fallback swaps and
staged final catalog swap. No boundary depends on SSO for correctness. The pinned
library does use zero native requests to render the short command/shape field
strings observed by these probes; those are recorded honestly as zero-request
controls rather than hypothetical refusal tests.

## Regression coverage and probe policy

[fields.py](../v5-02-x05-a3-c04-support/fields.py) creates an exactly reversible
private production copy and records substitutions, hashes, commands and outputs.
The class matrix covers all three kinds at original indices10/64, five-command
offset, earlier modules and mixed kinds. Remap cases include projection10 among
11 with only ABI10 invalid. Every private owner destruction checks zero tracked
Lua allocation.

The 69 scenarios comprise 60 per-kind/index windows and nine phase/adapter
controls. Removed callback/collection selectors and the full pure-error remapping
window arm replacement global new around actual production operations and observe
zero actual requests. Valid twins succeed; malformed-key and ABI/family controls
return exact original fields. Remaining projection outer formatting refuses its
actual 32-byte request and returns `profile.modules[2]`; short command/shape output
windows observe zero requests and retain exact ordinary fields. Controlled native
remaining-budget tightening exercises real callback charge refusal.

Additional controls cover actual prevalidation allocation refusal with module2,
zero-request final global-field installation, actual final provenance preparation
allocation refusal with `catalog`, and a callback vector-allocation refusal caught
by Lua followed by further registration/error attempts. The latter privately
shrinks vector capacity before arming a real allocation refusal; the first
projection10 token survives, and a second actual outer-render refusal falls back
to module2. These are controlled capacity/budget tests, not natural exhaustion.

Explicit pure-result adapters exercise an unmatched shipped canonical entry and
an owned global resource field. A controlled Lua status/exhaustion adapter after
retaining the descriptor reference exercises the raw-ref failure path, with and
without a real outer-render allocation refusal. These are adapter probes; they
do not claim natural validator failure, real Lua allocator growth refusal at that
reference operation, or that realloc throws C++ exceptions.

Two additional Catch cases cover successful callback reset, malformed/caught
callback retries followed by different registration calls, subsequent module
errors, and ordinary original-index remapping at10/64 for every kind.

Historical retained runners were copied into C04 support with output roots
changed. The C03 seal-only allocation runner additionally resets a private phase
flag from the protected-call kind before every call and scopes its allocator
predicate to actual sealing now that the token also exists in earlier phases;
its removed-selection anchor names the seal-root site unambiguously. Original
runners remain unchanged. [Adaptations](../v5-02-x05-a3-c04-support/adaptations.json)
record exact source hashes/deltas. Repeated sealing, pre-root/partial-freeze/union
locations, production dispatch, reachable natives, R1/R2 exact diagnostics and
high/exact/one-byte-short charged-native probes remain required in the final gate.

Development attempts are preserved. The first field runner incorrectly required
a real allocation for short command output; its actual zero-request result led
to honest conditional expectations. An unmatched-canonical fixture initially
selected canonical command1, which was a real external command, and correctly
returned command6. The fixture was corrected to canonical shipped export3; no
product change was needed. All failing logs/deltas/commands remain available.
No failures were removed or overwritten.

The first frozen final gate completed successfully, including 207 focused cases,
all probes, vectors, historical audits, lint and no-work checks. A read-only
inspection found that the copied allocator runner's new phase flag stayed true
into later modules. The coordinator authorized correcting this verification-only
predicate after that gate completed. The corrected runner resets the flag before
every protected call and passed its ten scenarios again before final-2.
[Supersession record](final-1-supersession.json) proves unchanged product, test and
reference bytes. The complete first gate is preserved as superseded, and final-2
repeats the authorized frozen focused gate. Only the copied runner and adaptation
manifest changed between those source snapshots; original historical runners
remain immutable. The additional freeze is documented rather than hidden.

## Frozen focused verification

Final gate results and source/artifact identities are recorded below after the
serialized frozen candidate run. Full-suite, RelWithDebInfo/release and Linux
verification are deferred until independent combined approval. Latest full gate
remains 520 per configuration. Public profile CLI, B/C adapters/semantics, X06
profile storage/history and acceptance, whole-invocation receipt/budget
orchestration, mandatory external-profile gate, public cutover, execution/RA01 and
Linux remain open. Public init continues creating format1.


Final-2 completed all **23 recorded commands** successfully. Debug all plus
planar_tests reports no work; inventory is **580 unique names**, preserving all
578 prior names. The exact focused selector executes **207/207**, with zero
failures/errors/skips/disabled cases. [Command record](final-2/commands.json),
[JUnit](final-2/focused.xml), [summary and matched names](final-2/test-summary.json),
[inventory preservation](final-2/inventory-preservation.json) and [raw output](final-2/focused.log).
All **69 class scenarios** pass, as do the ten retained C03 allocation scenarios,
repeated-root, location, production, reachable-native, R1/R2 and high/exact/one-byte-
short budget runners. Four vectors and B2/D6/D6-C01/D7-B source audits pass; A0
reversible source generation is checked exactly. Pinned formatting/Doxygen checks
**117 active sources**; the final all+planar_tests build reports no work.

[Release audit](release-audit.json) verifies 21700 complete source entries
and 931 build identities with zero mismatches. Source membership, modes and
symlinks match the final-2 freeze through release; AGENTS.md remains the CLAUDE.md
symlink. [Owned diff](final-2/owned.diff) and [scope audit](final-2/scope-audit.json)
cover only authorized product/tests/support/current references. Shipped Lua,
vendor, guard/API/build/codec and historical evidence remain unchanged.
[Finalization](finalization.json) binds this report, complete artifact manifest,
source freeze/release, owned diff and build identity hashes. No source or evidence
edits follow finalization and build-ownership release.
