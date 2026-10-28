# Review of the execution design pass (RA01, R15 to R19)

Reviewed 2026-10-28: [execution-design.md](execution-design.md), an uncommitted draft, against the
roadmap Phase 2 sections, v5 (cut 2 table and the execution sections), the acceptance map,
v5-alignment, the six exact contracts, the remaining-steps design with operator decisions Q1 to Q18,
the X07 addendum (Q-INIT, Q-G35), `docs/architecture.md` and the cited source. Records only: no
source, test, build or other document was touched, and nothing was run except reads. Round X07 was
editing source in parallel, so `runtime.cpp` line citations may drift.

## Verdict

**Ready for the operator, after the fixes made in this review.** The design covers every row the
roadmap assigns to RA01 and R15 to R19, respects the out-of-scope list (no scheduler, no model calls,
no checkout management; the E13 live run is driver-side) and keeps render database-free. Before the
fixes, it had three soundness defects in its state machines (D-R1 to D-R3) and one unflagged
contradiction of v5 fencing (D-R4). Those are now repaired or explicitly opened for R17-A in the
draft, but they are design changes the R15-A1 and R17-A plan reviewers must confirm, not just
read. Two operator questions were added (E16, E17). For E7 the operator should know that v5 already
states the contract-delivery rule (D-R8).

## Findings

Severity: **high** means a lost update, double effect or unrecoverable state; **medium** means an
authority or coverage defect; **low** means precision or citation.

