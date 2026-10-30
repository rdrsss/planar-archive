# V5-02-X08-C01-R7 gate report (2026-10-29)

Overall result: PASS (passed=true). Every command met its criterion. The gate runner edited no source file. This is the light gate agreed in the dispatch, because no C++, CMake or tests/acceptance file changed since the R6 gate (see identity). Raw logs of this run are under `raw/` with the prefix `gate-`; the coder's earlier `lint/selftest/validate` logs in the same directory were not touched.

## Candidate identity (`raw/gate-identity.txt`, identity.py, pathspec src CMakeLists.txt tests/acceptance)

- HEAD `48b8e526764a76de1392d397bb21659bf63e8b8d`
- sha256 of `git diff --binary HEAD -- src CMakeLists.txt tests/acceptance` (tracked only): `98c72551a86730f93f5c8b23464ef4c885f382464efd7d6aed4847c6cea88571`
- combined sha256 (diff plus untracked bytes): `92891b72bf602b03cddaf44f85d8aef357fbf86d5d31d5a14134f1b8509c494b`
- sha256 of the whole-tree `git diff --binary HEAD` (information only, includes docs): `f25cf40ca101f4f215ffe05d3588b26b1d2b963cffdcbd93d09199873d4eb925`

Both figures equal the identity recorded in the R7 dispatch. Per-file hashes of the five C++ files equal the R6 gate run 2 identity (`../v5-02-x08-c01-r6-evidence/raw/run2/identity.txt`), for example external_registration.t.cpp `22afc6e0...`, host_path.cpp `9207c2b7...`. Modification times of CMakeLists.txt and all five x08_*.py files predate the R6 gate run 2 ctest log. Hence the R6 gate logs remain the final test logs for the product.

## Commands

