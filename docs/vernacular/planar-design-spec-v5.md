# Planar design spec v5

2026-10-20 · Manuel

## Scope

Planar turns a Lua specification into per-role Markdown, compares declared inputs to identify work that needs attention, and records deterministic check results against the exact work observed. It never calls a model or runs a scheduling loop. The driver runs sessions, supplies checkouts, handles process interruption, and decides when to invoke Planar.

Planar emits prompts, the harness thinks, Planar validates what came back.

The product is useful even if ordinary Markdown files perform equally well in a compaction experiment. Scoped context, visible requirements, contract-first parallel work, mechanical prescriptions and explainable invalidation are independently useful. Compaction, grouping and model comparisons are separate, later experiments.

The first implementation is render-only. Persistence and finite check, claim and review operations follow after evaluating the rendered fragments. Immutable source archives, read-only verification mounts, authenticated human approvals, session supervision and OS isolation are optional adapters or future products; none is required to render useful context.

Revision note: v5 takes v4 (2026-10-19) as its text and folds in the v3 Claude Doc and the rulings of 2026-10-20. Rulings that supersede v4 where they differ: workspaces point at root paths wherever the driver put them and no checkout ever lives under `~/.planar`; the toolchain is a driver-supplied trusted registry allow-listed by the workspace (adopted from the implementation on 2026-10-21); Planar never creates or manages worktrees; the full adversarial matrix is the cut-2 gate; xxHash stays, with exact-byte comparison before any digest-based reuse; run tokens, generations and fencing are in cut 2 because 24–48 hour jobs die mid-run as a matter of course.

## Glossary

| Term | Meaning |
| --- | --- |
| Engine | The C++ core: Lua sandbox, canonical hash, confined file primitives, bounded subprocesses, conditional atomic ledger writes. Owns only what a job must never redefine. |
| Stdlib | Trusted Lua shipped with Planar: schemas, shapes, projections, normalizers, reconciliation, check definitions, prompt templates, verbs. Sees the full engine API. |
| Job tier | Harness-written Lua (`spec.lua`, `job.lua`) that sees only the curated `planar.*` surface. |
| Workspace | A directory holding one `workspace.lua`, its spec, jobs, contracts, docs and state. Named by path, or by bare name under `PLANAR_WORKSPACE_DIR` (by convention `~/.planar/workspaces`). It declares a root logically; it never contains a checkout. |
| Root | A logical ID and base label in the workspace, bound to a host directory by the driver on each invocation. Created and managed by the driver, never by Planar. |
| Driver / orchestrator | Whatever runs sessions and calls Planar: a person, a shell loop, Claude Code, Centurion. Bring-your-own. |
| Spec | `spec.lua`: goal, work units and milestones, constraints, prescriptions, contracts, work-scoped layout, questions, imports and exports. Names no roles. |
| Work unit | A stable, spec-defined unit of work with acceptance criteria, dependencies and a path boundary. The smallest assignable thing. |
| Job | `job.lua`: a shape, roles, the assignment of work units to roles, subscriptions, check selections, budgets, model preferences. Several per spec, one per grouping or arm. |
| Shape | A hand-written workflow rule set (`pipeline`, fan-out then review, …) a job parameterizes. No internal session loop. |
| Role | A job-defined unit of context and state: assigned work units and derived owned paths. The unit of state in the ledger. |
| Milestone | A deliverable with `done_when` acceptance criteria and dependencies on other milestones. |
| Facet | A named observable output a work unit maintains, such as an interface or a diff. |
| Material / immaterial | Whether an input participates in a work fingerprint. Requirements, contracts, ownership and required check policy are always material; rationale and checkpoints may be immaterial. |
| Observation digest | Hash of the exact bytes, paths, modes and types Planar read. What did Planar read? |
| Semantic digest | Hash of the normalized value plus facet kind and normalizer version. Did a subscribed meaning change? |
| Work fingerprint | Hash of a role's mandatory inputs and work revision. Does this role need to reconsider its work? |
| Render digest | Hash of the exact emitted fragment bytes. Did the context supplied to the session change? |
| Output identity | The exact observed output manifest or driver-supplied immutable revision a check or review was made against. Separate from the work fingerprint. |
| Projection | A stdlib function from a captured input manifest, role and labeled observations to a fragment. Pure; no I/O while assembling text. |
| Fragment | The rendered Markdown a session receives. A render, not a stored artifact; retained with its manifest once dispatched. |
| Recovery packet | Fragment, input manifest, run token, latest check results and review findings, session checkpoint. What a restarted session gets. |
| Checkpoint | Bounded, untrusted session notes: progress, next step, unresolved work. Changes rendered context, not work freshness. |
| Prescription | A rule with a scope (work units) and a check; enforced at the done gate. |
| Contract | A reviewed expected interface with an explicit conformance check, binding a facet. Hand-written before ingest. |
| Dependency | An edge between work units with a phase (`start` or `complete`), a source (`contract` or `facet`) and a required quality (`available`, `done`, `accepted`). |
| Layout | Ownership by boundaries: the union of a role's work `paths`, plus `frozen` (user-owned) and `excluded` paths, with a precise conflict test. No separate artifact. |
| Boundary gate | The check that a role's final diff touched only paths it owns. |
| Toolchain | An entry in the driver-supplied trusted registry naming how to build, test and lint; the workspace allow-lists IDs and checks reference them by ID. |
| Check | A trusted descriptor selected by ID with validated arguments. Zero tests, missing tools, timeouts and malformed results are failures. |
| Inbox | A driver convention, not a Planar directory: a place sessions leave proposals and checkpoints for the driver to forward in `--observations`. Editing a proposal never changes authority. |
| Out | Likewise a driver convention for session outputs; Planar receives them only through the driver (`--observations` now, verdict events from R18). |
| Question | A spec entry that blocks named work units until answered. |
| Import / export | A spec's declared needs and offers, by local name, each with a contract. Bound facet-to-facet in `workspace.lua`. |
| Binding | The only cross-spec edge: an import bound to another spec's export, with compatible contract identity and phase/quality. |
| Ledger | `state/ledger.db`: accepted declaration revisions, run identities, input manifests, dispatched packets, check evidence, review events, state transitions. |
| Revision | An accepted, validated declaration set activated by `load`. Chronological ID and parent; files are drafts until accepted. |
| Run | An ownership record: which session currently holds a role, under which request ID and token. Not a role state. |
| Token / generation | Proof of run ownership; the generation advances on takeover or structural change, fencing older tokens' ledger writes. |
| Fencing | Refusing ledger mutations from a superseded token. Fences the ledger, not the process or the files. |
| States | `blocked`, `ready`, `running`, `done`, `stale`, `retired`. |
| Reconcile | The deterministic pass that compares identities to the ledger and assigns states. No model involved. |
| Verdict | A reviewer's schema-validated event: target output vector, policy revision, approve or reject, findings, named repair targets. |
| Accepted | Derived: every required worker has applicable passing checks, the current output vector has a valid approval under current policy, no acceptance blocker remains. Distinct from `done`. |
| Reverse prompt | A prompt Planar emits for the harness to act on (`init`, `ingest`, a load rejection). |
| Reference orchestrator | The orchestrator fragment rendered per job: shape semantics, legal CLI actions, driving prescriptions. Advice, never a requirement. |
| Arm | One variant in an experiment: a job file, a model, an isolated view and ledger identity. |
| `impact` | Reconciler dry run of a captured draft against the active revision: the blast radius before accepting. |
| `explain` | Why a role is in its state: which identities moved, and whether raw or semantic. |
| Session | An LLM run performing one role against one fragment. Not a Planar concept beyond inbox, out and checkpoint. |

## Architecture and trust

Three tiers, one direction of authority. The engine owns what a job must never redefine; trusted Lua owns everything else.

> Figure: three tiers — job tier (harness-written Lua, sees only `planar.*`) → stdlib tier (trusted Lua, all semantics) → engine (C++26 with modules; sandbox, canonical hash, confined reads, bounded subprocesses, atomic ledger). Each tier sees only the one below it.

| Tier | Owns | Can access |
| --- | --- | --- |
| Job | Spec, role assignments, shape parameters, selected checks | Curated constructors and captured file references |
| Stdlib | Schemas, shapes, projections, normalizers, reconciliation, check definitions, prompt templates, verbs | Engine primitives |
| Engine | Lua isolation, canonical serialization and hashing, confined file primitives, bounded subprocesses, conditional atomic ledger writes | Host services required by those primitives |

A job cannot replace the stdlib, define a projection, run a subprocess, or write the ledger. The stdlib is trusted: a wrong normalizer or an inadequate check is a product bug even when the engine transaction is correct.

