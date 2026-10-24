# Repository map for the rewrite

R01 inventory at starting revision `3171c49e7b92d3abda01bfbb8bfb0c6b29d4c67a`
on `dev/planar-v2-vernacular`. This maps the existing checkout; it does not claim
that replacement targets exist or that any reused component is approved. The
[implementation plan](implementation.md) and [roadmap](roadmap.md) define the
approved destination and staged removal. R01 changes instructions and this map
only. No product build, install, migration or Planar command was run for it.

## Active instructions and authority

[AGENTS.md](../../AGENTS.md) is a symlink with target `CLAUDE.md`.
[CLAUDE.md](../../CLAUDE.md) and [CONTRIBUTING.md](../../CONTRIBUTING.md) now direct
rewrite work through file-based rounds, fresh agents and direct serialized
checks. Product references, authored operational agents and skills still contain
legacy requirements. They describe the old product and are superseded as rewrite
workflow authority. Do not follow their claims, queue, artifact or installation
rituals. Their eventual removal or refresh belongs to later rounds.

## Current build graph

| Surface | Source and symbols | Assessment |
| --- | --- | --- |
| Root configuration | [CMakeLists.txt](../../CMakeLists.txt): `cmake_minimum_required(VERSION 4.3)`, `planar_tests`, dependency/module includes and source/tool subdirectories | Keep pinned C++26/module capability; replace old product wiring in R04 onward. |
| Presets/toolchain | [CMakePresets.json](../../CMakePresets.json): `base`, `debug`, `release`, `dist`; [llvm-toolchain.cmake](../../cmake/llvm-toolchain.cmake) | Ninja, pinned LLVM and libc++ are reuse candidates. `dist` enables portable/runtime and version metadata settings; it still packages the old product. |
| Module registration | [module.cmake](../../cmake/module.cmake): `planar_module`, `planar_binary`, `planar_mark_test_binary`; [architecture.cmake](../../cmake/architecture.cmake): `planar_check_architecture`; [scan.cmake](../../cmake/scan.cmake): `planar_disable_third_party_scan` | Retain useful module/test mechanics; review old layer rules and execute/database guards against the new kernel architecture. |
| Source layers | [src/CMakeLists.txt](../../src/CMakeLists.txt), [lib registration](../../src/lib/CMakeLists.txt), [engine registration](../../src/engine/CMakeLists.txt), [command registration](../../src/cmd/CMakeLists.txt) | Current order is lib, engine, cmd. Comments include historical port narratives; actual registrations and link edges determine today's graph. |
| Embedded data | [db CMakeLists](../../src/lib/db/CMakeLists.txt), [cmake directory](../../cmake/), [migrations](../../migrations/), [template defaults](../../templates/defaults/) | Old migration and configuration embedding must leave the render build. Reuse of code-generation technique needs review. |

All five current binaries use `planar_binary` and target names prefixed with
`planar_cmd_`; the hyphenated names are output binaries:

| Binary / target | Wiring | Legacy purpose / disposition |
| --- | --- | --- |
| `planar` / `planar_cmd_planar` | [CMakeLists](../../src/cmd/planar/CMakeLists.txt), `DEPENDS` includes planning, runtime, identity, ingest, workbench and other engine buckets | Operator planning CLI. Replace with the thin render CLI; old handlers and catalogs are removal candidates. |
| `planar-agent` / `planar_cmd_planar_agent` | [CMakeLists](../../src/cmd/planar-agent/CMakeLists.txt), `engine_runtime`, `engine_routing`, `engine_planning`, `engine_hostqueue` | Claims, actions, dispatch and queue. Remove from the replacement product. |
| `planar-watch` / `planar_cmd_planar_watch` | [CMakeLists](../../src/cmd/planar-watch/CMakeLists.txt), runtime/planning/hostqueue and `ftxui` link libraries | Old read-only observation/TUI. Remove from the render product. |
| `planar-ext` / `planar_cmd_planar_ext` | [CMakeLists](../../src/cmd/planar-ext/CMakeLists.txt), external/extsync/identity/planning/template engines | Jira/GitHub integration. Outside the first slice; remove old surface. |
| `planar-execute` / `planar_cmd_planar_execute` | [CMakeLists](../../src/cmd/planar-execute/CMakeLists.txt), `engine_execute` | Workflow runner and engine client surface. Remove; it is not the replacement declaration loader. |

## Reuse and removal inventory

These are candidates, not permission to delete files in R01. Check incoming
references, tests and build edges when each removal is scoped. Keep retained
libraries only when their required behavior is reviewed and tested.

