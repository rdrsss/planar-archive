# R08-B-C01 independent corrective review

Disposition: **approve**. The original R08-B P2 host-path routing/refusal finding
is corrected. This approves the R08-B candidate with the three-file C01
correction, carrying forward the completed broad R08-B review and its stated
limits. No new blocking findings.

Reviewed the shared `dev/planar-v2-vernacular` checkout at HEAD
`3171c49e7b92d3abda01bfbb8bfb0c6b29d4c67a`. Product files, tests, goldens and
previous evidence stayed read-only; writes were confined to this report and
`tests/native/r08bc01-review-evidence/`. No Planar tools, commits, installation,
publication, delegation or unrelated reverts.

## Exact reviewed candidate and preserved evidence

`tests/native/r08bc01-evidence/candidate.sha256` has SHA256
`6a69d02528e5d05dcfa22c0b8c73e5a9347ad5e9609275f1eded445c098b4109`.
All three entries matched before and after review: `bootstrap.cpp`,
`tests/acceptance/bootstrap_cli.py` and new `bootstrap_paths.py`.
The coder evidence manifest has SHA256
`c7d6f295ea3d3c0821de86c11922e9ae0d80de7223014e8a3f52678587173c83`;
all 19 entries also matched before and after. See
`initial-identities.json` and `final-identities.json` in the review evidence.

All **23 unchanged R08-B candidate entries** match the frozen 25-file R08-B
manifest; its only exclusions are the two intentionally modified existing
files. All **20 R08-A-C01 approved entries** match. The original R08-B evidence
manifest also verifies (`prior-r08b-evidence-check.log`). No approved capture,
Lua, serializer or golden content changed. Reviewed the raw C01 patches,
red/green logs and original R08-B review. The red manifest identifies the
original R08-B bootstrap source, and its path-test helper is byte-identical to
the current helper. Red evidence demonstrates 39 failed assertions among 60
path checks; green evidence passes all 60 with 150 real CLI invocations.

## Path correction assessment

`src/cmd/planar/bootstrap.cpp:72`, `host_file()`, validates the raw final file
entry, resolves the original parent through the filesystem, and retains the
leaf for capture's no-follow open. It no longer removes `..` before expanding
an earlier symlink. Invalid directory/file spellings cannot be cleaned into
regular-file inputs; final file links remain refused.

`bootstrap.cpp:85`, `host_directory()`, obtains the actual canonical referent
and walks original path components plus expanded relative/absolute link
targets. Dot/parent interpretation remains filesystem-based. Comparing each
link target to the final directory implements the round's recorded rule:
equivalent final-root aliases are refused, including child/parent spellings;
intermediate links whose targets are different final directories are allowed.
The walk is bounded by an explicit 64-link cap; native canonical resolution
also rejects loops. Error transport continues to omit host paths and produce
typed failures with empty stdout.

This was checked against the implementation/roadmap, R08-B/C01 briefs, original
review, declaration/render host-path and capture rules, and the v4 render
boundary. Canonicalization occurs before capture as required. It does not add
a new filesystem-snapshot guarantee: the driver must hold host inputs stable
under the existing cooperative capture contract. Capture still opens canonical
components without following links and verifies its retained descriptors/data.
No lower-level capture change is needed for this correction.

## Independent verification

Commands ran directly and serially per build directory against the explicit
replacement debug binary. No other builder ran during this review.

- `cmake --build --preset debug --target all planar_tests`: success,
  `ninja: no work to do.` (`build.log`).
- `python3 tests/native/r08bc01-review-evidence/probes.py`: **89 independent
  real CLI processes**, all pass, zero skips (`probes.log`, `probes.json`).
  Across workspace, registry, observations, current and base inputs, five
  independently constructed parent routes expand relative/absolute nested
  targets and `..` in both supplied paths and link targets. Distinct lexical
  and canonical bytes produce different oracle identities; routed full
  envelopes equal the canonical invocation. The source case additionally
  verifies exact retained marker bytes. Other probes cover nested final-root
  aliases, target dots/parents, allowed `child-link/..` intermediate aliases,
  a 12-link ancestor chain, loops/dangling parents, absolute/relative/chained
  final file links, dangling final file links, FIFOs and Unix sockets. Every
  response is checked for canonical JSON, expected status/code, empty opposite
  output stream and absence of host paths.
- `python3 tests/native/r08bc01-review-evidence/original-repro-green.py`:
  **three additional CLI processes** using the original R08-B reproducer with
  success assertions reversed in this owned copy. The original script/log are
  preserved. `hop/../current` and the canonical external path now both capture
  the exact external marker, digest
  `xxh3_128:ce1509621d599a20a467bceb6aaed9c1`; the ordinary fixture remains
  distinct (`original-repro-green.log`).
- `ctest --test-dir build/rewrite-debug -R 'replacement.(json|help|missing explicit|legacy commands|cli|render|runtime|schema|semantic|selection)' --output-on-failure --output-junit <repo>/tests/native/r08bc01-review-evidence/ctest.xml`:
  **83 tests**, 82 unit and one acceptance, **zero failures/errors/skips**.
  Acceptance runs the 150 CLI processes and six native golden comparisons
  (`ctest.log`, `ctest.xml`).
- `python3 scripts/rewrite-check.py lint --build-dir build/rewrite-debug`:
  formatting/Doxygen pass for **25 active sources** (`lint.log`).
- Final `cmake --build --preset debug --target all planar_tests`: success,
  `ninja: no work to do.` (`no-change-build.log`).
- Candidate/evidence and prior approved manifest checks all match as above.

The earlier suspected coder build/test overlap is disproved by the original
artifact timestamps: CTest XML mtime `1792492292`, earlier no-change build log
mtime `1792492304`. It is not a product or verification finding.

## Limits

Native debug macOS verification only. Release/Linux and the complete integrated
gate remain R09. The original review's 16 MiB envelope limitation is unchanged:
the exact serializer boundary has unit evidence; CLI large-source probes reach
the trusted instruction limit first. This corrective review makes no new
end-to-end envelope-limit or hostile concurrent filesystem guarantee.
