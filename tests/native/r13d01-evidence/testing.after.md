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

The current registry contains **226 tests: 225 native cases and one Python
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

R12-A with correction R12-A-C01 adds thirteen standalone SQLite cases under `replacement.ledger`, each using
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

R12-A-C01 adds native filename persistence for bare `:memory:`, explicit relative
and absolute spellings, literal URI-looking names, existing-mode noncreation,
empty/NUL refusal, missing-parent/no-follow checks and typed unavailable-cwd
failure. Its red/green evidence is under `tests/native/r12ac01-evidence/`.
The preserved initial probe demonstrates SQLite collapsing `missing/../db`;
the correction checks original parent traversal before opening. Regressions
refuse missing and symlink ancestors followed by `..`, while an existing ordinary
directory followed by `..` succeeds. These checks do not certify hostile race safety.


R12-B adds thirteen native format cases to the same focused ledger selection
(26 ledger cases including R12-A/C01). Scratch-only coverage checks metadata,
seven STRICT tables, explicit index and thirteen triggers, typed Glaze event
payload and current state; read-only/writable reopen; byte-and-directory snapshots
across foreign/empty/truncated/old/future/wrong-application/schema/index/trigger,
malformed-btree, FK/projection/event/metadata refusals; explicit unsupported
populated state; all thirteen immutable trigger mutations; no-replace targets,
original-component traversal, no-follow paths, literal `:memory:` filenames and
sidecar recovery refusal. An abandoned schema/data transaction verifies rollback
without changing the initialized database. No product API exposes generic SQL;
raw trusted bindings create test corruptions and inspect SQLite metadata only.

Raw R12-B debug configure/build/focused CTest, formatting/Doxygen and no-work
logs, candidate/evidence manifests and protection audit are under
`tests/native/r12b-evidence/`. A focused pass is not the full R12 milestone:
backup/restore, VFS/disk-full injection, process death, audited hot-journal
recovery and integrated debug/release gates remain R12-C. Full record/history
semantic validation remains R13/R14. macOS is the executed platform; Linux
portability is implemented but not established by this evidence.

R12-B-C01 adds two format cases (28 focused ledger cases): initialization refuses
all journal/WAL/SHM suffixes with regular, symlink and directory entries, preserving
the complete sibling snapshot; all three metadata TEXT fields reject embedded-NUL
suffixes and multibyte values beyond their encoded byte limits without mutation.
Schema and metadata guards use SQLite `octet_length` before owned native text copies.
`tests/native/r12bc01-evidence/` retains the nine-case sidecar RED and a standalone
1 MiB C++ allocation-budget probe: the original opener requests over 2 MiB for each
NUL-suffixed metadata value; the correction refuses all six fixtures with no C++
request larger than 768 bytes on the tested macOS debug build. This bounds the
observed native copy path, not SQLite's own C allocations or total process memory.
The publication-time sidecar recheck is verified by inspection; deterministic
filesystem fault injection and integrated gates remain R12-C.


R12-C1 adds seven focused native cases (35 ledger cases total). They exercise
SQLite online backup with a pinned source snapshot and invalid-owner/transaction
refusals; new-target backup/restore with exact metadata and supported state;
existing regular/link/directory targets and sidecars; and invalid source refusal
before target creation. A forked child syncs a real SQLite rollback journal and
flushes dirty database pages, signals readiness through a pipe, then is killed
with SIGKILL. Ordinary opening refuses the pair unchanged; explicit quiescent
recovery restores initialized state. A second fixture retains that genuine journal
with reconstructed prior main bytes to cover redundant rollback; it models rather
than directly intercepts the before-first-main-write crash boundary.

The recovery refusal matrix checks wrong schema despite matching application and
version, future/foreign headers, unsupported populated state, malformed/truncated
journal headers and dimensions, corrupt recovered event pages, unsafe journal
entries and competing WAL/SHM. Refusals compare exact main/sidecar/directory bytes.
The initial corruption fixture damaged a journal-covered main page; successful
SQLite rollback correctly repaired it. Its raw failing test log is retained, and
the corrected fixture damages an unrelated event page that remains corrupt after
rollback. These tests do not authenticate arbitrary malicious journal changes.
Raw commands/logs, snapshots and manifests are under `tests/native/r12c1-evidence/`.
R12-C1-C01 adds one super-journal footer refusal case (36 ledger cases total).
Its existing regression variants verify typed refusal and unchanged main,
journal and external entries, including malformed footer fields. The fixed
eight-byte footer guard runs before scratch or SQLite recovery. The pre-fix
failure and corrected focused checks are under `tests/native/r12c1c01-evidence/`.
C2 retains the broader injected syscall/VFS I/O, actual full storage, concurrency,
transaction-boundary termination matrix and integrated debug/release gates.


R12-C2 adds ten focused cases (46 ledger cases total) for storage reliability.
`reliability.t.cpp` uses a scoped test-process SQLite VFS over the native default
VFS; it restores the original default after each case. The matrix injects
`SQLITE_IOERR_WRITE` and `SQLITE_IOERR_FSYNC` on main and rollback-journal paths,
checks typed commit/backup failure, complete old state on the same connection
and fresh reopen, and a subsequent complete committed state. A real
`SQLITE_FULL` from `max_page_count` checks retained rows and rollback; it is
configured SQLite capacity exhaustion, not physical host-disk exhaustion.

