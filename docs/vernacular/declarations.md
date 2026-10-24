# Declaration contract v1

Status: approved declaration v1 contract, implemented by the native render slice
through R08. Use this document and [the render contract](render-contract.md).
[V5](planar-design-spec-v5.md) is the product-direction baseline; explicit operator
decisions and these approved exact schemas govern supported behavior until scoped
reviewed amendments. V5 examples are illustrative. [Alignment](v5-alignment.md)
records pending V5-02 command/trust/workspace decisions and later execution work.
No declaration starts a run or proves a check passed.

## Representation and authority

Each UTF-8 Lua source chunk returns exactly one `planar.workspace`,
`planar.spec`, or `planar.job` value. The global `planar` is a read-only curated
constructor namespace; no `require` is needed. Constructor version fields must
be integer `1`. Only the signatures in this document are exposed. Records use
plain Lua tables with exactly the named string keys. Unknown keys are errors.
A missing optional key is absence, not a null value; `nil` is never a list item.

Every collection uses `planar.list { ... }`, including an empty list
`planar.list {}`. Its argument must have precisely the integer keys 1 through
N, with no holes, string keys, metatable or repeated table identity. `{}` in a
record field is an empty record, never an inferred list. Records and constructor
values are immutable after construction. No user maps, explicit nulls, floats,
functions, cyclic/shared tables, userdata, threads or metatables are accepted.
Constructors produce private tagged values which cannot be forged by adding
keys to a table. The returned graph is copied and validated before use.

Primitive types used below:

| Name | Exact meaning |
| --- | --- |
| `id` | ASCII `[a-z][a-z0-9_]{0,63}`; case-sensitive |
| `string` | Valid UTF-8, no NUL, at most 65,536 bytes unless specified otherwise |
| `uint` | Lua integer 0 through 2,147,483,647; booleans are not integers |
| `positive` | `uint` greater than zero |
| `bool` | Lua `true` or `false` only |
| `text` | `planar.md(string)` or `planar.file(path)`; see below |
| `list<T>` | `planar.list` of T; order retained unless explicitly declared a set |
| `set<T>` | A list with unique elements; normalized by ID or path byte order |
| `scope` | Nonempty `set<id>` of work IDs, or string `"global"` |

All records below are plain records inside the three top-level constructors.
`required` means no default. Unspecified optional lists default to an empty
list; unspecified optional strings/text/records are absent. Duplicate IDs in a
collection are errors, even when records are equal. References resolve only
within the selected spec/job unless explicitly stated. There is no implicit
name resolution, generated ID or “all roles” wildcard.

`planar.md` preserves bytes, including line endings and trailing whitespace.
`planar.file` takes a workspace-relative file path resolved relative to the
*declaring Lua file's directory*, then confined to the workspace directory.
It captures the exact UTF-8 file bytes, maximum 1 MiB; invalid UTF-8/NUL is an
error in text. It does not execute/include Lua. Path identity and content both
participate in material values. There is no Markdown whitespace normalizer v1.

`planar.immaterial(text)` is accepted only for `notes` and `rationale` fields.
Those fields accept bare text too and are always immaterial. Using the wrapper
in any other field fails, including a mandatory `file` value. Materiality is
chosen by the destination schema, not the source or author. No general-purpose
`material` wrapper exists.

The workspace/spec/job files are all untrusted declaration inputs. They select
IDs from a separately supplied **trusted registry**, which is driver/operator
configuration, never loaded from a declaration-provided location. A workspace
cannot turn its own files into trusted definitions. The driver is responsible
for choosing the registry; render labels this trust boundary. Definitions,
shape implementations, normalizers, selection and projection functions are
trusted Lua shipped by the replacement or configured by the driver. Jobs never
supply Lua callbacks, commands, adapters, templates or registry replacements.

## Paths and one root

