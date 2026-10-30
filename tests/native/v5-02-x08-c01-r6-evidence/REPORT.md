# V5-02-X08-C01-R6 gate report, run 2 (2026-10-29)

Overall result: PASS (passed=true). Every command met its criterion. The gate runner edited no source file.
This is a fresh full re-run of the gate at an identical candidate identity to the earlier R6 gate run, whose report is kept as `raw/REPORT-previous-gate-run1.md` and whose logs stay in `raw/` (not deleted).
All logs of this run are under `raw/run2/`.

## Candidate identity (`raw/run2/identity.txt`, identity.py output, pathspec `src tests/native`)

HEAD `48b8e526764a76de1392d397bb21659bf63e8b8d`.
sha256 of `git diff --binary HEAD -- src tests/native` (tracked only): `4eca3b80e9b318a64b32064f2758a240c603f8416fe955b15be59aad83467724`.
Combined sha256 (diff plus untracked bytes, evidence excluded): `8177d388939ecd09ee6498002906184dd22f5c5519d13d525ec3c5d0702dedc8`.
sha256 of the whole-tree `git diff --binary HEAD` (all tracked changes, including docs; for information): `0b0047f56ff1f48550a09303548f4d970e96d8e0888cc97f4bb93f1b05e7ad33`.
Per-file hashes are in `raw/run2/identity.txt` and are equal to the earlier gate run and to the prober's identity (`probes/raw/00-identity-full.txt`).

## Commands, exit codes, counts

| command | exit | result (log under `raw/run2/`) |
|---|---|---|
| `cmake --build build/rewrite-debug --target planar_tests` (`build-debug`) | 0 | `ninja: no work to do.` (already built at this identity) |
| `ctest --test-dir build/rewrite-debug --no-tests=error --output-on-failure` (`ctest-debug`) | 0 | 100% passed, 1260 of 1260, 0 failed, 0 skipped |
| `cmake --build build/rewrite-release --target planar_tests` (`build-release`) | 0 | `ninja: no work to do.` |
| `ctest --test-dir build/rewrite-release --no-tests=error --output-on-failure` (`ctest-release`) | 0 | 100% passed, 1260 of 1260, 0 failed, 0 skipped |
| `python3 scripts/rewrite-check.py lint --build-dir build/rewrite-debug` (`lint`) | 0 | clang-format --Werror clean; Doxygen checked 147 active source files |
| no-op debug build x2 (`noop-debug-1`, `-2`) | 0, 0 | `ninja: no work to do.` both (zero steps), after a successful build |
| no-op release build x2 (`noop-release-1`, `-2`) | 0, 0 | `ninja: no work to do.` both |
| `python3 scripts/redact-local-paths.py check` with no file arguments (`redact-noargs.log`) | 2 | usage error (needs `--repo-path` and `--home`); not a result |
| same with `--repo-path <repo> --home <home>` and the explicit files in `redact-files.txt` (tracked-modified plus untracked-unignored, evidence directories excluded; 23 files) (`redact`) | 0 | no findings |
| `df -h /System/Volumes/Data` (`df-before.txt`, `df-after.txt`) | 0 | 107 GiB available before and after (above 15 GiB) |
| `scripts/linux-gate.sh <scratch>/linux-r6b` (copied to `linux/`; `linux-gate.exit`) | 0 | docker build exit 0; host manifest equal before and after and equal to the container context manifest |
| Linux Debug container ctest (`linux/container/debug-ctest.*`) | 0 | 100% passed, 1260 of 1260, 0 skipped; no-work build `ninja: no work to do.` |
| Linux RelWithDebInfo container ctest (`linux/container/release-ctest.*`) | 0 | 100% passed, 1260 of 1260, 0 skipped; no-work build `ninja: no work to do.` |
| `docker builder prune -f` (run by linux-gate.sh, `linux/builder-prune.*`) | 0 | done |
| `python3 tests/native/v5-02-x08-support/validate.py --evidence tests/native/v5-02-x08-evidence --repo .` (record step, `validate`) | 0 | G01 to G09 PASS. G01, G02, G04, G05 report 1258 tests because they check the committed X08 evidence tree, not this run (prober noted the same) |

