# V5-02-R15-D (DRAFT): status, preview, claim, takeover, history --event, orchestrator live form

Draft mode (operator decision 2026-11-08): code- and feature-complete first, broken allowed. Lane worktree
`planar-lane-a`, branch `draft/lane-a`, on top of the R15-C draft (`8b60fdf2`, itself on base `1338637c`, the R15-B
draft base with the C01 R1 fix). Lane A continues with R15-E and R16 after this round.

Kept: Debug compile (`cmake --preset debug`, `cmake --build build/rewrite-debug --target planar_tests`), meaningful
colocated `*.t.cpp` tests (plain `//` header, per-test temporary roots, every new TEST_CASE name starts with `R15-D `
so `ctest --test-dir build/rewrite-debug -R 'R15-D'` matches them; the matched count must be nonzero), running the
tests each package adds, and an honest list of what fails. Dropped until the fixing rounds: full gate, rewrite-check,
Linux/Docker, mutation probes, reviews, evidence directories. R15-E (races, faults, restart) is deferred to the fixing
rounds.

## Scope

From [execution-design.md](../execution-design.md) R15 section 5 (row R15-D), the approved
[execution contract](../execution-contract.md) part 2 (P2-1 to P2-6, with the P2-7 constants) and the items the
R15-B and R15-C records hand to R15-D:

- CLI verbs `status` (bare, `--role`, `--request-id`, `--workspace`/`--registry` drafts), `preview`, `claim` and
  `takeover` end to end: CLI11 families, native intents, ABI rows, admission rows, services, Lua families, responses
  `status_v1`, `preview_v1`, `claim_v1`, refusal order (1) to (10) of P2-1 and the exempt-read order.