Engine confinement applies to its own file operations. A build command is a subprocess with the driver's OS permissions; restricting `engine.write` does not confine that process or an agent. The default execution contract assumes cooperative sessions, a trusted driver, and source files held stable during checks. Stronger enforcement is an adapter with explicitly advertised guarantees.

Engine module layout, each a few hundred lines; the sandbox and the ledger are the only two with real design in them:

```
engine/
  sandbox.cpp    Lua VM; two environments (stdlib, job); curated-table construction; load_declaration with memory and instruction limits
  canon.cpp      versioned canonical encoding of a Lua value + XXH3_128 hash; golden vectors
  fs.cpp         confined captured reads, glob, exists; write_state restricted to the workspace's state/
  git.cpp        git_observe (diff against base, head)
  exec.cpp       check_id + argv + bounded time, output, cwd, environment; every call logged
  ledger.cpp     read; commit(expected, changes) with invariants: current generation and token, expected revisions, unique request and event IDs, atomic write
  emit.cpp       fragment / prompt / diag / status, typed errors and durable operation IDs
  main.cpp       one positional, stdlib bootstrap, dispatch to stdlib.commands[verb]
```

Everything is `std::expected<T, Error>` outward; the Lua bindings are one thin layer converting those to `value, err`. Verbs are registered in Lua, so experimental ones live in `stdlib.d/` and never touch the engine.

## Declared model

`workspace.lua`, `spec.lua` and `job.lua` return versioned data through constructors. Unknown fields, unresolved references, duplicate IDs, invalid scope and unsupported schema versions are load errors with file, line where available, field path and a corrective message.

| Artifact | Meaning |
| --- | --- |
| Workspace | Roots, toolchain, specs, active jobs, cross-spec bindings |
| Spec | Goal, stable work units and milestones, constraints, prescriptions, contracts, work-scoped layout, questions, imports and exports |
| Job | Shape, role definitions, work-to-role assignments, model and session preferences, subscriptions, check selections, budgets |
| Shape | Trusted workflow rules such as fan-out followed by review; no internal session loop |
| Role | A job-defined unit of context and state with assigned work units and derived owned paths |
| Facet | A named observable output, such as an interface or a diff |
| Contract | Reviewed expected interface plus an explicit conformance check |
| Toolchain | A user-written workspace declaration, keyed by root, naming how to build, test and lint |
| Ledger | Observed state, run identities, input manifests, check evidence and review events |
| Inbox | Session proposals; editing a proposal never changes authority |

### Work belongs to the spec; roles belong to the job

The spec defines stable work units, their milestone acceptance criteria, dependencies, contracts and path boundaries. It does not name worker roles or models. `layout.tree` owners, prescription scopes, question scopes, contract producers and consumers and exported facets refer to work-unit IDs (or an explicit global scope), never job role names. Frozen user-owned paths are outside worker ownership.

The job defines roles and assigns work units to them. Each required work unit has exactly one implementing role in an active job; a role may implement several units; several roles may cover distinct units under one milestone. This is what makes different groupings possible without editing the spec. The smallest assignable unit is a declared work unit: splitting one between simultaneous writers means refining its boundaries, never duplicating ownership. Redundant trials of one unit are separate arms with separate views and runs, not shared writers in one job.

Requirements, contracts, questions, mandatory checks and owned paths derive from the union of a role's assigned work. Jobs may add checks or stricter policy but cannot drop requirements, leave required work unassigned, grant paths outside declared boundaries or weaken mandatory review separation. Validation rejects duplicate assignments, unknown units, ambiguous path ownership and incompatible shapes.

Dependencies are between work units. When a job groups both endpoints into one role, Planar keeps the contract and completion obligations without creating a self-wait on that role's own `done`. Cross-role dependencies resolve through the assignment map; an unsatisfiable scheduling cycle is rejected with the affected units and roles named. A reviewer role targets work units and cannot implement the work it approves.

```text
-- one spec: work units library and command, library -> command dependency
job together: builder = [library, command];      reviewer reviews [library, command]
job split:    library_worker = [library]; cli_worker = [command]; reviewer reviews [library, command]
```

The v1 constructors, abbreviated; `declarations.md` in the repository is the exact schema and wins on any difference:

```lua
-- spec.lua
return planar.spec {
  version = 1, id = "quota",
  goal = planar.md [[ A token-bucket quota library plus a CLI to exercise it. ]],
  rationale = planar.immaterial(planar.file "docs/prd.md"),
  constraints = planar.list { planar.md [[ No exceptions on the hot path. ]] },
  work = planar.list {
    { id = "library", paths = planar.list { "src/core/" },
      acceptance = planar.md [[ try_take is atomic; refill math is unit-tested. ]],
      checks = planar.list { "core_tests" } },
    { id = "command", paths = planar.list { "src/cli/" },
      acceptance = planar.md [[ rl --cap --rate --take --times prints allowed/denied. ]],
      checks = planar.list { "cli_tests" },
      dependencies = planar.list {
        { on = "library", phase = "start",    source = "contract", quality = "available" },
        { on = "library", phase = "complete", source = "facet",    quality = "done" } } },
  },
  prescriptions = planar.list {
    { id = "P1", scope = planar.list { "library", "command" },
      rule = planar.md [[ Use fmt; no <iostream> or <cstdio>. ]], check = "no_iostream" } },
  contracts = planar.list {
    { id = "bucket_api", producer = "library", consumers = planar.list { "command" },
      expected = planar.file "contracts/bucket.hpp", conformance = "bucket_conforms",
      review_note = planar.md [[ Reviewed 2026-10-18. ]] } },
  facets = planar.list {
    { id = "library.api", owner = "library", paths = planar.list { "src/core/bucket.hpp" } } },
  frozen = planar.list { "contracts/" }, excluded = planar.list { "build/" },
  questions = planar.list {
    { id = "Q1", text = planar.md [[ Clamp or reject negative --times? ]],
      scope = planar.list { "command" }, blocks = "complete" } },
}

-- jobs/split.lua
return planar.job {
  version = 1, id = "split", shape = "fanout_review",
  roles = planar.list {
    { id = "library_worker", kind = "worker" },
    { id = "cli_worker",     kind = "worker" },
    { id = "review",         kind = "reviewer" } },
  assignments = planar.list {
    { work = "library", role = "library_worker" },
    { work = "command", role = "cli_worker" } },
  subscriptions = planar.list { { role = "cli_worker", facet = "library.api" } },
  budget_bytes = 32768, max_rounds = 2,
}
```

Rules the schema enforces: every collection is `planar.list` (a bare `{}` is ambiguous in Lua); `planar.file` is workspace-confined and capped at 1 MiB; `planar.immaterial` is accepted only on `notes` and `rationale`, everything else is material by construction; a bare `{}` where a list is required, an unknown field, or an ID that resolves to nothing is a load error with file, field path and message.

**Ownership is boundaries, not a tree.** A role owns the union of its assigned work units' `paths`; `frozen` paths are user-owned and excluded from every session's writes; `excluded` paths are outside the observed tree entirely. Paths use a conservative portable grammar, and the loader refuses any pair that conflicts under the precise test (`x` against `x/`, `x/y`, `x/y/`; a work path against a frozen or excluded one), including when the files do not yet exist. Distinct siblings and mere string prefixes (`x/` against `xy/`) are legal. There is no separate layout artifact.

A job may declare model and session preferences and budgets per role; the driver resolves and executes them and records the actual configuration. Model identity alone does not change a requirement or invalidate accepted evidence, but requested and actual configuration belong in experiment provenance. Changing grouping changes role generations; a reused role name inherits no run token or approval.

### Prose and materiality in declarations

`md [[ ... ]]` holds inline prose; `file "docs/goal.md"` captures a file relative to the declaring document. A file reference inherits the materiality of its destination field: a mandatory constraint in a Markdown file is material; rationale and background may be immaterial. Enforced conventions belong in constraints or prescriptions, not in an immaterial style-guide field.

Schemas refuse `immaterial` around mandatory requirements, contracts, ownership, required subscriptions or required check policy. Functions, cyclic tables, metatables, mixed map/array structures and unsupported numeric values cannot become declaration values.

### Shapes

Shapes are few and hand-written; the harness selects and parameterizes, never authors workflow code. A refusal to express a real workflow is the evidence for a new shape.

| Shape | Status | Meaning |
| --- | --- | --- |
| `pipeline` | v1, implemented | Explicit `stages`, each an ordered list of roles; a stage completes before the next starts, and that obligation is derived from the declaration, never invented by filtering |
| `fanout_review` | v1, implemented | Parallel workers, one reviewer at fan-in targeting all work |
| `review_loop` | after R18 | Per-worker review rounds; relaxes the one-reviewer rule once verdict events exist |
| `tournament` | cut 3 | The same work unit given to N isolated arms (models or groupings) with byte-identical context; a judge ranks the arms' output vectors with identities stripped; the winner's output becomes the facet downstream depends on. The usable form of "redundant trials are separate arms, never shared writers" |
| `spike_then_implement`, `bug_hunt`, `refactor_lockstep`, `migration` | deferred | when the generic shapes have hurt on a real job |

