# R12-A-C01 correction evidence

Implementation handoff awaiting fresh independent review; this is not approval.
Shared branch `dev/planar-v2-vernacular`, HEAD
`3171c49e7b92d3abda01bfbb8bfb0c6b29d4c67a`. No commits, Planar tools,
installed binaries, operator database/data, install, publication or merge used.
Prior R12-A implementation and review evidence remains byte-identical.

The correction changes only `src/engine/ledger/sqlite.cpp`, `.cppm`, `.t.cpp`,
affected primitive references `docs/architecture.md` and `docs/testing.md`, and
this new evidence directory. Coordinator owns round records.

## Change

Validate original empty/NUL bytes first, then use `filesystem::absolute` with
`error_code` to disambiguate native `:memory:` and URI-looking names. Absolute
conversion failure returns typed SQLITE_CANTOPEN. No canonicalization, lexical
normalization or link resolution is introduced; SQLite URI flag stays absent,
and NOFOLLOW, existing-file modes and explicit scratch CREATE stay enabled.

A preserved initial probe also exposed SQLite's own collapse of `missing/../db`:
it could create `db` even though the original parent traversal is invalid.
The final implementation checks `filesystem::is_directory` on the unnormalized
absolute parent before SQLite sees the path, returning typed SQLITE_CANTOPEN
when traversal fails. Final tests retain this original regression, verify
missing/db refusal, symlink/../ refusal and ordinary directory/../ success.
This is no hostile race guarantee or safe ledger format opener. R12-B retains
format-aware opening responsibility; no traversal regression is deferred.

## Red evidence

`sqlite-before.cpp` is the unchanged reviewed implementation. The first test
source is frozen as `regression-initial.t.cpp`; `build-red.log` records its build.
Command:

```sh
cmake --build build/rewrite-debug --target all planar_tests
ctest --test-dir build/rewrite-debug -R 'replacement.ledger.*(bare memory|URI looking|unavailable cwd)' --output-on-failure --output-junit tests/native/r12ac01-red.xml
```

`red.log`: 4 matched, 3 failed, 1 passed; command exit 8. Bare :memory: wrongly
succeeded in both existing modes and scratch failed regular-file persistence.
The URI-looking filename itself persisted; its case failed on the extra
missing/../ traversal assertions. The unavailable-cwd case already passed.
No failing behavior was removed from the final tests. An intermediate candidate
changed that traversal assertion to missing/db and passed (`build-green.log`,
`focused.log`, `focused.xml`, `lint.log`, `no-work.log`), but those are superseded
by the final correction that restores and fixes the original traversal assertion.

## Final verification

Direct commands ran serially in the debug build directory, macOS Apple Silicon:

```sh
/opt/homebrew/opt/llvm/bin/clang-format -i src/engine/ledger/sqlite.cpp src/engine/ledger/sqlite.cppm src/engine/ledger/sqlite.t.cpp
cmake --build build/rewrite-debug --target all planar_tests
ctest --test-dir build/rewrite-debug -R 'replacement.ledger' --output-on-failure --output-junit <repo>/tests/native/r12ac01-evidence/focused-final.xml
python3 scripts/rewrite-check.py lint --build-dir build/rewrite-debug
cmake --build build/rewrite-debug --target all planar_tests
```

`build-final.log`: success. `focused-final.log`/XML: **13 matched, 13 passed,
zero skipped**. All prior 9 cases plus 4 filename/path cases pass. The new cases
cover bare missing existing modes, scratch persistence and bare/./absolute
reopens, literal URI-looking filename persistence, original empty/NUL refusal,
missing parents and dot traversal, NOFOLLOW traversal and removed-cwd errors.
Catch2 runs serially per executable; cwd RAII restores before scratch cleanup,
and each CTest process has its own cwd and temporary root.

`lint-final.log`: pinned formatting and Doxygen pass over **32 active sources**.
`no-work-final.log`: `ninja: no work to do.` Registry enumeration is 137 cases;
the full registry was not run. Full debug/release and Linux gates remain later
R12-C work. No broad gate was run for this bounded correction.

`protected-before.json` freezes 3248 files before edits; `protected-audit.json`
confirms 3246 unchanged, with the two explicitly owned reference edits separated.
Their original bytes are `architecture-before.md` and `testing-before.md`.
Historical R12-A/review logs and probes are among the protected files.

`candidate.sha256` freezes the full corrected 14-file R12-A candidate (including
unchanged dependency/build/vendor bytes), not merely this correction.
`correction.diff` compares implementation against `sqlite-before.cpp`;
`sqlite-final.diff` snapshots its complete source. `evidence.sha256` covers this
directory except itself. Validate hashes from repository root using
`shasum -a 256 -c` on either manifest. Candidate identity must remain current
through the independent review.
