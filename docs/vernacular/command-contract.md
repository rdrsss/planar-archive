# Embedded command and workspace contract v1

Status: V5-02-A independently approved exact target; B1 retained registry/runtime
approved with C01, and B2 CLI composition independently approved. [C workspace-name resolution and invocation snapshots](rounds/V5-02-C-review.md)
are independently approved. The [R14-D3 load amendment](load-command-contract.md)
is a documentation candidate pending independent review. Current commands remain render/init/history/export; load and
external profiles remain pending. The operator accepted A/B/C in [V5-02](rounds/V5-02.md#operator-decision).
This contract amends only the command bridge and workspace sections linked from
[render](render-contract.md) and [ledger](ledger-contract.md); all other exact
schemas, recipes, storage invariants and response bytes remain authoritative.
The [frozen proposal](v5-command-workspace-proposal.md) is historical design input.

Status pointer (X02, outside the exact embedded ABI below): R14-D7-B/D8 and the
[X01 parser spike](rounds/V5-02-X01-review.md) are independently approved.
The [external-profile amendment](external-profile-contract.md) and
[implementation path](external-profile-implementation.md) are proposed X02
candidates awaiting independent review and P1/P2 disposition. Current commands
include load; no external loading or v2 codec is shipped by this pointer.

Status pointer (RA01-A): the [authoring contract](authoring-contract.md) specifies six more mandatory
rows (`spec-prompt`, `critique`, `ingest`, `explain-rejection`, `observe`, `orchestrator`), so the catalog has
eleven mandatory rows at the RA01 integrated gate; it adds the six root-help lines, the opaque host-path slots
`diagnostic` and `from`, and the services `capture_authoring`, `capture_orchestrator`, `read_diagnostic` and
`read_observation_directory`, with their exact result values in the authoring contract (RA-2, RA-5, RA-6),
`capture_orchestrator` returns the sealed catalog's command names as a fifth key `commands` (authoring-contract
RA-2, operator decision 2026-10-31),
and it scopes the "no directory listing" sentence below to every family except `observe`. Nothing is shipped
by this pointer; the rows and bytes below stay current until
that gate.

Pointer (R15-A2): at the R15 integrated gate, the embedded catalog, the ABI rows, the option signatures, the services and the root help change as stated in this pointer. Catalog counts at that gate, per build (D148): ten mandatory rows in the production build (the current five plus the five R15 rows) and sixteen in a build that compiles the authoring bundle (the ten plus the six RA01 rows; the RA01 rows and the `orchestrator` verb exist only there). The history family gains `--event` in both builds; the orchestrator family gains the ledger form only where the orchestrator is compiled. Nothing is shipped until the R15 integrated gate; the rows, tables and bytes beside this pointer stay current until that gate, and this pointer narrows no approved clause.

