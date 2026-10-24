# X05-B3-B plus R01 native semantic integration candidate

Status: candidate awaiting fresh independent review of the combined original
B3-B and R01 implementation. No self-approval or full milestone/release gate is
claimed. Work uses shared dev/planar-v2-vernacular at
3171c49e7b92d3abda01bfbb8bfb0c6b29d4c67a. The intentional rewrite and others'
work remain. Writes are limited to runtime.cpp, external_registration.t.cpp,
new R01 support/evidence and six authorized current prose paths. runtime.cppm
belongs to the preserved original B3-B candidate and is unchanged in R01.
The coordinator owns the round record. AGENTS remains the CLAUDE.md symlink.
No Planar tools, legacy operational skills/roles, installed binaries, operator
data, commits, installation, publication or merge were used.

## Combined composition and authority

The original internal validate_profile operation retains exactly the three
trusted shape_prepare/check/finish exports before popping their export table.
seal_environment explicitly passes all three alongside shipped handlers,
globals and the external environment into the original shared audit/freeze
before external modules execute. Their retained Lua references survive module
registration, protected calls, GC, moves and failure cleanup. Exact native
identity enrollment and the existing sealed-cell guard remain authoritative.

Native final descriptors supply the shape IDs/stage policy. The caller supplies
only the existing bundle/capture validation record: no phase/export selector,
checked result, definition or callable context is accepted. Native owns prepared
values, requests, checked results and same-call definitions across sequential
protected top-level prepare/F/check/finish calls in the original profile VM.
There is no recursive public VM call from a Lua frame. Input and returned graphs
are copied before transient stack/proxy cleanup; no stack index spans calls.

Prepare finishes all-job mandatory validation and base-cycle checks before the
first F. Each real invoke_shape resolves the native final descriptor and exact
selected-source definition before its retained F executes. Check validates each
complete result and cumulative count before the next F. Semantic refusal or
malformed return stops later dispatch. Finish receives matching native-owned
index/job/shape definitions and checked values, preserves original graph edges,
adds grouped unit obligations and forms selected_shape. Original production
cases cover exact metadata, immutable F/native inputs, native stage policy,
all-job ordering, fail-fast behavior, cycles, injection, embedded equivalence,
moves/repeats and first terminal failures. The new production case compares the
complete encoded ordinary JSON schema error to embedded validation, then runs
healthy semantic operations on that same owner.

Generic profile invocation still refuses before provider effects. This operation
accepts no provider interface. W, render, selection, CLI/history/acceptance,
codec interfaces, Lua, vendor and CMake are unchanged by R01.

## Preallocation and sticky resource propagation

Only semantic_active selects codec_decode_reserved. Its parser/checker hook
charges every actual request before allocation; its conversion hook charges the
complete immutable reservation before persistent ownership. Both are synchronous
C++ expected-return callbacks. They call no Lua API and cannot longjmp across
native ownership. Deallocations and abandoned parser alternatives receive no
refund. Ordinary embedded decode still calls codec_decode with unchanged defaults.
The original validate pass remains after decode: its node/graph/transport charges
conservatively overlap the concrete codec ownership reservation. No speculative
input-length multiplier, new quota or replacement capacity estimate is used.

The audit also corrected two small phase-data staging gaps. Semantic codec error
data uses direct node/text/field construction, charging each actual pinned
make_shared control/value block, map tree node and long string buffer before
construction. String reserve-from-empty then assign and in-place map key
construction match D01-B's reviewed recipes. Keys are string_views until the
charged node constructs them; no initializer-list graph or intermediate owning
string copies are used. Partial graph construction unwinds before Lua error
raising. The embedded codec_failure branch remains exact. Successful semantic
encoding separately charges its native string-value/control wrapper before
codec_text; the already reserved encoded buffer moves without another buffer
charge. Existing subsequent validation remains conservative transport accounting.

Runtime now includes <map> and <memory> in its global fragment solely to expose
pinned libc++ internal allocation identities, as explicitly authorized. The
implementation uses the same __shared_ptr_emplace<value,allocator<value>> and
__tree_node<__value_type<string,value_ptr>,void*> sizes as D01-B. import std remains
the standard product API surface. Source Doxygen explains these concrete staging
boundaries. This is a narrow pinned-ABI dependency, requiring fresh review; there
is no guessed fallback constant. String alignment, terminator and exceptional
short/long encoding match the pinned libc++ layout/endian conditions. Separate
checked payload/alignment charges prevent overflow before summation or allocation.

Semantic codec resource errors bypass ordinary error data and retain the first
owned conversion_failure. Native allocation adaptation uses existing terminal
state if diagnostic ownership itself fails. The operation's RAII semantic flag
unwinds on success, ordinary semantic refusal and native failure. Existing shared
shape_failure ownership preserves code/message/limit/maximum/logical field through
subsequent APIs, fallible copies and retry. These are controlled one-shot/native
counter guarantees, not persistent exhausted-host-heap or RSS guarantees.

## Exact reference, membership and live heap evidence

The reversible observer executes actual product semantic functions and actual
registered F. It observes a two-module mixed fixture containing a selected and
unused shape, command and projection. All16 exact reference number/type/object
bindings are checked: five shipped handlers, three phases and four external
D/F pairs. This is the fixture's complete retained-reference set, not a claim
that every possible catalog has sixteen references.

Every constructor/phase/F cleanup boundary compares complete tag/backing key-value
membership, including value type, integer tag and backing object identity, to
its original constructor baseline. Counts alone cannot pass. Full Lua GC at
boundaries checks live heap, and lua_close must reduce native live-Lua accounting
to zero. The first snapshot's bookkeeping overhead is reported separately from
subsequent stable calls. Partial snapshot setup intentionally refuses growth
after the first pointer snapshot is published: original tag/backing membership
and all references survive, baseline_ready remains false and terminal teardown
releases the partial bookkeeping. No input proxy has been published at that point.