A relative file path consists of `/`-separated ASCII segments matching
`[A-Za-z0-9_.-]{1,128}`. Segments `.` and `..` are forbidden, as are
absolute paths, backslashes, empty segments, glob characters and reserved
segments `.git`. A directory boundary has a trailing `/`; a file boundary does
not. Maximum path length is 1,024 bytes. ASCII case-fold-equivalent paths or
segments with different spelling are rejected on every host, even on a
case-sensitive filesystem. This conservative portable grammar is deliberate;
arbitrary Unicode paths/glob overlap are not promised in v1.

There is exactly one source root, identified by a logical ID, and one logical
base label. Host directories are supplied by the driver on the CLI. Workspace
files have the reserved logical root ID `workspace`; the source root cannot
use that ID or reserved `registry` and `observations`. Workspace, source and base directories must be disjoint physical
trees, with no nested trees or directory aliases. When output capture is requested, source and base may represent
equal content but cannot be the same directory. Symlinks and special files are
refused throughout captured trees and declaration traversal, including links
that remain inside the tree. Directories are traversed without following links.
Hard-linked regular files with multiple names inside or across captured trees
are refused. The engine must detect traversal races and identity aliases.

A boundary denotes one file (including a not-yet-existing file), or all files
beneath a directory prefix. Its **canonical stem** is its path with the trailing
`/` removed if present, compared segment by segment under the portable
case-alias rule above. Two boundaries conflict when their stems are equal, or
one stem is a proper segment ancestor of the other. Equal stems conflict even
when one spells a file and the other a directory (`x` versus `x/`). For an
ancestor directory this is ownership overlap; for an ancestor file this is an
impossible file/directory topology (`x` versus `x/y` or `x/y/`). The latter is
refused even if neither path exists. String prefixes without a segment boundary
are disjoint (`x/` versus `xy/`).

Apply this same conflict test to every pair in the union of all work, frozen
and excluded boundaries: within a unit or list, across units (even grouped),
and across boundary classes. Thus each list is nonredundant and neither frozen
nor excluded boundaries can conceal an ownership/topology conflict. Report
`ownership` with both declared paths; physical capture aliases still use the
capture refusal rules. Captured files/directories must also have topology
compatible with declared boundary kinds; an existing regular file cannot be
an ancestor of a declared boundary, and a declared file cannot name an existing
directory. Neither jobs nor roles can add writable paths. All paths outside work boundaries are read-only;
frozen paths additionally appear as explicit user-owned constraints. Excluded
paths are specific generated-artifact boundaries omitted from source/base
observations and all facets. Exclusions are visible material policy, never
implicit ignore-file behavior. `.git/` metadata alone is always excluded;
`.git` as a regular file or link is refused. Other ignored/untracked files are
captured. No ownership grant depends on whether a file currently exists.

## Workspace schema

`planar.workspace { ... }` fields:

| Field | Type | Default / rule |
| --- | --- | --- |
| `version` | integer | required, `1` |
| `id` | id | required |
| `root` | `{id: id, base: id, excluded: set<boundary>}` | required; excluded defaults empty |
| `spec` | file path | required; relative to workspace.lua directory |
| `jobs` | nonempty list of `{id: id, file: file path}` | required; unique IDs/files; files relative to workspace.lua |
| `registry` | `{id: id, revision: id}` | required; exact match to trusted registry header |
| `toolchains` | nonempty set<id> | required; allowed registered toolchains |
| `checks` | nonempty set<id> | required; allowed registered checks |
| `notes` | text or immaterial(text) | absent |

`workspace.lua` is the workspace anchor; `spec` and every listed job are
captured and validated, not just the selected job. Each job's returned `id`
must equal its workspace entry. All use the same spec; there are no includes,
imports/exports, cross-spec bindings, additional source roots or automatic job
discovery in v1. These remain the explicit workspace-composition extension in
the roadmap, rather than silently accepted no-op fields.

## Spec schema

`planar.spec { ... }` fields:

