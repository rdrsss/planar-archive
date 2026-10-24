# R08-A-C01 independent review

Disposition: **approve**. The correction resolves both P2 findings from the
[R08-A review](R08-A-review.md). No additional blocking finding was identified
in the corrective scope. Together with that review's unchanged-scope assessment,
this approves the corrected R08-A component candidate. R08-B transport and the
integrated R09 acceptance gate remain outstanding.

Reviewed in the shared `dev/planar-v2-vernacular` checkout, HEAD
`3171c49e7b92d3abda01bfbb8bfb0c6b29d4c67a`. No product, test or coder evidence
file was changed. Reviewer writes are limited to this report and
`tests/native/r08ac01-review-evidence/`. No delegation, Planar tools, legacy
binary, installation, commit or publication was used.

## Candidate identity and scope

`tests/native/r08ac01-evidence/candidate.sha256` has SHA256
`5e2d0a4b3aca349e3fbd4e9e9a151e9c01c193144c38c7e3bd99db00679696d5`.
All 20 entries match before and after review. All 26 entries of the original
R08-A manifest other than the three explicitly superseded files also match.
The original complete `schema.t.cpp` is byte-identical to the corrected file's
first 2,079 lines, so the corrective tests are additions to the prior tests.
Raw checks: `initial-manifest.log` and `final-manifest.log` under reviewer
evidence. The prior manifest and original review evidence remain intact.

Read the rewrite authorities and relevant declaration/render contracts, the
prior findings, corrective brief, full observation validator, schema origin
tracking, semantic path alias policy, selection validation, renderer entry and
new tests/reference text. This is a corrective component review, relying on the
prior broad review for unchanged R08-A production files rather than claiming a
new full milestone review.

## Findings resolved

- **F1 resolved:** `observations.lua:63` introduces a local shared-prefix
  case-fold registry using the same segment policy as semantic validation.
  Lines 82–88 seed all work/frozen/excluded/facet declarations and both captured
  file/directory trees. Lines 89–95 validate normalized checkpoint paths against
  that namespace while reporting their original JSON indexes. This introduces
  no existence requirement or new filesystem access. Independent probes reject
  the original filename aliases, reversed shared-directory spellings, a base-only
  file, a base-only empty directory and an absent declared facet alias. Nearby
  valid sibling paths remain accepted in captured and context-only mode.
- **F2 resolved:** `observations.lua:36,48,53` use the schema context's retained
  table origins for check/finding records and nested vector members. Lines
  59–62 preserve raw indexes for unique primitive checkpoint paths. Original
  failures now pass; independent malformed nested digest and wrong-vector
  probes report exactly `observations.checks[2].vector[2].output` and
  `observations.checks[2].vector`. Reversed alias input reports exactly
  `observations.checkpoint.files[1]`. Valid reordered checkpoint lists produce
  equal packets and unchanged work identities.

The alias registry is local to the invocation; traversal uses bounded path-byte
loops and validated inventories without introducing native calls or mutable
shared state. Normalized ordering and native identity inputs remain unchanged.
No hidden existence requirement, root confusion, alias-policy weakening or
origin-tracking regression was found in the exercised cases.

## Independent verification

Executed directly, serially per build directory, with zero unexplained skips:

- `cmake --build --preset debug --target planar_runtime_tests`: success,
  `ninja: no work to do`; repeated after checks with the same result, preserved
  in `no-change-build.log`.
- `python3 tests/native/r08ac01-review-evidence/run.py`: compiles a scratch
  translation unit containing the exact candidate tests plus original and new
  independent probes, linking against the production embedded runtime. It
  neither replaces a build object nor runs the original evidence writer.
  Original broader probes: 4 cases / 237 assertions; original failing edge
  probes: 2 cases / 41 assertions; new corrective probes: 5 cases / 329
  assertions. All pass, zero skips. The new suite has 6 JUnit testcase records
  because sections expand records. Raw sources, runner, compile/link logs,
  console logs and XML are retained in reviewer evidence.
- `ctest --test-dir build/rewrite-debug -R 'replacement.(render|runtime|schema|semantic|selection)' --output-on-failure --output-junit <repo>/tests/native/r08ac01-review-evidence/ctest.xml`:
  76 matched tests, zero failures and zero skips (`ctest.log`, `ctest.xml`).
- `python3 scripts/rewrite-check.py lint --build-dir build/rewrite-debug`:
  formatter and Doxygen pass for 22 active sources (`lint.log`).
- `git diff --check`: success (`diff-check.log`).

Inspected coder raw red/green evidence: red is 3 cases / 279 assertions,
11 failed assertions in 2 cases; final corrective green is 3 cases / 309
assertions. Final render run is 15 cases / 27,176 assertions. Corrective and
render JUnit record counts are 10 and 22 because sections expand records;
the suite `tests` field counts assertions. Affected coder CTest XML has 76
tests. Green runs report zero errors/failures/skips. Those records agree with
the evidence summary; the preserved red failures demonstrate both defects
before the validator correction.

Limitations: debug macOS native production-export coverage. This review does
not establish Linux/release behavior, native CLI transport/output limits,
end-to-end computed goldens or full integrated R09 acceptance.