v1 topology: exactly one reviewer role per job, targeting all work, with no write scope. `max_rounds` defaults to 2 and `budget_bytes` to 32 KiB per role; both are job fields.

### Where the four planning documents land

| Document | Where it lands | What Planar does with it |
| --- | --- | --- |
| PRD | `goal`, `out_of_scope`, `rationale = file "docs/prd.md"` (immaterial) | Goal in every fragment; rationale only to roles needing intent |
| Tech spec | `constraints`, `prescriptions`, `contracts`, `layout`, work-unit facets; the rest as an immaterial file | Enforceable parts become gates; interfaces become contracts |
| Testing spec | `done_when` per milestone, selected check IDs, a testing philosophy file for the reviewer and test roles | Acceptance becomes the done gate |
| Roadmap | `milestones` and work-unit dependencies with phase and quality | Order is the DAG; `render roadmap` shows topological layers |

For the first job, keep writing all four as Markdown and let `planar ingest` take them as inputs to the spec prompt; which lines the harness turns into gates and which it leaves as prose is the test of this table.

## Identity and materiality

Four identities answer four different questions; conflating them is the source of most wrong wakes and wrong approvals.

| Identity | Contents | Question |
| --- | --- | --- |
| Observation digest | Exact source bytes, paths, modes and types for an observation | What did Planar read? |
| Semantic digest | Normalized value, facet kind, normalizer identity and version | Did a subscribed meaning change? |
| Work fingerprint | Role-scoped mandatory inputs and work revision | Does this role need to reconsider its work? |
| Render digest | Exact emitted UTF-8 fragment bytes | Did the context supplied to the session change? |

An **output identity** names the exact observed output manifest or driver-supplied immutable revision used for checking and review. It is separate from the worker's work fingerprint: changing implementation without changing instructions must invalidate old check and review evidence. It does not require Planar to retain all source bytes.

A role's work fingerprint includes:

1. Its goal, assigned work-unit IDs and their milestone acceptance criteria.
2. All global mandatory constraints and all prescriptions applicable to its assigned work units.
3. Applicable contracts, declared ownership, and root and base identities.
4. Subscribed material semantic values used for development, dependency phase and required quality. A completion-only dependency contributes its policy to work identity and its exact output combination to completion evidence; it need not restart source work when only that combination changes.
5. Required check definitions, the toolchain entries they reference, and compatibility versions for their semantics.
6. Material question answers and any admitted review-repair revision.

It excludes unrelated milestones and prescriptions, the role's own output bytes, timestamps, session and model names, optional rationale, and purely visual projection changes. A rendering change moves the render digest; a change to which requirements apply moves the work fingerprint through the input-selection version.

Raw source hashes are provenance, outside the semantic digest; otherwise whitespace-only changes would wake consumers through embedded raw hashes. Normalizers are versioned and conservative: source code uses byte identity unless a documented parser-backed normalizer exists. A normalizer change causes explicit semantic re-evaluation, never an unexplained global wake.

Canonical serialization is a versioned, typed encoding: distinct map and array tags, sorted UTF-8 map keys, length-prefixed strings, ordered arrays, explicit booleans, signed integers in the supported range, and rejection of NaN, infinity, sparse arrays and ambiguous numbers. `nil` is absence; an explicit null needs a schema-defined sentinel. Prose normalization happens before hashing only where the schema allows it; hashing never collapses meaningful whitespace in code or contracts. Golden vectors pin encoding bytes and XXH3_128bits results.

The hash is a pinned xxHash implementation, seed zero, canonical 128-bit byte representation, with hash and encoding versions recorded explicitly. xxHash is non-cryptographic, so a digest is an index, not an identity: before reusing a stored leaf or node by digest, compare the exact encoded payload, and refuse with an explicit collision error if it differs. One unit test exercises that path. These identities support local change detection and history lookup, not tamper-proof certification.

## Fragments and recovery context

A fragment is a deterministic view of a captured input manifest, a role, a projection version and labeled observations. No projection performs fresh I/O while assembling text. Maps are traversed through sorted helpers; Lua `pairs` order cannot affect emitted bytes. Volatile time and process data are omitted or placed in a separate transport envelope.

> Figure: projection — solid inputs (assigned work in scope; subscribed semantic values; ownership, checks, toolchain) and dashed optional inputs (rationale, checkpoint) flow into `projections.worker`, which emits the fragment. Work fingerprint = hash of the solid inputs; render digest = hash of the fragment bytes.

A worker receives the goal, its work units and acceptance criteria, applicable requirements and check names, required contracts, subscribed upstream content, its exact boundary, current blockers, and the toolchain entries it needs. It does not receive unrelated work. A reviewer receives the relevant intent and requirements, exact output targets, their diffs, check summaries and unresolved findings. A driver receives state and transition guidance, never worker source as a substitute for independent review.

The minimum recovery packet is:

- The current fragment and input manifest, including the acknowledged work fingerprint and render digest.
- The current run token, or an explicit statement that no run is active.
- The latest relevant check results and current review findings, each labeled with its target identity.
- A bounded session checkpoint supplied by the driver or session: progress, next step, unresolved work, referenced files.
- The actual current work in the driver's checkout, whose identity Planar re-observes.

Re-rendering reconstructs declared context. It cannot reconstruct private reasoning or unwritten progress. Checkpoints are untrusted session notes and do not prove completion; a new checkpoint changes rendered context, not work freshness.

Upstream text, verdict notes and checkpoints are visibly delimited as data. The projection states their provenance and does not let embedded instructions change role authority. This reduces confusion; it is not a complete defense against prompt injection in a model.

Budgets are per role and shape. Required context is never silently truncated: rendering reports section sizes and refuses with an actionable budget diagnostic if mandatory content cannot fit. Optional rationale may be omitted under a declared priority order with an omission notice. Byte counts are exact; token counts name the tokenizer or say they are estimates.

Every dispatched fragment, its input manifest and referenced recovery checkpoint are retained in SQLite, linked to its declaration revision and execution event. `state/recovery/` may cache the latest packets; it is not the history authority. `render --since <render_digest>` compares retained renders and reports material and render-only changes separately. A preview that was never dispatched need not be retained and may return `history_unavailable`; committed run history is not pruned automatically.

## Contracts and dependencies

Contracts let development start before a producer has completed. Availability and quality are explicit rather than overloaded into one `requires` edge.

| Requirement | Effect |
| --- | --- |
| Contract available for development | Consumer may start against the reviewed expected interface |
| Producer done required for completion | Consumer cannot complete until actual producer output passes required checks |
| Producer accepted required for downstream work | Rejection or missing approval blocks that phase |
| Integration check | Checks the exact selected producer and consumer output combination |

Each dependency declares its phase (`start` or `complete`), source (`contract` or `facet`) and required quality (`available`, `done` or `accepted`). The loader rejects invalid combinations, missing producers and deadlocks in phase requirements. Feedback edges from rejection are typed shape events, not reverse data dependencies in the DAG.

Contract-only development is not integration success. A consumer may develop and test against a local stub; completion that promises integration must check the actual producer output. A producer implementation change with the same interface need not restart consumer source work, but it invalidates integration evidence tied to the old output combination.

Contracts are hand-written and reviewed by the user and the model together before ingest. They are the artifact where a wrong assumption is cheapest to catch: before ingest it is an edit; after fan-out it reaches every consumer. `planar init` may ask which interfaces between work units should be pinned and draft files into `contracts/`, but ingest reads what stands.

```lua
contracts = planar.list {
  { id          = "bucket_api",
    producer    = "library",                      -- work unit
    consumers   = planar.list { "command" },      -- work units
    expected    = planar.file "contracts/bucket.hpp",
    conformance = "bucket_conforms",              -- check ID from the allow-list
    review_note = planar.md [[ Reviewed with the model on 2026-10-18. ]] },
}
```

In v1 a contract is text with a conformance check. The kinds below are the intended catalog for later versions, each selecting a render hint and a default check. Every check states its coverage; no kind acquires a strong guarantee by having a familiar name. A syntax-only C++ check proves selected call signatures, not ABI or behaviour; a schema sample proves that sample conforms, not all outputs.

