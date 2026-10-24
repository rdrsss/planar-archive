# V5-02-X05-C4-B0 independent feasibility review

Disposition: **approve**. The exact candidate demonstrates the authorized
cooperative, original-receipt SQLite content gate on macOS. No required changes
were found within this test-only scope. This approves feasibility, not a production
VFS, history service, capability execution or complete acceptance gate.

The review used the shared `dev/planar-v2-vernacular` checkout at HEAD
`3171c49e7b92d3abda01bfbb8bfb0c6b29d4c67a`. Ownership was limited to this report
and `tests/native/v5-02-x05-c4-b0-review-evidence/`, with exclusive serialized Debug
verification. Product/configuration, author artifacts, history, installation,
release and operator data remained read-only. No Planar tools, skills, roles,
queues, delegation, commits, installation, publication or merge were used.

## Reviewed identity and protection

The reviewer read the repository instructions, implementation plan, roadmap, v5
baseline, exact X-CAP contract, complete C4-B/B0 authorization and approved C4-A
review. Current product boundaries take precedence over historical prose.

| Author identity | SHA256 |
| --- | --- |
| `frozen-support.json` | `990846d7c18eb7fdc16b697f585bd5ac642a0792933edb398194ebe2163af3fe` |
| `probe.cpp` | `196b47d96c1af5cc7bcdab2bb8228ab657d2b8145e60f8a7017331a2e4819852` |
| `artifact-manifest.json` | `1d55cfc2f6a6f0b8f935c1193f8bad656994781f699d11317859d1b49dd57d95` |
| `REPORT.md` | `b3d5eb3507300f3411be6f77f87a8057a6e660b26a30f6a299d6a35e6fcc597a` |
| `finalization.json` | `1e0000acfd007991fb1d2faa7df2e591f9d80b5907884390753ef97ddee83ca7` |

All four frozen support files and all92 author artifact-manifest entries were
checked for exact membership, modes, bytes and symlink targets. Including the two
author root directories and manifest/finalization files, the independent immutable
snapshot contains100 entries. The72,017 protected entries match the author's
original baseline exactly and remain unchanged. This includes prior evidence,
product/configuration, vendor sources and installed release. `AGENTS.md` remains
the `CLAUDE.md` symlink; `build/rewrite-release` remains absent.

## Source assessment

Pinned `vendor/sqlite/b9c7/sqlite3.c:75907` calls the pager header reader during
B-tree open; `:63578` delegates that read through `sqlite3OsRead`. Therefore a check
performed only after `sqlite3_open_v2` returns cannot establish before-read
binding. The inner `unixOpen` path at`:46735` opens and initializes the descriptor;
the relevant header consumption occurs through the intercepted I/O method.

`probe.cpp:59` creates one stable owner, unique non-default VFS name and direct
`pAppData` reference. Each opened file retains its owner and separately aligned
inner `sqlite3_file`. There is no application routing map, TLS or active-owner
pointer. SQLite's own registration/unregistration at`:27699`/`:27727` uses its
static-main mutex, and `makeDflt=0` preserves the default VFS. The two-owner test
keeps both registrations live and interleaves actual queries; it is not a claim
of a concurrent-thread stress test.

The held no-follow descriptor is compared with the original admitted
`extension.ledger` receipt before raw pread. Every main `xRead` and `xFetch` calls
that same receipt verifier before delegation. `capture/admission.cpp:247` verifies
retained descriptors, named entries, necessary ancestors and relationships;
`:375` additionally compares the supplied held descriptor with the selected
original file. The full file comparison includes device/inode, mode, size and
nanosecond mtime/ctime. It neither refreshes the receipt nor reads file contents.
The independent trusted-profile and ancestor mutations confirm this is more than
a check of the main ledger pathname.

The second link is the inner `SQLITE_FCNTL_HAS_MOVED`, required to return OK and
unmoved before either content callback delegates. Pinned `fileHasMoved` at`:41820`
compares the current pathname's inode with the opened inode. This supports the
cooperative original-receipt/path/inner-open chain; it is not standalone descriptor
proof or protection against hostile substitution and change-and-revert. Unlike
SQLite's later `databaseIsUnmoved` check at`:63823`, the prototype refuses
NOTFOUND. The gate does not inspect vendor-private `unixFile` fields.

The main journal/WAL open refusal precedes inner VFS open. Main `xShmMap` refuses
before access; other temporary-file behavior forwards. Main writes/truncations
refuse and connections open READONLY. No immutable mode, whole-database copy or
deserialize reader, new format, quota or default VFS replacement appears. Scratch
replacement copies are deliberate mutation fixtures. Forwarding is appropriate
for this pinned local VFS; general optional-method/platform support is not inferred.

