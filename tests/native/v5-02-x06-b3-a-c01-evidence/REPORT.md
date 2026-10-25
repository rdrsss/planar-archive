# X06-B3-A-C01 gate report (profile invocation owner correction)

Date 2026-10-24. Gate runner: fresh run of the agreed commands, macOS Debug only. Raw logs from this run are in `raw/`. The coders' earlier evidence (`r1/`, `r2-r3/`, `audits.txt`, `dag-audit.txt`, and the top-level logs from their own run) is preserved unchanged next to it.

## Candidate identity

- HEAD: `d783c0b9514c50c8a2ad060281c4e75400d5b2dd` (dev/planar-v2-vernacular).
- `git diff | shasum -a 256` (tracked files, includes roadmap.md), measured at gate time: `37b200047abf0f6c879af01bc70feab26bf23d588b7920d5cd22589e9cf5835e` (raw/diff-sha256.txt). This equals the final hash the coders recorded in the round record, so the candidate is unchanged since their run.
- Untracked files (three round records, this evidence directory) are not in that hash.

## Gate commands (serial, unpiped, exit codes read directly; raw/exit-codes.txt)

| # | Command | Exit | Result |
|---|---|---|---|
| 1 | `cmake --build build/rewrite-debug --target planar_tests` | 0 | success (raw/1-build-tests.log) |
| 2 | `ctest --test-dir build/rewrite-debug --no-tests=error -j1` | 0 | "100% tests passed out of 961", 331.04 s; matched 961 = registered 961; zero failed. The only "skipped" text in the log is inside a test name (`command load refuses forged family order skipped duplicate...`); no test skipped, not run or disabled (raw/2-ctest.log) |
| 3 | `python3 scripts/rewrite-check.py lint --build-dir build/rewrite-debug` | 0 | "Formatting and Doxygen checked 136 active source files" (raw/3-lint.log) |
| 4 | `cmake --build build/rewrite-debug` | 0 | `ninja: no work to do` (raw/4-build-a.log). The tree was already built by command 1 and the coders, so this first full build did no work either |
| 5 | `cmake --build build/rewrite-debug` (repeat) | 0 | `ninja: no work to do`, zero steps (raw/5-build-b.log) |

Zero-work check: command 5 follows the successful build in command 4 and reports no work. passed = true for all five.
Caveat: because command 4 was itself a no-op, this run did not independently show that a build which does work is followed by a zero-work build; the coders' build-all-1.log (a real build) followed by build-all-2.log (no work) covers that.

## Inventory

