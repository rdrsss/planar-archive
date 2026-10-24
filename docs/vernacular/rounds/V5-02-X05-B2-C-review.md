# V5-02-X05-B2-C independent review

Disposition: **request-changes**. **B2C-R1** blocks approval: the first shape
invocation overwrites two already-retained shipped callable references with the
new temporary-registry snapshot tables. The bridge's focused tests pass, but its
retained-registration lifetime requirement does not.

Review used the shared `dev/planar-v2-vernacular` checkout at
`3171c49e7b92d3abda01bfbb8bfb0c6b29d4c67a`. Product, tests, support, candidate
prose and prior evidence remained read-only. Writes are confined to this report,
`tests/native/v5-02-x05-b2-c-review-evidence/`, and ordinary serialized verification
outputs in the exclusively owned Debug build directory. No Planar tools, old
skills/roles/queue, installed binaries, operator data, commits, installation,
publication or merge were used. No product fix was implemented.

## B2C-R1 — Snapshot storage overwrites retained render/init references

The new [fixed registry indices](../../../src/engine/runtime/runtime.cpp#L29)
are integers 10 and 11: pinned Lua defines `LUA_RIDX_LAST` as 3. They are first
written by [execute_shape's snapshot publication](../../../src/engine/runtime/runtime.cpp#L1307),
after all shipped and external registration has finished. Setup does not reserve
these indices before registration.

Shipped [register_command](../../../src/engine/runtime/runtime.cpp#L833) retains
F using `luaL_ref(LUA_REGISTRYINDEX)`. Pinned
[luaL_ref](../../../vendor/lua/0b10/src/lauxlib.c#L710) takes the next index after
the current registry length. In a selected-profile VM, the first shipped F gets
reference 10 (`render`), the second gets 11 (`init`). The new snapshots therefore
replace those functions. The native `command_entry.reference` values keep their
old integers; they no longer identify their original callable. The
[dispatch consumer](../../../src/engine/runtime/runtime.cpp#L1920) reads those
same references. Sealed-identity membership may keep the original function
objects alive, but cannot restore the overwritten reference binding.

The independent [observer](../../../tests/native/v5-02-x05-b2-c-review-evidence/registry.cpp)
creates an ordinary selected-profile owner and invokes its actual retained shape
successfully. A reversible copy of the exact candidate runtime adds only raw
registry reads before snapshot construction and after normal cleanup. It changes
no registry storage, dispatch, limits or error policy. The
[transformation record](../../../tests/native/v5-02-x05-b2-c-review-evidence/registry-transform.json)
proves exact reversal to candidate runtime SHA256
`123c785e971e9a81a4adf36d985f30b90d6dadc1c3fb9c7df3df7b273c45fc7f`.
The [raw run](../../../tests/native/v5-02-x05-b2-c-review-evidence/registry.log)
reports:

| Retained reference | Before first shape call | After successful call and cleanup | Exact identity |
| --- | --- | --- | --- |
| 10, render | function, Lua type 6 | table, Lua type 5 | lost |
| 11, init | function, Lua type 6 | table, Lua type 5 | lost |
| 12, 13, 14 | function | function | preserved |

The probe exits **42**, its deliberate retained-reference assertion failure.
Both compilation steps and linking pass. This is actual reference corruption
under ordinary successful execution, not simulated callable substitution or a
resource-exhaustion scenario. Generic public profile invocation remains closed;
no current public CLI execution failure is claimed.

This violates the assigned round's requirement to preserve retained D/F and
registration identities and the approved X-REG native reference ownership/no
substitution boundary. The author's tag/backing/sealed counts do not inspect the
integer callable-reference bindings and cannot detect it.

**Required correction:** allocate collision-free snapshot storage, or reserve
its complete registry namespace before any `luaL_ref` can allocate references.
Keep the existing alias, cleanup, protected-frame and cumulative-accounting
semantics. Add verification that all retained shipped and external descriptor/F
references resolve to their exact original identities after the first shape call,
repeated calls and GC, and after execution/resource failure cleanup. The
verification can use an owned reversible native observer; it must not enable
public/generic profile command invocation. Preserve this failing evidence and
use a fresh corrective search/coder/reviewer round.

## Reviewed boundaries and independent verification

The native operation binds the existing final owned definition before executing
the actual selected `function_reference`, with original shape order for diagnostic
attribution. It exposes no provider/services or public profile admission. The
input memo preserves container aliases, the freezer uses transient identities,
and raw conversion runs before snapshot cleanup so returned input aliases remain
readable. Registration-to-invocation instruction transition occurs once; subsequent
calls retain the invocation counter and cumulative native/node charges. Sticky
resource and bridge errors precede return/status adaptation, and the new terminal
shape error is consulted by later invocation and definition lookup. These source
checks and passing cases do not override R1's demonstrated registration corruption.

The independent focused selection passes **353/353** in **182.69 seconds**:
346 unit/native and seven acceptance cases, zero failures/errors/skips. It includes
all seven new production bridge cases and the inherited embedded command/runtime
coverage. A separate history/codec selection passes **11/11**, also with zero
failures/errors/skips. The production allocation runner is independently compiled
and linked against the production runtime library: **two cases / 482 assertions**
pass, covering all **67** observed native allocation requests and embedded/moved
controls. The native allocation controls deny one request then permit later
requests; they are not persistent exhausted-heap or natural host-exhaustion proof.

The existing Debug graph builds with no work; pinned formatting/Doxygen passes
for **118 active C++ sources**; the final `all planar_tests` build again reports
`ninja: no work to do.`. Exact commands and raw outputs are retained in the
review evidence. The first focused invocation used a relative selection path
that CTest resolved under the build directory; it exited 8 before running tests.
Its log remains as `focused-attempt01.log`. The corrected absolute-path run is
the only 353-case success claimed. A first scope-audit attempt stopped on the
AGENTS symlink entry; the corrected symlink-aware audit verifies the identities.

Author evidence was inspected independently: the preserved sticky-callback red
run has one failed case; green has seven passing cases. Final focused JUnit has
353 passing cases, supplemental JUnit 11, and allocation JUnit two. The canceled
earlier gate remains unsuccessful evidence. Padding tests establish raw 16 MiB
transport only, not shape semantic acceptance. Prior B2-A+C01 and B2-B+C01 review
dispositions are accurately reflected by the candidate's current prose; this
new B2-C candidate remains unapproved. B1 semantic integration, selected identity/W,
rendering, X06–X08, public admission/cutover, execution/RA01 and Linux remain open.
Public init still creates format1. No full B milestone or release gate is claimed.

## Candidate and review evidence identity

The [scope audit](../../../tests/native/v5-02-x05-b2-c-review-evidence/scope-before.json)
checks all **15** candidate members, all **153** author artifacts and all **392**
frozen source/config/fixture/support/current prose identities, including the
AGENTS symlink. All match. The same identities are rechecked at finalization.

| Reviewed author identity | SHA256 |
| --- | --- |
| Candidate manifest | `16e33d80d5379fcd0c3838b6b26ac24701556631bc08ac6ef852d8129d646f02` |
| Candidate diff | `e231466b1a0d9e7917963e33921b3c61b354c174c165d798b6deb31f4113b387` |
| Artifact manifest | `2bb7859199f905f212ec1e2eefb931da9134c5dda55d58c2e93a0b48f55a2ac9` |
| Author report | `9b7360b0bade465d0693fef30766478b4923b38936a3c21786832be6cf5b62d9` |

[Finalization](../../../tests/native/v5-02-x05-b2-c-review-evidence/finalization.json)
binds this report, reviewer artifact manifest, exact candidate identities and
unchanged final source audit. Debug build ownership is released only after final
hashing, with no reviewer edits afterward. No fixes or further dependent work are
approved by this request-changes disposition.