| Kind | Source | Default check | Proves |
| --- | --- | --- | --- |
| `cpp_header` | `.hpp` | syntax-only compile of a conformance TU | selected signatures compile |
| `proto` | `.proto` | `buf breaking` against the facet | wire compatibility |
| `openapi` | `openapi.yaml` | `oasdiff breaking` against the facet | path and schema compatibility |
| `graphql` | SDL | `graphql-inspector diff` | schema compatibility |
| `json_schema` / `avro` | schema file | validate a producer-emitted sample | that sample conforms |
| `cli` | usage and exit-code doc | usage lint | documented surface matches |
| `sql_schema` | DDL or migration | diff against the facet's dumped schema | structural match |
| `md` | prose | hand-written registered test; otherwise advisory | whatever the test covers |

Checks prove shape, not behaviour. Idempotency, latency and error semantics are registered contract tests or `md` prose the reviewer enforces; `planar load` reports which contracts are mechanically held and which are prompt-held.

## Checks, ownership and observed completion

A check is a selected trusted descriptor. Required checks have explicit IDs; `check("api_contract")` resolves a registration, never a guessed source path. Zero selected tests, missing tools, unsupported adapters, timeouts and malformed results are failures, not passes. Advisory requirements are labeled advisory and visible to review.

The job selects check IDs and validated arguments. Arbitrary `cmd(argv)` is not exposed to job-tier Lua. Toolchains and check definitions live in a **trusted registry**: a JSON file the driver passes with `--registry`, written by a person, shared across workspaces, never harness-editable. The workspace declares which registry ID it expects and which toolchain and check IDs it may use; a job can only select from that allow-list. `load` captures the registry's exact bytes into the accepted revision, so a changed command is a new revision, and the selected registry records are material to work fingerprints. If a deployment allows harness-written commands, its driver must opt into that trust policy explicitly. A job cannot grant itself approval by setting a field.

```json
// ~/.planar/registry.json  (driver-supplied; see declarations.md for the exact schema)
{ "version": 1, "id": "local-tools",
  "toolchains": { "cxx": { "build": ["cmake", "--build", "build/debug"],
                           "test":  ["ctest", "--test-dir", "build/debug", "-R"] } },
  "checks": { "bucket_conforms": { "toolchain": "cxx", "uses": "build",
                                     "args": ["-fsyntax-only", "tests/contract/bucket_conforms.cpp"],
                                     "covers": "selected call signatures compile" } } }
```

```lua
-- workspace.lua: the allow-list, not the commands
registry = { id = "local-tools", toolchains = planar.list { "cxx" },
             checks = planar.list { "bucket_conforms" } },
```

The same `workspace.lua` runs on another machine, or under another driver, by pointing at a different registry that defines the same IDs. A toolchain change for ten workspaces is one file. The workspace cannot become a place where commands are typed, because it has nowhere to put them.

The driver supplies root paths, base revisions, a stable observation view and an execution identity, and materializes role write scopes derived from the accepted spec and job mapping. Planar checks that the root mapping and path ownership are unambiguous: it resolves symlinks, aliases, path traversal, case equivalence on the host filesystem, nested roots and overlapping directories before accepting ownership. Frozen user-owned contract paths are excluded from session writes. Shared `owner = any` writes are not allowed in the first cut.

Boundary checks inspect final differences from the declared base: additions, deletions, renames, modes, symlink changes and untracked files. Ignored files are included unless a declared generated-artifact policy excludes them. The initial implementation requires the driver to supply an attributable role view or to serialize source-writing roles; disjoint path declarations alone cannot attribute edits by concurrent processes sharing one checkout. Worktrees are one way a driver provides that view; Planar neither creates nor requires them.

A completion operation:

1. Validate the current run token, acknowledged input revision, role generation and pending blockers.
2. Capture the declared check inputs and output manifest. Acquire a role verification reservation so two check operations cannot publish concurrently for the same run.
3. Run required checks in the supplied view; record command definition, environment policy, cwd, dependency identities, result and bounded diagnostics.
4. Observe those inputs and outputs again. A detected change makes the operation inconclusive; it cannot complete the role.
5. In one conditional ledger transaction, revalidate current run and generation, input revision and dependency quality. Record results, output identity, published facets and completion together.
6. Release the verification reservation and return an idempotent operation result.

The guarantee is **checks passed for the recorded observation under the driver's stability contract**. Before/after hashing catches many changes but not change-and-revert. Database atomicity does not freeze a filesystem. A driver that cannot keep check inputs stable cannot claim this completion guarantee; it must serialize work or supply a stronger adapter.

Published facets come from the successful observation, never from partially written live files. A changed current output invalidates applicability of old evidence; the historical result remains true about its old target. Planar may retain small published facet payloads needed by consumers without archiving whole checkouts. A missing historical output that a new review needs produces `target_unavailable`; the driver provides it or creates a new target.

## Runs, states and interruption

Six role states, moved by three verbs. A run is an ownership record, not an extra state. Run tokens, generations and fencing are in cut 2 because 24–48 hour jobs die mid-run as a matter of course (usage limits, HTTP errors, a driver that stops) and a half-dead session may return hours later and try to commit.

> Figure: state machine — blocked → ready (prerequisite satisfied) → running (`render --claim`) → done (applicable checks pass) → stale (inputs or evidence change) → running (`render --claim`, with `--since`). running → ready on `release`. Any state → retired when the role generation is removed; any non-retired state → blocked when a start prerequisite goes missing. A failed check changes no state; a running role whose inputs move stays running, flagged drifted.

`render <role>` is read-only. `render <role> --claim --request-id <id>` atomically reserves a run and returns its token with the dispatch packet. Tokens carry a monotonically increasing generation; `check`, `refresh` and `release` must present the current token. The driver chooses the request ID before dispatch; retrying it returns the same reservation and packet, and `status --request-id <id>` recovers after a crash before the token reached the caller. Reusing an ID with different parameters is an error.

Concurrent claims have one winner. A repeat render never launches a second session. Explicit takeover requires the current run identity, advances the generation and fences all old mutations. Fencing refuses stale ledger commits; it does not kill the old process. The driver must stop that process before granting the same writable view to a replacement. Looking up an old request also returns whether its reservation is current, completed, released or fenced; replaying a historical claim is not permission to launch its session again.

| Situation | Result |
| --- | --- |
| Never completed; start prerequisites satisfied | Ready |
| Current token claimed | Running |
| Running inputs change | Remains running, marked drifted; completion refused |
| Session incorporates new inputs | `render --refresh --run <token> --expect-fp <old> --request-id <id>` replaces the acknowledged manifest and packet; same run stays active |
| Applicable checks pass against acknowledged current inputs and outputs | Done |
| Required check fails | Running; failed evidence appears in the next recovery packet |
| Done work inputs, output applicability or required integration evidence change | Stale, unless a start blocker takes priority |
| Start prerequisite absent, no active run | Blocked; previous completion evidence preserved |
| Blocker clears with unchanged valid completion evidence | Done again |
| Blocker clears with obsolete evidence or admitted repair | Stale; ready only if never completed |
| Release or driver interruption recovery | Close current run; derive ready, stale, blocked or done from retained evidence |
| Role removed or structurally replaced | Retired generation; active token fenced; old rows remain addressable |

A running role with a new blocker stays running with the blocker recorded; the driver may release it. Refresh requires valid start prerequisites and a compatible role and ownership structure, and never makes earlier check receipts cover new inputs. A structural change to ownership or assignment requires release or takeover and a new generation; ordinary requirement drift is incorporated in place. The explicit refresh is the acknowledgment boundary: previewing a new fragment does not assert the session used it, and `--since` explains the difference without acknowledging it. If inputs move again before refresh commits, the conditional update fails and the driver retries with current context.

The atomic flip is the invariant carried from `planar-agent`: checks recorded, state set, output facets published in one conditional commit, or a crash leaves a role done with stale facets downstream. That commit lives in C++.

| `planar-agent` today | Planar v5 |
| --- | --- |
| `claim` (lease with TTL) | `render --claim` → running; token with generation, no TTL |
| `complete` | `check` passing → done |
| `fail` | a failed check: recorded, state unchanged |
| `release` | `release` → state derived from evidence |
| `block` | derived blocked, never set by hand |
| `agent_actions` | execution events |
| `agent_work_claims` | runs with request IDs and generations |
| flip claim and status in one transaction | `engine.ledger.commit(expected, changes)` |

The lease existed because agents raced for one task. With disjoint ownership there is no race; what remains is a session that returns after the driver moved on, and fencing is the answer to that, not a timeout.

## Review and acceptance

Validation is mechanical and lives in the gate; review is judgment and lives in the shape, as a role. Only validation flips state. A review never changes state directly: rejection is a typed shape event that increments a repair revision; approval is evidence, not an input that wakes anyone.

Review judges exact output identities. A worker's work fingerprint alone is insufficient because its implementation can change while its instructions stay the same. In v1 there is exactly one reviewer per job and it targets all work; a fan-in review targets a sorted vector of worker output identities, their work fingerprints and the review-policy revision; the driver provides those exact outputs to the reviewer.

