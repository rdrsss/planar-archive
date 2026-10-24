# R13-B1-C01 coder evidence

Correction candidate for independent review, not an approval. Shared checkout
`dev/planar-v2-vernacular`, HEAD `3171c49e7b92d3abda01bfbb8bfb0c6b29d4c67a`.
The existing rewrite, original B1 candidate and independent review evidence are
preserved. No Planar operations, operator data, installed binaries, commit,
installation, publication or merge were used. Builds ran directly and serially
in the exclusively held debug directory, released on delivery.

The private format owner now records sticky reader validity. Every failed typed
read returns its original error and invalidates all subsequent reads. This
conservative rule includes invalid IDs and missing records; only explicit reopen
obtains another verified snapshot. Moves transfer validity and cached immutable
metadata stays accessible. RAII still releases retained resources; an ordinary
non-storage refusal need not release a read lock immediately. SQLite primitives,
schema, writer behavior, record codec, CLI/Lua and vendors remain unchanged.

`src/engine/ledger/snapshot.t.cpp` permanently adapts the independent reviewer's
real SQLite VFS regression, removes unused write/sync/barrier machinery, and
registers the cases in the existing ledger test target. The reader case requires
an actual `SQLITE_IOERR_READ`, the typed `io` refusal, and a successful independent
writer commit while the original reader remains alive. The original reader must
refuse new data. Added assertions cover valid and invalid owner moves, cached
metadata, old-record refusal and a fresh verified reader seeing new record 9.
The paired writer case still requires a real fault and prefix rollback. It checks
old record 1 before its missing-record refusal so the conservative reader policy
is respected. The ordinary failure case independently covers zero/overflow IDs
and not-found, move assignment, metadata and explicit reopen. Existing reader
lock/commit rollback and all foundation tests remain active.

## Executed commands and raw results

- `cmake --build --preset debug --target all planar_tests`: before-fix
  `red-build.log`, initial fixed `green-build.log`, final name-registration
  `final-build.log`; all exit zero. CMake regenerated configured variants.
- `build/rewrite-debug/src/engine/planar_ledger_format_tests '[snapshot]'`
  before the product fix: `snapshot-red.log`, exact argv/exit in
  `red-command.json`, exit 42, two cases / one pass / one failure, 47 assertions
  / 46 pass / one failure. The sole failing assertion rejects new-record
  exposure after the real io refusal and independent commit. `red-source.t.cpp`
  reconstructs the exact unformatted source exercised at RED from the preserved
  reviewer probe and the preprocessing used before the build. `before/` freezes
  the ten original B1 files as they were before correction.
- Same snapshot command after correction: `snapshot-green.log`, exit zero,
  two cases / 59 assertions passed, with actual fault and independent commit
  messages retained. No deliberate product mutation was needed for this RED.
- `ctest --test-dir build/rewrite-debug -R 'replacement.(ledger|canonical|hash|record)' --output-on-failure --output-junit <repo>/tests/native/r13b1c01-evidence/focused.xml`:
  final `focused.log` / `focused.xml`, **79 passed, zero skips/failures**.
  An initial green selection matched only 77 cases because the two new case
  names lacked the `ledger` prefix used by this regex. Those results are
  preserved in `focused-initial77.*`; names were corrected and all 79 rerun.
- `python3 tests/vectors/generate.py --check`: `vectors.log`, 12 hand byte,
  20 upstream hashes, 41 canonical/domain vectors; exit zero.
- `python3 tests/vectors/records/generate.py --check`: `record-vectors.log`,
  10 independent vectors; exit zero.
- `python3 scripts/rewrite-check.py lint --build-dir build/rewrite-debug`:
  `lint.log` and final `lint-final.log`, pinned formatter/Doxygen checked 41
  active source files, exit zero.
- `python3 tests/native/r13b1c01-evidence/helper-lint.py`: `helper-lint.log`,
  same pinned formatter and Doxygen settings over both B1 helper C++ files,
  exit zero, zero warnings. Reproducible script retained here.
- `cmake --build --preset debug --target all planar_tests`: `no-work.log`,
  exit zero, exactly `ninja: no work to do.`
- `python3 tests/native/r13b1c01-evidence/audit.py`: `audits.log`, exit zero.
  Verifies all 1,834 original protected files, 50 prior B1 evidence entries,
  16 independent review evidence entries, exact fresh reversible regeneration
  of both C2 and B1 configured variants, both redirected digest call sites,
  normal non-mutated variant state, the 79-case JUnit and zero-work build.
  Correction-local generation manifests are retained; original manifests and
  prior audit scripts were not rewritten to fit changed source.

## Snapshot and manifest path bases

`candidate.json` freezes all ten B1 candidate files plus the new permanent
`snapshot.t.cpp`. Its `files` keys are relative to repository root; copies with
those same keys live under this directory's `candidate/`. Each value is file
SHA256. Candidate identity is SHA256 of the UTF-8 compact JSON files mapping,
with keys sorted, separators `,`/`:` and no newline.

`candidate.diff` compares the corrected full B1 candidate against the original
B1 `before/` snapshots (new files against empty). `correction.diff` compares
against this correction's `before/` snapshots. Those are distinct baselines.
All diff labels are repository-relative. `evidence.json` keys are relative to
this evidence directory and hash every retained artifact except itself. It
includes candidate snapshots and manifests. Neither manifest asserts approval.

Executed platform is macOS Apple Silicon debug. Full integrated R13 gates,
release/Linux execution, complete B2 trees and R14 acceptance remain outside
this corrective scope. No claim of those unexecuted checks is made.
