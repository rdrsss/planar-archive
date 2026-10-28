# Planar rewrite roadmap

This roadmap executes [the implementation plan](implementation.md) against
[the v5 design](planar-design-spec-v5.md), subject to explicit operator decisions
and approved exact contracts until scoped amendments. The [alignment assessment](v5-alignment.md)
records conflicts and the [acceptance map](acceptance-matrix.md) tracks every row. Work happens in the current
checkout on `dev/planar-v2-vernacular`, without existing Planar orchestration.
Superseded status checkpoints are preserved verbatim in [history](history.md).

## Current status

Rounds replace this section when status changes; move nothing older than the
latest checkpoint here. Round records under [rounds](rounds/) and evidence under
`tests/native/*-evidence/` are authoritative.

- **Approved through:** R01-R14 (D8), V5-01, V5-02 A/B/C, X01-X05 (C5),
  X06-A1/A2/A3, the [X06-A4 architecture plus C01](profile-preparation-architecture.md)
  documentation, X06-B1 (with C01), X06-B2 (with C01, C02), X06-B3 A, B and C
  (with C01, C02), and [X06-C](rounds/V5-02-X06-C.md) native profile transactions with
  correctives [C01](rounds/V5-02-X06-C-C01.md) to [C04](rounds/V5-02-X06-C-C04.md)
  ([review](rounds/V5-02-X06-C-C04-review.md), approved on the records re-check
  2026-10-26), all committed. Debug 1041/1041 (macOS Debug only). The profile
  overload of `complete_acceptance` is internal; the shared seam is a non-exported
  partition (operator decisions 2026-10-26). Public `init` still creates format1.
- **Approved and committed:** [X06-D](rounds/V5-02-X06-D.md) whole-invocation budgets
  with correctives [C01](rounds/V5-02-X06-D-C01.md) to [C04](rounds/V5-02-X06-D-C04.md)
  ([review](rounds/V5-02-X06-D-C04-review.md), approved on records re-check 2,
  2026-10-27). One factory budget owner is threaded through every stage, including role
  preparation. Ceilings: native and live Lua 39,845,888, invocation instructions
  19,900,000, nodes 25,000, registration instructions 1,000,000, each fixture measured
  with at least 25 percent margin in Debug and RelWithDebInfo. Gate run 2: 1114/1114 in
  both configurations, zero skips; the final comment-only fix re-ran X06 165/165 (Debug).
- **Approved:** [X07-DOC](rounds/V5-02-X07-DOC.md) design addendum [x07-addendum.md](x07-addendum.md) ([review](rounds/V5-02-X07-DOC-review.md), approved 2026-10-28): closed factory over the five shipped families (Q1 B), the X01 production outcome table, operator decisions Q-INIT A and Q-G35 A.
- **Approved and committed:** [X07](rounds/V5-02-X07.md) public cutover (CLI11 envelope, service/CLI admission, format2 init) with corrective [C01](rounds/V5-02-X07-C01.md) ([review](rounds/V5-02-X07-C01-review.md), approved 2026-10-28). The 1 s sidecar grace is gone; a leftover `-journal` refuses unless another process holds SQLite's RESERVED byte (liveness probe), `-wal`/`-shm` refuse always. Debug and RelWithDebInfo 1227/1227, zero skips; 38 of 38 probe mutants killed; reviewer CLI live-journal runs and 20-run Debug and 25-run Release CAS loops clean. Recorded limits: Option A (a writer killed during a busy wait is rolled back silently by SQLite), non-POSIX-lock filesystems, probe lock visible as RESERVED for microseconds, unlock safe only without an own RESERVED lock on the inode.
- **Latest complete approved gate:** macOS Debug and RelWithDebInfo, **876/876**
  each, zero failures/errors/skips/disabled ([X05-C5 review](rounds/V5-02-X05-C5-review.md)).
  Committed inventory is 1227 (X07+C01, macOS Debug and RelWithDebInfo).
