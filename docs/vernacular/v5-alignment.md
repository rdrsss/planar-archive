# V5 direction assessment and alignment

Current rewrite status is the [roadmap opening](roadmap.md#current-status).
Superseded status checkpoints that formerly opened this file are preserved in
[history](history.md#former-opening-of-v5-alignmentmd).

Status: V5-01 approved; V5-02 A/B/C accepted. V5-02-A exact contracts are
independently approved; B1 is approved with C01 and B2 CLI composition is
independently approved. [C workspace naming/invocation snapshots](rounds/V5-02-C-review.md)
are independently approved; D7-B public load and D8 applicable R14 acceptance are also approved. The [R14-D3 amendment](load-command-contract.md)
is independently approved in [D3 review](rounds/R14-D3-review.md).
R14-D4 typed native diagnostics are independently approved.
The [v5 input](planar-design-spec-v5.md) is preserved unchanged, SHA256
`c3d37e0e28407a9066eb51c515982ef6c93d82795eb56f97dea4a780b66fe08e`.
This assessment compares it with [historical v4](Planar%20design%20spec%20-%20v4.md),
the current exact contracts and approved checkpoint evidence. It does not amend
runtime APIs or authorize installation, publication, merge or operator-data changes.

## Authority and current evidence

V5 is the product-direction baseline. Explicit operator decisions recorded in
[the implementation plan](implementation.md) and [agent guide](../../CLAUDE.md)
remain higher priority. Approved [declaration v1](declarations.md),
[render v1](render-contract.md) and [ledger v1](ledger-contract.md) specify current
supported behavior until an explicitly scoped, independently reviewed amendment.
V5's examples and conceptual engine bindings do not add supported APIs.

The native foundation remains useful. R13 is complete, with
[its integrated review](rounds/R13-C2-review.md). R14 has native acceptance,
verified reads and exact export; current commands are `render`, `init`, `history`
and `export`, plus independently approved `load`. Render
is database-free, `init` creates an empty explicit ledger, and load composes native
acceptance with retained same-VM policy. Declaration-closure-
only load membership remains selected. [D2 review](rounds/R14-D2-review.md) approves
selected-file ancestor receipts and reports 337 debug tests on LLVM 23.1.4 with
zero skips. [G](rounds/R14-G.md) is the earlier 331-test release checkpoint on the
prior compiler; it is not release coverage of D2. C supersedes that checkpoint
with 370/370 debug and release tests on macOS ARM64/LLVM23.1.4, zero skips;
D8 supersedes C with 402/402 debug/release passes and public load coverage. Linux execution remains unverified.

## Source-linked dispositions

| Topic and v5 source | Disposition | Consequence |
| --- | --- | --- |
| [Scope](planar-design-spec-v5.md#scope), [trust](planar-design-spec-v5.md#architecture-and-trust) | Retained direction | Driver owns models, sessions, scheduling, checkouts and interruption. Native engine enforces isolation/capture/storage invariants; trusted Lua owns schemas, shapes, selection and projections. No foundation restart. |
| [Identity](planar-design-spec-v5.md#identity-and-materiality), [history](planar-design-spec-v5.md#declaration-history-in-sqlite) | Already implemented foundation, with execution remainder | Keep XXH3-128, versioned canonical bytes, exact collision refusal, complete immutable trees and separate exact/interpretation provenance. Run history still needs execution contracts. |
| [Runs](planar-design-spec-v5.md#runs-states-and-interruption), [checks](planar-design-spec-v5.md#checks-ownership-and-observed-completion), [review](planar-design-spec-v5.md#review-and-acceptance) | Already planned in v4; reaffirmed and made more explicit | Continue R15–R19 with tokens/generations/fencing, atomic checks and review/repair. These are not newly discovered scope or implemented by native acceptance alone. |
| [Authoring and driver channel](planar-design-spec-v5.md#bootstrap-and-external-driver), [open items 6–7](planar-design-spec-v5.md#open-items) | New scheduling obligation and clarified behavior | Insert RA01 between R14 and R15 for spec/critique/job/rejection prompts and explicit-directory observe; the exact text is the [authoring contract](authoring-contract.md) (RA01-A, pending review). Reverse prompting already appeared in v4; v5 makes delivery a dedicated round. |
| [Lua verbs and extension tier](planar-design-spec-v5.md#engine-surface-and-sandbox) | Accepted boundary; A/B1/C01/B2/C independently approved | [Command contract](command-contract.md) pins retained Lua semantic dispatch and CLI11/native invariants for B1/B2 before load; external V5-02-X remains mandatory after R14. No host extension loading exists now. |
| [Workspace naming](planar-design-spec-v5.md#workspace-loading-and-persistence) | Accepted exact selector policy; C implementation independently approved | [Workspace contract](command-contract.md#workspace-selector-and-environment) pins file/name/env and lexical-versus-physical semantics for V5-02-C; C implements render names and private path preparation, with public load replay independently approved in D7-B/D8. |
| [Reference orchestrator](planar-design-spec-v5.md#bootstrap-and-external-driver) | Strengthened deliverable; current projection gap | RA01 designs/renders graph and shape guidance ([authoring contract](authoring-contract.md#ra-7-orchestrator-projection-q12-a)); execution rounds add live actions/state incrementally; R19 proves an actual driver fragment with live APIs. No orchestrator projection is claimed today. |
| [Cut gates](planar-design-spec-v5.md#build-order-and-acceptance-gates), [matrix](planar-design-spec-v5.md#adversarial-acceptance-matrix) | Strengthened traceability | Track every row in the [acceptance map](acceptance-matrix.md), splitting native, command and execution portions. Full applicable gate remains ahead. |
| [Composition](planar-design-spec-v5.md#deferred-phase-workspace-composition), [measurement](planar-design-spec-v5.md#cut-3-measure-specific-claims) | Deferred | After R19: composition including impact/roadmap; later controlled experiments, metrics and tournament. No solver/caching project before evidence. |

## Conflicts and illustrative material preserved in v5

These findings describe the unchanged input, not silent corrections to it.

- The declared-model Toolchain row says workspace-written commands; the glossary,
  check section and decision table specify a driver-supplied trusted registry.
  Current exact registry v1 remains authoritative: workspace allow-lists IDs,
  jobs select descriptors, commands are not job-tier capabilities.
- “Never consults … the current directory” conflicts with current explicit relative
  host paths resolving from invocation cwd, and with v5's naming convenience.
  No ambient discovery is intended; exact distinction, lexical and physical path
  handling, environment resolution and initialization belong to V5-02.
- The conceptual single positional verb/Lua dispatch table and `stdlib.d/` differ
  from current native CLI11 registration and embedded trusted Lua. CLI11 parsing,
  thin `handlers/<family>/command.*`, Glaze transport and product code under `src/`
  are explicit operator requirements. V5-02 must design the bridge, not remove them
  or add a shadow parser. Native conditional transactions/capture checks remain
  unbypassable even when Lua supplies policy.
- The bootstrap calls impact and render roadmap cut-2 extensions, while the
  concrete deferred-phase section places them after R19. The roadmap follows the
  latter sequencing; basic pending-draft status remains R15. Rich incremental
  impact/composition is deferred, not omitted.
- Optional stronger execution mentions managed worktree lifecycle, but the scope,
  workspace and decisions forbid Planar managing checkouts. The operator's
  prohibition wins. Driver-owned views and separately justified isolation adapters
  remain possible; no Planar checkout-lifecycle round is scheduled.
- “Full matrix” cannot mean running deferred composition/measurement early: v5's
  precise cut-2 gate is every applicable Render and Execution obligation plus A34
  execution provenance. A24 is conditional on configured approval policy: disabled
  policy must refuse unsupported assertions, enabled policy must test exact-target
  invalidation. A21 and controlled A34 experiments remain later gates. This is the
  alignment plan's reading of v5's stated gates, not a new operator approval.
- The hash decision says “no dedicated matrix” despite A27. Retain A27 and its
  native collision evidence; prose cannot remove the adversarial obligation.
- R13 “in progress”, the 124 render-gate count, the old source map and remaining-
  native-foundation open item are historical or stale status. Use current evidence
  above. Example ID spellings, map-shaped registry/checks, abbreviated constructors,
  positional CLI and `render orchestrator` are illustrative. Exact schemas and
  separate-token CLI stay unchanged. The historical external fixture does not prove
  native execution; current quota computed packets prove only their stated scope.

## Ordered delivery and decision gates

1. **V5-02 — accepted decisions to exact contracts and implementation.** A/B/C
   are [operator accepted](rounds/V5-02.md#operator-decision). Review the exact
   [V5-02-A contract](command-contract.md), then implement B1 sealed runtime/
   registration, B2 real Lua semantic dispatch through CLI11 native families and
   C workspace names. Preserve Glaze, option lifetime, native capture/storage,
   parser/help/errors, existing packet bytes, replay-first and no Lua under write
   lock. The contract pins destinations and adversarial gates; no repeat operator
   choice is required. These remain targets until code/review evidence exists.
2. **R14 approved through D8.** Closure-only public load, replay-before-capture/CAS
   and applicable failure/history/export acceptance are independently approved.
   Final debug/release gates pass 402/402 each with zero skips on macOS ARM64.
   Preserve documented recovery/platform limits; no role reconciliation is inferred.
3. **External V5-02-X, then RA01 — Authoring and driver prompts.** After R14,
   complete the mandatory explicit-profile gate (CLI11 envelope spike, captured
   ordered modules, trust/capabilities, new versioned intent/provenance/interpretation
   and format coexistence, removed-profile replay). No current v2 codec is implied.
   Before dependent jobs/RA01, prove real external dispatch without overrides or
   discovery. Then pin spec prompt, critique
   prompt and ingest job prompt to exact v1 schemas, allowed check IDs and shapes.
   Specify rejection wrappers naming field paths while preserving raw typed
   diagnostics and refusal exits. Resolve `init` prompt/scaffold behavior alongside
   existing empty-ledger init before code; never overwrite authored files. Specify
   observation assembly schema, file limits and trust/extension policy. Observe
   reads an explicitly named directory only when invoked by the driver; no watcher
   or autonomous scanning. Design/render orchestrator graph/shape guidance using
   available-command labels; future actions must remain visibly unavailable.
4. **R15–R18.** R15 adds accepted-revision reads, draft status, role generations,
   ownership, request/packet atomicity, fencing and minimal execution provenance.
   R16 adds refresh, drift, retained recovery, release/interruption. R17 adds checks,
   boundaries, observed completion and preserved-evidence blocker rederivation.
   R18 adds exact-target review, atomic repairs, caps and conditional approval.
   Each begins with reviewed schema/protocol and grows the orchestrator projection
   only as corresponding commands/state become real. Explainability and incremental
   dependency rederivation must distinguish semantic/work changes, output-vector
   applicability and prerequisite availability, not blindly stale downstream work.
5. **R19.** Prove the single-root reference driver with actual APIs and usable
   orchestrator fragment, both job groupings, restart/fault recovery and historical
   trajectory inspection against accepted revisions. Close the entire applicable
   [matrix](acceptance-matrix.md), not merely the happy path or a 24-hour session.
6. **After R19.** Composition/imports/exports/multi-root, scoped reload, impact and
   roadmap render; then separate controlled compaction/grouping/model experiments,
   metrics and tournament. Design extension contracts before implementation.

A34 needs a bounded provenance data contract before execution: spec/baseline,
job/view/ledger/trial identity, requested and actual driver/model settings and
outcomes, with honest unknowns. R15 designs its minimal records, R16–R18 populate
applicable events and R19 verifies them. That does not implement metrics export or
claim a controlled comparison. Human mid-run milestones and package-registry
contract kinds remain open before any workflow depends on them.

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

The [combined X05-A0+C01 review](rounds/V5-02-X05-A0-C01-review.md)
independently approves bounded test-only lexical sealing feasibility and closes
A0-R1. A1/A2 production sealing and combined A3 through C05 captured registration
are independently approved. A4 records the integrated registration verification
candidate; actual shape/projection semantics and builtin adapters remain B/C.


The [X05-A4 integrated registration gate](x05-registration-evidence.md)
is a verification candidate pending fresh independent review. Full Debug and
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
