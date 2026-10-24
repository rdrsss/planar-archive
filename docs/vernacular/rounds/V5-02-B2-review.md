# V5-02-B2 independent review

Disposition: **approve**. No blocking findings or requested changes.

Fresh ordinary reviewer, shared checkout on `dev/planar-v2-vernacular`, HEAD
`3171c49e7b92d3abda01bfbb8bfb0c6b29d4c67a`. Review ownership was limited to this
report and `tests/native/v5-02-b2-review-evidence/`. No candidate edits, reversions,
historical evidence changes, Planar tools/skills/queue, installed binaries,
operator database, commits, installation, publication or merge were used.

## Exact candidate and preservation

Reviewed all 29 frozen candidate files against the baseline-relative diff and
current source. Independently verified every current candidate hash and all 122
frozen evidence artifacts, and regenerated `owned.diff` byte for byte:

| Input | SHA256 |
| --- | --- |
| `tests/native/v5-02-b2-evidence/candidate.json` | `35628357435d2708761f1a77bcc40e5a0614d7fb16092804d2ad00be24b4743f` |
| `tests/native/v5-02-b2-evidence/owned.diff` | `59d8e8f3210f5cac827b6efb71eb6abd76a38063700d21a25e6349d54757be65` |
| `tests/native/v5-02-b2-evidence/artifacts.json` | `ba9d890ad206af6656756af788c3bc9b6195b9ec8c8e84710709048744fd4ea5` |

All 5,816 protected inputs were checked: 5,815 are unchanged and only the
explicitly authorized `src/engine/runtime/runtime.cpp` amendment differs.
The coordinator-owned round record is correctly outside that protection manifest.
The exact command-contract body, user-owned v5 bytes, fixtures, goldens, prior
test/evidence inputs and `AGENTS.md -> CLAUDE.md` symlink remain preserved.
Current candidate snapshots and the frozen baseline inputs reproduce the diff.
Reference/status edits describe the implemented candidate and retain pending
workspace naming, load/full R14 and external-profile gates.

## Review conclusions

- `bootstrap.cpp:44` constructs invocation-owned CLI11 bindings and calls
  `root.parse` once. The sole `--...=` guard and CLI11 parsed-count/presence
  checks preserve separate-token syntax. Root help and usage return before
  registration. Family bindings stay alive through registration and execution.
  Decimal bindings retain canonical tokens while CLI11 fills signed native
  integers; presence is distinct from absence, and ID strings cross the Lua
  boundary without floating-point conversion.
- `bootstrap.cpp:12` compares the complete sealed catalog with compiled command
  and option names, requiredness, scalar/flag arity, numeric/text metadata,
  alias/positional constraints and duplicate policy. B1's approved seal enforces
  the exact family/handler/response/capability pairings. Invalid registration is
  redacted; bounded registration exhaustion remains typed. The compiled-drift
  real CLI variant independently exercises this comparison.
- All four family handlers now call `execute`, which invokes the retained VM
  (`invocation.cpp:268`). The actual shipped Lua callable owns routing,
  semantic policy and response projection. The altered embedded callable changes
  real CLI behavior after identical parser tokens. No fresh trusted semantic
  export remains in the command handlers or concrete adapter. The new module
  dependency graph remains below handlers/bootstrap and has no upward cycle.
- The finite adapter reuses existing capture, initialization, verified history
  and export primitives. Render capture retains the original reads, confinement,
  stability verification and immutable captured request. History preserves
  selector/default agreement, readonly native validation and verified facts;
  init/export cannot manufacture native facts. No Lua callback runs inside a
  native transaction or write lock. Native failures retain precedence over Lua
  suppression or manufactured success; host paths remain native slots and
  diagnostics are redacted. Whole-response encoding remains the Glaze codec.
- The authorized runtime amendment retains typed `root.id` and existing exact
  route/root/job/path, family, handle and ordering guards. Concrete capture checks
  mapping equality before spec/jobs/registry/source reads
  (`invocation.cpp:109`), restoring the historical `reference`/`root` exit-3
  refusal. The unchanged historical CLI regression and dedicated missing-input
  precedence test both pass. No broader runtime/API amendment was introduced.
