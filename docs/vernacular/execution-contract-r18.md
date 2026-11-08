# Execution contract R18: review and repair

Status: unreviewed draft contract (draft mode, operator decision 2026-11-08); binding for draft lanes, reviewed in the fixing rounds.

Scope: round R18-A of the [execution design](execution-design.md#r18-review-and-repair): clauses R18-1 to R18-8,
operator decision E17 A (count keyed per job activation), E9 A (strict JSON `verdict_v1`, policy identity is the
reviewer's `work_exec_v1` digest) and Q16 A (A24: explicit unsupported-policy refusal). It extends
[execution-contract.md](execution-contract.md) Parts 1 and 2 (cited as EX-n and P2-n) by insertion. Where this file
is silent, Parts 1 and 2 govern. It adds no error code and no message: every refusal below uses a code and message
already in EX-7 or the Messages section of Parts 1 and 2. Choices the sources leave open are marked **DR18-n** (draft
decision) and collected in [Draft decisions](#draft-decisions).

Dependencies on R16 and R17 (unreviewed drafts written in parallel): run-token parsing and authorization of
token-bearing verbs (R16), and the R17 notions *applicable completion*, *output identity of a unit* and *latest
evidence per (work, check)*. R18 calls the R17-7 `checks.lua` functions `current_completion(snapshot, role)`,
`applicable_completion(snapshot, role)` and `unit_output(completion, work)`, reads the R17-7 snapshot keys
`completions` and `evidence`, and uses the R16 token refusal order (R16 refusal step 6) and request-body key `run_id`.

## R18-T Tables, records and invariants

R18 fills the four tables labelled `Reserved for R18` in EX-1 (`verdicts`, `repairs`, `blockers`, `blocker_state`)
and writes `events` rows of type `verdict_recorded` and `requests` rows with operation `verdict` and outcome
`recorded`. **No format3 SQL changes.** The stored schema, triggers and indexes of EX-1 stay byte-exact.

| Table | Row written when | Column meaning |
| --- | --- | --- |
| `verdicts` | every committed `verdict` request, exactly one row | `request_id`: the verdict request ID (the verdict's event ID, E9 A); `run_id`: the reviewer run; `seq`: the `verdict_recorded` event; `body_id`: value record, codec ID `verdict_v1`, holding the normalized document of R18-2; `applicability`: R18-3; `decision`: the document's `decision` |
| `repairs` | admitted rejection only (R18-4), one row per role in the document's `repair` list | `verdict_id`; `job`, `role`: the repaired worker; `generation_id`: the worker's current generation at the verdict; `revision`: the worker's new repair revision in its structural lineage |
| `blockers` | a current rejection that opens a blocker (R18-5) | `seq`: the opening `verdict_recorded` event; `job`; `kind`: `max_rounds` or `unowned_rejection`; `verdict_id`: the opening verdict |
| `blocker_state` | with each `blockers` row (state `open`, `cleared_seq` NULL, `last_seq` = `seq`); updated once when it clears | `state` `cleared`, `cleared_seq` and `last_seq` = the clearing event |

Derived values (no column stores them):

- **Repair revision** of a worker = the `revision` of the latest `repairs` row of that (job, role) whose
  `generation_id` lies in the role's current structural lineage (EX-5 Lineage), or 0. `work_exec_v1.repair` (P2-3)
  is this value as a decimal string; `dispatch_manifest_v1.inputs.repair` likewise.
- **Activation of a verdict** = the latest `job_activations` row of the verdict's job with `event_seq` < `verdicts.seq`.
- **Review count** of the active job = the number of `verdicts` rows of that job whose activation is
  `current_state.active_activation_id` and that have at least one `repairs` row (admitted rejections, E17 A).
- **max_rounds** = `shape.max_rounds` of the active job in the active revision's recorded interpretation
  (`declarations.md:275`, default 2).

Added cross-row invariants (native writer and reader; numbered after EX-1's 20):

21. Each `verdicts` row names a `requests` row with operation `verdict` and outcome `recorded` whose `event_seq`
    equals `verdicts.seq`, and that event has type `verdict_recorded`, `events.run_id` = `verdicts.run_id`. Every
    `verdict_recorded` event has exactly one `verdicts` row.
22. At most one `verdicts` row per run; its run's `run_state` is `completed` at that `seq` (row N11) and the run's
    role structure has `kind` `reviewer`.
23. `repairs` rows exist only for a verdict with `decision` `reject` and `applicability` `current`; their (job, role)
    set equals the normalized document's `repair` list exactly; (verdict_id, job, role) is unique.
24. Repair revisions are contiguous from 1 per (job, role) and structural lineage (EX-1 invariant 8 read through EX-5).
25. A `historical` verdict and an `approve` verdict have no `repairs` and no `blockers` row. A verdict has at most one
    `blockers` row.
26. At most one `open` blocker per (job, kind). `cleared_seq` > `blockers.seq`. Each `blocker_state` row matches one
    `blockers` row and vice versa.
27. Admission replay: replaying `verdicts` in `seq` order with the max_rounds of the revision active at each verdict
    reproduces, per verdict, admitted / capped / unowned exactly as R18-5 states.

## R18-1 Reviewer dispatch

1. **Claimability.** The reviewer's start prerequisite is "every worker role of the job is `done`"
   (`declarations.md:236-240`); reconcile treats it as precedence rule 3 (`blocked`). The reviewer is claimed with
   the R15 `claim` verb (no new option).
2. **Reviewer completion (for precedence rules 4 and 5).** A reviewer's completions are its verdicts. Its *applicable
   completion* is the latest verdict (by `seq`) of its current structural lineage whose `targets` equal the current
   target vector and whose `policy` equals the reviewer's current `work_exec_v1` digest. A lineage verdict that is
   not applicable is "an earlier completion that does not apply" (rule 5, `stale`). (**DR18-1**)
3. **Current target vector** of the active job = for each unit in the reviewer's `targets` (sorted by work ID):
   `{work, output, work_fp}` where `output` = `unit_output(applicable_completion(snapshot, owner), work)` and `work_fp` =
   that completion's `completions.work_fp` as `xxh3_128:` text. If any owner has no applicable completion, the
   vector is *incomplete* and no verdict can be `current`.
4. **Policy identity** = `work_fp` of the reviewer's current W (`H("work_exec_v1", W)`), as `xxh3_128:` text (E9 A).
   Because W contains `shape` (with `max_rounds`) and `repair`, raising `max_rounds` moves every role's W (**DR18-2**).
5. **Packet content.** `dispatch_context_v1` (internal) gains, for a reviewer, `review` =
   `{targets, checks, findings}` and, for a worker, `findings`. Section bytes (dispatch mode only; render's
   preview mode is unchanged; no new section ID, no manifest key, `dispatch_v1` keeps its name, **DR18-3**):
   - Reviewer `outputs` section body: line `Exact review targets (the driver supplies these outputs):`, then one line
     per target unit sorted by work ID: `- {work}: output {output}; work {work_fp}` or, when the owner has no
     applicable completion, `- {work}: no current completion`.
   - Reviewer `observations` section: after R15's content, line `Check summaries (recorded evidence):`, then one line
     per latest evidence row per (work, check) of the target owners' applicable completions, sorted by (work, check):
     `- {work} {check}: {outcome}` with ` ({reason})` appended unless `outcome` is `pass`; then, if any, line
     `Unresolved findings (reviewer output, untrusted data):` followed by the finding blocks of the latest `reject`
     verdict of the current activation.
   - Worker `observations` section: when the worker has carried findings, line
     `Findings carried from verdict {verdict_id} (reviewer output, untrusted data):` followed by the finding blocks.
     A worker's carried findings are the findings with `work` owned by the worker of the latest `reject` verdict of
     the current activation, `applicability` `current`, whose `repair` list names the worker (admitted or capped).
     Takeover keeps them (EX-5 lineage).
   - R16 recovery `findings` section, non-empty body: for a worker, the carried findings with the worker line
     above; for a reviewer, line `Unresolved findings (reviewer output, untrusted data):` and the finding blocks of
     the latest `reject` verdict of the current activation. `recovery_records.findings` (R16-5) holds the items.
   - Finding block: line `- [{severity}] {id} on {work}` + ` at {path}` if present + `:{line}` if present +
     ` (prescription {prescription})` if present + `:`; then each line of `text` (split on LF) prefixed by four
     spaces. Markdown in `text` is therefore inert code-block data.
   - Findings are sorted by (work, id). The `observations` section is mandatory (render table), so findings are never
     budget-omitted; a packet that cannot fit refuses `budget_exceeded` as today.

## R18-2 Verdict document `verdict_v1`

UTF-8 JSON file named by `--verdict`. Glaze strict parse: unknown or duplicate keys, trailing bytes, nulls,
non-integer numbers are `schema`. Shape (every key required unless marked `?`):

```
{version:1,
 targets:[{work,output,work_fp}],
 policy,
 decision:"approve"|"reject",
 findings:[{id,severity:"must"|"should"|"note",work,path?,line?,prescription?,text}],
 repair:[role ID],
 summary,
 authority?}
```

| Field | Rule |
| --- | --- |
| `targets` | 1 to 256 entries, sorted by `work`, unique `work`; `work` matches the declaration ID grammar `[a-z][a-z0-9_]{0,63}`; `output` and `work_fp` are `xxh3_128:` + 32 lowercase hex |
| `policy` | `xxh3_128:` + 32 lowercase hex |
| `findings` | 0 to 256 entries; `id` matches `[A-Za-z0-9_.:-]{1,128}` and is unique; `work` must equal some `targets[].work`; `path` is a portable source path (the render observation path grammar), 1 to 4096 bytes; `line` integer 1 to 2147483647, only with `path`; `prescription` matches the declaration ID grammar; `text` 1 to 4096 bytes, no NUL |
| `repair` | sorted, unique, each the declaration ID grammar |
| `summary` | 0 to 16384 bytes, no NUL |
| `authority` | if present, any string; see R18-7 |

Normalization: the stored value is the canonical_v1 of the parsed document with `authority` set to
`"reviewer_role_v1"` (added when absent). It is stored as a value record, codec ID `verdict_v1`.

Check order, first failure wins (field paths as P2-4: `verdict` for grammar, else `verdict.` + keys, array indexes
as canonical decimals):

1. File capture refusals exactly as the `--provenance` table of P2-4 (`io`, `path_denied`, `capture_changed`),
   same messages.
2. Captured size > 1048576: `resource_limit` `verdict_bytes`.
3. JSON grammar: `schema`, field `verdict`.
4. `findings` count > 256: `resource_limit` `verdict_findings`; any finding `text` > 4096 bytes:
   `verdict_finding_bytes`; `summary` > 16384 bytes: `verdict_summary_bytes`. (Message
   `Execution resource limit exceeded`, `details:{limit,maximum}`.)
5. Shape and field rules of the table: `schema` with the field path.
6. `authority` present and not exactly `reviewer_role_v1`: `unsupported_policy` (R18-7).
7. Semantic rules: `decision` `approve` with any `must` finding, or with a nonempty `repair`, is `schema` field
   `verdict.decision`. A `repair` entry that is not a worker role of the active job owning at least one
   `targets[].work` is `reference`, message `Unknown selected role`, field `verdict.repair.{i}`.

Mismatched targets or policy are **not** refusals: they make the verdict `historical` (R18-3). A refused verdict
writes nothing ("treated as absent and flagged"). Steps 1 to 5 are native (`read_verdict`, bounds and Glaze);
steps 5 to 7 are re-run by `review.lua` (`validate_verdict`), and native refuses a change whose verdict value differs
from its own normalization (`internal`).

## R18-3 Applicability

`applicability` = `current` iff the vector of R18-1 item 3 is complete, `targets` equals it exactly (same works,
outputs and work fingerprints) and `policy` equals the policy identity of R18-1 item 4, all evaluated on the
snapshot of this invocation and re-asserted under the lock by the preconditions of R18-4. Otherwise `historical`:
the verdict row and the reviewer-run completion are written; no repair, approval, count or blocker effect.

## R18-4 The `verdict` commit

One EX-3 commit (A13: all effects or none). Effects, in one transaction:

- value record (`verdict_v1`), `verdicts` row, `verdict_recorded` event, `requests` row (outcome `recorded`);
- reviewer run `active` → `completed` (row N11);
- admitted rejection only: one `repairs` row per `repair` entry, revision = previous lineage revision + 1;
- blocker rows of R18-5, and `blocker_state` updates of blockers cleared by this commit (R18-5 clearing);
- `transitions` and `role_state` delta. Ordinal 1 is the reviewer; then the other roles sorted by role ID.

Transitions (new rows extend the role table; existing rows reused):

| Row | Before | Cause | After | Reason | Event |
| --- | --- | --- | --- | --- | --- |
| M37 (reused, reviewer) | running | approve or non-admitted rejection, `current` | done | completed | `verdict_recorded` |
| M42 (new) | running | verdict whose effects leave a reviewer start prerequisite absent (admitted rejection: repaired workers leave `done`) | blocked | completed | `verdict_recorded` |
| M43 (new) | running | `historical` verdict, workers all done | stale | completed | `verdict_recorded` |
| M39 (reused) | done | repaired worker (admitted rejection) | stale | repair_admitted | `verdict_recorded` |
| M44 (new) | blocked | repaired worker that was blocked with valid evidence; stays blocked, repair revision moves W | blocked | repair_admitted | `verdict_recorded` |
| M17 to M22 (reused) | as listed | `accepted`-quality prerequisites appear or vanish (R18-6) | as listed | prerequisite_available / prerequisite_unavailable | `verdict_recorded` |

N11's paired role reason is `completed`. A named worker that is neither `done` nor `blocked` cannot be in a current
vector; native refuses such a change with `internal`. `verdict_recorded` is a bumping event (EX-3); every role with
a row increments `fact_version` by 1.

Event `verdict_recorded`: version 2, `operation` `"verdict"`, `before`/`after` with unchanged active revision and
job, `references` = `{body, run, verdict}` (decimal strings: value record id, `runs.id`, `verdicts.id`); typed
columns `job` (active job), `role` (reviewer), `generation_id` (reviewer's current generation), `run_id` (reviewer
run). No provenance attachment (P2-4).

Precondition set (`expected`, exact, D151 enforcement): `request_unused(id)`, `active_revision == R`,
`active_job == J`, `fact_version(job, role) == v` for every role of the active job,
`role_state(reviewer) == (running, 0, run)`, `generation_current(reviewer, G)`, `run_active(run, G, secret)`.
Lock-time mapping: failed `run_active` or a `role_state` whose run differs is `fenced` with the locked
`{run_state}`; every other failure is `state_conflict`. Blockers cleared by the commit are also checked `open`
under the lock as part of the delta write (else `state_conflict`).

Replay: `request_v3` parameters `{run_id, verdict_path}` (`run_id` the token's run id as a decimal string; the
secret never enters the body; `verdict_path` normalized absolute). A same-ID equal retry returns the saved bytes
with no second increment; an unequal body is `request_conflict`.

## R18-5 Cap and blockers (E17 A)

For a `current` `reject` verdict, with `count` = review count before the commit:

| Case | Condition | Effect |
| --- | --- | --- |
| unowned | `repair` is empty | not admitted; opens `unowned_rejection` unless one is open for the job |
| admitted | `repair` nonempty and `count < max_rounds` | repairs written; count + 1 |
| capped | `repair` nonempty and `count >= max_rounds` | not admitted, no repair; opens `max_rounds` unless one is open for the job |

With `max_rounds` 2: rejections 1 and 2 are admitted, rejection 3 is capped (A14). Loads and refreshes never reset
the count; `activate` (including re-activation) starts a new count because the count is keyed on the activation.

Clearing (each written by the commit that causes it, as `blocker_state` → `cleared`; the facts of that commit gain
`blockers_cleared:[blocker id]`, see R18-T amendments):

- `max_rounds`: cleared by the first committed load whose new active revision gives the job `max_rounds > count`.
- `unowned_rejection`: cleared by the first commit after which its verdict is no longer `current` against the
  current vector and policy (load, `check_recorded`).
- Any open blocker of a job: cleared by every `activate` commit, and by a load that clears the active job (D94).

A blocker change writes no `transitions` row for a role whose state does not change; only roles with rows bump
(**DR18-4**, settling the open cell of EX-3 "Blocker change"). Blockers never block a start: they are acceptance
blockers (R18-6) and change role states only through `accepted`-quality prerequisites.

## R18-6 Accepted (derived)

A unit `u` of the active job is **accepted** iff all hold:

1. its owning worker is `done` with an applicable completion (R17);
2. some `verdicts` row V with `decision` `approve` and stored `applicability` `current` has `targets` equal to the
   current target vector and `policy` equal to the reviewer's current policy identity;
3. V's run lies in the reviewer's current structural lineage, and `V.seq` is greater than the `opened_seq` of the
   first generation of every target owner's current lineage (A22, A33: no approval crosses a recreation);
4. no blocker of the job is open.

An approval identity-matching a restored vector applies again (**DR18-5**). Acceptance never wakes a role except through
`accepted`-quality prerequisites. Why-not codes (closed, in this order of evaluation, all that hold are listed):
`worker_not_done`, `no_applicable_completion`, `no_current_approval`, `rejected` (the latest current verdict for
the current vector is `reject`), `blocker_open`.

`status_v1` gains (format3, every form): `review` =
`{max_rounds, count, external_approval:"not_configured", acceptance, blockers}` where `max_rounds` and `count` are
decimal strings (`max_rounds` `"none"` when no job is active), `acceptance` is the array sorted by work of
`{work, role, accepted, why}` (`why` `[]` when accepted), and `blockers` is the array of open blockers sorted by id
of `{id, kind, verdict, opened}` (decimal strings). With no active job: `count` `"0"`, `acceptance` and `blockers`
`[]`. `orchestrator --ledger` `live.blockers` uses the same objects.

## R18-7 A24 (Q16 A)

The verdict's authority is always the reviewer role. No external approval assertion API exists. `status` reports
`review.external_approval` = `"not_configured"`. The only refusal surface is `verdict_v1.authority` with a value
other than `reviewer_role_v1`: `unsupported_policy` (`Execution policy is not supported`, exit 3, no details),
check-order step 6 of R18-2. No output labels disabled policy as tested approval.

## R18-8 Orchestrator and history

- `orchestrator --ledger`: `verdict` moves from `future` to `available`. `orchestrator_state` adds, per reviewer
  state, added to the R16-7 lists: `ready` or `stale`: `claim`; `running`: `verdict` (or `takeover` when the session is lost); and one line per
  open blocker: `blocker {id}: {kind} (verdict {verdict}); clears when {condition}` with `{condition}` exactly
  `a later revision raises max_rounds above {count}` or `the reviewed outputs or review policy change`. Repaired
  workers (`stale`) list `claim` and the line `findings carried from verdict {verdict_id}`.
- `history --event` for `verdict_recorded` adds `execution` =
  `{verdict:{id,request_id,run,applicability,decision,body}, repairs:[{role,generation,revision}],
  blockers:{opened:[{id,kind}], cleared:[id]}, provenance:{native}}`; `body` is the decoded normalized document;
  `native` as P2-5 for the reviewer run. `history --request-id` of a verdict returns the saved
  `verdict_response_v1`. `--at-event` state is unchanged (blockers are not part of the replayed state). History
  never runs Lua.

## Verb `verdict`

`planar verdict --ledger PATH --run TOKEN --verdict FILE --request-id ID --actor TEXT`

| Item | Value |
| --- | --- |
| Options | all required, separate tokens; `--ledger` and `--verdict` opaque host-path slots (slot `verdict` joins the closed list); `--run` text in the R15-9 token grammar |
| ABI row | `verdict` / `verdict`, `shipped.verdict.v1`, response kind `verdict_response_v1`, capabilities `execution_commit_v1`, `verdict_read_v1` |
| Services | `read_execution({})`; `read_verdict({verdict:SLOT})`; `commit_execution({change:C})` |
| Root help | the line above, inserted after the R17 `check-recover` line, before the RA01 block (authoring bundle) or `planar --help` (production) (**DR18-6**) |
| With-profile | `verdict` joins the with-profile family list; known slots `ledger`, `verdict` |
| Catalog | one new row: after R17's 16/22 the counts are 17 production and 23 authoring; with-profile family list 17; external command capacity 48 to 47 |

Refusal order: (1) `usage` (includes a `--run` value outside the token grammar, **DR18-7**); (2)
`unsupported_ledger_version`; (3) early replay; (4) `interpretation_mismatch`; (5) `no_active_job`; (6) token, as R16 refusal step 6: a run
id naming no run, or a run whose role is not of the active job, is `state_conflict`; a wrong generation ordinal, a
wrong secret or a run not `active` in the role's current generation is `fenced` `{run_state}`; (7) the
run's role is not the reviewer: `reference` `Unknown selected role`, field `selection.role`; (8) reviewer drifted:
`drifted`; (9) verdict file, R18-2 order; (10) commit preconditions.

Response `verdict_response_v1` (canonical key order):
`{version:1, kind:"verdict", ledger, request_id, job, role, revision, run, event,
verdict:{id, applicability, decision, admitted}, repairs:[{role, generation, revision}],
review:{count, max_rounds}, blockers:{opened:[{id,kind}], cleared:[id]}, roles:[{role, state, drifted}]}`.
Ids, counts, ordinals and revisions are decimal strings; `admitted` is a boolean; `roles` lists the after-state of
every role with a transition in the commit, sorted by role ID. No token appears. Post-commit fallback: the EX-7
`verdict` rows.

## Native and Lua interfaces

Native (C++26 modules, `std::expected` boundaries, errors in the owning module's vocabulary):

| Module | Exported entry points | Errors |
| --- | --- | --- |
| `planar.engine.history.execution_codec` | `codec_verdict_v1`; `encode_verdict_v1(const verdict_v1&) -> expected<string, execution_codec_error>`; `decode_verdict_v1(string_view)`; struct `verdict_v1 {targets, policy, decision, findings, repair, summary, authority}` | `invalid`, `resource_limit` |
| `planar.engine.execution.verdict` (new, `src/engine/execution/verdict.{cppm,cpp}`) | `read_verdict_file(capture_slot) -> expected<value_ptr, execution_error>` (R18-2 steps 1 to 5 bounds and Glaze grammar); `normalize_verdict(value_ptr) -> expected<verdict_v1, execution_error>` | `io`, `path_denied`, `capture_changed`, `resource_limit`, `schema`, `unsupported_policy` |
| `planar.engine.ledger.execution` | snapshot gains `execution_review {count, verdicts, blockers, repairs}` (the DR18-8 `review` key; completions come from the R17 snapshot key `completions`); `validate_verdict_change(const execution_snapshot&, const verdict_v1&, const execution_change&)`; commit writer for operation `verdict`; count and repair-revision derivation; invariants 21 to 27 in `verify_execution_ledger` | `state_conflict`, `fenced`, `internal`, `ledger_corrupt`, `resource_limit` |
| `planar.engine.ledger.recovery` | `recovery_records.findings` filled for R16 `read_recovery_records` | `ledger_corrupt`, `resource_limit` |
| `planar.engine.history.read` | `--event` expansion for `verdict_recorded` | `ledger_corrupt`, `unsupported_interpretation`, `resource_limit` |
| `src/cmd/planar/handlers/verdict/command.{cppm,cpp}` | CLI11 options and dispatch | `usage` |
| `src/engine/runtime/` | ABI row, `read_verdict` service, capability `verdict_read_v1` | existing |

Native step-5 checks for operation `verdict`: the change's verdict equals `normalize_verdict` of the document read
by this invocation; `facts.current.policy` equals `H("work_exec_v1", facts.current.work)` natively; each
`facts.current.targets[i]` equals the output and `work_fp` of a `completions` row in that owner's current lineage;
`applicability` is `current` iff the document's `targets` and `policy` equal `facts.current`; the admission case
equals R18-5 computed from the native count and `facts.max_rounds`; repair revisions are native; transitions
satisfy EX-3. A mismatch is `internal`.

`execution_change_v1` for `verdict`: `operation` `"verdict"`, `facts` =
`{role, run, verdict, current:{targets, policy, work}, max_rounds, case, repairs:[role], blockers_opened:[kind],
blockers_cleared:[id]}` (`case` one of `approve`, `admitted`, `capped`, `unowned`, `historical`), `vector`,
`transitions`, `expected`; no `packet`.

R18 amendments to Part 2 shapes (**DR18-8**): `read_execution` snapshot gains key `review` =
`{count, verdicts:[{id,request_id,run,seq,applicability,decision,body}], blockers:[{id,seq,job,kind,verdict}],
repairs:[{job,role,revision}]}` (verdicts of the active job in the current activation plus the latest verdict of the
reviewer lineage; open blockers only; current lineage repair revisions); the facts of `load`, `activate` and R17's
check completion gain `blockers_cleared:[id]` (empty when none).

Lua (`src/stdlib/planar/review.lua`, new; called by `commands.lua`, `reconcile.lua`, `dispatch.lua`, status and
orchestrator handlers):

| Function | Input | Output |
| --- | --- | --- |
| `validate_verdict(doc, snapshot)` | parsed document, snapshot | normalized verdict, or typed error `{code,message,field}` (R18-2 steps 5 to 7) |
| `target_vector(snapshot, job)` | snapshot | `{complete, targets}` (R18-1 item 3) |
| `policy(snapshot, reviewer)` | snapshot | `{work, digest}` (reviewer current W and its digest) |
| `classify(verdict, snapshot, max_rounds)` | normalized verdict | `{applicability, case, repairs, blockers_opened}` |
| `blockers_to_clear(snapshot, next)` | snapshot and next facts of any commit | array of blocker ids |
| `acceptance(snapshot)` | snapshot | array `{work, role, accepted, why}` |
| `carried_findings(snapshot, role)`, `review_context(snapshot, reviewer)` | snapshot | `dispatch_context_v1` additions of R18-1 item 5 |

`reconcile.lua` uses `acceptance` for `accepted`-quality prerequisites and the reviewer-completion rule of R18-1
item 2; `dispatch.lua` renders R18-1 item 5.

## Bounds

The EX-8 row "Verdict file (R18)" applies unchanged (`verdict_bytes`, `verdict_findings`,
`verdict_finding_bytes`, `verdict_summary_bytes`). The snapshot `review` part counts toward
`execution_snapshot_bytes`. `targets` above 256 entries is a `schema` refusal (R18-2 table), not a new bound.

## Test obligations

Colocated `*.t.cpp` (plain `//` headers, per-test temporary roots) and Lua tests; CLI workflow tests for the verb.

| Clause | Required tests |
| --- | --- |
| R18-T | schema unchanged byte-exact; each invariant 21 to 27 refused by the reader on a hand-corrupted ledger |
| R18-1 | reviewer blocked until all workers done; reviewer packet target lines and check summaries byte-exact; worker packet carries findings after admitted rejection; takeover keeps repair revision, W digest and findings (permissive mutant "takeover generation starts empty") |
| R18-2 | each check-order step refuses with its code, message and field; boundary values 256/257 findings, 4096/4097 text, 16384/16385 summary, 1 MiB/+1; approve with `must`; repair outside target owners; ledger unchanged after every refusal |
| R18-3, A06 | stale output, old `work_fp`, wrong policy each recorded `historical`, no repair, no count, reviewer `stale` |
| R18-4, A13 | admitted rejection writes all effects in one commit; fault before COMMIT leaves nothing; fault after COMMIT before stdout then same-ID retry returns saved bytes and the repair revision stays +1; unequal-body retry `request_conflict`; fenced and wrong-secret tokens; drifted reviewer |
| R18-5, A14 | `max_rounds` 2: third rejection capped and opens `max_rounds`; a contract-only load does not reset; raising `max_rounds` clears it; re-activation resets the count; empty `repair` opens `unowned_rejection` and it clears when the vector changes |
| R18-6, A16, A22, A33 | approval makes units accepted; producer output change withdraws acceptance while the consumer stays fresh; recreated role has no approval; blocker open makes `accepted` false with `blocker_open` |
| R18-7, A24 | `authority:"external"` refuses `unsupported_policy`; `status` shows `not_configured` |
| R18-8, A29 | orchestrator labels and blocker lines; `history --event` reproduces verdict, repairs and blockers in a fresh process |

## Draft decisions

| ID | Decision | Reason |
| --- | --- | --- |
| DR18-1 | The reviewer's "completion" for precedence is its latest lineage verdict; a historical verdict leaves the reviewer `stale` (claimable again), an admitted rejection leaves it `blocked`; new rows M42 to M44 with reason `completed` / `repair_admitted`; N11 pairs with `completed` | EX-4 precedence must cover the reviewer and N11 left its role transition open; no new reason |
| DR18-2 | Raising `max_rounds` moves every role's W through `shape` (P2-3, render W), so workers go `stale` and old verdicts become historical | consequence of the approved W recipe, recorded for review rather than changed |
| DR18-3 | Review content goes into the existing mandatory `outputs` and `observations` sections; no new section or manifest key, `dispatch_v1` not renamed (no R15 packet byte changes because R15 has no completions or verdicts) | avoids a projection identifier bump in draft mode; flagged for review |
| DR18-4 | A blocker change writes no `transitions` row for unaffected roles | settles EX-3's open "Blocker change" cell without amendment |
| DR18-5 | An approval applies again when the exact target vector and policy return, within lineages | identity-based like R17-6 evidence restoration |
| DR18-6 | Root-help placement of the `verdict` line after the R16 and R17 lines | follows the E1 table order |
| DR18-7 | A malformed `--run` token is `usage`; a well-formed token with an unknown run id is `state_conflict` (as P2-1 takeover) | no new code; R16's token rules govern if they differ |
| DR18-8 | Part 2 shapes amended by insertion: snapshot key `review`, `blockers_cleared` in load, activate and check facts, and `status_v1.review` | blockers clear in commits other than `verdict`, and status needs acceptance |
| DR18-9 | Admission counts only rejections with a nonempty `repair`; unowned and capped rejections never count; at most one open blocker per (job, kind) | v5 counts "admitted reject-and-repair cycles" |
| DR18-10 | The request body carries `run_id`, never the token secret | the secret must not enter retained request records |