Matched tests: 1260 in each of macOS Debug, macOS RelWithDebInfo, Linux Debug and Linux RelWithDebInfo. Skipped: zero in all (the only grep hit for the word is a test name,
`replacement.command load refuses forged family order skipped duplicate and malformed arguments`, which Passed). Failures: zero.
Zero-work: four macOS no-op builds and both in-container no-work builds report no work, each after a separate successful build. Docker was run with the PATH export from the brief; no image, container or volume was removed beyond `builder prune -f`.

## Inventory

`inventory.txt` (next to this report) is `ctest -N` of macOS Debug (1260 tests, with ordinals; also `raw/run2/inventory-debug-N.txt`, release `inventory-release-N.txt`);
`inventory-sorted.txt` is the name list sorted with LC_ALL=C (equal to `raw/run2/inv-debug-sorted.txt`). The sorted macOS Release list and both Linux sorted lists are byte-equal to it.
Compared with the most recent committed-directory list, `../v5-02-x08-c01-evidence/inventory.txt` (1258 names): no name removed or renamed; two new names:
- `replacement.X08 C01 native decimal id facts are bounded at INT64_MAX exactly`
- `replacement.X08 C01 role preparation outputs-off boundary pins the request-record conversion charge`

## (a) Acceptance criteria and findings to tests

The round record (`docs/vernacular/rounds/V5-02-X08.md`, "Dispatch: V5-02-X08-C01 gate run 6" and "Gate run 6 repair report") lists the acceptance criteria and the operator decisions (a) and (b). Mapping:

| criterion or finding | tests (file:line) | status |
|---|---|---|
| Run 5 failure and decision (a): `engine/runtime/external_registration.t.cpp` audited with count 1 | `X08 platform branches are exactly the audited set` (src/cmd/planar/bootstrap.t.cpp:3121) | passes on all four; prober kills 4 mutants of it |
| Decision (b) BUD-CONV: fails when the request-record conversion charge is deleted | `X08 C01 role preparation outputs-off boundary pins the request-record conversion charge` (src/engine/runtime/external_registration.t.cpp:4570) | passes; killed by R6-BUD-CONV (macOS and Linux), -half, -up |
| Decision (b) I08, I08b: id one past INT64_MAX and UINT64 range rejected at the exact boundary | `X08 C01 native decimal id facts are bounded at INT64_MAX exactly` (src/engine/runtime/commands.t.cpp:309) | passes; killed by +1, UINT64_MAX, -1, 20-digit length mutants |
| Decision (b) I16b: module just over 1048576 bytes rejected | acceptance test `x08.adv_limits_cli` (tests/acceptance/x08_adversarial.py:201, registered at :226) | passes; killed by lower-bound and both-bounds mutants. The single profile.cpp raise survives as an equivalent mutant (redundant runtime.cpp:3288 guard) |
| Decision (b) I02: duplicate option rejected (Throw not TakeLast) | acceptance test `x08.e02_envelope_rows_binary` extra block (tests/acceptance/x08_cli.py:388, registered at :879) | passes; mutant SURVIVES as equivalent (`classify_prefix` also refuses count != 1). Honest limit, not a closure |
| Decision (b) HP-I3, HP-I24 killed on Linux | `host path maps a regular file followed by parent traversal to the root directory error` (src/engine/capture/host_path.t.cpp:84) | passes on Linux; prober killed both in the container |
| BUD-X12, BUD-EXCL macOS slack closed by the per-platform pin | `X06 D C02 role preparation charges enumeration and registry reads to the factory budget` (external_registration.t.cpp:4579); `X07 N30 role capture charges the factory budget owner` (:5109); fixed-root pin `X08 C01 the C02 budget fixture root has a fixed canonical byte length` (:4598) | pass on all four; prober kills X12, EXCL, I15 (4743), I9 |
| Carried C01 behaviours (file followed by `..`; missing/loop spellings) | `host path sanitizes invalid file traversal and missing parents` (host_path.t.cpp:73); `host path keeps missing and unresolvable directory spellings as input path errors` (:90) | pass |
| Equivalent or redundant, recorded with reasons: HP-I2a, X-I21, BUD-I15b, X-I04 | no test; see `probes/PROBES.md` | recorded |
| No existing test name removed or renamed | inventory diff above | met |
| No diagnostic changed | test-only round; gate does not diff messages, product source diff is limited to the C01 host_path.cpp change | met by scope, not separately diffed |
| macOS Debug and Release, Linux Debug and RelWithDebInfo green, zero skips; lint; two no-work builds; validator exit 0 | table above | met |