| Field | Type | Default / rule |
| --- | --- | --- |
| `version`, `id` | integer, id | required; version `1` |
| `goal` | text | required, nonempty bytes |
| `work` | nonempty list<work> | required |
| `milestones` | nonempty list<milestone> | required |
| `constraints` | list<requirement> | empty |
| `prescriptions` | list<requirement> | empty |
| `contracts` | list<contract> | empty |
| `facets` | list<facet> | empty |
| `dependencies` | list<dependency> | empty |
| `questions` | list<question> | empty |
| `frozen` | set<boundary> | empty |
| `rationale` | text or immaterial(text) | absent |

The nested record schemas are exact:

- **work**: `id` (required id), `summary` (required text), `paths` (required
  nonempty set<boundary>), `checks` (required nonempty set<check ID>). All work
  is required and assigned once in every job. Units remain role-independent.
- **milestone**: `id`, `work` (nonempty set<work ID>), `acceptance` (nonempty
  list of `{id: id, text: text}`), all required. Acceptance IDs are unique
  within the milestone. A unit must belong to at least one milestone. Each
  criterion applies to every unit listed in that milestone; use separate
  milestones for different scope. Shared criteria are rendered once with IDs.
- **requirement**: `id`, `scope`, `text`, all required. Constraint and
  prescription IDs share one namespace. Both are mandatory; the separate names
  organize prose and confer no different authority.
- **contract**: `id`, `producer` (work ID), `consumers` (nonempty set<work ID>),
  `expected` (text), `conformance` (check ID), `review_note` (text), all required.
  Producer cannot be a consumer. The conformance check is mandatory for the
  producer and for every consumer, irrespective of their work.checks lists.
  `review_note` records the spec author's account of interface review; it is
  visibly a declaration assertion, not an authenticated approval. The expected
  interface is available as captured specification data before output exists.
  A v1 contract kind is `text` implicitly: exact expected bytes and a registered
  conformance descriptor with stated coverage. No inferred ABI/schema guarantee.
- **facet**: `id`, `producer` (work ID), `paths` (nonempty set<file path>), all
  required. Each path must be inside its producer's boundary; no directory
  expansion in facet declarations. Kind and normalizer are `files` and
  `exact_bytes_v1` implicitly. Missing files mean unavailable, never empty output.
  Interface contracts are declarations; facet **values** are observed
  implementation data. The facet definition is material subscription policy,
  including for completion-only dependencies and without any captured value;
  normalization makes implicit kind and normalizer explicit.
- **dependency**: `id`, `consumer` (work ID), `source` (`"contract"` or
  `"facet"`), `ref` (ID in that source namespace), `phase` (`"start"` or
  `"complete"`), `quality` (`"available"`, `"done"`, `"accepted"`), all
  required. Consumer must be declared in a referenced contract's consumers.
  Facet producer must differ from consumer. Legal combinations are below.
- **question**: `id`, `scope`, `prompt` (text), all required; `blocks` defaults
  `"none"` (enum `"none"`, `"start"`, `"complete"`); `answer` (text) absent.
  An unanswered scoped question reports the selected phase as blocked. An
  answer is material clarification, not output approval. Answers are spec
  assertions; driver observations cannot answer a question or remove a blocker.

Every reference must resolve, even if outside the selected role. Contract,
facet, dependency and question IDs each have their own namespace. No arbitrary
subscription list exists: dependencies are the complete required subscriptions;
applicable contracts include every contract produced or consumed by assigned
work, even without a dependency. A consumer contract must have at least one
`start/contract/available` dependency, preventing accidental omission from its
development inputs. A completion integration obligation is **explicit**: add a
`complete/facet/done` or `complete/facet/accepted` dependency and the consuming
check's `output_vector` inputs. Contract availability alone never promises
integration. Required check coverage is described in the registry, not guessed.

## Dependencies, grouping and shapes

| Source | Phase | Quality | Interpretation |
| --- | --- | --- | --- |
| contract | start or complete | available | Captured expected interface is present |
| facet | start or complete | available | Captured files are present; no claim of producer completion |
| facet | start or complete | done | Later execution requires producer passing checks for exact output |
| facet | start or complete | accepted | Later execution requires exact producer output approval |