| Source | Candidate disposition and reason |
| --- | --- |
| [core](../../src/lib/core/), especially `check.cppm` | Reuse invariant checks and useful low-level helpers after review. |
| [json_text](../../src/lib/json_text/), [json_dom](../../src/lib/json_dom/), [textview](../../src/lib/textview/), [log](../../src/lib/log/) | Review escaping, parsing, text and logging individually. JSON serialization is not the new canonical identity encoding. |
| [sha256](../../src/lib/sha256/) | Existing hash code is evidence of conventions only; the replacement requires versioned seed-zero XXH3_128bits vectors. Do not substitute SHA256. |
| [cliapp](../../src/lib/cliapp/) | Review argument/help infrastructure independently of old catalogs, exit tables and CLI11 policy. The new command contract belongs to R02. |
| [process](../../src/lib/process/), [git](../../src/lib/git/) | Review low-level behavior for future driver/check needs. Render-only work does not execute checks or manage worktrees. Avoid importing process capabilities into declarations. |
| [db](../../src/lib/db/), [scope_ref](../../src/lib/scope_ref/), [policy](../../src/lib/policy/) | Remove old schema/path/audit coupling from render. Any later storage design is new work, with no legacy conversion. |
| [queuerule](../../src/lib/queuerule/), [installed_surface](../../src/lib/installed_surface/), [document_authority](../../src/lib/document_authority/), [activity_rollup](../../src/lib/activity_rollup/), [incident_model](../../src/lib/incident_model/) | Old operational vocabulary; removal candidates unless a narrowly justified independent utility is extracted. |
| [http](../../src/lib/http/), [adapter](../../src/lib/adapter/), [introspection_preview](../../src/lib/introspection_preview/) | Outside first-slice requirements. Review references before removing with old external/introspection surfaces. |
| [engine](../../src/engine/) and [migrations](../../migrations/) | Old planning, workbench, claims, routing, external sync, host queue and workflow domains retire in stages. The new trusted Lua/kernel split is not a rename of this tree. |

Dependencies are pinned in [dependencies.cmake](../../cmake/dependencies.cmake),
with sources cached under [vendor](../../vendor/). The existing declarations
include Catch2, SQLite, Glaze, CLI11, curl, spdlog, Lua, tree-sitter,
tree-sitter-zig and FTXUI. Catch2 and Lua are strong reuse candidates; Glaze,
CLI11 and spdlog need a concrete replacement use and boundary review. SQLite is
unneeded by render; later ledger requirements do not imply retaining the old
schema. Curl/external adapters, tree-sitter/Zig closure extraction and FTXUI/TUI
are removal candidates with their consumers. No xxHash package declaration was
found in this file; adding native hashing is later scoped work. Preserve pinned,
committed sources and offline configuration for dependencies that remain.

## Lua binding and isolation evidence

Lua **5.5.0** is already pinned and compiled as `lua_static`, alias `lua::lua`,
in [dependencies.cmake](../../cmake/dependencies.cmake). The standalone Lua and
luac mains are excluded. [engine_execute wiring](../../src/engine/execute/CMakeLists.txt)
links Lua and Glaze. Its exported [execute.cppm](../../src/engine/execute/execute.cppm)
describes `run_config`, `run_workflow`, surface introspection and host contracts.

[host.cpp](../../src/engine/execute/host.cpp) contains `guarded` for C++ exception
to Lua-error conversion, `open_sandboxed_libs`, `install_host_surface`,
`open_confined_parent`, `lua_state_handle` and `run_workflow`.
[manifest.cpp](../../src/engine/execute/manifest.cpp) defines the old host allowlist.
This host can shell legacy CLIs, run Git and write files; the new declaration
loader must not inherit those capabilities. Its use of `luaL_newstate` and
`luaL_loadbufferx` is not evidence of the replacement memory/instruction limits
or text-only loading policy. Curated libraries alone do not establish immutable
trusted tables, captured input consistency or confined read semantics.

Reuse the exception/lifetime and confined-path code only after independent
inspection and native adversarial tests. The new loader must prove its own
memory/instruction/input/depth limits, forbidden capability failures, bytecode
refusal, table integrity, traversal/symlink confinement and deterministic capture.
Existing [surface.t.cpp](../../src/engine/execute/surface.t.cpp),
[manifest.t.cpp](../../src/engine/execute/manifest.t.cpp) and
[workflow.t.cpp](../../src/engine/execute/workflow.t.cpp) are useful test-design
evidence; they pin the old workflow surface, not the new loader contract.

## Verification surfaces

