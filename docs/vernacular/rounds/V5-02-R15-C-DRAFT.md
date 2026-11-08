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

## Left for the fixing rounds

- Full gate (Debug, RelWithDebInfo, Linux Docker, format, Doxygen, no-change build), rewrite-check gate.
- Real-process races and fault variants (EX-10): load vs claim overlap, concurrent activate, crash before COMMIT,
  disk full, lost response, live-writer sidecar excusal (R15-B L3, L6, L13), owned by R15-E and the fixing rounds.
- Mutation probes, including the permissive mutants of execution-design R15 section 4 and the R15-B survivors (A3).
- Reviews; R15-B review findings R2 to R4 and advisories A2 to A6 (A5 to the operator before the R15-D fixing round).
- Evidence directories and acceptance-matrix rows A09, A10, A20, A22, A23, A26, A33.
- Any existing test broken by the cutover or by DD8 that the draft packages did not repair.
