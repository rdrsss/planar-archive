# R19-CMAKE plan review

Disposition: **approve the bounded plan**. No unresolved product decision or
blocking planning finding remains. This approves implementation and verification
of the engine CMake organization task; it is not a code approval, an executed
check result, or closure of the complete R19 acceptance matrix.

The prescribed Claude plan-review model is unavailable in this runtime. A fresh
ordinary agent performed this independent review instead. The checkout is shared;
the reviewer changed only this report and its ignored evidence folder. No build,
Planar command, source, workspace, ledger, installed binary or operator database
was executed or changed by this reviewer.

## Reviewed inputs and authority

Read `AGENTS.md`, the implementation plan and roadmap opening, the v5 scope,
`orchestration.md`, the R19 round requirement, `R19-CMAKE.md`, both search maps
and the full canonical `plan/DECISIONS.md` and `plan/INVARIANTS.md`. The accompanying
`plan-review/REVIEW.json` pins these task inputs and supporting source/helper files.
The starting candidate is `5812823430ea24804519f02620f05ed546386607`.

The operator named this real project and requested one CMake file per engine
directory. `add_subdirectory()` preserves the existing single-project semantics
and is the appropriate interpretation of the requested composition. No new
product schema, authority, dependency or runtime behavior is proposed. All 15
descendant directories, including `runtime/closure_guard`, are covered, making
16 CMake files with the engine coordinator.

## Contract and invariant assessment

| Rows | Assessment |
| --- | --- |
| D1-D4, I1-I4 | Scope, directory reachability, exact target/type/source membership and resolved graph edges are explicit. Moving complete declarations to owning children avoids a second domain graph or nested `project()` path change. Relative sources and left-hand target amendments receive specific scrutiny. |
| D5-D7, I5-I7 | Shared Python/helper discovery, ordinary variable scope, generated source rewrites, compile definitions/ceilings, test command arguments/timeouts and working directories are identified. Only exact owner-derived path rebases may normalize; arbitrary generated/source/definition differences still fail. |
| D3/D7, I8 | Default `all` exclusion and `planar_tests` membership are independently observed, rather than inferred from target existence. Do not add directory-level `EXCLUDE_FROM_ALL` or remove existing target exclusions. |
| I6/I9 | Fresh configuration and real builds/tests/no-work repeats are required; stale artifacts cannot certify the changed build. Nonzero selection and zero skips remain required, with applicable pinned formatting/Doxygen. |
| D8-D9, I10 | Historical evidence remains immutable, unrelated C++/Lua changes are outside ownership, current references change only as affected, and the real-task demonstration does not claim the outstanding quota/fault/R19 obligations. |

The recorded Debug and Release baseline CTest JSON each contain 1,844 tests.
The actual baseline codemodel and Ninja records are retained for comparison.
The table's concrete breaking edits are proposed sensitivity checks, not executed
mutation evidence; subsequent reports must name what actually ran and retain
failures or limitations.

A clean separate configuration is the most direct way to expose interpreter
variable scoping that an existing cache might mask. Compare actual codemodel
target/source/edge data and generated payloads, plus complete test properties
and default/test dependency closures. The reviewed owner-derived path allowlist
must be fixed before deciding whether differences pass. Do not accept an
unexplained difference merely because a build succeeds.

## Driver compatibility issue resolved in the plan

The initial map's executable example used the production `bin/` location for
the authoring target. The coordinator's File API check establishes the actual
artifact at `build/rewrite-debug/src/cmd/planar/planar_authoring_stack`; this
correction is now appended to the driver map. Resolve the actual target path
rather than assuming a copied or installed binary.

Source review also establishes that using one authoring-stack executable does
not ensure one runtime bundle. `bootstrap.cpp` dispatches execution families
through fully qualified original handlers without its authoring VM; the claim
handler imports the original invocation module. Renamed init/load/authoring
paths therefore coexist with original execution paths. This is a compatibility
question to measure, not evidence of a new bug or a reason to silently change
product code in this refactor.

The coordinator added an explicit pre-implementation condition to the round and
driver map: actual fresh-ledger init/load/activate/claim, live orchestrator and
recovery must be attempted and retained before coder dispatch. Continue the
authoring trial only on observed success. On refusal, preserve it and run a
separately initialized production-only task trial, recording the orchestrator
coverage gap. No success or shared-bundle identity may be assumed. This resolves
the planning concern without inventing a product decision.

The actual `src/engine` root, immutable starting-engine baseline and disjoint
scratch/state avoid repository instruction links and generated output bulk.
Sixteen exact CMake boundaries permit no C++ edits. Trusted adapters run actual
repository verification with explicit executable/environment values, retain raw
results outside the captured root and derive result counts from executed tests.
Coordinator prose changes outside this root remain separately reviewed work.
Independent review must precede any approving CLI verdict; replay and restart
claims require actual saved-result/retained-state evidence.

## Handoff

Proceed with the preflight and scoped implementation under the recorded ownership.
Final review must inspect the exact candidate, path/graph equivalence, actual gate
and driver results, and source-boundary integrity. Host-only execution is not
Linux validation; record platform limits. R19's full quota groupings, applicable
matrix/fault coverage, conditional A24 and minimal provenance obligations remain
separate unless actually demonstrated and reviewed. No installation, pushing,
publishing or merging is approved.
