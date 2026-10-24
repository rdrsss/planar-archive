# Build and verification

Use CMake 4.3/4.4, Ninja, LLVM 23+ with modules-enabled libc++, Python 3 and
Doxygen. See [toolchain details](toolchain-parity.md). Committed CLI11, Catch2, Lua, Glaze, SQLite and
xxHash sources configure offline; missing sources are a hard error.

Run commands from the repository root, serially per build directory:

```sh
make build
make test
make test-all
make test-all PRESET=release
```

Debug builds use `build/rewrite-debug`; release (`RelWithDebInfo`) uses
`build/rewrite-release`. Product output is `bin/planar` inside each build tree.
`make build` builds `all`, excluding test executables and Catch2. `make test`
builds `all planar_tests`, then runs every registered CTest case. `make test-all`
also checks pinned `clang-format --dry-run --Werror` and Doxygen warnings over
active first-party compilation sources, then requires the second build to say
`ninja: no work to do.` Zero selected tests, count mismatches and skipped tests
fail the gate. `make cpp-lint` runs the active formatting/documentation checks;
clang-tidy is advisory. No queue or installed binary is involved.

Direct equivalents for the complete debug gate:

```sh
cmake --preset debug
python3 scripts/rewrite-check.py gate --build-dir build/rewrite-debug
```

`cmake/rewrite.cmake` explicitly registers colocated Catch2 `*.t.cpp` sources.
The Python acceptance case invokes the source-built binary in temporary roots;
it never substitutes an installed command. The gate derives active C++ files
from `compile_commands.json`. Doxygen's bounded wrapper retries only signal
crashes without a diagnostic; actual findings fail immediately.

## Coverage and limits

Native cases cover restricted Lua capabilities, immutable constructors, text/
integer/structural validation, protected C++ lifetimes and resource exhaustion;
confined captured bytes/topology, aliases, links, membership and deterministic
mutation refusals; exact canonical/hash vectors; trusted bridge isolation and
limits; schema/graph/materiality, selection, budget and Markdown projections;
canonical JSON boundaries; and invocation/output handling.

The integrated registry contains **133 tests: 132 native cases and one Python
acceptance case**. Acceptance covers context-only and captured renders, six computed quota goldens,
fresh-process byte stability, scoped material/optional/output edits, assertions,
root routing/refusals and unchanged sentinel state. The complete acceptance suite
runs 247 real CLI invocations, including a 60-check host-path submatrix. Exact counts and host results for the integrated gate
for the earlier 113-test gate are in [R09](vernacular/rounds/R09.md) and its raw evidence under
`tests/native/r09-evidence/`. Prior round evidence remains preserved separately.

The native serializer tests establish the exact 16 MiB payload boundary (LF
excluded). Large-source CLI probes reach the trusted instruction ceiling first,
so they do not establish an end-to-end 16 MiB envelope-boundary success. Capture
is cooperative, not an atomic snapshot or a hostile race guarantee.

The independently encoded hash oracle and published vectors live in
[tests/vectors](../tests/vectors/README.md). Check without modifying vectors:

```sh
python3 tests/vectors/generate.py --check
```

Focused native checks can use `ctest --test-dir build/rewrite-debug -R
'replacement.(loader|capture)' --output-on-failure` after building `planar_tests`.
Always verify a nonzero matched count and zero skips. Full gate evidence is from
macOS Apple Silicon; the replacement Linux gate has not been validated. Legacy
CI/Docker workflows were removed rather than presented as replacement evidence.

`make install`, `make release`, `make dist` and `make run` refuse. No installer,
release or migration belongs to this work. Storage tests use explicit temporary paths. Never run a
historical legacy binary against operator data to validate this candidate.

N03-B exercises the trusted Glaze callbacks, ordinary typed refusals, declaration
capability denial, strict malformed-object regressions, and checkpoint accounting
at 8,192/8,193 bytes using actual library output. The independent Python wire
oracle uses standard-library JSON strings with Glaze hexadecimal casing. Frozen
R10 evidence retains its original bytes; current evaluation belongs under
`tests/native/n03b-evidence/`.

N04 restores CLI11 2.7.2 from its exact committed vendor tree and tests direct
handler registration separately from dispatch. The real-process parser matrix
covers every binding's duplicate, missing, empty and equals forms, required
options, flags, unknown verbs/options, help placement, positional terminators,
budget syntax/range, root mappings and capture/base pairing. Reordered valid
invocations preserve the same packet; the six native goldens remain unchanged.
See `tests/native/n04-evidence/` for the final debug/release gates, parser
regression traces and source/vendor identity audits.

R10-P01 adds three projection cases for complete readable check/argument/environment
policy, stable worker fragments under unrelated output edits, exact ordered
review targets, and dependency/facet/pipeline/question fields. Original R03
prose/source expectations remain checked. The 247-process CLI acceptance compares
the current six `projection_v2` goldens; policy arrays and coverage descriptions
are checked separately rather than requiring obsolete aggregate JSON formatting.
New evaluation evidence belongs under `tests/native/r10p01-evidence/`; historical
R10/N03/N04 evidence remains frozen.

R12-A adds nine standalone SQLite cases under `replacement.ledger`, each using
its own canonical temporary root. They cover exact owned text/BLOB bytes (NUL,
non-UTF-8, empty and NULL), retained/moved connection and statement lifetimes,
commit/abandon rollback, typed UNIQUE and deferred FOREIGN KEY failures,
failed-commit rollback and writer release, bounded two-connection contention,
read-only noncreation, no-follow refusal, and authorizer-enforced transaction
ownership. Focused command after building `all planar_tests`:

```sh
ctest --test-dir build/rewrite-debug -R 'replacement.ledger' --output-on-failure
```

The tests use the new internal primitive only, never legacy or installed Planar
or operator data. R12-A evidence is under `tests/native/r12a-evidence/`. These
cases do not verify ledger schema/format, initialization/publication, backup,
process termination, disk-full/VFS faults, record/tree semantics or acceptance;
those belong to later bounded rounds. Full integrated debug/release gates follow
R12-C. SQLite's no-follow mode also rejects symlinked ancestor paths; macOS test
roots canonicalize `/tmp` before opening.
