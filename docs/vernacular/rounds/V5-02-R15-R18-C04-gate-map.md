# C04 residual gate map

Read-only investigation for the full review/fix cycle. Evidence inspected: C03 catalog report, `tests/native/r15-r18-c03-evidence/catalog/runtime-tests-2.log` (484 cases, 465 passed, 19 failed), and `tests/native/r15-r18-c03-evidence/debug-gate-1.log` (1,820-test full Debug run). This map did not run builds or alter product or test sources. The full run predates some concurrent fixes; failures must be matched against the final rebuilt source before disposition.

## Historical catalog admission: separate product investigation required

`src/engine/history/effective_provenance.t.cpp` reconstructs independently recorded inputs from `PLANAR_EFFECTIVE_VECTORS/catalog.bin` and `files.tsv`. These committed historical catalogs have five shipped command rows, including the previous history signature. They are not a request that the current VM register only five commands. Related interpretation-v2, recorded-dispatch and ledger history-verification failures also consume historical provenance vectors.

The updated native effective catalog validator requires the current seventeen shipped rows and current history signature. This is a likely product compatibility gap: admission of a newly registered current catalog and validation of an already recorded supported historical catalog have different purposes. Review the supported format-1/format-2 history contracts and raw-provenance handling, then preserve recognition of the exact historical catalog shapes/signatures where those formats require it. Do not relax arbitrary command admission. Do not regenerate immutable historical goldens to conceal rejection by the current validator. The read-only evidence establishes the mismatch and failure cluster; the exact historical admission policy still needs a contract-focused implementation review and regression run.

Assign this independently from ordinary current-VM expectation updates. Current runtime tests expecting five registrations must become exact seventeen-command tests; immutable historical vectors must remain historical.

## Generated load failure: fixture generator deletes handlers

The base `tests/native/r14-d6-support/generate.py` replaced the span from `local function load_command` through the next `local function init_command`. R15–R18 inserted activation, dispatch, preview, status, acquisition, checkpoint and recovery functions inside that span. Replacement deleted their declarations but retained registrations, causing VM creation failures before the intended malformed-load protocol assertions in `commands.t.cpp` around lines 829, 855, 932 and 942. The C01 generator invokes this base generator and inherits the fault.

Root has now corrected the endpoint to `activate_command` and asserted that the replaced span contains exactly one local function. Rebuild the affected generated variants and confirm each negative test reaches its intended load boundary. This was an invalid test injection, not evidence that the production load protocol should accept malformed inputs.

## Resource failures: obsolete test calibration, not established product regression

`src/engine/CMakeLists.txt` supplies the small test variant with native/live-Lua ceiling 39,845,888, instruction ceiling 19,900,000, registration ceiling 1,000,000 and node ceiling 25,000. The production runtime native ceiling remains 512 MiB. Honest embedded-source growth increases registration costs: `register_profile` charges original and assembly payload passes, metadata/tree structure, and the catalog buffers. There is no evidence here supporting removal of these charges or an increase in production limits.

The ordinary `padded_probe(0, 0)` now measures native peak 31,009,820 versus the old 24,503,619, exceeding the small variant's 75% threshold of 29,884,416. Its real quota fixture, custom shape profile and two jobs should remain ordinary. The larger source also exhausts the old zero-padding controls, so the failed controls cannot be fixed by suppressing their positive assertions.

### Authority and calibration method

`V5-02-X06-D-C04.md` D9–D11 explicitly govern this:

- D9 defines per-column exclusions and makes a ceiling adjustment the default response to failed ordinary margins, rather than widening exclusions.
- D10 identifies this as test-variant tuning, not a product or authority choice.
- D11 selects the smallest 1 MiB native multiple satisfying at least 25% free native and live-Lua capacity for every non-bracketing fixture in both Debug and Release. It must remain below the deliberate big-Lua fixture (approximately 48 MB) and below 67,108,864. Native and live-Lua share the ceiling. Other prescribed increments are 1,000,000 instructions, 100,000 registrations and 500 nodes.
- D12 permits enumerated dependent calibration changes. D27 requires shipped/final and raised-scaling measurements; D29 invalidates measurements when source changes afterward.

The ordinary peak alone would fit at 40 MiB, but that is only a lower bound, not a recommended final ceiling. Other ordinary fixtures and the instruction cost of the new native boundary must also qualify.

Freeze the runtime/profile/catalog/publication source first. Record source hashes, measure all five columns in Debug and Release, state exclusions by column, choose the smallest qualifying test-only ceilings, then rerun positive/negative brackets and charge-deletion mutants. Keep production ceilings, quarter-free inequalities and deliberate refusals unchanged. Linux-specific offset pins require Linux evidence; do not infer them from macOS offsets.

