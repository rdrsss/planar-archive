# Trusted standard library

`bootstrap.lua` returns the fixed product export table. Its helpers and exports
are composed and embedded at configure/build time by `cmake/embed_lua.cmake`.
Editing any listed source regenerates the compiled source module.
The native runtime never opens Lua source paths or resolves `require`. R07-B1 supplies
schema exports; B2 adds graph/materiality and R08 adds projections. JSON captured as a
native string remains data for the trusted parser, not executable input.

`planar::engine::run_trusted(entry, input)` evaluates one export in a fresh state
and returns an independent immutable native value. The caller selects only a
name in the embedded export table. The bootstrap exposes generic `copy`,
`hash_value` (input `{domain, value}`), `file_digest` and `fragment_digest` exports.
Its input can contain declaration roots and captured file/registry/observation
bytes in ordinary structural records. Rich schema diagnostics are ordinary
returned records (including `field`, `file` and `details`); native failures retain
the small `diagnostic` boundary vocabulary.

Inside trusted code, booleans, signed integers and raw byte strings are primitive
Lua values. Records and workspace/spec/job tags have ordinary string-key fields;
lists have contiguous integer keys. Markdown and file nodes are tagged tables
`{value = <bytes>}`; immaterial nodes are tagged `{value = <md or file node>}`. Both native input validation and Lua return
conversion enforce that payload restriction.
Private tags live in a native registry map, never a reserved user key or exposed
metatable. `engine.kind(value)` returns `boolean`, `integer`, `string`, `object`,
`array`, `workspace`, `spec`, `job`, `md`, `file`, `immaterial`, or `unsupported`.
`engine.object(table)` and `engine.array(table)` tag a previously untyped plain
table, including empty containers; retyping an already typed table fails. Plain
untagged tables return as objects. Trusted Lua may mutate these copies; native
captured values remain unchanged. Preserve private wrappers by retaining their
typed table, or normalize to fresh ordinary objects under schema policy.

`engine.hash_value(domain, normalized_value)`, `engine.file_digest(bytes)` and
`engine.fragment_digest(bytes)` reuse the native canonical/hash primitives.
The first accepts normalized ordinary objects/arrays, not private declarations.
Raw strings can contain NUL/non-UTF-8; fragment validation and product text
validation apply at their respective boundaries. Record keys remain non-NUL
UTF-8. Conversion refuses nil values, noninteger numbers, functions, userdata,
threads, metatables, cycles, sparse/mixed lists and malformed private payloads.
Repeated acyclic tables are copied independently; there is no shared native
mutable state.

The trusted environment opens base/table/string/math/utf8 only. It removes
`load`, `loadfile`, `dofile`, `print`, `warn`, `collectgarbage`, `tostring`,
`setmetatable`, `xpcall`, `string.dump`, `string.format`, `string.find`,
`string.match`, `string.gmatch`, `string.gsub`, `string.rep`, `table.move`,
`table.insert`, `table.remove`, `table.sort`,
`math.random` and `math.randomseed`.
`string.format` and unrestricted `tostring` can expose object addresses. There is
no io/os/package/debug/coroutine library. Trusted code cannot create finalizers,
whose execution would bypass Lua instruction hooks. `pcall` uses a curated
native protected call with no user error handler; ordinary success/error arity,
argument forwarding and opaque error identity are preserved. Instruction
exhaustion is terminal before any protected call returns to trusted Lua, even
through nested calls. `xpcall` is unsupported because its error handlers can run
with Lua instruction hooks disabled. The host also checks terminal exhaustion
on both successful and failed protected returns. Product code must sort record
keys using explicit metered Lua when selecting ordered output; `pairs`/`next`
iteration order is not a product order.
No declaration state opens these libraries or receives `engine`.

