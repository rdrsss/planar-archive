# V5-02-X08-C01 gate run 6 report (2026-10-29)

Overall result: GATE FAILED (passed=false). One test fails in all four configurations (macOS Debug, macOS Release, Linux Debug, Linux RelWithDebInfo):
`replacement.X08 platform branches are exactly the audited set` (ctest 1216, `src/cmd/planar/bootstrap.t.cpp:3119`, failing check `:3123`
`counts == x08::audited_platform_counts()`; observed six entries versus the five audited). Every other command met its criterion. No source file was edited by the gate runner.
Earlier reports are kept as `REPORT-run1.md` to `REPORT-run5.md`; run 5 passed on the previous candidate. Run 6 raw logs: `raw/r6-*` and `raw/linux6/`.

Likely cause (diagnosis only, not verified by a fix): the uncommitted change to `src/engine/runtime/external_registration.t.cpp` since run 5 added a new
`#if defined(__APPLE__) ... #else ... #endif` block around `c02_outputs_gap` (about line 4556). The audit test counts platform branches per file and
`audited_platform_counts()` (bootstrap.t.cpp:3082) has no entry for `engine/runtime/external_registration.t.cpp`. The fix belongs to a coder: either audit the branch (new row, with the
X08 matrix and addendum) or remove the branch. The gate runner did not touch it.

## Candidate identity (`raw/r6-00-identity.txt`, byte-identical afterwards in `raw/r6-20-identity-after.txt`)

HEAD `48b8e526764a76de1392d397bb21659bf63e8b8d`; sha256(git diff --binary HEAD -- src tests/native, tracked) `487876bcd50eb1b05ca3d3494ff4306632504cf374e16c607384ffe7725bea7d`;
combined (diff plus untracked, src tests/native, evidence excluded) `c2ef83544eb77c23ef8045029a4043d0dbec3c561f09276c433572a867290ddf`.
Changed against run 5: `external_registration.t.cpp` `03d0564f...` (was `33121b6e...`) and `tests/native/v5-02-x08-support/validate.py` `02f55552...` (was `967a41e0...`).
`host_path.cpp` `9207c2b7...`, `host_path.t.cpp` `ddafdd85...` and `bootstrap.t.cpp` `0a4d0b4d...` are unchanged.

## Commands, exit codes, counts

| # | command | exit | result |
|---|---|---|---|
| r6-01 | `cmake --build build/rewrite-debug --target planar_tests` | 0 | built |
| r6-03 | `ctest --test-dir build/rewrite-debug --no-tests=error --output-on-failure` | 8 | 99%, 1 failed out of 1258, 0 skipped; failure is ctest 1216 above |
| r6-04 | `cmake --build build/rewrite-release --target planar_tests` | 0 | built |
| r6-05 | `ctest --test-dir build/rewrite-release --no-tests=error --output-on-failure` | 8 | 99%, 1 failed out of 1258, 0 skipped; same single failure |
| r6-06/07 | `ctest -N` debug / release | 0 / 0 | Total Tests 1258 each; sorted names identical |
| r6-08 | `python3 scripts/rewrite-check.py lint --build-dir build/rewrite-debug` | 0 | pass |
| r6-09/10 | no-op debug build twice, after successful r6-01 | 0 / 0 | no work (`raw/r6-09*.log`, `r6-10*.log`) |
| r6-11/12 | no-op release build twice, after successful r6-04 | 0 / 0 | no work |
| r6-14 | `python3 scripts/redact-local-paths.py --repo-path <repo> --home <home> check <21 files>` (`raw/r6-14-redact-files.txt`, same list as run 5) | 0 | no findings |
| r6-15 | `df -h /System/Volumes/Data` before Linux | - | 99 GiB available (above 15 GiB) |
| r6-17 | `scripts/linux-gate.sh <scratch>/linux-out6` (copied to `raw/linux6/`) | 0 | docker build exit 0; the script itself exits 0 even though ctest failed (it records exits and continues) |
| r6-17 Debug (container) | `container/debug-ctest.exit` | 8 | 99%, 1 failed out of 1258, same test; `ninja: no work to do.` |
| r6-17 RelWithDebInfo (container) | `container/release-ctest.exit` | 8 | 99%, 1 failed out of 1258, same test; `ninja: no work to do.` |
| r6-18 | `docker builder prune -f` | 0 | done |
| r6-19 | `python3 tests/native/v5-02-x08-support/validate.py --evidence tests/native/v5-02-x08-evidence --repo .` | 0 | G01 to G09 PASS (`raw/r6-19-validate.log`). Note G01 and G04 report "0 failed" for the committed X08 evidence, not for this run |

