# R20-CARRY-A final gate report (run1)

Final gate on the final candidate, run 2026-11-13 to 2026-11-14 by the gate runner. No source file was edited by the runner. The coder's own report is kept as `CODER-REPORT.md` (local only, not committed). The earlier preliminary report (prelim-3) is preserved at `raw/run1/REPORT-prelim-3-copy.md`. All raw logs are under `raw/run1/` (driver `raw/run1/driver.sh`; `.cmd`, `.log`, `.exit` per step; `ALLDONE` marker). Nothing was piped through tail or head; exit codes are read from the `.exit` files.

Verdict: every command met its criterion, with one deviation to the support-suite step (below) and one round acceptance criterion that is NOT met (loaded-host margin, see Limitations). `passed` is therefore true for the commands and the gate, but the reviewer must weigh the unmet loaded-margin criterion.

This report stays the gate run1 of the R20-CARRY-A candidate, with its identity below. The identity and gate results of the R20-CARRY-A-C01 candidate (the corrective round that raised the limits to 1680 and 840) are in `tests/native/r20-carry-a-c01-evidence/REPORT.md`, not here.

## Candidate identity (identity tool output, verbatim)

From `raw/run1/identity-before.txt` (equal to `identity-after.txt`; `diff` empty):

| identity | value |
|---|---|
| HEAD | `dec6f178daf10e2a1219ae56dc71485d42d839ca` |
| sha256(git diff --binary HEAD -- src tests/native), tracked only | `9a891be59df3f260098733b10c70b240542b09fa3b05f923f6dfb3bf66635355` |
| combined sha256, pathspec src tests/native, evidence excluded | `c80c07a863fbca7c2cc0cae22d11356f61791d3881cc0c0dd3b6b4120428c253` |

Wider pathspec (`src cmake scripts CMakeLists.txt CMakePresets.json tests/native`), `raw/run1/identity-src-before.txt` (equal to `identity-src-after.txt`), which also gives the per-file sha256 table:

| identity | value |
|---|---|
| HEAD | `dec6f178daf10e2a1219ae56dc71485d42d839ca` |
| sha256(git diff --binary HEAD -- src cmake scripts CMakeLists.txt CMakePresets.json tests/native), tracked only | `37067b8522d6519365dba54e2327b9bb33decdf5530fc045f21053c058e9fc3c` |
| combined sha256 (with untracked bytes) | `2f1890f54b5619a866e58e3a0895da72bd5c35ee7332af565c14289c44249ab8` |

The candidate is identical to the one measured by prelim-3 (same src/tests-native hashes). Changed product-tree files under `src/` are four `CMakeLists.txt` files only (test timeouts and test registration); no non-test product C++ or Lua changed. Because `src/`, `cmake/` and tests changed, the full gate was required and was run once.

## Commands

Host load average was about 13 at start; 46 GiB free on /System/Volumes/Data (above the 15 GiB floor).

