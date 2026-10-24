# R12-A candidate evidence

Candidate starts from HEAD `3171c49e7b92d3abda01bfbb8bfb0c6b29d4c67a` in the
shared breaking-rewrite checkout. Existing rewrite changes were retained.
This is implementation evidence awaiting independent review, not self-approval.

## Scope and invariants

Restored exact HEAD SQLite 3.53.3 committed sources and cache metadata; archive
provenance is `https://sqlite.org/2026/sqlite-amalgamation-3530300.zip`, SHA256
`646421e12aac110282ef8cc68f1a62d4bb15fc7b8f09da0b53e29ee690500431`. The public-domain
notice is at the top of the amalgamation. Only `sqlite3.c` is compiled. Definitions
are THREADSAFE=1, DQS=0 and DEFAULT_FOREIGN_KEYS=1. No configure-time download.
`vendor-audit.json` records every restored file and exact identity against HEAD.

The standalone native module retains connection ownership through statements and
transaction guards, copies bound/result bytes, preserves SQLite primary/extended
errors, and uses explicit paths/modes with a 0..5000ms busy timeout. All related
objects are confined to one caller thread. SQL transaction/savepoint control is
denied by the SQLite authorizer, with internal control scoped to synchronous
BEGIN/COMMIT/ROLLBACK. Prepare/bind/step failure rolls back an active transaction;
failed commit returns its original error and attempts rollback. Failed rollback
poisons the state. Guard generations prevent an invalid guard affecting a newer
transaction. Move assignment rolls back the replaced guard.

This is a trusted SQL primitive, not a safe ledger opener. Scratch CREATE can
open an existing SQLite file and does not provide exclusive publication. No
schema/format verification, ledger initialization, record/tree validation,
backup/restore, CLI, Lua binding, acceptance or execution is implemented. No
storage module link was added to render. SQLite NOFOLLOW also refuses symlinked
ancestors; test roots canonicalize macOS `/tmp`. Read-only/read-write missing
paths never create a database or parent directory. No operator data or installed
Planar was touched. No commits, install, publication or merge.

## Executed verification

Commands ran directly and serially per build directory on macOS Apple Silicon:

```sh
cmake --preset debug
cmake --build build/rewrite-debug --target all planar_tests
/opt/homebrew/opt/llvm/bin/clang-format -i src/engine/ledger/sqlite.cppm src/engine/ledger/sqlite.cpp src/engine/ledger/sqlite.t.cpp
ctest --test-dir build/rewrite-debug -R 'replacement.ledger' --output-on-failure
python3 scripts/rewrite-check.py lint --build-dir build/rewrite-debug
cmake --build build/rewrite-debug --target all planar_tests
```

`configure.log`, `build.log` and `build-final.log` preserve configure/build output.
After final formatting and source edits, `build-final.log`, `focused-final.log`,
`lint-final.log` and `no-work.log` are the final candidate checks:

- Build `all planar_tests`: success.
- Focused CTest: **9 matched, 9 passed, zero skips**, covering bytes/ownership,
  reset clearing, moves/lifetimes, transactional commit/destruction/move assignment,
  UNIQUE and deferred FOREIGN KEY failure rollback, bounded two-connection busy
  refusal/retry, read-only noncreation/no-follow, manual transaction denial, and
  invalid bind/prepare rollback.
- Formatting/Doxygen: success over **32 active source files**.
- No-change `all planar_tests`: `ninja: no work to do.`
- Current registry: **133 tests** (132 native + one Python acceptance). The full
  registry was enumerated, not run for this bounded round.

`focused-initial.log` records the first 9/9 failure: scratch paths still used
macOS's `/tmp` symlink and SQLite NOFOLLOW refused them. Canonical scratch roots
fixed the fixture, while product no-follow remained enabled. `lint-initial.log`
records missing API documentation before completion. These are preserved as
initial failure evidence, not successful verification. Later final logs replace
the transient intermediate runs; all final commands above passed.

`protected-before.json` froze 3189 existing files outside the owned paths under
source/vendor/CMake/config/fixtures/tests before any product edit.
`protected-audit.json` confirms all 3189 remain unchanged. Coordinator-owned
round/roadmap files are outside that audit. `candidate.sha256` freezes all owned
product/reference/vendor bytes; `evidence.sha256` freezes evidence files other
than itself. Verify manifests from the repository root with `shasum -a 256 -c`.

Limitations: only the focused debug/macOS gate ran. Full debug/release and Linux
validation remain later gates. SQLite busy handling is bounded by its busy
handler timeout, not a hard real-time bound on arbitrary OS I/O. Broader VFS,
disk-full, backup, termination/restart and format invariants belong R12-B/C;
R13/R14 own codecs and acceptance. No claim of complete ledger validation is made.
