# Planar rewrite roadmap

X05-C3-B2 and zero-capability X05-C4-A execution are independently approved.
C4-B0 independently approves test-only macOS feasibility of original receipt,
held no-follow descriptor and SQLite HAS_MOVED checks before content access.
The C4-B1 candidate adds the distinct native `connection::open_gated` read-only
primitive with a retained trusted verifier and a private nondefault per-connection
VFS. Main reads and mmap fetches require verifier success and supported, unmoved
HAS_MOVED before delegation. First typed failure remains sticky without allocation;
statements and transactions retain the verifier/VFS through actual close. The
existing ungated opener remains available. C4-B1 awaits fresh independent review.
Profile adapters, raw-header binding, actual history/read services and handler
integration remain closed. Public CLI, whole-invocation shared capture budgets,
X06–X08, execution/RA01 and Linux acceptance remain open; public init creates
format1. Focused Debug evidence is recorded in
`tests/native/v5-02-x05-c4-b1-evidence/REPORT.md`; B0 establishes feasibility only.

Current integrated shape checkpoint: [X05-B4-C evidence](x05-shape-evidence.md) records one full
Debug **676/676** gate (268.37 seconds) and one release-preset RelWithDebInfo
**676/676** gate (126.24 seconds), each with zero failures/errors/skips/disabled
cases and all 676 candidate plus 582 A4 names preserved. Six common vectors,
per-configuration source/runtime audits, pinned formatting/Doxygen over 118 active
sources and no-work builds pass. This verification-only gate is independently approved by the B4-C review;
the latest approved full gate is 676 per configuration. The B2-C+C01+C02, B3-A, B3-B+R01, B3-C, B4-A+C01 and
B4-B+C01 slices are independently approved; B4B-R1 is closed. Historical private
fault/retained-callable probes remain approved Debug evidence, without release
rerun claims. Public init still creates format1. C projection/read services, X06
recorded interpretation/history/acceptance and receipt/whole-invocation orchestration,
X07 public cutover, X08 mandatory external-profile acceptance, execution/RA01 and
Linux remain open. Detailed later historical checkpoints retain their original
limits; this current checkpoint supersedes their stale future-slice wording.

X03-A, X03-B1, X03-B2-A and the combined X03-B2-B+C01 native implementation
are independently approved; C01 closes B2B-R1. The independently approved [X03-B3 verification gate](x03-format-evidence.md)
runs full debug and RelWithDebInfo gates: 430/430 each, zero failures/errors/skips,
pinned lint/Doxygen and no-work builds. [Independent B3 review](rounds/V5-02-X03-B3-review.md) closes X03-B. Native format2 acceptance uses immutable request-v2 embedded
intent, codec-aware exact interning and explicit recovery under retained limits.
Public init still creates format1. Fresh format2 request-v1 and profile acceptance
refuse; profile accepted history remains unsupported until X06 provenance/
interpretation support. B2-A SQL reader fixtures provide no conversion tooling.
The mandatory external-profile gate, public cutover, Linux and execution remain open.


The [X01 CLI11 spike](rounds/V5-02-X01-review.md) is independently approved as
test-only feasibility; approved baseline inventory is 405 tests. X02-C01 independently approves the
[external-profile exact amendment](external-profile-contract.md) and
[bounded implementation path](external-profile-implementation.md), with operator-selected P1 A/P2 A recorded in X02-C01. These documents do not
implement external loading or close its mandatory gate. Current v1 contracts,
R14 evidence, execution/RA01 order and Linux limits remain authoritative.

R14 is independently approved through D8, including D7-B public load and the applicable acceptance portions. Final D8 debug/release gates each pass 402 tests with zero skips. The mandatory external-profile gate is next; execution, RA01 and Linux acceptance remain future work.

Status: R01–R10 complete; the requested presentation revision is independently
approved. R11 ledger/history design and R12 native storage foundation are complete
and independently approved. R13 immutable records, trees and provenance and applicable R14 acceptance portions are complete and independently approved.

This roadmap executes [the implementation plan](implementation.md) against
[the v5 design](planar-design-spec-v5.md), subject to explicit operator decisions
and approved exact contracts until scoped amendments. The [alignment assessment](v5-alignment.md)
records conflicts and the [acceptance map](acceptance-matrix.md) tracks every row. Work happens in the current
checkout on `dev/planar-v2-vernacular`, without existing Planar orchestration.

Each numbered round uses fresh search, implementation and review agents, in that
order. Models are `gpt-6-luna` high, `gpt-6.1-sol` medium and `gpt-6.1-sol` high,
respectively. Approval closes a round; requested changes create a fresh
corrective round before dependent work advances. Record evidence under
`rounds/<id>.md`. Split a round if its bounded brief becomes too large for
meaningful independent review.

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

## Current position

The current checkpoint includes independently approved R14 through D8, with
render/init/history/export/load and declaration-closure-only load membership.
D8 debug/release gates pass 402/402 each with zero skips; documented non-hot-journal
refusal and platform limits remain. V5-01 and V5-02 A/B1/C01/B2/C are approved.
Mandatory external V5-02-X is next, followed by RA01 and R15–R19. The milestone
list below records historical render/evaluation progression.

- Completed: repository inspection for planning and operator decisions about
  scope, current checkout, fresh-agent protocol and model routing.
- Written: this roadmap and the implementation plan.
- Completed R01: repository map and rewrite instructions; independent review approved.
- Completed R02: exact declaration/render contracts; five initial findings resolved
  in a fresh corrective round, then full independent approval.
