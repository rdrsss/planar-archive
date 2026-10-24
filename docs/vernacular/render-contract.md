# Render contract v1

Status: approved render v1 contract, implemented natively through R08, with
computed quota packets and independent review. This defines preview rendering
for [declaration v1](declarations.md); broader execution remains deferred.
The product now also has init/history/export; “this slice” below names the
original database-free render scope. [V5](planar-design-spec-v5.md) supplies product
direction; explicit operator choices and approved exact contracts remain current
until scoped reviewed amendments. The [V5-02-A bridge and workspace contract](command-contract.md)
is the exact implementation target pending review/code: current native fixed
registration and explicit-file behavior remain supported until V5-02-B/C land.
See [alignment](v5-alignment.md) and [acceptance traceability](acceptance-matrix.md).

## Invocation and trust

```sh
/path/to/replacement/planar render \
  --workspace /scratch/quota/workspace.lua \
  --registry /trusted/quota-tools.json \
  --root source=/scratch/quota-current \
  --capture-outputs --base-root source=/scratch/quota-base \
  --job split --role cli_worker \
  --observations /scratch/observations.json --budget-bytes 32768
```

`render` is the only product command in this slice. Required options appear
exactly once: `--workspace` (file), `--registry` (trusted JSON file), `--root`
(`logical_id=directory`), `--job` (ID), `--role` (ID). The root ID must match
workspace.root.id. Optional `--capture-outputs` is a valueless flag requiring
`--base-root` (`logical_id=directory`, same root ID). Either both appear or
neither. Without them, no source/base tree is scanned and no output identity
is emitted. Optional `--observations` (JSON
file) defaults to the empty observations below. Optional `--budget-bytes`
(positive decimal integer, no sign/leading zeros, at most 1,048,576) overrides
the role/job budget for this preview only. `--help` alone prints usage and exits
zero; no other flags, positional arguments, stdin inputs, workspace discovery or
config-file lookup exist in current code. Current workspace values are explicit
files. The [workspace selector amendment](command-contract.md#workspace-selector-and-environment)
adds named selection through only PLANAR_WORKSPACE_DIR in V5-02-C; explicit paths
ignore that variable. Relative explicit render paths use invocation cwd and
physically resolve their parents before capture, preserving intermediate links
and symlink/.. filesystem semantics; they are not first lexically collapsed.
The amendment separately pins absolute lexical load request intent before any
filesystem access. It grants no broader captured-file authority. Options with `=` are
not supported except the root mapping's value syntax shown above. Host paths
never enter payload identities or Markdown. No `HOME`, legacy DB or installed
Planar configuration is consulted.

A minimal context-only invocation is:

```sh
/path/to/replacement/planar render --workspace /scratch/quota/workspace.lua \
  --registry /trusted/quota-tools.json --root source=/scratch/quota-current \
  --job combined --role builder
```

The source directory must exist and pass root-mapping confinement checks, but
its files need not exist yet. A reviewer can preview requirements this way;
its packet explicitly reports that exact targets were not captured. A captured
empty tree is different: it has a computed empty output identity and reports
that no implementation files exist. Only the capture form supplies actual
checkout content for review/recovery.

`--registry` is explicitly trusted driver configuration. Choosing it is an
external authority decision; a workspace file naming a registry ID cannot
supply its command definitions. Render reports registry ID/revision and selected
exact definitions. It does not execute toolchain commands, test their presence,
or certify the registry's claims about coverage.

Successful stdout is exactly one canonical JSON envelope followed by LF:
`{"version":1,"kind":"render","packet":P,"manifest":M}` (key order is
canonical, so the textual order here is explanatory). `P` and `M` schemas are
below. Stderr is empty on success. No default output files, ledger, caches,
inbox, sessions or state directories are created. Output destinations are shell
redirections chosen by the caller. The command assembles the whole envelope
before writing; a failed/short stdout write is an I/O failure and any partial
bytes must be discarded. Output is never a publication or acknowledgment.

Every packet begins with the fixed statement:

> Render-only preview. No run is active. Checks have not been executed by this
> render; no completion or approval is established.

Claims, refresh, `--since`, persisted history, live check operations, repair
revisions, request IDs and run tokens are rejected as unsupported flags, not
emulated. No packet claims that an earlier run occurred. Future execution must
preserve the v4 accepted-revision/history, version detection, backup/restore,
collision, fencing and atomicity requirements. The approved lack of **legacy**
migration does not remove evolution or recovery of the new ledger's formats.

## Embedded command bridge amendment

[Command catalog, lifecycle and services](command-contract.md#catalog-and-structural-abi)
define the V5-02-B implementation target. The registered Lua render callable must
execute routing, semantic validation, selection and projection; native CLI11
families retain options and own confined capture/output. This is actual semantic
dispatch, not metadata over unchanged native policy. Fixed root help, current
parser diagnostics, stdout/error handling, W recipes, envelope/Markdown bytes and
existing capture restrictions remain unchanged. Added embedded Lua changes exact
stdlib provenance, not W or interpretation_v1 shape. New naming behavior is the
separately scoped V5-02-C amendment. Neither target is claimed implemented here.

## Capture boundary

The engine first captures workspace.lua, the spec, **every listed job**, every
referenced text file and the trusted registry. File constructors only register
references while evaluating; they never grant arbitrary file reads to Lua.
Relative references resolve as specified in declarations.md. All declaration
validation, projections and hashing consume those retained bytes/values.
Observations JSON is captured as a separate untrusted input. With `--capture-outputs`, the engine also
captures complete current and base source trees, after explicit exclusions,
including ignored/untracked regular files. It performs no Git invocation.
`root.base` is a driver label; the base **content identity is recomputed**, not
accepted from that label or a supplied Git object name. A base snapshot may be
empty. Without capture, the base label remains a visibly unverified declaration
assertion and there is no exact base-content identity. The driver supplies its
immutable view or holds files stable throughout capture; render does not freeze the OS filesystem.

Capture entries record logical root, relative path, regular-file kind, mode
(`"0644"` or `"0755"`, based only on the executable bit), byte length and exact
bytes. Directory permissions, mtimes, uid/gid, host paths and empty directories
are excluded. Renames are delete/add; non-executable permission changes outside
this normalized mode are not observed. Special files, symlinks, hardlink aliases,
case aliases and traversal are refused, as specified in declarations.md.
The registry and observations use reserved logical roots `registry` and
`observations`, path `input.json`; source root IDs must also avoid these names.
Declaration files use root `workspace` and workspace-relative paths.

The engine reads each file with stable descriptor identity and before/after
metadata checks, records membership, then re-enumerates and re-reads the entire
captured set before projection; bytes, normalized modes and membership must
match. Any detected difference returns `capture_changed`; no automatic retry.
This is a cooperative capture, not an atomic snapshot or defense against
change-and-revert. Once capture succeeds, no downstream filesystem reads occur.
References discovered during load are included in this same capture/final
verification. Separate projection of role packets cannot recapture a file.

Hard limits (not declaration-overridable): 1 MiB per declaration, text reference,
registry or observations file; 16 MiB per source/base file; 64 MiB total retained
input bytes; 10,000 regular files total. Tree enumeration has separate discovery
and final-verification budgets, each shared across all captured trees: 100,000
non-dot directory entries and 64 MiB (67,108,864 bytes) of conservative metadata
accounting. Every entry returned by readdir except `.` and `..` is charged before
copying/staging its name, including excluded names and `.git` directories (their
unvisited contents are not enumerated). The exact charge per entry is
`512 + 6 * (relative_path_bytes + 1) + 2 * (basename_bytes + 1)`, where the relative
path includes its parent prefix and separators but no trailing slash. This covers
staged names, spelling maps, retained/verified directory topology and its sealed
logical copy; it exceeds 128 bytes plus path/basename bytes and terminators.
Refuse the next excessive charge with `resource_limit`, limit
`enumeration_entries`, maximum 100,000, or `enumeration_bytes`, maximum
67,108,864 (entry count checked first). The budgets also bound per-directory
staging and retained topology. Exhaustion during name staging is a resource
refusal before complete membership comparison; an already detected retained
directory identity/membership difference or retained-file quota growth remains
`capture_changed`. No declaration can override these budgets.
Other hard limits: 64 nested declaration-value levels;
100,000 converted nodes; 64 MiB Lua allocation per chunk; 1,000,000 Lua VM
instructions per chunk. The trusted runtime must separately bound its own
conversion/allocation to these structural limits. Native declaration conversion
also has a separate 64 MiB conservative allocation-accounting budget per chunk,
including discarded intermediate constructor values. Accounting charges 512 bytes
per converted or constructor-copy node, 128 bytes per ordered entry, 128 bytes
plus key bytes and one terminator per record entry, and payload string bytes plus
one terminator for each native copy. Registered file references charge 128 bytes
plus path bytes and one terminator. This prevents repeated Lua scalar strings
from amplifying into unbounded native copies. Refuse before the allocation that
would exceed this budget, with `resource_limit`, limit `native_bytes`, maximum
67,108,864. Lua and native budgets are independent. Output envelope is at most
16 MiB. Limits count raw bytes before parsing. Refuse before excessive allocation;
zero executed product tests cannot be presented as validation of these limits.

Job-tier Lua sees only `planar` constructors plus ordinary language literals,
operators and control flow; no standard-library globals are exposed in v1.
In particular no io/os/debug/package/require/load/dofile, clock, randomness,
process, environment, engine or mutable trusted tables. Load text chunks only,
never bytecode; global writes and constructor-table writes fail. Trusted modules
load in a distinct environment that declarations cannot mutate. Resource exits
and Lua longjmp must not cross C++ objects with pending destructors; R05 tests
this native boundary. Determinism/security cannot be established by a fake loader.

## Observations JSON v1

All JSON inputs reject duplicate keys, unknown fields, invalid UTF-8/NUL strings,
non-integer numbers and out-of-range values. Arrays are distinct from objects.
Primitive bounds follow declarations.md unless overridden here. The exact
observations root keys are `version` (required 1), `checks` (default empty
array), `findings` (default empty array), `checkpoint` (optional). No nulls.
Canonical empty input is:

```json
{"version":1,"checks":[],"findings":[]}
```

- A **target** is exactly `{work: work ID, output: digest}`. It must name a
  selected job's unit and match the current captured unit output identity.
  Missing, stale or wrong targets are `observation_target_mismatch`, never
  silently attached to current output. A digest has the representation below. Any target assertion without
  --capture-outputs is refused as observation_target_mismatch; a checkpoint
  alone is allowed.
- A **check assertion** has required fields `id` (unique observation ID),
  `check` (registered check ID required for that target), `target` (target),
  `reported` (`"pass"`, `"fail"` or `"inconclusive"`), `summary` (string,
  at most 4,096 bytes), `vector` (array<target>, sorted/unique work IDs).
  For a check with output_vector inputs, vector is exactly the current own and
  complete-dependency output vector. Otherwise vector contains exactly target.
  Observations of an unavailable output fail validation. These are **driver
  assertions, unverified**, even if reported pass; no check receipt is created.
- A **finding** has required `id` (unique within findings), `target` (target),
  `severity` (`"note"` or `"required"`), `text` (string, at most 4,096 bytes).
  These are **driver assertions, unverified**; a required finding is visible
  unresolved context, never a recorded rejection/repair event. No finding can
  make a question answered, a producer done or output approved.
- A **checkpoint** has exactly `progress`, `next`, `unresolved` (required strings,
  each at most 2,048 bytes), `files` (required unique array<source file path>,
  at most 32). Paths must be in the selected worker's boundaries, or, for a
  reviewer, in the targeted units' boundaries; existence is not required.
  The complete object canonical JSON is at most 8,192 bytes. It is an optional
  **session note, unverified**, not a historical snapshot or evidence. Empty
  strings/empty files array are permitted. There is no acknowledged old work
  fingerprint, prior run token, or claimed durable event in this schema.

Observations are filtered per role: a worker sees its units and subscribed
producer units; a reviewer sees its targets. A checkpoint belongs to the selected
role invocation. Global parsing/reference/target validation still covers all
supplied assertions; unrelated records do not leak into a role's packet or work
identity. The manifest records the captured observations file digest as provenance.

A valid bounded recovery-note input (no outputs asserted):

```json
{"version":1,"checks":[],"findings":[],"checkpoint":{"progress":"Started the CLI parser.","next":"Connect the captured quota contract.","unresolved":"Integration has not been checked.","files":["src/cli/main.py"]}}
```

The packet combines that note with the **current captured checkout** when requested (otherwise “checkout
not captured”), current requirements and explicit absences. It cannot reconstruct private reasoning,
past progress or a dispatched context. Its recovery section says “Session note
supplied for this preview; history unavailable; no run is active.”

## Canonical encoding and hash representation

Interpretation identifiers are exactly `declarations_v1`, `selection_v1`,
`dependencies_v1`, `exact_bytes_v1`, `projection_v2`, `canonical_v1`,
`xxh3_128_v1`. They identify shipped semantics, not caller-chosen strings. Any
semantic change needs a new corresponding identifier and reviewed vectors.

Canonical **binary** encoding C is independent of JSON presentation:

| Value | Bytes (hex tags; concatenation in listed order) |
| --- | --- |
| false / true | `00` / `01` |
| signed integer | `02` then signed 64-bit two's-complement, big endian |
| UTF-8 string or raw byte string | `03` then unsigned 64-bit big-endian byte length then bytes |
| array | `04` then unsigned 64-bit big-endian item count then C of each item in order |
| object | `05` then unsigned 64-bit big-endian key count, then C(key), C(value) for each key |

Objects have string keys sorted lexicographically by unsigned UTF-8 bytes;
strings are never Unicode-normalized. Schema domain distinguishes raw bytes
from text. There are no null values/floats. Encoding accepts signed integers
within [-2,147,483,648, 2,147,483,647]; nonnegative fields still apply narrower
schema bounds. No Lua numeric coercion or decimal/floating roundtrip. Empty
array bytes are `04 0000000000000000`; empty object bytes are
`05 0000000000000000`. Those are encoding illustrations, not hash vectors.

Define `H(domain,value) = XXH3_128bits(C(["planar", "canonical_v1", domain,
value]))`, using the pinned xxHash implementation, default seed zero, no secret.
The digest representation is `xxh3_128:` followed by 32 lowercase hex digits
from `XXH128_canonicalFromHash`: high64 first in big-endian bytes, then low64
big-endian. A bare file hash is `H("file_bytes_v1", raw_bytes)`; a fragment hash
is `H("fragment_v1", exact_markdown_UTF8_bytes)`. No host-endian structs or
stringified Lua values are hashed. Native R06 vectors must pin the xxHash
release and validate the encoding and hash independently. No example here
pretends to be a computed digest vector. xxHash supports local change detection,
not authenticity; equal digests never justify skipping exact byte comparison
or reusing unequal records when both values are available.

Canonical **JSON** transport uses UTF-8, no whitespace outside strings, object
keys sorted as above, decimal integers without exponent/leading zeros, unescaped
non-control Unicode, and no escaped `/`. Escape `"` and `\` with backslash;
control characters U+0000–U+001F use Glaze short escapes (`\b`, `\t`,
`\n`, `\f`, `\r`) where available and uppercase hexadecimal `\u00XX`
otherwise (inputs disallow NUL,
but raw source bytes are not encoded as JSON strings). Arrays preserve order.
Captured raw source bytes are exposed as lowercase hex in manifests, so arbitrary
binary source data never becomes invalid JSON. Lua text declarations must still
be valid text. Transport LF is outside all payload identities.

## Normalized values and scoped identity recipes

Normalize records by inserting documented defaults, sorting set fields and
ID-keyed record collections by ID, and retaining genuine ordered lists:
pipeline stages and acceptance criteria. Text values normalize to
`{kind:"text",bytes:<UTF-8>,file:{root:"workspace",path:<resolved path>}}` for
file text, or `{kind:"text",bytes:<UTF-8>}` inline. Wrapper metadata disappears.
Absent optional fields remain absent. File line numbers and Lua comments do
not enter normalized values. No raw declaration hash is embedded in semantic
or work identity. A formatting-only declaration edit can move provenance
without moving work. A mandatory text byte change is material.

The identity recipes below use exact named records and domains. All digest
fields are recomputed from captured inputs; observation assertions can only
be checked against them. Identity arrays sorted by work ID carry separate unit
records even when multiple units map to one worker.

- **File entry** is exactly `{root,path,kind:"file",mode,size,bytes_hex,digest}`.
  Size is the raw byte count, bytes_hex is lowercase hex of captured bytes,
  digest is file_bytes_v1. Entries sort by `(root,path)` byte order. A tree
  value is its array of file entries **without** bytes_hex and digest, each
  extended with `content` equal to the file digest. A tree's exact bytes are
  also retained for collision/verification, not substituted by hash equality.
- **Base identity** is `H("base_v1",{root:<source ID>,label:<root.base>,
  entries:<base tree value>,excluded:<normalized excluded boundaries>})`.
  Only computed in output-capture mode. The label has no independent evidentiary force; same label/different bytes
  gives a different identity. Host base paths never participate.
- **Observation identity** is `H("observation_v1",{root:<source ID>,
  base:<base identity>,entries:<current tree value>,excluded:<boundaries>})`.
  Only computed in output-capture mode. It covers the full non-excluded checkout,
  not just role paths.
- **Unit output** is `H("unit_output_v1",{work:<unit ID>,root:<source ID>,
  base:<base identity>,paths:<unit boundaries>,entries:<current tree value
  restricted to those boundaries>})`. Empty output is a valid empty array,
  visibly marked absent if no files exist; a facet may still be unavailable.
- **Role output** is `H("role_output_v1",{role:<role ID>,units:<array of
  {work,output:<unit output>}>})`. Reviewer has no source role output and
  that field is absent. Output vector targets are unit outputs, avoiding
  grouping-dependent conflation of a library target with the whole builder.
- **Facet semantic identity** is `H("facet_v1",{kind:"files",
  normalizer:"exact_bytes_v1",root:<source ID>,paths:<declared paths>,
  entries:<selected current tree value>})`, only if every path exists.
  Raw content provenance is separate from the facet's normalized exact bytes
  representation; v1 exact-byte semantics happen to track content changes.
- **Contract semantic identity** is `H("contract_v1",{kind:"text",
  normalizer:"exact_bytes_v1",expected:<normalized text>})`. Check policy
  and review_note are applicable material fields separately, not invented
  proof of an actual conformance result.

For a role, construct material object **W** with exactly these fields:
`versions`, `workspace`, `spec`, `job`, `role`, `root`, `base`, `goal`, `units`,
`milestones`, `constraints`, `prescriptions`, `contracts`, `dependencies`,
`facets`, `pipeline_predecessors`, `questions`, `ownership`, `effective_checks`,
`checks`, `toolchains`, `shape`, `start_facets`.
Their values are:

- `versions`: array `["declarations_v1","selection_v1","dependencies_v1",
  "exact_bytes_v1","canonical_v1","xxh3_128_v1"]`.
- `workspace`, `spec`, `job`, `role`: their logical IDs. `root` is the source ID;
  `base` is `{status:"captured",digest:<base identity>}` in capture mode, or
  `{status:"not_captured",root:<source ID>,label:<base label>,
  authority:"declaration_assertion_unverified"}` otherwise. Context-only and
  captured-base work fingerprints intentionally differ: actual base identity
  is newly available material input, not a claim that the label verified it.
  `goal` is normalized spec.goal.
- `units`: normalized assigned work records for workers, targeted work for
  reviewer. `milestones`: normalized applicable milestone records restricted
  to selected unit IDs, retaining their acceptance lists. `constraints` and
  `prescriptions`: global or intersecting scope records with scope restricted
  to selected units (`"global"` remains global). Unrelated records are absent.
- `contracts`: normalized contracts produced/consumed by selected units,
  plus contract semantic IDs, represented as `{declaration,semantic}`.
  `dependencies`: normalized records whose consumer is selected, with their
  resolved producer unit ID. Include contract semantic ID on contract edges.
  `facets`: sorted normalized declarations for every facet referenced by those
  dependencies, including `id`, `producer`, `paths`, and explicit defaults
  `kind:"files"`, `normalizer:"exact_bytes_v1"`. Include these definitions for
  all phases/qualities, without capture and when files are missing. Do not
  substitute observed facet values for declaration policy.
  `pipeline_predecessors`: sorted derived obligation records from the full job
  graph whose consumer is selected, exactly as in declarations.md. Include
  producer/consumer work IDs and both worker-role assignments.
  `questions`: normalized global/intersecting scoped questions and answers.
- `ownership`: `{work:<array of {work,paths}>,frozen:<all frozen boundaries>,
  excluded:<all excluded boundaries>}`. Role grants derive from these records.
- `effective_checks`: sorted array of `{work,checks:<sorted array<check ID>>}`
  for every selected unit, using its owning worker's effective check set from
  declarations.md. Reviewer uses those same per-target sets, including worker
  additions. `checks`: sorted exact registry definitions for their union;
  `toolchains`: sorted definitions referenced by those checks. No registry-wide
  unrelated definition or revision-name hash is inserted. Equal definition
  unions do not erase different per-work applicability.
- `shape`: `{kind:<shape>,max_rounds:<cap>,mapping:<array of {work,role}>,
  reviewer:<reviewer ID>}`. Mapping is restricted to selected units, producers
  of their explicit dependencies and producers of their pipeline predecessor
  obligations. Mapping sorts by work ID. The full stage list is not copied:
  its scoped material meaning is in `pipeline_predecessors`, derived **before**
  scoping; no predecessor can disappear for lack of a facet subscription.
- `start_facets`: array of `{dependency,semantic}` for start/facet dependencies
  consumed by selected units. If a captured file is missing, record `{dependency,unavailable:true}`;
  without capture record `{dependency,not_captured:true}`.
  Complete-only facet values, producer quality and actual outputs are excluded;
  their **policy** remains in dependencies and facets; exact vectors enter
  future check evidence. Exclude values whose producer is a selected unit,
  even if start-bound: these are grouped own outputs for workers and target
  outputs for reviewer. Their phase/observation obligations remain explicit
  in dependencies and their declarations remain in facets.

Work fingerprint is `H("work_v1", W)`. Reviewer W includes requirements/check
policy but not current target outputs; its separate target vector is necessary
for evidence applicability. Budgets, preferences, notes, rationale, checkpoint,
observations and projection version are excluded from W. Render is preview,
so no repair revision or generation is fabricated. Later execution must add
those through a versioned recipe. Own implementation changes never move a
worker's fingerprint, even when grouped dependencies expose an own facet.
A changed upstream *start* facet can move a distinct worker's fingerprint;
a complete-only facet's observed-byte change moves output/target data, not
source-work identity. Changing its declared paths/producer/normalizer changes
subscription policy in W even when its ID and observed bytes are unchanged.

## Packet and manifest schemas

**P** has exactly `version:1`, `mode:"preview"`, `workspace` (ID), `spec` (ID),
`job` (ID), `role` (ID), `kind` (`"worker"` or `"reviewer"`), `work` (sorted
array<work ID>), `markdown` (UTF-8 string). Markdown has LF structural separators,
no BOM, one final LF. Embedded text bytes remain exact and may contain CRLF.
The packet does not embed its own render digest, avoiding recursive identity.

**M** has exactly these required fields:

| Key | Value |
| --- | --- |
| `version` | 1 |
| `versions` | W.versions plus `projection_v2` appended |
| `work` | `{value:W,digest:<work fingerprint>}` |
| `render` | `{digest:<fragment hash>,bytes:<markdown byte length>,budget_bytes:<effective budget>}` |
| `base` | `{status:"captured",digest:<base identity>,entries:<base tree value>}` or `{status:"not_captured",root:<source ID>,label:<base label>}` |
| `observation` | `{status:"captured",digest:<observation identity>,entries:<current tree value>}` or `{status:"not_captured"}` |
| `outputs` | sorted array of `{work,status:"captured",output,present:<bool>}` for all units in capture mode; otherwise `{work,status:"not_captured"}` records |
| `role_output` | role-output digest for worker in capture mode; otherwise absent |
| `targets` | sorted array<target> for reviewer units in capture mode; otherwise empty |
| `files` | sorted array<file entry> covering all captured declaration, registry, observations, current and base inputs |
| `assertions` | normalized, role-filtered observations object; every check/finding extended with `authority:"driver_assertion_unverified"`, checkpoint with `authority:"session_note_unverified"` |
| `sections` | ordered array of `{id,mandatory:<bool>,included:<bool>,bytes:<uint>}` |
| `omitted` | ordered array of optional section IDs |
| `prerequisites` | sorted array of prerequisite records as defined below |
| `registry` | `{id,revision,authority:"driver_selected_trusted_configuration"}` |

For example, these are literal manifest **subobjects** in context-only mode
for the combined job; they contain no fabricated digest values:

```json
{"base":{"status":"not_captured","root":"source","label":"initial"},"observation":{"status":"not_captured"},"outputs":[{"work":"command","status":"not_captured"},{"work":"library","status":"not_captured"}],"targets":[]}
```

The complete M additionally includes W, its computed digest, the fragment
digest, captured declarations and the other required fields above. These
subobjects are not a complete runtime envelope or a golden.

For files from current/base trees use roots `current:<source ID>` and
`base:<source ID>` **in files provenance only**; tree/identity values use the
source ID without those prefixes. Declaration file entries use `workspace`.
If no observations file was supplied, no observations file entry is invented;
assertions is still the empty normalized observations value. Same captured file
referenced twice appears once. Registry is captured once. Manifest bytes_hex
retains actual captured bytes for exact comparison, despite increasing output
size; output-envelope limit errors are explicit. No arbitrary host path is
included in file provenance. Base/current entries have their role-independent
root ID as specified in the tree recipes.

A prerequisite for an explicit dependency or blocking question has exactly
`{id,phase,status,source}`. IDs are dependency IDs or `question:<question ID>`;
`phase` is start/complete (`blocks:none` questions create no entry); `source`
is contract/facet/question. Status is `available`, `unavailable`, `unverified`,
`not_captured` or `answered`. Contract captured text → available; available facet
→ available/unavailable in capture mode, otherwise not_captured;
done/accepted → unverified even if files exist.
Unanswered blocking question → unavailable; answered → answered.

Each W.pipeline_predecessors entry also produces a prerequisite containing
**all** its fields (`id`, `source`, `producer`, `consumer`, `producer_role`,
`consumer_role`, `phase`, `quality`) plus `status:"unverified"`. It has no
captured-file availability shortcut: render cannot establish unit completion.
Sort the combined prerequisite array by ID. Render both kinds of obligation
even if they connect the same units; a pipeline predecessor does not replace
an explicit integration dependency. For grouped done edges add no pretend
success: prerequisite remains unverified and is rendered as an internal
completion obligation. No ready/running/done/accepted role state appears in M.
Successful rendering of blockers is allowed.

The manifest provides recomputable exact inputs, not a signature. Validators
must recompute file, tree, output, work and fragment hashes, not trust claimed
digests. Malformed expected identities in observations are errors. A future
immutable external Git-target adapter needs its own validation contract; v1
accepts only captured trees. No fake literal digest is printed in this document.

## Markdown projection and byte budget

`projection_v2` retains section order, selection and budget policy below. R10-P01
updates the current native goldens for readable policy fields and scoped identity
display. Original R03 expectations and prior round evidence remain historical.
Only M appends this projection version; W and canonical identity domains retain
their existing versions.

| ID / order | Content | Mandatory |
| --- | --- | --- |
| `authority` | Fixed render-only statement, role/work IDs, work fingerprint and trusted-registry provenance | yes |
| `goal` | Goal and assigned/targeted unit summaries | yes |
| `acceptance` | Applicable milestone/criterion IDs and exact acceptance text | yes |
| `requirements` | Scoped constraints, prescriptions, material question prompts/answers and blockers | yes |
| `contracts` | Applicable expected interface text, declaration review assertions, conformance IDs | yes |
| `boundary` | Exact work paths, frozen/excluded paths and read-only remainder | yes |
| `dependencies` | Explicit dependency IDs, resolved producer/consumer IDs, referenced facet policy (paths/kind/normalizer), phase/quality, internal versus cross-role obligations, availability and unverified quality; every pipeline predecessor ID and both work/role assignments with its unverified complete-before-start requirement | yes |
| `checks` | Effective check IDs keyed by each assigned/targeted work ID, including owning-worker additions; exact union of definitions/toolchains and coverage limits; “not executed by render” | yes |
| `outputs` | Selected current file inventory, base differences and exact unit outputs; reviewer full target vector | yes |
| `observations` | All applicable check/finding driver assertions, explicitly labeled; explicit absence if empty | yes |
| `preferences` | Requested model/session preferences, “driver preference; not executed” | yes, if present |
| `rationale` | Spec rationale | no |
| `workspace_notes` | Workspace notes | no |
| `job_notes` | Job notes | no |
| `checkpoint` | Bounded session note, captured/not-captured checkout status, history unavailable/no active run | no |
| `omissions` | Fixed omission notice described below | yes, when needed |

Without output capture, outputs is the explicit sentence “Outputs and base
not captured; exact review targets unavailable.” No diff or output identity is
invented. With capture, outputs include selected file content as data, using
the captured bytes. For
workers select own units plus declared upstream facet files; reviewers select
all targeted output files. Binary files render as lowercase hex with byte count;
UTF-8 files without NUL render exact text. Base differences list added/deleted/
modified paths, old/new modes and byte sizes; modified textual files include
**complete old and new content**, not an unspecified diff algorithm. Unchanged
files include current content once. Deletions include old content. This bounded
simple review view can overflow; refusal is preferable to hidden omission of
required review content. No guessed success if the checkout is empty: state
“no files captured in this unit; output absent.” The exact empty output identity
still exists. Registry commands are descriptions, never executable instructions
generated from assertion prose.

Each check appears separately with ID, toolchain, timeout in milliseconds,
output cap in bytes, selection policy, exact argv, input kinds and complete
coverage prose. Each applicable toolchain appears once with exact command argv,
environment entries and full description. Argv, inputs and environment use small
Glaze JSON arrays in dynamic fences; no shell command is invented. The effective
check IDs remain keyed by work unit. A single checks-section statement says
render did not execute them, coverage is unverified and no receipt exists.

Each explicit dependency appears once with ID, source/reference, producer and
consumer work/role assignments, phase, quality, internal/cross-role obligation
and prerequisite status. Facets show producer, kind, normalizer, complete paths,
capture availability, and captured file path/mode/size. Pipeline predecessors
show IDs, both role assignments and unverified complete-before-start policy;
question prerequisites retain phase and answered/unavailable status. The exact
semantic records and content digests remain in M, including W, prerequisites and
retained files. There is no repeated aggregate prerequisite JSON in Markdown.

Authority retains the full work fingerprint and trusted-registry provenance.
Outputs/checkpoints show capture status; the base driver label is fenced data.
Global observation/base hashes and per-file/facet content hashes are available
in M rather than repeated in Markdown. Worker unit/role output IDs remain
visible. Reviewers display the complete ordered work-to-output target vector
once in outputs, without duplicating those IDs as unit-output lines. Applicable
driver assertions still show their exact targets/vectors as unverified data.
Unrelated unselected output edits can therefore change global M.observation
without changing a worker's Markdown or fragment identity.

All captured prose, source content, check summaries and checkpoints appear as
delimited data: a label with logical path/ID and authority, followed by a fenced
block. Use backtick fences of length `max(3,1+longest backtick run in content)`;
no language tag. Add a separator LF before the closing fence if content lacks
one; this separator is framing and never changes content hashes. Section labels
and fixed headings are projection-owned. User content never supplies headings,
authority fields, or terminal-control output outside the data block. Escape
control characters other than LF/CR/TAB as `\u00xx` when rendering text, and
record that display escaping in the section label. This is provenance framing,
not a guarantee against a model obeying injected instructions.

Every generated section begins `## <ID>\n\n` and ends with exactly one structural
LF after its body; sections join with one LF. Section bytes count its heading,
body and trailing LF plus the join LF preceding it (first section has no join).
Sum of included section bytes equals exact markdown bytes. Mandatory empty
sections show an explicit absence sentence; optional absent sections are not
candidates and do not appear in omitted. `preferences` absent means no section.

Candidate sections are those whose content is applicable to this role. Each
list in a section follows its normalized ID/path order; source files sort by
logical path, and differences use the same order. Requirement/criterion labels
use their declared IDs. No timestamps, host paths, Lua insertion order or model
responses are projection inputs.

Budget is exact UTF-8 Markdown bytes, excluding envelope/manifest. Never estimate
tokens. Construct all candidate sections first. Try retaining all; if too large,
drop whole optional sections in order **checkpoint, job_notes, workspace_notes,
rationale**, regenerating the omission section after each drop. Its exact body
is `Optional sections omitted: <comma-space-separated IDs in removal order>.\n`.
Notice plus mandatory sections must fit; otherwise `budget_exceeded`. No
partial paragraphs/files, silent truncation or omitted mandatory assertions.
Manifest sections includes every candidate, with bytes as if included under
its original section position; after selection recompute included section/join
byte counts so their sum equals emitted bytes. Omitted section bytes report
the standalone headed section length without a join. This makes audit counts
unambiguous. If no optionals were dropped, omit omissions entirely. An optional
note change can alter render output/omission metadata but never W.

## Paper regression traces

These are contract-level cases for R02-C01, not executed renderer tests or
computed hash vectors. “W changes” means canonical material inputs differ;
future native checks must recompute identities and compare exact values.
Hold every input not named by the case fixed, including captured base content.

| Case | Change / trace | Required result |
| --- | --- | --- |
| Facet policy | Consumer `b` has complete/done facet `f` from `a`; replace `f.paths` from `src/a/one` to `src/a/two`, both within `a`'s boundary, leaving IDs fixed | `b` and reviewer W.facets change; mandatory dependency text names the new path. Same result without capture or with missing files; there is no dependence on start_facets. |
| Facet values | Leave that declaration fixed; edit only the current bytes of its files | `b` W remains equal for the completion-only edge; captured facet/output/target data changes. For a start edge to a distinct worker, start_facets changes. Grouped own outputs and reviewer target outputs stay out of W.start_facets. |
| Check applicability | Two workers own `a` and `b`; both require `base_check`. Move an allowed compatible role-added `extra_check` from `a_worker` to `b_worker` | Reviewer W.checks union stays equal, but W.effective_checks changes from `a:[base_check,extra_check], b:[base_check]` to `a:[base_check], b:[base_check,extra_check]`. Reviewer W and per-target check instructions change, as do both affected workers' policies. |
| Review barrier | `a` accepted → `b` start; both reviewed by `review` | Refuse `dependency_cycle`: `work:a:accepted → work:b:start → work:b:complete → role:b_worker:complete → role:review:start → role:review:complete → work:a:accepted`. Grouping both into `builder` substitutes `role:builder:complete` and still closes the cycle. |
| Legal completion | Replace accepted with complete/done from `a` to `b`, with a compatible output_vector check, no reverse edge and fanout_review | No cycle: `a.complete → b.complete → worker.complete → review.start → review.complete → a.accepted` has no edge back from acceptance. Combined grouping retains the local completion obligation; contract-available start still permits development before producer completion. |
| Pipeline scope | No explicit data dependency; stages `[a_worker]`, `[b_worker]` owning `a`, `b` | `b` W.pipeline_predecessors and prerequisites contain `pipeline:a:b`, producer `a`/`a_worker`, consumer `b`/`b_worker`, start/done, status unverified only in prerequisites. Markdown states `a` must complete before `b` starts. Reversing stages removes that obligation from `b` and adds `pipeline:b:a` to `a`, changing both workers' W and reviewer W. Remapping `a` changes the recorded assignment in `b` W. |
| Pipeline derivation | Stages `[a_worker]`, `[b_worker]`, `[c_worker]`; select only `c` with no data subscriptions | Derive full obligations first, then retain `pipeline:b:c` for `c`; never invent `pipeline:a:c` by filtering out `b`. `b_worker` remains in scoped mapping. The full graph retains transitive `a` before `c`. |
| Boundary topology | Pair `x` with each of `x/`, `x/y`, `x/y/`; also pair `x/` with `x/y` | Refuse `ownership` for each pair, including absent files. Apply to two units, grouped units, within one unit/list, work versus frozen/excluded, and frozen versus excluded. Captured existence cannot legalize a conflicting declaration. |
| Disjoint boundaries | `src/core/`, `src/cli/`, `contracts/` frozen and `build/` excluded; also pair `x/` with `xy/` | No boundary conflict. Distinct siblings and mere string prefixes remain valid; the combined/split declaration example retains its boundary grants. |
| Stable policy | Reorder Lua record keys or set entries without changing values; edit optional notes; keep facet policy, checks and pipeline obligations fixed | W stays equal. Notes may change Markdown/render identity; syntax-only changes may change file provenance. No completion or approval is established. |


## Errors and verification boundary

On failure, stdout is empty except unavoidable partial bytes after a write
failure. Stderr is one canonical JSON object plus LF:
`{version:1,kind:"error",error:{code,message,field,...}}`. Required `code` and
`message` are strings; `field` is a schema path string, empty for invocation or
system errors. Optional `file` is a logical path, `line` a positive integer when
known; never invent Lua line attribution. `details` is optional and has the
code-specific shape below. No traceback, host path, ANSI decoration or Lua
address leaks. Usage errors exit 2; schema/capture/resource/budget errors exit 3;
system I/O/internal failure exits 4; success exits 0. No unexplained warning
successes or skipped validation. Sort independent diagnostics by logical file,
field, then code and report the first; stop immediately for unsafe capture or
resource exhaustion. Messages may evolve; codes and details are the API.

| Code | Cause / details if present |
| --- | --- |
| `usage` | Missing/duplicate/unsupported option; details `{option:<string>}` |
| `unsupported_version` | Unknown schema; `{supported:[1],received:<integer>}` |
| `lua_syntax` | Text chunk syntax error |
| `capability_denied` | Forbidden global/capability/mutation/bytecode |
| `schema` | Wrong type, unknown field, invalid wrapper/number/list |
| `reference` | Missing/duplicate ID, unknown check/toolchain/job/role |
| `ownership` | Overlap, alias or out-of-bound facet; `{paths:[<sorted paths>]}` |
| `dependency_cycle` | Unsatisfiable phases; `{cycle:[<vertex labels in traversal order>]}` |
| `path_denied` | Unsafe path, link, special file or traversal |
| `capture_changed` | Detected input/membership change |
| `resource_limit` | `{limit:<name>,maximum:<uint>}` |
| `observation_target_mismatch` | Missing/stale target or wrong vector; `{work:<ID>}` |
| `budget_exceeded` | `{budget_bytes:<uint>,mandatory_bytes:<uint>,sections:[{id,bytes}]}` after optional removal, including omission notice |
| `io` | Read/write/open error, missing required file; `{operation:<string>}` |
| `internal` | Impossible trusted runtime condition; no partial success |

For example, a valid machine error for an unsupported legacy flag is:

```json
{"error":{"code":"usage","details":{"option":"--claim"},"field":"","message":"Unsupported render option: --claim"},"kind":"error","version":1}
```

Concrete trusted registry for the declaration example (descriptive only; these
commands and test files have not been executed or authored by R02):

```json
{"version":1,"id":"quota_tools","revision":"v1","toolchains":[{"id":"python","description":"Driver-pinned Python 3","command":["/usr/bin/python3"],"environment":[]}],"checks":[{"id":"contract","toolchain":"python","argv":["-m","unittest","tests.contract"],"coverage":"Selected quota API examples; not a proof of all behavior.","inputs":["expected_contracts","own_output"],"timeout_ms":10000,"max_output_bytes":65536,"selection":"nonempty"},{"id":"integration","toolchain":"python","argv":["-m","unittest","tests.integration"],"coverage":"Selected actual library/CLI output combination.","inputs":["own_output","output_vector"],"timeout_ms":10000,"max_output_bytes":65536,"selection":"nonempty"}]}
```

R02 verification is document/schema/example review only. It cannot establish
native sandboxing, race safety, canonical hash results, executable counts or
byte-identical native packets. R03 must provide the concrete output manifest
and Markdown examples with symbolic identities clearly labeled; R06/R08 replace
symbols with independently checked computed results and complete goldens.