| # | command | exit | result |
|---|---|---|---|
| 1 | `cmake --preset debug` | 0 | `01-configure-debug.log` |
| 2 | `python3 scripts/rewrite-check.py gate --build-dir build/rewrite-debug` | 0 | first run, no rerun needed. 100 percent, 1872 of 1872 passed, 0 failed, total test time 1460.95 s; "Formatting and Doxygen checked 251 active source files"; final step "ninja: no work to do." `02-gate-debug.log` |
| 3 | `cmake --build build/rewrite-debug` (separate build after the gate) | 0 | `03-build-debug.log` |
| 4 | `cmake --build build/rewrite-debug` (no-change check) | 0 | "[0/2] Re-checking globbed directories... ninja: no work to do." Zero steps. `04-nowork-debug.log` |
| 5 | `ctest --test-dir build/rewrite-debug -R "v2 every native allocation failure\|recorded effective history selects wider fixed tree profile"` | 0 | 2 matched, 2 passed, 0 skipped: #442 22.28 s, #658 10.92 s, both against a 120 s limit. `05-focused.log` |
| 6 | `ctest --test-dir build/rewrite-debug -R replacement.r20` (the new support tests) | 0 | 18 matched, 18 passed, 0 skipped, 11.87 s. `06-support-r20.log` |
| 7 | `cmake --preset release` | 0 | `07-configure-release.log` |
| 8 | `python3 scripts/rewrite-check.py gate --build-dir build/rewrite-release` | 0 | 100 percent, 1872 of 1872 passed, total 510.61 s; format and Doxygen checked 251 files; "ninja: no work to do." `08-gate-release.log` |
| 9 | `cmake --build build/rewrite-release` | 0 | `09-build-release.log` |
| 10 | `cmake --build build/rewrite-release` (no-change check) | 0 | "ninja: no work to do." Zero steps. `10-nowork-release.log` |
| 11 | six Python support suites under `tests/native/r19-accept-support` (`python3 -B -m unittest discover -s . -p '*_test.py' -v`, `PYTHONDONTWRITEBYTECODE=1`, scratch HOME and PLANAR_DB, `PLANAR_CANDIDATE=<repo>/build/rewrite-debug/src/cmd/planar/planar_authoring_stack`) | 1 | Ran 198 tests, FAILED (failures=2). Both failures are `AcceptError: candidate must not live in a shared build/rewrite-* directory` from `accept_lib.candidate_record` (checkpoint_test `test_checkpoint_command_writes_result_and_path_free_readback`, provenance_test `test_provenance_command_writes_result_json_with_both_readbacks`). That is the suites' own guard rejecting the path the task prescribed, not a product failure. `11-py-suites.log` (kept) |
| 11b | same command, with `PLANAR_CANDIDATE` pointing at a byte copy of the same binary at `<scratchpad>/r20cand/planar_authoring_stack` (outside `build/rewrite-*`), scratch HOME and PLANAR_DB | 0 | Ran 198 tests, OK, 0 failures, 0 errors, 0 skips (grep hits for "skipped" are test names). `11b-py-suites-copied-candidate.log`, `.cmd`, `.exit` |
| 12 | `python3 -B scripts/redact-local-paths.py --repo-path <repo> --home <home> check <files>` on the 26 files the round edits or adds (git diff names plus untracked, evidence excluded; `redact-files.txt`) | 0 | no output, no leak. Run without file arguments the tool only prints usage, so it was not run that way. `12-redact.log` |
| 13 | `scripts/linux-gate.sh <scratchpad>/lg-r20-run1` (Linux aarch64 Docker, new out-dir) | 0 | all inner exits 0: Debug ctest 100 percent of 1872 (1505.54 s), Release ctest 100 percent of 1872 (633.33 s), Debug no-work "ninja: no work to do.", Release no-work 0; manifest comparisons "equal" (host before vs after build, host vs container context). Linux inventory has the same 1872 names as macOS Debug. Logs copied to `raw/run1/linux/`. Disk before 46 GiB free. |
| 14, 15 | `docker builder prune -af --filter 'description~=home/planar'` twice | 0, 0 | both "Total: 0B". df before 46 GiB used 365 GiB; after 46 GiB used 366 GiB (`linux-df-before.txt`, `linux-df-after.txt`) |
| 16 | `ctest -N` before and after | 0 | 1872 and 1872; name diff empty (`ctest-N-before.txt`, `ctest-N-after.txt`; Release list `ctest-N-release.txt` also 1872) |

Scratch-copy configure-twice then ninja no-work is covered by the registered tests `replacement.r20.configure_zero_work`, `replacement.r20.embed_unchanged_mtime` and the four generator and manifest unchanged-mtime tests, all passed (command 6), and by the Debug and Release no-change builds above. Debug gate had no timeout failure, so no rerun was needed. The 198-test count in command 11b is a nonzero count with zero skips.

## Test inventory

Full list: `inventory.txt` (1872 tests, copy of `raw/run1/ctest-N-after.txt`, sha256 `f1040d06d7ee43c1b8654ca3ca3133f403bb0d7715f49042b9ee882bed324efd` of the file as produced by `shasum`). The most recent earlier list kept in evidence is the coder's pre-change baseline `raw/ctest-names-before.txt` (1854 names, sha256 `c63b8c780cd50c377e38e37d06793b9317b3e1558fe5b14cd193bc969ee5dd48`, the R19 inventory at the starting revision). Comparison with numbers stripped and names sorted: 1854 against 1872.

