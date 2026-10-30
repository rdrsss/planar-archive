# V5-02-X08-C01 gate run 3 report (2026-10-29)

Overall result: GATE FAILED (passed=false). All macOS criteria passed. The Linux gate ran this time (Docker restored) and the two budget
tests still fail on Linux in both configurations: `CHECK(off - on >= c02_outputs_gap)` gives `4737 >= 4742` (Linux gap is 5 bytes
smaller than the macOS-measured constant 4742; boundaries 228612 vs 223875). The task's final `validate.py` command also exits 1
(G01-G05, G07-G09 fail: it expects X08 round evidence such as raw/prior-names.txt and roadmap text that do not exist in
`tests/native/v5-02-x08-evidence`; G06 passes). No source file was edited. Earlier reports are kept as `REPORT-run1.md`, `REPORT-run2.md`;
run 3 logs are `raw/r3-*` and `raw/linux3/`.

## Candidate identity (`raw/r3-00-identity.txt`, byte-identical after the run in `raw/r3-20-identity-after.txt`)

HEAD `48b8e526764a76de1392d397bb21659bf63e8b8d`; sha256(git diff --binary HEAD -- src tests/native, tracked) `a5dc14dfa3ba4ea58501b4fe26cf0192a368c41efc73ca961ff8e11d3d4a368a`;
combined (diff plus untracked, src tests/native) `7bfbebce5c8d551e83927ae6c5c98f0bea820ac9c6b0a136bdd6ae3707dbd0a7`. This is the same candidate as run 2
(per-file hashes in `raw/r3-00-identity.txt`).

## Commands, exit codes, counts

| # | command | exit | result |
|---|---|---|---|
| r3-01 | `cmake --build build/rewrite-debug --target planar_tests` | 0 | built |
| r3-03 | `ctest --test-dir build/rewrite-debug --no-tests=error --output-on-failure` | 0 | 100% passed out of 1258, 0 failed, 0 skipped (588.9 s) |
| r3-04 | `cmake --build build/rewrite-release --target planar_tests` | 0 | built |
| r3-05 | `ctest --test-dir build/rewrite-release --no-tests=error --output-on-failure` | 0 | 100% passed out of 1258, 0 failed, 0 skipped (218.5 s) |
| r3-06/07 | `ctest -N` debug / release | 0 / 0 | 1258 each; sorted names identical |
| r3-08 | `python3 scripts/rewrite-check.py lint --build-dir build/rewrite-debug` | 0 | formatting and Doxygen checked 147 files |
| r3-09/10 | no-op debug build twice, after successful r3-01 | 0 / 0 | `ninja: no work to do.` both |
| r3-11/12 | no-op release build twice, after successful r3-04 | 0 / 0 | `ninja: no work to do.` both |
| r3-14 | `python3 scripts/redact-local-paths.py --repo-path <repo> --home $HOME check <20 files>` (`raw/r3-14-redact-files.txt`; no no-argument form exists) | 0 | no findings |
| r3-15 | `df -h /System/Volumes/Data` | - | 98 GiB available (above 15 GiB) |
| r3-17 | `scripts/linux-gate.sh <scratch>/linux-out3` (copied to `raw/linux3/`) | 0 | script exit 0, but container results below: ctest exit 8 in both configs |
| r3-17 Debug (container) | configure/build/inventory/no-work all exit 0; ctest exit 8 | 8 | 99%, 2 of 1258 failed (504 s): #618 `X06 D C02 role preparation charges enumeration and registry reads to the factory budget`, #641 `X07 N30 role capture charges the factory budget owner` |
| r3-17 RelWithDebInfo (container) | same | 8 | 99%, 2 of 1258 failed (234 s): the same two |
| Linux inventory | | | 1258 in each config, identical to each other; container no-work builds: `ninja: no work to do.` both; host manifest before/after and vs container context: equal |
| r3-18 | `docker builder prune -f` | 0 | reclaimed 0B (the gate script also pruned; 16.12GB freed there) |
| r3-19 | `python3 tests/native/v5-02-x08-support/validate.py --evidence tests/native/v5-02-x08-evidence --repo .` | 1 | G06 PASS; G01-G05, G07-G09 FAIL (`raw/r3-19-validate.log`). This evidence directory is the X08 round's, not C01's |

