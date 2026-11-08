# V5-02-R18-B-core (DRAFT): verdict commit, durable blockers, Lua review and accepted derivation

Status: draft-mode plan (operator decision 2026-11-08). Feature- and code-complete draft; compile and run the tests
this round adds. No full gate, no rewrite-check gate, no Linux/Docker run, no mutation probes, no review loop, no
evidence directory. Those belong to the fixing rounds (see "Left for fixing rounds").

Lane: worktree `/Users/mn/projects/github/rdrsss/planar-lane-b`, branch `draft/lane-b`. Base `1338637c` (R15-B draft
base with the C01 R1 fix); the lane already carries the R17-B draft (`bdea472a`, `src/engine/process/`), which this
round does not touch. No CLI: the `verdict` handler, ABI row, `commands.lua` registration, help, catalog, `status` /
`orchestrator` output and the R16 token refusal order are lane A's later R18-B-cli slice.

## Authority and inputs

- Binding interface: `execution-contract-r18.md` (unreviewed draft, clauses R18-T, R18-1 to R18-8, the native and Lua
  interface tables, DR18-1 to DR18-10). It was committed in `c7efd3ba`, which is **not** in this lane's base; coders
  read it, and the R16/R17 drafts it cites, read-only from the main checkout:
  `/Users/mn/projects/github/rdrsss/planar/docs/vernacular/execution-contract-r18.md`, `...-r17.md` (R17-7 snapshot
  keys `completions`, `evidence`; `checks.lua` functions), `...-r16.md` (token rules, `recovery_records`). They are not
  copied into the lane.
- Approved: `execution-contract.md` Parts 1 and 2 (EX-1 schema byte-exact, EX-3 commit protocol and precondition
  vocabulary, EX-5 lineage, EX-7 codes, EX-8 bounds; P2-3 W recipe, P2-4 file capture table, Part 2 section on
  `read_execution` / `execution_snapshot_v1` key lists, D137, D151).
- `execution-design.md` R18 sections 1 to 6 and EX-9 placement (`ledger/execution` verdict, repair, blockers;
  `engine/execution/` verdict service; stdlib `review.lua`); decisions E9 A, E17 A, Q16 A.
- R15-B records (`V5-02-R15-B.md` L1 to L23, `V5-02-R15-B-review.md`): nothing is assigned to R18. The R15-C items
  (Lua `reconcile.lua` / `dispatch.lua`, `work_exec_v1` construction, load reconciliation, `dispatch_context_v1`,
  no-row `role_state` members, D149 pins), R15-D (CLI) and R15-E (races, faults, SIGKILL) stay with those rounds.
  R18-B-core inherits L3/L6 (no real-process races or crash tests at native level) unchanged.

## Scope

In: R18-T (rows, derived values, invariants 21 to 27), R18-2 native steps 1 to 5 and the `verdict_v1` codec, R18-3
applicability check natively, R18-4 commit (one EX-3 transaction, replay, transitions M37/M39/M42/M43/M44), R18-5
admission cases and blocker open/clear (including the `blockers_cleared` hook on `load` and `activate` changes), the
snapshot `review` key (DR18-8), `verdict_response_v1` saved-result codec, `history --event` expansion for
`verdict_recorded` (native, read-only), and `review.lua` (R18-1 target vector, policy, packet additions as data,
R18-2 steps 5 to 7, R18-3 classify, R18-5 blockers-to-clear, R18-6 acceptance, `status_v1.review` value).

Out (later lanes / integration): CLI and runtime ABI (`read_verdict` service, capability `verdict_read_v1`,
with-profile slot `verdict`); `reconcile.lua` and `dispatch.lua` edits (those files do not exist here: R15-C); the
packet byte rendering inside `dispatch.lua` (review.lua returns the lines as data); `recovery_records.findings`
(`ledger/recovery` is R16); `orchestrator --ledger` guidance lines; A13 fault variants and two-process tests.

## Module placement

