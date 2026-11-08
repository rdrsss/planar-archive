# Planar

Planar compiles restricted Lua declarations and captured observations into scoped
worker and reviewer context. This checkout builds one `planar` binary with rendering,
ledger loading and history, and draft execution commands for
run ownership, recovery, finite checks and reviewer verdicts. The separate authoring
bundle includes authoring helpers and live orchestrator prompts. Render remains
database-free. Execution uses an explicit ledger and driver-supplied checkout;
Planar does not call models, schedule sessions or manage worktrees.

The execution implementation draft is integrated on `dev/planar-v2-vernacular`.
It is an unreviewed draft: development compilation and focused checks do not
constitute the final validation gate. The operator requested implementation
completion before review and fixing rounds. See the [roadmap](docs/vernacular/roadmap.md)
for current status and the [CLI reference](docs/cli-reference.md) for the command surface.

[V5](docs/vernacular/planar-design-spec-v5.md) is the product-direction baseline.
Exact contracts and explicit operator decisions govern current APIs; v5 examples
do not add commands. The [alignment assessment](docs/vernacular/v5-alignment.md)
tracks the relationship between the design and implementation.

This is a breaking rewrite. There is no legacy database migration, upgrade path
or compatibility CLI. Git preserves the previous implementation. No installer
or release bundle is available for this replacement.

## Build and render

Use CMake 4.3/4.4, Ninja, LLVM 23+ with modules-enabled libc++, Python 3 and
Doxygen for the full gate. See [toolchain details](docs/toolchain-parity.md).
CLI11 2.7.2, Glaze 8.1.0, Catch2, Lua 5.5.0, SQLite 3.53.3 and xxHash 0.8.3 sources are committed under `vendor/`;
configuration does not fetch dependencies.

```sh
make build
build/rewrite-debug/bin/planar --help
build/rewrite-debug/bin/planar render \
  --workspace fixtures/render/quota/workspace/workspace.lua \
  --registry fixtures/render/quota/registry/quota-tools.json \
  --root source=fixtures/render/quota/current \
  --capture-outputs --base-root source=fixtures/render/quota/base \
  --job combined --role builder
```

The command emits one canonical JSON envelope containing Markdown context and an
identity manifest. Without `--capture-outputs` and `--base-root`, it previews
requirements without scanning source/base files or computing output identities.
Inputs are explicit; no home configuration or legacy database is consulted.

Render does not execute registry commands, verify driver assertions, activate
runs, record completion or approval, call models, schedule sessions, manage
worktrees or persist state. The execution commands provide finite run, check and verdict operations; an
external driver owns sessions and scheduling.
Capture detects observed changes under a cooperative stable-input contract;
it is not an atomic OS snapshot.

## References and verification

Product C++ and trusted Lua live under `src/`; build/integrity tooling and
verification inputs live outside. The quota fixture models an external workspace
with source snapshots, observations and expected packets.

- [CLI reference](docs/cli-reference.md) and [declaration reference](docs/declarations.md).
- [Architecture](docs/architecture.md), [testing](docs/testing.md), and [distribution status](INSTALL.md).
- [Quota fixture](fixtures/render/quota/README.md) and its [computed native packets](fixtures/render/quota/native/README.md).
- [Acceptance traceability](docs/vernacular/acceptance-matrix.md).
- [Implementation plan](docs/vernacular/implementation.md), [roadmap](docs/vernacular/roadmap.md), and [contributor guide](CONTRIBUTING.md).

Run `make test-all` and `make test-all PRESET=release` serially for the complete
local gates. Replacement evidence is currently from macOS Apple Silicon; no
replacement Linux CI result is claimed. Retired product CI workflows have been
removed; local gates are authoritative until replacement CI is validated.

Licensed under [MIT](LICENSE); see [third-party notices](THIRD_PARTY_NOTICES.md).
