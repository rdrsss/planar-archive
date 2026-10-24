# X06 — Recorded profile interpretation and acceptance path

X06-A's notation clarification, [X06-A1 pure codec](rounds/V5-02-X06-A1-review.md)
and [X06-A2 historical helper](rounds/V5-02-X06-A2-review.md) are independently
approved. [X06-A3](rounds/V5-02-X06-A3-review.md) independently approves integration into
actual ledger integrity/history reads. Current inventory is 918; the latest full
approved macOS gate remains 876 per configuration.

Ledger verification preserves source/registry checks, then bounded canonical
classification chooses the fixed embedded16 MiB/4096 or effective32 MiB/8192
reconstruction profile. The helper independently verifies the actual recorded
stdlib tree, fixed interpretation syntax, workspace and digest. Accepted events
retain the verified family; noops already require exact active references and
reuse that family. After existing actor/head/event/result checks, request_v1 and
request_v2 embedded intent require embedded provenance, while request_v2 profile
intent requires effective provenance. Private selector paths are never compared
to portable profile/input.json. Schema/codec restrictions remain independent.

Revision views reconstruct raw stdlib files through the existing pinned snapshot
and return only supported decoded payload. Safely unsupported interpretation
still permits exact source export with known supported provenance, but revision
projection returns existing unsupported_interpretation. Unknown effective
manifests refuse existing format unsupported_state; old embedded unknown manifest
behavior remains corruption. Malformed/resource/tree/correspondence failures map
to existing corruption, with storage failures retained. No new public code is added.

SQL-only fixtures construct recorded format2 profile histories without adding a
profile writer or conversion tool. Original scratch source/registry/profile/runtime
files are removed before reads and saved accepted/noop replay. Existing exact
replay can return saved profile results with unusable preparation, while fresh
profile acceptance still refuses. A valid4100-file effective closure exercises
wider reconstruction bounds. This adds no same-VM preparation/transaction
acceptance authority, historical Lua execution, shared invocation budget or RSS
guarantee. Public init creates format1; later X06, X07, X08, execution/RA01 and
Linux remain open. Historical planning checkpoints below retain their scope;
this opening supersedes their stale reader-status/future-slice wording.

## X06-A4 architecture prerequisite

