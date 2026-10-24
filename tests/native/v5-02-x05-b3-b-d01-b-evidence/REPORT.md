# X05-B3-B-D01-B production reserved decode candidate

Implementation complete; **independent review pending**. The optional
`codec_decode_reserved` charges every actual request-local parser/duplicate-check
allocation before upstream allocation, then derives checked immutable output
capacity without allocation and reserves it before persistent construction.
Returned values retain ordinary immutable ownership independent of the request.
This is the codec prerequisite only: no runtime consumption, B3-B accounting,
request423 closure or milestone approval is claimed.

Shared checkout is `dev/planar-v2-vernacular`, starting HEAD
`3171c49e7b92d3abda01bfbb8bfb0c6b29d4c67a`, preserving all pre-existing rewrite
changes and incomplete B3-B source/evidence. The coordinator owns the round.
Writes are limited to codec.cppm/cpp/t.cpp, new D01-B support/evidence, and six
explicitly owned current prose paths. Other agents' work was not reverted.
No runtime, Lua, vendor, CMake, CLI, history or installed binary was edited;
no Planar tool/skill/role/queue, operator data, commit, installation, publication
or merge was used. AGENTS remains the symlink to CLAUDE.

## Mechanism and conversion proof

The [complete derivation](../v5-02-x05-b3-b-d01-b-support/source-and-capacity.md)
binds the implementation to the exact pinned library identities. Decode-only
allocator-aware strings/vectors bind the synchronous per-call state immediately
before unchanged pinned Glaze readers can grow them. Variant alternatives/order,
node/depth/scalar/schema/duplicate policy, furthest offset and later strict
validation remain exact. Rebound hash allocators cover duplicate nodes/buckets.
Glaze owns all grammar/escaping; no parser, global allocator routing, observer
arena, guessed multiplier or new quota appears in product code. Every abandoned
or freed temporary request remains cumulatively charged.

The fixed129-frame sizing walk and iterative builder share one concrete workspace.
Sizing validates copied text and counts each private parsed occurrence. Capacity
uses sizeof the actual pinned make_shared control block and map tree node, exact
reserved list slots, and derived empty-string/direct-key long capacity with
terminator/alignment and layout/endian adjustment. Inline string/shared-pointer
objects are already inside those allocations. No geometric output growth occurs.
The actual workspace sizeof includes all retained construction owners, completed
shared_ptr, source/index frames, depth and summary; its logical charge is not a
compiler stack-frame or RSS claim. The conversion callback runs after checked
sizing, before any persistent value/control block/container/key/string allocation.
No caller-visible detached plan or mutable parsed alias exists.

Both callbacks are optional, synchronous and nonowning. Input/context remain
stable through the complete call; no reentrant mutation through aliases is allowed.
Independent nested/concurrent calls use independent request state. Parser/checker
owners die before their allocation_state; standard returned ownership retains no
request pointer. Returned success and moved long typed refusal survive owner death.

Typed callback codec_error moves unchanged, including supplied offset. Callback
bad_alloc/length_error and actual allocation failures become existing input_bytes
resource errors; unexpected exceptions become redacted internal errors. An outer
typed boundary protects early guards and all inner catch diagnostic construction
in both decode entry points. Ordinary codec_decode signature, defaults, original
reader/conversion, normal errors/precedence/offsets and approved encode behavior
remain unchanged. C++ exception-runtime storage and final fallback allocation do
not imply persistent exhausted-heap recovery.

## Verification

Final verification runs after freezing all16 product/test/support/current-prose
members in frozen-source.json. Exact commands, selections, raw logs and XML are
under [final-gate](final-gate/commands.json). No code/support/prose edits follow
that freeze; final evidence/report/manifest writes are recorded separately.

| Frozen Debug check | Result |
| --- | --- |
| Direct serialized all + planar_tests build | Pass |
| Inventory |641, preserving all637 prechange names |
| Focused relevant callers |375/375, zero failures/errors/skips |
| Supplemental history codec |11/11, zero failures/errors/skips |
| Direct codec cases |18/18, including four new production cases |
| Isolated actual-allocation controls |5/5 cases,155950 assertions |
| Canonical/record/request/effective-provenance/shape vectors |Pass |
| Existing six exact source audits |Pass |
| Pinned formatting/Doxygen |Pass,118 active C++ sources |
| Pinned support formatting |Pass |
| Final no-change all + planar_tests |`ninja: no work to do.` |