Failing line (both Linux configs): `src/engine/runtime/external_registration.t.cpp:4571` and `:5098`, `CHECK( off - on >= c02_outputs_gap )`, 4737 >= 4742
(plus `resource_limit native_bytes Native conversion allocation exceeded` message at :4558 test). Raw: `raw/linux3/container/{debug,release}-ctest.log`.
Disk after the Linux run: 95 GiB free.

## Inventory

Compared with `../v5-02-x08-evidence/inventory.txt` (the most recent committed-evidence list, 1255 names). Current registered: 1258 (`inventory.txt`, sorted names).
Removed or renamed: none. New (3): `replacement.host path keeps missing and unresolvable directory spellings as input path errors`,
`replacement.host path maps a regular file followed by parent traversal to the root directory error`,
`replacement.X08 C01 the C02 budget fixture root has a fixed canonical byte length`. Debug, Release and both Linux configs list the same 1258 names.

## (a) Acceptance criteria to tests

| criterion | tests (file:line) | status |
|---|---|---|
| C01-1 file followed by `..` gives io "Cannot resolve explicit root directory" on both platforms | `host path maps a regular file followed by parent traversal to the root directory error` (src/engine/capture/host_path.t.cpp:84); `host path sanitizes invalid file traversal and missing parents` (:73); CLI `replacement.cli` (tests/acceptance/bootstrap_paths.py:125) | macOS pass Debug and Release; Linux (run 3) pass for the host path tests |
| C01-1b missing, loop and `file/x/..` spellings stay "Cannot resolve explicit input path" | `host path keeps missing and unresolvable directory spellings as input path errors` (host_path.t.cpp:90) | macOS pass; Linux run 3 pass |
| C01-2 the two budget tests pass on Linux | `X06 D C02 role preparation charges enumeration and registry reads to the factory budget` (src/engine/runtime/external_registration.t.cpp:4557, bound at :4571), `X07 N30 role capture charges the factory budget owner` (:5087, bound at :5098) | macOS pass both configs; Linux run 3 FAIL both (gap 4737 < 4742) |
| C01-3 fixed-root pin | `X08 C01 the C02 budget fixture root has a fixed canonical byte length` (external_registration.t.cpp:4576; `c02_root_bytes` :4512, `c02_outputs_gap` :4555) | present, passes on macOS and Linux (it pins the root length, not the gap) |
| Four Linux failures pass, no other Linux failure | the tests above | NOT MET (two budget tests still fail on Linux in run 3) |
| No existing test name removed or renamed; all 1255 names preserved in both configs; no diagnostic text changed | inventory diff above; lint pass | met |
| Prober shows both budget tests still fail under named mutants | not a gate-runner step; no prober output exists under this evidence directory | NOT RUN here |
| Three red-first records exist | see (b) | NOT MET (one of three-plus recorded) |
| macOS Debug and Release full runs, zero failures and skips | r2-03, r2-05 | met |
| Inventory identical in both configs | r2-06/07 | met |
| Lint, two no-work builds per config, redaction | r2-08 to r2-14 | met |
| Linux gate with Debug and RelWithDebInfo ctest in container | r2-17a/b | Ran in run 3; NOT MET (2 failures per config) |
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

- The Linux gap constant (4742) was measured on macOS; Linux measures 4737, so the two budget tests fail on Linux. The fixed-root pin fixed the root length but not the platform difference in the output gap.
- x86_64 Linux is not covered (arm64 Docker only).
- `x07.two_process_cas_one_winner` flaked once in run 1; it passed in runs 2 and 3.
- Mutation prober output and gate-stage red runs are absent from this directory.
- `validate.py` was run as instructed against the X08 evidence directory and fails there; it was not given C01 inputs.
- `inventory.txt` lists sorted names, not ctest ordinals.
