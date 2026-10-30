# V5-02-X08-C01 gate run report (2026-10-29)

Overall result: GATE FAILED (passed=false). Three independent failures: (1) RelWithDebInfo ctest failed one test, (2) the Linux gate did
not build (pinned LLVM apt version no longer available), (3) the registered inventory is 1257, not the 1258 the round record and
decision 14 require, because the budget-fixture work (`X08 C01 the C02 budget fixture root has a fixed canonical byte length`,
`c02_root_bytes`) is not present in the checkout. No source file was edited by the gate runner.

## Candidate identity (identity.py output, verbatim)

| identity | value |
|---|---|
| HEAD | `48b8e526764a76de1392d397bb21659bf63e8b8d` |
| sha256(git diff --binary HEAD -- src tests/native), tracked only | `8cd2d553f5a47f2a7db17410e0775f9d288980ac8644fd3750d2a7dab310852f` |
| combined sha256 (diff bytes, then each untracked path NUL bytes, sorted), pathspec src tests/native, evidence excluded | `a5f55a8baf416956e39db7e52e0a01b73c9b6a22ae13d4793f77dc7e7a7fbef1` |

Per-file hashes: `raw/20-identity-final.txt` (same values as `raw/16-identity.txt`, taken before the Linux run; unchanged after).
Modified in src/tests: `src/cmd/planar/bootstrap.t.cpp`, `src/engine/capture/host_path.cpp`, `src/engine/capture/host_path.t.cpp`;
untracked `tests/native/v5-02-x08-support/{generate.py,validate.py,x08.cmake}`.
`src/engine/runtime/external_registration.t.cpp` is UNMODIFIED (still `c02_outputs_gap = 4524` at line 4537, no `c02_root_bytes`).
Raw logs in `raw/` are named `NN-name.{cmd,log,exit}`.

## Commands, exit codes, counts

| # | command | exit | result |
|---|---|---|---|
| 01 | `cmake --build build/rewrite-debug --target planar_tests` | 0 | built |
| 02 | `ctest --test-dir build/rewrite-debug --no-tests=error -R 'host path\|X06 D C02\|X07 N30\|replacement.cli\|X08\|x08'` | 0 | 55 matched, 100% passed (27 acceptance, 28 unit), 0 skipped |
| 03 | `ctest --test-dir build/rewrite-debug --no-tests=error` | 0 | 1257 of 1257 passed, 0 failed, 0 skipped (52 acceptance, 1205 unit) |
| 04 | `cmake --build build/rewrite-release --target planar_tests` | 0 | built |
| 05 | `ctest --test-dir build/rewrite-release --no-tests=error` | 8 | 1256 of 1257 passed, 1 FAILED: `1249 - x07.two_process_cas_one_winner (Failed)` (acceptance, 0.18 s); 0 skipped |
| 13 | rerun of `x07.two_process_cas_one_winner` alone, release, `--output-on-failure` | 0 | passed (0.55 s); Debug alone also passed. The first failure's diagnostic was not captured (the log has no per-test output and LastTest.log was overwritten by the rerun), so the cause is UNKNOWN. It looks like a flaky two-process race but this is a hypothesis, not evidence. It does not turn the gate green. |
| 06/07 | `ctest -N` debug / release | 0 / 0 | Total Tests 1257 in each; sorted names identical in both configs (`raw/18-inventory-release-sorted.txt` vs `inventory.txt`) |
| 08 | `python3 scripts/rewrite-check.py lint --build-dir build/rewrite-debug` | 0 | clang-format and Doxygen checked 147 active source files |
| 09/10 | no-op `cmake --build` debug twice (after successful build 01) | 0/0 | each: `ninja: no work to do.` (zero steps) |
| 11/12 | no-op `cmake --build` release twice (after successful build 04) | 0/0 | each: `ninja: no work to do.` (zero steps) |
| 14 | `scripts/redact-local-paths.py --repo-path <repo> --home $HOME check <files>` on the 18 modified/untracked non-evidence files (`raw/14-redact-files.txt`) | 0 | no output, no findings. The script has no no-argument form: it requires `--repo-path`, `--home` and a mode, so only the explicit-files run exists |
| 15 | `df -h /System/Volumes/Data` before docker | - | 122 GiB available (above the 15 GiB halt) |
| 17 | `scripts/linux-gate.sh <scratch>/linux-gate-out` | 1 | `docker build` failed at `[gate 4/12] apt-get install`: `Version '1:23.1.5~++20261023090852+395ba2c4d274-1~exp1~20261023090919.90' for 'clang-23' ... was not found` (also libc++-23-dev, libc++abi-23-dev, lld-23, llvm-23, libclang-common-23-dev), apt exit 100. No Linux ctest ran. Host manifest before/after equal; container manifest missing. `docker builder prune -f` exit 0; df after 252491336 512-blocks (about 120 GiB) free. Copies under `raw/linux/` |

Linux evidence: NONE. The pinned LLVM apt snapshot version (`LLVM_APT_VERSION` in `scripts/linux-gate.Dockerfile`) is no longer served by
apt.llvm.org, which is an environmental or pin problem and is for the owner of `scripts/linux-gate.Dockerfile` to fix. Whether the four
Linux failures are fixed is therefore UNVERIFIED.

## Inventory

