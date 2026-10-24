# X05-C4-B0 test-only SQLite receipt-gate feasibility

Candidate for independent review; no self-approval. This supplies a standalone
source-bound feasibility prototype, not a production capability read service.
Only the new `v5-02-x05-c4-b0-support/` and `-evidence/` directories were written.
HEAD is `3171c49e7b92d3abda01bfbb8bfb0c6b29d4c67a`, in the intentionally dirty
shared `dev/planar-v2-vernacular` checkout. No product/configuration/history edits,
Planar tools, delegation, commits, installation, publication or merge occurred.

## Architecture and ordering

`probe.cpp` registers a unique non-default VFS per stable native owner. SQLite's
own registration list is the only routing mechanism: `pAppData` and each file's
context point directly to the owning object. There is no application global
active owner, TLS or routing map. The original `profile_capture` admits the
scratch ledger under `extension.ledger`; an O_NOFOLLOW held descriptor is verified
against that exact receipt before raw pread. Verification does not refresh receipts.
Main `xRead` and `xFetch` verify the same held descriptor and require the inner
`SQLITE_FCNTL_HAS_MOVED` to succeed with unmoved before delegation. The original
logical profile error moves into an owned optional using a statically checked
nothrow move; the first SQLite failure is sticky. Protected callback mutation
hooks catch all exceptions; gate allocation exceptions become retained NOMEM.

The built-in Unix descriptor/name comparison is a cooperative second link.
This does not defeat hostile swap/change-and-revert or a write after checking.
No vendor-private descriptor layout, immutable mode, database deserialization,
whole-database-copy reader, new format or public quota is introduced. Fixture
replacement copies are deliberate scratch mutations, not the service architecture.

Existing journal/WAL/shm paths fail the initial metadata precheck. Main journal
and WAL xOpen refuse without calling the inner VFS, and main xShmMap refuses
without access. Main writes/truncations return READONLY; connections use READONLY.
Other I/O and unrelated temporary files forward to the pinned VFS. No new temp
or ledger-size policy is added. A production wrapper still requires a reviewed
closed lower API rather than exporting this test's directly callable C functions.

Close precedes unregister and context destruction. A real SQLITE_BUSY close
retains connection/context; a controlled unregister refusal retains registration,
held descriptor and receipts after the connection closes. Retry succeeds. SQLite
unregister itself has not naturally failed; that boundary is an explicit adapter.
The stack fixture destructor expects successful final cleanup; a production owner
must define its teardown policy for abandoned statements and terminal close errors.

## Final verification

Final `build-final6.log` and `private/commands.json` bind exact compile/link/run
commands. The support identity was frozen before final compilation and checked
again afterward; earlier frozen candidates and their outputs remain separately
named. All Python commands used `python3 -B`.

- Thirteen final Catch cases pass, with zero failures, errors or skips. Count the
  JUnit `testcase` elements: Catch's suite `tests` attribute counts assertions.
- Healthy actual SQLite opens read the header before open returns, including the
  intentionally unguarded forwarding baseline. Guarded ordinary queries pass.
- Held-descriptor replacement before raw pread refuses with zero raw reads.
  Replacement before SQLite open and between inner open/first header read refuses
  with zero delegated reads. The latter also has zero intercepted content syscalls.
  Metadata-only verify_file has zero intercepted reads; healthy raw pread has one.
- Alias admission, post-open pathname move and chmod changes refuse. Real query
  refusal on one replaced owner does not affect another simultaneous owner.
- HAS_MOVED NOTFOUND, error and moved controls all refuse before first delegate.
- With explicit mmap_size=64MiB, actual queries make 753 delegated fetch callbacks,
  of which 752 return non-null mappings on this macOS host. A deliberate mutation
  hook inside the next actual SQLite-triggered xFetch yields one refused callback
  and zero additional fetch delegates. A separate direct callback checks null
  output on refusal. This is a controlled mutation at a real callback boundary,
  not natural concurrent mutation or merely a mocked non-null mapping.
