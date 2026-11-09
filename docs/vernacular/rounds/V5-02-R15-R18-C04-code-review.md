# R15–R18 C04 independent code review

Disposition: **request-changes** for R1. This is a scoped source review, not full
integration approval. The reviewed files match the coordinator's pinned candidate;
generated hashes and comparisons are in C04 `review/identity.json`.
No product/test edits or builds were performed. The coordinator requested this
bounded handoff after the API finding so a fresh corrective coder can unblock the
full runtime build. Unexamined surrounding behavior is not implicitly approved.

## R1 — P2: preserve the typed preparation entry boundary

`src/engine/runtime/runtime.cppm:747` changes the format2 preparation method to
`prepare_load(const request_intent&, value_ptr execution_snapshot = {})`.
This makes the intentionally forbidden generic two-argument shape callable and
breaks the existing compile-negative assertion at
`src/engine/runtime/runtime.t.cpp:1101`:
`!can_prepare_with<const acceptance::request_intent&, value_ptr>`.
The coordinator's Debug runtime build surfaced this failure; it must not be
removed as a stale count/fixture assertion.

The preparation architecture's steps3–5 and public-surface invariant
(`profile-preparation-architecture.md:284–310,453`) deliberately exclude caller
bundle/model/tree authority. C04 authorizes additional execution snapshot input,
not a return to an undifferentiated caller-value preparation interface. The
approved C04 map calls for a format3-specific input/overload with frozen snapshot
binding, and the plan review preserves the existing opaque authority boundary.

Correction: retain the original one-argument format2 `prepare_load` surface and
its generic-value negative assertions. Add a distinctly named format3 preparation
method or a narrowly named execution-input wrapper containing only the snapshot;
validate and copy it in the same owner/frame. Add a positive compile check for the
new deliberate surface and keep negative checks against unrelated bundle/model,
prepared objects and arbitrary multi-value overloads. Preserve the native adapter's
comparison between the frozen snapshot and its actual captured native snapshot.
The wrapper need not introduce a runtime-to-ledger dependency or claim authenticity
that a public wrapper constructor does not provide.

This is a concrete API/build regression, not a demonstrated forged-declaration
exploit. The current implementation structurally validates the snapshot and the
private completion adapter compares its frozen copy to the native snapshot;
those defenses should remain. The coordinator explicitly accepted this bounded
API correction. Final API/code verification must examine the corrected shape,
not merely delete the failing static assertion.

## Examined implementation and evidence

The profile adapter uses the original profile-bearing request intent for lookup
and execution staging. It returns a staged equal replay before native receipt
verification, and fresh commits verify original receipts before committing.
No internal load retry or second factory is visible. Reconciliation occurs through
the retained fixed export before `cleanup.run()`, and both input conversion and
nested call share the original cumulative owner. The candidate's source/registry/
effective-provenance records and interpretation_v2 are used for format3 staging;
the format2 completion path remains separate. The shipped history selector/ID
mapping correction does not widen the external capability ABI.

N8 derives the publication declaration from the dispatch's retained revision,
checks own-unit membership and exact declared file paths, reads the retained tree,
and recomputes the approved `facet_v1` semantic including normalized modes and
file digests. The check covers historical rows as well as current publications.
The event codec now binds each carrying event to its exact operation. These
source observations support the intended corrections but are not a claim that
all nearby corruption or race invariants have been exhaustively verified.

Inspected coder/coordinator evidence: profile CLI5 scenarios; grouped/split/profile
3 scenarios; native preparation2 cases/34 assertions; existing format2 acceptance
29 cases/3494 assertions; publication CLI5 scenarios/8 corruption assertions;
ledger163 cases/102531 assertions and codec31 cases/1555 assertions. These were
not rerun by this reviewer. The root's publication green log supersedes the
publication coder report's pending-CLI note.

C05 historical-catalog compatibility and test-budget recalibration are separate
ongoing packages. Full Debug/Release/Linux gates, remaining faithful probes and
fresh final integrated review are still required. No R19 real-project execution
claim is made.
