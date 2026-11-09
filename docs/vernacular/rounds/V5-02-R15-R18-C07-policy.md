# C07 policy correction

Implemented P-C06-1/P-C06-2 under the approved C07 plan. No independent approval or integrated acceptance claimed.

`checks.rederive` now replaces the complete publication set of the completing producer `(job,role)`, including absent facets; it retains unrelated producers and leaves evidence-only checks unchanged. The hypothetical snapshot supplied to blocker clearing and downstream state derivation therefore has the same current publication membership as the next native snapshot.

The original-role and explicit-transition maps are restricted to the active job. Repair lookup and hypothetical repair increments also use active-job identity, preserving unrelated same-name repair rows. The derived transition map was already inside an active-job loop. The `review.lua` after-transition map is built from the active-job verdict transitions and consumed only by `roles_of(snapshot,job)`; no change was needed there. Persistent completion/publication records, captured consumer vectors, W and current-versus-applicable semantics are unchanged.

## Evidence

Raw logs and copied smoke sources are under `tests/native/r15-r18-c07-evidence/policy/`. Red runs preceded the product correction:

- Actual shipped Lua via the existing fixture library: **6 tests,5 failures,1 passing evidence-only control** (`red-lua.log`). The failures cover complete/partial facet withdrawal, cross-job completing-role selection, downstream transition before-state and repair lookup. The wrong-job completion path raises its existing missing-dispatch error; the assertion-bearing runner records that expected regression failure.
- New CLI against the prior source-built Release: **3 scenarios,3 failures** (`red-cli.log`): withdrawn-facet state and repeated switching in each lexical direction. All start from the real arithmetic workflow with exact vectors and valid approval.
- Corrected standalone Lua: **6/6 passed**, zero skips (`green-lua.log`). It also verifies input-snapshot immutability, unrelated facet preservation, grouped partial replacement and active-only repair increment.
- Coherent Release build of `planar`, `planar_review_lua_tests`, `planar_runtime_tests` and `planar_lua_loader_tests` passed (`release-build-2.log`). The earlier attempt named the nonexistent `planar_loader_tests`; it failed before compilation and is excluded, preserved as `release-build.log`.
- Full actual-Lua native harness: **15 cases,990 assertions passed** (`green-native-lua.log`), including all prior review/C01/C06 controls and C07's six controls.
- New CLI: **3/3 scenarios passed**, zero skips (`green-cli.log`). Withdrawal yields blocked state, unavailable preview and refused claim; a fresh completion republishes and restores readiness. Both lexical job-switch directions run four alternating activations, complete every worker with exact replay, leave no open reservation and make the reviewer ready. Original persistent completion/publication rows remain byte-for-byte equal as SQL row values. The earlier pre-fix failing reservation is not imported into the green fixture; that historical failure's recoverability remains covered by existing check-recover controls, not a claimed new C07 reproduction.
- Prior CLI controls: C06 lineage **5/5** and C03 vectors **3/3** (grouped, split and grouped-profile; ten vector assertions and five accepted units each) passed (`green-c06-lineage.log`, `green-c03-vectors.log`), zero skips.
- Pinned formatter dry-run/Werror passed the owned C++ harness; Python syntax check and `git diff --check` passed. This package changes no exported C++ API. Full applicable Doxygen and integrated gates remain coordinator-owned.

The final CLI adds history/republish success controls beyond the initial failing assertions; those are green controls, not independently demonstrated red tests. Fresh mutation probes must validate all C07 I7–I10 fault sensitivities. No mutations were applied by this coder.

## Delegated loader verification

After the loader coder's final test-only anchor assertion was saved safely after its previous TU link, this agent rebuilt the exact loader target and ran its full suite: **21 cases,186 assertions passed** (`loader/build-final-delegated.log`, `loader/green-final.log`). The unchanged runtime big-Lua test also passed: **1 case,34 assertions** (`loader/biglua-green.log`). These are executed focused results, not loader code approval or complete calibration.

## Remaining integration

Final Debug/Release/Linux gates, measured reduced-budget calibration, coherent native/Lua mutation probes and independent review remain pending. No no-work claim, commit, install, publication, merge or operator-database use. Product source is frozen; all CLI ledgers/checkouts were disposable.
