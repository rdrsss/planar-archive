# X05-C4-B1 production gated SQLite prerequisite candidate

Status: implemented candidate, awaiting fresh independent review. No self-approval.
Shared checkout `dev/planar-v2-vernacular`, starting HEAD
`3171c49e7b92d3abda01bfbb8bfb0c6b29d4c67a`. Scope and authorization are in
`docs/vernacular/rounds/V5-02-X05-C4-B1.md`; coordinator owns that round.
No Planar tools/skills/queues, delegation, commits, install, publication, merge,
installed binary or operator-data access occurred. Only this coder used serialized
Debug during this implementation. Release remains absent; installed release is
protected and never invoked.

## Candidate and native ownership

The distinct `connection::open_gated(path, shared_ptr<read_guard>, busy_ms)` opens
read-only. Existing `open` has the original signature, defaults, path validation,
opening flags and authorizer policy. `sqlite_error` adds a trailing default-ready
status, so all existing three-field aggregate initializers retain their meaning.
`gate_status()` reads the first refusal without allocation, including after error
string construction throws. No raw SQLite handle is exposed in product APIs.
The generic native verifier recognizes only ready/refused/resource; allocation
and length exceptions become resource, other exceptions become verifier_exception,
and unknown statuses fail closed. Original richer errors stay adapter-owned.
All first-status publication is enum/integer assignment, without allocation.

Stable sqlite_state owns one fixed context containing the shared guard, copied VFS,
64-byte address-derived name, base pointer and sticky status. No application cache
or dynamic path/name buffer is introduced. Apart from existing path/error strings,
native ownership requests are one make_shared state/control block and one
make_unique context. SQLite requests `sizeof(file) + alignof(max_align_t) +
base.szOsFile` per wrapped file; signed size arithmetic is checked and the inner
file is max_align_t aligned. Names need at most prefix12 + uintptr hex16 + NUL on
64-bit targets. This does not compose VM512MiB/capture meters; X06 remains open.

Each VFS registration is explicitly nondefault and each file retains direct stable
context. VFS/file callbacks forward the real underlying pointers. Main xRead and
actual xFetch require the trusted verifier then supported/successful/unmoved
HAS_MOVED before any delegate. Fetch refusal nulls mapping; read refusal leaves
caller buffer untouched. HAS_MOVED is a cooperative inode/path supporting check,
not hostile swap or change-and-revert proof. No vendor-private structure is read.
One readonly main open is admitted. Main writes/truncation and journal/WAL opens
refuse; main shm map/lock refuse and shm cleanup/barrier do not access storage.
Temporary files retain read/write/truncate/cleanup behavior after main refusal.
Method versions and null optional methods follow the base. ATTACH/DETACH denial
is bound-only, authorized by the coordinator to prevent applying one original-main
verifier to an alternate database; unchanged ungated callers retain their policy.

Statements and transactions already retain sqlite_state until finalization and
rollback. Destruction now checks actual SQLite close before context unregister and
release. BUSY there is an internal fatal ownership invariant, not a supported
retry or retirement protocol. Pinned initialized VFS unregister cannot normally
fail; concurrent shutdown is unsupported. Partial xOpen failures close any live
inner file before returning. Register rc is checked, but initialized register has
no allocation/failing path after successful base discovery; no impossible injected
unregister/register failure is presented as a natural production condition.

The real existing online_backup_to exception gap is fixed by a local RAII owner.
Its diagnostic is still captured before finish with the same original code/message
semantics. Every throw after successful init now finishes the backup. Other raw
resource creation was inspected: prepare failure yields no statement; tail-parse
failure finalizes both handles before allocation; successful statement/transaction
construction only retains shared ownership and cannot allocate. No broader backup
or service refactor was made.

## Verification and reproducible identity

