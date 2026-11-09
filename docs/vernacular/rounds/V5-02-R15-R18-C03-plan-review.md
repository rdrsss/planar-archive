# R15–R18 C03 independent plan review

Initial disposition: **request-changes**. This is a plan/contract review only;
no product source, tests or build artifacts were modified and no builds ran.
Reviewed the C03 round brief and `plan/DECISIONS.md` / `plan/INVARIANTS.md`
against the approved declaration/render/execution contracts and R16–R18 drafts.
The existing fresh C02 reviews supply the bounded source mapping and reproducers.

## P1: derive vector membership per work, including grouped dependencies

The initial decision table mirrors `checks.plan`'s role-wide external producer
set. That implementation is not contract authority. `declarations.md:296–302`
keeps effective-check applicability keyed by work ID and requires each unit
receiving an output-vector check to have the appropriate dependency.
`declarations.md:352–355` defines exact own output plus dependency producer
outputs. `render-contract.md:211–213` similarly requires the exact own and
complete-dependency output vector. The declaration grouping rules preserve
internal obligations; no cited clause excludes a producer merely because it is
assigned to the same role.

Before implementation, specify the producer set for each evidence row from that
row's consumer work and retained dependencies, including a grouped producer.
Do not add another assigned unit's unrelated producers. Capture observations may
contain the union needed by all checks/units, while each evidence vector remains
scoped to its own consuming check. Add a valid grouped internal edge and an
unrelated-unit edge to the test/invariant mapping. Align Lua planning and native
verification; preserving the current plan's exclusion would codify the defect.
A new product choice is not necessary where the existing exact contracts decide
these semantics.

## P2: make the release amendment complete and narrow

The proposed rederivation resolves a real contradiction: R18-1 requires actual
done workers for reviewer readiness, while R16-4 incorrectly assumes release
cannot alter any dependent state. The no-fact-bump rule is correct and must stay:
EX-3 explicitly says a non-bumping operation can carry transitions without
incrementing any role's `fact_version`.

Amend all conflicting clauses: R16-4's no-derived-transitions text, the cross-row
one-transition invariant, and the native API's one-transition admission wording
(currently R16 lines 191, 329 and 371). Explicitly identify the scoped amendment
to EC EX-4 carrying events and the relevant M20–M22 event cells, so the added
`run_released`/`run_interrupted` carriers do not silently conflict with the
approved state-machine table. Target transition remains ordinal 1 with matching
release reason, inactive target and cleared drift; downstream rows use existing
lawful derivation reasons, deterministic order and the unchanged job/run facts.
Full hypothetical vector validation and delta writes remain required. Preserve
request replay, token fencing, all-role fact preconditions and checkpoint /
provenance behavior. No new event, transition reason or public field is needed.

N6's unconditional identity-only comparison is correctly scoped: keep weaker
head/fact preconditions for evidence-only results. Its negative test must prove
unchanged rows, with a same-identity stale-head/fact positive control. N5 output
comparison must use the retained reservation observation rather than the current
snapshot, so later valid observations do not invalidate historical evidence.

The coder was notified before dependent product edits. Revised plan tables and
the precise amendment will be rechecked; this report is not code approval or a
claim that full gates/probes passed.

## Revised plan recheck

Disposition: **approve the plan**, superseding the initial request-changes above.
The revised decisions require per-consumer complete-phase producers, including
grouped internal producers and excluding start-only facet dependencies and
unrelated consumers. Observation membership is owned units plus the union of
external producers actually selected by effective output-vector checks. A
complete-contract dependency with no output-vector check does not independently
add a vector producer. This is the existing approved render/declaration meaning,
not a new dependency policy; clarify the draft R17 slash wording accordingly if
the coordinator grants that document scope.

The invariant table now includes the grouped/internal and unrelated-consumer
planner regression; its fixture also includes a start-only facet edge. The
planned release amendment explicitly covers EC EX-4 carrying events/M20–M22 and
all three R16 one-transition clauses. No-fact-bump semantics, target-first
ordering and existing transition vocabulary remain intact. N6 retains the
identity-only guard and successful stale-head/fact positive control.

This approves the corrected plan and the stated scope of its contract amendment.
The exact amendment prose, implementation, real native tests, mutation kills and
full gates still need independent code review; none is certified by this report.

Reviewed plan file hashes (generated from file bytes):

- `tests/native/r15-r18-c03-evidence/plan/DECISIONS.md`: `4fc0946210c317fbaf109eb5f6614cbe99b5d74a20b9a2369b7e5276cb85557f`
- `tests/native/r15-r18-c03-evidence/plan/INVARIANTS.md`: `5a098023ec3c70fa0ddd406357d85f631e6f380b32feeff8156ae2f02b955fba`