- Added (18, all `replacement.r20.*`): configure_rebuilds_on_content_change, configure_zero_work, embed_callsite_audit, embed_content_change_rewrites, embed_unchanged_mtime, generator_content_change_rewrites_b1, generator_content_change_rewrites_d6, generator_content_change_rewrites_d6_c01, generator_content_change_rewrites_x05_a0, generator_unchanged_mtime_b1, generator_unchanged_mtime_d6, generator_unchanged_mtime_d6_c01, generator_unchanged_mtime_x05_a0, manifest_content_change_rewrites, manifest_unchanged_mtime, timeout_headroom_above4096, timeout_headroom_v2_allocation, timeout_names_registered.
- Removed or renamed: none.

## Acceptance criteria to tests

Registration lines are in `src/engine/runtime/CMakeLists.txt`; implementations in `tests/native/r20-carry-a-support/` (`timeout_headroom.py`, `generator_stability.py`, `embed_stability.py`, `configure_ninja.py`, `common.py`). No corrective-round findings are in scope for this round.

| criterion | test names | file:line |
|---|---|---|
| The two named tests keep names and assertions, and gain a limit of 120 s (four-times margin over idle) | `replacement.v2 every native allocation failure remains a redacted typed resource refusal`; `replacement.recorded effective history selects wider fixed tree profile for valid closure above4096 files`; guards `replacement.r20.timeout_headroom_v2_allocation`, `replacement.r20.timeout_headroom_above4096`, `replacement.r20.timeout_names_registered` | `src/engine/history/interpretation_v2_codec.t.cpp:495` with `src/engine/history/CMakeLists.txt:83`; `src/engine/ledger/history_verification.t.cpp:1325` with `src/engine/ledger/CMakeLists.txt:37`; guards `src/engine/runtime/CMakeLists.txt:287`, `:290`, `:293` |
| Both named tests pass under a much heavier load than the old failure | NOT MET. Loaded direct run passes (813 s and 406 s) but the registered 120 s limit times out under load 345-400. See Limitations | `CODER-REPORT.md` section 7, `raw/load/` |
| Name list differs only by additions | ctest -N diff | `raw/ctest-names-diff.txt`; this report's inventory section |
| Unchanged configure leaves generated mtimes untouched and the next build does zero work | `replacement.r20.generator_unchanged_mtime_b1`, `_d6`, `_d6_c01`, `_x05_a0`; `replacement.r20.manifest_unchanged_mtime`; `replacement.r20.embed_unchanged_mtime`; `replacement.r20.configure_zero_work` | `src/engine/runtime/CMakeLists.txt:296`, `:299`, `:302`, `:305`, `:320`, `:332`, `:326` |
| A content change still rewrites (test can fail both ways) | `replacement.r20.generator_content_change_rewrites_b1`, `_d6`, `_d6_c01`, `_x05_a0`; `replacement.r20.manifest_content_change_rewrites`; `replacement.r20.embed_content_change_rewrites`; `replacement.r20.configure_rebuilds_on_content_change` | `src/engine/runtime/CMakeLists.txt:308`, `:311`, `:314`, `:317`, `:323`, `:335`, `:329` |
| Every embed call site covered (other equally unconditional generators found) | `replacement.r20.embed_callsite_audit` | `src/engine/runtime/CMakeLists.txt:338` |
| Each new test has a stated mutant | (prober applies mutants; none applied by the runner) | `CODER-REPORT.md` section 9 |
| Production src unchanged beyond tests, shown by identity | identity section above | four `CMakeLists.txt` under `src/` only |
| Full gate on the final candidate: Debug, Release, Linux aarch64, 100 percent, zero skips, nonzero, no-change build no work, format, Doxygen | commands 2, 4, 8, 10, 13 | `raw/run1/` |
| Candidate identity only from the identity tool | identity section | `raw/run1/identity-*.txt` |

