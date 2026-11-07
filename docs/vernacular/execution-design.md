# Execution phase: up-front design pass (RA01, R15 to R19)

Written 2026-10-28, while round X07 was in progress. This is a planning input for the coordinator,
search, plan reviewer and reviewer of RA01 and R15 to R19. It is not a contract and amends none.
Where it says a source decides, the cited source wins over anything written here. Where it names an
operator question (E1 to E17 below), nothing dependent is coded until the operator answers.

It extends the [remaining-steps design](remaining-steps-design.md), whose
[operator decisions Q1 to Q18](remaining-steps-design.md#operator-decisions-2026-10-27) (all accepted
2026-10-27) are binding here, together with Q-INIT A and Q-G35 A from the
[X07 addendum](x07-addendum.md) (`x07-addendum.md:5`, `:52`, `:343`). Current status lives in the
[roadmap opening](roadmap.md#current-status).

## How to use this document

- The coordinator cites the relevant step section in each round's dispatch, as Q18 requires for the
  remaining-steps design (`remaining-steps-design.md:762`).
- Search copies the numbered contract-change clauses into the round's invariant table, one row each,
  and its `plan/DECISIONS.md` cites the tag given here for every choice
  ([orchestration](orchestration.md#round-sequence), step 2).
- A choice tagged NEW (engineering) may be accepted by plan review. A choice tagged NEW (operator: En)
  halts the round until En is answered.

Status tags used on every choice:

| Tag | Meaning |
| --- | --- |
| **C** | Decided by a cited source: an exact contract, the v5 baseline, the roadmap or the acceptance map |
| **Q** | Decided by an operator decision: Q1 to Q18 (2026-10-27), Q-INIT or Q-G35 (2026-10-28) |
| **NEW** | Introduced here. "engineering" means plan review may accept it; "operator: En" means it waits for question En |

## Binding inputs carried into every step

| Input | What it fixes for RA01 to R19 | Source |
| --- | --- | --- |
| Q10 B | `init` stays exact; read-only prompt verbs; no file scaffolding in RA01 | `remaining-steps-design.md:754` |
| Q11 A | `observe` is a shipped native family with a bounded, no-follow, explicit-directory read service, stdout only, by command-contract amendment | `:755` |
| Q12 A | A reserved graph-only `orchestrator` projection by render-contract amendment; RA01 prompts may stay embedded-shapes-only | `:756` |
| Q13 A | Execution state lives in a new format3 for new ledgers only; format1 and format2 are refused by execution verbs; no conversion tool | `:757` |
| Q14 A | Separate `claim`, `status` and `release` verbs; render stays database-free | `:758` |
| Q15 A | Check isolation is the v5 minimum: allow-listed IDs, bounded time, output, cwd and environment, every call logged; no OS sandbox; limit recorded | `:759` |
| Q16 A | A24 is explicit unsupported-policy refusal first | `:760` |
| Q17 A | R19 runs the quota fixture in both groupings plus one real spec the operator names before R19 | `:761` |
| Q18 A | Search outputs a decisions table; any new product or authority choice halts before coding | `:762` |
| Q2 A, Q9 A | Q2 A: new invocation stages charge the factory budget owner; CLI11 objects, snapshots, framing, help and response encoding are inherited own-bound stages. Q9 A: declaration-VM Lua memory composes as live acquire and release against the 512 MiB live ceiling | `:746`, `:753` |
| Q3 A | Races and post-commit faults are proved through generated CLI variant binaries with compile-time pause or fault points, plus real two-process races; no product hooks | `:747` |
| Q5 A, Q6 A | Linux is gated in the Docker recipe; Linux-only fixes may land in confined correctives | `:749`, `:750` |
| Q-INIT A | Under `with-profile`, the init ledger target gets the selected-file alias check only | `x07-addendum.md:52` |
| Q-G35 A | A Lua semantic error keeps the existing exit mapping by code: `usage` 2; `io`, `internal`, `ledger_corrupt`, `ledger_busy`, `storage_full` 4; others 3 | `x07-addendum.md:343`; `src/cmd/planar/response.cpp:162-170` |
| XD1 to XD6, P1 to P12 | Threat models, no observability hooks, exhaustion through `resource_limit`, test-name preservation, full-gate meaning, and the twelve standing brief requirements | `remaining-steps-design.md:43-69` |

Two binding facts shape everything below:

1. Render stays database-free (Q14 A; CLAUDE.md product shape). Every ledger-backed read or mutation
   is a new verb. "Accepted-revision render" (`roadmap.md:368`) is therefore a new read verb, not a
   `render` flag. The render contract still refuses `--claim`, `--refresh` and `--since` as unsupported
   render options (`render-contract.md:84-86`), and that stays true.
2. No Lua runs under a write lock, and native code checks the invariants it owns
   (`ledger-contract.md:72-77`; `command-contract.md:233-238`; v5 `planar-design-spec-v5.md:565`).
   Every execution mutation below follows one protocol (EX-3) built on that rule.

## Cross-cutting execution design

These mechanisms are shared by R15 to R18. Deciding them once keeps each contract round from
reopening them. R15-A writes EX-1 to EX-8 as exact contract text; later rounds only extend them.

### EX-1 Format3

- Format3 is a new schema for new ledgers only; format1 and format2 ledgers stay readable and keep
  their load behaviour; every execution verb refuses them with `unsupported_ledger_version` before
  any capture or Lua. **Q** (Q13 A).
- Format3 is the format2 schema (`external-profile-contract.md:564-573`) with the `events`,
  `requests` and `current_state` tables modified as listed below (their format2 CHECKs, for example
  `requests.outcome IN ('accepted','noop')` and `events.version=1` at `ledger-contract.md:224-246`,
  cannot hold execution rows), plus the added execution tables, `ledger_meta.format CHECK(format=3)`,
  `PRAGMA user_version=3`, same application ID. All other format2 SQL text stays exact.
  The records codec check becomes
  `CHECK((kind=4 AND codec IN (2,3)) OR (kind BETWEEN 1 AND 3 AND codec=1))`: format3 never writes
  request_v1. **NEW** (engineering; follows the X-STORE pattern).
- After the R15 cutover, public `init` creates format3 only, at a new explicit target, with
  unchanged CLI signature and response bytes and no format selector. Native fixtures keep explicit
  format1 and format2 creation for reader checks. **Q** (Q13 A "new ledgers only") plus **C**
  (the same mechanical cutover rule as X-STORE, `external-profile-contract.md:647-650`). Existing
  tests that pin "init creates format2" keep their names and change only that assertion (XD5).
- An old binary opening format3 refuses it unchanged (`ledger-contract.md:487-498`). **C**.

Format3 additions (normative SQL is written by R15-A; this is the required content). All tables are
STRICT, IDs allocate `MAX+1` under the write transaction, and every immutable table gets the
`<table>_no_update` and `<table>_no_delete` triggers (`ledger-contract.md:249-256`). **NEW**
(engineering) unless tagged.

| Table | Mutability | Required columns and invariants |
| --- | --- | --- |
| `events` | immutable | Existing columns, `type` enum extended (EX-4), `version CHECK(version IN (1,2))`, plus nullable typed references `job`, `role`, `generation_id`, `run_id`. Execution events must use version 2 payloads (EX-4). Required by `ledger-contract.md:322-325` ("must add typed affected role/run, acknowledged revision, exact before/after state and input/output/check/verdict references"). **C** for the requirement, NEW for the columns. |
| `requests` | immutable | `request_id PK`, `operation` (closed enum: `load`, `activate`, `claim`, `takeover`, `refresh`, `checkpoint`, `release`, `check`, `check-recover`, `verdict`), `body_id → records`, `event_seq UNIQUE → events`, `revision_id → revisions` (the active revision the operation ran against), `outcome` (closed enum per operation), `result BLOB ≤ 1 MiB`, `attachment_id → records NULL` (the retained packet record for claim, takeover and refresh). One request namespace per ledger across load and execution. |
| `job_activations` | immutable | `id`, `job TEXT`, `revision_id`, `event_seq`; the current active job is a `current_state` column. |
| `generations` | immutable | `id`, `job`, `role`, `ordinal` (per job and role, from 1, contiguous), `structure_id → records` (EX-5 structure value), `opened_seq`, `reason` (`activated`, `structural_change`, `takeover`, `recreated`). |
| `runs` | immutable | `id`, `generation_id`, `request_id`, `claimed_seq`, `secret BLOB(16)` (E11 A), `provenance_id → records NULL`. |
| `dispatches` | immutable | `id`, `run_id`, `seq`, `revision_id`, `kind` (`claim`, `takeover`, `refresh`), `packet_id → records` (exact Markdown file record), `manifest_id → records` (canonical manifest value), `work_id → records` (W value), `work_fp BLOB(16)`, `render_fp BLOB(16)`, `checkpoint_id → records NULL`. This is the retained dispatched context of v5 `planar-design-spec-v5.md:276`. |
| `checkpoints` | immutable | `id`, `run_id`, `seq`, `body_id → records` (canonical checkpoint value, ≤ 8 KiB, the render-contract checkpoint schema `render-contract.md:214-221`). R16. |
| `reservations` | immutable | `id`, `request_id UNIQUE`, `run_id`, `opened_seq`, `body_id`. R17. |
| `evidence` | immutable | `id`, `reservation_id`, `seq`, `work`, `check`, `definition_id → records` (exact check and toolchain records), `vector_id → records` (sorted output vector), `outcome` (`pass`, `fail`, `inconclusive`), `reason` (closed enum, R17), `matched INTEGER NULL`, `exit_status INTEGER NULL`, `duration_ms`, `output_id → records NULL`, `output_bytes`, `output_truncated`. R17. |
| `completions` | immutable | `id`, `run_id`, `seq`, `dispatch_id` (acknowledged), `work_fp`, `observation_id → records` (unit output vector value). R17. |
| `publications` | immutable | `id`, `completion_id`, `work`, `facet`, `semantic BLOB(16)`, `payload_id → records` (exact facet file tree). R17. |
| `verdicts` | immutable | `id`, `request_id UNIQUE`, `run_id` (reviewer run), `seq`, `body_id → records`, `applicability` (`current`, `historical`), `decision` (`approve`, `reject`). R18. |
| `repairs` | immutable | `verdict_id`, `job`, `role`, `generation_id`, `revision INTEGER` (per role and generation, from 1). R18. |
| `blockers` | immutable | `id`, `seq`, `job`, `kind` (`unowned_rejection`, `max_rounds`), `verdict_id`, `cleared_seq NULL` via `blocker_state`. R18. |
| `current_state` | mutable singleton | Existing columns plus `active_job TEXT NULL`, `active_activation_id NULL`. |
| `role_state` | mutable, no delete | `(job, role) PK`, `generation_id`, `state` (`blocked`, `ready`, `running`, `done`, `stale`, `retired`), `drifted 0/1`, `run_id NULL`, `fact_version INTEGER`, `last_seq`. The derived projection of EX-4. |
| `run_state`, `reservation_state`, `blocker_state` | mutable, no delete | Status projections: run `active`, `released`, `interrupted`, `completed`, `fenced`; reservation `open`, `completed`, `interrupted`; blocker `open`, `cleared`. |
| `transitions` | immutable | `(event_seq, ordinal) PK`, `job`, `role`, `generation_id`, `before_state`, `after_state`, `before_drifted`, `after_drifted`, `reason` (closed enum, EX-4). |

Cross-row invariants the native writer enforces and the reader verifies, in the style of
`ledger-contract.md:258-285` (**NEW**, engineering): each request names exactly one event and the
event names the request, with one stated exception: a `check` request row references its
`check_reserved` event, and the later `check_recorded` or `check_interrupted` event references the
reservation (and through it the request), never a second request row (R17-1, R17-7); at most one
active run per (job, role); an active run's generation is the
role's current generation; a dispatch's run is active at its sequence; every `transitions` row's
`before_state` equals the previous after-state of that role; `role_state` equals the replay of all
transitions; a completed reservation has exactly one terminal request; repair revisions per role and
generation are contiguous; `current_state` equals the final event's after-state. Reconstruction at
`history --at-event` replays these rows and never runs Lua (`ledger-contract.md:402-408`, **C**).

### EX-2 Request framing and replay for execution operations

- Execution operations use the same request semantics as load: the driver chooses the ID before
  dispatch; a retry with an equal body returns the original saved response bytes; a different body
  under the same ID refuses `request_conflict`; failed operations do not reserve their ID; a reused
  ID never relaunches anything. **C** (v5 `planar-design-spec-v5.md:368-370`;
  `ledger-contract.md:343-348`, `:393-400`).
- New `request_v3` framing, stored as record kind 4 codec 3: ASCII `PLANAR-REQUEST`, byte 03, then
  `bytes(operation) || bytes(actor) || bytes(parameters) || selector_tag || [bytes(profile_argument)]`,
  where `parameters` is the canonical_v1 encoding of the operation's exact parameter map (listed per
  verb below) and `selector_tag` is as in request_v2 (`external-profile-contract.md:552-562`).
  Host-path arguments (ledger excluded, root, base-root, scratch, file selectors) enter the map as
  lexically normalized absolute private strings, exactly as load does; file **contents** never enter
  the body, so a retry after the file changed replays the original result. **NEW** (engineering;
  mirrors `ledger-contract.md:337-342`). Load on format3 keeps request_v2.
- A body of another version under the same ID conflicts (`external-profile-contract.md:560-561`). **C**.
- Responses that carry a packet are assembled natively from the compact saved `result` plus the
  immutable `attachment_id` record, so replay is byte-identical without storing Markdown in the 1 MiB
  result column. **NEW** (engineering).
- `status --request-id` (EX table in E1) returns the saved result of any operation plus its current
  disposition: for claim and takeover requests one of `current`, `completed`, `released`,
  `interrupted`, `fenced` (v5 `planar-design-spec-v5.md:370`). **C** for the content and for the
  `status --request-id` spelling (v5 `planar-design-spec-v5.md:368`); whether `history --request-id`
  (`ledger-contract.md:446`) also returns execution results is part of E1.

### EX-3 The execution commit protocol

Every execution mutation (activate, claim, takeover, refresh, checkpoint, release, the two check
commits, verdict, and format3 load reconciliation) uses this order. **NEW** (engineering), built on
**C** rules: no Lua under lock, replay before CAS, native-owned invariants
(`ledger-contract.md:343-391`; `command-contract.md:233-238`; v5 `planar-design-spec-v5.md:561-565`).

1. **Prelude.** CLI11 parse; cwd and environment snapshot; normalize host arguments; verified
   read-only open; format3 required; exact request lookup (equal replays saved bytes, unequal
   conflicts). The read snapshot closes. Pure embedded registration may precede it.
2. **Snapshot.** Native reads one `execution_snapshot_v1` in a single read transaction at
   `last_seq = N`: active revision (recorded interpretation, versions, registry records), active job,
   per-role generation, state, drifted flag, `fact_version`, active run and acknowledged dispatch
   (W value and digests), latest completion, publications, latest evidence per unit, current repair
   revision and carried findings, open blockers. Bounded (EX-8). Tokens' secrets never enter it.
3. **Capture.** Only the verbs that need it (status with drafts, check, recover with a root) capture,
   outside any lock, with retained receipts.
4. **Trusted Lua.** The registered handler receives the snapshot, the typed options and capture
   results, and returns one `execution_change_v1` value: the operation's typed fact records, the
   **complete** next role-state vector for the active job, the `transitions` it implies, rendered
   packet bytes if any, and an `expected` precondition set. Lua is the reconciler (v5 glossary
   `planar-design-spec-v5.md:62`; product shape in CLAUDE.md).
5. **Native validation.** Closed schema, references, bounds, digests recomputed natively (render digest
   from the packet bytes, W digest from the W value), structure of transitions (before states must equal
   the snapshot), no fabricated facts (for example, a completion requires native evidence from this
   invocation).
6. **Locked commit.** `BEGIN IMMEDIATE`; identity, format and schema recheck; request relookup (equal
   replays before any CAS); verify every precondition natively; verify retained capture receipts;
   append facts, events, transitions and request; update projections; commit. Failed precondition
   rolls back everything and refuses (EX-7). The projections are updated as a **delta**: native
   writes `role_state` only for roles that have a `transitions` row in this change, and verifies under
   the lock that each such row's `before_state`/`before_drifted` equals the locked current row (an
   implicit `role_state` precondition per transitioned role). Lua's complete next-state vector is
   validated against the snapshot in step 5 but is never written wholesale, so a concurrent commit on
   another role cannot be overwritten by a stale vector (lost-update guard).
7. **Output.** Native saved bytes plus LF; the load-style completion receipt and post-commit fallbacks
   apply (`load-command-contract.md:195-230`), with operation-specific fallback text in the closed
   message table.

The precondition set (closed vocabulary, all checked under the lock) is:
`active_revision == R`, `active_job == J`, `fact_version(job, role) == v` for **every** role of the
active job, `role_state(target) == (state, drifted, run)`, `generation_current(target, G)`,
`run_active(run, G, secret)`, `reservation_open(id)` and `request_unused(id)`.
`fact_version` is bumped only by facts that can change another role's derived state: generation
change or retirement, completion, publication, repair admission, verdict, blocker change and
revision acceptance. Claims, refreshes, checkpoints, releases and reservations change only the
target role, so concurrent operations on different roles do not conflict. Two concurrent claims of
one role conflict on `role_state(target)`; native reports that loser as `claim_conflict`
(one winner, A09), not as a generic retry. **NEW** (engineering). This is v5's
`engine.ledger.commit(expected, changes)` (`planar-design-spec-v5.md:561`) with a closed `expected`.

Any other precondition failure is `state_conflict` (EX-7): nothing changed; retry with the same
request ID. The check completion commit is the one exception that retries internally (R17).

### EX-4 Facts, derived state and the reconcile pass

- States are `blocked`, `ready`, `running`, `done`, `stale`, `retired`; a run is an ownership record,
  not a state; drift is a flag on a running role; `accepted` is derived and distinct from `done`.
  **C** (v5 `planar-design-spec-v5.md:61`, `:364-387`, `:436`).
- Facts are the immutable rows of EX-1. `role_state` is a projection that only EX-3 writes, from
  Lua's complete next-state vector, so it is always current with respect to the facts. A status read
  never needs Lua to report state; explanation does. **NEW** (engineering) satisfying v5
  `planar-design-spec-v5.md:513` ("current-state tables are maintained transactionally").
- The reconcile rules (trusted Lua, `src/stdlib/planar/reconcile.lua`) follow the v5 table
  `planar-design-spec-v5.md:372-385` exactly, in this precedence: retired generation → `retired`;
  active run of the current generation → `running` (drifted iff current W ≠ acknowledged W);
  start prerequisite unavailable → `blocked` (evidence preserved); applicable completion → `done`;
  any earlier completion in this generation → `stale`; otherwise `ready`. A running role with a new
  blocker stays running with the blocker recorded. **C**.
- Prerequisites resolve from ledger facts, not from any checkout: contract `available` is the
  captured contract text in the active revision; facet `available` is an existing publication; `done`
  is the producer role's applicable completion; `accepted` is R18's derived acceptance; unanswered
  blocking questions are declaration facts. **C** for the qualities
  (`declarations.md:217-228`), NEW for "publications, not checkouts" (operator: E5).
- Event types added in format3: `job_activated`, `run_claimed`, `run_taken_over`, `run_refreshed`,
  `checkpoint_recorded`, `run_released`, `run_interrupted`, `check_reserved`, `check_recorded`,
  `check_interrupted`, `verdict_recorded`. A format3 load keeps its single `declaration_accepted` or
  `load_noop` event and attaches its `transitions` rows to it; activation uses `job_activated`.
  One operation is one event (the check operation is the stated two-event exception, EX-1); every
  role whose derived state changed gets one `transitions` row in that event (so revision, active
  pointer and reconciliation commit together, A26). Event payload v2
  is exactly `{version:2,operation,before:{active_revision,active_job},after:{...},reason,references}`,
  with references as record or row ID strings. **NEW** (engineering) within
  `ledger-contract.md:322-325` (**C**).
- Transition reasons (closed): `activated`, `prerequisite_available`, `prerequisite_unavailable`,
  `claimed`, `taken_over`, `fenced`, `released`, `interrupted`, `completed`, `work_changed`,
  `evidence_inapplicable`, `repair_admitted`, `structural_change`, `retired`, `recreated`,
  `drift_cleared`. `explain` (in `status --role`) reports them, together with the top-level W fields
  that changed, and keeps separate the three causes v5-alignment requires: work or semantic change,
  output-vector applicability, and prerequisite availability (`v5-alignment.md:130-132`). **C** for
  the distinction, NEW for the vocabulary (engineering).

### EX-5 Role structure and the execution work fingerprint

- Role structure value `role_structure_v1` = `{job, role, kind, work: sorted unit IDs (workers),
  targets: sorted unit IDs (reviewer), ownership: sorted {work, paths}, shape: job shape kind}`.
  A changed structure digest opens a new generation and fences the active run; a removed role
  retires its generation; a role re-added under an old name gets a new generation and inherits no
  token, completion or approval. Ordinary requirement drift is incorporated in place. **C**
  (v5 `planar-design-spec-v5.md:191`, `:385`, `:387`; A22, A33), NEW for the exact field list
  (engineering).
- Generations opened by takeover (reason `takeover`) only fence: the structure digest is unchanged,
  and the new generation carries forward the previous generation's completions, evidence, publications,
  repair revision, carried findings and approvals. Only `activated`, `structural_change` and
  `recreated` generations start empty, and `retired` ends one. Without this, a takeover after an
  admitted repair would reset the `repair` field of W (below) to its pre-rejection value, could make
  pre-rejection evidence applicable again and would drop carried findings, breaking "rejected feedback
  is applied exactly once" (v5 `planar-design-spec-v5.md:627`). Wherever this document says "in this
  generation" or "per role and generation" (EX-4 precedence, `repairs`, the `repair` field), read it
  as "in this structural lineage", the chain of takeover generations since the last empty one.
  **C** for the fencing-only meaning of takeover (v5 `planar-design-spec-v5.md:59`, `:370`), NEW
  (engineering) for the lineage rule.
- Execution W is a new versioned recipe `work_exec_v1`: the render W fields
  (`render-contract.md:337-395`) with three changes. `base` is the declared form
  `{status:"declared",root,label}`. `start_facets` take their semantic values from the current
  publication of each start/facet dependency (`{dependency,semantic}` or `{dependency,unavailable:true}`).
  A new field `repair` holds the role's repair revision in its current generation as a decimal string
  (`"0"` before any repair). `versions` gains `"work_exec_v1"`. It is computable from the ledger
  alone, so load, activate, status and claim can all compute it without a checkout. Base content still
  enters every output identity (`render-contract.md:311-326`), so a changed base invalidates evidence
  without moving W. **NEW** (operator: E5). Render-only W (`work_v1`) is unchanged and remains what
  `render` emits (`render-contract.md:396-406`, **C**: "later execution must add those through a
  versioned recipe").

### EX-6 Interpretation binding for execution

Every execution verb, and `preview` and `recover`, interprets the active revision with the exact
trusted code that accepted it: the invocation's effective stdlib digest (embedded, or with-profile)
must equal the active revision's recorded `stdlib_digest`, otherwise the verb refuses before any
mutation with `interpretation_mismatch` (EX-7). The check runs after early request replay, so a saved
result still replays across a binary or profile change, as for load (`load-command-contract.md:105-106`).
A revision accepted under a profile therefore requires `with-profile` with the same profile bytes, and
the new shipped families join the closed factory with their own X-CAP admission rows; the factory's
family variant is closed at five, so each added family needs an architecture revision, an admission
row and a contract amendment (`x07-addendum.md:60`). A binary or profile change is absorbed by a fresh load (a new
revision, reconciled normally). **NEW** (operator: E3). Basis: v5 `planar-design-spec-v5.md:507`
("a changed stdlib ... participates in revision interpretation"), `ledger-contract.md:104-106`
("never silently regenerated under current Lua"), Q1 B (closed factory).

### EX-7 Error codes and messages

Execution keeps the closed error envelope and the exit policy: usage 2; semantic, conflict, resource
and format refusals 3; I/O, corruption, busy and disk-full 4 (`ledger-contract.md:470-477`;
`render-contract.md:615-626`), and Q-G35 A for Lua semantic errors. Existing codes are reused where
their meaning fits: `unsupported_ledger_version` (format1/2), `request_conflict`, `reference` (unknown
job or role), `schema` (malformed verdict, observations, provenance), `observation_target_mismatch`
(wrong output vector in a check view), `resource_limit` (every new bound, per XD4), `capture_changed`,
`ledger_busy`, `storage_full`. The following codes are new and each is a flagged contract amendment
(**NEW**, operator: E16, because XD4 says "No new public code or message is added", `remaining-steps-design.md:67`; each code is then reviewed in R15-A and later contract rounds):

| New code | Exit | Meaning | Introduced |
| --- | --- | --- | --- |
| `no_active_job` | 3 | Execution verb with no active job, or `--job` not the active job | R15 |
| `claim_conflict` | 3 | Role not claimable (`blocked`, `running`, `done`, `retired`); details `{state}` only, never another run's token | R15 |
| `state_conflict` | 3 | A precondition changed between snapshot and commit; retry the same request ID | R15 |
| `fenced` | 3 | Token names a run that is not active or whose generation is not current; details `{run_state}` | R15 |
| `interpretation_mismatch` | 3 | EX-6 binding failed | R15 |
| `history_unavailable` | 3 | `--since` names a render digest that was never dispatched for this role (v5 `planar-design-spec-v5.md:276`) | R16 |
| `drifted` | 3 | Completion or refresh refused because acknowledged W is not current, or `--expect-fp` mismatched (A11) | R16 |
| `check_in_progress` | 3 | An open reservation exists for this run or request ID | R17 |
| `unsupported_policy` | 3 | External approval assertion while no assertion policy exists (Q16 A, A24) | R18 |

New native sites use a closed message table, one fixed message per code and site with no
substitutions, in the X-LIMIT style (`external-profile-contract.md:526-541`). Exhaustion keeps the
existing `resource_limit` code; because the X-LIMIT messages name external profiles, the execution
table adds one message `Execution resource limit exceeded` for execution native sites. That message
is a flagged amendment to XD4 (`remaining-steps-design.md:67`). **NEW** (operator: E16).

### EX-8 Execution bounds

All checked before allocation with checked addition; lower inherited bounds win; nothing resets the
invocation budget (Q2 A, X-LIMIT preamble `external-profile-contract.md:500-502`). **NEW** numbers
(engineering) unless tagged.

| Boundary | Bound |
| --- | --- |
| Execution snapshot | Recorded interpretation as stored (≤ 16 MiB, **C** `ledger-contract.md:162`) plus execution state ≤ 16 MiB canonical; conversion into the VM charges the factory owner |
| Dispatch packet | The role budget (≤ 1 MiB, **C** `declarations.md:276`); manifest value ≤ 16 MiB; the complete claim, takeover, refresh, preview or recover response stays within the 16 MiB output ceiling (**C** `external-profile-contract.md:515`) |
| Checkpoint | Exactly the render checkpoint schema, ≤ 8 KiB canonical (**C** `render-contract.md:214-221`) |
| Provenance file (E6) | ≤ 64 KiB, depth 4, ≤ 32 settings, strings ≤ 1024 bytes |
| Verdict file (R18) | ≤ 1 MiB; ≤ 256 findings; finding text ≤ 4096 bytes; summary ≤ 16 KiB |
| Observe directory (RA01) | ≤ 1024 entries; each file ≤ 64 KiB; total ≤ 4 MiB; output observations ≤ 1 MiB canonical, the observations input limit every consumer applies (**C** `render-contract.md:138-139`), so observe never emits a file that `--observations` would refuse |
| Check output per check | The registry `max_output_bytes` (≤ 16 MiB, **C** `declarations.md:350-351`); per invocation ≤ 64 MiB retained |
| Facet payload per publication | ≤ 1 MiB per file, ≤ 4 MiB per facet |
| Check observation (E15) | As E15 decides |
| History event expansion | Whole response ≤ 16 MiB (**C** `ledger-contract.md:445`) |

### EX-9 Module placement against the current graph

Current layers are in `docs/architecture.md:11-25` and the module graph in `src/engine/CMakeLists.txt`.
Dependencies stay downward; the native engine owns isolation, capture, encoding, hashing, storage,
subprocesses and output; trusted Lua owns schemas, shapes, selection, reconciliation, projections and
prompt templates (CLAUDE.md product shape; v5 `planar-design-spec-v5.md:74-84`). **NEW** (engineering).

| Destination | Owns | Depends on |
| --- | --- | --- |
| `src/engine/history/request_v3.{cppm,cpp}` | request_v3 framing, pure | `history/record`, canonical, hash |
| `src/engine/history/execution_codec.{cppm,cpp}` | Canonical encode and decode of `execution_change_v1`, snapshot, structure, W-exec, verdict and provenance values; pure | canonical, value |
| `src/engine/ledger/execution_schema.*`, `execution.*` | Format3 schema and opener, snapshot read, precondition verification, execution commit, cross-row verification | `ledger/sqlite`, `ledger/types`, `history/*` codecs |
| `src/engine/ledger/acceptance*` | Format3 load: reconciliation changes committed in the acceptance transaction | existing; triggers the Sonnet coder escalation (`orchestration.md:26-28`), as does any package touching `src/engine/runtime/` |
| `src/engine/history/read.*` | Format3 event pages, `--at-event` with role state, `--event` expansion | existing |
| `src/engine/capture/directory.*` | RA01 observe: bounded no-follow explicit-directory listing and reads | `capture/native`, `capture/admission` |
| `src/engine/process/` (new) | R17 bounded subprocess runner: posix_spawn, process group, cwd descriptor, replaced environment, timeout, output caps, macOS and Linux branches | value only |
| `src/engine/execution/` (new, mirrors `acceptance/`) | Finite execution services: snapshot, change validation, check orchestration (reserve, materialize, run, recapture, evidence), packet record handling | ledger, history, capture, process |
| `src/engine/runtime/` | Native ABI pairings for every new family (`runtime.cpp:1333-1344`), closed factory admission rows (EX-6), budget threading | existing |
| `src/cmd/planar/handlers/<family>/command.{cppm,cpp}` | One family per new verb, name equal to family as today (`commands.lua:56-80`) | response helpers below handlers |
| `src/stdlib/planar/` | `prompts.lua`, `orchestrator.lua`, `observe.lua` (RA01); `reconcile.lua`, `dispatch.lua` (R15); `recovery.lua` (R16); `checks.lua` (R17); `review.lua` (R18); registrations in `commands.lua` | existing stdlib |

### EX-10 Shared test infrastructure

- Fault points and pauses are compile-time definitions in generated variant targets, following
  `planar_cli_variant_*` and `planar_load_cli_probe`; no product flag, field or environment hook
  (Q3 A; XD3). Each execution round adds its named points to one shared generator under
  `tests/native/<round>-support/`. **Q**.
- Points needed across R15 to R18: after snapshot before lock (to force overlap), after commit before
  stdout (lost response), before COMMIT (rollback), after reservation commit before spawn, after child
  exit before the completion transaction, and inside the subprocess wait. **NEW** (engineering).
- Two-process races use real processes and a pause-point variant so the interleaving is forced, not
  timed. Crash tests use `SIGKILL` of the real `planar` process (including while a check child runs)
  and reopen in a fresh process. **NEW** (engineering) under Q3 A.
- Subprocess fixtures are small Python scripts under `tests/native/<round>-support/`, with injected
  small `timeout_ms` so no test joins #531 (P12). Use `perl -e 'alarm N; exec @ARGV'` style wrappers in
  harness scripts where a timeout is needed; `timeout` is not installed on this host. **NEW**
  (engineering).
- Determinism (A01, A35): fresh-process byte equality for prompts, packets and manifests. Tokens are
  outside packet bytes so random secrets do not disturb this (E11).
- Mutation probes prefer permissive mutants (precondition always passes, fence check skipped,
  inconclusive treated as pass, stale verdict admitted), because inverted mutants tend to die on the
  direction a fixture already asserts. Filter probe runs by label and check the matched count against
  the baseline, so a filter miss is not read as a survivor. **NEW** (engineering; probe practice).

## Proposed verb surface (subject to E1)

Each verb is one shipped family with its own native ABI row, exact option signature and finite
service list (`command-contract.md:37-82`, `:159-249`; `load-command-contract.md:17-72`). All options
are separate tokens, scalar values nonempty and not starting `--`, no defaults, aliases, positionals,
repeats or short flags (**C**). "Mutation" verbs require `--request-id` and `--actor` with the load
grammar (`ledger-contract.md:163`, **C**). Tokens use E11's grammar. Every new verb changes the fixed
root help bytes. Those bytes are fixed by two exact contracts, `command-contract.md:94-96` ("current
fixed embedded usage bytes") and X-CLI `external-profile-contract.md:133-134` ("Default root help stays
byte-for-byte old"), so each adding round's contract round amends both explicitly, as the load
amendment did when it added its usage line; the X07 must-not (`remaining-steps-design.md:99-102`)
restates the X-CLI clause for X07 and does not by itself permit later changes. New host-path options
(`--from`, `--diagnostic`, `--provenance`, `--verdict`, `--scratch`) also extend the closed opaque-slot
list of `command-contract.md:180-182`. **NEW** (operator: E1) for names and grouping.

| Round | Verb | Kind | Options (`!` required) | Response kind |
| --- | --- | --- | --- | --- |
| RA01 | `spec-prompt` | read, database-free | `--workspace!`, `--registry!` | `prompt` |
| RA01 | `critique` | read, database-free | `--workspace!`, `--registry!` | `prompt` |
| RA01 | `ingest` | read, database-free | `--workspace!`, `--registry!` | `prompt` |
| RA01 | `explain-rejection` | read, pure (E4) | `--diagnostic!` | `prompt` |
| RA01 | `observe` | read, database-free | `--from!` | `observations` |
| RA01 | `orchestrator` | read | database-free form `--workspace`, `--registry`, `--job`; R15 adds ledger form `--ledger`; exactly one form | `prompt` |
| R15 | `activate` | mutation (E2) | `--ledger!`, `--job!`, `--expect-revision!`, `--expect-job!` (`none` or ID), `--request-id!`, `--actor!` | `activate` |
| R15 | `status` | read | `--ledger!`, optional `--role`, `--request-id`, `--workspace`+`--registry` (drafts, both or neither); `--role` excludes `--request-id` | `status` |
| R15 | `preview` | read | `--ledger!`, `--role!`; R16 adds `--since` | `preview` |
| R15 | `claim` | mutation | `--ledger!`, `--job!`, `--role!`, `--request-id!`, `--actor!`, `--provenance` | `claim` |
| R15 | `takeover` | mutation | `--ledger!`, `--job!`, `--role!`, `--run-id!` (current run identity), `--request-id!`, `--actor!`, `--provenance` | `claim` |
| R16 | `refresh` | mutation | `--ledger!`, `--run!` (token), `--expect-fp!`, `--request-id!`, `--actor!`, `--observations` | `refresh` |
| R16 | `checkpoint` | mutation | `--ledger!`, `--run!`, `--observations!`, `--request-id!`, `--actor!` | `checkpoint` |
| R16 | `release` | mutation | `--ledger!`, `--run!`, `--reason!` (`released` or `interrupted`), `--request-id!`, `--actor!`, `--observations`, `--provenance` | `release` |
| R16 | `recover` | read | `--ledger!`, `--role!`, optional `--root`+`--base-root` (re-observe) | `recover` |
| R17 | `check` | mutation | `--ledger!`, `--run!`, `--root!`, `--base-root!`, `--scratch!` (E7), `--request-id!`, `--actor!`, `--provenance` | `check` |
| R17 | `check-recover` | mutation | `--ledger!`, `--run-id!`, `--reservation!`, `--confirm-stopped` (required flag value true), `--request-id!`, `--actor!` | `check` |
| R18 | `verdict` | mutation | `--ledger!`, `--run!` (reviewer token), `--verdict!`, `--request-id!`, `--actor!` | `verdict` |

Notes: `--confirm-stopped` is a flag and flags cannot be required (`external-profile-contract.md:258-259`),
so native refuses its absence as usage. `history` and `export` keep their signatures; `history` gains
one selector `--event` in R15 (A29). Existing `load` keeps its six options; on format3 it adds
reconciliation services, not options.

## RA01: authoring and driver prompts

### 1. Scope and rows

Spec, critique and job-ingest prompts; rejection prompts naming the field path while keeping raw typed
diagnostics and refusal exits; init coexistence; driver-invoked `observe` from an explicit directory;
the initial graph-only orchestrator projection with accurate available/future labels
(`roadmap.md:347-363`; v5 `planar-design-spec-v5.md:744-745`; `implementation.md:307-316`). **C**.
Rows: A35, A36 (`acceptance-matrix.md:57-58`), plus the two unnumbered deliverables (observe and
orchestrator projection, `acceptance-matrix.md:18-19`); preserve A01, A04, A07 (`:23-30`).
Excluded: watchers, model calls, scaffolding (Q10 B), execution state or verbs.

### 2. Contract changes

Destination per E14: a new exact `authoring-contract.md` with pointer amendments in
`command-contract.md` (catalog rows) and `render-contract.md` (projection rows). Clauses:

RA-1 Catalog. Add native ABI rows `spec-prompt`, `critique`, `ingest`, `explain-rejection`,
`observe`, `orchestrator`, each `shipped.<name>.v1`, `<name>_v1` response kind mapped to envelope kind
`prompt` (the four prompt verbs and orchestrator) or `observations` (observe), with capabilities
`authoring_capture_v1` (three authoring verbs), `diagnostic_read_v1`, `observation_directory_v1`,
`orchestrator_capture_v1`. Amends `command-contract.md:37-60` and the load amendment's five-row
completeness rule (`load-command-contract.md:28-33`): the catalog then has eleven mandatory rows.
**NEW** (operator: E1).

RA-2 Authoring capture service. `context.capture_authoring({route:R, include:"workspace"|"spec"})`:
captures workspace.lua, its referenced text, the registry and, for `spec`, the spec and its
references; never jobs, so a listed job file that does not exist yet is not an error during authoring.
Same confinement, receipts and stability rules as render capture (`render-contract.md:105-134`).
**NEW** (engineering) within Q10 B.

RA-3 Prompt envelope. `{version:1,kind:"prompt",prompt:{kind,markdown,bytes,digest,inputs}}`, `kind`
one of `spec`, `critique`, `job`, `rejection`, `orchestrator`; `digest` is the fragment hash of the
Markdown bytes (`render-contract.md:423`); `inputs` lists the captured file entries (without bytes)
and the registry ID and revision. JSON on stdout like every other command; no raw-Markdown mode.
**NEW** (engineering).

RA-4 Prompt content. Templates are trusted Lua (`prompts.lua`) rendered with existing presentation
helpers (`src/stdlib/planar/presentation.lua:11`). The schema sections are generated from the schema
tables in `src/stdlib/planar/schema.lua:33-60`, never hand-copied, so prompt and validator cannot
drift. Check IDs are exactly the workspace allow-list intersected with the registry, each with its
`coverage`, `inputs` and `selection` (`declarations.md:343-359`); the shape catalog is the embedded
shapes (`pipeline`, `fanout_review`) with their exact job rules (`declarations.md:267-329`). The spec
prompt ends by instructing the harness to run `critique` (v5 `planar-design-spec-v5.md:583`). Critique
findings are computed mechanically (for example a prescription whose check coverage is `not_applicable`,
a consumer contract without a `start/contract/available` dependency, a facet dependency without an
`output_vector` check) and listed in sorted order. **C** for content requirements
(`remaining-steps-design.md:491-494`), **Q** (Q12 A) for embedded shapes, NEW (engineering) for the
generated schema text and the critique rules.

RA-5 Rejection prompt (E4). Under E4 A: `explain-rejection --diagnostic FILE` reads a load error JSON
(≤ 64 KiB, strict closed error envelope of `render-contract.md:615-644`), and emits a prompt that names
`field`, `file` and `line` when present, the code and message, the schema excerpt for that field path,
and the raw error embedded verbatim as a labelled data block. `load` bytes and exit codes do not change.
A malformed diagnostic file is `schema` exit 3. **NEW** (operator: E4).

RA-6 Observe (Q11 A). `observe --from DIR`: native opens DIR as a retained no-follow directory,
lists entries once (bounded, sorted by unsigned name bytes), refuses symlinks, special files, hard
links to each other and subdirectories other than the two named below with `path_denied`, reads
each regular file through no-follow relative opens with receipts, rechecks membership and bytes, then
passes the bytes to trusted Lua, which validates and assembles canonical observations v1
(`render-contract.md:182-221`) on stdout. Items stay driver assertions; observe never verifies a
target. Directory layout per E12. **Q** (Q11 A) for mechanism, NEW (operator: E12) for layout.

RA-7 Orchestrator projection (Q12 A). Reserve projection ID `orchestrator` with input
`orchestrator_context_v1` = `{workspace, spec, job, shape:{kind,max_rounds,stages}, roles:[{id,kind,
work|targets}], graph:{vertices, edges} (the declaration phase graph of declarations.md:230-249),
prescriptions:[driving prescriptions], actions:[{command,status,purpose}]}`. It never receives worker
facets, outputs or observations (v5 `planar-design-spec-v5.md:587`). `status` is `available` iff the
command is in the invocation's sealed catalog (native passes the sealed command names), else `future`;
the future list is the E1 table. Adds `orchestrator` to the shipped-owned projection IDs
(`external-profile-contract.md:270-273`) and its section IDs to the reserved union
(`:366-373`). **Q** (Q12 A), NEW (engineering) for the input shape and the catalog-derived labels.

RA-8 Init coexistence. `init` unchanged in bytes and behaviour; no scaffold, prompt or overwrite.
**Q** (Q10 B), **C** (`ledger-contract.md:442`).

RA-9 Profiles. Under E3 A, RA01 verbs are admitted under `with-profile` only when R15 widens the closed
factory; RA01 itself ships them embedded-only, and `with-profile spec-prompt` stays a usage error until
then. **Q** (Q12 A "embedded shapes only in RA01 prompts if RA01 needs to stay small"), NEW for the
deferral (operator: E3). Once admitted under `with-profile`, the prompts' shape catalog is the sealed
catalog's shapes, external ones included; this is the narrowing-risk row of
`remaining-steps-design.md:493-494` and must appear in that round's invariant table (P1).

### 3. Module placement

`capture/directory.*` (RA-6), the authoring capture variant in `capture/` and `acceptance/source_capture`
style (RA-2), runtime ABI rows, `handlers/{spec-prompt,critique,ingest,explain-rejection,observe,orchestrator}/`,
`prompts.lua`, `observe.lua`, `orchestrator.lua`, `commands.lua` registrations. No ledger module is
touched. **NEW** (engineering).

### 4. Test and acceptance strategy

- A35: two fresh processes, same declarations and registry, byte-equal prompt envelopes for each prompt
  kind; reordered Lua table construction in the fixtures (A01 discipline).
- A36: a rejected load in a scratch ledger; its stderr fed to `explain-rejection`; the prompt contains
  the exact field path, and the embedded raw error is byte-equal to load's stderr; load's exit stays 3.
- Schema drift guard: mutate one field in `schema.lua`'s table (probe) and the prompt changes.
- Observe: symlink entry, FIFO, hard-link alias, oversize file, too many entries, duplicate IDs across
  files, unknown fields, a file replaced between list and read (`capture_changed`), empty directory
  giving canonical empty observations, deterministic order independent of creation order.
- Orchestrator: every `available` label is in the sealed catalog and every future verb is labelled
  `future`; probe by registering a test-bundle stub verb and seeing the label flip.
- Authoring capture with a listed job file absent succeeds for all three prompts; `ingest` with an
  invalid spec fails with the same typed error load would give.
- Hard to observe: the RA01 capture races are covered by `RA01-B2 capture_diagnostic refuses a file changed before verification` (`src/engine/capture/capture.t.cpp`, authoring diagnostics) and `RA01-B2 observe refuses a directory changed before the recheck` (`src/engine/capture/directory.t.cpp`, observe); the receipt-reuse claim is limited to what the render and load capture paths exercise, because the RA01 paths do not reach the admission receipt code (RA01-C-F2).

### 5. Split, waves and size

| Round | Content | Size |
| --- | --- | --- |
| RA01-A | Docs-only: `authoring-contract.md` and pointer amendments (RA-1 to RA-9), reviewed records-only | 1 run |
| RA01-B wave 1 | Catalog and ABI rows for all six verbs with stubs (owner: runtime, `commands.lua`, bootstrap, response, root help) | small |
| RA01-B wave 2 | In parallel, disjoint files: (i) prompts and explain-rejection with A35/A36; (ii) observe native service plus Lua; (iii) orchestrator projection | medium |
| RA01-C | Integrated gate (XD6) | small |

Expect 4 to 6 runs in total. Waves 2(i) to 2(iii) only share `commands.lua`, which wave 1 freezes.

### 6. Risks and dependencies

Depends on X08 approval (requirement E14 of the external-profile gate, not operator question E14 here;
`remaining-steps-design.md:369-370`) and operator questions E1, E4, E12 and E14. Risks: the root help and catalog-completeness changes break existing tests (handle under XD5);
generated schema text must be deterministic under Lua table iteration (use sorted helpers); observe
is the first directory listing capability and Linux directory order differs (sort, and gate on Linux
per XD6); the orchestrator's future list must be updated by every later round (each round's brief
lists it).

## R15: accepted reads and run ownership

### 1. Scope and rows

Accepted-revision render, basic pending-draft status, active job, generations, retirement, claim with
packet atomicity, fencing and explicit takeover; role activation and reconciliation committed with
revision acceptance before runs are enabled; minimal A34 provenance designed before runs; live
orchestrator guidance and explainable state reads (`roadmap.md:365-375`). **C**. Rows: A09, A10, A20,
A22, A23 dispatch, A26 reconciliation, A33, A34 minimal design (`acceptance-matrix.md:32-56`);
preserve A02, A31, A32 (`:24`, `:53-54`).

### 2. Contract changes

Destination per E14: a new exact `execution-contract.md` holding EX-1 to EX-8 and the R15 clauses,
with scoped amendments to `ledger-contract.md` (format evolution pointer, events vocabulary, command
surface), `load-command-contract.md` (format3 load), `command-contract.md` (ABI rows),
`render-contract.md` (dispatch projection, `work_exec_v1`), and `external-profile-contract.md`
(X-STORE format table row for format3; X-CLI family list per E3; reserved section IDs).

R15-1 Format3 and request_v3 as EX-1 and EX-2; init cutover to format3 (**Q** Q13 A).

R15-2 Active job (E2 A). `activate` sets the single active job of the ledger. Parameters
`{job, expect_revision, expect_job}`. Preconditions: active revision equals `--expect-revision`, active
job equals `--expect-job`, and the job exists in the active revision. Effects in one EX-3 commit: a
`job_activations` row, `current_state.active_job`, a generation for every role of the job (reason
`activated`; on re-activation of a previously active job, new generations, never reused), runs of the
previous job fenced and its generations retired, and the reconciled state vector. A ledger with no
active job refuses every run verb with `no_active_job`. **NEW** (operator: E2). Basis: v5 speaks of
"an active job" (`planar-design-spec-v5.md:125`) and puts separate arms in separate ledgers (`:489`);
the R11 foundation's load selects no job (`ledger-contract.md:34-35`, which describes the foundation
and defers active-job semantics to later contracts, `:579-581`). "One active job per ledger" is
therefore this document's choice, not a v5 statement: v5 lists "active jobs" among workspace
declarations (`:110`) and makes switching jobs "one revision change" (`:497`), which is why E2 is an
operator question.

R15-3 Format3 load. On format3 the load handler, after interpretation and outside any lock, calls a
new read service `read_execution({})` and then `accept_load({bundle, semantic, execution: CHANGE})`,
where CHANGE is the reconcile result for the active job under the candidate revision: structure
digests (new generation for changed structure, retirement for removed roles, `recreated` for re-added
names), fenced runs, drift flags on running roles, and the full next-state vector. The locked
transaction checks the expected head (existing CAS), then the EX-3 preconditions, then commits
revision, event, transitions, generations and request together (A26). If the active job is absent
from the new revision, its generations retire and the active job becomes none. A noop load (exact
equality) writes no transitions. Active runs keep their acknowledged revision until refresh
(v5 `planar-design-spec-v5.md:497`). **C** for the atomicity
(`ledger-contract.md:35-37`; `roadmap.md:370`; A26), NEW (engineering) for the service shape.
This amends the load services table (`load-command-contract.md:109-163`) for format3 only.

R15-4 Claim. Parameters `{job, role, provenance_path}`. The role must be `ready` or `stale`;
otherwise `claim_conflict` with `{state}`. Lua renders the dispatch packet (R15-6) from the snapshot.
One EX-3 commit creates the run, the dispatch with retained packet, manifest and W records, the
`running` transition and the request. Response `claim_v1`: `{version:1,kind:"claim",ledger,request_id,
job,role,revision,run:{id,generation,token},dispatch:{id,work_fp,render_digest,bytes},packet,manifest}`.
Retry with the same ID returns the same bytes, including the token, even after later events.
**C** (v5 `planar-design-spec-v5.md:368-370`), NEW for the response fields (engineering).

R15-5 Takeover. Parameters `{job, role, run_id}`. Requires `--run-id` to equal the current active run
of the role ("requires the current run identity", v5 `planar-design-spec-v5.md:370`); the secret is not
required, so a driver that lost the token can still take over. One commit: old run `fenced`, a new
generation (reason `takeover`), a new run and dispatch. The old token is fenced from then on; the old
process is not killed, and the driver must stop it before reusing the writable view (**C**, same
citation). **NEW** (engineering) for the no-secret rule; it depends on E11.

R15-6 Dispatch projection. A new packet mode `dispatch`: `P.mode:"dispatch"`; the fixed authority
statement names run, generation and revision and states that checks have not run in this dispatch;
upstream facet content comes from publications; prerequisites use ledger qualities; the mandatory
`run` section lists the legal next actions from the E1 table. W is `work_exec_v1` (EX-5). The manifest
is `dispatch_manifest_v1`: `{version:1,revision,job,role,generation,run,versions,work:{digest},
render:{digest,bytes,budget_bytes},sections,omitted,prerequisites,registry,inputs:{source,
interpretation,publications:[{work,facet,publication,semantic}],repair}}`, with record IDs instead
of `bytes_hex` file entries. Adds `run` to the reserved section union (`external-profile-contract.md:366-373`).
The worker and reviewer projection IDs stay shipped-owned. **NEW** (engineering) within the render
contract's "later execution" clause (`render-contract.md:396-401`).

R15-7 Status and preview. `status` returns `{version:1,kind:"status",ledger,active_revision,
active_job,last_event,roles:[{role,kind,generation,state,drifted,run:{id,generation}|"none"}],
drafts?,role_detail?,request?}`. `--role` adds `role_detail` with the acknowledged and current W digests,
the changed top-level W fields and the transition reasons (EX-4: the "explainable state read").
`--workspace`/`--registry` captures the working declarations and reports `drafts:{status:"unchanged"|
"changed"|"invalid",changed:[paths],error?}` without accepting anything (A20; v5
`planar-design-spec-v5.md:497`). `--request-id` returns EX-2's saved result and disposition. `preview`
renders the dispatch projection for a role from the active revision without claiming or retaining it.
**C** for the behaviours, NEW (operator: E1) for placement.

R15-8 Provenance (A34 minimal, E6 A). Planar records natively per run: ledger identity, revision,
source digest, job, role, generation, base label, W and render digests, requested preferences from the
job (`declarations.md:284-286`). The driver may attach `driver_provenance_v1` at claim, takeover,
release and check: `{version:1,driver:{name,version},trial:{id},view:{id},model:{requested,actual},
session:{requested,actual},settings:[{name,value}]}` where every leaf is a string or exactly
`{"status":"unknown"}`; absence of the file records every field unknown. Stored as a value record,
labelled `driver_assertion_unverified`. Outcomes are the events themselves. No metrics API.
**C** for the requirement (`v5-alignment.md:141-146`; `roadmap.md:372-374`), NEW (operator: E6).

R15-9 Tokens (E11 A). Token text is `<run id>.<generation ordinal>.<32 lowercase hex secret>`; the
secret is 16 OS-random bytes stored in `runs`; `status` (other than request lookup), `preview`,
history pages, `--at-event`, `--event` and packets never show it; claim and takeover responses and
request lookup of a claim or takeover request do, because the saved result carries the token and
replay is byte-identical. Request lookup includes the existing `history --request-id`
(`ledger-contract.md:446`) unless E1 excludes execution results from it. Anyone who can read the
ledger and learn a request ID can therefore recover the token: the secret guards against accidental
use, not against a reader of the ledger. Token-bearing verbs refuse a wrong secret
or non-current run with `fenced`. **NEW** (operator: E11).

R15-10 History. Format3 event pages include payload v2 and typed reference columns; `--at-event`
returns reconstructed role states; new selector `--event <seq>` returns the event with its referenced
execution records expanded (packet Markdown, manifest, transitions), bounded by 16 MiB, decoded without
Lua. **C** for A29 (`acceptance-matrix.md:51`), NEW for the selector (engineering).

R15-11 Orchestrator live form. `orchestrator --ledger` adds live state (role states, runs without
secrets, blockers) to `orchestrator_context_v1` and flips `activate`, `status`, `preview`, `claim`,
`takeover` to `available`. **C** (`roadmap.md:362-363`, `:375`).

### 3. Module placement

`history/request_v3`, `history/execution_codec`, `ledger/execution_schema`, `ledger/execution`,
`ledger/acceptance*` (format3 load), `history/read` (format3 reads), new `engine/execution/`, runtime
ABI rows and factory admission rows, handlers for `activate`, `status`, `preview`, `claim`, `takeover`,
stdlib `reconcile.lua` and `dispatch.lua`. **NEW** (engineering).

### 4. Test and acceptance strategy

- A09: two real processes claim one role with a pause variant forcing both past the snapshot; exactly
  one `claim` response, the other `claim_conflict`; lost response via the after-commit-before-stdout
  variant, then `status --request-id` returns the token; a retry returns identical bytes; same ID with a
  changed role conflicts.
- A10: takeover, then the old token on `refresh` or `release` (R15 has no completion yet; R17 repeats
  with `check`) refuses `fenced` with no effect.
- A20: an invalid draft in the workspace; `status` reports it invalid, the active revision and running
  roles are unchanged; a failed load activates nothing.
- A22 and A33: regroup the quota job (combined to split) by load and by `activate`; old generations
  retire, old tokens fenced, a re-added role name gets a new generation and no inherited token.
- A26: load during an active run with a structural change; a crash variant before COMMIT leaves
  revision, transitions and fencing all absent; after COMMIT all present; a concurrent claim between
  snapshot and commit makes load refuse `state_conflict` and a same-ID retry succeed.
- A23 dispatch: the dispatched packet is reconstructed from SQLite after restart byte-equal to the
  claim response; a disk-full fault during claim leaves no run.
- A02/A31/A32 preserved: `work_exec_v1` moves only for applicable inputs (scoped mutation cases run
  through `status --role`).
- Lost-update guard (EX-3 step 6): two claims of **different** roles forced to overlap with the pause
  variant both succeed, and both roles end `running` in `role_state` and in the replayed transitions;
  the permissive mutant "write Lua's complete vector" must be killed by this test.
- Interpretation binding (EX-6): a generated variant binary with different embedded Lua refuses
  `claim` with `interpretation_mismatch` and no effect, while a same-ID retry of a request committed by
  the original binary still replays its saved bytes (ordering row per P2).
- Hard to observe: concurrent claims and load-versus-claim overlap need the pause variant; the
  precondition check needs the permissive mutant "fact_version check always passes" killed by the
  load-versus-claim test.

### 5. Split, waves and size

R15 is the largest step. Split before coding (`roadmap.md:55-56`).

| Round | Content | Size |
| --- | --- | --- |
| R15-A1 | Docs: EX-1 to EX-8, format3 SQL, state machine, reconcile rules (execution contract part 1) | 1 to 2 runs |
| R15-A2 | Docs: verbs, services, dispatch projection, `work_exec_v1`, provenance, history selector (part 2) | 1 run |
| R15-B | Native format3 storage, request_v3, execution codecs, snapshot read, EX-3 commit with preconditions, cross-row verification; no CLI | large |
| R15-C | Lua reconcile and dispatch projection, format3 load reconciliation (ledger/acceptance), activate, init cutover | large |
| R15-D | CLI verbs status, preview, claim, takeover, history selector, orchestrator live form | medium |
| R15-E | Integrated gate: races, faults, restart, A09/A10/A20/A22/A23/A26/A33 | medium |

Parallelism: in R15-B, request_v3 and execution codecs (`history/`) and the format3 schema
(`ledger/execution_schema`) are disjoint files and can be separate packages; the commit path depends on
both. In R15-C, `reconcile.lua` and `dispatch.lua` are disjoint from the native load change. Expect 8
to 12 runs overall.

### 6. Risks and dependencies

Depends on RA01 and on E1, E2, E3, E5, E6, E11, E14, E16. Risks: format3 load touches
`ledger/acceptance*` and the X06-C seam (escalation; keep the seam's threat model XD1); spurious
`state_conflict` under heavy parallel driving (record as a limit; drivers retry same ID); the init
cutover changes existing init and load test expectations (XD5); A26 is the B1-style narrowing risk: the
reconcile must cover **every** role of the active job, not only the loaded one (P1); budget threading
into snapshot conversion and reconcile (Q2 A, clause-30 pattern).

## R16: refresh and recovery

### 1. Scope and rows

Same-run acknowledgment, `--since`, drift, checkpoints, interrupted and released runs, retained
historical recovery, keeping active and acknowledged revisions and the exact dispatched context and
evidence distinct (`roadmap.md:377-383`). **C**. Rows: A11 (drift and refresh; old-check refusal is
R17), A19, A29, recovery parts of A12, A20, A23 (`acceptance-matrix.md:33-51`); preserve A03.

### 2. Contract changes

Amend `execution-contract.md` (and the render contract for the recovery projection).

R16-1 Refresh. Parameters `{run, expect_fp, observations_path}`. Requires a current token, the run's
acknowledged W digest equal to `--expect-fp` (else `drifted`), valid start prerequisites (else
`claim_conflict` with `{state:"blocked"}`), and the same generation (a structural change has already
fenced the run, R15-3). Effects: a new dispatch (kind `refresh`) with a fresh packet under the current
W, drift cleared, the same run still active, an optional checkpoint recorded. Earlier evidence never
covers the new W. If inputs move between snapshot and commit, `state_conflict`. **C** (v5
`planar-design-spec-v5.md:377`, `:387`), NEW for parameters (engineering).

R16-2 Since. `preview --role R --since DIGEST` compares the would-be packet with the retained dispatch
of this role whose render digest equals DIGEST (latest one if several), reporting
`{since:{dispatch,work_fp,render_digest},current:{work_fp,render_digest},material:[changed top-level
W fields],render_changed,sections:[{id,before_bytes,after_bytes}]}`. No acknowledgment. A digest never
dispatched for this role refuses `history_unavailable`. **C** (v5 `planar-design-spec-v5.md:276`,
`:387`, `:539`), NEW for the output shape (engineering).

R16-3 Checkpoint. Records a checkpoint (observations whose `checks` and `findings` are empty; any target
assertion refuses `observation_target_mismatch`, since execution packets have no capture). A checkpoint
changes the next packet's optional `checkpoint` section and never W or state (A03). **C** (v5
`planar-design-spec-v5.md:43`, `:270`), NEW for the empty-assertions rule (engineering).

R16-4 Release. `--reason released|interrupted`: closes the active run (`released` or `interrupted`),
optional final checkpoint and provenance, and derives the role's state from retained evidence (ready,
stale, blocked or done). An open check reservation on the run is not closed by release; release
refuses `check_in_progress` until `check-recover` (R17). **C** (v5 `planar-design-spec-v5.md:384`),
NEW for the reservation rule (engineering).