Pipe barriers bracket commit before invocation, inside actual journal and main
`xSync` callbacks, and after successful return. Separate processes receive
SIGKILL at each boundary; exact wait statuses and complete old/new symbolic
storage rows are recorded. These rows test storage atomicity, not R14 history
semantics. A separate writer process produces a bounded busy refusal, then
rolls back and exits normally before retry. A separate reader makes commit busy;
rollback and subsequent retry preserve complete state. Barriers and child cleanup
are bounded; the tests use only canonical temporary roots.

`publication_fault.t.cpp` runs an isolated generated format module for
initialization, backup and restore. The generator reads the current production
source, changes only module/namespace identity, the SQLite namespace import,
the test-shim import and three POSIX call targets (`fsync`, `linkat`, `unlinkat`),
and proves byte-identical reverse substitution. It records source/generated
hashes and replacement counts. Product binaries link the original module only;
both original-module cases and the generated variant run in the focused suite.
Faults before publication leave no target. Parent sync and staging unlink/rmdir
failures after publication return explicit uncertainty and retain a complete
verified target. A sidecar appearing before the late recheck is preserved and
refused; a target arriving at `linkat` is preserved byte-for-byte.

SQLite SQL `ROLLBACK` does not propagate every underlying rollback I/O error:
the pinned `sqlite3RollbackAll` discards the Btree rollback return code. The
observed main rollback write fault returns SQL success, recovers complete old
state on the same owner and fresh reopen, and permits a normal committed retry.
This case verifies those safety properties; it does not claim coverage of the
wrapper's poison branch, which requires a failure reported by SQLite. Exploratory
journal-read/poison expectations and their failed logs are explicitly labeled in
`tests/native/r12c2-evidence/README.md`; they are not product defect evidence.
Likewise `sqlite3_backup_finish` returns the stored step result (DONE maps to OK)
in this pinned implementation, so destination faults exercise step failure plus
mandatory finish cleanup, not an independent finish-after-DONE failure.

R12-C2 raw focused fault traces, child statuses, complete debug/release gates,
mechanical-generation manifest and source/protection audits are retained under
`tests/native/r12c2-evidence/`. macOS is the executed platform; Linux execution,
persistent/repeated device failure and physical disk exhaustion are not claimed.

R13-A adds three canonical decoding cases and nine record codec cases. Focused
selection after building `all planar_tests` is:

```sh
ctest --test-dir build/rewrite-debug -R 'replacement.(canonical|hash|record)' --output-on-failure
python3 tests/vectors/records/generate.py --check
```

The selection contains 19 native cases (seven canonical, three hash and nine
record). Ten independent standard-library-framed record fixtures check exact
stored bytes and portable raw XXH3-128 digests against a separately compiled
pinned upstream reference. Canonical fixtures cover binary strings, signed-32
extremes, empty containers, UTF-8 key ordering, strict malformed/truncated input
and depth/node/byte limits. Record tests cover binary/CRLF/executable files,
child-ID storage versus portable hashes, local metadata/name/alias/ID validation,
unknown payload codecs, exact request arguments, malformed lengths/counts, all
proper truncations of the fixed vectors, exact 16 MiB file/value limits and the
64 MiB whole-record boundary. Large claimed lengths are refused without creating
the claimed buffers. Tests establish framing and local validation; graph integrity,
collision-safe storage and accepted history remain later R13/R14 scopes.

`tests/native/r13a-evidence/` retains raw focused checks, renderer CLI acceptance,
formatting/Doxygen, no-work evidence and exact source snapshots/manifests. The
initial two test failures are preserved as fixture-offset mistakes, corrected
without changing the corresponding codec behavior. Full integrated debug/release
gates remain the R13 milestone; the focused host evidence is macOS only.


R13-B1 adds typed record storage coverage under the same ledger selection:
all four kinds, binary/CRLF/mode payloads, exact reuse, ordered directory edges,
owned snapshot reads, missing/invalid IDs, independent readers, lock-blocked
commit rollback, invalid-operation and abandoned-prefix rollback, signed-ID
exhaustion, malformed record and graph refusal without mutation, unvalidated
revision refusal, and record-only backup/restore plus genuine killed-process
hot-journal recovery. The exact schema and earlier foundation refusal tests stay
active; the old malformed raw record now returns `corrupt` rather than an
unsupported-history placeholder.

`tests/native/r13b1-support/` mechanically generates a separate format module
that replaces digest calls with a kind-specific constant while importing the
original record types/encoder. It exercises file, directory, value and request
collisions, exact local directory child-ID equality, prefix rollback and ambiguous
buckets containing both exact and unequal members. Its manifest records every
substitution and proves reversibility. A separate verification mutation removes
only the interning exact-byte comparison; failing collision regressions establish
test sensitivity. The mutation is never applied to product source. Both the
protected generated variant and the ordinary product run green afterward.

