# R08-B-C01 coder evidence

Shared checkout `dev/planar-v2-vernacular`, starting HEAD
`3171c49e7b92d3abda01bfbb8bfb0c6b29d4c67a`. No commits, Planar tools,
installed binaries, installation, publication, delegation or changes to the
approved capture/Lua/kernel/goldens/contracts. Existing shared changes retained.
This is implementation evidence, not approval.

Owned candidate files are listed in `candidate.sha256`: the command resolver,
the acceptance entry point, and its new `bootstrap_paths.py` helper. The two
patch files describe modifications from the exact R08-B candidate; reconstructing
their before sides matched the original R08-B SHA256 entries. The new helper is
the complete additional file. No `bootstrap.t.cpp` changes were necessary.

## Correction

File inputs now validate their final filesystem entry without following a final
symlink, canonicalize the original parent without lexical cleanup, and retain
the final leaf for capture's no-follow open. Wrong types and invalid paths return
typed `io`, final file symlinks return `path_denied`, and diagnostics omit host
paths. Root directories use a separate filesystem component walk. It expands
nested relative/absolute symlink targets while preserving dot/parent meaning,
refuses when the final root is equivalent to a traversed link target, and permits
intermediate aliases whose targets are different directories. Native canonical
resolution plus an explicit 64-link expansion bound prevents unbounded walks.
Missing/non-directory inputs remain typed `io`; root link equivalence returns
`path_denied` before capture.

## Red before green

`red-cli.log` ran the new acceptance matrix before editing the product resolver.
`red-candidate.sha256` records that original product source and the then-current
test files. It demonstrates **39 failures among 60 host path checks**, including
all five symlink/parent input-routing cases and equivalent current/base link
spellings. The test import was subsequently moved after `dont_write_bytecode`
so the harness creates no Python cache. The matrix helper is unchanged.

Each of the five routing inputs has deliberately different bytes at the lexical
and canonical candidate. The tests compare complete successful envelopes against
the direct canonical invocation, show distinct independent oracle digests for
the lexical candidate, and call the existing independent retained-byte/digest
verification. Ordinary intermediate aliases, nested aliases, `./` and `../`
spellings succeed. Root link, slash, dot and child/parent spellings refuse for
both current and base. File trailing slash/dot and file/parent spellings,
directory-for-file, file-for-directory and missing-prefix paths cannot be cleaned
into valid inputs. All runs use scratch HOME/PLANAR_DB and the existing checks
for canonical diagnostics with no host path leakage.

## Commands and results

Commands run directly from the repository, with explicit candidate binary:

```sh
python3 tests/acceptance/bootstrap_cli.py build/rewrite-debug/bin/planar
cmake --build --preset debug --target all planar_tests
/opt/homebrew/opt/llvm/bin/clang-format -i src/cmd/planar/bootstrap.cpp
ctest --test-dir build/rewrite-debug -R 'replacement.(json|help|missing explicit|legacy commands|cli|render|runtime|schema|semantic|selection)' --output-on-failure --output-junit <repo>/tests/native/r08bc01-evidence/ctest.xml
python3 scripts/rewrite-check.py lint --build-dir build/rewrite-debug
git diff --check
```

- Initial build: success (`build.log`); post-format build: no work
  (`build-formatted.log`). Formatter introduced no changes.
- Scoped CTest: **83 tests**, 82 unit and one acceptance, **zero failures/skips**
  (`ctest.log`, `ctest.xml`).
- Standalone acceptance: **150 real CLI processes**, including **79 new path
  processes** supporting 60 path assertions; **six native golden comparisons**
  and independent canonical/hash/prose/section checks, zero skips
  (`green-cli.log`, final rerun `green-cli-final.log`).
- Formatting/Doxygen: success for **25 active C++ sources** (`lint.log`).
- Final no-change build: `ninja: no work to do.` (`no-change-build.log`).
  The earlier uncollected CTest session prompted a redundant no-change rerun;
  filesystem timestamps establish the earlier build was also after CTest ended
  (`ctest.xml` mtime 1792492292.856; earlier build-log mtime 1792492304.243).
- Remaining **23 R08-B candidate files** and all **20 R08-A-C01 approved files**
  match their prior manifests (`prior-r08b-check.log`,
  `prior-approved-check.log`). Original review evidence was not overwritten.
- `git diff --check`: success (`diff-check.log`).

The frozen candidate and evidence manifests identify exact content without a
commit. Debug macOS only; release/Linux and the integrated full gate remain R09.
Existing serializer envelope-limit evidence and limitations are unaffected.