| Module | Files | CMake target | New / changed |
| --- | --- | --- | --- |
| `planar.engine.history.execution_codec` | `src/engine/history/execution_codec.{cppm,cpp,t.cpp}` | `planar_history_execution_codec` | add `verdict_v1` codec, `verdict_result_v1` (response) codec, `"verdict"` in `payload_operations`, `verdict_recorded` payload references |
| `planar.engine.execution.verdict` | `src/engine/execution/verdict.{cppm,cpp,t.cpp}` | `planar_execution_verdict` (new) | R18-2 steps 1 to 5 and normalization |
| `planar.engine.ledger.execution` | `src/engine/ledger/execution.{cppm,cpp}` (minimal hooks) + new implementation unit `src/engine/ledger/execution_review.cpp` + tests `src/engine/ledger/execution_review.t.cpp` | `planar_ledger_execution` (SOURCES gains `execution_review.cpp`; a second TESTS file or a new test target, coder's choice) | snapshot `review`, DRAFT-STUB completions read, verdict writer, blockers, invariants 21 to 27 |
| `planar.engine.history.read` | `src/engine/history/read.{cpp,t.cpp}` | existing | `--event` expansion for `verdict_recorded` |
| stdlib `review.lua` | `src/stdlib/planar/review.lua` | not embedded yet (DB-R4) | new |
| Lua harness | `src/cmd/planar/review_lua.t.cpp` | `planar_review_lua_tests` (new, `EXCLUDE_FROM_ALL`, added to `planar_tests`, pattern of `planar_authoring_catalog_tests`) | new |
| reference | `docs/architecture.md` | | one row for `planar.engine.execution.verdict`, one sentence on `execution_review.cpp` |

The R18 code goes into new files wherever the module system allows, so the merge with lanes R16 and R17 (which edit
`execution.cpp`, `execution.cppm` and the codec in parallel) touches few lines: in `execution.cpp` only the
operation dispatch branch, the reserved-table list and the snapshot-read call site change, each marked
`// R18:`. Helpers shared between `execution.cpp` and `execution_review.cpp` are declared **non-exported** in
`execution.cppm` (module linkage), never by moving existing code. New code that stands in for R16/R17 work not
present in this lane carries a `DRAFT-STUB` comment naming the owning round.

Conventions: `namespace planar::engine::ledger` / `planar::engine::execution` / `planar::engine::history`;
`std::expected` boundaries; `out += std::format(...)`; Glaze for all JSON (strict `verdict_v1` parse uses the
existing Glaze adapter pattern of the provenance reader, never a handwritten parser); Doxygen `@file`/`@brief` on
every `.cppm`; tests `*.t.cpp` with plain `//` headers, per-test temporary roots, Catch2 names starting `R18-B ` with
tag `[r18]`. Focused run: `ctest --test-dir build/rewrite-debug -R 'R18-B'`, matched count nonzero. Debug preset
only, built from the lane worktree.

## Packages (sequential)

### P1: codecs and the verdict document

Files: `src/engine/history/execution_codec.{cppm,cpp,t.cpp}`; `src/engine/execution/verdict.{cppm,cpp,t.cpp}`;
`src/engine/CMakeLists.txt` (new `planar_execution_verdict` block); `docs/architecture.md` (row).

- Codec: `codec_verdict_v1 = "verdict_v1"`; `struct verdict_finding {id, severity, work, path?, line?, prescription?,
  text}`, `struct verdict_target {work, output, work_fp}`, `struct verdict_v1 {targets, policy, decision, findings,
  repair, summary, authority}`; `encode_verdict_v1` (canonical_v1 of the normalized document, `version:1`,
  optional keys omitted when absent, `authority` always present) and `decode_verdict_v1` (exact keys, re-validates
  the R18-2 field table). `struct verdict_result_v1` mirroring `verdict_response_v1` of the Verb section with
  `encode_verdict_result_v1` / `decode_verdict_result_v1` (canonical key order, decimal strings, `admitted`
  boolean, no token). Add `"verdict"` to `payload_operations`; `verdict_recorded` payload references exactly
  `{body, run, verdict}`.
- Verdict module: `struct verdict_error {code, message, field, limit, maximum}` with codes `io`, `path_denied`,
  `capture_changed`, `resource_limit`, `schema`, `unsupported_policy` (DB-R1). Exports
  `parse_verdict_bytes(std::string_view captured) -> expected<value_ptr, verdict_error>` (steps 2 to 5 in order:
  `verdict_bytes` 1048576; Glaze grammar → `schema` field `verdict`; `verdict_findings` 256, `verdict_finding_bytes`
  4096, `verdict_summary_bytes` 16384; shape and field table → `schema` with the P2-4 field path),
  `read_verdict_file(const std::filesystem::path&)` (step 1 through the existing no-follow single-file capture used
  for `--provenance`, mapping its refusals with the P2-4 messages, then `parse_verdict_bytes`) and
  `normalize_verdict(const value_ptr&) -> expected<history::verdict_v1, verdict_error>` (step 6 `authority`, absent →
  `reviewer_role_v1`, other → `unsupported_policy`, then the approve-with-`must` / approve-with-`repair` `schema`
  rule of step 7; the `reference` repair-owner rule needs the snapshot and is in P3 natively and P4 in Lua).
- Tests: every check-order step with its code, message and field; boundaries 256/257 findings, 4096/4097 text,
  16384/16385 summary, 1 MiB / +1; duplicate key, unknown key, trailing bytes, null, non-integer `line`, `line`
  without `path`, unsorted or duplicate `targets` / `repair`, bad digest text, finding `work` not in targets;
  `authority` absent/`reviewer_role_v1`/`external`; codec round trips and canonical bytes; result codec round trip.

### P2: ledger read side: snapshot `review`, derivations, invariants 21 to 27

Files: `src/engine/ledger/execution.{cppm,cpp}` (hooks only), `src/engine/ledger/execution_review.cpp` (new unit),
`src/engine/ledger/execution_review.t.cpp`, `src/engine/CMakeLists.txt` (SOURCES/TESTS of `planar_ledger_execution`).

- Remove `verdicts`, `repairs`, `blockers`, `blocker_state` from `load_reserved_empty`; and, as **DRAFT-STUB
  (R17-C)**, `completions` (DB-R2).
- Snapshot additions on `execution_snapshot`: `std::vector<execution_completion> completions` (**DRAFT-STUB R17-C**:
  `{id, job, role, generation, run, dispatch, work_fp, seq, units:[{work, output}]}` read from `completions` and the
  unit list of its observation value record, R17-7 shape, current lineages of the active job only) and
  `execution_review review` = `{count, verdicts, blockers, repairs}` exactly as DR18-8 (verdicts of the active job in
  the current activation plus the latest verdict of the reviewer lineage, each with decoded body; open blockers
  only; current-lineage repair revisions). Both count toward `execution_state_bytes` (extend the D24 measure in the
  one shared definition).
- Non-exported helpers for P3: `review_count(db, job, activation)`, `repair_revision(db, job, role, lineage)`,
  `verdict_activation(db, verdict)`, `open_blockers(db, job)`, `lineage_generations(...)` (reuse the existing EX-5
  lineage code by declaring it non-exported in the interface; do not duplicate it).
- `verify_execution_ledger` / snapshot read: invariants 21 to 27 (27 uses DB-R6). Blocker / blocker_state pairing,
  `cleared_seq > seq`, one open per (job, kind).
- Tests: empty-review snapshot on a fresh format3 ledger (existing fixtures from `execution.t.cpp`); each invariant
  21 to 27 refused as `ledger_corrupt` on a ledger whose rows are hand-inserted by the test through `ledger.sqlite`
  (the same corruption style as R15-B V-tests; insert a consistent row set first and show it is accepted, then
  corrupt one column); schema bytes unchanged (existing exact-SQL test still passes).

### P3: the `verdict` commit, blockers and history expansion

Files: `src/engine/ledger/execution.{cppm,cpp}` (operation branch hook, `execution_change` fields),
`src/engine/ledger/execution_review.{cpp,t.cpp}`, `src/engine/history/read.{cpp,t.cpp}`.

- `execution_change` gains `std::optional<execution_verdict_facts> verdict` (`role, run, verdict (history::verdict_v1),
  current:{targets, policy, work}, max_rounds, case, repairs:[role], blockers_opened:[kind]`) and
  `std::vector<std::uint64_t> blockers_cleared` (any operation; DR18-8). `operation == "verdict"` takes the request_v3
  framing `{run_id, verdict_path}`.
- Native step-5 checks (R18 "Native step-5 checks"; any mismatch `invalid_change`, the native spelling of `internal`
  per D151): verdict equals `normalize_verdict` of the document (the handler passes both; the ledger re-encodes and
  compares bytes); `facts.current.work` equals the reviewer dispatch's W and `policy == work_exec_digest(work)`;
  each `facts.current.targets[i]` matches the output and `work_fp` of a completion in that owner's current lineage;
  `applicability` from document vs `facts.current`; repair owners are workers of the active job owning a target
  (else `invalid_change`; the user-facing `reference` refusal is Lua's, P4); `facts.max_rounds` equals
  `shape.max_rounds` of the reviewer W (DB-R5); `case` equals R18-5 from the native count; `blockers_opened` equals
  the R18-5 table given the open blockers; repair revisions are allocated natively (lineage + 1).
- Rows, one transaction: value record `verdict_v1`, `verdicts`, `verdict_recorded` event (version 2, typed columns
  per R18-4), `requests` (outcome `recorded`, saved bytes `verdict_result_v1`), `run_state` → `completed`, `repairs`,
  `blockers` + `blocker_state` open, `blocker_state` cleared for `blockers_cleared` (checked `open` under the lock,
  else `state_conflict`), `transitions` (ordinal 1 reviewer, then by role ID; rows M37, M42, M43, M39, M44 and the
  M17 to M22 rows the change supplies), `role_state` delta with `fact_version + 1` for roles with a row. Preconditions
  exactly R18-4; lock-time `run_active` / foreign-run `role_state` → `fenced {run_state}`, else `state_conflict`.
  Same-ID equal replay returns saved bytes, unequal body `request_conflict`.
- `load` / `activate`: accept `blockers_cleared`; native enforces that `activate` and a job-clearing load clear every
  open blocker of the job; for other loads native checks each listed blocker is open and of the active job and, for
  `max_rounds` blockers, that the new revision's max_rounds exceeds the count when it can read it (DB-R7).
- `history --event` for `verdict_recorded`: the `execution` object of R18-8 (verdict, repairs, blockers opened /
  cleared, provenance native); `--request-id` returns the saved `verdict_response_v1`.
- Test fixture: a test helper (in the `.t.cpp`) that drives a real format3 ledger through the existing R15 commit
  path (`load`, `activate`, `claim` workers and reviewer) and, as **DRAFT-STUB R17-C**, writes the worker completion
  rows (`completions`, observation value record, `run_state` completed, `role_state` done, transitions) with direct
  SQL in one transaction so the snapshot looks as R17 `check_recorded` would leave it.
- Tests: approve current (reviewer done, M37, no repairs); admitted rejection writes every effect in one commit
  (A13 native part) and repaired workers go `stale` (M39) / stay `blocked` (M44), reviewer `blocked` (M42); stale
  output, old `work_fp`, wrong policy → `historical`, reviewer `stale` (M43), no repair or count (A06); `max_rounds`
  2 with three rejections → third `capped`, `max_rounds` blocker opened, no repairs (A14 native part); a load that
  raises max_rounds clears it; activate clears all and resets the count; empty `repair` → `unowned_rejection`;
  `blockers_cleared` naming a cleared blocker → `state_conflict`; replay same ID returns saved bytes and the repair
  revision stays +1; unequal body `request_conflict`; fenced / wrong-secret run; every step-5 mismatch
  `invalid_change` with nothing written; takeover after admitted rejection keeps the repair revision (lineage);
  history `--event` expansion in a fresh snapshot read.

### P4: `review.lua` and its harness

Files: `src/stdlib/planar/review.lua`; `src/cmd/planar/review_lua.t.cpp`; `src/cmd/planar/CMakeLists.txt` (new test
target only).

- `review.lua` defines one local table `review` with the functions of the R18 Lua table: `validate_verdict(doc,
  snapshot)` (steps 5 to 7 on the parsed value, including `reference` `Unknown selected role` field
  `verdict.repair.{i}`), `target_vector(snapshot, job)`, `policy(snapshot, reviewer)`, `classify(verdict, snapshot,
  max_rounds)`, `blockers_to_clear(snapshot, next)`, `acceptance(snapshot)` (R18-6 rules 1 to 4, why-not codes in
  the closed order, DR18-5), `carried_findings(snapshot, role)`, `review_context(snapshot, reviewer)` (returns the
  R18-1 item 5 lines as data: target lines, check-summary lines, finding blocks with four-space-indented text), plus
  `status_review(snapshot)` (the `status_v1.review` value) and `verdict_change(snapshot, verdict, run_id)` (the
  `execution_change_v1` for `verdict`: facts, reviewer and repaired-worker transitions, vector, expected; the M17 to
  M22 rederivation calls an optional `reconcile` hook and is the identity when absent, DB-R4).
- R17 functions `current_completion`, `applicable_completion`, `unit_output` are referenced through a `checks`
  local that `checks.lua` (R17-C) will define. In this lane the harness prepends a **test-only** `checks` shim
  string (in the `.t.cpp`, not under `src/`; DRAFT-STUB R17-C) implementing the R17-7 definitions over the snapshot
  keys `completions`, `observed`, `evidence`.
- Harness: raw Lua state as in `authoring_orchestrator.t.cpp` (`luaL_newstate`, base/table/string libs), load the
  shim, then `review.lua` from `PLANAR_SOURCE_ROOT`, then `return review`; snapshots are Lua tables in the
  `execution_snapshot_v1` key shape (Part 2 `read_execution`, R17-7 keys, DR18-8 `review`).
- Tests: target vector complete / incomplete; policy digest passthrough; classify current vs historical for stale
  output, old `work_fp`, wrong policy; admitted / capped / unowned with `max_rounds` 2 and counts 0, 1, 2; blockers
  to clear for raised max_rounds, vector change (unowned), activate; acceptance true after approve; withdrawn after a
  producer output change; `blocker_open`; recreated role (generation opened after the approval) has none (A22/A33);
  restored vector re-applies (DR18-5); why-not ordering; carried findings for repaired worker, kept across a
  takeover generation in the lineage; finding block bytes with multi-line text and every optional field;
  `validate_verdict` `reference` refusal; `status_review` with and without an active job.

## Draft decisions

| ID | Decision | Basis |
| --- | --- | --- |
| DB-R1 | `planar.engine.execution.verdict` uses its own `verdict_error` vocabulary (`io`, `path_denied`, `capture_changed`, `resource_limit`, `schema`, `unsupported_policy`) instead of `ledger::execution_error`, whose closed enum has no `schema`, `capture_changed` or `unsupported_policy`; the runtime service maps it to the EX-7 diagnostic. | CLAUDE.md: errors in the owning module's vocabulary; adds no code or message to EX-7 |
| DB-R2 | The ledger reader stops treating `completions` as reserved and reads a minimal completion list (DRAFT-STUB for R17-C); R17-C's snapshot extension replaces it at integration. | R18 native step 5 needs completions; R17-C is not in this lane |
| DB-R3 | The R18 ledger code lives in a second implementation unit `execution_review.cpp` of `planar.engine.ledger.execution`, with shared helpers declared non-exported in the interface; `execution.cpp` gets only marked hooks. | Disjoint files from the parallel R16/R17 lanes; EX-9 keeps the module |
| DB-R4 | `review.lua` is not added to any `planar_embed_lua` list or `commands.lua` this round; it is tested standalone. Packet bytes are returned as line data; `dispatch.lua` (R15-C) renders them. The accepted-quality rederivation (M17 to M22) goes through an optional `reconcile` hook (R15-C/R17-C). | `reconcile.lua`, `dispatch.lua`, `checks.lua` do not exist in this lane; no CLI in scope |
| DB-R5 | Native verifies `facts.max_rounds` equals `shape.max_rounds` of the reviewer's acknowledged W, and `facts.current.work` equals that W (a non-drifted running reviewer's current W is its dispatch W). | W contains `shape` (DR18-2); makes the policy and cap native-checkable |
| DB-R6 | Invariant 27 replays each verdict with the max_rounds of its reviewer run's acknowledged W. | A load between claim and verdict drifts the reviewer, which refuses the verdict, so this equals "the revision active at each verdict" |
| DB-R7 | For `load`, native checks only that each `blockers_cleared` id is an open blocker of the job and, for `max_rounds`, that the new max_rounds exceeds the count; that every clearable blocker is listed (the unowned "no longer current" rule) is trusted to Lua `blockers_to_clear`. `activate` and a job-clearing load must list every open blocker of the job natively. | Unowned clearing needs R17 applicability (Lua); keeps native checks exact where cheap |
| DB-R8 | The `reference` refusal for a repair role that owns no target is Lua's (`validate_verdict`); native treats the same condition in a change as `invalid_change`. | R18-2: steps 5 to 7 re-run in Lua; native mismatch is `internal` |
| DB-R9 | Test-only Lua shims and SQL-seeded completions are written inside the `.t.cpp` files, never under `tests/native/` and never under `src/stdlib/`. | Draft-mode rules; no shim ships |
| DB-R10 (P1) | Native `schema` messages for the verdict field table reuse existing schema-normalizer and observation-validator strings: grammar `JSON codec refused input` (field `verdict`); `Expected object`, `Expected list`, `Unknown field`, `Required field`, `Expected version integer`, `Unsupported schema version`, `List is too short`, `Invalid ID`, `Malformed output digest`, `Invalid enum value`, `Invalid portable path`, `Integer out of range`, `Invalid string`, `Duplicate value`. More than 256 `targets` is `Expected list`; an unsorted or duplicate `targets[].work` or `repair` entry is `Duplicate value` at the later index; a finding `work` that is not a target, and approve with a `must` finding or nonempty `repair` (step 7, field `verdict.decision`), are `Invalid enum value`; `line` without `path` is `Required field` at `verdict.findings.{i}.path`. `review.lua` (P4) must reproduce this table. | R18-2 and EX-7 add no message; these are the closest existing strings |
| DB-R11 (P1) | Step 5 check order: root kind; unknown keys (sorted, first wins); missing required keys in the order `version, targets, policy, decision, findings, repair, summary`; then fields in that order with `authority` last; inside a target `work, output, work_fp`; inside a finding `id, severity, work, path, line, prescription, text`, then finding-id uniqueness. Step 4 bounds apply only where the shape matches (a non-list `findings` is left to step 5). | R18-2 says "first failure wins" without an intra-step order |
| DB-R12 (P1) | Step 3 uses `codec_decode` with the 1 MiB / depth 128 / 100000-node limits; any codec refusal (syntax, duplicate key, null, non-integer or out-of-int32 number, NUL, its own depth or node ceiling) is `schema` field `verdict`. A `line` above 2147483647 is therefore a grammar refusal, not `Integer out of range`. | The shared parser refuses these before any shape check |
| DB-R13 (P1) | Step 1 resolves the path with `resolve_explicit_file` (a missing leaf remapped to `io` `Cannot resolve explicit input path`, as `--diagnostic`) and captures through `capture_diagnostic` with maximum 1048576; its `resource_limit` (`Capture limit exceeded`, `diagnostic_bytes`) is re-mapped to `verdict_bytes` with `Execution resource limit exceeded`. | P2-4 rows; no read_provenance exists in this lane |
| DB-R14 (P1) | The field table and bounds live in `planar.engine.history.execution_codec` (`check_verdict_bounds`, `verdict_from_value`, `check_verdict_decision`) and return `verdict_violation {field, message}`; the codec decoder and encoder reuse them (any violation is `invalid`), and the stored document must carry `authority:"reviewer_role_v1"`. Portable source paths use the declaration segment rules with a 4096-byte length (no render observation path grammar exists natively). | One rule set for reader and codec; no dependency from the codec on the reader |
| DB-R15 (P1) | `verdict_result_v1` encodes `review.count` and `review.max_rounds` as nonnegative decimals (`0` allowed), `roles[].drifted` as a boolean, blockers `opened` ascending by id and `cleared` ascending numerically; `admitted` is true exactly when `repairs` is nonempty, and then `decision` is `reject` and `applicability` `current`. | Verb section response; DR18-9 |
| DB-R16 (P2) | The D24 value S gains `completions` and `review` only when nonempty (review nonempty = count above 0 or any verdict, open blocker or repair revision), so a ledger without R17/R18 rows keeps the approved S bytes and the existing exact-S tests stay unchanged. | Part 2 S key list is approved; DR18-8 adds `review` by insertion |
| DB-R17 (P2) | max_rounds of a verdict is `shape.max_rounds` (integer or canonical decimal string) of the reviewer run's latest dispatch W before the verdict; a W whose `shape` is not a record or has no `max_rounds` uses the default 2 (`declarations.md:275`); any other value is `ledger_corrupt`. | R15 fixtures carry `shape` as a string; DB-R5/DB-R6 |
| DB-R18 (P2) | The reader replay accepts `verdict_recorded`: operation `verdict`, outcome `recorded`, typed columns job/role/generation/run, bumping, the event's run is the role's active run of that generation, ordinal 1 is that role leaving `running`, and its run becomes `completed` (N11) instead of `fenced`; the saved `verdict_response_v1` must name the ledger, request, job, role, revision, run and event. No `check_recorded` replay exists in this lane, so DRAFT-STUB completions are checked structurally only (run, a dispatch of that run with equal `work_fp`, `seq <= last_seq`, observation a `canonical_v1` value with sorted `{work, output}` units), not against a completing event or run state. | R17-C owns `check_recorded`; the event payload codec has no `check_recorded` key set yet |
| DB-R19 (P2) | For "unless one is open for the job" (R18-5) a blocker cleared by the same commit is not open; invariant 26 refuses two blockers of one (job, kind) whose open intervals overlap (the earlier must be cleared at or before the later opening seq). | R18-5 is silent on same-commit clear and open |
| DB-R20 (P2) | Invariant 21 also requires verdict ids ascending with `seq` and the event payload references `{body, run, verdict}` to equal the row; invariant 27 also cross-checks the saved result's `admitted`, `review.count` (after the commit) and `review.max_rounds` against the replay. | Saved result is part of the verdict commit (R18-4) |
| DB-R21 (P2) | Snapshot `review.verdicts` takes the latest reviewer-lineage verdict per reviewer role of the active job plus every active-job verdict of the current activation, sorted by id; `review.blockers` lists every open blocker (only the active job can hold one, since every `activate` clears all); `review.repairs` lists worker roles of the active job with a current-lineage revision above 0, in snapshot role order. | DR18-8 wording |
| DB-R22 (P3) | `planar.engine.history.read` has no format3 support in this lane (R15-D owns the format3 `--event` selector and `--request-id` results), so `read.{cpp,t.cpp}` are unchanged. The R18-8 expansion of `verdict_recorded` and the saved-result lookup are exported read-only from `planar.engine.ledger.execution` as `read_verdict_expansion(path, seq)` (the `execution` object, nullopt for any other event) and `read_saved_result(path, request_id)`; the format3 history path calls them at integration. | `read.cppm` cannot gain exports from `read.cpp`; history must not run Lua |
| DB-R23 (P3) | Lock-time mapping: when any verdict precondition fails, the refusal is `fenced` with the locked `run_state` of the token's run if the reviewer's locked run differs from it or that run is not `active` in the expected generation; otherwise `state_conflict`. A takeover (which also bumps `fact_version`) therefore reads `fenced`. | R18-4 mapping; EX-3 vocabulary order would otherwise report the earlier `fact_version` |
| DB-R24 (P3) | The `run_active` member carries the reviewer's job and role, `run` = the token run, `number` = the reviewer generation id. The secret is not in the change (DR18-10), so secret validity stays with the handler's token authorization (R16 step 6). | `execution_expectation` has no secret field |
| DB-R25 (P3) | `provenance.native` of the verdict expansion: `base_label` is the reviewer W `base` text; `preferences` are `{model, session}` each `{status:"unknown"}` (the recorded interpretation is not decoded natively in this lane). | P2-5 native needs the interpretation (R15-D) |
| DB-R26 (P3) | `fact_version` bumps for roles with a transition in the verdict commit, the existing EX-3 replay rule (D16), not "every role with a row" as R18-4 words it; the reader replay would refuse the latter. | Keep the approved reader rule; flagged for review |
| DB-R27 (P3) | `execution_verdict_facts` names R18-5 `case` as `admission` (C++ keyword); `document` is the parsed document and `verdict` its normalization; `current.targets` holds only complete vector entries and the vector is complete when its size equals the reviewer's `targets`. | Contract shape is a Lua table |
| DB-R28 (P3) | A `historical` verdict leaves the reviewer `stale` (M43) when every worker of the job is `done`, else `blocked` (reason `completed`). | M43 condition "workers all done" leaves the other case open |
| DB-R29 (P3) | Besides ordinal 1 (reviewer, reason `completed`), a verdict commit accepts only `repair_admitted` rows for exactly the admitted repair roles (M39 done→stale, M44 blocked→blocked) and `prerequisite_available` / `prerequisite_unavailable` rows of non-running roles (M17 to M22, supplied); no other run starts or ends. Transitions after ordinal 1 are strictly ascending by role. | R18-4 transition table |
| DB-R30 (P3) | `blockers_cleared` pre-lock: unique ids; claim and takeover clear none; `activate` and a job-clearing load must list every open blocker (`invalid_change`); for a verdict every listed id must be an open snapshot blocker. Openness (and the job for a job-keeping load) is checked under the lock (`state_conflict`). The `max_rounds` raise is not checked natively (DB-R7 "when it can read it": the interpretation is not decoded natively). | R18-4, R18-5, DB-R7 |
| DB-R31 (P3) | Test fixture: a worker completes through a real R15 load commit (running→done, its run `fenced`, a blocked reviewer →ready with `prerequisite_available`) plus the DRAFT-STUB R17-C completions row and observation record written by SQL; R17 `check_recorded` events do not exist in this lane and DB-R18 checks completions structurally only. | No R17 event type is replayable here |
| DB-R32 (P4) | `validate_verdict` reproduces the DB-R10/DB-R11 table with 0-based index paths (as P1, e.g. `verdict.findings.0.path`); step 4 bounds are not re-run (native did); an empty Lua table passes as either an object or a list (a Lua value cannot tell them apart); integers use `math.type` when `math` is loaded, otherwise a `tostring` check. Step 6 returns `unsupported_policy` with field `""` as native. | R18-2 steps 5 to 7 in Lua; P1 paths |
| DB-R33 (P4) | A role's current W comes from the optional hook `review.hooks.current_work(snapshot, role_entry) -> {work, digest}` (R15-C reconcile computes W); without the hook it is the acknowledged dispatch W of a non-drifted running role (DB-R5), else none, and then no vector or verdict can be current. `max_rounds(snapshot)` reads `shape.max_rounds` (integer or canonical decimal string) of the reviewer's current W, default 2 (DB-R17). | The snapshot carries no current W for non-running roles; Lua cannot hash |
| DB-R34 (P4) | Lineage data `review.lua` reads beyond DR18-8, all optional: `roles[].generation.lineage` (generation ids of the current structural lineage), `roles[].generation.lineage_opened` (`opened_seq` of its first generation, for R18-6 rule 3 / A22 / A33), `review.verdicts[].generation` (reviewer generation of the verdict's run) and `review.verdicts[].activation`. Absent means: the generation alone, 0, trusted to the native snapshot filter, and the current activation. The native snapshot (P2) must add them at integration, by insertion into DR18-8; flagged for review. | R18-6 rule 3 and the "current activation" rules need them; the approved snapshot keys do not carry them |
| DB-R35 (P4) | `target_vector(snapshot, job)` returns `{complete, targets, units, reviewer}`: `targets` holds only complete entries (as the native facts, DB-R27), `units` every reviewer target with its owner and entry. The reviewer of a job is its first non-retired reviewer role in snapshot order. `acceptance` lists every unit of the job's non-retired workers; approval and `rejected` are job-level: `rejected` holds when the latest (by seq) current, lineage, identity-matching verdict is `reject`. | R18-6 "unit of the active job" |
| DB-R36 (P4) | Carried findings come from the latest `reject` verdict of the current activation with applicability `current`; the worker carries them only when that verdict's `repair` names it and its seq is above the worker's `lineage_opened` (takeover keeps, recreation drops). The reviewer's `Unresolved findings` use the latest `reject` of the current activation regardless of applicability (the literal R18-1 text). Check summaries are the evidence rows on target units whose owner has an applicable completion, latest per (work, check) by seq. | R18-1 item 5 wording |
| DB-R37 (P4) | `blockers_to_clear(snapshot, next)` takes `next = {operation, job, max_rounds, snapshot}`: `max_rounds` clears only for operation `load` with `next.max_rounds` above the count; `unowned_rejection` clears when its opening verdict is no longer identity-current against `next.snapshot` (default: the same snapshot); `activate` or a changed `next.job` clears every open blocker. `classify` takes an optional fourth argument, the ids cleared by the same commit (DB-R19). | R18-5 clearing; contract leaves `next` open |
| DB-R38 (P4) | `verdict_change(snapshot, verdict, run_id, options)` (`options = {request_id, snapshot_seq, max_rounds}`) builds transitions, vector and expected with the native struct member names (`before_state`, `before_drifted`, `after_state`, `after_drifted`, `reason`; expected `kind` plus `job`, `role`, `text`, `revision`, `number`, `state`, `drifted`, `run`), since no Lua `execution_change_v1` codec exists in this lane (R15-C). A reviewer that is not running the named run, or a repaired worker neither `done` nor `blocked`, is `internal` `Invalid embedded command protocol`. The reconcile hook may append rows; ordinal 1 stays the reviewer, the rest sort by role, and the vector is rebuilt from the final transitions. | R18-4; DB-R28, DB-R29 |
| DB-R39 (P4) | Finding text is split strictly on LF, so a trailing LF gives a last line of four spaces. | R18-1 item 5 "each line of `text` (split on LF)" |

## Draft results (finisher, 2026-11-08)

Implemented (all four packages, unreviewed draft code):

- P1: `verdict_v1` and `verdict_result_v1` codecs, shared field table and bounds (`check_verdict_bounds`,
  `verdict_from_value`, `check_verdict_decision`), `"verdict"` in `payload_operations`, `verdict_recorded` payload
  references; new module `planar.engine.execution.verdict` (`parse_verdict_bytes`, `read_verdict_file`,
  `normalize_verdict`, `verdict_error`).
- P2: reserved-table list relaxed (`verdicts`, `repairs`, `blockers`, `blocker_state`, DRAFT-STUB `completions`);
  `verdict_recorded` replay; snapshot `completions` (DRAFT-STUB R17-C) and `review`; invariants 21 to 27 in
  `execution_review.cpp`; D24 S extended per DB-R16.
- P3: `verdict` commit (one EX-3 transaction, rows, transitions, repairs, blockers open/clear, replay and
  `request_conflict`), `blockers_cleared` on load/activate, step-5 native checks as `invalid_change`,
  `read_verdict_expansion` / `read_saved_result` (DB-R22).
- P4: `src/stdlib/planar/review.lua` and the standalone harness `planar_review_lua_tests` with a test-only
  `checks` shim (DB-R9).

Build (Debug, from the lane worktree): `cmake --preset debug` then
`cmake --build build/rewrite-debug --target planar_tests -- -k 0`: exit 0.

Tests:

- `ctest --test-dir build/rewrite-debug -R 'R18-B'`: 54 matched, 54 passed (9 verdict reader, 8 codec, 25 ledger
  review/commit, 12 review.lua).
- Whole binaries of touched modules, run directly: `planar_execution_verdict_tests` 9 cases / 241 assertions;
  `planar_history_execution_codec_tests` 30 / 1490; `planar_history_read_tests` 3 / 28 (module unchanged, DB-R22);
  `planar_ledger_execution_schema_tests` 23 / 1643; `planar_ledger_execution_tests` 145 / 96289;
  `planar_review_lua_tests` 12 / 869. All passed.
- Failing tests: none known. Full suite not run (draft mode).

Gaps:

- `capture_changed` in `read_verdict_file` is mapped but untested (no race seam in `capture_diagnostic`).
- DB-R10 reuses existing schema strings where no exact message exists; needs review, and `review.lua` must stay
  byte-identical with it.
- Portable source paths use declaration segment rules with a 4096-byte cap (DB-R14); no native observation-path
  grammar.
- The R18 interface table types `read_verdict_file` / `normalize_verdict` with `execution_error` and a capture slot;
  this lane uses `verdict_error` and a filesystem path (DB-R1); the lane-A `read_verdict` service must map it.
- Decimal ordering checks (length then bytes) are draft choices, not contract text.
- Snapshot lineage fields `review.lua` reads (DB-R34) are not yet emitted by the native snapshot.
- `history --event` / `--request-id` wiring for format3 is not in this lane (DB-R22); only the ledger exports exist.
- DRAFT-STUB R17-C completions are checked structurally only (DB-R18); no `check_recorded` replay.
- `fact_version` bump rule deviates from R18-4 wording (DB-R26), flagged for review.
- No format/Doxygen gate, rewrite-check, Linux run or mutation probes; clang-format 23.1.4 from Homebrew was used and
  assumed to match the pinned formatter.

Draft decisions: DB-R1 to DB-R39 above.

## Left for fixing rounds

- Full gate, format/Doxygen gate, rewrite-check, Linux/Docker, mutation probes (permissive mutants from R18 test
  obligations: "stale verdict admitted", "takeover generation starts empty", "count reset by load"), review loops,
  evidence directory.
- A13 fault variants (before COMMIT, after COMMIT before stdout), two-process races and SIGKILL tests (EX-10
  generator; R15-E style).
- Integration with R16 (token refusal order step 6, `recovery_records.findings`), R17-C (replace DB-R2 stub with the
  real snapshot `completions` / `evidence`; `checks.lua` replaces the test shim; `blockers_cleared` in R17 check
  facts), R15-C (`reconcile.lua` / `dispatch.lua` wiring of DB-R4, embed lists).
- Lane A: `verdict` verb, ABI row, `read_verdict` service, capability, help, catalog counts, `status_v1.review` and
  `orchestrator --ledger` output, CLI workflow tests, A24 CLI test, A29 fresh-process history test.