All other combinations fail. Without output capture, facet availability is not_captured, distinct from a
captured missing file. Render never derives `done` or `accepted` from
files or driver assertions; those conditions are reported `unverified`.
Unavailable contract text is a capture error. Missing facet files are a
reported prerequisite/output absence, not a successful empty facet.

Validation constructs the complete phase graph for each job **before role
scoping**. Vertex labels are `work:<id>:start`, `work:<id>:complete`,
`work:<id>:accepted` for every unit; `role:<worker ID>:complete` for every
worker; and `role:<reviewer ID>:start`, `role:<reviewer ID>:complete` for the
reviewer. There are no other vertices. Add all of these edges:

- Each unit.start → unit.complete → unit.accepted.
- Each unit.complete → its assigned worker-role.complete.
- Every worker-role.complete → reviewer.start.
- reviewer.start → reviewer.complete.
- reviewer.complete → unit.accepted for every targeted unit.
- Each explicit done dependency: producer.complete → consumer.phase;
  each accepted dependency: producer.accepted → consumer.phase.
- Each pipeline predecessor obligation below: producer.complete → consumer.start.

Available contract edges and observed available facets do not wait for producer
completion. Missing available facets are external availability blockers, not
scheduling edges. Questions report external blockers without adding vertices.
No feedback/repair edge is added. Reject any directed cycle in this fully
expanded graph with its unit/role/phase trace; scoping cannot remove a cycle.
For example accepted producer `a` → start of consumer `b` necessarily closes
`work:a:accepted → work:b:start → work:b:complete → role:b_worker:complete
→ role:review:start → role:review:complete → work:a:accepted` when both share
that reviewer. The reviewer start-to-complete edge is essential to this trace.

Within a grouped worker, a complete/done dependency remains a local unit
completion obligation, with its conformance and integration checks; it never
becomes “wait for my role to be done.” A start/done dependency within a group
requires producer-unit completion before consumer-unit start. Packet sections
show this internal order, even though render executes nothing. An accepted
edge involving units reviewed together can create a review barrier cycle and
must be rejected, including a cross-role `start/accepted` whose shared reviewer
waits for that consumer. Merely ignoring all edges whose endpoints share a
role is incorrect. Two mutually complete/done edges remain a cycle. A legal
contract-first pair has start/contract/available and complete/facet/done: both
units can develop, and producer completes before consumer's integration.

`planar.job { ... }` fields:

| Field | Type | Default / rule |
| --- | --- | --- |
| `version`, `id` | integer, id | required; version `1` |
| `shape` | `"pipeline"` or `"fanout_review"` | required |
| `roles` | nonempty list<role> | required |
| `stages` | list<nonempty set<worker role ID>> | required for pipeline; forbidden for fanout_review |
| `max_rounds` | uint | default `2`; future reject-and-repair cap, not a render loop |
| `budget_bytes` | positive | default `32768`; at most 1,048,576 |
| `notes` | text or immaterial(text) | absent |

**role** fields: `id` and `kind` (`"worker"` or `"reviewer"`) required;
`work` (set<work ID>) required/nonempty only for workers; `targets`
(set<work ID>) required/nonempty only for reviewer; the opposite field is
forbidden, not empty. `checks` (set<check ID>, default empty) adds required
checks. `budget_bytes` (positive, same cap) inherits job budget. `preferences`
is optional `{model: string, session: string}`, both optional; each max 256
bytes, empty record permitted. These are driver preferences and immaterial.
No arbitrary model parameters or session commands in v1.

