# Contributing

This is the breaking Planar rewrite on `dev/planar-v2-vernacular`.
There is no legacy migration, upgrade path or compatibility CLI. The active
build emits one binary with `render`, `init`, `history` and `export`; distribution
is deferred. Render is database-free. The other commands use explicit new-format
ledgers; native acceptance exists, but no load service or CLI is exposed. Load
membership is declaration-closure-only; load service/CLI and full R14 acceptance
remain pending.

Read [the agent guide](CLAUDE.md), [implementation plan](docs/vernacular/implementation.md),
[roadmap](docs/vernacular/roadmap.md), and assigned round before changing code.
[V5](docs/vernacular/planar-design-spec-v5.md) supplies product direction; explicit
operator decisions and approved exact contracts retain authority until scoped
reviewed amendments. Read [alignment](docs/vernacular/v5-alignment.md) and
[acceptance traceability](docs/vernacular/acceptance-matrix.md). V5-02 must resolve
command/Lua boundaries, extension trust and workspace naming before more load
implementation. RA01 authoring/observe follows R14; R15–R19 deliver execution.
Resolve unspecified product choices before implementing dependent behavior.

Planning, decisions and raw verification live in `docs/vernacular/` and round
specific evidence directories. Do not invoke legacy Planar tools for planning,
claims, heartbeats, queues or finalization. Use fresh sequential search
(`gpt-6-luna`, high), coder (`gpt-6.1-sol`, medium) and independent reviewer
(`gpt-6.1-sol`, high) agents. Assign explicit ownership in the shared checkout;
preserve others' work. Coders never approve their own changes. Record candidate
identities, commands, actual counts, results and limits; requested changes use a
fresh corrective round. The coordinator advances reviewed unchanged candidates.

Keep first-party product C++ and Lua under `src/`, including future product
workflows. Build/integrity tooling, tests and fixtures may live outside; fixture
workspaces, source snapshots, observations and expected packets are verification
inputs. Test-only Lua and bridge harnesses stay in `tests/stdlib/`.
Use C++26 modules, explicit error boundaries, pinned committed dependencies and
the engineering conventions in the guide. Keep product references current.
Run `make test-all` and `make test-all PRESET=release` directly, serially per
build directory. See [testing](docs/testing.md) for coverage and limitations.
Instruction-only edits need link/whitespace/consistency checks, not a product build.

Operator evaluation of actual rendered packets was completed before persistence
work began; the native ledger/history foundation is now implemented.
Rewrite authorization does not permit changing operator installations/data,
installing, publishing or merging. No installer or release script is retained.
Historical legacy executables can migrate databases automatically; if an
explicitly authorized historical test needs one, isolate both `HOME` and
`PLANAR_DB` in scratch paths and select that binary explicitly.

All contributions use the [MIT license](LICENSE) and must be signed off under
the [Developer Certificate of Origin 1.1](https://developercertificate.org).
Every authorized commit needs `Signed-off-by`; `git commit -s` adds it.