`baseline.json` records 72,151 pre-edit protected entries, including exact modes,
bytes/symlink targets, all prior artifacts and installed release. All three owned
product/test preimages are retained as `preedit-*`. Initial CTest inventory is756;
11 added colocated cases produce767 while preserving every original name.
The final focused suites comprise33 SQLite/reliability,80 format/snapshot/history/
acceptance and3 history-read cases (116 total), with zero failures/errors/skips.
These include real mmap with non-null mappings, first-header refusal with zero
underlying reads and positive read controls, moves/last-owner statement/transaction
lifetime, concurrent live independent owners, late pathname replacement, typed
sticky outcomes, version/null callbacks, partial-open cleanup, bound ATTACH refusal
before alternate-file open, and exact DB bytes/modes/no sidecar preservation.

Private source copies add6 cases: backup exception, direct callbacks/temp I/O,
actual WAL refusal with an ungated WAL positive control, original receipt chain,
actual native context allocation refusal, and24 SQLite allocation-fault positions.
Receipt verification uses the real profile_capture original receipts and held
no-follow descriptor as a test adapter only: healthy reads pass; profile mutation,
necessary ancestor replacement and substituted held FD refuse before actual reads.
There is no new ledger.sqlite capture/runtime dependency.

The backup regression seeds a real long prior SQLite diagnostic, then fails an
actual C++ allocation while copying it with an initialized backup handle live.
The red copy reconstructs the exact original backup body from the retained
preimage while retaining the current surrounding module/API. Count-only wrappers
call the real pinned init/finish. Red observes zero finishes and actual
sqlite3_close(source)==BUSY, cleans the leaked backup explicitly, and emits the
expected single failed assertion. Green finishes exactly once on the exception,
then successfully performs another real backup; ordinary owner destruction closes.
No natural host exhaustion is claimed. Native owner and SQLite allocator refusal
are controlled actual allocation failures; controlled HAS_MOVED outcomes and direct
callbacks are distinguished from actual SQLite-driven reads/mappings/WAL access.

All private compilation uses python3 -B, expands response files, copies all888
object/PCM/archive/dylib inputs (including explicit std modules and LLVM dylibs),
redirects outputs, disables implicit modules and compares exact shared/copied
membership, modes and hashes. The complete copied set remains in dependencies/.
No shared build input is linked into a private probe. Source/test/support/prose are
frozen before final builds/tests/lint/private checks; frozen-candidate.json is the
exact review identity. gate-results.json records argv, cwd, timing and exits;
private/commands.json and private/dependency-final.json preserve detailed evidence.
Pinned formatting/Doxygen covers118 active sources. A repeat of the same focused
build must report no work. final-protection.json compares every non-owned protected
entry with the original baseline, excluding only coordinator-owned round prose.
artifact-manifest.json describes all evidence entries except itself/finalization;
finalization.json binds the manifest, frozen candidate and report SHA256 values.
No files in earlier evidence directories were changed.

## Preserved attempts and limits

`gated-attempt1.xml` preserves the discovered null-handle NOMEM status loss; the
candidate fixes it. `gated-attempt2.xml` is its green rerun. Private attempt1 used
noncanonical /tmp and correctly hit the existing NOFOLLOW restriction; attempt2
canonicalizes scratch paths and demonstrates the real backup red/green. Attempt3
has a test-only mistaken profile diagnostic enum/name; attempt4 fixes the test;
attempt5 adds allocation coverage and actual WAL controls. These are setup/test
failures, separate from the intentional reconstructed backup regression. The
initial lint invocation tried importing the CLI script and hit its argparse guard;
the supported CLI preflight passes. Final attempt1 correctly passed both suites
but its runner mistook fork child XML records for registered test cases (80 format
cases emit94 raw records). The final gate uses exact CTest inventory selection
and a116-case CTest JUnit report instead. All attempts remain available.

This is a lower native prerequisite. Production receipt adapter, raw-header binding,
format/history/read service integration and handler capability remain closed.
No immutable URI, DB copy/deserialize, new ledger quota, format, public CLI,
whole-invocation accounting, full/release gate, Linux execution or mandatory
external-profile acceptance is claimed. Linux portability is source review only:
standard C++ plus SQLite public interfaces; no platform-private file structures.
Public init creates format1. Independent reviewer must assess this exact candidate.
