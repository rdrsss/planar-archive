# Planar rewrite implementation plan

X05-C4-B1 and C4-B2 are independently approved native read foundations.
The C4-B3 candidate runs an admitted history-only retained handler with lazy
`context.read_history({})` bound synchronously to its original moved capture,
ledger receipt and private exact selector. No ledger contents are read if the
handler returns a semantic error without calling the service. The original B2
helper closes every native snapshot, transaction, connection and descriptor
adapter before any Lua result allocation or execution resumes.

The immutable invocation exposes an opaque ledger slot and exact ID strings,
scalar defaults and presence flags. The service is once-only; failed service,
invalid arguments, repeated calls and missing declared-service access poison the
invocation even when caught. Execute preserves original typed registration,
format and history errors. History success must match the retained native facts
exactly; raw comparison precedes owned response copying. Original receipts are
reverified before release. Pure handlers retain their summary/error protocol.
Manual native history preparation cannot bypass handler service sequencing.
This candidate awaits fresh independent review; focused Debug evidence is in
`tests/native/v5-02-x05-c4-b3-evidence/REPORT.md`.

Role read, projection service and capability combinations beyond history-only
remain closed. X06 recorded profile interpretation/acceptance and shared
invocation accounting, X07 public cutover, X08 mandatory external-profile
acceptance, execution/RA01 and Linux remain open. Public init creates format1.
The latest complete approved gate remains 676 per configuration; no new full or
release gate is claimed. Earlier evidence retains its historical scope.

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

Status: R01–R09 are complete and independently approved. The replacement CLI
renders worker, reviewer and recovery packets from captured inputs. Engine naming,
product Lua under `src/`, Glaze serialization and CLI11 handlers are restored and
independently reviewed. R10 presentation feedback is resolved by approved
R10-P01. R11 ledger/history design and R12 native storage foundation are
independently approved. R13 record/tree/provenance and interpretation are complete
and independently approved. R14 native acceptance, history CLI and export CLI
components are independently approved. Current commands are render, init, history,
export and load. Load membership is declaration-closure-only;
D7-B and D8 are independently approved for the applicable R14 portions. The approved C checkpoint
reports 370/370 debug and release tests on macOS ARM64/LLVM23.1.4; no Linux
result or public load coverage is inferred. V5-02 A/B/C are operator-accepted. The [V5-02-A exact contract](command-contract.md)
is independently approved; V5-02-B1 is approved with C01, and V5-02-B2 CLI
composition and [V5-02-C workspace naming/invocation snapshots](rounds/V5-02-C-review.md)
are independently approved. The [R14-D3 load amendment](load-command-contract.md)
is independently approved in [D3 review](rounds/R14-D3-review.md).
R14-D4 typed native diagnostics are independently approved. The required external-profile gate follows
full R14 and precedes dependent jobs/RA01. See the
roadmap for round status and [v5 alignment](v5-alignment.md) for the assessment.

## Authority and working arrangement

The operator approved a complete, breaking rewrite with no migration or upgrade
path from existing Planar versions. Work uses the current checkout on
`dev/planar-v2-vernacular`. The operator has stopped other Planar work; this
checkout is the shared implementation directory. No additional worktree is
required by default.

The product-direction baseline is [the v5 design spec](planar-design-spec-v5.md).
Explicit operator decisions take priority; approved exact declaration, render and
ledger contracts remain authoritative until scoped reviewed amendments. V5's
illustrative CLI/registry/constructor examples do not change supported APIs.
[Historical v4](Planar%20design%20spec%20-%20v4.md) remains unchanged for provenance.
This plan and the [roadmap](roadmap.md) record implementation scope, sequencing,
and the operator's subsequent decisions. Historical legacy coexistence and
rollback requirements are superseded by the approved clean rewrite. Git retains
the old implementation. No legacy database conversion, compatibility CLI, or
permanent legacy source tree is required. This does not authorize deleting an
operator's installed binaries or data.

