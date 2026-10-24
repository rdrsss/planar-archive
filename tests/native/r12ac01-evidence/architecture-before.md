# Render architecture

The replacement builds one thin `planar` command over a small native engine and
an embedded trusted Lua standard library. There is no legacy engine or database,
command fleet or operational agent workflow in the source tree.

| Layer | Source | Responsibility |
| --- | --- | --- |
| Command | `src/cmd/planar/` | Parse explicit arguments, resolve host inputs, compose capture/render, buffer canonical output and check process writes. |
| Values and declaration loading | `src/engine/value/`, `src/engine/lua/` | Immutable typed values, text-only restricted constructors, bounded protected Lua evaluation. |
| Capture | `src/engine/capture/` | Confined no-follow reads, logical provenance, stable retained bytes and directory topology, final verification. |
| Identity | `src/engine/canonical/`, `src/engine/hash/` | Typed canonical_v1 binary encoding, domain-separated seed-zero XXH3-128. |
| Trusted runtime | `src/engine/runtime/` | Fresh separate Lua state, bounded typed bridge, native structural/hash capabilities. |
| Transport | `src/engine/json_codec/` | Pinned Glaze parsing and buffered canonical UTF-8 JSON with a 16 MiB payload ceiling. |
| Internal storage primitive | `src/engine/ledger/` | Explicit trusted SQLite connections, owned prepared statements and rollback guards; no format opener or CLI wiring. |
| Product policy | `src/stdlib/planar/` | Schema validation, semantic resolution, selection, observations, Markdown projection, budgeting and render orchestration. |

First-party product implementation lives under `src/`, including native C++ and
trusted Lua; future product workflows follow the same rule. Build and integrity
tooling may live in `cmake/` and `scripts/`. Fixtures remain outside `src/`: their
sample external workspaces, current/base source snapshots, observations and
expected packets are verification inputs, not runtime implementation. Test-only
Lua and bridge harnesses remain in `tests/stdlib/`.

Trusted Lua owns schemas, shapes, selection, materiality and identity recipes.
The engine owns isolation, captured filesystem access, canonical encoding,
hashing and output. `cmake/embed_lua.cmake` embeds exact configured stdlib bytes;
the runtime does not load product Lua from host paths. Test-only bridge exports
are independently embedded outside the product graph.

Declaration states expose only curated immutable constructors, no standard
libraries or engine access. The trusted state receives independent copies and
selected deterministic base/table/string/math/utf8 functions, with no I/O,
process, dynamic loading, clocks or randomness. Native conversion/setup/callbacks
use protected Lua boundaries so longjmp does not cross live C++ destructors.
The [bridge reference](../src/stdlib/planar/README.md) details its allowlist and bounds.

Capture retains explicit workspace/spec/jobs, text references, registry and
observations. With output capture it scans current/base trees, including
untracked/ignored files except declared exclusions. Discovery and final
verification check file bytes, normalized executable mode, identity and
membership; directory topology is retained separately from file provenance.
Reads use retained root descriptors and no-follow relative opens. The driver
must keep inputs stable: this cooperative contract does not freeze the OS or
defend against every concurrent change-and-revert. Downstream Lua consumes only
sealed captured values and never recaptures files.

Work fingerprints exclude optional prose, checkpoints, assertions and source
output bytes. Output identities remain per unit regardless of job grouping;
reviewer packets carry exact captured target vectors. Optional context is
omitted deterministically under budget; mandatory overflow refuses explicitly.
No hash substitutes for exact comparison at a boundary that requires it.

The C++26 module graph is declared by `src/engine/CMakeLists.txt` and
`src/cmd/planar/CMakeLists.txt`, using `cmake/rewrite.cmake`. Dependencies point
downward. Only pinned committed CLI11, Lua, Glaze, xxHash, SQLite and test-only Catch2 are retained.
Configuration performs no dependency download. Test executables remain outside
`all`; there are no product install rules or live Git version metadata inputs.

An external driver owns sessions and scheduling. Render executes no checks,
verifies no driver assertions, persists no state and establishes no completion
or approval. Later ledger/execution work is gated on operator evaluation of
actual packets. See [contracts](declarations.md), [CLI](cli-reference.md),
[testing](testing.md) and the [implementation plan](vernacular/implementation.md).

The trusted bridge alone exposes `engine.json_decode` and `engine.json_encode`.
Glaze owns all JSON grammar, unescaping and escaping; Lua applies schema policy.
Checkpoint byte accounting, Markdown descriptors and machine transport use the
same native codec. Binary canonical_v1 encoding and hash domains are unchanged.
Codec allocations are separate from Lua's allocator: each encode uses at most
96 MiB + 1 KiB scratch plus bounded structural copies and its caller-owned input.
Cumulative native bridge conversion retains its 512 MiB accounting ceiling.

`main.cpp` performs checked process writes. `bootstrap` creates the CLI11 root,
attaches `handlers/render/command.cppm` and dispatches its parsed command. The
render handler owns option bindings through parsing and execution, host path
resolution, capture routing and trusted render composition. The shared response
module sits below both bootstrap and handlers, avoiding a dependency cycle.
CLI11 owns verbs, option recognition, required counts and duplicate handling.
Planar retains semantic validation, fixed help and Glaze diagnostics. A narrow
preflight rejects long tokens containing `=`; parsed CLI11 metadata accounts
for separate value tokens and rejects discarded positional terminators.

The current Markdown presentation is `projection_v2`: labeled check/dependency
policy, fenced exact argument/environment arrays, and full ordered reviewer
targets. Global capture and per-file content identities remain in the manifest;
Markdown retains the work fingerprint and relevant output identities. This keeps
an unselected unit edit from changing an otherwise identical worker fragment.
Work selection, canonical identity domains and capture semantics are unchanged.

The standalone `planar.engine.ledger.sqlite` module restores SQLite 3.53.3 from
exact committed amalgamation bytes (public domain). Its offline static target
compiles only `sqlite3.c`, with `SQLITE_THREADSAFE=1`, `SQLITE_DQS=0` and
`SQLITE_DEFAULT_FOREIGN_KEYS=1`: serialized library internals, no double-quoted
string literals and foreign keys enabled by default. URI opening is not enabled.
The wrapper and every related statement/transaction are confined to one caller
thread. Statements and transaction guards retain the connection; result columns
are owned copies. Busy waits default to 5000 ms and cannot exceed that setting.
Manual transaction/savepoint SQL is rejected through SQLite's authorizer, and
prepare/bind/step failures roll back an active transaction. A failed commit is
always a failure; failed rollback poisons the connection against further use.

This module is an internal trusted SQL primitive, currently unreferenced by
render, Lua or CLI handlers. Explicit read-only/read-write modes do not create
missing files. `scratch_create` uses SQLite CREATE on a caller-owned staging or
scratch path; it is not exclusive creation or safe publication. No default paths,
parent creation, schema recognition, format verification, migrations, backup or
ledger acceptance are implemented here. R12-B owns the format-aware opener and
initializer; R12-C owns backup and broader storage/crash verification. Opening an
arbitrary file through this primitive does not establish ledger validity.