ABI rows at that gate (handler IDs, response kinds and capability sets of the execution contract's P2-1 and P2-7):

| name / family | handler_id | response_kind | capabilities |
| --- | --- | --- | --- |
| activate / activate | `shipped.activate.v1` | `activate_v1` | `execution_commit_v1` |
| status / status | `shipped.status.v1` | `status_v1` | `execution_read_v1`, `draft_capture_v1` |
| preview / preview | `shipped.preview.v1` | `preview_v1` | `execution_read_v1` |
| claim / claim | `shipped.claim.v1` | `claim_v1` | `execution_commit_v1`, `provenance_read_v1` |
| takeover / takeover | `shipped.takeover.v1` | `claim_v1` | `execution_commit_v1`, `provenance_read_v1` |

The response kind `claim_v1` is shared by `claim` and `takeover`. Also, only where the orchestrator is compiled, `orchestrator` gains capability `execution_read_v1` beside `orchestrator_capture_v1`; `orchestrator --ledger` exists only there.

Option signatures at that gate (`!` marks a required option):

- activate text `--ledger!`, `--job!`, `--expect-revision!`, `--expect-job!`, `--request-id!`, `--actor!`; the help metavariable of `--expect-job` is `JOB_OR_NONE` (the value `NONE` means no active job, and any other value is a job ID).
- status text `--ledger!`, `--role`, `--request-id`, `--workspace`, `--registry` (`--role` excludes `--request-id` and nothing else; `--workspace` and `--registry` both or neither).
- preview text `--ledger!`, `--role!`.
- claim text `--ledger!`, `--job!`, `--role!`, `--request-id!`, `--actor!`, `--provenance`.
- takeover text `--ledger!`, `--job!`, `--role!`, `--request-id!`, `--actor!`, `--provenance` and decimal `--run-id!`.
- history gains decimal `--event` (excluding `--revision`, `--request-id`, `--at-event`, `--after-event`, `--limit`).
- orchestrator text `--ledger`, `--workspace`, `--registry`, `--job` all optional with the native both-or-exactly-one-form rule.

Lua boundary: the rule that database IDs and cursors (`revision`, `at-event`, `after-event`) cross into Lua only as canonical decimal strings, absent `"0"`, extends to `run-id` and `event`. Opaque slots gain `provenance`.

Services: the five families have the service lists of [P2-1](execution-contract.md#p2-1-verbs-and-services); `orchestrator` gains `read_execution({})` where the orchestrator is compiled; `read_history` gains the selector `event` and is called as `read_history({selector:"event",id:ID,request_id:"",limit:50})`, `limit` being the existing absent-limit value 50 that `commands.lua` passes (the event selector ignores it, no `limit:1`).

Fixed root help at that gate, per build (exact bytes; the current bytes stay in force until that gate). Production build (no authoring path, D148): the second line of the history block becomes the first line below, and five lines are inserted immediately before the final `planar --help` line, in this order, with no orchestrator line.

```text
  or --revision ID | --request-id ID | --at-event ID | --event ID
```

```text
planar activate --ledger PATH --job ID --expect-revision ID_OR_NONE --expect-job JOB_OR_NONE --request-id ID --actor TEXT
planar status --ledger PATH [--role ID | --request-id ID] [--workspace FILE_OR_NAME --registry FILE]
planar preview --ledger PATH --role ID
planar claim --ledger PATH --job ID --role ID --request-id ID --actor TEXT [--provenance FILE]
planar takeover --ledger PATH --job ID --role ID --run-id ID --request-id ID --actor TEXT [--provenance FILE]
planar --help
```

Authoring bundle (sixteen rows): the production bytes above, then the splice of `authoring::splice_root_help`, which inserts the six RA01 lines immediately before the final `planar --help` line and the continuation `  or --ledger PATH` directly after the `planar orchestrator` line. The relative order is therefore the history block, the export and load lines, the five R15 lines, the six RA01 lines, the continuation line (a seventh spliced line), then `planar --help`. The spliced lines are:

```text
planar spec-prompt --workspace FILE_OR_NAME --registry FILE
planar critique --workspace FILE_OR_NAME --registry FILE
planar ingest --workspace FILE_OR_NAME --registry FILE
planar explain-rejection --diagnostic FILE
planar observe --from DIRECTORY
planar orchestrator --workspace FILE_OR_NAME --registry FILE --job ID
  or --ledger PATH
planar --help
```

Every other help byte, the `with-profile` help bytes and the parse and usage precedence are unchanged.

## Catalog and structural ABI

The embedded bundle registers through `registration.add(descriptor, callable)`.
This function exists only during trusted registration. The native collected data
is `command_catalog_v1`, exactly `{version:1,commands:[descriptor,...]}`. Functions
are retained native Lua registry references, never serialized into this value.
Descriptors have exactly `name`, `family`, `handler_id`, `options`, `help`,
`response_kind`, `capabilities`. Each option has exactly `name`, `type`, `required`,
`help`. No metatables, sparse arrays, cycles, floats, unknown keys or coercions.
`required` is boolean; `type` is `text`, `decimal` or `flag`. Command/family names
match `[a-z][a-z0-9_-]{0,63}`; option names are `--` plus that grammar. Handler IDs
match `[a-z][a-z0-9_]*(\.[a-z][a-z0-9_]*)+`, at most 128 ASCII bytes. Help is valid
UTF-8 without NUL or control characters except LF/TAB, at most 4096 bytes each.
The catalog has at most 64 commands, 32 options per command and 1 MiB canonical
encoded bytes. Bounds include shipped entries and all option help. These are
ceilings, not permission to introduce a native family or new options.

Native ABI v1 admits exactly the following pairings. Capabilities are exact sets,
not requested authority automatically granted by a descriptor. Each handler ID
must be registered exactly once by shipped module `planar/commands.lua`, with one
function taking `(invocation, context)` and returning a response value. The module
path is the embedded logical source path; no host lookup is involved. Adding that
module to the assembly is part of V5-02-B, not an existing-source claim.

| name / family | handler_id | response_kind | capabilities |
| --- | --- | --- | --- |
| render / render | `shipped.render.v1` | `render_v1` | `render_capture_v1` |
| init / init | `shipped.init.v1` | `init_v1` | `initialize_ledger_v1` |
| history / history | `shipped.history.v1` | `history_v1` | `read_history_v1` |
| export / export | `shipped.export.v1` | `export_v1` | `export_source_v1` |

`response_kind` is this four-value enum, mapped to existing envelope kinds
`render`, `init`, `history`, `export`. No command can select another family's
response, native service or option signature. All four entries are mandatory.
A missing entry, unsupported pairing, duplicate verb/handler/option, reserved
`with-profile`/`--help`, nonfunction callable, wrong module owner or changed
structural signature rejects the whole catalog. Entries sort by name, options by
name and capabilities by name for canonical sealing; declaration order confers
no priority. There are no aliases, positional arguments, short flags, defaults
in descriptors, repeated options or optional scalar values. `help` describes the
entry but never supplies executable output or a replacement usage renderer.

The exact option signatures are below; `!` means required, all others optional.
These annotations are table notation, not additional descriptor fields.

| Family | text | decimal | flag |
| --- | --- | --- | --- |
| render | `--workspace!`, `--registry!`, `--root!`, `--job!`, `--role!`, `--base-root`, `--observations` | `--budget-bytes` | `--capture-outputs` |
| init | `--ledger!`, `--workspace-id!` | none | none |
| history | `--ledger!`, `--request-id` | `--revision`, `--at-event`, `--after-event`, `--limit` | none |
| export | `--ledger!`, `--target!` | `--revision!` | none |

Native family validators preserve current restrictions: scalar tokens are nonempty
and cannot start `--`; root mappings have nonempty parts around their first `=`;
budget is canonical positive decimal 1..1048576. History/export IDs are canonical
positive decimal 1..9223372036854775807, no signs or leading zeros. Limit is 1..100;
absent history limit is 50, absent after-event is 0. Request keys are 1..128 bytes
from `[A-Za-z0-9_.:-]`. History revision/request/at-event selectors are mutually
exclusive and each excludes paging. Render capture/base presence must match,
and their mapping IDs must match. These checks remain native structural guards;
Lua repeats no argv parsing. Workspace/root/graph semantic validity remains Lua
policy plus the existing native capture constraints. Init retains its existing
native workspace-ID validation and export its existing native target validation.

## Bootstrap, errors and retained lifetime

Invocation order is fixed:

1. Apply the existing sole syntax guard rejecting any token starting `--` that
   contains `=`; return the current usage response. It does not consume argv.
2. Construct a CLI11 probe from the compiled family signatures above, with retained
   option structs and current exclusions/validators. Parse exactly once; apply
   current subcommand/presence/count metadata checks. A CLI11 parse error,
   zero/multiple commands, terminator, extra or positional token returns usage.
   Sole root `--help` returns the current fixed embedded usage bytes without Lua
   registration, filesystem access or environment resolution. Other help fails
   usage. No fallback into a different parser or registry after failure.
3. For a valid ordinary command, register the embedded bundle and validate its
   complete catalog, then snapshot needed cwd/environment as specified below. Compare
   the catalog signatures with the compiled probe; no second argv interpretation
   is allowed. Native family option state stays alive through registration,
   semantic invocation, services and response encoding. A missing or invalid
   shipped catalog is a product defect: redacted `internal`, exit 4. Bounded
   registration exhaustion is `resource_limit`, exit 3. A caller's parse/usage
   failure therefore precedes a shipped registration defect. Invalid workspace
   spelling/environment is checked after successful registry validation but
   before any command filesystem operation.
4. Bind the selected descriptor and callable by native registry reference from
   the same captured embedded bundle in that same VM. Seal canonical descriptor
   bytes, handler ID, module logical path and exact bundle digest together. No
   later global lookup, module reload or callable replacement. Invoke that Lua
   function; it chooses semantic policy and response projection through its
   finite family services. Encode only the validated result with Glaze.
5. Release all references, handles, native option storage and the VM on terminal
   success or failure. Nothing is shared across invocations or cached globally.

The probe is an ABI parser, not a second native semantic dispatcher: removing or
changing a Lua handler changes execution or fails registration. The native family
is selected by the validated descriptor and enforces only its fixed capabilities,
input structure and safety invariants. Tests must replace an embedded handler in
a test bundle and prove its semantic result is actually used after identical CLI11
parsing. A metadata-only catalog over current native execution is insufficient.

Registration has no capture, ledger, filesystem, environment, clock, process,
output or hashing services; no jobs are evaluated. It may create local tables
and functions. Freeze registration APIs, descriptors, trusted module/global tables
and callable bindings before invocation; do not expose mutable registry tables.
Handlers may allocate invocation-local scratch tables, but cannot alter the sealed
registry or read another invocation. Declaration VMs get only their existing
curated constructors and never registry functions, handler refs or service handles.
There is no new shape/projection registration API in this embedded cut; current
shipped semantic exports remain callable internally by the trusted handlers.

Registration limits are 64 MiB live Lua allocation, 10 million instructions and
2048 Lua stack slots. Invocation raises the live allocation ceiling to 512 MiB,
including retained registration objects, and starts one fresh cumulative
100-million-instruction execution budget. No instruction reset between native
service calls, projections or protected calls; no reset after caught exhaustion.
Native conversion limits remain depth 256, four million nodes and 512 MiB charged
allocation per invocation, counting all transfers cumulatively. Each native
service keeps its stricter capture/storage/output bounds. Registration and
execution exhaustion are terminal even if Lua catches a protected-call error.
Unrecoverable allocation may retain the existing empty-stderr exit-4 fallback;
otherwise emit the typed bounded resource error. Protected C frames must not
longjmp across C++ objects with destructors. Service argument conversion and
response ownership live outside allocating Lua frames.

User semantic refusals retain existing typed codes/envelopes/exit policy. Unknown
service/response fields, forged handles, cross-family use, invalid call order,
wrong response kind or malformed shipped return are redacted `internal` exit 4;
resource bounds remain `resource_limit`. Native operations return typed errors,
never traceback or raw host data. Diagnostic field paths are logical paths.
Registration collision/schema details may be recorded in test evidence, but
production diagnostics for malformed shipped registration reveal no Lua source,
host paths, function addresses or unbounded text. An invocation cannot downgrade
native errors, publish a partial success or choose an exit code. Existing
[render errors](render-contract.md#errors-and-verification-boundary) and
[ledger errors](ledger-contract.md#proposed-command-surface) retain their mappings.

## Invocation values and finite native service protocol

The handler returns exactly `{ok:true,value:ENVELOPE}` or
`{ok:false,error:E}`; ENVELOPE must match its selected response_kind and the
existing response schema. Success requires completion of all family services in
order. Native compares init/history/export facts against retained native results;
render returns the Lua-owned semantic packet subject to the current render
structural/output bounds. Skipped services or manufactured ledger success are
internal failures. Expected Lua semantic errors use the existing bounded E schema;
native service failure always wins if Lua attempts to suppress or replace it.

The handler receives exactly `{version:1,command,options,present}`. `command` is
the sealed name. `present` has a boolean for every option name, without `--`.
`options` includes every nonpath scalar under that name: text strings, typed
decimals as specified next, flags (absent false), absent text empty. Presence
is authoritative for distinguishing absence. CLI11 coerces decimal tokens into
native signed-64 integers and retains the validated canonical lexeme. Database
IDs/cursors (`revision`, `at-event`, `after-event`) cross into Lua only as canonical
decimal strings, absent `"0"`; budget/limit are bounded signed-32 integers, absent
0. This preserves ledger ID strings at the Lua/canonical boundary, whose integer
range remains signed-32. No float/double conversion or Lua reparsing is used.
Host-bearing text options are
replaced by opaque native userdata slots: workspace, registry, root, base-root,
observations, ledger and target. Optional absent slots are omitted. These slots
carry only invocation identity/type and have no string/JSON conversion or public
fields. Root IDs are exposed separately inside capture results, never by exposing
the mapping token's directory. No argv, cwd, environment or host path crosses to
Lua. Native owns original strings for capture, request intent and services.

`context` contains only the methods in the selected row below. No generic op
string dispatcher, SQL, arbitrary IO, commit callback, subprocess or directory
listing is exposed (scoped RA01-A amendment: the `observe` family's `read_observation_directory` service of the
[authoring contract](authoring-contract.md#ra-6-observe-q11-a-e12-a) is the one bounded native directory
listing; every other family still exposes none). Each service is callable at most once, in the stated order;
failed service poisons further use and the invocation must return that exact
native failure. Success returns `{ok:true,value:V}`; expected failure returns
`{ok:false,error:E}` with the existing typed error value, retained natively for
exact propagation. No retries inside this invocation. Argument maps have exact
keys as stated. Every transfer is structurally bounded and deep-copied before
native action; handles cannot be serialized, saved as evidence or supplied by jobs.

| Family | Allowed service calls, in order | Native result / Lua responsibility |
| --- | --- | --- |
| render | `open_workspace({workspace:slot})`; `capture_render({route:R})` | First returns `{file,workspace}` using captured portable filename and declaration value. Lua computes R with current `route_workspace` semantics. Second accepts exactly that normalized route (root/spec/jobs), checks reference paths and source mapping, captures/evaluates spec/all jobs/references/registry/observations and optional source/base trees, finishes existing stability verification, and returns the exact current runtime render input (`bundle,capture,files,job,role` and optional `budget_bytes`). Lua runs current render validation, selection, W derivation and projection on that value, returning the existing render envelope. |
| init | `initialize_ledger({workspace_id:TEXT})` | ID must equal the invocation's workspace-id; native revalidates before exclusive publication. Returns `{ledger,workspace,active_revision:"none",event:"1"}` only after existing native initialization completes. Lua validates ID policy and constructs existing init envelope. No Lua executes inside initialization transaction/publication. |
| history | `read_history({selector:S,id:ID,request_id:TEXT,limit:COUNT})` | S is `page`, `revision`, `request` or `state`; Lua chooses selector/defaults from typed presence; native verifies exact equivalence to parsed selector, ID/key and bounds. Returns the verified existing history envelope, decoded by recorded version without executing recorded/current semantic policy. Lua owns the current response projection, which for ABI v1 returns that envelope unchanged; native compares exact canonical value before encoding. |
| export | `export_source({revision:ID})` | Revision must equal parsed revision. Native owns ledger/target slots, verified reconstruction, staging, no-overwrite publication and cleanup. Returns `{ledger,revision,source}` with current ID strings. Lua constructs existing export envelope; native requires exact agreement with the retained published result. |

`ID` is a canonical positive decimal string up to INT64_MAX, except page cursor
`"0"` for absence; `COUNT` is integer 1..100. Unused history `id` is `"0"` and
unused `request_id` is empty. Result fields named `ledger` are the existing
32-lowercase-hex ledger identity, never the private host ledger path. All result
revision/source/event fields retain existing decimal string representation.

R has exactly `{workspace,file,jobs,root,registry,spec}`: `workspace` is the
normalized declaration-v1 workspace, `file` its captured portable filename,
`jobs` the sorted list of `{id,file}` resolved portable references, `root` its
normalized `{id,base,excluded}`, `registry` its `{id,revision}`, and `spec` its
resolved portable reference. It is the current trusted routing result consumed by
[render handler](../../src/cmd/planar/handlers/render/command.cpp); arbitrary host
references are rejected by native capture even when trusted Lua supplies them.
Native render orchestration manages captured declarations and receipts; it cannot
substitute its own graph/selection/projection result. Trusted evaluation consumes
sealed data only. The two render calls are aggregate bounded operations; native
capture performs reference discovery under current declaration rules rather than
letting Lua acquire arbitrary per-path service authority.

For init/history/export, native services necessarily own storage semantics and
verified facts. Lua still executes the registered callable, chooses the typed
operation/selector and constructs or selects the response projection. It cannot
invent committed facts, edit historical values or hide failed native operations.
This is a deliberate invariant check, not a bypass of Lua semantic dispatch.
Only the matching native family may issue a service handle. Scope checking is
native even if Lua mutates a local options table. Service methods cannot access
another command family's handles or invoke current native command entry points.

Freeze all service arguments before entering any native write transaction.
While a transaction/write lock is held, native code runs alone: no Lua callback,
handler, projection, custom comparator or error formatting. Release the lock
before creating a Lua result. A waiting Lua C boundary frame grants no callback
permission. Native final verification/collision/CAS/atomicity checks remain
unbypassable, including all existing prepared-acceptance receipt checks.

Native export preallocates both publication-uncertain fallback and stdout-failure
diagnostic before issuing its service. After publication, Lua error, invalid
projection, resource exhaustion or encoding failure must use the existing
“Export may be complete; inspect the explicit target before retrying” fallback,
not report an ordinary prepublication failure. Preserve safe staging-token-only
cleanup diagnostics. On success retain the existing stdout-specific uncertainty
response; a short stdout write may leave bytes and the caller must discard them.
No Lua output handle exists. All other families retain current whole-response
buffering and native stdout failure handling, including lost init/load response
recovery limits. No new success can be printed during a service.

## Load admission and exact provenance

Load is not present in catalog v1 and must remain unknown usage until a separately
reviewed remaining-R14 implementation. That review must add one fixed native load
ABI, registered shipped semantic handler, exact option/response/service signature,
and tests; catalog capacity is not authority to add it opportunistically.
The [R14-D3 amendment](load-command-contract.md) specifies the proposed mandatory
fifth pairing, six required typed options, three finite services, replay/transport
and typed native diagnostics. It is pending independent review; the four-entry ABI
above describes current support until its integrated load admission gate.

Its native prelude must parse/validate typed intent, expand workspace names,
normalize private request paths and perform saved-request lookup before capture,
new-candidate semantic evaluation or CAS. Embedded registration is pure and may
precede lookup; no declaration/profile registration or candidate work may be
required for replay. Same intent returns saved exact bytes without invoking the
load handler, rereading workspace/registry or executing current Lua policy.
Unknown intent proceeds to sealed closure capture, real Lua normalization/
interpretation and native prepared acceptance, then native transaction relookup
before CAS. No Lua runs under write lock. A generic mutation capability is
forbidden. Request_v1 stays exactly unchanged for embedded-only load: name and
explicit spelling share intent only when expanded lexical targets are equal.

The new embedded command module changes ordered original source bytes,
assembly/manifests and exact stdlib digest; capture/verify that provenance using
existing codecs. `interpretation_v1`, `versions_v1` and record_v1 retain their
shapes. W recipes and interpretation version meanings do not change merely to
hash a catalog. With the same captured declarations/registry/observations, current
render envelope/Markdown/W bytes must remain equal; exact stdlib provenance in a
new accepted revision can change. Do not edit old evidence or regenerate goldens
to conceal a semantic change.

## Workspace selector and environment

This section is the independently approved V5-02-C policy, implemented for render
and targeted for later load. Current init ledger syntax stays explicit and unchanged; RA01
scaffolding is a separate later contract.

Classify `--workspace VALUE` after CLI11 validation, without filesystem access:
absolute path, any value containing `/`, or any value ending case-sensitive `.lua`
is an explicit file. Otherwise `[A-Za-z0-9][A-Za-z0-9_-]{0,63}` is a name. All other
bare spellings are `usage` exit 2 (including `.`, `..`, spaces, backslash, colon,
65-byte names and `.LUA` bare suffixes). `/` is the macOS/Linux separator;
backslash is never a name separator. Explicit paths retain the host filesystem's
valid byte spelling subject to UTF-8/no-NUL checks; this is not the declaration
portable-path grammar. The classification does not consult file existence.

Only a name consults `PLANAR_WORKSPACE_DIR`. It must be set, nonempty, absolute,
valid UTF-8 and NUL-free. Unset/empty/relative/invalid text gives `usage` exit 2,
no lookup and no HOME fallback. Lexically normalize it (collapse dot/parent
components); do not expand tilde, variables or links. A name expands to
`<normalized env>/<name>/workspace.lua`. Explicit files ignore this variable
completely, even if malformed. They never search cwd or parent directories for
a workspace; relative explicit paths are resolved only against invocation cwd.

Snapshot cwd once per invocation if any selected command needs relative path
resolution; retain its absolute lexical-normal UTF-8 spelling. Snapshot the env
once only for a named selector. Do not reread either between request framing and
capture. Failure to obtain cwd is redacted `io` exit 4; malformed internal cwd is
`path_denied` exit 3. Invalid explicit path text is `path_denied` exit 3. Each
argument, cwd, env, joined target and normalized target is bounded by existing
64 MiB `path_bytes`; overflow (checked before allocation/concatenation) is
`resource_limit` exit 3, maximum 67108864. Classification applies before this
limit to reject invalid bare names as usage. For valid explicit/name selectors,
validate relevant input text, then size and checked joins, then normalization;
no file lookup may precede these checks. Do not include values in diagnostics.

Maintain two native representations: absolute spelling with parent components
preserved for render physical resolution, and absolute lexical-normal target
for future load request intent/capture. Produce the latter before filesystem
access. Render continues resolving the preserved spelling's parent physically,
including intermediate directory links and `symlink/..` semantics, with no-follow
file leaf and existing root/captured alias rules. Name expansion begins with the
lexically normalized env base and then uses the same render confinement. Load
uses its normalized target after replay lookup; therefore explicit
`symlink/../workspace.lua` can identify a different render file than load file.
This is the preserved render-versus-request-intent distinction, not alias merging.
Missing files/parents are `io`; unsafe leaf/root/captured aliases are `path_denied`;
retained closure receipt changes are `capture_changed`. Name use grants no weaker
capture policy. Host names/paths stay outside portable identities and public
history. Replay needs string expansion/normalization only, never physical path
resolution or current declaration evaluation.

## Required implementation and verification sequence

1. **V5-02-B1 — retained runtime and sealed registry.** Fresh search/coder/reviewer;
   own `src/engine/runtime/`, new `src/stdlib/planar/commands.lua`, scoped bootstrap
   exports/build embedding and colocated runtime tests. Implement the catalog,
   retained VM/reference owner, pure registration, budgets and finite service
   boundary from this contract, with test-only service providers. No CLI behavior
   changes yet. Focused tests prove atomic schema/collision rejection, readonly
   binding retention, same-bundle dispatch, no job access, no cross-family/forged
   services and terminal budgets. No broad redesign gate is needed for these
   specified interfaces.
2. **V5-02-B2 — CLI families and service composition.** Fresh agents; own
   `src/cmd/planar/bootstrap.*`, `response.*`, `handlers/{render,init,history,export}/`,
   native service adapters below handlers, shipped command callables and focused
   CLI/stdlib tests; scoped architecture/CLI/testing references and build wiring.
   Compose B1 without handler/bootstrap cycles; use existing capture/history/
   init/export primitives, not alternate policy implementations. Only this
   integrated candidate delivers the bridge. Prove actual
   registered Lua dispatch, immutable same-bundle callable retention, pure
   registration, bounds including caught exhaustion, malformed/colliding catalog,
   every wrong family/capability/pairing, option storage lifetime, parse/help/error
   precedence, canonical output equality and export postpublication fallback.
3. **V5-02-C — workspace naming.** Fresh agents; native resolver and colocated/CLI
   tests plus product references. Cover name boundary bytes, explicit slash/suffix,
   env invalid/ignored/unset, cwd snapshot, checked path limits, name/explicit
   lexical intent equality, changed env conflict, parent-symlink/.. render/load
   distinction, missing inputs, leaf/root/captured aliases and replay with removed
   inputs. Public load replay cases complete with remaining R14, not a fake CLI.
4. **Remaining R14.** Review the [R14-D3 exact load amendment](load-command-contract.md),
   then follow its bounded native/capture/runtime/public-admission/full-gate slices.
   Admit fixed shipped load ABI through its own reviewed round,
   integrate closure-only capture/receipt/interpretation/atomic acceptance and close
   its complete matrix. Real replay must precede candidate capture/evaluation and
   preserve response bytes despite disappeared inputs/advanced head.
5. **External-profile gate, required after R14 and before dependent jobs/RA01.**
   Accepted policy is explicit `with-profile --stdlib-profile FILE` envelope,
   ordered listed modules, no ambient discovery, aliases/overrides/collisions or
   job selection of authority. Shipped and selected external Lua ultimately use
   the same invariant-enforced typed primitives; initial new external verbs are
   read/projection-only. Review exact native envelope ABI after a CLI11 passthrough
   spike, profile capture/trust/provenance, shapes/projections/identity, storage
   coexistence and new request_v2/effective-provenance/interpretation codecs before
   implementation. Request_v1 must not acquire an invisible profile field; replay
   must work with profiles/modules removed. Those codecs and envelope are not
   specified or supported as current by this embedded contract. Accepted proposal
   limits remain ceilings for that gate, not implemented safety evidence.
6. **RA01, then R15–R19.** Separate scaffold/prompt/observe/orchestrator contracts;
   preserve explicit empty-ledger init and no overwrite. Finite execution services
   require their own typed/native invariant review. No scheduler, models, watcher
   or checkout management is added.

Each code round runs focused nonzero tests with zero skips, applicable pinned
format/Doxygen checks and no-change build. Integrated B/C and R14 gates run direct
serialized debug/release full gates, preserve current goldens, and record exact
counts/toolchain/platform limitations; no Linux result is inferred. New adversarial
cases must prove capability refusal independently of trusted policy and use scratch
paths only. This docs-only round runs link/anchor, snapshot and protection checks;
it runs no product binaries, builds or tests and claims no runtime safety result.
