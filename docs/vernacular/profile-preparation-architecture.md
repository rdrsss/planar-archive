# Authentic profile preparation and native completion architecture

Combined X06-A4 + C01 is a **documentation candidate awaiting independent review**.
C01 addresses [X06A4-R1](rounds/V5-02-X06-A4-review.md#required-change-x06a4-r1--admit-the-known-selected-workspace-file-before-registration):
the known selected workspace declaration file requires original metadata
admission alongside its root before external registration. This candidate selects
an internal architecture; its API sketch remains unimplemented and uncompiled.
[A1](rounds/V5-02-X06-A1-review.md),
[A2](rounds/V5-02-X06-A2-review.md) and
[A3](rounds/V5-02-X06-A3-review.md) are independently approved. The current
inventory is 918; the latest complete approved macOS gates remain 876 per
configuration. Fresh profile acceptance remains refused; public init creates
format1. Shared budgets, X07/X08, execution/RA01 and Linux acceptance remain open.

The exact [X-CAP, X-SHAPE, X-LIMIT and X-STORE contract](external-profile-contract.md)
controls behavior. This document changes no contract, byte recipe, schema,
public command or error message. The [X06 path](x06-implementation.md) records
preceding codecs/readers. The original [A4 round](rounds/V5-02-X06-A4.md) and
[corrective C01 round](rounds/V5-02-X06-A4-C01.md) control delivery. Original A4
evidence and its request-changes review remain preserved; this correction does
not approve either architecture candidate.

## Decision and present obstruction

Put authentic preparation in the **runtime named module**, where original
profile capture, registration, retained callables and all-job shape execution
can be one private operation. Export an opaque, move-only `prepared_profile`;
only the runtime's original `profile_invocation` can construct it. Put both
embedded and profile completion in a **higher ledger acceptance module**. Its
public profile overload accepts that opaque object, never a bundle, catalog,
codec result, permission flag, callback or caller claim of VM identity. Its one
nonexported transaction algorithm handles both families. Extract downward native
storage mechanics so the higher module does not need cross-module friendship
with `record_writer` or access to its private state.

This is a dependency refactor, followed by a new authentic runtime path. Merely
containing `prepared_acceptance` in a higher owner would fail: its factory decodes
v1 only and current completion explicitly rejects fresh profile intent. Neither
factory will be widened to accept public profile aggregates.

Source anchors for the obstruction and reusable seams:

| Existing source | Fact that constrains the implementation |
| --- | --- |
| [runtime interface](../../src/engine/runtime/runtime.cppm), `command_vm` and `extension_invocation` | `create(profile_snapshot)` accepts a structural snapshot without receipt authority. `extension_invocation::create(profile_capture)` pairs original capture and VM, but governs custom external handlers, not built-in load. |
| [runtime implementation](../../src/engine/runtime/runtime.cpp), `validate_profile_impl` | Phase0 prepares the complete bundle, each requested F executes in the retained VM, phase1 checks each response and cumulative obligations, phase2 finishes B/S. Empty selection returns before phase3. Phase8 is retained workspace routing. Generic `invoke` explicitly refuses profiles. |
| [load interface](../../src/engine/acceptance/load.cppm) and [implementation](../../src/engine/acceptance/load.cpp), `load_capture` | Own declaration-only membership/correspondence and sealed native receipts. Existing open is unbound; `prepare(bundle,semantic,files)` receives public aggregates and uses embedded preparation. Neither is a profile semantic origin capability. |
| [native provider](../../src/cmd/planar/invocation.cpp), `native_services` and load prelude | Built-in open_workspace/capture_load/accept_load currently own `load_capture`. The read-only early lookup releases its transaction before invocation. This provider is distinct from custom extension service sequencing. |
| [format interface](../../src/engine/ledger/format.cppm), `record_writer`; [format implementation](../../src/engine/ledger/format.cpp), `complete_acceptance_impl` | Completion currently accesses private `_state` after `begin_records`, exact late request lookup precedes format/family checks and CAS, then native interning/atomic writes/receipt checks/commit evidence run. It imports prepared-v1. |
| [history read interface](../../src/engine/history/read.cppm), [completion receipt](../../src/engine/ledger/completion.cppm) | Runtime imports history.read, which depends on ledger.format. The completion receipt is dependency-free vocabulary; runtime does not need transaction completion functions. |

## Named modules, targets and ownership

Arrows below mean imports/dependencies, not execution order. The proposed names
are internal design commitments; fresh search must confirm every current
consumer and generated test variant before extraction.

```text
cmd.planar.invocation -> ledger.acceptance -> runtime
                      -> acceptance.prepared (embedded only)
runtime -> history.read -> ledger.format -> ledger.storage
runtime -> acceptance.intent
runtime -> acceptance.source_capture -> capture / capture.native
runtime -> capture.profile / effective_provenance / interpretation_v2_codec
ledger.acceptance -> ledger.storage / acceptance.intent / ledger.completion
acceptance.prepared -> acceptance.intent / capture.native / embedded codecs
ledger.format -> history codecs / recorded_interpretation / ledger.storage
ledger.storage -> ledger.storage_types -> ledger.types / ledger.history_codec
```

`planar.engine.acceptance.intent` / `planar_acceptance_intent` receives unchanged
`request_data` and `request_intent` definitions/implementation from prepared.
Prepared re-exports intent for existing source compatibility. Intent validates
explicit normalized selectors; it has no filesystem, VM or ledger dependency.
This removes an unnecessary future runtime dependency on embedded preparation.
Plain request-v2 remains a data codec and grants no profile authority.

`planar.engine.acceptance.source_capture` / `planar_acceptance_source_capture`
receives reusable declaration capture, route and normalized/source correspondence
from load.cpp. It imports capture and native/value/host-path primitives only.
Existing `load_capture` delegates to it and retains its embedded factory. Runtime
uses its **bound** construction path: an original `retained_root_receipt` for the
workspace, an original `retained_file_receipt` for the **selected workspace
declaration file**, and an original `retained_file_receipt` for registry. Its
bound factory requires all three from the same original profile admission,
transfers the root through capture_session/add_root, explicitly reads the selected
declaration with its retained file token, and binds registry through native
add_input. Root-only construction cannot stand in for known selected-file
authority. Standalone registry retains file policy and only `input.json`, never its parent as an
admitted root. No public source primitive result is a prepared profile.

Existing mechanism anchors are [admission](../../src/engine/capture/admission.cpp),
`admit_root`/`admit_file(..., admission_file_role::declaration)`, and its
[retained receipt interface](../../src/engine/capture/admission.cppm),
`retain_file`/`same_selection`; [profile forwarding](../../src/engine/capture/profile.cppm)
keeps the original admission owner. [Capture-session forwarding](../../src/engine/capture/capture.cppm)
accepts retained root/file tokens for add_root, read and add_input. The [runtime extension path](../../src/engine/runtime/runtime.cpp)
pairs root and explicit declaration admission (around lines4799–4809), then
retains both for bound first read before declaration load (around lines5068–5076).
That existing opaque-extension admission occurs **after registration**; it is
mechanism precedent, not evidence of known-load pre-registration timing. The new
known-load factory must perform this pair of metadata admissions before external
registration. [Native bound read](../../src/engine/capture/native.cpp),
`read_file` (around lines201–300), discovers through the original root, requires
`same_selection` with an incoming original token and keeps that token as authority;
its actual opened-leaf verification precedes content access.

`planar.engine.ledger.storage_types` / `planar_ledger_storage_types` is a new
inert interface below format and storage. Relocate the exact `ledger_format`,
`ledger_metadata`, `ledger_access`, `history_head`, `revision_row`, `event_row`,
`request_row` and `event_page` definitions from format.cppm into it, preserving
namespace names and layouts. It imports ledger.types and ledger.history_codec
(for event_references/load_outcome), neither of which imports format/storage.
format re-exports storage_types; storage imports it directly. Keep acceptance_result
in the higher acceptance module. format_error and record-reference aliases remain
from ledger.types. Thus storage never imports a type owned by ledger.format.
This exact inert-type extraction is part of the first round's ownership, together
with both new storage files, both acceptance files and both intent files.

`planar.engine.ledger.storage` / `planar_ledger_storage` receives reusable native
storage internals currently in format.cpp: held `location`/descriptor identity,
`format_state` connection and snapshot ownership, selected-schema verification,
`same_metadata`, decoding/interning and locked writer mechanics. It depends on
SQLite, ledger types/codecs, tree/canonical/history verification, not runtime,
prepared or ledger.acceptance. Read-only format_file and generic record_writer
remain facades over this storage. Their public behavior stays unchanged.

Storage exports an opaque thread-confined `write_transaction`, not a SQLite
handle. Its finite methods are `metadata`, `read_request`, `read_head`,
`read_revision`, `intern_tree`, `intern_value`, `intern_request`,
`identical_content`, `append_acceptance_rows`, `verify_before_commit`,
`commit` and `abort`. Precise argument types are native records/references and
bounded values; methods enforce existing validation and rollback semantics.
`append_acceptance_rows` moves the existing revision/event/request/current-state
SQL as a single storage operation, with no Lua/value-provider interface.
It does not decide profile semantic validity, replay, CAS or outcome. The higher
private algorithm decides those. `begin_write(path)` performs exactly the current
read-only verification, descriptor/location retention, snapshot release,
BEGIN IMMEDIATE, metadata equality and locked validation from `begin_records`.
`verify_before_commit()` repeats held-file/name identity, full integrity and
complete metadata equality including selected format. `commit` retains the
confirmed-noncommit evidence contract. Every failed storage operation poisons
and rolls back this transaction; destruction/abort remains RAII and move-safe.
No callbacks or generic caller SQL are added.

`planar.engine.ledger.acceptance` / `planar_ledger_acceptance` receives the
`acceptance_result` and `complete_acceptance` declarations and the existing
completion algorithm from format.cpp. Remove their declarations, prepared import,
friend declarations and private `complete_acceptance_impl` from ledger.format;
update command/native test imports to ledger.acceptance. A compatibility
re-export from format is forbidden: it would recreate the cycle. The internal
breaking API change is within rewrite authority. Existing callers retain function
names after importing the new module. New public `complete_profile_acceptance`
requires runtime `prepared_profile` and intent plus the existing expected-ledger
identity/receipt arguments. A1/A2/A3 codecs/readers do not import this module.

In the final graph the acceptance implementation imports runtime; runtime's interface and
implementation import neither ledger.acceptance nor its target. It needs only
ledger.completion for native receipt storage. All new runtime classes are
**declared and implemented in planar.engine.runtime**; their friendship is within
one named module, not a forward redeclaration of a foreign module entity.
Pimpl storage permits native implementation changes without exporting Lua
references. No implementation-only dependency is hidden from CMake: runtime
links source_capture/intent/v2 codec, storage links its codec/verifier targets,
and acceptance links runtime/storage/prepared. CMake source and generated runtime
variants must follow this DAG; there is no format -> acceptance or runtime ->
acceptance link, even PRIVATE. Product-only and test-only targets remain separate.

The first extraction round is embedded-only: ledger.acceptance imports existing
prepared/intent/storage/completion, **not runtime**, and has no profile overload.
Keep the existing explicit fresh-profile refusal in its common algorithm. The
runtime preparation round later defines prepared_profile without importing
ledger.acceptance. Only the subsequent profile transaction round adds the
acceptance -> runtime target/interface edge and opaque overload. No staged round
references a type that has not been introduced.

The extraction is substantive and must be independently verified before adding
profile writing. Sharing helpers by copying the transaction or constructing a
second native connection with weaker checks is rejected.

## Constructor, parse and consumer protocol

The following API notation is a design sketch, **not compiled or verified code**.
All classes delete copying and public default construction; a private constructor
accepts their own unique implementation. Move transfers stable implementation
addresses and counters; source operations refuse. Same-module friends are limited
to the named classes below. No public constructor accepts implementation pointers.

```text
runtime::profile_invocation::open_known_load(request_intent, ledger_path)
    -> expected<profile_invocation, typed_error>
runtime::profile_invocation::open_opaque(profile_selector)
    -> expected<profile_invocation, typed_error>
profile_invocation::catalog() const -> immutable sealed final parser metadata
profile_invocation::prepare_load(final_parsed_load_intent) &&
    -> expected<prepared_profile, profile_preparation_error>
profile_invocation::into_extension() && -> expected<extension_invocation, typed_error>
prepared_profile::data() const -> expected<shared_ptr<const profile_acceptance_data>, typed_error>
prepared_profile::bound_intent() const -> immutable request_data
prepared_profile::verify_native_receipts() const -> expected<void, diagnostic>
ledger::complete_profile_acceptance(path, request_intent, const prepared_profile&,
                                    expected_ledger_identity, completion_receipt&)
```

`open_known_load` is the only factory that permits built-in load preparation.
It requires codec2 profile intent and extracts the profile selector itself; no
independent profile argument can disagree. The ledger selector is native-only,
used for initial selected-file alias checks, not appended to request framing.
Factory creation captures profile and ordered modules **once**, metadata-admits
the known supplied workspace root **and selected workspace declaration file**
(with `admission_file_role::declaration`), registry file and ledger selection
**before external registration**, then registers shipped and external chunks
**once** in its private command_vm. Reverify original receipts after registration. Captured
source bytes have not yet been read: these initial checks open only metadata/
ancestor descriptors, without declaration evaluation or source content capture.
Disjoint directory roots do not exclude a hard-link alias between a module and
the selected declaration; admitting the file now closes that gap before any
external chunk executes. Root and file tokens remain in this same owner through
registration and final parsing; later discovery cannot replace either. The mode
is assigned privately by factory,
not accepted as a caller flag; it cannot be changed by parsing or an adoption API.

`open_opaque` is for selected help/unknown external tails whose slots are not yet
known. It captures/registers once with no untrusted input bytes. Its final
extension admission occurs after CLI11 parse and before execution, using the
same original profile_capture and VM. It cannot prepare built-in load. X07's
successful embedded probe identifies shipped load before selected registration;
therefore calling the opaque factory and later upgrading to known load is never
needed. Unknown/extension final parse cannot claim that prior known-slot checks
occurred. The closed factory for the other shipped families is defined in the [X07 design addendum](x07-addendum.md), and the rule is stated here:

> Render, init, history and export are opened by the closed factory defined in the X07 design addendum; load is opened by open_known_load; no generic service factory exists, and a sixth with-profile family requires an amendment.

The X07 integration retains the **same profile_invocation** while borrowing its
sealed catalog for final CLI11 parsing. For known load, final native parameters
must exactly equal its bound initial intent (including presence, normalized
selectors, actor/head/request ID); final parser cannot substitute profile or
workspace. Both original workspace root and selected-file receipts survive
this parse continuation in the same owner. It then moves that owner, including
those receipts and their accounting state, into `prepare_load`. No reconstruction
of command_vm from snapshot/catalog, second capture or second registration is
allowed after final parse. Selected help may destroy the owner without any
operation. Extension dispatch uses `into_extension`: a new **private same-runtime
adoption constructor** transfers profile_capture, command_vm and original
counters directly into extension_invocation. Existing public extension create
can remain a fresh separate workflow, but X07 never calls it on an already
registered profile. The reverse conversion or extracting a mutable command_vm
is unavailable. Review must audit every actual creation site.

Within runtime, profile_invocation may call private command_vm helpers via
same-module friendship. Existing direct `command_vm::create(profile_snapshot)`
and `validate_profile(public aggregate)` remain useful structural/test APIs;
they have no route to a prepared_profile constructor. Passing an unrelated
command_vm, successful codec output or matching catalog/digest to this workflow
is impossible through its API. Its origin is execution within this private owner,
not equality of purported identifiers. A caller can legitimately open a new
invocation from a real selected profile; that creates its own capture and VM
and can only prepare from its own declarations.

## Preparation phases, native derivation and retained authority

1. After final parsing and intent equality, reverify the original capture and
   initial metadata receipts, including the selected workspace declaration file.
   Construct bound source capture with the original workspace root, original
   selected-declaration file token and original registry file token. Before first
   declaration content access, discover the relative declaration path through the
   original root and require exact `same_selection` with the incoming original
   file token, then verify the actual opened no-follow leaf against that token.
   The original file token remains captured authority after comparison; temporary
   discovery cannot replace it. A replaced leaf between registration and first
   read is terminal. Only then load that declaration and its references; newly
   discovered reference identities are admitted by the root before their first
   content/evaluation. Do not normalize away physical parent traversal. Cached
   reads, seal and acceptance reverify original authority without rediscovery or
   unbound retry.
2. Route this exact workspace using the retained phase8 `route_workspace` callable
   in this VM. Validate native route/raw correspondence with the shared existing
   load logic; it owns paths, declaration IDs and registry assertion checks.
   Capture routed spec/all jobs/references and the original standalone registry.
   Native source membership is exactly this declaration closure, including text
   references, with portable names/modes. No source/base tree enumeration, profile
   selector, module path, ledger, registry or unrelated sibling enters source.
3. Seal captured bytes/topology into original acceptance_capture before semantic
   interpretation. Build validation input privately from this capture, not caller
   bundle/capture fields. Reuse `validate_profile_impl` phase0, every F/phase1 and
   phase2 with **no selection/render request**. This requires a private active-frame
   refactor: current validate_profile_impl calls begin_invocation on its public
   path, so calling it unchanged after route would not establish this protocol.
   Split phase execution from invocation entry. profile_invocation begins exactly
   one active frame before phase8 and passes an internal same-module frame
   reference through route, all F/check/finalize calls and native preparation;
   these private helpers never call begin_invocation/reset counters or release
   outer roots. Public validation APIs keep their existing entry semantics and
   cannot supply this private frame or mint preparation. Add tests exhausting the
   cumulative budget across phase8/all-job/preparation, with nested root cleanup
   and first failure preserved. Record B from that operation's
   prepared normalized bundle and S from its final model. Add a private output
   parameter/helper for B/S if needed; do not reverse-engineer caller responses.
   Every job is validated, including custom jobs outside any role a later caller
   might select. No role selection, project or history service is called.
4. The private command state already owns original module/profile bytes, catalog,
   owner annotations, shipped sources and effective provenance. Assemble the exact
   effective raw tree using those originals, verify membership/modes/order/catalog
   with effective_provenance, and call the existing pure v2 encoder on private
   B/S plus that raw tree. Native derives E, V, tagged selected definitions and
   raw stdlib digest. The codec independently revalidates the actual tree; its
   public result alone is structural evidence, not the constructor authority.
   Separate V must byte-equal embedded V. Interpretation/versions bounds remain
   16 MiB each. No version/digest/encoded record/receipt/source set from Lua or a
   public caller is accepted.
5. Reuse shared native normalized/source correspondence from load preparation;
   verify B/S IDs, workspace ID, raw declaration correspondence and complete
   closure, independent of trusted shape semantic success. Derive sorted source
   tree and its digest from sealed native capture; registry is exact input.json
   provenance. Build immutable `profile_acceptance_data` privately: source and
   digest, registry, verified effective tree, decoded/encoded v2 interpretation
   and versions, workspace ID and exact bound intent. No writable aliases to
   temporary B/S/catalog/raw files escape; canonical-owned decoding severs them.
6. Verify all original receipts before publishing. Move the original
   profile_capture, acceptance_capture and dormant VM/accounting owner into
   prepared_profile's private implementation. Mark the invocation consumed before
   any fallible outward copy; failed construction publishes no prepared object.
   `data()` is observational and never grants construction authority.

Prepared ownership retains both sets of receipts, including the initial selected
workspace declaration-file token alongside its root. That exact selection survives
sealed source capture, prepared publication and native precommit verification;
no later seal or transaction re-admits a replacement. Native acceptance capture
checks selected closure identity/topology/bytes; original profile capture checks
profile/modules, admitted roots, selected aliases and ancestor relationships.
`verify_native_receipts()` invokes only these concrete native verifiers and
returns their first failure; it cannot execute Lua, call a virtual user provider,
reconstruct provenance, admit a new file, discover a new member or retry unbound.
The dormant VM remains inaccessible for calls once prepared and releases only
through RAII. This verifier is a fixed method, not an injected callback.

## One native transaction algorithm and replay order

Early lookup remains in command invocation/native prelude, before calling any
profile factory or reading workspace/registry/profile. Normalize intent and
create request_v2 without host reads; open verified ledger read-only, read exact
saved key/body/result. Equal request returns original saved bytes even when all
selected inputs have disappeared and head advanced. Different body conflicts
before profile access/CAS. Unknown format1 profile request refuses the contracted
unsupported ledger version before profile access. Close that read snapshot before
capture or write acquisition. The architecture never constructs a profile VM just
to perform early replay. Embedded bootstrap/probe is not external execution.

Higher ledger.acceptance exposes two authority-bearing entry paths: existing
embedded prepared_acceptance and new runtime prepared_profile. Its nonexported
common algorithm uses a private tagged native adapter over these types. Only
internal overload code creates the adapter; callers cannot supply its tag, fields
or a callback. Embedded adapter continues to refuse profile intent. Profile
adapter permits only codec2 profile intent with exact bytes/request ID matching
prepared_profile's bound intent, and only format2. A mutable data() copy or v2
encoder output cannot be passed to either profile completion API.

The common algorithm preserves current complete_acceptance_impl ordering:

1. Validate unused output receipt and immutable request; begin the native locked
   writer with retained actual file/name/metadata verification. Check prelude
   ledger identity, then exact **late** request lookup. Equal replay returns
   original bytes/receipt without examining the candidate, checking current head,
   touching receipts or executing Lua. Unequal body conflicts before CAS. A
   moved-from/bad candidate does not defeat exact late replay.
2. After late miss, enforce format/request family rules, then current-head CAS.
   Only then inspect candidate native data/bound intent, enforce family and
   workspace correspondence and stage exact interning. Native receipt failure is
   capture_changed; stale CAS retains precedence over candidate inspection.
   The profile adapter's private type check cannot be replaced by a bool in an
   acceptance_data aggregate. No Lua, user provider or semantic encoder runs
   under this lock; all B/S/V/E canonical work finished before acquisition.
3. Share existing source/registry/stdlib/interp/versions/request interning,
   exact collision comparisons, prior-reference byte equality, noop detection,
   ID allocation, event/result encoding and revision/event/request/current-state
   writes. Native event/result codecs are existing bounded native codecs, not Lua
   error/semantic encoders. Profile path does not copy this algorithm.
4. Recheck held ledger identity, complete integrity and exact metadata through
   storage, then concrete original native receipt verification for the selected
   opaque candidate immediately before COMMIT. Repeat every pre-existing native
   boundary; extraction must not collapse entry and precommit checks. Uncommitted
   errors abandon the writer and roll back all tentative records/rows.
5. Preserve allocation-before-COMMIT saved bytes and completion phase transitions:
   not_completed, commit_uncertain, committed/replayed with the existing
   confirmed-noncommit guard. Later native/Lua/encoding/stdout failures use the
   same saved receipt/fallback and same-ID recovery. Never promise rollback after
   uncertain commit or a stronger crash/hostile-writer guarantee.

Candidate data are immutable and transaction-local staging is native. An early
miss may still encounter real profile/capture failure before a concurrent late
replay; that contracted limit is preserved. Completing twice with the same valid
owner can only use ordinary replay/CAS rules; no prepared state is re-executed.

## Failure, lifetime, authority and resource accounting

Owners are thread-confined. Lifecycle is `capturing -> admitted -> registered ->
parsed -> preparing -> prepared/consumed`, with one terminal first failure from
any active stage. Opaque mode admits late slots between parsed and operation;
known mode checks slots before registered. Moved-from, consumed and failed owners
refuse; self-move preserves valid state. Admission, discovery, binding, receipt,
protocol, shape or resource refusal poisons the original shared authority and
session/VM operation before adapting/copying errors. A caught Lua error cannot
restore authority; semantic-error envelopes publish no prepared candidate.
Retain the first logical field/native typed diagnostic; use existing closed
X-LIMIT messages and existing load/format adapters. Do not label trusted live
semantic failure recorded-history corruption. No new public error vocabulary is
selected here.

Longjmp protection remains the existing native rooted/protected-frame scheme:
all capture/prepared RAII ownership lives outside Lua C frames across potential
longjmp. Registration and nested phases use one stable VM state, existing saved
roots and first failures; final native preparation does not reset invocation
instruction, conversion node or charged allocation counters.

Current content, admission metadata, declaration-VM and retained trusted-VM
metrics are now composed into the one factory budget owner. This architecture
does not prove RSS. The stage objects (capture/admission, native capture, declaration
VM, trusted VM) keep their own accounting storage but charge the one factory budget
owner; moves do not copy or reset counter state, temporary or cached storage does
not refund cumulative charges, the composed owner also enforces the 512 MiB live
Lua ceiling (declaration-VM live Lua acquire and release, decision Q9) next to the
per-stage ceilings, and
the selected declaration-file handle and temporary comparison/discovery staging
retain existing cumulative admission metadata charges without refunds. They add
no quota category and admission reads no source content. All new structs, retained
data, copies and fallback allocations require checked charges before allocation
under applicable stage ceilings. The private prepared implementation retains accounting lifetime with receipts/VM until completion and
response cleanup; destroying an outer wrapper cannot erase counters while the
prepared object survives. Whole-invocation composition injects a common
budget owner from the initial factory into these stages, preserving stricter
limits and counters, rather than recapture/re-register or mint a fresh budget at
preparation. The first code slice must inventory metric ownership/sizes so
composition is possible; the metrics are now composed into that one owner.

The encapsulation guarantee concerns ordinary C++ API clients: arbitrary value
aggregates, another VM, codec results, moved objects and observable immutable
snapshots cannot mint the private prepared type. It is not protection against
arbitrary process-memory writes, undefined behavior, const_cast abuse, a hostile
native binary writing raw SQL or a trusted module lying semantically. The lower
storage API, like existing SQLite/recordwriter primitives, provides **storage**
authority, not profile semantic attestation. It is never documented as a fresh
profile acceptance factory. The product's acceptance entry points require the
opaque constructors above; read-only historical validation remains structural.
No cryptographic identity/token, public permission flag or equality of digests is
used as origin proof.

## Adversarial acceptance matrix for following code rounds

These are required future evidence, not tests run by A4.

| Case | Required refusal/evidence |
| --- | --- |
| Caller bundle/S/catalog/codec-v2 tree with self-consistent hashes | No API constructs prepared_profile from them; compile-negative/public-surface audit plus runtime unrelated-VM/substitution tests. Existing pure/embedded APIs cannot upgrade. |
| Original snapshot aliases mutated/released; source owner moved/destroyed | Private original bytes/canonical results unaffected; moved original invocation transfers authority/counters, source refuses; prepared survives wrapper destruction and holds both receipt sets. |
| Known factory vs opaque parse | Disjoint trusted/workspace directories with a selected module hard-linked to the selected workspace declaration refuse during initial file admission before any external chunk executes. A missing selected declaration also refuses at initial admission before registration. Opaque final syntax precedes late checks and overlap precedes handler/service; that late-admission precedent does not establish known-load timing. Opaque owner cannot transition to built-in load. |
| Parse continuation | Registration side-effect test counter proves one capture/registration; final CLI11 uses exact catalog; built-in load and custom extension adopt same VM. No snapshot-based second registration after parse. |
| Extra invalid unselected job, many custom jobs, built-in/custom mix | Every job/F validated exactly once before any role selection; phase0/1/2 failure publishes no prepared input. Real obligation/cycle/total-limit regressions. |
| Root/ancestor/known/late file changes and selected aliases | Replace the selected declaration leaf between registration and first read: original-root discovery, same_selection and actual opened-leaf verification must refuse terminally before content, without replacement authority or unbound retry. Original root/file tokens survive parse, capture seal, prepared moves and precommit; changes after preparation or between transaction checks refuse/roll back. Positive registry colocation with trusted configuration succeeds when selected files are distinct; selected-file aliases still refuse. |
| Source/provenance substitution | Native derives closure-only source, exact registry and effective files/modes/order; rejects extra/missing paths/owner/catalog/definition/V/E changes; host selectors excluded. Profile-only change affects provenance/revision under exact recipes. |
| Semantic/protocol/resource failure caught by Lua | First terminal failure survives; no fresh owner/budget/unbound retry. Partial factory allocation and move-assignment cleanup retain safety. |
| Early replay with every input deleted; early conflict/format1 miss | Saved bytes/conflict/unsupported before capture or external Lua; read snapshot closes. |
| Concurrent exact late replay, different body, stale CAS | Equal replay before candidate inspection/CAS; conflict before CAS; different IDs one CAS winner. Prepared receipt callbacks/Lua do not run on replay. |
| Storage extraction and completion | All prior recordwriter abort/move/metadata-fault/collision/atomicity/commit-uncertain/transport cases preserved; one common transaction implementation. Native faults before/at/after commit distinguish rollback from uncertainty. |
| Limits/lifetimes | Boundary allocations/retained sizes charged; stages/nesting/moves cannot reset counters. Lower native admission/content ceilings still distinct until later shared composition. |

## Bounded delivery sequence and independent gates

Every row receives fresh search, coder and reviewer with exact shared-checkout
ownership and preserved candidate/evidence. No row is self-approved. The
coordinator scopes builds and source ownership after search; future paths below
are planned, not authorized edits in this documentation round.

Operator amendment, 2026-10-25: to cut round count, **Whole-invocation
composition** and **X06 integrated evidence** run as one round (budget work first,
then the integrated X06 gates on that frozen candidate). X07-A and X07-B run as
one round with two coder waves, consistent with the existing rule that the public
switch ships only in one frozen candidate with its service and replay tests. The
actual Linux gate runs inside X08 rather than as a separate round. Every
acceptance obligation in the rows below and in the external-profile rows is
unchanged; only the round boundaries merge.

| Round boundary | Owned product scope and required acceptance |
| --- | --- |
| Native completion dependency extraction | New ledger/storage_types.cppm and ledger/storage.{cppm,cpp}; move native helpers and completion to ledger/acceptance.{cppm,cpp}; acceptance/intent.{cppm,cpp}; scoped format/prepared/read-consumer imports and engine CMake/test variants. No profile writer. Preserve exact embedded APIs after changed import, every native boundary/atomic/recovery case and A3 readers. Dependency audit must prove no target cycle, all generated variants build, no raw handle/callback escape. |
| Original source-capture prerequisite | New acceptance/source_capture.{cppm,cpp}; delegate existing load internals, scoped capture bind forwarding only if search proves necessary. The bound factory requires original workspace root, selected declaration-file and registry receipts. Tests demonstrate pre-registration known-load metadata admission of both root and declaration, disjoint-directory module/declaration hard-link refusal before external chunk execution, replacement between registration and first content refusal, original-file same_selection/actual-leaf verification without receipt replacement, and retained discovery/seal/move/precommit/poison across real profile admission. Include a positive distinct-file registry-colocation case; no source content is read during admission and existing metadata charges/terminal failure remain. Existing profile transaction refusal remains. |
| Authentic runtime preparation | runtime.{cppm,cpp}, colocated tests, new internal files in the same named module if useful, explicit CMake dependencies. Implement opaque invocation/prepared classes, known/opaque construction, private adoption, phase8 then all-job phase0/1/2 and native v2 derivation. No caller result upgrade; direct scratch-native tests only. Frozen prepared objects retain both receipts; no public profile CLI or fresh transactions yet. |
| Native profile transactions | ledger/acceptance overload/private adapter and tests. Consume opaque runtime owner via the shared common algorithm, preserve replay/CAS/format/error/receipt/uncertainty order and embedded behavior. No init/envelope cutover; require original receipt races and no-Lua-under-lock evidence. |
| Whole-invocation composition | Scope after accounting inventory: factory-owned cumulative budget across capture/register/declarations/shapes/conversion/preparation/result, preserving lower limits. Exhaustion/caught failure/moves/no reset/refund tested. This is separate from merely retaining existing metrics. |
| X07 design addendum, before X07 | Docs-only prerequisite of X07: [x07-addendum.md](x07-addendum.md) records the closed factory over the five shipped families, the production outcome table and the carried-limit records, reviewed records-only before any X07 code. |
| X06 integrated evidence, then X07 | Focused nonzero zero-skip gates first; complete agreed macOS Debug/RelWithDebInfo gates preserving current918 names and all prior evidence; lint/Doxygen/no-work builds as applicable. X07 uses original catalog owner through real CLI11 parse and built-in/extension continuation, early replay and format2 init cutover only in its approved frozen candidate. X08 separately proves mandatory external-profile acceptance. |

No unresolved product choice is introduced by this design: selector, trust,
shape, replay, provenance and failure order are already fixed by exact contracts.
Implementation search may find engineering changes needed to preserve the stated
invariants; it must revise/review this architecture before dependent code when
those alter constructor or storage authority, rather than inventing a new public
permission surface. Product ambiguities, if actually discovered, return to the
operator with their governing source.
