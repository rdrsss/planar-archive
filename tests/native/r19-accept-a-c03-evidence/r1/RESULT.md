# R1 result (PKG-R1, R19-ACCEPT-A-C03)

Files edited: `fixtures/ledger/expected-schema.sql` (new, copy), `src/engine/ledger/CMakeLists.txt` (line 41),
`scripts/linux-gate.sh`, `scripts/linux-gate.Dockerfile`, `scripts/linux-gate.Dockerfile.dockerignore`,
`docs/testing.md`, `tests/native/r19-accept-support/ignored_context_check.sh` (new),
`tests/native/r19-accept-a-c03-evidence/r1/` (this directory).

## Fixture (C3-R1a, R1e)

`raw/fixture-identity.txt`: `cmp -s` exit 0, both sha256 equal
(`730e38be112f8024a8a8eea232a2c9728ea1f23aae0f5ea90e14e200347d5395`), `git check-ignore` exit 1 (not ignored).
Old copy untouched: `raw/old-copy-before.sha256` and `raw/old-copy-after.sha256` are identical.

## Product tree (C3-R1d)

`git diff --numstat HEAD -- src/engine/ledger/CMakeLists.txt` prints `1	1`. Identity after, pathspec src:

| identity | value |
|---|---|
| HEAD | `91e0df24f2167471e49ec057c7159bce5dff3608` |
| sha256(git diff --binary HEAD -- src), tracked only | `d9ca75a13fcf1c3aa11b79ee38fdb5d24d733501deb4caf24c42d1830a6726c0` |
| combined sha256 (diff bytes, then each untracked path NUL bytes, sorted), pathspec src, evidence excluded | `d9ca75a13fcf1c3aa11b79ee38fdb5d24d733501deb4caf24c42d1830a6726c0` |

| file | state | sha256 |
|---|---|---|
| `src/engine/ledger/CMakeLists.txt` | modified | `8141a8d56f504208eafd8f4ae6145e49f3f71e34f1300970bf7b678056b7779d` |
| `src/engine/ledger/execution.t.cpp` | modified | `250867b8bd74b86d718cbe17e5fa9983fda6a60a2b5e7c127829a3fec6f98950` |
| `src/engine/ledger/storage.cpp` | modified | `e4cf41dcb8cf44cc8004d4ba011543a557c742c623e20ede631eb975f237d822` |

`storage.cpp` (`e4cf41dc...d237d822`) and `execution.t.cpp` (`250867b8...98950`) hashes equal the pre-fix values.
Full post-fix identity for the five pathspecs: `raw/identity-after.txt`.

## SCHEMA612 (C3-R1b, R1c)

See RED.md: failed at format.t.cpp:115 on the clean export before the fix, passed after (1 passed, 0 skipped).

## Observer (C3-R1q)

Driver: `bash tests/native/r19-accept-support/ignored_context_check.sh scripts/linux-gate.sh <repo>`.
Before the edit: `raw/ignored-context-before.txt` is `tests/native/r12b-evidence/expected-schema.sql`, `count: 1`.
After: `raw/ignored-context-after.txt` is `count: 0`.
Verbatim one-line equivalent:

```
sed -n '/^is_excluded()/,/^}/p' <runner> > <scratch>/isx.sh; . <scratch>/isx.sh; git -C <repo> ls-files -z --others --ignored --exclude-standard | while IFS= read -r -d '' p; do is_excluded "$p" || printf '%s\n' "$p"; done
```

## Recipe pins (C3-R1f to R1i)

`python3 tests/acceptance/x08_recipe.py linux_recipe_pins x .` exit 0 (`raw/x08-after.txt`, `raw/x08-after.exit`).
`grep -n 'r12b-evidence/expected-schema' docs/testing.md` exits 1 (C3-R1k).

## Why the Linux gate passed (C3-D6), pre-edit line numbers

The gate context held a file a clean clone lacks, through four cooperating sites:
1. `scripts/linux-gate.sh` write_manifest, lines 125-129: an explicit `printf` appended the ignored schema path to the git list.
2. `scripts/linux-gate.sh` is_excluded case, lines 101-104: returned "not excluded" for that path.
3. `scripts/linux-gate.Dockerfile.dockerignore` re-include, line 47: `!tests/native/r12b-evidence/expected-schema.sql` re-included it after the evidence exclusion.
4. `scripts/linux-gate.Dockerfile` test -s, line 91: asserted the file, which was therefore present only because of sites 1 to 3.

All four were removed; the ledger format tests now read the tracked fixture.

## Cleanup

Scratch export removed (629M including its own build tree, `raw/scratch-export-size.txt`). No Docker command was run.