So the budget fixture work itself is fine: the two C02/N30 budget tests and the new fixed-root test passed on all four configurations (only test 1216 failed).

## Inventory

`inventory.txt` lists the 1258 registered test names sorted with `LC_ALL=C` (ordinals stripped); the run 5 list is kept as `inventory-run5.txt`. Compared with
`../v5-02-x08-evidence/inventory.txt` (most recent committed inventory, 1255 names) and with run 5: no name removed or renamed; the same three new names
(`replacement.host path keeps missing and unresolvable directory spellings as input path errors`,
`replacement.host path maps a regular file followed by parent traversal to the root directory error`,
`replacement.X08 C01 the C02 budget fixture root has a fixed canonical byte length`). Debug, Release, Linux Debug and Linux RelWithDebInfo lists are identical (the linux6 sorted lists are byte-equal to `inventory.txt`).
Zero-work: all four macOS no-change builds and both in-container no-work builds report no work.

## (a) Acceptance criteria to tests (test file:line unchanged from run 5 except the external_registration.t.cpp lines, which moved with the new edit and are not re-derived here)

| criterion | tests (file:line) | status |
|---|---|---|
| C01-1 file followed by `..` gives io "Cannot resolve explicit root directory" on both platforms | `host path maps a regular file followed by parent traversal to the root directory error` (src/engine/capture/host_path.t.cpp:84); `host path sanitizes invalid file traversal and missing parents` (:73); CLI `replacement.cli` (tests/acceptance/bootstrap_paths.py:123) | pass (all four) |
| C01-1b missing, loop and `file/x/..` spellings stay "Cannot resolve explicit input path" | `host path keeps missing and unresolvable directory spellings as input path errors` (host_path.t.cpp:90) | pass on all four |
| C01-2 the two budget tests pass on Linux | `X06 D C02 role preparation charges enumeration and registry reads to the factory budget` (src/engine/runtime/external_registration.t.cpp:4558, bound check :4572), `X07 N30 role capture charges the factory budget owner` (:5088, bound check :5099) | pass on all four (gap constant 4737, :4556) |
| C01-3 fixed-root pin | `X08 C01 the C02 budget fixture root has a fixed canonical byte length` (external_registration.t.cpp:4577; `c02_root_bytes` :4512) | pass on all four |
| The four earlier Linux failures pass, no other Linux failure | tests above | NOT MET as a whole: the Linux budget tests pass (1257 of 1258 each config) but test 1216 fails on Linux too |
| No existing test name removed or renamed; no diagnostic text changed | inventory diff; lint pass | met |
| Prober shows both budget tests still fail under named mutants | not a gate-runner step; no prober output in this directory | NOT RUN here |
| Red-first record for each regression test | see (b) | met for macOS; Linux reds limited (see (b)) |
| macOS Debug and Release full, zero failures and skips | r6-03, r6-05 | NOT MET: 1 failure each (test 1216), 0 skips |
| Inventory identical across configs | r6-06/07, linux6 | met |
| Lint, two no-work builds per config, redaction | r6-08 to r6-14 | met |
| Linux gate full Debug and RelWithDebInfo in container | r6-17 | NOT MET: ctest exit 8 in both, 1 failure each (test 1216) |
| X08 validator over final evidence | r6-19 | met (exit 0) |