## (b) Red-first

This round fixes test-only survivors, so red-first is shown by the prober's mutants (applied only in a scratch copy outside the checkout), not by pre-fix snapshots. No coder red log with a pre-fix snapshot hash exists for the two new R6 tests or the audit row;
that is missing and is not invented. The mutant that makes each test fail, with the mutant hash from `probes/PROBES.md` and logs in `probes/raw/<id>.log`:

| regression test | mutant (hash) | failing observation | real red or guard |
|---|---|---|---|
| `X08 C01 role preparation outputs-off boundary pins the request-record conversion charge` | R6-BUD-CONV `b63ee4bf0ae51d1b` (also -half `a2fbe0c7bd098646`, -up `d7ea75812eeff4b6`; Linux run too) | test fails by name in `probes/raw/R6-BUD-CONV.log` | real (kill by mutant); the test is new so it has no pre-existing green history |
| `X08 C01 native decimal id facts are bounded at INT64_MAX exactly` | R6-DEC-max+1 `9167fefdb18405ef`, R6-DEC-uint64 `db44429dac7c1768`, R6-DEC-max-1 `de20ecf11d0827ff`, R6-DEC-len20 `c7848b32bd096c88` | fails by name | real |
| `X08 platform branches are exactly the audited set` | R6-AUD-extra-branch `15e4a6f4faa05bcd`, row-count `8f1757626f934ec0`, row-missing `619c9fdedccacc15` (the run 5 failure), R6-AUD-HP `83e2dcf2073f8616` | fails by name | real |
| `x08.adv_limits_cli` | R6-I16b-lower `c5e9ec45b75ec5d3`, R6-I16b-both-runtime `0078a8d16fdebd40` | killed by name | real; R6-I16b `9d19b2f7fb2be363` alone survives (equivalent) |
| `x08.e02_envelope_rows_binary` extra block | R6-I02 `9ed3c1170497f7ed` | survived | guard only (honest limit) |
| host path tests | HP-I3 `4217fcc09a011018`, HP-I24 `fdcfd88174ae60f6` (Linux), HP-I1a/I1b/I2b/I25 (Linux) | killed by name | real on Linux |

The commands are the driver commands recorded in each `probes/raw/<id>.log` (patch, targeted `ninja`, `ctest -R <regex> --no-tests=error`); the exact per-row command lines are in those logs and in `probes/tools/driver.py`, `lin_driver.py`.
Earlier-lineage red records (macOS, scratch copy, pre-fix HEAD `host_path.cpp` sha256 `0df99615b5aba1266790891509c675830254bfd63d788132475a2cbe63296ada`, scratch `external_registration.t.cpp` `83112769e13d38f9faaf2ae7c8912ffdfb01cebc28ea998e073f6ec4411d0020`) are in `../v5-02-x08-c01-evidence/red/` (`*.cmd`, `*.log`, `*.exit`, `scratch-snapshots.txt`).

## (c) Deliberate behaviour differences

None in this round (test-only). The earlier C01 behaviour change stands: two macOS mappings in host_path.cpp (`alias/file/..` and `physical/file/../file` now give io "Cannot resolve explicit root directory" instead of path_denied),
permitted by round decision 5 (operator, 2026-10-29) because the closed message table is unchanged. The per-platform pins (macOS 4742 / 228586, Linux 4737 / 228612) are an operator-approved test-only platform branch (decision (a)).

## Limitations

- Debug-only mutants; RelWithDebInfo mutants not run. Linux mutated only BUD-CONV and the HP rows.
- Equivalent survivors I16b (profile.cpp alone), I02 and a looser pin (BUD-I15b) remain, as recorded.
- `validate.py` still counts 1258 names for the committed X08 evidence tree; the candidate has 1260.
- x86_64 Linux not covered (arm64 Docker only). `x07.two_process_cas_one_winner` is known flaky; it passed here.
- `redact-local-paths.py check` without file arguments is unusable; the explicit-file form was run.
- Docker usage: one build, `builder prune -f` run by the script; disk stayed at 107 GiB free.
