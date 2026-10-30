# V5-02-X08-C01 gate run 4 report (2026-10-29)

Overall result: GATE FAILED (passed=false), solely because the final command `validate.py` exits 1. Every other command met its criterion,
including the Linux gate, which now passes in Debug and RelWithDebInfo (1258 of 1258 each). No source file was edited by the gate runner.
Earlier reports: `REPORT-run1.md`, `REPORT-run2.md`, `REPORT-run3.md`. Run 4 raw logs: `raw/r4-*` and `raw/linux4/`.

`validate.py --evidence tests/native/v5-02-x08-evidence --repo .` fails G01-G05 and G07-G09 (G06 passes) because it inspects the X08 evidence
directory, which lacks `raw/prior-names.txt`, `raw/macos-*.inventory.txt`, `raw/*.exit`, lint/redaction/no-op logs, and roadmap text for X08/RA01
(and the roadmap still contains the '#531 timing' text). Those are round-record outputs (PK-POST) not produced by the gate. Identical to run 3.

## Candidate identity (`raw/r4-00-identity.txt`, byte-identical afterwards in `raw/r4-20-identity-after.txt`)

HEAD `48b8e526764a76de1392d397bb21659bf63e8b8d`; sha256(git diff --binary HEAD -- src tests/native, tracked) `67384e8674ed073152d2719fce60d0b857528e1baee50e55b069722a640e549c`;
combined (diff plus untracked, src tests/native) `f5041b7fae2ac751e3dc934f297f90aad2428245eac27ba1de7909e8750df92d`. Per-file hashes in the identity file.
This differs from run 3 only in `src/engine/runtime/external_registration.t.cpp` (now `33121b6e...`; `c02_outputs_gap` changed 4742 to 4737,
the lower of the macOS 4742 and Linux 4737 measurements).

## Commands, exit codes, counts

| # | command | exit | result |
|---|---|---|---|
| r4-01 | `cmake --build build/rewrite-debug --target planar_tests` | 0 | built |
| r4-03 | `ctest --test-dir build/rewrite-debug --no-tests=error --output-on-failure` | 0 | 100% passed out of 1258, 0 failed, 0 skipped/not run (584.0 s) |
| r4-04 | `cmake --build build/rewrite-release --target planar_tests` | 0 | built |
| r4-05 | `ctest --test-dir build/rewrite-release --no-tests=error --output-on-failure` | 0 | 100% passed out of 1258, 0 failed, 0 skipped (231.9 s) |
| r4-06/07 | `ctest -N` debug / release | 0 / 0 | Total Tests 1258 each; normalized sorted names identical |
| r4-08 | `python3 scripts/rewrite-check.py lint --build-dir build/rewrite-debug` | 0 | clang-format and Doxygen pass |
| r4-09/10 | no-op debug build twice, after successful r4-01 | 0 / 0 | `ninja: no work to do.` both |
| r4-11/12 | no-op release build twice, after successful r4-04 | 0 / 0 | `ninja: no work to do.` both |
| r4-14 | `python3 scripts/redact-local-paths.py --repo-path <repo> --home $HOME check <20 files>` (`raw/r4-14-redact-files.txt`, the files from git status; no no-argument form exists) | 0 | no findings |
| r4-15 | `df -h /System/Volumes/Data` before Linux | - | 109 GiB available (above 15 GiB) |
| r4-17 | `scripts/linux-gate.sh <scratch>/linux-out4` (copied to `raw/linux4/`) | 0 | docker build exit 0 |
| r4-17 Debug (container) | configure, build, inventory, ctest, no-work all exit 0 | 0 | 100% passed out of 1258; no-work `ninja: no work to do.` |
| r4-17 RelWithDebInfo (container) | same | 0 | 100% passed out of 1258; no-work `ninja: no work to do.` |
| Linux inventory | | | 1258 each, identical to macOS names; host manifest before/after equal, host vs container context equal |
| r4-18 | `docker builder prune -f` | 0 | Total 0B (the gate script already pruned) |
| r4-19 | `python3 tests/native/v5-02-x08-support/validate.py --evidence tests/native/v5-02-x08-evidence --repo .` | 1 | G06 PASS; G01-G05, G07-G09 FAIL (`raw/r4-19-validate.log`) |

## Inventory

Compared with `../v5-02-x08-evidence/inventory.txt` (most recent committed-evidence list, 1255 names; ctest -N format normalized). Current: 1258
(`inventory.txt`, sorted names, not ordinals). Removed or renamed: none. New (3):
`replacement.host path keeps missing and unresolvable directory spellings as input path errors`,
`replacement.host path maps a regular file followed by parent traversal to the root directory error`,
`replacement.X08 C01 the C02 budget fixture root has a fixed canonical byte length`.
Debug, Release, Linux Debug and Linux RelWithDebInfo list the same 1258 names. Zero-work: all four no-change builds on macOS and both in-container no-work builds report no work.

