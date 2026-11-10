# R19 actual task: engine CMake directory map

## Decisions and authority

The canonical [decisions table](../../../tests/native/r19-cmake-evidence/plan/DECISIONS.md)
and [invariant table](../../../tests/native/r19-cmake-evidence/plan/INVARIANTS.md)
are the handoff inputs. This is a source/build map, not approval or a completed
Planar demonstration. Baseline is `5812823430ea24804519f02620f05ed546386607`,
branch `dev/planar-v2-vernacular`. The prescribed Claude models are unavailable;
the available ordinary agent performed this bounded search. Code is read-only;
only this report and those two tables are owned. No build, test, Planar CLI,
installed binary or operator database was run or modified by search.

The real operator task is to give every directory in `src/engine/` its own
CMakeLists and have its parent add the components. Keep the single repository
`project()`; nested projects would change `PROJECT_SOURCE_DIR`, which currently
anchors every fixture, vendor source and product embedding. The existing
`src/CMakeLists.txt:1-2` already composes engine then cmd using add_subdirectory.

## Current layout and exact target ownership

At baseline engine has one 743-line CMake file and 15 descendant directories:
14 immediate children plus `runtime/closure_guard`. The requested result has 16
CMakeLists files including the engine coordinator. There are 76 textual
`planar_rewrite_module` call sites, some inside loops producing multiple targets,
plus embedded-module helper calls, shared/static support libraries and fixture
executables. Do not confuse textual calls with expanded configured target count.
The coordinator is independently capturing the actual configured baseline graph.

| Directory | Owning declarations and amendments from baseline engine CMake |
| --- | --- |
| `value` | `planar_value` (4-5). |
| `budget` | `planar_budget` (6-8). |
| `lua` | `planar_lua_loader`, its test definitions/link (9-13); trusted standard C libraries `planar_lua_trusted_libs` (87-94). |
| `capture` | host_path/admission/native/profile_codec/profile/capture/directory (14-41), `planar_admission_syscalls` and dependent test links (23-33), tree_digest (732-734). |
| `canonical` | `planar_canonical` (43-45). |
| `hash` | `planar_hash`, test-only capture edge and vector definition (46-51). |
| `json_codec` | `planar_json_codec` (82-84). |
| `history` | effective_catalog, record, request_v2/v3, execution_codec (53-75); tree_core/tree (332-339); provenance_codec/interpretation_codec/provenance/interpretation and tests (386-409); read and its test link (458-464); history fixture (466-472); effective_provenance (635-640); interpretation_v2_codec/recorded_interpretation (709-724). |
| `ledger` | completion (1-2); sqlite/types/storage_types/storage/acceptance/format/execution_schema/execution/recovery (239-315), execution snapshot probe and coverage test (297-307); private publication/collision/atomic/metadata variants (341-384,432-456,609-633); history_codec and format test amendments (411-421); D8 and X03-B1 fault test sources, format2 fixture (595-606); duplicate execution-test history-read link (738), kept semantically as existing. |
| `acceptance` | intent (258-260), prepared plus test edge/fixtures (423-430), source_capture/load plus test edges/fixtures (504-519). Intent is split out of the old ledger-contiguous block. |
| `export` | source_export and test links/fixtures (474-483); exact-source POSIX export fault variant (485-502). |
| `process` | resolve/runner (726-730). |
| `runtime` | shipped runtime embedding and runtime target (95-108,114-118); bridge/budget/tight variants and definitions (120-199); V5-02-B1 variants (201-237); private load/c01 runtime variants and test definitions (521-593); X05 closure audit/runtime variants (642-685); shapes bridge/source/harness and test source (687-707). This file adds its closure_guard child. |
| `runtime/closure_guard` | `planar_closure_guard` (110-112), using paths local to this directory. |
| `execution` | verdict (77-80), snapshot_value/change_value/drafts/provenance_input (317-330), check (735-737), recovery_capture (740-743). |

`include(embed_lua)` (86) and interpreter discovery (207,353) are shared setup.
Hoist interpreter discovery to the engine coordinator before children so every
sibling receives the ordinary variables even on a clean configure. Include the
embedding helper there or explicitly in each consumer. Do not depend on a prior
CMake cache or sibling variable propagation.

A workable explicit parent order is value, budget, lua, capture, canonical, hash,
json_codec, history, ledger, acceptance, export, process, runtime, execution.
The current file already has numerous right-hand forward target references
(e.g. capture to hash and runtime to history_read). CMake resolves those named
link targets during generation. They are distinct from trying to amend an
undefined left-hand target. Keep each left-hand target amendment after its local
definition or use a reviewed post-declaration phase; do not duplicate targets to
force a superficial topological source-file order. No existing product edge
needs to change for this organization.

## Directory-scope and generated-path hazards

`cmake/rewrite.cmake:11-35` creates ordinary targets in the caller's directory,
interprets interface bases relative to `CMAKE_CURRENT_SOURCE_DIR`, appends tests
to `planar_tests`, and discovers Catch2 tests. Every `capture/foo.cpp` becomes
`foo.cpp` in capture; every source/interface/test argument must be reviewed,
not just module interfaces. The installed CMake documentation confirms
`target_sources` interprets relative paths from the current calling directory
(CMP0076). This matters for the late `runtime/shapes.t.cpp` amendment and D8/X03
external test additions. Prefer amendments in their target owner's file and
absolute paths for external sources, as the current file already uses.

Current runtime source aliases (124-125), generator source arguments to ledger
(355,371,436,613) and export (492), and all configure-dependency source paths must
lose the now-redundant owning directory prefix. Preserve the complete source and
generator dependency list; merely obtaining a successful first configure does
not establish regeneration behavior.