Compared with the most recent committed-or-recorded inventory I found, `tests/native/v5-02-x08-evidence/inventory.txt` (the X08 gate
list, 1255 test lines plus header lines; untracked, produced by the earlier X08 gate; the prior-1227 names are not separately listed there).
- Current registered: 1257 (`inventory.txt`, sorted names, both configs identical).
- Removed or renamed vs 1255: none.
- New: `replacement.host path keeps missing and unresolvable directory spellings as input path errors`,
  `replacement.host path maps a regular file followed by parent traversal to the root directory error`.
- Missing vs the round record's 1258: `replacement.X08 C01 the C02 budget fixture root has a fixed canonical byte length`. The validator
  (`validate.py`, derives 1258) would fail G01/G02 on this count.

## (a) Acceptance criteria to tests

Round acceptance (round record lines 167-170) and the X08 criteria (lines 27-47):

| criterion | test names (file:line) | status |
|---|---|---|
| C01-1 file followed by `..` gives io "Cannot resolve explicit root directory" on both platforms | `host path maps a regular file followed by parent traversal to the root directory error` (src/engine/capture/host_path.t.cpp:84); existing `host path sanitizes invalid file traversal and missing parents` (:73); public CLI `replacement.cli` (tests/acceptance/bootstrap_paths.py:125) | macOS pass both configs; Linux unverified |
| C01-1b missing/loop/`file/x/..` stay "Cannot resolve explicit input path" | `host path keeps missing and unresolvable directory spellings as input path errors` (host_path.t.cpp:90) | macOS pass; Linux unverified |
| C01-2 four Linux failures pass | `X06 D C02 role preparation charges enumeration and registry reads to the factory budget` (src/engine/runtime/external_registration.t.cpp:4539), `X07 N30 role capture charges the factory budget owner` (:5059), host path tests above | NOT MET: fixture unchanged (4524), no Linux run |
| C01-3 pinned-root fixture test | `X08 C01 the C02 budget fixture root has a fixed canonical byte length` | ABSENT, not in tree |
| No existing test name removed or renamed (1255 preserved), no diagnostic text changed | inventory diff above (none removed); lint pass | met on macOS |
| Prober shows budget tests still fail under mutants | not part of this gate runner; no prober output exists under `c01-evidence/` | NOT RUN |
| Three red-first records exist | see (b); only the records-validator record exists | NOT MET |
| X08 1 (E01-E14 map) / 2 (adversarial rows) / 9 (requirements.json, README, REPORT) | `tests/native/v5-02-x08-evidence/requirements.json` exists (provisional); validator not rerun by the gate runner | not verified here |
| X08 5 macOS Debug/Release full runs, no-work builds, lint, redaction | commands 03, 05, 08-14 above | Debug met; Release FAILED (one test); the rest met |
| X08 6 Linux gate | command 17 | FAILED to build |
| X08 3 (E12), 4 (E07) | `x08.*` tests in the focused run (55 matched, all passed in Debug) | Debug pass |

## (b) Red-first

Present in `red/` (not rerun by me):
- `red/records-validator-pre.cmd`: `python3 tests/native/v5-02-x08-support/validate.py --evidence tests/native/v5-02-x08-evidence --repo .`,
  exit 1 (`records-validator-pre.exit`), failing lines G01-G09 (e.g. "G01 FAIL missing or unreadable raw/prior-names.txt"). Snapshot
  identity: `red/prefix-identity.txt`, combined hash `274e7a4f74a53fee38ee481d914825d9e892a849150a6c9f0d0d1b3ff9308354` (real red,
  behavioural validator failure; the validator tree has since changed hash to `c474c5fb...`).
- `red/head-bytes-identity.txt`: HEAD bytes of host_path.cpp and external_registration.t.cpp (empty diff, sha256 of empty input
  `e3b0c442...b855`); it defines the pre-fix snapshot only.

MISSING from the evidence (nothing invented): the host_path macOS red run (new test against HEAD host_path.cpp), the host_path Linux red,
and the budget Linux red (4468 < 4524) are not under `c01-evidence/red/`. No command, hash or failing line exists for them.
`tests/native/v5-02-x08-evidence/raw/superseded-2/` holds the second Linux gate log (4 failures) the plan names as the fallback, but I did
not verify it.

## (c) Deliberate behaviour differences

Two macOS mapping changes (decisions 5, operator 2026-10-29), permitted because the closed external-profile message table is unchanged
(both texts already exist as x07-addendum G10/G13 rows) and the operator accepted platform parity:
1. `alias/file/..` as a directory argument: path_denied "Explicit root is a symbolic link target" (exit 3) to io "Cannot resolve explicit root directory" (exit 4). Pinned by `...parent traversal...` at host_path.t.cpp:84.
2. `physical/file/../file` as a directory argument: io "Explicit root must be a directory" to io "Cannot resolve explicit root directory" (same test).
No message text changed.

## Limitations

- Linux (arm64 Docker) did not run; x86_64 Linux not covered by design.
- The Release `x07.two_process_cas_one_winner` failure is unexplained (diagnostic not captured; passes on rerun and in Debug).
- Budget-fixture fix, prober table and gate-stage red runs are absent from the checkout and evidence.
- I did not rerun validate.py; `inventory.txt` lists sorted names, not ctest ordinals.