- Completed R03: two same-spec jobs, concrete quota source/tests, six expected
  packets with symbolic manifests, coverage and mutation cases; independently approved.
- Completed R04: isolated native skeleton and direct debug/release verification gate.
- Completed R05: native restricted Lua and confined capture; corrective review approved.
- Completed R06: independently verified canonical encoding and XXH3 identities.
- Completed R07: trusted bridge, normalized schemas, capture topology, global semantics and scoped identities independently approved.
- Completed R08: actual native rendering, computed packets and CLI independently approved.
- Completed R09: obsolete repository surfaces removed; debug/release full gates and independent fresh build passed.
- R10: actual packet and identity demonstration package independently approved;
  operator requested improved presentation before continuing. R10-P01 is approved
  and resolves that request. R11 ledger/history design is independently approved.


## Subsequent operator corrections

[N01](rounds/N01.md) renamed the native layer to engine; [N02](rounds/N02.md)
moved product Lua to src/stdlib/planar while keeping fixtures outside src.
[N03](rounds/N03.md) restored Glaze and removed handwritten product JSON handling.
All three corrections are independently approved. Current Glaze wire-output
[evaluation evidence](../../tests/native/n03b-evidence/README.md) supersedes only
R10's serialization spelling; parsed packets/Markdown/identities remain equal.
The subsequent R10 presentation revision is recorded below.

[N04](rounds/N04.md) restored CLI11 parsing and the prior handlers/<command>/
structure. Independently approved; its debug/release gates each passed 121 tests
with zero skips.

[R10-P01](rounds/R10-P01.md) addresses the operator's request to improve packet
presentation first, then continue. Independently approved with no findings; both
full gates passed 124 tests with zero skips. R11 ledger/history design is
independently approved. Persistence was unimplemented at that checkpoint; R12 and
R13 have since delivered the native foundation, and D8 closes the applicable R14 portions.

Current command bridge status: V5-02-A is independently approved; B1 is approved
with C01. B2 retained-VM CLI composition is independently approved. V5-02-C
workspace naming and invocation snapshots are independently approved. The
[R14-D3 load amendment](load-command-contract.md) is independently approved in [D3 review](rounds/R14-D3-review.md).
D4 typed native diagnostics are independently approved. D7-B public load and D8 applicable R14 acceptance are independently approved; external profiles and RA01 remain pending
in their established order. This status adds no acceptance
claim for execution or Linux.

R14-D5 declaration-closure capture, D6/C01 retained same-VM binding, D7-A receipts,
D7-B public load and [D8 acceptance closure](rounds/R14-D8-review.md) are independently
approved. The [R14 evidence table](r14-acceptance-evidence.md) records applicable
acceptance portions and the conservative non-hot-journal refusal boundary. Final
D8 debug/release gates pass 402/402 each with zero skips on macOS ARM64; no
universal crash recovery, Linux or execution acceptance is claimed.
The mandatory external-profile gate starts with the test-only
[X01 CLI11 spike](rounds/V5-02-X01.md); X02-C01 independently approves the
exact contract with operator-selected P1 A/P2 A. External loading remains future
implementation and acceptance work. RA01 and dependent jobs remain gated.

The [X03-B2-B-C01 correction candidate](rounds/V5-02-X03-B2-B-C01.md) reuses complete metadata equality,
including the selected ledger format, at locked transaction entry and both native
precommit boundaries. Controlled verified-metadata fixtures exercise refusal and
rollback in both formats; they do not reproduce an external format change.
C01 independently approves the correction and combined B2-B candidate.
The [X03-B3 review](rounds/V5-02-X03-B3-review.md) independently approves the full
430-case debug/release gate and closes X03-B; the external-profile gate remains open.

The [X04-A1 review](rounds/V5-02-X04-A1-review.md) independently approves native metadata
admission and descriptor binding; its inventory is 451 (not a full 451-test run).
The [X04-A2 review](rounds/V5-02-X04-A2-review.md) independently approves strict bounded
profile/ordered module capture and retained staged input receipts. The
[X04-A3 integrated native gate](x04-capture-evidence.md) is independently approved by the X04-A3 review: debug and RelWithDebInfo each pass 489/489 with zero
failures/errors/skips, preserving all 488 prior names. No external Lua, public
profile CLI, provenance acceptance or whole-invocation budget sharing is inferred;
the mandatory external-profile gate remains open.

The [X04-B1 prerequisite candidate](rounds/V5-02-X04-B1.md) extracts
the unchanged bounded profile decoder into the pure `capture/profile_codec` module
and adds the distinct 32 MiB/8192-file `effective_provenance` tree budget.
Focused debug verification passes 177/177 with zero failures/errors/skips; the
497-name inventory preserves all 489 approved A3 names. The independent
[X04-B1 review](rounds/V5-02-X04-B1-review.md) approves these prerequisites.

The pure X04-B1/B2/B3 slices are independently approved. B2 validates and
immutably seals catalog-v2, including source owner ordinals, exact shipped native
signatures and external static capabilities. B3 assembles and structurally verifies
exact `effective_stdlib_v1` bytes, modes, ordered modules, old embedded assembly,
catalog owners and full membership. The catalog-only 1 MiB ceiling and combined
canonical manifest 1 MiB ceiling are separate checks. B1 tests the outer
32 MiB/8192-file tree budget; valid complete B3 inputs reach at most 4162 files
and remain below 32 MiB under the stricter component bounds.

