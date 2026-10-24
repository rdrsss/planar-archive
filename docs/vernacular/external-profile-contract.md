# External profile contract — X02 candidate

Status: **independently approved exact amendment with operator-selected P1 A/P2 A**,
recorded in [X02-C01](rounds/V5-02-X02-C01.md#operator-disposition--2026-10-22).
The original [X02 review](rounds/V5-02-X02-review.md) requested R1–R3;
[X02-C01 review](rounds/V5-02-X02-C01-review.md) independently approved the corrections.
This document does not ship external loading or close its mandatory runtime gate.
R14-D8 and X01 evidence remain unchanged. [Implementation path](external-profile-implementation.md).

Authority is [V5-02 operator A/B/C](rounds/V5-02.md#operator-decision),
[v5 shapes/projections/verbs](planar-design-spec-v5.md#engine-surface-and-sandbox), the
[X02 brief](rounds/V5-02-X02.md), [approved X01 review](rounds/V5-02-X01-review.md)
and [spike limitations](external-envelope-spike.md). The [declaration](declarations.md),
[render](render-contract.md), [ledger](ledger-contract.md), [command](command-contract.md)
and [load](load-command-contract.md) contracts remain current. This separate
amendment specifies approved additions; it does not rewrite their v1 bodies.
Normative “must” below describes the reviewed implementation target; X08 runtime acceptance remains open.

## X-POL — Product choices for review, then operator disposition

Accepted and fixed: explicit driver-selected `with-profile --stdlib-profile FILE`,
ordered modules after shipped modules; no discovery, aliases, overrides, collisions
or job-chosen trusted code; initial **new verbs** read/projection-only. That limit
does not prohibit trusted shape validation. Shapes and projections are required
in this gate; postponing all custom shapes would not deliver v5.

The operator selected A for both choices after independent X02-C01 review.
The alternatives below preserve the decision context; dependent slices implement
A. An alternative needs a scoped replacement contract and fresh review.

| ID | Alternatives and consequences | Selected policy / dependent slices |
| --- | --- | --- |
| P1 | A: custom shapes use existing worker/reviewer roles and job fields, may add phase obligations, stricter validation and guidance; no new state machine or arbitrary declaration fields. B: permit shape-defined schemas/role kinds now, requiring new constructors, versioned role/state/selection contracts before execution. | A, delivering genuine new workflows within current render/load semantics while retaining one independent reviewer and mandatory policy. X05/X06 implement selected P1 A. `review_loop` and tournament stay future v5 work. |
| P2 | A: reject trusted profile **directory tree** overlap with each supplied untrusted workspace/source/base root and selected standalone observation file, plus inode aliases. B: permit colocated trusted files when driver asserts protection, checking only selected-file aliases; less separation, stronger driver responsibility. | A, matching proposal's physical overlap refusal. Known shipped slots are checked before external registration; opaque extension slots only after pure registration and final CLI11 parse, before handler/read service use. A therefore does not promise separation from late-known inputs before registration. B retains the same admission phases with weaker overlap policy. No OS ownership proof or scanning unsupplied roots. X04 implements selected P2 A. |

The schema/response/byte mechanics below are independently reviewed implementation targets. Execution, mutation capabilities for
new verbs, arbitrary schemas and filesystem ownership attestation remain outside
this gate's authority.

## X-CLI — Envelope and admission order

Only these new forms exist (tokens separate):

```text
planar with-profile --stdlib-profile FILE COMMAND [OPTIONS]
planar with-profile --stdlib-profile FILE --help
planar with-profile --help --stdlib-profile FILE
```

New init targets use format2 after the atomic X admission switch; its existing
CLI signature and result bytes stay unchanged. Existing format1 remains readable
and supports embedded loads, with no conversion. This mechanical format cutover
avoids an unnecessary public format-selection option in an approved breaking
rewrite. No root/global profile flag, ambient
`stdlib.d` scan, environment profile selector, command alias or per-command help.
Ordinary sole `--help` stays fixed embedded help and performs no Lua/filesystem/
environment work. Selected-catalog help captures/registers the selected profile,
then prints deterministic catalog help. Help plus a command, duplicate help,
missing selector/tail, extras and unsupported options are usage errors.

1. Apply only the existing whole-invocation `--name=value` syntax guard. CLI11
   owns all token parsing. No manual split/option scan is added.
2. CLI11 root parses shipped signatures or `with-profile`. Envelope uses
   `prefix_command()`, scalar `--stdlib-profile` with Throw multiplicity,
   validator nonempty and not beginning `--`, and counted ordinary `--help`
   with default help disabled. Selector text is valid UTF-8 without NUL.
   `./--help` is a valid explicit filename. Raw CLI11's acceptance of empty or
   option-looking scalar values is not admission. Retained parsed option state,
   subcommand/option presence and token counts reject discarded terminators,
   duplicates and unexplained tokens. Require prefix count 3 (command plus two
   selector tokens), or 4 for sole selected help. Either help option order above
   is valid; neither allows a tail. Selector value is never reinterpreted as verb.
3. Copy CLI11's reverse-order `remaining_for_passthrough()` parse stack, preserving
   it unchanged for both probes. Embedded probe contains all shipped families
   plus optional vector positional `opaque-tail`, `expected(-1)` and
   `positionals_at_end()`. Require exactly one fully parsed shipped command with
   matching parsed token count and no opaque tokens/extras, **or** a nonempty
   opaque vector byte/order equal to the original tail, with no selected command,
   no extras and exact token count. Leading unknown flags/terminators fail before
   profile access. An unknown command's suffix remains opaque, including
   shipped-looking words, option-looking values and terminators. Known shipped
   signature failure returns usage immediately, never falls back to extension.
4. Snapshot cwd once and relevant workspace environment once. Apply current C
   workspace name rules to known slots; use this same snapshot for extension
   slots learned at step7. Normalize private profile selector lexically to absolute
   against this cwd without requiring existence. Host-path ceilings are 64 MiB
   for each input/cwd/env/joined/normalized path, and the complete request ceiling
   below still wins. No HOME or legacy configuration lookup. For shipped load,
   normalize all six existing flags (`ledger`, `workspace`, `registry`,
   `expect-revision`, `request-id`, `actor`) with current validators; ledger is
   the explicit target and not part of request equality.
5. **Load only:** verified read-only ledger open and exact saved-body lookup happen
   now, before external profile capture, registration, candidate evaluation or CAS.
   Equal request returns its original saved bytes and existing recovery fallback;
   unequal returns `request_conflict`. Neither touches profile/declarations/registry.
   Pure embedded registration is permitted before this lookup but is unnecessary.
   Format refusal is checked by the supported-format rules below. No current-head
   comparison is permitted before lookup. A v1 body and v2 body conflict.
6. On fresh intent (or selected help/other command), capture profile and module
   closure. After the early replay miss, check shipped-probe native path slots
   against that capture before external evaluation, then register the complete
   effective bundle and seal it. An opaque unknown-verb tail supplies no known
   path slots yet: pure registration may precede its initial overlap checks.
   All module capture completes before **any external Lua** executes. Failure
   abandons the invocation, with no partial catalog or response.
7. Parse original tail with a fresh CLI11 final parser generated from the sealed
   catalog, preserving option storage through dispatch. Apply same validators,
   counts and absence/extra/terminator rules. Unknown final command is usage **after**
   profile admission; a broken profile can therefore precede that unknown-command
   error. For shipped commands require exact equality to successful probe option
   state; no reinterpretation or fallback. `--stdlib-profile` after a verb has no
   selector role; extension text values beginning `--` are refused just like shipped
   text. A token spelling another command in a scalar position remains a value.
   Once this parse validates an extension descriptor and its native path slots,
   perform mandatory initial X-CAP checks for every supplied slot and retain
   receipts before binding/invoking its handler or any read service. No newly
   supplied untrusted input contents are consumed before their initial separation/
   alias checks; native early ledger lookup keeps step5 precedence.
8. Bind callable by sealed native registry reference, same retained VM and captured
   bundle. Dispatch once; destroy all state at terminal success/refusal.

All invocation syntax errors exit 2. Profile capture/schema/resource errors exit 3
unless native I/O/system errors require 4. Malformed trusted API/response failures
are redacted internal exit 4; collisions are `reference` exit 3 with logical fields.
No code, host path or traceback is printed. Current native failures and completion/
publication uncertainty fallbacks retain precedence over Lua returns. No external
Lua runs before early saved replay or under a native transaction/write lock.

Help is UTF-8 LF text: `Usage: planar with-profile --stdlib-profile FILE COMMAND [OPTIONS]\n\nCommands:\n`,
then commands sorted by unsigned name bytes, each `  NAME\tHELP\n`, then options
sorted by name as `    NAME TYPE REQUIRED\tHELP\n`; TYPE is descriptor type and
REQUIRED is `required` or `optional`. Help LF/TAB/control characters are escaped
for display as literal `\\n`, `\\t`, `\\u00XX`; backslash becomes `\\\\`. Default
root help stays byte-for-byte old. Buffer selected help within 1 MiB; no Lua output.

## X-CAP — Strict capture and trust

`stdlib_profile_v1` is UTF-8 JSON object with **exactly** `version:1` (JSON integer)
and `modules` (array of 0..64 distinct portable `.lua` file paths). Glaze parses
strict unknown/duplicate keys, trailing bytes, invalid UTF-8/NUL, nulls, floating
versions and wrong containers as `schema`; unsupported integer version is
`unsupported_version`. Leading/trailing JSON whitespace is allowed and retained
in exact profile bytes. No comments/BOM, recursion, globs, imports or discovery.
Modules resolve against the selected profile's containing directory, not cwd.
An explicit `stdlib.d/foo.lua` is allowed. Paths follow declaration portable file
rules (1024 bytes, segments 128), final suffix `.lua` case-sensitive. Reject
case-fold aliases of segments, duplicate paths, `.git`, and top segment
`planar` or `__planar__` (reserved shipped/synthetic namespaces). No bytecode.

Capture uses retained native directory/file descriptors, no-follow regular-file
reads, root/ancestor identity and no symlink traversal. Profile directory is one
retained trusted root; retain its selected ancestors, profile and every listed
module's identity, exact bytes, normalized 0644/0755 mode, path and stability
receipts. Reject distinct selected files sharing device/inode, including profile
versus module and existing registry/observation/declaration files. Retained bytes
never change when later verification runs. Profile and modules remain host-private
inputs; only logical `profile.json` and listed relative paths reach diagnostics.

Under selected P2 A, open/check supplied untrusted root descriptors and
selected standalone input identities at the admission phase defined below,
without reading their contents or evaluating declarations. Reject
physical equal/nested trees in **either direction** between trusted profile root
and workspace declaration root, supplied render source root and supplied base
root. Root checks apply even when render omits output capture; do not enumerate
source/base merely to check containment. Compare physical ancestor identities,
not lexical prefixes. Observation input is a selected standalone untrusted file:
reject if inside trusted root or aliases any trusted selected file. Registry is
separately driver-trusted; do not prohibit its directory sharing by policy, but
reject selected-file aliases with profile/modules. Ledger is native data: never
execute it; refuse selected-file alias with ledger, and on export reject trusted
root/selected files overlapping the proposed target tree before publication.
Missing supplied paths retain existing family capture errors. Selected help has
no untrusted roots supplied and checks only its trusted capture; history/init
have no inferred workspace root. No scan of omitted roots, operator directories,
unsupplied files or environment conventions is authorized.

Initial admission has two path-knowledge phases, with no shadow argv scan:

- **Known shipped tail:** after successful embedded probe, normalization and any
  early load replay miss, capture the profile closure, then check every supplied
  native root/file/target slot before external Lua evaluation. Early equal replay
  or conflict precedes capture and all these checks, as X-CLI specifies.
- **Opaque extension tail:** capture and pure registration/sealing occur first;
  registration has no read capabilities or untrusted input bytes. Only successful
  final CLI11 parsing of the sealed descriptor identifies host-bearing slots.
  Check every supplied slot now and retain native receipts before handler entry
  or any read service. A malformed profile/registration therefore precedes final
  unknown-command/option errors; final syntax/selector errors precede late-slot
  overlap errors; an initial overlap error precedes handler/service execution.
  There is no claim to check unknown slots before registration. Under P2 A this
  means trusted registration can already have run with an overlapping late-known
  input present on disk; no contents from that input are supplied to it.

Opening metadata/ancestor descriptors for these checks is not content capture.
All supplied slots require initial checks; a handler cannot avoid them by skipping
its service. Declaration closure can add files: check each discovered identity
for aliases before reading its contents, evaluating it or exposing its bytes to
trusted semantics. Abandon on any overlap/alias without mutation. Retain receipts
from initial checks, including late-slot checks. Reverify retained profile/module/
root/ancestor receipts after registration for known slots, after late-slot initial
admission, before service results authorize output, and with all acceptance
receipts immediately before load commit under lock. Recheck known root
relationships and selected aliases at these seals; a changed receipt refuses
`capture_changed` before result authorization/commit. No Lua runs under lock.
No unrelated sibling membership check in the trusted directory: only listed
closure and necessary ancestor identities matter. Render retains its existing
whole-tree untrusted capture rule; load retains declaration-closure-only membership.

This supplements the driver's decision to protect trusted configuration from
harness edits. It proves neither OS ownership nor immutability against hostile
writers, change-and-revert or a write after final receipt check. No sandbox can
make an intentionally hostile selected trusted module semantically correct.

## X-REG — Registration and immutable ownership

Shipped bundle executes first; external chunks execute in profile list order in
that same bounded trusted VM. Each chunk is loaded as source only under a native
logical chunk name, no host filename, and returns zero values. No `require`, package,
load/loadfile/dofile, debug, os, io, coroutine, clock, env, process, stdout/stderr
or filesystem access. Same curated pure standard helpers as embedded registration;
registration-only API contains exactly `add`, `shape`, `projection` below. No global
mutation API and no executable material shared with declaration states. Module
locals/closures may remain callable; module order gives no override priority.

Native assigns owner, never accepts owner claims from Lua. Owner is exactly
`{tier:"shipped"|"external",module:LOGICAL,ordinal:N}`; shipped ordinals follow
embedded manifest, external 1..64 follow profile. External LOGICAL is
`external/` plus listed path, shipped `shipped/` plus embedded path. The active
module owns every callable registered during its protected evaluation. Calling a
saved registration function later or from another phase is internal failure.
An `owner`, digest, schema-version override or foreign handle in any descriptor
is unknown-field failure. Native manifest annotates owner and ABI after validation.

`registration.add(D,F)` retains command F and exact descriptor D. Shipped entries
and signatures, including load, remain mandatory
native pairings. External D has exactly `name,family,handler_id,options,help,
response_kind,capabilities`, existing command grammar/limits and sorted canonical
options/capabilities. External family is exactly `extension_read_v1`, response_kind
exactly `extension_v1`; handler ID is existing dotted grammar. Options have exactly
`name,type,required,help`, types `text|decimal|flag|history_id`. General decimal
is canonical positive 1..2147483647 converted natively to a Lua signed-32 integer;
absent decimal is integer 0. Text is nonempty UTF-8/no NUL and not starting `--`,
max 65536 bytes; absent text is empty, absent flag false. `present` has a boolean
for every option, without `--`; absence never has `present:true`.

`history_id` is a distinct native capability-bound descriptor type, allowed only
for optional `--revision`, `--at-event`, `--after-event` on a command declaring
`history_read_v1`. Those three history options must use this type, never general
`decimal`; it is forbidden on every other name/capability and cannot be required.
CLI11 retains the canonical positive token and converts it to native signed-64
for range validation: grammar `[1-9][0-9]*`, maximum 9223372036854775807
(INT64_MAX). Lua receives the exact validated decimal **string**, absent `"0"`
with `present:false`; supplied zero is invalid. No float, Lua integer coercion or
Lua reparsing is used. A descriptor/type/capability violation is trusted protocol
`internal` exit4; malformed/overflow/conflicting invocation values are usage exit2
before handler/service use. This exception does not widen any general decimal.

No defaults/repeats/positionals/aliases/short flags. Flags
cannot be required. Reserved command `with-profile`, reserved option `--help`
or `--stdlib-profile`, shipped command/family/handler ID reuse and any duplicate
command/handler/option reject the **whole** catalog. Namespaces for shape IDs
and projection IDs are separate, but each refuses shipped/other duplicates.

`registration.shape(D,F)` has exact D `{id,abi,revision,stages,help}`. ID follows
declaration ID grammar, abi exactly `shape_v1`, revision is ID (semantic version
label, no automatic code-change guarantee), stages exactly `required|forbidden|optional`.
F is the pure shape validator/phase expander below. Shipped IDs `pipeline` and
`fanout_review` are reserved; adapter registration retains their old behavior.

`registration.projection(D,F)` has exact D `{id,abi,revision,input,help}`: ID and
revision are IDs, abi `projection_v1`, input `role_context_v1` or
`history_snapshot_v1`. F is the pure projection below. IDs `worker`, `reviewer`
and `recovery` remain shipped-owned; registering one cannot replace them.

All descriptors/callable refs/upvalues/reachable trusted tables and API tables
are sealed against writes after registration, including tables retained through
another reference. Sealed references cannot be changed by a later module or
handler. Reject unsealable cycles/functions with unsafe mutable environments;
closure identity is the retained reference, not function address or bytecode hash.
Invocation-local scratch is allowed but cannot mutate sealed objects. Native
seals one manifest containing all descriptors, owners, ABI IDs and effective
bundle digest; no later global lookup/reload/substitution. Pass typed services
only on invocation, never to registration or shape/projection pure functions.
Declaration VMs retain existing curated constructors and no trusted references.

## X-SHAPE — Shape semantics and selected identity

Selected P1 A introduces explicit profile-only declaration dialect
`declarations_shapes_v1`, with normalization dialect `normalization_shapes_v1`.
The old `declarations_v1` domain and its literal two-value shape enum stay exact
for embedded-only invocations. In the new dialect only `job.shape` admissible IDs
and `stages` policy change. The source constructor `version:1` remains the data
layout version, while native retained profile admission selects this separate
trusted dialect; jobs cannot opt into it or choose its implementation. There are
no added fields, arbitrary parameters, constructors, role kinds,
checks, execution state or job-defined callbacks. Other declaration-v1 schemas
and mandatory validation remain exact. Unknown shape ID is `reference` at
`job.shape`; stages presence must match D.stages. Stage entries are the existing
nonempty sets of worker IDs, each worker exactly once when stages are present.
Custom F receives exactly `{version:1,job,spec,assignments,reviewer}` after
normalization and base validation, all immutable normalized data with no host paths.
All jobs invoke their selected shape regardless of later role scoping.

Base trusted validation runs unchanged for unique full worker assignment,
independent reviewer targeting all work, ownership, required effective checks,
registry allowlists, valid subscriptions/questions and the mandatory phase edges.
F can refuse with existing semantic error or return exactly
`{ok:true,value:{obligations:LIST,guidance:TEXT}}`. TEXT is UTF-8/no NUL, 65536 bytes.
Each obligation is exactly `{id,producer,consumer,phase,quality}`: id is ID unique
within selected shape, producer/consumer distinct existing work IDs, phase
`start|complete`, quality exactly `done`. No facet/value/availability/approval
is fabricated. At most 10000 obligations/job. Material normalized obligation gains
`id:"shape:"..shape_id..":"..id`, `source:"shape"`, `producer_role`,
`consumer_role` from base assignments. These unforgeable synthetic IDs cannot
collide with declaration dependencies. Add producer.complete → consumer.phase,
then rerun the complete cycle check with all original edges. Grouping preserves
unit obligations; no role self-wait is substituted. F cannot delete any original
edge, check, requirement or boundary. F's semantic refusal uses bounded existing
logical diagnostics; invalid return is trusted internal failure.

For external shapes, normalized semantic job is existing model plus
`selected_shape:{id,revision,definition,obligations,guidance}`. `definition` is
H(`shape_definition_v1`, `{descriptor:D,owner:{tier:OWNER.tier,module:OWNER.module},module_bytes:HEX}`), where H
is current canonical-v1/seed-zero XXH3-128 typed domain function and HEX is exact
lowercase original selected-module bytes (bound before expansion). A selected
module byte change, including comments, conservatively changes this shape's
identity. No other external module, profile bytes, descriptor catalog or host
selector enters it; any semantic closure dependency must live in that module
or be represented by revision change. Imported callbacks/helpers across external
module boundaries are unavailable. Shipped pure helper semantics remain pinned
by the existing W version vector; helper changes require a reviewed version bump.
A lying revision label is a trusted policy bug, not something native hashes prove.

External-shape W uses new domain `work_external_shape_v1` and exact old W fields,
except its `versions` array appends `shape_selection_v1`, and `shape` appends
`definition,revision,obligations` to old `{kind,max_rounds,mapping,reviewer}`.
Obligations are scoped to selected work as consumer or producer, sorted by
synthetic ID. All selected shape definition changes affect all roles of that job;
normalized obligations remain material, guidance is projection text only. Pipeline
predecessors remain old fields/recipes for shipped pipeline. Every old W recipe,
including the old version vector and `work_v1` hash, stays byte-for-byte for
shipped shapes even with a profile selected. Unselected shape/module or
projection-only changes alter effective provenance, not shipped W. Same-module
shape+projection edits conservatively affect the selected custom shape; use
separate files to keep visual edits outside its definition identity.

Render of an external shape uses `render_external_v1` described below; current
render of shipped shapes without a custom projection stays old render-v1 JSON,
Markdown and W. Exact accepted-source/provenance changes still create revisions
under load even when W is equal. This gate claims no reconciliation, claims,
review loop, process execution or proof of done/accepted state.

## X-PROJ — Pure projections and native read capabilities

External projections receive only immutable data. `role_context_v1` is exactly
current selected-role value (`context,work,base,observation,outputs,targets,files,
prerequisites,facets`, optional `role_output`) plus
`shape_guidance` (external selected guidance or empty). Existing selection and
observation validation happen before projection; projection cannot reselect W,
rewrite manifests or reinterpret history. `history_snapshot_v1` is exactly a
verified native history envelope with private request bodies/host paths excluded.

F returns exactly `{version:1,sections:LIST}`; each section exactly
`{id,text,mandatory}`, unique declaration ID, UTF-8/no NUL text, boolean mandatory.
Max 64 sections, text max 1 MiB each, cumulative generated text max 16 MiB. Native
validates shape/limits including collisions with globally reserved built-in section
IDs. Both input kinds reserve the same complete union, including conditional
sections, generated omissions and recorded history:
`authority`, `goal`, `acceptance`, `requirements`, `contracts`, `boundary`,
`dependencies`, `shape_policy`, `checks`, `outputs`, `observations`, `preferences`,
`rationale`, `workspace_notes`, `job_notes`, `checkpoint`, `omissions`,
`recorded_history`. Reservation does not depend on which sections a selected input
currently emits; the later native result scanner owns this shared semantic policy.
Trusted Lua renders section bytes with current headed/join
convention. Current mandatory packet sections are generated by the shipped
projection and retained verbatim; custom sections append, never replace/drop or
relabel them. All custom `mandatory:true` sections count toward refusal. Optional
custom sections are omitted in reverse projection result order before existing
optional omission priorities (`checkpoint`, `job_notes`, `workspace_notes`,
`rationale`). Append omitted IDs in actual removal order, and use that same order
in the notice. Section metadata stays in original section order, followed by the
notice metadata. Exact omission notice/counts use current algorithm, including
section joins; never sort omitted IDs back to original order. No truncation.
Role context budget remains max 1 MiB;
history projection budget is exactly 1 MiB and includes first mandatory section
`recorded_history`, whose body is exactly `Recorded history; ledger ` plus the
retained ledger identity plus `; last event ` plus retained `last_event` plus
`. No current-policy reinterpretation.\n`, with no implication of current-policy historical reevaluation.

New external command handler F receives existing invocation record and a context
with exactly the declared subset of these methods (capabilities are an exact set,
unknown capability refuses registration). Services are each at most once and in
table order; failed service poisons invocation; no retries or swallowed failures.
A missing capability yields internal refusal, not an implicit grant.

| Capability / method | Exact arguments and result / safeguards |
| --- | --- |
| `role_context_read_v1` / `context.read_role({})` | Requires command descriptor options exactly including required text workspace,registry,root,job,role and optional base-root,observations; optional decimal budget-bytes and flag capture-outputs, with current render matching/presence guards. No other host option is accepted. Native captures through existing two-stage route/capture semantics; shipped trusted selection/observations validates all jobs and yields immutable role_context_v1. Slots stay native and scope to this invocation. |
| `history_read_v1` / `context.read_history({})` | Requires required text ledger; optional revision,at-event,after-event of type history_id, optional limit decimal and request-id text, with the exact guards/defaults and ID-string ABI below. Returns native verified history_snapshot_v1. It never executes old/current semantic Lua. |
| `projection_apply_v1` / `context.project({id:ID})` | Requires exactly one of previous read capabilities; no both. ID must name sealed projection with matching input type. Invokes it once on retained result, no caller replacement input. Returns bounded text with section/omission metadata. |

History read guards are native-owned: `revision`, `at-event` and `request-id`
are mutually exclusive selectors; any selector excludes both `after-event` and
`limit`. `after-event` and `limit` may occur together for page reads. All three ID
options are optional; absent strings stay `"0"`, with absence authoritative from
`present`. Page cursor defaults natively to `"0"`, limit to 50; absent invocation
limit remains integer 0 with `present:false`. Supplied limit is canonical 1..100,
Lua integer, not history_id. Request-id uses current ASCII `[A-Za-z0-9_.:-]`,
1..128 bytes; absent empty with `present:false`. Ledger is required opaque slot.

Required ID boundary vectors (for each of the three history_id names):

| Supplied token | Native/Lua result before service |
| --- | --- |
| absent | `options[name]="0"`, `present[name]=false` |
| `1`, `2147483647`, `2147483648`, `9223372036854775807` | accepted native signed-64 ID, exact same Lua string, `present[name]=true` |
| `0`, `00`, `01`, `+1`, `-1`, `1.0`, `1e0`, empty, whitespace, `9223372036854775808` | usage exit2, no handler/service |
| valid ID plus another selector, or selector plus after-event/limit | usage exit2 regardless of ID magnitude |

General decimal boundary vectors remain: `2147483647` accepted as integer where
that option's narrower bound permits; `2147483648` and INT64_MAX refused, never
string-converted. History limit still accepts 1/100 and refuses 0/101 regardless
of the wider history_id type. X05-C and X07 must prove Lua value type and presence,
not merely successful native parsing, including valid after-event+limit paging.

The first two capabilities are mutually exclusive. Without either, handler may
compute pure bounded scalar summary from options; project capability requires one.
Additional nonhost text/decimal/flag options use X-REG limits and confer no IO;
`history_id` is unavailable for additional options.
Any recognized host-bearing option listed above becomes an opaque native slot;
slots have no stringify/JSON fields and cannot be substituted. Unknown text is
data, never interpreted as a path. Registry ownership remains separate from
stdlib profile; a job cannot choose either. No arbitrary path read, SQL, directory
listing, general dispatch, output handle, ledger mutation, export/init/load service,
subprocess, write_state, clock/env or execution capability is present.

Response is closed `extension_v1`: exactly
`{version:1,kind:"extension",command:NAME,data:DATA}`. DATA is one of:

- `{type:"summary",text:TEXT}`; bounded UTF-8/no NUL text, max 1 MiB, zero service
  capabilities, no asserted ledger/acceptance facts.
- `{type:"role",value:ROLE}`; exact retained role_context value after `read_role`.
- `{type:"history",value:HISTORY}`; exact retained native history result.
- `{type:"projection",id:ID,revision:ID,definition:HEX,text:TEXT,sections:SECTIONS,omitted:IDS}`; exact retained
  project result. Definition is H(`projection_definition_v1`, the same descriptor,
logical owner without ordinal and exact module-byte record recipe as shapes). It
affects response/render identity only. SECTIONS is the original section-order list
  `{id,bytes,included}`, followed by `omissions` when present, with nonnegative
  signed-32 counts; IDS contains omitted IDs in actual removal order.

Multiple-omission byte vector, isolated assembly input (ASCII, every `\n` below
is one LF; it is an algorithm vector, not a replacement for required shipped
packet sections): original sections are mandatory `base` with fully headed text
`"## base\n\nB\n"` (11 bytes), optional `a` with
`"## a\n\nAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA\n"` (37), and optional `b` with
`"## b\n\nBBBBBBBBBBBBBBBBBBBBBBBBBBBBBB\n"` (37), in that order. Budget is
60 bytes. Initial joined size is 87. Removing `b` appends `b` to omitted and adds
`"## omissions\n\nOptional sections omitted: b.\n"` (44 bytes); total is
94, so remove `a` next. Final omitted is `["b","a"]`; notice is exactly
`"## omissions\n\nOptional sections omitted: b, a.\n"` (47 bytes). Final
text is `"## base\n\nB\n\n## omissions\n\nOptional sections omitted: b, a.\n"`,
59 bytes: 11 base + 1 joining LF + 47 notice. SECTIONS is exactly
`[{id:"base",bytes:11,included:true},{id:"a",bytes:37,included:false},
{id:"b",bytes:37,included:false},{id:"omissions",bytes:48,included:true}]`.
Omitted entries retain their own fully headed text byte counts with no join;
included entries count a joining LF only if preceded by an included section.
The notice is mandatory and counts its joining LF; exceeding budget even after
all optional removals refuses, with no truncation. This preserves the assembly/
notice/removal algorithm in `src/stdlib/planar/render.lua:160–187`; that source is
an implementation basis, not evidence of external projection execution.

Handler returns existing `{ok:true,value:ENVELOPE}` or `{ok:false,error:E}`.
Native requires retained service/response correspondence and mandatory services
exactly once. Lua cannot fabricate authoritative read facts, select exit codes,
output raw JSON or suppress native errors. Error E is exactly existing render
error schema and bounded fields, not a free-form map. New external-shape render
uses `{version:1,kind:"render_external",packet:P,manifest:M,shape:{id,revision,
definition}}`; P/M are current structural packet/manifest including amended W,
with manifest field `selected_shape:{id,revision,definition}` added; M.versions
is exactly amended W.versions plus `projection_v2` as before (no duplicated
shape version). Every scoped shape obligation also adds a prerequisite containing
all normalized obligation fields plus `status:"unverified"`, sorted with existing
prerequisites by ID. This extends the custom envelope prerequisite source enum
with `shape`; ordinary render-v1 keeps its old enum. Packet P remains structural
version1. A mandatory `shape_policy` section follows existing `dependencies`
section: its body is current `presentation_data("shape guidance; trusted policy",
guidance)` followed by `presentation_data("shape obligations; canonical JSON",
Glaze_json(scoped_obligations))`. Labels, fencing and display escaping follow
current presentation helpers; it never asserts done/accepted. Existing section
IDs are reserved against custom projection result IDs. Custom projection selection occurs
only through extension `context.project`, never by adding a shipped-render flag. Ordinary
external verbs always return extension envelope, never masquerade as load/history.
One buffered Glaze JSON response plus LF on stdout, error JSON plus LF on stderr;
no output during registration/services. Whole-response ceiling 16 MiB, lower
projection/budget/result limits win. New shape/projection output does not establish
execution or independent review.

## X-LIMIT — Cumulative ceilings and errors

All bounds are checked before allocation/copy/action, with checked addition.
Lower applicable inherited bound wins; stages/services/protected calls do not
reset budgets. Ceilings are not capacity/RSS guarantees.

| Boundary | Exact candidate bound |
| --- | --- |
| Profile | 65536 exact input bytes; 64 modules; JSON depth 4, 132 nodes, path/string <=1024 bytes except scalar fields fixed above |
| Module capture | 1 MiB each, 8 MiB total original external bytes; profile+modules <=65 selected files; total retained capture including existing inputs <=64 MiB and 10000 files |
| Catalog | 64 commands, 32 options each, 64 shapes, 64 projections **including shipped**; each help 4096 bytes; entire canonical combined manifest <=1 MiB |
| Registration | 64 MiB live Lua, 10 million cumulative instructions including shipped modules, stack 2048, sealed graph depth256 |
| Invocation | 512 MiB live Lua including retained registration; 100 million cumulative instructions; depth256, 4 million converted nodes, 512 MiB charged native conversion allocation |
| Declarations | Existing per-chunk depth64/nodes100000, Lua64 MiB/native64 MiB, 1 million instructions; input/text <=1 MiB; no relaxed list/string/count rules |
| Shape | 10000 obligations/job; total obligations across all jobs <=100000; complete shape return <=16 MiB; graph uses existing validation limits plus same cumulative native/Lua ceilings |
| Projection | 64 sections, 1 MiB/section, 16 MiB generated total; role budget <=1 MiB; history projection 1 MiB; error/message/help inherited stricter bounds |
| Persistence | Source64 MiB/4096 files/8192 directories/depth64; registry16 MiB/4096 files; effective stdlib32 MiB/8192 files/depth64; paths1024/segments128; single BLOB16 MiB; interpretation/versions each16 MiB |
| Request/result | Complete framed request64 MiB; ID128 bytes, actor256; event/result1 MiB each; history1..100 default50; output16 MiB; busy5000ms; IDs through INT64_MAX |

Profile schema/path errors use existing `schema`, `path`, `unsafe_path`,
`unsupported_version` as applicable; collisions `reference`, physical overlap/
selected aliases `unsafe_path`, changed receipts `capture_changed`, exhaustion
`resource_limit`. Missing/no-follow/capture I/O uses existing native mapping.
Unexpected trusted Lua error/invalid descriptor/protocol is `internal` exit4,
redacted and bounded; semantic validation/resource exit3; usage exit2; success0.
Logical fields are `profile`, `profile.modules[N]`, `catalog.commands[N]`,
`catalog.shapes[N]`, `catalog.projections[N]`, or existing job field paths; indices
are one-based original order. Diagnostic details never carry profile host selector,
module bytes, request digest/body or traceback. New native sites use this closed message table (existing propagated native
errors keep their exact messages); no new public code is introduced.

| New site | Code / exact message |
| --- | --- |
| Malformed profile JSON structure | `schema` / `Invalid external profile structure` |
| Unsupported profile integer version | `unsupported_version` / `Unsupported external profile version` |
| Module portable spelling | `path` / `Invalid external module path` |
| Trusted/untrusted overlap or selected alias | `unsafe_path` / `External profile overlaps a supplied input` |
| Registration duplicate/reserved identity | `reference` / `External registration identity conflict` |
| Profile/module/root receipt changed | `capture_changed` / `Captured external profile changed` |
| New bound exceeded | `resource_limit` / `External profile resource limit exceeded` |
| Wrong trusted API/return/owner/handle | `internal` / `Invalid external profile protocol` |

Messages contain no substitutions; field identifies only logical location.
Existing load saved-byte fallbacks remain exact.
C binding protected frames retain C++ ownership outside Lua longjmp paths.

## X-STORE — Byte codecs, exact provenance and supported formats

Canonical values remain `canonical_v1`; signed-32 integer domain, list/map
separation, UTF-8, sorted keys and XXH3 byte representation do not change.
Existing `PLANAR-RECORD` byte01 tags 01..04, `request_v1`, embedded manifest,
interpretation_v1 and versions_v1 remain exact and retain independent readers.
No hidden field is appended to any v1 record.

New `request_v2` framing is exact ASCII `PLANAR-REQUEST`, byte02, then
`bytes(operation) || bytes(actor) || bytes(expected_head) || bytes(workspace_argument)
|| bytes(registry_argument) || selector_tag || [bytes(profile_argument)]`.
`bytes` means unsigned big-endian u64 byte length followed by bytes, no terminator;
selector_tag is byte00 embedded (no suffix) or byte01 profile (one suffix).
Operation exactly `load`; old actor/head/path validation applies. Profile argument
is normalized absolute lexical selector from X-CLI, valid UTF-8/no NUL/nonempty.
No trailing bytes, unknown tag, captured content/digest, current head, Lua version,
ledger path or time. Request digest is seed-zero XXH3-128 over **complete framing**,
and equality is complete exact bytes including version; same ID different-version
body conflicts. Selector paths are private and absent from portable provenance.

Format2 stores this as SQL record kind4, codec2, encoded bytes above. No v1 wrapper
around it. Request-v1 remains kind4 codec1. File/directory/value records stay
record_v1 codec1. SQL v2 is exactly ledger-contract v1 schema/triggers/indexes,
with `ledger_meta.format CHECK(format=2)`, `PRAGMA user_version=2`, and the
records codec CHECK replaced by
`CHECK((kind=4 AND codec IN (1,2)) OR (kind BETWEEN 1 AND 3 AND codec=1))`.
Application ID unchanged. All other SQL text, triggers, event-v1/load result-v1,
IDs/chronology, integrity/identity/collision/CAS/recovery rules remain exact.
The schema verifier compares the selected exact v1 or v2 schema; no ALTER on open.
Format2 foreign-key/tree decoder forbids request children regardless of codec.

`effective_stdlib_v1` is a separate provenance tree using record_v1 files/directories:

| Portable path | Exact contents / normalized mode |
| --- | --- |
| `shipped/<original path>` | Exact shipped original source bytes, 0644 |
| `__planar__/embedded.lua` | Exact existing shipped assembly bytes, 0644 |
| `__planar__/embedded.bin` | Exact existing embedded canonical manifest bytes, 0644 |
| `profile/input.json` | Exact selected profile JSON bytes, captured normalized mode |
| `external/<listed path>` | Exact module bytes, captured normalized mode |
| `__planar__/effective.bin` | canonical_v1 manifest below, 0644 |

No concatenated external assembly duplicates the 8 MiB original set. Each module
is a separate protected chunk in exact recorded order. No extra/missing files.
Manifest is exactly `{version:1,scheme:"effective_stdlib_v1",registration:
"external_registration_v1",embedded:"__planar__/embedded.bin",assembly:
"__planar__/embedded.lua",profile:"profile/input.json",modules:LIST,catalog:C}`.
LIST is ordered `{path:"external/"..P,mode:420|493}`. C exactly
`{version:2,commands:LIST,shapes:LIST,projections:LIST}`; each entry exactly
`{descriptor:D,owner:OWNER}`. Catalog lists sort by ID/name, options/capabilities
sort by name; owners retain capture ordinals. Profile order must equal modules
and owner ordinals; shipped original order/assembly is verified with old manifest
rules. Canonical manifest has no effective digest field (no self-reference).
The complete tree's portable record_v1 Merkle digest is effective stdlib digest.
Verification reconstructs every byte/manifest/correspondence with **no Lua**;
callability or semantic honesty is not historically re-proved. Collision-safe
interning compares exact complete encoded records after digest lookup.

For an embedded-only acceptance in format2, use unchanged embedded provenance
and v1 interpretation/versions, request_v2 selector embedded. For profile
acceptance, use effective_stdlib_v1 tree and interpretation_v2/versions_v2 even
if no module changes semantics. `interpretation_v2` canonical value is exactly
`{version:2,bundle:B,semantic:S,versions:V,extensions:E}`. B is existing complete
normalized bundle except registered job.shape IDs permitted; S existing complete
semantic model except custom job models' selected_shape described in X-SHAPE.
E is exactly `{registration:"external_registration_v1",catalog:C,
selected_shapes:LIST}`; sorted LIST entries exactly `{job:ID,id:ID,revision:ID,
definition:DEFINITION}`, one for each custom-shape job. Validate B/S one-to-one sorted
job correspondence, no extra selected_shapes, manifest C equality and effective
digest equality natively before preparation. `DEFINITION` and S.selected_shape.definition are the exact same existing
41-byte H result from X-SHAPE: ASCII `xxh3_128:` followed by exactly 32 lowercase
hex digits. Native recomputation uses the verified catalog descriptor, logical
owner `{tier,module}` without its capture ordinal, and lowercase hex of the exact
original selected-module bytes under `shape_definition_v1`. Require exact byte
equality with both recorded fields; do not strip the tag or accept an untagged
alternative. This clarifies notation only and preserves H and all W recipes.
Historical validation is structural; no current shape function/schema policy runs.

V is exactly old versions keys/values except `interpretation:"interpretation_v2"`,
`declarations:"declarations_shapes_v1"`, `normalization:"normalization_shapes_v1"`,
`semantics:"dependencies_shapes_v1"`, `selection:"selection_shapes_v1"`, plus
`registration:"external_registration_v1"`, `provenance:"effective_stdlib_v1"`.
`stdlib_digest` is the raw 32-lowercase-hex complete effective tree digest, distinct
from the tagged shape definition above; `projection` remains
`projection_v2` as baseline; actual custom projection identities belong to render
manifest only. V is stored independently as canonical_v1 bytes (`versions_v2`
is this schema name, not a changed canonical codec). Embedded V in interpretation
must byte-equal separate V. All other old version values remain exact. The new
selection version records decoder support; it does not replace old W recipes.
A profile-only byte change changes exact provenance/digest and accepted revision,
not automatically W. Native cannot accept Lua-supplied encoded versions/digests,
records, source membership, outcome or receipts.

| Ledger / invocation | Reader and writer behavior after X gate |
| --- | --- |
| Format1, ordinary embedded load | Existing request_v1/provenance/interpretation/versions unchanged; old replay exact. |
| Format1, profile load | Derive request_v2 and look up ID: any saved v1 body conflicts; unknown ID refuses `unsupported_ledger_version` before profile read. Never insert v2 into v1 or upgrade. |
| Format2, ordinary embedded load | request_v2 embedded; old embedded provenance/interpretation/versions; replay exact. |
| Format2, profile load | request_v2 profile, effective_stdlib_v1 and interpretation_v2/versions_v2, exact early/locked replay. |
| New binary history/export, formats1/2 | Verify respective schema/record codecs; decode v1/v2 recorded interpretations and effective provenance without old/current Lua/profile. Export source only unchanged. |
| Old binary, format2 | Existing unsupported-version refusal, no journal/schema/data changes. |
| Unknown format/request/interpretation/manifest | Refuse typed unsupported format/codec/interpretation as appropriate; no best-effort parse or implicit conversion. Unsupported interpretation may still permit raw source export when record/tree format supported. |

New init creates format2 only at a **new explicit target** after X admission;
there is no public format selector or format1 creation path. Native test fixtures
retain explicit format1 creation for backward-reader checks. Publication and
no-overwrite rules and public init response stay old. Ordinary history/export never need a profile. Explicit envelope
history/export does validate its selected profile before handler dispatch, but
that profile does not reinterpret recorded semantics; use ordinary reads after
profile removal. Full format2 integrity verifies request-v2 selector privacy and
record correspondence. A future offline format conversion requires a separately
reviewed explicit backup/new-target tool; none is delivered here.

Fresh load performs all Lua validation/preparation outside lock, retaining exact
same-VM bundle provenance and all original receipts. Inside transaction: identity/
format recheck, exact request relookup (equal returns original bytes before CAS),
locked CAS, exact interning/equality/noop/revision/event/request, native receipt
verification then commit. No Lua/callback/error encoder under lock. Existing
native completion receipt, commit-uncertain, postcommit/early-replay/stdout
fallbacks and recovery-by-same-ID remain unchanged. A late equal replay never
reactivates a revision; current-head changes cannot defeat saved intent. An early
miss may still encounter a real profile error before a concurrent late replay.