The focused375 selection preserves the prior361 caller cases and adds all ten
unapproved B3-B WIP cases plus four reserved-decode production cases. The separate
11-case history codec selection is retained exactly.641 is inventory, not a full
641-case run; last approved inventory remains627, latest full approved gate A4
remains582/configuration. No full B, release or Linux gate is claimed.

The [private actual-allocation probes](../v5-02-x05-b3-b-d01-b-support/probes.cpp)
compare accepted graph content and complete ordinary diagnostic fields/offsets over
scalar/numeric/null, all escapes/surrogates, malformed UTF-8/NUL, empty/mixed/deep
containers, decoded duplicates, loose separators, partial/trailing input, deterministic
mutations and tighter byte/node/depth combinations. Long/1MiB strings,
100000/+1 nodes and depth128/+1 retain ordinary behavior.0..1024 key/value lengths
cover every short/long/alignment transition there; larger cases include4095,
4096,65536 and1048574 payload bytes.

For successful cases every observed pre-conversion allocation matches the exact
parser/checker charge sequence; sizing itself allocates nothing. Every observed
persistent allocation sum equals measured total minus actual logical workspace.
This corroborates the source recipes rather than estimating from input length.
The mixed nested fixture has18 actual parser requests and20 conversion requests;
all38 actual upstream ordinals are separately failed. Partial/duplicate/numeric
fixtures add3/7/2 actual ordinals. All return typed input-resource refusal and free
earlier allocated owners. Each hook ordinal is separately denied before upstream
allocation, preserving long first refusal/offset73 and freeing prior owners.
A scalar conversion denial makes zero new requests even with the next actual
allocation armed to fail. Successful returned graphs and long4096-byte message/
1024-byte limit remain owned after request-state destruction. Production cases
also exercise independent nested/concurrent calls and cumulative repeated state.

Callback exception controls are distinct from actual one-shot global-new faults,
including real diagnostic allocation faults inside early and catch adapters.
The isolated observer/fault injector only verifies allocations/cleanup; it never
selects request state or participates in reservation. It is disabled for the
independent production concurrent tests. Final private commands recursively expand
response files and isolate object/dependency/potential PCM outputs; shared compiled
object/PCM/library membership and hashes remain fixed around private compilation.

[Baseline red reproduction](red/commands.json) privately compiles the exact saved
ordinary codec implementation, without replacing shared objects. Actual first
allocation refusal escapes std::bad_alloc for null/schema catch adaptation and
oversized early guard; two expected-red assertions reproduce those gaps. The
production private controls return typed resource refusal at both boundaries.
This is an inherited ordinary diagnostic gap covered by the authorized decode
outer boundary, separate from the new API/conversion implementation.

## Development attempts and scope preservation

Build01 compiled the initial product API/conversion. Build02 added the four
production cases;18/18 passed. Attempt01 private compilation failed only on a
copied range-loop variable under -Werror; its raw source/commands/error remain.
Attempt02 corrected that harness warning and passed all five cases. Intermediate
Doxygen checks reported missing internal/member/parameter documentation; those
logs remain, and documented final source passes. Build03 moved the temporary
completed pointer into the explicit workspace; the final gate additionally uses
the exact pinned string layout/endian capacity adjustment. No failed product
behavior was discarded or weakened.

Protected-baseline.json binds5310 product/vendor/release/B3-B evidence paths.
Finalization permits only the three owned codec source changes within that set,
checks exact remaining membership/hashes, verifies all53 B3-B checkpoint evidence
files, original runtime source and every release file unchanged, and checks the
reviewed D01-A author/reviewer artifact manifests plus prior support candidates.
Current prose is explicitly authorized to change and is not claimed byte-identical
to earlier checkpoint prose. Candidate diff compares owned current files against
prechange copies; support additions, artifacts and report are separately bound.

Finalization binds candidate-manifest.json, candidate.diff, artifact-manifest.json
and this report; it rechecks the frozen candidate and protected content and
releases Debug with no later coder edits. Independent codec review precedes fresh
B3-B native integration agents. B3-B/request423, B3-C W, B4 render, X06–X08,
public profile admission/cutover, mandatory external-profile acceptance,
execution/RA01 and Linux remain open. Public init creates format1.
