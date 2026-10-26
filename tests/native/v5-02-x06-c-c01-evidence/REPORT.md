# Gate report, round V5-02-X06-C-C01 (gate run 3)

Result: passed=true. Every command met its criterion. This run's raw logs are raw/g3-*; earlier runs (raw/g2-*, raw/build*, raw/ctest*, raw/lint*) are retained. This run supersedes g2 because the candidate changed after the mutation probes (tests repaired, record edited).

## Candidate identity

HEAD caaf0396ae90a7f7d2d511b9a9a6e185e4362d52, working tree uncommitted.
- sha256 of `git diff HEAD` (tracked files): 3ad35762c7dfab9f09c4fa005a0f68b8c8ce8c9ee8a8a4e37ef5a8eba14ab1e6
- sha256 of that diff followed by `== <path>` and the full text of each untracked non-tests/native file, sorted by path (8 files: three docs/vernacular/rounds records, acceptance_seam.{cpp,cppm}, profile_acceptance.{cpp,cppm,t.cpp}): 5b5d9d20714cbe07fafcf3b0185751694386fd389c7659f04a65d41f613c9c04
- These differ from the hashes in probes/PROBES.md (tracked 9a7a6941..., combined 7cbe345e...). The probes ran on the earlier tree; the tracked diff and the records changed afterwards. Mutation results therefore describe the earlier tree, not this exact one.

Files from git status (15, used for the redact check): modified docs/vernacular/roadmap.md, src/engine/CMakeLists.txt, src/engine/ledger/acceptance.{cpp,cppm,t.cpp}, tests/native/r14c3b2-support/generate.py, tests/native/v5-02-x03-b2-b-c01-support/generate.py; untracked docs/vernacular/rounds/V5-02-X06-C{,-C01,-review}.md, src/engine/ledger/acceptance_seam.{cpp,cppm}, src/engine/ledger/profile_acceptance.{cpp,cppm,t.cpp}.

## Commands (serial, no pipes, exit codes in raw/g3-*.exit)

| command | exit | result |
|---|---|---|
| cmake --build build/rewrite-debug --target planar_tests | 0 | built (raw/g3-build1.log) |
| ctest --test-dir build/rewrite-debug --no-tests=error --output-on-failure | 0 | "100% tests passed, 0 tests failed out of 1031". Real time 366.34 s. Labels: unit 1024 tests, acceptance 7 tests. Skips: none reported by ctest. The "skipped" text in the log is only in the name of test #560 (command load refuses forged family order skipped duplicate and malformed arguments), which passed. #531 (X06 B3 C a file replaced at any allocation inside prepare_load ...) Passed in 15.27 s (g2: 15.28 s; limit about 15.3 s). |
| ctest -N | 0 | Total Tests: 1031 (raw/g3-ctest-N.log) |
| python3 scripts/rewrite-check.py lint --build-dir build/rewrite-debug | 0 | clang-format --dry-run --Werror clean; "Formatting and Doxygen checked 141 active source files" |
| cmake --build build/rewrite-debug (first, after the successful planar_tests build) | 0 | built (raw/g3-build2.log) |
| cmake --build build/rewrite-debug (second) | 0 | "ninja: no work to do." Zero steps; criterion met. This covers the default target only; planar_tests is excluded from `all`, and no separate no-change run of planar_tests was in the agreed list. |
| python3 scripts/redact-local-paths.py check (no file args) | not run | The script requires `--repo-path`, `--home` and files; a bare call is a usage error (exit 2 in the earlier run). |
| python3 scripts/redact-local-paths.py --repo-path <repo> --home <home> check <the 15 files above> | 0 | passed, empty output (raw/g3-redact.log) |

## Inventory