The reviewer is a distinct role with a declared driver or session identity, no worker write scope, and its own fenced run. Planar enforces declared role separation. It does not prove that different humans or models performed the work; strong identity attestation is a driver adapter.

A schema-validated verdict carries a unique event ID, target vector, policy revision, approve or reject, findings, and explicitly named repair targets on rejection. Approval with unresolved mandatory findings is invalid. Every repair target must belong to the reviewed vector. A judgment that cannot name a repair owner becomes a blocking question, not a guessed revision of every worker.

```lua
-- a verdict event (R18 shape; fields illustrative until that round fixes them)
return planar.verdict {
  version  = 1,
  event_id = "rv-7f3a-0003",
  targets  = planar.list { { role = "cli_worker", output = "o:4c1d…", work_fp = "7f3a…" } },
  policy   = "rev-12",
  verdict  = "reject",                 -- approve | reject
  findings = planar.list {
    { severity = "must",   role = "cli_worker", path = "src/cli/main.cpp", line = 41,
      prescription = "P1", note = planar.md [[ Uses std::cout; P1 requires fmt. ]] },
    { severity = "should", role = "cli_worker", path = "src/cli/args.cpp",
      note = planar.md [[ --times accepts negative values; clamp or reject. ]] } },
  repair   = planar.list { "cli_worker" },
  summary  = planar.md [[ Core logic is right; one prescription violation and one input edge case. ]],
}
```

On an admitted rejection, one transaction records the verdict, increments each named worker's repair revision, carries forward its findings and updates the round count: all effects or none. Retrying the same event returns the original result without another increment. Stale or mismatched targets are historical findings only and cannot reject or approve current work. A malformed verdict is treated as absent and flagged, never interpreted. A finding that cites a prescription is the signal that the prescription's check was too weak; reviews improve gates over time, they do not replace them.

`max_rounds` is the maximum number of admitted reject-and-repair cycles for the active job and shape generation. It is cumulative across contract edits and input refreshes, which cannot reset it. The rejection that uses the final allowed cycle still permits that repair and its review. A further rejection is recorded, leaves the work unaccepted, and opens a blocking question instead of another cycle; continuing requires an explicit operator policy change.

`accepted` is derived: every required worker has applicable passing checks, the exact current output vector has a valid approval under current policy, and no acceptance blocker remains. A change to code, required inputs, review policy or the selected integration vector invalidates applicability; the old approval stays historical. `done` and `accepted` remain distinct, so "done but rejected" is a visible, explainable condition.

Three review points in a job:

| Point | When | Who | Mechanism |
| --- | --- | --- | --- |
| Spec review | before ingest | user and model together | `planar init` ends with a critique prompt: ambiguities, unenforceable prescriptions, missing contracts; the spec and contracts are edited before anything is projected |
| Code review | in the shape | reviewer role | verdict events, as above |
| Acceptance | at closeout | checks, then a person when required | required checks; a human approval must refer to an exact target vector, and a generic question answered yesterday is not approval of today's changed code |

Human pre-work clarification remains a question. The first cut reports `awaiting_external_approval` when final human approval is required and accepts an explicitly configured driver assertion only if that policy is enabled; it makes no stronger authentication claim.

## Workspace, loading and persistence

The workspace is external to repositories and contains no checkout. It points at roots wherever the driver put them; nothing under `~/.planar` is ever a worktree. There is one authoritative location for each declaration and no workbench synchronization layer. Specs and contracts may opt into version control inside a repository per repo; the ledger never does.

Planar has no default location. Every invocation names the workspace file, the trusted registry, the ledger and the host directory for each root; Planar never consults `HOME`, a config file or the current directory. One environment variable is the only convenience: when `PLANAR_WORKSPACE_DIR` is set, a bare name resolves under it, and `init <name>` creates that directory. Nothing is guessed when it is unset.

```text
$PLANAR_WORKSPACE_DIR/              by convention ~/.planar/workspaces; set by the user or a wrapper
  <name>/
    workspace.lua                   logical root, registry and allowed IDs, spec, jobs, bindings
    spec.lua                        one spec per workspace in v1
    jobs/<job>.lua                  one per grouping or arm, never overwritten by a run
    contracts/                      reviewed interfaces
    docs/                           authored prose
    state/ledger.db                 accepted revisions and the complete recorded run history
    state/recovery/                 cache of latest recovery packets; rebuildable
```

```lua
-- workspace.lua (v1 shape; see declarations.md for the exact schema)
return planar.workspace {
  version  = 1,
  id       = "billing-v2",
  root     = { id = "source", base = "base-2026-10-18" },   -- logical only
  registry = { id = "local-tools", toolchains = planar.list { "cxx" },
               checks = planar.list { "bucket_conforms" } },
  spec     = "spec.lua",
  jobs     = planar.list { "jobs/split.lua", "jobs/together.lua" },
}
```

```text
planar render --workspace billing-v2 --registry ~/.planar/registry.json \
              --root source=/code/wt/billing-v2 --job split --role cli_worker
```

- The workspace declares a root by logical ID and base label only; the driver binds the host directory per invocation. A declaration is therefore machine-independent: the same files run on any box, and two experiment arms bind different directories without a declaration edit.
- Planar validates that the bound directory resolves to a real, unambiguous location and that the base label is what the declaration expects; it never creates, merges or deletes checkouts, and deleting a workspace removes declarations and history only.
- A workspace inside a repository is the same mechanism with a full path (`--workspace /code/repo/planar/workspace.lua`); nothing else changes.
- A name that already exists under `PLANAR_WORKSPACE_DIR` is never silently suffixed; `init` refuses and shows the existing one.
- There is no current-workspace state, no index, no discovery by root, no scope stack and no `use` verb. Finding the workspace for a checkout is a driver convenience.
- Separate experimental arms use separate ledgers and separate writable views supplied by the driver.

### Working files and accepted revisions

Workspace files are the editable working copy. SQLite owns the accepted revision history and the recorded execution trajectory. There is one publication direction: files become an accepted revision through an explicit successful `load`. Editing a file, a watcher notification or a status query never accepts it. Acceptance here means activation of a validated declaration revision, distinct from acceptance of reviewed worker output.

`load` captures the complete declaration input set, validates the captured bytes and resolved graph, then commits a new revision and activates it atomically with the resulting role-state changes. It records a caller-supplied request ID and expected active revision so retries are idempotent and concurrent lost updates are refused. A failed validation or transaction leaves the active revision unchanged. Re-accepting identical content and interpretation versions is a no-op; reverting to older content after intervening changes creates a new chronological revision that may reuse the older content tree.

Pending edits are visible as drafts in `status`. Invalid or incomplete drafts do not suspend work on an accepted revision. New claims and checks use the active revision; active runs keep their acknowledged revision until explicitly refreshed. If a check needs a contract or other declaration file, it consumes the accepted captured version; a driver that supplies only changed working bytes must materialize the accepted version or receive a revision-mismatch refusal. `impact` compares a captured draft with the active revision without accepting it. Switching jobs validates assignment, ownership and review policy as one revision change; changed role structure creates new generations; renaming is retire plus new.

### Declaration history in SQLite

Planar keeps a built-in Merkle tree with XXH3_128bits for declaration change detection and revision storage. It never initializes or requires a Git repository for this history; source repositories and their driver-supplied base revisions are a separate concern.

The rule that keeps this from becoming the old 39-migration database: SQLite holds captured content and observed events, never a second editable copy of a declaration. The tree covers workspace, spec and job declarations and every contract or prose file captured through their declared references, each at a stable logical path within its declared root. Runtime state, the database itself, caches, inbox and out directories and implementation files are excluded.

Leaf encodings carry a domain and version tag, entry kind and exact content bytes, with relevant metadata encoded explicitly. Directory nodes carry sorted, length-prefixed entry names, kinds and child digests, so additions, deletions and renames change the root. Nodes and payloads are immutable records reused across revisions when byte-identical (exact payload compared before reuse; a digest is an index). A revision points at a complete logical tree; restoring it needs no patch replay. Deltas are derived by comparing trees.

The source tree preserves exact authored bytes; a separate validated representation records resolved values and interpretation versions, and the per-role materiality rules decide which work fingerprints move. An accepted comment-only or optional-prose edit creates a revision without staling work. A changed stdlib, schema or normalizer version participates in revision interpretation even when bytes are unchanged. Capture validates the same in-memory input set it loads; edits detected during capture cause a retry or refusal. Unchanged modification time is never proof of unchanged content.

### Recorded execution trajectory

