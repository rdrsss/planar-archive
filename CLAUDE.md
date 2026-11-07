# Planar rewrite agent guide

`AGENTS.md` is a symlink to this file. Edit this target and preserve the symlink.
These are the active repository instructions for the approved breaking rewrite
on `dev/planar-v2-vernacular`.

**Current status lives in the opening of [the roadmap](docs/vernacular/roadmap.md)**,
with per-round detail under [rounds](docs/vernacular/rounds/) and evidence under
`tests/native/*-evidence/`. This file holds durable instructions only: rounds
update the roadmap opening, never this file.

## Authority and scope

The operator approved replacing Planar completely, with no migration, upgrade
path, compatibility CLI or permanent legacy source tree. Git preserves the
previous implementation. This approval does not authorize deleting installed
binaries or operator data, installing the replacement, publishing or merging it.

Before working, read [the implementation plan](docs/vernacular/implementation.md),
[roadmap](docs/vernacular/roadmap.md), the assigned round and the
[v5 design](docs/vernacular/planar-design-spec-v5.md). Authority, highest first:

1. Explicit operator decisions.
2. Approved exact contracts — [declarations](docs/vernacular/declarations.md),
   [render](docs/vernacular/render-contract.md), [ledger](docs/vernacular/ledger-contract.md),
   [command/workspace](docs/vernacular/command-contract.md),
   [authoring](docs/vernacular/authoring-contract.md),
   [execution](docs/vernacular/execution-contract.md),
   [load](docs/vernacular/load-command-contract.md) and
   [external profile](docs/vernacular/external-profile-contract.md) — until a scoped,
   reviewed amendment.
3. V5 as the product-direction baseline; see [v5 alignment](docs/vernacular/v5-alignment.md)
   and [acceptance traceability](docs/vernacular/acceptance-matrix.md).

Historical v4 is archival. Resolve unspecified product choices with the operator
before implementing dependent behavior.

Product shape: trusted Lua owns schemas, shapes, selection and projections; a
small native engine owns isolation, captured file access, canonical encoding,
hashing, the ledger and output. Current commands are `render`, `init`, `history`,
`export` and `load`; render is database-free. Claims, check execution, model
calls, scheduling and worktree management are outside the current product.

## Planning records

