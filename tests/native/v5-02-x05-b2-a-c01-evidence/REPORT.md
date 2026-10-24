# X05-B2-A-C01 — Complete definition lookup error boundary

Status: correction candidate for fresh independent combined B2-A+C01 review.
The sole product edit is `command_vm::resolve_shape_definition` in
`src/engine/runtime/runtime.cpp`. Work used the shared
`dev/planar-v2-vernacular` checkout at
`3171c49e7b92d3abda01bfbb8bfb0c6b29d4c67a`, preserving the intentional dirty
rewrite tree, earlier candidates, failures and review evidence. No Planar tools,
legacy skills/roles/queue, installed binaries, operator data, commits,
installation, publication or merge were used. This author held sole serialized
Debug build ownership.

## Correction and retained behavior

B2A-R1 identified allocating early diagnostic construction before the lookup's
`try`. The complete owner guards and exhausted-state adaptation now run inside
that boundary. Allocation/length refusal adaptation handles a null moved-from
owner without dereferencing it; when an owner exists, it installs the external
resource diagnostic only if no sticky conversion diagnostic already exists.
Copying an existing diagnostic can fail once without replacing its original
message, limit or maximum. This applies equally to an already-sticky native-byte
quota refusal. All returned logical locations remain `catalog.shapes`.

Ordinary unsupported/unknown/projection-namespace refusals retain the existing
`internal` / `Invalid external profile protocol` behavior. A valid profile owner
can still resolve a registered shape after an ordinary unknown-ID refusal.
A real allocation refusal makes a live profile owner terminal for later lookups.
Embedded owners remain unsupported by this definition API and retain the
protocol-first refusal on retry, even if allocation adaptation installed sticky
resource state. A moved-from object has no owner state to mutate and continues
to refuse protocol on an ordinary retry; its moved-to owner remains independently
usable. The correction does not claim that embedded retries return resources.

There is no public API, CMake, vendor, Lua, codec, history, descriptor, hash,
identity or reservation-policy change. The method still recomputes exact
`shape_definition_v1` identity from the final descriptor, logical external owner
and original captured module bytes without F/Lua/provider execution or invocation
budget transition/reset. Generic selected-profile invocation remains closed.

## Actual one-shot regression evidence

The immutable original review `adapter.cpp` is compiled directly against the
production API, with only its private replacement global `operator new` runner.
The runner fails exactly one actual native allocation and then permits subsequent
allocations; observations end before Catch assertions. No product test knob,
source variant, private state injection or persistent exhausted-heap guarantee
is introduced. Strings in typed diagnostics can still require allocation under
persistent host memory exhaustion, which these probes do not simulate.

Before editing production source, [before/commands.json](before/commands.json)
compiled/linked that unchanged source and reran all three original cases. The
run exits **42** with three failed cases and six failed assertions: embedded,
moved-from and already-exhausted diagnostic adaptation each escapes
`std::bad_alloc`. Both immutable earlier review artifacts and this fresh red run
remain preserved.

The same unchanged three cases pass against the correction. The unchanged review
adversarial suite also passes **377 assertions in three cases**, including every
observed allocation ordinal on a successful lookup, exact independent identity,
same-ID projection separation, redacted unknown IDs and unknown-ID allocation
adaptation. New unchanged-production-API `boundary.cpp` controls add **56
assertions in three cases**: ordinary protocol recoverability, embedded/null-owner
resource category and field with moved-to independence, and a real cumulative
native-byte quota refusal followed by a second actual one-shot failure copying
its diagnostic. The second refusal and a normal subsequent retry retain the
first exact code/message/limit/maximum/field. Native production test additions
would require invasive allocation hooks; the existing meaningful cumulative
native test remains unchanged, supplemented by this isolated allocator runner.

Development and final runs retain exact compile/link/run argv and JUnit under
[development](development/commands.json) and [final-probes](final-probes/commands.json).
The original three green cases contain 23 assertions. No failing run is erased.

## Frozen focused gate

A single pre-gate scoped snapshot fixes source, build configuration, fixtures,
non-evidence test/support files and current reference documentation. The
coordinator-owned round record and historical evidence are outside that scoped
snapshot. The gate configures Debug and builds `all planar_tests`, preserves
**606 registered names**, including all 600 B1 and 582 A4 names, and selects
**252 focused cases** (245 unit/native and seven acceptance cases). Inventory is
not a full 606-case test run. This is a focused correction gate, not a B milestone
or release gate.

The final gate passes **252/252 in 133.54 seconds**, with zero failures, errors,
skips or disabled cases. All three allocator executables pass; pinned formatting
and Doxygen check **118 active C++ sources**. All vectors and inherited source
audits pass, and `all planar_tests` finishes with `ninja: no work to do.`.
Executed commands and results are bound in [final-gate/commands.json](final-gate/commands.json),
[final-gate/test-summary.json](final-gate/test-summary.json) and
[raw-evidence-audit.json](raw-evidence-audit.json). The raw audit independently
matches every selected name to JUnit, console starts and the immediately saved
LastTest log. Source snapshot equality is checked at gate completion and again
after final report/manifests. All previous author/reviewer artifacts are verified
against their immutable manifests. The prior 378-member scoped source inventory
is retained, with only seven owned existing-file changes and three new support
files. AGENTS.md remains a symlink to CLAUDE.md.

The independent shape vector retains the 733-byte known answer
`xxh3_128:b6d3718ddeb0cd1bc49b9e866a638b99`. Four existing vectors and all inherited
exact-source audits remain part of the gate. Source audits complement behavioral
controls and do not establish callable execution. Candidate files, original
bytes, scoped diff, built binary identities and evidence are bound by
[candidate-manifest.json](candidate-manifest.json), [owned.diff](owned.diff),
[scope-audit.json](scope-audit.json) and [finalization.json](finalization.json).
Build ownership is released only after final hashing; no author edits follow
release.

B2-B codec accounting, B2-C retained F/immutable input/complete-return transport,
B3 identity/W, B4 rendering, X06–X08, execution/RA01 and Linux remain open. Public
init still creates format1; the latest full gate remains A4's 582 per build.
Fresh independent combined review is required before advancement.
