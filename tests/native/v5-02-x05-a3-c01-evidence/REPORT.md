# X05-A3-C01 corrective registration candidate

Status: pending fresh independent review of combined A3+C01; no self-approval.
Shared checkout `dev/planar-v2-vernacular`, starting HEAD
`3171c49e7b92d3abda01bfbb8bfb0c6b29d4c67a`. Coordinator owns the round.
Scope: runtime.cpp, external_registration.t.cpp, six current reference/instruction
files, and new C01 support/evidence. The complete pre-edit source baseline includes
modes/symlinks. A3 author/reviewer evidence and historical sources remain immutable.
No Planar tools/roles/skills/queue, installed binary, operator data, commit,
installation, publishing or merging occurred. Serialized Debug build ownership
was granted exclusively to this coder. No integrated full/release/Linux gate is
claimed; latest full gate remains520 each configuration.

## Corrections and regression evidence

R1 validates the native name/id string structure while each descriptor's original
logical field is current, before any later descriptor advances that field and
before catalog sorting. Pure catalog validation retains spelling, exact schemas,
identity policy, and existing canonical-to-original error remapping. No public
error shape or combined-fault precedence changes. The new ordinary-runtime test
covers commands, shapes and projections, each missing/wrong type at early/middle/
late positions among valid entries in noncanonical z/a/m order:18 combinations.
All assert exact internal/protocol message and original field. Handler-ID and
option-name controls assert original command7 after canonical reorder. The
preserved independent canonical-order shape control still checks original2.

Before fixes, [red commands](red/commands.json) freshly compiled the ordinary frozen
runtime shape/projection detectors and preserved both expected detector failures.
They returned the last descriptor's field instead of original1. All controls were
retained. [Original runtime](original-source/src/engine/runtime/runtime.cpp) preserves
the pre-correction candidate; historical A3 outputs were never overwritten.

R2 distinguishes initial prepare_stdlib resource_limit before mapping other
failures to protocol. New helper adapters were audited: profile decoding preserves
its typed detail/field; pure catalog maps resource/identity separately; effective
preparation already maps resource separately. Both caught canonical bad_alloc and
direct typed prepare-result regressions were red before fixes. Controlled canonical
injection throws an actual std::bad_alloc *inside* canonical_encode's existing try;
its existing catch produces the typed result. This is distinct from exceptions
thrown through constructor/callback boundaries and is not host exhaustion.
Corrective probe copies add only reversible injection/cleanup checks; production
canonical/provenance source remains unchanged. Green detectors assert exact
resource_limit / External profile resource limit exceeded / catalog. Lua destructor
checks abort unless all tracked live Lua allocation is released, on both paths.
The original A3 constructor/callback bad_alloc/unexpected controls and catch-all
red control remain covered independently.

## Complete logical capacity derivation for R3

All reservations charge the existing cumulative512MiB native_bytes metric, with
no refund/reset. They are logical payload/graph/path capacities, not allocator
instrumentation, process RSS, allocator-header or string-growth promises. Helpers
retain their own structural/byte checks. Checked reserve multiplication refuses
before allocation; metadata additions check the16MiB component bound. Source/count/
path bounds precede all remaining bounded multiplications. No public quota changes.

Six distinct final constructions each receive a separate1MiB slot, even if their
actual sizes differ:

| Slot | Construction | Source |
| --- | --- | --- |
|1|decode_catalog owned canonical bytes|effective_catalog.cpp decode_catalog|
|2|canonical combined effective manifest|effective_provenance.cpp prepare_effective_stdlib|
|3|effective.bin file_record payload copy|tree_core.cpp digest_tree|
|4|encoded effective.bin file record|record.cpp digest_record|
|5|result.files owned effective.bin payload|effective_provenance.cpp result ownership|
|6|catalog-plus-digest runtime seal|runtime.cpp register_profile|

Slot4 additionally pays27 record bytes:14-byte magic including version, one kind,
four mode, eight payload length. Slot6 additionally pays32 ASCII digest bytes.
No embedded metadata, source payload or graph/path reservation reuses these slots.
Catalog boundary encoding already pays its own3MiB per module boundary (encode,
validated owned bytes, runtime retained bytes). Its graph and the final decoded
catalog graph receive separate complete validate() structural charges; the
manifest shares the decoded catalog graph without copying it.

The immutable embedded metadata bound M is derived exactly from canonical_v1:
9 map header, four string keys (9+key length), integer version9, scheme string
9+17, files list header9, separator string9+separator bytes, and sum(9+path bytes)
for original-order files. Its fixed part is125bytes. Current M=410bytes and the
private probe checks the actual encoded metadata equals M. No assumed small
metadata or spare final slot is used. Two prepare_stdlib calls encode metadata;
four digests construct metadata record payload+encoded buffers; three owned trees
copy metadata. Thus M receives2+4*2+3=13independent byte passes. Four metadata
graph passes (two constructors, two decodes: early effective-order metadata and
verify_stdlib metadata) receive4*(original_count+16)*4096 plus4*separator bytes.
4096 per file/root allowance covers value/control blocks, path strings up to1024,
list/maps/order/set vectors and codec traversal scratch; fixed16covers four keys,
root/list/version/scheme/separator and their maps. These graph allowances are
separate from metadata encoded bytes and final catalog structural validation.

