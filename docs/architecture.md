# Planar architecture

Current rewrite status is the [roadmap opening](vernacular/roadmap.md#current-status).
Superseded status checkpoints that formerly opened this file are preserved in
[history](vernacular/history.md#former-opening-of-architecturemd).

The replacement builds one thin `planar` command over a small native engine and
an embedded trusted Lua standard library. There is no legacy engine or database,
command fleet or operational agent workflow in the source tree.

| Layer | Source | Responsibility |
| --- | --- | --- |
| Command | `src/cmd/planar/` | Parse explicit arguments once, validate the embedded catalog, invoke retained Lua command handlers through finite native render/init/history/export/load services, buffer canonical output and check process writes. |
| Values and declaration loading | `src/engine/value/`, `src/engine/lua/` | Immutable typed values, text-only restricted constructors, bounded protected Lua evaluation. |
| Capture | `src/engine/capture/` | Confined no-follow reads, logical provenance, stable retained bytes and directory topology, final verification. |
| Identity | `src/engine/canonical/`, `src/engine/hash/` | Typed canonical_v1 binary encoding and bounded decoding, domain-separated seed-zero XXH3-128. |
| Trusted runtime | `src/engine/runtime/` | Fresh fixed-export states, invocation-local retained command VMs, bounded typed bridges and native structural/hash capabilities; internal original-receipt-bound native history preparation closes its ledger snapshot before returning. |
| Transport | `src/engine/json_codec/` | Pinned Glaze parsing and buffered canonical UTF-8 JSON with a 16 MiB payload ceiling. |
| Pure record codec | `src/engine/history/record.cppm`, `record.cpp` | Bounded record_v1 framing and raw portable XXH3-128; independent of storage, runtime and filesystem access. |
| Complete immutable trees | `src/engine/history/tree.cppm`, `tree.cpp` | Captured logical views to immutable directory roots and complete owned reconstruction, bounded by explicit source/provenance/effective-provenance profiles; no filesystem capture or Lua replay. |
| Native ledger foundation | `src/engine/ledger/` | Internal trusted SQLite primitives plus opaque typed record transactions, complete record and accepted-history verification, atomic acceptance and safe new-file publication. |
| Native acceptance preparation | `src/engine/acceptance/` | Declaration-closure load capture and immutable prepared candidates backed by native receipts; identity-bound atomic completion. |
| Recorded reads | `src/engine/history/read.cppm`, `read.cpp` | Bounded history projections from one verified snapshot, without replaying recorded policy; the current retained handler selects the verified envelope. |
| Exact source export | `src/engine/export/` | Reconstruct recorded source and publish a new target without replacing existing entries. |
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

X04-A1 adds a native-only metadata admission prerequisite in
`capture/admission.cppm`: explicit trusted directory and selected files, staged
supplied roots/files/export targets, retained no-follow ancestor/mode/identity
receipts, and file-descriptor binding before a native content read. Receipt
verification reads no selected content and does not enumerate siblings. Registry
directory colocation is allowed; trusted aliases and untrusted tree overlap refuse.
X04-A2 composes it in `capture/profile.cppm`: the native-only service captures
strict Glaze profile JSON through the pure `capture/profile_codec` decoder and the entire ordered module closure, selecting every
module identity before reading any module content. Profile bytes are capped at
65536, modules at 1 MiB each/8 MiB total, and strings/paths at 1024 decoded bytes.
Glaze decodes into fixed bounded scratch before retained string allocation; raw
integer version views avoid machine-integer classification limits. The owner
retains both metadata admission and native content receipts; staged admission
and descriptor binding consume no supplied file content. Repeated verification
compares original receipts, and failed/moved owners refuse. Only the listed
closure and its ancestors are selected; sibling membership remains irrelevant.
A1, A2 and the integrated A3 native verification gate are independently approved.
The full A3 gates pass 489/489 in debug and RelWithDebInfo.
Public CLI and result/commit phase wiring remain future
integration. External-profile acceptance remains open.

Each profile factory (`profile_invocation::open_known_load` and `open_opaque`)
creates one factory budget that composes capture/admission metadata, staging,
the declaration loader VM (instructions, native bytes and live Lua as
acquire/release), retained values, the source-capture bundle copy, registration,
shapes, conversion, preparation and extension role preparation. Contract
ceilings are enforced with checked addition before allocation, and stricter
inherited limits still win. `extension_invocation::create`, the embedded source
capture path (`source_capture::open_embedded` in
`src/engine/acceptance/source_capture.cpp`) and the CLI render session
(`src/cmd/planar/invocation.cpp`) are outside the factory. No resident-memory
(RSS) claim is made. The enforced ceilings are the 512 MiB live Lua ceiling
(`memory_max = 512 * 1024 * 1024` in `src/engine/runtime/runtime.cpp:32`; the
declaration loader has its own separate 64 MiB `memory_max` in
`src/engine/lua/loader.cpp:12`), enforced for declaration-VM Lua by
`factory_budget::acquire_lua` as live acquire and release (decision Q9 in
`docs/vernacular/remaining-steps-design.md`), the cumulative native conversion
ceiling, the 100,000,000 invocation instruction ceiling and 4,000,000 conversion
nodes. Small-ceiling test variants inject smaller numbers through
`PLANAR_RUNTIME_*` compile definitions. The budget interface lives in `planar.engine.budget`
(`src/engine/budget/budget.cppm`, depending only on `planar_value`), exported by
capture/admission and imported by the Lua loader.

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
or approval. The native ledger/history foundation followed completed operator
packet evaluation. The D7-B load candidate exposes native acceptance through a
retained Lua handler and declaration-closure capture; independent review and the
full R14 acceptance matrix remain pending. Execution workflows remain outside this slice. See [contracts](declarations.md), [CLI](cli-reference.md),
[testing](testing.md) and the [implementation plan](vernacular/implementation.md).

The trusted bridge alone exposes `engine.json_decode` and `engine.json_encode`.
Glaze owns all JSON grammar, unescaping and escaping; Lua applies schema policy.
Checkpoint byte accounting, Markdown descriptors and machine transport use the
same native codec. Binary canonical_v1 encoding and hash domains are unchanged.
Codec allocations are separate from Lua's allocator: each encode uses at most
96 MiB + 1 KiB scratch plus bounded structural copies and its caller-owned input.
Cumulative native bridge conversion retains its 512 MiB accounting ceiling.

`main.cpp` performs checked process writes. `bootstrap` creates the CLI11 root,
attaches render, init, history and export command handlers, counts
bindings across all five, and dispatches exactly one selected command. The
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

This module is an internal trusted SQL primitive, unreferenced directly by
render, Lua or CLI handlers. Explicit read-only/read-write modes do not create
missing files. `scratch_create` uses SQLite CREATE on a caller-owned staging or
scratch path; it is not exclusive creation or safe publication. No default paths,
parent creation, schema recognition, format verification, migrations or ledger
acceptance are implemented by this primitive. Its narrow `online_backup_to` uses
SQLite backup init/step/finish, always finishes the handle, preserves typed codes,
and refuses invalid owners, a shared source/destination or an active destination
transaction. A pinned source read snapshot is supported. One step copies all pages;
there is no retry loop extending the configured busy wait. The separate format
module adds the bounded typed format API below. Opening an
arbitrary file through this primitive does not establish ledger validity.

The primitive makes supplied paths absolute without normalizing components or
resolving links before calling SQLite. This disambiguates native filenames such
as `:memory:` and `file:literal?mode=memory&cache=shared` from SQLite special
filenames and URI syntax. SQLite itself can collapse dot components, so the
wrapper first requires the OS to traverse the original absolute parent to an
existing directory. Thus `missing/../db` cannot silently open `db`. SQLite
NOFOLLOW remains enabled, including for symlink ancestors followed by `..`.
These checks are not a hostile filesystem race guarantee or format validation.


`planar.engine.ledger.format` exposes `initialize_new`, `open_existing`,
`backup_new`, `restore_new`, `recover_quiescent`, `begin_records` and opaque move-only owners
with verified metadata and typed record reads. It exposes no SQL or SQLite
handle. Initialization accepts an explicit new filename and declaration workspace
ID, obtains sixteen identity bytes from OS `getentropy`, and records a UTC
millisecond timestamp once. The initialization event actor is the fixed native
assertion `planar:init`; it is not an authenticated operator identity. Meta,
event 1 and the empty current projection commit together, with bound values and
the existing Glaze codec for the typed event payload.

The format opener traverses original parent components using retained no-follow
directory descriptors before normalizing the SQLite path. It retains a regular
file descriptor and compares device/inode identities before and after read-only
verification and on the writable transition. It checks the SQLite header,
application/version/encoding, exact named stored schema SQL including automatic
indexes and all thirteen triggers, integrity and foreign keys. Foreign keys ON,
synchronous FULL and trusted_schema OFF are session settings; existing-file
verification never assigns persistent journal/version settings. DELETE journal
mode and UTF-8 encoding are required. Both access modes retain a read snapshot.
The explicit recovery operation described below can write an existing ledger.

The internal read-only `open_existing_gated` additionally accepts a generic
`format_read_guard`. Its bind operation receives the actual no-follow held file
descriptor before the first raw header pread. The same object gates SQLite reads
and fetches through `connection::open_gated`. Format reads verify it before and
after even cached SQL results, inspect SQLite's sticky status, and retain the first
format-boundary refusal. No writable transition is exposed by this opener.
`format_file::gate_status()` does not allocate; cached `metadata()` remains
inspectable after refusal and does not authorize further facts.

The internal `extension_invocation::prepare_history()` composes that boundary with
its original moved profile capture and admitted `extension.ledger` receipt. It
uses only the private prepared selector and path, verifies the actual held
descriptor before reading, and owns a duplicate without reopening or recapturing.
Native history conversion runs without Lua. The complete format snapshot,
transaction and connection close before the final original receipt seal; the
duplicate closes after the final verifier use and before return. The native
history envelope stays private. Original profile diagnostics, format failures and
history misses retain separate typed alternatives. Preparation is once-only and
failed owners remain terminal. The C4-B3 candidate admits a history-only handler
through lazy `context.read_history({})`. The synchronous native bridge returns only
after this helper closes, then freezes its private facts for Lua. Exact raw fact
comparison precedes response copying; original receipts are checked before release.
Pure handlers remain supported; role/projection services and X06 profile
interpretation/shared accounting remain open.

Initialized, record-only and accepted-history states are supported. Initialized
state has exact singleton metadata/current state and initialization event, with
empty revision/request tables.
The streaming validator decodes and hashes every bounded record, checks stored
kind/codec, exact ordered directory edges, older child IDs and child kind/digest,
and refuses non-directory edges or any unequal member of a digest bucket.
Only one record and bounded comparison buffers are retained at a time; total
history is not materialized. Complete accepted-history verification is described
below. Any journal/WAL/SHM sidecar returns `recovery_required` before SQLite opens
the file. There is no writable fallback after a read-only refusal. Explicit
recovery is separate.

`format_file::read_record` returns owned typed data from the retained verified
snapshot. Every failed read returns its original typed error and permanently
invalidates further reads, including invalid-ID and not-found refusals. This
prevents a storage error that rolls back SQLite from silently starting a newer
snapshot. Moves transfer this validity state; cached immutable initialization
metadata remains accessible. Resources release normally through RAII; a non-storage
refusal need not release the retained read lock immediately. An explicit reopen
is required for a fresh verified snapshot. `begin_records` independently performs the safe read-only/writable
transition, releases its own read snapshot, acquires `BEGIN IMMEDIATE`, and
revalidates before exposing a `record_writer`. It never promotes an existing
reader. The writer can read and intern all four record kinds in one transaction,
with atomic acceptance composition described below. Interning validates child
references and scans every digest-bucket member for exact encoded equality;
directory local child IDs participate in equality even though portable hashes
omit them. Unequal candidates return `hash_collision`. New IDs use guarded
signed-64 `MAX(id)+1` under the writer lock. Ordered edges and records commit
together only on explicit `commit`, after full revalidation. Every failed writer
operation, failed commit or abandonment invalidates and rolls back the complete
transaction. Owners are move-only and caller-thread confined; no SQL or raw
handles escape. Separate reader snapshots can block DELETE-mode writer commits;
a busy commit fails and rolls back rather than claiming partial success.

Creation uses a fresh exclusive 0700 sibling staging directory, retained directory
identity, a committed/closed database, and file/directory sync. POSIX `linkat`
publishes the regular database as a hard link atomically without replacing any
existing target, followed by parent sync; this requires a filesystem supporting
hard links and durable file/directory sync. Failure never falls back to an
overwriting rename. Cleanup checks owned staging directory identity and owned
regular entries before unlinking only its database/journal and directory. After
publication, cleanup or sync failures return `publication_uncertain` and never
remove the published target. Failures before publication report incomplete
cleanup if necessary. Cooperative directory ownership remains a precondition;
these checks do not defend against a hostile path swapper.


Backup and restore require a quiescent source and a new explicit target. Both
validate the source in a retained read snapshot, use SQLite's online backup API
into exclusive owned staging, close it, sync it, and revalidate full supported
state and exact metadata before publication. They share initialization's early
and late target-sidecar refusal and no-replace publication/cleanup. Identity,
workspace and creation time are preserved. No arbitrary file-copy restore,
overwrite, ambient path discovery or CLI is provided. Typed storage errors include
`busy` and `storage_full` where SQLite reports them.

`recover_quiescent` requires callers to close all external writers for the whole
operation and own the directory cooperatively. This is an explicit precondition;
no automatic quiescence guarantee or external-writer coordination is claimed.
It retains no-follow regular main/journal descriptors, rejects WAL/SHM entries,
and checks main application/version and hot-journal magic and dimensions. Before
creating scratch or opening SQLite for recovery, a fixed eight-byte tail read
refuses the SQLite super-journal footer magic. This single-database boundary
does not interpret or open any encoded external pathname; malformed or coincidental
matching footers are conservatively refused as well. Bounded
buffers copy both files into exclusive 0700 scratch, retaining immutable captured
inputs beside a working pair. SQLite alone recovers the working pair; exact schema,
integrity, foreign keys, complete supported state, recovered size and journal consumption
must pass before any writable original open. Main/journal identity and exact bytes
are then rechecked, along with competing sidecars. SQLite recovers the original,
which is fully verified again against the scratch metadata and synced. No VFS lock
method is called outside SQLite's pager. Refused scratch validation never mutates
the original pair. Failures after original recovery begins may have completed
rollback; they do not report success or attempt to replace original bytes.

A genuine hot journal can be redundant when death occurs before its first main
page write; recovery need not change main bytes. Header/consumption checks and
SQLite interpretation establish this bounded recognition policy. SQLite rollback
journal checksums are not authentication, and scratch validation cannot establish
the provenance of arbitrary malicious journal bytes. Accepted-history verification also applies to recovery. Historical R12-C1
functional/crash evidence preceded R12-C2 broad I/O/full-storage/concurrency fault
injection and integrated gates; see the preserved round records.

`planar.engine.history.record` is a pure leaf above canonical values and pinned
xxHash, with no ledger dependency. It encodes and decodes file, directory, value
and request records under the 64 MiB whole-record and 16 MiB file/value bounds.
File bytes remain arbitrary binary. Directory mode is fixed at 0755; portable
ASCII child names are sorted, unique and case-alias free. Stored bytes retain
positive signed-64 child IDs, while portable digest preimages omit exactly those
IDs. Raw seed-zero XXH3-128 digests use canonical sixteen-byte order, without
changing render-domain recipes. Local framing accepts defined child kinds;
record existence, matching references, parent ordering and tree membership
remain contextual history policy.

Value envelopes validate nonempty UTF-8 codec identifiers and currently support
only `canonical_v1`; unknown identifiers return `unsupported_codec` before
payload interpretation. Canonical decoding preserves arbitrary string bytes and
empty map/list distinctions, validates UTF-8 non-NUL sorted unique map keys and
signed-32 integers, and clamps depth/node/encoded-byte ceilings before allocation.
Request arguments retain exact UTF-8 spellings; argument normalization, operation
semantics and ledger state are outside the codec. Codec operations have no shared
mutable state and return redacted typed failures. These primitives do not enable
populated-history opening, acceptance or new CLI commands.

`planar.engine.ledger.types` is an inert leaf containing the exact ledger error
vocabulary and record DTOs, importing only std and the pure record codec.
`ledger.format` exports explicit aliases, preserving existing caller names and
error values while retaining all owner/access/metadata policy.

`planar.engine.history.tree_core` depends downward on those DTOs and the portable
path validator. It owns the shared bounded validation, trie construction, digest
and reconstruction algorithms. Synchronous typed per-call reader/interner
callbacks carry only leaf DTOs and exact ledger failures; the core retains no
callback state and owns no storage, SQL, transaction, retries or commit.
Its independent test target links no format, SQLite, runtime or embedded Lua.
The `history.tree` module re-exports core DTOs and `digest_tree`, and adapts
existing writer and pinned reader owners to the core. Writer adapters create
the abort guard before any allocation or callback conversion; readers call
`read_record` on the original owner, preserving sticky failure semantics.

`build_tree` accepts sealed one-root path/payload views and normalized numeric
modes. It emits only the root and file ancestors, sorts names by ASCII
bytes, and refuses duplicates, case aliases and file/directory conflicts. Roots
are exact typed directory references, including the digest. Reconstruction owns
all returned bytes, sorts logical files, refuses non-tree children and non-root
empty directories, and counts shared subtree occurrences separately.

Source budgets are 64 MiB, 4096 files and 8192 directories; provenance independently
uses 16 MiB and 4096 files. Both use depth 64 and paths of at most 1024 bytes, with
root at depth zero and counted as one directory. Provenance's directory ceiling
is the induced `1 + 4096 * 63`, rather than the source directory ceiling. Build
scratch stores local names; traversal retains one decoded record at a time and
bounds pending occurrences before allocation. Payload limits apply across the
expanded tree rather than unique stored records.

Tree writes compose in a caller-owned `record_writer` with other records and
never commit implicitly. Every tree failure or exception aborts the entire writer,
including earlier pending inserts. Explicit `abort() noexcept` is idempotent and
releases transaction resources through existing RAII abandonment; it does not
promise additional rollback-error reporting. Tree errors retain original typed
ledger errors. Historical tree reads use recorded bytes and require no current
Lua interpretation. Higher-level acceptance, history and export compose these
primitives as described below.

`digest_tree` uses the same bounded validation, trie and original record hashing
as `build_tree`, with positive placeholder IDs omitted from portable preimages.
It computes identity before a transaction without opening a ledger. Writer paths
retain complete abort-on-refusal and exception behavior.

`planar.engine.history.provenance_codec` prepares owned sorted files plus that
pure root digest, depending only on tree_core and canonical/value leaves. Registry provenance is exactly captured `input.json` bytes and its
normalized mode. Embedded stdlib provenance retains ordered originals at portable
`src/stdlib`-relative names, all mode 0644, plus two explicit synthetic files:
`__planar__/assembly.lua` is the exact runtime chunk and
`__planar__/assembly.canonical` is a canonical_v1 map with exactly `version:1`,
`scheme:"ordered_concat_v1"`, ordered original path `files`, and exact `separator`.
Every original is followed by separator, including the last; generated embedding
uses LF and preserves the existing chunk. Source boundaries, ordering and separator
therefore remain part of identity even for equal concatenated chunks.

Preparation verifies assembly without constructing a second chunk and applies
the provenance 16 MiB/4096-file ceilings to originals and both synthetics together.
Count/path/separator and payload sums bound metadata scratch before canonical
encoding; the combined encoded total is checked before file payload ownership.
Reconstructed stdlib sets decode and validate this fixed manifest without Lua,
refusing reserved namespace aliases, duplicate/unsafe paths, missing/extra files,
unsupported or malformed manifests, inconsistent chunks and modes other than 0644.
The higher `history.provenance` wrapper re-exports this codec and the existing
owner-backed tree convenience APIs. Only its `prepare_embedded_stdlib` imports
generated source; original views use embedded bytes without runtime source reads.
The codec has no format, SQLite, runtime, embedded-source or Lua dependency.

`planar.engine.history.interpretation` prepares complete recorded interpretation
before any transaction. It internally prepares the exact embedded stdlib and
calls only the fixed `interpret_bundle` export. Trusted `acceptance.lua` composes
existing validation (which already normalizes), uses `not_captured` source
topology, preserves complete normalized and semantic values, and sorts both job
lists by ID. Registry provenance remains separately prepared from the same
captured registry bytes/mode; the complete normalized parsed registry is inside
the bundle. Neither preparation adds database, filesystem or Lua capabilities.

The owned result contains immutable interpretation/versions values, their exact
canonical bytes (each bounded to 16 MiB), and prepared embedded stdlib provenance
for native acceptance transaction composition. The fixed ten-key versions map includes the
internally computed portable stdlib digest; callers cannot supply that digest.
The ninth embedded file deliberately changes stdlib identity while existing
render/work recipes and packets remain unchanged.

The pure `history.interpretation_codec` owns recorded types/errors, the single
supported-version table, fixed envelope checker, historical decoder and
`encode_interpretation`. Its encoder accepts bundle/semantic values and a digest
as recorded data, assembles the original envelope and finishes through that same
decoder; it makes no authenticity claim about the supplied digest. The live
`history.interpretation` wrapper re-exports the codec, retains runtime invocation
and typed live diagnostics, and passes only its internally derived digest.
The codec depends only on canonical/value, with no storage or Lua link.

Historical decoding validates canonical framing, exact envelope/version keys and
types, digest spelling, normalized/semantic container structure, and sorted
paired jobs with matching IDs/files. The separate versions record must equal the
envelope's versions byte for byte. Recorded policy identifiers may be unsupported,
which differs from malformed structure, excessive resources, live declaration
validation and runtime failure. It does not execute Lua, compare old stdlib
identity to today's, or rerun current declaration/ownership policy. Historical
schema extensions remain data. This adds no load transaction, populated history
activation or CLI command.


`planar.engine.ledger.history_codec` is a pure typed boundary for fixed v1 event
payloads and saved load results. It depends only on the Glaze structural JSON
adapter and its leaves, with no format, SQLite, filesystem, Lua or runtime edge.
Enums, optional signed-64 heads and named record references encode initialization,
accepted revisions and no-op. Version and result kind are codec constants.
Exact field/type/state checks, positive decimal-string IDs, ledger/request grammar
and canonical Glaze re-encoding enforce stored bytes under one MiB without LF.
Failures expose only malformed, unsupported-version or resource categories.
A result parent is the returned revision's parent, including no-op. Cross-row
reference agreement belongs to format verification; R14-B2 integrates it with
complete accepted-history and deep referenced-content validation below.

R14-B2 integrates those codecs into the private shared format verifier. Ordinary
read and writable open, record-write acquisition and precommit, staged backup and
restore, and scratch/original explicit recovery all verify complete accepted
history. A chronological SQL cursor retains only the current head, its parent,
five references and counters. Accepted revisions start at 1, then increase with
the immediate prior head as parent; event sequences are gap-free. Record and
later revision IDs may have gaps. No-op results retain the existing revision's
parent, and timestamps have validated syntax without imposing clock order.
Counts and reverse request/event agreement reject orphan or multiply accepted
history; actor/time, request body, result, event, revision and projection links
must agree. Identical active content cannot allocate another revision, including duplicate
record IDs with exactly equal encodings. Hashes alone never decide this equality.

Each accepted revision reconstructs source, registry and stdlib independently,
releasing each bounded tree before the next. Registry is exactly `input.json`;
stdlib must satisfy its fixed assembly manifest and recorded digest. The pure
interpretation decoder checks canonical bytes, fixed shape and separate versions;
workspace and stdlib metadata must match the ledger and provenance. A well-shaped
unsupported interpretation policy remains available as raw records only after
these integrity checks. Unknown event/result versions refuse with
`unsupported_state`; malformed persisted data is `corrupt`. The format depends
only on pure tree/provenance/interpretation codecs below owner wrappers, with no
Lua execution or filesystem recapture during history verification. The native
acceptance writer and history CLI compose this verifier as described below.

Historical integrity verifies complete bounded stored topology and fixed recorded
relationships. It does not rerun Lua or rederive the semantic capture closure
using current policy. Exact declaration-closure capture belongs to acceptance
preparation and its writer; stored bytes do not prove semantic derivation.

The verified format owner also exposes bounded native historical reads on its
existing pinned snapshot. `read_head` returns the active revision and last event;
`read_revision`, `read_event` and `read_request` return owned optional rows, with
valid missing keys represented as successful misses. `read_events_after` returns
ascending events strictly after a signed-64 cursor (zero starts before event one)
with an explicit limit of 1..100. Its optional continuation cursor is present only
when this same snapshot has more events. No all-history cache or SQL handle is
exposed. Invalid arguments, storage failures and decoding failures invalidate all
read methods; immutable cached initialization metadata remains available. Existing
`read_record` missing-record failure semantics are unchanged.

Event payloads, saved successful results and request codec bytes are copied
exactly from storage after bounded typed validation. Request records contain
private host argument bytes and are native replay inputs; public history must
never emit those arguments. Higher callers reconstruct state at an event by
purely decoding its after-state and reading that revision. Raw history remains
available for a structurally verified unsupported interpretation version without
rerunning historical policy. Under the supported DELETE journal policy a pinned
reader blocks a concurrent commit; releasing it allows the commit, and a fresh
verified owner sees the later history. Reads do not alter schema or journal mode.

R14-C2 adds `capture_session::finish_for_acceptance()` beside the existing render
`finish()`. Successful sealing transfers the original root descriptors, file
receipts, registered tree membership and directory identities to an opaque,
move-only `acceptance_capture`. The capability retains the exact immutable file
snapshot and logical topology independently of the discovery session. A
transferred session refuses further discovery or sealing; moved-from capabilities
return `capture_changed`. Destruction and move assignment release owned roots.

`acceptance_capture::verify()` repeats the native root, membership, directory and
file comparisons without Lua, callbacks or replacing published values. It checks
exact bytes, file identity and normalized executable mode (`0644` or `0755`), not
raw permission equality across separate checks. Registered trees retain their
existing exclusions; standalone registry inputs retain their file and parent
identity without enumerating the parent's unrelated contents. The sealing seam
is discarded before transfer. Both successful and failed verification preserve
the original snapshot and topology; a failed check does not poison later retries.

R14-D2 retains private directory identity receipts for the workspace-relative
ancestors traversed by successful selected-file reads. Shared ancestors are keyed
by logical root and portable path; inconsistent discovery reads refuse. Initial
sealing and repeated verification compare these receipts through retained root
descriptors with no-follow traversal. They inspect identity only, so unrelated
sibling bytes, membership, types and permissions do not affect selected closure
acceptance. No unrelated directory enumeration is needed. These receipts do not
populate public directory topology; whole-tree capture keeps its separate full
membership and topology checks. Receipt storage uses a separate instance of the
existing 100,000-entry/64 MiB enumeration accounting and holds no ancestor file
descriptors between operations. Verification does not mutate aliases or receipts.
Host ancestors above registered roots retain the existing root-reopen contract.

The native acceptance writer requires this capability and invokes its native
check immediately before every commit attempt. Capability possession alone does
not establish a current check. Capture itself adds no command. Cooperative
changes detected during comparison produce `capture_changed` without host paths;
it does not prevent writes after a check or guarantee detection of changes that
are reverted before comparison.

R14-C3-A places native discovery and retained acceptance receipts in
`planar.engine.capture.native`, depending only on `planar.engine.value`. Both
private receipt implementations and the full validating seal live in that module.
The existing `planar.engine.capture` API re-exports native values and composes
`native_capture_session`; it retains declaration loading and delegates capture
operations. Native consumers can construct and verify real receipts without
linking the declaration loader, Lua runtime, or ledger. This extraction changes
no capture algorithms, refusal policy, seal seam, or public facade behavior.

`acceptance.prepared` composes only native capture and the pure tree, provenance,
interpretation and record codecs. Its move-only prepared owner selects nonempty
unique portable paths from the sealed `workspace` snapshot, stores only those
exact bytes and modes, and requires exactly `registry/input.json` for registry
provenance. Additional logical roots are not declaration sources. External
stdlib files and canonical interpretation/versions are reverified, including
supported versions and stdlib digest agreement. Immutable owned data includes
source/provenance digests and the structurally validated workspace identity;
semantic declaration closure remains a higher service responsibility.

Preparation performs no filesystem reads or receipt checks and remains possible
after captured files change. A distinct `verify()` delegates to the retained
native capability without replacing prepared data. The format module completes
acceptance through the transaction described below; preparation itself exposes no
writer. Immutable request intent separately
validates explicit already absolute lexically normalized UTF-8 arguments,
request/actor grammar and optional positive signed-64 head, retains exact fields,
and encodes the fixed `load` body with the shared record codec. It performs no
implicit cwd resolution or content capture. A future CLI must normalize arguments
explicitly before creating intent.


`ledger::complete_acceptance(path, intent, candidate)` and its identity-bound
receipt overload compose atomic native acceptance. It acquires the existing validated writable opener and
`BEGIN IMMEDIATE`, then rechecks identity and complete history under that lock.
An exact saved request body returns the original result bytes before any candidate
inspection or expected-head comparison. Different body bytes produce
`request_conflict`. An unknown request checks the locked head first
(`revision_conflict`), then the prepared owner and workspace (`invalid_argument`).
The later public service must also look up replay before doing capture work.

The transaction shares the record writer's private collision-safe interner and
uses storage-independent `tree_core` for all three roots. Complete encoded records,
including local directory child IDs, determine no-op equality. A no-op appends a
new event/request but retains its revision and that revision's parent; restoring
older content creates a new chronological revision. One guarded signed-64 allocator
protects record, revision and event increments. A native timestamp is created once
for the matching event/revision. Revision, event, request, saved canonical result
and current projection commit together. Precommit refusals, exceptions and
abandonment roll back tentative writes; failed COMMIT outcomes retain native evidence.

The exact response is encoded and owned before complete pending-state verification.
After that verification and metadata checks, the retained native receipt check runs
immediately before SQLite commit, with no intervening SQL, capture or Lua work.
A receipt refusal returns `capture_changed`. Response bytes remain buffered until
commit succeeds. There is no callback, alternate commit method or bypass flag.
Product format dependencies include native prepared/capture and pure codecs; Lua,
the loader and higher interpretation wrappers remain outside that link graph.
No public acceptance orchestration or CLI is added by this storage API.

V5-02-X06-C adds a third overload, `complete_acceptance(path, intent, profile)`,
taking the opaque runtime `prepared_profile`. It lives in the re-exported
`:profile` partition of `planar.engine.ledger.acceptance`, so callers keep the
single import. The embedded and profile overloads are thin adapters over one
shared transaction body in the non-exported `:seam` partition, whose entities sit
in a nested internal namespace that no exported type is associated with; code
outside the module can neither import nor name it, and a source-scan test confirms
that only the two adapters call it. The profile overload preserves the same
replay, conflict, format, CAS, error, receipt and uncertainty order as the
embedded one, checks the source and profile receipts immediately before COMMIT,
and runs no Lua while the write transaction is open.

The R14-D7-A overload takes the prelude's expected ledger identity and an initially
empty shared `completion_receipt`. It compares identity before locked request lookup,
refusing replacement with `unsafe_path`. Exact replay moves saved bytes into the
receipt and sets `replayed` before allocating the independent returned response.
Fresh accepted/noop results own saved bytes before COMMIT. The receipt enters
`commit_uncertain` immediately before COMMIT and becomes `committed` only on success.
The SQLite commit overload publishes confirmed noncommit without allocating: a
still-active failed COMMIT followed by successful explicit rollback proves noncommit;
automatic return to autocommit after failure alone remains uncertain. SQLite-owned
COMMIT diagnostics survive rollback and are converted to C++ errors only after that
observation. A stack guard reflects confirmed rollback in the receipt even if error
allocation throws. Retained phases/bytes survive every later error or exception.
Receipt reuse refuses while preserving its existing evidence; callers use a fresh
owner for a new attempt. The three-argument API retains its single response owner.

`planar.engine.capture.host_path` depends only on value diagnostics. Its explicit
file and directory resolvers share the renderer's existing physical traversal:
canonical file parents, no final file links, and no final-root link aliases,
including trailing slash/dot and child/.. spellings. Render passes original option
bytes; intermediate links and physical link/.. semantics remain unchanged.
Filesystem exceptions become the same redacted capture I/O diagnostic; allocation
failures propagate to the caller's boundary. Resolution remains cooperative.

The separate `normalize_intent_path(argument, explicit_cwd)` is a pure lexical
boundary for future load request identity. It requires nonempty non-NUL UTF-8 and
an absolute already normalized explicit cwd, normalizes absolute arguments or
joins relative arguments to that cwd, and bounds input, joined spelling and output
to 64 MiB before joining. It performs no filesystem or implicit cwd lookup.
Load must use the returned spelling both in request intent and actual capture;
render does not use this normalizer. The request factory still bounds the complete
encoded record separately. This extraction adds no load service or membership
policy.

The init handler passes the explicit target directly to `initialize_new`; it does
not use capture's existing-file resolver or normalize away parent components.
Native storage owns validation, staging, commit, no-replace publication and sync.
Only returned committed metadata enters the small init envelope. No SQL escapes
into command code. The shared response module depends on `ledger.types` for typed
redacted error mapping, without depending on the storage owner or SQL wrapper.
CLI11 owns required scalar options and duplicate refusal; the existing long-equals
preflight and parsed binding accounting preserve separate-token grammar for both
commands. Init only initializes a ledger; history and export are separate handlers.
No load service or CLI is exposed.


`history.read` composes public page/revision/request/state projections above
`ledger.format`, retaining one verified `format_file` snapshot for all reads.
Pure metadata and event conversion performs no I/O; the read service calls only
opaque typed snapshot methods and existing history/interpretation codecs. Its
imports exclude runtime, Lua, current policy and embedded code; format transitively
depends on native prepared capture types. Revision reads decode complete stored
values; request reads project only saved public results. CLI11 history bindings
live in `handlers/history/`; Glaze buffers the complete envelope with room reserved
for LF before stdout is written. Storage, schema and capture implementations are
unchanged by this command addition.

`planar.engine.source_export` reconstructs the selected revision's complete source
from one verified read-only `ledger.format` snapshot through `history.tree`, before
creating target-side entries. Its only product dependency is the immutable tree
wrapper and its lower native graph. Unsupported recorded interpretation policy
remains exportable; neither current Lua nor registry/stdlib injection participates.
`export_source` takes explicit ledger/revision/new-target arguments and returns
native identity fields. The CLI export adapter is implemented as described below.

Export retains a real no-follow target-parent descriptor, preallocates a bounded
ancestor/file plan and identity receipts, and creates an unpredictable exclusive
sibling stage. Files use exclusive no-follow creation, exact writes, normalized
0644/0755 modes and fsync; directories become 0755 and sync child before parent.
Traversal reopens and verifies recorded ancestors with descriptors bounded by tree
depth, rather than retaining one per entry. Source ceilings remain 64 MiB, 4096
files, 8192 directories, depth 64 and 1024-byte paths. The valid empty root is
supported. Original parent components are traversed before normalization, so
missing/.. cannot hide a missing directory.

Publication uses macOS `renameatx_np(RENAME_EXCL)` or guarded Linux
`renameat2(RENAME_NOREPLACE)` through the syscall interface. Missing platform or
filesystem capability refuses without an overwrite fallback. After rename, a
publication flag precedes parent sync and descriptor closure; any subsequent
failure reports publication uncertainty and preserves the complete target.
Before publication, cleanup removes only identity-matching invocation entries
through verified ancestor descriptors. Replaced/unrecognized entries remain;
cleanup-incomplete errors expose only the generated stage basename. Crash stages
are never adopted or deleted by later exports. This is a cooperative directory
ownership contract, without hostile-owner or hardware power-loss guarantees.

The `handlers/export/` command module (`planar.cmd.planar.handlers.source_export`)
attaches invocation-owned CLI11 options and passes their original path spellings
to the native source-export service. It projects only the returned ledger,
revision and source identities through the shared Glaze codec. The command owns
redacted export diagnostics and validates the generated cleanup-location basename
before exposing it. Storage refusals retain the shared ledger mapping.

The buffered response has an optional prebuilt stdout-write diagnostic. Export
prepares both this buffer and a post-publication uncertainty response before
calling the native service. Only successful export attaches the stdout context;
`main` writes these existing bytes when stdout fails. Encoding or allocation
failure after native success returns the prebuilt uncertainty response by move.
Other commands retain the existing stdout failure behavior.


## V5 architecture alignment pending

[V5](vernacular/planar-design-spec-v5.md) retains the engine/trusted-Lua/job trust
split and extends the intended Lua semantic/verb surface. Current command
registration is native CLI11 and current product Lua is embedded; no stdlib.d
host extension loader or orchestrator projection is implemented. V5-02 must review
exact bindings, registration, extension trust/provenance/identity, diagnostics and
workspace naming before load resumes. CLI11 parsing and thin handlers, Glaze JSON,
engine naming, product code under src and unbypassable native capture/conditional-
storage invariants remain operator requirements.

[Alignment](vernacular/v5-alignment.md) records unresolved mechanics and v5's
internal inconsistencies. RA01 adds reviewed authoring/observe contracts and initial
orchestrator graph/shape rendering after R14; R15–R18 extend guidance with actual
state/actions. R19 proves usable external-driver execution and the full applicable
[acceptance map](vernacular/acceptance-matrix.md). Incremental reconciliation must
distinguish semantic inputs, output-vector applicability and prerequisite quality;
Planar does not schedule sessions or manage checkouts. Composition, impact/roadmap
and controlled measurements remain after R19.

The internal `command_vm` registers exactly five shipped command callables from
`planar/commands.lua`, copies and validates the complete catalog, and seals its
canonical descriptors, handler/module identities and exact embedded bundle digest.
It retains callable registry references in the same VM. Registration is pure;
execution receives typed native intent with opaque path userdata and only its
ordered family services. Native providers retain storage/capture authority and
exact error/fact ownership. Every terminal invocation releases its Lua state,
references, handles and private option strings. Export publication state remains
available so composition can apply its preallocated uncertainty fallback.

This engine API is exercised with test-only providers and altered embedded bundles.
The CLI composes the retained command VM through independently approved V5-02-B2.
The [V5-02-C workspace-name/snapshot implementation](vernacular/rounds/V5-02-C-review.md)
is independently approved; load and external profiles remain pending. Fresh fixed-export VMs have no registration capability, so the new
module is inert there; its exact authored bytes participate in stdlib provenance.

Registration sealing walks retained Lua callables and trusted globals, following
Lua closure upvalues and table keys/values with memoized identity. Opaque C
closure implementation state is excluded. A fixed 256-frame native walk checks
live Lua stack space before pushes (2048 slots), charges every graph edge to the
existing cumulative node/native-byte budget, and refuses depth beyond 256.
The fixed trivial frames and Lua-owned memo/backing storage remain safe across
protected Lua errors; Lua allocation remains under the registration ceiling.
Tables retain their identity through private read-only backing tables. Native
read wrappers are installed before registration, preserving captured next/rawget/
pairs aliases, indexed reads, ipairs and length; native result/codec conversion
reads those same backings while preserving original tags. Invocation-local
scratch tables are created after sealing and remain mutable.

The command bootstrap retains CLI11 bindings through registration, native services
and encoding. Root help and usage refusals finish before registration. The complete
sealed catalog is compared with the compiled bindings; its selected same-VM callable
owns routing, policy and projection. `invocation.cppm` sits below all five handler
families and bootstrap, composing existing capture and storage primitives without
an upward dependency. Native services return typed values/errors directly; history
facts are compared against verified native results. Write locks are released before
Lua receives results. Export allocates both uncertainty diagnostics before invoking
its service and retains publication state in both the adapter and VM, including
allocation failure while preparing the first post-publication service result.


C prepares selected private invocation paths after successful catalog
admission and before native service creation. `capture.host_path` classifies the
workspace selector and expands valid names using only the explicitly supplied
absolute `PLANAR_WORKSPACE_DIR`; explicit files do not read it. Native invocation
preparation validates every selected path, captures cwd once only if required,
and makes every path absolute from that snapshot. No host spelling crosses into
Lua options or portable/public identities. Pure preparation returns preserved
absolute spelling for physical render traversal and a separate lexical target for
future load request intent. Existing `normalize_intent_path` retains its explicit
cwd contract. No load/replay service, runtime VM or storage format changes are
part of C; request-intent equality tests exercise only pure native framing.
Only render currently has a workspace selector.


The [R14-D3 load amendment](vernacular/load-command-contract.md) is a documentation
candidate pending independent review. It proposes the fifth mandatory catalog
entry, native replay prelude, three finite fresh-load services, typed authoritative
CAS/collision diagnostics and saved-byte transport. None of these load surfaces
is implemented today; the existing native transaction/receipt and v1 codec
boundaries remain the implementation foundation.


Native ledger errors own an optional finite detail variant. The acceptance write
transaction records expected/actual revision heads at its locked comparison,
after request relookup. The interner records collision kind and public digest;
request collisions retain only kind. Copies through tree callbacks and rollback
retain these values. The shared response adapter validates category, positive
heads and kind/digest pairing before canonical Glaze encoding; malformed detail
becomes the fixed internal protocol refusal. Generic errors keep their existing
bytes. No extra head read or native-message parsing supplies diagnostics.
R14-D4 is independently approved; this adds no load service or CLI.

R14-D5 introduces `planar.engine.acceptance.load`, an internal move-only owner
above capture and pure preparation. `open_workspace` accepts explicit absolute
lexically normalized workspace/registry paths, loads the workspace and referenced
text, and returns an owner whose one-shot `workspace()` supplies `{file,workspace}`.
`capture(route)` checks the fixed route, opened identity, sorted unique jobs and
portable declaration paths before loading the spec, every job and their referenced
text. The explicit registry is separately captured as `registry/input.json`.
It never enumerates a workspace tree or opens source/base labels, observations or
unrelated siblings. It seals with `finish_for_acceptance`, retaining the original
root, selected ancestor, file, mode and registry receipts.

`prepare(B,M,stdlib_files)` consumes the owner. It checks fixed interpretation
structure with the existing codec, declaration header/job/file correspondence and
all normalized text-reference paths/bytes against the retained closure. It derives
all sorted unique workspace source paths from the sealed native snapshot; callers
cannot supply source membership. Exact trusted-native stdlib provenance is
reverified and copied, then pure `encode_interpretation` and `prepared_acceptance`
produce immutable data with the original native verifier. No Lua or callback
executes in preparation or receipt verification. A failed operation is terminal;
consumption and moves leave the old owner unusable, while self-move preserves it.

This candidate checks structural correspondence, not whether semantic code ran.
The trusted caller still owns all defaults, normalization, schemas and graph
policy. Product load orchestration does not call `run_trusted`, look up ambient
embedded sources or supply a replacement semantic dispatcher. Tests invoke actual
shipped routing/interpretation and pass actual embedded provenance, but do not
prove same-VM binding. D6 must bind both actual handler execution and its original
provenance to the retained command VM. Public load remains absent until D7.


R14-D6 adds retained load protocol support behind private generated five-entry test
catalogs. Production still requires exactly render/init/history/export, and the
CLI admits no load. Pure registration seals bundle identity and retains immutable
views of that imported bundle's exact original files, separator and assembly.
Only a fresh accept_load call charges the provenance copies against the same
cumulative conversion budget and invokes the pure provenance codec, before the
native provider. Registration and ordinary families therefore bypass acceptance
provenance assembly, preserving the future native early-replay ordering. The
provider receives only a synchronous const span of those verified owned files;
Lua supplies neither provenance nor source membership. R14-D6-C01 corrects this
ordering and remains pending independent review.

The finite load provider receives retained typed options and exactly ordered
open_workspace, capture_load and accept_load calls. D5's native owner checks the
route, complete closure and normalized B/M correspondence; the actual retained
Lua handler calls route_workspace and interpret_bundle in its sealed VM.
Providers own preparation and transaction locks, releasing them before any Lua
result construction. Existing providers inherit fail-closed load methods.
The shared dependency-free ledger.completion receipt owns saved JSON and one of
not_completed/committed/replayed/commit_uncertain. Its owner survives terminal
VM cleanup and moves; phase assignment and owned-string move require no
allocation. Native providers must preserve receipt state on exceptions. The
runtime checks exact saved-response equality and exposes the const native receipt
to transport. D7-A produces real transaction receipts; native provider and public
transport integration remain D7-B work.

Private D6 test callables exercise real D5 capture/preparation with finite simulated
completion outputs. An altered retained callable changes interpretation and carries
its own exact commands.lua and provenance digest. These tests establish bundle
binding and protocol behavior, not native semantic certification or atomic load
integration. The D6 candidate still requires independent review.


The D7-B public load candidate adds a mandatory fifth retained callable. After
catalog validation, native code snapshots relevant environment/cwd once, frames
immutable lexical request intent and performs a verified read-only request lookup.
Exact replay bypasses capture and current policy; its read owner is released before
fresh capture or writable acceptance. The native provider uses D5 capture and the
same retained VM's D6 provenance, then passes the prelude ledger identity and the
VM-owned completion receipt directly into D7-A atomic acceptance. Native saved
bytes own the success transport. Preallocated load/stdout diagnostics and retained
receipt phase survive postcommit Lua, codec, allocation and process-write failures.
No Lua or adapter diagnostic encoding executes under the native write lock.


X03-B1 adds a native-only format2 new-target initializer and exact schema1/schema2
selection, retaining the verified format in ledger metadata through publication,
backup and restore. Existing public init creates format1. This is partial lifecycle
support independently approved for B1. B2-A independently approves codec1 and
codec2 embedded recorded-history reads under the selected exact schema. The approved B2-B+C01
implementation adds native format2 writes and explicit recovery with immutable
request-v2 intent, full-byte comparison inside codec/kind/digest buckets, and
locked saved-body replay before fresh-format/selector checks, CAS or candidate
access. Request digests remain private in collision diagnostics. Fresh profile
acceptance remains unsupported; no schema alteration or conversion is performed.


X03-A, X03-B1, X03-B2-A and the combined X03-B2-B+C01 native implementation
are independently approved; C01 closes B2B-R1. The independently approved [X03-B3 verification gate](vernacular/x03-format-evidence.md)
runs full debug and RelWithDebInfo gates: 430/430 each, zero failures/errors/skips,
pinned lint/Doxygen and no-work builds. [Independent B3 review](vernacular/rounds/V5-02-X03-B3-review.md) closes X03-B. Native format2 acceptance uses immutable request-v2 embedded
intent, codec-aware exact interning and explicit recovery under retained limits.
Public init still creates format1. Fresh format2 request-v1 and profile acceptance
refuse; profile accepted history remains unsupported until X06 provenance/
interpretation support. B2-A SQL reader fixtures provide no conversion tooling.
The mandatory external-profile gate, public cutover, Linux and execution remain open.

The [X03-B2-B-C01 correction candidate](vernacular/rounds/V5-02-X03-B2-B-C01.md) reuses complete metadata equality,
including the selected ledger format, at locked transaction entry and both native
precommit boundaries. Controlled verified-metadata fixtures exercise refusal and
rollback in both formats; they do not reproduce an external format change.
C01 independently approves the correction and combined B2-B candidate.
The [X03-B3 review](vernacular/rounds/V5-02-X03-B3-review.md) independently approves the full
430-case debug/release gate and closes X03-B; the external-profile gate remains open.

The [X04-A1 review](vernacular/rounds/V5-02-X04-A1-review.md) independently approves native metadata
admission and descriptor binding; its inventory is 451 (not a full 451-test run).
The [X04-A2 review](vernacular/rounds/V5-02-X04-A2-review.md) independently approves strict bounded
profile/ordered module capture and retained staged input receipts. The
[X04-A3 integrated native gate](vernacular/x04-capture-evidence.md) is independently approved by the X04-A3 review: debug and RelWithDebInfo each pass 489/489 with zero
failures/errors/skips, preserving all 488 prior names. No external Lua, public
profile CLI, provenance acceptance or whole-invocation budget sharing is inferred;
the mandatory external-profile gate remains open.

The [X04-B1 prerequisite candidate](vernacular/rounds/V5-02-X04-B1.md) extracts
the unchanged bounded profile decoder into the pure `capture/profile_codec` module
and adds the distinct 32 MiB/8192-file `effective_provenance` tree budget.
Focused debug verification passes 177/177 with zero failures/errors/skips; the
497-name inventory preserves all 489 approved A3 names. The independent
[X04-B1 review](vernacular/rounds/V5-02-X04-B1-review.md) approves these prerequisites.

The pure X04-B1/B2/B3 slices are independently approved. B2 validates and
immutably seals catalog-v2, including source owner ordinals, exact shipped native
signatures and external static capabilities. B3 assembles and structurally verifies
exact `effective_stdlib_v1` bytes, modes, ordered modules, old embedded assembly,
catalog owners and full membership. The catalog-only 1 MiB ceiling and combined
canonical manifest 1 MiB ceiling are separate checks. B1 tests the outer
32 MiB/8192-file tree budget; valid complete B3 inputs reach at most 4162 files
and remain below 32 MiB under the stricter component bounds.

The [X04-B4 integrated provenance gate](vernacular/x04-provenance-evidence.md)
is independently approved by the [B4 review](vernacular/rounds/V5-02-X04-B4-review.md): full Debug and
RelWithDebInfo each pass 520/520 with zero failures/errors/skips/disabled cases,
preserving all 520 prior names. Pinned formatting/Doxygen checks 113 active
sources; vectors, exact source audits and no-work builds pass. No product or test
bytes changed in B4, and tested sources remained fixed through both gates.
This pure structural gate adds no live external registration/Lua, public profile
CLI, storage/profile-history acceptance or history dispatch, whole-invocation
shared budgets, or callability/semantic-honesty proof. The mandatory external-profile
gate, public cutover, execution/RA01 and Linux remain open; public init creates
format1. The independent B4 review closes integrated pure X04-B.


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


X05-C1 adds a private pure assembly seam within `src/stdlib/planar/render.lua`,
shared by ordinary rendering with an empty custom list. Prepared headed base
sections remain verbatim; already admitted custom bodies append with current
heading/join framing. Assembly copies bookkeeping and supports immutable inputs.
The private projection adapter emits exact `{id,bytes,included}` metadata without
changing ordinary manifest fields. Native projection response admission, retained F
execution and read/receipt orchestration remain separate future boundaries; the
helper authenticates no caller data. The [focused candidate report](../tests/native/v5-02-x05-c1-evidence/REPORT.md)
records verification; independent review is pending.


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
their separate bounded metrics; whole-invocation shared capture/VM accounting stayed
open at X05 (historical: X06 composed it, see the composed-budget paragraph above). New wrapper/path staging charges the existing cumulative VM native
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

## Native gated SQLite connection prerequisite

`ledger/sqlite` owns a downward native `read_guard` interface without capture,
profile or runtime dependencies. `open_gated` retains the caller's shared guard
in stable per-connection VFS context. The verifier may throw; noexcept callback
adapters translate allocation/length failures to resource refusal and other
exceptions to verifier refusal. Only ready/refused/resource verifier results are
recognized; other values fail closed. Original adapter diagnostics stay with the
adapter. The connection's allocation-free `gate_status()` and trailing typed
`sqlite_error::gate_status` preserve first-failure priority independently of SQLite
error strings, including the null-handle open failure path.

A fixed 64-byte address-derived VFS name, fixed context and fixed per-file wrapper
introduce no growing application cache or buffer. The native allocations are one
shared SQLite-state/control block and one context allocation, plus SQLite-owned
`sizeof(file) + alignof(max_align_t) + base.szOsFile` file allocations, with checked
integer size arithmetic. The caller's existing shared guard is retained without
copying its receipts. The SQLite/VFS primitive itself charges nothing; the
composition with VM, native and capture budgets happens in the factory budget
owner described earlier.

The VFS is explicitly nondefault, carries direct stable pAppData, and forwards
using the actual base VFS/file pointers. File method versions and optional null
callbacks follow the base. It admits one read-only main open, gates main xRead and
actual xFetch, refuses main writes/truncation, journal/WAL opens and shared-memory
mapping before delegation, and forwards legitimate temporary-file operations.
Gated SQL denies ATTACH/DETACH before alternate-file access. HAS_MOVED supports
cooperative binding; it is not hostile substitution or change-and-revert proof.
Original receipt and held-descriptor integration belongs to the following slice.

Statements/transactions retain sqlite_state; each finalizes or rolls back before
releasing it. A small online-backup RAII owner now also finishes the backup if
copying its pre-finish diagnostic throws. Final state destruction closes SQLite
before unregistering the VFS and releasing its guard. A BUSY close or unregister
failure at that internal destructor is a fatal ownership invariant violation,
not a supported retry protocol; concurrent SQLite shutdown is unsupported.
No global routing, TLS, retirement queue, immutable URI or database copy is used.
