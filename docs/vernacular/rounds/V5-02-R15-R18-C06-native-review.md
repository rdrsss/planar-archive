# C06 integrated native review

Disposition: **request-changes** for N-C06-1 and N-C06-2. This is a fresh native source review and two executed counterexamples against the corrected C06 candidate; it is not final integrated approval. Product source and shared build outputs were read-only.

## N-C06-1 — P2: verdict replay accepts contradictory saved results

`src/engine/ledger/execution.cpp::check_request` checks the saved verdict header against the event. `execution_review.cpp::check_verdict_rows` checks verdict ID, applicability, decision, run and event; `check_admission_replay` checks admission and, only for current rejections, count and cap. It does not reconstruct the complete saved response from the retained event, transitions, repairs and blocker rows. Approve/historical branches skip count and max_rounds verification entirely. Saved repaired generations/revisions, opened/cleared blockers and returned role states lack exact cross-row checks.

Executed four independent corruptions after an ordinary successful worker check and reviewer approval. Each changes only `requests.result` for the verdict, dropping and restoring the exact immutable trigger SQL around the scratch corruption:

- `review.count`: `0` → `999`;
- `review.max_rounds`: `2` → `999`;
- returned reviewer role state: `done` → `ready`;
- `blockers.cleared`: `[]` → `["999"]`, where blocker999 does not exist.

Every corrupted ledger passes `status` with exit0. Every identical verdict request replays the fabricated field with exit0. For example status reports actual review count0 while replay reports999. This is reader-integrity failure, not a claim normal API clients can bypass immutable triggers. R18-4's exact replay rule and the verdict_response_v1 response schema (R18 contract line286 onward) and the ledger reader's cross-row verification obligation require `ledger_corrupt` before returning such retained output. The analogous saved-check defect was already fixed in C02; its exact reconstruction approach should be applied here.

Evidence: `tests/native/r15-r18-c06-evidence/native-review/verdict-saved-probe.py`, `verdict-saved-probe.log` (first count-only demonstration) and `verdict-saved-probe-matrix.log` (all four). Release executable SHA256: `a58dd96edab4c547ef529536ced04f83d51da0a1f9c93299e0678fc95a4ce79c`. The collection harness exits0 to retain observations; this is **not a correctness-test pass**.

C07 boundary: `ledger/execution_review.cpp` verification and `execution_review.t.cpp` corruption tests; `execution.cpp` only if shared verification context is needed. Reconstruct the complete canonical verdict response, including event header, normalized verdict identity/decision/applicability, native admission/count/cap, actual repair ordinals/revisions, opened/cleared blocker rows and transition after-states. Use the revision/activation at the original event, not today's state. Test approve, admitted rejection, capped/unowned and historical outcomes independently, including exact unchanged replay after later load/activation. No schema or result shape change is needed. Test corruption of each omitted field family without another mismatch masking it.

## N-C06-2 — P2: current observed output is still seeded from a consumer's historical vector

C06 correctly filters retired roles from current completions and adds ownership checks to evidence, publication and check-event observation projections. However, `execution.cpp::read_check_rows` still initializes `observed` from **every unit of every retained completion** without checking that the completion's role owns that unit. `execution_review.cpp::load_completions` populates `completion.units` from the entire captured observation, which also contains consumed producer units. A surviving consumer completion can therefore reintroduce output from a retired producer. The later event scan skips retired producer events and does not erase this seed.

Executed the actual five-unit split arithmetic workflow (real registry checks, exact vector assertions and approval), then loaded a job replacing complete-phase producer `a_worker` by `a_recreated`. The new owner had never run. A tiny native executable calling `read_execution_snapshot` returned:

```
observed a xxh3_128:466bf1b4072acc8671f868af757c0ff3 15
completion b_worker a xxh3_128:466bf1b4072acc8671f868af757c0ff3
completion b_worker b xxh3_128:1c925e44c10c2e500a351a73a5ab42b3
```

The old `a_worker` completion is correctly absent; the contaminating value comes from surviving `b_worker`'s captured producer target. The assertion requiring no current observed `a` failed. R17-7 defines observed output from the owning role's current lineage and its own-unit observation; C06 I14 requires the same. A start-only facet test misses this because its producer is absent from the consumer's selected output vector.

Evidence: `observed-owner-probe.py` and assertion-failing `observed-owner-probe.log`. `snapshot-probe.cpp` is a tiny read-only reader, compiled and linked outside the checkout using the existing coherent Release module map and libraries under an explicit temporary build-lane grant. Compile/link logs and command record are retained. No shared build artifact was modified. The compile lane was returned immediately. All fixture ledgers were temporary and removed.

C07 boundary: `ledger/execution.cpp` observed initialization, plus native regression in `execution.t.cpp` or `execution_review.t.cpp` and/or tracked CLI fixture invoking an existing native test seam. Filter seeded units by the current owning role and completion lineage, or remove the redundant seed if complete event scanning proves equivalent. Do not delete consumer vector history or change completion payload grammar simply to hide the leak. Add complete-phase producer replacement with surviving consumer completion, absence before new producer observation, correct output after fresh producer check, and unchanged retained history. The existing singleton retired-completion test is insufficient.

## C06 corrections and reviewed paths

The diagnostic package addresses F-FINAL-1/2 within its approved boundary: check-owned error transport retains registry/scratch fields and producer work, the adapter supplies delivery revision from verified native state, native verdict parse and normalize fields survive, and the overlapping-check helper emits the exact message. Generic `diagnostic` and unrelated serialization stay unchanged. Inspected the12-scenario CLI/native evidence report and the assertion-bearing scripts; did not rerun those suites. Expanded cases have green-only evidence and still need independent mutation controls as recorded by the coder.

Current native publication filtering and `read_review` retirement filtering address the original publication leak while preserving verification before filtering. N-C06-2 is a remaining separate projection path, not a request to undo those corrections. Lineage Lua policy is under the separate reviewer; no approval of that package is implied here.

This pass read the repository instructions and inherited implementation/roadmap/v5 direction, C01–C06 findings and correction records, the revised C06 plan and R17/R18 exact clauses. It examined the integrated native source rather than only the working diff: request/result checks, event replay boundaries, evidence/completion/publication projection and verification, verdict row/admission/repair/blocker consistency and locked writer checks, check preparation/execution/outcome classification, process spawn/drain/termination lifecycle, verdict capture/normalization, request framing and command diagnostic transport. Prior broad native reviews were read for their findings and scope; no new defect is claimed in the inspected process cleanup, boundary capture or lock-time verdict admission paths.

This was a bounded failure-finding pass, not a fresh line-by-line audit of every schema/codec/storage/helper branch since `031ea4fc`. The full historical codec/schema matrix, generic storage changes and every claim/refresh/load transaction path were not exhaustively re-reviewed here; existing scoped reviews/tests remain evidence, not automatic blanket approval. No new disk-full, kill-at-commit, concurrency or platform experiment was run. The coordinator requested report/slot return for C07 after the confirmed findings. Full gates, final native mutations, calibration and final evidence review remain outstanding.

Identity inventory and raw evidence hashes are in `native-review/source-identity.json` and `evidence-hashes.json`; an inventory entry does not imply every line was reviewed. No installed or legacy Planar, operator data, commit, merge, publish or installation was used. R19 real-project acceptance remains separate.
