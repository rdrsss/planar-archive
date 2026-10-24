# N02 independent review

Disposition: **approve**. No findings requiring correction.

Reviewed the shared `dev/planar-v2-vernacular` checkout independently of the
coder, with product files and prior evidence read-only. Reviewer writes are
limited to this report and `tests/native/n02-review-evidence/`. No Planar tools,
installed binaries, product changes, commits, installation or publication were
used. The JSON-library follow-up is outside this review's source-layout scope.

## Exact candidate

Starting revision: `3171c49e7b92d3abda01bfbb8bfb0c6b29d4c67a` with the pre-existing
shared changes captured in the coder evidence.

- Candidate: `tests/native/n02-evidence/candidate-sha256.json`, SHA256
  `9d7199a73694e12651182af7057dec919a0aa3b31f2eaafedfdb6dab0d1e18db`.
- Prior evidence: `tests/native/n02-evidence/evidence-sha256.json`, SHA256
  `13df22c0b91c6e4e83ce4be1d73db184921ac6a0d1de98022863ae9fb5b97ad7`.

Both manifest identities and every manifest entry were checked at review start
and again after independent verification: 16 candidate files and 24 evidence
files. This disposition applies only to that exact candidate. The coordinator
must check it remains current before advancing.

## Scope and correctness

The eight Lua product sources plus their bridge README have identical SHA256
values at `src/stdlib/planar/` to the recorded pre-move values. Top-level
`stdlib/` is absent. The seven assigned build/reference/instruction edits exactly
reproduce the preserved `scoped.diff`. CMake differs only in adding `src/` to the
eight product embed paths; source ordering is unchanged.

For both debug and release, the configured Ninja graph contains all eight new
source regeneration dependencies and no old top-level embed dependency. The
generated embedded module contains exactly the ordered source bytes with the
existing separator LF after each file. Its payload SHA256 is
`6d29c15216cbff11a6f7c237f7ee118bff5d051975c21a9da578c531569f78f5`.
The unchanged `cmake/embed_lua.cmake` adds each input to
`CMAKE_CONFIGURE_DEPENDS`, so edits continue to trigger regeneration.

An independent scan of 65 active source/build/reference files found no old
`stdlib/planar/` path references; 45 relative documentation links resolve. The
remaining first-party C++/Lua outside `src/` consists of test-only bridge/harness
files, toolchain probes, and the quota fixture's external-workspace declarations.
Fixture workspaces/source snapshots/observations/packets are verification inputs,
consistent with the accepted boundary. The active architecture, contributor and
agent instructions express the rule for future product workflows.

All 899 protected fixture, test, vector and historical/evidence files retain
their recorded hashes. `AGENTS.md` remains a symlink to `CLAUDE.md`. Historical
records and frozen manifests correctly retain their earlier source paths.

## Verification and evidence

Reviewed both raw coder gate logs and detailed CTest logs. Each complete gate
records 113 passing registered tests (112 native and one acceptance), zero skips,
150 CLI processes, six computed native goldens, formatting/Doxygen over 25 active
C++ sources, and a no-work second build. Release preserves the first Doxygen
signal-only exit 138 and the existing approved wrapper's successful second
attempt; the retry is not represented as a clean first attempt.

Reviewer commands executed successfully, serialized:

```sh
python3 tests/native/n02-review-evidence/audit.py
cmake --build build/rewrite-debug --target all planar_tests
cmake --build build/rewrite-release --target all planar_tests
ctest --test-dir build/rewrite-debug -R '^replacement.cli$' -V --output-on-failure
python3 tests/native/n02-review-evidence/audit.py
```

Both independent builds returned `ninja: no work to do.` The focused independent
CTest run matched one acceptance test, passed 150 real CLI processes and six
native goldens, and reported zero skips. The reviewer also independently audited
local links and source paths with Python and `rg`. An initial reviewer audit
used the wrong textual spelling for the acceptance-summary match; it was
corrected to the actual raw log text before the successful checks. This was an
audit predicate issue, not a product/test failure.

Reviewer evidence:

- `tests/native/n02-review-evidence/audit.py`, `audit.json`, `final-audit.log`:
  identity, move/preservation, exact diff, build dependency and embedded-byte checks.
- `tests/native/n02-review-evidence/path-audit.json`: active references, local links
  and external C++/Lua inventory.
- `tests/native/n02-review-evidence/smoke-{1,2,3}.log` and `smoke-summary.json`:
  raw independent build/acceptance output and command results.

The complete coder gates were not repeated: exact byte preservation, raw full-gate
inspection, independent dependency/embedding checks and the independent CLI
acceptance run cover this relocation. Evidence is from macOS Apple Silicon;
Linux verification is not claimed. R10 operator context evaluation remains
pending and is unaffected by this approval.
