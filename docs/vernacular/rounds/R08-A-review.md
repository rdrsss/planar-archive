# R08-A independent review

Disposition: **request-changes**. Two P2 contract defects reproduced against the
frozen production `render` export. No candidate product/test/evidence file was
changed. Reviewer writes are limited to this report and
`tests/native/r08a-review-evidence/`.

Candidate: shared `dev/planar-v2-vernacular`, HEAD
`3171c49e7b92d3abda01bfbb8bfb0c6b29d4c67a`;
`tests/native/r08a-evidence/candidate.sha256` digest
`73e5adba99d6097e6e7fa7c48a74774bc7201da8dfdb028e4d4cc9fdee2e9d01`.
All 29 entries verified before and after review. Final raw check:
`tests/native/r08a-review-evidence/final-manifest.log`.

## Findings

### F1 — P2: checkpoint file lists accept portable aliases

Location: `stdlib/planar/observations.lua:59–62` (schema at line 5).

The checkpoint checks exact uniqueness, individual path grammar, and containment,
but never checks portable aliases. A context-only `split/cli_worker` render with
`checkpoint.files:["src/cli/Foo.py","src/cli/foo.py"]` succeeds. These are two
case-fold-equivalent spellings, which `declarations.md:75–77` explicitly rejects
on every host. Nonexistent checkpoint files are allowed, so native capture cannot
supply the missing rejection. Segment aliases such as `Dir/a` and `dir/b` need
the same policy. This leaves the advertised portable path contract inconsistent
for checkpoint notes and can describe ambiguous files on supported hosts.

Require portable alias validation for checkpoint paths, including their relevant
source/declaration path spellings, before returning the role-filtered result.
Keep ordinary existence-independent checkpoint paths valid. Add a regression
for equivalent filenames and differing spellings of a shared directory segment.

Reproduction: `independent checkpoint portable aliases are refused` in
`tests/native/r08a-review-evidence/probes.cpp`. The production export returns
success where the probe expects `ownership`.

### F2 — P2: sorted observations report the wrong original array index

Location: `stdlib/planar/observations.lua:35–48,53,59–62`.

Normalization sorts check/finding records by ID, vectors by work, and checkpoint
paths by byte order. Semantic validation then interpolates the normalized loop
index into the diagnostic field. With original findings `[id:z valid, id:a stale]`,
the stale second finding reports `observations.findings[1].target`, naming the
valid original first record. Check applicability/targets and vector member target
errors have the same issue; checkpoint scope errors can also shift after sorting.
The required error field is a schema path into the supplied input
(`render-contract.md:563–565`), and the existing schema/semantic APIs preserve
original declaration indexes. A driver/operator following this diagnostic can
edit the wrong record while the invalid record remains.

Use the schema context's retained record origins (`ctx.index`) for semantic
record/vector diagnostics, and preserve original path indexes when checking
sorted primitive path sets. Keep deterministic normalized output order.

Reproduction: `independent observation diagnostic indexes address original JSON`
in the same probe source. Expected `[2]`, actual `[1]`, recorded verbatim in
`tests/native/r08a-review-evidence/edge-probes.log`.

## Verification and review scope

Executed serially against the existing debug candidate:

- `cmake --build --preset debug --target planar_runtime_tests`: success,
  `ninja: no work to do`.
- `ctest --test-dir build/rewrite-debug -R 'replacement.(render|runtime|schema|semantic|selection)' --output-on-failure --output-junit /tmp/r08a-review-ctest.xml`:
  73 matched tests, zero failures and zero skips. Copied raw output and XML to
  `tests/native/r08a-review-evidence/ctest.log` and `ctest.xml`.
- Independent scratch native production-export probes: four passing cases,
  237 assertions, zero skips. Mandatory-only exact/one-byte-under budgets;
  unselected target owning-worker check additions; reversed vector normalization;
  upstream facet deletion/full old data and exclusion of unsubscribed private
  content; mixed CR/backticks/C0/DEL/C1 display framing. Raw `probes.log` and
  initial JUnit `probes.xml` are in reviewer evidence.
- Two additional edge probes: both fail their contract expectations, 37
  assertions/2 failed, exit 42. Raw `edge-probes.log`.
- `python3 tests/native/r08a-review-evidence/run.py` compiles a scratch translation
  unit by appending reviewer probes to the frozen test source, links against the
  exact production runtime libraries, and reproduces pass exit 0 / edge exit 42.
  It never replaces a build object or executable. `compile.log` and `link.log`
  contain no errors. The runner requires the existing configured debug graph.
- `git diff --check`: success.

Reviewed coder raw evidence: final XML contains exactly 12 focused testcase
records, 26,867 assertions, zero failures/errors/skips; affected CTest XML has
73 tests and zero skips. The formatter/Doxygen log reports 22 active sources;
no-change build records zero work. Both pre-freeze red/green regression pairs
show a concrete failure before and success after the fix. These are credible
candidate checks, but do not cover F1/F2.

Read the rewrite authorities, R08/R08-A scope, full declaration/render contracts,
the new observation/presentation/render helpers, bootstrap and CMake wiring,
production tests, unchanged selection/schema/semantic boundary as relevant, and
R03 expected-packet comparison. No additional blocking finding was identified
in global assertion applicability, complete selected/upstream data, dynamic
fences and control escaping, truthful capture/history/check labels, per-work
checks, grouped versus split obligations, exact structural P/M fields, native
fragment/work identities, or whole-section omission/budget accounting.
The documented R03 wording changes retain the applicable fixture data and
requirements; original expected files were not regenerated.

This is a component review. Native CLI transport, final canonical JSON/output
limits, end-to-end computed goldens and the full replacement gate remain the
explicit R08-B/R09 obligations. Linux/release were not exercised in this review.
A fresh corrective round should retain this candidate and the failing evidence.