- Journals and WAL arriving after initial precheck cause actual SQLite xOpen
  refusal (observed rc8 and rc14 respectively, one rejected auxiliary open each).
  Direct controlled journal/WAL/shm callbacks separately check the forwarding seam;
  main xShmMap refusal is controlled, not an actual mapped shm positive control.
  An unrelated anonymous SQLite temporary file opens/closes through the inner VFS.
- Controlled before-inner-open NOMEM, throwing gate and throwing after-open hook
  leave zero content reads and correct close counts. These are injected callback
  failures, not natural memory exhaustion or a global allocator sweep.
- Real BUSY close and controlled unregister-failure retry preserve owner lifetime.
  Default VFS identity remains unchanged after ordinary/multiple-owner probes.

The pre-edit CTest inventory is756; no product test registration changes were
made. No full suite, release gate, production build or installed binary was used.
`build/rewrite-release` remains absent; install-release and all prior history are
protected. The protection comparison covers72,017 membership/mode/byte or symlink
entries and reports zero changes; AGENTS.md remains the CLAUDE.md symlink.

Private compilation copies and protects all888 compiled object/PCM/archive/dylib
members, including module and link closure and pinned LLVM dylib targets. Response
files expand recursively. Object/PCM/dependency outputs are private; implicit
modules/maps are disabled. Shared/copy membership, modes and hashes compare equal.
Archives and explicit module inputs link/use copied paths. The macOS executable's
pinned libc++ absolute install name continues to load the protected installed
LLVM runtime; those external dylib bytes are copied/protected rather than edited.
OS runtime libraries are normal platform prerequisites. No compiled output is
written to the shared Debug graph. Disposable compiled copies are removed at
finalization, while source, commands, identities and raw output remain.

## Retained exploratory failures and refinements

The initial inventory helper referenced a nonexistent `bin/planar_runtime_tests`;
its empty output is preserved, and the independent CTest JSON establishes756.
The first private compilation passed but linking omitted the pinned SQLite archive;
`private-attempt1` and `build-attempt1.py/log` retain the undefined-symbol failure.
The corrected explicit copied archive link passed seven initial cases (attempt2).
Attempt3 failed one syscall assertion because fixture copy_file itself read the
watched original inode; the watcher was correctly reset after the deliberate
mutation. That raw XML and exact source are preserved. Attempt4 passed12 cases.

Frozen1 passed12, then typed native-error retention was added. Frozen2 did not
compile because a whitespace-sensitive edit omitted the new optional member;
its source, frozen manifest and compiler errors remain. Frozen3 passed12. Frozen4
failed the exploratory assertion that a post-mutation query would first reach
xFetch: actual SQLite instead hit gated xRead, with zero fetch delegates. The final
controlled before-fetch mutation isolates the requested mapped callback; frozen5
passed12. Frozen6 additionally exercises BUSY close/unregister retry and passes13.
These are test-fixture/setup failures, not claimed product regressions. No failure
output or source was overwritten by a later candidate; shared dependency identity
remains fixed throughout.

## Production ownership and remaining work

A prospective production round should place the stable receipt-bound readonly
connection/VFS owner below ledger/history (a new engine ledger module or a reviewed
capture-to-ledger bridge), with existing `profile_capture::verify_file` as native
receipt authority. Storage owns SQLite flags, schema/format/recovery/no-sidecar
policy, native reader errors and transaction lifecycle. Runtime command services
should receive an unforgeable admitted slot/owner, never a caller path/receipt
assertion, and should translate retained native errors without reopening by path.
Thin command/runtime orchestration retains all-slot admission, exact capability,
once/order/poison and original pre/output seals. Tests belong beside those product
modules; this support directory is deliberately not compiled into Planar.

This does not implement history_read_v1, selector/opaque-slot ABI, projection,
handler response/native fact correspondence, original all-slot integration,
whole-invocation capture budgets or public profile invocation. Real recorded
profile interpretation/history/acceptance remains X06 work. X07 public cutover,
X08 mandatory external-profile acceptance, execution/RA01 and Linux remain open.
The current public init still creates format1; latest approved full gate remains
676 per configuration. macOS native feasibility is all this candidate claims.

Final manifests bind support and raw evidence with SHA256/modes/links. All owned
processes finish before finalization; exclusive Debug and all owned paths are
released to the coordinator. Independent review decides feasibility.