The [X04-B4 integrated provenance gate](x04-provenance-evidence.md)
is independently approved by the [B4 review](rounds/V5-02-X04-B4-review.md): full Debug and
RelWithDebInfo each pass 520/520 with zero failures/errors/skips/disabled cases,
preserving all 520 prior names. Pinned formatting/Doxygen checks 113 active
sources; vectors, exact source audits and no-work builds pass. No product or test
bytes changed in B4, and tested sources remained fixed through both gates.
This pure structural gate adds no live external registration/Lua, public profile
CLI, storage/profile-history acceptance or history dispatch, whole-invocation
shared budgets, or callability/semantic-honesty proof. The mandatory external-profile
gate, public cutover, execution/RA01 and Linux remain open; public init creates
format1. The independent B4 review closes integrated pure X04-B.


The [combined X05-A0+C01 independent review](rounds/V5-02-X05-A0-C01-review.md) approves bounded
test-only lexical sealing feasibility and closes A0-R1. The
[X05-A1 production foundation](../../tests/native/v5-02-x05-a1-evidence/REPORT.md)
is independently approved in the X05-A1 review. The embedded five-command VM audits
original global/callable roots together before alias-preserving table freezing.
The isolated `runtime/closure_guard` module reads pinned Lua compiler metadata,
rejects retained upvalue writers and unknown reachable native identities, and
permits invocation-local scratch. Exact strong native identity capture precedes
module evaluation; the five native engine callbacks enroll immediately after
native installation, without intervening module execution. Proxy natives are
created only after this audit and are not claimed to pass a later raw audit.

The protected sealing frame reserves 2048 slots before root pushes/native RAII;
inspection entry and every traversal push use nonallocating active-frame checks.
A shared identity/prototype memo preserves existing graph depth256 semantics;
prototype entries include the inherited sealed-cell mask. PMR allocation requests
charge the existing checked cumulative 512 MiB native conversion accounting before
allocation, with no refund or root reset. Static metadata inspection has checked
reported counters and is not charged as Lua instructions or converted nodes.
The finite compiled input remains bounded by the registration Lua heap; no fresh
4 MiB audit arena or public scan quota is introduced. Existing embedded invocation
instruction reset remains unchanged.

A1 does not execute external modules or admit public profile flags. Later X05-A
rounds retain source-owner/ordered module admission, the three descriptor APIs,
module-end audit/seal before later chunks, frozen global/API state, persistent
frozen-identity memo, add-time snapshot versus retained mutation semantics, and
whole-registration cumulative gates. The mandatory external-profile gate, public
cutover, storage/profile history dispatch, execution/RA01 and Linux remain open;
public init creates format1. No full milestone or release gate is claimed for A1.


The [X05-A2 repeatable sealing foundation](../../tests/native/v5-02-x05-a2-evidence/REPORT.md)
is independently approved by the X05-A2 review. A private protected root-argument seam
seals new retained roots after the initial embedded environment boundary. Its
strong native-owned successful-identity set is distinct from each boundary's DFS
memo: the guard treats prior sealed tables/Lua closures as immutable atoms and
the freezer preserves their proxy backing and aliases. New identities publish
only after complete audit/freeze success; any failure leaves the owner terminal,
including partial freezing or publication failure. Direct native callable roots
and unknown reachable native closures still refuse. `pairs` returns the exact
stable global `next` closure captured before module execution; native snapshot
never traverses C upvalues. Repeated sealing never writes frozen globals.

The production public runtime interface and old invocation tail remain unchanged
except that a failed sealing owner cannot invoke. Synthetic root bookkeeping adds
no graph depth; every boundary shares existing cumulative native charges and
inspection totals, without resets. Real private production probes exercise three
successive boundaries, fresh retained roots, prior proxies/aliases, exact iterator
identity across GC, scratch, writer/native refusal, resource failures and same-owner
failed retry. A3 still owns captured ordered modules, owner frames, three exact
registration descriptors and final catalog; final D is validated/copied at module
end before sealing D/F. No external execution, public profile flags, profile
storage/history acceptance or full registration gate is delivered by A2. The latest
full gate candidate is A4 (582 per configuration); the mandatory external-profile gate,
public cutover, execution/RA01 and Linux remain open. Public init creates format1.


The [X05-A3 captured registration](../../tests/native/v5-02-x05-a3-evidence/REPORT.md) adds native
`command_vm::create(profile_snapshot)` with structurally checked, privately copied
captured bytes and module order. Source-only zero-result chunks execute in one
bounded VM with native owner ordinals. Three exact descriptor APIs retain D/F;
final descriptors validate and seal with their callables at each module boundary.
Catalog-v2 and exact effective provenance bind to the original captured bytes/modes.
Selected-profile errors retain redacted native logical fields separately from
existing embedded diagnostics. External `_ENV` is natively created before shipped
evaluation with constructor-only engine, self `_G`, shared pure libraries and
three exact registration functions; original globals and this environment freeze
together before external execution. Embedded handlers retain their original engine.

A3 is registration-only and independently approved with C01–C05 by the
[C05 review](rounds/V5-02-X05-A3-C05-review.md). Profile-owner invocation
refuses before any provider action; retained alternate callable execution is
verification through private reversible production copies. Shipped shape/projection
IDs remain reserved; real adapters and semantics belong to B/C. No shipped Lua,
public CLI admission, profile history/storage interpretation or acceptance changes
are delivered. Native conversion/audit charges and explicit conservative pure-helper
capacity reservations remain cumulative; these are not process RSS accounting.
The mandatory external-profile gate, public cutover, execution/RA01 and Linux
remain open. Public init creates format1; the latest full approved gate is A4
(582 per build).


