# R07-B2B candidate evidence

This is implementation evidence, not reviewer approval. Shared checkout; no
Planar commands, installed binaries, commits, installation or fixture edits.
The coordinator owns round records. Only the five product/test files listed in
`candidate.json` and this evidence directory were written by this coder.
`candidate.json` records exact SHA256 bytes and a combined product snapshot
identity; it is not a Git commit or a runtime identity recipe.

The production `validate_bundle` export always normalizes with B1 first and
validates every job. Its exact intermediate model and topology-only capture
interface are documented in `stdlib/planar/README.md`. B2C owns W, source byte/
mode identities and role scoping; R08 owns observations and projection. Directory
metadata never becomes a file provenance or identity input in this pass.

## Raw progression

- `initial-tests.log`: first implementation against ten production-export
  cases. Eight passed; two exposed cascading output-vector errors masking
  missing dependency references and redundant no-worker diagnostics.
- `tests-initial-corrections.log`: all ten cases pass, 695 assertions, after
  deferring reference-dependent subscription/applicability checks and removing
  redundant missing-assignment diagnostics when no worker exists.
- `tests-expanded-red.log`: twelve of thirteen expanded cases passed. The new
  captured-file-ancestor/facet regression failed before its fix: an ancestor
  regular file was accepted as compatible with a declared file facet.
- `tests-green.log`: all thirteen cases pass, 800 assertions, after checking
  facet topology against captured files as well as directories.
- `tests-final.log`: sixteen semantic cases pass, 916 assertions. This includes
  the real quota current/base capture adapter after deleting its temporary
  source trees, both grouping models, all phase/quality combinations, full
  ownership unions/segment aliases/empty directories, exact closed cycle
  traces, per-unit worker-added checks and eight adjacent-stage obligations
  over six units with grouped workers and a multi-worker stage.

Every fixture mutation occurs in an immutable structural copy or its per-test
scratch directory. Expected packets, declarations and reviewed fixture files
are unchanged. No expected identities or render goldens are replaced.

## Final commands and checks

Commands were serialized for `build/rewrite-debug`. The final commands are:

```sh
cmake --build build/rewrite-debug --target all planar_runtime_tests
build/rewrite-debug/src/kernel/planar_runtime_tests '[r07b2b]'
ctest --test-dir build/rewrite-debug -R 'replacement.(runtime|schema|semantic)' --repeat until-fail:2 --output-on-failure --output-junit <repo>/tests/native/r07b2b-evidence/ctest.xml
python3 scripts/rewrite-check.py lint --build-dir build/rewrite-debug
cmake --build build/rewrite-debug --target all planar_runtime_tests
git diff --check
```

`build-final.log`, `tests-final.log`, `ctest.log`, `ctest.xml`, `lint.log` and
`no-change-build.log` retain raw output. `counts.log` validates 51 nonzero unique
cases: 18 bridge, 17 B1 schema, 16 B2B semantic. Every case ran twice, so 102 test
executions passed with zero skips. The direct new suite reports 916 assertions.
Pinned formatting and Doxygen passed for 22 active first-party source files.
The final build reports `ninja: no work to do.`; `git diff --check` was clean.

This is the focused macOS debug scope, not the R09 complete integration gate or
a Linux execution claim. It establishes neither execution/check success for
the declared application nor rendered role packets, W, completion or approval.
