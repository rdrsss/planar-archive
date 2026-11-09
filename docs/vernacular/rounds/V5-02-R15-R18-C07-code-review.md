# C07 independent source review

Disposition: **request-changes** for N-C07-1, the remaining saved-response consistency family below. The six C07 corrections are sound in the inspected source and focused evidence; this is not integrated acceptance. Product source and shared build outputs were read-only. No build or mutation campaign was run by this reviewer.

## N-C07-1 — P2: remaining execution replay responses are not bound to their recorded facts

`src/engine/ledger/execution.cpp::check_request` verifies only part of several saved responses. The C07 verdict reconstruction closes its own branch, but the same reader-integrity defect remains in activation, claim/takeover and recovery operations:

- `job_activated` (around1204): header checks omit the complete `roles` array.
- `claim`/`takeover` (around1278): run/token/dispatch ID checks omit dispatch work fingerprint, render digest and packet byte length.
- `refresh`/`checkpoint`/`release` (around1233): typed codecs plus canonical re-encoding validate shape, but cross-row checks cover only kind/ledger/request/job/role and the attachment. Run/generation, revision/dispatch/checkpoint/event, drift-cleared and run/role-state details are not bound to their original event rows.

A real five-unit workflow first completed and approved normally, then executed activation, claim, checkpoint, refresh, takeover and release. The probe independently altered **27 structurally valid fields**, restoring each original response before the next case. All27 corrupted ledgers passed `status` with exit0 and replayed the fabricated field with exit0. Cases include activation role state/generation; claim and takeover work fingerprint/render digest/bytes; refresh run ID/generation/revision/drift-cleared/dispatch fields/checkpoint ID; checkpoint run and checkpoint/event IDs; release run ID/generation/state, role state, event and checkpoint ID. Invented ID999 and false output identities therefore reach callers despite correct underlying recorded facts.

The probe drops and restores the exact requests immutable triggers solely in disposable ledgers. This demonstrates reader-integrity failure, not mutation permission for normal clients. The native cross-row reader obligation is EX-1 invariants14/20 plus the exact P2-1 and R16-1/3/4 response meanings and exact request replay contract. These references describe IDs and state of the original commit; codec-valid but contradictory values cannot be trusted.

Evidence: `tests/native/r15-r18-c07-evidence/review/saved-response-probe.py`, `saved-response-red.log` and `saved-response-red.json`. The executable was a private copy of the coordinator-confirmed coherent Release candidate; its SHA256 is recorded in the JSON and identity manifest. The collection script exits0 after logging observations; that exit is not correctness-test success. An initial probe attempted to index a refresh checkpoint that was legitimately `none`; its incomplete log remains `saved-response-setup-1.log`, excluded from the27-case claim. The final setup supplies observations to refresh/release and executes every case.

## Single C08 correction boundary and complete schema inventory

Use the existing typed codecs and original immutable facts. Do not add a serializer or derive expected values from the saved response/current head. A complete canonical reconstruction or exhaustive typed comparison is suitable.

| Request family | Original authoritative facts and required closure |
|---|---|
| activate | Event/request/meta headers; `job_activations` row; previous job from payload; all event transitions for the activated job only, including their role, after-state and generation ordinal via generation row. Exclude transitions retiring another job. Compare exact role membership/order as well as fields. |
| claim/takeover | Existing verified run/token/generation/event/dispatch associations plus dispatch work_fp/render_fp (public digest text) and decoded packet file payload size; previous_run only for takeover. Preserve attachment-to-packet check and assembled packet/manifest behavior. |
| refresh | Event/request/run/generation and revision; payload dispatch ID and its original hashes/packet size; checkpoint is the row created at this event or none (not an earlier carried dispatch checkpoint); drift_cleared is original before-drift, recoverable from this event's transition/absence under the validated refresh grammar. The replay state passed into check_request is already after apply_transitions, so do not take its now-cleared drift as the original value. |
| checkpoint | Original event/run/generation; checkpoint row whose run/seq match the event, with exact id/event pair. No attachment. |
| release/interrupted | Original event/run/generation; terminal reason/run state from event type; target transition's after-state; event sequence; optional checkpoint created by this event. Do not use the run or role's later state. No attachment. |
| load accepted/noop | Already uses full typed `load_result` equality with original revision/parent/event/meta and rejects attachment. Preserve this path and positive old replay after later loads. |
| check/check-recover | `verify_check_results` already reconstructs row-backed fields for both original check request and terminal check-recover request, including interrupted outcome, reservation, evidence, observation/completion/publications. It runs after the header check. Preserve this C02/C03 correction; do not replace it with header-only validation. Withheld reasons are bounded recorded Lua policy output and intentionally validated as such rather than re-executing policy. |
| verdict | C07 now reconstructs full response from original verdict/admission/repair/blocker/transition facts; preserve it. |
| init and read-only commands | Init has no execution requests row; status/history/preview/recover/export do not create saved mutation responses. There is no missing init request branch to invent. |

