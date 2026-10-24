# R12-A independent review

Disposition: **request-changes**.

Reviewed the frozen 14-file candidate against R12/R12-A and the approved ledger
contract. The shared checkout remained on `dev/planar-v2-vernacular`, HEAD
`3171c49e7b92d3abda01bfbb8bfb0c6b29d4c67a`. Product, reference and vendor files
were read-only during review. Reviewer ownership was limited to this report and
`tests/native/r12a-review-evidence/`. No fixes, Planar operational tools,
installed binaries/data, commits, installation or publication were used.

## Finding

**P2 — enforce real-file path semantics before calling SQLite.**
`connection::open` passes `path.string()` directly to `sqlite3_open_v2` after
checking only emptiness/NUL and timeout (`src/engine/ledger/sqlite.cpp:67–86`).
SQLite interprets the bare filename `:memory:` specially even without URI
opening; its vendored API documentation explicitly records that behavior
(`vendor/sqlite/b9c7/sqlite3.h`, database-opening documentation). Consequently:

- `connection::open(":memory:", open_mode::read_only, 0)` succeeds with no
  existing file.
- `connection::open(":memory:", open_mode::read_write, 0)` succeeds with no
  existing file, and a prepared `CREATE TABLE` succeeds.
- `scratch_create` at that path also creates only an ephemeral database; its
  successful writes disappear on close.

This violates the module's existing-file modes and explicit native-path
contract (`sqlite.cppm` opening documentation; `docs/architecture.md:104–106`).
A trusted caller supplying this legal native filename can silently access a
new ephemeral database instead of the requested file. The absence of URI flags
and use of NOFOLLOW do not prevent this SQLite pseudo-filename behavior.

Normalize an explicit filesystem path to an unambiguous file pathname before
passing it to SQLite, or reject unsupported pseudo-path spellings with a typed
failure. Preserve support for real files named `:memory:` through an unambiguous
pathname such as `./:memory:` or an absolute path. Add focused regressions for
missing-file read-only/read-write refusal and actual file-backed scratch
persistence, including the literal colon-prefixed filename distinction. Do not
weaken no-follow or add an implicit in-memory mode. The exact reproduction is
in `tests/native/r12a-review-evidence/probe.cpp`, with output in `probe.log` and
rebuild command in `probe-command.sh`.

## Independent verification and assessment

The following commands ran directly, with exclusive use of the debug build
directory and no simultaneous build process:

```sh
ctest --test-dir build/rewrite-debug -R 'replacement.ledger' --output-on-failure
cmake --build build/rewrite-debug --target all planar_tests
```

Focused CTest: **9 matched, 9 passed, zero skipped**. Incremental build:
`ninja: no work to do.` Raw output is in `focused.log` and `no-work.log` under
`tests/native/r12a-review-evidence/`. The extra compiled probe linked against
the candidate module and static libraries, exited zero, and confirmed:

- A transaction with pending `INSERT ... RETURNING` cannot falsely commit:
  typed `SQLITE_BUSY` is returned and rollback removes its inserted rows.
- Destroying a stale guard after starting a newer transaction preserves the
  newer transaction and its successful commit.
- The pseudo-filename behavior above is reproducible; an explicit scratch
  `./:memory:` path creates a real file.

The reviewed ownership structure retains the connection for statements and
transaction guards, finalizes statements before releasing retained state, and
handles moved-from objects explicitly. Binding uses explicit byte lengths and
SQLITE_TRANSIENT; column reads copy data and distinguish NULL, empty, text,
BLOB and integer without coercion. SQLite authorizer callbacks reject manual
transaction/savepoint control; wrapper transaction generations fence stale
guards; failed rollback marks the state poisoned. Caller thread confinement is
an explicit API precondition, consistent with wrapper state lacking locking.
No other concrete correctness finding was identified in this bounded review.

Independently compared all seven restored vendor files against Git HEAD: exact
matches. The historical dependency pin records the same SQLite 3.53.3 official
archive URL and SHA256 as the candidate, and the amalgamation carries its
copyright disclaimer. Offline CMake refuses missing amalgamation/header and
compiles only sqlite3.c with THREADSAFE=1, DQS=0 and DEFAULT_FOREIGN_KEYS=1;
legacy URI/FTS/opening/migration policies were not restored. The storage module
has no render/CLI/Lua import. Reviewed existing raw final lint evidence:
32 active source files passed formatting and Doxygen; no redundant broad gate
was run.

Candidate manifest SHA256:
`510e0f5453f66a6973462989aa51e02a1923171e1fa27a536674e15ebc01006d`.
Implementation-evidence manifest SHA256:
`3987f7f6706ef98ed737cff96eece4079942bfc1698e95af0749521a636068b0`.
All 14 candidate and 14 evidence entries matched before and after review.
Independently rehashed all **3189 protected files**, with zero changes. Reviewer
raw evidence includes those audits and final manifest checks.

Limitations: focused debug/macOS only. Full debug/release and Linux verification,
rollback-failure fault injection, backup, disk exhaustion and termination/restart
remain later R12 gates. This review does not claim safe format opening,
initialization, full ledger semantics or completion of R12. Preserve this
finding/candidate/evidence and start a fresh corrective search/coder/reviewer
round before advancing R12-A.
