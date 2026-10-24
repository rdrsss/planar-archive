# V5-02-X05-A3+C01+C02+C03+C04 independent review

Disposition: **request-changes**. C04 corrects the demonstrated A3-C03-R6
registration/collection/remapping allocation-attribution sites and preserves
R1–R5. A separate real Lua reference-allocation refusal can be caught without
retaining resource failure, allowing later callbacks to clear the first token.
A3-C04-R7 below blocks combined approval. Preserve the candidate and evidence;
a fresh corrective search/coder/reviewer round must precede the integrated gate.

Review used shared `dev/planar-v2-vernacular`, HEAD
`3171c49e7b92d3abda01bfbb8bfb0c6b29d4c67a`, with sole serialized build ownership.
Writes are confined to this report and
`tests/native/v5-02-x05-a3-c04-review-evidence/`. Product, current references,
coordinator records and all author/historical evidence remained read-only; other
work was preserved. No legacy Planar tools/roles/skills/queue, installed binary,
operator data, commit, installation, publication or merge was used. Review read
active CLAUDE.md, implementation/roadmap/v5 direction, exact X-REG/X-LIMIT/X-STORE,
the implementation path, C04 authorization and prior A3/C01/C02/C03 findings.

## A3-C04-R7 — Caught Lua memory failure loses resource state and first location

[Allocator](../../../src/engine/runtime/runtime.cpp):49 sets `exhausted` when
its logical heap ceiling refuses, but an actual allocation returning null reaches
Lua without setting that flag. [Native pcall](../../../src/engine/runtime/runtime.cpp):79
receives `LUA_ERRMEM`, checks existing instruction/heap/stack/conversion flags,
and otherwise returns `false` to Lua. It does not persist the memory status.

[External registration](../../../src/engine/runtime/runtime.cpp):826 selects a
correct token and installs native vector ownership before its two raw `luaL_ref`
calls. A Lua memory longjmp at either raw boundary bypasses C++ catches. With no
bridge error or exhaustion signal, a later callback passes the sticky check and
clears/replaces the first token. Successful callbacks then clear it normally.
The module-end host adapter sees only the final module status, not the swallowed
memory failure. The partially retained entry later fails descriptor collection,
or an unrelated later Lua error can determine the whole constructor diagnostic.

The independent reversible production probe uses a valid source-only module with
64 projections, each registered inside the existing native `pcall`. It arms only
actual Lua growth requests during reference retention, returns null for the first
allocation operation and Lua's emergency retry, then permits all later requests.
It does **not** set `exhausted`, inject a Lua status, throw a C++ exception or
modify product source. A private observer disarms after `lua_pcall` returns and
records its real memory status; successful reference retention also disarms.

Both refused requests are **24 bytes**, at original projection2's **descriptor**
reference. Exactly one `LUA_ERRMEM` is caught with `exhausted=false` and no
conversion failure. Later successful callbacks proceed through projection64's
function reference. The expected-resource detectors each return **1**:

| Module tail after the one failed reference operation | Actual constructor result |
| --- | --- |
| Normal completion | `internal / Invalid external profile protocol / catalog.projections[2]` |
| A later ordinary Lua error | `internal / Invalid external profile protocol / profile.modules[1]` |

The no-fault twin succeeds. A function-reference-only allocation window observes
**zero actual growth requests** for this 64-projection fixture and succeeds; no
function-reference allocation failure is claimed. Every owner destruction checks
zero tracked live Lua allocation. Registration refuses in the demonstrated fault
cases; this is not a capability escape or a claim of natural host exhaustion.

[Generator](../../../tests/native/v5-02-x05-a3-c04-review-evidence/reference5.py),
[reversible substitutions](../../../tests/native/v5-02-x05-a3-c04-review-evidence/reference5-probes/delta.json),
[commands/status](../../../tests/native/v5-02-x05-a3-c04-review-evidence/reference5-probes/commands.json),
[normal-tail detector](../../../tests/native/v5-02-x05-a3-c04-review-evidence/reference5-probes/run-one.log),
[later-error detector](../../../tests/native/v5-02-x05-a3-c04-review-evidence/reference5-probes/run-one-later.log),
[valid twin](../../../tests/native/v5-02-x05-a3-c04-review-evidence/reference5-probes/run-control.log),
[function-reference control](../../../tests/native/v5-02-x05-a3-c04-review-evidence/reference5-probes/run-function.log).