| ID | Sev. | Finding | Disposition |
| --- | --- | --- | --- |
| D-R1 | high | EX-3 wrote Lua's **complete** next-state vector, but the locked preconditions checked only `role_state(target)`, and claims on different roles were designed not to conflict. Two overlapping claims of roles A and B: B's vector carries A's snapshot state `ready` and overwrites A's committed `running`. That is a lost update, and it would also break the "role_state equals replay of transitions" invariant. | Fixed: delta application with a locked before-state check per transitioned role (EX-3 step 6); R15 test plus permissive mutant added. |
| D-R2 | high | Takeover opens a new generation (v5 `:59`), while `repairs`, the W `repair` field and the EX-4 "earlier completion in this generation" rule were all keyed per generation. A takeover after an admitted rejection would reset `repair` to its pre-rejection value, move W back, make pre-rejection evidence applicable again and drop carried findings, against v5's "rejected feedback is applied exactly once" (`:627`). It also discarded valid evidence on every takeover. | Fixed: takeover generations fence only and carry the structural lineage (EX-5); R18 test added. Plan review should confirm the lineage wording in EX-1 and EX-4. |
| D-R3 | high | The check request and event model was inconsistent with `requests.event_seq UNIQUE` and the bidirectional request/event invariant. A check spans two events (reserve, record), and `check-recover` wrote a terminal row for the reserved ID **plus** its own request row against one `check_interrupted` event. | Fixed: stated exception. The check request references `check_reserved`, and the recording or interruption event references the reservation (EX-1, R17-1, R17-7). |
| D-R4 | high | R17-1 "always records the evidence" even when the run had been fenced by a takeover mid-check, which is a ledger write by a superseded token. v5 says takeover "fences all old mutations" (`:59-60`, `:370`). This was not flagged as an amendment. | Opened: R17-1 now names two options for R17-A (refuse `fenced` and leave the reservation for `check-recover`, or a native-only interrupted closure recorded as an amendment); the A10 test follows the choice. |
| D-R5 | medium | Completion did not require complete-phase dependency qualities or the absence of a start blocker (v5 `:351`, `:355`, `:285-286`). The vector-equality check covers only `output_vector` checks and never `accepted` quality. Conversely, it required a producer completion even for `complete/facet/available` producers, which is stricter than the declared quality. | Fixed for the missing gate (R17-1 step 3; test added). The over-strict `available` case is left to R17-A. |
| D-R6 | medium | XD4 ("No new public code or message is added", listed as cross-cutting for every remaining step) was contradicted by nine new codes tagged "engineering". Only the message was flagged. | Fixed: tagged operator E16; E16 added. |
| D-R7 | medium | Unflagged amendments of exact contracts: the internal completion retry against `command-contract.md:190-194` ("No retries inside this invocation"); reserved check environment variables against `declarations.md:338-339` ("replacement with these exact values"), which the draft said needed amending only under E7 B; root help, fixed by X-CLI `external-profile-contract.md:133-134` as well, not only an "X07-scoped" rule; new opaque path slots (`command-contract.md:180-182`). | Fixed: each is now flagged where it arises. |
| D-R8 | medium | E7 is partly settled by the v5 baseline: v5 `:497` says the driver "must materialize the accepted version or receive a revision-mismatch refusal", which is option C's delivery rule. A adds a Planar write and cleanup capability under `--scratch`. | Note added under E7; the recommendation is unchanged (see my view below). |
| D-R9 | medium | E2 and R15-2 cited v5 `:125` and `:489` for "one active job per ledger", but v5 says neither. v5 lists "active jobs" in the workspace (`:110`) and makes switching jobs "one revision change" (`:497`). `ledger-contract.md:34-35` describes the R11 foundation and defers this (`:579-581`). | Fixed: citation and basis corrected; note under E2. |
| D-R10 | medium | E5's source `render-contract.md:349-351` supports a base identity in W, the opposite of A, and v5 `:241` puts "root and base identities" in W. A narrows v5, which the option text did not say. | Note added under E5. |
| D-R11 | medium | R18-7's refusal surface (the verdict field `authority`) was unreachable: `verdict_v1` in R18-2 had no such field, so strict Glaze would refuse `schema` first. | Fixed: optional `authority` field. |
| D-R12 | medium | The observe output bound (4 MiB) exceeded the 1 MiB observations input limit every consumer applies (`render-contract.md:138-139`), so observe could emit a file that `--observations` refuses. | Fixed: 1 MiB. |
| D-R13 | medium | The roadmap requires orchestrator guidance and provenance growth in R16, R17 and R18 (`roadmap.md:382-383`, `:394`, `:402-403`). Only RA01, R15 and an R17-D label mention were concrete. | Fixed: R16-7, R17-8 and R18-8 added. |
| D-R14 | medium | The token secret is retrievable by any ledger reader through request lookup, including the existing `history --request-id`, while R15-9 said "history ... never show it". | Fixed: R15-9 states the exposure, and E1 option C now carries the history sub-choice. |
| D-R15 | medium | EX-6 binding placement: if checked before early replay, a rebuilt binary could not replay a lost claim response, unlike load (`load-command-contract.md:105-106`). | Fixed: binding follows early replay; test added. Remaining concern under E3 below. |
| D-R16 | low | `role_reconciled` "used by load and activate" contradicted "one operation is one event" and load's existing event types. | Fixed: removed. |
| D-R17 | low | "Format3 is the exact format2 schema plus tables", yet `events`, `requests` and `current_state` change (their format2 CHECKs cannot hold execution rows). | Fixed. |
| D-R18 | low | E1 option C claimed v5 spells request lookup "only through history"; v5 `:368` spells `status --request-id`. EX-2 tagged that placement as an operator choice. | Fixed. |
| D-R19 | low | The sequencing table never placed R17-A, although R17-B must wait for it. | Fixed. |
| D-R20 | low | Not addressed and left for the owning contract rounds: `target_unavailable` (v5 `:360`) has no mechanism in R18; `--confirm-stopped` as a natively required flag works around "flags cannot be required" (`external-profile-contract.md:258-259`), so a text option such as `--confirm stopped` would be cleaner; the format3 event payload version for init and load is unspecified; the registry reserved-name collision is detected only at check time (it could refuse at format3 load); the effects of an approve verdict are not spelled out the way R18-4 spells rejection; the R17 limits do not restate v5 `:347` (an attributable role view or serialized writers is required); RA01 wave 2 must edit the `commands.lua` that wave 1 "freezes"; the `impact`/`render roadmap` deferral should cite `v5-alignment.md:73-76`; v5 open items 2 and 3 (human mid-run milestones, package-registry contracts; `v5-alignment.md:145-146`) become operator questions if the R19 real spec needs them. | Recorded for R15-A, R17-A, R18-A and R19-A briefs. |
| D-R21 | low | Citation precision: `orchestration.md:28-31` should be `:26-28`; `schema.lua:33-60` covers helpers, while the schema tables are `:48-63`; the `runtime.cpp:1333-1344` ABI pairing citation is correct today but sits in a file X07 is editing. | First fixed; others noted. |

