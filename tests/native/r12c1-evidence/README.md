# R12-C1 native backup and recovery candidate

Shared checkout `dev/planar-v2-vernacular`, starting HEAD
`3171c49e7b92d3abda01bfbb8bfb0c6b29d4c67a`. The existing uncommitted rewrite
was preserved. This is a candidate for fresh independent review, not approval or
completion of R12. No installed Planar, operator data, queue, commit, installer,
publisher or merger was used. Tests create per-case canonical temporary roots.

## Candidate and protected state

Nine-file `candidate-manifest.json` uses repository-relative paths and includes
six ledger source/test files, unchanged engine CMake registration and two affected
reference documents. `candidate/` is the exact candidate snapshot; `baseline/`
contains those files before edits, and `candidate.diff` is the C1-only patch.
Candidate manifest SHA256:
`56fdd9f733c5fff1c99e1b9cbb1dfb7d3391535f147b383262250ff46d9a4699`.

The baseline audit hashes 3,345 source/vendor/CMake/native-evidence/document files
(excluding this new evidence directory and coordinator-owned round records).
3,337 are unchanged, eight have permitted C1 edits, none is missing, and there
are no unexpected changes. All vendor, renderer/CLI/Lua/codec sources and every
previous evidence file remain unchanged. `frozen-contract-audit.json` separately
proves byte identity of the SQL schema block, complete schema/state verifier
(including octet_length byte guards) and ordinary opener. CMake needed no change.

## Implementation

`connection::online_backup_to` encapsulates SQLite backup init/step/finish without
exposing a handle. It always finishes an initialized backup, preserves primary
and extended SQLite result codes, and refuses moved-from/poisoned owners,
self-backup and an active destination transaction. A pinned source read snapshot
is supported. There is one complete step and no retry loop extending the
configured busy wait; the new-target/internal primitive precondition is explicit.

`backup_new` validates the source and backs up its retained snapshot into owned
staging. `restore_new` follows the same verified SQLite protocol. Both preserve
identity/workspace/time and share initialization's no-follow, exclusive staging,
early/late destination sidecar checks, sync and no-replace link publication.
The staging database is closed and fully verified, including exact expected
metadata, before publication. Uncertain published targets are never removed.

`recover_quiescent` has an explicit whole-operation quiescence/cooperative-directory
precondition; it cannot detect or enforce that precondition against external
writers. It pins regular main/journal files, rejects competing WAL/SHM entries,
checks main/header/version and hot-journal dimensions, and captures exact input
bytes in owned 0700 scratch with fixed-size copy/compare buffers. SQLite recovers
only the working scratch pair first. Full initialized-state verification, exact
recovered size and journal consumption must pass. Original identities and bytes
are rechecked before SQLite recovers the original; full verification and metadata
comparison repeat afterward. No SQLite VFS lock method is invoked outside the
pager. Ordinary `open_existing` is byte-identical and remains nonmutating and
sidecar-refusing. Populated history remains unsupported pending R13/R14.

## Commands and results

Commands were serialized using the granted debug directory:

```sh
cmake --preset debug
cmake --build build/rewrite-debug --target all planar_tests
ctest --test-dir build/rewrite-debug -R 'replacement.ledger' --output-on-failure
python3 scripts/rewrite-check.py lint --build-dir build/rewrite-debug
cmake --build build/rewrite-debug --target all planar_tests
```

- `configure.log`: successful debug configure.
- `build-initial.log`: initial new-code compilation caught a missing aggregate
  field initializer; fixed before functional verification.
- `build-tests.log`: successful all+planar_tests with new functional cases.
- `focused-initial.log`: 34/35 passed; one incorrect corruption fixture expectation.
- `build-test-correction.log`, `build-refined.log`: successful subsequent builds.
- `focused-final.log`: 35/35 focused ledger cases passed, zero skips (14 primitive,
  21 format cases). Seven new C1 cases were added to the prior 28.
- `lint-initial.log`: pinned clang-format --Werror and Doxygen passed for all 35
  active C++ files. No source changes occurred after this lint run.
- `no-work.log`: final all+planar_tests build reports `ninja: no work to do.`

The initial corruption test wrote offset 100 of the main file, which was covered
by the genuine rollback journal. SQLite correctly repaired that uncommitted
corruption, so expecting recovery refusal was wrong. The corrected fixture reads
the events rootpage/page size and corrupts that unrelated page; corruption remains
after rollback and scratch validation refuses the original pair unchanged. The
raw failing log is retained. This was a fixture correction, not weakening a
corrupted-recovered-state refusal and not a discovered product regression.

The real crash test forks a child, uses SQLite to insert a large uncommitted
record and `sqlite3_db_cacheflush` to sync a genuine hot journal and dirty main
pages, then signals readiness through a pipe. The parent kills it with SIGKILL,
checks changed main bytes and sidecar presence, demonstrates ordinary opener
refusal without mutation, then verifies explicit recovery restores initialized
state. The redundant-journal fixture retains that real journal but reconstructs
the pre-transaction main bytes: this models death before the first main write;
it is not claimed as direct interception of that crash boundary.

Backup/restore refusal loops cover every regular/symlink/directory target and
journal/WAL/SHM suffix. Invalid source loops cover foreign, future, wrong schema,
populated, sidecar-bearing, truncated and symlink inputs. Recovery's fourteen
variants cover deceptive matching-app/version schema, future/foreign headers,
unsupported populated state, invalid magic/truncation/dimensions, unjournaled
corrupt event pages, unsafe journals and competing sidecars. Refusals compare
recursive entries and exact file bytes, with no target/staging residue.

## Limits carried to C2

This host run is macOS/arm64 debug only. Broad injected syscall/VFS I/O, actual
full-storage, publication/cleanup/sync uncertainty, hostile process/concurrency
and comprehensive transaction-boundary termination tests, plus full debug/release
and renderer gates, remain fresh C2 work. No claim is made here to have exercised
poisoned-owner or backup-finish I/O failures; those paths are checked in code but
need C2 fault injection.

SQLite owns journal interpretation and checksum behavior; rollback journal
checksums are not authentication. Full scratch validation establishes this bounded
supported recovered state, not provenance or arbitrary malicious corruption
proof. Redundant genuine journals can legitimately leave main bytes unchanged.
Failure after original writable recovery starts may have completed rollback;
it is reported as failure without attempting to overwrite or remove the original.