Native Lua pattern matching can backtrack for unbounded practical time even on
small strings without invoking VM instruction hooks. All four pattern entry
points are absent, including string method access; `find` has no plain-search
exception. Future validators can use explicit Lua byte loops, which are metered.
Numeric-count native loops also bypass VM hooks: empty `string.rep` can iterate
without producing bytes, and `table.move` can copy nil entries over an arbitrary
range without growing a table. Both are absent, including string method `rep`.
Explicit Lua loops can perform required repetition and remain metered. Sparse
tables can also have computed lengths far larger than their stored contents.
Native `table.insert`, `table.remove` and `table.sort` can traverse those lengths
without proportional allocation or VM instructions; even a native sort comparator
need not execute Lua instructions. These helpers are absent. Future schema
sorting and shifting must use explicit metered Lua algorithms.
The remaining string operations (`byte`, `char`, `len`, `sub`, `lower`, `upper`,
`reverse`, `pack`, `unpack`, `packsize`) perform work bounded by their
input/output and binary format sizes, subject to the allocation ceiling. Native
C work is not counted as Lua instructions; the instruction limit is not a
wall-clock deadline or an instruction meter for native library calls. The table
library is constructed from the explicit allowlist `concat`, `create`, `pack`,
`unpack`. `concat` requires an actual string/number at each traversed index and
fails at the first missing or invalid entry; successful traversal needs populated
input. `create` allocates its requested capacity under the allocation limit.
`pack` is bounded by its argument stack; `unpack` checks result stack capacity
before iteration. UTF-8 traversal is bounded by the input string even when
an offset count is supplied. These are data/allocation bounds, not native VM
instruction metering.

Each evaluation limits Lua allocation and cumulative native conversion allocation
to 512 MiB each, Lua instructions to 100 million, conversion depth to 256 and
visited value nodes to four million. Limits cover conversion in hash callbacks
as well as final results; strings have no declaration-sized 64 KiB ceiling.
These bounds accommodate the captured 64 MiB byte inventory and its possible
128 MiB hex representation. Native validation checks inputs before protected
transfer. Lua allocation/setup/execution run inside protected calls. Output
conversion uses reserved stack and nonallocating raw API calls; registry
anchoring has its own protected call. Native callback temporaries unwind before
raising a Lua error or allocating a Lua result. Traversal membership is scoped to
an active conversion and unwinds on failure; resource counters remain cumulative.
Native hash callback failures raise opaque Lua-owned diagnostic tokens, so catching a
failure does not overwrite the identity of a later escaping failure. Retained
and rethrown tokens preserve their original diagnostic. Token metadata is charged
to Lua allocation and reclaimed by Lua GC; it has no native finalizer.

`tests/stdlib/bridge.lua` is embedded into a separate test-only module/library.
The production export table has no test exports and no source-injection API.

## Schema v1 exports (R07-B1)

The production chunk is composed, in order, from `json.lua`, `schema.lua`, `semantic.lua` and
`bootstrap.lua` using an explicit CMake source list. Helpers are lexical locals
in this one product chunk. There is no filesystem module lookup, `require`, or
request-selected source. The B2 semantic pass calls these lexical helpers; R08
calls the exports through `run_trusted`.

All three exports below return exactly `{value: <normalized value>}` on schema
success or `{error: {code,message,file,field,details}}` on refusal. Containers
are native-tagged arrays/objects (never inferred from emptiness). Diagnostics
carry validated portable logical paths, no host paths or invented line numbers.
Independent declaration/schema diagnostics are collected and sorted by logical
file bytes, field bytes, then code bytes; the first is returned. Resource
exhaustion stops immediately. Messages are explanatory; code/field/details are
the stable interface. Invalid request envelopes use file `""` and fields under
`route`, `json` or `bundle`. Native bridge/resource failures remain the separate
`std::expected` failure boundary.

- `parse_json({file, bytes})`: reusable non-executable strict JSON data parser.
  `file` is a portable logical file path; `bytes` is the retained byte string.
  It distinguishes arrays and objects, checks duplicate decoded object keys,
  UTF-8 scalar validity, escapes/surrogate pairs and non-NUL text, and accepts
  only signed 32-bit integers, booleans, strings, arrays and objects. Null,
  floats, leading zeroes, malformed syntax and trailing data are refused.
  Schema errors use field `$` with details `{reason,offset}`; offset is a
  one-based byte position (including end-of-input). Bounds are 1,048,576 input
  bytes, 128 nested container levels (including empty terminal containers) and
  100,000 value nodes. A scalar inside 128 containers is allowed; a 129th
  container is refused whether empty or populated. Resource details
  are `{limit: "json_bytes"|"json_depth"|"json_nodes", maximum}`. Schema-specific
  string bounds are applied after parsing, so the generic parser is reusable.
  Glaze performs the bounded structural read and complete strict validation;
  structural schema/resource refusals can precede a later syntax error. Library
  offsets identify the failing token or enclosing object and do not promise the
  previous parser's exact diagnostic position or reason wording.