R16-5 Recovery. `recover --role R` returns the recovery packet of v5 `planar-design-spec-v5.md:262-268`:
the retained acknowledged fragment and manifest byte-exact, the run identity or "no run is active",
the latest evidence per unit labelled with its target identity (R17 fills it), current findings (R18
fills it), the latest checkpoint, and with `--root`/`--base-root` the re-observed unit outputs of the
checkout, labelled as observed now. It is rendered as a recovery fragment by the shipped `recovery`
projection with a new input `recovery_context_v1`; nothing is retained by the read. **C** for content,
NEW for the input shape (engineering).

R16-6 Recovery cache (E10 A). Planar writes no `state/recovery/` files; `recover` always reads SQLite,
and the A23 "rebuild missing caches" portion is satisfied by construction and recorded so. **NEW**
(operator: E10). Basis: v5 `planar-design-spec-v5.md:276` ("may cache ... not the history authority").

R16-7 Orchestrator and provenance. `orchestrator --ledger` flips `refresh`, `checkpoint`, `release`
and `recover` (and `preview --since`) to `available` and adds refresh and recovery guidance; release
and interruption events carry R15-8 provenance. **C** (`roadmap.md:382-383`).

### 3. Module placement

`ledger/execution` (new operations), `engine/execution/` (refresh, release, checkpoint), stdlib
`recovery.lua` and `dispatch.lua` changes, handlers `refresh`, `checkpoint`, `release`, `recover`,
`preview` (`--since`). **NEW** (engineering).

