# Gate report V5-02-RA01-B-W2-C01 (run 3)

Gate runner result: **PASSED**. Every command met its criterion. Lint exit 0. macOS Debug and RelWithDebInfo ran 1437 of 1437 tests with zero failures and zero skips. Two no-work builds per configuration did zero steps. Linux Debug and RelWithDebInfo ran 1437 of 1437 with zero skips, and `x07.two_process_cas_one_winner` is included in both. The redaction check on explicit files is recorded in the Redaction section.

This is the third gate run. Run 1 is `REPORT-run1.md` (one Debug timeout). Run 2 is `REPORT-run2.md` (passed, 1436 tests). The checkout changed again after run 2: the combined identity moved from `afc66baa...` to `1f30a26e...` and one test was added (1436 to 1437). Run 3 raw logs are under `raw/run3/`; nothing was deleted. The gate runner edited no source file.

## Candidate identity

HEAD `3343ad38304db57388bc7ac2438a40043eba1b7d`. Output of the workflow `identity.py` (default pathspecs `src tests/native`, per-file table in `raw/run3/identity.txt`):

| identity | value |
|---|---|
| sha256(git diff --binary HEAD -- src tests/native), tracked only | `d7071fd2cdacee177cd4d447c707c11c5349541bb1fea790d91377912f454665` |
| combined sha256 (diff bytes, then each untracked path NUL bytes, sorted), pathspec src tests/native, evidence excluded | `1f30a26e7150d3de4795dfebaf924707beeadaf54bffbe8b93fe63ed03d67b55` |

The tracked-diff hash equals runs 1 and 2. The combined hash differs from run 2 (`afc66baa...`), from run 1 and from the round record's final-record block (`bd1ce8b7...`), so at least one untracked file changed since run 2 (the run 3 tracked-diff hash is the same as run 2, so the added test must live in an untracked file or the change is outside the tracked diff; I did not diff the files). The round record's identity block and the probe record's identity (`afc66baa...`) are stale relative to this candidate and need refreshing by their owners.

## Commands

All ran serially from the repository root, unpiped, exit codes read from files in `raw/run3/`.

| # | command | exit | result |
|---|---|---|---|
| 1 | `python3 scripts/rewrite-check.py lint --build-dir build/rewrite-debug` | 0 | `Formatting and Doxygen checked 171 active source files` (lint.log) |
| 2 | `cmake --build build/rewrite-debug --target planar_tests` | 0 | `ninja: no work to do.` (the Debug tree already held these bytes; build-debug.log) |
| 3 | `ctest --test-dir build/rewrite-debug --no-tests=error --output-on-failure -j4` | 0 | 100% passed, 1437 of 1437, 0 failed, 0 skipped, 0 not run; real time 178.51 s (ctest-debug.log) |
| 4 | `ctest --test-dir build/rewrite-debug -N` | 0 | `Total Tests: 1437` |
| 5 | `cmake --build build/rewrite-release --target planar_tests` | 0 | real rebuild (steps 2 to 5 of 5 shown, directory.t.cpp recompiled and linked; build-release.log) |
| 6 | `ctest --test-dir build/rewrite-release --no-tests=error --output-on-failure -j4` | 0 | 100% passed, 1437 of 1437, 0 failed, 0 skipped; real time 74.89 s (ctest-release.log) |
| 7 | `ctest --test-dir build/rewrite-release -N` | 0 | `Total Tests: 1437` |
| 8 | `cmake --build build/rewrite-{debug,release} --target planar_tests`, twice each, after the successful builds above | 0 x4 | each `[0/2] Re-checking globbed directories...` then `ninja: no work to do.`: zero steps (nowork-{debug,release}-{1,2}.log) |
| 9 | `scripts/linux-gate.sh <scratch dir outside the checkout>` | 0 | docker build exit 0; container configure, build, ctest, inventory and no-work exits all 0 in both configurations; builder prune exit 0 (raw/run3/linux/) |
| 10 | redaction check on explicit files | see Redaction | see Redaction |

