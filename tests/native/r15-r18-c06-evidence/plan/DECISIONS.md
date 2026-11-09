# C06 decisions

| ID | Decision | Status and authority |
|---|---|---|
| D1 | Preserve verdict grammar/shape field paths through the command bridge; resource fields remain resource-only. | Contract-decided: R18-2 steps3–7, execution-contract-r18.md:128–142. |
| D2 | Check refusal transport carries registry/scratch fields, delivery expected revision/working_bytes, and first mismatching producer work. | Contract-decided: R17-4:237–248, R17-10:469–478. |
| D3 | Use execution-owned typed check error and command-local adapters; leave generic engine diagnostic and unrelated native_error behavior unchanged. | Engineering implementation of D1/D2; AGENTS typed owning-module boundary. No new product field or authority. |
| D4 | Active revision in delivery detail comes from the verified execution snapshot, not caller/Lua plan input. | Contract-decided: R17-4 step2, R17-5 verified plan; existing snapshot has active_revision. No check_plan ABI addition needed. |
| D5 | Open reservation under another request emits exact check_in_progress message; same-ID replay/conflict behavior and ordering retained. | Contract-decided: R17-9:415–425, R17-10:476. |
| D6 | Current publications belong to active non-retired owning role and its latest completion in current structural lineage. Retired history remains retained/verified/readable in persistent and historical read surfaces; retired completions are excluded from the current execution snapshot. | Contract-decided: EX-5:722 retirement ends lineage; R17-7:333–369 current/publication definitions. |
| D7 | Activation has no inherited publications; load uses ownership and lineage after all structural resets, independent of role iteration order. | Contract-decided: EX-4 after-operation precedence, EX-5 activated/structural_change/recreated start empty. |
| D8 | Post-operation blocker-clearing view uses the same filtered publications and projected roles as material/reconciliation. | Contract-decided: EX-4 after-operation facts, R18 blocker semantics; current review.lua adapter must not rebuild stale view. |
| D9 | Takeover retains publications and completions; ordinary material-only change does not erase current publication merely because completion becomes inapplicable. | Contract-decided: EX-5 takeover carry, R17-7 current versus applicable distinction. |
| D10 | No new producer-only facet fingerprint, storage schema, history rewriting or CLI shape. | Existing exact contracts, C05 scoped review advisory; F-FINAL-3 concerns lineage only. |
| D11 | Native current observed/evidence projections must not select retired/nonowning producer records; audit them alongside publication filtering without removing persistent historical completion records. Remove retired completions from the current snapshot as R17-7 requires; audit fallback consumers. | Contract-decided: R17-7 observed owning role/current lineage and evidence definition. This is an adjacent source hazard, not a separately reproduced finding. |

No new product/authority choice identified. Implementation API names are routine engineering choices. Plan review is still required; this table is not approval.
