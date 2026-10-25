# X06-B3-B-C01 gate report (macOS Debug, re-run on the repaired candidate)

Role: gate runner. No source file was edited by this run. Nothing was committed, pushed or installed. This
report supersedes an earlier gate report written before the R2/R4 repair; that earlier report is preserved
unchanged at `raw/prior-REPORT-before-regate.md.orig`. The first coder report is at `raw/coder-REPORT.md.orig`.

## Verdict

passed = true: all six gate commands met their criterion on the current candidate. This is a gate result only.
It does not say the mutation evidence is complete (see sections b and Limitations: the repair's probes have no raw
logs, and `reset-c-instr` is recorded as an equivalent mutant rather than killed).

## Candidate identity

- HEAD `0cc29923383b39508e31cbf0cb62651185963480`, branch `dev/planar-v2-vernacular`.
- `git diff` sha256 `04fda68d8a48cb57728554ef5b43bd0d9c72bf0818e5cdc3157ca0345f748bf5`, identical before and after the
  gate. It differs from the earlier gate's `05c3f070...` because the repair edited the runtime files after that gate.
- Modified tracked files: `docs/vernacular/roadmap.md`, `src/engine/capture/profile.{cppm,cpp,t.cpp}`,
  `src/engine/runtime/runtime.{cppm,cpp,t.cpp}`. Untracked: `docs/vernacular/rounds/V5-02-X06-B3-B.md` and `-review.md`.
- Files this run wrote: this REPORT.md, `inventory.txt`, `new-names.txt`, `removed-names.txt`, `raw/{1..6}-*.log`,
  `raw/{1..6}.exit`, `raw/prior-REPORT-before-regate.md.orig`. Nothing else.

## Gate commands (serial, exit codes read directly, no pipes)

| # | Command | Exit | Result | Log |
| --- | --- | --- | --- | --- |
| 1 | `cmake --build build/rewrite-debug --target planar_tests` | 0 | rebuilt, 218 steps, last `Linking ... planar_x05_closure_audit_tests` | `raw/1-build-tests.log` |
| 2 | `ctest --test-dir build/rewrite-debug --no-tests=error` | 0 | `100% tests passed, 0 tests failed out of 982`; real time 332.60 s; serial | `raw/2-ctest.log` |
| 3 | `ctest --test-dir build/rewrite-debug -N` | 0 | `Total Tests: 982` | `raw/3-ctest-N.log` |
| 4 | `python3 scripts/rewrite-check.py lint --build-dir build/rewrite-debug` | 0 | `Formatting and Doxygen checked 136 active source files` | `raw/4-lint.log` |
| 5 | `cmake --build build/rewrite-debug` (first) | 0 | built (3 steps, relinked `bin/planar`) | `raw/5-build-all.log` |
| 6 | `cmake --build build/rewrite-debug` (second, no-change check) | 0 | `ninja: no work to do.` (zero steps) | `raw/6-build-nochange.log` |

Skips, failures, disabled, not run, timeouts: none. Grepping the ctest log for skipped/disabled/failed/not-run hits
only test names containing those words (e.g. "failed", "skipped"), all reported `Passed`. Labels: 975 unit, 7 acceptance.
The no-change criterion is met: command 6 ran after the successful command 5 and reported no work.

## Inventory