Both shapes require exactly one reviewer, targeting **all** work exactly once,
and at least one worker. The reviewer owns no source paths; future verdict
outputs live outside the source root. Its checks field must be empty in v1;
its packet lists the checks required of targets, without claiming to run them.
For each work unit `u`, derive `effective_checks(u)` as the sorted unique
union of `u.checks`, conformance checks of every contract it produces or
consumes, and `checks` additions on **u's assigned worker role**. An addition
applies to every unit on that worker. Worker mandatory definitions are the
union of those sets, but applicability remains keyed by work ID. Reviewer
context uses exactly the same effective set for each target, including the
owning worker's additions; it does not derive additions from the reviewer.
Moving an added check between workers changes the affected per-work policy
even if the union of definitions stays equal.

Each unit receiving an output_vector check must have at least one complete
facet dependency; a role addition incompatible with any assigned unit fails.
Every selected check/toolchain must be allowed by the workspace and exist in
the trusted registry. Roles cannot
remove required checks or weaken separation.

`fanout_review` adds no worker ordering beyond dependencies. `pipeline` lists
each worker exactly once in ordered stages. For each pair of adjacent stages,
form every pair of producer unit in the earlier stage and consumer unit in the
later stage. Each pair creates a derived **pipeline predecessor obligation**:
`{id, source:"pipeline", producer, consumer, producer_role, consumer_role,
phase:"start", quality:"done"}`. The ID is exactly
`pipeline:<producer work ID>:<consumer work ID>`; `:` is outside declared ID
grammar, so it cannot collide with a dependency ID. Role IDs are taken from
the full assignment map. These obligations are distinct from explicit facet
subscriptions and imply no facet, output vector or extra check. Completion of
all immediately preceding units is required; transitive ordering follows the
full graph. Stages can contain independent workers.

Add all derived edges before cycle detection. Retain each obligation whose
consumer is selected in that role's W, prerequisites and mandatory dependency
instructions, including predecessor IDs and assignments even without a data
dependency. A reviewer selects all targeted consumers and therefore receives
all pipeline obligations. Do not filter stages first and then derive edges:
that would lose predecessors or create false adjacency. Reviewer runs after
all workers through the phase graph. Pipeline ordering is material policy.
Both shapes retain all explicit unit obligations. They describe finite
prerequisites only; the external driver schedules sessions.

## Trusted registry v1

The driver selects a registry through `--registry`; it is a UTF-8 JSON file,
not executable job Lua. Exact root keys are `version:1`, `id`, `revision`,
`toolchains` (array), `checks` (array). IDs obey the declaration ID grammar.
Every toolchain record has exactly `id`, `description` (string), `command`
(nonempty array<string>, first element executable), `environment` (array of
`{name:string,value:string}`; unique names `[A-Z][A-Z0-9_]*`). Environment policy
is replacement with these exact values, never inherited implicitly; executable
resolution in future execution uses this environment's PATH or an absolute
executable path. Registry strings have the primitive string bounds.

Each check record has exactly `id`, `toolchain` (toolchain ID), `argv`
(array<string>, appended to toolchain.command), `coverage` (nonempty string),
`inputs` (nonempty set of `"expected_contracts"`, `"own_output"`,
`"output_vector"`), `timeout_ms` (positive), `max_output_bytes` (positive),
`selection` (`"nonempty"` or `"not_applicable"`). `nonempty` requires a future
adapter to report positive matched-test count; `not_applicable` is for checks
that are not test selectors and must explicitly say so in coverage. No check
executes during render. Limits are at most 3,600,000 ms and 16,777,216 output
bytes. No check arguments can be overridden by declarations in v1; add a new
trusted descriptor for a different invocation. `output_vector` means exact own
output plus every complete/facet dependency producer output, sorted by work ID;
a check requesting it must have at least one such dependency. Every complete
facet dependency requires at least one consuming check with output_vector.
Contract conformance descriptors must include expected_contracts and own_output.
Selected registry records, not unrelated records, enter material identities.
Registry revision is provenance; definition bytes and interpreter versions,
not merely an author-chosen revision name, determine check semantics.

## Small combined/split example

These declarations are illustrative R02 schema examples; full quota fixtures
and expected packets are R03 work. All `L` aliases below mean `planar.list`.
Both jobs use exactly the same spec bytes and workspace base.

