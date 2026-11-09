# C07 independent plan review

Disposition: **approve the bounded plan** in C07-map.md and the authoritative C07 DECISIONS/INVARIANTS tables. This is planning approval only; no implementation, tests, calibration or integrated gate is approved. No product source, tests, build outputs or database was changed by this reviewer.

All six confirmed findings have a contract-grounded correction and observable regression/mutation obligation. The three proposed packages have disjoint source ownership; command replay belongs with native ledger work, while Lua policy and loader remain separate. Explicit build lanes and the current calibration source-read boundary still apply.

## Native and replay boundaries

Full saved-verdict verification must use original-event facts for every response field, including approve and historical cases. The plan correctly forbids using the saved response itself or today's role/count state as the reconstruction source. `max_rounds_at` selects the reviewer run's latest dispatch preceding the verdict; writer validation binds `facts.current.work` and its cap to that same acknowledged dispatch. Preserve this equivalence when reconstructing all branches. Repaired generation ordinals differ from generation row IDs; blocker cleared/opened sets and role after-state rows must be tied to the verdict event. The planned later-load/reactivation positive replay controls are necessary to detect accidental present-state reconstruction.

R17-7 makes own-unit observation authority belong to the current owning lineage. A surviving consumer completion's captured producer unit cannot supply that authority. Removing the completion seed is acceptable only after proving the complete ordered check-event scan includes every verified completion and failed/inconclusive own observation; otherwise use the same owner and lineage checks on the seed. Do not change historical vectors or completion payload shape to hide the leak.

The four early replay response constructors must retain their already-built stdout fallback. EX-7 specifies operation-specific retry guidance on output failure; the plan covers both command option families, both embedded/profile compositions and subsequent ordinary exact replay. No diagnostic vocabulary change is required.

## Lua policy boundary

R17-7 current publications come from the producer's latest current completion. On a completing check, replacement must remove the complete prior publication set for that producer job and role, including facets absent from the new set. Evidence-only checks preserve publications, and unrelated producers survive. Partial/grouped disappearance and restored publication controls prevent an overbroad clearing shortcut.

Multi-job snapshots require job-and-role identity, or an explicitly active-job-scoped map. The proposed audit includes original, explicit, derived and repair/transition maps, with both lexical switching directions and unchanged inactive history. This fixes selection authority without changing role IDs or the W recipe.

## Loader terminal-refusal boundary

The retained big-Lua red log is a real correctness failure: the unchanged guard reports a prepared result despite a composed allocator refusal. The loader records `s.composed` but checks it only under `LUA_ERRMEM`, so Lua's successful GC retry bypasses terminal failure. The approved profile-preparation architecture requires one first terminal failure, preserving its typed diagnostic before adaptation; a retry cannot restore authority.

The plan correctly calls for a first-refusal latch and checks at protected-stage boundaries, including syntax and conversion exits, with deterministic test-only budget sinks. The implementation must preserve temporal precedence across `s.error`, allocator refusal, hook/constructor errors and protected conversion anchoring rather than always prioritizing an allocator error regardless of which happened first. Preserve existing inherited-limit policy and C-longjmp destructor safety. A later refusal must not overwrite the first diagnostic. The no-refusal control and released-live-charge assertions are essential alongside the unchanged runtime big-Lua guard. No larger fixture, production ceiling change or public fault hook is authorized by this plan.

## Evidence reviewed and remaining work

Read repository instructions, implementation/roadmap/v5 direction, C07 brief/map/tables, both C06 source-review reports and their retained reproduction descriptions, relevant EX-7/R17/R18 clauses, profile-preparation failure/lifetime clauses, the loader red log and the implicated current source. Inspected loader protected-stage control flow, verdict admission/count verification, original cap lookup and writer response assembly. No executable experiment or build was run in this review.

Preserve assertion-bearing red runs before fixes, then focused tests and fresh independent mutation/code review. Calibration must be rerun against final Lua/loader bytes; exploratory budget measurements are not acceptance. Coherent Debug/Release/Linux gates, zero skips/nonzero counts, final formatting/Doxygen/no-work checks and evidence integration remain outstanding. R19's unselected real project remains distinct from fixture verification.