## Coverage check (summary)

Every roadmap row now has a mechanism: RA01 covers A35 and A36, observe and the orchestrator projection,
and preserves A01, A04 and A07. R15 covers A09, A10, A20, A22, the A23 dispatch part, the A26
reconciliation part, A33 and minimal A34. R16 covers A11, A19, A29 and the recovery parts of A12, A20
and A23. R17 covers A05, A12, A15 to A18, the completion parts of A23 and A28 (a test was added in
review) and the check part of A29. R18 covers A06, A13, A14, the approval part of A16, the approval
parts of A22 and A33, the review part of A29 and A24. R19 covers the full map, with A21 and controlled
A34 deferred.

Items from v5's cut 2 that are narrowed or deferred: `render --claim` becomes `claim` (Q14 A); init
emits no prompt (Q10 B); `impact` and `render roadmap` are deferred (v5-alignment `:73-76`; cite it);
`explain` moves into `status --role` (E1); the base identity leaves W (E5, now flagged); Planar-side
contract materialization departs from v5 `:497` (E7, now flagged). None is silent any longer.

## Citation spot-check

All checked by reading the cited lines. Correct unless marked.

| Citation | Result |
| --- | --- |
| `remaining-steps-design.md:754`, `:755`, `:756`, `:757`, `:758`, `:759`, `:760`, `:761`, `:762` (Q10 to Q18) | Correct |
| `remaining-steps-design.md:746`, `:753` (Q2, Q9) | Lines correct; the row text described only Q2 (fixed) |
| `remaining-steps-design.md:43-69`, `:67` (P1 to P12, XD1 to XD6, XD4) | Correct |
| `remaining-steps-design.md:99-102` | Lines correct; the "X07-scoped" claim was wrong (D-R7) |
| `x07-addendum.md:5`, `:52`, `:343` | Correct |
| `src/cmd/planar/response.cpp:162-170` | Correct (`failed()` mapping) |
| `ledger-contract.md:34-35`, `:72-77`, `:104-106`, `:162`, `:163`, `:249-256`, `:258-285`, `:322-325`, `:337-348`, `:393-408`, `:442`, `:445`, `:470-477`, `:487-498`, `:579-581` | Correct; `:34-35` is a foundation description (D-R9) |
| `external-profile-contract.md:258-259`, `:270-273`, `:366-373`, `:500-502`, `:526-541`, `:552-562`, `:564-573`, `:647-650` | Correct |
| `command-contract.md:37-60`, `:94-96`, `:188-196`, `:233-238` | Correct |
| `load-command-contract.md:28-33`, `:109-163`, `:195-230` | Correct |
| `render-contract.md:84-86`, `:105-134`, `:138-141`, `:182-221`, `:214-221`, `:311-326`, `:327-331`, `:337-395`, `:396-406`, `:423`, `:615-626` | Correct; `:349-351` correct but cited for the opposite conclusion (D-R10) |
| `declarations.md:217-228`, `:236-240`, `:267-329`, `:276`, `:284-286`, `:336-352`, `:343-359`, `:347-349`, `:350-351` | Correct |
| `acceptance-matrix.md:18-19`, `:23-30`, `:32-56`, `:45`, `:46`, `:51`, `:57-58` | Correct |
| `roadmap.md:52-56`, `:347-363`, `:353-355`, `:365-375`, `:368`, `:370`, `:377-383`, `:385-394`, `:396-403`, `:405-415` | Correct |
| v5 `:59-62`, `:191`, `:235`, `:262-268`, `:270-276`, `:323-325`, `:345-356`, `:360`, `:364-387`, `:395`, `:406-436`, `:497`, `:507`, `:513`, `:517`, `:561-565`, `:579-589`, `:625-627`, `:744-745` | Correct |
| v5 `:125` (as "one active job per ledger") | Wrong attribution (D-R9, fixed) |
| `v5-alignment.md:130-132`, `:141-146` | Correct |
| `implementation.md:307-316` | Correct |
| `docs/architecture.md:11-25` | Correct |
| `orchestration.md:28-31` | Wrong; the escalation rule is `:26-28` (fixed) |
| `src/stdlib/planar/commands.lua:56-80`, `presentation.lua:11`, `runtime.cpp:1333-1344` | Correct as of today |
| `src/stdlib/planar/schema.lua:33-60` | Approximate; the schema tables are `:48-63` |