Receipt allocation and mutation-hook exceptions are caught inside callbacks.
Original native `profile_error` ownership moves without allocation using the
statically checked nothrow move; the first gate failure remains sticky. On fetch
refusal the returned mapping is null. Partial-open failures are closed, a real
BUSY close retains the connection/context, and the controlled unregister refusal
retains registration/receipts until retry. Destruction occurs after successful
close/unregister in the exercised fixtures.

## Evidence and independent verification

The author final13-case JUnit and earlier raw attempts were inspected. The missing
archive link, syscall-counter reset after fixture copy, omitted optional member,
and mistaken expectation that a post-mutation query must first reach xFetch are
preserved as exploratory/setup failures. The frozen4 mmap failure actually reached
the gated xRead with no additional fetch delegate. The final probe deliberately
mutates inside a real SQLite-triggered xFetch to isolate that boundary. This is a
controlled mutation at an actual callback, not a naturally occurring race.

Independent commands, generated source identities, JUnit and logs are retained in
the reviewer evidence directory. All Python commands used `python3 -B`.

| Independent check | Result |
| --- | --- |
| Exact frozen author cases |13/13, zero failures/errors/skips,0.91s wall time |
| Reviewer-authored adversarial cases |6/6, zero failures/errors/skips,0.36s wall time |
| Actual mmap positive control |753 delegated fetches,752 non-null mappings |
| Actual xFetch-entry controlled mutation | One refused callback; zero additional delegates |
| CTest inventory before/after |756 names, identical inventory |
| Shared/copied compiled dependencies |888 members, modes and hashes unchanged |
| Author/support and protected final comparison |100 and72,017 entries unchanged |

The six reviewer cases exercise same-inode pwrite refusal before delegation with
an untouched output buffer; a substituted held descriptor while the original path
remains healthy; original trusted-profile and necessary-ancestor changes; a late
symlink introduced between inner open and first content read; complete healthy
ledger-byte/mode/mtime preservation plus READONLY mutation refusal and no sidecars;
and temporary-file write/read/truncate forwarding after terminal main refusal.
The original typed capture error survives later controlled gate failures.

The author rerun also independently reproduces early-header positive controls,
zero delegated content reads on first-header receipt refusal, the read/pread/mmap
interposer controls, NOTFOUND/error/moved refusal, actual late journal/WAL rejected
opens, two simultaneous owners, controlled allocation/callback failures, BUSY close
and unregister retry. Shared-memory refusal is a controlled direct callback;
unregister failure and allocation faults are controlled adapters. No actual shm
mapping, natural unregister failure or natural host exhaustion is claimed.

Private compilation copied all888 object/PCM/archive/dylib dependencies, including
all explicit module/link inputs and pinned LLVM dylib targets. Response files were
recursively expanded, outputs redirected and implicit module/map builds disabled.
Full shared and copied membership/modes/hashes were checked before cleanup. The
executable's macOS libc++ absolute install name still loads the protected installed
pinned LLVM runtime; `otool -L` records this. Its bytes were copied and protected,
but the load path was not rewritten. OS runtime libraries remain platform
prerequisites. No shared Debug build, full suite, release gate or installed Planar
binary was used. Disposable copied dependencies and compiled outputs are removed.

## Production prerequisites and handoff

The prospective storage/capture bridge needs a reviewed closed API below
ledger/history that owns the immutable original admitted path/slot, no-follow held
descriptor, stable per-connection VFS and retained original capture authority.
Do not expose this fixture's mutable fields, direct C callbacks, arbitrary SQL or
caller-supplied receipt assertions. Production construction/partial-open cleanup
must be nonthrowing at C boundaries and define teardown for abandoned statements,
BUSY close and terminal unregister errors; fixture Catch assertions are not that
policy. Storage must own schema/format/recovery/no-sidecar rules, fail-closed
metadata error adaptation and typed first-error propagation. Preserve temporary
SQLite behavior and audit optional I/O methods for each supported platform.

The real read service must bind an unforgeable admitted slot and exact native
history selector to that owner, without reopening by a caller path. Runtime must
integrate complete initial all-slot admission, exact capabilities, once/order/
poison behavior, retained native facts and response correspondence, original
pre-service/output seals and whole-invocation accounting. Cached/mapped data and
writes after a check remain within the documented cooperative limits; these
callbacks do not authenticate arbitrary trusted Lua semantics.

Those are production round requirements, not delivered behavior or reasons to
withhold this feasibility approval. History_read_v1, role/projection services,
X06 recorded interpretation/history/acceptance, X07 public cutover, X08 mandatory
external-profile acceptance, execution/RA01 and Linux remain open. Public init
still creates format1; the latest approved complete gate remains676/configuration.

All reviewer verification processes have finished. All owned paths and exclusive
Debug are released to the coordinator; no subsequent reviewer edits or builds
will follow final handoff. Reviewer finalization binds this report and evidence.