The [authentic preparation architecture](profile-preparation-architecture.md) is
a documentation candidate; combined A4+C01 awaits independent review of the
correction to [X06A4-R1](rounds/V5-02-X06-A4-review.md#required-change-x06a4-r1--admit-the-known-selected-workspace-file-before-registration).
Known load must metadata-admit both the workspace root and selected workspace
declaration file before external registration. Its bound source-capture factory
consumes those original root/file receipts plus the registry receipt; first read
requires original-root discovery with exact same_selection and actual opened-leaf
verification, retaining the original file token through final parsing, sealed
capture, prepared ownership and native precommit checks. Future evidence must
cover a module/declaration hard-link alias across disjoint directories before any
external chunk, replacement before first read, and valid registry colocation.
Existing extension late admission supplies the mechanism, not the required
known-load timing. No new quota, content read at admission or product policy is
added. No product implementation or build/test gate is claimed. It selects
runtime-owned original capture and registration through final parsing, private same-VM route/all-job B/S preparation,
immutable opaque prepared ownership and a higher ledger acceptance consumer.
A downward native storage/source-capture refactor removes the current
runtime -> history.read -> ledger.format -> prepared obstruction without a
cross-module friend, public raw-profile factory, permission flag or second VM.
The exact constructor/consumer protocol, native transaction sharing, receipt and
budget lifetimes, adversarial matrix and bounded follow-up rounds are specified
there. Independent architecture approval precedes dependent product code; fresh
profile acceptance, public cutover and shared budgets remain closed.

The following earlier dependency-first plan retains its historical scope; A1/A2/A3
are now approved, and the architecture prerequisite governs the next live slice.

## Existing boundaries and the next slice

| Boundary | Existing source and retained behavior |
| --- | --- |
| Request and transaction foundation | [request_v2](../../src/engine/history/request_v2.cppm), [ledger acceptance tests](../../src/engine/ledger/acceptance.t.cpp) and [load](../../src/engine/acceptance/load.cpp): exact embedded format2 acceptance is implemented; this does not admit fresh profile acceptance. |
| Historical interpretation | [interpretation_codec](../../src/engine/history/interpretation_codec.cpp) decodes v1 only, bounded to 16 MiB per interpretation/versions, depth256 and four million canonical nodes. Structural validation and separate-version equality precede unsupported-version classification. Preserve this function and its error precedence. |
| Historical reader and preparation | [read](../../src/engine/history/read.cpp) calls the v1 decoder for revision views; [prepared acceptance](../../src/engine/acceptance/prepared.cpp) binds its existing embedded provenance/interpretation to captured source and receipts. Neither is silently upgraded by the first slice. |
| Pure effective provenance | [effective_provenance](../../src/engine/history/effective_provenance.cppm) accepts raw files and verifies membership, manifests, profile/module order and catalog with no Lua, host or storage access. [Catalog](../../src/engine/history/effective_catalog.cppm) independently validates canonical catalog-v2 bytes and ownership. |
| Live same-VM identity | [runtime shape resolution](../../src/engine/runtime/runtime.cpp) computes `shape_definition_v1` from the original descriptor, logical owner and module bytes; [hash_value](../../src/engine/hash/hash.cpp) emits `xxh3_128:` plus 32 lowercase hex digits. [C4-C2](rounds/V5-02-X05-C4-C2.md) provides the prior tagged-definition transport precedent. |

The smallest dependency-first implementation round is **X06-A1: pure v2 codec and
raw-tree correspondence only**. Suggested owned files are a new
`src/engine/history/interpretation_v2_codec.{cppm,cpp,t.cpp}`, narrowly scoped
CMake module/test registration if needed, and round evidence/reference updates.
The fresh round must confirm these paths and module edges before implementation.
It leaves existing interpretation-v1 source, read dispatch, live preparation,
SQL/schema, CLI and shipped Lua untouched. Run focused nonzero codec/tree tests;
a later integrated gate preserves all 876 approved names and behavior.

There is no existing `verified_effective_stdlib` capability type.
[prepared_provenance](../../src/engine/history/provenance_codec.cppm) exposes mutable
files and digest. `validated_catalog` owns canonical bytes and severs caller
aliases. Its `data()` returns a recursively const-facing `value_ptr`; ordinary
consumer access cannot mutate descendant nodes. A caller-supplied aggregate,
claimed digest or catalog still cannot prove correspondence to a referenced raw
tree. This authenticity requirement is independent of explicit const casts. X06-A1 must receive raw canonical interpretation/
versions bytes and raw tree files, internally call `verify_effective_stdlib`,
independently decode the verified raw manifest/catalog bytes, and recompute the
portable tree digest and selected-shape identities. Inputs stay stable during the
call; owned results sever aliases. Later acceptance must retain private ownership
rather than trusting mutable public aggregates.

## Pure codec and verification contract

Both public candidate APIs require raw files:
`decode_interpretation_v2(encoded, versions_encoded, raw_files)` and
`encode_interpretation_v2(bundle, semantic, raw_files)`. Structural parsing stays
private. Each operation internally calls `verify_effective_stdlib`, derives actual
ordered shipped/external module paths from verified manifests, and validates
asserted catalog ownership in that context. Encoding derives V/E from this private
verified context; it accepts no caller versions, catalog, digest or aggregate.
Canonical roundtrip severs B/S aliases. The returned ordinary owned data proves
structural recorded correspondence, without live same-VM selection authority.
Both operations are pure native boundaries without runtime, FS or storage.

The exact canonical envelope is `{version:2,bundle:B,semantic:S,versions:V,
extensions:E}`. B retains the existing complete normalized bundle with registered
job shape IDs admitted. S retains the complete semantic model, with exactly
`selected_shape:{id,revision,definition,obligations,guidance}` on custom-shape jobs.
Apply existing structural container/ID/text rules and X-SHAPE correspondence;
require B/S one-to-one sorted job identity and no missing, duplicate or extra job.
Built-in jobs retain their old representation. This is fixed historical structure,
not invocation of today's shape functions or declaration schema policy.

E is exactly `{registration:"external_registration_v1",catalog:C,
selected_shapes:LIST}`. LIST is sorted by job ID and has exactly one entry per
custom-shape job: `{job,id,revision,definition}`. Its selection ID/revision and
exact definition bytes agree with S and the selected verified catalog descriptor.
C must equal the verified effective manifest catalog in canonical bytes. Extra
fields, selections on built-in jobs, missing/duplicate entries or an unrelated
catalog owner refuse. Obligations retain the contracted X-SHAPE structure:
original `{id,producer,consumer,phase,quality}`, with normalized shape-prefixed ID,
`source:"shape"`, `producer_role` and `consumer_role`; phase is start/complete,
quality is done, and referenced work/roles must correspond. Preserve the 10000/job
and 100000 total bounds and guidance UTF-8/no-NUL/65536-byte bound.

Both S.selected_shape.definition and E.selected_shapes.definition contain the
existing **41-byte** `xxh3_128:<32 lowercase hex>` H identity. Recompute
H(`shape_definition_v1`, `{descriptor:D,owner:{tier,module},module_bytes:HEX}`)
from the verified original module bytes, with lowercase byte hex and no owner
ordinal. Compare exact bytes; stripping the tag changes identity and is forbidden.
V.stdlib_digest instead contains the raw **32 lowercase hex** portable effective
tree digest. A structurally self-consistent forged digest/catalog/definition still
fails independent native tree verification/recomputation.

V keeps every old key/value except the exact X-STORE substitutions:
interpretation_v2, declarations_shapes_v1, normalization_shapes_v1,
dependencies_shapes_v1 and selection_shapes_v1; it adds registration
external_registration_v1 and provenance effective_stdlib_v1. Projection remains
projection_v2. Embedded V canonical bytes must equal the separately recorded V
bytes exactly. No current embedded version/digest or current Lua is substituted.
No hash, W recipe or version constant changes in this documentation round.

Interpretation and separate versions each remain at most 16 MiB; the pure v2
codec uses depth256/four-million-node canonical limits and checked preallocation
arithmetic, with lower inherited bounds applying. Effective provenance retains
32 MiB/8192 files/depth64 and its component/manifest bounds. Expected failures
use closed native categories for malformed structure, unsupported versions,
resource exhaustion and invalid tree/correspondence; outer adapters later map
these to their existing recorded-history/preparation error vocabulary. Do not
invent new public messages or conflate trusted live validation with recorded
corruption. Freeze v2 malformed/future-version/error ordering in its own reviewed
tests; leave the v1 malformed-before-unsupported precedence unchanged.

Structural historical verification proves recorded correspondence and identities.
It does not prove callability, current policy validity or semantic honesty of a
trusted module/revision label. No historical Lua execution is permitted.

## Dependency sequence after X06-A1

Each row needs a fresh scoped search/coder/reviewer round and approval before the
next dependent behavior. Labels after A1 describe work boundaries, not claims of
completed implementation or fixed file ownership.

| Step | Scope and acceptance boundary |
| --- | --- |
| Recorded reader dispatch | Dispatch v1/v2 interpretations and embedded/effective provenance for formats1/2 at existing history/integrity boundaries. Bind v2 raw records to the referenced verified tree. Preserve old readers and typed unsupported behavior. Unsupported interpretation can still allow raw source export when record/tree formats are supported; export remains source-only. Read after profile/source/runtime removal with no Lua. |
| Same-VM prepared profile acceptance | Extend preparation from the authentic original capture, sealed catalog/callables, all-job shape results and original invocation. Derive source membership, effective tree and V/E natively, retain immutable private prepared values and original receipts; refuse substitution of bundle/catalog/selection from another VM or owner. No caller-built mutable aggregate becomes acceptance authority. |
| Profile format2 transactions | Admit exact request_v2 profile intent under X-STORE; unknown format1 profile intent refuses unsupported ledger version after exact request lookup and before profile read. Preserve request conflict/CAS order, exact collision comparisons/noop/revision/event/request atomicity, rollback, uncertain commit and same-ID recovery. Reverify original receipts at existing native commit boundaries; no Lua/callback/error encoder under lock. |
| Whole-invocation composition | Compose original capture, registration, retained VM, declarations, nested shape/projection/read services, conversions, preparation and result/acceptance lifetimes under genuine cumulative ownership/accounting. Preserve lower stage ceilings without resets/refunds/double admission; do not call the current distinct content, metadata and VM budgets a shared budget. Known slots are checked after early replay miss before external execution; late slots before handler/service use. Original receipts survive through result and acceptance seals. |
| Integrated X06 evidence | Exercise readers, preparation, replay/races/recovery and cumulative budgets together; preserve old format1 and embedded format2 behavior and all 876 names. Require focused, then complete configuration gates agreed in the round; macOS evidence does not establish Linux acceptance. |

Exact early replay precedes profile/workspace content access and current-head CAS.
After a miss, valid original capture/preparation happens outside the transaction.
Inside it, identity/format recheck and exact late request relookup precede CAS;
equal late replay returns original bytes and never reactivates a revision. An
early miss can still encounter a real profile failure before concurrent late
replay. Existing postcommit receipt, saved-result/stdout fallback and uncertain
commit limits remain as contracted; this path promises no stronger recovery.
[X-CAP/X-STORE](external-profile-contract.md) governs these ordering details.
X07 owns public flags/envelope parsing and init cutover; X08 separately closes the
mandatory external-profile acceptance gate. No whole-process RSS or natural
host-exhaustion guarantee follows from any current or planned charged budget.

## Verification matrix

These are required future checks, not tests executed in X06-A.

| Layer | Evidence required |
| --- | --- |
| A1 known bytes | Independent canonical v2 and separate-V vectors, exact tagged H vs raw tree digest, encode/decode roundtrip and no mutation/alias leakage. Retain existing v1 vectors unchanged. |
| A1 malformed/future versions | Missing/extra fields, wrong kinds, duplicate/unsorted B/S/E jobs, selected-shape fields/obligations/guidance, unsupported envelope/version vector, separate-V byte mismatch, invalid UTF-8/NUL/tag/case/length, exact 16 MiB/node/depth/resource edges and error precedence. |
| A1 cross-record | Forge every digest/descriptor/owner/ordinal/module/mode/order/membership/catalog and S/E selection independently; recompute from actual raw canonical/tree bytes, including forged mutable prepared_provenance and catalog pointers. Matching caller claims must not bypass verification. |
| Readers | SQL reader fixtures for both formats, v1/v2 and unknown interpretation/provenance; restart with profile/runtime removed; exact source export and independent unsupported interpretation behavior. No conversion tool inferred from fixtures. |
| Preparation | Same-VM authentic capture versus substituted bundle/catalog/selection, all jobs before role scoping, changed original known/late roots/files, immutable ownership through moves/destruction and precommit verification. |
| Transaction | Early/late exact replay, intent mismatch before CAS, concurrent head change, noop/changed revision, collision safety, busy/full/crash/commit-uncertain/postcommit transport recovery by same ID, no Lua while locked. |
| Invocation | Cumulative nesting/stage exhaustion with bounded fallback, no counter reset/refund, retained original receipts on success and caught semantic/resource failures; explicit accounting of retained and temporary native/VM storage. |

X06-A itself runs only documentation/link/identity/protection checks. It changes
no product/test source, claims no new product tests, and owns no build directory.
Its [evidence report](../../tests/native/v5-02-x06-a-evidence/REPORT.md) preserves
preedit documents, baseline/final manifests and the exact documentation diff for
independent review.