Successful repeated complete operations keep exact membership/reference identity
and a stable post-warmup live Lua heap while nodes/native charges grow and
instructions never decrease. The initial invocation transition resets instructions
once; no later phase/F or repeat resets/refunds work. Short phases are not assumed
to individually consume a sampled instruction tick. Separate real repeated
operations reach the original instruction and cumulative native ceilings.
Controlled node/Lua failures and parser/conversion hook denials retain the first
typed terminal error, restore semantic_active and prevent F dispatch. Error-data
staging controls deny node/map/string storage at the concrete remaining-budget
boundary; exact-one-node capacity allocates that one node and refuses the next.
A valid semantic refusal exercises the encoded-string wrapper; its denial stops
before a later F. Actual operator-new observations check request counts/sizes at
each charged staging construction and zero denied data allocations.

## Historical request423 and allocation controls

Original B3-B development-probes01/allocation.xml remains the actual failing
request423 evidence: ordinary codec allocation failure became semantic data.
The original checkpoint already contains its scoped sticky correction. R01's
baseline request423 control therefore passes; it must not be presented as a new
red regression. Current request423 also passes with the same first-error/retry
checks. No assertion is made that ordinal423 still names the original decoder
allocation after reserved decode changed allocation order. The final current
sampler explicitly includes400–450 in addition to early, evenly spaced and tail
requests. It samples actual one-shot global-new refusals against the production
library, not a replacement decoder. The decoder quota hooks and concrete staging
probes provide separately labelled refusal coverage.

## Verification and candidate identities

The final frozen Debug gate passed, with zero failures/errors/skips/disabled
cases in every selected suite:

| Check | Executed result |
| --- | --- |
| Focused CTest gate | 376/376, 192.945 seconds |
| Supplemental history/codec | 11/11 |
| Production semantic selection | 11/11, 283 assertions |
| Production codec control | 18/18, 413 assertions |
| Reversible semantic/lifetime/resource/staging modes | 16/16 runs, 383 assertions |
| Production actual allocation sampling | 1/1, 1246 assertions; 137 one-shot refusals from5618 observed requests |
| Checkpoint/current request423 controls | 1/1 each,22 assertions each; both pass |
| Inventory | 642, preserving all641 prior names |
| Shape/common canonical/record/request/effective-provenance vectors | Pass |
| Six exact source audits | Pass |
| Pinned formatting/Doxygen | Pass,118 active C++ sources |
| Support formatting | Pass |
| No-change all+planar_tests build | ninja: no work to do. |

All16 fixture reference identities and exact47 tag/267 backing entries remain
constant. After GC, the success fixture starts at391899 live Lua bytes and
settles at428947 after its first complete operation, remaining exactly428947
through operation25. This37048-byte bookkeeping/VM warmup difference is not represented as leaked
per-call data. The partial
snapshot probe retains393451 bytes with ready=false, then closes at zero.
The instruction fixture reaches100000000 at operation48; the ordinary repeated
fixture refuses cumulative native work at operation112 with native_bytes
536870827 before a request that would exceed536870912. Counters are not reset
between protected calls. These are deterministic work ceilings, not host RSS.

The actual healthy ordinary codec error graph allocates six112-byte control/value
blocks, five72-byte tree nodes and two32-byte copied buffers:1096 concrete bytes.
The encoded-text wrapper allocates one112-byte control/value block while moving
the reserved buffer. The staging observer verifies these exact request sizes;
near-budget denials do not enter their denied data allocation. A one-node exact
remaining-budget control enters precisely one112-byte allocation before the next
node refuses. Original terminal details survive retries in every failure mode.

Every private compile preserves the exact membership and hashes of748 shared
object/PCM/library files. The2094-entry pre-gate source/vendor/support/current-
prose freeze rehashes identically after the complete gate. The174-entry original
baseline separately protects approved codec and historical support/evidence;
all entries outside the eight owned existing paths remain unchanged.
The gate preserves all641 existing names and adds one production case; inventory
is not a full642-case run. Required focused/supplemental XML, raw command logs,
per-configuration source audits and stable-source checks bind the final candidate.
No release or Linux run is claimed; latest full approved gate remains A4's582 per
configuration. B3-C W, B4 render, X06–X08, public profile admission/cutover,
mandatory external-profile acceptance, execution/RA01 and Linux remain open.
Public init creates format1.

Private compile commands recursively expand response files and redirect object,
dependency and possible PCM outputs. Exact shared object/PCM/library membership
and hashes are checked around every compile. Owned runtime copies reverse exactly
to the frozen production bytes; they never replace shared product libraries.
The original checkpoint, codec source and D01-B evidence hashes are rechecked.
Combined candidate.diff starts at the original B3-B baseline; r01.diff separately
starts at the preserved R01 baseline. Candidate/source/artifact manifests and
finalization bind the final report, release Debug and prohibit later edits.

Development helper failures are preserved: an initial substring transform match,
private link missing the interface vtable, and a thirty-call instruction test that
correctly had not exhausted100M were corrected in support only. One attempted
Python edit had quoting syntax errors and made no source edits. The first frozen
gate stopped before tests because the new selection lacked CTest's replacement.
prefix. The next gate was explicitly interrupted to add required concrete
error-graph/wrapper precharges. Neither is claimed as a completed focused pass.
The first concrete recipe compile exposed hidden libc++ ABI types under import
std; the authorized narrow global-fragment includes resolved that build issue.
Raw development logs remain; current candidate results supersede these attempts.