The [X05-A3-C01 corrective](../../tests/native/v5-02-x05-a3-c01-evidence/REPORT.md)
preserves original entry fields before command-name/shape/projection sorting and
propagates initial provenance preparation's typed resource refusal. Its final
logical reservation enumerates six canonical/payload buffers with separate record
framing/digest room, independently derived embedded metadata encoding/payload and
graph passes, and unchanged eleven embedded/three external source payload passes.
The existing cumulative 512 MiB charged native metric remains; this is not RSS
accounting. A valid catalog near the 1 MiB ceiling and controlled remaining-budget
probes exercise these reservations. Combined A3 through C05 is independently
approved; A4 records the latest full approved gate. Profile CLI, B/C/X06, public
cutover, execution/RA01 and Linux remain open; public init creates format1.


The [X05-A3-C02 location correction](../../tests/native/v5-02-x05-a3-c02-evidence/REPORT.md)
retains the selected original root through the guard's single shared audit and
maps module D/F pairs to one-based original catalog fields. Actual root freezing
uses the same mapping; failures before root selection and during identity-set
union use the active `profile.modules[N]` boundary. The existing enum-only guard
API delegates to the detailed result. Initial embedded/environment sealing and
private repeated-root sealing do not acquire external entry locations. Terminal
module retries preserve the first location; the public constructor discards failed
owners rather than retrying. Original C01 fixes and cumulative budgets remain.
Controlled probes exercise actual pre-root, partial-freeze and union refusal,
terminal state and cleanup; they do not claim natural resource exhaustion.
Combined A3 through C05 is independently approved. A4 records the latest
full gate candidate; public profile CLI, B/C/X06, mandatory external-profile
gate, cutover, execution/RA01 and Linux remain open. Public init creates format1.


The [X05-A3-C03 allocation-safe location](../../tests/native/v5-02-x05-a3-c03-evidence/REPORT.md)
selects seal locations with a trivial kind/index token. Root audit/freezing no
longer formats fields. Every protected seal failure adapter renders the token
outside Lua; a rendering allocation refusal returns the existing external resource
diagnostic with the preinstalled active module field. Two independent module-field
strings per module are staged before processing, charged cumulatively, and swapped
without allocation after descriptor collection. Staging failure identifies profile;
first-failure terminal retries retain their token. The shared audit, freeze order,
depth, publication and embedded paths remain unchanged.
Ordinary index10/64 and mixed-module controls, actual native allocation refusal,
controlled Lua allocation/stack/status and throwing-allocator adapters verify these
boundaries with cleanup. The throwing allocator is a controlled adapter probe,
not a claim that realloc throws C++ exceptions or natural host exhaustion occurred.
Combined A3 through C05 is independently approved. A4 records the latest
full gate candidate; public profile CLI, B/C/X06, mandatory external
profile gate, cutover, execution/RA01 and Linux remain open. Public init creates
format1.


The [X05-A3-C04 registration attribution](../../tests/native/v5-02-x05-a3-c04-evidence/REPORT.md)
extends the nonallocating original-location token across callback admission,
reference retention, final descriptor collection and pure catalog error remapping.
Successful callbacks clear their token; caught failures retain the first token,
and catalog prevalidation clears completed entry ownership. Error adaptation
renders only outside protected Lua calls. Rendering refusal retains the owned
module fallback; unmatched canonical entries retain the truthful caller boundary.
Pure global error fields move their owned storage. A separately charged/staged
catalog fallback installs by swap before final provenance work as preventative
consistency; no allocation failure at the previous short assignment is claimed.
C03 module staging/sealing, shared audit and cumulative budgets remain intact.
Controlled actual allocation probes distinguish removed zero-request operations,
remaining projection rendering refusal, sticky callback failures and module/global
phase work. Raw-reference status and unmatched pure errors use explicit controlled
adapters. No new quota or public interface is added. Combined A3 through C05 is
independently approved. A4 records the latest full approved gate; public profile
CLI, B/C/X06, mandatory external-profile gate, cutover, execution/RA01 and Linux remain open. Public init creates
format1.


The [X05-A3-C05 caught-memory](../../tests/native/v5-02-x05-a3-c05-evidence/REPORT.md) makes a completed
restricted `lua_pcall` memory failure sticky in native state. The bit is set
only after `LUA_ERRMEM`, preserving a successful emergency allocation retry;
logical quota refusal remains separate. Outer restricted calls rethrow and host
adapters retain the existing resource diagnostic and original entry token.
Actual descriptor-reference allocation probes cover normal/later-error tails,
three nested catches, direct host failure and successful retry after one null
return. The function-reference-only control has zero actual growth requests;
no function-reference allocation failure or natural host exhaustion is claimed.
Ordinary caught Lua errors remain recoverable. Shared accounting, protected
ownership and existing diagnostic priority are retained. The [C05 review](rounds/V5-02-X05-A3-C05-review.md)
independently approves combined A3 through C05 and closes R1–R7. A4 records the
latest full approved gate; public profile CLI, B/C/X06, mandatory external
profile gate, cutover, execution/RA01 and Linux remain open. Public init creates
format1.