## (b) Red-first

Records under `red/` (made by the repair coder in a scratch copy outside the checkout; see `red/README.md`, `red/scratch-snapshots.txt`; not rerun by the gate runner).
Pre-fix snapshot: HEAD `host_path.cpp` sha256 `0df99615b5aba1266790891509c675830254bfd63d788132475a2cbe63296ada`; scratch `external_registration.t.cpp` `83112769e13d38f9faaf2ae7c8912ffdfb01cebc28ea998e073f6ec4411d0020`.

| test | command (from `red/*.cmd`) | failing line | real red or guard |
|---|---|---|---|
| host path maps a regular file followed by parent traversal... | `build/rewrite-debug/src/engine/planar_capture_host_path_tests "host path maps a regular file followed by parent traversal to the root directory error"` (HEAD host_path.cpp) | exit 42; host_path.t.cpp:29 `"path_denied" == "io"`, :30 message "Explicit root is a symbolic link target" | real red (macOS) |
| host path keeps missing and unresolvable... | `... planar_capture_host_path_tests "host path keeps missing and unresolvable directory spellings as input path errors"` | exit 0 | guard, green at HEAD by design |
| X08 C01 fixed canonical byte length | `build/rewrite-debug/src/engine/planar_runtime_tests "X08 C01 the C02 budget fixture root has a fixed canonical byte length"` (fixture padding disabled) | exit 42; external_registration.t.cpp:4580 `36 == 64` | real red against a mutated fixture (scratch only) |
| X06 D C02 ... enumeration and registry reads | `... planar_runtime_tests "X06 D C02 role preparation charges enumeration and registry reads to the factory budget"` (unpadded root) | exit 42; :4572 `4525 >= 4737` | real red (macOS, unpadded fixture) |
| X07 N30 role capture charges the factory budget owner | `... planar_runtime_tests "X07 N30 role capture charges the factory budget owner"` (unpadded root) | exit 42; :5099 `4525 >= 4737` | real red (macOS, unpadded fixture) |
| records validator | `python3 tests/native/v5-02-x08-support/validate.py --evidence tests/native/v5-02-x08-evidence --repo .` (`red/records-validator-pre.*`, `red/prefix-identity.txt`) | exit 1, "G01 FAIL missing or unreadable raw/prior-names.txt" through G09 | real red (validator behaviour) |

MISSING, nothing invented: the host_path Linux red, and the original Linux budget red (4468 < 4524, 28-byte root); no Linux log of those survives.
The repair-1 Linux failure (`4737 >= 4742`) is an observed failure of the earlier candidate in `raw/linux3/container/debug-ctest.log`, not a snapshot-hashed red.

## (c) Deliberate behaviour differences

Two macOS mapping changes (round decision 5, operator 2026-10-29), permitted because the closed message table is unchanged (both texts exist as x07-addendum G06/G13 rows):
1. `alias/file/..` as a directory argument: path_denied "Explicit root is a symbolic link target" (exit 3) becomes io "Cannot resolve explicit root directory" (exit 4).
2. `physical/file/../file` as a directory argument: io "Explicit root must be a directory" becomes io "Cannot resolve explicit root directory".
Pinned by the `...parent traversal...` test at host_path.t.cpp:84. No message text changed.

## Limitations

- The red-first section (b) and table (a) line numbers for `external_registration.t.cpp` were carried from run 5; that file changed afterwards (hash above) and the lines were not re-verified.

- `c02_outputs_gap` is 4737 (Linux measurement); on macOS the observed gap is 4742, so the bound is 5 bytes loose there (comment near external_registration.t.cpp:4549).
- x86_64 Linux not covered (arm64 Docker only).
- `x07.two_process_cas_one_winner` flaked once in run 1 (known X07 race, in the round record); passed in runs 2 to 5.
- Mutation prober output is absent from this directory; the Linux reds listed in (b) are missing.
- `inventory.txt` lists sorted names, not ctest ordinals.
