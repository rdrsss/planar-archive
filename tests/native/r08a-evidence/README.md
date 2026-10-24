# R08-A trusted renderer candidate evidence

This is implementation evidence for independent review, not approval. The shared
checkout's prior rewrite edits were preserved. This slice changes only:

- `stdlib/planar/presentation.lua`, `observations.lua`, `render.lua` (new trusted helpers).
- `stdlib/planar/bootstrap.lua` (fixed `render` export), `README.md` (request/result API).
- `src/kernel/CMakeLists.txt` (explicit helper embedding).
- `src/kernel/runtime/schema.t.cpp` (production-export verification using existing fixture helpers).
- This evidence directory.

No approved parser/schema/semantic/selection/native bridge/CLI/contract/original
fixture file was changed. No Planar command, queue, claims, legacy binary,
installation, commit or external state was used. The implementation consumes
approved `select_role` and calls the original strict parser on retained
`observations:input.json` bytes only.

## Verification commands and counts

Commands were serialized for the debug build directory:

```sh
cmake --build --preset debug --target planar_runtime_tests
build/rewrite-debug/src/kernel/planar_runtime_tests '[r08a]' --reporter junit --out tests/native/r08a-evidence/render-tests.xml
ctest --test-dir build/rewrite-debug -R 'replacement.(render|runtime|schema|semantic|selection)' --output-on-failure --output-junit <repo>/tests/native/r08a-evidence/ctest.xml
python3 scripts/rewrite-check.py lint --build-dir build/rewrite-debug
cmake --build --preset debug --target planar_runtime_tests
git diff --check
```

Final focused run: 12 native production-export cases, 26,867 assertions, zero
failures and zero skips. Catch2's JUnit `tests` attribute counts assertions;
the report contains 12 testcase elements. Final affected CTest gate: 73 matched
cases, zero failures/skips (including existing native runtime/schema/semantic/
selection cases and the scaffold CLI refusal case). Formatting and Doxygen
checked 22 active C++ sources. `no-change-build.log` records zero build work.
`build.log`, `render-tests.xml`, `ctest.log`, `ctest.xml`, and `lint.log` are final
candidate evidence. Earlier `initial-tests.log` and `runtime.log` are historical
partial checks, not the final acceptance count.

Coverage includes all six quota role/recovery scenarios; no observations file;
context-only versus captured-empty outputs; global stale/malformed/unknown/
unselected targets; required check applicability and exact vectors; no assertions
for unavailable outputs; strict schemas, duplicate keys/IDs and vector duplicates;
checkpoint role scope/32 paths/2048-byte strings/exact 8192-byte canonical bound;
dynamic backticks, ASCII/Unicode C1 controls, UTF-8, binary hex, deleted content,
unchanged content once and complete upstream old/new contents; explicit/internal/
cross-role/pipeline prerequisite obligations; exact budget boundaries, whole
optional removal order and notice, role/job/override precedence and byte-count
sums; native fragment/work hash recomputation; repeated fresh calls, retained
input immutability and rendering after workspace removal.

Two bugs found before candidate review have explicit native red/green evidence:

- `before-reviewer-grouping.lua` and `red-reviewer-grouping.log` reproduce a
  split reviewer incorrectly labeling dependencies internal because both units
  were targeted. The final projection compares owning-worker assignments;
  `green-reviewer-grouping.log` passes the same test (combined reviewer remains
  internal, split reviewer is cross-role).
- `before-canonical-controls.lua` and `red-canonical-controls.log` reproduce
  the JSON helper using short newline escapes and therefore accepting a
  checkpoint whose contract-canonical JSON exceeds 8192 bytes. The final helper
  uses lowercase `\u00xx` for every U+0000–U+001F character;
  `green-canonical-controls.log` passes the same 2048-newline regression.

## Comparison with the original R03 expected packets

The six production cases reference the original unchanged
`fixtures/render/quota/expected/*.md` files. They extract every original fenced
prose/source block and require its exact contents to occur in the computed
packet, excluding structured JSON blocks whose whitespace/spelling changes.
Each original goal, unit summary, criterion, requirement, contract expectation,
review assertion, selected old/new source, rationale, workspace note and recovery
prose is therefore checked against the original examples. IDs, ordering, complete
check/toolchain definitions, applicability and native identities are checked as
structural values or projection assertions rather than fake literal hashes.
Reviewer observations resolve the original symbolic target placeholders using
computed native unit-output identities before invoking `render`.

Intentional phrasing/presentation differences from R03:

- Descriptor arrays/objects use compact canonical JSON with canonical object-key
  order instead of the examples' pretty JSON. Check definitions/toolchains retain
  their exact normalized values. Labels explicitly state canonical JSON string
  escaping; assertion authority fields are present in the actual manifest/data.
- Dependencies and facets include their complete normalized policy records,
  exact captured availability and the combined prerequisites array. Concise
  projection-owned sentences name both worker assignments and internal versus
  cross-role obligations. Pipeline obligations have their own complete record
  and explicit complete-before-start sentence. The samples' quota-specific
  explanatory narration is replaced by these general, deterministic statements.
- File framing uses concise added/deleted/modified/unchanged lines followed by
  old/new/current labels containing logical root/path, exact mode, size and native
  content digest. Complete content remains unchanged. Unit/role outputs and
  reviewer target vectors use computed identities rather than `$...` symbols.
- Boundary text lists exact work/frozen/excluded grants and read-only remainder
  without the examples' fixture-specific commentary about absent directories.
- Optional section labels use the exact section IDs (`workspace_notes`, etc.)
  rather than replacing underscores with spaces. Recovery records current
  checkout identity and the exact fixed history/no-active-run statement.
- The fixed initial authority statement and section order are retained. Empty
  capture/context-only/empty assertion statements are explicit; no completion,
  check receipt or authenticated review is inferred.

These are computed trusted structural packets, not native CLI JSON goldens.
R08-B owns final native canonical JSON, envelope limits, CLI capture composition,
stdout/exit handling and complete end-to-end golden output. Linux was not run;
this evidence was produced with the pinned macOS LLVM/libc++ debug toolchain.

## Candidate verification

`candidate.sha256` freezes every changed product/test file and all evidence
files (except itself). From the repository root run:

```sh
shasum -a 256 -c tests/native/r08a-evidence/candidate.sha256
```

The coordinator/reviewer must verify this snapshot is still current before
acceptance. The candidate contains no commit and makes no self-approval claim.
