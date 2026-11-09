# C08 invariants and mutation obligations

Proposed coverage, not executed evidence. Reviewer27-case raw red is retained separately. Every corruption starts from a valid original result and changes one structurally valid fact; reader/status/replay/history must refuse ledger_corrupt. Compile failure, invalid grammar and zero-match runs are never mutant kills.

| ID | Required controls | Exact fault |
|---|---|---|
| I1 | Activation roles exact membership/order/state/generation; activated-job only when retiring other job; ordinary and repeated same-job activation replay. | Omit roles comparison; compare length only; include other-job transitions; use generation row ID. |
| I2 | Claim and takeover dispatch hashes/packet payload bytes match original; token/run/generation/header/previous_run and attachment remain checked. | Skip each work_fp/render_digest/bytes comparison; use encoded packet length; use current dispatch. |
| I3 | Refresh run/generation/revision/dispatch metadata independently bound; historical replay after later dispatch/load/takeover. | Header-only recovery check; use latest revision/dispatch/current generation. |
| I4 | Refresh true drift_cleared versus false independently correct; absent newly recorded checkpoint despite carried older dispatch checkpoint; present new checkpoint points to original event/run. | Use post-transition drift; use dispatch.checkpoint instead of event-created checkpoint; omit checkpoint comparison. |
| I5 | Checkpoint exact run/generation/id/event, attachment absent; replay after later checkpoint/release. | Skip each typed ID family; use latest checkpoint. |
| I6 | Both release reasons; correct original run/generation/terminal state/target role after-state/event/optional checkpoint; target-first with later dependent transitions. | Use current run/role state; wrong event type mapping; choose last transition; omit checkpoint/state/event checks. |
| I7 | Equal historical replay bytes unchanged and no event written after later load/repair/takeover/activation; unequal body conflicts; fresh unknown run refuses. | Consult current role/token state before equal replay; reconstruct from current head; invent no-op release success. |
| I8 | Load accepted/noop, original check/interrupted recover, verdict full replay guards stay green and independent corruptions still refuse. | Replace later full verification with common header-only path; bypass attachment rules. |
| I9 | Retained packet/manifest assembly and hashes remain verified; results use immutable payload size without re-running Lua/current sources. | Skip check_dispatch_content; compute packet bytes from another representation/current render. |
| I10 catalog test-only | Valid historical descriptor baseline accepted; grammar-valid wrong handler rejected specifically by exact epoch pairing. | Disable historical handler correspondence while leaving generic handler grammar enabled. Use `shipped.unbound_probe.v1` mismatch for each historical epoch5/10/14/16/17; confirmed scratch5/5 kill, no product change. |
| I11 loader test-only | Preconstruct declaration, incur recoverable allocator refusal during execution, then return it without later constructor/anchor; first refusal still returned. Scratch witness confirmed baseline5pass/mutant failure; use existing retry_sink and preconstructed planar.spec. | Delete execute-stage first-error guard while retaining parse/anchor guards. |

Source rows I1–I9 are new/changed execution.cpp scope and require fresh isolated coherent native probes. Carry independent unchanged Lua/loader/catalog product probes only by dependency hashes. Full Debug/Release/Linux gates, formatting/Doxygen/no-work, zero skips and integrated review remain required.
