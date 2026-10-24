# Ledger foundation contract v1

Status: R11 design independently approved; R12 storage, R13 immutable records,
trees/provenance and R14 native acceptance plus init/history/export components are
implemented and independently approved. Current commands are render/init/history/
export; render is database-free. No load service or CLI is exposed. Closure-only
load membership is selected; remaining integration and full R14 acceptance are pending.
See [D2 review](rounds/R14-D2-review.md), [C approval](rounds/V5-02-C-review.md)
and [the roadmap](roadmap.md). The [R14-D3 load amendment](load-command-contract.md)
is an exact documentation candidate pending independent review, not implementation.

[V5](planar-design-spec-v5.md) is the product-direction baseline, subject to explicit
operator decisions. This approved exact contract and [declaration v1](declarations.md)/
[render v1](render-contract.md) retain authority until scoped reviewed amendments.
The historical v4 rationale below records R11's origin; planned destinations and
command surfaces below are not proof of implementation. [Alignment](v5-alignment.md)
records accepted V5-02 A/B/C and separates later execution obligations. The
[exact command/workspace amendment](command-contract.md), embedded bridge
V5-02-B1/B2 and naming V5-02-C are independently approved and precede remaining
load integration. It does not register or expose load today.

## Scope and authority

The v4 sections “Working files and accepted revisions”, “Built-in Merkle tree”
and “Recorded execution trajectory” require explicit acceptance, complete trees,
exact collision checks, chronology, CAS, request replay, reconstructable history,
transactional projections, retention and safe format detection. See the
[v4 design](Planar%20design%20spec%20-%20v4.md) and acceptance A20, A23, A25–A29.
The concrete format, limits, SQL, command spelling and backup procedure here are
R11 implementation decisions, rather than previously implemented v4 APIs.

This foundation stores declarations and their interpretation, with one active
revision per ledger. It has no roles in execution, claims, run tokens, checks,
processes, sessions, scheduling, completion or approval. All declared jobs are
validated and stored; load does not select an active execution job. Later execution
rounds must extend the format and atomically reconcile role generations with
revision activation before enabling execution. No partial execution state is
pretended here. Acceptance means declaration activation, not output approval.

There is one explicit ledger path and one logical workspace per database. No
search, environment default, legacy database fallback or implicit initialization
is allowed. Editable files remain drafts. Reads and render do not publish them.
The ledger is authoritative for accepted history; source worktrees remain external.

## Boundaries and implementation destinations

SQLite owns transactions, constraints, prepared statements, BLOB persistence,
locking and online backup. Restore the previously pinned public SQLite 3.53.3
amalgamation in R12 through `cmake/dependencies.cmake` and committed `vendor/`
sources, after verifying URL, SHA256 and license against its historical pin.
Never restore legacy database openers, automatic migrations or command wrappers.
No SQLite JSON functions replace Glaze, and no handwritten JSON codec is added.

Proposed downward module edges are:

- `src/engine/ledger/`: SQLite ownership, typed failures, exact schema/opener,
  transactions, invariant checking and backup primitives; no declaration policy.
- `src/engine/history/`: versioned binary record codec, immutable tree interning,
  exact reconstruction and export; uses capture, hash and ledger.
- `src/engine/acceptance/`: finite capture/interpret/commit orchestration; uses
  existing restricted loader/runtime and the two modules above.
- `src/stdlib/planar/acceptance.lua`: trusted normalization/validation and the
  versioned stored interpretation record, using existing schema/semantic policy.
- `src/cmd/planar/handlers/{init,load,history,export}/command.cppm` and
  `command.cpp`: CLI11 option state retained through dispatch. Shared response
  helpers remain below handlers and bootstrap; no handler/bootstrap cycle.

These are proposed paths, not pre-existing modules. Colocated Catch2 tests own
scratch roots. Product code stays under `src/`; fixtures and test harnesses stay
outside. Native entry points return `std::expected` typed boundary errors.

Trusted Lua gets sealed captures and bounded values, never a database handle,
SQL, clocks, host paths, filesystem reads or a new ambient capability. Native
orchestration calls separate existing normalization and semantic validation phases
for every job, then stores their complete typed output. Lua owns schema and
interpretation; native code validates envelope structure, version identifiers,
limits, references and atomicity. Do not rerun Lua while holding a write lock.