R13-B1 raw checks, mutation evidence, generation audits, protected-file hashes and
exact candidate snapshots live in `tests/native/r13b1-evidence/`. Run focused
`ctest --test-dir build/rewrite-debug -R 'replacement.(ledger|canonical|hash|record)'
--output-on-failure`, both vector generators with `--check`, active formatting/
Doxygen and supplementary helper lint, then a no-change build. Complete tree
construction/reconstruction remains R13-B2; acceptance remains R14. This focused
host evidence is macOS only; full integrated debug/release gates follow R13.

R13-B1-C01 adds a permanent real SQLite VFS read-fault regression in
`src/engine/ledger/snapshot.t.cpp`. It requires an actual `SQLITE_IOERR_READ`,
the original typed `io` failure, and an independent successful writer commit
while the failed reader stays alive. That reader refuses old and new records
after moves; cached metadata survives, and an explicit fresh reader sees the
new record. The paired fault case verifies writer prefix rollback. Ordinary
invalid-ID/not-found failures also invalidate future reads through move assignment
without losing cached metadata. The focused selection now contains 79 cases
with zero skips. RED/GREEN logs, audits and corrected candidate snapshots are in
`tests/native/r13b1c01-evidence/`; previous B1 and review evidence is preserved.

R13-B2 adds complete-tree construction and owned reconstruction checks under
`replacement.tree`, plus explicit idempotent writer abort under
`replacement.ledger`. Focused verification includes the existing record,
canonical, hash, storage and forced-collision cases:

```sh
ctest --test-dir build/rewrite-debug -R 'replacement.(ledger|canonical|hash|record|tree)' --output-on-failure
```

Tree cases cover binary/CRLF/empty/executable bytes, portable digests despite
local ID displacement, sorted topology, immutable older trees, rename/delete/
same-length edits, composition, explicit abort, abandonment and refusal after
prior pending inserts. They exercise exact and excessive payload/file/path/depth/
source-directory bounds, independent provenance directory bounds, and shared
subtree expansion (including 258,049 directory occurrences at the provenance
maximum). Typed storage propagation seeds a valid record one below signed-64 exhaustion,
allows a new file at the maximum ID, then refuses its directory and verifies
rollback of that file,
without a product hash override. Malformed root kinds/digests/references,
non-tree children, aliases and empty non-root directories refuse. Captured views
contain no timestamps, host metadata or filesystem capabilities. Evidence lives
under `tests/native/r13b2-evidence/`; release/Linux and full integrated R13 gates
remain milestone work.

R13-C1 adds six provenance cases to the focused
`replacement.(ledger|canonical|hash|record|tree|provenance)` selection. Generated
original names/bytes are compared against all eight authored stdlib files, and
the exact pre-change ordered concatenation plus LF after each part is compared
to `source()`. Tests cover binary registry bytes/modes, canonical historical
verification without Lua, original/order/separator/boundary identities, reserved
aliases, topology/mode/manifest refusals and whole-set synthetic-inclusive count
and byte limits. Existing tree success helpers compare pure digests to interned
roots, including displaced database-local IDs; all B2 boundary/rollback cases
remain active. C1 raw logs, exact candidate/protection and generation manifests
live under `tests/native/r13c1-evidence/`. This focused macOS debug gate is not
the integrated R13 debug/release milestone or Linux validation.

R13-C2 adds `replacement.interpretation` cases for the complete stored
`interpretation_v1` envelope. Reviewed full JSON and byte-exact canonical
fixtures under `tests/native/r13c2-fixtures/` pin both quota job groupings,
referenced text, complete registry, semantic dependencies/assignments/effective
checks/graphs and the exact versions map. Tests independently compare all
existing normalization and validation fields, reverse routing/capture order,
preserve absent optional notes, and check internal stdlib provenance. The
provenance inventory now contains nine original Lua files plus two synthetics;
this deliberate addition changes code identity without changing render output.

Historical checks cover malformed/extra/missing/wrong-type envelopes and version
fields, unknown recorded versions, digest spelling and an older differing valid
digest, container kinds, paired sorted jobs, canonical byte stability, signed-32
limits and the inclusive 16 MiB interpretation boundary. The real record-storage
roundtrip loads saved canonical fixtures, stores/reopens them and decodes without
any Lua/capture call. Typed live schema/ownership/unsupported diagnostics retain
logical locations; native runtime/resource refusals remain separate.

Focused commands select
`replacement.(ledger|canonical|hash|record|tree|provenance|interpretation)`, plus
CLI acceptance, both vector generators, source formatting/Doxygen and exact
generated-variant audits. Complete frozen-candidate debug/release commands are
`make test-all` followed serially by `make test-all PRESET=release`, requiring
nonzero counts, zero skips and zero-step no-change builds. C2 raw logs, reviewed
fixtures, exact candidate/protection/generation identities and milestone results
live in `tests/native/r13c2-evidence/`. These are host macOS checks; Linux execution
remains unverified. No acceptance/history/export CLI behavior is claimed.