### 4. Test and acceptance strategy

- A11: a contract edit loaded while a worker runs; `status` shows drifted; `preview --since` shows the
  contract field as material; `refresh` with the old fp succeeds and clears drift; a second refresh with
  the stale fp refuses `drifted`; a load landing between the refresh's snapshot and commit refuses
  `state_conflict` (pause variant).
- A19: kill the driver mid-run; `recover` returns the retained packet byte-equal, the latest checkpoint,
  the re-observed checkout identity and "run active" with the run ID; after `release --reason
  interrupted`, "no run is active".
- A03: a checkpoint-only change moves the next packet's render digest, never W or state.
- A29 and A23: after restart, `history --event` for an early claim reproduces its packet; a missing
  packet record (corruption fixture) is `ledger_corrupt`, never a silent regeneration.
- A12 recovery part: release a role whose evidence is still valid (seeded through native fixtures until
  R17) and see `done` derived.

### 5. Split, waves and size

| Round | Content | Size |
| --- | --- | --- |
| R16-A | Docs: R16-1 to R16-7 | 1 run |
| R16-B | Native and Lua operations plus CLI (waves: native operations; then handlers and projection in parallel) | medium |
| R16-C | Integrated gate | small |

Expect 3 to 5 runs. R17-B (the process runner) can be built in parallel with R16-B because it touches
only the new `src/engine/process/` directory (see the overview).

