# R10-P01 presentation candidate evidence

This is implementation evidence for fresh independent review, not approval.
Shared dev/planar-v2-vernacular checkout; no Planar operational tools, commits,
installation, publishing, source checks or persistence operations were invoked.
All CLI examples are render-only previews: no active run, authenticated approval,
check execution/receipt or verified dependency quality is established.

## Actual current examples

- [Split library worker](debug/split-library_worker.md): owned library source,
  complete contract/check policy and one toolchain description.
- [Split command worker](debug/split-cli_worker.md): readable contract/facet
  dependencies, complete contract and integration check fields, owned source and
  full subscribed upstream old/new source.
- [Combined builder](debug/combined-builder.md): both work boundaries and retained
  internal obligations.
- [Split reviewer](debug/split-reviewer.md) and
  [combined reviewer](debug/combined-reviewer.md): full ordered exact targets,
  complete source/requirements and unverified exact-vector assertions.
- [Recovery](debug/split-cli_worker-recovery.md): bounded unverified session note,
  capture status, history unavailable and no active run.

Every Markdown file has its complete actual CLI JSON envelope beside it. Release
examples are under `release/`. Historical R10 guide/evidence is unchanged.

## Preservation and readability

`before/` contains exact copies of the six prechange native envelopes/Markdown
and reviewer input, captured before editing presentation. `compare_projection.py`
checks all six against the current real-CLI-generated goldens and writes
[golden-comparison.json](golden-comparison.json), including complete per-packet
check/toolchain, effective-check, requirement, acceptance, dependency, facet,
target and section counts. The exact original R03 prose/source comparison still
runs in CLI acceptance; only obsolete aggregate JSON formatting assertions were
replaced by complete labeled-field/array/coverage/description assertions.

| Packet | Before Markdown bytes | Current Markdown bytes |
| --- | ---: | ---: |
| combined-builder | 13405 | 12496 |
| combined-reviewer | 14415 | 13311 |
| split-library_worker | 7578 | 7542 |
| split-cli_worker | 11350 | 10543 |
| split-reviewer | 14436 | 13332 |
| split-cli_worker-recovery | 11961 | 11121 |

Every manifest field except `versions`, `render`, and `sections` is exactly
unchanged in all six pairs. Version changes only M's appended projection tag to
`projection_v2`. W (including W.versions), base, observation, outputs, role output,
files/provenance, target vectors, assertions and prerequisites are identical.
Packet metadata is identical; Markdown naturally differs. No mandatory content
is truncated; optional omission/budget policy is unchanged and exercised at exact
boundaries. Coverage prose, full descriptions, argument boundaries and environment
values are retained. Paths/modes/byte counts/full old/new contents remain visible;
file/facet and global capture hashes remain in the manifest. Authority retains the
full work fingerprint and registry, worker output IDs remain visible, reviewers
show their complete ordered target vector once in outputs. Unverified assertion
vectors may repeat their applicable target values as evidence, without a duplicate
unit-output listing.

`regression_probe.py before` fails against the retained actual prechange output;
its current run passes. [regression-probe.log](regression-probe.log) records both
statuses. This is an output-level regression demonstration using frozen CLI
packets, not a claim of rebuilding the prior source candidate. Native tests
exercise fresh renders with newline/backtick/quote/control bytes in exact argv
and environment JSON, dynamic fences, all dependency policies, pipeline/questions,
absence/availability, unchanged manifests, full target ordering, and unrelated
unit edit stability. Existing cases retain no-capture/empty/deleted/binary and
exact budget/omission tests.

## Executed verification

All commands run from repository root, serially per build directory:

```sh
make build
cmake --build build/rewrite-debug --target planar_tests
build/rewrite-debug/src/engine/planar_runtime_tests '[render]'
python3 tests/acceptance/bootstrap_cli.py build/rewrite-debug/bin/planar --write-goldens
python3 tests/native/r10p01-evidence/compare_projection.py
make test-all
make test-all PRESET=release
python3 tests/acceptance/render_evaluation.py build/rewrite-debug/bin/planar tests/native/r10p01-evidence/debug
python3 tests/acceptance/render_evaluation.py build/rewrite-release/bin/planar tests/native/r10p01-evidence/release
```

Focused projection: 18 test cases, 28,070 assertions, zero skips. Real CLI
acceptance: 247 processes, six exact goldens, including 60 host-path checks.
Each complete gate: 124 tests (123 native + one acceptance), zero skips;
formatting/Doxygen cover 29 active files; no-change build reports no work.
Each evaluation: 19 CLI invocations, six exact goldens, 11 identity comparisons,
independent hashes verified, inputs/HOME unchanged, zero skips. `own-library`
now expects render stability while global observation/command output change;
all other scoped materiality comparisons retain their prior expectations.
`cross-build.json` records debug/release non-provenance byte equality.

## Candidate identity and protected scope

`candidate.sha256` freezes the 21 assigned product/test/reference/golden files.
`evidence.sha256` freezes this directory except itself. `protected-source.sha256`
freezes 3,032 nonowned source/vendor/build/fixture/historical-evidence and planning
files; coordinator-owned R10/roadmap/implementation/current-round records are
excluded. `protected-audit.json` compares 2,313 paths to N04's before-files plus
approved candidate hashes: 2,312 match; `docs/vernacular/rounds/N04.md` differs
because its coordinator finalization postdates that candidate. This is a known
pre-existing record difference; this coder did not edit it. Another 719 later
evidence files have only the current freeze as a baseline. No separate P01
protected prehash existed, so this audit does not claim one. Original R03 inputs,
expectations and historical R10 guide are included and unchanged against the
available baseline. Verify manifests before review/advancement.

Limits: macOS Apple Silicon debug/release evidence; Linux was not executed.
Finite subprocess checks and persistence remain outside this render-only change.
