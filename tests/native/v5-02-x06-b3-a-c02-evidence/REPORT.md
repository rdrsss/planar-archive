# X06-B3-A-C02 gate report (gate runner)

Platform: macOS Debug only (`build/rewrite-debug`). No RelWithDebInfo, Linux, RSS or
whole-invocation budget claim. The gate runner edited no source file. The coder's
earlier report is preserved unchanged as `CODER-REPORT.md`; its other artifacts
(`probes/`, `c02-vs-c01.diff`, `audits.txt`, `NOTES-C01-r2-r3-correction.md`, etc.) are the coder's.
Raw logs from this gate run are only under `raw/`.

## Candidate identity

- HEAD `d783c0b9514c50c8a2ad060281c4e75400d5b2dd` on `dev/planar-v2-vernacular`; tree uncommitted.
- `git diff` (whole tree, includes the coordinator's `roadmap.md`): sha256
  `d99f01da977265ea97a12084292aeff1d169da422b04bd3bb286caa0abd8ed75`.
- `git diff HEAD -- src`: sha256 `76d1b84fdd1c2a6ec09705b20aaad5ae3ce5ba94e006475c2d6c8a19f37d68f6`,
  equal to the C02 candidate hash recorded in the round record (C01 was `84c68c2e...`).

## Commands (serial, unpiped, exit codes read directly)

| # | Command | Exit | Result | Log |
| - | --- | - | --- | --- |
| 1a | `cmake --build build/rewrite-debug --target planar_tests` | 0 | success | `raw/build-tests.log`, `build-tests.exit` |
| 1b | `ctest --test-dir build/rewrite-debug --no-tests=error --output-on-failure` | 0 | "100% tests passed out of 963"; matched 963; 0 failed; no skipped or disabled tests (the only grep hit for "skipped" is the test name `command load refuses forged family order skipped duplicate...`, which passed); total 330.10 s | `raw/ctest.log`, `ctest.exit` |
| 2 | `python3 scripts/rewrite-check.py lint --build-dir build/rewrite-debug` | 0 | "Formatting and Doxygen checked 136 active source files" | `raw/lint.log`, `lint.exit` |
| 3 | `cmake --build build/rewrite-debug` (first) | 0 | successful build; ninja reported "no work to do" because the planar_tests step and the coder's earlier builds had already built everything | `raw/build-all-1.log` |
| 4 | `cmake --build build/rewrite-debug` (second, no-change check) | 0 | `[0/2] Re-checking globbed directories...` then `ninja: no work to do.` (zero steps) | `raw/build-all-2.log` |

All criteria met; passed=true. The ctest command was not run with `-j` (serial).
Lint was run after ctest and before the builds, as ordered.

## Inventory

Registered tests from `ctest -N`: 963 (`raw/ctest-N.log`), sorted names in `inventory.txt`.
Compared against `tests/native/v5-02-x06-b3-a-c01-evidence/inventory.txt` (961 names, the
reviewed C01 candidate inventory; the most recent inventory in earlier evidence) and
`tests/native/v5-02-x06-b3-a-evidence/inventory.txt` (957 names, B3-A).

- Removed versus C01: none. Removed versus B3-A: none.
- New versus C01 (2): `source capture N1 unreadable declaration is not reported as missing`
  (restored, R2) and `X06 B3 A registration count stays one across into_extension` (R1).
  961 + 2 = 963. The dispatch's expected 962 did not count the restored name, which was
  absent from the C01 inventory.
- New versus B3-A (6), all C01 or C02 additions: `capture admission reports only final-component ENOENT as absence`,
  `source capture absence is a signal from admission, not a message match`,
  `X06 B3 A declaration in an unsearchable directory refuses as open, not inspect`,
  `X06 B3 A factories capture exactly once and adoption reads nothing`,
  `X06 B3 A factory registration counters are deterministic across factories`,
  `X06 B3 A registration count stays one across into_extension`.
- All 957 B3-A names and all 961 C01 names are present (acceptance criterion 4).

## (a) Acceptance criteria and findings to tests

| Criterion / finding | Tests (file:line) | Notes |
| --- | --- | --- |
| 1. Per-owner registration count, exposed via `profile_invocation_observation`, one per factory and none extra across `into_extension` (X06B3AC01-R1) | `X06 B3 A factory registration counters are deterministic across factories`, `src/engine/runtime/runtime.t.cpp:1239` (asserts at 1253 known, 1254 opaque, 1255 repeated known); `X06 B3 A registration count stays one across into_extension`, `runtime.t.cpp:1257` (asserts at 1260, 1263 known; 1267, 1270 opaque) | Product code: `profile_capture::note_registration()` / `registration_count()` in `src/engine/capture/profile.cpp:143-148`, counter at `profile.cpp:38`, incremented in `register_vm` (runtime.cpp), surfaced as `observation.registrations`. |
| 2. Three double-registration mutants fail; real candidate passes | Same assertions as criterion 1; mutant logs `probes/run-opaque.log`, `run-known.log`, `run-both.log`; green: this gate's `raw/ctest.log` and `probes/baseline.log` | See section (b). |
| 3. Misleading comment at `runtime.t.cpp:1237` and NOTES claim corrected | Comment now at `runtime.t.cpp:1240-1241` above test at 1239; correction in `NOTES-C01-r2-r3-correction.md` (reviewed C01 NOTES untouched) | Documentation only; no test. |
| 4. Names preserved incl. restored name (X06B3AC01-R2) | `source capture N1 unreadable declaration is not reported as missing`, `src/engine/acceptance/source_capture.t.cpp:587` | Inventory comparison above. |
| 5. Admission messages byte-identical, absence behaviour unchanged, no CLI change, no cycle, no runtime assert | `capture admission reports only final-component ENOENT as absence` `src/engine/capture/admission.t.cpp:847`; `source capture absence is a signal from admission, not a message match` `source_capture.t.cpp:517`; `X06 B3 A declaration in an unsearchable directory refuses as open, not inspect` `runtime.t.cpp:1172`; `source capture N1 ...` `source_capture.t.cpp:587` | Source audits (admission.cpp and source_capture.cpp hashes equal to C01 review, CMake sections identical, no added `assert(`) are the coder's `audits.txt`; not re-run by me beyond the unchanged `git diff HEAD -- src` hash. |
| 6. Rest of C01 preserved, diff touches only listed files | No test; `c02-vs-c01-files.txt` and `c02-vs-c01.diff` (seven files) are the coder's | Not independently re-derived by me. |
| Optional N2 (`SUCCEED` instead of `SKIP`) | `runtime.t.cpp:1174` inside test at 1172 | Applied. |
| Optional N3 (`last_refusal_was_absent` documentation) | `src/engine/capture/admission.cppm` near line 97; doc only, gated by the lint/Doxygen pass | Applied. |
| N1 (embedded versus known message parity) | none | Deferred to X07 per round record. |

## (b) Red-first

The base for the mutants is the C02 candidate itself (`git diff HEAD -- src` sha256
`76d1b84f...68f6`, applied with `git apply candidate-c02.patch` in a scratch worktree at
d783c0b9, outside the shared checkout and `build/rewrite-debug`). The mutation tool is
`probes/mutate.py` (byte copy of the reviewer's C01 script). Commands, as recorded in
`probes/INDEX.txt`: configure with `cmake -S wt -B build -G Ninja -DCMAKE_TOOLCHAIN_FILE=wt/cmake/llvm-toolchain.cmake -DCMAKE_BUILD_TYPE=Debug -DPLANAR_WARNINGS_AS_ERRORS=ON`;
per mode mutate, build `planar_runtime_tests` (rc 0), run the `[x06b3a]` tag (rc 42), restore.
The exact test-runner invocation line is not in the evidence beyond "run `[x06b3a]`"; the
run logs hold the output. I did not rerun the mutants myself.

| Regression test | Mutant | Mutant file hash (runtime.cpp, sha256) | Real red or guard | Failing line(s) and text |
| --- | --- | --- | --- | --- |
| registration count tests | opaque double `register_vm` | `4d67e5e1024d26e662a7174efec3ea491c9a54978fb96a85c1439d492909d857` | Real red: 3 failed of 312 assertions | `runtime.t.cpp:1254`, `1267`, `1270`: `CHECK( opaque->observe().registrations == 1 )` with `2 == 1` |
| same | known double `register_vm` | `5f93f71afcfb514a6fb2e72350f8d9975caed65e408ce1b4e713b34ba959c761` | Real red: 4 failed | `runtime.t.cpp:1253`, `1255`, `1260`, `1263`: `known/again/owner ... registrations == 1`, `2 == 1` |
| same | both | `b86983276b183dd12391a880f131bf77b0bc77b7c6ff033da67aefebdbf2afdc` | Real red: 7 failed | lines 1253, 1254, 1255, 1260, 1263, 1267, 1270 (same text) |

Unmutated candidate runtime.cpp sha256 `7393ba852aa56ca831d22dbfd13f2025736266910cb24cab1abdeb06babf4976`;
restored files equal it in all three modes (`equal_to_saved=yes`). Baseline green: 312 assertions / 17 cases.

Missing or limited evidence:
- The new tests were not shown red against the C01 source by compiling them there: the
  `registrations` observable does not exist at C01, so they cannot compile; the red is the mutant evidence above.
  The C01 review's own probes show the same three mutants surviving at C01 (294 assertions / 16 cases passed),
  in `tests/native/v5-02-x06-b3-a-c01-review-evidence/probes/`.
- The restored name (R2) is a rename, not a behaviour regression test; no red exists or applies.
  It is a guard that the name is present, verified by the inventory diff.
- No separate red-snapshot hash for a pre-fix `git diff HEAD` exists in the C02 evidence; the mutant hashes above are the only red identities.

## (c) Deliberate behaviour differences

- Product behaviour change: only the observable. `profile_capture` gains a registration counter
  (`note_registration` / `registration_count`) and `profile_invocation_observation` gains `registrations`.
  Permitted by acceptance criterion 1 and the "any product behaviour change beyond the observable" exclusion in the round record.
- Test count 963 versus the dispatch's expected 962: explained above (restored name was not in the C01 inventory).
- Optional N2 and N3 applied; permitted by the round's "optional if trivial" clause.
- Gate used serial ctest; the coder's `ctest-full.log` used `-j4`. Results agree (963/963).

## Limitations

macOS Debug only; no RelWithDebInfo, Linux, RSS or shared-budget claim. The first full build did no
work because the test-target build had already built it. Mutant probes and source audits are the
coder's artifacts, cited not rerun. N1 deferred to X07; B3-A N2 to N4 remain for B3-B and X07.
