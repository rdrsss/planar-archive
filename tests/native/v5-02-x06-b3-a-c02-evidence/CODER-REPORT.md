# X06-B3-A-C02 evidence (coder P3)

Platform: macOS Debug only. Candidate = `git diff HEAD -- src` at HEAD `d783c0b9`,
sha256 `76d1b84fdd1c2a6ec09705b20aaad5ae3ce5ba94e006475c2d6c8a19f37d68f6`
(`candidate-c02.patch`; C01 candidate was `84c68c2e...f2b6`). C02 versus C01
(`c02-vs-c01.diff`, sha256 `a708f93d2f3e02ca76c9e551adb8dca8157c9c6ef0b950d5b402a9b7ffa30d39`)
touches exactly seven files: runtime.cppm, runtime.cpp, runtime.t.cpp, profile.cppm,
profile.cpp, admission.cppm (doc only), source_capture.t.cpp (`c02-vs-c01-files.txt`).

## Criteria

| # | Criterion | Evidence |
| - | --- | --- |
| 1 | Per-owner registration count, exposed via `profile_invocation_observation`, tests assert one per factory and none extra across `into_extension` | `profile_capture::note_registration()/registration_count()` (profile.cpp), `note_registration()` called in `register_vm` (runtime.cpp), `observation.registrations`; tests: factory counters case (known, opaque, repeat each == 1) and new case "X06 B3 A registration count stays one across into_extension". See `c02-vs-c01.diff`; `probes/baseline.log` 312 assertions / 17 cases green. |
| 2 | Three double-registration mutants red; real candidate green | `probes/INDEX.txt`, `probes/run-{opaque,known,both}.log`. opaque: 3 failed assertions (312, 309 passed); known: 4 failed (308 passed); both: 7 failed (305 passed); each rc 42, failing text `CHECK( x.observe().registrations == 1 )` with `2 == 1`. Mutant sha256: opaque `4d67e5e1...d857`, known `5f93f71a...c761`, both `b8698327...afdc`. Restored runtime.cpp sha256 `7393ba85...4976` equals the saved copy and the shared checkout file in all three. Green: baseline.log, ctest-full.log. |
| 3 | `runtime.t.cpp:1237` comment and NOTES claim corrected | Comment rewritten in runtime.t.cpp (hunk in `c02-vs-c01.diff`); `NOTES-C01-r2-r3-correction.md` (reviewed C01 NOTES untouched). |
| 4 | All 957 B3-A, 961 C01 names and restored name present | `inventory.txt` (963 names from `ctest -N`, `ctest-N.log`). Missing from current: 0 for both baselines (`removed-names.txt` empty). `new-names.txt` vs C01: the restored `replacement.source capture N1 unreadable declaration is not reported as missing` and `replacement.X06 B3 A registration count stays one across into_extension`. Total 963 = 961 + 2 (the task's expected 962 did not count the restored name as new; it was absent from the C01 inventory). |
| 5 | Admission messages byte-identical, absence behaviour unchanged, no CLI change, no new cycle, no runtime assert | `audits.txt`: admission.cpp `7434866d...3299` and source_capture.cpp `a3e671cd...e5de` equal the C01 review hashes; CMake sections of the C02 and C01 patches identical; no `assert(` added outside `static_assert`; only C01's pre-existing CMakeLists edits touch `src/cmd`, no handler change. |
| 6 | Rest of C01 preserved, diff touches only listed files | `c02-vs-c01-files.txt`, `c02-vs-c01.diff` (7 files). N2 (SUCCEED instead of SKIP at the root branch) and N3 (`last_refusal_was_absent` doc) applied. |

## Gates (shared build/rewrite-debug)

- `cmake --build build/rewrite-debug` rc 0 (`build-all-1.log`), `--target planar_tests` rc 0 (`build-tests.log`).
- Full Debug `ctest --no-tests=error -j4`: 100% passed, 963/963, rc 0 (`ctest-full.log`); zero failed; no skipped or disabled tests (the only grep hits for "skipped/failed/disabled" are test names).
- `python3 scripts/rewrite-check.py lint --build-dir build/rewrite-debug`: rc 0, "Formatting and Doxygen checked 136 active source files" (`lint.log`, same command as the C01 lint).
- Second `cmake --build build/rewrite-debug` and `--target planar_tests`: `ninja: no work to do.` (`build-all-2.log`, `build-tests-2.log`).

## Limitations

macOS Debug only; no RelWithDebInfo, Linux, RSS or whole-invocation budget claim.
N1 (embedded versus known message parity for unsearchable directories) is deferred to X07;
N2 and N3 were applied as optional trivial items; B3-A N2 to N4 remain for B3-B and X07.
Mutation probes ran in a scratch worktree/build (removed afterwards); the shared checkout was
never mutated. The gate ran with `ctest -j4`.