All generated variants currently live directly under the engine binary root.
Natural child binary directories relocate them. In runtime, `planar_runtime_unit`
and its interface alias must remain in the same scope as the budget/tight/shapes
consumers. Generated `r14-d6` variables must remain in scope for both the load
variants and `PLANAR_LOAD_GENERATED` test definition (555-557). Moving declarations
between siblings does not propagate these ordinary variables upwards; avoid
unnecessary `PARENT_SCOPE` by retaining each complete generation/use block in one
owner. The other ledger/export generation variables are internally local.

`cmake/embed_lua.cmake:2-34` emits generated modules in the caller binary directory
and computes each logical original path from its source. Variant users then
rewrite build-local command paths to `planar/commands.lua` (226-230,543-547,
582-587,674-678). Continue reading exactly the helper's new output path; do not
read stale copies left under the old root. Compare generated byte payloads and
logical originals against the baseline, with only explicit generated path fields
permitted to relocate. Never satisfy this check by broadly ignoring generated
sources or every compile definition containing a path.

Catch2 defaults discovered-test `WORKING_DIRECTORY` to the creating directory's
binary path (`vendor/catch2/c601/extras/Catch.cmake:178-179`) and installs discovery
includes into that directory (`:312-315`). Thus a real add_subdirectory split
moves discovery registrations and native test cwd as well as artifacts. Compare
sorted full CTest JSON by name, not numeric order. Either preserve old cwd
explicitly or accept only the exact documented per-owner rebase. Keep labels,
timeouts (including 120-second runtime/file-replacement cases), fixture arguments
and selected binaries unchanged. The explicit ledger sections coverage test
already uses the repository as cwd and should remain so (304-307).

Do not put EXCLUDE_FROM_ALL on the new add_subdirectory calls. Existing test
executables and private variants have their own reviewed exclusions; production
all dependency membership must remain equal by target name. No change to
`cmake/rewrite.cmake` or `cmake/embed_lua.cmake` is inherently required.

## Historical support and current references

Active `scripts/rewrite-check.py`, current acceptance registrations, X08 tests
and command CMake refer to targets via target expressions or CTest. They contain
no hardcoded engine artifact directory assumption requiring adaptation. The
command sources use engine **source** paths for generated authoring variants;
those source paths stay unchanged (`src/cmd/planar/CMakeLists.txt:328,389-397`).

Many one-off historical scripts under `tests/native/*support` hardcode engine
root artifacts/generated sources and sometimes exact root-CMake substrings.
Examples are `v5-02-b2-support/audit-variants.py`,
`v5-02-x05-b1-support/audit.py:13-30`,
`v5-02-x05-a1-support/probe.py:23-24,36`, and
`v5-02-x05-c4-d2-b2-support/quota.py`. Their retained snapshots/manifests describe
specific prior rounds. Preserve those historical bytes rather than rewriting
them to certify this layout. Any new calibration/probe in this task must use
current codemodel/compile commands or explicit new paths and record its candidate.
Old raw artifacts are not evidence of a new-layout build.

`docs/architecture.md:115-120` should describe the engine coordinator and the
component CMake files. Current `docs/testing.md:1777,1823` names the old root file
for ledger fixture/runtime timeout wiring; update to owning children or clearly
label the historical references. `scripts/linux-gate.Dockerfile:89` has a source
line/path comment for the ledger schema input; update that comment if included
in the implementation scope. No Docker behavior or dependency pins need change.
The bootstrap all-CMake production-definition scan is recursive already
(`src/cmd/planar/bootstrap.t.cpp:3394-3425`); it should continue to pass without
weakening its assertion or adding exclusions.

## Implementation and verification handoff

The assigned `R19-CMAKE.md` round confines the coder to all 16 engine CMake
files. Do not parallelize extraction within the same root file. The coordinator
owns reference prose, the graph/check adapter, round/status records, declarations
and actual new-candidate Planar demonstration state. Fresh plan review precedes
code; fresh independent code/evidence review follows the actual gate. The actual
engine source root has sixteen explicit owned CMake file boundaries and an
immutable baseline engine tree with physically disjoint scratch; these scopes
do not require any product C++/Lua or historical evidence edit.

Capture the untouched configured baseline before extraction. Compare configured
target name/type sets, exact per-target native/interface/test sources, resolved
link dependencies and all compile definitions/options/includes. File API target
IDs and CMake directory lookup markers may change; normalize only those structural
identifiers and explicit owner-based build-path translations, keeping actual edges
and source memberships exact. Compare normal all and planar_tests dependency
closures, full actual test names/properties and generated payloads. A directory
reachability audit should show every descendant CMakeLists reached once by its
immediate parent and no nested project().

Then run direct supported configure/build/test gates, record nonzero exact counts
and zero skips, applicable pinned lint/Doxygen, and actual no-work repeat. Fresh
configure matters for Python/sibling-scoping errors. Existing debug/release output
roots may retain old files: use actual current codemodel/test registrations and
build dependencies, not existence of an old executable as proof. The coordinator
sets build-lane ownership and exact platform commands before the coder builds.
The source invariant table names omitted-directory/source/dependency/ceiling and
regeneration mutants; no such fault has been executed by this search agent.

The user's actual task selects a useful R19 subject, but successful CMake work
alone does not close R19's full matrix, quota groupings, restart/fault or live
check/verdict requirements (`roadmap.md:419-431`). Record the actual driver workflow
and any scope limitations separately; do not claim them from this map.