- **Unchanged limits:** public `init` creates format1; no RSS guarantee; Linux
  acceptance has not run.

## Next work

| Step | Scope |
| --- | --- |
| Carried limits CL-1 to CL-6 | Operator-approved carried limits (option A, 2026-10-27), not findings: the D5b exhaustion-first sites where a first failure caught by Lua can be replaced by `resource_limit` because exhaustion is tested before `bridge_error`. Each is repaired by a later round (X06-D repairs none). Clauses: `profile-preparation-architecture.md:396` ("one terminal first failure from any active stage") and the row "Semantic/protocol/resource failure caught by Lua | First terminal failure survives" (`:459` after X06-D; `:455` before it). **CL-1** `call_retained_projection` (`runtime.cpp:2016-2022`, exhaustion test at 2019, `bridge_error` at 2021): REACHABLE; untrusted callers `invoke_projection` (4498) and the extension projection callback (6335-6338). **CL-2** registration module loop in `register_profile` (`runtime.cpp:3503-3508`, exhaustion at 3505): REACHABLE; untrusted caller is the profile module chunk. **CL-3** `setup_commands`/`seal_environment` frames (`runtime.cpp:4219-4222` and `4235-4238`): NOT REACHABLE by untrusted callers (trusted environment-setup natives only). **CL-4** non-nested `phase()` pcall in `validate_profile_phases` (`runtime.cpp:4713`): PROBABLY REACHABLE, not exercised; untrusted caller is a public `validate_profile`, `select_profile` or `render_profile` call on a profile-snapshot command VM. **CL-5** command-service response in `command_vm::invoke` (`runtime.cpp:4888-4891`): NOT REACHABLE by untrusted callers (profile VMs refuse invoke at 4844-4845). **CL-6** catch-all fallbacks (`runtime.cpp:4298`, 4442, 4549, 4632, 6182): reachable only on host `bad_alloc` or `length_error`, not injectable. |
| Other carried limits | P01 survives (masked); N1 phase coverage unmeasured; #531 timing test about 15.5 s of 30 s; red snapshot tracked-only; Linux not run; ceiling pins: the instruction-ceiling equality pin was replaced by a derivation in C04 (`budget_instruction_max` derived from the margin model), the registration pin `budget_registration_instruction_max == 1000000` is kept and must change with any deliberate registration ceiling change; the knife-edge margin cells (about 25.1 percent native and nodes, 25.6 percent instructions) hold only for exactly the C04 constants |
| X07 follow-ups | No open code advisories from the X07-C01 review (all four findings were records-only and applied). Carried: Linux Docker probe for the storage liveness probe not run; mutation probes Debug only. |
| X08 (next) | Mandatory full external-profile acceptance gate, including the actual Linux gate |
| RA01, R15-R19 | Authoring/driver prompts, then observed-state execution (Phase 2 below) |

## Round protocol

Each numbered round uses fresh search, implementation and review agents, in that
order, following [orchestration](orchestration.md). Approval closes a
round; requested changes create a fresh corrective round before dependent work
advances. Record evidence under `rounds/<id>.md`. Split a round if its bounded
brief becomes too large for meaningful independent review.

## Phase 0 — Make the product contract concrete

### R01 — Repository map and rewrite instructions

Status: complete; [approved R01 record](rounds/R01.md). Depends on: the approved planning baseline.

- Map build targets, reusable libraries, Lua bindings, tests, installation and
  authored workflow surfaces. Identify obsolete dependencies and constraints.
- Find the referenced fixture if available; otherwise record its absence.
- Update repository agent/contributor instructions for the rewrite, file-based
  planning, direct serialized builds and the approved model/review protocol.
- Preserve useful C++ engineering rules and remove contradictory operational
  requirements. Follow the actual AGENTS/CLAUDE symlink structure.

Acceptance: a cited source map and removal/reuse inventory; instructions cannot
send an implementation agent into Planar claims, queues or legacy planning.

