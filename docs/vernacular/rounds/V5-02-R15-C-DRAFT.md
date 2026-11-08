# V5-02-R15-C (DRAFT): reconcile, dispatch projection, format3 load, activate, init cutover

Draft mode (operator decision 2026-11-08): code- and feature-complete first, broken allowed. Base commit
`1338637c` (R15-B draft base with the C01 R1 fix), lane worktree `planar-lane-a`, branch `draft/lane-a`. Lane A then
continues with R15-D, R15-E and R16, so nothing here is split with lane B.

Kept: Debug compile (`cmake --preset debug`, `cmake --build build/rewrite-debug --target planar_tests`), meaningful
colocated `*.t.cpp` tests (plain `//` header, per-test temporary roots), running the tests each package adds with a
nonzero matched count, and an honest list of what fails. Dropped until the fixing rounds: full gate, rewrite-check,
Linux/Docker, mutation probes, reviews, evidence directories.

## Scope

From [execution-design.md](../execution-design.md) R15 section 5 (row R15-C) and the R15-B record:

- Trusted Lua `reconcile.lua` (EX-4 precedence, EX-5 structure, generations and lineage) and `dispatch.lua`
  (`work_exec_v1`, dispatch projection P2-2, `dispatch_manifest_v1`, the internal `dispatch_context_v1` input).
- Native execution services: `read_execution({})` snapshot value, decoding of Lua's `execution_change_v1` into the
  R15-B `ledger::execution_change`, `commit_execution({change})`.
- Format3 load reconciliation (R15-3, EX-4 "Format3 load reconciliation", "Absent active job"): four services,
  `accept_load({bundle,semantic,execution})`, revision + event + transitions + generations + request in one commit.
- `activate` (R15-2, EX-4 "Activation", "Re-activation", P2-1 option map, refusal order, `activate_v1`) end to end.
- Init cutover to format3 (EX-1, Q13 A, XD5), with the format3 read compatibility that existing `history` and
  `export` need after the cutover.