The [X05-A4 integrated registration gate](x05-registration-evidence.md)
is independently approved by the X05-A4 review. Full Debug and
release-preset RelWithDebInfo each pass 582/582, with zero failures/errors/skips/
disabled cases, preserving all current 582 and prior 520 names. Pinned formatting/
Doxygen checks 117 active C++ sources; common source vectors, per-configuration
runtime/CLI and reversible variant audits, and no-work builds pass. Product/test
bytes remain identical across both gates; only current status documentation is
finalized afterward. Earlier private retained-callable/fault probes remain
approved Debug evidence and are not claimed as release reruns. Profile-owner
invocation refuses before provider effects. B/C semantics/adapters, X06 recorded
history/interpretation/acceptance and receipt/whole-invocation orchestration,
X07 public cutover, X08 mandatory external-profile acceptance, execution/RA01
and Linux remain open. Public init creates format1.


The [X05-B1 pure shape foundation](../../tests/native/v5-02-x05-b1-evidence/REPORT.md) is independently approved by the X05-B1 review. An optional trusted internal context selects profile-only shape IDs and
stages policy while retaining declaration version1 and the embedded default.
Every job's mandatory policy and base phase cycles finish before custom shape
callbacks; bounded additive unit obligations then join the original full graph.
A distinct test-only exact-runtime adapter exercises the actual product Lua
helpers. Its internal `shape_results` sidecar is not final selected-shape identity.
B1 does not prove retained native F dispatch, immutable F input, exact source
identity, the complete return's 16 MiB boundary, or whole-invocation accounting.
B2–B4 retain those native/identity/W/render duties. Public profile admission,
X06 history/acceptance, X08 mandatory acceptance, execution/RA01 and Linux remain
open; public init still creates format1. Focused verification is not a full B gate.


The [X05-B2-A native shape-definition foundation](../../tests/native/v5-02-x05-b2-a-evidence/REPORT.md)
received a request-changes review for early error adaptation; the combined
B2-A+C01 correction is independently approved by the X05-B2-A-C01 review. The internal selected-profile VM
resolves registered external shape IDs to owned ID, final revision and exact
`shape_definition_v1` digest. Identity includes the final descriptor, native
external logical module path and lowercase hex of that selected module's original
captured bytes; module ordinal and unrelated modules do not enter this identity.
Each lookup recomputes with cumulative checked native charges, without executing
Lua or starting/resetting invocation budgets. Resource failures remain terminal.
Generic profile invocation stays closed. Independently approved B2-B codec accounting precedes B2-C retained
F dispatch/immutable input/complete-return transport, B3 selected identity/W and
B4 rendering remain open, as do X06–X08, execution/RA01 and Linux. This focused
foundation does not close B2 or the B milestone; public init creates format1.

The [X05-B2-A-C01 error-boundary correction](../../tests/native/v5-02-x05-b2-a-c01-evidence/REPORT.md)
includes embedded, moved-from and already-exhausted lookup refusal construction
inside the typed boundary. An actual one-shot native allocation refusal returns
`resource_limit` at `catalog.shapes`; existing sticky conversion diagnostics keep
their first message, limit and maximum. Profile owners remain terminal. Ordinary
embedded/moved-from refusals retain protocol-first behavior on retry; a moved-to
owner remains independent. This covers controlled one-shot failures, without a
persistent exhausted-heap guarantee. Identity, cumulative reservation policy and
public admission remain unchanged. The combined candidate is independently approved by the X05-B2-A-C01 review;
B2-C and the mandatory external-profile gate remain open.


The [X05-B2-B codec reservation candidate](../../tests/native/v5-02-x05-b2-b-evidence/REPORT.md)
adds optional native `codec_encode_reserved` without changing ordinary callers.
One shared validator derives checked logical graph, string, workspace and fixed
Glaze output capacity before the synchronous native charge hook. Successful
validation uses a bounded active path without heap allocation; each copied
container reserves its exact entry count. Input stability through the hook is a
trusted C++ precondition. The moved output buffer is counted once; conservative
workspace slots are not independent heap allocations or an RSS guarantee.
Focused Debug evidence and independent allocation phase probes accompany the
candidate; combined B2-B+C01 is independently approved by the X05-B2-B-C01 review. B2-C retains raw-return conversion
charges, retained F execution, immutable input and invocation integration. B2/B,
X06–X08, execution/RA01 and Linux remain open; public init creates format1.


The [X05-B2-B-C01 encode diagnostic correction](../../tests/native/v5-02-x05-b2-b-c01-evidence/REPORT.md)
is independently approved by the combined X05-B2-B-C01 review. A shared outer
allocation boundary protects the existing encode handlers' diagnostic construction,
including validation refusals and callback exception adaptation. Controlled one-shot
native diagnostic allocation refusal returns the existing `resource_limit` /
`output_bytes` diagnostic; normal precedence and moved typed callback refusals
remain unchanged. This is not a persistent exhausted-heap guarantee. Encoding,
reservation math, decode and limits are unchanged. B2-C retained F dispatch and
invocation integration, B3/B4, X06–X08, execution/RA01 and Linux remain open;
public init creates format1. Focused correction evidence does not close B2/B.