Planning and orchestration live in this directory and in the coordinating
session. Do not invoke existing Planar tools for tasks, claims, heartbeats,
planning, documentation bookkeeping, build coordination, or finalization. The
operator's instruction supersedes the old repository workflow requirements.
Builds and tests run directly and serially per build directory. Before code work,
update the repository instructions to make this exception durable and remove
instructions that would route agents back into the old operational system.

Retain applicable engineering conventions: C++ modules, explicit error
boundaries, pinned vendored dependencies, meaningful tests, and signed-off
commits when commits are made. The current task does not authorize publishing,
installing, or merging the replacement.

## Product boundary

Planar compiles restricted Lua declarations and captured observations into scoped
role context. Trusted Lua owns schemas, shapes, input selection, materiality and
projections. A small native engine owns isolation, captured file access,
canonical encoding, hashing, native ledger transactions, recorded reads and exact
source export. Finite execution evidence remains a later cut. An external driver
owns sessions and scheduling.

The first slice of implementation work was render-only: one spec, one source
root, manually selected job grouping, and explicit observation inputs. It did
not create a ledger. Current `init`, `history` and `export` commands operate on
explicit new-format ledger paths; render remains database-free. Native prepared
acceptance and atomic transactions are implemented. The approved load service/CLI
populates accepted history; D8 closes applicable R14 acceptance portions. The product does
not claim work, execute checks, call models, schedule sessions, manage worktrees,
or install a replacement over existing Planar.

## Agent protocol

The coordinator scopes rounds, dispatches agents, maintains this plan and the
roadmap, and checks evidence. Implementation and independent review use fresh
subagents for each round.

| Role | Model | Reasoning | Responsibility |
| --- | --- | --- | --- |
| Search | `gpt-6-luna` | high | Read-only repository mapping for the exact round |
| Coder | `gpt-6.1-sol` | medium | Implement the assigned scope and verify it |
| Reviewer | `gpt-6.1-sol` | high | Independently review changes and acceptance evidence |

The operator explicitly approved `gpt-6-luna` with high reasoning in place of the
initially requested, unavailable `gpt-6.1-luna`. Do not substitute other models
silently. Use ordinary subagents with explicit briefs; existing Planar-managed
agent roles and skills are not this rewrite's orchestration mechanism.

Each round follows this sequence:

1. The coordinator records the objective, dependencies, owned files, acceptance
   criteria and verification commands in a round record.
2. A fresh search agent maps relevant source, build wiring, tests and hazards.
   It cites paths and symbols and identifies what can be reused or removed.
   It does not implement changes. Its report becomes part of the round record.
3. A fresh coder receives the bounded brief and search report. It implements in
   the shared checkout, runs the agreed checks, and reports the exact changes,
   results and limitations. It may not approve its own work.
4. A fresh reviewer examines the candidate diff and raw evidence against the
   acceptance criteria. It may run verification but does not implement fixes.
   It returns `approve`, `request-changes`, or `blocked`, with cited findings.
5. On `request-changes`, start a fresh corrective round with new search, coder
   and reviewer agents. Carry forward the findings and existing candidate;
   do not erase previous evidence. Search can be narrow but is not skipped.
6. On `approve`, the coordinator verifies that the approved candidate is still
   the current candidate, records completion, and proceeds to the next round.
   A subsequent edit affecting that scope requires renewed review.

Completed agents are not reused for later rounds. Once their reports are saved
and the round disposition is recorded, stop any remaining activity and retire
them from dispatch. If the harness offers termination, terminate them at that
point; otherwise leave completed agents idle and never redispatch them. Do not
claim a termination operation occurred when the harness only supports stopping
activity.

Use one search agent, one coder and one reviewer sequentially by default. The
coordinator plus these roles fits the available four slots. Parallel coding
requires a documented reason and disjoint file ownership; it is not assumed.
Every brief states that the checkout is shared, names permitted writes, and
forbids reverting another agent's changes. Fresh agents receive a self-contained
brief rather than depending on inherited conversation context.

