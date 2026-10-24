# V5-02-X05-B3-B-D01-B independent review

Disposition: **approve**, limited to the production reserved JSON decode codec
prerequisite. No blocking finding remains. This approves optional
`codec_decode_reserved`, its request-local parser/checker accounting, checked
immutable-conversion reservation and both decode entry points' allocation-safe
diagnostic adaptation. It does not approve native runtime integration, B3-B
accounting, request423 closure or the B milestone.

The review used shared `dev/planar-v2-vernacular`, HEAD
`3171c49e7b92d3abda01bfbb8bfb0c6b29d4c67a`, under exclusive Debug ownership.
The active guide, implementation/roadmap, v5 direction, exact X-LIMIT contract,
full D01-B authorization, approved D01-A prototype/review/conversion proposal,
approved B2-B+C01 encode boundary and incomplete B3-B checkpoint informed the
review. Candidate, product, support, vendor and historical evidence remained
read-only. Writes are confined to this report, new
`tests/native/v5-02-x05-b3-b-d01-b-review-evidence/`, and ordinary serialized Debug
gate outputs. Shared compiled inputs remained fixed through isolated private
compilation. Other agents' work was not reverted. No Planar tools, legacy
operational skills/roles/queue, installed binaries, operator data, commits,
installation, publication or merge were used.

## Parser interception and ownership

