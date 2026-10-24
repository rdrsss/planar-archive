# R07-B1-C01 coder evidence

Candidate awaiting independent review; this is not approval. All edits are
within stdlib/planar/{schema.lua,json.lua,README.md},
src/kernel/runtime/schema.t.cpp and this evidence directory. No bridge, native
capture, contract, fixture, build, coordinator record or unrelated files edited.
No Planar tool, queue, installed binary, database, commit or installation used.

## Changes

- Final inline/file text uses kind:text and exact bytes; captured file provenance
  is exactly file:{root:workspace,path}. Route-only uncaptured file descriptors
  remain provisional kind:file/path. Wrapped notes normalize identically.
- An explicit sort_records property is separate from collection uniqueness.
  ID-keyed records sort by byte ID; acceptance criteria, outer stages,
  command/argv and environment lists retain their order. Sets still sort.
  Workspace/job/toolchain intrinsic diagnostics use original declaration indexes;
  duplicate workspace files report an original later occurrence even when ID
  sorting reverses encounter order.
- JSON depth is checked on container entry, so exactly 128 containers work and
  129 fail for empty or scalar terminals, in arrays, objects and mixed nesting.
- Final workspace route joins are validated before matching/lookups or dependent
  missing/unlisted-route diagnostics. Invalid joins report path_denied on the
  valid declaring workspace path and original field. All supplied jobs still
  undergo schema/intrinsic validation before selection/matching.

Existing tests expecting kind:file or direct path were corrected to the approved
kind:text/file provenance shape. No refusal expectation was loosened. Five new
cases exercise the four review findings plus original-index diagnostics. Every
listed sorted spec collection is tested in both ID permutations, as are registry
records. Reversed workspace jobs and supplied jobs still return ID order; job
roles sort; acceptance uniqueness is retained independently of its order.
Joined spec/job/text paths test exactly 1024 bytes (successful route and final
normalization), 1025 and 1031 bytes (path_denied); retained file bytes and wrapped
provenance are asserted. Pipeline stages are constructed explicitly because the
quota split fixture uses fanout_review.

## Reproducible before-fix evidence

`beforefix-schema.lua` and `beforefix-json.lua` are exact original B1 production
sources reconstructed by reversing these scoped fixes. Their SHA256 identities
were checked against tests/native/r07b1-evidence/candidate.sha256 before use:

- schema: b593d55009763070674d5e8482532ed924b92cf9858015b148348fd2e7141626
- json: 425a5fe6559629afd38c84f06100b6ea1d62b6bb8c6f1e220f48454a85c4011a

The final test source is retained as regression-schema.t.cpp (identical to the
candidate schema.t.cpp). Exact baseline Lua was briefly substituted in the
owned source paths, built via the normal product embedding, and tested with the
final regression source. A Python try/finally restored candidate Lua and rebuilt
before any final checks. `red-commands.json` records exact commands/statuses.

- `exact-red-build.log`: normal all/planar_runtime_tests build exits 0.
- `exact-red-tests.log`: executable '[r07b1c01]' exits 42; 5 cases,
  1 passes/4 fail; 293 assertions, 243 pass/50 fail. All four findings reproduce;
  no unexpected exceptions. The diagnostic-index case already passes baseline.
- `restored-build.log`: candidate restored and rebuilt, exit 0.

Earlier exploratory red/green logs are retained without alteration. Initial
red-tests.log also fails four cases, but contains an incorrect assumption that
split already has pipeline stages and an incorrect wrapper-byte assertion.
These test issues were corrected before exact-red-tests.log, which supersedes
that preliminary evidence. green-schema.log and green-schema-2.log record the
intermediate failures and fixture adjustments. Initial invocation of nonexistent
planar_schema_tests failed (unknown target); actual target is planar_runtime_tests.
No zero-match execution is treated as success.

## Final verification

Commands were serialized against build/rewrite-debug on macOS with the pinned
toolchain. Direct executable invocations use production embedded Lua exports;
there is no test-source injection or native schema implementation.

- `cmake --build build/rewrite-debug --target all planar_runtime_tests`:
  final-build.log and restored-build.log; exit 0.
- `build/rewrite-debug/src/kernel/planar_runtime_tests '[schema]'`:
  final-schema.log; 17 cases / 1,206 assertions pass.
- `build/rewrite-debug/src/kernel/planar_runtime_tests '[runtime]'`:
  final-bridge.log; 18 cases / 432 assertions pass.
- `ctest --test-dir build/rewrite-debug -R '^replacement\.(schema|runtime)' --output-on-failure --timeout 30`:
  final-ctest.log, after exact-red restoration; 35/35 pass, zero skips, 7.69s.
- `python3 scripts/rewrite-check.py lint --build-dir build/rewrite-debug`:
  final-lint.log; pinned formatting and Doxygen pass on 22 active sources.
- `cmake --build build/rewrite-debug --target all planar_runtime_tests`:
  final-nochange.log; ninja: no work to do.

candidate.sha256 freezes exactly the four owned source/doc files. evidence.sha256
freezes every evidence file except itself, including this report and candidate
manifest. No release/Linux verification or R09 full integrated gate was run.
B2 reference/ownership/assignment/phase/selection/identity semantics and R08
observations/projections/CLI remain outside this corrective slice.
