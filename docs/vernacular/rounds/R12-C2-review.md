# R12-C2 independent final milestone review

Disposition: **approve**. No findings. The **R12 storage foundation gate is
satisfied within the limits below**. This closes the verification obligations
in R12-C2 and composes with the independently approved corrected R12-A, R12-B
and R12-C1 candidates. It does not claim implemented R13/R14 record, history,
acceptance or execution semantics.

## Candidate and scope

Fresh ordinary reviewer `/root/r12c2_reviewer` reviewed the shared
`dev/planar-v2-vernacular` checkout at HEAD
`3171c49e7b92d3abda01bfbb8bfb0c6b29d4c67a`. Product/test sources were read-only;
writes were limited to this report and new
`tests/native/r12c2-review-evidence/`. Other intentional rewrite edits and frozen
evidence were preserved. No Planar operational tools, old queue/skills,
installed binaries, operator data, commits, installation, publication or merge
were used. Both build directories were held exclusively; commands ran serially.

The review basis is CLAUDE.md, implementation plan/roadmap, relevant v4
requirements, the normative ledger contract, R12/R12-C/R12-C2 briefs, the earlier
A/B corrected approvals, full corrected C1 approval and exact unchanged ledger
sources, C2 baseline/diff/snapshots/generator/shims/tests, and raw evidence.
This is final R12 storage-foundation review, not approval based only on
instrumentation hit counters.

Seven-file candidate manifest SHA256:
`e567b706a8ccbafe5a53dcde3d8d06d36052cb93fc3ef14d3f9cb33a67454a56`.
42-entry coder evidence manifest SHA256:
`5d14c88e75c216f3eaed3afee2fa166cae91f1a45c8611370fb756943d8ba473`.
Candidate paths are repository-relative; evidence paths are folder-relative.
Before/after audits independently verify all current files, all candidate
snapshots and every evidence entry. All six ledger files from the corrected C1
manifest (`2a90623073df705d33b4c4db0cd37cd933f31e8bae48e9456e0cec5bba81202f`)
remain exact. Across the 11,401-entry pre-C2 baseline no file is missing; only
`docs/testing.md`, `src/engine/CMakeLists.txt` and expected coordinator R12/R12-C2
round records differ. C2 adds five verification source files. The frozen
protection audit agrees; no product or vendor fault switch was introduced.

## Fault assessment

`reliability.t.cpp:258` obtains real `SQLITE_FULL` (primary/extended 13) by
setting the actual database's page limit to its current page count and then
requesting a larger allocation. It verifies abandoned commit, the complete
ordered prior rows on the same connection and a fresh reopen, then a successful
complete retry and another reopen. This establishes SQLite configured-capacity
failure handling, without a claim of physical host-disk exhaustion.

`reliability.t.cpp:297` wraps the actual default VFS and enumerates each matching
commit callback through the first unhit successful commit. The independent
traces confirm four journal writes, two journal syncs, two main writes and one
main sync. Every injected case must fail commit with primary `SQLITE_IOERR` and
the expected WRITE/FSYNC extended code, retain all old rows on the owner and a
fresh connection, then commit/reopen the complete new state. Enumeration is
bounded at 64 and requires reaching its terminal successful case. The VFS
restores the original default; ordinary product-native cases still run.

`reliability.t.cpp:348` exercises actual online-backup destination write/sync
failure, preserves the source's complete rows and succeeds on a subsequent
backup using the same destination. The production wrapper always calls
`sqlite3_backup_finish` after initialization. Pinned SQLite at
`sqlite3.c:85413` returns its stored step result, mapping DONE to OK, and frees
the handle; its internal Btree rollback result is discarded. These tests
meaningfully verify step-error propagation and finish cleanup/reusability. An
independent finish-after-DONE error is correctly not claimed.

`reliability.t.cpp:369` kills real children before commit, inside actual journal
and main xSync entry callbacks, and after successful commit return. Each barrier
requires pipe readiness and a SIGKILL wait status. Reopen checks complete old
state at the first three boundaries and complete new state after return, plus
subsequent transaction usability. Repeated earlier stdout lines are inherited
fork buffers, not extra barrier iterations. Separate writer and reader processes
at lines 415 and 452 prove bounded busy failure, complete rollback, release and
successful retry. The writer exits normally after explicit rollback; the reader
is killed and reaped. This is observable storage atomicity with symbolic rows,
not an R14 acceptance/event/projection demonstration.

The rollback correction is legitimate. The preserved failed poison probes
expected an error that SQLite's SQL ROLLBACK API does not report:
`sqlite3RollbackAll` at `sqlite3.c:188857` discards the return of
`sqlite3BtreeRollback` at line 188879, and OP_AutoCommit calls that void path at
line 100633. A lower-level fault hit alone does not establish an API error.
The final case at `reliability.t.cpp:482` still requires the main rollback write
fault to hit, compares all 160006 prior-state bytes on the same owner before
opening another connection, compares the complete fresh-reopened old state,
and commits/reopens complete new state. Its 52 assertions pass independently
in both configurations. The safety obligation remains tested; poison-branch
coverage is explicitly unclaimed. No production fix or vendor patch was hidden
in this change.

