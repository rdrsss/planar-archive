# Authoring and driver-prompt contract v1

Status: exact contract text for RA01, written by round [V5-02-RA01-A](rounds/V5-02-RA01-A.md) and
pending records-only review. It specifies behaviour; it ships nothing. RA01-B implements it and RA01-C
gates it. Until those rounds land, the current commands remain `render`, `init`, `history`, `export` and
`load`.

Authority: the accepted decisions E1, E3, E4, E12 and E14 of the
[execution design](execution-design.md#decisions-table) (2026-10-28), Q10 B, Q11 A, Q12 A, Q14 A and Q18 A,
and the approved [render](render-contract.md), [command](command-contract.md),
[load](load-command-contract.md), [external profile](external-profile-contract.md) and
[ledger](ledger-contract.md) contracts. This document amends none of their v1 bodies except through the
pointer amendments listed in [Pointer amendments](#pointer-amendments). It narrows no clause of any approved
contract. It specifies no execution state, claim, status, check or R15 to R19 verb.

Design source (not authority): [execution-design.md](execution-design.md#ra01-authoring-and-driver-prompts),
clauses RA-1 to RA-9. Clause numbers below keep the design's numbering.

## Common rules

1. Every RA01 verb is one shipped family with one native ABI row. Options are separate tokens; scalar values
   are nonempty and do not start `--`; there are no defaults, aliases, positionals, repeats, short flags or
   flags. CLI11 owns parsing ([command-contract](command-contract.md#bootstrap-errors-and-retained-lifetime)).
2. No RA01 verb opens a ledger, writes any file, runs a check, starts a process, calls a model or watches a
   directory (Q10 B, Q14 A). Output is exactly one JSON document and LF on stdout, in the canonical JSON form
   of [render-contract](render-contract.md#canonical-encoding-and-hash-representation). There is no
   raw-Markdown mode.
3. Failures use the closed error envelope of
   [render-contract](render-contract.md#errors-and-verification-boundary) on stderr with its exit mapping:
   `usage` 2; `schema`, `reference`, `ownership`, `dependency_cycle`, `path_denied`, `capture_changed`,
   `resource_limit`, `unsupported_version`, `lua_syntax`, `capability_denied`, `budget_exceeded` 3; `io` and
   `internal` 4 (Q-G35 A mapping by code). Stdout is empty on failure.
4. No new public error code and no new message text is introduced (XD4 holds; E16 is not used by RA01). Every
   RA01 failure site uses an existing native or Lua message, mapped in [Messages](#messages). Every existing
   diagnostic message stays byte-identical.
5. Inputs are explicit host paths classified like the existing options: `--workspace` follows the
   [workspace selector](command-contract.md#workspace-selector-and-environment) (file or name); `--registry`,
   `--diagnostic` and `--from` are explicit paths, relative paths resolved only against the one cwd snapshot,
   lexical-normalised target, bounded by the existing 64 MiB `path_bytes` check before any filesystem access.
   `--diagnostic` resolves exactly like an explicit input file and `--from` exactly like an explicit root
   directory (the existing `resolve_explicit_file` and `resolve_explicit_directory` rules and messages). The
   workspace name and file classification and the `PLANAR_WORKSPACE_DIR` policy of the
   [workspace selector](command-contract.md#workspace-selector-and-environment) apply to `spec-prompt`,
   `critique`, `ingest` and `orchestrator` exactly as to render and load; `explain-rejection` and `observe` have
   no workspace slot and read no environment variable.
6. Determinism (A01 discipline): output bytes depend only on the captured bytes and the shipped stdlib. Every
   list is sorted as stated; every Lua table is walked through sorted helpers, never `pairs` order. Two fresh
   processes over the same inputs print byte-equal stdout.
7. Error precedence for every verb, first match wins: `usage`; registration of the sealed catalog; workspace
   selector and environment resolution (verbs with a workspace slot); resolution of the other explicit paths;
   capture or directory read; Lua parsing and semantic validation; output encoding bounds. A refusal earlier in
   the list is never replaced by a later one.
8. Resource accounting follows [command-contract](command-contract.md#bootstrap-errors-and-retained-lifetime):
   registration limits, the 512 MiB live ceiling, the 100-million-instruction invocation budget and the
   native conversion limits apply unchanged to every RA01 invocation.

## RA-1 Catalog, rows, options and capabilities

After RA01 the catalog has **eleven mandatory rows**: the five existing rows plus these six. A missing, extra,
duplicated or mispaired row rejects the whole catalog as `internal` exit 4, exactly as the load amendment
specifies for five. `response_kind` has eleven values; each maps to the envelope kind shown. The five prompt kinds print the
envelope of RA-3; `observe` prints the bare observations document of RA-6 (no envelope).

| name / family | handler_id | response_kind | envelope kind | exact capabilities |
| --- | --- | --- | --- | --- |
| spec-prompt / spec-prompt | `shipped.spec_prompt.v1` | `spec_prompt_v1` | `prompt` | `authoring_capture_v1` |
| critique / critique | `shipped.critique.v1` | `critique_v1` | `prompt` | `authoring_capture_v1` |
| ingest / ingest | `shipped.ingest.v1` | `ingest_v1` | `prompt` | `authoring_capture_v1` |
| explain-rejection / explain-rejection | `shipped.explain_rejection.v1` | `explain_rejection_v1` | `prompt` | `diagnostic_read_v1` |
| observe / observe | `shipped.observe.v1` | `observe_v1` | none (bare observations v1) | `observation_directory_v1` |
| orchestrator / orchestrator | `shipped.orchestrator.v1` | `orchestrator_v1` | `prompt` | `orchestrator_capture_v1` |

Handler IDs replace the hyphen of the verb name with an underscore because the handler-ID grammar of the
command contract has no hyphen; command and family names keep it (the name grammar allows it). Handlers
are registered once each by embedded `planar/commands.lua`, retained and sealed in the same bundle as the
other callables. Existing pairing, collision and capability rules apply.

Option signatures (`!` required; all options are type `text`):

| Family | Options |
| --- | --- |
| spec-prompt | `--workspace!`, `--registry!` |
| critique | `--workspace!`, `--registry!` |
| ingest | `--workspace!`, `--registry!` |
| explain-rejection | `--diagnostic!` |
| observe | `--from!` |
| orchestrator | `--workspace!`, `--registry!`, `--job!` (database-free form) |

`--job` is a declaration ID (the declaration ID grammar of [declarations](declarations.md)); other validation
is as stated in this document. The orchestrator's database-free form requires all three options together;
omitting any is `usage`. R15 later adds a ledger form by its own contract amendment and then exactly one form
is accepted; RA01 specifies none of it.

Invocation values: options cross into Lua as the existing `{version:1,command,options,present}` value.
`--workspace`, `--registry`, `--diagnostic` and `--from` are opaque native slots (see
[Pointer amendments](#pointer-amendments)); `--job` is a text string.

Root help (exact). Sole `--help` prints the current fixed bytes with these six lines inserted immediately
before the final line `planar --help\n`, in this order:

```text
planar spec-prompt --workspace FILE_OR_NAME --registry FILE
planar critique --workspace FILE_OR_NAME --registry FILE
planar ingest --workspace FILE_OR_NAME --registry FILE
planar explain-rejection --diagnostic FILE
planar observe --from DIRECTORY
planar orchestrator --workspace FILE_OR_NAME --registry FILE --job ID
```

Every other help byte, the `with-profile` help bytes and parse/usage precedence are unchanged. The
root-help rule is amended in the three contracts that fix it (see Pointer amendments). Until the integrated
RA01 gate, the five-row validator and five-family help stay coherent, exactly as the load amendment staged its
own admission; intermediate runtime support may use test-only eleven-row bundles and must not expose a
partially working public verb.

## RA-2 Authoring capture service

Services for the three authoring verbs, each callable once in order: `open_workspace({workspace:slot})`, then
`capture_authoring({route:R,include:INC})`. `open_workspace` and the normalised route `R` are exactly as for
render ([command-contract](command-contract.md#invocation-values-and-finite-native-service-protocol)); `INC` is
`"workspace"` or `"spec"` and is chosen by the verb: `spec-prompt` uses `"workspace"`; `critique` and
`ingest` use `"spec"`. The registry root is `--registry` as for render.

- `include:"workspace"` captures workspace.lua, every text file it references (declaration references),
  and the registry.
- `include:"spec"` captures all of the above plus the spec file and the text files the spec references.
- Neither value captures any job file, so a job file listed in the workspace but not yet written is not an
  error during authoring. A job file that is present is not read.
- Same confinement, receipts, stability verification, hard limits and refusals as render capture
  ([render-contract](render-contract.md#capture-boundary)): no-follow leaves, alias and special-file refusal,
  re-enumeration and re-read before projection, `capture_changed` on a detected difference, no retry, no
  downstream filesystem read. A required file that is missing is `io`.
- Result `V` of `capture_authoring`: exactly `{bundle:B,capture:C,files:[F...]}`, the render capture value
  ([command-contract](command-contract.md#invocation-values-and-finite-native-service-protocol) render row)
  restricted to the captured set, with no `job`, `role` or `budget_bytes`. No source or base tree and no
  observations file is captured.

Lua then runs the current workspace validation and, for `"spec"`, the current spec validation (schema, IDs,
references, ownership, phase-graph checks that do not need a job) against the captured bytes and registry.
A failure is the same typed error, code, field and exit that `load` or `render` would give for the same bytes
(for example `schema` exit 3, `reference` exit 3), provided every listed job file exists and is valid: `load`
also captures and validates every job and reports the first sorted error, whereas authoring never reads a job,
so a job-file error that `load` would report first is not reported here. `spec-prompt` validates only the
workspace and registry.
A prompt is produced only for inputs that pass; `explain-rejection` is the surface for failures.

The orchestrator uses its own capture service `capture_orchestrator({route:R,job:ID})`, called after
`open_workspace`. It captures what `include:"spec"` captures plus exactly one job file: the file the workspace
lists for `ID`. An `ID` that the workspace does not list is `reference` exit 3, field `selection.job`, message
`Unknown selected job` (the existing message of render's unlisted `--job`, `selection.lua`). That job file is
evaluated and validated as `load` would, so a malformed job gives the same typed error. Other listed job files
are never read. Result `V`: exactly `{bundle:B,capture:C,files:[F...],job:ID}`, where `bundle.jobs` holds only the
selected job and there is no `role` or `budget_bytes`.

## RA-3 Prompt envelope

Stdout of the five prompt-producing verbs is exactly:

```json
{"version":1,"kind":"prompt","prompt":{"kind":K,"markdown":M,"bytes":N,"digest":D,"inputs":I}}
```

(shown in the logical order; actual key order is the canonical sorted order.)

| Field | Rule |
| --- | --- |
| `kind` | `spec` (spec-prompt), `critique` (critique), `job` (ingest), `rejection` (explain-rejection), `orchestrator` (orchestrator) |
| `markdown` | UTF-8 text without NUL, at most 1,048,576 bytes; newlines are LF; ends with exactly one LF |
| `bytes` | integer byte length of `markdown` |
| `digest` | fragment hash of the exact Markdown bytes: `H("fragment_v1", markdown_UTF8_bytes)` in the `xxh3_128:` representation of [render-contract](render-contract.md#canonical-encoding-and-hash-representation) |
| `inputs` | `{files:[F...],registry:{id,revision}}`; `rejection` uses the form stated below |

`files` lists the captured file entries sorted by `(root,path)` in unsigned byte order, each exactly the
render file entry without `bytes_hex`: `{root,path,kind:"file",mode,size,digest}`. `registry` is the registry's
`id` and `revision` strings. For `rejection` there is no capture: `inputs` is exactly
`{files:[],diagnostic:{bytes:<size>,digest:<file hash>}}` where `digest` is `H("file_bytes_v1", file_bytes)`;
the `registry` key is absent. Markdown over 1,048,576 bytes is `resource_limit`, limit `prompt_bytes`, maximum
1048576. The envelope is within the 16 MiB output ceiling.

## RA-4 Prompt content

Templates are trusted Lua in embedded `planar/prompts.lua`, rendered with the existing presentation helpers
(`src/stdlib/planar/presentation.lua`). Data blocks (captured text and the `code`, `message`, `field` and `file` values of the `error` section) use
the presentation helper's labelled-data rendering (`presentation_data`) so control characters are escaped and
cannot be read as instructions. The one exception is the `raw_error` block, which is verbatim and unescaped
under the exact rule of [RA-5](#ra-5-rejection-prompt-e4-a).

Document structure. Each prompt is a sequence of sections in the exact order below. A section is
`## <Heading>` + LF + body + LF, sections separated by one blank line, and the document starts with a single
`# <Title>` line. Section IDs are for the contract and tests; headings are fixed strings owned by
`prompts.lua`. Tests assert section presence, order and the stated generated content, not the prose.

| Prompt kind | Sections in order |
| --- | --- |
| `spec` | `authority`, `task`, `workspace`, `spec_schema`, `checks`, `shapes`, `critique_step` |
| `critique` | `authority`, `task`, `workspace`, `spec_schema`, `checks`, `findings` |
| `job` | `authority`, `task`, `workspace`, `spec_summary`, `job_schema`, `checks`, `shapes`, `constructors` |
| `rejection` | `authority`, `task`, `error`, `schema_excerpt`, `raw_error` |
| `orchestrator` | rendered by the orchestrator projection; see RA-7 |

Generated content (each rule is exact):

1. **Schema text** (`spec_schema`, `job_schema`, `schema_excerpt`) is generated from the schema tables of
   `src/stdlib/planar/schema.lua` (the `spec`, `job` and `workspace` record tables), never hand-copied, so
   the prompt and the validator cannot drift. A record renders one line per field, fields sorted by name in
   unsigned byte order, as `- NAME (required|optional[, default D]): TYPE`; nested record and list-of-record
   fields render their own fields beneath, indented two spaces per level. TYPE is rendered from the table
   (`id`, `string`, `string MAX` for a bounded string primitive such as `preferences.model` (MAX bytes),
   `version` for the `version` primitive (the integer `1`), `text`, `note`, `path`, `boundary`, `scope`,
   `number MIN..MAX`, `enum a|b`, `set of T`, `list of T`, `record`; any other primitive name renders as that
   name from the table). Enum values and set members are listed in the table's own order, which is fixed
   by the table; a table with unspecified order is sorted. Mutating one field of a table changes the prompt.
2. **Check IDs** (`checks`) are exactly the workspace `checks` allow-list intersected with the registry's
   check IDs, sorted by unsigned byte order. Each lists the registry record's `coverage`, `inputs` (sorted) and
   `selection`, and its toolchain ID. An allow-listed check absent from the registry, and a registry check not
   allow-listed, are omitted without comment. An empty intersection renders the fixed line
   `No checks are available.`
3. **Shape catalog** (`shapes`) is the embedded shapes `pipeline` and `fanout_review` (Q12 A), in that order,
   with the exact job rules of [declarations](declarations.md) (stages required or forbidden, exactly one
   reviewer targeting all work once, at least one worker, `max_rounds`, budget fields). RA01 prompts do not
   list external shapes (see RA-9).
4. **Workspace and spec facts** (`workspace`, `spec_summary`): workspace ID, spec path, jobs list, registry ID
   and revision; `spec_summary` lists spec ID, goal, work unit IDs with their `checks` and `paths` sorted by
   ID, and prescription, contract, facet and dependency IDs sorted by ID. No file bytes beyond these fields
   are embedded.
5. **Spec prompt tail** (`critique_step`) is the last section of the `spec` prompt and its last body line is
   exactly: `Run planar critique --workspace <workspace> --registry <registry> and resolve every finding before ingesting the spec.`
   (literal placeholders; the prompt never contains a host path).
6. **Critique findings** (`findings`) are computed mechanically by trusted Lua from the validated spec and
   registry. Exactly three rules exist; each yields zero or more findings `{rule,subject,text}`:
   - `prescription_unchecked`: a prescription whose scope (a work-ID set, or `"global"` meaning every work
     unit) contains no work unit that has a check, among the unit's own `checks` and the conformance checks of
     contracts it produces or consumes, whose registry `selection` is `nonempty`. Subject: prescription ID.
   - `blocking_question_unanswered`: a spec question with `blocks` equal to `start` or `complete` and no
     `answer`. Subject: question ID. A valid spec can trigger it: validation accepts an unanswered scoped
     question ([declarations](declarations.md) question rule: it reports the selected phase as blocked at
     render time).
   - `facet_dependency_without_output_vector`: a dependency with `source:"facet"` and `phase:"complete"`
     whose consumer unit has no check, among its own and its contract conformance checks, whose registry
     `inputs` contain `output_vector`. Subject: the dependency ID.

   The design's example of a contract consumer without a `start/contract/available` dependency
   (`semantic.lua` consumer check, [declarations](declarations.md)) is not a critique rule: spec validation
   refuses it before critique can run, so a rule on it could never fire. `facet_dependency_without_output_vector`
   is not that dead case: the hard check of the same condition runs only per job and counts role checks, and
   authoring reads no job, so a valid spec can trigger the spec-only advisory.

   Findings are sorted by `(rule, subject)` in unsigned byte order and listed one per line as
   `- RULE SUBJECT: TEXT`; no findings renders the fixed line `No mechanical findings.` The `text` strings are
   fixed per rule and contain only the subject. Critique makes no model judgement and is advisory: it exits 0
   with findings; it never refuses on a finding.
7. **Constructors** (`constructors`, job prompt): the names of the `planar` job-tier constructors
   usable in a job file, sorted, and the statement that the job tier has no standard-library globals
   ([render-contract](render-contract.md#capture-boundary) job-tier text).

Ingest produces a prompt describing how to author job files for the validated spec; it does not read or
create a job file.

## RA-5 Rejection prompt (E4 A)

`explain-rejection --diagnostic FILE` is pure: no workspace, registry, ledger or load invocation. One service
`read_diagnostic({})` returns `V = {bytes:S,size:N,digest:D}`: `S` the exact file bytes as one Lua string, `N`
their length and `D` the `H("file_bytes_v1", S)` hash. The leaf is opened no-follow, regular file only, read once
in full and re-checked for size and identity (`capture_changed` on a difference), at most 65,536 bytes. A larger
file is `resource_limit` limit `diagnostic_bytes`, maximum 65536 (checked from the descriptor size before
reading and again on the bytes read). Native refusals use the existing messages of [Messages](#messages): a
missing or unresolvable path is `io` exit 4, a symbolic link `path_denied`, a non-regular file `io`.

Strict validation by Lua, any violation is `schema` exit 3 with `file` absent and `field` starting `diagnostic`
(the message is the existing one the shared JSON parser or schema normalizer gives):

- the file is one UTF-8 JSON value parsed by the same strict parser as other JSON inputs: duplicate keys, invalid
  UTF-8, NUL and trailing non-whitespace data are refused, and JSON whitespace before and after the value is
  accepted exactly as the shared parser accepts it; unknown fields, nulls and non-integers are
  refused by the envelope below;
- exactly `{version:1,kind:"error",error:E}`, `E` with required string `code` and `message`, required string
  `field` (empty allowed), optional string `file`, optional positive integer `line`, optional object
  `details` (closed JSON values only);
- `code` is any string matching `[a-z][a-z0-9_]{0,63}` (the code grammar). The set is deliberately not
  enumerated: it covers every render, ledger and load code, the external-profile codes `path` and `unsafe_path`
  that `with-profile load` can emit ([X-LIMIT](external-profile-contract.md#x-limit--cumulative-ceilings-and-errors)),
  and codes added by later rounds. A code outside the grammar is refused.

The file need not be canonical JSON; it is embedded as read. Bytes are never normalised, so what is embedded is
what the producer printed.

The prompt (`kind:"rejection"`) has these generated parts: the section `error` names `code`, `message`, and
`field`, `file` and `line` when present (through `presentation_data`); `schema_excerpt` holds the schema text
(RA-4 rule 1) for the field path; `raw_error` embeds the diagnostic by this exact rule, **verbatim and
unescaped**.

Raw-error embedding rule (code-decided from the fence rule of `presentation_data`,
`src/stdlib/planar/presentation.lua`, minus its escaping). The section body is one fixed label line, then a
fence line of backticks, then the file bytes, then a closing fence line. The fence is `max(3, L + 1)` backticks,
where `L` is the longest run of consecutive backticks in the file bytes. The file bytes are written without any
escaping or rewriting, so DEL, C1 and other control bytes that canonical JSON can carry appear raw. If the file
does not end with LF, one LF is added before the closing fence. Equality is defined exactly: the bytes between
the LF that ends the opening fence line and the closing fence line equal the file bytes (with the one added LF
when the file lacked a final LF). A rejected load's stderr always ends with LF, so for A36 the embedded block is
byte-equal to load's stderr. The file is valid UTF-8 without NUL (the parser guarantees it), so the Markdown is
valid UTF-8. The rest of the prompt is escaped as RA-4 states. A reader of the prompt should treat the block as
untrusted data; its raw control bytes are a recorded limit of this choice.

Field path resolution: split `field` on `.`; drop every `[N]` index suffix and descend into the list item. The
first segment names the schema and must be `workspace`, `spec` or `job` (the root of every path the validator
emits, for example `workspace.root.id` or `job.roles[1].work`); `registry`, `observations`, `bundle`, `$`, empty
and any other first segment select no schema. The remaining segments descend from that schema's record: the path
resolves when each segment names a field of the current record. A path of only the schema name resolves to the
complete schema text of that record. The excerpt is the resolved field's line with its subtree, in the RA-4
rule 1 format. When no schema resolves the path, `schema_excerpt` holds the fixed line
`No schema excerpt applies to this field.` A diagnostic never changes any `load` byte or exit code.

## RA-6 Observe (Q11 A, E12 A)

`observe --from DIR` reads one explicit directory and prints canonical observations v1. One service
`read_observation_directory({})`; capability `observation_directory_v1`. Native owns the mechanism; trusted Lua
validates and assembles. `DIR` resolves like an explicit root directory: a missing or unresolvable path is `io`
exit 4, a path that is not a directory is `io`, and a symbolic link to the directory is `path_denied`, each with
the existing message of [Messages](#messages). The directory is untrusted input (XD2). `observation_directory_v1`
is shipped-only; no external verb can request it, and extending it to `with-profile` is deferred (RA-9).

Directory layout (E12 A). DIR may contain only: optional regular file `checkpoint.json` (one checkpoint object),
optional directory `checks` holding regular files `*.json` (one check assertion each) and optional directory
`findings` holding regular files `*.json` (one finding each). A missing `checks` or `findings` directory is
empty. Names starting with `.` are ignored at both levels (not opened, not inspected) but count toward the entry
bound. Any other name (a subdirectory other than `checks` and `findings`, an extra top-level file, a file under
`checks` or `findings` not ending `.json`, a name that is not valid UTF-8, contains a control byte or fails the
portable file-name grammar of capture) is `path_denied`, and the error carries no `file` because the raw name is
never echoed. A `checks` or `findings` entry that is not a directory, a `checkpoint.json` that is not a regular
file, a symlink, a FIFO, socket, device or other special file is `path_denied`. Two listed entries with the same
device and inode (a hard-link alias) are `path_denied`.

Mechanism, in order: (1) open DIR as a retained no-follow directory (parent physically resolved like other
explicit paths; the leaf is not followed); (2) list the entries of DIR and then of each of `checks` and
`findings` once, bounded, sorted by unsigned name bytes; (3) read each regular file through no-follow
relative opens, with before/after metadata checks and receipts; (4) re-list and re-read everything, and
require the same membership, identities and bytes (`capture_changed` on any difference, no retry); (5) pass
the bytes to trusted Lua. A file or entry replaced, added or removed between listing and the final recheck is
`capture_changed`. Nothing is read after step 4, and observe never writes.

Result `V` of `read_observation_directory({})`: `{entries:[E...]}` with `E` exactly
`{category:C,name:N,size:Z,digest:D,bytes:S}`, `C` one of `checkpoint`, `check`, `finding`; `N` the logical
relative name (`checkpoint.json`, `checks/a.json`, `findings/b.json`); `S` the exact file bytes as a Lua string,
`Z` their length and `D` the `H("file_bytes_v1", S)` hash. Entries are sorted by category in the order
`checkpoint`, `check`, `finding`, then by `N` in unsigned byte order. Ignored dot names are absent. Native never
parses the JSON; Lua does.

Bounds (each refusal is `resource_limit` exit 3 with `{limit,maximum}` and the existing native message; these
are the numbers of design EX-8): at most 1024 listed names in total across the root, `checks` and `findings`
listings, counting dot names and subdirectories (`observation_entries`, 1024); at most 65,536 bytes per file
(`observation_file_bytes`, 65536); at most 4,194,304 bytes read in total (`observation_total_bytes`, 4194304);
and the canonical output observations value at most 1,048,576 bytes, the render observations input limit
(`observation_bytes`, 1048576). `observation_bytes` is the single name of the render-compatible output limit; the
total-read limit has its own name `observation_total_bytes`. Limits are checked before the allocation that would
exceed them, and an unsafe entry takes precedence over a later size limit. The output limit is enforced by the
native encoder before printing.

Assembly by Lua. Each file is parsed by the strict JSON parser and then validated with the structural rules of
render's [observations validator](render-contract.md#observations-json-v1) (no duplicate keys, unknown fields,
nulls, non-integers, invalid UTF-8 or NUL). `checks/*.json` is exactly a check assertion object without wrappers
(`id`, `check`, `target`, `reported`, `summary`, `vector`); `findings/*.json` exactly a finding object
(`id`, `target`, `severity`, `text`); `checkpoint.json` exactly a checkpoint object (`progress`, `next`,
`unresolved`, `files`). Codes, messages and details are those render's validator gives for the same bytes:
structural violations are `schema` with the message the shared normalizer gives; a target digest that is not a
well-formed `xxh3_128:` digest is `schema` `Malformed output digest` (a structural check observe applies, field
`target.output` or `vector[N].output`); a duplicate check `id` among checks, or finding `id` among findings, is
`reference` `Duplicate value` (the later file in name order is refused, field `id`); checkpoint bound violations
are `resource_limit` with details `{limit:"checkpoint_files",maximum:32}` and `{limit:"checkpoint_bytes",maximum:8192}`
and the messages `Too many checkpoint paths` and `Checkpoint canonical JSON too large`. For every such
error, `file` is the relative logical name (`checks/a.json`) and `field` the path within that one object, that is
the render field path with its `observations.`, `checks[N]`, `findings[N]` or `checkpoint` prefix removed (`$`
for a failure of the root value). Validation visits files in unsigned name order, checks then findings then the
checkpoint.

Observe does not apply the semantic rules that need a render selection or a captured output tree: target
existence and match (`observation_target_mismatch`), check applicability to the target, the expected output
vector, and checkpoint path ownership. Those stay with `render --observations`. Observe therefore refuses exactly
the structural refusals of observations v1 plus the filesystem cases above, no more and no fewer.

Output: stdout is the bare canonical observations value `O` followed by LF, with no wrapper or envelope key:
`{version:1,checks:[...],findings:[...]}` plus `checkpoint` only when `checkpoint.json` exists; `checks` and
`findings` are sorted by `id` in unsigned byte order. An empty directory gives
`{"version":1,"checks":[],"findings":[]}`. Output is independent of file creation order and of the filesystem's
listing order. Because the output is the document `render --observations` reads, it is structurally accepted
there (design EX-8: observe never emits a file that structural validation of `--observations` would refuse);
render may still refuse its targets. Items remain driver assertions: observe never verifies a target and never
matches a target against current output. The driver writes `O` itself and passes the file to render. Observe
writes no file.

## RA-7 Orchestrator projection (Q12 A)

`orchestrator --workspace W --registry R --job ID` prints a prompt (`kind:"orchestrator"`) rendered by a
reserved projection with ID `orchestrator`, ABI `projection_v1` and input `orchestrator_context_v1`. It is
graph-only and never receives worker facets, outputs or observations. Services: `open_workspace`, then
`capture_orchestrator` (RA-2). The embedded shapes only (RA-9).

`orchestrator_context_v1` is exactly:

| Key | Value |
| --- | --- |
| `workspace` | `{id}` workspace ID |
| `spec` | `{id}` spec ID |
| `job` | `{id}` job ID |
| `shape` | `{kind,max_rounds,stages}`; `stages` is the stage list (array of arrays of worker role IDs, order preserved) for `pipeline` and `[]` for `fanout_review` |
| `roles` | array of `{id,kind,work}` for workers and `{id,kind,targets}` for the reviewer, sorted by `id`; `work`/`targets` are sorted work-ID arrays |
| `graph` | `{vertices,edges}` of the declaration [phase graph](declarations.md): `vertices` the sorted label array, `edges` the sorted array of `{from,to}` (by `from` then `to`, unsigned bytes), including derived pipeline edges |
| `prescriptions` | the spec prescriptions that drive this job: scope `"global"` or intersecting the job's work units, sorted by ID, each `{id,scope,text}` |
| `actions` | array of `{command,status,purpose}` sorted by `command` |

`actions` has one entry per name in the union of (a) the sealed catalog's command names for this invocation
and (b) the future list below. `status` is `available` exactly when the name is in the invocation's sealed
catalog (native passes the sealed command names), otherwise `future`. `purpose` is a fixed one-sentence UTF-8
string of at most 200 bytes owned by `orchestrator.lua`. The future list is exactly the verbs of the E1
[verb table](execution-design.md#proposed-verb-surface-subject-to-e1) not shipped by RA01:
`activate`, `status`, `preview`, `claim`, `takeover`, `refresh`, `checkpoint`, `release`, `recover`, `check`,
`check-recover`, `verdict`. A name in both the catalog and the list is `available`. Each later execution round
updates this list in its own brief. `with-profile` is never an action.

Sections. The projection returns `{version:1,sections}` with sections in this order (all `mandatory:true`):
`orchestrator_authority`, `orchestrator_graph`, `orchestrator_roles`, `orchestrator_prescriptions`,
`orchestrator_actions`. The prompt Markdown is the headed join of these sections under one `#` title with the
current presentation convention. The text states that the driver schedules sessions and that Planar does not,
lists legal available actions and names the future actions as unavailable. It states no execution state.

Reserved identifiers (see Pointer amendments): projection ID `orchestrator` becomes shipped-owned (with
`worker`, `reviewer` and `recovery`) so no external registration can replace it, and the five section IDs
above join the reserved built-in section-ID union for both external input kinds. `orchestrator_context_v1` is
not an admissible external projection `input`; the external input set stays `role_context_v1` and
`history_snapshot_v1`.

## RA-8 Init coexistence

`init` is unchanged: its signature, bytes, diagnostics, exit codes and no-overwrite rules
([ledger contract](ledger-contract.md#proposed-command-surface)). It creates no scaffold, prompt, Git state or
checkout. None of the RA01 verbs reads, creates or overwrites a ledger. `load`, `render`, `history` and
`export` bytes, diagnostics and exit codes are unchanged (the root help bytes change as RA-1 states, the only
byte change visible to existing commands).

## RA-9 Profiles (E3 A)

RA01 ships the six verbs embedded-only. The `with-profile` mechanics are decided here (NEW engineering, E1 A and
E3 A basis, for the contract reviewer to accept):

1. **Probe membership.** The embedded probe of
   [X-CLI step 3](external-profile-contract.md#x-cli--envelope-and-admission-order) holds only the five
   X08-admitted families (`render`, `init`, `history`, `export`, `load`). The six RA01 families are not probe
   members, so `with-profile --stdlib-profile FILE COMMAND ...` with `COMMAND` one of the six is an opaque
   extension tail, never a shipped-signature failure. A malformed shipped-looking tail therefore does not
   short-circuit to usage before profile capture.
2. **Precedence.** Profile capture, admission and registration run first and keep their own errors, which take
   precedence. After a successful admission, a tail whose command is not in the sealed catalog is `usage` exit 2
   with the existing closed message `Expected command options or --help; see --help`. So `with-profile P observe
   ...` is `usage` after admission, and an invalid profile gives its profile error instead.
3. **Collision rule.** The shipped-name collision rule of X-REG covers all eleven shipped command, family and
   handler names: an external profile that registers a command or family named `spec-prompt`, `critique`,
   `ingest`, `explain-rejection`, `observe` or `orchestrator`, a handler ID of a shipped RA01 row, the projection
   ID `orchestrator`, or any of the five `orchestrator_*` section IDs of RA-7 is refused by the existing
   registration collision error (`reference`). This is how `with-profile` plus an RA01 verb stays `usage`: no
   external command can answer to those names.
4. **Catalog and completeness.** The effective sealed catalog under `with-profile` is the five shipped rows plus
   the external rows, and its completeness rule stays five. The embedded rule is eleven. RA01 does not change the
   with-profile catalog.
5. **Family and variant.** The `command_options` variant of
   [x07-addendum](x07-addendum.md#13-family-registry) stays closed at five and keeps describing the with-profile
   factory. The six RA01 families form an embedded-only set outside that variant. The architecture revision,
   the admission rows for the six families, the `docs/architecture.md` update and the reviewed amendment of
   x07-addendum section 1.3 that these need are owned by RA01-B wave 1; this contract edits none of them.
6. **Capacity and namespace consequences (decided by E1 and Q12 A).** X-LIMIT counts 64 commands and 64
   projections including shipped. The with-profile catalog holds five shipped commands and no embedded-only
   RA01 command or projection, so external capacity is unchanged. The namespace shrinks: the six command and
   family names, the projection ID `orchestrator`, the five `orchestrator_*` section IDs and the six shipped
   handler IDs become unavailable to external catalogs, so a profile that registers any of them is valid today
   and refused after RA01. The X-LIMIT table itself is unedited.

R15 widens the closed factory to admit these verbs under `with-profile` only by its own reviewed amendment, and
that amendment decides which reads are exempt from the later binding check. When a later round admits them under
`with-profile`, the prompts' shape catalog becomes the sealed catalog's shapes, external ones included. That is
the narrowing-risk row of [remaining-steps-design.md](remaining-steps-design.md) and must appear in that round's
invariant table.

## Messages

No new public code and no new message. Existing messages are unchanged and XD4 holds. Every RA01 site uses an
existing message, byte-identical, by this mapping (details are the existing `resource_limit` `{limit,maximum}`
values, `limit` names being details values, not codes):

| Site | Code / existing message |
| --- | --- |
| `--diagnostic` missing or unresolvable | `io` / `Cannot resolve explicit input path` |
| `--diagnostic` is a symbolic link | `path_denied` / `Explicit input file is a symbolic link` |
| `--diagnostic` not a regular file | `io` / `Explicit input must be a regular file` |
| `--from` missing or unresolvable | `io` / `Cannot resolve explicit root directory` |
| `--from` not a directory | `io` / `Explicit root must be a directory` |
| `--from` is a symbolic link target | `path_denied` / `Explicit root is a symbolic link target` |
| Observation entry symlink, FIFO, socket, device or other special file | `path_denied` / `Links and special files forbidden` |
| Observation entry with a disallowed name or kind (layout violation, bad name, `checks` or `findings` not a directory, `checkpoint.json` not a file) | `path_denied` / `Invalid captured path` |
| Observation hard-link alias | `path_denied` / `Hard-link input alias` |
| Observation directory membership or identity changed | `capture_changed` / `Directory membership or identity changed` |
| Diagnostic or observation file changed during read | `capture_changed` / `File changed during read` |
| Bounds `diagnostic_bytes`, `prompt_bytes`, `observation_entries`, `observation_file_bytes`, `observation_total_bytes`, `observation_bytes` | `resource_limit` / `Capture limit exceeded` |
| Diagnostic file grammar or envelope violation, observation file validation, duplicate observation ID, checkpoint bounds, malformed digest | The existing message of the shared JSON parser, schema normalizer or render observations validator for the same bytes (RA-5, RA-6) |
| Orchestrator job not listed | `reference` / `Unknown selected job` (field `selection.job`) |

No new logical source root is introduced. The `diagnostic` key of the rejection `inputs` and the observation
logical names (`checks/a.json`) are not roots and do not join the reserved source-root names of
[render-contract](render-contract.md#capture-boundary); they narrow no valid root ID.

Capture, validation and Lua semantic errors during authoring capture keep the messages render or load give the
same bytes. The enforcement of `prompt_bytes` and `observation_bytes` is by the native response encoder (a native
site), so no Lua site needs a message of its own.

## Pointer amendments

Short, additive; none narrows an approved clause. Each is recorded in the pointed document's own text.

1. [command-contract](command-contract.md): the catalog has eleven mandatory rows after RA01 (RA-1); root help
   gains the six lines of RA-1; the opaque host-path slots gain `diagnostic` and `from`; the services table
   gains the authoring, diagnostic, observation and orchestrator rows with the result values `V` of RA-2, RA-5
   and RA-6; and the sentence "No ... subprocess or directory listing is exposed" is amended, scoped to the
   `observe` family's bounded native listing of RA-6 (every other family still exposes no directory listing).
2. [load-command-contract](load-command-contract.md): the five-row completeness rule becomes eleven at the
   RA01 integrated gate; the root-help sentence follows RA-1.
3. [render-contract](render-contract.md): RA-6 assembles the observations value this contract defines and
   changes none of it; the capture boundary is reused for authoring capture (RA-2). Render stays database-free.
4. [external-profile-contract](external-profile-contract.md): `orchestrator` is shipped-owned
   (projection IDs); the five orchestrator section IDs join the reserved section-ID union; "default root help
   stays byte-for-byte old" means the pre-with-profile bytes plus only the amendments listed in command-contract;
   the with-profile mechanics of RA-9 (probe membership, precedence, collision, capacity and namespace) are
   recorded by pointer. The X-LIMIT table is unedited.
5. [CLAUDE.md](../../CLAUDE.md): this contract joins the exact-contract authority list (E14 A).

## Traceability

| Item | Contract section |
| --- | --- |
| RA-1 catalog rows, capabilities, options, help | [RA-1](#ra-1-catalog-rows-options-and-capabilities) |
| RA-2 authoring capture | [RA-2](#ra-2-authoring-capture-service) |
| RA-3 envelope | [RA-3](#ra-3-prompt-envelope) |
| RA-4 prompt content, critique | [RA-4](#ra-4-prompt-content) |
| RA-5 rejection prompt | [RA-5](#ra-5-rejection-prompt-e4-a) |
| RA-6 observe | [RA-6](#ra-6-observe-q11-a-e12-a) |
| RA-7 orchestrator, labels, reserved IDs | [RA-7](#ra-7-orchestrator-projection-q12-a) |
| RA-8 init coexistence | [RA-8](#ra-8-init-coexistence) |
| RA-9 profile deferral | [RA-9](#ra-9-profiles-e3-a) |
| E1 one family per verb | RA-1 |
| E3 embedded-only, with-profile mechanics | RA-9 |
| E4 pure explain-rejection | RA-5 |
| E12 observe layout | RA-6 |
| E14 new contract, authority list | Pointer amendments 5 |
| XD4 preserved (no new code or message) | Messages |
| Q10 B, Q11 A, Q12 A, Q14 A, Q18 A | Common rules; RA-2, RA-6, RA-7; decisions table |
| A35 deterministic prompts | RA-3, RA-4, Common rule 6 |
| A36 rejection prompt names field path | RA-5 |
| A01, A04, A07 preserved | Common rule 6; RA-2 (validation unchanged); RA-7 (mandatory sections) |

## Acceptance checklist (design section 4 test list)

Each item must be writable from this text without a design choice.

- A35: two fresh processes, same declarations and registry, byte-equal envelopes for each of the five prompt
  kinds; fixtures with reordered Lua table construction (RA-3, RA-4, Common rule 6).
- A36: a rejected load in a scratch ledger; its stderr as the diagnostic file; the prompt contains the exact
  field path, the `raw_error` block bytes equal load's stderr (including a diagnostic carrying raw DEL and C1
  bytes), the excerpt is produced for a path such as `job.roles[1].work`, load's exit stays 3 (RA-5).
- Critique: each of the three rules fires on a spec that passes validation; a spec triggering none prints
  `No mechanical findings.` (RA-4 rule 6).
- Schema drift guard: a probe mutating one `schema.lua` field changes the prompt (RA-4 rule 1).
- Observe: symlink entry, FIFO, hard-link alias, oversize file, too many entries, duplicate IDs across files,
  unknown fields, file replaced between listing and read (`capture_changed`), empty directory (canonical empty
  observations), deterministic order independent of creation order, dot names ignored (RA-6).
- Orchestrator: every `available` is in the sealed catalog, every future verb is `future`; a test-bundle stub
  verb flips its label (RA-7).
- Authoring capture with a listed job file absent succeeds for all three authoring verbs; `ingest` and
  `critique` with an invalid spec fail with the same typed error load gives (RA-2).
- Root help with six lines; eleven-row completeness; `with-profile` plus an RA01 verb is `usage` after profile admission, a profile registering any of the six names or `orchestrator_*` IDs is refused, and a broken profile reports its own error first (RA-1, RA-9).
- Existing `render`, `init`, `history`, `export` and `load` bytes and exits unchanged (RA-8).

## Decisions table

Tags: **C** contract-decided (cited), **Q/E** operator-decided, **NEW** new engineering choice the contract
reviewer may accept. None needs an operator question.

| Choice | Tag | Basis |
| --- | --- | --- |
| Six verbs, one family each, option sets | E1 A | Execution design verb table |
| Handler ID hyphen replaced by underscore | NEW (engineering) | Handler-ID grammar in command-contract; design said `shipped.<name>.v1` |
| `include` values per verb (`workspace` for spec-prompt, `spec` for critique and ingest) | NEW (engineering) | RA-2 design lists two values without the mapping |
| Orchestrator capture service reading exactly one job | NEW (engineering) | `orchestrator_capture_v1` is listed with the job option; jobs never captured in authoring |
| Orchestrator options all required in RA01 | NEW (engineering) | R15 later adds the ledger form |
| Prompt envelope `inputs` shape and rejection form | NEW (engineering) | RA-3 names `inputs` without a shape |
| Generated schema text format; section order | NEW (engineering) | RA-4 |
| Critique: three rule IDs (`prescription_unchecked`, `blocking_question_unanswered`, `facet_dependency_without_output_vector`), advisory exit 0; the design's consumer-without-dependency example is not used because spec validation already refuses it | NEW (engineering) | RA-4 "for example"; `semantic.lua` consumer check |
| Diagnostic file strict grammar, any code matching the code grammar, no canonical-form requirement, 64 KiB | E4 A with NEW details | RA-5 |
| Raw error embedded verbatim and unescaped in a max(3, L+1) backtick fence; equality defined over block bytes; other prompt data escaped | NEW (engineering); fence length rule code-decided from `presentation.lua` | RA-5; A36 |
| Field path: first segment selects the `workspace`, `spec` or `job` schema | NEW (engineering) | RA-5; `schema.lua` field paths |
| Service result shapes `V` for `capture_authoring`, `capture_orchestrator`, `read_diagnostic`, `read_observation_directory` | NEW (engineering) | RA-2, RA-5, RA-6; command-contract service protocol |
| Observe layout | E12 A | Decisions table |
| Observe bounds: 1024 entries in total, 64 KiB per file, 4 MiB total read, 1 MiB output; limit names `observation_entries`, `observation_file_bytes`, `observation_total_bytes`, `observation_bytes` (output) | NEW (engineering), numbers from design EX-8 | RA-6; EX-8 observe row. No deviation from EX-8 |
| Observe stdout is the bare observations v1 document, no wrapper | NEW (engineering) | RA-6; design RA-6 and EX-8 (observe never emits a file `--observations` refuses) |
| Observe validation uses render's codes and messages (duplicates `reference`, checkpoint bounds `resource_limit`, digest format `schema`), skips target and ownership semantics | C | RA-6; render-contract errors table and observations section |
| Hard-link alias by device+inode | NEW (engineering) | RA-6 |
| Critique prompt carries mechanical findings only; no model-judgement review-topic headings (deviation from the v5 review topics, which need judgement the mechanical critique does not make) | NEW (engineering) | design RA-4 "computed mechanically" |
| Orchestrator context, action labels, future list | Q12 A and NEW | RA-7; list from E1 table |
| Reserved projection and section IDs | C | Pointer amendments to external-profile-contract |
| No new code and no new message; every site maps to an existing message | C (XD4) | Messages; E16 is not used |
| `with-profile` plus RA01 verb stays usage after profile admission; probe stays five; collision, capacity and namespace consequences; variant closed at five with RA01-B wave 1 owning the architecture revision | E3 A and NEW (engineering) | RA-9 |
| `init`, `render`, `load`, `history`, `export` unchanged | Q10 B, C | RA-8 |