## (a) Acceptance criteria to tests

| criterion | tests (file:line) | status |
|---|---|---|
| C01-1 file followed by `..` gives io "Cannot resolve explicit root directory" on both platforms | `host path maps a regular file followed by parent traversal to the root directory error` (src/engine/capture/host_path.t.cpp:84); `host path sanitizes invalid file traversal and missing parents` (:73); CLI `replacement.cli` (tests/acceptance/bootstrap_paths.py:125) | pass macOS Debug/Release and Linux Debug/RelWithDebInfo |
| C01-1b missing, loop and `file/x/..` spellings stay "Cannot resolve explicit input path" | `host path keeps missing and unresolvable directory spellings as input path errors` (host_path.t.cpp:90) | pass on all four |
| C01-2 the two budget tests pass on Linux | `X06 D C02 role preparation charges enumeration and registry reads to the factory budget` (src/engine/runtime/external_registration.t.cpp:4558, bound :4572), `X07 N30 role capture charges the factory budget owner` (:5088, bound :5099) | pass on all four (gap constant 4737) |
| C01-3 fixed-root pin | `X08 C01 the C02 budget fixture root has a fixed canonical byte length` (external_registration.t.cpp:4577; `c02_root_bytes` :4512, `c02_outputs_gap` :4556) | pass on all four |
| Four Linux failures pass, no other Linux failure | tests above | MET (1258/1258 both configs) |
| No existing test name removed or renamed; no diagnostic text changed | inventory diff; lint pass | met |
| Prober shows both budget tests still fail under named mutants | not a gate-runner step; no prober output under this evidence directory | NOT RUN here |
| Red-first records for each regression test | see (b) | NOT MET (one of four-plus recorded) |
| macOS Debug and Release full, zero failures and skips | r4-03, r4-05 | met |
| Inventory identical across configs | r4-06/07, linux4 | met |
| Lint, two no-work builds per config, redaction | r4-08 to r4-14 | met |
| Linux gate full Debug and RelWithDebInfo in container | r4-17 | met |
| X08 validator over final evidence | r4-19 | NOT MET (exit 1, see top) |

## (b) Red-first

Present under `red/` (from the coders; not rerun by the gate runner):
- `red/records-validator-pre.{cmd,exit,log}`: `python3 tests/native/v5-02-x08-support/validate.py --evidence tests/native/v5-02-x08-evidence --repo .`,
  exit 1, failing line "G01 FAIL missing or unreadable raw/prior-names.txt" through G09. Pre-fix snapshot `red/prefix-identity.txt`, combined hash
  `274e7a4f74a53fee38ee481d914825d9e892a849150a6c9f0d0d1b3ff9308354`. Real red (validator behaviour).
- `red/head-bytes-identity.txt`: defines HEAD bytes of `host_path.cpp` and `external_registration.t.cpp` as the pre-fix snapshot (not a run).

MISSING from the evidence, nothing invented: the host_path macOS red (new test against HEAD `host_path.cpp`), the host_path Linux red, the budget Linux
red (4468 < 4524, described only in the round record), the repair-1 Linux red (4737 < 4742, visible only as the run 3 Linux failure in `raw/linux3/`; that is
an observed failure of the previous candidate, not a red-first run with a snapshot hash), and any red for the fixed-root pin test.

## (c) Deliberate behaviour differences

Two macOS mapping changes (round decision 5, operator 2026-10-29), permitted because the closed message table is unchanged (both texts exist as x07-addendum G06/G13 rows):
1. `alias/file/..` as a directory argument: path_denied "Explicit root is a symbolic link target" (exit 3) becomes io "Cannot resolve explicit root directory" (exit 4).
2. `physical/file/../file` as a directory argument: io "Explicit root must be a directory" becomes io "Cannot resolve explicit root directory".
Pinned by the `...parent traversal...` test at host_path.t.cpp:84. No message text changed.

## Limitations

- `c02_outputs_gap` is 4737 (Linux measurement); on macOS the observed gap is 4742, so the bound is 5 bytes loose there. The comment at external_registration.t.cpp:4549-4555 records this.
- x86_64 Linux not covered (arm64 Docker only).
- `x07.two_process_cas_one_winner` flaked once in run 1 (known X07 race, recorded in the round record); passed in runs 2 to 4.
- Mutation prober output and gate-stage red runs are absent from this directory.
- `validate.py` was run as instructed against the X08 evidence directory and fails there.
- `inventory.txt` lists sorted names, not ctest ordinals.
