# C06 independent policy, profile and command review

Disposition: **request-changes** for three reproduced defects. This review covers the integrated execution changes since `031ea4fc`, not only C06. It does not approve the full gate, resource calibration or mutation evidence, which remain pending. No product source, tests, build artifacts or operator data were modified. Only this report and its scratch evidence were written.

## P-C06-1 — high: a new completion can retain a withdrawn facet in reconciliation

At `src/stdlib/planar/checks.lua:197–198`, `rederive` adds new publications and retains old publications unless their facet ID occurs in the new list. This is not replacement of the producer's current publication set. If a new successful completion has no publication for a formerly published facet, the old publication incorrectly survives in the hypothetical snapshot used to derive downstream states.

Reproduction: complete and approve the real five-unit split arithmetic fixture, make worker requirements for `s` and `b` material again, claim `s_worker`, remove its owned `s/value`, and pass a valid trusted check that allows the absence. The new check completes with `publications:[]`. Native current publication selection correctly makes `s_to_b` unavailable, as preview shows. However `b_worker` stays `stale` rather than `blocked`; a fresh claim succeeds with an explicitly unavailable start prerequisite in its returned manifest.

Authority: R17-6 publication membership, R17-7 current publication belongs to the producer's current completion, and EX-4 missing-start precedence. The correction should replace all current publications of the completing producer/role with the new completion's exact set, preserving other producers. It must cover an empty set, partial disappearance when a producer owns several facets, and ordinary replacements. Do not change the separate reviewed producer-only facet-definition fingerprint policy.

Evidence: `tests/native/r15-r18-c06-evidence/policy-profile-review/missing-facet-probe.py`, `missing-facet-red.log`, `missing-facet-red.json`. Script exits 1 on the required blocked-state assertion after recording successful premature claim. This is a public workflow; no ledger corruption or injected source fault was used.

## P-C06-2 — high: role-only map aliases roles of different jobs

At `checks.lua:185`, `rederive` copies every snapshot role, including retired roles of other jobs, into `original[r.role]`. At lines194–196 it retrieves `original[plan.role]` to build the completing role's hypothetical completion, and line226 retrieves the original transition state the same way. Role IDs are only unique within a job. A later other-job row overwrites the active role.

Reproduction: add `other` with the same role IDs as `job`; complete/approve `job`, activate `other`, then reactivate `job`. Claim `s_worker` and run its first check. The check returns exit4 `internal` / `Invalid embedded command protocol`, leaving the recorded reservation open. The retired `other/s_worker` has overwritten the active `job/s_worker`; its non-running row is used where the code expects the active run/dispatch. The failure is sensitive to job ordering; the existing one-way switch test moves to the lexically later job and does not catch it.

Authority: execution roles and generation ownership are keyed by `(job,role)` throughout EX-1/EX-3/EX-5. Scope the `original` map to the active job or key it by both identifiers, and audit the other hypothetical indexes for the same assumption. Cover switches in both lexical directions, repeated switches and shared role IDs; prove successful check completion, downstream states and recoverability of previously open reservations.

Evidence: `switch-back-probe.py`, `switch-back-red-4.log` and `switch-back-red.json` in the same directory. The annotated final run identifies `recheck role=s_worker` before the exit4 assertion and reads the resulting `again_check_s_worker` reservation as `open`. `switch-back-red-2.log`/`switch-back-red-3.log` are earlier versions of the same failure; the first `switch-back-red.log` is an excluded probe-construction syntax error, not evidence of the product defect.

## P-C06-3 — medium: early execution replay drops the stdout failure message

The early replay returns for run and recovery option families in `src/cmd/planar/invocation.cpp:2353,2359,2527,2534` construct only three response fields. They omit the already-built `stdout_error`. A broken output stream therefore falls through `main.cpp` to generic `Cannot write stdout`, instead of the operation-specific EX-7 message explaining that the operation may be complete and can be recovered by retry/history.

Independently reproduced for claim replay in both embedded and with-profile compositions: the complete fixture succeeds, then the identical original claim is replayed with stdout connected to a pipe whose read end is closed. Both return exit4, code io, operation stdout, but the wrong generic message. The same omitted-field source branches cover takeover, refresh, checkpoint and release. Neighboring check/activate/load replay branches preserve the fallback correctly.

Authority: EX-7 post-commit fallback table and EX-3 output/replay recovery policy. Preserve the prebuilt operation-specific stdout fallback in all four returns. Add closed-pipe replay controls for both option families and both compositions, plus ordinary exact replay controls; no fault hook is needed.

Evidence: `replay-stdout-probe.py` and `replay-stdout-red.log`. This collection script catches and prints two expected assertion failures and exits zero; its exit status is not passing product evidence. Both outputs are recorded explicitly.

## Reviewed scope and previous corrections

Read the active repository guide, implementation/roadmap and v5 direction, exact execution R15–R18 clauses and associated render/load/profile/authoring contracts, and C01–C06 records including the revised C06 plan. Examined the complete new checks/reconcile/review/dispatch Lua modules and command changes, including per-work evidence vectors, current versus applicable completions, stale producer vectors at completion, repair identity, reviewer completion/approval lineage, blocker clearing, release rederivation, and the C06 post-operation load/activation view. The two new policy findings above are in check rederivation, distinct from C06's load/activation corrections.

Also reviewed authoring orchestrator changes and CLI composition/parser/catalog integration: live state/actions and blockers, graph-only behavior, the mutually exclusive ledger versus workspace/registry/job forms, sealed available-command construction, profile history event selection, opaque host-slot use, and command service order/argument checks. The current historical catalog split retains strict17-row live admission and exact documented historical epochs; no new issue was found in that correction.

Reviewed C04/C05 profile preparation and commit paths: named preparation API, null refusal, bounded copied execution snapshot, retained fixed reconciliation export inside the original frame, frozen prepared binding, original receipts, effective-provenance/interpretation-v2 staging, exact early/late replay ahead of receipt checks, and post-commit uncertainty. No additional source defect was identified in that scope. This source review does not substitute for the pending full gates or profile mutation evidence.

The native reviewer separately owns ledger/history/process internals and identified saved-verdict cross-row verification and consumed-unit observation concerns; this report neither duplicates nor approves that scope. Existing carry limits, resource calibration and final exact artifact coherence remain coordinator obligations. R19's unselected real project is not certified by these fixtures.

## Execution and identity

All three scripts use the explicit source-built Release executable, SHA256 `a58dd96edab4c547ef529536ced04f83d51da0a1f9c93299e0678fc95a4ce79c`, only disposable ledgers and temporary source roots. Each derives from the independent C03 real workflow and preserves its five accepted units, exact evidence vectors, real checks and retained facet-byte controls before the added scenario. No build, mutation campaign, installation or commit was run. `SOURCE-IDENTITY.json` records the reviewed principal source hashes and baseline HEAD; hashes are an identity record, not blanket approval.