The stored `interpretation_v1` map has exactly `version:1`, `bundle`, `semantic`,
and `versions`. `bundle` is the complete successful `normalize_bundle` value:
`workspace`, `spec`, `jobs` (each `{file,value}`, sorted by value.id), `registry`,
`workspace_file`, `spec_file`, `registry_file`. These are existing normalized
schema outputs including captured text values and complete parsed trusted
registry. `semantic` has exactly `dependencies` and `jobs`, copied without field
loss from successful `validate_bundle` output, with jobs sorted by ID. Load
validates with source topology `{status:"not_captured"}`: implementation sources
are not declaration authority. Capture topology is not stored as interpreted
source output. All dependency/ownership declaration checks still run.

`versions` is exactly `{declarations:"declarations_v1",
normalization:"normalization_v1",semantics:"dependencies_v1",
selection:"selection_v1",normalizer:"exact_bytes_v1",canonical:"canonical_v1",
hash:"xxh3_128_v1",interpretation:"interpretation_v1",
projection:"projection_v2",stdlib_digest:<32 lowercase hex>}`. These metadata
versions record existing policy plus new envelope/normalization names; they do
not change W or existing render recipes. `stdlib_digest` is the portable
record_v1 digest of the exact embedded trusted stdlib tree. Projection version
is context and has no effect on work fingerprints under the render contract.
R13 pins byte fixtures for these output structures. Any changed record shape
requires a new interpretation version. Database-local record IDs are separate
revision foreign keys, never inserted into portable semantic/version values.
The exact trusted stdlib bytes and exact registry bytes are also stored as
immutable provenance trees; names or digests alone cannot recover their content.
Historical interpretation is decoded using its recorded codec, never silently
regenerated under current Lua. Keeping these bytes does not promise execution
of old trusted code. Source anchors are
[schema.lua](../../src/stdlib/planar/schema.lua),
[semantic.lua](../../src/stdlib/planar/semantic.lua),
[selection.lua](../../src/stdlib/planar/selection.lua) and
[capture.cppm](../../src/engine/capture/capture.cppm).

## Identities, binary records and limits

SQLite INTEGER IDs are positive signed 64-bit numbers, max 9223372036854775807.
At JSON, Lua and canonical_v1 boundaries every database ID/sequence is a canonical
decimal **string**, `[1-9][0-9]*` within that maximum; absence is a separate
`"none"` head token. No sign or leading zero is accepted. Existing signed-32
canonical value bounds, W domains and render hashes do not change. Content IDs
are database row IDs, distinct from chronological revision IDs and digests.
IDs have meaning only with the ledger identity.

`record_v1` is a separate binary storage encoding, not JSON or a change to
canonical_v1. All integer fields below are unsigned big-endian; `u32` is four
bytes, `u64` eight bytes. `bytes(x)` is `u64(length(x)) || x`. No trailing bytes,
unknown tag, noncanonical length or truncation is accepted. Hash is seed-zero
XXH3_128bits, canonical 16-byte representation. For file/value/request records
the digest preimage is the complete encoding. For directories it is the same
encoding with every u64(child record ID) omitted: sorted names, kinds, metadata
and child digests determine its portable Merkle digest. Stored encoding retains
the exact IDs for collision-safe equality and foreign-key integrity.

| Kind | Exact encoding after the common ASCII bytes `PLANAR-RECORD` and byte `01` |
| --- | --- |
| file (tag `01`) | tag, u32(normalized mode), bytes(exact payload) |
| directory (tag `02`) | tag, u32(493), u32(child count), then each child: bytes(ASCII segment), one-byte kind, u64(child record ID), 16-byte child digest |
| value (tag `03`) | tag, bytes(codec ID UTF-8), bytes(encoded typed value) |
| request (tag `04`) | tag, bytes(operation ASCII), bytes(actor UTF-8), bytes(expected head token ASCII), bytes(workspace argument UTF-8), bytes(registry argument UTF-8) |

Directory children sort by unsigned name bytes, unique with the declaration
case-alias rule. Child references always name an already inserted immutable
record of the stated kind/digest. Record IDs are local; tree digests are portable for equal source bytes, names
and normalized metadata under record_v1. Portable render identities remain those
already specified by render v1. Equal directory digest is insufficient for reuse:
exact child-record IDs and metadata must also match, or collision refusal applies.

