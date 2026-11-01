# Gate report V5-02-RA01-B-W2-C01

Gate runner result: **FAILED**. One test, `RA01-B2 C01 prompt of exactly 1048576 bytes is accepted and one more byte is refused`
(`src/cmd/planar/authoring_prompts.t.cpp:2049`), times out at the 30 s ctest limit in both Debug configurations (macOS Debug and Linux Debug).
It passes in both RelWithDebInfo configurations, but only just (21.4 s macOS, 21.0 s Linux). Everything else passed.
The red-first run already recorded this guard timing out at 30 s (RED.md row 12), so the problem was known and not fixed.
The gate runner edited no source file.

## Candidate identity

HEAD `3343ad38304db57388bc7ac2438a40043eba1b7d`. The workflow `identity.py` output (default pathspecs `src tests/native`, raw/identity.txt) is:

| identity | value |
|---|---|
| sha256 of `git diff --binary HEAD -- src tests/native`, tracked only | `d7071fd2cdacee177cd4d447c707c11c5349541bb1fea790d91377912f454665` |
| combined sha256 (diff bytes then untracked paths and bytes), evidence excluded | `bd1ce8b7cb86b1b1a99060ce7154f2a314c2c36e256a637fa29ad563ec37b76f` |

Both values equal the values in the final record of the round file, so the gate ran on the candidate the coders recorded. The per-file table is in raw/identity.txt.

## Commands

All commands ran from the repository root, unpiped. Raw logs are in `raw/`.

| # | command | exit | result |
|---|---|---|---|
| 1 | `python3 scripts/rewrite-check.py lint --build-dir build/rewrite-debug` | 0 | `Formatting and Doxygen checked 171 active source files` (raw/lint.log) |
| 2 | `cmake --build build/rewrite-debug --target planar_tests` | 0 | `ninja: no work to do.` (an earlier build already held these bytes; raw/build-debug.log) |
| 3 | `ctest --test-dir build/rewrite-debug --no-tests=error --output-on-failure -j4` | **8** | 99% passed, **1 failed out of 1436**: the prompt test above, `***Timeout 30.03 sec` (raw/ctest-debug.log). 0 skipped, 0 not run |
| 3b | rerun of that one test alone, Debug, to rule out load from `-j4` | **8** | `***Timeout 30.02 sec` with 1 test matched (raw/rerun-prompt-debug.log) |
| 4 | `cmake --build build/rewrite-release --target planar_tests` | 0 | `ninja: no work to do.` (raw/build-release.log) |
| 5 | `ctest --test-dir build/rewrite-release --no-tests=error --output-on-failure -j4` | 0 | 100% passed out of 1436, 0 failed, 0 skipped (raw/ctest-release.log); the prompt test took 21.43 s |
| 5b | rerun of the prompt test alone, RelWithDebInfo | 0 | Passed 21.61 s (raw/rerun-prompt-release.log) |
| 6 | `ctest -N` in each config | 0, 0 | 1436 tests each (raw/ctest-N-*.log) |
| 7 | `cmake --build ... --target planar_tests` twice more per config, after the successful builds | 0 x4 | each `[0/2] Re-checking globbed directories...` then `ninja: no work to do.` (raw/nowork-{debug,release}-{1,2}.log). Zero steps |
| 8 | `python3 scripts/redact-local-paths.py --repo-path <repo> --home $HOME check <54 explicit files>` | 0 | no output (raw/redact.log; list raw/redact-files.txt, see Redaction) |
| 9 | `scripts/linux-gate.sh <scratch>` (one container build runs Debug then RelWithDebInfo; no -t, one build) | 0 | docker-build.exit 0, configure, build, inventory and no-work exits all 0 in both configs; Debug ctest exit **8**, RelWithDebInfo ctest exit 0; builder prune exit 0; host manifests equal before and after and equal to the container context manifest (raw/linux/) |

The Linux gate script returned 0 because it returns the docker build status. The container ctest exits are the real result:

