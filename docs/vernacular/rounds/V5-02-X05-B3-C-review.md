# V5-02-X05-B3-C independent review

Disposition: **approve**. No blocking finding. This approves bounded native
profile selection, exact custom work identity and the semantic-active hash
accounting/error correction. B4 rendering and a full B gate remain required.

The review used shared `dev/planar-v2-vernacular`, HEAD
`3171c49e7b92d3abda01bfbb8bfb0c6b29d4c67a`. Active instructions, implementation,
roadmap, v5 direction, exact X-SHAPE/old W contracts, B3-C authorization, approved
B3-A ABI, combined B3-B/R01 and D01-B informed the review. The checkout was
shared; others' work was preserved. Product, candidate support/current prose,
vendor and historical evidence remained read-only. Writes are confined to this
report, `tests/native/v5-02-x05-b3-c-review-evidence/` and ordinary serialized Debug
verification outputs. No fixes, Planar tools/legacy skills/roles/queues,
installed binaries, operator data, commits, installation, publication or merge
were used. Debug ownership was exclusive; release artifacts remained read-only.

## Authentic selection and exact identity

The [native operation](../../../src/engine/runtime/runtime.cpp#L2955) accepts
original selection material, constructs bundle/capture validation, runs the
approved semantic phases once, and passes only its own finalized same-call
`value` to the retained fourth selection phase. Caller model/context/definitions
never become that model. Input and transported results have immutable native
ownership; protected transport does not expose mutable native aliases. All-job
base validation, retained F and complete additive-cycle checks precede selected
job/role lookup. Selection does not revalidate or invoke F again.

The new trusted export is captured before external module execution and joins
the original shared audit/freeze with globals, five handlers and external
environment: **11 initial roots**. The mixed observer fixture retains **17 exact
references**, including four phase functions and eight external D/F references.
These are different sets and counts. Existing cumulative phase transport,
transient cleanup, first terminal diagnostic and invocation-budget ownership
remain in force. Public profile generic invocation stays closed; no provider or
filesystem service is introduced.

The [shared selection body](../../../src/stdlib/planar/selection.lua#L31) preserves
the old W/material selection rules. Embedded `select_role` retains original
normalization and validation. Only an authentic custom `selected_shape` adds
`shape_selection_v1`, definition/revision and producer-OR-consumer scoped,
synthetic-ID-sorted unit obligations, and selects `work_external_shape_v1`.
Grouping retains both endpoints and role assignments. Guidance is excluded.
Shipped pipeline/fanout W retains the old field recipe, version vector and
`work_v1`, including under a selected profile. Selected source/revision coupling,
separate unrelated-module provenance changes and conservative same-module visual
coupling match X-SHAPE. B4 presentation/prerequisite behavior is not inferred.

The independently authored author fixture is bound to declarations/contracts,
not product selection output. A second [reviewer encoder](../../../tests/native/v5-02-x05-b3-c-review-evidence/oracle.py)
uses its own canonical implementation and verifies the complete preimage, exact
selected-module definition, shape fields, version vector and domain digest. The
wrong old domain differs. Existing vectors and render goldens pass unchanged.

## Complete hash reservation and failure boundary

The [semantic-active digest branch](../../../src/engine/runtime/runtime.cpp#L403)
reserves before calling unchanged hash/canonical implementations. Inspection of
the pinned libc++ string source confirms growth uses max(required, twice prior
capacity), followed by alignment including the terminator. Successive heap
requests at least double; the last request is bounded by `2*N+2*A`, giving
cumulative requests at most `4*N+4*A`, with A=16 on this target. This covers
large appends followed by framing or suffix growth. Assignment from empty has
one request bounded by `2*n+A`, including the inline-to-heap transition. The
41-character digest has one 48-byte request. Result/canonical buffers move;
XXH3 reads the canonical buffer without an owned copy.

All canonical scalar/container/key framing and payload bytes enter the bound.
Each source occurrence has a 512-byte scratch slot; each map key has a separate
slot. Concrete graph/control allocation is 112 bytes per new value. Active-set
nodes require 24 bytes per visited value; load-factor-one power-of-two bucket
growth sums to less than 32 bytes per maximum-active-depth entry. Sorted map
views require eight bytes per member. Four envelope slots also cover the four
new values and 64-byte pointer vector. The file/fragment branches separately
reserve their raw text owner/copy. These allocations fit the charged slots;
source graph ownership is already established and no graph copy is assumed.
Canonical active entries are erased on return, so wide siblings do not require
a bucket population equal to their combined count. Recursion remains bounded
by existing conversion/canonical depth limits.

Variable byte totals use repeated checked `charge` calls before allocation;
there is no unchecked caller-size multiplication. A refusal can retain partial
charges, with no refund. Hash traversal does not call `visit` or increment the
converted-node count. Ordinary embedded digest execution follows its previous
branch. Hash/canonical allocation diagnostics become sticky resource failures
while semantic-active; direct bad_alloc/length_error uses the existing native
memory bit. Host adaptation prioritizes the existing first conversion failure,
and same-owner retries return the first terminal registration error.

The [reviewer hash corpus](../../../tests/native/v5-02-x05-b3-c-review-evidence/hash.xml)
passes **303 assertions**, measuring actual aggregate operator-new requests
around unchanged hash/canonical calls. It covers all three digest variants,
inline/growth boundaries through 1 MiB, long keys plus payload/suffix/domain,
empty maps, widths through 8192 and depths through 254. Actual allocations remain
within precharged bytes; converted-node counts remain zero. Remaining budgets
0/1/511 refuse before hashing. The large mixed case allocates **8,390,032** bytes
against **14,686,292** reserved. The 1 MiB file and fragment cases each allocate
**2,098,200**, below their respective **6,295,070/6,295,058** reservations.
This supplements the author's exact reservation-delta and 18-assertion corpus.
These bounds are pinned-platform logical accounting, not process RSS or Linux
acceptance.

## Independent verification

The [recorded Debug gate](../../../tests/native/v5-02-x05-b3-c-review-evidence/commands.json)
passes **389/389** focused cases and **11/11** supplemental history/codec cases,
plus **13** selection cases/334 assertions, **11** semantic cases/283 assertions
and **18** codec cases/413 assertions. Inventory 655 preserves all 642 preceding
names; this is not a full 655-case run. Six vector families, six exact source
audits, pinned formatting/Doxygen for 118 active sources and final no-work builds
pass. Raw XML, immediately saved LastTest records and exact selected-name sets
agree for author and reviewer, with zero failures/errors/skips/disabled cases.

Three additional [production reviewer cases](../../../tests/native/v5-02-x05-b3-c-review-evidence/adversarial.cpp)
pass **160 assertions**. They compare complete captured shipped selection for
all five valid fixture job/role combinations under two distinct profile source
identities; bind two external definitions under opposite module orders while
note-driven return ordering/guidance leave W equal; and preserve authentic
earlier results across recoverable selection errors, repeats and VM destruction.
An initial reviewer fixture used a hyphenated revision, correctly refused by
the existing descriptor grammar. Both initial XML records/source are retained;
only the reviewer fixture changed to an underscore. This was no product defect.

The [exhaustive production allocation sweep](../../../tests/native/v5-02-x05-b3-c-review-evidence/allocation.xml)
fails **every one of 7,924 observed allocation ordinals**, using the real product
library without runtime instrumentation: **71,329 assertions**, zero failures.
Every controlled one-shot refusal returns `resource_limit` and preserves code,
message, limit, maximum and logical field on same-owner retry. This covers the
new selection/hash boundary beyond the author's 138 sampled refusals. Recovery
uses later successful allocations; no persistent exhausted-host-heap or C++
exception-runtime allocation guarantee is claimed.

The inspected reversible [observer replay](../../../tests/native/v5-02-x05-b3-c-review-evidence/probes/commands.json)
passes all 19 lifecycle/resource modes plus the separate 18-assertion hash case.
Exact reference bindings and all 47 tag / 267 backing entries remain stable;
settled post-GC Lua heap is 431680 bytes through operation 25. The 25 operations
execute exactly 50 Fs, 25 prepare/finalize/selection phases and 50 check phases.
Partial snapshot, parser/conversion/Lua/node/native/instruction, selection-only,
hash-precharge and error-adapter controls retain terminal cleanup and first
diagnostics. Repeats reach the unchanged 100M instruction ceiling at operation 44
and cumulative native ceiling at operation 77 without reset/refund. These are
controlled probes, not claims of natural resource exhaustion.

## Identity, isolation and release

[Scope verification](../../../tests/native/v5-02-x05-b3-c-review-evidence/scope-final.json)
reconstructs the incremental diff and verifies all 25 candidate members, 391 author
artifacts and 164 frozen source/support/current-prose entries. The 5682-entry
protected source/vendor/release/candidate collection retains exact membership and
hashes. Pinned hash-accounting sources match. AGENTS remains the CLAUDE symlink.
Private compilations recursively expand response files and redirect object,
dependency and possible PCM outputs; each preserves exact shared compiled
membership/hashes and never replaces product libraries. The interrupted author
gate remains incomplete historical evidence; the final completed gate contains
the explicitly discovered hash assertion case.

| Reviewed author identity | SHA256 |
| --- | --- |
| Candidate manifest | `5bd96ff593119c05bb9683ed159b97ff091e270dcdd4ed953a5ef35ece8ff427` |
| Candidate diff | `26f03ff16e78320538305d9cc8031888395953d3a3bbf54eaca702cfe504bb15` |
| Artifact manifest | `f6ba8a1700fb1a866c2ab52fa586e41d73d4ea4191746e6319655707be34d3af` |
| Author report | `07a42d3153f8b8d13b5fe2ce7827183af3ac58260c932361226e144ae03477e9` |

[Finalization](../../../tests/native/v5-02-x05-b3-c-review-evidence/finalization.json)
binds this report, reviewer artifacts and final unchanged source/candidate
identities, then releases Debug ownership with no later reviewer edits. The
coordinator must verify the reviewed candidate is still current before advancing.
No full B, release or Linux gate is claimed. B4 rendering, X06 history/
interpretation/acceptance and whole-invocation orchestration, X07 public cutover,
X08 mandatory external-profile acceptance, execution/RA01 and Linux remain open.
Public init creates format 1; latest full approved gate remains A4's 582 per build.