### 6. Risks and dependencies

Depends on R15 and E10. Risks: retained packets grow the ledger (no pruning, by v5; record growth per
claim in evidence); the `--since` diff must not become a second acknowledgment path; the checkpoint is
untrusted text in packets and must be delimited as data (v5 `planar-design-spec-v5.md:272`).

## R17: checks, boundaries and observed completion

### 1. Scope and rows

Bounded trusted check adapters, reservations, exact input and output observations, before and after
recheck, boundary attribution, evidence applicability, published facets and prerequisite rederivation;
missing tools, zero tests, timeouts and malformed results fail; driver stability and race limits stay
explicit; semantic dependency changes, output-vector changes and blocker availability have distinct
effects; valid historical evidence can restore done (`roadmap.md:385-394`). **C**. Rows: A05, A12, A15,
A16, A17, A18, completion parts of A23 and A28, check-history A29 (`acceptance-matrix.md:27-51`).

### 2. Contract changes

Amend `execution-contract.md`. Amend `declarations.md` for the registry v2 of E7 B, and under E7 A or
C as well for the environment rule: registry v1 says environment policy is "replacement with these
exact values" (`declarations.md:338-339`), so adding Planar's reserved variables is a scoped amendment
of that sentence. Amend `command-contract.md:190-194` ("each service callable at most once ... No
retries inside this invocation") for the bounded completion retry of R17-1 step 3.

R17-1 Check protocol (v5 `planar-design-spec-v5.md:349-356`, **C**), in three commits:

1. **Reserve.** EX-3 commit with preconditions: token current, role `running`, not drifted (else
   `drifted`), no open reservation on the run (else `check_in_progress`), request ID unused (no request
   row and no reservation with that ID; an open reservation with an unequal body is
   `request_conflict`). Writes the reservation and `check_reserved`. The request row is written only at
   the end, referencing the `check_reserved` event (EX-1 exception), so a retry while the reservation
   is open refuses `check_in_progress` ("an in-progress operation is not executed twice", v5
   `planar-design-spec-v5.md:517`).
2. **Run, outside any lock.** Capture the root and base (E15), compute unit outputs; for every check
   with `output_vector` input, every producer unit in the vector must equal the producer's current
   applicable completion output, else `observation_target_mismatch` with `{work}` and no checks run;
   materialize accepted contracts (E7); run the role's complete required check set in sorted
   `(work, check)` order through the process runner; then recapture and compare membership, modes and
   content (exact bytes under E15 B; size plus per-file digest under E15 A, a recorded limit since the
   digest is non-cryptographic, v5 `planar-design-spec-v5.md:252`).
3. **Complete.** Fresh snapshot, Lua evaluates evidence and completion, then an EX-3 commit that
   records the evidence and closes the reservation, and additionally records completion, publications
   and the `done` transition only if every required check passed, the boundary gate passed, the
   recapture matched, the run is still current and not drifted, the role has no open start blocker,
   and every complete-phase dependency quality of its units holds (`done` producers have an applicable
   completion, `accepted` producers are accepted, R18-6) (v5 `planar-design-spec-v5.md:351`, `:355`,
   `:285-286`, **C**). On `state_conflict` the check re-snapshots and re-evaluates up to three times
   (the evidence is immutable data); after that it commits evidence only. The request row carries the
   final result. **Open (engineering, R17-A must settle):** if the run has been fenced by then, "always
   record the evidence" would be a ledger write by a superseded token, which v5 forbids ("fences all
   old mutations", `planar-design-spec-v5.md:59-60`, `:370`). R17-A chooses either (a) refuse `fenced`,
   write nothing, and leave the reservation open for `check-recover`, or (b) a native-only closure of
   the reservation as `interrupted` with no evidence attached to the fenced run, recorded as an
   explicit amendment.

R17-2 Check execution (Q15 A plus E7). The command is the toolchain `command` plus the check `argv`
from the **accepted** registry records (`declarations.md:336-352`; v5 `planar-design-spec-v5.md:325`).
cwd is the bound root directory descriptor. The environment is exactly the toolchain `environment`
plus Planar's reserved variables (E7); a registry name colliding with a reserved name refuses
`reference`. Executable resolution uses that environment's `PATH` or an absolute path. The child runs
in its own process group; on `timeout_ms` the group gets SIGTERM, then SIGKILL after a 2 s grace; output
(stdout and stderr merged in order) is capped at `max_output_bytes`, overflow is a failure with the
first bytes retained. Every call is recorded as evidence with definition, environment names, root ID,
vector, outcome, reason, matched count, exit status, duration and output. Escaping descendants
(`setsid`) are not tracked: recorded limit (Q15 A). **Q** (Q15 A), NEW (operator: E7) for the result
and contract-delivery protocol.

R17-3 Outcomes and reasons (closed): `pass`; `fail` with reason `exit_status`, `zero_selected`,
`missing_executable`, `timeout`, `output_limit`, `malformed_result`, `boundary`; `inconclusive` with
reason `input_changed`. Zero selected tests, missing tools, timeouts and malformed results are
failures (**C** `declarations.md:347-349`; v5 `planar-design-spec-v5.md:323`). The CLI exit for a
recorded non-pass outcome follows E8.

R17-4 Boundary gate. A built-in, always-required gate `boundary_v1` recorded as evidence: compare the
current and base trees after `root.excluded` (the declared generated-file exclusion); every added,
deleted, renamed or mode-changed path must lie inside the role's owned boundaries, or inside a vector
producer's boundaries with that producer's output equal to its completion output (merged producer
work); frozen paths must be unchanged; symlinks, special files and aliases already refuse at capture.
**C** (v5 `planar-design-spec-v5.md:345-347`; A18), NEW for the gate's form (engineering).

R17-5 Completion and publication. One commit: completion row (acknowledged dispatch, W, unit output
vector), one publication per facet the role's units produce (facet semantic identity of
`render-contract.md:327-331` computed from the post-check observation, exact facet bytes retained),
the run `completed`, the `done` transition, and the downstream transitions Lua computed (consumers
whose W moved become drifted or stale; consumers whose integration vector no longer matches become
stale through `evidence_inapplicable`, not `work_changed`). Facets come only from the verified
observation. **C** (v5 `planar-design-spec-v5.md:355`, `:360`, `:389`; A16).

