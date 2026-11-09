# R15–R18 C01 trusted policy corrective

Implementation outcome: scoped fixes and standalone regressions pass; independent review and parent-owned native gate remain required. No self-approval, commit, installation or shared build was performed by this lane.

## Changes

- F2: every captured producer target in native check evidence is compared with the producer's current publication on each evaluation/retry. A mismatch withholds completion with `input_changed`, retains the active run and still records evidence/observation.
- F3: `checks.role_completion` delegates reviewer completion/history to actual verdict applicability, retaining worker-only publication APIs. Check reconciliation, load reconciliation and release derive done/stale from the same role-kind-aware evidence. Blocked roles with obsolete retained evidence become stale; roles without evidence become ready. Matching restored verdicts become done. Reviewer applicability uses structural lineage, complete exact targets and current policy, independent of whether the verdict originally recorded a historical classification.
- F4: both completion and evidence-only checks install their own-unit observations into the hypothetical snapshot before deriving blocker clearing and downstream state. Clearing uses the post-result facts; cleared blockers are excluded before accepted-quality rederivation. Input snapshots remain unchanged.
- F1 coordination: review lineage no longer trusts absent verdict generation. Native snapshot provenance is owned by the ledger lane. Synthetic pending verdicts carry the reviewer generation so same-commit approval remains usable.
- Actual-module test integration exposed two adjacent current-work adapter details: acknowledged raw fingerprints now normalize to the canonical prefixed form, and absent material explicitly returns nil rather than zero Lua return values.

Owned product edits: `src/stdlib/planar/checks.lua`, `review.lua`, `reconcile.lua`, `commands.lua`. Tests: `tests/native/r15-r18-c01-policy.lua`, `src/cmd/planar/review_lua.t.cpp`.

## Regression evidence

The new suite exercises real `checks.lua` plus `review.lua`, not the old test shim. Initial run before product fixes: **10 tests, 9 failures, zero skips** (`tests/native/r15-r18-c01-evidence/policy/red.log`). Two additional never-completed unblock controls were added after correcting the retained-evidence branch. Final standalone run: **12 tests, 0 failures, zero skips** (`green.log`). Coverage includes stale captured producer vector, unchanged reviewer identity, recreated/missing reviewer lineage, target change/restoration, completion and evidence-only rejection clearing, preserved input snapshots, retained running target and blocked/obsolete/never-completed precedence.

`review_lua.t.cpp` now loads shipped checks and review modules together and registers the same 12-scenario regression program as a native Catch2 test. Existing assertion intent remains unchanged. Two fallback-material tests now disable the actual checks material hook. One admitted-verdict expected serialization used obsolete draft `text`/`number` precondition keys and omitted vector `run`; it was updated to the current approved execution-change keys (`request_id`, `job`, `fact_version`, `generation`) and run identity, without changing expected states, transitions, reasons or preconditions.

Executed commands:

- `lua tests/native/r15-r18-c01-policy.lua`: 12/12 pass, zero skips (final log above).
- `lua tests/native/r17-checks-policy.lua`: all three existing grouped applicability/readiness assertions pass (`r17.log`). This program reports grouped outcomes rather than a test count.
- `lua <scratch-root>/c01-native-lua-expressions.lua`: 79 exact string expectations extracted from the native test source, each with fresh actual-module fixture setup; 79/79 pass (`native-expressions.log`). This scratch extraction is supplemental and does not replace compiling/running Catch2.
- `/opt/homebrew/opt/llvm/bin/clang-format -i src/cmd/planar/review_lua.t.cpp`, followed by `--dry-run --Werror`: pass.
- `git diff --check`: pass at handoff.

## Limits

No native build, CTest, Doxygen, Linux or release run belongs to this lane. The parent owns serialized builds and full gates. The standalone fixtures test pure policy, not a timed native producer-publication race, CLI load/release roundtrip or persisted evidence-only transition. Native F1 provenance and final independent review are separate lanes. No approved contract amendment was made.
