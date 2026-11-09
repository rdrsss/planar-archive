# Integrated R15–R18 policy review

Disposition: **request-changes**. Reviewed candidate `ae02a12bf77c6b72d3060253dd47800c645796a7`, diff base `1338637c`, 2026-11-08. Source remained read-only. This report is the only repository write owned by this reviewer.

Scope: trusted checks/review/dispatch/reconcile/commands/orchestrator policy and the execution snapshot boundary. Read the repository instructions, implementation plan, roadmap opening, v5 baseline, execution contracts and integrated draft record. Findings below are semantic defects, not missing full-gate findings. No builds, installed binaries, old Planar tools, operator data or commits were used.

## F1 — P1: preserve reviewer lineage in verdict snapshots

Locations: `src/stdlib/planar/review.lua:183-186`; `src/engine/ledger/execution_review.cpp:1067-1079`; `src/engine/execution/snapshot_value.cpp:189-201`.

`in_reviewer_lineage` accepts every verdict without a `generation` member, trusting a native filter. Native actually selects **all verdicts in the current activation**, in addition to the latest current-lineage verdict, and drops their generation before passing them to Lua. Consequently the lineage condition in acceptance is unenforceable.

Concrete trigger: approve a completed job; load a reviewer rename and then restore the original reviewer name within the same activation, leaving worker ownership unchanged. The restored reviewer has a new lineage. The original target vector and W identities can return, and worker lineages/completions remain usable. The old approval is still in the activation snapshot and is accepted for the new reviewer lineage. The floor comparison in `approvals_and_rejection` protects target-owner lineages only, not the reviewer.

Impact: `status.review.acceptance` can return accepted without approval from the recreated reviewer, and accepted-quality dependencies inherit that unsupported approval. R18-6 item 3 explicitly requires V's run to lie in the current reviewer structural lineage; EX-5 says recreation starts empty, A22/A33.

Tested: actual `checks.lua` + `review.lua`, synthetic snapshot using the native serialized shape. Replacing reviewer generation 2 with generation 3 while preserving the old verdict gives `accepted=true`. Supplying the missing verdict generation 2 gives `accepted=false`. The rename/load CLI sequence is inferred from the inspected generation and snapshot code, not executed end to end.

## F2 — P1: revalidate captured producer vectors before recording completion

Locations: `src/stdlib/planar/checks.lua:128-149`, `:170-175`, `:201`; native forwarding/writer at `src/engine/ledger/execution.cpp:4650-4676`.

`evaluate` checks pass outcomes, recapture, drift and whether producer completion prerequisites currently exist, but never compares the run result's producer vector with the current producer publication. `rederive` synthesizes the completion with the stale vector, then excludes the explicitly completed role from applicability derivation.

Concrete trigger: consumer checks capture producer output P1 and pass with a matching recapture. Before the post-run `read_execution` (or before a retry), the producer completes P2 in the same accepted revision. Contract-only dependency inputs leave the consumer W unchanged. The new snapshot contains a valid producer completion for P2, so dependency checks pass, but the consumer's evidence still names P1. The proposed result is `completion` plus consumer `done`, even though that completion is immediately inapplicable under R17-7(iii). Native snapshot fencing uses the newly read snapshot and does not close this earlier capture-to-evaluation interval.

Impact: false `done`/`completed` is recorded for integration evidence covering the wrong producer output; the consumer run ends instead of retaining a usable run for another check. R17-7 requires the vector targets to equal current published outputs, and EX-4 permits `done` only with applicable evidence; A16 requires old integration evidence to become unusable when implementation changes.

Tested: actual policy evaluation with P2 current and passing captured evidence naming P1 returns `withheld=0`, `completion=true`, consumer `done`. Native `complete_check` was inspected: its early completion validation checks pass/recapture/transitions and locks the supplied snapshot, not captured vectors versus that snapshot. No timed native concurrency reproduction was run.

## F3 — P2: derive reviewer state from verdict applicability on every reconciliation path

Locations: `src/stdlib/planar/review.lua:779`; `src/stdlib/planar/reconcile.lua:198-203`; `src/stdlib/planar/checks.lua:209-215`.

The load hook `dispatch_internal.applicable` is bound unconditionally to worker `checks.applicable_completion`. Reviewers have verdicts, not worker completions. Thus accepting an optional-prose-only change after a reviewer finishes makes that reviewer `stale` even though the exact vector and review policy are unchanged. Conversely, check rederivation never checks verdict applicability: it handles worker completion and only `blocked -> ready` for reviewers. A done reviewer whose targets change can remain done.

Impact: needless reviewer reruns after immaterial loads; in the opposite direction the driver can see a done reviewer with no applicable verdict and cannot claim it to review current outputs. R18-1 item 2 explicitly defines reviewer applicable completion and stale state for EX-4 precedence rules 4/5.

Tested: actual check policy with changed target output leaves the reviewer `done`. Optional-prose load behavior is source-confirmed at the unconditional hook and load branch, not a native CLI reproduction. Fix all reconciliation users with role-kind-aware completion semantics, including restoration of a matching old verdict.

## F4 — P2: clear output-dependent blockers using post-check facts

Locations: `src/stdlib/planar/checks.lua:149`, `:171-180`; `src/stdlib/planar/review.lua:445-477`.

`evaluate` invokes `blockers_to_clear(S,{operation="check",job=...})` before constructing the hypothetical post-check snapshot, and does not provide that snapshot. The hook therefore compares the opening rejection against the **old** vector. `rederive` never revisits clearing after inserting new completions/observations. A check commit that changes the reviewed output vector consequently retains its obsolete `unowned_rejection` blocker.

Impact: the output change that should clear rejection leaves acceptance blocked until an additional qualifying commit happens. R18-5 requires the first commit after which the opening verdict is no longer current to clear the blocker, explicitly including `check_recorded`; R17-6 includes blocker clearing in both check commit modes.

Tested: actual policy with an open unowned rejection for P1 and a completing check for P2 returns zero `blockers_cleared`; the same `review.blockers_to_clear` called against the post-result snapshot returns one. This is a policy-level post-check fixture, not an end-to-end scheduling trace. Evidence-only observations also sit outside the hypothetical-snapshot update at line 171; that adjacent issue needs its own native transition review.

## Verification and limits

Executed `/opt/homebrew/bin/lua <scratch-root>/planar-policy-review-probes.lua` (also available as `lua`): one standalone scratch probe program, five scenarios including a lineage control, zero runtime errors after correcting a scratch-script brace. The program loads the candidate's real `checks.lua` and `review.lua`, supplies deterministic current-work hooks and hand-built snapshots; it does not use the older review test shim. Its final output:

```text
P1 recreated reviewer old approval accepted=true
P1 same snapshot with generation retained accepted=false
P2 changed target reviewer after=done
P3 changed target blockers_cleared=0
P3 clearing evaluated against post snapshot=1
P4 obsolete completion unblocked after=ready
P5 stale integration vector withheld=0 completion=true
P5 stale integration vector consumer after=done
```

P4 is an observed additional precedence discrepancy (blocked worker with obsolete completion becomes ready), left outside the four prioritized findings. These probes demonstrate policy behavior, not full native CLI success; native persistence, concurrent race execution, release/Linux builds and full acceptance remain outside this review slice. Recovery evidence/findings transport is owned by the parallel runtime review and deliberately not duplicated here.