- R15-B carried items naming R15-C: L2 residuals (five format2-era cross-row checks on format3), L10 (D149 pins),
  L16 (`snapshot_seq` equals `snapshot.last_seq`), L17 (raw provenance file size), L18 (spellings), L20 / review A4
  (handler emits no-row `role_state` members), G2 (load interning in the load's own write transaction),
  `dispatch_context_v1`, review advisory A1 (dispatch revision in `execution_state_bytes`) if cheap.

Out of scope (R15-D on this lane): `status`, `preview`, `claim`, `takeover` verbs, `history --event`, format3
`history --request-id` assembly for claim/takeover, orchestrator live form, the other four admission rows and root help
lines. R15-E: real-process races, SIGKILL/fault variants, A09/A10/A20/A22/A23/A26/A33 integration.

## Module placement (EX-9)

| Destination | Content |
| --- | --- |
| `src/stdlib/planar/dispatch.lua` (new) | `work_exec_v1` W, `role_structure_v1` values, dispatch packet and manifest |
| `src/stdlib/planar/reconcile.lua` (new) | reconcile pass for load and activate; builds `execution_change_v1` |
| `src/stdlib/planar/commands.lua`, `bootstrap.lua` | format3 load path, `activate` handler and descriptor, fixed exports |
| `src/engine/execution/` (new, mirrors `acceptance/`) | snapshot value, change decoding, provenance input |
| `src/engine/ledger/execution.*` | G2 interning (additive), L2 residual checks, A1 |
| `src/engine/ledger/acceptance*` | format3 `complete_acceptance` branch |
| `src/engine/ledger/storage*`, `src/engine/history/read.*`, `src/engine/export/source_export.*` | format3 read compatibility |
| `src/engine/runtime/runtime.{cppm,cpp}` | `read_execution` / `commit_execution` services, load call count 4 on format3, `activate` ABI and admission row |
| `src/cmd/planar/handlers/activate/command.{cppm,cpp}` (new), `bootstrap.cpp`, `invocation.cpp` | CLI11 entry, prelude, native services |

## Packages (sequential)

### P1 - Lua reconcile and dispatch projection (pure)

Files: `src/stdlib/planar/dispatch.lua`, `src/stdlib/planar/reconcile.lua` (new); `src/stdlib/planar/bootstrap.lua`
(fixed exports `work_exec`, `render_dispatch`, `reconcile_execution`); every stdlib file list in
`src/engine/CMakeLists.txt` (main list and the generated variant lists, before `acceptance.lua`/`commands.lua`);
tests `src/engine/runtime/reconcile.t.cpp`, `src/engine/runtime/dispatch.t.cpp` (new, registered with the existing
runtime test helper).

- `dispatch.lua`: W per P2-3 (24 fields; render-contract field definitions reused from `render.lua`; `base` declared
  form; `start_facets` all `unavailable:true` in R15; `repair` `"0"`; versions list), `role_structure_v1` per EX-5,
  dispatch packet per P2-2 (mode `dispatch`, `run` second and mandatory, both authority templates, the D147 run-section
  bytes, budget algorithm of render with `run` never omitted), `dispatch_manifest_v1` (14 keys) for dispatch and
  preview, all from a `dispatch_context_v1` value (draft decision DD3).
- `reconcile.lua`: input = snapshot value (P2-1 `read_execution` result shape), operation (`load` with the candidate
  semantic and interpretation, or `activate` with job and expectations). Output = `execution_change_v1`: generations
  (structural change, retirement, `recreated`, `activated`; never reused), `fenced_runs`, transitions with the closed
  reasons and NULL-first before states, the complete next vector, drift on running roles (current W vs acknowledged W
  from `snapshot.roles[].run.dispatch.work`), absent active job (D94), re-activation rules, and `expected` exactly per
  EX-3 "Format3 load and activate preconditions" including one no-row `role_state` member per role of the active job
  (and of the activated job) without a row (L20, A4). `snapshot_seq` is the snapshot's `last_seq` (L16).
- Tests through the fixed exports with handcrafted snapshot values: precedence of the six rules, structural change vs
  in-place drift, removed role, re-added role, absent active job, re-activation, expected-set exactness and no-row
  members, W stability under takeover-only changes, D147 run-section bytes, manifest key set, fresh-VM byte equality.

### P2 - Native execution services

Files: `src/engine/execution/snapshot_value.{cppm,cpp,t.cpp}`, `change_value.{cppm,cpp,t.cpp}`,
`provenance_input.{cppm,cpp,t.cpp}` (new), `src/engine/CMakeLists.txt` (module and test registration),
`src/engine/ledger/execution.{cppm,cpp,t.cpp}` (L2 residuals, A1, the additive G2 interning hook used by P3).

- `snapshot_value`: `ledger::execution_snapshot` to `{version:1,ledger,last_seq,snapshot}` exactly as P2-1, with the
  L18 spellings R15-B fixed (`run.generation` ordinal; D24 registry/revision reduction), no secrets.
- `change_value`: closed-shape decode of `execution_change_v1` into `ledger::execution_change`; refuses
  `snapshot_seq != last_seq` (L16) and a load/activate change that omits a required no-row member (L20); the request
  framing (`request_v2` for load, `request_v3` with the P2-1 parameter map otherwise) is built natively from typed
  options, never from Lua. Failures map to `internal` ("handler response mismatch", D151).
- `provenance_input`: `read_provenance` single-file capture with P2-4 check order, raw size before parse (L17); wired
  into claim/takeover by R15-D.
- `ledger/execution`: L2 residuals on format3 (tree reachability, directory edges vs `record_children`, embedded
  versions vs `versions`, workspace ID vs `ledger_meta`, recorded family vs profile selector) using `tree_core` and
  `recorded_interpretation`, one corrupting test each; A1 (`execution_state_bytes` uses each dispatch's own revision);
  G2: `execution_change` gains an optional list of prepared records to intern, written collision-safe inside the
  staged transaction before `check_revision_records` (existing fields unchanged).

### P3 - Format3 load reconciliation and activate end to end

Files: `src/engine/ledger/acceptance.{cppm,cpp}`, `acceptance_seam.{cppm,cpp}` (format3 branch),
`src/engine/runtime/runtime.{cppm,cpp}`, `src/cmd/planar/invocation.cpp`, `src/stdlib/planar/commands.lua`,
`src/cmd/planar/handlers/activate/command.{cppm,cpp}` (new), `src/cmd/planar/bootstrap.cpp`,
`src/cmd/planar/CMakeLists.txt`; tests `src/engine/ledger/acceptance.t.cpp` (added cases),
`src/cmd/planar/activate.t.cpp` and `src/cmd/planar/load_format3.t.cpp` (new). Ledgers in tests come from
`initialize_format3` directly (the cutover is P4).

- Load prelude on format3: early replay through `lookup_execution_request` with `request_v2` framing; the load
  context exposes `read_execution` only on format3 (DD1); Lua calls `read_execution`, `reconcile_execution`, then
  `accept_load({bundle,semantic,execution})`; native prepares the candidate, interns its records in the same write
  transaction (G2), CAS of the expected head, EX-3 preconditions, one commit; noop load writes `load_noop`, no
  transitions; saved result and response bytes are the existing load envelope.
- `activate`: CLI11 options per P2-1 / P2-7 (`--expect-job` metavariable `JOB_OR_NONE`), descriptor
  `shipped.activate.v1` / `activate_v1` / `execution_commit_v1`, services `read_execution` then `commit_execution`,
  refusal order (1)-(7) of P2-1 with EX-6 `interpretation_mismatch`, `revision_conflict`, `no_active_job`, D149 site
  `--expect-job NONE` while a job is active is `state_conflict` before the lock (L10 pin: test asserts the code and
  that a same-ID retry still refuses), `reference` for an unknown job; response `activate_v1`.
- Catalog: production catalog becomes six rows, the activate admission row joins the with-profile family list
  (DD4), root help gains the `planar activate ...` line at the P2-7 position.
- Tests: load on a format3 ledger with no job, with an active job (structural change fences and retires, removed
  role retires, absent active job clears it, re-added role `recreated`), noop load, same-ID replay and conflict,
  format1/format2 load unchanged; activate first time, switch job (previous job retired, runs fenced), re-activation,
  each refusal step, replay bytes, D149 activate pin, interpretation mismatch on a revision recorded with another
  stdlib digest.

### P4 - Init cutover and format3 read compatibility

Files: `src/cmd/planar/invocation.cpp` (`initialize_ledger` calls `initialize_format3`, response bytes unchanged),
`src/engine/ledger/storage.{cppm,cpp}`, `storage_types.cppm` (`ledger_format::v3` read access for history/export),
`src/engine/history/read.{cppm,cpp,t.cpp}` (format3 event summary and `--at-event` state of P2-5; `--request-id`
for `load` and `activate`), `src/engine/export/source_export.{cpp,t.cpp}`, and existing tests that pin "init creates
format2" (XD5: names kept, only that assertion changes; listed in the coder report).

- Tests: `planar init` then `planar load`, `planar activate`, `planar history` (page, revision, request, at-event),
  `planar export` on the new format3 ledger; format1/format2 fixtures created explicitly still read unchanged.
- Run the whole existing load, history, export, init and bootstrap test sets once and list every failure with its
  cause; fixing them all is not required in draft mode.

## Draft decisions

- DD1: format selection for load stays native. The load invocation context exposes `read_execution` only on a
  format3 ledger; Lua tests `context.read_execution` and otherwise runs the unchanged three-service path, so format1
  and format2 keep exactly three services (P2-1 services table). No new option or field crosses into Lua.
- DD2: `activate` is implemented end to end in R15-C (handler, Lua family, ABI and admission row, help line),
  following execution-design R15 section 5, which lists activate under R15-C; the R15-B record's "R15-D: activate entry
  points" is superseded. It is the only way to test load reconciliation with an active job before R15-D.
- DD3: `dispatch_context_v1` is the internal value `{revision,job,role,generation,run,snapshot_role,interpretation,
  publications,repair,budget_bytes}` that `dispatch.lua` consumes (`run` is `{request,kind[,previous]}` or
  `{kind:"preview"}`); it is not an admissible external projection input and has no codec or record.
- DD4: admission and catalog grow one row at a time: R15-C adds `activate` (catalog 6 production, 12 authoring;
  with-profile family list 6); R15-D adds the other four to reach the P2-7 counts of ten and sixteen. The x07
  addendum "closed at five" consequence is accepted as the P2-7 amendment already states.
- DD5: the D149 takeover site (unknown `--run-id`) is pinned with the takeover verb in R15-D, because the verb does not
  exist before then; R15-C pins the activate site.
- DD6: L17 lives in `engine/execution/provenance_input`, a native function built and tested in R15-C and wired to
  `claim` and `takeover` in R15-D.
- DD7: after the cutover, `history` and `export` read format3 through the existing reader extended with a format3
  variant (format3 is the format2 tables plus the execution tables), not through the R15-B execution reader; `history
  --event` and claim/takeover request assembly stay in R15-D.
- DD8: new stdlib files change the embedded stdlib digest and VM instruction counts. Pinned digests or budget-ceiling
  fixtures that move are updated or reported as failing; ceilings are not raised in this draft.

### P1 draft decisions (Lua reconcile and dispatch projection)

Interfaces P2 and P3 code against. Fixed exports: `work_exec`, `render_dispatch`, `reconcile_execution`.

- DD9 (`work_exec` input/output): `{interpretation:{bundle,semantic},job,role,publications?,repair?}` returns
  `{work:W,digest,structure,structure_digest}`; `digest` is `H("work_exec_v1",W)`, `structure_digest` is
  `H("role_structure_v1",structure)` (the same domain as native `role_structure_digest`). Unknown job or role is
  `reference` with `Unknown selected job`/`selection.job` or `Unknown selected role`/`selection.role`.
- DD10 (`dispatch_context_v1` exact form): `revision` is `{id,source,interpretation}` (decimal strings: the revision id
  and the record ids of `revisions.source_id` and `revisions.interpretation_id`, which the snapshot value does not carry,
  so the handler supplies them); `interpretation` is the decoded recorded `{bundle,semantic,...}` envelope; `generation`
  is the ordinal decimal string; `run` is `{request,kind:"claim"}`, `{request,kind:"takeover",previous}` or
  `{kind:"preview"}`; `snapshot_role` is accepted and unused; `publications` and `repair` default to `[]` and `"0"`;
  `budget_bytes` defaults to the role budget. `render_dispatch` returns `{version:1,kind:"dispatch",packet,manifest,work}`.
- DD11 (packet body): every section other than `authority` and `run` reuses the render base-section builder
  (`role_base_material`) with `observation` `not_captured`, no files and no observations, so outputs, checks and
  dependencies keep render prose ("Not executed by render", "Outputs and base not captured"). Rewording those bodies
  for dispatch is left to the fixing rounds; the contract pins only the authority and run bytes.
- DD12 (prerequisite qualities in R15): contract dependencies are `available`; a facet dependency of quality
  `available` needs a publication of the facet; quality `done` (and pipeline predecessors) are available only when the
  producer role's ledger state is `done`; `accepted` is `unavailable` until R18; questions are `answered` or
  `unavailable`. A dependency whose producer unit belongs to the same role never blocks that role. A reviewer is
  blocked until every worker role of the job is `done` (semantic graph edge `role:<worker>:complete ->
  role:<reviewer>:start`); this rule is not listed in the manifest prerequisites.
- DD13 (`execution_change_v1` spellings): `snapshot_seq` is `snapshot.last_seq` (L16). `facts` is
  `{job,generations,fenced_runs}` for both load and activate (`job` is the active job after the commit, `""` when D94
  clears it; activate needs `fenced_runs` too because ledger `execution_change` carries them, a superset of the
  P2-1 `{job,generations}`). A generation is `{job,role,reason,structure}`; `fenced_runs` are sorted decimal strings.
  A transition is `{job,role,before_state?,before_drifted?,after_state,after_drifted,reason}`; NULL-first means the
  two before keys are absent. A vector entry is `{job,role,state,drifted,run}` with `run` a decimal string or `"none"`;
  the vector holds every `role_state` row the active job has after the commit (retired rows included) and is `[]`
  when no job is active afterwards. Transitions, generations and vector are sorted by (job, role).
- DD14 (`expected` spellings): `{kind:"request_unused",request_id?}`, `{kind:"active_revision",revision}` (decimal or
  `none`), `{kind:"active_job",job}` (`""` for none), then per (job, role) sorted: `{kind:"fact_version",job,role,
  fact_version}` and `{kind:"role_state",job,role,state,drifted,run}` for a role with a row, or
  `{kind:"role_state",job,role,no_row:true}` for one without (L20, A4). "Roles of the active job" is the union of its
  `role_state` rows, the roles it declares in the active revision and, for load, in the candidate; for activate the
  same union is taken for the activated job.
- DD15 (no state-machine row): a role newly added to the active job by load (no row, NULL before) gets a new
  generation and transition with reason `structural_change`; the role table has no load row with a NULL before.
- DD16 (no completion facts in R15): `done` becomes `stale` (`work_changed`, M24) when the role's W under the candidate
  differs from its W under the active revision; a `blocked` role whose prerequisites return becomes `ready` (M20, never
  M21/M22); `stale` never returns to `done` (M40 needs R17 facts). Activation computes fresh states with no producer
  `done` because every generation starts empty.

### P2 draft decisions (native execution services)

- DD17 (snapshot value `revision.source`): P2-1 names `source` among the revision keys without its form. It is
  `{id,digest}` (decimal record id and 32 lowercase hex digest), the same form as `registry`. `interpretation` and
  `versions` are the decoded recorded values, `stdlib_digest` the bare hex digest. Dispatch `work_fp` and `render_fp`
  are the bare 32-hex texts the D24 value S already uses; the `xxh3_128:` text form stays a response spelling (claim,
  status). The snapshot value minus the revision expansion is byte-for-byte the D24 value S (pinned by a test).
- DD18 (empty lists from Lua): `decode_execution_change` reads an empty record as an empty list, because an empty Lua
  table crosses the bridge without an array tag. Any non-empty record where a list is expected is still refused.
- DD19 (`generation_current` spelling): `{kind:"generation_current",job,role,generation}` with `generation` the decimal
  generation identifier (what `execution_expectation::number` stores), not the ordinal. Claim/takeover handlers (R15-D)
  must emit this spelling.
- DD20 (claim/takeover facts and lineage): claim facts are exactly `{role,provenance?}` and takeover facts
  `{role,previous_run,provenance?}` (P2-1). `facts.provenance` must be present exactly when native read a provenance
  file and must be canonically equal to that document; the ledger change always takes the native document. The
  takeover generation (`reason:"takeover"`, the snapshot row's structure) and `fenced_runs = [--run-id]` are built
  natively, not read from Lua. A claim/takeover change carries `packet` `{markdown,manifest,work}`; load/activate never.
- DD21 (L20 enforcement site): the decoder requires a no-row `role_state` member for every (job, role) whose first row
  the change writes (a transition without before keys, or a generation for a role with no snapshot row), and refuses a
  no-row member that names a role with a snapshot row. The rest of the exact-set rule (members for declared roles that
  the change leaves untouched) stays with the ledger's `check_snapshot` and the Lua builder; native cannot enumerate
  declared roles without the interpretation.
- DD22 (change failures): every decode failure is `change_error{field}` with a dotted member path for diagnostics; the
  runtime service maps it to redacted `internal` ("handler response mismatch", D151). No new message.
- DD23 (provenance messages): P2-4 schema refusals reuse the schema normalizer messages of `schema.lua`: `Invalid
  JSON` (grammar, field `provenance`), `Required field`, `Unknown field`, `Expected object`, `Expected list`,
  `Expected version integer`, `Unsupported schema version`, `Invalid string` (empty, NUL, wrong type or an object other
  than `{"status":"unknown"}`) and `Duplicate value` (repeated setting name, code `schema`). Capture refusals reuse the
  authoring `--diagnostic` rows; a file that grows past 65536 bytes between the gate and the read is
  `resource_limit provenance_bytes`. The capture goes through the shared `capture_diagnostic` after a native size gate.
- DD24 (L2 residuals without the full stdlib proof): the format3 verifier checks the five residuals natively:
  directory edges equal the encoded children (and every edge parent is a directory), each revision's three trees
  reconstruct through `tree_core` under their profiles (the stdlib profile follows the recorded family), the
  interpretation's embedded `versions` equals the versions record byte for byte, the bundle workspace id equals
  `ledger_meta.workspace_id`, and each load request's selector (profile argument present or not) matches the revision's
  recorded family (`classify_recorded_interpretation`). It does not run `verify_recorded_interpretation`, which would
  recompute the stdlib tree digest against the shipped stdlib file set; the stdlib digest is already tied to the
  versions record by the existing D23 check. Running the full proof is left to the fixing rounds (it would reject the
  R15-B test fixtures, which seed a one-file stdlib).
- DD25 (G2 hook shape): `execution_change::prepared` is an optional `execution_prepared_revision` (three
  `{files,profile}` trees and the interpretation and versions canonical bytes). It is allowed only on an accepted
  (non-noop) load; under the lock, after the preconditions and before `check_revision_records`, native interns every
  tree record and value record collision-safe (D38 exact-row reuse, `hash_collision` otherwise), writes the edges of a
  newly inserted directory, and uses the interned ids in place of the five ids of `revision`. A noop load names the
  active revision's ids directly and carries no prepared records.
- DD26 (A1): `execution_dispatch` gains `revision` (the dispatch row's `revision_id`), and `execution_state_bytes`
  measures each dispatch at that revision, so it equals the reader's `state_bytes`.

### P3 draft decisions (format3 load and activate)

- DD27 (activate handler module and compositions): the CLI11 family lives in `src/cmd/planar/handlers/activate/` as the
  module `planar.cmd.execution.activate` (namespace `planar::cmd::handlers`), deliberately outside the
  `planar.cmd.planar.handlers.` prefix. The audited composition generators under `tests/native/` copy `bootstrap.cpp`
  and a fixed handler list and rewrite that prefix; with the prefix the copies would import a module that no generator
  produces. Every composition therefore binds and dispatches activate through the one real module with a fresh VM of the
  real runtime (the bootstrap calls it with fully qualified names and never passes its own VM, whose type differs in a
  composition). The format3 modules reach the generated invocation copies through `planar_command_response` and the
  activate module reaches the generated bootstrap copies through `planar_command_envelope` (PUBLIC link items in
  `src/cmd/planar/CMakeLists.txt`); the generators keep their fixed dependency lists. Extending the generators is left
  to the fixing rounds.
- DD28 (format3 completion site): the format3 branch of load completion is implemented in the native load service of
  `src/cmd/planar/invocation.cpp` (prepare the candidate, decide noop natively, decode the change with the `request_v2`
  framing, `stage_execution` with the G2 prepared records, final receipt verification, COMMIT, completion receipt), not in
  `src/engine/ledger/acceptance{,_seam}.cpp`. Those sources are cloned by the audited `r14c3b2` and `x03-b2-b-c01`
  generators whose targets cannot import the execution ledger; adding the import there breaks the clones. The
  acceptance module is unchanged, and `acceptance.t.cpp` gains no case (its test target does not link the format3
  schema); the format3 completion is covered by `src/cmd/planar/load_format3.t.cpp`.
- DD29 (no-row members, EX-3 wording): only roles of the snapshot's active job assert a no-row `role_state` member.
  For `activate`, roles of the job being activated assert members only when they already have a row (EX-3 "Format3
  load and activate preconditions"), and the R15-B ledger refuses a no-row member outside the active job. This narrows
  DD14 (P1) and DD21 (P2): `reconcile.lua` and `change_value.cpp` were adjusted in P3 so that a first activation and a
  job switch commit.
- DD30 (noop decision): native decides a format3 noop load by comparing the candidate's five records with the active
  revision (the three tree root digests via `digest_tree`, the registry digest from the snapshot registry value, and the
  canonical bytes of the interpretation and versions values). A noop names the active identifiers and carries no
  prepared records; Lua's change for an identical candidate has no transitions, generations or fenced runs.
- DD31 (activate refusal sites): steps (2) and (3) run in the native prelude (`lookup_execution_request` with the
  `request_v3` framing built natively, which refuses format1 and format2 first); steps (4) and (5) run inside the
  `read_execution` service on the very snapshot Lua receives (the EX-6 digest is the embedded stdlib tree digest of
  this binary); step (7) is the reconcile pass (`Unknown selected job`, `selection.job`), which also covers a ledger
  with no active revision. A decode failure of the Lua change is `internal` with the existing message `Invalid embedded
  command protocol`.
- DD32 (with-profile activate): the activate row joins the with-profile catalog completeness rule (six shipped rows,
  `effective_catalog.cpp`) and the admission visitor admits its ledger slot. The EX-6 binding under a profile still
  compares the embedded stdlib digest, which is wrong for a revision accepted under a profile; recorded as a gap.
- DD33 (activate output): the response is the saved `activate_v1` bytes from the ledger plus LF, as load prints its saved
  bytes; the runtime checks the Lua envelope equals the decoded saved value. The post-commit fallback is the EX-7
  `Activate may be complete ...` pair.

- DD34 (runtime latch): the runtime latches `load_format3` in the command state when the load handler starts, because the
  services pointer is cleared before the response is validated; the format3 load expects four service calls and the
  activate handler two.

P3 results (draft, Debug). New tests: `src/cmd/planar/load_format3.t.cpp` (5 cases) and `src/cmd/planar/activate.t.cpp`
(6 cases); `ctest -R` over them plus the P1 reconcile/dispatch and P2 change-value suites matched 45 tests, all passing
after the DD29 test updates in `reconcile.t.cpp` and `change_value.t.cpp`. One run of every `unit`-labelled test (1618)
had 101 failures, grouped by cause: (a) the catalog growing to six production rows and the variant growing to six
alternatives (effective catalog, C4, X05, RA01, v2/recorded profile, `command load` private-catalog and registry
fixtures, `X07 root help bytes`, RA01 root help and orchestrator verb labels); (b) the new EX-7 activate messages
flagged by `X07 command sources add no diagnostic outside the closed table`; (c) DD8 stdlib growth from P1 (provenance
originals 12 vs 10, interpretation goldens, effective provenance); (d) unrelated to P3 (`ledger format initializes exact
metadata` cannot read its expected-schema evidence file; three timeouts under `-j8`). Found, not fixed: a load of a
workspace with exactly one job fails `internal` in every format, because `acceptance.lua` passes the already-typed
single-entry `bundle.jobs` list to `sorted()`, which retags a list of at most one item (`already_typed`); the D94 test
uses a two-job workspace for that reason.

### P4 draft decisions (init cutover and format3 read compatibility)

- DD35 (storage format3 read variant): `ledger_format` gains `v3`. The storage header and `native::verify` accept
  user_version 3 with a reduced check: the format3 table set (24 tables, including the seven execution tables named in
  the check), event versions 1 and 2, record codecs 1 to 3, `integrity_check`, `foreign_key_check`, the singleton rows
  and the `ledger_meta` metadata with `format=3`. A file marked 3 without the format3 tables stays
  `unsupported_version` ("Unsupported ledger format version"), so forged-marker guards keep their code. It skips
  `validate_records` and `verify_history`, because format3 holds codec-3 request records and execution value codecs
  that the storage record codec cannot decode, and the format3 history rules live in the execution module. Storage
  cannot import that module (the `planar_ledger_execution_schema` target depends on storage, and the generated storage
  clones have fixed dependency lists), so the command services for `history` and `export` in `invocation.cpp` run
  `verify_execution_ledger` before reading a format3 ledger. Writable access to format3 through storage
  (`open_existing(..., read_write)`, `begin_write`, record writers, format1/format2 acceptance) is refused as
  `unsupported_version`, the same bytes as before the cutover.
- DD36 (format3 events in storage): storage validates the row shape, the 14 event types, version 1 only for
  `ledger_initialized` at sequence 1 and 2 otherwise, and the typed `job`, `role`, `generation_id`, `run_id` columns
  (carried as new trailing fields of `event_row`). Payload version 2 is decoded by the history projection through the
  execution codec (`planar_history_read` now depends on `planar_history_execution_codec`).
- DD37 (format3 `--at-event` reading of P2-5): `active_revision` and `active_job` come from the event's own payload
  (`after`; `ledger_initialized` is none and empty). `activation` is the latest `job_activations` row with `event_seq`
  at most the event, even after D94 cleared the job (literal P2-5). A role's `run` is reported only while its last
  after-state is `running`: the run whose `claimed_seq` is the role's last transition into `running`, with that run's
  generation ordinal. `runs` are exactly the runs of the running roles, sorted by id. The replay is a typed storage
  read (`read_execution_state_at`) with no Lua.
- DD38 (format3 `--request-id`): the request row is read without its body (`read_execution_request`); `load` is
  checked with the existing load result codec, `activate` with `activate_v1`, and `claim`/`takeover` with `claim_v1`
  and returned as saved, without packet and manifest assembly, which R15-D adds. The data stays
  `{selector:"request",result}`.
- DD39 (init diagnostics): `initialize_format3` failures map onto the existing `format_error` categories and are
  rendered by the unchanged `ledger_error`, so every init diagnostic keeps its bytes (an invalid workspace stays
  `schema` "Invalid ledger arguments"; `execution_error_value` would have said `internal`).
- DD40 (load prelude after the cutover): the format3 load prelude recognizes a format3 ledger by `open_existing`
  succeeding with `format == v3` (it used to rely on the open refusing it). The with-profile load path refuses a format3
  ledger as `unsupported_version` before any request lookup, as before.
- DD41 (outside the file list): `src/engine/ledger/format.{cppm,cpp}` gained the two `format_file` forwarders the history
  projection needs, and `src/engine/CMakeLists.txt` gained the history-read codec edge and two test-only link edges
  (history read tests to the format3 initializer, the non-fault export tests to the execution ledger API). The
  configure-time source generators accepted the edited storage and format sources unchanged.

## Round result (draft finisher, 2026-11-08)

Implemented across P1 to P4: `reconcile.lua` and `dispatch.lua` (EX-4/EX-5 reconcile, `work_exec_v1`,
`role_structure_v1`, dispatch packet and `dispatch_manifest_v1`); native execution services under
`src/engine/execution/` (`snapshot_value`, `change_value`, `provenance_input`) and the execution ledger staging API;
format3 load reconciliation in the load service and `activate` end to end (`handlers/activate/`); init cutover to
format3 with the storage `v3` read variant, format3 history (page, revision, request, `--at-event`) and export.

Build: `cmake --preset debug` then `cmake --build build/rewrite-debug --target planar_tests`, exit 0 (1369 steps,
incremental on the lane's existing tree, no compile or link error to fix in the finisher pass).

New tests: `ctest --test-dir build/rewrite-debug -R 'R15-C|format3|replacement\.(reconcile|dispatch|execution|provenance
input) |activate' -j6` matched 221 tests, 221 passed. The match covers the P1 reconcile/dispatch suites, the P2
`execution_change_value`, `execution_provenance_input` and `execution_snapshot_value` suites and the ledger execution
additions, the P3 `load_format3` and `activate` suites and the P4 `[format3]` bootstrap, history-read, storage and
export cases (plus pre-existing tests whose names contain those words).

Existing tests of the touched modules (1100 tests from `planar_runtime_tests`, `planar_command_tests`, the ledger
storage/execution/execution-schema/format/acceptance suites, history read/effective catalog/effective
provenance/provenance/interpretation/execution codec suites, source export and export fault suites, and the three
execution suites) run through `ctest --tests-from-file -j6`: 1026 passed, 74 failed. The full suite was not run.

Failing tests, by cause (all expected draft breakage, none repaired):

- Catalog grew to six production rows / options variant to six alternatives (P3): `effective catalog count and
  canonical byte ceilings`, `... external descriptors and reserved namespaces`, `... help and grammar bounds`, `... history
  id capability matrix`, `... roundtrip and immutable aliases`, `... static read capability signatures`; C4 A (9 cases:
  `Lua allocation exhaustion ...`, `absent scalar defaults ...`, `closed context exposes no service`, `lifecycle refuses
  ...`, `maximum summary and UTF8 boundaries ...`, `original command index ...`, `rejects malformed return arity ...`,
  `rejects public wrapper unknown code ...`, `semantic text has no standalone quota ...`); C4 B3 (3: `C01 history missing
  known services ...`, `C01 missing raw service lookup ...`, `caught missing capability calls ...`); C4 D4 (2: `order once
  arguments ...`, `role projection refuses mismatched input ...`); RA01-B1 (10: `command options variant stays closed at
  five members`, `default embedded catalog stays the five HEAD rows`, the five `profile registering a reserved ...`/
  `reserved command ...`/`reserved pre check ...` cases, `authoring root help inserts the six RA-1 lines ...`, `default
  invoke and production binary refuse all six authoring verbs as usage`); RA01-B2 `orchestrator labels every catalog
  verb ...`; X05 A3 (10 cases, C01 to C04 and the base set); `X07 root help bytes are unchanged and omit with-profile`;
  `command load ...` (11 private-catalog/registry cases, 739-749) and `command registry seals mandatory embedded families
  without services`; recorded profile (`recorded effective history selects wider fixed tree profile ...`, `recorded
  profile history reconstructs effective v2 ...`, `recorded profile saved accepted and noop replay ...`, `recorded profile
  unknown interpretation preserves raw export ...`).
- New EX-7 activate diagnostics flagged by the closed-table guard: `X07 command sources add no diagnostic outside the
  closed table`.
- DD8 stdlib growth (P1): `provenance generated originals and legacy assembled chunk match exact authored bytes`,
  `interpretation complete quota outputs match reviewed JSON and canonical goldens`, the nine `effective provenance ...`
  cases, `X06 D C03 a recovered allocation refusal ...` and `X08 C01 role preparation outputs-off boundary pins the
  request-record conversion charge` (Lua memory pins).
- P4 behavior change (DD35): `storage header check refuses user version 3 on its own` (a format3 header is now a read
  variant, not a refusal on its own).
- Not caused by this round: `ledger format initializes exact metadata schema and Glaze event` (cannot read its
  expected-schema evidence file).

Gaps (in addition to the P1 to P4 decision notes): the one-job workspace load failure in `acceptance.lua`
(`sorted()` on an already-typed single-entry list, found in P3) is not fixed; EX-6 binding under a profile compares the
embedded stdlib digest (DD32); the audited `tests/native/` composition generators are not extended (DD27, DD28); the
Lua output is not cross-checked against the native `encode_work_exec_v1`/`encode_dispatch_manifest_v1` codecs; M21,
M22, M40, M41 are not produced (DD16); the dispatch packet body sections keep render prose (DD11); `--request-id` for
claim/takeover returns the saved `claim_v1` without packet assembly (DD38, R15-D). Draft decisions DD9 to DD41 above
stand as recorded; the finisher added none.

## Left for the fixing rounds

- Full gate (Debug, RelWithDebInfo, Linux Docker, format, Doxygen, no-change build), rewrite-check gate.
- Real-process races and fault variants (EX-10): load vs claim overlap, concurrent activate, crash before COMMIT,
  disk full, lost response, live-writer sidecar excusal (R15-B L3, L6, L13), owned by R15-E and the fixing rounds.
- Mutation probes, including the permissive mutants of execution-design R15 section 4 and the R15-B survivors (A3).
- Reviews; R15-B review findings R2 to R4 and advisories A2 to A6 (A5 to the operator before the R15-D fixing round).
- Evidence directories and acceptance-matrix rows A09, A10, A20, A22, A23, A26, A33.
- Any existing test broken by the cutover or by DD8 that the draft packages did not repair.
