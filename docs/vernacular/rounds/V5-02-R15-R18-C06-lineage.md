# C06 lineage correction

Implemented within the approved revised plan; **not self-approved**. Shared checkout changes outside ownership were preserved. Product source is frozen for independent review/calibration. Current file hashes and exact red-policy source hashes are in `tests/native/r15-r18-c06-evidence/lineage/IDENTITY.json`.

## Changes

`execution_review.cpp` excludes retired roles from current completion/repair projection. Persistent rows and independent historical validation remain untouched. `execution.cpp` filters current evidence and observed outputs by active non-retired owning worker; current publication selection also requires that owner. Every historical publication still undergoes payload verification before current-view filtering. Current completion lineage selection remains native and does not depend on the later-populated `role.lineage` field.

`reconcile.lua` constructs the complete post-operation owning-role view before deriving states. Activation starts with empty lineage and no inherited publications. Load resets changed structures, retires removed owners and creates new owner rows before filtering completions, publications, observed outputs and evidence. Both direct material and policy material use that view. Blocker clearing receives this same view through the internal `review.lua` adapter instead of recreating a shallow old snapshot. Historical facts are not deleted. Material-only changes preserve current publications even when a completion becomes inapplicable; takeover carries lineage.

`checks.lua` refuses retired current completions and constrains the observation fallback to the actual current owning role and lineage. Thus an obsolete completion with a later sequence cannot reintroduce old output when the explicit current-observation list is empty.

No ledger schema, public fault hook, producer-only fingerprint rule or generic error API changed.

## Tests and red evidence

The new tracked CLI suite derives the existing independent five-unit arithmetic workflow, preserving its genuine checks, ten exact vector assertions and five accepted units. It then exercises same-job activation, switching jobs sharing facet names, producer rename, structural regrouping/removal with consumer before producer, and material-only change. Each destructive-lineage scenario asserts blocked state, unavailable preview, refused claim, then readiness/availability after fresh producer completion. Every scenario checks persistent completion/publication/evidence counts and stable historical event data.

`red-cli.log`: four intended state assertion failures on the prior source-built Release binary; material-only control passed. Early test-construction mistakes (history envelope metadata comparison, role job field, and diagnostic envelope level) were corrected without changing product expectations. The final red log has the four intended assertion failures, not those packaging failures.

New actual-Lua controls use shipped checks/review modules with the existing policy fixture harness: obsolete observed fallback, takeover lineage control, and blocker clearing against a structural-reset view. Running these against the exact C05 copied source gives **2 intended assertion failures / 3 tests**, with takeover control passing (`red-policy.log`). The standalone host Lua run is supplemental; the shipped vendored-Lua C++ harness runs the green tests. The blocker regression follows R18-5: a changed target vector invalidates an old unowned rejection. An unchanged-view control retains it; there is no universal “must not clear” assertion.

The colocated ledger section retires a completed owner while keeping the same job active. It asserts empty current completion/publication/observed/evidence projections, retained persistent counts, readable old completion event, then corrupts the historical publication and requires ledger-corrupt refusal. Its first green attempt held a test reader open during corruption and hit SQLite locking; scoping the reader corrected this fixture defect. All semantic assertions before the lock already passed. Raw `green-ledger-1.log` is retained.

## Executed verification

All commands use source-built Release artifacts and temporary ledgers. No installed/legacy binary or operator database was used.

- Initial build typo `planar_ledger_tests` failed as unknown target (`release-build.log`); no success counted. Correct target is `planar_ledger_execution_tests`.
- `cmake --build build/rewrite-release --target planar planar_ledger_execution_tests planar_runtime_tests -j3`: passed (`release-build2.log`). Additional ledger fixture and policy target builds passed (`ledger-rebuild.log`, `policy-build.log`).
- `planar_ledger_execution_tests`: **163 cases, 103343 assertions passed**, zero skips (`green-ledger-all.log`).
- `planar_runtime_tests '[reconcile]'`: **12 cases, 1202 assertions passed**, zero skips (`green-reconcile.log`).
- `planar_review_lua_tests`: **14 cases, 979 assertions passed**, including C01 fifteen Lua controls and C06 three controls (`green-policy-all.log`). C06 focused: 1 case / 18 C++ assertions, all three Lua controls passed (`green-policy-native.log`).
- `python3 tests/native/r15-r18-c06-lineage-cli.py build/rewrite-release/bin/planar`: **5/5 scenarios**, zero skips (`green-cli-2.log`).
- C03 vectors CLI: **3/3** grouped, split, grouped-profile workflows (`green-vectors.log`).
- C04 publication CLI: **5 scenarios / 8 corruption assertions passed**, zero skips (`green-publication.log`).
- `python3 scripts/rewrite-check.py lint --build-dir build/rewrite-release`: pinned formatting and Doxygen passed over **249 active files** (`lint.log`). `git diff --check` passed.

A requested no-change build encountered regenerated/shared dependency work while calibration was active; **no no-work pass is claimed**. The command `cmake --build build/rewrite-release --target planar planar_ledger_execution_tests planar_runtime_tests planar_review_lua_tests -j3` remains running as session50564 at handoff, raw `lineage/no-work.log`; coordinator owns completion. Do not repeat this while shared calibration/CMake changes are possible. Final whole gates own stable no-work validation.

## Remaining independent work

Fresh review, faithful mutation probes for I7–I14, final calibration and Debug/Release/Linux gates remain coordinator obligations. No mutation was applied by this coder. This report does not claim full integration approval or R19 real-project acceptance. The C06 product source hashes are stable; later changes require renewed evidence/review.