- Export preallocates both uncertainty diagnostics before invocation
  (`invocation.cpp:260`). The adapter marks native publication immediately before
  constructing facts (`invocation.cpp:226`), and the response boundary combines
  that bit with VM publication state (`invocation.cpp:251`). Native facts
  allocation, Lua throw, invalid projection, caught exhaustion, encoding and
  post-invocation allocation failures preserve the prebuilt uncertainty result.
  Existing process I/O retains the separate stdout uncertainty diagnostic and
  short-write behavior. Cleanup exposes only validated staging tokens.
- The distinct service-facts allocation red/green proof is meaningful: its
  frozen generated sources differ only in the adapter publication-bit assignment.
  Red observes the wrong internal diagnostic after publication; green passes.
  The historical response encoding/allocation test retains its assertions after
  moving its exact generated fault seam to the shared invocation boundary.

## Independent verification

Normal checks ran serially within each build directory. No stress probes or
new behavior outside B2 were introduced. Raw reviewer JUnit, LastTest, inventories,
audit results and no-work output are retained under
`tests/native/v5-02-b2-review-evidence/`.

For each of `build/rewrite-debug` and `build/rewrite-release`, ran:

```sh
ctest --test-dir BUILD -R '^replacement\.(command |(retained_|init_|history_|export_)?cli$|render mapping refusal|export postpublication|export service facts)' --show-only=json-v1
ctest --test-dir BUILD -R '^replacement\.(command |(retained_|init_|history_|export_)?cli$|render mapping refusal|export postpublication|export service facts)' --output-on-failure --output-junit REVIEW_OUTPUT
python3 tests/native/v5-02-b2-support/audit-variants.py BUILD REVIEW_OUTPUT
cmake --build BUILD --target all planar_tests
```

Both focused runs matched and passed **29/29, zero skips**: 21 retained-command
runtime cases, all four historical real CLI suites, the new retained CLI suite,
the root-mapping regression and both export response-boundary cases. The new CLI
suite reports **99 real CLI processes plus one fixture helper** per build. Both
exact audits regenerated all seven variant families and the fixed-export runtime
implementation/interface, matched generated bytes and frozen coder audits, and
include the amended runtime source. Both final builds report
`ninja: no work to do.`

Also ran `python3 tests/vectors/generate.py --check`: **12 hand byte vectors,
20 upstream hashes and 41 canonical/domain vectors** passed unchanged.

Independently parsed and checked the frozen full debug/release inventories and
JUnit: **361/361 each, zero failures/skips, all 358 baseline names preserved**.
Inspected the full raw gate logs, final no-work logs, formatting/Doxygen results
over 89 active sources, publication red/green records, mapping failure evidence,
variant audits, platform and release-cache action. The archived stale release
tree was preserved and the candidate uses LLVM23.1.4. No unexplained skip or
test expectation/golden weakening was found. A redundant full-suite rerun was
unnecessary after the focused checks and exact source/evidence audits.

Two initial reviewer preservation-script assumptions were corrected: the
coordinator round is excluded from the protected manifest, and new files have
no existing baseline hash. A red/green log check was adjusted to the recorded
CTest summary spelling. These were reviewer audit setup errors, not candidate
failures; no product or frozen evidence input changed.

## Limits and release

Evidence applies to macOS26.5.2 ARM64 with Clang23.1.4, CMake4.4.4 and Ninja1.13.2.
Linux execution and hardware power-loss behavior remain unverified. This approval
does not complete workspace naming, load/full R14, external profiles or later
execution work, and does not authorize installation, publication or merge.

Debug and release are idle; **reviewer ownership of both build directories is
explicitly released**. The coordinator must recheck the reviewed candidate hashes
before advancing. Review-output hashes are recorded by the reviewer evidence
manifest and handoff.