- `route_workspace({file, workspace})`: `workspace` is a native private workspace
  constructor root, `file` its portable workspace-relative declaration path.
  Validates its exact schema, version, IDs, portable paths, unique jobs/files,
  reserved source-root IDs, lists, defaults and text constructors. It returns
  `{file,workspace,spec,jobs,root,registry}`. `spec` and each `jobs[].file` are
  resolved workspace-relative paths; `jobs[]` sorts by job ID byte order.
  `workspace` is the normalized workspace; `root` and `registry` repeat its
  metadata for capture routing. Notes with file text are descriptors `{kind:
  "file",path}` here; routing deliberately cannot validate uncaptured bytes.
  The driver must load **every** returned spec/job path and use `root.excluded`
  for source/base capture. This route is provisional until final normalization.
  Both routing and final normalization validate the resolved path after joining
  it to the declaring directory, before lookup or diagnostic use. An invalid join
  reports `path_denied` on the valid declaring file and original field index.
- `normalize_bundle({workspace,spec,jobs,registry,files})`: `workspace` and
  `spec` are `{file,value}` records, jobs is an array of `{file,value}` records,
  where values retain their private constructor tags. `registry` is
  `{file,bytes}` for the driver-selected trusted JSON input. `files` is an array
  of `{path,bytes}` for retained **workspace-root** entries from capture. These
  are captured values, never host filenames. Request fields are exact; retained
  file paths are unique. The caller must supply the retained declarations and
  bytes from one sealed native capture; this API does not authenticate a
  fabricated caller record or perform fresh reads.

The normalized bundle is `{workspace,spec,jobs,registry,workspace_file,spec_file,
registry_file}`. Each job remains `{file,value}` in job ID byte order. All listed
jobs must be supplied exactly once, no unlisted declarations are accepted, and
returned IDs and spec path must match the workspace. The trusted registry header
must match workspace registry ID/revision. Every declaration is schema-validated
before selection. Toolchain/check definitions use exact schemas, enum/bounds,
unique IDs/environment names and nonempty command executable/coverage rules.
B2 owns reference resolution and trusted check applicability/coverage policy.

Normalization removes private constructor tags. Text is an ordinary object
`{kind:"text",bytes}` for inline text or
`{kind:"text",bytes,file:{root:"workspace",path}}` for file text. File paths are resolved from
**their declaring Lua file's directory**, and contents come only from `files`;
text validation refuses missing, invalid UTF-8/NUL or over-1-MiB file bytes.
Markdown is non-NUL UTF-8 at most 65,536 bytes. Bytes, CRLFs, trailing whitespace
and path provenance are retained exactly. Goal bytes must be nonempty.
Immaterial wrappers are allowed only at notes/rationale, whose destination
fields remain immaterial regardless of the wrapper. Normalized facets add
`kind:"files"` and `normalizer:"exact_bytes_v1"`.

Optional collections become tagged empty arrays. Job `max_rounds` defaults to 2,
`budget_bytes` to 32768; role budget inherits the normalized job budget.
Question `blocks` defaults to `none`. Other optional values remain absent.
Set lists are unique and sorted in byte order by an explicit metered Lua merge
sort. ID-keyed record collections also sort by ID: workspace jobs and returned
bundle jobs; spec work, milestones, constraints, prescriptions, contracts, facets,
dependencies and questions; job roles; registry toolchains and checks. Acceptance
criteria retain declaration order despite requiring unique IDs. Outer pipeline
stages, command/argv and environment lists retain their order too (each stage's
role set is sorted). Uniqueness and sorting are independent schema properties.
Diagnostics retain original declaration indexes after sorting. B2 resolves
shared/cross-collection namespaces.
Intrinsic job rules enforce required/forbidden worker work/reviewer targets and
pipeline stages/fanout stage absence. B2 owns assignment completeness, reviewer
coverage/independence, ownership and aliases, references, effective checks,
dependency legality, full phase graph, W, identity recipes and selection.
R08 owns observation schemas/target applicability, Markdown and CLI integration.
No B1 result proves those deferred semantics, checks, completion or approval.