Planning, decisions and evidence live under `docs/vernacular/`: current status in
the [roadmap](docs/vernacular/roadmap.md#current-status), per-round records in
[rounds](docs/vernacular/rounds/) and evidence under `tests/native/*-evidence/`.
Do not use old Planar tools, queues, skills or agent roles for this work;
historical references to them in `docs/vernacular/` are archival. If you were
dispatched as part of a round, your brief defines the files you own; never revert
others' work in this shared checkout. Unresolved product choices go to the
operator.

## Database safety

Never run a from-source legacy binary against the real database. Legacy runtime
code resolves `PLANAR_DB`, falls back to `~/.planar/planar.db`, and can apply
migrations automatically. If a scoped test must invoke legacy code, set both
`PLANAR_DB` and `HOME` to temporary scratch paths and use an explicit binary
path; keep subprocesses in that scratch environment. `PLANAR_HOME` alone does
not isolate the database. Do not use `make run`, which targets real data. Do not
invoke installed Planar binaries as a substitute for testing the candidate.
The replacement must have no dependency on the old database. Do not run the
legacy installer or release commands during rewrite work.

## Build and verification

```sh
make build
make test
make test-all
```

Run builds and tests directly, serialized per build directory; never use the old
host queue. Agree commands in the round brief and report executed commands,
results, matched test counts and limitations. Zero matching tests is not success;
expected unexplained skips are zero. Run focused checks for the changed scope.
An instruction- or documentation-only round does not require a product build.

CMake is at least 4.3; presets use the pinned LLVM and `libc++` with Ninja.
The debug/release presets use `build/rewrite-debug` and `build/rewrite-release`.
Test binaries stay outside default `all`; `planar_tests` builds them. The complete
gate builds, runs a nonzero count with zero skips, checks active source
formatting/Doxygen, and requires a no-change build to do no work. See
[testing](docs/testing.md) for direct equivalents and coverage,
[architecture](docs/architecture.md) for the active module graph and
[toolchain details](docs/toolchain-parity.md) when changing build tooling.

## Engineering conventions

- Keep first-party product C++ and Lua implementation under `src/`, including
  future product workflows. Build and integrity tooling, test-only Lua/harnesses,
  and fixtures may live outside. Fixture workspaces, source snapshots, observations
  and expected packets are verification inputs rather than runtime implementation.
- Use C++26 named modules: `.cppm` interfaces and `.cpp` implementations,
  `import planar.<dotted.path>;`, and `import std;` for the standard library.
  First-party headers are limited to test harnesses under `src/cmd/`.
- Keep dependencies downward: thin command entry points compose lower-level
  modules. Avoid cycles and edges between command binaries. Review the
  engine/Lua boundary explicitly rather than copying the old domain graph.
- Use `snake_case` for functions, variables, namespaces and types; leading
  underscores for private/protected members, none for public struct fields;
  CamelCase template parameters.
- Use `std::expected<T, E>` for expected boundary failures. Wrap library errors
  in the owning module's error vocabulary. Exceptions represent unrecoverable
  conditions. For Lua bindings account for C `longjmp` across C++ destructors.
- Do not use runtime `assert()` for required boundary validation; release builds
  define `NDEBUG`. Use explicit typed failures; `static_assert` remains valid.
- Append formatted text with `out += std::format(...)`. Avoid
  `std::format_to(std::back_inserter(out), ...)`: the pinned `libc++` has a known
  stack-buffer failure for some argument byte lengths divisible by 256.
- Delegate command and option parsing to pinned CLI11 through `import cli11;`.
  Keep entry-point I/O and bootstrap registration/dispatch thin. Command families
  own `handlers/<family>/command.cppm` and `command.cpp`, with CLI11 option state
  alive through parsing and dispatch. Shared response helpers sit below handlers
  and bootstrap; never introduce a handler-to-bootstrap cycle or shadow argv parser.
  Preserve the separate-token CLI contract with only its documented syntax guard
  and checks based on CLI11 parsed metadata. Keep semantic validation, fixed help,
  exit policy and Glaze-encoded diagnostics in Planar.
- Delegate JSON grammar, string escaping and serialization to the pinned Glaze
  library; adapters enforce bounds and structural policy, trusted Lua owns schemas.
  Do not add handwritten product JSON parsers or serializers.
- Avoid global mutable state; use explicit cancellation signals or atomics.
  Keep machine output stable and undecorated. Support macOS and Linux.
- Review reusable libraries and dependencies individually. Existing sandbox,
  process, serialization or hashing code does not prove the new contract.
  Review storage formats before application code when persistence is in scope;
  never reinterpret the legacy database as the new ledger.
- Pin third-party release archives with URL and SHA256 through
  `cmake/dependencies.cmake`, and commit their sources under `vendor/`.
  For new GitHub pins use `codeload.github.com/.../tar.gz/refs/tags/<tag>`.
  Do not add application dependencies through submodules, `GIT_REPOSITORY`,
  `FetchContent` or `find_package`, or depend on private repositories. A public
  clone must configure without network access or tokens. Check references before
  removing dependencies. Installation/runtime prerequisites must match the
  shipped product when distribution is redesigned.
- Keep Catch2 tests in colocated `*.t.cpp` files with per-test temporary roots.
  Test files begin with plain `//` comments. Add meaningful regression tests for
  bugs and realistic workflow tests for new CLI behavior. Demonstrate the failing
  regression before the fix; preserve evidence in the round record.
- Use the reviewed CMake module/test helpers while applicable. Test executables
  must stay outside default `all`. A no-change build should run zero steps.
  Version metadata is opt-in; do not invalidate the graph with live Git metadata.
- Use the pinned formatter. `clang-format --Werror` and Doxygen gate applicable
  C++ changes; `clang-tidy` is advisory. Every `.cppm` opens with `/// @file` and
  `/// @brief` describing purpose, entry points and error boundaries. Document
  exported declarations with summary sentences and useful `@param`/`@return`;
  state invariants and thread safety for shared state.
- Update affected reference documentation with behavior or architecture changes.
  Keep planning and round evidence in `docs/vernacular/`, product references in
  `docs/`, and generic gate helpers in `scripts/`. Do not author generated
  vendor instruction copies in `.claude/`, `.codex/` or `.github/`.
- Make only scoped edits; do not revert unrelated changes. Every commit, when
  authorized, carries `Signed-off-by` (`git commit -s`). Do not install, publish
  or merge without authorization for that action.