Declaration history and execution history are linked but distinct. Every accepted revision records its parent, source root, captured validated representation, interpretation versions, request identity, actor as declared by the driver and acceptance time. Every state-changing operation records an ordered event, active and acknowledged revision references, affected role and run, previous and resulting state, reason, and relevant input, output, check and verdict references. Check failures, refreshes, releases, takeovers, rejection repairs and interruptions are part of this history even when no declaration changed.

A per-workspace sequence establishes order; wall-clock timestamps are context, not causality. Current-state tables are maintained transactionally with the append-only history. Historical inspection never re-executes commands or reinterprets old events under today's policy. This answers: what instructions were active, what context did this session receive, what changed, which checks failed, why did work go stale, what happened after rejection.

SQLite stores declaration content and tree nodes, revisions, dispatched packets and manifests, role generations, runs and request IDs, facet observations and publications, check records, review events and state transitions, in as many tables as integrity needs. Accepted revisions and recorded history are retained through and after a run by default; archival or pruning is an explicit operation with stated coverage and reference-integrity checks. Large raw tool logs may have their own retention policy, but outcome, target, diagnostic summary and any omission stay visible in the timeline. Storage exhaustion fails the pending mutation and never discards history or claims an unrecorded success.

Claim packets are committed with their recoverable request and run records; filesystem caches can be rebuilt. Interrupted checks never leave roles done; reservation recovery records the interruption and permits a fresh operation after the driver confirms the old process stopped. A retry of a committed operation returns its stored result; an in-progress operation is not executed twice. The ledger has a versioned format, tested migrations and a backup and restore procedure; unsupported future versions are refused unmodified, and the legacy Planar database is never interpreted as this ledger.

## Multi-spec composition

A workspace is a spec whose roles are specs. Several full specs with dependencies between them share one workspace, one ledger, one root set and one toolchain. A spec never names another spec; it declares imports and exports, and only `workspace.lua` binds an import to another spec's export. Internal facets stay private.

> Figure: three specs. plan1 exports `billing_api` (from `core.api`, pinned by `contracts/billing.proto`); plan2 imports it, develops on the contract and completes on real output, and exports `invoice_model`; plan3 imports `invoice_model`. Bindings in `workspace.lua` are the only cross-spec edges; `core.diff` inside plan1 is invisible outside it.

Three rules:

1. A spec declares **imports** (facets it needs, by local name, each with the contract it expects) and **exports** (facets it offers, each pinned by a contract). It never names another spec.
2. Inside a spec, work units depend on imports exactly as on local facets; the spec cannot tell whether an import is local or bound elsewhere.
3. The workspace is the only place a binding is written, and a binding is facet to facet. Roots and ownership are validated across the full workspace, including aliases; compatible contract identity and dependency phase and quality are checked at every binding.

Standalone contract development proceeds with an unbound non-live import. Integration completion cannot pass if it requires an actual producer that is absent. A changed export semantic value affects its subscribers; an unrelated internal change does not move their work fingerprints. Output changes may still invalidate integration evidence even when the export's semantic value is stable. A producer becoming stale or unaccepted changes prerequisite availability without pretending that interface bytes changed.

There is no whole-spec completion barrier when a consumer needs only one export; plan 2 can start the moment that export's contract check passes. The driver may schedule at the role or spec level; Planar reports the same underlying dependencies. Roles are namespaced in the ledger (`plan2/cli`), so `status`, `explain` and `impact` work across the whole workspace.

Cascade handling across specs:

- Fragments are renders; invalidation never costs fragments, only work already done against moved inputs. Planar records `stale` and never re-wakes anyone; the driver decides, in topological order, and only for roles whose semantic inputs moved.
- Stale does not cascade on its own. A stale role's published facets do not change until it is re-run and re-checked, so invalidation propagates one hop per completed re-run.
- `render --since <render_digest>` shows an in-progress session exactly what changed; `render --refresh` is how it acknowledges it.
- `impact` reports the blast radius of an uncommitted edit before it is accepted.
- Cascade width is decided by subscriptions. Dependencies that run through contracts and exports re-wake on interface change only; dependencies on `diff` facets re-wake on every published change. Prefer the former across spec boundaries.

Not in v1. One root and one spec per workspace until the deferred composition phase after R19; this section is the design that phase implements.

## Engine surface and sandbox

The job-tier environment exposes validated constructors, `md`, captured `file` references, selected shape and check descriptors and allowed materiality wrappers. It omits `io`, `os`, `debug`, `package`, `require`, dynamic loading, process calls and mutable stdlib or engine tables. Safe library functions are listed explicitly, not inherited wholesale. Each load has memory and instruction limits, and diagnostics distinguish syntax, schema, resource and capability errors, with file and line where available.

The trusted stdlib uses this conceptual engine surface; exact binding signatures are implementation work:

```lua
engine.hash(value)                        -- XXH3_128bits over versioned canonical encoding
engine.hash_bytes(bytes)
engine.read(root_id, relative_path)       -- confined captured reads
engine.glob(root_id, pattern)             -- sorted, confined results
engine.write_state(relative_path, bytes)  -- workspace state/ only
engine.git_observe(root_id, base, scope)
engine.exec(check_id, argv, options)      -- bounded time, output, cwd and environment; every call logged
engine.load_declaration(path, environment, limits)
engine.ledger.read(query)
engine.ledger.commit(expected, changes)   -- conditional atomic mutation
engine.emit(kind, payload)
```

The engine checks the transaction invariants it owns: current generation and token, expected revisions, uniqueness of request and event IDs, valid references and atomic writes. The stdlib supplies applicability and check semantics through validated records. The engine API never claims to prove semantic facts it has no independent evidence for.

Projections stay in Lua. The ownership rule is about trust, not compute: C++ returns data, Lua decides what it means and what it looks like. A engine primitive is added only when the data is expensive to produce or must be exactly reproducible (a tree walk of a very large root; a grouping solver), and rendering stays in Lua where golden-file tests on `render` pin it cheaply.

Render-only needs hashing, captured reads, loading and emit. Process execution and ledger mutation enter in cut 2. A fake engine in Lua supports fast stdlib tests for shapes and reconciliation; its success never certifies native sandbox or filesystem security. The first security test to write is the one that asserts the job environment's globals are exactly the curated names and nothing else.

`stdlib.d/` holds the user's own shapes, projections and verbs, loaded after the shipped stdlib with the same trust. A shape Planar does not ship is written there, not forked in; experimental verbs live there before promotion.

## Bootstrap and external driver

Planar never calls a model or owns the scheduling loop. Each Planar step is a finite operation over files and the ledger; each harness step is a session following a prompt Planar emitted; the user reviews between them.

> Figure: swimlanes User / Harness / Planar. `planar init` (spec prompt) → harness writes `spec.lua` and contracts → user reviews and edits (critique prompt) → `planar ingest` (job prompt) → harness writes `job.lua` → `planar load` (accept revision; a rejection returns diagnostics as a prompt back to the harness) → `planar render` to the driver. Shaded steps are deterministic; the driver loop starts after render.

A session has no direct channel to Planar. Everything it produces reaches Planar through the driver: check results, review findings and a bounded checkpoint arrive as one `--observations` JSON per render, each item tagged as a driver assertion that Planar has not verified; verdicts arrive as events the driver submits (R18). The inbox/out directories of earlier revisions are therefore a driver convention: the reference orchestrator tells sessions where to leave proposals and checkpoints, and the driver forwards them. To give that convention one implementation, the stdlib ships `planar observe --from <dir>`, which assembles an `--observations` file from such a directory; it is driver-invoked, and Planar never scans a directory on its own (decided 2026-10-21).

As implemented, `init` creates an empty ledger. The prompts described here are the authoring round scheduled between R14 and R15 (open item 6); until it lands, specs and jobs are hand-written.

`init` emits a specification prompt and can scaffold workspace declaration files; it does not provision checkouts. The harness drafts the spec, the user reviews it and its contracts (with the critique prompt `init` ends on), `ingest` emits a job-authoring prompt, and the harness writes a job using known shapes. `load` validates and activates the graph; a rejected job returns precise diagnostics to the harness as a prompt.

After loading, the driver queries status, claims ready work, runs an external session, refreshes context when needed, checks output and dispatches review. Planar performs each finite operation and returns. A rendered orchestrator fragment explains the shape and the legal CLI actions; prose alone does not make different drivers deterministic, so comparative experiments pin or record the actual driver implementation.

The reference orchestrator is a role like any other, rendered by `projections.orchestrator`: the job graph as a dependency summary, the shape's loop semantics in prose, the CLI contract (`status`, `render --claim`, `check`, `release`), and driving prescriptions (never run two roles with overlapping ownership in one shared view; always `check` before treating a role as done; re-render after `stale`; give each role an isolated checkout, worktrees being one way). Its inputs are the graph and ledger state only, never worker facets. `planar render orchestrator | claude -p` is the default way to run a job; a shell loop, Centurion or a person are equally valid drivers.

