# V5-02-X08-C01 gate run 2 report (2026-10-29)

Overall result: GATE FAILED (passed=false), for one reason only: the Linux gate could not run because Docker is not usable on this
host. Every macOS criterion passed. No source file was edited by the gate runner. The earlier run's report is kept as
`REPORT-run1.md` (its inventory as `inventory-run1.txt`); run 2 logs are `raw/r2-*`, run 1 logs are untouched.

## Candidate identity (identity.py output, verbatim; `raw/r2-00-identity.txt`, identical after the run in `raw/r2-16-identity-after.txt`)

| identity | value |
|---|---|
| HEAD | `48b8e526764a76de1392d397bb21659bf63e8b8d` |
| sha256(git diff --binary HEAD -- src tests/native), tracked only | `a5dc14dfa3ba4ea58501b4fe26cf0192a368c41efc73ca961ff8e11d3d4a368a` |
| combined sha256 (diff bytes, then each untracked path NUL bytes, sorted), pathspec src tests/native, evidence excluded | `7bfbebce5c8d551e83927ae6c5c98f0bea820ac9c6b0a136bdd6ae3707dbd0a7` |

| file | state | sha256 |
|---|---|---|
| `src/cmd/planar/bootstrap.t.cpp` | modified | `0a4d0b4da795c8e3bf8b57c31b3446870ae5f1f951aa20608b57237bacddf6c9` |
| `src/engine/capture/host_path.cpp` | modified | `9207c2b79ecc9fce3064c865bb8e253931795a503244304efcc172a5eb8ca7d3` |
| `src/engine/capture/host_path.t.cpp` | modified | `ddafdd85acc47e0b0c400e9f19158709029fc4a988e06edf235c89c0137308e8` |
| `src/engine/runtime/external_registration.t.cpp` | modified | `796a5be7b15dda1465128ec42cfc10f7409f1262ec78153b93ef9d67a8b411c3` |
| `tests/native/v5-02-x08-support/generate.py` | untracked | `0211ae98cd8064385b628d8f89e744bbfb314de95f8489032efbc8bc364b7420` |
| `tests/native/v5-02-x08-support/validate.py` | untracked | `c474c5fb9e36ffc49d287ec2228582ba42ac6987dc2da50fa6e31ba8b1a7d267` |
| `tests/native/v5-02-x08-support/x08.cmake` | untracked | `5c6d671fac9f016be46ccd136c529e0d3be55b8c866aa234247deb868948c7ea` |

`external_registration.t.cpp` changed since run 1 (fixed-root fixture, `c02_root_bytes = 64`, `c02_outputs_gap = 4742`).

## Commands, exit codes, counts

| # | command | exit | result |
|---|---|---|---|
| r2-01 | `cmake --build build/rewrite-debug --target planar_tests` | 0 | built |
| r2-02 | `ctest --test-dir build/rewrite-debug --no-tests=error -R 'host path\|X06 D C02\|X07 N30\|replacement.cli\|X08\|x08'` | 0 | 56 matched, 100% passed, 0 failed, 0 skipped |
| r2-03 | `ctest --test-dir build/rewrite-debug --no-tests=error` | 0 | 1258 of 1258 passed, 0 failed, 0 skipped (593.6 s) |
| r2-04 | `cmake --build build/rewrite-release --target planar_tests` | 0 | built |
| r2-05 | `ctest --test-dir build/rewrite-release --no-tests=error` | 0 | 1258 of 1258 passed, 0 failed, 0 skipped (228.5 s). `x07.two_process_cas_one_winner` passed this time (flaky in run 1; still an open product finding per the round record) |
| r2-06/07 | `ctest -N` debug / release | 0 / 0 | Total Tests: 1258 in each; sorted names identical in both configs |
| r2-08 | `python3 scripts/rewrite-check.py lint --build-dir build/rewrite-debug` | 0 | clang-format and Doxygen checked 147 active source files |
| r2-09/10 | no-op `cmake --build build/rewrite-debug` twice, after successful build r2-01 | 0 / 0 | each `ninja: no work to do.` (zero steps) |
| r2-11/12 | no-op `cmake --build build/rewrite-release` twice, after successful build r2-04 | 0 / 0 | each `ninja: no work to do.` (zero steps) |
| r2-14 | `python3 scripts/redact-local-paths.py --repo-path <repo> --home $HOME check <files>` on the modified/untracked non-evidence files (list in `raw/r2-14-redact-files.txt`) | 0 | no output, no findings. The script has no no-argument form, so only the explicit-files run exists |
| r2-15 | `df -h /System/Volumes/Data` | - | 113 GiB available (above the 15 GiB halt) |
| r2-17a | `scripts/linux-gate.sh <scratch>/linux-out2` | 5 | `linux-gate: docker is unavailable or its daemon is not running` (`docker` not on PATH) |
| r2-17b | same, with `~/.docker/bin` on PATH | 5 | same message. `~/.docker/bin/docker` is a dangling symlink to `/Applications/Docker.app/Contents/Resources/bin/docker`; `/Applications/Docker.app` is an empty directory (modified 06:08 today, during this run), so Docker Desktop is not installed or is mid-update. I did not install or restart anything |

Linux evidence: NONE. No docker build, no Linux ctest, no `docker builder prune` (nothing was built), no manifest comparison. Whether the
four earlier Linux failures are fixed (host path file-then-`..`, the two budget tests at the new 4742 constant, and the host-path
platform test) is UNVERIFIED. The refreshed `LLVM_APT_VERSION` in `scripts/linux-gate.Dockerfile` is also untested. Re-run
`scripts/linux-gate.sh` once Docker is restored.

