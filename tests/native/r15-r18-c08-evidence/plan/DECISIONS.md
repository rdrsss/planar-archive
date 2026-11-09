# C08 decisions

| ID | Decision | Authority |
|---|---|---|
| D1 | Bind every saved result field to original immutable event facts, using existing typed codecs and exact canonical bytes; no current-head or self-derived expectation. | EX-1 invariants14/20; P2-1/P2-5; R16-1/3/4; C07 N-C07-1 reproduced27cases. |
| D2 | Activation roles are precisely activated-job event transitions, with generation ordinals and after-states; omit other-job retirement transitions. | P2-1 activate response; EX-3 activation; existing writer4449–4472. |
| D3 | Dispatch metadata comes from recorded dispatch and decoded packet payload, with existing packet/manifest/W verification retained. | P2-1 claim/takeover, P2-5 assembly, EX-1 dispatch integrity; R16-1 refresh. |
| D4 | Refresh drift flag is original before-drift and checkpoint is created by this event, not a carried older checkpoint. | R16-1 response84–90; validated refresh grammar1408. |
| D5 | Recovery run generation uses original run generation ordinal; terminal state and role state derive from original release event and target transition. | R16-3/4 exact responses165/206; EX-5 lineage. |
| D6 | Equal original request replays after later changes; fresh unknown-run release remains refusal and unequal body remains conflict. | EX-3 replay order; R16-7 token authorization/DR16-9. No new idempotence behavior. |
| D7 | Preserve complete load/check/check-recover/verdict validation and opaque historical packet authority. Init/read-only create no execution saved request. | Existing exact contracts; C02/C03/C07 fixes; P2-5 native history assembly. |
| D8 | Narrow private execution.cpp implementation and tests; no schema/API/Lua/limit changes. | Routine engineering under existing contracts and AGENTS. |
| D9 | Add grammar-valid historical handler mismatch test to independently expose exact-pairing predicate; do not alter catalog product policy. | Existing C05 historical exact epoch/handler contract; independent prober masked-mutant finding, grammar-valid `shipped.unbound_probe.v1` witness confirmed across five historical epochs. |
| D10 | Independently test loader execute-stage terminal latch without later constructor/anchor masking; product loader unchanged unless a new defect is proven. | C07 first-terminal contract and prober execute-guard survivor; preconstructed-return witness confirmed baseline5pass/mutant assertion failure. |

No new product/authority choice identified. Plan approval remains required; no implementation authorized by this map alone.
