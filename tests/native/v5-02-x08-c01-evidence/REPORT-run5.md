# V5-02-X08-C01 gate run 5 report (2026-10-29)

Overall result: GATE PASSED (passed=true). Every command met its criterion, including the Linux gate (Debug and RelWithDebInfo, 1258 of 1258
each) and the post-gate `validate.py`, which now exits 0 (G01 to G09 all PASS). No source file was edited by the gate runner.
Earlier reports are kept as `REPORT-run1.md` to `REPORT-run4.md`. Run 5 raw logs: `raw/r5-*` and `raw/linux5/`.

## Candidate identity (`raw/r5-00-identity.txt`, byte-identical afterwards in `raw/r5-20-identity-after.txt`)

HEAD `48b8e526764a76de1392d397bb21659bf63e8b8d`; sha256(git diff --binary HEAD -- src tests/native, tracked) `67384e8674ed073152d2719fce60d0b857528e1baee50e55b069722a640e549c`;
combined (diff plus untracked, src tests/native, evidence excluded) `5cdb43323d7f9b5a865c68e0bef71715f9ee5512126f1cc8f892a284d7ef9ff0`.
The tracked diff hash is identical to run 4; the combined hash differs only because untracked support files changed. Per-file hashes are in the identity file
(`external_registration.t.cpp` `33121b6e...`, `host_path.cpp` `9207c2b7...`, `host_path.t.cpp` `ddafdd85...`, `bootstrap.t.cpp` `0a4d0b4d...`).

## Commands, exit codes, counts

| # | command | exit | result |
|---|---|---|---|
| r5-01 | `cmake --build build/rewrite-debug --target planar_tests` | 0 | built |
| r5-03 | `ctest --test-dir build/rewrite-debug --no-tests=error --output-on-failure` | 0 | 100% passed out of 1258, 0 failed, 0 skipped |
| r5-04 | `cmake --build build/rewrite-release --target planar_tests` | 0 | built |
| r5-05 | `ctest --test-dir build/rewrite-release --no-tests=error --output-on-failure` | 0 | 100% passed out of 1258, 0 failed, 0 skipped |
| r5-06/07 | `ctest -N` debug / release | 0 / 0 | Total Tests 1258 each; sorted names identical |
| r5-08 | `python3 scripts/rewrite-check.py lint --build-dir build/rewrite-debug` | 0 | "Formatting and Doxygen checked 147 active source files" |
| r5-09/10 | no-op debug build twice, after successful r5-01 | 0 / 0 | `ninja: no work to do.` both |
| r5-11/12 | no-op release build twice, after successful r5-04 | 0 / 0 | `ninja: no work to do.` both |
| r5-14 | `python3 scripts/redact-local-paths.py --repo-path <repo> --home $HOME check <21 files>` (`raw/r5-14-redact-files.txt`: the run 4 list plus `docs/vernacular/roadmap.md`, from git status; the script has no no-argument form) | 0 | no findings, empty log |
| r5-15 | `df -h /System/Volumes/Data` before Linux | - | 96 GiB available (above 15 GiB) |
| r5-17 | `scripts/linux-gate.sh <scratch>/linux-out5` (copied to `raw/linux5/`) | 0 | docker build exit 0 |
| r5-17 Debug (container) | configure, build, inventory, ctest, no-work | all 0 | 100% passed out of 1258; `ninja: no work to do.` |
| r5-17 RelWithDebInfo (container) | same | all 0 | 100% passed out of 1258; `ninja: no work to do.` |
| Linux manifests | host manifest before vs after; host vs container context | - | both equal (`raw/linux5/manifest-compare.txt`) |
| r5-18 | `docker builder prune -f` (plus the gate script's own prune, `raw/linux5/builder-prune.exit` 0) | 0 | done |
| r5-19 | `python3 tests/native/v5-02-x08-support/validate.py --evidence tests/native/v5-02-x08-evidence --repo .` (sync-records step, after logs copied) | 0 | G01 to G09 PASS (`raw/r5-19-validate.log`) |

## Inventory

`inventory.txt` lists the 1258 registered test names, sorted (ctest ordinals stripped). Compared with `../v5-02-x08-evidence/inventory.txt`
(most recent committed inventory, 1255 names): removed or renamed none; new three:
`replacement.host path keeps missing and unresolvable directory spellings as input path errors`,
`replacement.host path maps a regular file followed by parent traversal to the root directory error`,
`replacement.X08 C01 the C02 budget fixture root has a fixed canonical byte length`.
Debug, Release, Linux Debug and Linux RelWithDebInfo lists are identical, and identical to the run 4 `inventory.txt`.
Zero-work: all four macOS no-change builds and both in-container no-work builds report no work.

## (a) Acceptance criteria to tests

| criterion | tests (file:line) | status |
|---|---|---|
| C01-1 file followed by `..` gives io "Cannot resolve explicit root directory" on both platforms | `host path maps a regular file followed by parent traversal to the root directory error` (src/engine/capture/host_path.t.cpp:84); `host path sanitizes invalid file traversal and missing parents` (:73); CLI `replacement.cli` (tests/acceptance/bootstrap_paths.py:123) | pass macOS Debug/Release, Linux Debug/RelWithDebInfo |
| C01-1b missing, loop and `file/x/..` spellings stay "Cannot resolve explicit input path" | `host path keeps missing and unresolvable directory spellings as input path errors` (host_path.t.cpp:90) | pass on all four |
| C01-2 the two budget tests pass on Linux | `X06 D C02 role preparation charges enumeration and registry reads to the factory budget` (src/engine/runtime/external_registration.t.cpp:4558, bound check :4572), `X07 N30 role capture charges the factory budget owner` (:5088, bound check :5099) | pass on all four (gap constant 4737, :4556) |
| C01-3 fixed-root pin | `X08 C01 the C02 budget fixture root has a fixed canonical byte length` (external_registration.t.cpp:4577; `c02_root_bytes` :4512) | pass on all four |
| The four earlier Linux failures pass, no other Linux failure | tests above | met (1258/1258 both configs) |
| No existing test name removed or renamed; no diagnostic text changed | inventory diff; lint pass | met |
| Prober shows both budget tests still fail under named mutants | not a gate-runner step; no prober output in this directory | NOT RUN here |
| Red-first record for each regression test | see (b) | met for macOS; Linux reds limited (see (b)) |
| macOS Debug and Release full, zero failures and skips | r5-03, r5-05 | met |
| Inventory identical across configs | r5-06/07, linux5 | met |
| Lint, two no-work builds per config, redaction | r5-08 to r5-14 | met |
| Linux gate full Debug and RelWithDebInfo in container | r5-17 | met |
| X08 validator over final evidence | r5-19 | met (exit 0) |

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

- `c02_outputs_gap` is 4737 (Linux measurement); on macOS the observed gap is 4742, so the bound is 5 bytes loose there (comment near external_registration.t.cpp:4549).
- x86_64 Linux not covered (arm64 Docker only).
- `x07.two_process_cas_one_winner` flaked once in run 1 (known X07 race, in the round record); passed in runs 2 to 5.
- Mutation prober output is absent from this directory; the Linux reds listed in (b) are missing.
- `inventory.txt` lists sorted names, not ctest ordinals.
