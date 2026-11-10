# Build and verification

Current rewrite status is the [roadmap opening](vernacular/roadmap.md#current-status).
Superseded status checkpoints that formerly opened this file are preserved in
[history](vernacular/history.md#former-opening-of-testingmd).

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

Engine targets and colocated tests are declared in each component's
`src/engine/<component>/CMakeLists.txt`, composed by the engine coordinator.
Their generated sources, libraries and native test executables live under the
matching component directory in the build tree; `runtime/closure_guard` is nested.
Use CTest or CMake target names to select tests instead of assuming the former
flat engine binary directory. Current ledger-format wiring is in
`src/engine/ledger/CMakeLists.txt`; runtime test timeouts are in
`src/engine/runtime/CMakeLists.txt`. Round-specific flat-layout references below
retain their historical meaning. An idle Ninja invocation may first print
`[0/2] Re-checking globbed directories...`; the final `ninja: no work to do.`
confirms no compilation or linking was needed.

`replacement.authoring_execution_lifecycle` runs the source-built production
and separate authoring executables in isolated roots. Its five scenarios cover
same-bundle activation, complete worker/check/reviewer acceptance, recovery with
a genuinely stopped check, cross-bundle refusal and saved-response replay, and
actual external-profile parity. Build `planar_tests` before selecting it with
CTest. `replacement.retained_cli` also changes an execution callable in a compiled
test bundle to detect dispatch that silently creates a different VM. The four
generated command compositions and the authoring stack audit reversible source
renames during configuration.
`replacement.authoring_execution_wiring` checks all twelve retained-VM arguments
and typed handler imports in production and fourteen generated bootstraps. This
structural guard covers VM lifetime even when constructing a second identical
VM would give the same process result.
The two `command embedded binding native budget` cases use a separate compiled
runtime fixture with checked budget preconsumption immediately before binding.
They compare bound/exempt execution thresholds and require insufficient room to
refuse before service callbacks, leaving the owner consumed. The fixture is
excluded from default `all` and included by `planar_tests`; it adds no product
option or runtime hook. The threshold gap includes execution side costs, so it
is paired with source allocation analysis and one/two-pass mutation controls.

`cmake/rewrite.cmake` explicitly registers colocated Catch2 `*.t.cpp` sources.
The Python acceptance suites invoke the source-built binary in temporary roots;
they never substitute an installed command. The gate derives active C++ files
from `compile_commands.json`. Doxygen's bounded wrapper retries only signal
crashes without a diagnostic; actual findings fail immediately.

## Coverage and limits

Native cases cover restricted Lua capabilities, immutable constructors, text/
integer/structural validation, protected C++ lifetimes and resource exhaustion;
confined captured bytes/topology, aliases, links, membership and deterministic
mutation refusals; exact canonical/hash vectors; trusted bridge isolation and
limits; schema/graph/materiality, selection, budget and Markdown projections;
canonical JSON boundaries; and invocation/output handling.

Native Catch2 cases and the Python CLI acceptance suites are registered through
CTest; inspect the configured inventory with `ctest --test-dir build/rewrite-debug
-N`, and use gate logs for exact run totals. Render acceptance covers context-only
and captured renders, six computed quota goldens,
fresh-process byte stability, scoped material/optional/output edits, assertions,
root routing/refusals and unchanged sentinel state. The render suite runs 247
real CLI invocations, including a 60-check host-path submatrix. Separate init,
history and export suites run 60, 115 and 55 actual CLI processes respectively.
Native cases additionally cover verified ledger history, retained capture receipts,
prepared acceptance, atomic transactions, bounded recorded reads and exact source
export. No load service or CLI is exposed; load membership is declaration-closure-only,
and load integration/full R14 acceptance remain pending. Exact counts and host results
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

The sections below retain round-specific coverage and limits at each historical
checkpoint. The current composition checkpoint is recorded at the end; later
results supersede earlier pending release-gate statements without changing their
raw evidence.

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


R14-A1 adds four pure `replacement.tree core` cases in a separate tree-core test
executable. In-memory callbacks verify exact binary/mode reconstruction and
portable digest parity, input refusal before interning, original typed callback
errors, exception propagation, root identity and empty-directory refusal. The
core target has no format/SQLite/runtime/embedded-Lua link dependency. Existing
owner-backed tree, storage-prefix rollback, reader-invalidation and allocation
abort probes remain applicable to the same shared algorithms. The focused
selection contains all 109 prior cases plus these four cases. Round-local raw
checks, exact generator audits, module link graph and candidate hashes live in
`tests/native/r14a1-evidence/`. This is macOS debug validation; the integrated
R14 milestone retains the full debug/release gate and Linux remains unverified.
No accepted-history support is enabled by this refactor.


R14-A2 extracts pure `history.provenance_codec` and `history.interpretation_codec`
modules beneath the existing wrappers. Their direct test executables import no
format, SQLite, runtime, embedded-source or Lua modules. Five provenance codec
cases repeat the exact registry/mode, assembly/order, path, historical-manifest
and whole-set bounds under that isolated link. Four interpretation codec cases
load saved canonical fixtures directly, exercise fixed envelope/version/container/
ordering/correspondence refusals and historical policy retention, preserve signed-32 and
16 MiB limits, and require pure assembly to reproduce both saved canonical files
byte for byte. A differing historical digest remains data. Existing live-wrapper,
storage and CLI tests retain their original names and behavior.

The focused selection remains
`replacement.(ledger|canonical|hash|record|tree|provenance|interpretation)`, which
includes both codec prefixes: all 113 previous cases plus nine isolated codec
cases. Round-local algorithm and envelope-composition audits, protected original
Lua/fixture hashes, exact reversible C2/B1 generation, direct codec link commands,
247-process CLI acceptance, both vector checks, lint and zero-work evidence live
in `tests/native/r14a2-evidence/`. This is macOS debug verification; release/Linux
and the integrated R14 milestone remain outstanding. No accepted-history support
or schema/refusal behavior changes in this extraction.


R14-B1 adds pure `replacement.ledger` history codec cases. Independent exact
JSON vectors cover initialization (matching the existing format payload), initial
acceptance, changes, no-op, restoration as a newer revision, maximum signed-64
IDs and the existing revision parent returned by no-op. Typed round trips and
refusals cover fixed key sets/types, enum/state relationships, decimal spelling,
ledger/request grammar, duplicate keys, noncanonical bytes, versions and the
one-MiB ceiling. Fixed v1 fields cannot form a valid payload near that ceiling;
exact-limit malformed and over-limit resource inputs check the boundary without
inventing padding fields. The isolated test link excludes SQLite, format, runtime
and Lua. B1 does not enable accepted history or alter schema/refusal behavior.
Raw checks and exact candidate/protection/generation hashes are under
`tests/native/r14b1-evidence/`. This is macOS debug evidence; the integrated R14
debug/release gate and Linux execution remain outstanding.

R14-B2 adds ten native accepted-history verification cases to the focused ledger
selection (143 focused cases, including all 133 prior cases with one negative
case renamed from blanket-history refusal to orphan-revision corruption). The
scratch fixture publishes initial/change/no-op/revert history at revisions 1,3,8,
with backwards clocks, original quota source/registry, exact embedded stdlib and
stored interpretation. It checks successful read/RW reopen, record precommit,
backup/restore and real hot rollback recovery. Mutations cover parents, actors,
timestamps, event/request/result/body/projection links, duplicate equivalent
record IDs, invalid roots, registry naming, stdlib assembly/digest, separate
versions, workspace, malformed interpretation and fixed expanded tree limits.
Well-shaped unsupported interpretation remains readable as raw records only
after metadata checks; unknown event/result payload and event-row versions refuse
as unsupported state. Oversized metadata and corrupt-history open/RW/write/backup
refusals preserve exact file bytes and directory entries.

Run the same focused selection and both vector checks documented above, plus
`ctest --test-dir build/rewrite-debug -R '^replacement.cli$' -V`, active formatting
and Doxygen, reversible generated-variant audits, protected-file comparison and
an all-plus-tests no-change build. Exact raw logs, prior-format feature-refusal
evidence, fixture provenance and frozen candidate identity live under
`tests/native/r14b2-evidence/`. Verification is macOS debug; release/Linux remains
an integrated-gate obligation. No public acceptance writer or history CLI is
introduced here.

R14-C1 adds eight native typed-read cases while preserving all 143 prior focused
cases. They reuse the B2 valid accepted-history fixture and check exact saved
initial/change/no-op/revert event, request and result bytes; revision metadata;
state-at-event composition; optional misses; malformed keys/IDs/page arguments;
sticky failure across all old and new methods; moves and owned result lifetimes;
115-event ascending exclusive pagination; initialized ledgers; and raw history
with unsupported interpretation. A scoped test VFS injects a real SQLite
`IOERR_READ` while copying a large private request body, then verifies that all
reads remain unavailable and a fresh owner can reopen the unchanged history.
A test-only writer proves a DELETE snapshot blocks commit, retains the old event
page, and releases the commit before a fresh reader sees the later event.

Use the same focused selection, CLI247, vector, formatting/Doxygen, reversible
variant and no-change gates above. Exact candidate and raw evidence live in
`tests/native/r14c1-evidence/`. This is macOS debug verification; release/Linux
execution and the integrated acceptance writer/CLI remain outstanding.

R14-C2 adds five native capture acceptance cases alongside all sixteen existing
capture cases. Postseal mutations cover retained bytes, normalized executable
mode, file identity, added/removed files and empty directories, directory identity,
links, special files and replaced roots. Separate registry tests cover file
bytes/mode/identity/link and parent replacement while accepting unrelated parent
membership changes. Exclusion and normalized-mode tests retain render semantics.
Repeated checks preserve the precise original snapshot/topology on success and
failure. Move/session lifetime tests verify safe refusals and release the sealing
callback; descriptor counts by root identity verify transfer, move assignment,
repeated success/failure checks, failed sealing and destruction without leaks.

Run `ctest --test-dir build/rewrite-debug -R 'replacement.capture'
--output-on-failure` after building `all planar_tests`. Round-local raw evidence
and exact candidate/protection identities live in `tests/native/r14c2-evidence/`.
The prior 151 ledger/history focused cases, CLI247, vectors, active/helper lint,
reversible generation and a zero-work rebuild remain required. This focused gate
covers the macOS host; Linux and the full R14 milestone gates remain outstanding.

R14-C3-A adds four `replacement.capture native` cases importing only
`planar.engine.capture.native`. They construct real workspace and standalone
registry receipts, verify full sealing and repeated comparisons, preserve sealed
values after changes, refuse parent replacement, and exercise ownership after
session destruction and moves. The native test executable links only native
capture, values and Catch2; all 21 existing facade capture cases remain unchanged.
The capture selection now contains 25 cases. Exact extraction/load-body audits,
pure link commands, protected source hashes and raw checks live under
`tests/native/r14c3a-evidence/`. The unchanged 151 history-focused cases, CLI247,
vectors, lint, generated-variant audits and zero-work rebuild remain the focused
gate. Evidence covers macOS debug; Linux and integrated R14 release gates remain
outstanding.

R14-C3-B1 adds native-only `replacement.acceptance` cases for immutable prepared
inputs and explicit request intent. Tests construct actual native captures and a
small test stdlib, load historical canonical fixtures, and prepare only from the
sealed snapshot. Coverage includes selective workspace binding, exact binary
bytes/modes despite later disk changes, absent/malformed registry roots, invalid/
duplicate/missing selections, exact/excess source-directory bounds, forged stdlib
assembly/manifest/digest/modes/aliases, canonical and version correspondence,
workspace structure, owned lifetimes/moves, final native change detection, and
request grammar, normalization and signed-64 heads. Selection and encoded-input
bounds refuse before payload ownership. Native capture already prevents malformed
captured modes and case aliases; shared tree validation remains active here.

Run `ctest --test-dir build/rewrite-debug -R 'replacement.acceptance'
--output-on-failure` after the serialized `all planar_tests` build. The original
151 history-focused cases, 25 capture cases, CLI247, both vector checks, active
formatting/Doxygen, helper lint, exact generated variants and no-work rebuild
remain required. Round-local frozen candidate, native-only actual link and raw
checks are in `tests/native/r14c3b1-evidence/`. This unit does not enable acceptance
writes or CLI behavior. Evidence covers macOS debug; Linux and the integrated
R14 release gate remain outstanding.


R14-C3-B2 adds `replacement.atomic` cases for the complete native acceptance
transaction. They cover initial/no-op/changed/restored content, exact parent and
timestamp relationships, replay after later heads and missing/moved captures,
body conflicts, stale CAS before candidate inspection, workspace mismatch, final
receipt refusal, and finite same/different-request thread races. Real SQLite trace
interrupts exercise each mutating table and pending-state verification; a real
commit hook refuses commit. Every failure compares the original database bytes,
fully reopens it, and retries successfully. These are transaction rollback tests,
not process-termination or power-loss claims.

Maximum-record and sparse later maximum-revision fixtures pass the full opener;
no-op at the maximum revision remains legal while changed allocation refuses.
An actual gap-free maximum event history is impractical, so a precisely reversible
test-only format variant exports the exact shared private allocator boundary.
The same variant forces only one digest kind at a time to reach file, directory,
interpretation and request collisions while retaining real hashes for other kinds
and all provenance/history checks. Removing the final native receipt condition in
an isolated generated mutation must make its receipt regression fail.

After serialized `all planar_tests`, run the complete debug gate as above. Raw
commands/results, generated manifests, protected source hashes and frozen candidate
identity live in `tests/native/r14c3b2-evidence/`. Include the prior 151 history
cases, 25 capture cases, 12 prepared cases, CLI247, both vector checks, active and
helper lint, product dependency audit and a zero-work rebuild. This round covers
the macOS debug host; Linux, power-loss and integrated R14 release verification
remain outstanding. No installed binary or operator data is involved.

R14-D1 adds native host-path tests for unchanged physical traversal, root/file
link refusals, malformed trailing file spellings, missing/special/loop inputs and
redacted exact diagnostics. Pure intent-path cases use nonexistent absolute cwd
values and cover lexical equivalence, Unicode, invalid UTF-8/NUL, cwd shape and
64 MiB bounds without filesystem dependence. The host-path target links only
value plus the test harness; Lua and ledger are absent. Existing CLI host-path
cases additionally pin complete failure envelopes with no new processes: the
acceptance suite remains 247 processes and 60 host-path checks. Evidence and the
exact extraction/protection audit are under `tests/native/r14d1-evidence/`.

R14-E1 registers `replacement.init_cli`, a separate actual-binary acceptance suite
with explicit scratch targets, HOME and PLANAR_DB sentinels. It checks exact
committed init envelopes against read-only SQLite metadata, empty ledger contents,
relative and absolute targets, original parent traversal, existing file/directory/
link refusal, invalid arguments, strict CLI11 grammar, redacted complete diagnostics
and unchanged refused targets. Existing `replacement.cli` retains all 247 processes.
Bootstrap tests additionally pin all typed storage error mappings, ledger I/O
operation details and publication/recovery guidance. Native publication-fault
coverage remains in the existing ledger suite; the CLI tests do not inject fake
publication results. Raw commands, protection/generation checks and the frozen
candidate are under `tests/native/r14e1-evidence/`. This round verifies macOS debug;
Linux and the integrated R14 release gate remain separate obligations.


R14-E2 adds native `history_read` projection cases and an actual-binary
`replacement.history_cli` suite. A test-only native helper creates scratch ledgers
through initialize_new and complete_acceptance, covering initial/no-op/change/revert
history. It adds a large historical extension to establish real CLI output-limit
refusal with empty stdout. Unsupported interpretation fixtures narrowly replace
only the two value records with real encodings/digests, restore immutable triggers
and pass the complete production opener; ordinary fixtures use no SQL publication.
The suite checks all selectors, nested shapes, exact string IDs, cursor bounds,
strict syntax, misses, private intent redaction and unchanged database/directory
contents after every read/refusal. Unit metadata covers IDs above int32 through
signed-64 maximum. Existing render and init suites remain unchanged. Raw debug
logs, protected-file/generator audits and candidate hashes live under
`tests/native/r14e2-evidence/`; release/Linux remain integrated R14 obligations.

R14-F1 adds native exact source export checks, with no CLI export registration.
After building `all planar_tests`, select its ten cases with:

```sh
ctest --test-dir build/rewrite-debug -R 'replacement.source export' --output-on-failure
```

The scratch accepted-ledger matrix covers binary/NUL/non-UTF-8, CRLF and empty
bytes, executable/nonexecutable and directory modes under umask 0077, source-only
membership, byte-unchanged ledger, old/change/revert revisions, relative targets,
existing files/directories/links, unsafe parent spellings and typed misses. An
existing unchanged fixture supplies unsupported historical interpretation. A
narrow scratch-only record adaptation restores exact schema triggers and verifies
the opener before exercising a valid empty source root. Breadth reaches 4096
files/8191 directories, and depth reaches 64; exports execute with a 128-descriptor
process ceiling. Existing tree tests retain independent malformed/bound coverage.

The new `tests/native/r14f1-support/` generator proves exact reversible source
transformation and compiles a separate test-only module/namespace. Only native
syscall boundaries and the private pre-rename boundary are redirected; production
exports no fault callback. Tests cover mkdir/open/write/zero/short/EINTR/chmod/
file-sync/directory-sync/close/rename/unavailable-capability/allocation failures,
cleanup errors and replacement of file/directory/stage identities, plus unowned
entries. A target arrival immediately before the real platform rename must
survive unchanged. Failures after publication preserve the complete target and
report uncertainty. Subprocess exits immediately before rename and at parent sync
prove absent-or-complete target behavior and that later export leaves crash
stages alone. Evidence records actual host results under
`tests/native/r14f1-evidence/`; Linux runtime and hardware power loss are not tested.

R14-F2 adds actual export CLI acceptance using the unchanged native history
fixture. It checks canonical exact envelopes, recorded source identities against
a read-only SQLite oracle, original/changed/reverted revisions, unknown
interpretation, exact source-only bytes and normalized modes under umask 0077,
unchanged ledger bytes and sibling membership, path traversal refusals, all
existing target kinds, strict CLI11 invocation failures and root help.

The stdout checks close the pipe read end before child launch. A successful
export must publish a complete tree and then return status 4 with inspect-target
guidance and operation `stdout`; an unknown revision retains its original refusal
with no publication context. Init and history keep their existing stdout error,
and the existing render acceptance pins render output failures.

`tests/native/r14f2-support/` generates an exact reversible command-only variant.
It uses real native publication, then restricts result encoding to one byte or
throws `std::bad_alloc` immediately after native success. Both return the prebuilt
uncertainty buffer with empty stdout, operation `export`, preserved complete target
and unchanged ledger. This is controlled boundary evidence, not real allocator
exhaustion, hardware power loss or Linux runtime coverage. Direct adapter tests
cover redacted native categories and malformed cleanup-token refusal; they do not
claim actual CLI fault injection for unreachable native I/O categories. Raw logs,
protected-file audits, generated manifests and candidate hashes are under
`tests/native/r14f2-evidence/`.

## Current command composition checkpoint — R14-G

The approved composition builds one binary with `render`, `init`, `history` and
`export`. Native acceptance preparation and atomic transactions are implemented;
load membership is declaration-closure-only; load service/CLI and the full R14
acceptance matrix remain pending. This checkpoint does not close R14 or claim accepted-revision rendering,
execution workflows, installation or distribution support.

On macOS 26.5.2 ARM64, `make test-all PRESET=release` configured RelWithDebInfo,
built `all planar_tests`, passed all 331 registered tests with zero skips, checked
formatting/Doxygen over 86 active C++ sources, and finished with a zero-work
rebuild. The four actual CLI suites cover 247 render, 60 init, 115 history and 55
export/output processes. The approved F2 debug331 full gate and subsequent
focused strengthened-export evidence are reused with unchanged product/build/test
inputs; R14-G retains the same 331-name debug inventory and a new zero-work build.

Raw release logs, retained focused CLI JUnit/LastTest output, toolchain details,
protected-input audit and exact documentation/composition manifests are under
`tests/native/r14g-evidence/`. The full-gate helper removes its temporary JUnit;
a later inventory query replaced its LastTest log before archival. A focused
four-suite rerun retains both outputs and actual process counts. Historical raw
evidence remains unchanged. Linux compilation/execution and hardware power loss
remain unverified; no load integration or end-to-end R14 completion is inferred.

R14-D2 adds selected-file ancestor identity regressions to the existing native
capture target. A retained file inode hardlinked into a replacement parent must
refuse both sealing paths and repeated acceptance verification. Nested-chain
replacement during discovery, removal and symlink replacement, shared/repeated
reads, immutable topology, normalized modes, aliases and moved capabilities are
covered. Unrelated sibling edits, rename/removal, additions, large files,
unreadable files, symlinks and FIFOs remain outside selected closure receipts.
Whole-tree membership and render packet/identity goldens remain independently
covered by the existing capture and actual CLI suites. Frozen pre-fix failing
source/logs and final debug evidence live in `tests/native/r14d2-evidence/`.
This prerequisite introduces no load service or CLI; release composition and
Linux execution remain later gates.


## V5 traceability and current verification checkpoint

The [acceptance map](vernacular/acceptance-matrix.md) maps every v5 A01–A34 row
and locally assigned authoring A35/A36 to current evidence and remaining work.
Native storage/capture tests do not establish dispatch, run reconciliation,
checks, approval or the complete recorded execution trajectory. Full cut-2
acceptance requires all applicable Render/Execution obligations, conditional A24
assertion/refusal policy and minimal A34 execution provenance; A21 composition and
controlled A34 experiments remain deferred. [Alignment](vernacular/v5-alignment.md)
records the interpretation and pending V5-02/RA01 gates.

[Approved D2](vernacular/rounds/R14-D2-review.md) retains 337 debug passes with zero
skips on macOS ARM64, LLVM 23.1.4 and CMake 4.4.4. Its reviewer independently reran
31 capture cases and 28 dependent cases serially and reused the hash-verified
full debug gate. [G](vernacular/rounds/R14-G.md) remains the earlier 331 release
checkpoint on the prior compiler; it does not prove D2 release coverage. Linux
runtime, hardware power loss and full load integration remain unverified.
V5-01 changes prose only and runs document/protection checks, with no product
build or test rerun. Future implementation must run its scoped checks and integrated
gates on the actual candidate; historical counts are evidence, not a current target.

V5-02-B1 adds internal retained-command tests with finite native fake providers;
these do not establish CLI integration. Exact-source runtime/interface variants
under `tests/native/v5-02-b1-support/` alter only captured test bundles and module
identities. They cover complete catalog admission, callable ownership and retained
binding, read-only globals/libraries, typed selectors and 64-bit decimal strings,
ordered opaque path handles, native failure poisoning, terminal caught budgets,
real render equivalence and export postpublication failure state. Production
creation accepts only the actual build-embedded bundle, never caller Lua source.
The new command module changes embedded provenance; interpretation tests derive
the expected tree digest independently from authored originals and exact assembly,
then compare every other reviewed field against immutable historical fixtures.
B1 evidence is under `tests/native/v5-02-b1-evidence/`; B2 owns actual retained-VM
CLI composition and its full debug/release gate.

V5-02-B1-C01 adds bounded captured-state regressions: direct/new/nested/transitive
closure and table-key mutations refuse, shared/cyclic table and callable-key
identity and reads survive sealing, captured next/rawget/pairs/ipairs aliases and
length remain usable, and invocation-local scratch stays mutable. A captured
module table can cross the native result converter without losing its contents.
Small 256/257-depth graphs test acceptance and typed resource refusal without a
stress run. New red/final evidence is retained separately under
`tests/native/v5-02-b1-c01-evidence/`; prior B1 evidence and goldens remain intact.

V5-02-B2 composes the retained command VM with real CLI11 parsing and native
capture/init/history/export adapters. Six exact-source real-process CLI variants
exercise changed Lua policy, malformed registration, caught registration resource
exhaustion, compiled option-signature drift, native error poisoning and post-export
Lua/projection/caught-budget failures. A dedicated native regression injects an
allocation failure between native publication and service-facts construction;
its red/green evidence preserves the missing-publication-bit failure. Existing
response encoding/allocation and actual CLI goldens remain checked. A render
mapping mismatch must retain its typed reference refusal before reading missing
spec/registry/source inputs. The direct B2 gates retain raw JUnit and LastTest
before later inventory, compare all 358 prior test names, audit every generated
variant family and require formatting/Doxygen, vectors and zero-work rebuilds.
Evidence is under `tests/native/v5-02-b2-evidence/`. B2 is independently approved; the C workspace-name implementation remains a
candidate pending independent review. Load/full R14 and Linux execution remain
pending.

The B2 candidate's full debug and fresh RelWithDebInfo gates each pass 361 tests,
zero skips, preserving all 358 prior test names. Both gates check 89 active C++
sources and finish with zero-work rebuilds on macOS26.5.2 ARM64, Clang23.1.4 and
CMake4.4.4. The added real CLI suite executes 99 CLI processes plus one fixture
helper; its deterministic passing path evaluates 410 explicit Python assertions.
Raw full JUnit/LastTest, exact variant audits, vector checks and candidate hashes
are retained in B2 evidence. These B2 results are independently approved, with no Linux result inferred.


V5-02-C adds pure/native workspace selector and invocation preparation tests.
The selector classifies names independently of existence, ignores the environment
for explicit files, and checks UTF-8/NUL, 64-byte grammar and checked 64 MiB path
joins. The native path pair retains physical render spelling separately from
lexical future request intent; link/parent traversal selects distinct real files
in the regression. Deterministic injected readers count one cwd/environment read
when required and zero for absolute/explicit inputs, and pin redacted failures.
A borrowed oversized environment view refuses before snapshot ownership or cwd;
invalid text wins over the same excessive size. Cwd acquisition uses the standard
filesystem library's owning result, then validation and checked joins precede
derived path concatenation. Pure `request_intent` tests compare name/explicit equality and changed environment
inequality without any public load or replay service.

`replacement.workspace_selector_cli` uses the actual candidate and two unchanged
exact-source registration variants. Scratch workspace copies establish exact
name/explicit render equality, environment refusal/ignoring, selector grammar,
missing input and leaf/root/captured alias refusals, source-mapping error
precedence and physical intermediate-link/parent traversal. Processes launched
from a deleted cwd prove absolute explicit and name-expanded inputs need no cwd,
while relative input reports redacted io. The existing 60-check host-path suite
changes only its workspace-directory input from bare `workspace` to
`./workspace`, preserving the regular-file error and process count. The separate
name-with-unset-env case pins its new selector meaning.

C is an implementation candidate pending independent review. Names are supported
by render only; pure intent tests do not deliver load/replay. Raw regressions,
focused results, full checkpoint JUnit/LastTest, variant/protection audits and
frozen baseline/candidate identities are retained under
`tests/native/v5-02-c-evidence/`. No historical evidence, fixture or golden is
regenerated. Linux runtime, hardware power loss and remaining R14 load integration
are outside this checkpoint.

The final C checkpoint gates each pass 370 registered CTest entries (364 unit,
six actual CLI acceptance suites), zero failures/skips, preserving all 361 prior
names. Debug and RelWithDebInfo both check 89 active C++ sources with the pinned
formatter/Doxygen and finish with zero-work rebuilds. The new selector suite runs
41 actual CLI processes per gate; these are separate from CTest entry counts.
Both exact variant audits match across configurations; the unchanged vectors
pass 12 hand byte, 20 upstream hash and 41 canonical/domain checks. An earlier
369-entry debug checkpoint is retained as superseded evidence before the final
environment ownership-bound regression. Evidence applies to macOS26.5.2 ARM64,
Clang23.1.4, CMake4.4.4 and Ninja1.13.2, pending independent candidate review.

R14-D4 adds typed native diagnostic regressions without exposing load. The
locked-CAS case copies expected/actual heads after rollback, advances the ledger,
and checks the original values remain intact. Existing forced-collision cases
now inspect owned details for file, directory, value and request, both through
record writers and through complete acceptance/tree callbacks; existing prefix
rollback and unchanged-database assertions remain. Request collisions retain no
digest. Shared response tests pin canonical complete error envelopes, empty
stdout, LF and exit status, all existing generic ledger mappings, and malformed
category/head/kind/digest rejection with the fixed redacted internal response.

The D4 candidate's focused debug selection covers 153 CTest entries: 147 native
and command-unit cases plus all six current CLI suites. It preserves the 370
prior names and adds three cases. The selection is:

```sh
ctest --test-dir build/rewrite-debug -R 'replacement\.(atomic |ledger |command |runtime |.*cli$)' --output-on-failure
```

Evidence under `tests/native/r14-d4-evidence/` retains the old API's two failing
missing-detail assertions, final JUnit/LastTest before inventory, direct incremental
builds, pinned formatter/Doxygen, unchanged vectors, exact seven generated variant
families plus fixed runtime harness, protection audit and candidate identities.
D4 remains pending independent review. This bounded macOS ARM64 debug check reuses
the approved 370-case debug/release baseline; it is not a full milestone rerun,
a release/Linux result, load integration or hardware power-loss verification.

R14-D5's internal load-capture candidate adds seven native workflow cases. The
harness uses actual embedded `route_workspace` and `interpret_bundle` with every
job, including invalid unselected jobs, wrong constructors/headers and duplicate
job/work/role IDs. It checks logical Lua diagnostics, exact six-file declaration
and reference closure, sorted source membership, registry bytes/mode and all
embedded original/synthetic provenance. Native checks cover route shape/order,
B/M membership/header/reference-byte disagreement, sealing and later receipt
changes, selected modes/ancestors/root/links/registry, unrelated sibling tolerance,
missing/oversized inputs, owner moves/consumption/terminal failure and corrupt or
excessive explicit provenance. Fixtures contain no source/base checkout.

The focused debug selection includes capture, acceptance preparation, load,
interpretation, atomic acceptance, command/runtime cases and all six current CLI
suites. Raw JUnit/LastTest, executed commands/exits, pinned format/Doxygen, unchanged
vectors, exact generated variants, no-work build and frozen baseline/candidate
identities are under `tests/native/r14-d5-evidence/`. Same-VM handler/provenance
binding is D6 work; public load is D7 and full R14 is D8. This candidate's bounded
macOS ARM64 debug verification is not a full release/Linux milestone.


R14-D6's retained-load candidate adds ten command tests using private generated
five-entry runtimes; production and all six current CLI suites retain the four
public commands. The real D5 owner captures every job and prepares B/M supplied by
route_workspace and interpret_bundle in the actual retained VM. An altered
callable changes interpreted goal text and asserts its own commands.lua bytes and
digest. Completion outputs are finite test-provider results, not ledger commits.
Tests cover exact service order, opaque-slot forgery, cross-family calls, duplicate
and skipped services, poisoned native errors, malformed capture/B/M/returns,
changed saved success, provider exceptions, caught allocation/instruction and
cumulative native transfer exhaustion before/after completion, validated typed
intent, missing/duplicate/wrong-capability/wrong-signature private catalogs and
retained committed/replayed/uncertain receipts. Existing registration/execution
cumulative-budget tests remain in the focused selection.

Raw attempts, focused JUnit/LastTest, preserved inventory, pinned lint, vectors,
all seven historical variant families plus fixed harness and six D6 exact runtime
variants, no-work output, protected-input audit and frozen candidate identities
are under tests/native/r14-d6-evidence/. Verification is scoped macOS ARM64 debug;
D7 owns public load/transaction/transport integration and D8 full R14 acceptance.
No release/Linux or full milestone result is inferred from these tests.


R14-D6-C01 adds a private exact-source regression with unchanged D6 load code and
assembly bytes. Its sole original-metadata defect changes non-command
`planar/json.lua` to `../json.lua`, rejected by provenance tree validation. The
command logical slot and registration line ownership are intact. Before the fix,
both sections fail VM creation. With deferred preparation, registration and an
ordinary init invocation succeed; fresh load performs real D5 open/capture and
then refuses before the native accept provider, with no completion. Existing
normal/altered D6 successful binding tests remain required. New support audits
native/interface identities, the exact logical-path change, every original byte,
separator and assembly; historical generators/evidence are preserved. Raw red,
focused debug JUnit/LastTest, audits and candidate identities are retained under
`tests/native/r14-d6-c01-evidence/`. This is a correction candidate pending
independent review; public replay/CLI and real completion remain D7 work.


R14-D7-A adds identity-bound native acceptance receipts without exposing load.
Tests retain real accepted/noop saved bytes, refuse a replacement ledger even when
it contains an equal request, and replay before stale CAS or moved/missing capture.
Concurrent identical requests, different request bodies and competing heads retain
one committed row set and authoritative per-call phases. Existing prefix mutation,
verification and commit-hook faults now inspect receipts. A separate reader forces
a real busy COMMIT: explicit rollback produces `not_completed` with owned proposed
bytes and permits retry. Commit-hook automatic rollback conservatively retains
`commit_uncertain`, because autocommit alone after failure proves no outcome.
SQLite primitive tests preserve deferred-constraint diagnostics across confirmed
rollback, check inactive/success observations, and record write/sync VFS evidence.

Raw direct build/focused debug JUnit/LastTest, inventory, pinned lint, unchanged
vectors and exact historical/D6/C01 variants are retained under
`tests/native/r14-d7-a-evidence/`. This private feature slice has no baseline API to
run as a failing runtime regression. Allocation-failure behavior follows ownership
and no-throw phase/move ordering; no deterministic C++ allocator-failure seam is
claimed. This candidate requires independent review. Public load, transport faults,
full debug/release R14 acceptance, Linux and hardware power loss remain later work.


R14-D7-B adds `replacement.load_cli` and three command-unit tests. The scratch-only
public suite covers all six required text bindings, native saved-byte transport,
noop/advanced-head/deleted-input replay, exact intent conflicts, name/environment
mapping, all-job and missing-reference refusal, concurrent equal/conflicting
requests and CAS. Concurrent first attempts can conservatively refuse while a
writer owns a sidecar; the CLI suite uses finite same-intent retries only for the
exact existing busy/recovery diagnostics, logs every transient and still requires
exact final request/event/revision counts and conflict/CAS outcomes. An independently embedded exact-source CLI probe supplies finite
file barriers for deterministic early-miss/late-replay and prepared-capture races,
postcommit provider/transport synthetic `std::bad_alloc`, real bounded Lua memory
and instruction exhaustion, codec refusal, malformed/changed returns and closed
stdout. Synthetic C++ throws do not establish actual C++ allocator exhaustion.
Generated probe identities and each small native substitution are reversible and
audited against current source, including exact embedded originals/assembly.
Historical frozen evidence and fixture goldens stay unchanged. D8 owns full R14
acceptance; these macOS checks do not establish Linux or hardware power-loss safety.
Raw attempts, final debug/release JUnit and LastTest, vectors, lint, variant audits,
no-work builds and baseline/candidate identities are retained under
`tests/native/r14-d7-b-evidence/`. Independent review remains required.

The D7-B final path regression preserves ledger parent spelling while normalizing
only workspace/registry targets. Its pre-fix unit run failed on
`/missing/../ledger.db` becoming `/ledger.db`; the final unit and CLI checks require
preserved spelling and refusal of a missing ledger parent before replay. The first
399-case debug pass predates this correction and is retained separately; final
debug/release evidence is identified explicitly.

A final direct `cmd::execute` regression uses two valid absolute lexical 32 MiB
paths plus request framing. The aggregate native request bound must return the
exact redacted `resource_limit` envelope before touching a nonexistent ledger.
The pre-fix run returned `schema`; the narrowly corrected branch preserves the
native preparation category. Completed 399-case debug/release passes precede this
mapping correction. Final affected command/runtime/load/CLI checks, lint, exact
variant audits and no-work builds run on both corrected configurations; no claim
is made that those older full passes executed the changed source or the new
400th registered case. This focused retest disposition was coordinator-directed.

## R14-D8 acceptance verification candidate

`replacement.load_cli` now exercises initial/noop/comment/prose/semantic/restore
chronology through the public command, exact saved responses and historical
revision/event/request reads and source export after input removal using both the
public binary and a changed-policy test binary. Two additional native tests in
`tests/native/r14-d8-support/acceptance_fault.t.cpp` enumerate SQLITE_FULL write
failures during actual `complete_acceptance` and kill isolated children before
journal/database writes/syncs or after returned completion. Non-hot journal
remnants preserve the original main bytes and refuse open, recovery and retry;
recognized hot journals use explicit recovery and same-key retry. These native
faults do not establish public CLI VFS failure or hardware power-loss coverage.
See the [R14 evidence candidate](vernacular/r14-acceptance-evidence.md) for precise
row scope and the recovery limitation; independent review remains required.


## X03-B2-B native format2 acceptance candidate

The candidate parameterizes all 14 existing native acceptance cases and both D8
VFS fault cases across formats1/2, retaining their CTest names. Actual native
`initialize_new_v2` and `complete_acceptance` exercise acceptance/noop/restoration,
replay before stale CAS or unavailable capture, concurrent requests, final native
receipts, commit rollback, all journal/database SQLITE_FULL write ordinals and
all five process-death boundaries. Non-hot remnants still refuse unchanged;
hot journals require explicit recovery before same-key retry. These checks
retain the D8 cooperative-filesystem and power-loss limits.

Additional native tests cover codec/version mismatch ordering, fresh profile
refusal, accepted format2 history/export/backup/restored targets after inputs
are removed, and reversible production-interner variants for private codec2
collisions and cross-codec bucket separation. SQL reader fixtures remain reader
fixtures. Debug focused results and exact fault counts live under
`tests/native/v5-02-x03-b2-b-evidence/`; independent review and the subsequent
integrated verification round remain required. No release, Linux, public
format2 init or profile acceptance is established by this candidate.


## X03-B3 integrated native verification candidate

The combined B2-B+C01 native implementation is independently approved; C01 closes
the selected-format metadata finding. Subsequent B3 full debug and RelWithDebInfo
gates each pass **430/430**, with zero failures/errors/skips, pinned format/Doxygen
across 98 active sources, formatting of 15 compiled native support files, exact
current/inherited source-variant audits, 10+2 byte/hash vectors and no-work builds.
All 430 current names and all 405 X01 baseline names remain. The [X03 evidence map](vernacular/x03-format-evidence.md)
cites actual tests, both per-format full/crash matrices, old-binary retained proof,
source/binary identities and the complete baseline/final filesystem distinction.

This is a verification candidate pending independent review, not self-approval
or completion of the mandatory external-profile gate. Public init still creates
format1; native format2 fresh acceptance requires embedded request-v2 intent.
Profile acceptance/history, X04/X06/public cutover, Linux and execution remain
open. C01's alternate verified-metadata fixtures prove comparison/rollback plumbing,
not an external format-change reproduction. Source/test/configuration bytes were
unchanged through both full gates; these status documents were finalized afterward.
Raw logs, JUnit, immediate LastTest copies and exact manifests are under
`tests/native/v5-02-x03-b3-evidence/`.

The [combined X05-A0+C01 independent review](vernacular/rounds/V5-02-X05-A0-C01-review.md) approves bounded
test-only lexical sealing feasibility and closes A0-R1. The
[X05-A1 production foundation](../tests/native/v5-02-x05-a1-evidence/REPORT.md)
is independently approved in the X05-A1 review. The embedded five-command VM audits
original global/callable roots together before alias-preserving table freezing.
The isolated `runtime/closure_guard` module reads pinned Lua compiler metadata,
rejects retained upvalue writers and unknown reachable native identities, and
permits invocation-local scratch. Exact strong native identity capture precedes
module evaluation; the five native engine callbacks enroll immediately after
native installation, without intervening module execution. Proxy natives are
created only after this audit and are not claimed to pass a later raw audit.

The protected sealing frame reserves 2048 slots before root pushes/native RAII;
inspection entry and every traversal push use nonallocating active-frame checks.
A shared identity/prototype memo preserves existing graph depth256 semantics;
prototype entries include the inherited sealed-cell mask. PMR allocation requests
charge the existing checked cumulative 512 MiB native conversion accounting before
allocation, with no refund or root reset. Static metadata inspection has checked
reported counters and is not charged as Lua instructions or converted nodes.
The finite compiled input remains bounded by the registration Lua heap; no fresh
4 MiB audit arena or public scan quota is introduced. Existing embedded invocation
instruction reset remains unchanged.

A1 does not execute external modules or admit public profile flags. Later X05-A
rounds retain source-owner/ordered module admission, the three descriptor APIs,
module-end audit/seal before later chunks, frozen global/API state, persistent
frozen-identity memo, add-time snapshot versus retained mutation semantics, and
whole-registration cumulative gates. The mandatory external-profile gate, public
cutover, storage/profile history dispatch, execution/RA01 and Linux remain open;
public init creates format1. No full milestone or release gate is claimed for A1.

A1 adds colocated production guard regressions under `src/engine/runtime/closure_guard/`.
The focused Debug gate includes all A0 compatibility tests, command VM tests,
runtime/interpretation/load checks and affected CLI tests. Reversible production
probes compile the frozen candidate directly and retain compile/link commands,
source transformations, native allocation/stack refusal logs and baseline writer
acceptance. Evidence is under `tests/native/v5-02-x05-a1-evidence/`; historical
A0/C01 fixtures remain immutable. This focused gate preserves all 535 prior test
names; its registry count is an inventory, not a claim of full-suite execution.


The [X05-A2 repeatable sealing foundation](../tests/native/v5-02-x05-a2-evidence/REPORT.md)
is independently approved by the X05-A2 review. A private protected root-argument seam
seals new retained roots after the initial embedded environment boundary. Its
strong native-owned successful-identity set is distinct from each boundary's DFS
memo: the guard treats prior sealed tables/Lua closures as immutable atoms and
the freezer preserves their proxy backing and aliases. New identities publish
only after complete audit/freeze success; any failure leaves the owner terminal,
including partial freezing or publication failure. Direct native callable roots
and unknown reachable native closures still refuse. `pairs` returns the exact
stable global `next` closure captured before module execution; native snapshot
never traverses C upvalues. Repeated sealing never writes frozen globals.

The production public runtime interface and old invocation tail remain unchanged
except that a failed sealing owner cannot invoke. Synthetic root bookkeeping adds
no graph depth; every boundary shares existing cumulative native charges and
inspection totals, without resets. Real private production probes exercise three
successive boundaries, fresh retained roots, prior proxies/aliases, exact iterator
identity across GC, scratch, writer/native refusal, resource failures and same-owner
failed retry. A3 still owns captured ordered modules, owner frames, three exact
registration descriptors and final catalog; final D is validated/copied at module
end before sealing D/F. No external execution, public profile flags, profile
storage/history acceptance or full registration gate is delivered by A2. The latest
full gate candidate is A4 (582 per configuration); the mandatory external-profile gate,
public cutover, execution/RA01 and Linux remain open. Public init creates format1.


The [X05-A3 captured registration](../tests/native/v5-02-x05-a3-evidence/REPORT.md) adds native
`command_vm::create(profile_snapshot)` with structurally checked, privately copied
captured bytes and module order. Source-only zero-result chunks execute in one
bounded VM with native owner ordinals. Three exact descriptor APIs retain D/F;
final descriptors validate and seal with their callables at each module boundary.
Catalog-v2 and exact effective provenance bind to the original captured bytes/modes.
Selected-profile errors retain redacted native logical fields separately from
existing embedded diagnostics. External `_ENV` is natively created before shipped
evaluation with constructor-only engine, self `_G`, shared pure libraries and
three exact registration functions; original globals and this environment freeze
together before external execution. Embedded handlers retain their original engine.

A3 is registration-only and independently approved with C01–C05 by the
[C05 review](vernacular/rounds/V5-02-X05-A3-C05-review.md). Profile-owner invocation
refuses before any provider action; retained alternate callable execution is
verification through private reversible production copies. Shipped shape/projection
IDs remain reserved; real adapters and semantics belong to B/C. No shipped Lua,
public CLI admission, profile history/storage interpretation or acceptance changes
are delivered. Native conversion/audit charges and explicit conservative pure-helper
capacity reservations remain cumulative; these are not process RSS accounting.
The mandatory external-profile gate, public cutover, execution/RA01 and Linux
remain open. Public init creates format1; the latest full approved gate is A4
(582 per build).


The [X05-A3-C01 corrective](../tests/native/v5-02-x05-a3-c01-evidence/REPORT.md)
preserves original entry fields before command-name/shape/projection sorting and
propagates initial provenance preparation's typed resource refusal. Its final
logical reservation enumerates six canonical/payload buffers with separate record
framing/digest room, independently derived embedded metadata encoding/payload and
graph passes, and unchanged eleven embedded/three external source payload passes.
The existing cumulative 512 MiB charged native metric remains; this is not RSS
accounting. A valid catalog near the 1 MiB ceiling and controlled remaining-budget
probes exercise these reservations. Combined A3 through C05 is independently
approved; A4 records the latest full approved gate. Profile CLI, B/C/X06, public
cutover, execution/RA01 and Linux remain open; public init creates format1.


The [X05-A3-C02 location correction](../tests/native/v5-02-x05-a3-c02-evidence/REPORT.md)
retains the selected original root through the guard's single shared audit and
maps module D/F pairs to one-based original catalog fields. Actual root freezing
uses the same mapping; failures before root selection and during identity-set
union use the active `profile.modules[N]` boundary. The existing enum-only guard
API delegates to the detailed result. Initial embedded/environment sealing and
private repeated-root sealing do not acquire external entry locations. Terminal
module retries preserve the first location; the public constructor discards failed
owners rather than retrying. Original C01 fixes and cumulative budgets remain.
Controlled probes exercise actual pre-root, partial-freeze and union refusal,
terminal state and cleanup; they do not claim natural resource exhaustion.
Combined A3 through C05 is independently approved. A4 records the latest
full gate candidate; public profile CLI, B/C/X06, mandatory external-profile
gate, cutover, execution/RA01 and Linux remain open. Public init creates format1.


The [X05-A3-C03 allocation-safe location](../tests/native/v5-02-x05-a3-c03-evidence/REPORT.md)
selects seal locations with a trivial kind/index token. Root audit/freezing no
longer formats fields. Every protected seal failure adapter renders the token
outside Lua; a rendering allocation refusal returns the existing external resource
diagnostic with the preinstalled active module field. Two independent module-field
strings per module are staged before processing, charged cumulatively, and swapped
without allocation after descriptor collection. Staging failure identifies profile;
first-failure terminal retries retain their token. The shared audit, freeze order,
depth, publication and embedded paths remain unchanged.
Ordinary index10/64 and mixed-module controls, actual native allocation refusal,
controlled Lua allocation/stack/status and throwing-allocator adapters verify these
boundaries with cleanup. The throwing allocator is a controlled adapter probe,
not a claim that realloc throws C++ exceptions or natural host exhaustion occurred.
Combined A3 through C05 is independently approved. A4 records the latest
full gate candidate; public profile CLI, B/C/X06, mandatory external
profile gate, cutover, execution/RA01 and Linux remain open. Public init creates
format1.


The [X05-A3-C04 registration attribution](../tests/native/v5-02-x05-a3-c04-evidence/REPORT.md)
extends the nonallocating original-location token across callback admission,
reference retention, final descriptor collection and pure catalog error remapping.
Successful callbacks clear their token; caught failures retain the first token,
and catalog prevalidation clears completed entry ownership. Error adaptation
renders only outside protected Lua calls. Rendering refusal retains the owned
module fallback; unmatched canonical entries retain the truthful caller boundary.
Pure global error fields move their owned storage. A separately charged/staged
catalog fallback installs by swap before final provenance work as preventative
consistency; no allocation failure at the previous short assignment is claimed.
C03 module staging/sealing, shared audit and cumulative budgets remain intact.
Controlled actual allocation probes distinguish removed zero-request operations,
remaining projection rendering refusal, sticky callback failures and module/global
phase work. Raw-reference status and unmatched pure errors use explicit controlled
adapters. No new quota or public interface is added. Combined A3 through C05 is
independently approved. A4 records the latest full approved gate; public profile
CLI, B/C/X06, mandatory external-profile gate, cutover, execution/RA01 and Linux remain open. Public init creates
format1.


The [X05-A3-C05 caught-memory](../tests/native/v5-02-x05-a3-c05-evidence/REPORT.md) makes a completed
restricted `lua_pcall` memory failure sticky in native state. The bit is set
only after `LUA_ERRMEM`, preserving a successful emergency allocation retry;
logical quota refusal remains separate. Outer restricted calls rethrow and host
adapters retain the existing resource diagnostic and original entry token.
Actual descriptor-reference allocation probes cover normal/later-error tails,
three nested catches, direct host failure and successful retry after one null
return. The function-reference-only control has zero actual growth requests;
no function-reference allocation failure or natural host exhaustion is claimed.
Ordinary caught Lua errors remain recoverable. Shared accounting, protected
ownership and existing diagnostic priority are retained. The [C05 review](vernacular/rounds/V5-02-X05-A3-C05-review.md)
independently approves combined A3 through C05 and closes R1–R7. A4 records the
latest full approved gate; public profile CLI, B/C/X06, mandatory external
profile gate, cutover, execution/RA01 and Linux remain open. Public init creates
format1.


The [X05-A4 integrated registration gate](vernacular/x05-registration-evidence.md)
is independently approved by the X05-A4 review. Full Debug and
release-preset RelWithDebInfo each pass 582/582, with zero failures/errors/skips/
disabled cases, preserving all current 582 and prior 520 names. Pinned formatting/
Doxygen checks 117 active C++ sources; common source vectors, per-configuration
runtime/CLI and reversible variant audits, and no-work builds pass. Product/test
bytes remain identical across both gates; only current status documentation is
finalized afterward. Earlier private retained-callable/fault probes remain
approved Debug evidence and are not claimed as release reruns. Profile-owner
invocation refuses before provider effects. B/C semantics/adapters, X06 recorded
history/interpretation/acceptance and receipt/whole-invocation orchestration,
X07 public cutover, X08 mandatory external-profile acceptance, execution/RA01
and Linux remain open. Public init creates format1.


The [X05-B1 pure shape foundation](../tests/native/v5-02-x05-b1-evidence/REPORT.md) is independently approved by the X05-B1 review. An optional trusted internal context selects profile-only shape IDs and
stages policy while retaining declaration version1 and the embedded default.
Every job's mandatory policy and base phase cycles finish before custom shape
callbacks; bounded additive unit obligations then join the original full graph.
A distinct test-only exact-runtime adapter exercises the actual product Lua
helpers. Its internal `shape_results` sidecar is not final selected-shape identity.
B1 does not prove retained native F dispatch, immutable F input, exact source
identity, the complete return's 16 MiB boundary, or whole-invocation accounting.
B2–B4 retain those native/identity/W/render duties. Public profile admission,
X06 history/acceptance, X08 mandatory acceptance, execution/RA01 and Linux remain
open; public init still creates format1. Focused verification is not a full B gate.


The [X05-B2-A native shape-definition foundation](../tests/native/v5-02-x05-b2-a-evidence/REPORT.md)
received a request-changes review for early error adaptation; the combined
B2-A+C01 correction is independently approved by the X05-B2-A-C01 review. The internal selected-profile VM
resolves registered external shape IDs to owned ID, final revision and exact
`shape_definition_v1` digest. Identity includes the final descriptor, native
external logical module path and lowercase hex of that selected module's original
captured bytes; module ordinal and unrelated modules do not enter this identity.
Each lookup recomputes with cumulative checked native charges, without executing
Lua or starting/resetting invocation budgets. Resource failures remain terminal.
Generic profile invocation stays closed. Independently approved B2-B codec accounting precedes B2-C retained
F dispatch/immutable input/complete-return transport, B3 selected identity/W and
B4 rendering remain open, as do X06–X08, execution/RA01 and Linux. This focused
foundation does not close B2 or the B milestone; public init creates format1.

The [X05-B2-A-C01 error-boundary correction](../tests/native/v5-02-x05-b2-a-c01-evidence/REPORT.md)
includes embedded, moved-from and already-exhausted lookup refusal construction
inside the typed boundary. An actual one-shot native allocation refusal returns
`resource_limit` at `catalog.shapes`; existing sticky conversion diagnostics keep
their first message, limit and maximum. Profile owners remain terminal. Ordinary
embedded/moved-from refusals retain protocol-first behavior on retry; a moved-to
owner remains independent. This covers controlled one-shot failures, without a
persistent exhausted-heap guarantee. Identity, cumulative reservation policy and
public admission remain unchanged. The combined candidate is independently approved by the X05-B2-A-C01 review;
B2-C and the mandatory external-profile gate remain open.


The [X05-B2-B codec reservation candidate](../tests/native/v5-02-x05-b2-b-evidence/REPORT.md)
adds optional native `codec_encode_reserved` without changing ordinary callers.
One shared validator derives checked logical graph, string, workspace and fixed
Glaze output capacity before the synchronous native charge hook. Successful
validation uses a bounded active path without heap allocation; each copied
container reserves its exact entry count. Input stability through the hook is a
trusted C++ precondition. The moved output buffer is counted once; conservative
workspace slots are not independent heap allocations or an RSS guarantee.
Focused Debug evidence and independent allocation phase probes accompany the
candidate; combined B2-B+C01 is independently approved by the X05-B2-B-C01 review. B2-C retains raw-return conversion
charges, retained F execution, immutable input and invocation integration. B2/B,
X06–X08, execution/RA01 and Linux remain open; public init creates format1.


The [X05-B2-B-C01 encode diagnostic correction](../tests/native/v5-02-x05-b2-b-c01-evidence/REPORT.md)
is independently approved by the combined X05-B2-B-C01 review. A shared outer
allocation boundary protects the existing encode handlers' diagnostic construction,
including validation refusals and callback exception adaptation. Controlled one-shot
native diagnostic allocation refusal returns the existing `resource_limit` /
`output_bytes` diagnostic; normal precedence and moved typed callback refusals
remain unchanged. This is not a persistent exhausted-heap guarantee. Encoding,
reservation math, decode and limits are unchanged. B2-C retained F dispatch and
invocation integration, B3/B4, X06–X08, execution/RA01 and Linux remain open;
public init creates format1. Focused correction evidence does not close B2/B.


The [X05-B2-C retained shape bridge](../tests/native/v5-02-x05-b2-c-evidence/REPORT.md) is independently approved with C01/C02; their reviews close B2C-R1 and B2C-R2.
Internal `command_vm::invoke_shape` binds owned final definition metadata before
calling the actual retained F in its original selected-profile VM. Exact normalized
five-field inputs preserve nested aliases and freeze transient proxies without
publishing permanent sealed identities. Exact pre-invocation tag/backing membership
survives cleanup after raw response conversion, including returned input aliases;
invocation scratch/result tags are released. One actual JSON-compatible return
passes separately charged raw conversion and reserved Glaze encoding with the
complete 16 MiB ceiling. Padding transport fixtures do not prove shape semantics.
Registration transitions once to cumulative invocation instructions; native/node
charges never reset. Admission protocol errors are nonconsuming; execution/resource
failures retain the first terminal original shape location. B1 Lua still owns shape
success/refusal/obligation semantics. B3 selected identity/W and B4 rendering,
X06–X08, public profile admission, execution/RA01 and Linux remain open. Generic
profile command invocation stays closed; public init creates format1. This focused
Debug slice is not a full B or release gate.

The [X05-B2-C-C01 correction candidate](../tests/native/v5-02-x05-b2-c-c01-evidence/REPORT.md) stores transient
snapshots under private immutable address keys, separate from every integer
`luaL_ref`. Retained shipped F and external D/F bindings survive first/repeated
shape calls, GC and failure cleanup in the owned reversible observer. The original
reviewer regression is retained and reproduced. This correction preserves existing
cleanup, cumulative budgets and public boundaries; the combined C02 review independently approves the bridge. No full milestone or release gate is claimed.

The [X05-B2-C-C02 terminal diagnostic correction candidate](../tests/native/v5-02-x05-b2-c-c02-evidence/REPORT.md) shares the profile owner's first terminal error across definition lookup and shape invocation.
A resource failure owns its original code, message, limit, maximum and field before
fallible response copying; later ID/input guards and allocation adaptation preserve
that error. Natural cumulative lookup exhaustion reproduces the prior masking and
location change, then verifies missing/wrong-kind/valid IDs, invalid inputs and
retries. Controlled one-shot native allocation probes cover ownership and copies;
no persistent host-memory guarantee is claimed. C01 private registry keys remain.
The combined independent B2-C+C01+C02 review approves this scope. The focused Debug gate adds
one regression (620 registered names, 354 focused plus 11 supplemental); no full
milestone or release gate, public profile admission, B3/B4, X06–X08, execution/RA01
or Linux acceptance is claimed. Public init creates format1.


The [X05-B3-A pure phase candidate](../tests/native/v5-02-x05-b3-a-evidence/REPORT.md) factors existing trusted validation into structural prepare/check-one/finish exports.
Every mandatory/base-cycle check precedes custom requests; checked responses retain
B1 exact obligations, guidance and count policy. Finish retains original graph edges
and forms selected_shape only from explicitly supplied trusted request-corresponding
definition metadata. The old embedded validator and B1 shape_results adapter remain.
The [phase ABI](../tests/native/v5-02-x05-b3-a-support/phase-abi.md) records handoff requirements.
The X05-B3-A review independently approves this pure slice. It does not prove native metadata
authenticity, immutable F input, same-VM dispatch or cumulative phase conversion;
B3-B owns those. B3-C W, B4 rendering, X06–X08, public profile admission/cutover,
mandatory external-profile acceptance, execution/RA01 and Linux remain open.
Public init creates format1; latest full approved gate remains A4 582/configuration.


The original X05-B3-B incomplete prerequisite checkpoint is preserved in
`tests/native/v5-02-x05-b3-b-evidence/REPORT.md`, including its actual request423
regression and scoped sticky correction. The R01 integration candidate below
supersedes that checkpoint's remaining accounting/lifetime status.


The [X05-B3-B-D01-A test-only decode feasibility](../tests/native/v5-02-x05-b3-b-d01-a-evidence/REPORT.md) demonstrates
request-local allocation interception with pinned Glaze readers, including variant
rewind, duplicate-check storage, malformed partial cleanup and typed refusal
adaptation. Independent D01-A review approves the test-only boundary. No product, vendor, codec API or runtime
bytes change. Production conversion reservation is now the separate D01-B candidate; B3-B accounting/request423 and its final gate remain open. The
637-name product inventory includes WIP; last approved inventory remains 627.
No full milestone, release or Linux gate is claimed; public init creates format1.


The [X05-B3-B-D01-B production reserved decode candidate](../tests/native/v5-02-x05-b3-b-d01-b-evidence/REPORT.md) adds optional
`codec_decode_reserved`: every actual parser/duplicate-check request charges before
allocation, followed by one checked allocation-free immutable-conversion sizing
pass and reservation before persistent ownership. Request-local Glaze wrappers
retain original grammar/error ordering; returned values use ordinary allocators.
Exact pinned control-block/tree-node/list/string recipes and the actual bounded
workspace determine capacity. Typed supplied errors move unchanged, and outer
allocation protection covers early/catch diagnostic construction in both decode
entry points. Ordinary decode API/defaults and approved encoding remain unchanged.

Independent D01-B review approves the reserved decoder prerequisite. The
X05-B3-B-R01 native integration candidate consumes both decoder hooks only while
profile semantic composition is active. Actual parser/checker allocation requests
and immutable conversion capacity charge the existing cumulative native budget
before allocation, without refunds. The inherited node and graph/transport charges
remain deliberately conservative overlapping accounting. Embedded decode keeps
its ordinary path. Semantic error-data staging also charges concrete pinned
control blocks, map nodes and copied string buffers before allocation; successful
encoded text separately charges its native wrapper while moving the already
reserved output buffer. Direct in-place construction avoids initializer-list
copies. Two narrowly scoped global-fragment libc++ headers expose only the pinned
allocation type identities; product operations retain `import std`. Reserved
encoding and first typed terminal diagnostics remain.

The combined original B3-B plus R01 candidate retains audited private phase roots
and native-owned request/result/definition values across sequential protected
same-VM prepare/F/check/finish calls. Production coverage includes recoverable JSON
refusal followed by healthy calls. Private reversible observers cover exact tag/
backing key-value membership and all16 retained references in a mixed fixture
(five shipped, three phases, four external descriptor/callable pairs), stable live
Lua heap after warmup/GC, partial baseline setup and terminal cleanup. Controlled
parser/conversion/node/Lua refusals prove no F dispatch; repeated real operations
exercise cumulative instruction/native limits. Near-budget staging controls
verify exact native error-graph requests and deny node/map/string/wrapper storage
before allocation, retaining ordinary healthy schema diagnostics. These probes supplement actual
production F and semantic execution.

The frozen candidate gate targets642 registered names (all641 prior names
preserved),376 focused Debug cases and11 supplemental history cases, production
semantics, actual allocation sampling, request423 checkpoint/current controls,
vectors, six exact source audits, pinned formatting/Doxygen and no-work builds.
See `tests/native/v5-02-x05-b3-b-r01-evidence/REPORT.md` for executed results and
limitations. The independent combined B3-B/R01 review approves this integration. No full B,
release or Linux gate is claimed; latest full approval remains A4's582/config.
B3-C W, B4 render, X06–X08, public profile admission/cutover, mandatory external
profile acceptance, execution/RA01 and Linux remain open. Public init creates
format1. Codec and original checkpoint artifacts remain unchanged.


The independently approved X05-B3-C selection adds internal native `select_profile` over original
bundle/capture/files/job/role intent. Its semantic phases run once, then a fourth
privately retained trusted phase selects from that exact finalized same-call model.
All four phase roots join the original pre-module audit/freeze; phase transport,
codec work and repeated operations share cumulative accounting and cleanup. No
caller model, shape definition or export selector is admitted. Generic profile
invocation remains closed.

Embedded `select_role` retains its original normalization and whole-bundle
validation. Shipped selection remains exact with profiles. Custom W preserves old
fields, appends `shape_selection_v1` and shape definition/revision plus synthetic-ID
sorted unit obligations where selected work is either endpoint, and hashes
`work_external_shape_v1`. Guidance is excluded. Selected source comments/revisions
change every custom-job role; separate unselected/projection modules affect only
provenance. Same-module visual source changes conservatively affect definition/W.
The independently authored minimal-fixture W and complete canonical preimage live
under `tests/native/v5-02-x05-b3-c-support/`; the candidate verification report is
`tests/native/v5-02-x05-b3-c-evidence/REPORT.md`; the B3-C review independently approves this selection scope.
This is selection infrastructure: B4 presentation, X06 history/acceptance, X07
public cutover, X08 mandatory external-profile acceptance, execution/RA01 and Linux
remain open. Public init creates format1; no full B/release gate is inferred.

Selected-profile hash work additionally reserves conservative pinned-library
cumulative string growth, owned source/domain copies, encoder scratch and digest
storage before allocation. These byte charges do not add converted-node counts.
Actual allocation probes retain typed first resource refusal; embedded digest
behavior remains unchanged. The derivation and private aggregate measurements are
recorded in `tests/native/v5-02-x05-b3-c-support/hash-accounting.md` and the C report.

The X05-B4-A candidate factors the shared trusted render body and adds the private
`render_profile_material({request,model,selected})` helper. Its caller must supply
authentic finalized semantic and selection results; logical job/role/shape identity
checks do not authenticate arbitrary tables. The helper normalizes render intent,
reads retained observations, and renders without validation, selection, F dispatch
or mutation of model/selection. Custom shapes add exact three-field identity,
full scoped unverified prerequisites and mandatory `shape_policy` after dependencies;
shipped shapes preserve the old envelope and presentation. B4-A verifies pure helper
composition only. Native same-call retention, immutable transport, cumulative full
render accounting and lifecycle proof are delivered by the B4-B candidate below; the full Debug/release B gate
remains B4-C. Public profile CLI, X06–X08, execution/RA01 and Linux remain open;
public init creates format1. Candidate evidence is
`tests/native/v5-02-x05-b4-a-evidence/REPORT.md`; independent B4-A review requested
the correspondence correction recorded below.


The X05-B4-A-C01 corrective candidate requires complete positional one-to-one
bundle/semantic job correspondence before reading any selected shape. Both arrays
must have equal lengths, each row must carry a valid matching job ID, and IDs
must be unique across the complete mapping. Reordered, missing, extra, duplicate,
ambiguous and malformed identities refuse with the existing schema diagnostic at
`render.selected`; there is no ID lookup fallback, revalidation or input mutation.
Two new production cases reproduce same-shape/different-guidance substitution and
exercise selected/unselected rows and malformed containers. The actual pre-edit
baseline and red regression are retained in
`tests/native/v5-02-x05-b4-a-c01-evidence/REPORT.md`. B4-A's independent review
requested this correction (B4A-R1); the combined B4-A+C01 review independently approves the correction and closes B4A-R1.
Pure helper correspondence does not establish native same-call authenticity;
Combined B4-B+C01 native composition is independently approved; B4-C integrated evidence awaits independent review. Public init
still creates format1; X06–X08, public cutover, execution/RA01 and Linux remain open.


The X05-B4-B candidate adds internal native `render_profile` over original
bundle/capture/files/job/role intent and optional budget. A trusted normalization-only
preflight preserves full render schema then budget-stripped selection schema
precedence before any semantics or F call. Six fixed trusted roots seal before
external modules. Native-owned normalized intents, same-call finalized model and
selection flow through sequential protected calls to the approved renderer; each
custom job's F runs once. Existing selection behavior and public generic profile
refusal remain. All phases, observation/presentation codecs, hashes and a final
reserved complete-response encoding share cumulative limits; final output is
bounded by the existing 16 MiB codec ceiling and lower role/render budget.

B3-C and combined B4-A+C01 are independently approved. combined B4-B+C01 is independently approved; B4-C records the complete Debug/RelWithDebInfo gate candidate. Focused Debug
coverage, exact root/reference and frozen-membership GC observations, preflight
zero-F and valid once-per-job dispatch, actual native allocation refusals and
controlled transport failures are recorded in
`tests/native/v5-02-x05-b4-b-evidence/REPORT.md`; allocation derivation is in
`tests/native/v5-02-x05-b4-b-support/transport-accounting.md`. This candidate adds
no provider or public profile CLI, projection dispatch or storage interpretation.
X06–X08, mandatory external-profile acceptance, public cutover, execution/RA01 and
Linux remain open; public init creates format1. Latest full approved gate remains
A4's 582 per configuration.


The X05-B4-B-C01 correction candidate routes every ordinary native render
response, including normalization, preparation, shape-check, finalization and
selection errors, through the same existing reserved complete-response encoder.
The unchanged 16 MiB output ceiling and cumulative native charges apply before
returning the original native response pointer. Validation-only/selection-only
transport and native unexpected failures retain their prior paths. The genuine
oversized original unknown-key schema regression now returns terminal typed
`output_bytes`/16777216; small ordinary errors retain their bytes and recovery.
Production phase/lifetime controls and reversible exact writer/phase/F observers
supplement focused Debug verification. The combined B4-B+C01 review independently approves this correction and closes
B4B-R1. B4-C records the separate integrated macOS gate; Linux remains open.
Public profile admission, X06–X08, mandatory external-profile acceptance,
execution/RA01 and Linux remain open. Public init creates format1; latest full
approved gate remains A4's582 cases per configuration.


The [X05-C1 focused Debug candidate](../tests/native/v5-02-x05-c1-evidence/REPORT.md)
checks pure section assembly through the private trusted export, including the
87→94→59 byte vector, exact metadata/notice joins, omission priority, mandatory
refusal, UTF-8 byte counts, supplied trailing LFs and input immutability. Private
reversible adapters compare the complete current response with the actual archived
pre-edit renderer and exercise the existing native freezer. These algorithm
fixtures do not replace required packet sections or establish native F-response
admission. Focused checks are distinct from the later full C integration gate;
no release or private historical fault-probe rerun is claimed. Review is pending.


The X05-C2-A candidate adds internal native projection-definition identity lookup
without executing the retained projection callable. It hashes the exact final
five-string descriptor, logical external owner without ordinal, and original
privately captured selected module bytes under `projection_definition_v1`.
Owned ID/revision/digest results survive VM destruction. Unknown IDs refuse
nonterminally; resource failures retain the shared first owner error across shape,
projection, validation, selection, render and moves. Prospective graph, string
copies, cumulative canonical growth and encoder scratch charge existing native
limits before allocation; no invocation starts or budget resets.
Focused Debug evidence is in `tests/native/v5-02-x05-c2-a-evidence/REPORT.md`;
fresh independent review is pending. Retained projection execution/native result
admission remains C2-B. Read services, capability/receipt orchestration, public
profile admission, X06–X08, execution/RA01 and Linux remain open. No full C or
release gate is claimed; public init creates format1.


The X05-C2-B1 candidate adds only a native exact projection JSON size prerequisite
in `engine/json_codec`: stable borrowed section views, complete input preflight,
one synchronous charge for fixed 6,325,248-byte scratch and actual fixed writer
workspace, and pinned Glaze serialization without an AST copy or retained output.
Encoded overflow is sticky and nonthrowing; bounded writing completes before
refusal. Native invalid input, encoded ceiling, resource and scratch/internal
failures remain distinct typed outcomes. Callback diagnostics move unchanged;
independent nested/concurrent owners share no routing state. The operator's global
18-name built-in reservation is now explicit in X-PROJ for both input kinds;
semantic reserved-name admission remains the later native scanner's duty.
Evidence is `tests/native/v5-02-x05-c2-b1-evidence/REPORT.md`; fresh independent
review is pending. This prerequisite does not execute F, scan/root Lua results,
open profile invocation or implement read services, public CLI, X06–X08,
whole-invocation orchestration, execution/RA01 or Linux. Public init creates
format1; no full or release gate is claimed.


C2-B2 adds the internal typed retained-projection bridge and native raw-result
admission candidate. Descriptor kind and immutable structural transport are checked
before one retained F call; callers own authenticated selected-role/history data.
The protected scanner enforces exact version1 section schema, the shared GLOBAL18
reserved union and raw limits before borrowed-view exact encoded counting and
native result copying. Original projection indices and shared first terminal errors
survive retries and moves. This candidate awaits fresh independent review; native
read services, rendering composition, CLI/provider/receipt integration and the
mandatory external-profile gate remain open. See
[the C2-B2 evidence](../tests/native/v5-02-x05-c2-b2-evidence/REPORT.md).


The independently approved X05-C3-A adds `command_vm::prepare_extension_invocation` for an
actual retained sealed external command. Borrowed parsed records use names without
`--`; omitted records mean absent, explicit absent records have no token, present
flags have no token, and present scalars have exactly one token. Native owned
variants preserve every descriptor option and presence, exact history-ID strings,
INT32 scalars and effective history query defaults. Capability-bound host options
contain typed indices into a separate native-only pending-selector list; absent
optional host options retain an empty index. Descriptor canonical order determines
option and supplied-host order. Pending selectors have no filesystem normalization,
admission, handle or receipt. Capability-free same-spelled text remains ordinary
data under the approved X04-B2 contract.

Preparation is reusable without starting/resetting invocation. Usage and protocol
refusals leave a healthy owner usable; resource refusal preserves the shared first
terminal diagnostic across APIs and moves. Embedded, moved, failed or consumed
owners and unknown external commands refuse. Prospective vector storage and all
owned string copies charge cumulative native accounting before allocation.
No handler, provider, Lua invocation graph or argv parser is introduced. Native
typed-result tests do not prove the future Lua string/presence ABI. C3-A itself adds no receipt-backed
host admission, read services, capability/order/once-only/poisoning enforcement or
response correspondence; the bounded C3-B2 admission candidate below leaves read
services, handler dispatch, public CLI and X06–X08 open. Generic profile
invocation stays closed; public init creates format1. Approved C3-A evidence is
`tests/native/v5-02-x05-c3-a-evidence/REPORT.md`; its independent review is recorded
in `docs/vernacular/rounds/V5-02-X05-C3-A-review.md`.
No full milestone, release or Linux gate is claimed.


X05-C3-B1 adds bounded metadata-only physical parent traversal to
`capture/admission`. Each component is opened without following links before a
parent component can exit it. Missing directories and non-directory components
therefore cannot cancel away. Every visited name and upward edge retains its
original descriptor receipt; only the final active ancestry determines physical
containment. Replacing an exited directory still refuses verification. Existing
file-role, alias, absent-target and original profile-owner policies remain.
Focused native admission/profile/host-path/capture checks, regression red evidence
and exact candidate identities are recorded in
[the C3-B1 report](../tests/native/v5-02-x05-c3-b1-evidence/REPORT.md).
This prerequisite introduces no runtime orchestration, read service, public profile
CLI or dispatch. No full suite, release or Linux execution is claimed.


X05-C3-B2 adds a bounded internal `extension_invocation` owner in the runtime
module. Creation consumes the original `profile_capture` and constructs the VM
from that capture's snapshot; no independently created VM or mutable C3-A result
can enter admission. The sealed catalog is borrowed only while healthy. The owner
retains final prepared scalars, mapping IDs and absolute host spelling/intent
privately. Its single admission attempt prepares the actual sealed command,
normalizes all supplied host slots before metadata checks, admits workspace root
and selected file, source/base directories and registry/observation/ledger files,
then verifies original profile/module and late-slot receipts. Every failure is
terminal and retains its original typed diagnostic; moves transfer this state.
Verification is available only after successful admission and authorizes no service
or output by itself. Generic profile invocation remains closed.

Relative paths use only the supplied cwd snapshot. Bare workspace names use the
approved C lexical normalization of the supplied workspace-directory snapshot,
then append the name and `workspace.lua`; explicit workspace paths and expanded
results retain physical traversal spelling through `prepare_host_path.spelling`.
First-equals mapping RHS normalization preserves later equals and parent steps.
No ambient cwd/environment lookup, recapture, late-input content read, Lua handler,
provider, userdata or read-service implementation is added. Capture subsystems keep
their separate bounded metrics; whole-invocation shared capture/VM accounting stays
open for X06. New wrapper/path staging charges the existing cumulative VM native
metric before allocation, using pinned libc++ component/string bounds documented
in [the accounting proof](../tests/native/v5-02-x05-c3-b2-support/accounting.md).
This conservative metric is not process RSS accounting.

Seventeen colocated C3-B2 cases exercise real scratch captures for pure, role and
history commands, optional unused hosts, explicit snapshots, workspace expansion,
physical parent traversal, overlap/alias and native path refusals, original receipt
mutation, moves, poisoning and actual cumulative native reservation refusal.
Three host-path preflight cases preserve text/size and lexical-cwd/join precedence.
Controlled private allocation and inode-read probes use copied production objects;
[the evidence report](../tests/native/v5-02-x05-c3-b2-evidence/REPORT.md) records
commands, failed attempts, exact identities and limits. Focused Debug evidence does
not establish later Lua ABI, services/dispatch, complete shared budgets, public
profile admission, a full/release/Linux gate or mandatory X08 acceptance.

## X05-C4-B1 gated SQLite prerequisite

Eleven colocated SQLite cases exercise the distinct read-only `open_gated`
primitive: first-header refusal before actual reads, typed verifier/exception and
HAS_MOVED failures, null-handle resource refusal, retained statement/transaction
ownership and moves, independent owners, sticky failures, bound-only ATTACH
refusal, partial base-open cleanup, versioned/null optional callbacks, actual path
replacement and non-null mmap controls, and exact file bytes/mode preservation.
Existing ungated SQLite, format and history-read suites remain the regression gate.

Private reversible copies add direct production callback checks for unchanged
read buffers and null fetch output on refusal, journal/WAL/shm denial and temporary
I/O after terminal main refusal, actual WAL refusal with an ungated positive
control, original profile/ancestor/held-descriptor receipt tests, and actual native
and SQLite allocation failures. A reconstructed original backup demonstrates a
real C++ diagnostic-allocation exception leaving SQLite BUSY; the RAII candidate
finishes and permits subsequent backup and actual close. The raw failed attempts,
exact copied dependency membership and source identities are retained in the
[C4-B1 evidence](../tests/native/v5-02-x05-c4-b1-evidence/REPORT.md).

Only focused Debug checks are claimed. The generic primitive has no capture or
runtime dependency, and supplies neither a production profile adapter/raw-header
binding nor a history service or public handler. There is no full, release or
Linux execution claim; Linux is assessed for source portability only. C4-B1
awaits independent review; C4-A and test-only C4-B0 are independently approved.

## X07 test layout

- Catch2 matrices live in `src/cmd/planar/bootstrap.t.cpp` and
  `src/engine/runtime/external_registration.t.cpp`. They enumerate every X01
  row (44 E and 12 O) exactly once and assert exit, exact stdout/stderr bytes
  and a tri-state profile outcome.
- Oracle: per test, a work tree holds three profile trees. TA is a valid
  profile, TB a malformed profile (accessed exactly when its bytes differ from
  TA) and TC a valid profile whose module spins at chunk level (registered
  exactly when its bytes differ from TA). Limit: rows that never access the
  profile are indistinguishable among themselves.
- Source scans locate sources through `__FILE__` (parent directory) and assert
  structure: no manual argv scan in `envelope.cpp` or `bootstrap.cpp`, closed
  message literals, the early-lookup scope closing before profile capture, and
  adoption counts.
- Production sources carry `X07-ANCHOR:<name>` comment markers (for example
  `after-early-miss`, `after-factory-open`, `after-prepare-load`,
  `after-commit`, `response-ceiling`). Generated variants built from those
  anchors inject hooks in scratch copies; the checkout is never mutated.
- Negative-compile probes assert that forbidden API shapes (for example a
  mutable `command_vm` extraction or a public constructor) do not compile.
- Python process suites are `tests/acceptance/x07_*.py` (replay, services,
  substitution, workflow); the fixture generator is
  `tests/native/v5-02-x07-support/generate.py`; evidence is under the
  gitignored `tests/native/v5-02-x07-evidence/`.

## X08 acceptance gate

X08 closes the external-profile acceptance matrix with binary-level evidence.
Its raw logs, exact counts and results are recorded in
`tests/native/v5-02-x08-evidence/REPORT.md`; this section describes what the
gate runs and what it does not establish. The Linux recipe exists, but its
results live only in that evidence. This document claims no Linux pass, and
x86_64 is not covered.

Gate order, with every build directory serialized:

1. Focused runs of the `x08.*` cases and the two Catch2 cases, plus the affected
   prior selections, against the Debug build.
2. macOS Debug: `ctest --test-dir build/rewrite-debug --no-tests=error
   --output-on-failure` over the full inventory, with no failed and no skipped
   test.
3. macOS RelWithDebInfo: the same command against `build/rewrite-release`.
4. Inventory comparison: the sorted `ctest -N` name lists of both macOS
   configurations are identical, and they preserve every prior test name.
5. `make cpp-lint` (active formatting and Doxygen), run once, on macOS only.
6. Two consecutive no-work builds, each reporting `ninja: no work to do.`
7. Redaction check with `scripts/redact-local-paths.py check` over the explicit
   list of evidence files.
8. Linux gate: Debug and RelWithDebInfo full ctest and no-work builds in the
   container, with each sorted `ctest -N` list equal to the macOS list of the
   same configuration.

The closeout validator, `tests/native/v5-02-x08-support/validate.py --evidence
<evidence-dir> --repo <repo-root>`, reads the raw logs and evaluates gates G01 to
G09. It never runs ctest, a build or a product binary. Its self-test is
`ctest --test-dir build/rewrite-debug -R x08.validator_selftest`.

The X08 suites run the production binary, or the generated E12 variant, in
temporary roots with scratch `HOME` and `PLANAR_DB`. They assert that the
forbidden legacy database path is never created, and they never use an installed
binary. The Python cases are registered with the `acceptance` label. Each case
below has one requirement:

| Case | Requirement |
| --- | --- |
| `x08.e01_no_discovery` | E01: environment variables, and `stdlib.d` trees in the working directory or `HOME`, never select or load a profile; only `with-profile` does. |
| `x08.e01_registration_policy` | E01 and X-POL: root-level flag, aliases, overrides, duplicate or reserved identities, job-chosen trusted code and unselected `stdlib.d` files are refused or ignored with the closed messages. |
| `x08.e02_envelope_rows_binary` | E02: all 44 production and 12 opaque envelope rows give the tabulated exit and stderr class; accessed classes differ with and without the trusted directory, and not-accessed classes do not. |
| `x08.e03_closure_separation_binary` | E03: known-slot overlap, ledger aliasing and late-slot overlap refuse `unsafe_path` before registration or the handler; disjoint trees succeed. |
| `x08.e07_old_goldens_unselected_profile` | E07: six quota renders without `with-profile` match the committed goldens, and renders under an empty-module profile are byte-identical to them. |
| `x08.e07_old_w_vectors_binary` | E07: old work values and digests agree across unselected, empty-profile and golden renders; fresh-ledger history agrees except for justified provenance fields. |
| `x08.e08_projection_vector_binary` | E08: projection omission, exact-fit padding and `budget_exceeded` vectors at the real 1 MiB budget. |
| `x08.e09_history_id_binary` | E09: `history_id` boundaries, invalid spellings, selector conflicts and after-event paging through the binary. |
| `x08.e10_removed_profile_replay_binary` | E10: an identical `with-profile` request replays with equal bytes after the profile, workspace and registry are deleted; a different actor or selector conflicts. |
| `x08.e12_variant_reads_identically` | E12: the variant reads a normal ledger's history byte for byte with its trusted directory deleted. |
| `x08.e12_variant_exports_identically` | E12: variant exports equal the normal export and the pre-deletion source files. |
| `x08.e12_variant_poison_is_live` | E12 liveness for render: the normal binary exits 0, and the variant exits nonzero with empty stdout. |
| `x08.e12_variant_poison_is_live_load` | E12 liveness for embedded and `with-profile` load: each fails with empty stdout and no revision. |
| `x08.e12_variant_poison_is_live_profile_render` | E12 liveness for custom-shape `with-profile` render: failure with empty stdout. |
| `x08.e12_variant_audit` | E12: generated C++ equals product C++ apart from module names, and embedded Lua differs only in the planned substitutions. |
| `x08.e12_sources_and_profile_removed` | E12: after the profile, workspace and registry are deleted, a fresh normal process reads and exports byte for byte. |
| `x08.adv_hardlink_module_refuses_before_chunk` | Matrix row 3: a module hard-linked to the selected declaration refuses at initial admission before any chunk runs. |
| `x08.adv_missing_declaration_initial_admission` | Matrix row 3: a missing selected declaration refuses at initial admission before registration. |
| `x08.adv_opaque_ordering_binary` | Matrix row 3: opaque final syntax errors precede late checks, and overlap precedes the handler. |
| `x08.adv_invalid_unselected_job_refuses_all` | Matrix row 5: one invalid unselected job refuses the whole load and publishes nothing. |
| `x08.adv_job_mix_and_phase_failures` | Matrix row 5: many custom jobs and built-in/custom mixes load; failures in phases 0 to 2 publish nothing; the obligation bound is enforced. |
| `x08.adv_registry_colocation_binary` | Matrix row 6: a registry colocated with the modules but in a distinct file succeeds; selected-file aliases refuse. |
| `x08.adv_limits_cli` | Matrix row 12: an unbounded-loop module and an oversize module each refuse with the closed `resource_limit` and empty stdout. |
| `x08.timeout_headroom_531` | Q7 A guard: the CTest timeout of the named `prepare_load` test is at least 120 seconds. |
| `x08.linux_recipe_pins` | Linux recipe pins: base digest, CMake SHA256, exact LLVM apt pin, single COPY, non-root user, schema exception, disk and prune rules, no tag or bind mount. |
| `x08.validator_selftest` | Closeout validator: accepts a good synthetic evidence tree and rejects each defined bad tree. |
| `replacement.X08 platform branches are exactly the audited set` | Platform guard: widened platform-token counts under `src/` equal the audited per-file table. |
| `replacement.X08 Linux publish and metadata branches keep their no-replace and path-only flags` | Linux publish, metadata and open branches keep `RENAME_NOREPLACE`, `O_PATH` and `st_mtim`; the `ENOTSUP` fallback stays in the final `#else`. |

The E12 variant is a generated, test-only binary. `tests/native/v5-02-x08-support/generate.py`
produces it from the shipped sources, and `tests/native/v5-02-x08-support/x08.cmake`
registers it. Its generated tree is built under the build directory, and the
checkout is never modified. The embedded `json.lua` differs by one comment line.
A generated poison chunk, inserted between `acceptance.lua` and `commands.lua`,
reassigns the shared chunk-local exports (semantic, render, shape, role and
profile-material functions), and the generated bootstrap poisons `parse_json`.
All embedded files form one chunk, so the reassignment reaches the shipped
handlers and the bootstrap export table.

The E12 cases prove two things. Differing embedded stdlib bytes read and export
byte for byte the same stored data as the normal binary, and the poison is live,
so the failing exports are actually reached. They do not prove that no Lua runs:
the history and export handler callables in `commands.lua` run on the public
path. The claim is narrower. No poisoned bootstrap export is reached by history
or export. If history or export ever reaches a poisoned export, that is a
product-defect candidate. The round halts for review with the exact call chain,
and the poison is never narrowed to make the test pass.

Linux gate: run `scripts/linux-gate.sh <new-out-dir>`. The out directory must not
exist and must lie outside the checkout. The runner refuses below 15 GiB free on
the data volume before a build, records Docker usage, and writes host-side
manifests of the image context. It runs one `docker build --target evidence
--output type=local,dest=<out-dir>/container` from the repository root, with no
image tag and no bind mount. It then compares the host manifest before and after
the build with the in-image context manifest, and finishes with
`docker builder prune -f`, which prunes only the builder cache. It never runs
system prune, image removal or `-a`. Building needs network access to
apt.llvm.org, the Kitware CMake release archive and the base image registry. The
runner targets macOS.

`scripts/linux-gate.Dockerfile` pins the base image by digest (Ubuntu 24.04, arm64
native under Docker on the Mac), pins the LLVM 23 apt packages to one exact 23.x
version at or above 23.1.0, and pins a CMake 4.4.x archive by version and SHA256.
Source is copied into the image filesystem, and build directories stay in the
container. The gate runs as a non-root user (uid 1000). The permission tests
branch on `geteuid() == 0`, and a root container would take the weaker branch and
skip the intended Linux rows. Each gate command writes its raw log and exit status
to the output directory, and the Dockerfile does not fail the layer on a test
failure. In each configuration, the gate configures, builds `planar_tests`, runs
full ctest with `--no-tests=error`, runs a no-work build and records a sorted
inventory. Lint and Doxygen run only on macOS. x86_64 is not covered.

`scripts/linux-gate.Dockerfile.dockerignore` excludes git metadata, build
products, round evidence and local agent state. It has one exception: the
gitignored `tests/native/r12b-evidence/expected-schema.sql` is re-included,
because `src/engine/CMakeLists.txt` points the ledger format tests at it. A public
clone that tracks only committed files lacks that schema, so the replacement
ledger format tests cannot run there until the file is relocated to a tracked
fixture path.

Platform branches. The eight source rows below record the original X08 coverage.
The Linux-only CMake branches in `cmake/llvm-toolchain.cmake` also belong to that audit: the
`UNIX AND NOT APPLE` flag at line 235 and the one at line 283, and the
`_planar_runtime_system STREQUAL "Linux"` branch at line 252. The line-252 branch
is inside `if(PLANAR_PORTABLE)`, which is off by default, so the default
configuration does not execute it.

| Source | Apple arm | Linux arm |
| --- | --- | --- |
| `src/engine/capture/native.cpp:46` | `st_mtimespec` | `#else`: `st_mtim` |
| `src/engine/capture/admission.cpp:62` | `st_mtimespec` and `st_ctimespec` | `#else`: `st_mtim` and `st_ctim` |
| `src/engine/capture/admission.cpp:71` | `O_EVTONLY` for files | `#else`: `O_PATH` for files; directories keep `openat` |
| `src/engine/export/source_export.cpp:7-12` | `<sys/stdio.h>` | `<linux/fs.h>` and `<sys/syscall.h>` |
| `src/engine/export/source_export.cpp:179-189` | `renameatx_np` with `RENAME_EXCL` | `SYS_renameat2` with `RENAME_NOREPLACE`; the `#else` `ENOTSUP` fallback (lines 183-188) must not be taken |
| `src/engine/runtime/runtime.t.cpp:2839` | `st_mtimespec` | `#else`: `st_mtim` |
| `src/engine/acceptance/source_capture.t.cpp:626` | `O_EVTONLY` admission refuses with `EACCES` | `#else`: `O_PATH` admits, and the failure belongs to the later content read |
| `src/engine/runtime/external_registration.t.cpp:4557` (test only) | `c02_outputs_gap` 4742 and `c02_off_boundary` 228586 | `#else`: 4737 and 228612 (libc++ container and string sizing differ by a few native bytes); audited in `audited_platform_counts()` with count 1 and recorded as validator row G06i |

The current exact per-file token inventory is `x08::audited_platform_counts()` in
[bootstrap.t.cpp](../src/cmd/planar/bootstrap.t.cpp). Later reviewed branches are
recorded there. The R17/C13 runner test has two macOS-only guards: one creates
the intentionally inheritable subject, and the other checks its flags immediately
before spawning. Its common descriptor probe and descriptor-9 control also run
on Linux. These additions retain the original X08 row values and validator IDs.

The `geteuid() == 0` branch in `source_capture.t.cpp` is the reason for the
non-root container user described above.

Q6 A rule. A Linux-only corrective is permitted only inside a Linux branch. The
Linux arms of `capture/native.cpp` and `capture/admission.cpp` are the `#else` arms
of `#ifdef __APPLE__`; since the repository supports only macOS and Linux, every
non-Apple arm counts as Linux code. A Linux failure in the `#else` arm of an
existing test (`runtime.t.cpp` or `source_capture.t.cpp` in the table above) halts
the round for review. The resolution is either a product defect fixed in the Linux
arm with a regression test, or a reviewed corrective that changes only the Linux
arm of the test and shows the exact assertion delta. Assertions are never weakened
silently. Any other Linux failure needs a fresh corrective round.

#531 note. The test identified by name, `X06 B3 C a file replaced at any allocation
inside prepare_load is never published as verified` (in `src/engine/runtime/runtime.t.cpp`),
runs under the `planar_runtime` module. That module's CTest TIMEOUT is 120 seconds
in `src/engine/CMakeLists.txt`, a change that landed in V5-02-X06-D; the module
previously ran at the 30-second default. The Q7 A timeout requirement is
therefore already satisfied, and X08 makes no CMake edit. The case is named rather
than numbered, because its CTest number changes between inventories. The guard
`x08.timeout_headroom_531` pins the timeout at 120 seconds or more. If the Linux
container shows this case above 60 seconds, raising the module value is a
closeout test-infrastructure edit, not a product change.

C01 additions. The Linux gate of the second X08 run failed four pre-existing tests; C01 repairs them and the
final Linux results again live only in the evidence directory, so this document claims no Linux pass. The
host path fix is a platform-neutral pre-check, covered by `host path maps a regular file followed by parent
traversal to the root directory error` and `host path keeps missing and unresolvable directory spellings as
input path errors`. The C02 budget-gap tests (`X06 D C02 ...` and `X07 N30 ...`) use a fixture root pinned to a
fixed canonical byte length so the pinned gap does not depend on the platform temporary path; the case
`X08 C01 the C02 budget fixture root has a fixed canonical byte length` pins it (`c02_root_bytes` = 64).

Per-platform budget pins (C01-R6). The outputs-on gap `c02_outputs_gap` is measured at the pinned root:
4742 in Debug and RelWithDebInfo on macOS and 4737 in both on Linux, because the libc++ container and string
sizing differ by a few native bytes. Each platform's test uses its own measured value, with no slack, so a
two-byte charge deletion fails on either platform; the single 4742 pin and its "keep the smaller value" rule are
withdrawn. The test-only `#if defined(__APPLE__)` that selects the pins is an audited platform branch (see the
table above). The outputs-off boundary `c02_off_boundary` is an absolute measurement: 228586 on macOS and 228612
on Linux (the macOS value was measured in Debug only and the Linux value in Debug and RelWithDebInfo; the macOS RelWithDebInfo pass shows only that its value is within the window of 8), checked inside a window of 8 padding bytes by `X08 C01 role
preparation outputs-off boundary pins the request-record conversion charge`, which kills deletion of the
request-record conversion charge in `runtime.cpp` (about 33 padding bytes). Re-measure rule: any change to native
accounting on the role preparation path (a charge added, removed or resized, a changed ceiling, a changed
canonical root length or a changed libc++) moves the boundary and the gap, so re-measure both constants in Debug
and RelWithDebInfo on each platform with `scripts/linux-gate.sh` for Linux, and set each platform's pin to its
own measured value. Never widen the window or take the smaller platform's value to absorb a move.

Decimal id facts are bounded at INT64_MAX by `X08 C01 native decimal id facts are bounded at INT64_MAX
exactly`. The expected inventory is 1260 names (1227 prior, 28 X08, 3 C01, 2 R6).

Closeout validator. `tests/native/v5-02-x08-support/validate.py` reads the assembled evidence tree. Gate G06
now requires each platform row's `pass_line` to occur verbatim in `linux/container/debug-ctest.log`, so a
stale line fails; the rows are G06a to G06i.

Known limits, restated and not repaired. There is no RSS (process memory)
guarantee, and no defence against a hostile writer. CL-1 to CL-6 and the X07-C01
limits remain as recorded in their contracts. A crash during the busy wait is
accepted behaviour, per `docs/vernacular/ledger-contract.md` lines 493 to 497. `tests/native/r12b-evidence/expected-schema.sql` is gitignored,
so a public clone cannot run the ledger format tests until it is relocated. Row 3's
opaque-to-load transition and row 6's leaf replacement between registration and
first read are native-only: the binary cannot observe them. Linux on x86_64 is
not covered.