## Global semantic validation (R07-B2B)

The fixed product chunk now includes `semantic.lua` between `schema.lua` and
`bootstrap.lua`. `validate_bundle({bundle, capture})` first calls
`normalize_bundle(bundle)` with the exact raw B1 request above. It returns the
same `{error:{code,message,file,field,details}}` refusal vocabulary, or
`{value:<validated model>}`. No role/job selection occurs: every listed job must
satisfy the full policy. Native resource failures remain native failures.
Semantic diagnostic indexes refer to the original declaration lists, even
though their normalized records sort by ID. Independent diagnostics sort by
logical file, field and code; dependent subscription/applicability checks wait
for valid references. There are no host paths or invented line numbers.

`capture` is exactly one of:

- `{status:"not_captured"}`;
- `{status:"captured",files:[{root,path},...],directories:[{root,path},...]}`.

The latter lists are generic sealed-capture topology, including empty directories.
Roots are exactly `current:<workspace.root.id>` or `base:<workspace.root.id>`;
paths are portable file-path grammar without trailing `/`, including directories.
A root/path can occur only once across the two lists. Empty lists are explicit
captured emptiness, distinct from not capturing. The driver adapts native retained
file/directory records to these structural values. This envelope authenticates
neither a fabricated caller record nor its claimed sealing. It contains no
source bytes or file identities; B2C receives retained byte/mode records for
identity construction separately. No export performs filesystem access.
Directory topology serves validation only, never file provenance or identity
inputs. A captured directory cannot satisfy a file boundary/facet; a captured
file cannot be an ancestor of a declared path. Missing facets remain permitted
and unavailable; this pass establishes neither done nor accepted quality.

The validated model has exactly:

```
{
  bundle: <complete normalized B1 bundle>,
  capture: <validated topology state above>,
  dependencies: [
    {id,producer,consumer,source,ref,phase,quality}, ...
  ],
  jobs: [
    {
      id, file,
      assignments: [{work,role}, ...],
      reviewer: <reviewer role ID>,
      effective_checks: [{work,checks:[<check ID>,...]}, ...],
      pipeline_predecessors: [
        {id,source:"pipeline",producer,consumer,producer_role,consumer_role,
         phase:"start",quality:"done"}, ...
      ],
      graph: {
        vertices: [<vertex label>, ...],
        edges: [{from:<vertex label>,to:<vertex label>}, ...]
      }
    }, ...
  ]
}
```

Jobs/dependencies/pipeline records sort by ID; assignments/effective checks sort
by work ID, and each check set sorts by check ID. Graph vertices sort by label;
edges sort by `from`, then `to`, with duplicate edges removed. Capture topology
preserves caller list order; it is not a material identity value. All ordering
and graph traversal use metered Lua loops and the B1 merge sort. Cycle details
contain an actual deterministic DFS stack suffix followed by its first vertex
again, so every neighboring pair is a real directed edge. Graph labels and edges
are precisely those in declarations.md, including reviewer start-to-complete,
all worker-completion barriers, acceptance edges, explicit done/accepted unit
edges and complete-to-start adjacent pipeline cross-products. Available
subscriptions and questions add no scheduling edges. Grouped units keep their
unit obligations; no edge waits on their own worker role.

Validation resolves all references, shared constraint/prescription IDs, milestone
membership, trusted check/toolchain allowlists, contract conformance inputs and
both-side mandatory checks. Every consumer needs its start-contract dependency.
For each job, workers exclusively cover all units, one independent reviewer
covers all units with no added checks, and pipeline stages cover workers exactly
once. Effective checks include each unit's mandatory checks, every produced or
consumed contract's conformance check, and its owning worker's additions.
`output_vector` applicability is checked for each unit in both directions against
complete facet dependencies. Pipeline obligations imply no facet or extra check.
Ownership checks cover every pair of work/frozen/excluded boundaries (including
same-list/grouped pairs), portable aliases of shared path segments, facet
containment and both captured trees' topology. B2C can scope this complete model
and call native hashes; this model is neither W nor evidence of execution.

## Scoped material selection (R07-B2C)

