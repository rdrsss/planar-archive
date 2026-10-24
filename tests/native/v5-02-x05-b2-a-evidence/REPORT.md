# X05-B2-A native retained shape definition foundation

Status: candidate pending fresh independent review. This bounded slice does not
close X05-B2, the B milestone, or the mandatory external-profile gate.

The shared `dev/planar-v2-vernacular` checkout starts at
`3171c49e7b92d3abda01bfbb8bfb0c6b29d4c67a`. Existing intentional changes are
preserved. Ownership covers `runtime.cppm`, `runtime.cpp`,
`external_registration.t.cpp`, three new B2-A support scripts, this evidence,
and the six authorized current reference files. The coordinator owns round
records. No Planar tools/legacy workflows, commits, installed binaries,
installation, publication, merge or operator data were used. Debug builds and
tests were serialized under this author; no release or full-suite run is claimed.

## Production behavior and identity

`command_vm::resolve_shape_definition(id)` resolves only registered external
shape entries of an already-created selected-profile owner. It returns owned
`id`, final `revision` and `digest` strings. No descriptor, callable, source or
mutable alias escapes. The native final descriptor, native owner ordinal's
privately copied module selection, and original captured bytes determine:

```
H(shape_definition_v1, {
  descriptor: FINAL_D,
  owner: {tier: "external", module: "external/" + SELECTED_PATH},
  module_bytes: LOWERCASE_HEX(ORIGINAL_SELECTED_BYTES)
})
```

Existing `hash_value` supplies canonical-v1, domain framing and pinned seed-zero
XXH3-128; ordinal is used for selection and never hashed. The operation does no
Lua evaluation, retained F invocation, reload, global lookup, filesystem access,
provider call, phase transition or budget reset. Generic profile invocation
continues to refuse before providers. No cache is added: every successful lookup
recomputes deterministically and consumes cumulative native work. Protocol refusal
is recoverable; existing failed sealing owners refuse, and sticky resource failure
makes later definition lookups refuse. Errors use the existing redacted
protocol/resource vocabulary with logical `catalog.shapes` location. User-facing
unknown-shape `reference` at `job.shape` remains B1 semantic policy.

## Reservation derivation

All reservations precede HEX, graph, canonical/hash or returned-string
allocations. A captured selected module is at most 1 MiB and its path is already
bounded by structural admission. The final descriptor is the validated flat
five-string object and is borrowed read-only, so no descriptor graph/string copy
is charged or made.

The operation derives D's exact canonical size from nine-byte record/string
headers and actual keys/string bytes. It adds exact owner/input headers and
payloads and the four-item hash list framing with `planar`, `canonical_v1` and
`shape_definition_v1`. `hash_value` constructs one envelope and canonical output;
XXH3 reads that output directly. There is no hidden canonical decode or second
encoding pass.

Nine new graph nodes comprise input/owner/three string values plus the hash list
and its three string values. Six borrowed D nodes and ten encoded map keys complete
25 traversal slots. The existing checked 512-byte node reservations cover
value/control blocks and canonical active-set/sorted-pointer scratch. The five
new map entries reserve separate 128-byte slots. Payload reservations enumerate
two HEX strings (staging and graph), two module paths (staging and graph), new
map keys/tier/envelope text, the single canonical output, the 41-byte digest
that moves to the result, and copied returned ID/revision strings. These are
logical capacities under the existing cumulative 512 MiB metric, with existing
node accounting; they do not claim process RSS, allocator overhead measurement,
a new quota, or whole-invocation orchestration.

## Verification and preserved exploratory evidence

Six production native cases exercise actual captured profile creation and lookup:

- Final D initialization mutation binds revision `final`; an independently
  constructed object passed to existing `hash_value` agrees with the lookup.
  A separate Python canonical encoder plus directly compiled upstream XXH3 oracle
  yields the fixed 733-byte known answer
  `xxh3_128:b6d3718ddeb0cd1bc49b9e866a638b99`.
- Selected comment bytes, owner path, final D changes and same-module projection
  code change identity. Unrelated projection edits, changed module ordinal/order,
  profile whitespace and file modes leave selected identity equal.
- Actual `profile_capture` calls with different host selector filenames give
  identical selected definitions. Later on-disk source replacement refuses receipt
  verification yet retained snapshots keep the original definition.
- Caller-owned profile mutation and returned metadata mutation cannot change
  subsequent lookup results. F deliberately throws if invoked; repeated lookup
  succeeds without executing it.
- Unknown IDs, command IDs, embedded shape IDs, embedded owners and moved-from
  owners refuse; a recovered valid lookup and moved-to owner continue to work.
  The existing no-provider generic invocation control now first resolves a shape.
- Repeated lookups of a large selected source eventually hit the existing
  cumulative `native_bytes` limit; same-owner retry stays resource-refused while
  a fresh owner succeeds. This is genuine logical-quota exhaustion and cleanup
  evidence, not a claim of natural host allocation failure.

`build-01.log` preserves a test-fixture C++ string concatenation compilation
failure. `probes-01.log/xml` preserve an accidentally early zero-match discovery
attempt while the prior build was still active; it is not accepted verification.
All later builds/tests were serialized after completion. `probes-02.log/xml`
preserve 4/5 passing cases and the invalid host-selector-as-logical-profile-path
fixture refusal; the fixture was corrected to use real capture selectors.
`probes-03.log/xml` then passes all six cases. These exploratory records are
separate from the final frozen gate. `finalize-attempt01.py/log` preserve a
finalization helper failure caused by an incorrect build binary location; the
helper was corrected to the configured `bin/planar` path without source edits
or rerunning verification.

The final gate's commands, raw console/JUnit/LastTest and summary live under
[final-gate](final-gate/commands.json). It preserves every 600-name B1 inventory
entry and all 582 A4 names among 606 registered names. Inventory is not a full
606-case run. The focused set includes runtime/CLI/B1, acceptance/load, capture,
canonical/hash, effective catalog/provenance and closure audit coverage.
The final focused gate passes **252/252** cases (245 unit, seven acceptance)
in **133.67 seconds**, with zero failures, errors, skips or disabled cases.
Four existing vector generators, all five inherited exact-source audits, the
new narrow lookup source audit and pinned formatting/Doxygen for **118 active
C++ sources** pass. The final `all planar_tests` build reports
`ninja: no work to do.`. The source audit checks absence of Lua/provider/callable/reset access;
it complements the actual production behavioral cases and is not execution
proof by itself.

## Freeze, limits and handoff

A single pre-gate source/configuration/fixture/support/current-doc snapshot is
compared after the complete gate and again at finalization. Candidate source and
test bytes remain fixed. Final-owned files, reconstructed scoped diff, candidate
manifest, source scope audit, build identities and artifact manifest are bound by
[finalization](finalization.json). The scope audit compares the previous B1 frozen
membership and bytes, and verifies archived existing-file originals against that
baseline. It does not claim a whole-tree initial snapshot; unrelated historical
evidence and coordinator round files are outside the scoped freeze.

No retained F dispatch, immutable F input, complete-return 16 MiB transport,
one-time invocation instruction transition or later shared orchestration is
proved. B2-B codec accounting and B2-C call bridge, B3 semantic/selected identity/W,
B4 render, X06 recorded interpretation/acceptance/receipts, X07 public cutover,
X08 mandatory external-profile acceptance, execution/RA01 and Linux remain open.
Public init still creates format1. Fresh independent review is required before
advancement. Build ownership is released only after final hashes and evidence
are complete; no author edits follow release.