X-LIMIT states that exhaustion uses `resource_limit`, logical indices are
one-based original order, and protected frames preserve native ownership.
The runtime wrapper itself requires that Lua not resume after exhaustion,
including caught errors. Actual allocator failure has the same terminal resource
meaning when it surfaces as `LUA_ERRMEM`; this is distinct from a transient
initial allocator refusal that emergency GC successfully recovers. The probe
forces the complete allocation operation to fail before permitting later work.
Retain that failure and its original token before returning control to Lua;
a later successful callback or module error must not replace it. Preserve current
message/limit semantics, longjmp safety, cumulative budgets and C04's nonallocating
selection/fallback design. No new quota or public policy is requested.

The bounded [related-class audit](../../../tests/native/v5-02-x05-a3-c04-review-evidence/caught-memory-class-audit.json)
covers both descriptor/function raw references, native diagnostic/bad_alloc
callbacks, charge/stack/instruction failures, initial setup/sealing, final
collection/sealing and the common nested-pcall boundary used by curated native
constructors/helpers. Bridge and explicit resource flags are sticky; host-owned
protected calls inspect `LUA_ERRMEM` correctly. The missing persistent signal is
at caught Lua memory status. No additional actual helper/function-reference
failure is asserted. Allocator and nested-pcall bodies are byte-identical to the
pre-A3 and C03 baselines: R7 is an earlier residual found during combined review,
not a regression introduced by C04.

## C04 closure and preserved combined boundary

**A3-C03-R6 closes for its demonstrated sites.** All registration field writes
were audited: initial profile ownership, staged module swaps, owned pure global
field move, outer rendered-field swap, and staged final catalog swap. Callback,
collection, remapping and sealing selectors only write a trivial original
kind/index token. Callback validation starts at the module; successful callbacks
clear the token; bridge/explicit resource failures are checked before another
selection. Collection selects before conversion/key validation/normalization.
Catalog prevalidation clears the last completed entry before global work.

Canonical-to-original remapping reads the pure error by const reference and uses
literal string views, checked parsing, nonallocating map iteration and descriptor
identity. Unmatched canonical entries retain the caller boundary; owned global
fields move. Every protected module-call failure renders outside Lua. Rendering
publishes by swap, so actual output-allocation refusal retains the independently
owned module fallback. Final `catalog` storage is staged/charged then swapped;
no allocation defect at the old short-string assignment is inferred.

The independently recompiled **69** class scenarios pass for all three kinds,
original indices10/64, five shipped command offsets, mixed kinds and modules,
first sticky native failures, raw-reference status adapters and module/global
phases. Removed selector/remap windows make zero actual native allocation
requests. Actual projection rendering refuses a 32-byte request and uses module2;
short command/shape rendering records zero requests honestly.

An additional **12** independent scenarios refuse the first real allocation
inside descriptor conversion, after nonallocating selection, for each kind at
index10/64. All pass: the actual 112-byte conversion refusal reports the exact
original entry; separately refusing projection diagnostic rendering's real
32-byte request yields the module fallback. These successful cases do not cover
or supersede R7's additional real Lua allocator detectors.
[Generator/adaptation](../../../tests/native/v5-02-x05-a3-c04-review-evidence/conversion-adaptation.json),
[commands](../../../tests/native/v5-02-x05-a3-c04-review-evidence/conversion-probes/commands.json).

R1's pre-sort key validation and ordinary original-index remapping remain correct.
R2 preserves typed initial provenance resource errors. R3 retains six final
canonical/payload slots, separate framing/digest room, independently derived
13 embedded-metadata byte passes/four graph passes, and eleven embedded/three
external payload passes. The high catalog is 1,041,459 bytes and effective
manifest 1,041,840 bytes; metadata410 and exact/one-byte-short native charge
controls pass. These are conservative logical recipes, not RSS accounting.

R4/R5's shared all-root audit, original D/F root mapping, nonallocating tokens,
separately staged module fallbacks and terminal retry behavior remain intact.
The guard uses one identity/prototype memo, unchanged root order/depth and
all-root inspection before freezing. Successful sealed identities publish only
after full audit/freeze success; failed owners cannot invoke. Protected frames
retain only trivial locals across allocating Lua calls. All ten corrected C03
allocation scenarios and retained repeat/location runners pass.

