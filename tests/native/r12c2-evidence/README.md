# R12-C2 coder verification evidence

Shared checkout: `dev/planar-v2-vernacular`, HEAD
`3171c49e7b92d3abda01bfbb8bfb0c6b29d4c67a`. No product source or vendor edits.
This is a coder candidate, not independent approval. Only macOS execution is
claimed. All database/process work uses test-owned temporary roots.

## Commands and raw output

- `cmake --preset debug`: `configure-debug.log`.
- `cmake --build build/rewrite-debug --target all planar_tests`: `build-debug.log`.
- `ctest --test-dir build/rewrite-debug -R replacement.ledger -V`:
  `focused-debug.log`; final candidate count is 46, with zero skips.
- `make cpp-lint`: `lint-debug.log`.
- `make test-all`: `gate-debug.log`.
- `make test-all PRESET=release`: `gate-release.log`.
- The final release focused trace is `focused-release.log`.
- `ctest --test-dir build/rewrite-release -R ^replacement.cli$ -V`:
  `acceptance-release.log` exposes the otherwise-hidden successful CLI totals.
- Supplementary helper formatting/Doxygen checks are `support-lint.log`.

Final full gates must each show 170 registered/executed cases, zero skips,
247 CLI acceptance invocations with six computed goldens, active formatting and
Doxygen, and a no-work second build. Gate output and final identity/protection
manifests are authoritative, rather than this expected-count description.

## Fault coverage and limits

`SQLITE_FULL` is real SQLite capacity exhaustion configured with max_page_count,
not host-disk exhaustion. Test VFS faults wrap the actual default VFS and are
one-shot, scoped, process-local and restored afterward. Both commit and online
backup destination write/sync failures propagate typed SQLite codes. Full old
and new ordered rows are compared; no row pruning is accepted. Commit barriers
are reached through pipe notification from before commit, actual journal/main
xSync callback entry, and after successful commit return. Parent wait statuses
must prove SIGKILL. Writer contention ends with explicit rollback and normal
child exit; reader contention exercises failed commit and rollback.

The generated publication module is exact current production source except the
substitutions in `generation.json`. Reverse replacement verifies all other bytes.
The CMake target is test-only and EXCLUDE_FROM_ALL; no fault API reaches product.
Three operations each exercise file-sync/link prepublication errors and parent
sync/file cleanup/directory cleanup uncertainty after publication. Sidecar and
no-replace arrival checks preserve sentinel bytes. Original native tests also run.

## Exploratory probes: unsupported poison expectation, not product failures

`focused-debug-initial.log` is the first six-case addition (42 focused cases).
`focused-debug-poison-probe.log`, `poison-probe.log`,
`poison-header-first.log`, `poison-header-probe.log`,
`poison-header-unclassified.log`, `poison-header-complete.log`,
`poison-main-write.log`, and `poison-main-complete.log` preserve exploratory
failures from expecting SQL ROLLBACK to expose underlying I/O failure and poison
the wrapper. The first probe mixed generic write injection with read injection;
subsequent probes isolated journal read-at-EOF/header and main rollback writes.
The fault hit does not establish a SQLite API failure. EOF trailer reads may be
optional; more importantly pinned SQLite's SQL ROLLBACK path calls
`sqlite3RollbackAll` (sqlite3.c:188857), which discards `sqlite3BtreeRollback`'s
return (188879), from OP_AutoCommit (100633). The wrapper receives success.
The complete-state probes verify fresh reopen restores all 160006 old-state
bytes. `rollback-final-probe.log` verifies the same-owner complete old state,
fresh reopen complete old state, successful subsequent commit and full new
state after reopen: 52 assertions pass. Coordinator confirmed this is an API
observation limit, not a product finding; final assertions check actual safety.
No poison branch coverage is claimed.

`sqlite3_backup_finish` (sqlite3.c:85413–85463) maps stored `SQLITE_DONE` to OK
and otherwise returns the stored step result. Its Btree rollback result is also
discarded. VFS destination write/sync failures therefore verify failing step
followed by finish cleanup and subsequent usable destination, not an independent
finish-after-DONE failure. No vendor patch or fake API return was introduced.

## Frozen candidate and provisional integrated runs

The seven-file candidate manifest SHA256 is
`e567b706a8ccbafe5a53dcde3d8d06d36052cb93fc3ef14d3f9cb33a67454a56`.
`candidate/` retains exact source snapshots. No source changes follow this freeze.
The final ordinal VFS matrix discovers all nine matching commit callback positions:
four journal writes, two journal syncs, two main writes and one main sync. Each
position receives one error; the first beyond-last position commits successfully,
proving the enumeration reached the end rather than choosing arbitrary offsets.

`gate-debug-provisional.log` passes 170 cases but predates ordinal enumeration.
`gate-release-provisional.log` passes 170 cases, then correctly fails the no-work
check because the verification source changed during that run. It is provisional
build evidence, not a product failure or final gate. Final debug/release gates
run serially against the frozen candidate and use the unsuffixed gate filenames.
The initial helper-Doxygen setup selected no output format and was corrected to
XML before the passing `support-lint.log`; no product behavior changed.

Successful focused stdout can repeat earlier parent boundary lines: the child
flushes inherited pre-fork stdout when emitting its xSync callback trace. These
are duplicated buffered lines, not additional subprocess executions. The test
performs four commit-boundary iterations and requires a SIGKILL wait status for
each; the two interior barriers originate inside actual VFS xSync callbacks.

Final debug and release gates both pass 170/170 cases (169 native plus one CLI
acceptance), zero skips, formatting/Doxygen across 37 active sources, and a
no-work second build. Supplementary shim formatting/Doxygen passes separately.
The release verbose acceptance rerun records the exact 247-process/six-golden
result suppressed by successful CTest output in the integrated gate. The final
release verbose ledger run records 46/46 cases, zero skips.