The embedded chunk includes `selection.lua` after `semantic.lua`.
`select_role({bundle,capture,files,job,role})` returns `{value: selection}` or
`{error:{code,message,file,field,details}}`, with native resource failures still
using `std::expected`. `bundle` is the raw `normalize_bundle` input, `capture`
is the `validate_bundle` topology state, and job/role are logical IDs. It calls
`validate_bundle` on the complete bundle before selecting anything. An invalid
unselected job therefore refuses every selection.

`files` is the complete sealed retained inventory, each record **exactly**
`{root,path,mode,bytes}`. Mode is `0644` or `0755`; bytes is a raw string, including
binary source bytes. Roots are `workspace`, `registry`, optional `observations`,
`current:<source ID>` or `base:<source ID>`. Registry/observations paths must be
`input.json`. Paths are portable file paths; root/path pairs are unique and
portable segment aliases are refused. Workspace bytes must exactly match
`bundle.files`; all workspace/spec/job declarations and registry bytes must be
present. No extra workspace/registry bytes are accepted. Source/base entries
must correspond exactly to capture.files, with no excluded entries; no source
entry is allowed without capture. Directory topology remains separate and
never participates in identities or file provenance. The same 1 MiB input-file,
16 MiB source-file, 10,000-file and 64 MiB retained-byte limits apply before
hashing/hex expansion. Bounds are reported as `resource_limit` with details
`{limit:"file_bytes"|"files"|"input_bytes",maximum}`. Malformed fields are
schema/path refusals; mismatched inventories use `reference` and aliases or
excluded source files use `ownership`. These checks do not authenticate a
fabricated caller inventory: the driver must pass one sealed native capture.

The selection value has exactly the following required fields:

- `context`: `{workspace,spec,job,role,kind,work,budget_bytes,registry,optional}`.
  The first four fields are IDs; work is the sorted selected unit-ID array.
  Budget is the normalized role budget. Registry is
  `{id,revision,authority:"driver_selected_trusted_configuration"}`. Optional
  contains only present `workspace_notes`, `rationale`, `job_notes` and
  `preferences`; these values prepare projection without entering W.
- `work`: `{value:W,digest}`, using the exact render-contract W fields and
  native `work_v1` hash. All set ordering comes from schema normalization;
  scoped applicability, pipeline predecessor mappings, effective per-unit
  checks and selected exact registry definitions remain material. Observed
  external start facets enter W; own/grouped/reviewer target values and
  completion-only facet values do not.
- `base`, `observation`, `outputs`, `targets`, `files`, `prerequisites`:
  exactly the corresponding render-contract manifest data. Outputs cover all
  spec units, targets cover selected reviewer units only, and file provenance
  covers the entire inventory. Source/base prefixes occur only in provenance;
  identity tree entries use the unprefixed source ID. `files[].bytes_hex`
  retains exact bytes for collision checks and later projection. No observation
  file entry is invented when one was not supplied.
- `facets`: sorted `{id,status}` records for all facet declarations. Status is
  `not_captured`, `unavailable` or `available`. Available records additionally
  contain `{semantic,entries}`, with exact selected current tree entries.
  A facet is available only if every declared file exists; an absent file is
  never an available empty facet.

Only captured workers additionally receive `role_output`, the native role
output digest. A captured empty tree computes exact empty unit/role outputs
and marks units `present:false`; absence of capture emits no output digests.
Contract, base, observation, unit/role output and facet hashes all use the exact
named native H records in render-contract.md. Prerequisites preserve every
explicit, pipeline and blocking-question obligation. Files cannot establish
done or accepted quality; those prerequisites remain `unverified`, including
internal grouped edges. R08 will validate observation applicability and project
Markdown/CLI envelopes. This export performs no I/O or execution and establishes
no authenticated observation, completion or approval.

## Trusted render projection (R08-A)

`render({bundle,capture,files,job,role,budget_bytes?})` accepts precisely the raw
`select_role` request plus an optional integer byte override in `[1,1048576]`.
It calls the production `select_role` export first, retaining global bundle and
inventory validation. Native callers must pass the sealed inventory rather than
build a second observation source. The optional `observations:input.json` record
in `files` is the sole observation input; absence means the canonical empty
observations value and creates no provenance file. `render` performs no I/O.