The [X05-B2-C retained shape bridge](../../tests/native/v5-02-x05-b2-c-evidence/REPORT.md) is independently approved with C01/C02; their reviews close B2C-R1 and B2C-R2.
Internal `command_vm::invoke_shape` binds owned final definition metadata before
calling the actual retained F in its original selected-profile VM. Exact normalized
five-field inputs preserve nested aliases and freeze transient proxies without
publishing permanent sealed identities. Exact pre-invocation tag/backing membership
survives cleanup after raw response conversion, including returned input aliases;
invocation scratch/result tags are released. One actual JSON-compatible return
passes separately charged raw conversion and reserved Glaze encoding with the
complete 16 MiB ceiling. Padding transport fixtures do not prove shape semantics.
Registration transitions once to cumulative invocation instructions; native/node
charges never reset. Admission protocol errors are nonconsuming; execution/resource
failures retain the first terminal original shape location. B1 Lua still owns shape
success/refusal/obligation semantics. B3 selected identity/W and B4 rendering,
X06–X08, public profile admission, execution/RA01 and Linux remain open. Generic
profile command invocation stays closed; public init creates format1. This focused
Debug slice is not a full B or release gate.

The [X05-B2-C-C01 correction candidate](../../tests/native/v5-02-x05-b2-c-c01-evidence/REPORT.md) stores transient
snapshots under private immutable address keys, separate from every integer
`luaL_ref`. Retained shipped F and external D/F bindings survive first/repeated
shape calls, GC and failure cleanup in the owned reversible observer. The original
reviewer regression is retained and reproduced. This correction preserves existing
cleanup, cumulative budgets and public boundaries; the combined C02 review independently approves the bridge. No full milestone or release gate is claimed.

The [X05-B2-C-C02 terminal diagnostic correction candidate](../../tests/native/v5-02-x05-b2-c-c02-evidence/REPORT.md) shares the profile owner's first terminal error across definition lookup and shape invocation.
A resource failure owns its original code, message, limit, maximum and field before
fallible response copying; later ID/input guards and allocation adaptation preserve
that error. Natural cumulative lookup exhaustion reproduces the prior masking and
location change, then verifies missing/wrong-kind/valid IDs, invalid inputs and
retries. Controlled one-shot native allocation probes cover ownership and copies;
no persistent host-memory guarantee is claimed. C01 private registry keys remain.
The combined independent B2-C+C01+C02 review approves this scope. The focused Debug gate adds
one regression (620 registered names, 354 focused plus 11 supplemental); no full
milestone or release gate, public profile admission, B3/B4, X06–X08, execution/RA01
or Linux acceptance is claimed. Public init creates format1.


The [X05-B3-A pure phase candidate](../../tests/native/v5-02-x05-b3-a-evidence/REPORT.md) factors existing trusted validation into structural prepare/check-one/finish exports.
Every mandatory/base-cycle check precedes custom requests; checked responses retain
B1 exact obligations, guidance and count policy. Finish retains original graph edges
and forms selected_shape only from explicitly supplied trusted request-corresponding
definition metadata. The old embedded validator and B1 shape_results adapter remain.
The [phase ABI](../../tests/native/v5-02-x05-b3-a-support/phase-abi.md) records handoff requirements.
The X05-B3-A review independently approves this pure slice. It does not prove native metadata
authenticity, immutable F input, same-VM dispatch or cumulative phase conversion;
B3-B owns those. B3-C W, B4 rendering, X06–X08, public profile admission/cutover,
mandatory external-profile acceptance, execution/RA01 and Linux remain open.
Public init creates format1; latest full approved gate remains A4 582/configuration.


The original X05-B3-B incomplete prerequisite checkpoint is preserved in
`tests/native/v5-02-x05-b3-b-evidence/REPORT.md`, including its actual request423
regression and scoped sticky correction. The R01 integration candidate below
supersedes that checkpoint's remaining accounting/lifetime status.


The [X05-B3-B-D01-A test-only decode feasibility](../../tests/native/v5-02-x05-b3-b-d01-a-evidence/REPORT.md) demonstrates
request-local allocation interception with pinned Glaze readers, including variant
rewind, duplicate-check storage, malformed partial cleanup and typed refusal
adaptation. Independent D01-A review approves the test-only boundary. No product, vendor, codec API or runtime
bytes change. Production conversion reservation is now the separate D01-B candidate; B3-B accounting/request423 and its final gate remain open. The
637-name product inventory includes WIP; last approved inventory remains 627.
No full milestone, release or Linux gate is claimed; public init creates format1.


The [X05-B3-B-D01-B production reserved decode candidate](../../tests/native/v5-02-x05-b3-b-d01-b-evidence/REPORT.md) adds optional
`codec_decode_reserved`: every actual parser/duplicate-check request charges before
allocation, followed by one checked allocation-free immutable-conversion sizing
pass and reservation before persistent ownership. Request-local Glaze wrappers
retain original grammar/error ordering; returned values use ordinary allocators.
Exact pinned control-block/tree-node/list/string recipes and the actual bounded
workspace determine capacity. Typed supplied errors move unchanged, and outer
allocation protection covers early/catch diagnostic construction in both decode
entry points. Ordinary decode API/defaults and approved encoding remain unchanged.

Independent D01-B review approves the reserved decoder prerequisite. The
X05-B3-B-R01 native integration candidate consumes both decoder hooks only while
profile semantic composition is active. Actual parser/checker allocation requests
and immutable conversion capacity charge the existing cumulative native budget
before allocation, without refunds. The inherited node and graph/transport charges
remain deliberately conservative overlapping accounting. Embedded decode keeps
its ordinary path. Semantic error-data staging also charges concrete pinned
control blocks, map nodes and copied string buffers before allocation; successful
encoded text separately charges its native wrapper while moving the already
reserved output buffer. Direct in-place construction avoids initializer-list
copies. Two narrowly scoped global-fragment libc++ headers expose only the pinned
allocation type identities; product operations retain `import std`. Reserved
encoding and first typed terminal diagnostics remain.