File mode is exactly 420 (`0644`) or 493 (`0755`), preserving the existing
normalized executable-bit semantics, not host ownership, ACLs, xattrs, timestamps
or other permission bits. Directories have normalized mode 493. Raw file payloads
are BLOBs and may contain arbitrary bytes; declaration/text/schema inputs retain
the stricter UTF-8 and NUL rules. Binary content never crosses the JSON/Lua value
adapter as a string. Value records use `canonical_v1` for interpretation and
version records; decoding must preserve map/array distinctions and enforce its
current signed-32 numeric domain. Result/event JSON uses Glaze and the canonical
JSON spelling already implemented, UTF-8 bytes plus no terminal LF in storage.

| Limit | Foundation maximum / behavior |
| --- | --- |
| Declaration and referenced text | Existing loader/capture/schema limits apply unchanged; the lower applicable bound wins |
| Captured source set per load | 64 MiB total payload, 4096 files, 8192 directories, depth 64; portable paths max 1024 bytes |
| Registry and trusted stdlib provenance | Separately 16 MiB and 4096 files each; same path/depth rules |
| Single file BLOB | 16 MiB, additionally subject to existing declaration/text limits |
| Interpretation value | 16 MiB encoded; existing Lua memory/instruction/depth/conversion ceilings still apply |
| Request ID / actor | 1–128 ASCII `[A-Za-z0-9_.:-]` bytes / 1–256 valid UTF-8 bytes without NUL; actor is a driver assertion |
| Event/result JSON | 1 MiB each; decimal-string IDs; no binary payload |
| History page | 1–100 entries, default 50; never unbounded output |
| SQLite busy wait | 5000 ms; timeout is a typed busy failure, no silent retry loop |
| IDs, sequence, history storage | Refuse before signed-64 exhaustion; no automatic pruning or total-history cap |

Native bounded decoding validates lengths before allocation and limits aggregate
capture plus encode scratch; do not multiply limits through recursive expansion.
Read/export validates a bounded complete reachable tree before side effects;
reject depth, file-count, payload or reference corruption, including cycles.

## New format and exact schema

Format v1 uses SQLite `application_id = 1347178034` (hex `504c4e32`, ASCII PLN2),
`user_version = 1`, UTF-8 encoding, foreign keys ON, journal mode DELETE,
synchronous FULL and trusted_schema OFF. Use local filesystems supporting SQLite
locking and durable file/directory sync; unsupported network filesystems are not
a supported durability environment. No WAL/SHM or deferred write-behind policy.
A successful commit is subject to SQLite and the host honoring fsync; no stronger
hardware, tamper, encryption or adversarial-writer guarantee is claimed.

All tables are STRICT; BLOB equality is byte equality. SQL below is normative.
The opener verifies the complete expected schema, including triggers and indexes,
before enabling writes; matching two pragma numbers alone is insufficient.

```sql
CREATE TABLE ledger_meta (
  singleton INTEGER PRIMARY KEY CHECK(singleton=1),
  format INTEGER NOT NULL CHECK(format=1),
  identity TEXT NOT NULL CHECK(length(identity)=32),
  workspace_id TEXT NOT NULL,
  created_at TEXT NOT NULL
) STRICT;
CREATE TABLE records (
  id INTEGER PRIMARY KEY CHECK(id>0),
  kind INTEGER NOT NULL CHECK(kind BETWEEN 1 AND 4),
  codec INTEGER NOT NULL CHECK(codec=1),
  digest BLOB NOT NULL CHECK(length(digest)=16),
  encoded BLOB NOT NULL CHECK(length(encoded)<=67108864)
) STRICT;
CREATE INDEX records_digest ON records(codec,kind,digest);
CREATE TABLE tree_edges (
  parent_id INTEGER NOT NULL REFERENCES records(id),
  ordinal INTEGER NOT NULL CHECK(ordinal>=0),
  name TEXT NOT NULL,
  child_id INTEGER NOT NULL REFERENCES records(id),
  PRIMARY KEY(parent_id,ordinal), UNIQUE(parent_id,name),
  CHECK(parent_id>child_id)
) STRICT;
CREATE TABLE revisions (
  id INTEGER PRIMARY KEY CHECK(id>0),
  parent_id INTEGER REFERENCES revisions(id),
  source_id INTEGER NOT NULL REFERENCES records(id),
  registry_id INTEGER NOT NULL REFERENCES records(id),
  stdlib_id INTEGER NOT NULL REFERENCES records(id),
  interpretation_id INTEGER NOT NULL REFERENCES records(id),
  versions_id INTEGER NOT NULL REFERENCES records(id),
  actor TEXT NOT NULL,
  accepted_at TEXT NOT NULL,
  CHECK(parent_id IS NULL OR parent_id<id)
) STRICT;
CREATE TABLE events (
  seq INTEGER PRIMARY KEY CHECK(seq>0),
  type TEXT NOT NULL CHECK(type IN ('ledger_initialized','declaration_accepted','load_noop')),
  version INTEGER NOT NULL CHECK(version=1),
  revision_id INTEGER REFERENCES revisions(id),
  request_id TEXT,
  actor TEXT NOT NULL,
  occurred_at TEXT NOT NULL,
  payload BLOB NOT NULL CHECK(length(payload)<=1048576)
) STRICT;
CREATE TABLE current_state (
  singleton INTEGER PRIMARY KEY CHECK(singleton=1),
  active_revision_id INTEGER REFERENCES revisions(id),
  last_seq INTEGER NOT NULL REFERENCES events(seq)
) STRICT;
CREATE TABLE requests (
  request_id TEXT PRIMARY KEY,
  body_id INTEGER NOT NULL REFERENCES records(id),
  event_seq INTEGER NOT NULL UNIQUE REFERENCES events(seq),
  revision_id INTEGER NOT NULL REFERENCES revisions(id),
  outcome TEXT NOT NULL CHECK(outcome IN ('accepted','noop')),
  result BLOB NOT NULL CHECK(length(result)<=1048576)
) STRICT;
```