Success is `{value:{version:1,kind:"render",packet:P,manifest:M}}`. P and M have
exactly the fields in `docs/vernacular/render-contract.md`; `role_output` appears
only for a captured worker. The containing `value` is the complete structural
success envelope for R08-B to serialize with native canonical JSON and append LF.
Failure is `{error:{code,message,file,field,details}}`; native bridge failures
remain `std::expected` diagnostics. No stdout or exit-code behavior is claimed by
this Lua export. Native transport/output-limit enforcement and CLI capture
composition belong to R08-B.

`observations.lua` parses strict JSON with the approved parser, validates schemas,
IDs, every target and check/vector globally, then filters visibility. Check
applicability includes owning-worker additions even for another selected role.
Targets and every output-vector member must match a present captured unit.
Vectors include the target and all explicit complete-dependency producers when
required by the selected check; other checks accept only their target. Workers
see own units plus explicitly subscribed producers; reviewers see their targets.
Checkpoints use selected unit boundaries independently of existence, with 32
unique paths, 2048-byte strings and an 8192-byte canonical JSON object limit before
adding their authority label. Checkpoint paths reject portable shared-segment
aliases against each other, declared source paths and captured files/directories.
Semantic observation errors name original JSON array indexes even when normalized
assertions, vector members or checkpoint paths sort differently. Assertions remain unverified, and checkpoints
remain session notes. Neither changes source work identity or prerequisite status.

`render.lua` projects exact requirement/source data, per-unit check applicability,
explicit/internal/pipeline prerequisites, truthful capture absence and empty
outputs. Workers include full old/new selected upstream facet contents as well
as owned files; reviewers include every target's contents and exact target
vector. Deleted files retain old contents. Text is fenced using a delimiter longer
than any embedded backtick run; ASCII and Unicode C1 control characters appear
visibly escaped except LF/CR/TAB, with the label recording display escaping.
Binary data uses lowercase hex with original byte count; content identities
remain in the manifest. `projection_v2` shows each check's complete policy as
labeled fields, with small Glaze arrays for exact argv, input kinds and environment.
Toolchains appear once with full descriptions. Dependencies appear once with
roles, phases, quality/status and facet paths/availability; pipeline and question
prerequisites remain explicit. Structured assertion/preference values stay in
Glaze fences. Registry descriptions are configuration data and are never executed.
Authority keeps the full work fingerprint. Global observation/base and file/facet
content digests stay in the manifest; outputs/checkpoints show capture status.
Reviewer outputs show the full ordered target vector once. W and all canonical
identity versions stay unchanged; only M appends `projection_v2`.

`presentation.lua` delegates JSON descriptors and checkpoint accounting to
`engine.json_encode`, the same pinned Glaze codec used for final transport.
Keys sort by unsigned UTF-8 bytes; integers use minimal signed decimal. Glaze
uses short control escapes where available and uppercase hexadecimal otherwise;
slash and non-ASCII UTF-8 remain unescaped. Display-control escaping in prose
and dynamic fence selection remain Lua presentation policy.

`engine.json_decode(bytes)` and `engine.json_encode(value)` return `(value,nil)`
or `(nil,diagnostic)` with ordinary Lua diagnostic tables. Decode syntax maps to
`schema`; `json_parse` adds logical file and `$`, and converts the library's
zero-based offset to one-based bytes. Declaration states have no codec access.
Codec work finishes before allocating Lua results, and state owns the pending
native tree across protected pushes. Failures do not reuse hash diagnostic tokens.
Repeated conversion retains cumulative native byte/node limits. Glaze per-call
allocations are separate from the Lua allocator and conversion accounting:
encode scratch is bounded to 96 MiB + 1 KiB, in addition to bounded structural
copies and the caller's tree. This is not a total process RSS limit.

Budget selection uses override, role, then normalized job defaults. All applicable
sections are built first, whole optional sections drop in checkpoint/job_notes/
workspace_notes/rationale order, and the exact omission notice is rebuilt each
time. Included section bytes include their actual preceding join LF; omitted
section bytes contain only their standalone headed section. Their included sum
matches the exact UTF-8 Markdown length. Mandatory overflow reports final required
section sizes including the notice. No mandatory paragraph or file is truncated.