R17-6 Applicability and restoration. Evidence applies when its W equals current W and its output vector
equals the current outputs it targets (own output as last observed, producer outputs as currently
published). A cleared blocker with still-applicable evidence restores `done` with no rerun (A12). A
failed check changes no state (v5 `planar-design-spec-v5.md:395`). **C**.

R17-7 Reservation recovery. `check-recover --run-id --reservation --confirm-stopped` closes an open
reservation as `interrupted` (event `check_interrupted`, whose request is the check-recover request)
and writes the terminal request row for the reserved check request ID with an interruption result,
referencing its `check_reserved` event (EX-1 exception; `requests.event_seq` stays UNIQUE because the
two request rows reference different events), so retrying that ID replays the interruption; a fresh
check uses a new ID. Interrupted checks never leave roles done. **C** (v5 `planar-design-spec-v5.md:517`),
NEW for the verb (operator: E1).

R17-8 Orchestrator. `orchestrator --ledger` flips `check` and `check-recover` to `available` and adds
check and completion guidance (always `check` before treating a role as done, v5
`planar-design-spec-v5.md:587`); check events carry R15-8 provenance. **C** (`roadmap.md:394`).

### 3. Module placement

New `src/engine/process/` (runner; macOS and Linux branches; no Lua, no ledger), `engine/execution/check.*`
(orchestration), `ledger/execution` (reserve, evidence, completion), capture extension for E15,
stdlib `checks.lua` (applicability, boundary policy, downstream reconcile), handlers `check` and
`check-recover`. **NEW** (engineering).