Baseline: `tests/native/v5-02-x06-b3-b-review-evidence/inventory.txt` (977 names, the B3-B review's `ctest -N` list). It is
the most recent inventory I found; it is untracked, gitignored review evidence. No committed inventory is newer than the
B3-A-C02 gate (963 names). Chain: 963 (B3-A-C02) -> 977 (B3-B reviewed) -> 982 (this candidate).

- Current: 982 registered, all unique (`inventory.txt`). Removed: 0 (`removed-names.txt` empty). New: 5 (`new-names.txt`):
  1. `replacement.X06 B3 B capture change before prepare_load refuses capture_changed with zero declaration reads`
  2. `replacement.X06 B3 B captured profile content is reverified before source binding, after capture and after sealing`
  3. `replacement.X06 B3 B failure paths release nested temporaries`
  4. `replacement.X06 B3 B one cumulative instruction and native byte budget spans route validation and preparation`
  5. `replacement.X06 B3 B success releases nested temporaries to the exact pre invocation membership`
- The record's C01 results still say 981 and 4 new; they predate the repair test (item 2) and need updating by the coordinator.

## (a) Acceptance criteria and findings to tests

Lines are `TEST_CASE` lines in the current tree, in `src/engine/runtime/runtime.t.cpp` unless stated. Criteria are from the
C01 record (`docs/vernacular/rounds/V5-02-X06-B3-B.md`, "C01 Acceptance criteria").

| Item | Test (file:line) | Status |
| --- | --- | --- |
| C01-1 / R2 pre-capture reverify | "capture change before prepare_load refuses capture_changed with zero declaration reads" :1812; "captured profile content is reverified before source binding, after capture and after sealing" :1951; related "declaration leaf replaced before prepare_load refuses terminally before content" :1695 | Passing. Per the repair note, :1951 kills the `reverify` mutant (observed 6 reads vs 4). The mutant run was in place, no raw log (see b). |
| C01-1 / R3 no counter reset | "one cumulative instruction and native byte budget spans route validation and preparation" :1848; "one frame cumulative counters never reset" :1762 | Probe table (pre-repair candidate) shows reset a, c-bytes, d killed. `reset-c-instr` (reset before phase 8) survives and is claimed behaviourally equivalent by the repair. |
| C01-1 / R4 success release | "success releases nested temporaries to the exact pre invocation membership" :1879 | `release-success` killed by this test. `baseline` removal survived pre-repair; repair claims it is now killed through `baseline_before_route`. |
| C01-1 / R4 failure release | "failure paths release nested temporaries" :1897; "cumulative instruction exhaustion preserves the first error and cleans nested roots" :1783 (name kept) | `release-failure` killed by both. |
| C01-2 names preserved | inventory section | Met: 0 removed, 5 new, 982 total. |
| C01-3 diagnostics, no assert, typed errors, docs | lint exit 0 over 136 files; "X06 B3 A profile invocation static surface offers no prepare, vm or upgrade access" (existing, in runtime.t.cpp) plus stage static asserts | Gate part met. Diagnostic byte-identity not re-audited by this run. |
| C01-4 six invariants probed | single frame/no reset: `reset-*` (above); reverify: :1812/:1951; same_selection: native `D2 B2 C01 first retained reads require original root selection` `src/engine/capture/native.t.cpp:782` (killed, 1 of 1); every job: :1677 (`everyjob` kills 9 tests); consumed-before-copy: :1717, :1741, :1457 (`consumed` kills 3); N3: :1578, :1205 area (`n3` kills 3 incl. "X06 B3 A factories capture exactly once and adoption reads nothing" and "registration count stays one across into_extension") | Table in `raw/probes/table.md` (pre-repair). |
| C01-5 B3-B criteria 1, 2, 4, 5, 7 | :1615, :1650, :1666, :1677, :1695, :1717, :1741 | Passing in the 982 run. |
| R1 probes and red-first | section (b) | Partly evidenced; see below. |
| N1 charge before allocate | :1603 "owner implementation allocation is charged identically by both factories"; :1265 "factory registration counters are deterministic across factories" | `n1-known`, `n1-opaque` each kill both. Removing the charge from both factories was not probed. |
| N2 drifted B3-A tests | :1457 "begin_frame is once only and blocks adoption"; :1500 "into_extension adopts the same VM and receipts without recapture" | Names kept. `consumed` kills :1457. Content of :1500 not verified by me. |
| N3 same_selection native-only | `native.t.cpp:782` | Recorded; `samesel` survives `[x06b3b]` by design and is killed by the native test. |
| N4 unreached refusal branches | :1931 "profile frame branches stay reachable and first terminal native failures are preserved" | Covers reachable branches only. The unreachable ones are recorded, not tested. |
| N5 final `catch (...)` | none | No test or probe forces it. The record says "maps to protocol"; the repair report and prior gate say it maps to `external_resource`. I did not check which the code does. |
| N6 verification list | record "C01 verification results" | Filled but stale (981). |
| X06B3AC02-N3 note_registration | `src/engine/capture/profile.t.cpp:876`, `:907` | Passing. |

## (b) Red-first

Pre-fix snapshot hashes recorded by the coders (the reviewed B3-B candidate): runtime.cpp
`7783183ff6f35d010af86a0c3d3cfe54f9674d270cc4813e788f0ebe0be82fab`, runtime.cppm
`f30b48ea86be40d03f7cc34148fcd091634636e548792483d3d7b18af208b144`, runtime.t.cpp
`af373673fcafa7ae89c1b4173ab1d307ae86467e55062b165869d6e60666df31`. The record's pre-repair candidate diff hash is
`05c3f070c9ae8445cf07900b0047e6c7746d89f8a726bd4682644ee97a00e055`.

I found no red-first record (exact command, failing line, real-red-or-guard) for any of the five new tests in the evidence
directory. The coder report states package A's record was never delivered. I do not invent one. What exists:

| New test | Evidence | Real red or guard |
| --- | --- | --- |
| capture change before prepare_load ... (:1812) | `probe-reverify.log` shows `reverify` surviving at diff `05c3f070` | Guard: it passed with the mutant. The repair added :1951 for this. |
| captured profile content is reverified ... (:1951) | Repair note only: mutant sha256 `62086754001d5869b8a627db36f190880f45289c0cb56f747cbe8860a2744e91`, 6 reads expected, mutant gives 4 | Mutant-red claimed; no raw log and no pre-fix run in the evidence. |
| one cumulative ... budget ... (:1848) | `probe-reset-a-*`, `reset-c-bytes`, `reset-d-*` logs show named failures; the reviewer's `reset` probe survived the pre-fix tests | Mutant-red; no pre-fix test run recorded. |
| success releases nested temporaries ... (:1879) | `probe-release-success.log`, sole killer | Mutant-red only. |
| failure paths release nested temporaries (:1897) | `probe-release-failure.log` | Mutant-red only. |

Probe commands visible in the evidence: `raw/probes/run-probes.sh` and `raw/probes/probe.py` (scratch worktree outside the
checkout, ctest under `perl -e 'alarm 900'`). The repair's command, quoted from the coder note, was
`cmake --build build/rewrite-debug --target planar_runtime_tests && ctest -R "X06 B3" -j4`, with mutants applied in place in
the checkout and restored to runtime.cpp sha256 `1ff83396d7684368cc9b524e7587f9325f7791b1334036d4e7f54b9da762c890`. That
departs from the standing rule to mutate only a scratch copy, and it left no raw logs. I did not re-run any probe.

Pre-repair probe results (`raw/probes/table.md`, 33 tests matched by `X06 B3`): killed consumed, everyjob, n1-known,
n1-opaque, n3, release-failure, release-success, reset-a-bytes, reset-a-instr, reset-c-bytes, reset-d-bytes, reset-d-instr,
and native samesel. Survived: `baseline` (a8bc0457e3e1), `reset-c-instr` (98ddd3afa475), `reverify` (6130251d646c),
`samesel` within `[x06b3b]`.

## (c) Deliberate behaviour differences

- `into_extension` on a known-load owner is refused with a non-consuming `invalid_state` (round scope item 6, X06B3A-N3).
- `prepare_load` releases nested shape temporaries on success and failure, as the extension path does (review R4).
- Owner implementation allocation is charged before `make_unique` (N1, C01 advisory).
- `prepare_load` has a final `catch (...)` after consumption. The C01 record requires a protocol mapping; the coder notes say
  `external_resource`. Which clause permits the latter is not stated; flagged for the reviewer.
- New observation-only field `baseline_before_route` on the stage observation, added by the repair to pin R4. No new public
  accessor on the stage is claimed.

## Limitations

- macOS Debug only: no RelWithDebInfo, Linux, RSS or whole-invocation budget claim.
- Serial ctest, 332.6 s. The coders' earlier `-j 8` run timed out two heavy cases at the default 30 s; both passed alone.
- No mutation probe was re-run by this gate. Repair probes lack raw logs and ran in place; `reset-c-instr` is asserted equivalent,
  not shown killed.
- Red-first records for the five new tests are absent or mutant-only.
- Not verified here: diagnostic-message byte identity, content of the :1500 test, the `catch (...)` mapping.
- The inventory baseline is untracked, gitignored review evidence.
