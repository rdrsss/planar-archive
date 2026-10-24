# Distribution status

The replacement supports `render`, `init`, `history` and `export`, with explicit
new-format ledger paths and database-free render. It has no installer, release bundle or supported
upgrade from earlier Planar versions. The previous installer and release
scripts have been removed from this branch. Installed binaries and operator
data are outside this repository cleanup.

Build from source using the [README](README.md) and [toolchain reference](docs/toolchain-parity.md).
The executable is `build/rewrite-debug/bin/planar` (or
`build/rewrite-release/bin/planar`). Invoke that path explicitly.
`make install`, `make release`, `make dist` and `make run` refuse with status 2.
Distribution requires a separate design and authorization.

[V5 direction](docs/vernacular/planar-design-spec-v5.md) does not change distribution
status. [The alignment path](docs/vernacular/v5-alignment.md) completes contracts,
load, authoring and single-root execution before later distribution design.
Workspace scaffolding, if implemented after its contract review, will not provision
or manage checkouts. Current init only creates an explicitly named empty ledger.