- `ctest -N`: Total Tests 961 (raw/6-ctest-N.log); sorted names saved as `inventory.txt` (identical to the coders' list).
- Compared against `tests/native/v5-02-x06-b3-a-evidence/inventory.txt` (957 names), the most recent inventory list found. That directory is gitignored (`.gitignore:37 /tests/native/*-evidence/`), so it is an on-disk list, not a committed one; no committed inventory exists.
- 956 preserved, 1 removed, 5 new (raw/new-names.txt, raw/removed-names.txt).
- Removed: `replacement.source capture N1 unreadable declaration is not reported as missing`. Round criterion 6 says all 957 names are preserved, so this is a deviation needing reviewer disposition (see section c).
- New: `capture admission reports only final-component ENOENT as absence`; `source capture absence is a signal from admission, not a message match`; `X06 B3 A declaration in an unsearchable directory refuses as open, not inspect`; `X06 B3 A factories capture exactly once and adoption reads nothing`; `X06 B3 A factory registration counters are deterministic across factories` (all prefixed `replacement.` in ctest).

## (a) Acceptance criteria and findings to tests

| Item | Tests (file:line) |
|---|---|
| C01-1 messages byte-identical | No test; diff audit: `git diff HEAD -- admission.cpp | grep '^[+-].*"'` printed nothing; `Cannot open admitted input` and `io_error_present` absent from src/tests (audits.txt). Behavioural check inside src/engine/acceptance/source_capture.t.cpp:517 asserts the unchanged `Cannot open confined input` / `Cannot inspect confined input` messages |
| C01-2 / R1 absence signal; EACCES dir, unreadable file, genuine missing | src/engine/capture/admission.t.cpp:847 (ENOENT only is absence); src/engine/acceptance/source_capture.t.cpp:517 (sections for permission-denied directory, mode 0000 file, missing); src/engine/runtime/runtime.t.cpp:1135 (missing declaration), :1172 (unsearchable directory refuses as open) |
| C01-3 / R2 real counters or side-effect tests | src/engine/runtime/runtime.t.cpp:1187 (capture once, adoption reads nothing), :1237 (deterministic counters) |
| C01-4 / R3 creation-site audit | audits.txt, r2-r3/creation-site-audit.txt: product `extension_invocation::create` defined once (runtime.cpp:4809), `into_extension` once (runtime.cpp:4965), five test-only create calls |
| R3 N1 behavioural | same tests as R1 (source_capture.t.cpp:517, runtime.t.cpp:1172) |
| R3 N3 behavioural | src/engine/acceptance/source_capture.t.cpp:649 |
| C01-5 / R4 docs | No test. Applied per coders; lint passes (Doxygen). I did not audit the wording line by line |
| C01-6 names preserved | see Inventory: 956/957, deviation |
| B3-A 1 known construction admits before chunks; hard-link refusal, no content read | runtime.t.cpp:1073, :1084, :1097, :1116; source_capture.t.cpp:222, :255 |
| B3-A 2 opaque cannot reach built-in load prep | runtime.t.cpp:1070 (static surface), :1273 |
| B3-A 3 owner semantics | runtime.t.cpp:1294 (moves), :1347 (adoption without recapture), :1370 (opaque adopts), :1253 |
| B3-A 4 one active frame, public validation unchanged | runtime.t.cpp:1327, :1383 |
| B3-A 5 receipt role checks, N1, N3 | source_capture.t.cpp:613 (swapped roles), :649 (N3), :517 (N1) |
| B3-A 6 dependency audit, no CLI change | dag-audit.txt (no cycle, no runtime to ledger_acceptance/acceptance_load/acceptance_prepared edge); no CLI handler files in the diff |
| B3-A 7 no new assert() | not re-audited by me; no test |

Findings: X06B3A-R1 = rows C01-2/3 above; R2 = counters row; R3 = audit/N1/N3 rows; R4 = docs row.

## (b) Red-first

Only the coders' evidence is available; I did not re-run any red.

| Test | Command (from evidence) | Snapshot hash | Failing line | Real red? |
|---|---|---|---|---|
| admission.t.cpp:847 absence signal | `cmake --build build/rewrite-debug --target planar_acceptance_source_capture_tests planar_capture_admission_tests; ctest -R "source_capture|admission" --output-on-failure` (r1/NOTES.md) | `7f605280895467b59928879de0f3cc154851bc8d7881adb1357fe6309d9a6c38` | admission.t.cpp:862 | Red against B3-A code with temporary stubs (signal returns false). Stubbed, not pure B3-A |
| source_capture.t.cpp:517 (permission-denied directory, mode 0000 file) | same command | same hash | source_capture.t.cpp:527, 538, 566, 567, 584, 589 (e.g. got "Cannot inspect confined input", expected "Cannot open confined input") | Red, same stubbed snapshot. The red build needed a temporary one-line runtime.cpp edit, later reverted |
| runtime.t.cpp:1172 unsearchable directory | none recorded | n/a | n/a | No red recorded; the red build covered only the admission and source_capture targets. Missing evidence |
| runtime.t.cpp:1187 capture exactly once | mutant: second `profile_capture::open` in `open_opaque`; `cmake --build build/rewrite-debug --target planar_runtime_tests && planar_runtime_tests '[x06b3a]'` (r2-r3/red-capture-twice.txt) | `082c30a6701dcb2a2c18f78008b3f990c182ea617895d0f2f2394989fc58d6e9` | runtime.t.cpp:1206, 1212, 1225 (10 == 6) | Real mutation red (test against mutant, not a pre-fix snapshot) |
| runtime.t.cpp:1237 counters deterministic | none recorded | n/a | n/a | No red; likely a guard. Missing evidence |
| source_capture.t.cpp:649 N3 | mutant removing the moved_from -> invalid_state branch; `planar_acceptance_source_capture_tests "source capture N3 moved-from*"` (r2-r3/red-n3.txt) | pre-mutation diff `082c30a6...58d6e9`, file `a3e671cd...e5de` | source_capture.t.cpp:674 (3 == 0) | Real mutation red |

Notes: the R2/R3 hash differs from the final hash because later edits landed (coders' own note). The round asked for N1/N3 reds against the pre-fix B3-A snapshot (`e2191749...`); the N1 red used a stubbed variant and the N3 red a mutant, not that snapshot. No red command was recorded for the five-test inventory beyond those above.

## (c) Deliberate behaviour differences

- Removed test name `source capture N1 unreadable declaration is not reported as missing`, replaced by the R1 tests. Round criterion 6 requires 957 names preserved; nothing in the round record permits a rename, so this needs reviewer disposition.
- Platform difference (documented per R1): macOS `O_EVTONLY` makes a mode 0000 present file fail at admission with EACCES; Linux `O_PATH` would admit it and fail at the content read. Neither is absence. Linux behaviour is reasoned from source, not run.

## Carried duties (from round record)

N2 profile-frame branches dead until B3-B (needs cumulative-exhaustion test); N3 `into_extension` accepts known-load owners, B3-B or X07 decides; N4 `profile_invocation::implementation` heap not separately charged; N1 `begin_frame` stays public as a documented composition seam; N5 runtime @file brief updated.

## Limitations

macOS Debug only; no RelWithDebInfo, Linux, RSS or whole-invocation budget claim. Reds were not re-run by the gate runner. command_vm exposes no counters, so counter tests compare across owners only (r2-r3/NOTES.txt). No commit was made.