## Inventory

Compared with `inventory-run1.txt` (run 1 of this round, 1257 sorted names; its own baseline was `tests/native/v5-02-x08-evidence/inventory.txt`,
1255 names). Current registered: 1258 (`inventory.txt`, sorted names; Debug and Release identical).
- Removed or renamed versus run 1 (and versus the 1255 names): none.
- New in run 2: `replacement.X08 C01 the C02 budget fixture root has a fixed canonical byte length`.
- New in run 1 (versus 1255): `replacement.host path keeps missing and unresolvable directory spellings as input path errors`,
  `replacement.host path maps a regular file followed by parent traversal to the root directory error`.
- Total 1255 + 3 = 1258, matching the round record and the validator's derived expectation. `validate.py` was not rerun by the gate runner.

## (a) Acceptance criteria to tests

| criterion | tests (file:line) | status |
|---|---|---|
| C01-1 file followed by `..` gives io "Cannot resolve explicit root directory" on both platforms | `host path maps a regular file followed by parent traversal to the root directory error` (src/engine/capture/host_path.t.cpp:84); `host path sanitizes invalid file traversal and missing parents` (:73); CLI `replacement.cli` (tests/acceptance/bootstrap_paths.py:125) | macOS pass Debug and Release; Linux unverified |
| C01-1b missing, loop and `file/x/..` spellings stay "Cannot resolve explicit input path" | `host path keeps missing and unresolvable directory spellings as input path errors` (host_path.t.cpp:90) | macOS pass; Linux unverified |
| C01-2 the two budget tests pass on Linux | `X06 D C02 role preparation charges enumeration and registry reads to the factory budget` (src/engine/runtime/external_registration.t.cpp:4557, bound at :4571), `X07 N30 role capture charges the factory budget owner` (:5087, bound at :5098) | macOS pass both configs; Linux NOT RUN |
| C01-3 fixed-root pin | `X08 C01 the C02 budget fixture root has a fixed canonical byte length` (external_registration.t.cpp:4576; `c02_root_bytes` :4512, `c02_outputs_gap` :4555) | present, passes on macOS |
| Four Linux failures pass, no other Linux failure | the tests above | NOT MET (no Linux run) |
| No existing test name removed or renamed; all 1255 names preserved in both configs; no diagnostic text changed | inventory diff above; lint pass | met |
| Prober shows both budget tests still fail under named mutants | not a gate-runner step; no prober output exists under this evidence directory | NOT RUN here |
| Three red-first records exist | see (b) | NOT MET (one of three-plus recorded) |
| macOS Debug and Release full runs, zero failures and skips | r2-03, r2-05 | met |
| Inventory identical in both configs | r2-06/07 | met |
| Lint, two no-work builds per config, redaction | r2-08 to r2-14 | met |
| Linux gate with Debug and RelWithDebInfo ctest in container | r2-17a/b | NOT MET (Docker unavailable) |
| X08 E12 / E07 tests (`x08.*`) | focused run r2-02, 56 matched | pass on macOS |

## (b) Red-first

Present under `red/` (from the coders; I did not rerun them):
- `red/records-validator-pre.{cmd,exit,log}`: `python3 tests/native/v5-02-x08-support/validate.py --evidence tests/native/v5-02-x08-evidence --repo .`,
  exit 1, failing line "G01 FAIL missing or unreadable raw/prior-names.txt" through G09. Pre-fix snapshot `red/prefix-identity.txt`, combined
  hash `274e7a4f74a53fee38ee481d914825d9e892a849150a6c9f0d0d1b3ff9308354`. Real red (behavioural validator failure).
- `red/head-bytes-identity.txt`: defines HEAD bytes of `host_path.cpp` and `external_registration.t.cpp` as the pre-fix snapshot (not a run).

MISSING from the evidence, nothing invented: the host_path macOS red (new test against HEAD `host_path.cpp`), the host_path Linux red, the
budget Linux red (4468 < 4524, only described in the round record from the second gate's log), and any red for the fixed-root pin test. No
command, snapshot hash or failing line exists for them under `red/`. The round record says the repair coder's pin test "would fail on the
default 36-byte macOS root"; that is a claim, not an executed red.

## (c) Deliberate behaviour differences

Two macOS mapping changes (round decision 5, operator 2026-10-29), permitted because the closed external-profile message table is unchanged
(both texts already exist as x07-addendum G06/G13 rows):
1. `alias/file/..` as a directory argument: path_denied "Explicit root is a symbolic link target" (exit 3) becomes io "Cannot resolve explicit root directory" (exit 4).
2. `physical/file/../file` as a directory argument: io "Explicit root must be a directory" becomes io "Cannot resolve explicit root directory".
Both are pinned by the `...parent traversal...` test at host_path.t.cpp:84. No message text changed.

## Limitations

- Linux (arm64 Docker) did not run; x86_64 Linux is not covered by design.
- The new 4742 gap constant is measured on macOS only; its Linux value is unverified.
- `x07.two_process_cas_one_winner` flaked once in run 1 Release and passed in run 2; the round record lists it as an open product finding.
- Mutation prober output and gate-stage red runs are absent from this directory.
- `inventory.txt` lists sorted names, not ctest ordinals.
