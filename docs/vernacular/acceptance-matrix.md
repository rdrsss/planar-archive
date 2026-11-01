# V5 acceptance traceability

Current rewrite status is the [roadmap opening](roadmap.md#current-status).
Superseded status checkpoints that formerly opened this file are preserved in
[history](history.md#former-opening-of-acceptance-matrixmd).

This is a planning map of the unchanged [v5 matrix](planar-design-spec-v5.md#adversarial-acceptance-matrix),
not a new test run or a claim that cut 2 is complete. Current evidence is approved
round evidence; future obligations have not been executed. See [alignment](v5-alignment.md)
for authority, conflicting classifications and sequence. “Partial” deliberately
splits foundations from the command and execution behavior still missing.

Cut 2 requires all applicable Render and Execution obligations, conditional A24
policy behavior and A34 minimal execution provenance. A21 composition and A34
controlled experiments remain deferred. A35/A36 are local IDs assigned by this
alignment plan to v5's two unnumbered authoring rows in
[open item 6](planar-design-spec-v5.md#open-items); they are not IDs in the input.
RA01 also owns explicit-directory observation assembly and initial orchestrator
projection; these are deliverables even though v5 gave them no numbered row.

| ID | Obligation / gate | Current evidence and limits | Remaining owner / gate |
| --- | --- | --- | --- |
| A01 | Insertion order/fresh-process determinism; Render | [R06](rounds/R06.md), [R09](rounds/R09.md): canonical vectors and actual repeated renders. | Preserve through RA01/execution; R19 integrated recheck. |
| A02 | Scoped inclusion/exclusion and fingerprints; Render | [R07](rounds/R07.md), [R09](rounds/R09.md): scoped material mutations. | Preserve accepted-revision selection in R15; R19 recheck. |
| A03 | Optional rationale/checkpoint affects render only; Render | [R09](rounds/R09.md): preview identity evidence. | R16 retained refresh/recovery must preserve distinction; R19. |
| A04 | Mandatory files stay material, illegal wrappers refuse; Render | [R07](rounds/R07.md), [R09](rounds/R09.md): schema/materiality refusal. | Preserve through load/authoring; R19. |
| A05 | Own implementation changes output, not own work; Render/Execution | Partial: [R09](rounds/R09.md) preview output/fingerprint separation. | R17 invalidates check applicability against changed output; R19. |
| A06 | Wrong/stale output cannot gain applicable review; Execution | Partial: [R08](rounds/R08.md) exact preview reviewer targets, no approval mutation. | R18 exact-target verdict/approval; R19. |
| A07 | Mandatory budget overflow refuses; Render | [R08](rounds/R08.md), [R09](rounds/R09.md): byte counts/refusals. | RA01/execution projections retain mandatory context; R19. RA01 orchestrator sections: finding RA01-C-F1 (projection returns a bare section array without version:1 and sections not mandatory:true, authoring-contract.md:422; src/stdlib/planar/orchestrator.lua:112-140), open, [RA01-C](rounds/V5-02-RA01-C.md); execution projections and R19 unchanged. |
| A08 | Capability/mutation/resource abuse bounded; Render | [R05 correction](rounds/R05-C01.md), [R09](rounds/R09.md): native sandbox. | [V5-02-A contract](command-contract.md) and B1/C01 approved; B2 CLI dispatch approved; C names/snapshots are independently approved; public load replay is independently approved in D7-B/D8. External X after R14 requires independent adversarial proof; no runtime evidence from prose. External portion: X08 with corrective C01 ([round record](rounds/V5-02-X08.md), evidence `tests/native/v5-02-x08-evidence/`) maps E01 to E14 and the twelve adversarial matrix rows to named tests and raw logs, with the Linux gate recorded. Limits restated, not repaired: no RSS guarantee; no hostile-writer guarantee ([ledger contract](ledger-contract.md) writer limits); the X-CAP last paragraph of the [external profile contract](external-profile-contract.md) applies; CL-1 to CL-6 and the X07-C01 limits (a writer killed during a busy wait is rolled back silently); x86_64 Linux is not covered. |
| A09 | One claim, lost-response recovery, request conflict; Execution | Pending; native load request replay is not run ownership. | R15 claims/packet atomicity/replay; R19 restart/race proof. |
| A10 | Takeover fences old session mutations; Execution | Pending; no runs. | R15 token/generation enforcement; R17 checks; driver must stop old writers; R19. |
| A11 | Drift, stale-check refusal, same-run refresh; Execution | Pending. Preview mutations do not acknowledge a run. | R16 refresh/drift, R17 check revalidation; R19. |
| A12 | Blocker clears, still-valid done evidence restored; Execution | Pending; no completion states. | R16 recovery/rederivation design, R17 evidence applicability; R19. |
| A13 | Atomic verdict plus repair; retry once; Execution | Pending; acceptance transactions do not implement verdicts. | R18 transactional verdict/repair and fault/replay tests; R19. |
| A14 | Repair cap creates durable blocking question; Execution | Pending. | R18 cumulative cap, final permitted repair/review and further rejection; R19. |
| A15 | Check input/output races are inconclusive; Execution | Partial: [D2](rounds/R14-D2-review.md) native capture receipts, not stable check execution. | R17 reservation/before-after capture/driver stability; R19. No change-and-revert guarantee without adapter. |
| A16 | Stable interface may retain work, changed output invalidates integration; Execution | Partial: [R07](rounds/R07.md) dependency selection. | R17 exact output combinations and prerequisite derivation; R18 approval applicability; R19. |
| A17 | Missing/zero tests, timeout/tool absence fail; Execution | Pending; render only describes selected checks. | R17 real trusted adapters and bounded actionable diagnostics; R19. |
| A18 | Boundary/alias/ignored/rename/mode checks; Execution | Partial: [R05](rounds/R05-C01.md), [R09](rounds/R09.md), [D2](rounds/R14-D2-review.md) confined capture and topology. | R17 final base diff, attribution, ignored/generated policy and realistic boundary checks; R19. |
| A19 | Recovery includes actual work/checkpoint/findings/current inputs; Execution | Partial: [R08](rounds/R08.md) render-only recovery explicitly has no active run. | R16 retained run packets, checkout observations, findings/evidence targets; R19. |
| A20 | Drafts never activate implicitly, invalid draft leaves active revision; Execution | Partial: [D8](rounds/R14-D8-review.md) approves public invalid-draft refusal with unchanged accepted history; status and active-run reads remain future work. | R15 accepted reads/draft status; R16 active-run revision behavior; R19. |
| A21 | Unbound import permits development, blocks integration; Extension | Pending/deferred; one spec/root now. | Post-R19 composition and exact binding/output-vector tests. |
| A22 | Removed/recreated role gets new generation; Execution | Pending. Stored declaration history has no execution generations. | R15 retirement/new-generation fencing; R18 old approvals; R19. |
| A23 | Format/storage failure, missing packets/cache rebuilding; Execution | Partial: [R12 review](rounds/R12-C2-review.md), [G](rounds/R14-G.md) format, disk-full/failure/read/export evidence. | R14 failure-safety portions approved in D8 under documented recovery limits; R15 atomic retained dispatch/cache rebuild; R16 interruption recovery; R17/R18 no partial completion; R19. |
| A24 | Old external approval cannot approve changed code; conditional policy | Pending; no human authentication or assertion API. | R18 explicitly enabled exact-target assertion policy or explicit unsupported-policy refusal; R19 records applicability. Never label disabled policy as tested approval. |
| A25 | Change/no-op/restore chronological revisions; Execution | Partial: [C3-B2](rounds/R14-C3-B2.md) native sequence; [E2](rounds/R14-E2.md), [F2](rounds/R14-F2.md) reads/export of native scratch history. | D8 public chronology/retry portions independently approved; R19 preservation. |
| A26 | CAS and atomic activation/reconciliation under crashes/races; Execution | Partial: [R12](rounds/R12-C2-review.md) foundation crashes; [C3-B2](rounds/R14-C3-B2.md) native transaction/races, not power-loss proof. | D8 native acceptance crash/disk-full and public CAS portions approved, with explicit conservative recovery limits; R15 role activation/reconciliation in same transaction; R19. |
| A27 | Equal digest/unequal exact payload refuses; Execution | Native guarantee: [R13 review](rounds/R13-C2-review.md), [C3-B2](rounds/R14-C3-B2.md) file/internal/value/request collision tests. | D8 R14 portions independently approved; preserve in new execution record paths; R19. V5's “no matrix” decision does not erase row. |
| A28 | Exact optional-byte/deletion/rename/mtime history, selective staleness; Execution | Partial: [R13](rounds/R13-C2-review.md) exact complete trees; [C3-B2](rounds/R14-C3-B2.md) atomic acceptance. | D8 load workflow portions independently approved; R15/R16 accepted input reconciliation; R17 evidence applicability; R19. |
| A29 | Early event reconstructs declarations/context/state/evidence after restart; Execution | Partial: [C1](rounds/R14-C1.md), [E2](rounds/R14-E2.md), [F2](rounds/R14-F2.md): verified declaration history/exact export without current Lua. | R15 retained dispatch, R16 recovery, R17 check/R18 review references, R19 full trajectory without re-execution. |
| A30 | Same-spec combined/split complete scope and independent review; Render | [R07](rounds/R07.md), [R09](rounds/R09.md), [R10-P01](rounds/R10-P01.md): computed quota packets/evaluation. | R19 real workflows in both groupings; no execution inferred from goldens. |
| A31 | Missing/duplicate assignment and invalid boundaries refuse; Render | [R07](rounds/R07.md), [R09](rounds/R09.md): native schema/graph refusal. | D8 public load preserves refusals; R15 accepted mapping; R19. |
| A32 | Grouped obligations without self-wait; cycles refuse; Render | [R07](rounds/R07.md), [R09](rounds/R09.md): graph and packet evidence. | R15/R17 actual scheduling prerequisites/completion retain semantics; R19. |
| A33 | Regrouping fences generations, preserves mappings, no inherited approval; Execution | Partial: immutable declaration history only. No active-role switching. | R15 structural reconciliation/token fencing; R18 target/policy approval; R19. |
| A34 | Identical baseline/spec, isolated trials, model settings/outcomes; Measurement plus cut-2 provenance | Pending. Declaration provenance alone is not complete execution or experiment provenance. | R15 designs minimal execution provenance before runs; R16–R18 record applicable events; R19 verifies. Controlled grouping/model comparisons and metrics remain cut 3. |
| A35 | Same declarations/registry yield byte-identical authoring prompts; local authoring ID | RA01-B2 A35 five prompt kinds are byte-equal across two fresh processes and reordered declarations (`authoring_prompts.t.cpp:1052`); RA01-C gate green on macOS and Linux (1437/1437, [RA01-C](rounds/V5-02-RA01-C.md), C-15a). Exact contract text: [authoring contract](authoring-contract.md) RA-3 and RA-4. | RA01 deterministic spec/critique/job templates with schema/check/shape inputs and fresh-process tests. |
| A36 | Rejected job diagnostic prompt names failing field path; local authoring ID | RA01-B2 A36 rejected load stderr round-trips through explain-rejection (`authoring_prompts.t.cpp:1933`), raw_error block equals the diagnostic bytes with DEL and C1 raw (`:1881`), excerpt for job.roles[1].work (`:1825`); RA01-C gate green (C-15b, C-15p, C-15s, C-15t). Exact contract text: [authoring contract](authoring-contract.md) RA-5. | RA01 wrapper preserves raw typed errors, field path and refusal; R14 load provides validated diagnostic boundary. |

No row above claims full execution coverage today. V5-02 A/B/C are operator
accepted; A, B1/C01, B2 and C are independently approved. Remaining R14,
mandatory external-profile X, RA01 and R15–R19 follow. The
[R14-D3 matrix](load-command-contract.md#bounded-follow-on-rounds-and-acceptance-matrix)
pins load coverage for A01–A08/A20/A23/A25–A29/A31/A36 and is independently
approved in [D3 review](rounds/R14-D3-review.md). D4 typed native diagnostics are
independently approved; D5 bounded native capture/preparation is a candidate
pending review. This adds no full-load acceptance claim.
The [command contract](command-contract.md#required-implementation-and-verification-sequence)
pins new tests without counting them as executed: parser/option lifetime/error
precedence, actual Lua dispatch, malformed catalog/capability bypass, name/env/
cwd/alias limits and replay before capture. Preserve A01–A08 and A25–A29 facts;
external codecs and load/public replay require their own implementation evidence.
Each scoped brief
must name the row portions it proves and cite actual evidence before changing
this map's status. Minimal provenance, explainable incremental dependency changes,
observation assembly and the orchestrator projection must remain explicit in
those briefs even where a single matrix row does not name them.

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