```lua
local p = planar
local L = p.list
return p.workspace {
  version = 1, id = "quota",
  root = { id = "source", base = "initial", excluded = L {} },
  spec = "spec.lua",
  jobs = L { { id = "combined", file = "combined.lua" },
             { id = "split", file = "split.lua" } },
  registry = { id = "quota_tools", revision = "v1" },
  toolchains = L { "python" }, checks = L { "contract", "integration" }
}
```

```lua
local p = planar
local L = p.list
return p.spec {
  version = 1, id = "quota", goal = p.md "Provide an atomic quota and CLI.",
  work = L {
    { id = "library", summary = p.md "Implement try_take.",
      paths = L { "src/core/", "tests/core/" }, checks = L { "contract" } },
    { id = "command", summary = p.md "Implement quota CLI.",
      paths = L { "src/cli/", "tests/cli/" }, checks = L { "integration" } }
  },
  milestones = L {
    { id = "api", work = L { "library" },
      acceptance = L { { id = "atomic", text = p.md "A failed take changes no balance." } } },
    { id = "cli", work = L { "command" },
      acceptance = L { { id = "exit", text = p.md "Exhaustion returns exit status 2." } } }
  },
  constraints = L { { id = "safe", scope = "global", text = p.md "Never permit a negative balance." } },
  contracts = L { { id = "quota_api", producer = "library", consumers = L { "command" },
    expected = p.file "contracts/quota.md", conformance = "contract",
    review_note = p.md "Spec author records operator review of this interface." } },
  facets = L { { id = "library_files", producer = "library", paths = L { "src/core/quota.py" } } },
  dependencies = L {
    { id = "develop", consumer = "command", source = "contract", ref = "quota_api",
      phase = "start", quality = "available" },
    { id = "integrate", consumer = "command", source = "facet", ref = "library_files",
      phase = "complete", quality = "done" }
  },
  frozen = L { "contracts/" }, rationale = p.immaterial(p.md "Keep the example small.")
}
```

The `expected` file above is `contracts/quota.md` in the **workspace**. The
frozen boundary is `contracts/` in the **source** tree; these are different
logical paths. For a single authoritative interface, keep the source directory
absent or use a read-only driver copy; a second copy is not loaded implicitly.

```lua
local p = planar
local L = p.list
return p.job {
  version = 1, id = "combined", shape = "fanout_review",
  roles = L {
    { id = "builder", kind = "worker", work = L { "library", "command" } },
    { id = "review", kind = "reviewer", targets = L { "library", "command" } }
  }
}
```

```lua
local p = planar
local L = p.list
return p.job {
  version = 1, id = "split", shape = "pipeline",
  stages = L { L { "library_worker" }, L { "cli_worker" } },
  roles = L {
    { id = "library_worker", kind = "worker", work = L { "library" } },
    { id = "cli_worker", kind = "worker", work = L { "command" } },
    { id = "review", kind = "reviewer", targets = L { "library", "command" } }
  }
}
```

For contract-first parallel development use the same split roles with
`shape = "fanout_review"` and omit stages. The pipeline intentionally delays
CLI start until library completion; the spec still permits contract-first work.
Combined has one internal library-complete → command-complete obligation, two
unit outputs and one worker output union; split resolves that obligation
across workers. Neither mapping drops integration or introduces a self-wait.
The reviewer receives both units' requirements and exact target vector.

## Rationale and extension boundary

Typed lists remove Lua's ambiguous empty tables. File/directory boundaries make
exclusive ownership decidable. Exact-byte facets avoid unreviewed semantic
normalizers. Explicit local completion obligations preserve contract-first
work when units are grouped. One all-work reviewer keeps shape validation
bounded; multiple review groups and arbitrary workflow shapes need a later
version, not a job callback. Multi-root/spec bindings and live accepted-state
semantics remain deferred as planned. No mandatory first-slice capability is
intended to be removed by these API choices; mismatches found during fixture
review must be resolved before native implementation.