Compared with tests/native/v5-02-x06-b3-c-c02-evidence/inventory.txt, the most recent committed list (1003 names). Current list is inventory.txt next to this file (1031 names, sorted, from ctest -N, `replacement.` prefix kept; it replaces the 1030-name list from gate run 2). Removed names: 0. New names: 28 (1003 + 28 = 1031): the 25 X06-C names plus three from this round, listed below. No existing name was renamed. Two C01 names were present in g2 and one is new since:
- replacement.X06 C C01 profile completion stores a built-in job beside a custom job
- replacement.X06 C C01 seam is unreachable from a plain acceptance importer
- replacement.X06 C C01 seam interns the registry tree with the provenance limits (added after the probes; the probe record's PROBES.md describes it as the C14 repair)

The 25 X06-C names (all containing "X06 C " without "C01"): all present, none removed.

## (a) Acceptance criteria and findings to tests

Line numbers are TEST_CASE lines at gate time.

| criterion or finding | tests |
|---|---|
| 1 seam unreachable from a plain importer; CLI variants still build | X06 C C01 seam is unreachable from a plain acceptance importer, src/engine/ledger/acceptance.t.cpp:770 (textual check of acceptance.cppm on a comment-stripped, whitespace-squashed view plus a scan of src/**/*.cpp(m) for the seam names, plus embedded-entry static checks). Limit: textual, not a name-lookup probe, so an indirect re-export through another module is not detected. Per PROBES.md C22 (CLI importing the seam directly) and C02-r2/r3 survived on the earlier tree; the repair says the scan and squashing kill them, but no prober re-run on this tree exists. Variants planar_atomic_acceptance_seam_test and planar_metadata_acceptance_seam_test build in the planar_tests run. The CLI binary was not built separately beyond the default no-work build. |
| 2 shared transaction body, behaviour unchanged, 1003 + 25 names kept | whole suite 1031/1031 passed; 0 removed names. Through the adapter: X06 C seam interns an effective stdlib closure beyond the original tree limits, profile_acceptance.t.cpp:756; X06 C C01 seam interns the registry tree with the provenance limits, :859; profile completion tests in the same file. |
| 3 / R2 built-in plus custom job stores and replays | X06 C C01 profile completion stores a built-in job beside a custom job, profile_acceptance.t.cpp:791 |
| 4 I1i reworked; invariant rows R1 and R2 | I1i: profile_acceptance.t.cpp:756; R1: acceptance.t.cpp:770; R2: profile_acceptance.t.cpp:791. Mutant and killer table: probes/PROBES.md (rows I-R1a to I-R5; all KILLED on the earlier tree). |
| 5 authority analysis | record text only; not checked by this gate. |
| R1 (seam restriction) | acceptance.t.cpp:770 (above) |
| R2 | profile_acceptance.t.cpp:791 |
| R3 diagnostic text | X06 C profile completion refuses an intent that differs from the bound intent, profile_acceptance.t.cpp:340 (message asserted at :346); refuses a same id intent whose body differs from the bound intent, :349 (message asserted at :356). Text lives at profile_acceptance.cpp:24. Record: "Decision B" still appears at docs/vernacular/rounds/V5-02-X06-C.md lines 267 and 289 (rows I1g, X16), so the record correction is not yet made. |
| R4 moved-from refusal after replay, format and CAS, before any row | X06 C profile completion moved-from profile refuses after CAS before any row, profile_acceptance.t.cpp:691. Record reconciliation of Carried item 1 not verified. |
| R5 I3b row, no-Lua comment, placement and audit | no-Lua test at profile_acceptance.t.cpp:562; opaque-surface test :713. Record rows and audit sections not verified; PROBES.md names I3b as a different-code mutant killed by an acceptance.t.cpp test. |
| 6 records R3 to R5 | records only; X06-C.md still has "Decision B" (R3 open). |
| 7 dependency audit | not run by this gate; no command for it in the agreed list. |

## (b) Red-first

Source: red/RED.md only. Command pattern: `ctest --test-dir build/rewrite-debug -R '^replacement\.<name>$' --no-tests=error --output-on-failure`, raw logs in red/raw/. Snapshot: `git diff HEAD -- src` sha256 9681ec4d736fe60caf671b62167845b74bc924f70f7377fdedf5991f2755b184 (tracked files under src; untracked files not stated as included).

| test | outcome | failing line |
|---|---|---|
| X06 C C01 seam is unreachable from a plain acceptance importer | real red (earlier probe form) | acceptance.t.cpp:754, 755, 756, also 752 at that snapshot. The body was rewritten afterwards to the textual form; no red record for the rewritten form. |
| X06 C seam interns an effective stdlib closure beyond the original tree limits | guard, already passed | none |
| X06 C C01 profile completion stores a built-in job beside a custom job | guard, already passed | none |
| refuses a same id intent whose body differs from the bound intent | guard, already passed | none |
| moved-from profile refuses after CAS before any row | guard, already passed | none |
| surface accepts only the opaque prepared profile | guard, already passed | none |
| X06 C C01 seam interns the registry tree with the provenance limits | no red record (added after RED.md) | missing |

Missing: behavioural red for the reworked R1 test, a red record for the registry-tree test, and a snapshot hash covering untracked files. The guard tests have mutant evidence in PROBES.md (kills) instead of a red.

## (c) Deliberate behaviour differences

None reported by the coders or visible in the gate; the full suite including byte-exact diagnostic tests passes. The only new diagnostic text, "Prepared profile differs from request intent" (invalid_argument, ledger-internal), is analysed in the record (R3, still open). No contract clause is invoked here.

## Limitations

- No-change build covers the default target only.
- Mutation probes (PROBES.md) ran on an earlier tree; survivors there: X12 (equivalent), X18, X20, C01 (expected), C05, C09, C14, C17, C19, C22, C02-r2, C02-r3. The repair notes address C02-r2/r3, C22, C14, C17, X28; X18, X20, C09 are not addressed in any record I read; C05 and C19 are stated as unkillable. None re-probed.
- Records R3 to R5, authority analysis and dependency audit appear not yet written; coordinator-owned.
- Redact check ran on the 15 listed files only.