### R02 — Declaration and render contracts

Status: complete; [approved corrective record](rounds/R02-C01.md). Depends on: R01.

- Create `declarations.md` with exact versioned constructor schemas.
- Create `render-contract.md` with CLI, packet/manifest structure, capture rules,
  identity inputs, diagnostics and budget behavior.
- Resolve ownership path syntax, empty map/array representation, source-root
  identity, supported shapes and phase/quality combinations.
- Define explicit reviewer observations and render-only recovery semantics.
- Reconcile v4's transition language with the approved breaking rewrite.

Acceptance: constructors and examples are implementable without inventing field
semantics; open product decisions are visible and resolved before dependent code.

### R03 — Two jobs and expected packets

Status: complete; [approved R03 record](rounds/R03.md). Depends on: R02.

- Author `fixtures/render/quota/` with a single spec and combined/split jobs.
- Include shared and scoped requirements, mandatory file content, optional prose,
  a reviewed contract, check/toolchain descriptors and explicit ownership.
- Provide output observations and a checkpoint for reviewer/recovery examples.
- Author expected packets and a requirement-to-role coverage table.
- Describe mutation cases for selective identity changes and invalid mappings.

Acceptance: both jobs preserve requirements, checks, independent review and
ownership; packets are concrete enough to inspect for usefulness. Examples are
clearly labeled as expectations until native tests compute and verify them.

Phase 0 delivers the first agreed slice. It does not build a second mock runtime.

## Phase 1 — Build and prove native rendering

### R04 — Replacement build and command skeleton

Status: complete; [approved R04 record](rounds/R04.md). Depends on: R03.

- Establish `src/engine/`, `src/stdlib/planar/` and the replacement CLI target.
- Rewire module/dependency rules deliberately; keep the required pinned toolchain.
- Provide direct build, unit-test and acceptance-test commands with no database
  or old host queue requirement.
- Define the replacement full gate and retire conflicting legacy targets as
  their implementation surfaces leave the active build.

Acceptance: a clean checkout builds the replacement command; basic invocation
tests pass; no invocation opens the old database. Record the supported host
verification and any platform coverage still outstanding.

### R05 — Restricted loading and captured files

Status: complete; [approved R05 correction](rounds/R05-C01.md). Depends on: R04.

- Implement declaration environments, curated constructors and data conversion.
- Add memory/instruction/input/depth limits and typed load diagnostics.
- Implement confined captured reads with provenance and deterministic ordering.
- Test forbidden capabilities, mutation attempts, loops, large allocations,
  unsupported values, traversal, symlink escapes and inconsistent capture.

Acceptance: native tests establish bounded failures and confined reads; all
downstream consumers receive one captured input set.

### R06 — Canonical encoding and identities

Status: complete; [approved R06 record](rounds/R06.md). Depends on: R05.

- Implement versioned canonical encoding and pinned XXH3_128bits hashing.
- Pin native encoding/hash vectors, including empty containers, integer bounds,
  ordering and invalid inputs.
- Define observation and output manifests separately from work/render identity.

Acceptance: exact bytes and digest representation match independently specified
vectors; insertion order and fresh processes cannot change canonical results.

### R07 — Trusted schemas, grouping and scoped inputs

Status: complete; [approved component records](rounds/R07.md). Depends on: R06.

- Implement workspace/spec/job validation in trusted Lua.
- Resolve work assignments, ownership, mandatory policy and review separation.
- Implement pipeline and fan-out/review semantics within the declared scope.
- Resolve phase dependencies and reject unsatisfiable cycles.
- Derive scoped material manifests and interpretation-version inputs.

Acceptance: both fixture jobs validate; omissions, duplicates, overlap, unknown
references, policy weakening and invalid wrappers fail. Grouped work retains its
internal obligations without self-wait. Scoped mutation cases pass.

### R08 — Worker, reviewer and recovery rendering