Combined A3 still structurally verifies and privately copies bounded captured
bytes/modes/order, executes zero-result source-only modules with native owners,
validates/copies final D and seals D/F before later modules. External `_ENV`
is built before shipped evaluation with constructor-only engine, self `_G`,
shared pure libraries and exactly three registration functions; pristine native
identities enroll before module code. Embedded handlers retain their engine.
Catalog/effective provenance bind original inputs. Profile-owner invocation
refuses before provider work. No B/C adapters/semantics or shipped Lua changes
are delivered. The original embedded invocation tail is byte-identical except
for that profile-owner refusal.

## Independent verification, identity and release

[Command record](../../../tests/native/v5-02-x05-a3-c04-review-evidence/commands.json)
runs serialized Debug all+planar_tests, inventory, the exact focused selector,
eight private runners, high catalog, pinned lint/Doxygen and a no-work build.
Both builds report no work. Inventory is **580** unique names, preserving all
**578** previous names. Focused execution is **207/207**, zero failures/errors/
skips/disabled cases.
[JUnit](../../../tests/native/v5-02-x05-a3-c04-review-evidence/focused.xml),
[counts/names](../../../tests/native/v5-02-x05-a3-c04-review-evidence/test-summary.json),
[raw log](../../../tests/native/v5-02-x05-a3-c04-review-evidence/focused.log).
Pinned formatting/Doxygen checks **117 active sources**. Seven retained runners
plus the class runner preserve deliberate red controls and all cleanup checks.
Output-root-only [adaptations](../../../tests/native/v5-02-x05-a3-c04-review-evidence/runner-adaptations.json)
reverse exactly; historical originals remain unchanged.

The author's 23-command final-2 gate, red R6 replays, vector/historical/A0 audits,
and final-1 supersession were inspected. Vectors/historical checks are inspected
author evidence, not independently rerun claims. The superseded copied C03
allocator flag is correctly reset from protected-call kind before every call in
final-2. Product/test/reference bytes were identical across both author gates.

[Scope audit](../../../tests/native/v5-02-x05-a3-c04-review-evidence/scope-audit.json)
independently reconstructs the exact complete C04 owned diff. Shipped Lua,
existing guard/API/build/codec/vendor and historical evidence remain unchanged.
Before/after/final release identity checks verify **21,700 source entries**,
including complete membership/modes/symlinks, **1,171 author artifacts** and
**931 build identities**, with zero mismatches. Source membership excludes only
root .git/build, author/reviewer evidence and this reviewer-owned report.
AGENTS.md remains the CLAUDE.md symlink.

Reviewer probe development is retained: two conversion-generator quote/anchor
mistakes occurred before product probe execution, and the first reference status
observer failed `-Werror` for an unused parameter. Corrected copies have separate
paths. Early observational reference probes return zero to record behavior;
reference5's two explicit product detectors return expected exit1. None of these
harness attempts changed product, author evidence or the frozen candidate.

Author identities are source freeze/release
`4c011b6f9582daf395fc5435c06e37f286da3c6a6c4aadad76ec5be3935a117e`,
owned diff `8c104bd45f9b32c9851fb1ff39cf452b753f4cb96cc3ba400bd2d850a1305462`,
report `56e600476d1166ed58bc2f11019f1c20bcfc1a9ba52b5643fa03dcbee66baa42`,
artifact manifest `299be645df52fdd07d84ee2f2a0e3a1eae6eb342802bacb1909f0b8286b8fca5`,
and build identities `c0a988b1abef0383059ca40e76e5e045eadb0e5ec2dc5a71af0fb07d41818613`.
Review [finalization](../../../tests/native/v5-02-x05-a3-c04-review-evidence/finalization.json)
binds this report and complete reviewer artifact manifest. Serialized build
ownership is released after final hashing; no subsequent reviewer edits follow.

No full580 suite, RelWithDebInfo/release or Linux gate was run. Latest full gate
remains520 per configuration. Combined A3 approval and the integrated X05-A gate
remain open because of R7. Public profile CLI, B/C semantics/adapters, X06
history/storage/interpretation/acceptance, caller receipt/budget orchestration,
mandatory external-profile gate, public cutover, execution/RA01 and Linux remain
open. Public init creates format1.
