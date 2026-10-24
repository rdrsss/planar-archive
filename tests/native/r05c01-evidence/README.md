# R05-C01 verification evidence

Candidate implementation evidence; independent review remains required. Commands
ran serially in `build/rewrite-debug` on macOS arm64 with the configured pinned
toolchain. Linux and release-mode verification were outside this corrective scope.

Two new tests retain three-byte text/binary files and grow them with a deterministic
pre-verification callback to 1,048,577/16,777,217 bytes using sparse file resize.
`regression-before-fix.log` records both failures against the original capture
implementation: expected `capture_changed`, actual `resource_limit/file_bytes`.
The selected two-case CTest command exited 8. `regression-build.log` records its
successful test build before the production fix.

The fix supplies an explicit internal verification flag to the bounded reader.
An oversized final reread returns `capture_changed` at the size check before
allocation or content reads. Initial discovery retains `resource_limit/file_bytes`;
the existing text/binary discovery cases now assert both fields explicitly.

Executed commands and outcomes:

- `cmake --build build/rewrite-debug --target all planar_tests`: passed (`build.log`).
- `ctest --test-dir build/rewrite-debug -R 'replacement.(loader|capture)' --output-on-failure --output-junit tests/native/r05c01-evidence/native.xml`:
  17 matched/executed cases passed, zero skipped (`focused-tests.log`, `native.xml`).
- `ctest --test-dir build/rewrite-debug -R '^replacement.cli$' --output-on-failure --output-junit tests/native/r05c01-evidence/cli.xml`:
  one acceptance case passed, covering the existing 22 subprocess scenarios,
  zero skipped (`cli-test.log`, `cli.xml`).
- `python3 scripts/rewrite-check.py lint --build-dir build/rewrite-debug`:
  pinned formatting and Doxygen passed for 12 active sources (`lint.log`).
- Repeated `cmake --build build/rewrite-debug --target all planar_tests`:
  `ninja: no work to do.` (`nochange.log`).

CTest writes relative JUnit paths beneath its build directory; the two reports
were copied here and checked for exact counts, no failures and no skipped cases.
`candidate.sha256` lists both changed source files and all evidence files except
itself, sorted by path. No other product scope was changed for this correction.