Status: complete; [approved round record](rounds/R08.md). Depends on: R07.

- Implement projections over captured inputs without fresh reads.
- Emit Markdown and machine-readable identity manifests through the CLI.
- Implement exact section byte counts, mandatory overflow refusal, optional
  omission notices and labeled provenance.
- Turn reviewed examples into computed goldens, with explicit review of changes
  from the expected packets rather than blindly updating snapshots.

Acceptance: actual packets meet the contract; reviewer targets are exact;
recovery states no active run; missing checks/outputs never appear as success.

### R09 — Remove obsolete product surfaces and integrate the gate

Status: complete; [approved round record](rounds/R09.md). Depends on: R08.

- Complete staged removal of old binaries, domain engine, migrations, operational
  agents/skills/workflows, unused dependencies and obsolete tests or scripts.
- Refresh README, architecture, CLI, declaration and testing references.
- Remove or explicitly disable obsolete installation/release entry points.
- Verify no active path implicitly accesses the legacy database or old queue.
- Run the complete replacement gate on the integrated candidate, including
  fresh-process renders and the adversarial render matrix.

Acceptance: the active repository describes and builds one coherent replacement
product. Required tests execute with reported counts, no unexplained skips and
no stale references to operational features as available functionality.

### R10 — Evaluate useful context

Status: complete; operator feedback addressed by independently approved
[R10-P01](rounds/R10-P01.md). [Round record](rounds/R10.md). Depends on: R09.

- Present actual combined and split worker/reviewer packets and recovery examples
  to the operator, together with identity-change demonstrations.
- Assess missing context, distracting content, ownership clarity and actionable
  check instructions. Record limitations and requested revisions.
- Address changes through fresh reviewed rounds and rerun affected verification.

Acceptance: operator evaluation of actual fragments is recorded and required
changes are resolved. This is the boundary before persistence implementation.

## Phase 2 — Ledger foundation, then observed-state execution

### R11 — Ledger/history foundation contract

Status: complete; [independent review approved](rounds/R11-review.md).
[Round record](rounds/R11.md). Depends on approved R10 and R10-P01.

Specify [ledger foundation v1](ledger-contract.md): exact new-format schema,
bounded complete immutable trees, captured source and interpreted-state separation,
chronological revision IDs, CAS, original-result request replay, append-only typed
events/current projections, exact export, failure safety and backup evolution.
Writer owns that contract and updates to this plan/implementation plan; coordinator
owns round evidence. No product code, database creation or vendor changes.

Acceptance: independently reviewed implementable contract, resolved bounded product
choices and validated document links. Original v4 traceability is A20, storage/
format parts of A23, and A25–A29. Execution portions remain later obligations.

### R12 — SQLite dependency, new format and transaction foundation

Status: complete; [R12 record](rounds/R12.md) and [milestone review](rounds/R12-C2-review.md).

Own `cmake/dependencies.cmake`, required committed SQLite vendor files/build wiring,
new `src/engine/ledger/` modules and colocated tests; affected architecture/testing
references and `rounds/R12*` evidence. Fresh search first verifies the historical
SQLite 3.53.3 URL/SHA256/license and current CMake integration. Do not restore old
openers/wrappers, commands, migration files or implicit database discovery.

Implement only contract-v1 schema/format verification, explicit scratch initialization,
prepared statements, typed failures, bounded locking, rollback/commit and SQLite
backup/restore primitives. No CLI, capture policy, content trees, load or execution.
Dependencies: existing downward module/build conventions and pinned libraries.

Acceptance: direct serialized focused tests report nonzero counts/zero skips;
foreign/empty/future/truncated files remain unchanged; immutable rows refuse edits;
constraints and cross-row invariants reject malformed state; write failure, disk
full and deliberate subprocess termination leave full old/new state; concurrent
writers obey locking; explicit new-target backup/restore passes integrity and
foreign-key checks. Run affected formatting/Doxygen and no-change build checks.
Agree exact direct test commands after fresh search identifies registered targets;
never invoke installed Planar, legacy tools or real operator data. Record raw
failure-injection and restart evidence, then fresh independent review.

