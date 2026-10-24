# R07-B1-C01 independent full B1 review

Disposition: approve.

Reviewed the complete effective B1 candidate through C01 against CLAUDE.md,
implementation/roadmap, the assigned B1/C01 records, v4, exact declaration/render
contracts, stdlib API documentation, retained prior review, all seven effective
candidate files and coder raw evidence. No product source/test edits, Planar tools,
queue, commits, installation, real database or concurrent builders.

## Candidate identity

C01 candidate manifest SHA256
144c5b36e27837cd355c0f9521ee5380d0d474cc1aea7e9c835922d853a46004;
evidence manifest 78a9eefe7978f59939f901992d8afda29fa99ec473f7f59c51e366c11c820b32.
All four C01 files and every evidence member matched before review and again at
completion. The other three effective B1 files (cmake/embed_lua.cmake,
src/kernel/CMakeLists.txt, stdlib/planar/bootstrap.lua) still match original B1
candidate.sha256, whose manifest identity is
29269386db7ad14a320af1ee829d528151b309e9704ee80e8cff91cd652d5ce4.
Final raw checks are in final-manifests.log.

## Findings and corrected contracts

No remaining findings within B1 scope.

- schema.lua:114-132 now emits the exact final inline/file text contract,
  preserving bytes and workspace-root declaration-relative provenance. Route-only
  uncaptured file descriptors remain explicitly provisional. Wrapper allowance
  stays restricted to notes/rationale.
- schema.lua:38-63 separates uniqueness from ID sorting. schema.lua:95-110
  sorts ID-keyed records with metered Lua merge sort, retains original-index
  provenance, and preserves acceptance criteria, outer stages, command/argv and
  environment order. workspace/job/toolchain diagnostics at :171-194/:292 use
  original positions. The returned bundle jobs follow sorted workspace IDs.
- json.lua:115-127 checks depth at container entry, allowing exactly 128
  containers whether empty or populated and refusing the 129th.
- schema.lua:122-124/:208-213/:254-280 validates joined text/spec/job paths
  before lookup or dependent route diagnostics. Invalid joined paths use the
  valid declaring workspace path and original workspace field/index.

The rest of B1 is coherent: exact record fields, private constructor types,
primitive/list/set bounds, defaults, intrinsic role/shape rules, strict JSON,
trusted registry schemas/header matching, every listed/supplied job normalized
before matching, and capture-only text resolution. No filesystem capability or
native schema policy was added. Fixed product embedding is compile-selected,
with no runtime require, module paths or job-selected source.

## Independent verification

- cmake --build build/rewrite-debug --target all planar_runtime_tests:
  exit 0, ninja reports no work.
- ctest --test-dir build/rewrite-debug -R '^replacement\.(schema|runtime)'
  --output-on-failure --timeout 30: 35/35 passed, zero skips/failures, 7.61s.
  The raw CTest LastTest.log is retained as ctest-LastTest.log.
- Compiled review.cpp from an exact copy of current schema.t.cpp plus five
  independent cases, using the pinned compiler/module map and production runtime,
  capture and loader libraries. No test-source injection or candidate mutation.
  Every compile/link/test subprocess had a finite timeout; commands.json records
  exact commands, cwd, bounds and statuses.
- review '[independent]': 5 cases / 336 assertions passed. Adds exact JSON
  1,048,576-byte and 100,000-node acceptance/refusal boundaries; decoded duplicate
  false/Unicode keys, integer overflow and surrogate/syntax vectors; ID 64/65,
  segment 128/129 and portable path edge cases; duplicate sets, malformed nested
  collections; budget inheritance/overrides, optional absence and uint bounds;
  nested registry unknown fields and deterministic unlisted-job diagnostic.
- review '[r07b1c01]': 5 cases / 345 assertions passed through production exports,
  independently running all correction regressions (depth patterns, final text,
  every collection permutation/order exception, joined boundaries and indexes).
- python3 scripts/rewrite-check.py lint --build-dir build/rewrite-debug:
  exit 0, pinned formatting/Doxygen checked 22 active source files.
- Final cmake build above: exit 0, ninja: no work to do.

Initial scratch run was 5 cases / 336 assertions with two failures in one case:
I incorrectly expected an unlisted job's intrinsic budget diagnostic to precede
its route diagnostic. Documented file/field/code byte ordering selects field
'job' before 'job.budget_bytes', correctly producing reference/job. I corrected
only that scratch expectation; initial-review.cpp/log retain the original run.
No product failure was found or changed. All final raw logs are adjacent.

Coder red evidence was also inspected: retained beforefix Lua SHA256s exactly
match original B1 production sources, and exact-red-tests.log reports 5 cases,
4 failures / 50 failed assertions, without unexpected exceptions; final restored
schema17/1206 and bridge18/432 evidence is consistent with independent CTest.

## Limits

Approve full effective B1 structural schema/normalization, not B2 cross-entity
reference/shared namespace/assignment/review/check/ownership/alias/topology/phase
graph/W/selection/identity semantics or R08 observations/target applicability,
projection and CLI integration. No check execution/completion/approval is implied.
Pinned macOS debug verification only; no Linux/release run or full integrated
R09 gate. Completed review; no further candidate activity planned.