The [allocator edge](../../../src/engine/json_codec/codec.cpp#L27) checks
`count*sizeof(T)`, calls the current request's nonowning charge adapter, then
enters the ordinary allocator. Denial throws the moved owned `codec_error`
before allocation. Freed or abandoned buffers receive no refund. There is no
process-global/thread-local routing, fixed observer arena, input multiplier or
new product quota.

The [decode-only wrappers](../../../src/engine/json_codec/codec.cpp#L49) bind each
string/vector before delegating its grammar to the unchanged pinned Glaze reader.
Default variants and default pair keys are empty and cannot grow before their
reader binds. The seven alternatives and order are unchanged. Pinned
`json/read.hpp:4801–4859` performs the same speculative alternative/depth/iterator
rewinds; partial buffers remain charged and unwind normally. The instantiated
array and pair-range readers at 2301–2620 own only their supplied vectors and
inline children; string/escape work at 887–1345 grows the supplied strings.
Numeric readers use scalar/fixed fast-float storage, without owned spelling or
locale workspace. No product parser or escape implementation was added.

Allocator copies/rebinds retain the request pointer. Pinned libc++ string
move-assignment (`string:2853`) and vector move-assignment
(`__vector/vector.h:945`) release old storage before propagating the allocator.
The duplicate set explicitly binds its allocator; `__hash_table:1747/1837` routes
bucket and node requests through its rebound allocator. Borrowed key views remain
stable until the set dies. `allocation_state` precedes the parsed graph in the
[call scope](../../../src/engine/json_codec/codec.cpp#L678); parsed/partial values
and checker owners die before the state, including typed denial and upstream
allocation failure.

Glaze `core/read.hpp:135–213` reads the supplied unpadded string_view without an
input copy. The instantiated graph never grows context `scratch/current_file`.
The unchanged strict `validate_json` pass uses `skip.hpp`/`parse.hpp` pointer,
scalar and bounded stack state, with no owning destination or heap workspace.
D01-A's source proof still applies to this concrete production reader graph;
actual-request observation independently corroborates it. The result retains no
allocator/context pointer. Nested independent calls have their own state; input
and context stability through callbacks and prohibition of reentrant input
mutation are explicit trusted C++ preconditions. No Lua longjmp is permitted.

## Complete immutable conversion capacity

The [129-frame workspace](../../../src/engine/json_codec/codec.cpp#L330) measures
only the privately owned parsed graph after structural/schema/strict validation.
Root depth0 through a scalar at depth128 fits; a 129th container refuses during
parsing. The iterative walk uses checked additions/multiplications and scalar
UTF-8 checks without heap allocation. Each source occurrence is counted once.
No externally mutable alias or caller-visible detached sizing plan exists.
The conversion callback runs once after sizing and before any persistent value,
control-block, container, key or text allocation.

The byte recipes match the actual pinned implementations, independently checked
against source rather than inferred from successful totals:

- `shared_ptr.h:650–669` allocates exactly one complete
  `__shared_ptr_emplace<value,allocator<value>>` per visit. Its sizeof includes the
  value, control state and allocator; inline members are not added twice.
- `map:970–973` and `__tree:923–934/2042–2046` select and allocate exactly one
  `__tree_node<__value_type<string,value_ptr>,void*>` per object entry, including
  key/shared-pointer objects and tree links. Piecewise key construction uses the
  node directly; there is no temporary key buffer or sorting workspace.
- `vector.h:1016`, `__split_buffer:831–843` and `allocator.h:97–101` reserve exactly
  the requested `entries*sizeof(value_ptr)` storage. Appending the already
  measured number of children does not grow the destination vector.
- Empty/SSO strings require no separate heap buffer. `string:2182–2225/2280–2307`
  derives each long allocation by aligning `length+1`, including the exceptional
  min-capacity adjustment and actual ABI-layout/endian factor. Direct key
  construction and value-string reserve-from-empty use this same path. Subsequent
  assignment fits the reserved capacity and does not take the amortized growth
  path. Terminators/rounding are counted, inline string objects already are.
- The workspace charge is `sizeof(decode_workspace)`, including fixed frames,
  partial ordinary owners, completed shared ownership, depth and summary. It is
  a logical traversal-ownership allowance, not a compiler stack-frame or RSS
  measurement. There is no heap traversal container.

[Source checks](../../../tests/native/v5-02-x05-b3-b-d01-b-review-evidence/source-checks.json)
bind the additional split-buffer/allocator-traits/fast-float identities; the
14 author-pinned source identities also match. The measured sum is checked before
the hook. Hard input/node/depth clamps bound every admitted traversal and size;
no larger public limits or test-only product knobs bypass those bounds.

The [builder](../../../src/engine/json_codec/codec.cpp#L423) uses ordinary
allocators throughout. Lists reserve before fill, keys construct directly, and
texts reserve from empty. Partial workspace owners unwind on every failed node,
key, string or container request. On success, all mutable workspace owners die;
the returned graph has ordinary immutable shared ownership and outlives request
state. Actual aggregate persistent requests equal `total_bytes-workspace_bytes`
across the retained capacity corpus and new mixed deep/wide trees.

## Ordinary behavior and typed failures

The original decode signature/defaults and original ordinary reader/conversion
remain. Node-before-whitespace, depth, scalar/schema, duplicate, furthest-offset
and strict-validation ordering is preserved in the reserved reader. Differential
checks compare complete graph contents and all error fields, including message,
limit, maximum and offset. New frame traversal and exact allocation choices do
not change accepted JSON values. [Byte comparisons](../../../tests/native/v5-02-x05-b3-b-d01-b-review-evidence/source-checks.json)
confirm the complete approved encoder implementation and encode entry-point tail
are unchanged.

Supplied parser denial and conversion denial move their owned `codec_error`
unchanged, including long message/limit and offset73, without a new propagation
allocation. Callback bad_alloc/length_error and actual allocator failures produce
the existing effective `input_bytes` resource diagnostic. Unexpected exceptions
produce a redacted internal diagnostic. The outer allocation boundary encloses
all early guards and inner diagnostic handlers in both decode entry points.
Real diagnostic allocation faults are distinct from callbacks that throw.

The saved prechange executable independently reproduces both inherited ordinary
decode escapes (null/schema catch adaptation and oversized early guard): its
`CHECK(escaped)` case passes two assertions. Current ordinary/reserved controls
return typed resource failures at the corresponding faults. A reviewer-only
runner first expected exit42 from the label "red"; actual exit0 correctly matched
those escape assertions. The original command/XML and corrected expectation are
both retained. A results-summary helper also initially assumed every JUnit root
was `testsuites`; its first source and diagnostic are preserved before handling
CTest's `testsuite` root. Neither helper issue was a product test failure or
required candidate edits.

Final fallback diagnostics may allocate, and C++ exception-runtime storage is
separate. This approval covers controlled one-shot refusal with later allocation
permitted, not persistent exhausted-host-heap recovery or process RSS accounting.

## Independent verification and exact identity

The [new reviewer cases](../../../tests/native/v5-02-x05-b3-b-d01-b-review-evidence/extended.cpp#L442)
add mixed map/list trees at every depth0–129, frame reuse with siblings, a 257-key
wide object, and list widths through4095. All actual upstream allocation ordinals
and every parser-hook ordinal of a depth20 mixed tree and 17-key long-text object
are separately failed; prior allocations are freed and no denied request enters
the allocator. These supplement the author's partial/duplicate/numeric controls.
Exact-prefix/tightened node/depth/byte combinations and1400 deterministic mutations
exercise competing schema/resource precedence. Optional-hook combinations,
conversion-callback nesting with an independent denied decode, retained child
ownership, and actual early/schema/syntax/depth/node diagnostic faults verify
context isolation and effective byte maxima.

Private commands recursively expand response files and isolate object,
dependency and potential PCM outputs. The test-only global-new observer/fault
injector verifies requests/cleanup; it never routes product resource ownership.
Production nested/concurrent tests run without active observation. The exact
[commands and raw gate](../../../tests/native/v5-02-x05-b3-b-d01-b-review-evidence/gate/commands.json)
and [result checks](../../../tests/native/v5-02-x05-b3-b-d01-b-review-evidence/results.json)
record:

| Independent check | Result |
| --- | --- |
| Focused caller gate | **375/375**, zero failures/errors/skips |
| Supplemental history codec | **11/11**, zero failures/errors/skips |
| Production codec | **18/18**, 413 assertions |
| New private reviewer cases | **5/5**, 299167 assertions |
| Retained author private cases | **5/5**, 155950 assertions |
| Combined private executable | **10/10**, 455117 assertions |
| Inventory | **641**, preserving all637 prior names |
| Shape/common canonical/record/request/effective-provenance vectors | Pass |
| Six exact source audits | Pass |
| Pinned formatting/Doxygen | Pass,118 active C++ sources |
| Reviewer source formatting | Pass |
| Initial/final all + planar_tests builds | `ninja: no work to do.` |

All passing private selections have zero failures/errors/skips. Inventory is not
a full641-case run. No release or Linux gate was run; latest full approved gate
remains A4's582 per configuration. The pre-existing B3-B WIP names are preserved,
without independently approving their incomplete runtime composition.

[Scope verification](../../../tests/native/v5-02-x05-b3-b-d01-b-review-evidence/scope-verification.json)
reconstructs the candidate diff, verifies all16 candidate members and98 author
artifacts, and checks all6312 review-protected source/vendor/release/checkpoint/
author/shared-compiled entries unchanged. The author's5310-entry protected
baseline differs only at the three authorized codec files; all53 B3-B checkpoint
artifacts, runtime source and release content remain fixed. AGENTS remains the
symlink to CLAUDE. These are explicit scoped identities, not a whole-tree claim.

| Reviewed author identity | SHA256 |
| --- | --- |
| Candidate manifest | `e4c0f606852b51128aabb08614aa5efd19d5a4b99a57c50f3d50c7952eaffd89` |
| Candidate diff | `5114d99baaed6cc99ea6a32441d32dc7fb002f671ee0f3887cb52996d7d95b3d` |
| Artifact manifest | `9296d23b2c171dc69100d360bb326fa7a592b8ec54323498fd304f7f997e2a36` |
| Author report | `7c5f46663cc7aa3056073810cf6201a4eeef31766b86fa9253820e32bb43a0d5` |

Review finalization binds this report and all review artifacts, rehashes protected
sources and the exact candidate, and releases Debug with no later reviewer edits.
Fresh B3-B integration/search/coder/reviewer work must still consume the seam,
prove cumulative runtime accounting, first-error ownership and stable-reference/
GC/partial-setup behavior, close request423 and complete its agreed gate. B3-C W,
B4 render, X06–X08, public profile admission/cutover, mandatory external-profile
acceptance, execution/RA01 and Linux remain open. Public init creates format1.