## Operator questions: my view

| Q | Real open choice? | View |
| --- | --- | --- |
| E1 | Yes, for names and grouping only. Q14 A fixes `claim`, `status` and `release`, and Q10 B fixes `ingest`; the note now says so. | **Agree with A.** One finite service sequence per family matches the command contract. Replace the natively required `--confirm-stopped` flag with a text option (D-R20). Decide whether `history --request-id` returns execution results, because that exposes tokens. |
| E2 | Yes. | **Agree with A, with a caveat.** A per-ledger activation suits arms that share one workspace file (v5 `:67`, `:489`). Under C every arm would need its own workspace.lua. But A deviates from v5 `:110` and `:497`, and that must be recorded in v5-alignment. The original rationale overstated `ledger-contract.md:34-35`. |
| E3 | Yes. Q12 A settles only RA01's embedded-shape scope. | **Agree with A**, with two adjustments: the check must follow early replay (now fixed), and the operator should consider exempting pure reads that need no current Lua (status without drafts, retained-packet recovery). Otherwise a binary upgrade blocks A19-style recovery until a fresh load. Each added family needs the closed-factory architecture revision (`x07-addendum.md:60`). |
| E4 | Yes. | **Agree with A.** |
| E5 | Yes. | **Agree with A, provided the narrowing is recorded.** A drops the base identity from W, against v5 `:241`. A changed base then invalidates evidence but never asks a running worker to reconsider. Acceptable for the cooperative single-root cut, but the operator should choose it knowingly. |
| E6 | Yes. | **Agree with A.** |
| E7 | Partly. The contract-delivery half is stated by v5 `:497` (option C's rule); the result-reporting half is open. | **Disagree on delivery; agree on results.** I recommend C for contract delivery (the driver materializes the accepted bytes and Planar verifies them, refusing on mismatch). That follows v5, needs no Planar file-write or cleanup capability and no `--scratch` option, and contracts already live in the root as frozen paths (v5 `:345`). Keep A's `PLANAR_CHECK_RESULT` result file. Either way, the reserved variables amend `declarations.md:338-339`. |
| E8 | Yes. | **Agree with A.** A failed check is a successful recording (v5 `:395`). |
| E9 | Yes; v5 `:415` calls its fields illustrative. | **Agree with A.** |
| E10 | Yes. | **Agree with A.** |
| E11 | Yes. | **Agree with A**, on the corrected understanding that the secret stops accidental reuse, not a ledger reader (R15-9). |
| E12 | Yes. | **Agree with A.** Keep its total within the new 1 MiB output bound. |
| E13 | Yes. | **Agree with A.** The live run is a driver invoking a model, not Planar calling one, so the out-of-scope rule holds. |
| E14 | Yes. It edits CLAUDE.md's authority list, which only the operator can approve. | **Agree with A.** |
| E15 | Yes. | **Agree with A**, recording that the recheck then compares non-cryptographic digests (v5 `:252`), a limit of the cooperative stability contract. |
| E16 (added) | Yes. XD4 forbids new public codes or messages for every remaining step. | Recommend **A**: execution needs distinct typed codes (v5 `:589`; `render-contract.md:626`). |
| E17 (added) | Yes. v5 says only "the active job and shape generation". | Recommend **A**: count per job activation; loads and refreshes never reset it. |

No question repeats a binding decision outright. E1 overlaps with Q14 and Q10 (now noted), and E7's
delivery half overlaps with v5 `:497` (now noted). Further questions may be needed later: the
fenced-completion choice is engineering for R17-A unless it amends v5's fencing statement, in which
case it goes to the operator; v5 open items 2 and 3 become questions only if the R19 spec needs them.

## What changed in execution-design.md

See its changelog section. In summary: D-R1 to D-R3, D-R5 to D-R7 and D-R9 to D-R19 were fixed in
place; D-R4 was converted into an explicit open choice; D-R8 and D-R10 became notes under E7 and E5;
E16 and E17 were added; no recommendation for E1 to E15 was changed.