### Exact dependent controls to inspect

| Source/control | Existing calibration and observed issue | Required preservation |
| --- | --- | --- |
| `external_registration.t.cpp`, `c02_off_boundary` | macOS 227275, Linux 227301; current macOS observation 145332 | Remeasure after source freeze. Preserve window 8; request-record mutation shifts about 33 padding units and must still be detected. |
| Same, `c02_outputs_gap` | macOS 4742, Linux 4737 | Remeasure fixed-root off/on gap; preserve a discriminating bound and its mutation protection. |
| Same, C03 registry controls | `c03_registry_pad=200000`, entries 1, role-pad files 2, window `1<<18`; current base boundary 141136 means added registry padding no longer fits at zero external padding | Preserve successful zero-padding controls, exact boundary success/next refusal, and charge-deletion mutants. Recalculate dependent floors (3391, 198276, 174140) from measured slopes if needed. |
| Same, N30 `role_run` | 262144-byte projection built by 18 doublings; projected boundary -1 versus plain 145661 | Restore a real positive projected control; preserve meaningful transport-cost separation (existing minimum 1000) and mutation coverage. |
| `runtime.t.cpp`, quarter-free composed-family test | Ordinary probe and 20000-padding slope extrapolate instruction use at the native boundary | Preserve at least 25% instruction headroom at the recalibrated native boundary, as well as all ordinary margins. |
| Same, composed C01 family | Extra-job stages 10, sink 16, first 15, native 17; spin 900000/950000 | Inspect dependent brackets if instruction/native calibration changes; preserve the intended refusal reasons. |
| Same, deliberate large-Lua fixture | Doubling construction produces roughly 48 MB total Lua allocation | Must continue refusing with `lua_bytes`; raising the test shared limit cannot erase this control. |
| Same, registration equality/tight variant | 1000000 equality; separate 500000 tight variant | Keep unless measurements justify explicitly enumerated recalibration; preserve exact success/refusal semantics. |

Assign calibration to a fresh test-fixture coder after the production edits stabilize. Allowed ownership is the test definitions in `src/engine/CMakeLists.txt`, dependent calibration constants in runtime tests, and fresh evidence. Correct product resource accounting must not be optimized away merely to preserve obsolete test constants.

## Reconcile fixture lacks real completion evidence

`src/engine/runtime/reconcile.t.cpp` around lines 451–457 constructs two `done` worker rows and a blocked reviewer, changes the workspace goal, and expects three transitions. Its `quota::snapshot` and `quota::row` helpers provide no completion history, observed outputs or completion lineage. The current reconciler looks for applicable or lineage completion evidence before applying the historical stale/restore decisions. With no such evidence the workers retain their mock `done` state and only the reviewer transitions, producing one rather than three.

Repair the fixture to represent actual completed work: create completion evidence tied to the recorded workspace/work fingerprint, then change the material input. Both workers should become stale. Under the R18 requirement that reviewer readiness depends on workers actually being done, the reviewer must remain blocked; the correct scenario has two worker transitions, not a reviewer-ready third transition. Add or preserve a separate restoration scenario where the recorded workspace returns and valid inactive worker completions restore `done`, allowing the reviewer to become ready. Running workers must still block it. Do not change product readiness to satisfy fabricated done rows or simply change the assertion to one.

## Other failures to reconcile after rebuild

- `runtime.t.cpp` baseline catalog expectations and `tests/native/v5-02-x05-a0-support/shipped.t.cpp` still expect five current registrations in the inspected source. Update independent exact current schemas to seventeen, including history event and the check/review response commands. The interpretation variant count also changed (observed twelve versus old five). Keep this work separate from historical-vector compatibility.
- `execution_schema.t.cpp` around 1097–1118 expects the low-level header reader to reject user version 3 on its own. Format 3 is now supported. Verify the current boundary contract: the header can recognize version 3, while the full schema opener must reject a format-2 schema carrying a forged version-3 header. Preserve that negative assertion at the proper supported boundary.
- `interpretation.t.cpp` complete quota normalized JSON/canonical golden failures include semantic field differences, not only a changed stdlib digest. Classify the exact differences against the approved interpretation contract before refreshing current goldens.
- Generated-original/legacy-assembly authored-byte checks need source-versus-snapshot classification before updates. Historical byte fixtures should not be refreshed blindly.
- Some authoring/help/catalog failures came from binaries preceding the catalog coder's final fixture changes. Rebuild before assigning duplicate work.

Profile load format-3 handling, history-event routing, init diagnostics, publication binding and history codec operation checks are already assigned to other C04 coders and are excluded from new ownership here. This map is a triage artifact, not final approval or evidence that the complete gate passes.