### R13 — Immutable records, complete trees and interpretation provenance

Status: complete; [integrated R13 review](rounds/R13-C2-review.md) approved.
Debug/release each pass 226 tests; the test-count prose correction is approved in [D01](rounds/R13-D01.md).
Depends on approved R12. Own new `src/engine/history/` codec/tree
modules, acceptance Lua envelope if needed, colocated tests, bounded fixture
inputs and affected references/evidence. No commands or execution mutation.

Implement record_v1 exact binary framing, bottom-up tree interning, exact collision
refusal, complete reconstruction, bounds and separate exact registry/stdlib plus
versioned normalized/semantic output fixtures. No JSON parser substitute.

Acceptance: byte/mode/topology round trips; sorted metadata, deletion/rename and
mtime-preserving edits; malformed/oversized graphs refuse; forced file/internal/
value/request digest collisions fail atomically without aliasing; complete old
trees reconstruct without patch replay or modern Lua. Demonstrate collision
regression before its fix and retain evidence. Covers A27 and tree parts of A28/A29.

### R14 — Acceptance, read history and explicit export

Status: independently approved through [D8](rounds/R14-D8-review.md); [A pure boundaries](rounds/R14-A.md) approved, [B history validation](rounds/R14-B.md) approved; [C1 typed reads](rounds/R14-C1.md) approved; [C2 capture receipts](rounds/R14-C2.md) approved; [C3-B2 atomic storage](rounds/R14-C3-B2.md) approved; [D1 paths](rounds/R14-D1.md) approved; D closure-only policy selected; [E1 init](rounds/R14-E1.md) approved; [E2 history](rounds/R14-E2.md) approved; [F1 native export](rounds/R14-F1.md) approved; [F2 export CLI](rounds/R14-F2.md) approved; [G composition checkpoint](rounds/R14-G.md) approved (debug/release331); [D2 ancestor receipts](rounds/R14-D2-review.md) approved (337 debug on LLVM
23.1.4). G release331 is prior-compiler evidence, not D2 release coverage. Load
integration follows independently approved A/B1/C01/B2/C through the
[R14-D3 exact load amendment](load-command-contract.md), independently approved in [D3 review](rounds/R14-D3-review.md).
D4–D8 are independently approved and pin typed errors, capture,
retained runtime, atomic public admission and complete R14 verification.
Depends on approved R13 and the V5-02 gate before further implementation. Own new `src/engine/acceptance/`, trusted
acceptance Lua, CLI11 `handlers/{init,load,history,export}/`, bootstrap registration,
colocated/native acceptance tests and affected product references/evidence.
Split into separately searched/coded/reviewed acceptance, read and export subrounds
if needed to keep ownership/review bounded.

Implement initial load, exact no-op, fresh chronological revert, CAS, original-
result replay, typed append events/current projection in one transaction, bounded
historical inspection and exact new-target export. Keep existing render unchanged.

Acceptance: contract's complete R14 matrix, especially two competing loads,
replay before CAS/capture after files disappear, changed parameter conflict,
invalid draft leaving active history intact, request recovery after lost response,
crash/disk-full rollback, early-event reconstruction without current policy,
collision refusal and byte/mode exact export with safe partial-failure cleanup.
Existing-target export never overwrites. Covers A20, storage/format A23, A25–A29;
no role-state, missing dispatch-packet or check guarantee is claimed yet.

### V5-01 — Direction assessment and documentation alignment

Status: independently approved after D2; [round](rounds/V5-01.md).
No build, runtime API change or code dispatch. Preserve the user-owned v5 input;
record retained/new/stale/pending dispositions, exact authority and every acceptance
row. The independently reviewed alignment remains the baseline for advancement.

