# R15–R18 integrated review: C06 required

Disposition: **request-changes**. This fresh broader pass found three actionable defects after C01–C05. It is not final integrated approval. No product source edits, builds or commits were performed by this reviewer. Tests below used the existing source-built Release executable and disposable ledgers, with no installed binary or operator data.

## F-FINAL-3 — high: publication availability crosses producer lineage retirement

Two public workflows reproduce a start-prerequisite bypass:

1. Complete and approve the five-unit split arithmetic fixture. Unit `b` has a start/available facet dependency on `s_files` from unit `s`. Reactivate the same job. Every worker receives a fresh generation without completion. `b_worker` is nevertheless persisted `ready`; its preview correctly says `s_to_b` unavailable. A fresh claim of `b_worker` succeeds.
2. From the same complete/approved fixture, load a job replacing owner `s_worker` with `s_recreated` for unit `s`. The former owner is retired and the new owner has never completed. `b_worker` becomes `stale`, its preview still calls `s_to_b` available, and a claim succeeds. The old owner's retained publication is being treated as the new owner's current prerequisite.

Authority: EX-4 precedence requires a missing start prerequisite to yield blocked; EX-5 opens a new structural lineage on activation/recreation; R17-7 defines current publication through the producer role's current completion. Retained historical publications are valid history, but cannot be used as completion evidence of a new owning lineage.

Source map:

- `src/stdlib/planar/reconcile.lua`, activation branch: `publications=S.publications` is passed to `role_material` while every role receives a new generation. Empty `states` does not remove an `available`-quality facet.
- The load hypothetical view copies `S.publications` while resetting/removing producer generations. Both its material calculation and the clear-blocker adapter need a view coherent with the post-load ownership/lineages.
- `src/engine/ledger/execution.cpp::read_check_rows`: publication selection finds its completion in `snapshot.completions`, then checks only whether a newer completion exists for the same old role. It does not require that this non-retired role remains the active owner of the published work. Retired roles remain in the snapshot for legitimate history/recovery use, so existence in the snapshot is insufficient.
- `src/stdlib/planar/dispatch.lua::role_material` indexes publications by facet; `checks.lua::material` tests facet ID presence. These rely on the native/hypothetical publication view being current.

Correction boundary: retain historical rows and recovery access; correct the current publication view and post-operation hypothetical views. Cover same-job activation, switching jobs with shared spec facet names, owner replacement/removal, and structural replacement of the same producer role. Check downstream states, unavailable prerequisite, refused claim, and restoration after a fresh valid producer completion. This does **not** call for changing the reviewed W recipe or introducing producer-only facet declaration fingerprints.

Red evidence (each final script exits 1 on the expected blocked-state assertion; each includes a successful premature claim):

```
python3 tests/native/r15-r18-final-evidence/review/reactivate-facet-probe.py
python3 tests/native/r15-r18-final-evidence/review/recreate-facet-probe.py
```

Both scripts derive from the C03 independent arithmetic workflow and preserve its actual checks, ten vector assertions, retained facet checks and five accepted units before the new negative scenario. JSON observations and complete failure logs have matching `*-red` names in the review evidence directory. No ledger corruption was used.

## F-FINAL-1 — medium: typed execution refusal context is dropped

`invocation.cpp` converts `verdict_error` to the generic `diagnostic` at its `read_verdict` service boundary. It copies code/message/limit/maximum but omits the existing `field` member. Generic `response.cpp::native_error` always emits an empty field. Consequently malformed JSON emits schema field `""` instead of `verdict`, and native verdict shape failures also lose `verdict.<key>` paths. R18-2 steps 3/5 explicitly require these fields; the native verdict module already computes them correctly.

The check preparation boundary has the same incomplete transport: `execution/check.cpp::prepare_check` returns generic `diagnostic` with no field/context, and invocation forwards it directly through `native_error`. Reproduced scratch overlap and reserved `PLANAR_` environment errors have field `""` instead of `scratch` and `registry` (R17-10). Source also shows no carrier for the R17-4 contract-delivery `{expected:<active revision>,actual:"working_bytes"}` or vector-precheck `{work:<producer>}` details. Those two missing detail cases are source findings, not independently executed reproductions in this pass.

Narrow source map: `execution/verdict.cppm::verdict_error`, `execution/verdict.cpp::{parse_verdict_bytes,normalize_verdict}`, `execution/check.cpp::{fail,prepare_check}`, `execution/check.cppm` exported error boundary, `cmd/planar/invocation.cpp` check service and verdict service, and `cmd/planar/response.cpp::native_error`. Prefer an explicit execution-owned error adapter/type carrying approved fields/details; do not encode semantic fields as resource `limit` or change unrelated generic error bytes. Preserve native verdict field information through both parse and normalize failures. Add CLI assertions for exact code/message/field/details and unchanged ledger on refusal, plus delivery/vector negatives with multiple producers to prove the named first mismatch.

## F-FINAL-2 — medium: open-check guard emits another error's message

`cmd/planar/invocation.cpp::execution_code` lacks a `check_in_progress` arm. The check `read_execution` service calls that helper when a different request targets a run with an open reservation. It emits code `check_in_progress` but message `Execution state changed since the request was prepared`; R17-9/10 requires `A check is already in progress for this role`.

The repro starts an ordinary trusted three-second check, waits until its reservation is open, then invokes check using a fresh request ID. The original child completes normally. Add a CLI regression distinguishing this path from same-ID early reservation replay, whose native mapping may already be correct. Audit fixed-message call sites rather than weakening expected diagnostics.

Combined red command:

```
python3 tests/native/r15-r18-final-evidence/review/diagnostic-probe.py
```

`diagnostic-probe-red.json` records five scenarios: malformed verdict JSON, empty verdict object, scratch overlap, reserved registry name, and concurrent open reservation. All return exit 3; their incorrect context/messages are retained. This exploratory command exits zero after collecting observations; it is evidence of defects, **not** a passing correctness test. Earlier individual JSON observations are also preserved.

## Review scope and limits

Read repository authority and current C01–C05 consolidated records/C05 scoped approval, execution commit/lineage/recovery/check/review clauses and v5 execution direction. Examined the complete checks/reconcile/review/commands Lua modules, relevant dispatch material, native digest capture/check execution/process runner, check invocation transport and commit path, snapshot publication/observation selection, and diagnostic boundaries. Traced previous corrected behavior including stale-vector withholding, reviewer-specific completion, repair identity, evidence-only blocker clearing, release rederivation and native locked identity.

The initial broad review was stopped on these confirmed findings at coordinator request to free a slot for a fresh C06 correction. It did **not** finish an exhaustive review of every native ledger/history/profile/authoring path; no approval of unexamined scope is implied. C05's existing historical-catalog/profile-API approval remains separately scoped. A fresh final reviewer must cover the corrected integrated candidate and all remaining broad scope after C06.

`review/source-sha256.json` captures per-file source inventory at this pass, including unexamined files; it is an identity inventory, not a claim that every file was reviewed. Full Debug/Release/Linux gates, final mutation campaign and resource calibration were pending. The coordinator reports Linux toolchain warmup blocked on unavailable pinned LLVM packages; no candidate Linux pass is claimed. C06 Lua changes require calibration identity to be revisited; calibration agent was notified.

R19's real operator project remains unselected. Fixture success cannot certify that external acceptance exercise. Producer-only facet definition behavior described by the C05 reviewer remains contract-defined and is distinct from the new retirement/ownership leakage above.
