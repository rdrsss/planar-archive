# C06 independent plan review

Current disposition: **approve the revised bounded plan**. Initial review requested one narrow planning correction R1 below; the coordinator applied it before implementation. Diagnostic transport, exact-message and post-operation publication-view design otherwise fit the cited contracts. No product edits, builds or tests were performed.

## R1 — distinguish current snapshot projection from retained history

The proposed map says execution_review.cpp deliberately retains retired-role completions for history/recovery, and I13 treats removal from the snapshot like deletion of historical data. That is not established by the source or contract. R17-7 defines snapshot.completions as every completion of every current lineage of the active job; EX-5 states retirement ends a lineage. execution_review.cpp itself comments that only current lineages contribute, but currently does not filter retired roles.

Persistent completion/publication records and historical event/recovery readers must remain intact and verified. That does not require exposing retired completions in the current execution snapshot. In addition, checks.lua observed() falls back to all S.completions when S.observed has no entry: preserving retired completions there can reintroduce the very stale output removed by the new observed filter.

Revise the plan to distinguish those boundaries, authorize scoped execution_review.cpp current-projection filtering if needed, and assert absence of retired/nonowning records from current projections alongside unchanged historical readability. Include a regression for the Lua observation fallback; changing only native publications is insufficient. No storage deletion or historical reinterpretation is authorized.

## Other plan conclusions

The execution-owned check error plus command-local adapter is a suitable bounded implementation of R17-4/R17-10 and R18-2. Generic diagnostic serialization need not change. Preserve first-error order, resource details, native verdict field paths and verified active revision context. The new check-in-progress arm must retain same-ID replay/conflict behavior.

The activation/load view must represent all new ownership and reset lineages before deriving any consumer state, and the blocker-clearing adapter must use that same view. Takeover carries lineage; material-only changes cannot erase current publication solely because completion becomes inapplicable. The plan correctly excludes a new producer-only fingerprint policy.

The I1–I14 test/mutant obligations otherwise cover the reported failures and adjacent current-projection hazard. Tests must prove intended assertion failures on coherent baseline binaries; historical payload integrity remains verified before filtering. Source freeze, fresh calibration, focused regressions, mutation probes and full host/Linux gates remain outstanding. This review grants no implementation or integrated approval.

## Re-check

R1 is closed in revised map, DECISIONS D6/D11, INVARIANTS I13/I14 and C06 ownership. They now distinguish persistent/history retention from current snapshot projection, authorize the scoped execution_review.cpp selection correction, require retired-completion absence from current snapshots, and audit the observation fallback. The revised plan is approved for implementation. No new product decision is required.

This approval is limited to the plan, not candidate code, executed tests or final acceptance. Parallel coding is bounded by the stated disjoint ownership; builds require coordinator lane grants.
