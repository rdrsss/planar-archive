# X05-B1 pure Lua shape foundation candidate

Status: candidate pending fresh independent review. This slice adds optional
trusted internal context to the exact product `schema.lua` and `semantic.lua`.
The product runtime API, product Lua embedding/export list, selection, W recipes,
render envelopes, acceptance and public CLI admission remain
unchanged. The shared dirty rewrite checkout starts at
`3171c49e7b92d3abda01bfbb8bfb0c6b29d4c67a`; unrelated changes are retained.
No Planar tools, installed binaries, real databases, commits, installation,
publication or merge are used. Build ownership is serialized Debug only.

The context selects `declarations_shapes_v1` shape-ID/stages admission separately
from job/envelope version1 data. The default literal two-shape enum is unchanged.
Custom shapes resolve from a trusted final-descriptor lookup and use a trusted
call adapter. Unknown IDs are `reference` at `job.shape`; required/forbidden/
optional stages use the existing nonempty worker-ID sets with exact coverage.
The version1 normalization, ownership, registry allowlists, subscriptions,
questions, assignment, reviewer, effective-check and mandatory graph policy run
for every job before any custom callback. Base graph cycles also precede calls.

The second pass supplies exact normalized `{version:1,job,spec,assignments,reviewer}`
and validates exact success/refusal envelopes. Obligations enforce declaration IDs,
unique IDs, distinct existing work endpoints, start/complete phase and done quality.
Material records acquire the synthetic shape namespace and original assignment
roles. Their edges join every original unit/role/acceptance graph edge; grouped
work remains unit-level. Deterministic full cycle checking reruns afterward.
Validated original IDs are sorted before material ordering, avoiding repeated
comparison of the identical synthetic prefix. The profile-only internal
`shape_results` sidecar contains job ID, obligations and guidance; it is not the
final B3 selected-shape/definition identity.

Guidance enforces UTF-8, no NUL and 65536 bytes. The implementation checks 10000
obligations per job and a cumulative 100000 across jobs. Real helper probes accept
10000/job and refuse 10001 with the typed semantic resource error. Three jobs
with 10000 each (30000 total) succeed. An eleven-job ×10000 attempted workload
hits the unchanged real native 100000000-instruction limit first. Thus B1 does
not dynamically prove exact 100000 acceptance or 100001 semantic refusal.
The generator, validation, sort and graph work all share that existing instruction
counter; no quota override or counter-only substitute is introduced. Sorting the
original IDs reduces avoidable common-prefix comparisons without changing limits.

The distinct test runtime uses an exact forward/reversible copy of production
runtime/interface and embeds exact product JSON/schema/semantic bytes plus a
controlled test-only provider. It never injects callbacks into job data or changes
product bootstrap exports. Controlled returned values, actual native input/output
conversion, instruction accounting and the real product helper algorithms are
exercised. This does not prove production retained profile-F dispatch, native
source identity, immutable actual F input, the complete returned value's 16 MiB
boundary, or cumulative actual shape invocation orchestration. B2 owns those
native boundaries; B3 owns selected identity/W integration and B4 render behavior.
Current shipped W and render golden checks remain in the focused gate.

Exploratory failures are preserved. `build-01.log` records a Catch INFO expression
issue in the new test helper; `build-02.log` records its unused copied helper.
`probes-01.log` and `probes-02.log` preserve the product double-tag defect: empty
or singleton normalized obligation arrays were tagged before the existing sort,
which attempted to tag the same table again (`already_typed`). The 10000 case
passed because merge sort produced a fresh table. The correction sorts validated
original IDs and tags the final result exactly once. A malformed NUL test input
also initially failed native JSON fixture creation; NUL/invalid-UTF8 probes now
construct the malformed returned guidance inside the test-only Lua provider.
These fixture/compiler issues are separate from the product double-tag defect.
`probes-03.log` preserves the first attempted ten-job cumulative workload's
instruction refusal. `probes-04.xml` records 17/17 exploratory passes before the
final eighteenth sorting/base-policy-preservation case.

The final frozen Debug gate passes **218/218** in **96.52 seconds**: 211 unit
and seven acceptance cases, zero failures/errors/skips/disabled, every status run.
All eighteen B1 cases pass. The exact 600-name registered inventory preserves all
582 approved A4 names; this is an inventory, not a full 600-case execution.
`final-gate/selected-tests.txt` binds the complete selected set, including all
runtime/schema/semantic/selection/render tests, command composition tests,
interpretation, load preparation, hash, A0 exact shipped-source audit tests and
all seven acceptance scripts. Old quota W, Markdown and JSON goldens remain exact.

Four source vector generators, four inherited runtime/CLI exact-source audits,
the B1 native forward/reverse and embedded-original/assembly audit, and pinned
formatting/Doxygen all pass. Lint checks 118 active C++ sources. The final
all+planar_tests build reports `ninja: no work to do.`. Configure/build/test/audit/
lint/no-work commands serialize in one debug directory. Source membership,
SHA256, modes and AGENTS symlink are unchanged across every command boundary for
the frozen 375-entry active source/configuration/fixture/support/current-doc set.
`final-gate/source-freeze.json` lists the precise freeze scope, excluding evidence
and inactive/generated paths. No product/test/support/current-doc edits follow
that freeze. Full commands, JUnit, immediate LastTest/cost data, inventory,
source audits and count checks are retained under `final-gate/`.

`owned.diff`, `final-owned/` and `candidate-manifest.json` bind the thirteen owned
source/support/current-document files. `build-identities.json` binds tested
binary/configuration identities; `artifact-manifest.json` and `finalization.json`
bind the released evidence. Only this new B1 evidence directory is finalized
after the gate. Fresh independent review is required before advancing B1.
No full B milestone or release suite is claimed. X05-C adapters/services, X06
history/interpretation/acceptance and receipt/whole-invocation sharing, X07 public
cutover, X08 mandatory external-profile acceptance, execution/RA01 and Linux remain
open. Public init still creates format1.