Review approval advances ordinary rounds without repeated operator permission.
Unresolved product choices return to the operator. At the render evaluation
gate, inspect the actual fragments with the operator before extending into
persistence. Planning approval is not approval to skip that evaluation.

## Durable work records

Use these repository files:

- `implementation.md`: product boundary, architectural decisions and protocol.
- `roadmap.md`: ordered rounds, dependencies, status and acceptance gates.
- `declarations.md`: exact workspace/spec/job constructors and validation rules.
- `render-contract.md`: render invocation, observations, identities, diagnostics
  and budget behavior.
- `command-contract.md`: V5-02-A exact embedded registry/services and workspace
  selector target, with separate implementation and external-profile gates.
- `ledger-contract.md`: independently approved new-format declaration/history storage,
  acceptance, replay and exact export contract; applicable R14 integration is approved through D8.
- `load-command-contract.md`: R14-D3 approved exact load ABI, replay/transport,
  typed native diagnostics and bounded follow-on slices; independently approved in D3.
- `v5-alignment.md`: retained/new/pending direction, conflicts and contract gates.
- `acceptance-matrix.md`: every v5 matrix row, current evidence and remaining owner.
- `rounds/<id>.md`: brief, search findings, implementation evidence, reviewer
  disposition and any unresolved questions for one round.

The last three surfaces are created as their rounds begin. Do not fabricate
completed evidence or reviewer approval. A round record identifies its starting
revision and existing working changes, candidate files, executed commands and
results, and reviewer findings. Until commits exist, preserve a candidate diff
identity or equivalent exact snapshot reference so later edits cannot be
mistaken for the approved candidate.

## First slice: declarations and useful packets

Finalize a real constructor API before native implementation. Specify required
and optional fields, defaults, schema versions, stable IDs, prose/file values,
materiality wrappers, supported dependency combinations, ownership semantics,
review separation, checks and toolchain descriptors. Specify the supported path
grammar rather than accidentally promising arbitrary glob overlap analysis.

Author the quota fixture with role-independent `library` and `command` work
units, their contracts, acceptance requirements, path boundaries and dependency.
Both jobs use byte-identical spec inputs:

- Combined: one builder implements both units; an independent reviewer reviews
  both.
- Split: separate library and command workers; an independent reviewer reviews
  both.

The fixture referenced by v4 is absent at its linked location in this checkout.
Search for an available copy during the first round. If unavailable, author the
new fixture from the specified behavior and label its provenance accurately.
Do not treat an absent historical trace as executed evidence.

Write expected worker, reviewer and recovery packets before building the native
pipeline. Initially these are reviewed examples; they become executable goldens
when the renderer is implemented. Include identity-manifest examples with a
specified format; do not invent computed digest values as native test vectors.

Reviewer packets consume explicitly captured output observations with exact
target identities. Before outputs exist, report their absence. Render-only
packets do not assert that checks passed or approval was granted. Recovery
examples carry a bounded checkpoint and labeled observations, explicitly state
that no run is active, and make no claim of durable execution history.

## Native render implementation

Implement a new thin `planar` command with no dependency on the old planning
database. Finalize command syntax in `render-contract.md`; example syntax is
`planar render --workspace <path> --job split --role library_worker`.

Capture declaration and referenced-file bytes before projection. The same
captured set drives validation, identity and rendering; projections perform no
fresh filesystem reads. Logical root/path identities must be distinguished from
host checkout locations. Define deterministic ordering and interpretation
versions explicitly.

The native loader exposes only the curated declaration surface. Bound memory,
instructions, input size and conversion depth. Refuse forbidden capabilities,
unsupported values, mutable trusted tables and bytecode loading. Verify confined
reads, traversal and symlink behavior now, because render already reads files.
Do not defer native security checks to the later execution cut.

Canonical encoding distinguishes maps and arrays, including empty values, and
pins string, integer, ordering, absence and invalid-value behavior. Specify and
test seed-zero XXH3_128bits output representation and encoding/hash versions.
Native golden vectors are separate from any fake-engine identities.