### 4. Test and acceptance strategy

- A17: fixture checks for exit 1, `matched:0`, a missing executable, a sleep past `timeout_ms`, a
  malformed result file, an output flood; each records `fail` with its reason and the role stays
  running.
- A15: a fixture check that edits a source file in the root while running gives `inconclusive`
  (`input_changed`) and no completion; a change-and-revert fixture is shown **not** detected and
  recorded as the documented limit (XD2 style).
- A05: worker edits after completion; the next `check` records a new output identity, old evidence
  becomes inapplicable, the role becomes stale; the role's W is unchanged.
- A16: producer re-completes with the same interface but different output; consumer W unchanged,
  consumer `stale` by `evidence_inapplicable`; consumer `check` with the old producer output in its
  view refuses `observation_target_mismatch`.
- A18: writes outside ownership, into frozen paths, renames, mode flips, symlink, untracked and ignored
  files (included unless excluded) each fail the boundary gate or capture.
- A12: a question blocks start after completion; answering it by load restores `done` without a rerun.
- A10 again: a fenced old run's check refuses at reserve, and a takeover landing during the run makes the
  completion commit follow R17-A's choice for the fenced case (R17-1 step 3), never a completion and
  never evidence attached to the fenced run without an explicit amendment.
- A28 completion part: an mtime-only touch, an optional-byte change outside every unit boundary, and a
  rename inside one unit's boundary; only the last moves that unit's output identity and only that
  unit's evidence becomes inapplicable (selective staleness).
- Completion gating: a `complete/facet/done` producer that is not done, or an open start blocker,
  makes a passing check record evidence without completion.
- Crashes: SIGKILL `planar` while the child runs; the reservation stays open; a retry refuses
  `check_in_progress`; `check-recover` then a fresh check completes. Fault variants after reservation
  commit and after child exit.
- Linux: the runner's process-group, signal and `posix_spawn` paths run in the Docker gate (Q5 A, Q6 A).

### 5. Split, waves and size

| Round | Content | Size |
| --- | --- | --- |
| R17-A | Docs: R17-1 to R17-8, E7, E8, E15 outcomes, the fenced-completion choice | 1 run |
| R17-B | Native process runner with its own tests (can start during R16, disjoint directory) | medium |
| R17-C | Check orchestration, evidence, completion, publications, Lua applicability and boundary gate | large |
| R17-D | CLI `check`, `check-recover`, orchestrator labels, integrated gate with Linux | medium |

Expect 5 to 8 runs. R17-B must not start until R17-A pins the runner contract (timeouts, signals,
environment) unless the operator allows a spike labelled as such.

### 6. Risks and dependencies

Depends on R16 and E7, E8, E15. Risks: first execution of untrusted code (Q15 A limit must be stated
in every reference doc); platform differences in process groups and signal delivery; time-based
flakiness (use injected small timeouts; never join #531); check output storage growth; large real
checkouts against capture ceilings (E15); the completion commit retry loop is new behaviour and must be
bounded and tested with the permissive mutant "retry forever".

## R18: review and repair

### 1. Scope and rows

Exact-target independent review, atomic verdict, repair and retry, cumulative `max_rounds`, durable
blockers; A24 conditional; no authenticated-human guarantee; orchestrator review and repair guidance
(`roadmap.md:396-403`). **C**. Rows: A06, A13, A14, A16 approval part, approval parts of A22 and A33,
review-history A29, A24 (`acceptance-matrix.md:28-46`). `review_loop` stays after R18
(v5 `planar-design-spec-v5.md:207`).

### 2. Contract changes

Amend `execution-contract.md` and `render-contract.md` (reviewer dispatch content).

R18-1 Reviewer dispatch. The reviewer is claimable when every worker role is `done` (the phase graph,
`declarations.md:236-240`, **C**). Its packet lists the exact target vector
`[{work,output,work_fp}]` from current completions, check summaries and unresolved findings; the driver
supplies the outputs themselves (v5 `planar-design-spec-v5.md:408`, **C**). The policy identity is the
reviewer's `work_exec_v1` digest. **NEW** (operator: E9).

R18-2 Verdict format (E9 A). JSON `verdict_v1`, strict Glaze: `{version:1,targets:[{work,output,work_fp}],
policy,decision:"approve"|"reject",findings:[{id,severity:"must"|"should"|"note",work,path?,line?,
prescription?,text}],repair:[worker role IDs],summary,authority?}`; `authority` is optional and exists
only so that R18-7's refusal surface is reachable (without it, strict parsing would refuse the field
as `schema` before any policy check). The verdict's event ID is the request ID.
Schema-invalid verdicts refuse `schema` with no ledger change ("treated as absent and flagged").
Approve with any `must` finding, or a repair target outside the vector's owners, is `schema` or
`reference`. **NEW** (operator: E9); rules **C** (v5 `planar-design-spec-v5.md:412`, `:432`).

R18-3 Applicability. Targets and policy equal to the current vector and policy: `current`. Otherwise
the verdict is recorded `historical` with no repair, approval or count effect (v5
`planar-design-spec-v5.md:432`, **C**).

R18-4 Admitted rejection, one commit (A13): verdict row, each named worker's repair revision +1 in its
current generation (W moves through `repair`), findings carried forward to those workers' packets,
review cycle count +1, reviewer run completed, transitions. A retry with the same ID returns the saved
result without another increment. **C**.