For each of `ledger_meta`, `records`, `tree_edges`, `revisions`, `events` and
`requests`, create two triggers named `<table>_no_update` and `<table>_no_delete`:
`BEFORE UPDATE ON <table>` and `BEFORE DELETE ON <table>`, each with body
`BEGIN SELECT RAISE(ABORT,'immutable ledger row'); END;`. Only `current_state`
may update; it cannot delete, with the same no-delete trigger. No generic
SQL-writing API is exposed. The schema fingerprint compares the parsed expected
SQLite schema entries using fixed names and stored SQL; it is not an authenticity
check against a hostile database writer.

The native writer additionally enforces these cross-row invariants before commit
and the reader verifies them before trusting referenced state:

- Exactly one meta/current row. Meta identity is 16 OS-random bytes represented
  as 32 lowercase hex characters and workspace_id obeys declaration ID grammar.
  Time is UTC RFC3339 with milliseconds; it provides context, never ordering.
- IDs and event sequences allocate `MAX+1` under the write transaction, beginning
  at 1. Committed sequences have no gaps; rolled-back allocation consumes nothing.
  Revision parent is exactly the old active head; first parent is SQL NULL.
- File/value/request records have no edges. Directory edge ordinals are contiguous
  from 0 and agree byte-for-byte with its encoded children, kinds and digests.
  Names/paths satisfy the portable grammar; references to older record IDs make
  cycles impossible. Tree roots name directory records; interpreted/version
  values name value records with the declared codec and validated schema.
- Source root is the workspace-relative captured declaration closure, including
  explicit workspace/spec/all jobs and every referenced file, with only their
  ancestor directories (and the empty root). It excludes unrelated files and
  runtime data. Entry paths preserve original logical spelling. Registry and
  embedded stdlib are separate complete provenance trees, never declaration
  authority granted by a source file. Repeated references share one path entry.
- `versions_id` decodes to exactly the `versions` in interpretation, including
  the portable stdlib digest/version; the exact stdlib record remains a separate
  revision foreign key. Every revision's workspace ID equals meta.workspace_id.
- Event/request cross-links agree in both directions, including outcome/revision.
  Each load event has exactly one request. Initialization has no request/revision.
  Every revision has exactly one declaration_accepted event. Its parent and source
  references agree with the event. `current_state` is exactly the final event's
  after-state. Foreign keys are checked before successful mutation.

All operations use bound parameters and explicit BLOB lengths. No SQL string
interpolation, SQLite JSON parsing, auto-vacuum, mutable object replacement or
ON CONFLICT REPLACE is allowed. Immutability is a product API invariant, not a
security boundary against an external SQL editor.

## Collision-safe tree interning

For every candidate, calculate the specified digest preimage and digest, then fetch all rows with equal
(codec, kind, digest). Compare complete encoded bytes, including metadata and
exact child IDs, before reusing a row. A directory also verifies corresponding
edge rows. If any candidate row differs, refuse `hash_collision`; do not choose
an equal row from an already ambiguous bucket. Never insert unequal records with
the same digest or use digest uniqueness as a correctness mechanism. Hash indexes
are deliberately nonunique. Verify stored hashes/references on reading; malformed
or ambiguous persisted records are `ledger_corrupt`, not acceptable alternatives.