Trusted Lua validates and resolves the graph, derives ownership and requirement
coverage, selects material inputs and renders role packets. Job configuration
cannot drop mandatory checks or broaden ownership. Grouping dependent units
preserves internal obligations without creating role self-waits. Reject actual
unsatisfiable phase dependencies with actionable diagnostics.

Budget handling counts bytes exactly, labels any token estimate, and never
silently truncates mandatory context. Optional omissions follow declared
priorities and appear in the output. Errors distinguish syntax, schema,
capability, resource, filesystem and budget failures.

## Repository destination

The current approved command composition has this structure:

```text
src/cmd/planar/                 thin CLI, bootstrap, response,
                               handlers/{render,init,history,export}/
src/engine/{value,lua,capture,canonical,hash,runtime,json_codec}/
src/engine/{ledger,history,acceptance,export}/
src/stdlib/planar/              embedded schema, semantic, selection, observations,
                               presentation, render, acceptance, JSON and bootstrap Lua
fixtures/render/quota/          declarations, source, observations and goldens
tests/{stdlib,native}/          test-only Lua/harnesses and CLI acceptance evidence
docs/vernacular/                implementation planning and round evidence
docs/                          current product reference
cmake/, vendor/, scripts/       replacement build and verification support
```

Native unit tests are colocated with their modules. R09 removed legacy sources,
migrations, operational commands and distribution scripts; Git retains them.
There is no permanent legacy source tree or current `src/lib/` layer. Future
engine and trusted Lua additions remain under `src/`, with CLI11 command
families under `handlers/<family>/`. Implemented ledger/history/acceptance/export
boundaries and exact format are in [ledger foundation v1](ledger-contract.md).
Do not reintroduce legacy openers, implicit database paths or automatic migration.

Review reusable libraries and dependencies individually. Lua 5.5.0 is already
vendored in this checkout; the historical inventory in v4 is not a current source
map. Existing sandbox or process code is useful evidence, not automatically a
trusted implementation. Remove unused dependencies and obsolete installation
paths only after checking references. Do not leave a runnable installer that
ships the wrong product or implies an upgrade path.

## Render acceptance gate

Required evidence includes fresh-process deterministic renders, canonical/hash
vectors, native isolation tests and the retained render cases A01–A05, A07–A08,
A30–A32. A05 covers separate output identity at this gate; execution evidence
invalidation remains a later obligation. Add confined-read cases now even though
the broader A18 boundary-check case belongs to execution.

For both groupings prove complete requirement/check coverage, exclusive path
ownership, independent review and correct dependency resolution. Changes to
scoped material inputs move only applicable fingerprints. Optional prose and
checkpoints change renders without changing work identity. Own implementation
edits change output identity without changing producer input identity.

Run targeted checks per round and one complete replacement gate on the integrated
render candidate. Record selected test counts and reject zero-match success.
The obsolete product's full gate is not automatically the replacement gate;
define the new gate during build bootstrap and document its coverage.

Finally, evaluate the actual worker, reviewer and recovery packets with the
operator. Passing deterministic tests alone does not establish useful context.
Persistence work begins only after that evaluation resolves required changes.

## Next slice: accepted declarations and recorded history

R10 evaluation and the independently approved R10-P01 presentation correction
satisfied the operator's request to improve presentation before continuing.
R11 is the independently approved documentation design for [ledger foundation v1](ledger-contract.md),
not an implemented persistence claim. It bounds R12 dependency/schema/transaction
work, R13 immutable complete trees and interpretation records, then R14 finite
acceptance/history/export. Existing render remains read-only and database-free.

The implemented native ledger foundation has an explicit new format, complete captured declaration
closure, exact trusted registry/stdlib provenance, versioned interpreted state,
chronological revisions distinct from immutable content IDs, append-only typed
events and atomic current-state projections. Exact no-op retains the revision;
a revert after changes creates a new revision that may reuse an old tree.
Explicit request parameters identify an original intent: replay returns its saved
result before capture/CAS, while a fresh ID is required to accept new draft bytes.
Digests index exact records; unequal equal-digest candidates refuse atomically.

