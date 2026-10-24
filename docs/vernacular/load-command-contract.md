# Embedded load command amendment v1

R14-D5, D6 with C01, and D7-A are independently approved. R14-D7-B integrates the fifth public load command as a candidate pending independent review. Full R14 acceptance remains D8 work; the external-profile gate follows it.

Status: R14-D3 independently approved in [D3 review](rounds/R14-D3-review.md).
R14-D4 typed native diagnostics are independently approved; D5 bounded native capture/preparation is independently approved.
This specifies proposed implementation; it exposes no command or service today. V5-02-A,
B1/C01, B2 and [C](rounds/V5-02-C-review.md) are independently approved. Current
commands remain `render`, `init`, `history`, `export`; render is database-free.
Full R14 acceptance, Linux verification and external profiles remain pending.
This scoped amendment extends the [command ABI](command-contract.md) and clarifies
[ledger acceptance](ledger-contract.md#transactions-typed-events-and-replay). All original
v1 declaration, record, request, interpretation, version, event/result and storage
schemas remain unchanged. It grants no execution, source checkout, generic mutation
or external-profile authority. [R14-D3](rounds/R14-D3.md) owns the review gate.

## Fixed catalog admission and typed intent

At the integrated load gate, add exactly one mandatory descriptor to the existing
four mandatory descriptors. `command_catalog_v1` keeps its exact shape/version.

| name / family | handler_id | response_kind | exact capabilities |
| --- | --- | --- | --- |
| load / load | `shipped.load.v1` | `load_v1` | `accept_load_v1` |

The callable belongs to embedded `planar/commands.lua`, registered once, retained
and sealed in the same VM/bundle as the other callables. `response_kind` then has
five values; `load_v1` maps only to envelope kind `load`. The complete catalog
must contain all five, once each. Existing duplicate, unknown pairing, wrong
module, callable, signature, family and capability refusals apply atomically;
capacity limits do not authorize a sixth command. A missing load entry is then
`internal` exit 4 for every ordinarily valid command. Root help and caller usage
still precede registration. Until that integrated gate, the four-entry validator
and parser stay coherent and load remains unknown usage. Intermediate runtime
support may use test-only five-entry bundles, never expose a partially working
public load or invalidate the shipped four-entry catalog.

The exact public invocation is:

```text
planar load --ledger <path> --workspace <file-or-name> --registry <registry.json> --expect-revision <id-or-none> --request-id <id> --actor <text>
```

All six options below are required, take one separate scalar token, and occur
exactly once. No flags, defaults, decimal descriptors, aliases, positionals or
optional values exist for load.

| option name | catalog type | native validation / invocation value |
| --- | --- | --- |
| `--ledger` | `text` | Explicit ledger path; opaque `ledger` slot. |
| `--workspace` | `text` | C file/name selector; opaque `workspace` slot. |
| `--registry` | `text` | Explicit registry file path; opaque `registry` slot. |
| `--expect-revision` | `text` | Exactly `none` or canonical positive decimal through 9223372036854775807; Lua receives that exact canonical string. |
| `--request-id` | `text` | 1–128 ASCII bytes `[A-Za-z0-9_.:-]`; Lua receives text. |
| `--actor` | `text` | 1–256 valid UTF-8 bytes without NUL; Lua receives text. Driver assertion, no authentication. |

As for existing families, scalar tokens are nonempty and cannot start `--`.
CLI11 owns parsing and parsed presence/count checks; no second argv parser or Lua
numeric conversion. Native validates the expected token, retaining both its
canonical string and optional signed-64 value. `none` is required explicitly for
an empty ledger, never inferred from omission. `--expected-revision` is unknown.
Load accepts no root/base mapping, job, role, observation, output capture, budget,
profile or source-selection option.

The invocation is exactly `{version:1,command:"load",options,present}`. `options`
has exactly `ledger`, `workspace`, `registry` opaque slots plus `expect-revision`,
`request-id`, `actor` strings. `present` has exactly those six keys, all `true`.
Slots have the existing native invocation/type identity and no public fields,
string/JSON conversion or cross-invocation use. Lua never receives argv, private
host paths, cwd, environment, request encoding, SQL or a ledger handle. Presence
and retained native options remain authoritative if Lua mutates its scratch maps.

## Native prelude and early replay

After ordinary parse success, pure embedded registration/catalog validation and
same-bundle binding, prepare the load invocation as follows, before calling Lua:

1. Apply C workspace classification, relevant environment validation and one cwd
   snapshot. Normalize expanded workspace, explicit registry and ledger paths as
   specified by C; workspace/registry intent and capture use their absolute
   lexical-normal targets. No physical resolution, declaration evaluation or file
   existence check is allowed while framing intent. Explicit workspace spelling
   ignores `PLANAR_WORKSPACE_DIR`; a valid name reads it once. Name/explicit
   aliases share intent only for identical expanded lexical targets. Render
   retains physical parent resolution and its current `symlink/..` behavior.
2. Create immutable native `request_intent` from request ID, actor, expected head,
   normalized workspace and registry arguments. Its unchanged request_v1 body is
   operation `load`, actor, expected token and the two private path strings.
   Request ID indexes it but is excluded from body equality. Ledger path, current
   head, captured content, catalog and runtime version are excluded.
3. `open_existing(ledger, read_only)` verifies the existing explicit ledger, then
   `read_request(request_id)` returns a verified optional private request row.
   Native compares `body_encoded` with the exact intent encoding, never only a
   hash. Equal returns the original saved `result` bytes; unequal returns the
   redacted `request_conflict` below. A miss alone permits the handler. Release
   the read owner before candidate capture/write so it cannot pin an old read
   transaction across acceptance.

This is a native replay shortcut, not a Lua service. Early replay never invokes
`shipped.load.v1`, current declaration/schema/shape/projection policy, workspace or
registry capture, provenance assembly, receipt verification or current-head CAS.
It works with changed/removed inputs, changed current policy and an advanced head;
it neither activates the saved revision nor appends anything. Registration and
intent validation still precede lookup; corruption/invalid catalog/invalid intent
is not silently ignored. A new build may admit the same v1 request independent of
its changed embedded digest. Public history request lookup remains available when
original invocation arguments are unavailable.

## Finite fresh-load service protocol

Only an early miss invokes the actual retained handler. It receives exactly three
context methods, each callable once, in order; other family services are absent.
All existing exact-key, bounded deep-copy, forged-handle, family, terminal-error
and cumulative-budget rules apply. Services return exactly `{ok:true,value:V}` or
`{ok:false,error:E}`. Native retains the original error; a failure poisons further
service use and wins over caught/replaced Lua errors. No generic mutation method,
per-file capability, bytecode, executable source, host path or callback is accepted.

| call and exact argument map | exact successful V | authority and required handler action |
| --- | --- | --- |
| `open_workspace({workspace:SLOT})` | `{file,workspace}` | Slot must equal invocation workspace. Native captures/evaluates that declaration at the normalized load target under existing confinement, retains its session/receipts and returns its portable filename and declaration value. Lua calls existing `route_workspace(V)` and propagates its typed error. |
| `capture_load({route:R})` | `{bundle:B}` | R is the exact normalized workspace route described in the command contract: `{workspace,file,jobs,root,registry,spec}`. Native verifies route structure, portable references and agreement with the retained opened declaration, then captures/evaluates the spec, every listed job, all referenced text and explicit registry. It retains one sealed acceptance capture plus native closure membership. B is the existing complete declaration bundle with captured text/registry values, ready for `interpret_bundle`. |
| `accept_load({bundle:B,semantic:M})` | Existing complete load success envelope | B and M are the successful normalized bundle/semantic pair from the same VM's existing `interpret_bundle(captured.bundle)`. Native checks bounded fixed structure and correspondence to that capture, assembles exact provenance/versions/interpretation, creates immutable prepared acceptance, calls native atomic completion, retains its saved bytes and returns the decoded saved envelope after the lock is released. |

Here SLOT is the opaque workspace slot, R keeps its existing command-contract
shape, and B/M are existing values, not new storage schemas. Capture uses existing
restricted declaration VMs with curated constructors and sealed inputs. Trusted
routing/normalization/semantic interpretation executes in the retained same-bundle
command VM; all jobs are validated, regardless of grouping or any later render
role. `interpret_bundle` calls `validate_bundle` with exactly
`capture={status:"not_captured"}` and preserves sorted normalized bundle/jobs and
complete dependencies/jobs semantic data. No source topology is fabricated; root
and base labels are declaration data only. A bad unselected job refuses the load.
The handler propagates validation diagnostics with logical field paths unchanged.
It returns `{ok:true,value:V}` from successful accept, or the exact service/Lua
failure. A skipped service, changed response or malformed return is `internal`.

Native capture discovers only the declaration/reference closure: workspace file,
spec, all listed jobs, referenced text, plus separately captured registry. Duplicate
references use retained bytes. No recursive workspace tree, source/base checkout,
observation, `.git` traversal or unrelated sibling is added. Native retains the
unique sorted portable workspace-file membership, root/file identities, selected
ancestors, modes and registry receipts; unrelated additions/edits/removals do not
invalidate this closure. Existing root/captured alias and no-follow leaf policies
still apply. Changing a selected ancestor identity, selected byte/mode/identity or
registry refuses `capture_changed`. The first seal uses
`finish_for_acceptance`; no new values replace the original capture on later
verification. Render's whole-tree membership behavior is unchanged.

Native preparation derives source membership from that retained closure, never a
Lua-chosen subset. It derives registry provenance from exact captured `input.json`
bytes/mode and stdlib provenance from the exact ordered embedded originals,
separator, assembly and manifest belonging to the retained command VM. Bundle
identity cannot be recaptured from a path or substituted after registration.
`prepare_stdlib`/`verify_stdlib`, `encode_interpretation(B,M,stdlib_digest)` and
`prepared_acceptance::create` enforce existing v1 codecs, correspondence and
bounds; Lua cannot supply encoded versions, request, provenance, source members,
digests, IDs, parent, event, outcome or stored response. Exact original authored
stdlib bytes include the command module. Adding load changes stdlib provenance;
it does not change W recipes, interpretation version meanings or current render
JSON/Markdown/W bytes for identical render inputs. Historical decoding never
compares recorded stdlib to today's code or executes recorded/current Lua policy.

## Atomic completion and saved-byte transport

All service arguments and native preparation finish before entering the write
transaction. Use existing `complete_acceptance(ledger,intent,prepared)` through the
verified writable opener/`BEGIN IMMEDIATE`, retaining the prelude ledger identity
and rechecking identity/schema. Its first request lookup under the lock is
mandatory: an intervening equal request returns its exact saved bytes before CAS
or candidate access; unequal returns request_conflict. Do not replace this with
an early head read. For an unknown request, compare expected head to the locked
actual head before candidate interning; mismatch refuses even for equal content.

Exact tree/provenance/interpretation/version equality with active head yields
`noop`: one event/request, no revision. Otherwise allocate a new chronological
revision, including restore of old exact content; comment/prose-only source edits
create revisions while unchanged material W may stay equal. In one transaction,
append event and request with exact encoded success bytes and update head/sequence.
All collision, bounds, constraint, busy, storage, CAS or receipt failures roll
back tentative changes; failed request IDs remain reusable. Immediately before
commit, native `prepared.verify()` MUST reread retained filesystem receipts under
the lock. These native filesystem reads are required. No Lua, callback, projection,
comparator or error encoding runs under that lock. The cooperative check does not
prevent writes after verification or prove hostile change-and-revert detection.

The exact envelope is unchanged: `{version:1,kind:"load",ledger,request_id,outcome,
revision,parent,event}`; outcome is `accepted` or `noop`, all database IDs are
canonical decimal strings, absent parent is `none`, and ledger is its 32-digit
lowercase identity. No `replayed`, current-head or private request field is added.
Native owns `acceptance_result.result` before commit and retains completion
state before any post-operation allocation or Lua conversion. The native-only
load provider owns a completion receipt with exactly phase `not_completed`,
`committed`, `replayed` or `commit_uncertain` and retained saved result bytes when
available. The load overload is
`complete_acceptance(path,intent,prepared,expected_ledger_identity,completion_receipt&)`: the
identity is the native 32-hex identity retained from early verified open, checked
against the writable transaction before request relookup. A mismatch is the
existing unsafe-path refusal, never acceptance into a replacement ledger. The
receipt is a native owned output (not a callback or Lua argument); retain the
existing three-argument acceptance API for other callers. Native saved-response storage is allocated before COMMIT. Set `commit_uncertain`
just before issuing COMMIT; set `committed` only on confirmed commit and
`not_completed` only on confirmed noncommit/rollback. An equal locked request
sets `replayed` and retains its saved bytes before returning, without implying
a fresh mutation. Early replay sets that same phase. Both adapter and VM retain
this receipt through result conversion/handler execution; updating the phase or
transferring owned bytes must not allocate. A later exception cannot erase it. Decode the saved
result for the service; after handler success require exact canonical equality of
its envelope to that native result. Lua cannot choose a different saved success.
For fresh commit, late transaction replay and early replay alike, stdout is the
native saved JSON bytes plus exactly one LF, never a reencoded Lua success.
No terminal LF is stored. Success is exit 0, empty stderr, one buffered response.

Preallocate both fallback diagnostics before the acceptance operation can commit.
After known commit or late replay, Lua error, resource/allocation failure, invalid
return/projection or encoding failure uses exit 4 and this exact redacted error:
`{version:1,kind:"error",error:{code:"io",message:"Load may be complete; retry the same request ID or inspect request history",field:"",details:{operation:"load"}}}`.
Use the same fallback if commit outcome is uncertain; never call this an ordinary
precommit semantic refusal or imply rollback. Preserve native commit/replay state
in the adapter and VM, including failure allocating the first service result.
After early replay, failed transport preparation uses that same saved-result
fallback. A stdout write failure uses exit 4 and exactly
`{version:1,kind:"error",error:{code:"io",message:"Load may be complete; retry the same request ID or inspect request history",field:"",details:{operation:"stdout"}}}`.
A short write may leave partial stdout: discard it, retry the original intent with
the same request ID, or use history request lookup. Never suggest a new ID for a
lost response. A new ID is for changed intent/new acceptance. If even a preallocated
diagnostic cannot be written, retain exit 4; unrecoverable allocation before
fallback preparation retains the existing empty-stderr exit-4 limit. No success
or partial semantic error is emitted during a service. Ordinary native failures
known to roll back retain their original typed diagnostics and precedence.

## Typed authoritative diagnostics

Extend native `format_error` with an optional owned detail variant; retain its
existing code/message. This is an in-memory API amendment, not a SQL/codec version.
Exactly these variants are admitted: revision detail containing expected/actual
optional signed-64 heads, and collision detail containing record_kind plus optional
record_digest. No Lua value, host string, request body or free-form map is stored.
Absence is valid for existing nonapplicable failures. A detail/code mismatch,
invalid head/kind or request-kind detail containing a digest is an internal
contract defect, redacted `internal` exit 4; never print unsafe detail as fallback.

Construct revision detail at `complete_acceptance`'s authoritative locked CAS from
intent and actual locked head. Construct collision detail at `intern_record` when
it discovers unequal exact records in one digest bucket, preserving it through
tree callbacks/transaction rollback. File/directory/value may carry the candidate
portable digest; request carries kind only and MUST omit its private digest.
Neither CLI nor Lua parses error messages, rereads head, derives a request digest
or reads request private body for a diagnostic. Preserve typed ownership through
format, service/runtime and shared response helper; native failure wins.

Public errors use canonical Glaze JSON plus LF on stderr, empty stdout (subject
to postcommit short writes above), existing envelope, `field:""`, and exactly:

| native failure | code / exact message | exact optional details | exit |
| --- | --- | --- | --- |
| locked head mismatch | `revision_conflict` / `Ledger revision does not match the expected revision` | `{expected:HEAD,actual:HEAD}`; HEAD is canonical decimal or `none` | 3 |
| unequal file/directory/value digest bucket | `hash_collision` / `Ledger record identity collision` | `{kind:K,digest:HEX}`; K is `file`, `directory` or `value`, HEX is 32 lowercase hex digits | 3 |
| unequal request digest bucket | `hash_collision` / `Ledger record identity collision` | `{kind:"request"}`; digest key absent | 3 |
| reused request ID / unequal body | `request_conflict` / `Ledger request conflicts with its saved intent` | absent | 3 |
| selected receipt changed | `capture_changed` / `Captured declaration inputs changed` | absent | 3 |

New authoritative load CAS/interner sites always provide applicable details;
existing unrelated error sites without a detail retain their existing generic
mapping. No invented `expected_head`, `actual_head`, private digest, request field,
record payload or host path is added. Public kind names are stable strings, not
C++ enum ordinals. All other native ledger codes/messages/exit policy remain
[existing mappings](../../src/cmd/planar/response.cpp); typed schema/reference/Lua
errors retain the existing render diagnostic schema and logical field paths.
Malformed shipped protocol uses `internal` / `Invalid embedded command protocol`,
`field:""`, no details, exit 4. Bounded exhaustion stays
`resource_limit`, and storage/busy/corrupt/io stay exit 4. Before completion can
have occurred, the standard bounded failure buffering applies. Once completion
may have occurred, only the uncertainty/stdout fallbacks above override a later
failure; they do not hide ordinary errors known to precede commit.

## Bounds and failure precedence

The lower applicable bound wins; no service resets a budget or multiplies limits.
All checks precede unbounded allocation/action and all native transfers count
cumulatively, including service results and retained registration objects.

| boundary | exact inherited maximum |
| --- | --- |
| catalog | 64 commands; 32 options/command; 1 MiB canonical bytes; names 64 ASCII bytes, handler IDs 128, help 4096 UTF-8 bytes |
| registration | 64 MiB live Lua; 10 million instructions; 2048 stack slots; sealed graph depth 256 |
| invocation | 512 MiB live Lua including registration; 100 million cumulative instructions; conversion depth 256, 4 million nodes, 512 MiB charged native allocation |
| host paths | 64 MiB each input/cwd/env/joined/normalized path; C checked joins/text-before-size order; name 1–64 bytes |
| request | ID 128 bytes; actor 256; complete record encoding 64 MiB including framing, operation, head and both private paths |
| restricted declaration evaluation | 64 value levels, 100000 converted nodes, 64 MiB live Lua and separate 64 MiB native accounting per chunk, 1 million instructions per chunk; original constructor/reference accounting retained |
| captured input | 1 MiB per declaration/text/registry read; 64 MiB cumulative retained bytes, 10000 files; no tree enumeration for load |
| stored source closure | 64 MiB payload, 4096 files, 8192 directories, depth 64, portable paths 1024 bytes; normalized modes 0644/0755 |
| registry / stdlib provenance | separately 16 MiB and 4096 files each; existing tree/path/depth bounds; stdlib includes synthetic assembly/manifest |
| stored values/files | single file 16 MiB subject to stricter text bound; interpretation and separate versions each 16 MiB canonical bytes |
| transaction/result | busy wait 5000 ms; positive IDs/sequences through INT64_MAX; result/event each 1 MiB; transport retains existing 16 MiB ceiling |

Declaration/schema list/string and loader limits in the
[render contract](render-contract.md#capture-boundary) remain unchanged; the table does not
relax them. Argument/registration/native conversion/response limits remain those
of the command contract. Resource exhaustion is terminal even if Lua catches it;
no instruction-budget reset across routing, validation, service calls or projection.
C++ ownership is outside allocating Lua frames so longjmp never skips destructors.

Precedence is parse/help, complete registration, selector/path/intent framing,
verified early replay/conflict, then fresh capture/routing/all-job interpretation
and preparation, then verified writable transaction/request relookup, locked CAS,
exact intern/equality/event assembly, final receipt verification, commit and
transport. Do not perform speculative current-head CAS before early replay or
reorder replay behind missing/invalid current input. Native errors cannot be
suppressed by handler return. Race tests distinguish an early miss followed by a
late equal replay from an early hit; early miss can encounter a genuine current
capture error before reaching the final transaction.

## Bounded follow-on rounds and acceptance matrix

Each slice needs a fresh read-only search, scoped coder and independent reviewer;
shared checkout, explicit ownership and no reversions. Review this exact amendment
before dependent implementation. No slice claims full R14 until the integrated
matrix passes. Proposed file ownership below is bounded by responsibility; each
round brief records its exact files and any justified build wiring before edits.

| slice / order | specific ownership | required evidence / traceability |
| --- | --- | --- |
| D4 native typed errors | `src/engine/ledger/{types.cppm,format.cpp,format.t.cpp,acceptance.t.cpp}`, `src/cmd/planar/{response.cpp,response.cppm}` and colocated response tests | Demonstrate failing revision/collision diagnostics before fix; locked expected/actual despite later head advance, file/internal/value/request collisions and rollback, request digest/key/body/path redaction. Preserve existing generic error bytes. A23/A26/A27/A36. |
| D5 bounded load capture/preparation | `src/engine/acceptance/` narrow new orchestration module/tests, `src/engine/capture/` only required load composition, `src/engine/history/` only codec composition tests; scoped engine CMake | Whole bundle/all jobs including invalid unselected jobs and duplicate job/work/role IDs, no selected execution role/source/base reads, normalized C target, full reference closure/ancestor receipts, unrelated sibling changes accepted; selected byte/mode/link/root/ancestor/registry races refuse. Prove B/M correspondence, native membership/provenance/versions and bounds. A01–A08/A20/A23/A28/A31. |
| D6 retained load protocol, private admission | `src/engine/runtime/{runtime.cppm,runtime.cpp,commands.t.cpp}`, test-only bundle/service providers; minimal embedding wiring | Three-call order/arguments/results/capability-only admission, same-bundle identity and actual altered callable dispatch, forged/cross-family/double/skipped services, malformed B/M/return, cumulative/caught exhaustion, no Lua under lock. Test-only fifth-entry catalogs until D7; shipped four entries stay valid. A23/A26/A36. |
| D7 integrated fifth command + replay/transport | `src/cmd/planar/handlers/load/{command.cppm,command.cpp}`, `bootstrap.*`, `invocation.*`, shared response/stdout composition; `src/stdlib/planar/commands.lua`, runtime catalog switch and focused CLI tests; scoped CMake, CLI/architecture/testing docs | Atomically admit parser, mandatory fifth catalog, callable/provider and fixed help. Early same-ID replay before handler/capture/current policy/CAS with deleted inputs/advanced head; changed actor/head/workspace/registry conflict; name/explicit equality and changed-env conflict. Same-ID concurrent exact replay once, unequal-ID-body conflict, different-ID CAS one winner; late relookup replay before stale CAS. Postcommit allocation/Lua/encoding/stdout faults and uncertain commit recover same saved bytes via retry/history. No private details. A20/A23/A25–A29/A36. |
| D8 full R14 acceptance closure | verification-only new CLI/native acceptance tests, scoped testing docs and round/evidence records; product fixes require fresh corrective scope | Initial/noop/comment/prose/change/restore chronology and exact source/provenance, invalid draft leaves head/history unchanged, all-job refusal, atomic crash/disk-full/busy/collision/capture rollback, lost-response recovery. Historical early revision/event/request reads and exact export after source/runtime removal without current policy; fixed original history/result/W/render bytes. All R14 portions of A20/A23/A25–A29/A31/A36. |

D7 is the only public admission switch; no permissive partial catalog or placeholder
load is shipped between slices. D5/D6 dependency seams may be adjusted mechanically
in their briefs without widening capabilities. Exact semantic/storage changes
require a reviewed amendment. Each code slice runs focused nonzero tests, zero
unexplained skips, pinned format/Doxygen and a zero-work rebuild; D7/D8 run the
complete serialized debug/release gate, preserving existing test names, fixtures,
goldens and variant audits. Existing approved baseline is 370/370 debug and release
on macOS ARM64/LLVM23.1.4, not load or Linux evidence. Capture actual commands,
counts, failures and platform limitations. The mandatory external-profile X gate
still follows full R14 and precedes dependent jobs/RA01; request_v2, profile loading
and execution remain outside this amendment.

## Source basis for the amendment

This candidate composes existing boundaries rather than declaring them implemented
as load services: [request/prepared ownership](../../src/engine/acceptance/prepared.cppm),
[verified request reads and atomic acceptance](../../src/engine/ledger/format.cppm),
[authoritative locked CAS/interner](../../src/engine/ledger/format.cpp),
[receipt ownership](../../src/engine/capture/native.cppm),
[whole-bundle trusted semantics](../../src/stdlib/planar/acceptance.lua),
[retained runtime services](../../src/engine/runtime/runtime.cppm),
[existing shipped callables](../../src/stdlib/planar/commands.lua),
[invocation composition](../../src/cmd/planar/invocation.cpp) and
[redacted response mappings](../../src/cmd/planar/response.cpp).
Native typed detail, completion receipt and load-family interfaces above are
proposed additions requiring code review; these source links are baseline evidence.