[Makefile](../../Makefile) currently defines `build`, `test`, `test-all`,
`ctest-registry-check`, `cpp-lint`, `fmt`, `cli-usage-check` and `surface-lint`.
`test-all` composes old unit, coverage, usage/surface, exit-code, eval and C++
gates. It is not the replacement full gate. R04 defines replacement commands and
coverage; R09 runs the integrated gate. Run checks directly and serialize access
to each build directory; no legacy queue is used for this rewrite.

[Module helpers](../../cmake/module.cmake) discover colocated `*.t.cpp` Catch2
cases and keep test executables out of default `all` with
`planar_mark_test_binary`; `planar_tests` is their aggregate. Preserve nonzero
selected counts and explicit skip accounting. The current
[registry checker](../../scripts/ctest-registry-check.sh) is a reuse candidate.
[parity_harness.hpp](../../src/cmd/parity_harness.hpp) provides `make_arena` and
`run_pinned` for black-box commands. Review its database/setup assumptions before
using it for render acceptance.

Command tests include colocated parity/capability cases, multi-command
[integration tests](../../src/cmd/integration_tests/) and
[execute goldens](../../src/cmd/planar-execute/golden.t.cpp).
[Tools](../../src/tools/CMakeLists.txt) register CLI usage/docs coverage, surface
lint and queue contention tooling plus a Python renderer test. [Scripts](../../scripts/)
include formatter/Doxygen, architecture, installation and release checks;
[evals](../../evals/README.md) and [CI](../../.github/workflows/) encode old product
behavior. Retain generic mechanisms after review and replace obsolete assertions
with the new render contract rather than relabeling old passes as new evidence.

The Makefile's `run` invokes the source binary against real data; `smoke` sets
`PLANAR_DB` but does not itself isolate `HOME`. Any scoped legacy invocation must
set both to scratch locations explicitly. Never install or run against operator
data to validate this rewrite. See the active guide for the database guard.

## Installation, release and authored surfaces

| Existing surface | Citation and retirement consideration |
| --- | --- |
| Installer and prerequisites | [install.sh](../../install.sh), [INSTALL.md](../../INSTALL.md): builds or accepts prebuilt binaries, probes/migrates the old database, stages managed trees and places vendor surfaces. Do not invoke for this rewrite. |
| Managed state and cleanup | [install-lib](../../scripts/install-lib/), especially [data-paths.sh](../../scripts/install-lib/data-paths.sh), [db-probe.sh](../../scripts/install-lib/db-probe.sh), [install-cleanup.txt](../../install-cleanup.txt) | Old install transactions and data ownership need reference review. Removal from this branch must never be confused with deleting operator data. |
| Release/bootstrap/uninstall | [get-planar.sh](../../scripts/get-planar.sh), [uninstall.sh](../../scripts/uninstall.sh), [release-publish.sh](../../scripts/release-publish.sh), [release gates](../../scripts/release-gates.sh), Makefile `dist`/`release-cut`, CI release workflow | Retire or explicitly disable old distribution paths by R09 so no runnable installer ships the wrong product or promises an upgrade. Replacement distribution is deferred. |
| Operational authorship | [agents](../../agents/), [models.md](../../agents/models.md), [skills/planar](../../skills/planar/), [workflows](../../workflows/) | Installed agents/skills and Lua command policy belong to the old product. Their model table and claim/queue rituals do not override the approved fresh-agent protocol. Remove old surfaces by the render gate. |
| Vendor packaging | [render-codex-agents.py](../../scripts/render-codex-agents.py), installer vendor placement, [templates](../../templates/) | Inventory references before removal. Do not generate new vendor copies during R01. |
| Public references | [README](../../README.md), [architecture](../architecture.md), [CLI reference](../cli-reference.md), [testing](../testing.md), [skill reference](../skill-reference.md) | Still legacy product documentation. Refresh affected references as implementation changes; finish coherent replacement documentation by R09. |

## Historical fixture availability

The [v4 design](Planar%20design%20spec%20-%20v4.md) links
`planar-v4-fixture/README.md` beside itself (the fixture discussion near line 598
at this baseline). That path is absent in this checkout. The R01 search agent
also searched for a directory named `planar-v4-fixture` under
`<home>/projects/github`, bounded to depth six, and found no copy, as recorded
in [R01](rounds/R01.md). This is a bounded local absence finding; it does not
establish absence outside that root/depth or at a remote location. No historical
fixture trace has been executed or accepted as verification.

R03 will author `fixtures/render/quota/` from the approved behavior after R02
fixes the contracts, with provenance labeled as new expected examples. The v4
fixture description mentions a fake identity and pure-Lua reference renderer;
neither can supply native hash vectors or prove the new native isolation boundary.
