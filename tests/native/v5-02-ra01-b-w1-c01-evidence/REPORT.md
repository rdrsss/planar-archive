# V5-02-RA01-B-W1-C01 gate report (run 4)

Gate runner result: **passed=true** for every command (lint, both macOS builds and ctest runs, four no-work builds, redaction, both Linux configurations).
Open items that are not gate commands are listed under Limitations (PROBES.md predates two test-file edits; one new test has no red or probe evidence).
No source file was edited by the gate runner. Date 2026-10-31. Docker Desktop was up (server 29.9.2); the docker CLI came from the normal PATH
(`/opt/homebrew/bin/docker`), the obsolete Caskroom path was not used. Run 4 raw logs are under `raw/run4/`; earlier reports are preserved as
`raw/run1-REPORT.md`, `raw/run2-REPORT.md` and `raw/run3-REPORT.md`.

## Candidate identity (verbatim identity.py output, pathspec `src tests/native`; `raw/run4/identity.txt` holds the full per-file table)

| identity | value |
|---|---|
| HEAD | `307026288615e317d145ba6846c0692929c50a17` |
| sha256(git diff --binary HEAD -- src tests/native), tracked only | `eb11d7a210bfb7036f08ed98b900ef3953fffd6e11c5e4e4b399f0f4602d48eb` |
| combined sha256 (diff bytes, then each untracked path NUL bytes, sorted), pathspec src tests/native, evidence excluded | `de65ea1fe7bab915d77bf3e9329b9e2a226f64ac04e3ddad5bdd409e2dabb3d5` |

Compared with run 3 (tracked diff `0de21618...`, combined `b15bb717...`): the tree changed. Two files differ: `src/cmd/planar/authoring.t.cpp`
(`50e9333b...` to `bfa67729...`) and `src/engine/runtime/external_registration.t.cpp` (`23b7e4fa...` to `5386b9d9...`). All other per-file hashes, including
`src/stdlib/planar/commands.lua` (`23b597fa...072f`), are identical, so the X08 pin does not move.

## Commands, exit codes and counts (run 4)