## Publication and integration assessment

The generation script makes only declared module/namespace/import and three
POSIX-call substitutions. Independent reverse substitution reproduces exact
production bytes, and current source/generated hashes match the frozen
`generation.json` in both build directories. CMake makes both helper modules
EXCLUDE_FROM_ALL; the preserved default-all and product-link graph evidence
shows no test helper in the product graph. The algorithm is generated from the
current implementation, rather than maintained as a separate copy.

`publication_fault.t.cpp:34` requires each selected fault to hit and a typed
failure for initialization, backup and restore. Prepublication file-sync/link
errors leave no target. Parent-sync and staging-file/directory cleanup errors
return `publication_uncertain`; subsequent full supported-format opening
verifies the published target and preserved metadata, with source bytes
unchanged. Cleanup faults retain staging for diagnosis. Production
`publish_new` at `format.cpp:561` never removes the target after linking and
only removes staging whose retained ownership matches. Late sidecar arrival at
`publication_fault.t.cpp:75` refuses without publication and preserves sentinel
bytes for all three operations. A target arriving at link time is preserved
byte-for-byte by the no-replace operation at line 92.

The earlier A/B/C1 implementations remain exact: typed retained SQLite owners,
transaction rollback, schema/metadata bounds, initialized-state verification,
byte-preserving foreign/future/corrupt refusal, immutable constraints, online
backup and explicit quiescent recovery are covered by their corrected reviews
and the now-repeated complete ledger suite. The C1 super-journal footer guard
and regression remain unchanged and passing. Populated history still refuses
as unsupported rather than being presented as fully validated history.

Both final full-gate raw logs independently count 170 passing cases (169 native,
one CLI acceptance), zero skips, formatting/Doxygen on 37 active sources, and
a no-work final build. The release acceptance trace records 247 real CLI
processes, six native goldens and 60 host-path checks. The provisional logs are
properly excluded: the earlier debug run predates ordinal enumeration and the
earlier release run detects a mid-run edit through its failed no-work check.
The frozen final source identity, generated hashes and independent no-work
builds/focused executions support the final unsuffixed gates. No new concern
required another full-milestone run. The documented diagnostic-free Doxygen
SIGBUS retry in the final debug gate succeeded under the existing reviewed
policy; the independent lint run passed directly.

## Independent commands and results

Raw logs, command arrays, JUnit, counts and before/after audits are in the
reviewer evidence directory. Executed commands were:

```sh
python3 tests/native/r12c2-review-evidence/audit.py identity-before.json
cmake --build build/rewrite-debug --target all planar_tests
ctest --test-dir build/rewrite-debug -R replacement.ledger -V --output-junit <repo>/tests/native/r12c2-review-evidence/focused-debug.xml
cmake --build build/rewrite-release --target all planar_tests
ctest --test-dir build/rewrite-release -R replacement.ledger -V --output-junit <repo>/tests/native/r12c2-review-evidence/focused-release.xml
python3 scripts/rewrite-check.py lint --build-dir build/rewrite-debug
/opt/homebrew/opt/llvm/bin/clang-format --dry-run --Werror tests/native/r12c2-support/posix.cppm tests/native/r12c2-support/posix.cpp
bash scripts/cpp-lint-doxygen.sh tests/native/r12c2-review-evidence/support-Doxyfile
cmake --build build/rewrite-debug --target all planar_tests
cmake --build build/rewrite-release --target all planar_tests
python3 tests/native/r12c2-review-evidence/gate-audit.py
python3 tests/native/r12c2-review-evidence/audit.py identity-after.json
```

Every command exits zero. Each focused run selects **46**, passes **46**, fails
**0**, skips **0**, independently counted from JUnit. Both initial and final
builds report `ninja: no work to do.` Active formatting/Doxygen checks **37**
sources; supplementary shim formatting/Doxygen also passes. An initial
reviewer-only log-count expression assumed fixed CTest spacing and was corrected
to accept its padded `Test #` columns; it did not indicate a product/test failure.
`manifest.json` freezes this report and reviewer artifacts except itself.

Executed platform: macOS Apple Silicon, debug and release. Linux execution,
physical disk exhaustion, persistent/repeated device failure, arbitrary hostile
filesystem writers and stronger hardware durability are not claimed. Quiescent
recovery and cooperative local directory ownership remain explicit preconditions.
Full record/tree semantics, historical reconstruction, CAS/request replay and
accepted-declaration projections remain R13/R14 obligations. These limits do
not prevent approval of the bounded R12 storage foundation. Both rewrite build
directories are released; no verification process remains active.