## Red-first

Sources: `red/RED.md`, `red/raw/*.log` and `CODER-REPORT.md` section 2. Red-time snapshot: `red/snapshot-2`, `candidate.diff` sha256 `136cb6973278e9cb8d86892c39eaa182c8ae49eeba680c6343123804c9b9be6e`, SHA256SUMS sha256 `108d7585cfdb577ea0722b6ca34cb0c6c5b398d6ecc9b4e6c5948018d34e9b64`, HEAD `dec6f178`. The coder's separate pre-fix identity (`raw/redfirst/identity-prefix.txt`) has combined sha256 `14ca21adabf61562a195bef78d0e188058cac8165f2e16587a899d64674657d9`. The earlier `red/snapshot` was incomplete and is superseded. The tree was built with `cmake --build build/rewrite-debug --target planar_tests` (rc 0, `red/raw/build.log`); each support case was then run and its output is in `red/raw/<case>.log`. The exact per-case invocation is the one recorded in `red/RED.md` and `CODER-REPORT.md` section 2; the runner did not reconstruct any command not recorded there.

| test (replacement.r20.*) | red or guard | failing line |
|---|---|---|
| generator_unchanged_mtime_b1 | real red | AssertionError (rewrote 63 of 63 files) |
| generator_unchanged_mtime_d6 | real red | AssertionError (18 of 18) |
| generator_unchanged_mtime_d6_c01 | real red | AssertionError (3 of 3) |
| generator_unchanged_mtime_x05_a0 | real red | AssertionError (7 of 7) |
| manifest_unchanged_mtime | real red | AssertionError |
| embed_unchanged_mtime | real red | AssertionError (module mtime moved) |
| embed_callsite_audit | real red | AssertionError |
| configure_zero_work | real red | AssertionError (ninja did work, "[1/1] Generating stamp") |
| timeout_headroom_v2_allocation | real red | AssertionError (limit not raised) |
| timeout_headroom_above4096 | real red | AssertionError (limit not raised) |
| generator_content_change_rewrites_b1, _d6, _d6_c01, _x05_a0; manifest_content_change_rewrites; embed_content_change_rewrites; configure_rebuilds_on_content_change; timeout_names_registered | guard, already passed at red | none; they assert behaviour that must stay true |

The two timeout-limit changes have no deterministic behavioural red beyond the headroom checks; the idle and loaded measurements stand instead (CODER-REPORT section 7). The exact failing line of each case is in its `red/raw/<case>.log`.

## Deliberate behaviour differences

None in production code. Per-test `TEST_TIMEOUT 120` on two test targets is a verification setting permitted by the round record (the "Objective" item 1 allows a per-target TEST_TIMEOUT through the owning module's CMakeLists). The generators now write only if different, matching the R19-AUTHORING-C01 precedent named in the round record; generated bytes are unchanged (`raw/embed-bytes-diff.txt` empty).

## Limitations

- Round criterion "both named tests pass under a load much heavier than the previous failure" is not met. PKG-E recorded LOAD-MARGIN-FAIL: under K=160 busy loops (host load 345-400) the tests took 813 s and 406 s directly and timed out at 121 s and 122 s under ctest at the 120 s limit. Margin limit/loaded is 0.15 and 0.30 against the required 2. PKG-E proposed 1680 s and 840 s; the checkout still carries 120 s. The idle margin (limit/idle 5.07 and 11.03, measured on a non-idle host) is met. This is for the coordinator to decide.
- Near-limit untouched tests (`replacement.cli` 18.03 s of 30, RA01-B2 C01 16.40 s of 30) remain on 30 s limits (CODER-REPORT section 8, decision D28).
- Command 11 as literally specified failed (2 of 198) because the suites refuse a candidate under `build/rewrite-*`; command 11b with a byte copy of the same binary passed 198 of 198. The gate-runner did not modify the suites.
- Mutants were not applied by the runner; kill claims are the coder's (CODER-REPORT section 9) pending the prober.
- Linux covers aarch64 only. Redaction check ran on explicit files (26) only.