Four embedded digests remain: initial prepare; verify's first digest; verify's
nested prepare; final effective digest. Each creates record payload plus encoded
record; three owned trees copy sources:4*2+3=11passes for assembly and every
original source. Profile/modules use final digest plus final ownership:1*2+1=3.
Every embedded record gets4*27 separate framing bytes; profile/module final
records get27each; effective.bin framing was paid by slot4. Metadata payload11
passes are paid by its13-pass reservation, never source loops.

Path/tree recipes retain4passes for each embedded original/assembly/metadata and
1for profile/modules/effective.bin. Each pays2048per upper-bounded component and
8*(path bytes+32) for prefix/path/folding/names, tree vectors/maps, directory record
children and encoded directory children. Prefixes shipped/ and external/ fit+32.
An extra4096per original/module plus16fixed entries covers effective manifest
module/owner/view/order nodes and retained path ownership. Catalog composition's
per-entry4096and complete structural validation remain separate. These are the
same pinned logical graph/path recipes, now without borrowing for byte buffers.
Changed helper allocation recipes or dependencies require renewed derivation.

The bounded high-catalog fixture includes all64commands/64shapes/64projections,
4096-byte help for187external entries and50valid4096-byte option helps. Actual
canonical catalog is1041459bytes (99.32%of1MiB); combined effective.bin1041840bytes.
This checks validity near the independently enforced catalog/combined ceilings.
Private copies print actual sizes and check metadata equality; ordinary-runtime
Catch2 also enforces catalog>1040000and<1048576.

The remaining-budget probe measures real final cumulative charges, then seeds the
native counter to leave exactly that much and one byte less. Empty profile final
reservation charge9133708; high catalog11754667. Exact control proceeds through real
pure provenance/digest creation; one-byte-short refuses in real charge(), before
pure entry, at catalog. It preserves the existing propagated native message
`Native conversion allocation exceeded` (X-LIMIT permits existing native messages).
This is a controlled remaining-budget boundary, not naturally exhausted input,
new quota, native allocator or host-memory exhaustion. Both paths check zero
tracked live Lua allocation after destruction.

## Development outputs and verification limits

Every failed output remains preserved. High-catalog fixture initially used
string.rep, which the curated environment intentionally removes; this fixture
was corrected to a native-generated bounded literal. Two additional runs improved
its diagnostic visibility, preserving the same failure. The first remaining-budget
detector expected the new external resource message; actual charge() correctly
preserves the existing native message. The detector was corrected, with original
failed output retained. These are fixture defects, separate from reproduced
product R1/R2. No observed failure was suppressed or interpreted as a passing gate.

Final frozen focused gate results, exact hashes and handoff are recorded below
once completed. Public profile CLI, B/C semantics/adapters, X06 interpretation and
profile-history acceptance, caller receipt/budget orchestration, public cutover,
execution/RA01 and Linux remain open. Public init createsformat1.


## Frozen focused gate and release

All 19 serialized [gate commands](attempt01/commands.json) pass.
Debug all+planar_tests builds complete; inventory is **573** unique names,
preserving all **570** starting names with no missing cases. The focused combined
selector runs **200/200**, including all 18 A3/C01 cases and 182 inherited relevant
cases, with zero failures/errors/skips/disabled cases in **100.71 seconds**.
[JUnit](attempt01/focused.xml), [matched names](attempt01/test-summary.json),
[raw output](attempt01/focused.log). This is focused verification, not a full573 run.

The original production and supplemental reachable-native suites each recompile
and pass eight scenarios plus their intentional catch-all red control. Corrective
R1/R2 probes, exact error/cleanup detectors, high-catalog actual-byte probe, and
empty/high exact-remaining versus one-byte-short budget controls pass. The latter
counter-seeding is explicitly controlled; no natural host exhaustion is claimed.
Canonical/record-v1/request-v2/effective vectors and historical B2/D6/D6-C01/D7-B
and A0 exact forward/reverse/shipped audits pass. Pinned formatting/Doxygen checks
117 active sources. Final no-change build reports `ninja: no work to do.`.

All 19961 source entries, modes and symlinks remain identical before the gate,
after verification and at release. [Scope audit](attempt01/scope-audit.json) proves
only authorized files changed from the pre-edit baseline; shipped Lua, vendor,
old tests, and every historical support/evidence artifact remain unchanged.
AGENTS.md remains the CLAUDE.md symlink. Full-build artifacts are separately bound.

- Source freeze/release SHA256: `b0c54d9e17b121747aaa17e19ad03f4c46e1e48fd05ec17da80c0f3caf838049`.
- Exact C01 owned diff SHA256: `efa84f690b7c4f1392ac629fc4894b703ef9e4f537a8a6c2399b313afd2341ab`.
- Build identity manifest SHA256: `ec0e697b2d8ddcdc42cb759e2bdd70a30b260c7b4a1d278322bb8013ef3812e3`.

Finalization binds this completed report and every C01 artifact without a circular
report/manifest identity. Build ownership is released after this final source and
artifact hashing; no subsequent coder source or evidence edits follow release.
Combined A3+C01 requires fresh independent high review before any integrated gate.