### V5-02 — Command, trust and workspace contract alignment

Status: A/B/C [operator accepted](rounds/V5-02.md#operator-decision); V5-02-A
exact contract independently approved; B1 is approved with C01 and B2 CLI
composition and [C naming/invocation snapshots](rounds/V5-02-C-review.md) are
independently approved; public load follows in approved D7-B/D8. Depends on approved V5-01; bridge and naming precede R14 load.

- **V5-02-A:** [exact catalog/services/workspace contract](command-contract.md),
  scoped render/ledger amendments and sequence. Docs-only, no product build.
- **V5-02-B1:** retained bounded runtime/registry in `src/engine/runtime/`, shipped
  `src/stdlib/planar/commands.lua`, scoped embedding/bootstrap exports and tests.
  Prove pure atomic registration, immutable same-VM callable refs, bounds,
  malformed/collision refusal and native service isolation before CLI integration.
- **V5-02-B2:** CLI11 bootstrap, retained `handlers/{render,init,history,export}/`
  state and finite native adapters below handlers. Prove real Lua semantic dispatch,
  exact option ABI, parser/help/error precedence, unchanged current response bytes,
  no capability bypass and preserved export publication/stdout fallbacks.
- **V5-02-C:** workspace selector/resolver and realistic CLI/native tests. Pin bare
  name/explicit file/env/cwd rules, lexical request target versus physical render,
  aliases/limits and replay intent. Public load replay is independently approved in D7-B/D8.

Use the contract's [per-slice acceptance](command-contract.md#required-implementation-and-verification-sequence)
and fresh sequential search/coder/reviewer agents. No broad redesign is needed
for the specified embedded interfaces. Focused nonzero/zero-skip tests, applicable
format/Doxygen and no-change build, then integrated direct debug/release gates;
record actual compiler/platform limits. C adds explicit-environment workspace names
and invocation path snapshots; its approved debug/release gates each passed
370 tests with zero skips on macOS ARM64/LLVM23.1.4. D7-B/D8 subsequently add approved public load.

### V5-02-X — Mandatory external-profile gate after R14

Status: X01 test-only CLI11 spike and X02-C01 exact contract/implementation path are independently approved, with operator-selected P1 A/P2 A. External runtime implementation and acceptance remain open before dependent jobs and RA01. Embedded-only
R14 does not satisfy v5 extension delivery. Accepted selection is an explicit
native `with-profile --stdlib-profile FILE` envelope with ordered modules, no
ambient discovery and collision/override refusal. First review/spike the CLI11
passthrough bootstrap and exact profile capture/trust/registration ABI, then new
versioned request intent/effective provenance/interpretation and format coexistence
codecs and selected-shape identity. Implement only after that scoped review.
Prove removed-profile replay before evaluation, selector conflicts, sealed external
handlers, bounds/native capability refusal and recorded history without current
Lua. Native request_v2 is supported by the independently approved X03 slices;
interpretation_v2 and external runtime admission remain future work. Initial new
external verbs remain read/projection-only; native finite mutation/execution
services require separately reviewed authority. Preserve request_v1 and embedded
provenance codecs through R14. See [external gate](command-contract.md#required-implementation-and-verification-sequence).

### RA01 — Authoring and driver prompts

Status: future, inserted between R14 and R15 without renumbering historical IDs.
Depends on completed R14, embedded V5-02-B/C and external V5-02-X;
declaration/render foundation is already present.

Specify then implement deterministic spec and critique prompts, ingest job prompt,
and load-rejection prompts naming field paths while retaining raw typed diagnostics
and refusal semantics. Contract init authoring/scaffolding alongside current empty-
ledger init before implementation; never overwrite user files. Templates carry
exact schemas, allowed check IDs and shape catalog. Cover local A35/A36.

Specify driver-invoked observe assembly from an explicitly named directory, with
schema, limits and trust/extension policy. No autonomous scan/watcher. Design and
render useful reference-orchestrator graph/shape guidance, accurately labeling
available and future commands. No orchestrator projection exists at this checkpoint.
Execution rounds extend it as live state/actions arrive; R19 proves actual use.

### R15 — Accepted reads and run ownership

Status: future reviewed schema/protocol then bounded implementation. Depends on
RA01 and completed R14. Add accepted-revision render, basic pending-draft status,
active job, generations, retirement, claim request/packet atomicity, fencing and
explicit takeover. Role activation/reconciliation commits with revision acceptance
before runs are enabled. Cover A09/A10/A20/A22/A23 dispatch/A26 reconciliation/A33.
Design minimal A34 execution provenance before runs: spec/baseline, job/view/ledger/
trial identities, requested/actual driver/model settings and outcomes with honest
unknowns. No controlled experiment or metrics API is implied. Add matching live
orchestrator guidance and explainable state reads.

### R16 — Refresh and recovery

Depends on R15. Design/implement same-run acknowledgment, --since, drift,
checkpoints, interrupted/released runs and retained historical recovery. Preserve
active versus acknowledged revisions, exact dispatched context and evidence.
Cover A11/A19/A29 and recovery portions of A12/A20/A23; add actual orchestrator
refresh/recovery guidance and provenance events.

### R17 — Checks, boundaries and observed completion

Depends on R16. Design/implement bounded trusted check adapters, reservations,
exact input/output observations, before/after recheck, boundary attribution,
evidence applicability, published facets and prerequisite rederivation. Cover
A05/A12/A15–A18, completion portions of A23/A28 and check-history A29. Missing tools,
zero tests, timeout and malformed results fail. Driver stability and race limits
remain explicit. Semantic dependency changes, output-vector changes and blocker
availability have distinct effects; valid historical evidence can restore done.
Add live orchestrator check/completion guidance and provenance.

### R18 — Review and repair

Depends on R17. Design/implement exact-target independent review, atomic verdict/
repair/retry, cumulative max_rounds and durable blockers. Cover A06/A13/A14/A16,
approval parts of A22/A33 and review-history A29. A24 is conditional: explicitly
configured external assertion policy must target exact outputs, or unsupported
approval must refuse. No authenticated-human guarantee. Grow actual orchestrator
review/repair guidance and retained provenance.

### R19 — Single-root external-driver acceptance

Depends on R18. Demonstrate a real single-root job, both groupings, with the
reference orchestrator rendering usable guidance against live APIs and real
checks. Inspect accepted declarations, dispatch, state/check/verdict references
through restart and faults without manual database edits or re-execution.
Close every applicable Render/Execution matrix portion, conditional A24 and minimal
A34 provenance using the [acceptance map](acceptance-matrix.md). A24 applicability
must be recorded. A21 composition and controlled A34 experiments are deferred.
A 24-hour experience run cannot replace the full gate. No scheduler/model calls
or Planar-managed checkout lifecycle.

The native foundation alone does not establish cut 2. Every execution item starts
with independently reviewed exact schema/protocol; split into subrounds as needed.
Historical round IDs/evidence remain intact. V5's earlier cut-2 impact wording is
recorded as a conflict; its concrete post-R19 composition sequence governs here.

## Later phases (deferred)

1. Workspace composition: imports/exports, multi-root bindings, scoped reload and
   impact and render roadmap, with alias and exact-output-vector tests (A21).
2. Measurement: controlled context, grouping and model trials with fixed starting
   inputs and recorded driver provenance (controlled A34); metrics and tournament
   follow end-to-end execution, not before it.
3. Optional adapters: stronger isolation, archives, driver-owned views or identity
   attestation only for demonstrated requirements. Planar never creates/manages
   checkouts; v5's conflicting managed-worktree optional wording is not a new round.
4. Distribution: replacement installation/release design after an end-to-end
   gate, with explicit format detection and no legacy migration promise.