- Driver provenance input wired to `claim` and `takeover` (P2-4; `engine/execution/provenance_input` from R15-C DD6).
- `history --event` (P2-5) on format1, format2 and format3, and format3 `--request-id` assembly of `claim`/`takeover`
  results with `packet` and `manifest` (P2-5, replacing R15-C DD38's saved-only form), shared with
  `status --request-id`.
- Orchestrator live form `orchestrator --ledger` (P2-6) in the authoring bundle, the `orchestrator_state` section,
  the `live` context key and the action labels.
- Catalog and namespace closure of P2-7: ten production rows, sixteen authoring rows, with-profile family list of ten,
  reserved section IDs `run` and `orchestrator_state` (D135), the five command and handler names leave the external
  namespace (capacity 59 to 54), root help bytes (production and authoring).
- Carried: D149 takeover site pin (R15-C DD5), DD19 `generation_current` spelling at claim/takeover, DD20 facts and
  lineage, R15-B review advisory A5 (Q-PREC option A consequence; see DD-D9).

Out of scope: R15-E (real-process races, SIGKILL and fault variants, A09/A10/A20/A22/A23/A26/A33 integration, the
F3 to F7 test-only items), `preview --since` and every R16 to R19 verb (they stay reserved names, `usage`), R19 (Q17 is
owed by the operator).

## Module placement (EX-9)

| Destination | Content |
| --- | --- |
| `src/cmd/planar/handlers/{status,preview,claim,takeover}/command.{cppm,cpp}` (new) | CLI11 families; modules `planar.cmd.execution.<verb>` outside the generated handler prefix (R15-C DD27) |
| `src/cmd/planar/bootstrap.cpp`, `CMakeLists.txt` | attach, dispatch, with-profile dispatch, root help bytes, PUBLIC link items through `planar_command_envelope` |
| `src/cmd/planar/invocation.{cppm,cpp}` | native services and preludes: format refusal, early replay, EX-6 binding, `read_request`, `capture_drafts`, `read_provenance`, `commit_execution` for claim/takeover, response output and post-commit fallbacks |
| `src/engine/runtime/runtime.{cppm,cpp}` | four intents (variant grows to ten alternatives), ABI rows with capability lists, option signatures, service plumbing and call counts, response validation, admission and with-profile rows, reserved section IDs and names |
| `src/engine/execution/drafts.{cppm,cpp,t.cpp}` (new) | compare a captured declaration closure with the active revision's source tree (logical paths changed, absent, added) |
| `src/engine/execution/run_result.{cppm,cpp,t.cpp}` (new) | native assembly of `claim_v1` (packet and manifest from records) for the response, `status --request-id` and `history --request-id` |
| `src/engine/ledger/execution.{cppm,cpp,t.cpp}` | additive read-only helpers: named-run lookup `{job,role,state}` for takeover step (8), request/run/dispatch/record reads for result assembly and `--event` expansion (never selecting `runs.secret`) |
| `src/engine/history/read.{cppm,cpp,t.cpp}` | selector `event`, format3 event expansion, format3 claim/takeover request assembly |
| `src/stdlib/planar/dispatch.lua` | status role list and `role_detail` (`changed`, `explain`), preview dispatch context |
| `src/stdlib/planar/reconcile.lua` | claim and takeover `execution_change_v1` builders (expected six members, vector, transitions) and the step (6) to (8) refusals Lua can decide |
| `src/stdlib/planar/commands.lua`, `bootstrap.lua` | four family registrations and descriptors, history `event` selector, any new fixed export |
| `src/stdlib/planar/orchestrator.lua` | live context key, `orchestrator_state` section, action labels |
| `src/cmd/planar/handlers/orchestrator/command.{cppm,cpp}`, `authoring.{cppm,cpp}` | four optional orchestrator options with the native one-form rule; ledger input slot |

## Packages (sequential)

Each package compiles, adds its own tests with names starting `R15-D `, runs them with a nonzero matched count, and
reports what fails. Coders touching `src/engine/runtime/` follow the standing escalation (`orchestration.md`). Packages
run in order because each extends `runtime.cpp`, `invocation.cpp`, `bootstrap.cpp` and `commands.lua`.

### P1 - Read verbs: `status` and `preview`

Files: `src/engine/runtime/runtime.{cppm,cpp}`, `src/cmd/planar/invocation.{cppm,cpp}`,
`src/cmd/planar/handlers/status/command.{cppm,cpp}`, `src/cmd/planar/handlers/preview/command.{cppm,cpp}` (new),
`src/cmd/planar/bootstrap.cpp`, `src/cmd/planar/CMakeLists.txt`, `src/engine/execution/drafts.{cppm,cpp,t.cpp}` (new),
`src/engine/CMakeLists.txt`, `src/stdlib/planar/dispatch.lua`, `src/stdlib/planar/commands.lua`,
`src/stdlib/planar/bootstrap.lua`; tests `src/cmd/planar/status.t.cpp`, `src/cmd/planar/preview.t.cpp` (new),
`src/engine/runtime/dispatch.t.cpp` (added cases), `src/engine/execution/drafts.t.cpp`.

- Intents `command_status_options` (`ledger`, optional `role`, `request_id`, `workspace`, `registry`) and
  `command_preview_options` (`ledger`, `role`); ABI rows `shipped.status.v1`/`status_v1`/`execution_read_v1`,
  `draft_capture_v1` and `shipped.preview.v1`/`preview_v1`/`execution_read_v1` (DD-D1); option signatures of P2-7;
  CLI11 rules: `--role` excludes `--request-id`, `--workspace` and `--registry` both or neither (`usage`).
- Services: status `read_execution`, then `read_request` with `--request-id`, then `open_workspace` and
  `capture_drafts` with `--workspace`/`--registry`; preview `read_execution`. Order of P2-1 refusals: format1/format2
  `unsupported_ledger_version` for every status form and preview; EX-6 binding only for `status --role` and preview
  (DD-D2); `no_active_job` for preview and `status --role` without an active job; `reference` per D152.
- `status_v1`: roles from `snapshot.roles` of the active job with `kind` from the structure, `run` `none` or
  `{id,generation}`; `role_detail` from the acknowledged W (`snapshot.roles[].run.dispatch.work`) and the current W
  (`work_exec`), `changed`, `explain` (`output_vector` `[]`, `prerequisite` the non-available prerequisites), `reasons`
  (last 16 transitions, oldest first; DD-D4), D152 retired form; `drafts` via `capture_drafts` (native compare in
  `engine/execution/drafts`, Lua `interpret_bundle` decides `invalid`); `request` via `read_request` (load and activate
  results; claim/takeover gets its assembled form in P3, saved form until then) with `disposition` for claim/takeover.
- `preview_v1`: `render_dispatch` with `run:{kind:"preview"}`, current generation ordinal, no write, no token.
- Catalog: production eight rows, authoring fourteen; with-profile family list gains `status` and `preview`; root help
  gains the two lines after the activate line (P2-7 order).
- Tests: every status form on a format3 ledger created with `planar init` + `load` + `activate`; format2 refusal of each
  form; status `--role` on ready/blocked/retired-undeclared roles; drafts `unchanged`/`changed`/`invalid`; unknown
  request `request_not_found`; preview bytes stable across fresh processes, preview authority and `run request:
  (preview)`, preview of an undeclared retired role is `reference`; nothing written by either read (ledger bytes equal).

### P2 - Mutation verbs: `claim` and `takeover`

Files: `src/engine/runtime/runtime.{cppm,cpp}`, `src/cmd/planar/invocation.{cppm,cpp}`,
`src/cmd/planar/handlers/claim/command.{cppm,cpp}`, `src/cmd/planar/handlers/takeover/command.{cppm,cpp}` (new),
`src/cmd/planar/bootstrap.cpp`, `src/cmd/planar/CMakeLists.txt`, `src/engine/ledger/execution.{cppm,cpp,t.cpp}`
(named-run lookup), `src/engine/execution/run_result.{cppm,cpp,t.cpp}` (new), `src/engine/CMakeLists.txt`,
`src/stdlib/planar/reconcile.lua`, `src/stdlib/planar/commands.lua`, `src/stdlib/planar/bootstrap.lua`; tests
`src/cmd/planar/claim.t.cpp`, `src/cmd/planar/takeover.t.cpp` (new), `src/engine/runtime/reconcile.t.cpp` (added
cases), `src/engine/execution/run_result.t.cpp`.

- Intents `command_claim_options` and `command_takeover_options` (`run_id` canonical decimal, optional `provenance`
  opaque slot); ABI rows `shipped.claim.v1`/`shipped.takeover.v1`, response kind `claim_v1`, capabilities
  `execution_commit_v1`, `provenance_read_v1`; services `read_execution`, `read_provenance` only with `--provenance`,
  `commit_execution`.
- Native prelude: format refusal, early replay through `lookup_execution_request` with `build_request_v3` (parameter map
  with `provenance_path` only when supplied; replay never reads the file), then EX-6 binding inside `read_execution`
  (as R15-C DD31). `read_provenance` through `engine/execution/provenance_input` after `read_execution`.
- Lua (`reconcile.lua`): step (6) `no_active_job`, step (7) `Unknown selected role`, step (8) claim `claim_conflict`
  `{state}` for `blocked`/`running`/`done`/`retired`; takeover proceeds only when the role is `running` and its active
  run is `--run-id`, otherwise DD-D5. Then `render_dispatch` with `run:{request,kind[,previous]}` and the change:
  facts `{role,provenance?}` / `{role,previous_run,provenance?}`, `expected` exactly the six-member set with one
  `fact_version` per role of the active job and `generation_current` in the DD19 spelling, the complete next vector,
  ordinal-1 target transition (`running`, reason `claimed` or `taken_over`, DD-D7), packet.
- Native commit: `decode_execution_change` (DD20: takeover generation and `fenced_runs` built natively), token secret
  from the OS random source, `ledger::commit_execution`; `claim_v1` assembled natively by `engine/execution/run_result`
  (`run`, `dispatch` with `xxh3_128:` digests and packet byte length, `packet` P, `manifest` M, `previous_run` for
  takeover); the saved result column holds the value without `packet` and `manifest`. Post-commit fallbacks are the EX-7
  `Claim may be complete ...` / `Takeover may be complete ...` pairs.
- Catalog: production ten rows, authoring sixteen; with-profile list ten; root help gains the claim and takeover lines.
- Tests: claim of a ready role then status shows `running` with `{id,generation}` and no token; same-ID replay prints
  identical bytes including the token; unequal body is `request_conflict`; claim of `blocked`/`running`/`done` role
  `claim_conflict` with `{state}`; claims of two different roles both commit; claim with `--provenance` (valid,
  oversize `provenance_bytes`, bad grammar `schema` field `provenance`, a refusal at step (8) wins and the file is not
  read); takeover of the active run (old run fenced, new generation, `previous_run`, W digest unchanged), takeover with
  a stale run id `fenced` `{run_state}`, D149 pin: an unknown `--run-id` is `state_conflict` and a same-ID retry still
  refuses; packet and manifest contain no token; packet bytes equal across a fresh-process preview of the same
  generation except the authority and run sections.

### P3 - History `--event` and format3 request assembly

Files: `src/cmd/planar/handlers/history/command.{cppm,cpp}`, `src/engine/runtime/runtime.{cppm,cpp}` (selector `event`,
history option signature with `--event`), `src/stdlib/planar/commands.lua` (selector `event` with `request_id:""`,
`limit:50`), `src/engine/history/read.{cppm,cpp,t.cpp}`, `src/engine/ledger/execution.{cppm,cpp}` (read-only record
helpers if `read.cpp` cannot reach them), `src/engine/execution/run_result.{cppm,cpp}` (reuse),
`src/cmd/planar/invocation.cpp` (history and `status --request-id` services use the assembled result),
`src/cmd/planar/bootstrap.cpp` (history continuation line `  or --revision ID | --request-id ID | --at-event ID |
--event ID`), `src/engine/CMakeLists.txt`; tests `src/engine/history/read.t.cpp` (added cases),
`src/cmd/planar/history_event.t.cpp` (new).

- `--event ID` excludes `--revision`, `--request-id`, `--at-event`, `--after-event`, `--limit` (`usage`); format1,
  format2 and format3; unknown event `event_not_found`. Data `{selector:"event",event,transitions,execution?}` with the
  format3 summary keys `job`, `role`, `generation_id`, `run_id`; `execution` for `job_activated`, `run_claimed`,
  `run_taken_over` with the `provenance` `native`/`driver` split and the all-unknown driver document when no record.
- Format3 `--request-id` for `claim`/`takeover`: the `claim_v1` value with `run.token`, `packet` and `manifest`
  assembled natively; `status --request-id` uses the same assembly. Output above 16777216 bytes is `resource_limit`
  `output_bytes` with empty stdout. No Lua; the reader never selects `runs.secret`.
- Tests: `--event` on each format; `run_claimed` and `run_taken_over` expansions (packet bytes equal the claim response
  packet, manifest and W decoded, provenance native keys, driver record and the all-unknown form); `job_activated`;
  an event without transitions; every exclusion is `usage`; token absent from every `--event` and page output and
  present in `--request-id` of a claim.

### P4 - Orchestrator live form and catalog closure

Files: `src/cmd/planar/handlers/orchestrator/command.{cppm,cpp}`, `src/cmd/planar/authoring.{cppm,cpp}`,
`src/engine/runtime/runtime.{cppm,cpp}` (orchestrator capability `execution_read_v1` beside
`orchestrator_capture_v1`, ledger form service `read_execution`, reserved section IDs `run` and `orchestrator_state`,
reserved command and handler names, external capacity 54), `src/cmd/planar/invocation.cpp` (ledger-form services),
`src/stdlib/planar/orchestrator.lua`, `src/stdlib/planar/commands.lua` (orchestrator descriptor: four optional
options), `src/cmd/planar/bootstrap.cpp` (authoring root help: the six RA01 lines after the five R15 lines and
`  or --ledger PATH` after the orchestrator line); tests `src/cmd/planar/authoring_orchestrator_live.t.cpp` (new),
`src/engine/runtime/external_registration.t.cpp` (added cases for the reserved IDs and names).

- Exactly one form: `--workspace`, `--registry`, `--job` together, or `--ledger` alone; anything else `usage`.
  Ledger form: format1/format2 `unsupported_ledger_version`, `no_active_job`, otherwise `orchestrator_context_v1` from
  the active revision's recorded interpretation for the active job plus `live` `{ledger,revision,last_event,activation,
  roles,runs,blockers}` (`blockers` `[]`), and the mandatory `orchestrator_state` section after `orchestrator_actions`
  with the legal next action per role state. Database-free form unchanged byte for byte.
- Action labels: `available` exactly when the verb is in the sealed catalog; `activate`, `status`, `preview`, `claim`,
  `takeover` available; the future list `refresh`, `checkpoint`, `release`, `recover`, `check`, `check-recover`,
  `verdict`.
- Reserved IDs and names apply in every build, production included: an external projection with section `run` or
  `orchestrator_state`, or an external command or handler named after one of the five R15 verbs, is refused with the
  existing reserved-name diagnostics.
- Tests: ledger form on a format3 ledger with an active job and one running role (live roles and runs, no token),
  without an active job, on a format2 ledger, mixed-form `usage`; database-free form bytes unchanged; reserved ID and
  name refusals; production binary still has no `orchestrator` verb.
- Finisher step of P4: run every `R15-D` test, then the unit tests of the touched modules once
  (`planar_runtime_tests`, `planar_command_tests`, ledger execution, history read, execution suites) and list every
  failure with its cause in this note; fixing them all is not required in draft mode.

## Draft decisions

Taken under the draft-mode instruction (choose the option most consistent with the contract and design, record it,
continue); none is an operator decision.

- DD-D1 (capability lists): the runtime ABI row type holds a single capability today. It becomes an ordered list so
  that `status` (`execution_read_v1`, `draft_capture_v1`), `claim` and `takeover` (`execution_commit_v1`,
  `provenance_read_v1`) and the authoring `orchestrator` (`orchestrator_capture_v1`, `execution_read_v1`) carry the
  P2-7 capabilities; descriptor `capabilities` arrays in `commands.lua` list the same values in the same order.
- DD-D2 (binding site of the reads): as R15-C DD31 for activate, the EX-6 binding check of `preview` and
  `status --role` runs inside the `read_execution` service on the snapshot Lua receives; bare `status`,
  `status --request-id`, `status` with drafts and `orchestrator --ledger` skip it (exempt reads, D108). The format
  refusal (step 2) runs in the native prelude before any service for all of them.
- DD-D3 (dispatch manifest record ids): `dispatch_manifest_v1.inputs.interpretation` needs the record id of
  `revisions.interpretation_id`, which the exact P2-1 `read_execution` shape does not carry (R15-C DD10 left the source
  to "the handler"). The snapshot value's `revision` gains one internal key `interpretation_id` (decimal string),
  beside DD17's `source:{id,digest}`; it is not driver-visible and no output byte changes. A fixing round may replace it
  with native completion of `inputs` if review prefers the exact shape.
- DD-D4 (`reasons` source): `role_detail.reasons` (last 16 `transitions` rows, oldest first) is not in the P2-1
  snapshot shape. Native computes it from the `transitions` table in the same read transaction and passes it to Lua
  under the same internal extension rule as DD-D3 (`snapshot.roles[].reasons`, present only for `status --role`'s
  target role); Lua copies it.
- DD-D5 (takeover step (8) for a run that is not current): Lua sees only active runs, so it cannot name the state of
  a fenced or unknown run. When the role is not `running` or its active run is not `--run-id`, the Lua takeover handler
  returns `fenced` with the EX-7 message and no `run_state`; native then resolves the named run with a read-only ledger
  lookup on the same snapshot connection: no run of that job and role is `state_conflict` (D149 site, amendment A-13),
  otherwise `fenced` with `{run_state}` of the named run. The `claim_conflict` code is never produced for takeover.
- DD-D6 (status drafts compare): `capture_drafts` captures the declaration closure through the existing load capture
  route and compares each logical portable path's bytes digest with the active revision's source tree records;
  `changed` lists differing, absent and added paths sorted by unsigned bytes. A captured bundle that fails
  `interpret_bundle` is `invalid` with the typed error and the same `changed` list; capture-class failures fail the
  command (P2-1 results list).
- DD-D7 (transition reasons of claim and takeover): the ordinal-1 target transition has reason `claimed` (claim) or
  `taken_over` (takeover), with before and after states taken from the State machines role table rows for those
  operations as the R15-B ledger validates them (the coder reads the rows, not this note); the takeover generation and
  `fenced_runs` are built natively (DD20). Derived transitions of other roles are not produced in R15-D, because a
  claim moves no fact that another role's derived state reads (claims bump no `fact_version`).
- DD-D8 (saved vs printed `claim_v1`): the saved `result` column holds `claim_v1` without `packet` and `manifest` and
  with `run.token` (P2-5); stdout, replay, `status --request-id` and `history --request-id` all print the native
  assembly from `engine/execution/run_result`, so a replay prints bytes identical to the first response.
- DD-D9 (R15-B advisory A5, delegated): Q-PREC option A stays. Under the lock a superseded takeover fails its
  `fact_version` member first and is `state_conflict`; the lock-time `fenced{run_state}` mapping of P2-1 is unreachable
  (L15) and `generation_current` stays an equivalent-mutant site (L21). R15-D maps snapshot-time refusals per P2-1 and
  takes the ledger's lock-time categories as they are. A5 still goes to the operator before the R15-D fixing round.
- DD-D10 (Lua diagnostics): the EX-7 messages Lua emits (`claim_conflict`, `no_active_job`, `fenced`) are copied
  byte-exact from the Messages table. The X07 closed-table guard test that already flags activate (R15-C) will also
  flag these; extending the guard's allowed set is left to the fixing rounds.
- DD-D11 (catalog growth per package): as R15-C DD4, rows join one package at a time (P1 eight/fourteen, P2
  ten/sixteen). Tests that pin catalog counts, option variants or root help bytes are updated only when a package owns
  them; otherwise they are listed as failing.
- DD-D12 (orchestrator option presence): the orchestrator descriptor's four options become `required:false` and the
  one-form rule is native in the handler (P2-6), so the authoring catalog row changes; RA01 tests that pin the
  three-required signature are updated by P4 or listed as failing.

## Left for the fixing rounds

- Full gate (Debug, RelWithDebInfo, Linux Docker, format, Doxygen, no-change build), rewrite-check gate.
- R15-E: real-process races (claim vs claim, load vs claim, takeover vs claim), pause-point and SIGKILL variants, disk
  full, lost response, restart byte equality of dispatched packets (A23), interpretation-binding variant binary,
  acceptance rows A09, A10, A20, A22, A23, A26, A33, and the F3 to F7 test-only items.
- Mutation probes, including the permissive mutants of execution-design R15 section 4 (lost-update guard,
  fact_version always passes) and the R15-B survivors.
- Reviews; R15-B review findings R2 to R4 and advisories A1 to A6 (A5 to the operator before the R15-D fixing round).
- The X07 closed-table guard, the audited `tests/native/` composition generators (R15-C DD27, DD28), boundary and
  Lua-memory pins moved by stdlib growth (R15-C DD8), and every existing test broken by catalog growth that the draft
  packages did not repair.
- Evidence directories and the acceptance-matrix rows.

## Resume implementation, 2026-11-08

Resumed the existing P1 edits and added status/preview, claim/takeover and R16 token
mutation/recovery command wiring. Runtime intents, CLI11 handlers, catalog rows, native
services and trusted Lua handlers are present. Native R16/history is committed separately
as `634be228`. Draft mode remains in force: no review, mutation probes, Linux or full gate.

Development checks: `cmake --build build/rewrite-debug --target planar -j 3` succeeded.
A temporary-root CLI workflow completed init, load, activate, bare status, preview, claim,
recover and release. The same development run exposed a repeated Lua array-tag operation
in status role detail and raw/prefixed digest mismatches in refresh and preview since;
these were corrected after the run and require the next integrated development build.
The pre-existing one-job acceptance bug was corrected by copying normalized lists before
calling the metered sorting helper. No installed binary or operator database was used.

Remaining implementation work moves to the integrated checkout: recover output capture
(the root options presently have no capture implementation); checkpoint section binding
for notes supplied during refresh; R17 evidence-dependent release reconciliation; live
orchestrator service wiring by the sibling command agent. These are implementation gaps,
not review-only debt. Acquisition/recovery profile framing and capture admission need
integrated attention. The command draft compiles but is not claimed validated.