R18-5 Cap (A14). Admitted rejections are counted per job activation (the "active job and shape
generation") and are not reset by loads or refreshes. While the count is below `max_rounds`, a
rejection is admitted. A further rejection is recorded, admits no repair, leaves the work unaccepted
and opens a durable `max_rounds` blocker; the blocker clears only when a later active revision raises
`max_rounds` above the count. A rejection with an empty `repair` list opens an `unowned_rejection`
blocker. **C** (v5 `planar-design-spec-v5.md:412`, `:434`), NEW for the clearing rule (engineering); the
counting key ("per job activation", including whether a regrouping load resets it) is **NEW**
(operator: E17), because v5 says only "the active job and shape generation".

R18-6 Accepted (derived). A unit is accepted when its worker is `done` with applicable evidence, the
current output vector has a `current` approval under the current policy, and no acceptance blocker is
open. `status` reports `accepted` per unit and why not. `accepted`-quality dependencies use it. An
approval never wakes anyone; a changed output, input, policy or vector makes the old approval historical.
A role recreated under an old name has no approval (A22, A33). **C** (v5 `planar-design-spec-v5.md:406`,
`:436`).

R18-7 A24 (Q16 A). The verdict's authority is always the reviewer role. There is no external approval
assertion API; `status` reports `external_approval:"not_configured"`; the R18-A contract fixes one
refusal surface for an external assertion attempt (the verdict field `authority` with any value other
than `reviewer_role_v1`), which refuses `unsupported_policy`. Disabled policy is never labelled as
tested approval (`acceptance-matrix.md:46`). **Q** (Q16 A), NEW for the surface (engineering).

R18-8 Orchestrator. `orchestrator --ledger` flips `verdict` to `available` and adds review and repair
guidance (findings carried to repaired workers, cap and blocker status); verdict events carry R15-8
provenance. **C** (`roadmap.md:402-403`).

### 3. Module placement

`ledger/execution` (verdict, repair, blockers), `engine/execution/` (verdict service), stdlib
`review.lua` and changes to `reconcile.lua` and `dispatch.lua`, handler `verdict`. **NEW** (engineering).

### 4. Test and acceptance strategy

- A06: a verdict naming a stale output or old work fp is recorded historical and approves nothing.
- A13: fault variants before COMMIT (no verdict, no increment) and after COMMIT before stdout (both
  present; the same-ID retry returns the saved result and the repair revision stays +1).
- A14: with `max_rounds = 2`, three rejections: the second still permits repair and review, the third
  opens the blocker; a load that only edits a contract does not reset the count; raising `max_rounds`
  clears it.
- A16 approval: the producer's output changes after approval; acceptance is withdrawn while the consumer
  stays fresh.
- A22, A33: approvals of a retired generation never attach to the recreated role.
- Takeover lineage (EX-5): after an admitted rejection, a takeover of the repaired worker keeps its
  repair revision, W digest and carried findings; the pre-rejection evidence stays inapplicable.
  Permissive mutant: "takeover generation starts empty".
- A24: the external assertion attempt refuses `unsupported_policy`; R19 records the applicability.
- A29: `history --event` reproduces each verdict and its repair effects after restart.

### 5. Split, waves and size

| Round | Content | Size |
| --- | --- | --- |
| R18-A | Docs: R18-1 to R18-8, E17 | 1 run |
| R18-B | Native verdict commit and blockers in parallel with Lua review and acceptance derivation (disjoint files), then the CLI | medium |
| R18-C | Integrated gate | small |

Expect 3 to 5 runs.

### 6. Risks and dependencies

Depends on R17 and E9, E17. Risks: verdict JSON is untrusted session output (strict schema, data
delimitation in packets); counting by job activation must survive restarts and regrouping; accepted
derivation must keep the three causes of EX-4 distinct.

## R19: single-root external-driver acceptance

### 1. Scope and rows

A real single-root job in both groupings, driven by the reference orchestrator against live commands
and real checks, inspected through restarts and faults without manual database edits or re-execution;
closes every applicable Render and Execution matrix portion, conditional A24 and minimal A34; records
A24 applicability; A21 and controlled A34 stay deferred; a 24-hour run cannot replace the gate; no
scheduler, model calls or checkout lifecycle in Planar (`roadmap.md:405-415`; v5
`planar-design-spec-v5.md:625-627`). **C**. Subjects: the quota fixture in both groupings plus one real
spec the operator names before R19 (**Q** Q17 A).

### 2. Contract changes

None expected. R19 is verification and evidence. A product defect found in R19 goes to a fresh
corrective round of the owning step. **C** (round protocol, `roadmap.md:52-56`).

### 3. Module placement

Driver harness and fixtures outside `src/` (`tests/native/r19-support/`, `tests/acceptance/`), as
CLAUDE.md requires for test-only harnesses. The real spec's workspace and roots live in scratch
directories created by the harness, never under `~/.planar` and never against operator data.
**C** (CLAUDE.md database safety and conventions).

### 4. Test and acceptance strategy

- The driver follows E13. The scripted driver reads the orchestrator fragment's legal actions and
  performs: activate, status, claim, (scripted session edits), check, release or refresh, reviewer claim,
  verdict submission, repair, approval, and regrouping; every action's response is retained raw.
- Fault scenarios, each through a generated variant or SIGKILL (Q3 A): claim response loss; takeover
  while the old session still checks; check crash with reservation recovery; crash around verdict
  commit; a contract change mid-run with refresh; disk-full during completion; restart between every
  phase.
- Trajectory inspection: for each early event, `history --event` and `--at-event` reproduce accepted
  declarations, dispatched context, transitions and evidence references after restart, without Lua
  re-execution (A29).
- A matrix evidence table maps every row A01 to A36 to test names and raw logs, marking A21 and
  controlled A34 deferred and A24 "policy disabled, refusal tested".
- Full gate per XD6, macOS and Linux (Q5 A).

### 5. Split, waves and size

| Round | Content | Size |
| --- | --- | --- |
| R19-A | Harness, scripted driver, real-spec fixture (after the operator names the spec), matrix evidence map skeleton | medium |
| R19-B | Runs, fault scenarios, restart inspection, both groupings, both subjects | medium |
| R19-C | Records: acceptance-matrix and roadmap closure, limits restated | small |

Expect 3 to 5 runs, plus correctives for any product defects.

### 6. Risks and dependencies

Depends on R18, E13, E15 and the operator naming the real spec (Q17 A). Risks: the real spec may
exceed capture ceilings (E15) or need checks whose tools are not in the Docker image; a live
model-driven run is non-reproducible and must not be presented as gate evidence; Linux timing for
subprocess-heavy suites.

## Overview of sequencing and parallelism

| Order | Rounds | Can run alongside | Shared-file constraint |
| --- | --- | --- | --- |
| 1 | RA01-A docs | X08 (docs vs verification; different files) | none |
| 2 | RA01-B, RA01-C | none | wave 1 owns runtime ABI, `commands.lua`, bootstrap, response, root help |
| 3 | R15-A1, R15-A2 docs | RA01-C gate | none |
| 4 | R15-B to R15-E | R16-A docs during R15-E (re-reviewed against R15's final contract) | runtime and `ledger/acceptance*` single owner per wave |
| 5 | R16-B, R16-C | R17-A docs (after R16-A is approved), then R17-B process runner (new directory only) | none |
| 6 | R17-C, R17-D | R18-A docs | `ledger/execution`, `reconcile.lua` single owner |
| 7 | R18-B, R18-C | R19-A harness (tests only) | none |
| 8 | R19-B, R19-C | none | none |

Rough total: 30 to 45 workflow runs, assuming E1 to E17 are answered in one batch now.

## Operator questions

Each question lists options, a recommendation and the governing source. None repeats a choice that a
contract, v5, the roadmap, Q1 to Q18, Q-INIT or Q-G35 already settles. E1, E3, E14 and E16 affect
several steps; the rest are tied to one step. E16 and E17 were added by the design review
([execution-design-review.md](execution-design-review.md)).

**E1. Verb surface and grouping (all steps).**
- A: the fine-grained table in "Proposed verb surface": one family per verb, reads separate from
  mutations (`status`, `preview`, `recover` read; `activate`, `claim`, `takeover`, `refresh`,
  `checkpoint`, `release`, `check`, `check-recover`, `verdict` mutate; RA01 adds `spec-prompt`,
  `critique`, `ingest`, `explain-rejection`, `observe`, `orchestrator`).
- B: compact: `status` with selectors for preview, recover and request lookup; `claim --takeover`;
  `check --recover-reservation`; `refresh` also records checkpoints.
- C: v5 spellings where possible (`explain` as its own verb, v5 `planar-design-spec-v5.md:589`).
  Request lookup is `status --request-id` under every option, because v5 spells it so (`:368`); the
  open sub-choice is whether `history --request-id` (`ledger-contract.md:446`) also returns execution
  results (and therefore tokens, R15-9).

Already fixed by operator decisions: separate `claim`, `status` and `release` verbs (Q14 A) and an
`ingest` verb for the job prompt (Q10 B). E1 decides only the remaining names and grouping.

Recommend A: every mutation then has exactly one finite service sequence, which is how the command
contract is written (`command-contract.md:188-196`), and the names match Q14 A's separate verbs.
Source: Q14 A; v5 `planar-design-spec-v5.md:589`; `command-contract.md:37-82`.

**E2. How the active job is selected (R15).**
- A: a separate `activate` mutation with expected revision and expected job, one active job per
  ledger; load reconciles the active job against each new revision; a removed job deactivates.
- B: a `--job` option on load, making activation part of revision acceptance (a job switch without a
  declaration change then needs a new non-noop revision).
- C: a new workspace declaration field `active_job` (a declarations amendment; job switching becomes a
  file edit plus load).

Recommend A: it leaves load's request and noop semantics unchanged, and the ledger contract says load
selects no job. Note v5's "switching jobs ... as one revision change" leans to B or C. Source:
`ledger-contract.md:34-35`; v5 `planar-design-spec-v5.md:125`, `:489`, `:497`.
Note (review): `ledger-contract.md:34-35` describes the R11 foundation and defers active-job semantics
to a later contract (`:579-581`); it does not forbid B. v5 also lists "active jobs" among the
workspace declarations (`planar-design-spec-v5.md:110`), which supports C. Choosing A is a recorded
deviation from v5 `:497` for `v5-alignment.md`.

**E3. Interpretation binding and profiles for new verbs (RA01 to R18).**
- A: exact binding (EX-6): execution and preview verbs require the invocation's effective stdlib digest
  to equal the active revision's; profile-accepted revisions require the same profile; all new shipped
  families join the closed factory with X-CAP admission rows from R15; RA01 ships embedded-only.
- B: execution is embedded-only: new verbs refuse under `with-profile`, and revisions accepted with a
  profile cannot be executed until a later round.
- C: reinterpretation: new verbs reinterpret the active revision's stored bundle with the current
  stdlib or profile and record both digests.

Recommend A: v5 makes stdlib changes part of revision interpretation, and the ledger contract forbids
silent regeneration under current Lua. Cost: any rebuild that changes embedded Lua requires a fresh load
before execution continues. Source: v5 `planar-design-spec-v5.md:507`; `ledger-contract.md:104-106`;
Q1 B; Q12 A.

**E4. How a load rejection becomes a prompt (RA01, A36).**
- A: a pure `explain-rejection --diagnostic FILE` verb over load's stderr JSON; load bytes and exits
  unchanged.
- B: a database-free `validate` verb that repeats load's capture and validation and returns a prompt
  (exit 0) with the raw error embedded.
- C: add a prompt field to load's error envelope (changes error bytes).

Recommend A: smallest surface, trivially deterministic, and load stays exact. Source:
`roadmap.md:353-355`; v5 `planar-design-spec-v5.md:744`; `render-contract.md:615-626`.

**E5. Execution work fingerprint (R15).**
- A: `work_exec_v1` is ledger-closed: declared base label, start-facet values from publications, plus
  the repair revision; base content enters output identities only.
- B: captured base identity in W: the driver supplies `--base-root` at claim; Planar stores it and
  status and load reuse the stored identity.
- C: as A, but start-facet values from the claiming role's own checkout.

Recommend A: status, load and claim can then compute W without a checkout, and base changes still
invalidate evidence through output identity. Source: `render-contract.md:349-351`, `:396-401`;
v5 `planar-design-spec-v5.md:237-245`, `:360`.
Note (review): two of these sources weigh against A. v5 lists "root and base identities" among W's
contents (`planar-design-spec-v5.md:241`), and `render-contract.md:349-351` treats a captured base
identity as "newly available material input". Under A a changed base content never moves W, so a
rebased worker is not asked to reconsider its work; only its evidence becomes inapplicable. Choosing
A is a recorded narrowing of v5 `:241`.

**E6. Minimal A34 provenance (R15).**
- A: Planar records its own identities per run natively; the driver may attach a fixed
  `driver_provenance_v1` file at claim, takeover, release and check, with explicit
  `{"status":"unknown"}` leaves.
- B: as A, but only at claim.
- C: individual CLI options (`--model`, `--driver`) instead of a file.

Recommend A: actual model and session settings are often known only when the session ends. Source:
`v5-alignment.md:141-146`; `roadmap.md:372-374`.

**E7. Check result and contract delivery protocol (R17).**
- A: no registry change. `selection:"nonempty"` checks must write a JSON result file
  `{version:1,matched,passed,failed}` to the path in reserved variable `PLANAR_CHECK_RESULT`;
  `not_applicable` checks use exit status only. Accepted contract bytes are materialized read-only into
  a fresh directory under a required `--scratch DIR`, passed as `PLANAR_EXPECTED_CONTRACTS`; the scratch
  subdirectory is removed afterwards with ownership checks. Wrapper scripts for ctest or pytest are the
  driver's trusted registry entries.
- B: registry v2 with an explicit adapter enum, including built-in parsers for JUnit XML (a new pinned
  dependency) and CTest JSON.
- C: as A, but the driver materializes contracts itself and Planar only verifies their bytes in the
  root, refusing on mismatch.

Recommend A: it keeps registry v1 bytes and needs no parser dependency. Source: `declarations.md:347-349`;
v5 `planar-design-spec-v5.md:323-325`, `:497`; Q15 A.
Note (review): v5 `:497` itself states the contract-delivery half: "a driver that supplies only changed
working bytes must materialize the accepted version or receive a revision-mismatch refusal". That is
option C's delivery rule, so A's Planar-side materialization (a new Planar write and cleanup capability
under `--scratch`) departs from the v5 baseline. Under A and C alike, Planar's reserved variables amend
the registry environment rule (`declarations.md:338-339`). The result-file half (`PLANAR_CHECK_RESULT`)
is a genuinely open choice.

**E8. Exit status of a recorded failing or inconclusive check (R17).**
- A: exit 0; the operation recorded evidence, and `outcome` in the JSON says fail or inconclusive.
- B: a new exit 1 meaning "recorded, did not pass", with the result JSON on stdout.
- C: exit 3 with the result in a stderr error envelope.

Recommend A: it keeps the closed exit table and "a failed check changes no state". B is friendlier to
shell drivers. Source: `ledger-contract.md:476-477`; v5 `planar-design-spec-v5.md:395`.

**E9. Verdict submission (R18).**
- A: a strict JSON `verdict_v1` file; event ID is the request ID; policy identity is the reviewer's
  execution work fingerprint.
- B: v5's `planar.verdict{}` Lua constructor evaluated in the declaration sandbox, with an explicit
  `event_id`; policy identity is the active revision ID.
- C: JSON as A, with policy identity the active revision ID.

Recommend A: reviewer output is untrusted session data, like observations and registries, which are JSON;
the reviewer W digest moves only for policy-relevant changes. Source: v5 `planar-design-spec-v5.md:412-430`
("fields illustrative until that round fixes them").

**E10. Recovery cache files (R16).**
- A: Planar writes no `state/recovery/` files; `recover` always reads SQLite.
- B: Planar writes latest-packet caches under the workspace's `state/recovery/` with no-replace staging,
  rebuilt from SQLite when missing.

Recommend A: v5 makes caches optional, and A adds no workspace write capability. Source: v5
`planar-design-spec-v5.md:276`; `acceptance-matrix.md:45`.

**E11. Run token form (R15).**
- A: `<run>.<generation>.<32-hex secret>`; the secret is shown only in claim and takeover responses and
  request lookup; takeover needs only the run ID.
- B: deterministic `<run>.<generation>`, visible in `status` (cooperative model only).

Recommend A: it makes "lost response, recover by request ID" meaningful and stops accidental use of
another run's identity, at no cost to packet determinism. Source: v5 `planar-design-spec-v5.md:59-60`,
`:368-370`.

**E12. Observe directory layout (RA01).**
- A: fixed names: optional `checkpoint.json` (one checkpoint object), `checks/*.json` and
  `findings/*.json` (one assertion each); anything else refuses; names starting with `.` are ignored.
- B: any `*.json` file at top level, each tagged `{"kind":"check"|"finding"|"checkpoint",...}`.
- C: a single `observations.json` passed through with validation.

Recommend A: sessions write one file per item without shared-file edits, and the layout is
self-describing. Source: v5 `planar-design-spec-v5.md:51-52`, `:579`; Q11 A.

**E13. R19 driver (R19).**
- A: a scripted deterministic driver under `tests/` that follows the orchestrator fragment's legal
  actions, with scripted session edits, as the gate; plus one recorded live model-driven run (Claude Code)
  as non-gating experience evidence.
- B: live model-driven runs only.
- C: scripted only.

Recommend A: the gate must be reproducible, and v5 still asks for an actual driver using the fragment.
Source: `roadmap.md:407-415`; v5 `planar-design-spec-v5.md:587`, `:627`.

**E14. Where the new exact contracts live (RA01, R15).**
- A: two new exact contracts, `authoring-contract.md` (RA01) and `execution-contract.md` (R15 to R18),
  added to CLAUDE.md's authority list item 2, with short pointer amendments in the existing contracts.
- B: amend the existing contracts in place only.

Recommend A: the ledger contract is already 582 lines, and execution is a separate trust boundary.
Source: CLAUDE.md "Authority and scope"; `ledger-contract.md:579-581` ("need their own reviewed
contracts").

**E15. Checkout capture ceilings for checks (R17, R19).**
- A: execution observations are digest-only (bytes hashed and receipted, not retained, except facet
  payloads and the boundary diff set), with larger ceilings: 1 GiB hashed bytes, 100,000 files, depth 64.
- B: keep render's retained-capture ceilings (64 MiB, 10,000 files); real repositories must exclude
  enough paths to fit.

Recommend A: v5 says output identity "does not require Planar to retain all source bytes", and real
checkouts exceed 10,000 files. Source: `render-contract.md:138-141`; v5 `planar-design-spec-v5.md:235`.
Note (review): under A the before/after recheck compares sizes and per-file XXH3 digests rather than
exact bytes (R17-1 step 2); since the digest is an index, not an identity (v5 `:252`), that is a
recorded limit of the cooperative stability contract, not a collision-safe comparison.

**E16. New public error codes and the execution resource message (R15 to R18). Added in review.**
- A: the execution contract adds the codes of EX-7 (`no_active_job`, `claim_conflict`,
  `state_conflict`, `fenced`, `interpretation_mismatch`, `history_unavailable`, `drifted`,
  `check_in_progress`, `unsupported_policy`) and one closed message `Execution resource limit exceeded`
  for execution exhaustion sites, as a scoped amendment of XD4 for execution only.
- B: no new codes: reuse existing codes (`revision_conflict`, `request_conflict`, `reference`,
  `schema`, `resource_limit`) with typed `details` distinguishing the cases, and reuse the existing
  X-LIMIT exhaustion message verbatim.
- C: new codes as A, but reuse the existing X-LIMIT message for exhaustion.

Recommend A: drivers must tell fencing, drift, lost races and an in-progress check apart without
parsing messages (v5 "typed errors", `planar-design-spec-v5.md:589`; render contract "codes and
details are the API", `render-contract.md:626`), and the X-LIMIT message names external profiles.
Source: XD4 (`remaining-steps-design.md:67`), which says "No new public code or message is added" and
is listed as a cross-cutting decision for every remaining step; X-LIMIT
(`external-profile-contract.md:526-527`); v5 `planar-design-spec-v5.md:276` (`history_unavailable`).

**E17. What the `max_rounds` count is keyed on (R18). Added in review.**
- A: per job activation; loads, refreshes and regrouping loads never reset it; re-activation (new
  generations, no inherited approvals or repairs) starts a new count.
- B: per job activation and role structure: a load that changes any role's structure (a regrouping)
  starts a new count.
- C: per job for the life of the ledger; only a raised `max_rounds` in a later revision permits more
  cycles.

Recommend A: it reads v5's "active job and shape generation" as the activation, which is an explicit
driver act and already starts every role in a fresh generation with nothing inherited, while
contract edits and refreshes cannot reset it as v5 requires. B lets an ordinary load reset the cap;
C makes an unrelated re-activation inherit an exhausted cap. Source: v5
`planar-design-spec-v5.md:434`; A14 (`acceptance-matrix.md:36`); E2.

## Decisions table

Operator answer 2026-10-28: all recommendations accepted as listed below (E7 per the design review: C plus A's result file; E3 with the read exemption). These answers are binding for RA01 and R15-R19.

"Choice" is the selected option letter; any change to a recommendation
is a scoped instruction for the owning step's contract round.

| ID | Choice | Operator answer | Date |
| --- | --- | --- | --- |
| E1 | A | A: one command per verb, reads separate from mutations; replace `--confirm-stopped` and settle the `history --request-id` sub-choice in R15-A1 | 2026-10-28 |
| E2 | A | A: separate `activate` with expected revision and job; recorded deviation from v5 :110 and :497 | 2026-10-28 |
| E3 | A | A: exact stdlib and profile that accepted the revision; reads that need no current Lua are exempt so an upgrade cannot block recovery | 2026-10-28 |
| E4 | A | A: pure `explain-rejection --diagnostic FILE`; load stays byte-exact | 2026-10-28 |
| E5 | A | A: fingerprint from the ledger alone; accepted knowingly as a narrowing of v5 :241 | 2026-10-28 |
| E6 | A | A: fixed driver provenance file at claim, takeover, release and check, explicit unknowns | 2026-10-28 |
| E7 | C + A | C for contract delivery (driver places accepted contract bytes, Planar refuses on mismatch, v5 :497) plus A's JSON result-file protocol; reserved environment variables amend declarations.md:338-339 | 2026-10-28 |
| E8 | A | A: exit 0 with the outcome in JSON | 2026-10-28 |
| E9 | A | A: strict JSON `verdict_v1` | 2026-10-28 |
| E10 | A | A: SQLite only, no recovery cache files | 2026-10-28 |
| E11 | A | A: `<run>.<generation>.<secret>`; the secret guards against accidental reuse, not ledger readers | 2026-10-28 |
| E12 | A | A: `checkpoint.json`, `checks/*.json`, `findings/*.json` | 2026-10-28 |
| E13 | A | A: scripted deterministic driver gates; one recorded live model-driven run as non-gating evidence | 2026-10-28 |
| E14 | A | A: new `authoring-contract.md` and `execution-contract.md`, added to the CLAUDE.md authority list (approved) | 2026-10-28 |
| E15 | A | A: hash without retaining bytes, 1 GiB / 100,000 files / depth 64; digest-only recheck recorded as a limit | 2026-10-28 |
| E16 | A | A: add the new public codes and the execution message as flagged contract amendments | 2026-10-28 |
| E17 | A | A: review-cycle cap keyed per job activation | 2026-10-28 |

## Changelog

- 2026-10-28, design review ([execution-design-review.md](execution-design-review.md)). No
  recommendation of E1 to E15 was changed; disagreements are recorded in the review and as "Note
  (review)" lines under E2, E5, E7 and E15.
  - Binding inputs: the Q9 A row now states Q9 (declaration-VM memory), not Q2's text.
  - EX-1: format3 is format2 with modified `events`, `requests` and `current_state` plus added tables,
    not "the exact format2 schema plus"; the request/event cross-row invariant names the check
    exception.
  - EX-2: `status --request-id` is v5's spelling (C), not a placement choice.
  - EX-3: projections are written as a delta with a locked before-state check per transitioned role
    (lost-update guard); R15 gains the matching overlap test.
  - EX-4: removed `role_reconciled`, which contradicted "one operation is one event"; load keeps
    `declaration_accepted`/`load_noop` with attached transitions.
  - EX-5: takeover generations fence only and carry the structural lineage (completions, evidence,
    repair revision, findings, approvals); R18 gains the matching test.
  - EX-6: the binding check follows early replay; cites the closed five-family variant
    (`x07-addendum.md:60`).
  - EX-7: new codes and the new message are tagged operator E16 (XD4), not engineering.
  - EX-8: observe output bounded at 1 MiB (the observations input limit); responses bounded by the
    16 MiB output ceiling.
  - EX-9: escalation citation corrected to `orchestration.md:26-28`.
  - Verb surface: root help is fixed by `command-contract.md:94-96` and X-CLI
    `external-profile-contract.md:133-134`, not an X07-scoped rule; new host-path slots noted.
  - RA01: with-profile prompts carry sealed-catalog shapes (narrowing-risk row); "E14" disambiguated.
  - R15-2: basis citation corrected (v5 does not say "one active job per ledger"; `:110`, `:497`).
  - R15-9: request lookup (including `history --request-id`) exposes the token; history pages do not.
  - R16-7, R17-8, R18-8: orchestrator guidance and provenance clauses the roadmap requires.
  - R17: amendments of `declarations.md:338-339` and `command-contract.md:190-194` flagged; completion
    also requires complete-phase dependency qualities and no open start blocker; fenced completion is
    an open R17-A choice instead of an unflagged ledger write by a superseded token; recheck
    comparison follows E15; check request/event rows made consistent with `event_seq UNIQUE`; A28 and
    completion-gating tests added.
  - R18-2: optional `authority` field so R18-7's refusal is reachable; R18-5 counting key tagged E17.
  - Sequencing: R17-A placed before R17-B.
  - Added operator questions E16 and E17; E1 option C corrected (v5 spells request lookup as
    `status --request-id`).