Linux (one container build runs Debug then RelWithDebInfo; no `-t`; one build at a time). `df -h /System/Volumes/Data` before the build showed 66 GiB available (139387704 512-byte blocks free) against the 15 GiB halt threshold; after the build 99325424 blocks (about 47 GiB). `docker builder prune -f` ran once through the script, exit 0. No foreign image, container or volume was touched.

| Linux config | ctest exit | result | no-work build |
|---|---|---|---|
| Debug | 0 | 100% passed, 1437 of 1437, 0 skipped; real time 613.49 s | exit 0, `ninja: no work to do.` |
| RelWithDebInfo | 0 | 100% passed, 1437 of 1437, 0 skipped; real time 242.52 s | exit 0, `ninja: no work to do.` |

`x07.two_process_cas_one_winner` (Test #1428) passed in macOS Debug and RelWithDebInfo and in both Linux configurations (Linux Debug 0.95 s, Linux RelWithDebInfo 0.46 s). The pin test `X08 C01 role preparation outputs-off boundary pins the request-record conversion charge` (`src/engine/runtime/external_registration.t.cpp:4589`) passed in all four runs. The previously timing-out prompt test took macOS Debug 17.68 s, macOS RelWithDebInfo 4.91 s, Linux Debug 15.41 s, Linux RelWithDebInfo 4.67 s, all Passed (limit 30 s). Host manifests before and after the build were equal, and equal to the container context manifest (linux/manifest-compare.txt).
The gate did not re-measure the Linux acceptance-5 pin value; the pin test passing at its recorded constants is all this gate shows.

Skips: no log reports Skipped, Not Run or Failed.

## Inventory

All four `ctest -N` or container inventories hold 1437 tests and are identical after normalisation: macOS Debug equals RelWithDebInfo by cmp, Linux Debug equals RelWithDebInfo by cmp, and Linux equals macOS once the `replacement.` prefix is stripped. Saved as `inventory.txt` (sorted, with the `replacement.` prefix); `new-names.txt` and `removed-names.txt` sit beside it.

Comparison list: `tests/native/v5-02-ra01-b-w2-evidence/inventory.txt`, the wave 2 gate inventory (1416 names), which is the most recent committed-style inventory list from before this corrective (wave 1 is older). **Removed: 0**: all 1416 wave 2 names are kept. **Added: 21** (`new-names.txt`), 1416 + 21 = 1437. Compared with the run 2 inventory (1436), the only difference is one added name, marked below.

- RA01-B2 C01 capture_diagnostic bad_alloc refusal carries diagnostic_bytes
- RA01-B2 C01 capture_diagnostic maps a failed open to cannot resolve
- RA01-B2 C01 capture_diagnostic maps a post-gate add_input failure to file changed
- RA01-B2 C01 capture_diagnostic post-gate failures are remapped at every native call
- RA01-B2 C01 capture_diagnostic refuses a non-regular file as io
- RA01-B2 C01 capture_diagnostic refuses a symlink as a symbolic link
- RA01-B2 C01 critique global prescription needs a nonempty check
- RA01-B2 C01 observe applies precedence in sorted name order
- RA01-B2 C01 observe bad_alloc refusal carries observation_total_bytes
- RA01-B2 C01 observe checkpoint canonical size is bounded at exactly 8192 bytes
- RA01-B2 C01 observe digest allocation failure is a bound refusal
- **RA01-B2 C01 observe maps a failed entry inspection after listing to cannot inspect** (new since run 2; `directory.t.cpp:341`)
- RA01-B2 C01 observe maps an unreadable entry to cannot inspect
- RA01-B2 C01 observe recheck bad_alloc is a limit and not a membership change
- RA01-B2 C01 observe recheck growth past the total bound stays membership changed
- RA01-B2 C01 observe refuses a symlink root
- RA01-B2 C01 observe refuses an unreadable entry as io exit 4
- RA01-B2 C01 observe refuses an unreadable root as io exit 4
- RA01-B2 C01 prompt of exactly 1048576 bytes is accepted and one more byte is refused
- RA01-B2 C01 rejection excerpt with a non-digit index is the fixed line
- RA01-B2 C01 slot type checks are pinned in the Lua handlers

## Acceptance criteria and findings to tests

Test names omit the ctest `replacement.` prefix. Line numbers are the `TEST_CASE` lines of the final candidate (the added test shifted later `directory.t.cpp` lines by 36 compared with run 2). "Passed" means passed in all four configurations of run 3. Mutation-probe results are in `probes/PROBES.md`, run on the `afc66baa...` candidate (20 C01 tests); I did not read or verify them for this report, so no kill claim is made here, and the added test post-dates that probe run.

| criterion or finding | test name | file:line | result |
|---|---|---|---|
| 1 / R1, unreadable `checks` directory and unreadable file (Q-C01-1 row) | RA01-B2 C01 observe maps an unreadable entry to cannot inspect | src/engine/capture/directory.t.cpp:290 | passed |
| 1 / R1, entry inspection failing after listing (fstatat/open race site) | RA01-B2 C01 observe maps a failed entry inspection after listing to cannot inspect | directory.t.cpp:341 | passed; new in run 3 |
| 1 / R1, CLI level, exit 4, closed envelope | RA01-B2 C01 observe refuses an unreadable entry as io exit 4 | src/cmd/planar/authoring_observe.t.cpp:666 | passed |
| 1 / R1, unreadable observation root | RA01-B2 C01 observe refuses an unreadable root as io exit 4 | authoring_observe.t.cpp:700 | passed |
| 1 / R1, `scan_once` bad_alloc carries `observation_total_bytes` | RA01-B2 C01 observe bad_alloc refusal carries observation_total_bytes | directory.t.cpp:396 | passed |
| 1 / R1, recheck bad_alloc is a limit, legitimate growth stays membership changed | RA01-B2 C01 observe recheck bad_alloc is a limit and not a membership change; RA01-B2 C01 observe recheck growth past the total bound stays membership changed | directory.t.cpp:411; :423 | passed |
| 1 / R1, observe digest allocation failure | RA01-B2 C01 observe digest allocation failure is a bound refusal | authoring_observe.t.cpp:768 | passed |
| 1 / R1, `capture_diagnostic` symlink race site | RA01-B2 C01 capture_diagnostic refuses a symlink as a symbolic link | src/engine/capture/capture.t.cpp:949 | passed |
| 1 / R1, non-regular file is `io` | RA01-B2 C01 capture_diagnostic refuses a non-regular file as io | capture.t.cpp:986 | passed |
| 1 / R1, failed open | RA01-B2 C01 capture_diagnostic maps a failed open to cannot resolve | capture.t.cpp:1010 | passed |
| 1 / R1, post-gate failures | RA01-B2 C01 capture_diagnostic maps a post-gate add_input failure to file changed; RA01-B2 C01 capture_diagnostic post-gate failures are remapped at every native call | capture.t.cpp:1038; :1064 | passed |
| 1 / R1, `capture_diagnostic` bad_alloc carries `diagnostic_bytes` | RA01-B2 C01 capture_diagnostic bad_alloc refusal carries diagnostic_bytes | capture.t.cpp:1053 | passed |
| 1 / R1, existing diagnostics byte-identical | existing test RA01-B2 capture_diagnostic refuses a file changed before verification (sections updated to the new pairs, name kept) and every pre-existing suite | capture.t.cpp:835 | passed |
| 2 / R2, X16 (dangling symlink and symlink to 70000 bytes) | RA01-B2 C01 capture_diagnostic refuses a symlink as a symbolic link | capture.t.cpp:949 | passed |
| 2 / R2, X17 (symlink root at engine level) | RA01-B2 C01 observe refuses a symlink root | directory.t.cpp:377 | passed |
| 2 / R2, X03 (checkpoint bound 8192 against 8193) | RA01-B2 C01 observe checkpoint canonical size is bounded at exactly 8192 bytes | authoring_observe.t.cpp:718 | passed |
| 2 / R2, X04 (prompt of exactly 1048576 bytes) | RA01-B2 C01 prompt of exactly 1048576 bytes is accepted and one more byte is refused | src/cmd/planar/authoring_prompts.t.cpp:2049 | passed in all four (run 1 timed out in Debug) |
| 2 / R2, X05 (`global` prescription with only `not_applicable` checks) | RA01-B2 C01 critique global prescription needs a nonempty check | authoring_prompts.t.cpp:2115 | passed |
| 2 / R2, X07 (non-digit `[x]` index gives the fixed line) | RA01-B2 C01 rejection excerpt with a non-digit index is the fixed line | authoring_prompts.t.cpp:2140 | passed |
| 2 / R2, W2-51 (sort precedence, early unsafe and oversize entries) | RA01-B2 C01 observe applies precedence in sorted name order | directory.t.cpp:443 | passed |
| 3 / R3, handler slot check, W2-06n | RA01-B2 C01 slot type checks are pinned in the Lua handlers; RA01-B2 diagnostic and from reach Lua only as opaque slots (existing) | src/cmd/planar/authoring_orchestrator.t.cpp:614; :210 | passed |
| 4 / R4, `docs/architecture.md` and the stale `@note Stub` | no test; Doxygen check inside the lint command | not applicable | lint exit 0 |
| 4 / R5, R6, R7, R8 records | no test; records in the round file and c02-pin.md | not applicable | not machine-checked here |
| 5, 1416 kept, new listed, identical inventory | inventory comparison above | not applicable | 0 removed, 21 added, four identical inventories |
| 5, boundary pin | X08 C01 role preparation outputs-off boundary pins the request-record conversion charge | src/engine/runtime/external_registration.t.cpp:4589 | passed in all four runs |
| 6, opus review | not part of this gate | not applicable | not run here |

## Red-first

Source: `red/RED.md`. Red-time identity: HEAD `3343ad38304db57388bc7ac2438a40043eba1b7d`, snapshot `red/snapshot-2/`, sha256 of `candidate.diff` (`git diff HEAD -- src tests/native`) `316a0581affa8fc08c6bfb90378bcf8d831f099472973a19a2d874c6c619a9ca`, sha256 of `SHA256SUMS` (candidate.diff plus 11 untracked files) `954f2b5112a1bb93316bc4224ae8c377cd02daea1c83df82fd3314f7bd5ef228`.
The pre-fix product sources are in `red/snapshot/`. Build: `cmake --build build/rewrite-debug --target planar_tests`, exit 0.
Command per test, as recorded in RED.md: `ctest --test-dir build/rewrite-debug -R '^replacement\.<escaped name>$' --no-tests=error --output-on-failure`. Raw logs: `red/raw/NN.log`.
RED.md says the pre-fix product is not hashed separately from `snapshot/`. I found no hash of `red/snapshot/` itself.

| log | test | real red or guard | failing line at red time |
|---|---|---|---|
| 04 | observe bad_alloc refusal carries observation_total_bytes | real red | directory.t.cpp:370 |
| 06 | capture_diagnostic refuses a symlink as a symbolic link | real red | capture.t.cpp:962 |
| 07 | capture_diagnostic refuses a non-regular file as io | real red | capture.t.cpp:996 |
| 08 | capture_diagnostic maps a failed open to cannot resolve | real red | capture.t.cpp:1019 |
| 09 | capture_diagnostic bad_alloc refusal carries diagnostic_bytes | real red | capture.t.cpp:1060 |
| 10 | RA01-B2 capture_diagnostic refuses a file changed before verification (existing name) | real red | capture.t.cpp:894 |
| 18 | capture_diagnostic maps a post-gate add_input failure to file changed | real red | capture.t.cpp:1049 |
| 19 | capture_diagnostic post-gate failures are remapped at every native call | real red | capture.t.cpp:1072 |
| 20 | observe recheck bad_alloc is a limit | real red | directory.t.cpp:382 |
| 22 | observe digest allocation failure is a bound refusal | real red | authoring_observe.t.cpp:771 |
| 23 | slot type checks are pinned in the Lua handlers | real red | authoring_orchestrator.t.cpp:621 |
| 01, 02, 03, 05, 11, 13, 14, 15, 16, 17, 21 | observe unreadable entry (01), unreadable entry exit 4 (02), symlink root (03), sort precedence (05), checkpoint 8192 (11), critique global prescription (13), non-digit index (14), opaque slots (15), outputs-off boundary (16), unreadable root exit 4 (17), recheck growth (21) | guard that already passed on the pre-fix code (expectRed false in RED.md) | none |
| 12 | prompt of exactly 1048576 bytes is accepted and one more byte is refused | guard, Timeout at 30 s under ctest, not an assertion failure | none (timeout) |
| none | observe maps a failed entry inspection after listing to cannot inspect | **no red-first record exists**: the test is new since RED.md and is not in its table; no command, snapshot hash or failing line is available | not available |

Failing line numbers are red-time numbering and differ from the file:line column above. Gaps: no Linux red run exists; the pre-fix `red/snapshot/` has no hash of its own; the new test `observe maps a failed entry inspection after listing...` has no red-first evidence, so whether it is a real red or a guard is unknown.

## Deliberate behaviour differences

The intended message changes are those the round record lists, each under a contract clause (`authoring-contract.md` Messages table, Common rules 4 and 8, `command-contract.md:144-154`):

- `capture_diagnostic` symlink race: `path_denied` / `Explicit input file is a symbolic link` instead of `Only regular files can be captured` (Messages row 493, Q-C01-2).
- `capture_diagnostic` non-regular file: `io` / `Explicit input must be a regular file` (row 494); failed open or fstat: `io` / `Cannot resolve explicit input path` (row 492).
- Observation entry that exists but cannot be opened, listed or stat-ed: `io` / `Cannot inspect confined input`, exit 4 (row added by the Q-C01-1 option A amendment, row 501 of the contract).
- `bad_alloc` in `scan_once`, the recheck scan, the observe digest and `capture_diagnostic`: `resource_limit` with limits `observation_total_bytes` and `diagnostic_bytes` and their maxima (Q-C01-2 option A, row 504).
- Post-gate failures in `capture_diagnostic` are remapped to `capture_changed` / `File changed during read` (D-C01-08, D-C01-25).
No gate output contradicts these. The gate did not independently re-verify byte-identity of existing diagnostics beyond the existing suites passing unchanged.

## Run history

Run 1 (`REPORT-run1.md`): Debug timeout of the prompt test on macOS and Linux Debug (30.03 s and 30.04 s), everything else passed. Run 2 (`REPORT-run2.md`): prompt test edited, 1436 of 1436 passed in all four configurations. Run 3 (this report): checkout changed again (combined identity `1f30a26e...`, one added test), 1437 of 1437 passed in all four configurations.

## Redaction

`python3 scripts/redact-local-paths.py --repo-path <repo> --home $HOME check <files>` over 72 explicit files (raw/run3/redact-final-files.txt): every modified or untracked file that `git status -uall` shows outside ignored build output, plus the committable evidence files of this round (REPORT.md, REPORT-run1.md, REPORT-run2.md, README.md, inventory.txt, new-names.txt, removed-names.txt, c01-only.patch, plan/DECISIONS.md, plan/INVARIANTS.md, red/RED.md, probes/PROBES.md) and the W2 evidence records: **exit 0**, no output (raw/run3/redact-final.log). The evidence directories are gitignored, so git status does not list them.
Run without file arguments the script exited 1 because it scans ignored build output and local settings (raw/run3/redact-noargs.log); that run is not meaningful for the commit, and the explicit-file run is the criterion. Raw logs under `raw/` and the `red/snapshot*` copies are not redaction-checked and must not be staged.

## Limitations

- The added test `observe maps a failed entry inspection after listing to cannot inspect` has no red-first record, and `probes/PROBES.md` predates it, so its ability to fail has not been shown by this gate.
- I did not read `probes/PROBES.md` results into this report; kill evidence is the prober's record, on identity `afc66baa...`, not on this candidate.
- The macOS Debug build (step 2) had no work to do, so the Debug build was proven by the earlier tree state plus the no-work and full ctest runs; the RelWithDebInfo build was a real (partial) rebuild.
- The macOS Debug prompt test (17.68 s) and Linux Debug (15.41 s) have 12 to 15 s of margin against the 30 s ctest limit.
- Linux is arm64 only. The container ran Debug and RelWithDebInfo in a single build.
- The round record's final-record combined identity (`bd1ce8b7...`) and the probe record's (`afc66baa...`) no longer match the checkout (`1f30a26e...`).