The narrow production owner is `ledger/execution.cpp`, with native execution tests and a tracked real-workflow corruption regression. History and command replay should consume the corrected shared verifier, not receive parallel patches. Add independent field-family corruption assertions through reader/status/replay/history lookup as appropriate, positive exact original replay after subsequent load/activation/repair/run changes, drifted and nondrifted refresh, refresh with/without newly recorded checkpoint, both release reasons and activation switching jobs. Include unchanged load/check/check-recover/verdict controls so the inventory stays complete. Schema/CLI changes are unnecessary; Lua/runtime/loader bytes need not change, so resource calibration can remain scoped to its unchanged dependencies.

## C07 corrective assessment

Inspected full `check_admission_replay` reconstruction against writer result assembly, original reviewer dispatch cap, activation-local count replay, generation ordinals, all opened/cleared blocker rows and original transition after-states. Approve/historical branches no longer skip response consistency. The existing codec canonical comparison covers every response field. The37-corruption/five-branch test matrix and later repair/load replay controls are meaningful; their first-red limitation is accurately reported and independent mutations remain pending.

The observed projection now scans verified check_recorded events in order and admits only original own units of a currently owning role lineage. `load_completions` proves each completion has that linked event, so dropping the redundant consumer-inclusive seed preserves successful and newer failed/inconclusive own observations. Historical completion vectors remain intact.

Lua rederivation replaces the completing job/role's whole publication set, including absence, while preserving unrelated producers and evidence-only behavior. Active-job scoping protects original/explicit maps and repair lookup/increment. Existing actual-Lua controls exercise grouped partial facets, unrelated preservation, input immutability and inactive same-name roles; public workflows cover both lexical switch directions and retained history.

All four early run/recovery replay constructors now carry prebuilt stdout_error in embedded/profile paths. The ten real closed-pipe cases cover both option families and exact ordinary replay without changing ledger data.

The loader shares a first-diagnostic latch across composed allocator, hook and constructor failures and inspects it after state creation, setup, parsing, execution and protected anchoring, including successful GC retry. Later errors cannot replace the earlier typed error. Native temporaries finish before allocating Lua error calls, and anchoring retains its protected longjmp boundary. Test sinks deliberately permit recovery, so the tests examine loader authority rather than a sticky sink. Final loader21/186 and unchanged bigLua1/34 evidence was supplied by the policy lane; no larger fixture or product ceiling change was used.

## Broader source assessment and limits

Read active instructions, implementation/roadmap/v5, C07 plan/tables/approved plan review, both C06 broad reviews, coder reports and relevant exact contracts. In addition to the C07 sources, traced the remaining high-risk claim/takeover/refresh/load writer and reader paths: format/locked identity before replay, body equality before CAS, load expected-head guard, token/precondition/locked-before checks, open reservation refusal for refresh/release, collision-safe interning, fresh run allocation/fencing order, generation/fact-version delta writes, shared post-write verifier, transaction ownership/rollback and historical event replay. Inspected schema constraints and event/transition/generation/dispatch coupling and the historical reconstruction boundary. N-C07-1 emerged from this remaining reader review.

The C06 reviews cover the wider unchanged policy/profile/catalog/authoring/check-process paths; this pass did not repeat every prior line or every environmental experiment. No additional concrete defect was found in the inspected C07 fixes or transactional ordering. That is a bounded code-review conclusion, not an exhaustive proof. Full coherent host/Linux gates, reduced-budget calibration, native/Lua mutation evidence, no-work verification and final evidence integration remain pending. R19's unknown real project is not certified by fixture success.

Only report/evidence files were written. Raw test reports were inspected, not independently rerun here; the27-corruption scratch experiment is this reviewer's executed evidence. Source and evidence identities are recorded beside it. No installed/legacy Planar, operator data, commit, merge, publication or installation was used.