The core command set is `init`, `ingest`, `load`, `status`, `render`, `check`, `release` and `explain`; `impact` and `render roadmap` are read-only extensions in cut 2. Machine-readable responses include typed errors, durable request and operation IDs, and the exact identities that human-readable output summarizes. `render` never starts a session.

## Reference fixture

The accompanying fixture (`planar-v4-fixture/`) holds Lua declarations for the fixed-role example, a pure-Lua reference renderer, golden fragments, a small Python reference application with real tests, and an in-memory protocol model used to exercise failure traces. It is separate from the production repository. Its fake identity function compares canonical values and is not the native xxHash or Merkle implementation; its loader is not the production sandbox; its synthetic base and target aliases are replaced by real root and output identities in the native execution cut.

The fixture job is a small quota library and CLI: worker `core` owns `src/core/` and implements an atomic `try_take` against a reviewed contract; worker `cli` owns `src/cli/` and develops against that contract immediately, completing only on actual core output and integration; reviewer `review` owns only its verdict output and reviews the exact core and CLI output vector. One shared constraint, separate scoped prescriptions, an optional rationale, a driver checkpoint, and an explicit toolchain and check registry.

The fixture still encodes fixed spec roles and is a historical reference. Before the native render gate, migrate its library and command work into role-independent work units and render both the combined and split jobs against the same spec. The existing protocol trace exercises only the original split job and does not certify regrouping.

The trace covers: initial fan-out; a material contract change mid-run; same-token acknowledgment; stale check refusal; producer then consumer completion; rejection of one named worker; atomic retry of that rejection; repair and approval; changed code with unchanged instructions; claim-response loss; takeover fencing; blocker removal with still-valid evidence. It is executable evidence about the design model, not evidence that the C++ runtime, SQLite transactions, subprocess checks or OS boundaries work.

## Build order and acceptance gates

Each cut has one question it exists to answer and a gate drawn from the adversarial matrix. Cut 1 ends with a `render` you would feed a session; cut 2 ends with a completed job that passes the full matrix.

### Cut 1: render useful context

Implement the restricted declaration loader, schema, canonical encoder, scoped input selection and the worker, reviewer and orchestrator projections. Migrate the fixture to work-scoped declarations and provide combined and split jobs against one spec. Expose a development `render` that reads captured files and emits a fragment plus an identity manifest. No ledger, check execution or claim is needed. A Lua fake engine makes stdlib tests fast.

Gate: every matrix row whose gate is Render (A01–A04, A07, A08, A30–A32). Fresh-process renders are byte-identical; scoped requirements appear exactly where needed and nowhere else; optional prose moves only the render digest; own output changes do not wake their producer; reviewer targets are exact; mandatory overflow is explicit; both groupings preserve complete coverage, exclusive ownership, independent review and correctly resolved dependencies with no self-wait. Then evaluate the actual worker and recovery fragments for usefulness before extending the runtime.

### Cut 2: observed-state execution, as rounds

The implementation runs cut 2 as reviewed rounds, each with a fresh search agent, coder and reviewer, and each closed by a recorded approval before the next starts. Working files remain drafts until accepted by `load`; a 24-hour run is useful experience, not a replacement for the matrix.

| Round | Scope | Matrix rows |
| --- | --- | --- |
| R11–R12 (done) | Ledger and history contract; SQLite format and transaction foundation with crash and disk-full tests | A23 (format), A26 (foundation) |
| R13 (in progress) | Immutable records and Merkle trees; exact and interpretation provenance | A27, A28 |
| R14 | `init`, `load`, `history`, `export`: accepted revisions, content-addressed storage, request replay, exact export | A20, A23, A25, A26, A29 |
| Authoring (new) | `init` spec prompt and critique prompt, `ingest` job prompt, rejection prompts; `planar observe` | two new rows: prompt determinism; rejection names the field path |
| R15 | Run ownership: `render --claim`, tokens with generations, request IDs, fencing, explicit takeover | A09, A10, A22, A33 |
| R16 | Refresh and recovery: `--since`, `--refresh --expect-fp`, recovery packets, drift | A11, A12, A19 |
| R17 | Real check execution and observed completion; boundary checks; published facets | A05, A15, A17, A18 |
| R18 | Review and repair: verdict events, atomic rejection, `max_rounds`, derived acceptance | A06, A13, A14, A16, A24 |
| R19 | Single-root driver demonstration: a spec previously run on old Planar completes end to end with the reference orchestrator driving, under the whole matrix | all |

Gate for the cut: every Execution row, plus the measurement provenance in A34. Contract edits are incorporated without losing the session; failed or stale checks cannot complete; approval cannot cover changed output; rejected feedback is applied exactly once; interrupted dispatch, check and review operations recover without manual database edits; the recorded trajectory can be inspected against its accepted revisions.

### Deferred phase: workspace composition

After R19, not between execution rounds. Add imports and exports, phase-aware bindings, multi-root integration, scoped reloads, `impact` and `render roadmap`. Gate: the Extension rows (A21); independent specs remain usable during a local edit elsewhere; explicit refusal of ownership aliasing, binding mismatch and integration evidence for the wrong output vector.

### Cut 3: measure specific claims

Export compact ledger events (`planar metrics`). Run the compaction experiment with raw-context, Markdown-file and Planar-fragment controls, scoring continuation quality at fixed interruption points. Test grouping and model choice separately, each with its own control; compaction is the paper, the other two are instrumentation that happens to exist. Add the `tournament` shape. Do not interpret deterministic fragments as deterministic model behaviour. Add caching or a grouping solver only after measurements identify the problem they solve.

### Optional stronger execution

Introduce immutable source archives, read-only verification views, authenticated human identity, managed worktree lifecycle or OS isolation only for a demonstrated requirement, each as an adapter naming its guarantees and test suite. Their absence does not stop the core context plane from being useful. Mt-KaHyPar stays behind `engine.partition` on a branch until it wins a ledger-metric comparison against greedy grouping.

Guardrail: cut 1 ends with a usable `render`; cut 2 ends with a completed job. Building cut-3 instrumentation before a job has finished is the broad-exploration pattern; metrics feel productive while nothing ships.

## Adversarial acceptance matrix

The full matrix is the gate: Render rows gate cut 1, Execution rows gate cut 2, Extension rows gate the workspace extension, Measurement rows gate cut 3.

| ID | Failure or change | Required result | Gate |
| --- | --- | --- | --- |
| A01 | Different Lua map insertion order and fresh processes | Same captured inputs produce identical fragments | Render |
| A02 | Work-scoped requirement and unrelated requirement edited separately | Only applicable work fingerprints move; required text present, unrelated text absent | Render |
| A03 | Optional rationale or session checkpoint changes | Render changes; work freshness unchanged | Render |
| A04 | Mandatory content supplied through `file` or an immaterial wrapper | File contents remain material; illegal wrapper refused | Render |
| A05 | Worker edits its implementation | Own input fingerprint stable; output identity changes | Render/Execution |
| A06 | Reviewer receives stale or wrong worker output | Exact target mismatch prevents applicable approval | Execution |
| A07 | Mandatory context exceeds budget | Clear refusal; no silent truncation | Render |
| A08 | Undeclared capability, stdlib mutation, infinite loop, huge allocation | Bounded refusal in native sandbox | Render |
| A09 | Two claims; lost response; reused request with changed parameters | One owner, recoverable token and packet, parameter conflict | Execution |
| A10 | Old session checks after takeover | Fenced mutation; driver prevents concurrent writes | Execution |
| A11 | Contract changes while worker runs | Drift shown; old check refused; explicit same-run refresh allowed | Execution |
| A12 | Completion blockers clear without changing valid historical evidence | Done restored; no unnecessary rerun | Execution |
| A13 | Crash between verdict recording and repair revision | Both atomic effects or neither; retry increments once | Execution |
| A14 | Review rejected beyond cap | Durable blocking question, no automatic extra repair cycle | Execution |
| A15 | Source or check dependency changes during checks | Inconclusive result, no completion; stronger race guarantees only with adapter | Execution |
| A16 | Same interface but different producer implementation | Consumer work may remain fresh; old integration evidence unusable | Execution |
| A17 | Missing tests, zero selected tests, timeout or missing executable | Failed check with actionable diagnostics | Execution |
| A18 | Symlink escape, root alias, ignored source, rename, mode change | Correct boundary refusal or declared generated-file exclusion | Execution |
| A19 | Context loss after partial work | Recovery includes checkpoint, findings, actual checkout and current inputs | Execution |
| A20 | Unaccepted local edit or invalid shared draft | Active revision continues unchanged; status reports pending edits; failed load does not activate them | Execution |
| A21 | Unbound contract-only import | Development allowed; actual-producer completion remains blocked | Extension |
| A22 | Role removed and later recreated under same name | New generation; old token and approvals cannot attach | Execution |
| A23 | Packet missing, disk full or unsupported ledger format | Explicit failure; no silent history loss or partial completion; rebuild missing caches from SQLite | Execution |
| A24 | Required final human approval predates changed code | Old assertion cannot approve new output | Optional approval policy |
| A25 | Accept changed declaration, repeat identical load, then restore an earlier declaration | One changed revision, no duplicate on unchanged acceptance, new chronological revision for restoration | Execution |
| A26 | Crash or competing acceptance while publishing a revision | Revision, active pointer and reconciliation events commit together; stale expected revision refused | Execution |
| A27 | Equal xxHash values for different leaves or child records | Exact comparison refuses collision; distinct content is never silently reused or skipped | Execution |
| A28 | Accepted optional-prose edit, file deletion, rename or timestamp-preserving edit | History records actual byte and tree changes; only applicable material work becomes stale | Execution |
| A29 | Inspect an early event after many revisions and a restart | Recover its accepted declarations, dispatched context, state changes and evidence references without re-execution | Execution |
| A30 | Render combined and split jobs against byte-identical spec | All required work, acceptance criteria, checks and boundaries covered; correct scoped fragments and independent reviewer | Render |
| A31 | Omit or duplicate a work assignment, or grant overlapping or out-of-bound paths | Precise validation refusal; no accepted partial mapping | Render |
| A32 | Group dependent units into one role; introduce a cross-role scheduling cycle | Preserve internal contract and completion checks without self-wait; reject unsatisfiable cycles | Render |
| A33 | Change grouping during active work and reuse role names | Fence affected generations; preserve historical mappings; no inherited token or unsupported approval | Execution |
| A34 | Compare job grouping and model variants | Identical spec and baseline identified; isolated trial state; requested and actual model settings and outcomes recorded | Measurement |

