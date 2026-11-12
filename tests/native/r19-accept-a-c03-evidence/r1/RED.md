# R1 red-first evidence (C3-R1c)

Method (C3-D7): scratch export of exactly the files listed by
`git ls-files -z --cached --others --exclude-standard` (working-tree bytes; the ignored
`tests/native/r12b-evidence/expected-schema.sql` is therefore absent), `cmake --preset debug`
inside the export, build target `planar_ledger_format_tests`, then
`ctest --test-dir build/rewrite-debug -R "ledger format initializes exact metadata schema" --no-tests=error`.
The checkout build directories were not touched.

## Pre-fix identity (identity.py src fixtures scripts tests/acceptance docs/testing.md, verbatim)

| identity | value |
|---|---|
| HEAD | `91e0df24f2167471e49ec057c7159bce5dff3608` |
| sha256(git diff --binary HEAD -- src fixtures scripts tests/acceptance docs/testing.md), tracked only | `2cb15eb83efa6c00d393d1e339deac84d30378231c9bc8c304909e32cec6f418` |
| combined sha256 (diff bytes, then each untracked path NUL bytes, sorted), pathspec src fixtures scripts tests/acceptance docs/testing.md, evidence excluded | `2cb15eb83efa6c00d393d1e339deac84d30378231c9bc8c304909e32cec6f418` |

| file | state | sha256 |
|---|---|---|
| `src/engine/ledger/execution.t.cpp` | modified | `250867b8bd74b86d718cbe17e5fa9983fda6a60a2b5e7c127829a3fec6f98950` |
| `src/engine/ledger/storage.cpp` | modified | `e4cf41dcb8cf44cc8004d4ba011543a557c742c623e20ede631eb975f237d822` |
| `tests/acceptance/x08_recipe.py` | modified | `cc04a0fbc557759a6e912596488969ce7308a507b8fde9b19f001a9c4be71014` |

## Export list (NUL separated)

Before the fix: `raw/export-list-before.sha256`; `grep -zc '^fixtures/ledger/expected-schema.sql$'` prints 0
(`raw/export-list-before-grep.txt`), so the export lacked the fixture.
After the fix: `raw/export-list-after.sha256`; the same grep prints 1 (`raw/export-list-after-grep.txt`).

```
3f8d205bfa6aa3b91353e94d1144be74221d33f4796b0be42c97df8fff03dd04  r1-export-before.list
68ff3c11b8755aa6792d2e980babed3f81e86396ef7c5ac80f36eba66eff4bf9  r1-export-after.list
```

## Pre-fix run (behavioural red)

Configure exit 0 (`raw/red-configure.log`), build exit 0 (`raw/red-build.log`, `raw/red-build.exit`),
ctest exit 8, 1 test failed (`raw/red-ctest.log`, `raw/red-ctest.exit`). Rerun with output
(`raw/red-ctest-output.log`): `format.t.cpp:115: FAILED: REQUIRE_FALSE( expected_sql.empty() )`.

## Post-fix run

The fixture and the edited `src/engine/ledger/CMakeLists.txt` were copied into the same export,
incremental build exit 0 (`raw/green-build.log`), same ctest command exit 0, `100% tests passed, 0 tests
failed out of 1` (`raw/green-ctest.log`, `raw/green-ctest.exit`); no test skipped.