| command | exit | result |
|---|---|---|
| identity.py src CMakeLists.txt tests/acceptance (`gate-identity`) | 0 | as above |
| python3 scripts/rewrite-check.py lint --build-dir build/rewrite-debug (`gate-lint`) | 0 | clang-format clean, Doxygen checked 147 active source files. Doxygen died with the known SIGBUS flake on attempt 1 and succeeded on attempt 2 of 3 (the script's retry, documented in docs/toolchain-parity.md) |
| validate.py --self-test (`gate-selftest`) | 0 | validator_selftest passed |
| validate.py --evidence tests/native/v5-02-x08-evidence --repo . (`gate-validate`) | 0 | G01 to G09 PASS; G05 "1260 tests, 0 failed, 0 skipped; inventory equal to macOS release" |
| cmake --build build/rewrite-debug --target all planar_tests (`gate-build-debug`) | 0 | ninja: no work to do (already built) |
| same again (`gate-noop-debug`), after the successful build | 0 | ninja: no work to do, zero steps. Pass |
| ctest -N (`gate-ctest-N`) | 0 | Total Tests: 1260 |
| ctest -j4 --output-on-failure, macOS Debug (`gate-ctest-debug`) | 0 | 100% tests passed out of 1260; 0 failed; no skipped or not-run lines; labels unit 1208 and acceptance 52 |
| redact-local-paths.py --repo-path <repo> --home <home> check on the 25 explicit files (`gate-redact-files2.txt`: the 21 git-status files plus the 4 files in tests/native/v5-02-x08-support; `gate-redact`) | 0 | no findings |
| same check on the two evidence REPORT.md files named by finding R6 (`gate-redact-reports`) | 0 | no findings |
| redact-local-paths.py check with no file arguments, first without then with required flags (`gate-redact-noargs`) | 2, then 1 | the first is a usage error; the second scans ignored build output and .claude/settings.local.json, which is not a to-be-committed file. Not a criterion |

The full gate (Release build and ctest, Linux container) was not run: the dispatch's condition for it (a C++, CMake or acceptance change) did not occur. This report file was written after the redaction check; it contains no local home path (the paths above are repository-relative).

## Inventory

`inventory.txt` is the `ctest -N` name list (1260 names) of this run. Compared with the R6 gate run 2 sorted name list, `../v5-02-x08-c01-r6-evidence/raw/run2/inv-debug-sorted.txt` (1260 names), after stripping ordinals and sorting: identical. No name removed, renamed or added in R7. (Against the C01 list `../v5-02-x08-c01-evidence/inventory.txt`, 1258 names, R6 added two names, see the R6 report.)

## (a) Acceptance criteria and findings to evidence

R7 is a records and validator round; it adds no test. The mapping is to evidence, not to new test names.

| criterion or finding | evidence (file:line where applicable) | status |
|---|---|---|
| R1 validator tree is the final R6 logs, total 1260 | validate.py G01 G02 G04 G05 PASS (`raw/gate-validate.log`); self-test literal 1260 (`raw/gate-selftest.log`) | met |
| R2 G06i row and real pass_line check | G06 PASS "9 platform rows"; audited branch test `X08 platform branches are exactly the audited set` at src/cmd/planar/bootstrap.t.cpp:3121; branch at src/engine/runtime/external_registration.t.cpp:4557; self-test mutants refused by G06 | met |
| R3 I16b prober re-run | done by the mutation prober (`probes/PROBES.md`): `R7-I16b` KILLED by `profile capture module ceiling checked before observed module read` (src/engine/capture/profile.t.cpp:368); `R6-I16b-both` remains a survivor (redundant `runtime.cpp` guard) | met (closed by the prober, not by this gate) |
| R4 docs/testing.md per-platform pins | validate G08 PASS (anchors); content not otherwise re-reviewed by the gate | met per validator; reviewer to confirm prose |
| R5 roadmap opening | validate G08 PASS (roadmap opening) | met per validator |
| R6 home path redaction and count 23 | gate-redact-reports exit 0 | met (path check); count of 23 is in the R6 report, not re-derived here |
| R7 numbering and CAS history | validate G09 PASS (stale text absent); wording is in the round record | met per validator |
| R8 INVARIANTS and re-measure rule | plan/INVARIANTS.md exists; G08 testing gate PASS | met per validator |
| Acceptance: both validator commands exit 0 over 1260 names | exits 0 and 0 | met |
| I17 no existing test renamed or removed | inventory identical to R6 list | met |

## (b) Red-first

No regression test was added or changed in R7 (no C++ or acceptance file changed), so there is no red snapshot and no red-first command to list. The R6 tests' red evidence is the prober's mutants (`../v5-02-x08-c01-r6-evidence/probes/PROBES.md`); the round record states they have no pre-fix snapshot hash because the R6 repair was test-only.

## (c) Deliberate behaviour differences

None. No product behaviour changed in R7. Diagnostics are byte-identical (no C++ edit).

## Limitations

- R3 (I16b) is closed by the prober's KILLED result for `R7-I16b`; `R6-I16b-both` stays a survivor because of the `runtime.cpp` guard.
- Release and Linux were not re-run; they rely on the R6 gate run 2 logs at an unchanged C++, CMake and acceptance identity.
- validator output passing G08 and G09 shows anchors and absence of stale text, not prose quality.
- The Doxygen SIGBUS flake occurred once and the script's retry passed.

## Re-run confirmation (second gate run, raw/g2-*)

The light gate was run again serially with no source edits. Identity output (`raw/g2-identity.txt`) is byte-identical to `raw/gate-identity.txt`. Exit codes: identity 0, lint 0 (147 files), validate self-test 0, validate evidence 0 (G01 to G09 PASS, 1260 tests, 0 failed, 0 skipped), build 0, immediate no-change build 0 with "ninja: no work to do" (zero steps), redaction check on the explicit file list (`raw/g2-redact-files.txt`, including this report) exit recorded in `raw/g2-redact.exit`, `ctest -N` total in `raw/g2-ctest-N.log`. No C++, CMake or acceptance file changed, so the full gate was not re-run.
