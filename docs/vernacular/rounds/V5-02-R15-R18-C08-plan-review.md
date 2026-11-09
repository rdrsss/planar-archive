# C08 independent plan review

Disposition: **approve the bounded plan** in C08-map.md and the authoritative C08 DECISIONS/INVARIANTS tables. This is planning approval only. No product source, tests, build outputs or database were changed or executed by this reviewer.

The plan covers the complete remaining saved-response family with existing typed codecs and original retained facts. It does not use a codec-valid saved field as its own authority or reconstruct historical answers from today's head. The six affected operations share the existing execution reader boundary; separate patches in command/history consumers are unnecessary.

## Native reconstruction boundaries

Activation roles must exactly match the activated job's event transitions, sorted by role, including original generation ordinals and after-states. Excluding another job's retirement transitions is required. Claim/takeover metadata must match the original dispatch and the decoded packet file payload length, retaining run/token/ordinal/previous-run and attachment checks. Encoded record size is not packet Markdown size.

Refresh needs special historical handling: the replay state has already applied transitions when check_request runs, so drift_cleared comes from the validated original refresh transition grammar, not the now-cleared role flag. Its response checkpoint is the checkpoint recorded by that refresh event, or none; an older checkpoint carried into the dispatch is a different fact. Checkpoint response event/run/generation likewise refer to its own recorded event. Release uses original event type for terminal run state and the target-first transition for role state, rather than a later dependent transition or current run/role state.

The source map agrees with the existing writer field inventory, typed recovery/claim/activation structures and exact P2-1/R16 response contracts. Original packet and manifest records remain trusted recorded projection artifacts under the existing record, digest and dispatch checks. Re-running current Lua would incorrectly substitute a new interpretation for historical authority. Preserve the existing complete load, check/check-recover and verdict verification passes and attachment rules. Init/read-only commands do not need invented saved-request branches.

Positive controls are as important as corruption controls: old byte-exact replay must survive later load, repair, takeover and activation without adding an event. Fresh invalid run authorization remains a refusal, while the equal original request continues to replay before current authorization. The plan correctly preserves that distinction rather than introducing new release idempotence.

## Independent test coverage repairs

The historical catalog test currently permits generic handler grammar to mask missing exact handler pairing. The grammar-valid shipped.unbound_probe.v1 mismatch across the five documented historical epochs isolates the intended predicate. The prober's retained baseline/mutant results support adding this test without changing catalog product policy.

The loader test must construct its declaration before recoverable growth and return it afterward, so no later constructor anchor can mask deletion of the execute-stage first-error check. The retained preconstructed-declaration witness distinguishes baseline success of the refusal assertion from the execute-guard mutant's unexpected accepted result. Reusing retry_sink and keeping loader implementation unchanged is the right boundary.

## Verification and limits

Read repository instructions, implementation/roadmap/v5 direction, C08 brief/map/tables, C07 source-review inventory and reproduction descriptions, relevant P2-1/P2-5 and R16 response clauses, and implicated reader/writer/codec/dispatch source. Inspected both independent witness result records. No build, test, corruption experiment or mutation was run by this reviewer.

Every typed response field must be covered by exact reconstruction or exhaustive comparison in implementation; header-only checks and array-length checks are insufficient. Independent structurally valid mutations, original-history positive cases, nonzero focused counts, zero skips and fresh code review remain required. The calibration, full host/Linux gates, final mutation campaign, no-work check and integrated acceptance remain pending. This plan introduces no schema, public API, Lua, loader, budget or product-policy change. R19 real-project acceptance remains separate.
