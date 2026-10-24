# R13-B1 independent review evidence

Fresh reviewer `/root/r13b1_reviewer` worked in shared checkout
`dev/planar-v2-vernacular`, HEAD
`3171c49e7b92d3abda01bfbb8bfb0c6b29d4c67a`. Only this directory and
`docs/vernacular/rounds/R13-B1-review.md` were authored. Product, candidate,
prior evidence, vendor and operator data remained unchanged. No Planar tools,
legacy queue/skills, installed binaries, commit, install or publication.

`audit.py` independently compares all ten current candidate files and frozen
snapshots, fifty coder evidence entries and 1,834 protected files. Before/after
results are retained. The candidate manifest SHA256 is
`a654e1fe4444074f8b59ac82e031f5fbd9b148bf43fc5bb94de71fc9377e71e7`;
coder evidence manifest SHA256 is
`d52c516e07573c326960b4a0ddd53b08d04c19ddc9668f44e73b100367d9c4a0`.

Executed commands and logs:

- `python3 tests/native/r13b1-review-evidence/audit.py identity-before.json`
- `cmake --build build/rewrite-debug --target all planar_tests`: `build.log`,
  exit zero, exactly `ninja: no work to do.`
- `ctest --test-dir build/rewrite-debug -R 'replacement.(ledger|canonical|hash|record)' --output-on-failure --output-junit <absolute review directory>/focused.xml`:
  `focused.log` and JUnit, 76 passed, zero failures/skips. Exact argv in `commands.json`.
- `python3 tests/native/r13b1-support/audit.py`: `generation-audit.log`, exit zero;
  protected identities, both independently regenerated exact/reversible variants,
  coder test count, mutation RED and restored protected variant confirmed.
- `python3 tests/native/r13b1-review-evidence/build-probe.py`: `probe-build.log`,
  exit zero. Compiles independent `snapshot.t.cpp` using the existing exact
  compilation command/module map, then links the unchanged candidate static
  libraries and pinned Catch2. No product source substitution or CMake edit.
- `tests/native/r13b1-review-evidence/snapshot-probe '[snapshot]'`:
  `snapshot-red.log`, expected Catch2 exit 42 retained in `snapshot-command.json`;
  two cases, one passes / one fails, 47 assertions, 46 pass / one fails.
- `python3 tests/native/r13b1-review-evidence/audit.py identity-after.json`

The probe wraps the pinned SQLite default VFS in its own serial executable;
its structure is adapted from the protected reliability tests. It injects one
real `SQLITE_IOERR_READ` at the original VFS xRead boundary and then disables the
fault. It never substitutes the product or codec/hash algorithms. Every scope
restores the prior default VFS; all databases are per-test temporary paths.

The reader case commits eight distinct 1 MiB file records, opens the verified
reader, then injects a read fault while reading an evicted early record. It
requires that the VFS fault actually hits and that the API refuses with `io`.
While that reader remains alive, an independent writer successfully commits
record 9. The original reader then exposes record 9, which did not exist in its
verified snapshot. The sole failing assertion rejects that exposure. This is
a real candidate snapshot-preservation failure, not a missing fault hit or a
harness exception. The same probe's writer case proves a read fault invalidates
the writer and rolls back its uncommitted record 9, while retaining old records.

`snapshot-initial.t.cpp` and `snapshot-initial.log` retain an initial supplemental
writer harness mistake: it assumed writer record 1 must be evicted; that read
was cached, so the required fault-hit assertion failed. The corrected writer
case scans the original eight records only until the first actual read fault,
still requires exactly one hit and an io refusal, and passes. The reader bug
reproduced identically before and after this verification-only correction.
Derived reviewer objects/binary were removed after execution; the reproducible
source and exact compile/link command remain.

The coder's mutation RED was also examined as raw bytes: it removes only the
intern equality condition, and actual failing return/success/rollback assertions
include all four kind iterations and local-ID-only directory differences. The
ambiguous-bucket case continues passing under that mutation because it tests
the separate full-open validator. Normal generated variants and all product
sources remain restored and unchanged.

Executed platform is macOS Apple Silicon debug. Full integrated R13 gates,
release/Linux execution, tree expansion limits/B2, complete provenance and R14
acceptance semantics remain outside this review. No broad gate was repeated
once the blocking independent regression was established. The report is hashed
separately; `evidence.json` covers retained evidence except itself.