The combined original B3-B plus R01 candidate retains audited private phase roots
and native-owned request/result/definition values across sequential protected
same-VM prepare/F/check/finish calls. Production coverage includes recoverable JSON
refusal followed by healthy calls. Private reversible observers cover exact tag/
backing key-value membership and all16 retained references in a mixed fixture
(five shipped, three phases, four external descriptor/callable pairs), stable live
Lua heap after warmup/GC, partial baseline setup and terminal cleanup. Controlled
parser/conversion/node/Lua refusals prove no F dispatch; repeated real operations
exercise cumulative instruction/native limits. Near-budget staging controls
verify exact native error-graph requests and deny node/map/string/wrapper storage
before allocation, retaining ordinary healthy schema diagnostics. These probes supplement actual
production F and semantic execution.

The frozen candidate gate targets642 registered names (all641 prior names
preserved),376 focused Debug cases and11 supplemental history cases, production
semantics, actual allocation sampling, request423 checkpoint/current controls,
vectors, six exact source audits, pinned formatting/Doxygen and no-work builds.
See `tests/native/v5-02-x05-b3-b-r01-evidence/REPORT.md` for executed results and
limitations. The independent combined B3-B/R01 review approves this integration. No full B,
release or Linux gate is claimed; latest full approval remains A4's582/config.
B3-C W, B4 render, X06–X08, public profile admission/cutover, mandatory external
profile acceptance, execution/RA01 and Linux remain open. Public init creates
format1. Codec and original checkpoint artifacts remain unchanged.


The independently approved X05-B3-C selection adds internal native `select_profile` over original
bundle/capture/files/job/role intent. Its semantic phases run once, then a fourth
privately retained trusted phase selects from that exact finalized same-call model.
All four phase roots join the original pre-module audit/freeze; phase transport,
codec work and repeated operations share cumulative accounting and cleanup. No
caller model, shape definition or export selector is admitted. Generic profile
invocation remains closed.

Embedded `select_role` retains its original normalization and whole-bundle
validation. Shipped selection remains exact with profiles. Custom W preserves old
fields, appends `shape_selection_v1` and shape definition/revision plus synthetic-ID
sorted unit obligations where selected work is either endpoint, and hashes
`work_external_shape_v1`. Guidance is excluded. Selected source comments/revisions
change every custom-job role; separate unselected/projection modules affect only
provenance. Same-module visual source changes conservatively affect definition/W.
The independently authored minimal-fixture W and complete canonical preimage live
under `tests/native/v5-02-x05-b3-c-support/`; the candidate verification report is
`tests/native/v5-02-x05-b3-c-evidence/REPORT.md`; the B3-C review independently approves this selection scope.
This is selection infrastructure: B4 presentation, X06 history/acceptance, X07
public cutover, X08 mandatory external-profile acceptance, execution/RA01 and Linux
remain open. Public init creates format1; no full B/release gate is inferred.

Selected-profile hash work additionally reserves conservative pinned-library
cumulative string growth, owned source/domain copies, encoder scratch and digest
storage before allocation. These byte charges do not add converted-node counts.
Actual allocation probes retain typed first resource refusal; embedded digest
behavior remains unchanged. The derivation and private aggregate measurements are
recorded in `tests/native/v5-02-x05-b3-c-support/hash-accounting.md` and the C report.

The X05-B4-A candidate factors the shared trusted render body and adds the private
`render_profile_material({request,model,selected})` helper. Its caller must supply
authentic finalized semantic and selection results; logical job/role/shape identity
checks do not authenticate arbitrary tables. The helper normalizes render intent,
reads retained observations, and renders without validation, selection, F dispatch
or mutation of model/selection. Custom shapes add exact three-field identity,
full scoped unverified prerequisites and mandatory `shape_policy` after dependencies;
shipped shapes preserve the old envelope and presentation. B4-A verifies pure helper
composition only. Native same-call retention, immutable transport, cumulative full
render accounting and lifecycle proof are delivered by the B4-B candidate below; the full Debug/release B gate
remains B4-C. Public profile CLI, X06–X08, execution/RA01 and Linux remain open;
public init creates format1. Candidate evidence is
`tests/native/v5-02-x05-b4-a-evidence/REPORT.md`; independent B4-A review requested
the correspondence correction recorded below.


The X05-B4-A-C01 corrective candidate requires complete positional one-to-one
bundle/semantic job correspondence before reading any selected shape. Both arrays
must have equal lengths, each row must carry a valid matching job ID, and IDs
must be unique across the complete mapping. Reordered, missing, extra, duplicate,
ambiguous and malformed identities refuse with the existing schema diagnostic at
`render.selected`; there is no ID lookup fallback, revalidation or input mutation.
Two new production cases reproduce same-shape/different-guidance substitution and
exercise selected/unselected rows and malformed containers. The actual pre-edit
baseline and red regression are retained in
`tests/native/v5-02-x05-b4-a-c01-evidence/REPORT.md`. B4-A's independent review
requested this correction (B4A-R1); the combined B4-A+C01 review independently approves the correction and closes B4A-R1.
Pure helper correspondence does not establish native same-call authenticity;
Combined B4-B+C01 native composition is independently approved; B4-C integrated evidence awaits independent review. Public init
still creates format1; X06–X08, public cutover, execution/RA01 and Linux remain open.