Insert file records first and directories bottom-up with sorted names. Repeated
identical files may share records. New rows are tentative until the transaction
commits. A forced leaf, internal-node, interpretation or request digest collision
rolls back all newly inserted rows, events, requests and projections. Exact no-op
comparison walks complete candidate records and interpretation/version bytes;
matching root digests alone never skip verification. Reverting after intervening
changes reuses the old complete source tree when exact, but allocates a fresh
revision with the current parent. No patch chain is required for reconstruction.

## Transactions, typed events and replay

Initialization commits meta, event 1 and empty current state together. Event v1
payload is Glaze JSON with exact keys `version:1`, `before`, `after`, `reason`,
`references`; each state is `{active_revision:<decimal string or "none">}`.
Initialize has before/after `none`, reason `initialized`, references `{}`.
Accepted load has reason `declaration_accepted`, before old head, after new head,
and references `{source,registry,stdlib,interpretation,versions}` as record ID
strings. No-op has reason `identical_declaration`, equal before/after heads, and
the same references. Event columns carry request, actor, time and revision; JSON
never substitutes for foreign keys. This is the complete typed event vocabulary
in v1. Future execution formats must add typed affected role/run, acknowledged
revision, exact before/after state and input/output/check/verdict references;
they cannot squeeze execution into an untyped message or rerun modern policy.

For a new load request:

1. Apply the [workspace selector rules](command-contract.md#workspace-selector-and-environment)
   when V5-02-C lands: explicit file or named env expansion, with one captured cwd
   and relevant env value. Normalize expanded workspace and explicit registry
   arguments to absolute lexical paths relative to that cwd (collapse `.` and
   `..`, without dereferencing or requiring their existence). Names and explicit
   paths have equal intent only when these exact normalized targets agree; env
   changes that move the target change intent. Capture uses the normalized load
   target, preserving its documented distinction from render physical resolution.
   Record these exact UTF-8 argument strings,
   operation `load`, actor and expected-head token as request_v1 encoded body.
   The ledger path is excluded because identity is already scoped to the opened
   ledger. No captured content, current head, clock or implicit runtime version
   enters request equality. These stored host arguments are private request
   metadata and never appear in public history/diagnostics.
2. Open/verify the existing ledger read-only and look up request ID **before
   capture and before current-head CAS**. Exact stored body equality returns
   the original result bytes even when files changed, disappeared or the head
   advanced. Different bytes refuse `request_conflict`. A reused ID always
   denotes the original operation intent; accepting a changed draft requires a
   new request ID. No replay reactivates a revision or reevaluates old policy.
   Pure embedded command registration may precede lookup; the registered load
   handler and declaration/candidate evaluation must not. Workspace name expansion
   and lexical normalization do not require file existence or physical resolution.
3. For an unknown ID, capture all declarations/references with existing confinement
   and cooperative stability rules, registry and exact trusted stdlib provenance.
   Evaluate declarations in fresh bounded restricted Lua states; validate all jobs
   through the retained same-bundle trusted handler as specified by the
   [R14-D3 protocol candidate](load-command-contract.md#finite-fresh-load-service-protocol).
   Reopen through the validated
   writable opener and use `BEGIN IMMEDIATE`, bounded busy wait, identity/schema
   recheck. Look up the request again: another writer may have committed during
   capture; exact body replay/conflict still precedes CAS. Only then intern
   candidate records and the exact request body inside this transaction. Never
   run Lua under the write lock. Tentative writes roll back on every refusal.
4. For an unknown request require expected head equal current active head,
   including `none` for an empty ledger. A mismatch refuses `revision_conflict`,
   even if content equals the current head. No new request row survives refusal.
5. Recheck retained filesystem capture receipts immediately before committing
   acceptance. R14 adds an acceptance-only native verifier that retains root
   descriptors and declaration/reference-closure byte/mode/identity receipts through
   the transaction and compares them with the sealed capture. For load, closure
   membership is determined by the captured declarations and references: unrelated
   workspace additions/removals/edits do not block acceptance. Necessary ancestor
   directory identities, selected file/root identities and normalized file modes
   remain checked. The separately captured registry has its own receipts. This
   closure-only choice was adopted when the operator continued on the recommendation
   on 2026-10-21. Whole-tree capture used by render keeps its existing membership
   checks. The acceptance-only native verifier is implemented; load integration
   composes it without replacing sealed values. Render and Lua consume only sealed
   values, without fresh reads. A detected closure byte/mode/identity change refuses
   `capture_changed`. No fresh values replace sealed capture bytes. Cooperative
   writers must keep input stable through this boundary; this does not stop an
   OS write immediately after recheck or detect every change-and-revert attack.
6. Exact equality of source tree, registry, stdlib and interpretation/version
   records with active revision yields no-op. Allocate no revision; append one
   load_noop event and its successful request result. Otherwise allocate a new
   revision with current parent and append declaration_accepted. Initial load
   always creates revision 1. Optional prose/comment edits create revisions even
   when existing material work fingerprints remain equal.
7. In the same transaction update active head/last_seq and insert request with
   its fully encoded success result. Commit only after all bytes/references and
   bounds are validated. Buffer stdout until commit succeeds. Write failure after
   commit may hide a successful response; durable request lookup recovers it.

Result JSON has exactly `version:1`, `kind:"load"`, `ledger`, `request_id`,
`outcome` (`accepted` or `noop`), `revision`, `parent` (or `none`), and `event`
(the ID strings above). A replay returns the original bytes; current head is
queried separately. A changed draft on retry does not change the original intent or result; use a
fresh ID to accept it. Changed explicit parameters conflict. Request lookup also
recovers a lost response without supplying the original invocation arguments. No pending/in-progress requests are durable in this
finite foundation: a crash before commit leaves no request; a crash after commit
leaves the complete recoverable result. Failed requests do not reserve their ID.

Read/history pins a SQLite read transaction. It reconstructs from initialization
plus ordered typed before/after records and immutable referenced values; each
before-state must equal the previous after-state. It compares the reconstructed
last state to the projection. Historical inspection neither executes Lua/checks
nor uses current declarations to reinterpret old facts. Old interpretation
codec unsupported by this binary gives `unsupported_interpretation` while raw
source export remains available if record_v1 itself is supported.

## Embedded bridge and provenance amendment

The [command contract](command-contract.md#load-admission-and-exact-provenance)
pins Lua-owned semantic dispatch with native fixed-family invariants. V5-02-B
retains real Lua callable references in one bounded invocation VM; init/history/
export issue only their finite typed services. Historical values are verified
and returned without current or recorded Lua reinterpretation. Export preserves
preallocated publication-uncertain and stdout-failure fallbacks, including failure
in the postpublication Lua projection. No Lua runs while native write locks
are held. Load admission remains a separate reviewed R14 implementation round.

Embedded command source additions change exact originals/assembly/manifests and
stdlib_digest under existing provenance codecs. Request_v1, interpretation_v1,
versions_v1 and record_v1 shapes remain unchanged; no W recipe changes. A fresh
acceptance captures the new embedded provenance; old revisions are never rewritten.
External profiles are a mandatory gate after full R14 and before dependent jobs/
RA01, requiring separate reviewed selector intent, provenance, interpretation and
format coexistence codecs. No profile selector is smuggled into request_v1 and
no request_v2 or interpretation_v2 is introduced by this embedded amendment.

## Proposed command surface

Init, history and export are implemented and independently approved in R14-E1/E2
and R14-F1/F2; load remains planned until its recorded reviews pass. Existing render syntax is unchanged. All
options take separate tokens under CLI11, reject duplicates/unknowns/positionals
and `--x=y` using the reviewed syntax guard, and produce buffered Glaze JSON.
Paths below are explicit host paths except the separately staged workspace-name
selector; none are interpreted from declarations. Init remains explicit-ledger
only. RA01 authoring/scaffolding must be separately contracted.

| Invocation | Semantics |
| --- | --- |
| `planar init --ledger <new-path> --workspace-id <id>` | Create only a new empty ledger; fail if any target already exists, including empty files/links. Existing parent must be a real confined directory. No declaration scaffold, prompt, Git initialization or checkout creation in this foundation. This narrows v4's illustrative init until authoring workflows receive their own design. |
| `planar load --ledger <path> --workspace <workspace.lua> --registry <registry.json> --expect-revision <id-or-none> --request-id <id> --actor <text>` | Capture and validate complete declaration closure and trusted registry; identity must match initialized workspace. No source/base checkout is required or captured. Commit only through the protocol above. |
| `planar history --ledger <path> [--after-event <id>] [--limit <1..100>]` | Read chronological event summaries, ascending seq, exclusive cursor; absent cursor starts before event 1. Return ledger ID, snapshot last_seq, events and next cursor if more. Resume pages may observe newer events; IDs never change. |
| `planar history --ledger <path> --revision <id>` | Return exact revision metadata, complete recorded interpretation and versions with record IDs; mutually exclusive with paging/request options. Refuse output above existing 16 MiB transport ceiling rather than truncating. |
| `planar history --ledger <path> --request-id <id>` | Return saved load result and separately current head/last_seq, without capture, CAS or replay. Unknown request is `request_not_found`. |
| `planar history --ledger <path> --at-event <id>` | Return reconstructed state after that event plus event/revision metadata; mutually exclusive with all selectors/paging. This is a read, never activation. |
| `planar export --ledger <path> --revision <id> --target <new-directory>` | Reconstruct source tree exact bytes and normalized modes into a new target; no registry/stdlib injection into authored paths, no interpretation regenerated as source. No activation or ledger mutation. |

No-selector history includes current head even for an empty ledger. CLI-visible
metadata never contains BLOB payloads or host paths. Init returns
`{version:1,kind:"init",ledger:<hex>,workspace:<id>,active_revision:"none",event:"1"}`.
History returns `{version:1,kind:"history",ledger,active_revision,last_event,data}`;
`data` is a selector-tagged record: `page` with `events` and optional `next_after`,
`revision` with metadata/interpretation/versions, `request` with `result`, or
`state` with event/state. Event summaries include all typed event columns and
payload, revision metadata includes all revision columns with ID string conversion.
Export returns `{version:1,kind:"export",ledger,revision,source:<record ID>}`.

The concrete history JSON field names, selector grammar, default page size and
whole-response byte accounting are pinned in [R14-E2](rounds/R14-E2.md#search-outcome-and-concrete-contract).
These clarify this surface before implementation; they do not add load policy.

Accepted-revision rendering and draft status/impact are later read workflows,
not aliases that let current render silently accept files. Invalid drafts leave
accepted history usable now; reporting draft status is not claimed until its
bounded design is implemented. This foundation's history command reports only
recorded state.

Errors use the existing `{version:1,kind:"error",error:{code,message,field,...}}`
envelope and empty stdout on failure, subject to existing process-write limits.
Add stable codes `ledger_exists`, `ledger_not_found`, `foreign_ledger`,
`unsupported_ledger_version`, `ledger_corrupt`, `ledger_busy`, `storage_full`,
`revision_conflict`, `request_conflict`, `request_not_found`, `revision_not_found`,
`event_not_found`, `hash_collision`, `unsupported_interpretation`, `target_exists`
and `export_incomplete`. Invocation errors exit 2; semantic/conflict/resource and
format refusals exit 3; I/O, corruption, disk-full and busy errors exit 4.
Existing capture/validation codes retain their meanings. The [R14-D3 typed diagnostic candidate](load-command-contract.md#typed-authoritative-diagnostics)
pins authoritative `expected`/`actual` head strings and collision kind/safe digest.
Request-kind collisions omit digest entirely; request_conflict has no private
details. These details are proposed native API work, not current implemented output.
Never expose captured content or private request metadata. Export-incomplete details identify only cleanup state,
with host path handling consistent with existing diagnostic redaction.

## Initialization, failure safety, export and format evolution

Before opening an existing database writable, open the explicit regular file
read-only without following symlinks and verify SQLite header, application/user
versions, exact schema and internal integrity. Refuse foreign, empty, truncated,
future or unsupported older databases unchanged, including no journal creation,
pragma mutation, migration or repair. Revalidate file identity on transition to
writable access and inside the transaction. Cooperative local directory ownership
is required; do not claim defense against a hostile file swapper. A recognized
v1 database with a hot journal may require SQLite crash recovery: check header/
identity/version read-only without modifying foreign files, then let SQLite
recover only a recognized owned v1 database and validate schema/integrity before
any new operation. An ambiguous/inconsistent header refuses for diagnosis; no
best-effort repair command exists. Never inspect the old default database path.

Init builds a fully committed database in a fresh exclusively created sibling
staging directory, closes and syncs it, then publishes its database file with an
atomic no-replace filesystem operation and syncs the parent. Fail if no safe
no-replace primitive is available; never emulate it with check-then-overwriting
rename. On publication/sync ambiguity return failure with recovery guidance to
inspect the explicit target; do not delete a possibly published ledger. Random
identity and init timestamp are recorded once. Failed staging is removed only
when descriptor/inode ownership still matches this invocation.

Export validates the entire source tree in one read snapshot before creating
anything. Open and retain the real target parent descriptor; reject symlinked
parent traversal, unsafe paths and existing targets. Create an exclusive sibling
staging directory with restrictive permissions. Write all regular files through
no-follow/exclusive relative opens, preserve exact bytes (including CRLF, empty
files and binary payloads), apply 0644/0755, then directories 0755, sync files and
directories bottom-up, and publish staging with atomic no-replace rename followed
by parent sync. Refuse unsupported safe publication. Never merge into or replace
an existing worktree. Export's source set contains only captured declaration
closure, not arbitrary workspace empty directories or implementation checkouts.

Before publication, any failure removes only staging entries created by this
operation after ownership checks. Cleanup failure returns `export_incomplete`
and a clearly labeled leftover staging location via the diagnostic's redacted
location token; it never reports successful export. Crash may leave a uniquely
named staging directory; future exports do not adopt/delete it automatically.
After publication, a parent sync/output failure may leave a complete target;
report uncertainty and require inspection, never remove a published target.
There are no hidden file overwrites and no successful partial export.

SQLite rollback covers constraint, I/O, full-disk, collision, capture race and
CAS failures. Reopen after crash checks integrity and foreign keys, not merely
the presence of a head pointer. A commit error of uncertain outcome is not success:
report storage failure and recover by request lookup. If a transaction cannot
append history it cannot advance the head. Full storage never prunes prior rows.
No automatic expiration, current/previous retention or vacuum removes history.
There is no claim of power-loss immunity beyond SQLite/filesystem guarantees.

Format v1 has no migration from any legacy format. Future **new-ledger** versions
require a separately reviewed offline migration tool: close writers, create an
SQLite online-backup snapshot to a new destination, validate it with integrity/
foreign-key/reconstruction checks, migrate a separate copy transactionally, verify
all IDs/history/bytes, and atomically publish only to a new target. Preserve the
original and backup; never auto-migrate on open. R12 tests SQLite backup/restore
primitives using a quiescent source and new explicit target, not a new public
backup CLI. Copying only a live database file is not the backup protocol. Restore
validates identity/version/schema/integrity/history before opening the copied
ledger for writes. No legacy import/export promise is implied.

## Bounded implementation and acceptance

R12 owns dependency restoration, the ledger module, exact v1 schema/opener,
transaction and backup primitives with scratch-only tests. It exposes no new CLI
and does not capture/interpret declarations. Prove initialization/open/refusal,
foreign/future files byte-for-byte unchanged, transaction rollback at every write,
nonzero busy/concurrent tests, disk-full fault injection and backup/restore.
Use both an injected failure and subprocess termination around BEGIN/COMMIT;
verify either complete committed state or complete prior state. Record test counts.

R13 owns record_v1 codec, bounded immutable tree construction/reconstruction,
provenance and interpretation fixtures. Test exact bytes/modes, sorted topology,
empty root, rename/delete, timestamp-preserving changes, malformed/cyclic graphs,
forced leaf/internal/value/request collisions and rollback. Demonstrate the
collision regression fails without exact comparison. It depends on approved R12.

R14 owns finite acceptance, CLI11 handlers and exact export. Split it into fresh
bounded subrounds for acceptance transactions, history/read commands and export
if the combined review becomes too large. It depends on approved R13. Prove
initial/no-op/change/revert chronology; comment/prose revision with stable scoped
work identities; invalid draft leaves accepted revision readable; two concurrent
loads have one winner; request replay after a later revision returns original
bytes without activation; changed expected/actor/path arguments conflict; changed or absent drafts replay
the original result; a fresh ID accepts a new draft; lost stdout
recovers; crash at each write keeps request/event/head atomic; exact old revision
export succeeds into a new target and refuses an existing one; write/full-disk/
rename/sync/cleanup failures never overwrite user files or report partial success.
Read every early event after restart and reconstruct exactly without executing
Lua. Cover v4 A20, storage/format portions of A23, and A25–A29; missing dispatched
packet recovery and role reconciliation remain execution obligations.

No product choice is left open for this bounded foundation. New execution-state,
active-job, draft status/impact and dispatch semantics need their own reviewed
contracts before dependent code. R11 review is design review only; implementation
and the acceptance cases above require fresh independent rounds and evidence.