| # | command | exit | result | log |
|---|---|---|---|---|
| 1 | `python3 scripts/rewrite-check.py lint --build-dir build/rewrite-debug` | 0 | clean | raw/run4/lint.log |
| 2 | `cmake --build build/rewrite-debug --target planar_tests` | 0 | success | raw/run4/build-debug.log |
| 3 | `ctest --test-dir build/rewrite-debug --no-tests=error` | 0 | 100% passed, 1344 of 1344, 0 failed, 0 skipped (599.61 s) | raw/run4/ctest-debug.log |
| 4 | `cmake --build build/rewrite-release --target planar_tests` | 0 | success | raw/run4/build-release.log |
| 5 | `ctest --test-dir build/rewrite-release --no-tests=error` | 0 | 100% passed, 1344 of 1344, 0 failed, 0 skipped (239.53 s) | raw/run4/ctest-release.log |
| 6 | no-work build Debug, twice, after command 2 | 0, 0 | both `ninja: no work to do.` (zero steps) | raw/run4/nowork-debug-1.log, -2.log |
| 7 | no-work build RelWithDebInfo, twice, after command 4 | 0, 0 | both `ninja: no work to do.` (zero steps) | raw/run4/nowork-release-1.log, -2.log |
| 8 | `python3 scripts/redact-local-paths.py --repo-path "$PWD" --home "$HOME" check <36 explicit files from git status>` | 0 | empty output | raw/run4/redact.log, redact-files.txt |
| 8a | same script with no file arguments | 1 | not meaningful (scans ignored build output and evidence); recorded only | raw/run4/redact-noargs.log |
| 8b | same check on the evidence and round files | 0 | empty output (9 files, including PROBES.md and this report) | raw/run4/redact-evidence.log, redact-evidence-files.txt |
| 9 | `df -h /System/Volumes/Data` before docker | n/a | 96 GiB free per df -h (floor 15 GiB); `raw/run4/linux/df-before.txt` (script's own 1K-block record) shows 201587632 blocks available | raw/run4/linux/df-before.txt |
| 10 | `scripts/linux-gate.sh <out-dir outside checkout>` (one run builds and tests Debug and RelWithDebInfo; no `-t`) | 0 | docker build exit 0; Debug and RelWithDebInfo: configure, build, ctest, inventory and no-work exits all 0; ctest 100% passed, 1344 of 1344 in each; `x07.two_process_cas_one_winner` Passed in both (test #1335); both no-work logs `ninja: no work to do.`; host manifest before = after = container context manifest | raw/run4/linux/ (container/ per-config logs, stdout.txt script output) |
| 11 | `docker builder prune -f` | 0 | run inside the script (`builder-prune.exit` 0) | raw/run4/linux/builder-prune.log, df-after.txt |

Skips: 0. The only match for "skipped" in any ctest log (macOS and Linux) is the name of `replacement.command load refuses forged family order skipped duplicate and malformed arguments`, which Passed.
The Debug and Release `ctest -N` lists are byte-identical, and the Linux Debug and RelWithDebInfo lists are byte-identical to each other and to the macOS list (1344 names).

## Inventory

`inventory.txt` is the sorted (`LC_ALL=C`) `ctest -N` list for Debug (1344 names, `replacement.` prefix); `raw/run4/inv-debug.txt` and `inv-release.txt` are identical.
Compared with the most recent committed-style inventory, `tests/native/v5-02-ra01-b-w1-evidence/inventory.txt` (wave 1 candidate, 1339 names): removed names none
(`removed-names.txt` empty); new names 5 (`new-names.txt`):
- `RA01-B1 missing leaf remap applies only to diagnostic and from`
- `RA01-B1 path text validation precedes the cwd read`
- `RA01-B1 reserved pre check continues past a malformed entry sorted before a reserved entry` (new since run 3)
- `RA01-B1 reserved pre check skips an entry with any ill typed identity field`
- `RA01-B1 reserved strings in non reserved fields keep their HEAD diagnostic`

1339 + 5 = 1344. Run 3 had 1343 (the fifth name is the only difference).

## Acceptance criteria and findings to tests

Line numbers are the current candidate. "Evidence" means the check is by document or probe, not by a test.

| criterion / finding | tests (file:line) | status at this gate |
|---|---|---|
| Criterion 1, R1 (final probe pass, PROBES.md, I06 applies) | killers for I06: `RA01-B1 authoring option signatures are exactly the RA-1 table` (src/cmd/planar/authoring_catalog.t.cpp:216); I14: `RA01-B1 validator refuses a duplicated authoring row` (authoring_catalog.t.cpp:283) | Probe record now exists at `probes/PROBES.md` in this round's evidence directory (identity there: tracked diff `0de21618...`, combined `b15bb717...`). It is NOT current for the run 4 tree: `authoring.t.cpp` and `external_registration.t.cpp` changed after the prober ran (see Candidate identity), so those two files need byte-identity carry-forward or a re-probe. Not a gate command. |
| R2 a (E17: reserved strings only in non-reserved fields keep HEAD diagnostic) | `RA01-B1 reserved strings in non reserved fields keep their HEAD diagnostic` (src/engine/runtime/external_registration.t.cpp:5335) | passes Debug, Release and both Linux configurations (run 4) |
| R2 b (do-not-skip-malformed: `{name='observe', handler_id=5}` keeps HEAD diagnostic) | `RA01-B1 reserved pre check skips an entry with any ill typed identity field` (external_registration.t.cpp:5393); existing `RA01-B1 reserved pre check skips malformed descriptors and keeps their diagnostic` (:5320); controls `RA01-B1 profile registering a reserved authoring command name / family name / handler id / projection id is refused as identity conflict` (:5240, :5251, :5261, :5271) | passes |
| R2 c (added after the red and probe runs: a malformed entry that sorts before a reserved entry must not stop the pre-check) | `RA01-B1 reserved pre check continues past a malformed entry sorted before a reserved entry` (external_registration.t.cpp:5419) | passes Debug, Release, both Linux. NEW since run 3; no red run and no named mutant for it in the evidence (see Red-first and Limitations) |
| R3 (remap only for `--diagnostic` and `--from`; `--registry`/`--workspace`/`--job` keep existing messages) | `RA01-B1 missing leaf remap applies only to diagnostic and from` (src/cmd/planar/authoring.t.cpp:868); `RA01-B1 explicit path stage keeps the existing messages` (authoring.t.cpp:808); `RA01-B1 selector stage precedes the explicit path stage` (:660); `RA01-B1 explicit path stage precedes the stub refusal` (:678) | passes |
| R4 (I35 restated, I52 corrected) | I35 behaviour pin: `RA01-B1 authoring verbs refuse positionals unknown options and equals syntax as usage` (authoring.t.cpp:548); I52: `RA01-B1 non reserved near miss names stay registrable` (external_registration.t.cpp:5297) | tests pass; record wording is evidence (not verified by this gate) |
| R5 (X08 pin disclosure, re-measure if commands.lua changed) | `X08 C01 role preparation outputs-off boundary pins the request-record conversion charge` (external_registration.t.cpp:4589; constants 227525 macOS at :4570, 227551 Linux at :4573) | passes on macOS Debug and Release and in both Linux configurations (the 227551 Linux constant holds, 1343 of 1343 passed); `commands.lua` hash `23b597fa...072f` unchanged, so no re-pin |
| R6 (record scope D13, owned list) | evidence only (documentation) | not checked by this gate |
| R7 (redaction clean) | evidence: commands 8 and 8c | met, see Redaction |
| R8 (architecture.md sentences) | evidence only (documentation) | not checked by this gate |
| R9 (retag D7, D8, D13, D14; D23 wording) | evidence only (documentation) | not checked by this gate |
| Criterion 2 (new and changed tests fail under named mutants, shown by PROBES.md) | see the red-first section for the pre-fix behaviour; mutant kills are the prober's | Not decided by this gate; PROBES.md exists but predates two test-file edits (see row R1) |
| Criterion 3 (1339 names kept, new names listed, no removed name, diagnostics byte-identical, help and binary path unchanged) | `X07 command sources add no diagnostic outside the closed table` (src/cmd/planar/bootstrap.t.cpp:2096); `RA01-B1 default invoke and production binary refuse all six authoring verbs as usage` (bootstrap.t.cpp:3287) | met: 1339 wave 1 names all kept (comm shows 0 removed), 5 new names, both tests pass |
| Criterion 4 (X08 pin re-measured if commands.lua changed) | as R5 | met (file unchanged) |
| Criterion 5 (redaction clean, docs per R8 and R9) | commands 8 and 8c | redaction met; docs not checked |

## Red-first

Source: `red/RED.md`, `red/raw/t1.log` to `t15.log`, `red/raw/build.log` and `red/raw/NOTE.txt` in this round's evidence directory. Red-time identity from
RED.md: HEAD `307026288615e317d145ba6846c0692929c50a17`; sha256 of `red/snapshot/candidate.diff` (git diff HEAD -- src tests/native)
`7b880280948d74e70409b4b72e7a75dd319dfe45d05f99d564a308775d3da324`; sha256 of `red/snapshot/SHA256SUMS`
`16f5ec126387d1ab0f9b7b547c404c558d577df6b8f1791c4f190236c03b8f90`. Command for every row (exact text from RED.md):
`ctest --test-dir build/rewrite-debug -R '^replacement\.<escaped name>$' --no-tests=error --output-on-failure`, after
`cmake --build build/rewrite-debug --target planar_tests` (success). Failing line numbers are those of the red snapshot (the test files moved
afterwards, current lines are in the table above).

| test | real red or already-passing guard | failing line in the red snapshot |
|---|---|---|
| `RA01-B1 reserved pre check skips an entry with any ill typed identity field` | real red (expectRed true; got `reference` / `External registration identity conflict` where `internal` / `Invalid external profile protocol` is required) | external_registration.t.cpp:5411, 5412, 5413 (and 5414, 5415) |
| `RA01-B1 missing leaf remap applies only to diagnostic and from` | real red (the `--registry` side fails) | authoring.t.cpp:886 (and :919) |
| `RA01-B1 explicit path stage keeps the existing messages` | real red | authoring.t.cpp:855 |
| `RA01-B1 selector stage precedes the explicit path stage` | real red | authoring.t.cpp:672 |
| `RA01-B1 explicit path stage precedes the stub refusal` | real red | authoring.t.cpp:693 |
| `RA01-B1 reserved strings in non reserved fields keep their HEAD diagnostic` | guard, already passed (expectRed false; kills mutant E17 only under the prober) | none |
| `RA01-B1 path text validation precedes the cwd read` | guard, already passed | none |
| `RA01-B1 reserved pre check continues past a malformed entry sorted before a reserved entry` | NO RED EVIDENCE: the test was added after the red run (not among red t1 to t15); no pre-fix command or hash exists for it | n/a |
| `RA01-B1 reserved pre check skips malformed descriptors and keeps their diagnostic` | existing test, already passed | none |
| `RA01-B1 profile registering a reserved authoring command name is refused as identity conflict` | control, already passed | none |
| `RA01-B1 authoring verbs refuse positionals unknown options and equals syntax as usage` | existing, already passed | none |
| `RA01-B1 non reserved near miss names stay registrable` | existing, already passed (body changed after red per R4) | none |
| `X08 C01 role preparation outputs-off boundary pins the request-record conversion charge` | existing, already passed | none |
| `RA01-B1 authoring option signatures are exactly the RA-1 table`, `RA01-B1 validator refuses a duplicated authoring row`, `X07 command sources add no diagnostic outside the closed table` | existing, already passed | none |

Missing from the evidence: the per-test red logs name the red tests but I could not see in them a separate "after-fix green" command per test (the
after-fix green evidence is this gate's full ctest). The RED.md notes that several invariants share one test case, so per-invariant red is attributed at
test-case granularity. The red evidence is behavioural (assertion failures, not compile failures). No red run exists for the four mutants themselves;
those are the prober's.

## Deliberate behaviour differences and the clause that permits each

- `--registry` (and `--workspace`, `--job`) missing leaf keeps `io` / `Explicit input must be a regular file` while `--diagnostic` and `--from` report
  `Cannot resolve explicit input path` / `Cannot resolve explicit root directory`: authoring-contract Messages rows (about :489 to :492) win over Common
  rule 5 (:37 to :42); D8 read literally matches `render` (R3, operator hint).
- Entries with a missing or non-string `name`, `handler_id` or `family` are skipped by the reserved-name pre-check and fall to the existing validation:
  wave 1 plan D14 (R2).
- X08 C01 boundary constants unchanged at 227525 (macOS) and 227551 (Linux) in this round; both earlier moves are authorised by the operator decision
  of 2026-10-30 "re-pin and resume" (R5).

## Redaction

Command 8 covered the 36 committable source and doc files (`raw/run4/redact-files.txt`); command 8b covers the evidence and round files after this report was written (`raw/run4/redact-evidence-files.txt`). Both exit 0 with empty output (see the table above). Evidence directories are git-ignored, so explicit arguments are required. The no-argument invocation (8a) exits 1 on ignored build output and is recorded only.

## Limitations

- `probes/PROBES.md` was produced on an earlier tree. Since then `src/cmd/planar/authoring.t.cpp` (`50e9333b...` to `bfa67729...`) and `src/engine/runtime/external_registration.t.cpp` (`23b7e4fa...` to `5386b9d9...`) changed (the latter adds the test named in R2 c). All other files are byte-identical to the probed tree. The new test has no probe row.
- The new test `continues past a malformed entry sorted before a reserved entry` has no red-first evidence.
- Docs-only findings R6, R8 and R9 and the record wording of R4 and R5 were not verified by this gate.
- Linux covers linux/arm64 only; x86_64 is not covered.
- A stale directory `scratchpad/linux-run4` from an earlier session made the first linux-gate.sh invocation refuse (out-dir exists, exit 2, nothing ran); the real Linux run used `scratchpad/linux-c01-run4` and is stored under `raw/run4/linux/`.