The X05-B4-B candidate adds internal native `render_profile` over original
bundle/capture/files/job/role intent and optional budget. A trusted normalization-only
preflight preserves full render schema then budget-stripped selection schema
precedence before any semantics or F call. Six fixed trusted roots seal before
external modules. Native-owned normalized intents, same-call finalized model and
selection flow through sequential protected calls to the approved renderer; each
custom job's F runs once. Existing selection behavior and public generic profile
refusal remain. All phases, observation/presentation codecs, hashes and a final
reserved complete-response encoding share cumulative limits; final output is
bounded by the existing 16 MiB codec ceiling and lower role/render budget.

B3-C and combined B4-A+C01 are independently approved. combined B4-B+C01 is independently approved; B4-C records the complete Debug/RelWithDebInfo gate candidate. Focused Debug
coverage, exact root/reference and frozen-membership GC observations, preflight
zero-F and valid once-per-job dispatch, actual native allocation refusals and
controlled transport failures are recorded in
`tests/native/v5-02-x05-b4-b-evidence/REPORT.md`; allocation derivation is in
`tests/native/v5-02-x05-b4-b-support/transport-accounting.md`. This candidate adds
no provider or public profile CLI, projection dispatch or storage interpretation.
X06–X08, mandatory external-profile acceptance, public cutover, execution/RA01 and
Linux remain open; public init creates format1. Latest full approved gate remains
A4's 582 per configuration.


The X05-B4-B-C01 correction candidate routes every ordinary native render
response, including normalization, preparation, shape-check, finalization and
selection errors, through the same existing reserved complete-response encoder.
The unchanged 16 MiB output ceiling and cumulative native charges apply before
returning the original native response pointer. Validation-only/selection-only
transport and native unexpected failures retain their prior paths. The genuine
oversized original unknown-key schema regression now returns terminal typed
`output_bytes`/16777216; small ordinary errors retain their bytes and recovery.
Production phase/lifetime controls and reversible exact writer/phase/F observers
supplement focused Debug verification. The combined B4-B+C01 review independently approves this correction and closes
B4B-R1. B4-C records the separate integrated macOS gate; Linux remains open.
Public profile admission, X06–X08, mandatory external-profile acceptance,
execution/RA01 and Linux remain open. Public init creates format1; latest full
approved gate remains A4's582 cases per configuration.


## X05-C1 candidate checkpoint

The private pure section-assembly candidate is extracted within existing
`render.lua`, with ordinary rendering passing an empty custom list and a separate
exact projection-metadata adapter. The [focused Debug report](../../tests/native/v5-02-x05-c1-evidence/REPORT.md)
is pending independent review. This slice does not close X05-C or admit external
projection responses/callables/read services. Native admission/dispatch, initial
receipt checks, operational read/project services and final C integration follow
in the approved sequence. Public profile CLI, X06–X08, execution/RA01 and Linux
remain open; public init still creates format1.


The X05-C2-A candidate adds internal native projection-definition identity lookup
without executing the retained projection callable. It hashes the exact final
five-string descriptor, logical external owner without ordinal, and original
privately captured selected module bytes under `projection_definition_v1`.
Owned ID/revision/digest results survive VM destruction. Unknown IDs refuse
nonterminally; resource failures retain the shared first owner error across shape,
projection, validation, selection, render and moves. Prospective graph, string
copies, cumulative canonical growth and encoder scratch charge existing native
limits before allocation; no invocation starts or budget resets.
Focused Debug evidence is in `tests/native/v5-02-x05-c2-a-evidence/REPORT.md`;
fresh independent review is pending. Retained projection execution/native result
admission remains C2-B. Read services, capability/receipt orchestration, public
profile admission, X06–X08, execution/RA01 and Linux remain open. No full C or
release gate is claimed; public init creates format1.


The X05-C2-B1 candidate adds only a native exact projection JSON size prerequisite
in `engine/json_codec`: stable borrowed section views, complete input preflight,
one synchronous charge for fixed 6,325,248-byte scratch and actual fixed writer
workspace, and pinned Glaze serialization without an AST copy or retained output.
Encoded overflow is sticky and nonthrowing; bounded writing completes before
refusal. Native invalid input, encoded ceiling, resource and scratch/internal
failures remain distinct typed outcomes. Callback diagnostics move unchanged;
independent nested/concurrent owners share no routing state. The operator's global
18-name built-in reservation is now explicit in X-PROJ for both input kinds;
semantic reserved-name admission remains the later native scanner's duty.
Evidence is `tests/native/v5-02-x05-c2-b1-evidence/REPORT.md`; fresh independent
review is pending. This prerequisite does not execute F, scan/root Lua results,
open profile invocation or implement read services, public CLI, X06–X08,
whole-invocation orchestration, execution/RA01 or Linux. Public init creates
format1; no full or release gate is claimed.


C2-B2 adds the internal typed retained-projection bridge and native raw-result
admission candidate. Descriptor kind and immutable structural transport are checked
before one retained F call; callers own authenticated selected-role/history data.
The protected scanner enforces exact version1 section schema, the shared GLOBAL18
reserved union and raw limits before borrowed-view exact encoded counting and
native result copying. Original projection indices and shared first terminal errors
survive retries and moves. This candidate awaits fresh independent review; native
read services, rendering composition, CLI/provider/receipt integration and the
mandatory external-profile gate remain open. See
[the C2-B2 evidence](../../tests/native/v5-02-x05-c2-b2-evidence/REPORT.md).
