# External profile implementation path — X02 candidate

X05-C5 is [independently approved](rounds/V5-02-X05-C5-review.md):
the complete macOS Debug and RelWithDebInfo gates each pass **876/876**, with zero
failures/errors/skips/disabled cases; ten private cases/709 assertions also pass
per configuration. Prior evidence and its original limitations remain preserved.

X06-A1/A2 and [X06-A3 recorded-history integration](rounds/V5-02-X06-A3-review.md)
are independently approved. Actual pinned embedded/effective provenance, workspace
and digest verify; known provenance permits exact source export even when the
interpretation is safely unsupported. Saved profile accepted/noop requests replay
through the existing exact path; fresh profile acceptance remains refused. No
historical Lua or current profile/runtime executes. Current inventory is 918;
the latest complete approved macOS gate remains 876 per configuration.

[X06-A4 architecture](profile-preparation-architecture.md) is a documentation
candidate; combined A4+C01 awaits independent review of the correction to
[X06A4-R1](rounds/V5-02-X06-A4-review.md#required-change-x06a4-r1--admit-the-known-selected-workspace-file-before-registration).
Known load must metadata-admit the selected workspace declaration file alongside
its root before external registration and retain both originals through native
precommit verification. It selects an original captured/registered
runtime owner through final parsing, authentic all-job preparation and a higher
shared native transaction consumer, with a downward storage dependency refactor.
It implements none of those APIs and runs no product builds/tests. Later X06
preparation/transactions/shared budgets, X07, X08, execution/RA01 and Linux remain
open. Public init creates format1; no RSS guarantee is claimed.

The checkpoints below retain their historical scope; this opening supersedes
stale current-status and future-slice wording. C5 approval establishes the latest
complete approved gate at 876/configuration.

The X05-C4-D3 candidate adds lazy role-only `read_role({})` under the original
captured profile/invocation and same retained VM. Original root/file receipts
bind native capture; all-job shapes and observation assertions validate before
exact immutable role publication. Nested trusted calls preserve outer roots and
cumulative limits. Role+projection, public CLI, profile-ledger acceptance and X06
remain open. Focused Debug evidence is in
`tests/native/v5-02-x05-c4-d3-evidence/REPORT.md`; independent review is pending.
The latest approved full gate remains 676/configuration; no release/Linux gate is added.

X05-C4-D2-B2-C01 corrects the B2-R1 original-root correspondence refusal.
Before an initial explicit retained read, the bound original root discovers the
relative path and requires exact `same_selection` with the incoming receipt.
Actual leaf-FD equality and shared ownership alone do not establish selection.
The incoming original token remains the captured authority after equality; cached
reads, tree revisits, seal and acceptance never rediscover or replace it.
The temporary comparison receipt and discovery staging retain existing cumulative
metadata charges without refunds. Failed correspondence, discovery or publication
poisons original root/session and incoming authority before error adaptation.
No admission API, budget category, role service or public CLI is added.

B2's independent review requested this correction; combined B2+C01 awaits fresh
independent review. Focused Debug evidence and actual pre-fix regression are in
`tests/native/v5-02-x05-c4-d2-b2-c01-evidence/REPORT.md`. D1, D2-A, D2-B1
and C4-C2 remain independently approved. Latest full approval remains
676/configuration; no full/release/Linux, X06 or RSS gate is claimed.

X05-C4-D2-B2 adds internal original-authority root discovery. A move-only
`retained_root_receipt` shares the original admission and supplied-root selection;
portable relative files are metadata-admitted through its retained descriptors
before content access. A charged root/path index preserves repeated selections,
with bounded private ordinal keys in a separate namespace. Existing explicitly
admitted files retain their original selection even before any native cache entry.
Root identity/mode and the actual leaf FD verify before enumeration/content;
owned cached entries retain their file authority and mixed modes cannot replace it.
Bound failures poison original shared authority and native ownership before error
adaptation, without an unbound retry. Initial tree discovery uses this authority;
final tree membership comparison admits and reads no new member. Unread roots,
wrapper destruction/moves, cached rereads and acceptance retain original receipts.

The narrow `capture_session::add_root` forwarder accepts root authority so existing
`load` declaration/reference reads use it automatically. Capture imports native
and the Lua declaration loader; it adds no profile/runtime import or role semantics.
Original metadata/content limits stay separate; no X06 shared-budget or RSS claim
is added. Focused Debug evidence is in
`tests/native/v5-02-x05-c4-d2-b2-evidence/REPORT.md`; independent review requested B2-R1, addressed by the C01 candidate above.
D1, D2-A, D2-B1 and C4-C2 are independently approved. Role services, public CLI,
profile-ledger acceptance, X06–X08, mandatory external-profile acceptance,
execution/RA01 and Linux remain open. Latest full approval remains
676/configuration; no full or release gate is claimed.

X05-C4-D2-B1 adds internal original-receipt standalone input binding. Native
`add_input` and profile `bind_input` verify the actual selected no-follow leaf FD
before metadata-only publication, retaining the same original supplied-file token
pending the first read. Physical parent `..` is opened without lexical cancellation;
registry colocation keeps the original file policy without supplied-root admission.
Plain first reads move that handle into the captured entry; pending and captured
mixed modes cannot replace, upgrade or downgrade authority. Unread seal/acceptance
opens and verifies metadata only; input roots expose only `input.json` and refuse
tree enumeration. Binding/read failures poison original shared state and bound
sessions before error adaptation, with no unbound fallback. Wrapper destruction
and input-root relocation preserve authority. Original metadata/content limits
remain distinct; no X06 or RSS guarantee is added. Focused Debug evidence is in
`tests/native/v5-02-x05-c4-d2-b1-evidence/REPORT.md`; independent review is pending.
D1, D2-A and C4-C2 are independently approved. Root discovery, declaration/reference
wiring, role services, public CLI, profile-ledger acceptance, X06–X08, mandatory
external-profile acceptance, execution/RA01 and Linux remain open. Latest full
approval remains 676/configuration; no full or release gate is claimed.


X05-C4-D2-A adds an internal retained supplied-file receipt candidate. A move-only
handle shares the same original private admission and immutable selection index;
only already admitted supplied-file slots qualify. Metadata descriptors survive
admission/profile wrapper moves and destruction. Explicit retained native reads
store that authority and verify the actual opened file before content at initial
read, cache reuse through every overload, existing tree visits, seal and acceptance
verification. Cache authority cannot be upgraded, replaced or downgraded: retained
repeats require the same selection; borrowed reads of retained entries refuse;
plain reads use the stored original authority. Unbound and borrowed entries retain
their previous behavior. Refusals poison original shared state before error copying.

The profile facade forwards through its private original admission. Its snapshot
observes late native poison without filesystem access; in-progress terminal guards
poison only on failure. Fixed shared state and each issued handle plus possible
native optional storage are charged to existing admission metadata accounting.
The native content ceiling remains content-only; file-count bounds limit inline
record growth. These separate metrics do not claim process RSS or X06 accounting.
Trusted profile content verification remains separate, without ownership cycles.

Focused Debug evidence is in
`tests/native/v5-02-x05-c4-d2-a-evidence/REPORT.md`; fresh independent review is
pending. D1 and C4-C2 are independently approved. Discovery/load/reference wiring,
new bound tree discovery, role services, public CLI, profile ledger acceptance,
X06–X08, execution/RA01, Linux and the mandatory external-profile gate remain open.
Latest full approval remains 676/configuration; no full or release gate is claimed.

X05-C4-D1 adds an internal metadata-only supplied-root binding candidate. The
original admission verifies an opened root's identity and mode plus all retained
relationships and name edges; only original supplied-root slots are eligible.
Native binding verifies before overlap checks or publication and walks physical
parent components without lexical cancellation. The profile owner forwards through
its private original admission and becomes terminal on refusal. No tree enumeration,
content capture, role service, Lua, public CLI or shared invocation budget is added.
Focused Debug evidence is in `tests/native/v5-02-x05-c4-d1-evidence/REPORT.md`;
fresh independent review is pending. Latest full approval remains 676/configuration;
release, Linux, X06–X08, execution/RA01 and the mandatory external-profile gate stay open.

The X05-C4-C2 candidate runs the retained history+projection handler with lazy
`read_history({})` then optional once-only `project({id=ID})`. Only the exact
history/projection capability pair is newly admitted. Call-time lookup binds the
active original owner/context and sealed same-VM projection, retaining the public
pre-invocation guard. The approved C4-C1 nested frame is reused without resets;
original native history and receipts remain authoritative.

Native preparation supplies the mandatory fully headed `recorded_history` section
and exact 1 MiB budget to the original trusted assembly export, now retained in
the initial audit/freeze root set without changing shipped Lua bytes. Assembly
keeps reverse optional removal and exact omission metadata. Projection DATA is
immutable and compared raw to the private result before response copying.
Performed history DATA remains valid whether or not project was also called;
semantic errors may omit both services. Trusted assembly errors retain their
exact inner E/details through an execute-only `assembly_error` alternative.
First service refusal stays terminal even when caught; outer cleanup and original
receipt verification remain mandatory. The compiled command/VM/invocation layouts
are 1048/1456/528 bytes; aligned root/bridge additions and the 56-byte nested frame
are charged before use through existing cumulative accounting.

Native failures inside selected projection resolution, F, scan and conversion
retain the original `catalog.projections[N]` location. Resource/protocol fallbacks
and that field are charged/staged before the phase; failure adaptation moves
owned storage before fallible copies. Command admission and trusted assembly keep
their separate attribution. This adds no persistent layout or public resolver
behavior change.

C4-C2 awaits fresh independent review. Focused Debug evidence is in
`tests/native/v5-02-x05-c4-c2-evidence/REPORT.md`. C4-C1 and combined B3+C01 are
independently approved. The latest full approved gate remains 676/configuration;
no full, release, Linux, public CLI, profile-ledger acceptance or X06 shared-budget
gate is claimed. Role-read, X06–X08 and execution/RA01 remain open. Public init
continues to create format1. Earlier evidence retains its historical scope.

X05-C4-B1 and C4-B2 are independently approved native read foundations.
The C4-B3 candidate runs an admitted history-only retained handler with lazy
`context.read_history({})` bound synchronously to its original moved capture,
ledger receipt and private exact selector. No ledger contents are read if the
handler returns a semantic error without calling the service. The original B2
helper closes every native snapshot, transaction, connection and descriptor
adapter before any Lua result allocation or execution resumes.

The immutable invocation exposes an opaque ledger slot and exact ID strings,
scalar defaults and presence flags. The service is once-only; failed service,
invalid arguments, repeated calls and missing declared-service access poison the
invocation even when caught. Execute preserves original typed registration,
format and history errors. History success must match the retained native facts
exactly; raw comparison precedes owned response copying. Original receipts are
reverified before release. Pure handlers retain their summary/error protocol.
Manual native history preparation cannot bypass handler service sequencing.
The B3 review requested C4B3-R1: proxy-aware rawget bypassed sticky missing-service
refusal. The corrective B3+C01 candidate binds both lookup paths to the exact live
context and one registry backing, so history exposes one method consistently to
indexing/rawget/next/pairs and pure contexts enumerate none. Missing known methods
still return nil and poison even without a call; unrelated tables keep ordinary
lookup semantics. The extra pointer adds exactly eight compiled/charged bytes;
cleanup clears its identity before releasing rooted context storage.
Combined B3+C01 is independently approved. Corrective focused Debug evidence
is in `tests/native/v5-02-x05-c4-b3-c01-evidence/REPORT.md`; original B3 and failed
review evidence remain preserved.

Role read and capability combinations beyond zero, history-only and the exact
history+projection pair remain closed. X06 recorded profile interpretation/acceptance and shared
invocation accounting, X07 public cutover, X08 mandatory external-profile
acceptance, execution/RA01 and Linux remain open. Public init creates format1.
The latest complete approved gate remains 676 per configuration; no new full or
release gate is claimed. Earlier evidence retains its historical scope.

Current integrated shape checkpoint: [X05-B4-C evidence](x05-shape-evidence.md) records one full
Debug **676/676** gate (268.37 seconds) and one release-preset RelWithDebInfo
**676/676** gate (126.24 seconds), each with zero failures/errors/skips/disabled
cases and all 676 candidate plus 582 A4 names preserved. Six common vectors,
per-configuration source/runtime audits, pinned formatting/Doxygen over 118 active
sources and no-work builds pass. This verification-only gate is independently approved by the B4-C review;
the latest approved full gate is 676 per configuration. The B2-C+C01+C02, B3-A, B3-B+R01, B3-C, B4-A+C01 and
B4-B+C01 slices are independently approved; B4B-R1 is closed. Historical private
fault/retained-callable probes remain approved Debug evidence, without release
rerun claims. Public init still creates format1. C projection/read services, X06
recorded interpretation/history/acceptance and receipt/whole-invocation orchestration,
X07 public cutover, X08 mandatory external-profile acceptance, execution/RA01 and
Linux remain open. Detailed later historical checkpoints retain their original
limits; this current checkpoint supersedes their stale future-slice wording.

X03-A, X03-B1, X03-B2-A and the combined X03-B2-B+C01 native implementation
are independently approved; C01 closes B2B-R1. The independently approved [X03-B3 verification gate](x03-format-evidence.md)
runs full debug and RelWithDebInfo gates: 430/430 each, zero failures/errors/skips,
pinned lint/Doxygen and no-work builds. [Independent B3 review](rounds/V5-02-X03-B3-review.md) closes X03-B. Native format2 acceptance uses immutable request-v2 embedded
intent, codec-aware exact interning and explicit recovery under retained limits.
Public init still creates format1. Fresh format2 request-v1 and profile acceptance
refuse; profile accepted history remains unsupported until X06 provenance/
interpretation support. B2-A SQL reader fixtures provide no conversion tooling.
The mandatory external-profile gate, public cutover, Linux and execution remain open.


Status: independently approved X02-C01 design with operator-selected P1 A/P2 A.
The [exact contract](external-profile-contract.md) is the reviewed amendment.
[X02-C01](rounds/V5-02-X02-C01.md#operator-disposition--2026-10-22) records the
policy disposition and authorizes dependent implementation. Current public
commands remain render, init, history, export and load; external runtime admission
and X08 acceptance remain open. R14-D8 and X01 evidence remain unchanged.

## Gate and delivery discipline

[X02-C01 review](rounds/V5-02-X02-C01-review.md) independently approved R1–R3,
and the operator selected P1 A/P2 A. This closes the design/policy gate for
dependent slices. Review approval is design approval; it is never product test
evidence. Accepted A/B/C and read/projection-only authority remain fixed.

Each slice uses fresh ordinary agents sequentially: Luna-high search,
Sol-medium coder, Sol-high independent reviewer, per CLAUDE.md. A scoped brief
records starting revision, complete pre-existing changes, explicit exact files,
commands/count expectations, owned build directory and raw evidence location.
The checkout is shared: forbid reverting others, preserve intentional rewrite,
protect historical contracts/evidence and never invoke legacy Planar tools,
skills, queue, installed binaries or operator data. No commit/install/publish/merge
is authorized. Approval advances ordinary reviewed slices without repeated operator
permission; request-changes starts fresh search/coder/reviewer corrective round.
Completed agents are retired from dispatch, never reused. Parallel coding requires
new documented justification and disjoint ownership.

## Source seams and bounded implementation rounds

The paths below identify ownership **destinations**, not implemented APIs or a
license to edit an entire directory. Fresh search scopes exact existing files
and necessary new module/test pairs before each brief. X02 changed no product source. Generic tooling stays in scripts, all runtime C++/Lua stays in src.
Only X07 switches public admission; earlier seams are native/unit test-only.

| Round / dependencies | Bounded owned files and responsibility | Required independent gate |
| --- | --- | --- |
| X03-A request/record codec, after X02 | New `src/engine/history/request_v2.{cppm,cpp,t.cpp}` and narrow module/test wiring; existing record/prepared modules unchanged | Exact known-byte request vectors including discriminant, lengths and full digest; malformed/trailing/overflow/unknown tags, actor/path/head bounds, v1/v2 exact inequality. Typed coexistence representation and forced interner request collision/redaction proof are explicitly allocated to X03-B with their storage consumer; pure codec tests do not prove storage collision behavior. Preserve all old record-v1 vectors/readers and old W. No schema opener/CLI switch. |
| X03-B format coexistence, after X03-A | `src/engine/ledger/{types.cppm,format.cppm,format.cpp,format.t.cpp,acceptance.t.cpp}`, narrowly required `src/engine/history/read.*` and `src/engine/acceptance/prepared.*` typed request coexistence seams, colocated tests | Separate exact schema1/schema2 verifier and new-target initializer; no conversion. Format1 old loads byte-exact, v2 request records only in2, old binaries/future/foreign formats refuse unchanged without journal mutation. Forced v1/v2 request interner collision/redaction proof and typed request coexistence. Format2 CAS/replay/atomicity/crash/disk-full/collision/backup new-target safety under existing recovery limits. Format2 schema has only contracted delta. |
| X03-B1 schema/native lifecycle, after X03-A | `src/engine/ledger/format.{cppm,cpp,t.cpp}`, narrow test wiring and owned scratch support | Exact selected schemas, retained format discriminator and explicit native format2 new target; preserve format1 public init until X07. Backup/restore preserve identity/format, actual pre-B1 binary refuses unchanged, staged/no-overwrite faults. Independently approved partial lifecycle. Format2 writable/recovery guards remain through B2-B; B2-A reader candidate supplies codec2 embedded reads. No request-v2 native acceptance claim. |
| X03-B2 typed storage/acceptance, after reviewed B1 | Original X03-B typed request/prepared/storage/history seams and native tests | Complete v1/v2 stored request dispatch/equality/interner and collision redaction; remove B1 temporary restrictions only with reviewed typed integration. Full format2 CAS/replay/atomicity/crash/disk-full/recovery obligations remain mandatory. No public cutover before X07. |
| X04-A native profile closure, after X02/P2 | New `src/engine/capture/profile.cppm`, `.cpp`, `.t.cpp`; `native.cppm/native.cpp` only shared no-follow/receipt integration | Strict Glaze JSON; every field/duplicate/version/path/byte/node/count boundary; 0/64 modules; ordered capture, no discovery/bytecode/require; final/intermediate symlink, hardlinks/case/ancestor/race aliases; retain original bytes/modes; selected closure unaffected by unrelated siblings. Prove known shipped slots after early replay miss and before external evaluation, plus late extension slots after sealed final CLI11 parse and before handler/read-service use; retain initial receipts and recheck result/commit seals. Registration may precede late-known checks under selected P2 A; no shadow argv scan or untrusted content before checks. No Lua executes in this round's service. |
| X04-B1 prerequisites, after X04-A/X03-A | Pure `capture/profile_codec.*`; scoped `history/tree_core.*` | Extract approved decoder unchanged; distinct effective32 MiB/8192-file budget with inherited lower limits. Independently approved B1; focused177/177. |
| X04-B2 catalog, after B1 | New `src/engine/history/effective_catalog.*`, value/canonical dependencies only | Candidate pure catalog-v2 codec validates closed descriptors, sorted identities/options/capabilities, exact native shipped pairings, source owners and external static read signatures. Bounded canonical sealing severs caller aliases. Historical callability is not re-proved; B2 is independently approved. |
| X04-B3 effective provenance, after B2 approval | New `src/engine/history/effective_provenance.*` plus pure codec tests; existing provenance_codec only compatibility dispatch if required | Exact tree/manifest fixtures including embedded originals/assembly, raw whitespace profile, order, module bytes/modes/catalog; no host selector; bound32 MiB/8192 files and other lower bounds. Reconstruct/verify with no runtime import or Lua; malformed order/owners/extra/missing files/digest collisions refuse. Old embedded provenance fixtures byte-exact. |
| X05-A registration/sealing, after X04-A | `src/engine/runtime/{runtime.cppm,runtime.cpp,commands.t.cpp}`, new colocated extension registration tests; narrow engine Lua sandbox binding source if needed | Same VM/retained source identity, assigned owner and atomic full-catalog admission; mandatory shipped signatures; all duplicate/reserved/foreign-owner/unknown/capability cases; immutable aliased upvalues/tables/callables; later-module replacement attempts; actual alternative handler dispatch. Cumulative caught memory/instruction/native conversion exhaustion, no C++ destructor skipped by longjmp. No profile flags admitted. |
| X05-B shape semantics/identity, after X05-A/P1 | `src/stdlib/planar/{schema.lua,semantic.lua,selection.lua,render.lua,acceptance.lua}`; new `shapes.lua` if search warrants; colocated runtime tests and `tests/stdlib/` tests; narrow embedding list | New custom shape on unchanged version1 job fields; all-job validation; stages required/forbidden/optional; added obligations/cycles including grouping, unknown refs, attempted weaker checks/reviewer/ownership refused. Native integration proves W recipe and custom render envelope, not only fake engine. Old shipped W/Markdown/JSON/goldens exact with and without unselected extensions. Selected module/semantic revision affects only job roles; unselected module/projection changes provenance only; same-module visual coupling documented. |
| X05-C projections/read services, after X05-A/X05-B | New `src/stdlib/planar/projections.lua`; bounded `runtime.*`/new extension provider module; `handlers/history` and `handlers/render` only extract shared lower service if needed; native/stdlib tests | Read-role/history result correspondence, mandatory initial checks for every late native slot before handler/read-service use with retained receipts and result rechecks, native slots/one-call order, malformed response/error redaction, capability bypass/forged handle/cross-family/double/skipped/failed service refusal; pure projection receives sealed data, mandatory base sections retained and budgets/omission bytes exact, including the 87→94→59-byte multi-removal vector, omitted/notice [b,a] and original-order metadata. Prove history_id capability/name/type restrictions, absent string "0"/present=false, Lua string at 2147483647/2147483648/INT64_MAX and overflow/lexical/selector-conflict failures; general decimal remains INT32, limit1..100/default50; current/recorded Lua not used by history decoder. No mutation/process/arbitrary IO service. |
| X06 v2 recorded interpretation/preparation, after X03-B/X04-B/X05-B | New `src/engine/history/interpretation_v2.*`; existing interpretation_codec/read dispatch only; `src/engine/acceptance/{load,prepared}.*` as mapped; native tests | Exact interpretation/versions/C/selected-shapes correspondence and 16 MiB bounds; native derived provenance/versions/source membership; same VM bundle cannot be substituted. Both v1/v2 historical structural decoding independent of runtime, even after profile/runtime removal; source export unchanged; no Lua under lock. Profile receipts participate final transaction seal; early/late exact replay precedes CAS. |
| X07-A envelope parser integration, after previous approved rounds | `src/cmd/planar/{bootstrap.cppm,bootstrap.cpp,bootstrap.t.cpp,invocation.cppm,invocation.cpp}`; scoped response/extension family modules | Combined production pinned CLI11 envelope/opaque probe/validator, all X01 argv rows plus final text ABI/help ordering; shipped malformed signature before missing profile; leading flag before profile; unknown verb after profile validation; final syntax/conflicts before late-slot overlap, overlap before handler/service; known shipped path checks before external evaluation after early replay miss, and opaque extension path checks only after pure registration/seal/final CLI11 parse; no shadow argv scan. One cwd/env snapshot; option storage lifetime. Fixed embedded help unchanged; selected help exact. Public switch only with complete following service/replay tests in same frozen candidate. |
| X07-B service/CLI admission with X07-A | New `src/cmd/planar/handlers/extension/command.*`; existing load/native provider orchestration, minimal CMake wiring; `tests/native/` realistic scratch CLI suites | Unchanged init CLI creates new format2 targets after cutover; full profile custom shape load/render + custom verb/projection + recorded read/export. Repeat X05-C history_id boundary/type/presence/selector and decimal-limit vectors through real CLI; reject capability/type/name misuse at registration. Extension role-read workspace/source/base/observation overlap and aliases refuse before handler/read service even if a handler would skip it; initial late-slot receipts survive to result seal. Record profile-before-final-syntax and final-syntax-before-overlap precedence. Early saved replay with deleted profile/modules/workspace/registry and advanced head; changed selector/actor/head/targets conflict; same selector changed bytes fresh ID captures new provenance. Exact locked relookup races: equal replay before stale CAS, unequal body conflict, different IDs one CAS winner; known/late root/file/ancestor receipt races refuse before result authorization; final profile-root/file/ancestor race rollback before load commit, with no Lua under lock. Postcommit allocation/Lua/encoding/stdout failures keep completion receipt and old recovery fallbacks. |
| X08 full external-profile acceptance, after X07 review | New verification-only native CLI/adversarial tests, fixtures and frozen evidence; current docs references and testing docs; product fixes require fresh corrective round | Full serialized debug/release gates with nonzero counts/zero unexplained skips; preserve all405 baseline names, old fixtures/goldens/W; lint/Doxygen and no-work builds, relevant source variant audits; historical reads/export after profile and current trusted runtime removal. Exercise complete requirements table below. Linux remains explicitly pending until actual Linux gate. |

Build wiring in cmake/rewrite.cmake is assigned narrowly when a round adds modules;
tests remain outside default all. Do not broaden ownership ad hoc. Interface or
storage changes outside this exact amendment require reviewed amendment before
implementation. If a round becomes too large, split it before coding while
retaining the final atomic X07 admission gate. No interim permissive catalog or
public profile flag without complete safety/provenance/replay composition.

Current source anchors: [CLI11 composition](../../src/cmd/planar/bootstrap.cpp),
[path snapshot](../../src/cmd/planar/invocation.cpp),
[retained runtime](../../src/engine/runtime/runtime.cpp),
[receipts](../../src/engine/capture/native.cppm),
[prepared acceptance](../../src/engine/acceptance/prepared.cppm),
[record framing](../../src/engine/history/record.cpp),
[pure embedded provenance](../../src/engine/history/provenance_codec.cpp),
[interpretation reader](../../src/engine/history/interpretation_codec.cpp),
[ledger verifier/locked acceptance](../../src/engine/ledger/format.cpp),
[schema](../../src/stdlib/planar/schema.lua),
[semantic graph](../../src/stdlib/planar/semantic.lua),
[selected W](../../src/stdlib/planar/selection.lua),
[current packet budget/projection](../../src/stdlib/planar/render.lua).
These are existing seams; external APIs above are not present there yet.

## Requirement-to-evidence obligations

Identifiers X-* name contract sections, not executed tests. Operator choices are
P1/P2. Local external requirements E01–E14 avoid renumbering v5 A01–A36.
Every later round report must cite actual test names/raw outputs and record its
portion, preserving unresolved execution and platform obligations.

| Requirement | Exact candidate boundary | Implementation proof / v5 portions |
| --- | --- | --- |
| E01 explicit ordered authority | X-CLI/X-CAP, no discovery/aliases/overrides/job authority | X04/X05/X07; A08/A31 |
| E02 parser/error/help precedence | X-CLI/X-CAP, retained original tail; profile/registration before final syntax, final syntax before late-slot overlap, overlap before handler/service; early replay first | X07, all X01 matrices plus phased path/error and history ID syntax/conflict integration; A01/A08 |
| E03 trusted closure separation | X-CAP/P2, known shipped slots before external evaluation after replay miss; late extension slots after pure registration/final parse, before handler/read use; initial receipts retained | X04/X05-C/X06/X07 phase/alias/race tests and result/commit rechecks, no untrusted contents before initial checks; P2 A selected; A08/A18 capture portions |
| E04 bounded source-only JSON/modules | X-CAP/X-LIMIT, strict fields/aliases/bounds and no bytecode/require | X04/X05 adversarial boundary and cumulative-budget tests; A08 |
| E05 immutable provenance/owners | X-REG/X-STORE, same VM captured originals/manifest/callables | X05/X06 real semantic dispatch/substitution refusal; A01/A08/A27 |
| E06 full custom shapes | X-SHAPE/P1, existing declaration fields plus additive semantics | X05/X06 all-job/cycle/mapping/check/ownership tests; A02/A04/A30–A32 |
| E07 shape identity/materiality | X-SHAPE, selected definition and old W preservation | X05/X08 scoped mutations, independent exact-byte old vectors; A01–A05 |
| E08 custom pure projections | X-PROJ, mandatory base sections; reverse-result optional removal, omitted IDs/notice in removal order, metadata original order | X05-C/X07/X08 actual packet and 87→94→59-byte vector with LF joins, notice and omitted counts; A03/A07/A08 |
| E09 closed read-only new verbs | X-REG/X-PROJ, capability-bound history_id through INT64_MAX as Lua strings; general decimals INT32; exact read/response correspondence | X05-C/X07 boundary/type/absence/presence/selector-conflict and capability/name restriction matrix, native initial slot checks before handler/read services; A08, no execution claim |
| E10 selector intent and removed-profile replay | X-CLI/X-STORE request_v2, no profile digest in body | X03/X06/X07 exact replay/conflict/race/lost-output; A25–A29 acceptance portions |
| E11 byte codecs/format coexistence | X-STORE, exact SQL/record/provenance/interpretation versions | X03/X04/X06 preservation/refusal/roundtrip/collision tests; A23/A27 |
| E12 historical reads without policy | X-STORE, retained bytes + native structural readers | X06/X08 remove source/profile/runtime and restart/read/export; A28/A29 recorded portions |
| E13 transactional safety | X-CAP/X-STORE, retain known/late initial receipts; recheck before result authorization and applicable load commit, no Lua/callback under lock | X03/X04/X06/X07 forced crash/busy/full/CAS/known-and-late receipt races/postcommit transport, early/late replay precedence; A23/A26 |
| E14 complete delivery before dependent jobs | All sections, no runtime proof from docs | X08 full gates, then RA01 permitted; A08/A35/A36 remain later authoring work |

## Verification and handoff protocol

For X02 (and fresh X02-C01 correction evidence), freeze complete baseline outside .git/build and this new evidence root,
plus baseline git status/diff/HEAD and owned-document bytes. Validate all candidate
local links/anchors, internal schema/ID consistency, E01–E14 section/round coverage,
protected current contract/input/evidence hashes, AGENTS symlink, and additions/
changes against explicit ownership. Save machine-readable reports and exact
candidate/owned diff hashes in `tests/native/v5-02-x02-evidence/` for original
X02, and `tests/native/v5-02-x02-c01-evidence/` for this correction. Preserve
original X02 candidate/review/evidence. Correction checks cover R1–R3 definitions,
phased admission, history ID and multi-omission vectors and future test obligations;
they remain text/byte-arithmetic checks, not executable schema/runtime tests. Candidate manifests
exclude coordinator round and review files, which have separate owners. Self-hash
manifests are excluded from their own entry sets. Report observed limits honestly;
text checks do not prove runnable codecs, Lua isolation or OS confinement.

For code rounds, direct commands agreed after fresh search: serialized
`cmake --build build/rewrite-debug --target all planar_tests`, focused registered
CTest cases with nonzero expected counts and zero skips, pinned
`python3 scripts/rewrite-check.py lint --build-dir build/rewrite-debug`, followed
by no-change build. X07/X08 run complete debug/release gates via current documented
`make test-all` equivalents, with one owner/build directory at a time. No old host
queue, installed binary or real database. Preserve failing regression before fix;
retain raw argv/UTC/status logs, JUnit/LastTest before inventory, baseline/candidate
and artifact manifests and exact comparison reports. Do not count an unsupported
format refusal as successful profile load or a fake Lua engine as native proof.

X02 author releases document ownership and both build directories idle for a
fresh reviewer. Only independent `approve` followed by current-candidate hash
verification lets coordinator record approval; P1 A/P2 A are now operator-selected in X02-C01. Design approval does
not close the mandatory external-profile runtime gate; X08 does.

The [X03-B2-B-C01 correction candidate](rounds/V5-02-X03-B2-B-C01.md) reuses complete metadata equality,
including the selected ledger format, at locked transaction entry and both native
precommit boundaries. Controlled verified-metadata fixtures exercise refusal and
rollback in both formats; they do not reproduce an external format change.
C01 independently approves the correction and combined B2-B candidate.
The [X03-B3 review](rounds/V5-02-X03-B3-review.md) independently approves the full
430-case debug/release gate and closes X03-B; the external-profile gate remains open.

The [X04-A1 review](rounds/V5-02-X04-A1-review.md) independently approves native metadata
admission and descriptor binding; its inventory is 451 (not a full 451-test run).
The [X04-A2 review](rounds/V5-02-X04-A2-review.md) independently approves strict bounded
profile/ordered module capture and retained staged input receipts. The
[X04-A3 integrated native gate](x04-capture-evidence.md) is independently approved by the X04-A3 review: debug and RelWithDebInfo each pass 489/489 with zero
failures/errors/skips, preserving all 488 prior names. No external Lua, public
profile CLI, provenance acceptance or whole-invocation budget sharing is inferred;
the mandatory external-profile gate remains open.

The [X04-B1 prerequisite candidate](rounds/V5-02-X04-B1.md) extracts
the unchanged bounded profile decoder into the pure `capture/profile_codec` module
and adds the distinct 32 MiB/8192-file `effective_provenance` tree budget.
Focused debug verification passes 177/177 with zero failures/errors/skips; the
497-name inventory preserves all 489 approved A3 names. The independent
[X04-B1 review](rounds/V5-02-X04-B1-review.md) approves these prerequisites.

The pure X04-B1/B2/B3 slices are independently approved. B2 validates and
immutably seals catalog-v2, including source owner ordinals, exact shipped native
signatures and external static capabilities. B3 assembles and structurally verifies
exact `effective_stdlib_v1` bytes, modes, ordered modules, old embedded assembly,
catalog owners and full membership. The catalog-only 1 MiB ceiling and combined
canonical manifest 1 MiB ceiling are separate checks. B1 tests the outer
32 MiB/8192-file tree budget; valid complete B3 inputs reach at most 4162 files
and remain below 32 MiB under the stricter component bounds.

The [X04-B4 integrated provenance gate](x04-provenance-evidence.md)
is independently approved by the [B4 review](rounds/V5-02-X04-B4-review.md): full Debug and
RelWithDebInfo each pass 520/520 with zero failures/errors/skips/disabled cases,
preserving all 520 prior names. Pinned formatting/Doxygen checks 113 active
sources; vectors, exact source audits and no-work builds pass. No product or test
bytes changed in B4, and tested sources remained fixed through both gates.
This pure structural gate adds no live external registration/Lua, public profile
CLI, storage/profile-history acceptance or history dispatch, whole-invocation
shared budgets, or callability/semantic-honesty proof. The mandatory external-profile
gate, public cutover, execution/RA01 and Linux remain open; public init creates
format1. The independent B4 review closes integrated pure X04-B.


The [combined X05-A0+C01 independent review](rounds/V5-02-X05-A0-C01-review.md) approves bounded
test-only lexical sealing feasibility and closes A0-R1. The
[X05-A1 production foundation](../../tests/native/v5-02-x05-a1-evidence/REPORT.md)
is independently approved in the X05-A1 review. The embedded five-command VM audits
original global/callable roots together before alias-preserving table freezing.
The isolated `runtime/closure_guard` module reads pinned Lua compiler metadata,
rejects retained upvalue writers and unknown reachable native identities, and
permits invocation-local scratch. Exact strong native identity capture precedes
module evaluation; the five native engine callbacks enroll immediately after
native installation, without intervening module execution. Proxy natives are
created only after this audit and are not claimed to pass a later raw audit.

The protected sealing frame reserves 2048 slots before root pushes/native RAII;
inspection entry and every traversal push use nonallocating active-frame checks.
A shared identity/prototype memo preserves existing graph depth256 semantics;
prototype entries include the inherited sealed-cell mask. PMR allocation requests
charge the existing checked cumulative 512 MiB native conversion accounting before
allocation, with no refund or root reset. Static metadata inspection has checked
reported counters and is not charged as Lua instructions or converted nodes.
The finite compiled input remains bounded by the registration Lua heap; no fresh
4 MiB audit arena or public scan quota is introduced. Existing embedded invocation
instruction reset remains unchanged.

A1 does not execute external modules or admit public profile flags. Later X05-A
rounds retain source-owner/ordered module admission, the three descriptor APIs,
module-end audit/seal before later chunks, frozen global/API state, persistent
frozen-identity memo, add-time snapshot versus retained mutation semantics, and
whole-registration cumulative gates. The mandatory external-profile gate, public
cutover, storage/profile history dispatch, execution/RA01 and Linux remain open;
public init creates format1. No full milestone or release gate is claimed for A1.


The [X05-A2 repeatable sealing foundation](../../tests/native/v5-02-x05-a2-evidence/REPORT.md)
is independently approved by the X05-A2 review. A private protected root-argument seam
seals new retained roots after the initial embedded environment boundary. Its
strong native-owned successful-identity set is distinct from each boundary's DFS
memo: the guard treats prior sealed tables/Lua closures as immutable atoms and
the freezer preserves their proxy backing and aliases. New identities publish
only after complete audit/freeze success; any failure leaves the owner terminal,
including partial freezing or publication failure. Direct native callable roots
and unknown reachable native closures still refuse. `pairs` returns the exact
stable global `next` closure captured before module execution; native snapshot
never traverses C upvalues. Repeated sealing never writes frozen globals.

The production public runtime interface and old invocation tail remain unchanged
except that a failed sealing owner cannot invoke. Synthetic root bookkeeping adds
no graph depth; every boundary shares existing cumulative native charges and
inspection totals, without resets. Real private production probes exercise three
successive boundaries, fresh retained roots, prior proxies/aliases, exact iterator
identity across GC, scratch, writer/native refusal, resource failures and same-owner
failed retry. A3 still owns captured ordered modules, owner frames, three exact
registration descriptors and final catalog; final D is validated/copied at module
end before sealing D/F. No external execution, public profile flags, profile
storage/history acceptance or full registration gate is delivered by A2. The latest
full gate candidate is A4 (582 per configuration); the mandatory external-profile gate,
public cutover, execution/RA01 and Linux remain open. Public init creates format1.


The [X05-A3 captured registration](../../tests/native/v5-02-x05-a3-evidence/REPORT.md) adds native
`command_vm::create(profile_snapshot)` with structurally checked, privately copied
captured bytes and module order. Source-only zero-result chunks execute in one
bounded VM with native owner ordinals. Three exact descriptor APIs retain D/F;
final descriptors validate and seal with their callables at each module boundary.
Catalog-v2 and exact effective provenance bind to the original captured bytes/modes.
Selected-profile errors retain redacted native logical fields separately from
existing embedded diagnostics. External `_ENV` is natively created before shipped
evaluation with constructor-only engine, self `_G`, shared pure libraries and
three exact registration functions; original globals and this environment freeze
together before external execution. Embedded handlers retain their original engine.

A3 is registration-only and independently approved with C01–C05 by the
[C05 review](rounds/V5-02-X05-A3-C05-review.md). Profile-owner invocation
refuses before any provider action; retained alternate callable execution is
verification through private reversible production copies. Shipped shape/projection
IDs remain reserved; real adapters and semantics belong to B/C. No shipped Lua,
public CLI admission, profile history/storage interpretation or acceptance changes
are delivered. Native conversion/audit charges and explicit conservative pure-helper
capacity reservations remain cumulative; these are not process RSS accounting.
The mandatory external-profile gate, public cutover, execution/RA01 and Linux
remain open. Public init creates format1; the latest full approved gate is A4
(582 per build).


The [X05-A3-C01 corrective](../../tests/native/v5-02-x05-a3-c01-evidence/REPORT.md)
preserves original entry fields before command-name/shape/projection sorting and
propagates initial provenance preparation's typed resource refusal. Its final
logical reservation enumerates six canonical/payload buffers with separate record
framing/digest room, independently derived embedded metadata encoding/payload and
graph passes, and unchanged eleven embedded/three external source payload passes.
The existing cumulative 512 MiB charged native metric remains; this is not RSS
accounting. A valid catalog near the 1 MiB ceiling and controlled remaining-budget
probes exercise these reservations. Combined A3 through C05 is independently
approved; A4 records the latest full approved gate. Profile CLI, B/C/X06, public
cutover, execution/RA01 and Linux remain open; public init creates format1.


The [X05-A3-C02 location correction](../../tests/native/v5-02-x05-a3-c02-evidence/REPORT.md)
retains the selected original root through the guard's single shared audit and
maps module D/F pairs to one-based original catalog fields. Actual root freezing
uses the same mapping; failures before root selection and during identity-set
union use the active `profile.modules[N]` boundary. The existing enum-only guard
API delegates to the detailed result. Initial embedded/environment sealing and
private repeated-root sealing do not acquire external entry locations. Terminal
module retries preserve the first location; the public constructor discards failed
owners rather than retrying. Original C01 fixes and cumulative budgets remain.
Controlled probes exercise actual pre-root, partial-freeze and union refusal,
terminal state and cleanup; they do not claim natural resource exhaustion.
Combined A3 through C05 is independently approved. A4 records the latest
full gate candidate; public profile CLI, B/C/X06, mandatory external-profile
gate, cutover, execution/RA01 and Linux remain open. Public init creates format1.


The [X05-A3-C03 allocation-safe location](../../tests/native/v5-02-x05-a3-c03-evidence/REPORT.md)
selects seal locations with a trivial kind/index token. Root audit/freezing no
longer formats fields. Every protected seal failure adapter renders the token
outside Lua; a rendering allocation refusal returns the existing external resource
diagnostic with the preinstalled active module field. Two independent module-field
strings per module are staged before processing, charged cumulatively, and swapped
without allocation after descriptor collection. Staging failure identifies profile;
first-failure terminal retries retain their token. The shared audit, freeze order,
depth, publication and embedded paths remain unchanged.
Ordinary index10/64 and mixed-module controls, actual native allocation refusal,
controlled Lua allocation/stack/status and throwing-allocator adapters verify these
boundaries with cleanup. The throwing allocator is a controlled adapter probe,
not a claim that realloc throws C++ exceptions or natural host exhaustion occurred.
Combined A3 through C05 is independently approved. A4 records the latest
full gate candidate; public profile CLI, B/C/X06, mandatory external
profile gate, cutover, execution/RA01 and Linux remain open. Public init creates
format1.


The [X05-A3-C04 registration attribution](../../tests/native/v5-02-x05-a3-c04-evidence/REPORT.md)
extends the nonallocating original-location token across callback admission,
reference retention, final descriptor collection and pure catalog error remapping.
Successful callbacks clear their token; caught failures retain the first token,
and catalog prevalidation clears completed entry ownership. Error adaptation
renders only outside protected Lua calls. Rendering refusal retains the owned
module fallback; unmatched canonical entries retain the truthful caller boundary.
Pure global error fields move their owned storage. A separately charged/staged
catalog fallback installs by swap before final provenance work as preventative
consistency; no allocation failure at the previous short assignment is claimed.
C03 module staging/sealing, shared audit and cumulative budgets remain intact.
Controlled actual allocation probes distinguish removed zero-request operations,
remaining projection rendering refusal, sticky callback failures and module/global
phase work. Raw-reference status and unmatched pure errors use explicit controlled
adapters. No new quota or public interface is added. Combined A3 through C05 is
independently approved. A4 records the latest full approved gate; public profile
CLI, B/C/X06, mandatory external-profile gate, cutover, execution/RA01 and Linux remain open. Public init creates
format1.


The [X05-A3-C05 caught-memory](../../tests/native/v5-02-x05-a3-c05-evidence/REPORT.md) makes a completed
restricted `lua_pcall` memory failure sticky in native state. The bit is set
only after `LUA_ERRMEM`, preserving a successful emergency allocation retry;
logical quota refusal remains separate. Outer restricted calls rethrow and host
adapters retain the existing resource diagnostic and original entry token.
Actual descriptor-reference allocation probes cover normal/later-error tails,
three nested catches, direct host failure and successful retry after one null
return. The function-reference-only control has zero actual growth requests;
no function-reference allocation failure or natural host exhaustion is claimed.
Ordinary caught Lua errors remain recoverable. Shared accounting, protected
ownership and existing diagnostic priority are retained. The [C05 review](rounds/V5-02-X05-A3-C05-review.md)
independently approves combined A3 through C05 and closes R1–R7. A4 records the
latest full approved gate; public profile CLI, B/C/X06, mandatory external
profile gate, cutover, execution/RA01 and Linux remain open. Public init creates
format1.


The [X05-A4 integrated registration gate](x05-registration-evidence.md)
is independently approved by the X05-A4 review. Full Debug and
release-preset RelWithDebInfo each pass 582/582, with zero failures/errors/skips/
disabled cases, preserving all current 582 and prior 520 names. Pinned formatting/
Doxygen checks 117 active C++ sources; common source vectors, per-configuration
runtime/CLI and reversible variant audits, and no-work builds pass. Product/test
bytes remain identical across both gates; only current status documentation is
finalized afterward. Earlier private retained-callable/fault probes remain
approved Debug evidence and are not claimed as release reruns. Profile-owner
invocation refuses before provider effects. B/C semantics/adapters, X06 recorded
history/interpretation/acceptance and receipt/whole-invocation orchestration,
X07 public cutover, X08 mandatory external-profile acceptance, execution/RA01
and Linux remain open. Public init creates format1.


The [X05-B1 pure shape foundation](../../tests/native/v5-02-x05-b1-evidence/REPORT.md) is independently approved by the X05-B1 review. An optional trusted internal context selects profile-only shape IDs and
stages policy while retaining declaration version1 and the embedded default.
Every job's mandatory policy and base phase cycles finish before custom shape
callbacks; bounded additive unit obligations then join the original full graph.
A distinct test-only exact-runtime adapter exercises the actual product Lua
helpers. Its internal `shape_results` sidecar is not final selected-shape identity.
B1 does not prove retained native F dispatch, immutable F input, exact source
identity, the complete return's 16 MiB boundary, or whole-invocation accounting.
B2–B4 retain those native/identity/W/render duties. Public profile admission,
X06 history/acceptance, X08 mandatory acceptance, execution/RA01 and Linux remain
open; public init still creates format1. Focused verification is not a full B gate.


The [X05-B2-A native shape-definition foundation](../../tests/native/v5-02-x05-b2-a-evidence/REPORT.md)
received a request-changes review for early error adaptation; the combined
B2-A+C01 correction is independently approved by the X05-B2-A-C01 review. The internal selected-profile VM
resolves registered external shape IDs to owned ID, final revision and exact
`shape_definition_v1` digest. Identity includes the final descriptor, native
external logical module path and lowercase hex of that selected module's original
captured bytes; module ordinal and unrelated modules do not enter this identity.
Each lookup recomputes with cumulative checked native charges, without executing
Lua or starting/resetting invocation budgets. Resource failures remain terminal.
Generic profile invocation stays closed. Independently approved B2-B codec accounting precedes B2-C retained
F dispatch/immutable input/complete-return transport, B3 selected identity/W and
B4 rendering remain open, as do X06–X08, execution/RA01 and Linux. This focused
foundation does not close B2 or the B milestone; public init creates format1.

The [X05-B2-A-C01 error-boundary correction](../../tests/native/v5-02-x05-b2-a-c01-evidence/REPORT.md)
includes embedded, moved-from and already-exhausted lookup refusal construction
inside the typed boundary. An actual one-shot native allocation refusal returns
`resource_limit` at `catalog.shapes`; existing sticky conversion diagnostics keep
their first message, limit and maximum. Profile owners remain terminal. Ordinary
embedded/moved-from refusals retain protocol-first behavior on retry; a moved-to
owner remains independent. This covers controlled one-shot failures, without a
persistent exhausted-heap guarantee. Identity, cumulative reservation policy and
public admission remain unchanged. The combined candidate is independently approved by the X05-B2-A-C01 review;
B2-C and the mandatory external-profile gate remain open.


The [X05-B2-B codec reservation candidate](../../tests/native/v5-02-x05-b2-b-evidence/REPORT.md)
adds optional native `codec_encode_reserved` without changing ordinary callers.
One shared validator derives checked logical graph, string, workspace and fixed
Glaze output capacity before the synchronous native charge hook. Successful
validation uses a bounded active path without heap allocation; each copied
container reserves its exact entry count. Input stability through the hook is a
trusted C++ precondition. The moved output buffer is counted once; conservative
workspace slots are not independent heap allocations or an RSS guarantee.
Focused Debug evidence and independent allocation phase probes accompany the
candidate; combined B2-B+C01 is independently approved by the X05-B2-B-C01 review. B2-C retains raw-return conversion
charges, retained F execution, immutable input and invocation integration. B2/B,
X06–X08, execution/RA01 and Linux remain open; public init creates format1.


The [X05-B2-B-C01 encode diagnostic correction](../../tests/native/v5-02-x05-b2-b-c01-evidence/REPORT.md)
is independently approved by the combined X05-B2-B-C01 review. A shared outer
allocation boundary protects the existing encode handlers' diagnostic construction,
including validation refusals and callback exception adaptation. Controlled one-shot
native diagnostic allocation refusal returns the existing `resource_limit` /
`output_bytes` diagnostic; normal precedence and moved typed callback refusals
remain unchanged. This is not a persistent exhausted-heap guarantee. Encoding,
reservation math, decode and limits are unchanged. B2-C retained F dispatch and
invocation integration, B3/B4, X06–X08, execution/RA01 and Linux remain open;
public init creates format1. Focused correction evidence does not close B2/B.


The [X05-B2-C retained shape bridge](../../tests/native/v5-02-x05-b2-c-evidence/REPORT.md) is independently approved with C01/C02; their reviews close B2C-R1 and B2C-R2.
Internal `command_vm::invoke_shape` binds owned final definition metadata before
calling the actual retained F in its original selected-profile VM. Exact normalized
five-field inputs preserve nested aliases and freeze transient proxies without
publishing permanent sealed identities. Exact pre-invocation tag/backing membership
survives cleanup after raw response conversion, including returned input aliases;
invocation scratch/result tags are released. One actual JSON-compatible return
passes separately charged raw conversion and reserved Glaze encoding with the
complete 16 MiB ceiling. Padding transport fixtures do not prove shape semantics.
Registration transitions once to cumulative invocation instructions; native/node
charges never reset. Admission protocol errors are nonconsuming; execution/resource
failures retain the first terminal original shape location. B1 Lua still owns shape
success/refusal/obligation semantics. B3 selected identity/W and B4 rendering,
X06–X08, public profile admission, execution/RA01 and Linux remain open. Generic
profile command invocation stays closed; public init creates format1. This focused
Debug slice is not a full B or release gate.

The [X05-B2-C-C01 correction candidate](../../tests/native/v5-02-x05-b2-c-c01-evidence/REPORT.md) stores transient
snapshots under private immutable address keys, separate from every integer
`luaL_ref`. Retained shipped F and external D/F bindings survive first/repeated
shape calls, GC and failure cleanup in the owned reversible observer. The original
reviewer regression is retained and reproduced. This correction preserves existing
cleanup, cumulative budgets and public boundaries; the combined C02 review independently approves the bridge. No full milestone or release gate is claimed.

The [X05-B2-C-C02 terminal diagnostic correction candidate](../../tests/native/v5-02-x05-b2-c-c02-evidence/REPORT.md) shares the profile owner's first terminal error across definition lookup and shape invocation.
A resource failure owns its original code, message, limit, maximum and field before
fallible response copying; later ID/input guards and allocation adaptation preserve
that error. Natural cumulative lookup exhaustion reproduces the prior masking and
location change, then verifies missing/wrong-kind/valid IDs, invalid inputs and
retries. Controlled one-shot native allocation probes cover ownership and copies;
no persistent host-memory guarantee is claimed. C01 private registry keys remain.
The combined independent B2-C+C01+C02 review approves this scope. The focused Debug gate adds
one regression (620 registered names, 354 focused plus 11 supplemental); no full
milestone or release gate, public profile admission, B3/B4, X06–X08, execution/RA01
or Linux acceptance is claimed. Public init creates format1.


The [X05-B3-A pure phase candidate](../../tests/native/v5-02-x05-b3-a-evidence/REPORT.md) factors existing trusted validation into structural prepare/check-one/finish exports.
Every mandatory/base-cycle check precedes custom requests; checked responses retain
B1 exact obligations, guidance and count policy. Finish retains original graph edges
and forms selected_shape only from explicitly supplied trusted request-corresponding
definition metadata. The old embedded validator and B1 shape_results adapter remain.
The [phase ABI](../../tests/native/v5-02-x05-b3-a-support/phase-abi.md) records handoff requirements.
The X05-B3-A review independently approves this pure slice. It does not prove native metadata
authenticity, immutable F input, same-VM dispatch or cumulative phase conversion;
B3-B owns those. B3-C W, B4 rendering, X06–X08, public profile admission/cutover,
mandatory external-profile acceptance, execution/RA01 and Linux remain open.
Public init creates format1; latest full approved gate remains A4 582/configuration.


The original X05-B3-B incomplete prerequisite checkpoint is preserved in
`tests/native/v5-02-x05-b3-b-evidence/REPORT.md`, including its actual request423
regression and scoped sticky correction. The R01 integration candidate below
supersedes that checkpoint's remaining accounting/lifetime status.


The [X05-B3-B-D01-A test-only decode feasibility](../../tests/native/v5-02-x05-b3-b-d01-a-evidence/REPORT.md) demonstrates
request-local allocation interception with pinned Glaze readers, including variant
rewind, duplicate-check storage, malformed partial cleanup and typed refusal
adaptation. Independent D01-A review approves the test-only boundary. No product, vendor, codec API or runtime
bytes change. Production conversion reservation is now the separate D01-B candidate; B3-B accounting/request423 and its final gate remain open. The
637-name product inventory includes WIP; last approved inventory remains 627.
No full milestone, release or Linux gate is claimed; public init creates format1.


The [X05-B3-B-D01-B production reserved decode candidate](../../tests/native/v5-02-x05-b3-b-d01-b-evidence/REPORT.md) adds optional
`codec_decode_reserved`: every actual parser/duplicate-check request charges before
allocation, followed by one checked allocation-free immutable-conversion sizing
pass and reservation before persistent ownership. Request-local Glaze wrappers
retain original grammar/error ordering; returned values use ordinary allocators.
Exact pinned control-block/tree-node/list/string recipes and the actual bounded
workspace determine capacity. Typed supplied errors move unchanged, and outer
allocation protection covers early/catch diagnostic construction in both decode
entry points. Ordinary decode API/defaults and approved encoding remain unchanged.

Independent D01-B review approves the reserved decoder prerequisite. The
X05-B3-B-R01 native integration candidate consumes both decoder hooks only while
profile semantic composition is active. Actual parser/checker allocation requests
and immutable conversion capacity charge the existing cumulative native budget
before allocation, without refunds. The inherited node and graph/transport charges
remain deliberately conservative overlapping accounting. Embedded decode keeps
its ordinary path. Semantic error-data staging also charges concrete pinned
control blocks, map nodes and copied string buffers before allocation; successful
encoded text separately charges its native wrapper while moving the already
reserved output buffer. Direct in-place construction avoids initializer-list
copies. Two narrowly scoped global-fragment libc++ headers expose only the pinned
allocation type identities; product operations retain `import std`. Reserved
encoding and first typed terminal diagnostics remain.

The combined original B3-B plus R01 candidate retains audited private phase roots
and native-owned request/result/definition values across sequential protected
same-VM prepare/F/check/finish calls. Production coverage includes recoverable JSON
refusal followed by healthy calls. Private reversible observers cover exact tag/
backing key-value membership and all16 retained references in a mixed fixture
(five shipped, three phases, four external descriptor/callable pairs), stable live
Lua heap after warmup/GC, partial baseline setup and terminal cleanup. Controlled
parser/conversion/node/Lua refusals prove no F dispatch; repeated real operations
exercise cumulative instruction/native limits. Near-budget staging controls
verify exact native error-graph requests and deny node/map/string/wrapper storage
before allocation, retaining ordinary healthy schema diagnostics. These probes supplement actual
production F and semantic execution.

The frozen candidate gate targets642 registered names (all641 prior names
preserved),376 focused Debug cases and11 supplemental history cases, production
semantics, actual allocation sampling, request423 checkpoint/current controls,
vectors, six exact source audits, pinned formatting/Doxygen and no-work builds.
See `tests/native/v5-02-x05-b3-b-r01-evidence/REPORT.md` for executed results and
limitations. The independent combined B3-B/R01 review approves this integration. No full B,
release or Linux gate is claimed; latest full approval remains A4's582/config.
B3-C W, B4 render, X06–X08, public profile admission/cutover, mandatory external
profile acceptance, execution/RA01 and Linux remain open. Public init creates
format1. Codec and original checkpoint artifacts remain unchanged.


The independently approved X05-B3-C selection adds internal native `select_profile` over original
bundle/capture/files/job/role intent. Its semantic phases run once, then a fourth
privately retained trusted phase selects from that exact finalized same-call model.
All four phase roots join the original pre-module audit/freeze; phase transport,
codec work and repeated operations share cumulative accounting and cleanup. No
caller model, shape definition or export selector is admitted. Generic profile
invocation remains closed.

Embedded `select_role` retains its original normalization and whole-bundle
validation. Shipped selection remains exact with profiles. Custom W preserves old
fields, appends `shape_selection_v1` and shape definition/revision plus synthetic-ID
sorted unit obligations where selected work is either endpoint, and hashes
`work_external_shape_v1`. Guidance is excluded. Selected source comments/revisions
change every custom-job role; separate unselected/projection modules affect only
provenance. Same-module visual source changes conservatively affect definition/W.
The independently authored minimal-fixture W and complete canonical preimage live
under `tests/native/v5-02-x05-b3-c-support/`; the candidate verification report is
`tests/native/v5-02-x05-b3-c-evidence/REPORT.md`; the B3-C review independently approves this selection scope.
This is selection infrastructure: B4 presentation, X06 history/acceptance, X07
public cutover, X08 mandatory external-profile acceptance, execution/RA01 and Linux
remain open. Public init creates format1; no full B/release gate is inferred.

Selected-profile hash work additionally reserves conservative pinned-library
cumulative string growth, owned source/domain copies, encoder scratch and digest
storage before allocation. These byte charges do not add converted-node counts.
Actual allocation probes retain typed first resource refusal; embedded digest
behavior remains unchanged. The derivation and private aggregate measurements are
recorded in `tests/native/v5-02-x05-b3-c-support/hash-accounting.md` and the C report.

The X05-B4-A candidate factors the shared trusted render body and adds the private
`render_profile_material({request,model,selected})` helper. Its caller must supply
authentic finalized semantic and selection results; logical job/role/shape identity
checks do not authenticate arbitrary tables. The helper normalizes render intent,
reads retained observations, and renders without validation, selection, F dispatch
or mutation of model/selection. Custom shapes add exact three-field identity,
full scoped unverified prerequisites and mandatory `shape_policy` after dependencies;
shipped shapes preserve the old envelope and presentation. B4-A verifies pure helper
composition only. Native same-call retention, immutable transport, cumulative full
render accounting and lifecycle proof are delivered by the B4-B candidate below; the full Debug/release B gate
remains B4-C. Public profile CLI, X06–X08, execution/RA01 and Linux remain open;
public init creates format1. Candidate evidence is
`tests/native/v5-02-x05-b4-a-evidence/REPORT.md`; independent B4-A review requested
the correspondence correction recorded below.


The X05-B4-A-C01 corrective candidate requires complete positional one-to-one
bundle/semantic job correspondence before reading any selected shape. Both arrays
must have equal lengths, each row must carry a valid matching job ID, and IDs
must be unique across the complete mapping. Reordered, missing, extra, duplicate,
ambiguous and malformed identities refuse with the existing schema diagnostic at
`render.selected`; there is no ID lookup fallback, revalidation or input mutation.
Two new production cases reproduce same-shape/different-guidance substitution and
exercise selected/unselected rows and malformed containers. The actual pre-edit
baseline and red regression are retained in
`tests/native/v5-02-x05-b4-a-c01-evidence/REPORT.md`. B4-A's independent review
requested this correction (B4A-R1); the combined B4-A+C01 review independently approves the correction and closes B4A-R1.
Pure helper correspondence does not establish native same-call authenticity;
Combined B4-B+C01 native composition is independently approved; B4-C integrated evidence awaits independent review. Public init
still creates format1; X06–X08, public cutover, execution/RA01 and Linux remain open.


The X05-B4-B candidate adds internal native `render_profile` over original
bundle/capture/files/job/role intent and optional budget. A trusted normalization-only
preflight preserves full render schema then budget-stripped selection schema
precedence before any semantics or F call. Six fixed trusted roots seal before
external modules. Native-owned normalized intents, same-call finalized model and
selection flow through sequential protected calls to the approved renderer; each
custom job's F runs once. Existing selection behavior and public generic profile
refusal remain. All phases, observation/presentation codecs, hashes and a final
reserved complete-response encoding share cumulative limits; final output is
bounded by the existing 16 MiB codec ceiling and lower role/render budget.

B3-C and combined B4-A+C01 are independently approved. combined B4-B+C01 is independently approved; B4-C records the complete Debug/RelWithDebInfo gate candidate. Focused Debug
coverage, exact root/reference and frozen-membership GC observations, preflight
zero-F and valid once-per-job dispatch, actual native allocation refusals and
controlled transport failures are recorded in
`tests/native/v5-02-x05-b4-b-evidence/REPORT.md`; allocation derivation is in
`tests/native/v5-02-x05-b4-b-support/transport-accounting.md`. This candidate adds
no provider or public profile CLI, projection dispatch or storage interpretation.
X06–X08, mandatory external-profile acceptance, public cutover, execution/RA01 and
Linux remain open; public init creates format1. Latest full approved gate remains
A4's 582 per configuration.


The X05-B4-B-C01 correction candidate routes every ordinary native render
response, including normalization, preparation, shape-check, finalization and
selection errors, through the same existing reserved complete-response encoder.
The unchanged 16 MiB output ceiling and cumulative native charges apply before
returning the original native response pointer. Validation-only/selection-only
transport and native unexpected failures retain their prior paths. The genuine
oversized original unknown-key schema regression now returns terminal typed
`output_bytes`/16777216; small ordinary errors retain their bytes and recovery.
Production phase/lifetime controls and reversible exact writer/phase/F observers
supplement focused Debug verification. The combined B4-B+C01 review independently approves this correction and closes
B4B-R1. B4-C records the separate integrated macOS gate; Linux remains open.
Public profile admission, X06–X08, mandatory external-profile acceptance,
execution/RA01 and Linux remain open. Public init creates format1; latest full
approved gate remains A4's582 cases per configuration.


## X05-C1 pure assembly candidate

C1 extracts a shared pure helper within existing `render.lua`, avoiding a new
embedded module and changes to native source-count accounting. It consumes
authentic prepared base sections, separately admitted custom records and a
validated budget; it performs no native response admission or callable dispatch.
Custom optional records drop in reverse projection order before the shipped
checkpoint/job_notes/workspace_notes/rationale priorities. Base bytes and mandatory
records survive, omitted IDs retain actual removal order, and metadata retains
original section order followed by the mandatory notice. A private exact
`{id,bytes,included}` projection adapter preserves ordinary manifest structure.
The [focused Debug report](../../tests/native/v5-02-x05-c1-evidence/REPORT.md) awaits
independent review. C2 retains native schema/limit/reserved-ID admission and F
identity/dispatch; services remain behind initial receipt orchestration. No public
profile admission or whole-C acceptance is inferred.


The X05-C2-A candidate adds internal native projection-definition identity lookup
without executing the retained projection callable. It hashes the exact final
five-string descriptor, logical external owner without ordinal, and original
privately captured selected module bytes under `projection_definition_v1`.
Owned ID/revision/digest results survive VM destruction. Unknown IDs refuse
nonterminally; resource failures retain the shared first owner error across shape,
projection, validation, selection, render and moves. Prospective graph, string
copies, cumulative canonical growth and encoder scratch charge existing native
limits before allocation; no invocation starts or budget resets.
Focused Debug evidence is in `tests/native/v5-02-x05-c2-a-evidence/REPORT.md`;
fresh independent review is pending. Retained projection execution/native result
admission remains C2-B. Read services, capability/receipt orchestration, public
profile admission, X06–X08, execution/RA01 and Linux remain open. No full C or
release gate is claimed; public init creates format1.


The X05-C2-B1 candidate adds only a native exact projection JSON size prerequisite
in `engine/json_codec`: stable borrowed section views, complete input preflight,
one synchronous charge for fixed 6,325,248-byte scratch and actual fixed writer
workspace, and pinned Glaze serialization without an AST copy or retained output.
Encoded overflow is sticky and nonthrowing; bounded writing completes before
refusal. Native invalid input, encoded ceiling, resource and scratch/internal
failures remain distinct typed outcomes. Callback diagnostics move unchanged;
independent nested/concurrent owners share no routing state. The operator's global
18-name built-in reservation is now explicit in X-PROJ for both input kinds;
semantic reserved-name admission remains the later native scanner's duty.
Evidence is `tests/native/v5-02-x05-c2-b1-evidence/REPORT.md`; fresh independent
review is pending. This prerequisite does not execute F, scan/root Lua results,
open profile invocation or implement read services, public CLI, X06–X08,
whole-invocation orchestration, execution/RA01 or Linux. Public init creates
format1; no full or release gate is claimed.


C2-B2 adds the internal typed retained-projection bridge and native raw-result
admission candidate. Descriptor kind and immutable structural transport are checked
before one retained F call; callers own authenticated selected-role/history data.
The protected scanner enforces exact version1 section schema, the shared GLOBAL18
reserved union and raw limits before borrowed-view exact encoded counting and
native result copying. Original projection indices and shared first terminal errors
survive retries and moves. This candidate awaits fresh independent review; native
read services, rendering composition, CLI/provider/receipt integration and the
mandatory external-profile gate remain open. See
[the C2-B2 evidence](../../tests/native/v5-02-x05-c2-b2-evidence/REPORT.md).