These concrete decisions implement v4 A20, storage/format parts of A23, and
A25–A29 without prematurely adding execution state. Later contracts separately
cover role generations, runs/fencing, refresh/recovery, checks and review/repair;
only their integrated gate enables observed-state execution. Accepted-revision
rendering and basic draft status require R15 read-workflow design; rich impact and
render roadmap remain post-R19 composition work.
No legacy migration, installation, publication or merge is authorized.


## V5 continuation gates

The [alignment assessment](v5-alignment.md) preserves the existing foundation:
load, runs/fencing, checks and review were already planned. V5 strengthens Lua verb
ownership, workspace naming, authoring/observe delivery, reference-orchestrator
rendering and full-matrix traceability; none is claimed implemented by this prose.

The ordered path is V5-02-A exact contract review, V5-02-B1 sealed registry/runtime,
V5-02-B2 CLI11 families and finite native services, V5-02-C workspace naming,
approved R14, mandatory external-profile V5-02-X, RA01 authoring and driver
prompts, then R15–R19. A/B/C were [accepted by the operator](rounds/V5-02.md#operator-decision);
no repeat permission is required. The [command contract](command-contract.md#required-implementation-and-verification-sequence)
assigns bounded destinations and adversarial acceptance to each embedded slice.
Each receives fresh search/coder/reviewer agents and exact candidate evidence.
The sealed embedded catalog and render names are independently approved through C;
D7-B adds the fifth command, load, under the independently approved D3 amendment.
D4 typed diagnostics, D5 capture, D6/C01 binding, D7-A receipts, D7-B public load
and D8 applicable acceptance closure are independently approved. Native
CLI11/Glaze/storage invariants remain fixed; registered Lua must execute semantics.
External profiles require a separate envelope parser spike, versioned request/
provenance/interpretation/format coexistence contracts and implementation before
extensions become usable. No ambient discovery or unreviewed v2 codec is current.
Keep product C++/Lua under src and engine naming.

RA01 follows completed R14 and mandatory external V5-02-X without
renumbering historical rounds. Its contract
must settle spec/critique/job prompts, typed rejected-load field-path prompts,
existing ledger-init coexistence and scaffold refusal, explicit-dir observe
assembly/schema/limits, and useful orchestrator graph/shape guidance. No watcher,
model calls or Planar-managed checkout lifecycle enters the product. The driver
owns forwarding observations and session interruption. Orchestrator actions/state
grow with actual execution APIs and receive live driver verification in R19.

The [acceptance map](acceptance-matrix.md) owns A01–A34 plus local authoring IDs
A35/A36. Cut 2 closes all applicable Render/Execution portions, conditional A24
assertion/refusal policy and minimal A34 execution provenance. R15 must contract
that provenance before runs; full experiments and metrics remain cut 3. Execution
reconciliation must track semantic dependency changes, output applicability and
prerequisite availability separately, preserving valid evidence when blockers clear.
After R19, composition adds imports/exports, multi-root, scoped reload, impact and
roadmap rendering; measurement follows separately. No foundation-only evidence
closes dispatch, role reconciliation, approval or full historical trajectory rows.

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


## X05-C1 private pure assembly candidate

C1 shares the existing renderer assembly algorithm with a private projection
adapter in the same Lua source. Native callers must supply authentic prepared
base sections, separately admitted custom records and a validated budget. The
helper preserves base bytes/mandatory identity, appends custom headed bodies,
copies inclusion state and removes custom optionals in reverse result order
before the existing shipped priorities. Ordinary manifests retain their fields;
projection metadata is exactly `{id,bytes,included}`. No new embedded module or
source-count reservation is introduced. The [report](../../tests/native/v5-02-x05-c1-evidence/REPORT.md)
records focused Debug verification pending independent review. Native admission,
retained F dispatch, read services and receipts remain successor scope.


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