## Relationship to existing Planar

The legacy product was removed in R09 on `dev/planar-v2-vernacular`; git keeps it. There was no migration and no coexistence: the new ledger is a separate format that never reads the legacy database, and no legacy declaration is converted. This is the clean rewrite the implementation plan approved, and the "pending verification" question of earlier revisions is closed.

The replacement destination as it stands:

```text
src/cmd/planar/            one binary; one positional verb, rest dispatched to the stdlib
src/engine/value/          declaration value model
src/engine/lua/            restricted loader: two environments, memory and instruction limits, bytecode refusal
src/engine/capture/        confined captured reads, path grammar, conflict test
src/engine/canonical/      versioned canonical encoding with golden vectors
src/engine/hash/           pinned XXH3_128bits, seed zero, with vectors
src/engine/runtime/        render pipeline and scoped identities
src/engine/json_codec/     observations, registry and machine-readable output
src/engine/ledger/         SQLite format, conditional transactions, crash and disk-full tests
src/engine/history/        immutable records, Merkle trees, provenance (R13)
src/stdlib/planar/         trusted Lua: schemas, shapes, projections, normalizers, verbs
fixtures/render/quota/     the quota spec with combined and split jobs and computed goldens
docs/vernacular/           declarations, render and ledger contracts, roadmap, rounds/R*.md
```

Lua 5.5 is vendored and compiled statically; xxHash was added as pinned, offline source. Catch2 colocated `*.t.cpp` cases make up the gate (124 tests at the render gate, zero skips). What survived from the old tree is technique, not code: module and test registration in CMake, the pinned LLVM toolchain, the black-box parity harness idea, and the exception-to-Lua-error and confined-path patterns from the old execute host, each re-proven by native adversarial tests rather than carried across.

Round protocol: each round has a search agent that reads only what the round names, a coder that implements against the written contract, and an independent reviewer that records a disposition in `rounds/R*.md`; the next round starts only on an approved disposition. Matrix rows are assigned per round and claimed only when their tests pass.

## Decisions

| Decision | Choice | Reason |
| --- | --- | --- |
| Orchestrator | Removed; bring-your-own | Planar emits prompts and validates; it never loops |
| Source of truth | Lua for every declaration; Markdown is render output | One parser, per-field materiality, validated graph |
| Prose | `md [[ ]]` inline or `file` by reference, inheriting the field's materiality | People and models write prose badly inside string literals; mandatory prose stays material |
| Work units vs roles | Spec declares work units; job assigns them to roles | Same spec, different groupings; grouping is a controlled variable |
| Workflow structure | Few hand-written shapes; harness composes a job | Validation needs a fixed vocabulary; evals need like-for-like |
| Projections and checks | Stdlib only; job tier selects, never defines | LLM proposes, code validates |
| Staleness | Identity comparison only; no model judges whether a fragment is still good | OpenProse removed its judge loop for the same reason |
| Identities | Observation, semantic, work fingerprint, render, plus output identity | Each answers a different question; conflating them caused wrong wakes and wrong approvals |
| Hash | XXH3_128 over a versioned canonical encoding; digest is an index, exact bytes compared before reuse | Chosen; one collision unit test, no dedicated matrix |
| C++ vs Lua | Engine owns what a job must not redefine; Lua owns the rest | Three-tier sandbox; engine small enough to rewrite |
| Declared history | Accepted revisions and captured content in SQLite via a Merkle tree; no git | No git for Planar's own history; SQLite never holds a second editable copy of a declaration |
| Runs | Tokens with generations, request IDs, fencing, explicit takeover; no TTL | 24–48 hour jobs die mid-run; a half-dead session must not commit over a replacement |
| Review | Verdict is an atomic shape event; approval is evidence, rejection increments a repair revision | An approval as a subscribed facet would stale the worker |
| Dependencies | Phase and quality on every edge | One `requires` edge overloaded availability and integration |
| Checks | Registry IDs with validated arguments; no `cmd(argv)` in the job tier | Closes the exec hole |
| Toolchain | A trusted registry JSON the driver supplies, allow-listed by ID in the workspace, captured into the accepted revision at load | Hand-written, never harness-editable, material and versioned, and the workspace stays machine-independent |
| Roots and worktrees | Workspace points at external root paths with pinned bases; Planar never creates or manages checkouts; nothing under `~/.planar` is a checkout | The orchestrator owns the environment |
| Workspace location | No default location; a workspace is a file the driver names, with `PLANAR_WORKSPACE_DIR` as the only convenience for bare names; host directories are bound per invocation | Declarations are machine-independent; projects stay unpolluted; ~/.planar/workspaces stays the home by the user's environment, not by Planar's assumption |
| Multi-spec | A workspace is a spec whose roles are specs; imports and exports bound facet-to-facet in `workspace.lua` | Dependencies between facets, never between specs; cascade width is subscriptions |
| Grouping solver | Greedy in stdlib; Mt-KaHyPar on a branch | Must win a ledger-metric comparison first |
| Cut-2 gate | The full adversarial matrix | Chosen over a reduced set |
| Orchestrator prompt | Rendered per job by a stdlib projection; advice, not a requirement | Same driver text across runs keeps experiments attributable |

## Open items

1. Closed 2026-10-21: the legacy tree was removed in R09; the relationship section now describes the replacement destination.
2. Human-performed work is representable only as a frozen path plus a blocking question and, at closeout, `awaiting_external_approval`. Decide whether that is enough before a job needs a human milestone mid-run.
3. Shared libraries consumed through a package registry rather than a checkout need a contract kind whose check is registry-shaped; none is sketched.
4. Exact Lua constructor APIs for `planar.workspace`, `planar.spec`, `planar.job` and `planar.verdict` are illustrative throughout; cut 1 fixes them.
5. The remaining implementation work is concrete: native sandbox and resource limits, canonical byte and hash vectors, the ledger schema and crash tests, check adapters with stated coverage, and a real driver integration. The reference fixture makes the contracts reviewable; it does not substitute for those gates.
6. **Authoring round (decided 2026-10-21).** Reverse prompting has no round in the implementation roadmap; it gets a dedicated round between R14 and R15, depending only on finished declaration and render work. Scope: `init` emits the spec prompt and ends with the critique prompt (stdlib templates rendered like any fragment, carrying the v1 schema, the workspace's allow-listed check IDs and the shape catalog); a new `ingest` emits the job-authoring prompt; `load` rejections are wrapped as prompts naming the field path. Two new matrix rows: a prompt is byte-deterministic for the same declarations and registry; a rejected job's diagnostic prompt names the failing field path. Until then, specs and jobs are hand-written.
7. **Session channel (decided 2026-10-21).** Adopt the driver-only channel (`--observations`, verdict events). Ship `planar observe --from <dir>` as a stdlib verb in the authoring round so the inbox/out convention has one implementation; no Planar-side directory scanning.