| Linux config | ctest exit | result |
|---|---|---|
| Debug | 8 | 99% passed, 1 failed out of 1436: the same prompt test, `***Timeout 30.04 sec`; 0 skipped |
| RelWithDebInfo | 0 | 100% passed out of 1436, 0 skipped; prompt test 20.95 s |

`x07.two_process_cas_one_winner` (Test #1427) passed in both Linux configs (0.95 s and 0.46 s). `X08 C01 role preparation outputs-off boundary pins the request-record conversion charge`
(`external_registration.t.cpp:4589`, pin 227275 macOS and 227301 Linux, unchanged) passed in all four runs. The two no-work builds in the container (debug-nowork.exit and release-nowork.exit) are 0.
Disk before the docker build: 114 GiB free (df raw/linux/df-before.txt shows 238832120 512-byte blocks, about 114 GiB); after: about 89 GiB (185852176 blocks); `docker builder prune -f` ran once afterwards. No foreign image, container or volume was touched.

Skips: no log line reports Skipped or Not Run. The only text match for "skipped" is the passing test named `replacement.command load refuses forged family order skipped duplicate and malformed arguments`.

## Inventory

`ctest -N`: 1436 tests in macOS Debug, macOS RelWithDebInfo, Linux Debug and Linux RelWithDebInfo; the sorted lists are identical across all four (verified with cmp after stripping the Linux and macOS formatting, raw/inv-*.sorted and raw/linux/container/*-inventory-sorted.txt).
The full list is `inventory.txt` (sorted, with the `replacement.` prefix as ctest prints it).

Comparison list: `tests/native/v5-02-ra01-b-w2-evidence/inventory.txt`, the wave 2 gate inventory (the most recent committed-or-recorded inventory; wave 1 C01 is older). It had 1416 names.
**Removed: 0** (`removed-names.txt` is empty), so all 1416 wave 2 names are kept. **Added: 20** (`new-names.txt`), 1416 + 20 = 1436:

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

Test names below omit the ctest `replacement.` prefix. Line numbers are the `TEST_CASE` lines in the final candidate. "Passed" means passed in macOS RelWithDebInfo and Linux RelWithDebInfo
(and in Debug on both, except the prompt test, which timed out in both Debug runs). Probe results (kill evidence) are not in this report: `probes/PROBES.md` does not exist yet (the round record says the prober has not run).

| criterion or finding | test name | file:line | result |
|---|---|---|---|
| 1 / R1, unreadable `checks` directory and unreadable file (Q-C01-1 row) | RA01-B2 C01 observe maps an unreadable entry to cannot inspect | src/engine/capture/directory.t.cpp:290 | passed |
| 1 / R1, CLI level, exit 4, closed envelope | RA01-B2 C01 observe refuses an unreadable entry as io exit 4 | src/cmd/planar/authoring_observe.t.cpp:666 | passed |
| 1 / R1, unreadable observation root | RA01-B2 C01 observe refuses an unreadable root as io exit 4 | src/cmd/planar/authoring_observe.t.cpp:700 | passed |
| 1 / R1, `scan_once` bad_alloc carries `observation_total_bytes` | RA01-B2 C01 observe bad_alloc refusal carries observation_total_bytes | src/engine/capture/directory.t.cpp:360 | passed |
| 1 / R1, recheck bad_alloc is a limit, legitimate growth stays membership changed | RA01-B2 C01 observe recheck bad_alloc is a limit and not a membership change; RA01-B2 C01 observe recheck growth past the total bound stays membership changed | directory.t.cpp:375; :387 | passed |
| 1 / R1, observe digest allocation failure | RA01-B2 C01 observe digest allocation failure is a bound refusal | authoring_observe.t.cpp:768 | passed |
| 1 / R1, `capture_diagnostic` symlink race site | RA01-B2 C01 capture_diagnostic refuses a symlink as a symbolic link | src/engine/capture/capture.t.cpp:949 | passed |
| 1 / R1, non-regular file is `io` | RA01-B2 C01 capture_diagnostic refuses a non-regular file as io | capture.t.cpp:986 | passed |
| 1 / R1, failed open | RA01-B2 C01 capture_diagnostic maps a failed open to cannot resolve | capture.t.cpp:1010 | passed |
| 1 / R1, post-gate failures | RA01-B2 C01 capture_diagnostic maps a post-gate add_input failure to file changed; RA01-B2 C01 capture_diagnostic post-gate failures are remapped at every native call | capture.t.cpp:1038; :1064 | passed |
| 1 / R1, `capture_diagnostic` bad_alloc carries `diagnostic_bytes` | RA01-B2 C01 capture_diagnostic bad_alloc refusal carries diagnostic_bytes | capture.t.cpp:1053 | passed |
| 1 / R1, existing diagnostics byte-identical | existing test RA01-B2 capture_diagnostic refuses a file changed before verification (sections updated to the new pairs, name kept) and every pre-existing suite | capture.t.cpp:835 | passed |
| 2 / R2, X16 (dangling symlink and symlink to 70000 bytes) | RA01-B2 C01 capture_diagnostic refuses a symlink as a symbolic link | capture.t.cpp:949 | passed; kill not shown (no PROBES.md) |
| 2 / R2, X17 (symlink root at engine level) | RA01-B2 C01 observe refuses a symlink root | directory.t.cpp:341 | passed; kill not shown |
| 2 / R2, X03 (checkpoint bound 8192 against 8193) | RA01-B2 C01 observe checkpoint canonical size is bounded at exactly 8192 bytes | authoring_observe.t.cpp:718 | passed; kill not shown |
| 2 / R2, X04 (prompt of exactly 1048576 bytes) | RA01-B2 C01 prompt of exactly 1048576 bytes is accepted and one more byte is refused | src/cmd/planar/authoring_prompts.t.cpp:2049 | **FAILED in Debug (macOS and Linux): 30 s timeout**; passed in RelWithDebInfo |
| 2 / R2, X05 (`global` prescription with only `not_applicable` checks) | RA01-B2 C01 critique global prescription needs a nonempty check | authoring_prompts.t.cpp:2115 | passed; kill not shown |
| 2 / R2, X07 (non-digit `[x]` index gives the fixed line) | RA01-B2 C01 rejection excerpt with a non-digit index is the fixed line | authoring_prompts.t.cpp:2140 | passed; kill not shown |
| 2 / R2, W2-51 (sort precedence, early unsafe and oversize entries) | RA01-B2 C01 observe applies precedence in sorted name order | directory.t.cpp:407 | passed; kill not shown |
| 3 / R3, handler slot check, W2-06n | RA01-B2 C01 slot type checks are pinned in the Lua handlers; RA01-B2 diagnostic and from reach Lua only as opaque slots (existing) | src/cmd/planar/authoring_orchestrator.t.cpp:209; :210 | passed; kill not shown |
| 4 / R4, `docs/architecture.md` and the stale `@note Stub` | no test; Doxygen check inside the lint command | not applicable | lint exit 0 |
| 4 / R5, R6, R7, R8 records | no test; records in the round file and c02-pin.md | not applicable | not machine-checked here |
| 5, 1416 kept, new listed, identical inventory | inventory comparison above | not applicable | 0 removed, 20 added, four identical inventories |
| 5, boundary pin | X08 C01 role preparation outputs-off boundary pins the request-record conversion charge | src/engine/runtime/external_registration.t.cpp:4589 | passed in all four runs; pin unchanged 227275 and 227301 |
| 6, opus review | not part of this gate | not applicable | not run here |

## Red-first

Source: `red/RED.md`. Red-time identity: HEAD `3343ad38304db57388bc7ac2438a40043eba1b7d`, snapshot `red/snapshot-2/`, sha256 of `candidate.diff` (`git diff HEAD -- src tests/native`)
`316a0581affa8fc08c6bfb90378bcf8d831f099472973a19a2d874c6c619a9ca`, sha256 of `SHA256SUMS` (candidate.diff plus 11 untracked files) `954f2b5112a1bb93316bc4224ae8c377cd02daea1c83df82fd3314f7bd5ef228`.
The pre-fix product sources are in `red/snapshot/`. Build: `cmake --build build/rewrite-debug --target planar_tests`, exit 0.
Command per test, as recorded in RED.md: `ctest --test-dir build/rewrite-debug -R '^replacement\.<escaped name>$' --no-tests=error --output-on-failure`. Raw logs: `red/raw/NN.log`.
RED.md also says: the pre-fix product is not hashed separately from `snapshot/`; the recorded hashes cover the whole red-time tree, not the snapshot directory alone. I did not find a hash of `red/snapshot/` itself.

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
| 12 | prompt of exactly 1048576 bytes is accepted and one more byte is refused | guard, outcome Timeout at 30 s under ctest, not an assertion failure | none (timeout) |

Failing line numbers are red-time numbering; files have changed since, so they differ from the file:line column above. Row 12 is the same test that fails this gate: its 30 s timeout was visible at red time and carried into the final candidate.
RED.md records no weakening of the six C01 tests that were guards for X03, X04, X05, X07, X16, X17 and W2-51: of these, X16 (06), the checkpoint and prompt guards (11, 12) differ: X16 was a real red, X03 (11), X05 (13), X07 (14), W2-51 (05), X17 (03) and X04 (12) were guards against already-correct production code, which is what R2 asked for (missing tests, not missing behaviour).
Gaps: no Linux red run exists; the plan files list which mutant each guard is meant to kill, but no probe result exists yet.

## Deliberate behaviour differences

The only intended changes of messages are the ones the round record lists, each under a contract clause (`authoring-contract.md` Messages table, Common rules 4 and 8, `command-contract.md:144-154`):

- `capture_diagnostic` symlink race: `path_denied` / `Explicit input file is a symbolic link` instead of `Only regular files can be captured` (Messages row 493, Q-C01-2).
- `capture_diagnostic` non-regular file: `io` / `Explicit input must be a regular file` (row 494); failed open or fstat: `io` / `Cannot resolve explicit input path` (row 492).
- Observation entry that exists but cannot be opened, listed or stat-ed: `io` / `Cannot inspect confined input`, exit 4 (the row added by amendment under Q-C01-1 option A, row 501 of the contract).
- `bad_alloc` in `scan_once`, the recheck scan, the observe digest and `capture_diagnostic`: `resource_limit` with limits `observation_total_bytes` and `diagnostic_bytes` and their maxima (Q-C01-2 option A, row 504).
- Post-gate failures in `capture_diagnostic` are remapped to `capture_changed` / `File changed during read` (D-C01-08, D-C01-25).
No gate output contradicts these. The gate did not independently re-verify that existing diagnostics are byte-identical beyond the existing suites passing unchanged.

## Redaction

`python3 scripts/redact-local-paths.py --repo-path <repo> --home $HOME check <files>` over every modified or untracked file outside `raw/` and the red `snapshot*` directories
(raw/redact-files.txt: 72 files: the git-visible changes plus, explicitly, every committable evidence file of this round including REPORT.md, inventory.txt, new-names.txt, removed-names.txt, README.md, c01-only.patch, plan and red/RED.md, and the W2 evidence records; the evidence directories are gitignored so git status does not list them): exit 0 (raw/redact-final.log). Row 8 of the command table used the first 54-file list; the final 72-file run also exited 0.
Without file arguments the same script exits 1 because it scans ignored build output and `.claude/settings.local.json` (raw/redact-noargs.log); that run is not meaningful for the commit. Raw logs under `raw/` and the `red/snapshot*` copies are not redaction-checked and must not be staged.

## Limitations

- The gate failed on the Debug timeout. The test is slow by design (it calibrates a 1 MiB prompt with measured runs); 21 s in RelWithDebInfo leaves little margin against the 30 s limit, and Debug exceeds it. Options for the coordinator: shorten the test, or give it a larger `TIMEOUT` through the reviewed CMake helper. The gate runner did neither.
- Probes (acceptance 2 kill evidence) and the Opus review are not part of this gate.
- Linux is arm64 only; the container built Debug and RelWithDebInfo in a single build.
- The raw Linux copy under `raw/linux/` omits `dpkg-list.txt` and the host manifests (large, not needed for the verdict); they remain in the scratch directory outside the checkout.
