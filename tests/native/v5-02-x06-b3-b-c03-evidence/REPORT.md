# V5-02-X06-B3-B-C03 gate report (run 3, candidate with two repair tests)

Verdict: **passed = true.** All six commands executed and met their criteria on the current candidate under the amended Dispatch 2 criteria (four product file hashes unchanged, test names equal to the 985 C02 names plus only the new C03 names). Raw logs for this run are in `raw/run3/`. Earlier runs are kept: run 1 (`raw/00-*` to `07-*`, `raw/REPORT.run1.md`, `raw/inventory.run1.txt`) and run 2 (`raw/run2/`, `raw/REPORT.run2.md`, `raw/inventory.run2.txt`). No source file was edited by the gate runner.

## Candidate identity

- HEAD: `a2aa1cc0ed4ad8c5b9e879a04231591839e7dc48` (`raw/run3/00-head.txt`).
- `git diff HEAD -- src | shasum -a 256`: `96e44f54086469fe3ee0ccfa431ff4e3fadd12a15278834e5d6714613990531b` before and after the gate (`00-diff-start.sha256`, `07-diff-end.sha256`). Dispatch 2 says this whole-tree hash is no longer a gate value.
- `git diff HEAD` (whole tree, including roadmap): `3c6d71bb792483c862a479d28ffa30f052413f8663f08806fd9d933b1482ed4f` (`00-diff-full.sha256`). Untracked files are not part of a git diff.
- Product file hashes (first 12), required values met: `runtime.cpp` `ab70f5441bea`, `runtime.cppm` `331bd1b629df`, `profile.cpp` `91ac81f3d466`, `profile.cppm` `7bb805634449`. Full values in `raw/run3/00-files-start.sha256`.
- Test files: `runtime.t.cpp` `3b22d1378165` (was `b2aeb97ef32a` in run 2, `162997debb60` in run 1), `profile.t.cpp` `d94976e9c9ea` (unchanged).

## Commands (serial, unpiped, exit codes read from files)

Driver: `raw/run3/run-gate.sh`.

| # | Command | Exit | Result |
|---|---|---|---|
| 1 | `cmake --build build/rewrite-debug --target planar_tests` | 0 | `ninja: no work to do.` (tree already built by the repair coder; see limitations) |
| 2 | `ctest --test-dir build/rewrite-debug --no-tests=error --output-on-failure` | 0 | `100% tests passed, 0 tests failed out of 987` (unit 980, acceptance 7). Zero failures. The only log lines matching "skipped" are the passing test whose name contains the word (`replacement.command load refuses forged family order skipped duplicate and malformed arguments`). Zero skips. |
| 3 | `ctest -N`, then sorted diff against `tests/native/v5-02-x06-b3-b-c02-evidence/inventory.txt` | ctest -N 0, diff 1 (additions only) | `Total Tests: 987`. 985 C02 names kept, none removed or renamed, two added (below). |
| 4 | `python3 scripts/rewrite-check.py lint --build-dir build/rewrite-debug` | 0 | `Formatting and Doxygen checked 136 active source files`. Doxygen died once from the known SIGBUS flake and passed on attempt 2 of 3 (logged in `04-lint.log`). |
| 5a | `cmake --build build/rewrite-debug` (first) | 0 | `ninja: no work to do.` |
| 5b | `cmake --build build/rewrite-debug` (second) | 0 | `ninja: no work to do.` Zero steps. |
| 6 | sha256 of the four product files | n/a | equal to the required prefixes (above) |

Diff exit 1 only means the lists differ; the diff (`raw/run3/03-inventory.diff`) contains only `>` lines.

## Inventory

Compared against `tests/native/v5-02-x06-b3-b-c02-evidence/inventory.txt`, the most recent earlier inventory (985 names). Current list: `inventory.txt` (987 names, sorted, numbers stripped; raw `raw/run3/03-ctest-N.log`).

- New (2): `replacement.X06 B3 B a route phase refusal is ordinary semantic data before any job is captured` and `replacement.X06 B3 B a declaration the bound source cannot open refuses with its native diagnostic`.
- Removed or renamed: 0.

## (a) Acceptance criteria to tests

Line numbers are in `src/engine/runtime/runtime.t.cpp` unless noted. Most C03 criteria are records and evidence criteria that no test covers; the round records were not verified by the gate runner.

| Criterion | Evidence | Tests |
|---|---|---|
| R1 records describe the final candidate | round records, not verified here | 41 `X06 B3` names expected (39 of C02, plus the 2 C03 tests); not counted by this run |
| R2 `owner_charge` amendment | C03 Dispatch | `X06 B3 B owner allocation charge is observable and covers the owner's own storage` :1615 |
| R3 full probe set on final candidate | `probes/PROBES.md`, written before the repair tests (candidate `8b2f73ea...`); not re-run on the 987-test candidate | e.g. `X06 B3 B prepare_load reverifies captured profile content exactly three times` :2021; `X06 B3 B exhaustion depends on route consumption because the instruction budget is one cumulative counter` :2031; `X06 B3 B success releases nested temporaries to the exact pre invocation membership` :1948; `X06 B3 B one frame cumulative counters never reset` :1832; `X06 B3 B prepare_load reads the declaration through the watched interposer` :1666; `X06 B3 B capture change before prepare_load refuses capture_changed with zero declaration reads` :1882; `X06 B3 B declaration leaf replaced before prepare_load refuses terminally before content` :1765; `X06 B3 B final intent must equal the bound intent` :1787; `X06 B3 B opaque consumed failed and moved-from owners refuse prepare_load` :1811; `X06 B3 B a custom job refusal after earlier shape dispatch is ordinary semantic data` :1736; `X06 B3 B an invalid unselected job fails all-job validation and publishes nothing` :1675; `X06 B3 B every job is validated exactly once` :1747; `X06 B3 B note_registration is a pure counter that never affects authority` (`src/engine/capture/profile.t.cpp`:876); `X06 B3 B note_registration is a no-op on moved-from owners and the count moves with the capture` (`profile.t.cpp`:907) |
| Survivor `semantic-routed-as-protocol` (`b7a8d1bc017a`) closed | Repair 1 | `X06 B3 B a route phase refusal is ordinary semantic data before any job is captured` :1691 |
| Survivors `open-bound-refusal-protocol` (`febc237bd88c`), `source-error-mapping` (`664b8217a4bf`) closed | Repair 2 | `X06 B3 B a declaration the bound source cannot open refuses with its native diagnostic` :1718 |
| Survivor `fail-overwrites-shape-failure` (`939c80e88815`) | stays a survivor with the blocking predicate explained in Repair 2 | none |
| Survivor `p-wrap-no-saturation`, `reverify-moved` | recorded unreachable or equivalent | none |
| N-c known limit | C03 Dispatch | the zero-declaration-read check in `X06 B3 A into_extension adopts the same VM and receipts without recapture` (about :1500, lines 1528 to 1539 at dispatch time; line numbers have shifted by the added tests) cannot fail; not a tested claim |
| N-d C02 README | `tests/native/v5-02-x06-b3-b-c02-evidence/README.md` exists | none |
| Test names (985 C02 names kept plus only new C03 names) | `raw/run3/03-inventory.diff` | met: 2 additions, 0 removals |
| Diagnostics byte-identical | four product files hash-equal to C02-reviewed values | no product diagnostic can have changed |

## (b) Red-first

- `X06 B3 B a route phase refusal is ordinary semantic data before any job is captured` (:1691). Red snapshot: pre-repair `runtime.t.cpp` sha256 `162997debb605ea82fcfa855e24ee527dcac63c72f4ae6c5baf659e95c4557c7`, which lacks the test. The only evidence is that mutant `semantic-routed-as-protocol` SURVIVED on that candidate (0 failed of 39, `probes/PROBES.md`). The record states the `REQUIRE` on `semantic` would fail under the mutant, but no command or failing line has been recorded: the repair coder did not mutate and no prober re-ran. **Missing evidence: the red is claimed, not shown. Not a demonstrated real red.**
- `X06 B3 B a declaration the bound source cannot open refuses with its native diagnostic` (:1718). Red snapshot: `runtime.t.cpp` `b2aeb97ef32a...` (run 2 candidate, lacks the test; full hash only in `raw/run2/00-files-start.sha256`). Targets mutants `open-bound-refusal-protocol` and `source-error-mapping`, which SURVIVED standalone (0 failed of 37, `probes/PROBES.md`). The record says the repair coder applied no mutant, and the prober has not re-run. **Missing evidence: no command or failing line; red claimed, not shown.**
- C01 and C02 tests: the C01 pre-fix snapshot does not compile against the tests (B3-B record), so red-first is by killing mutants, listed in `probes/PROBES.md` and `tests/native/v5-02-x06-b3-b-c02-evidence/probes/`. The probe command recorded for that is `ctest -R 'X06 B3' --no-tests=error -j 4 --timeout 300 --output-on-failure` in a scratch copy. No separate red-first log exists for them in the C03 folder.

## (c) Deliberate behaviour differences

None in product code (four product files hash-equal to C02). The one recorded deviation is the N-c amendment adding `profile_invocation_observation::owner_charge` (coordinator decision R2 in the C03 Dispatch). It relaxes the brief's advisory N-c, not an external-profile contract clause.

## Limitations

- Build steps 1 and 5a reported no work because the repair coder had already built the tree. The zero-work check (5b) is meaningful, but 5a is not a fresh rebuild. Run 1 (`raw/01-*`, `raw/05-*`) holds a real build log for an earlier candidate.
- Mutation probes ran on the pre-repair candidate (`8b2f73ea...`) and do not confirm the kills by the two new tests. The mutation prober must re-run.
- The ctest log in this run was produced without `-j`, so it ran serially (346 s real).
- macOS Debug only. No Linux run.

## Run 4 (independent repeat by a second gate runner, same candidate)

Verdict: passed = true. HEAD `a2aa1cc0ed4ad8c5b9e879a04231591839e7dc48`; `git diff HEAD | shasum -a 256` `3c6d71bb...ed4f` (identical to run 3). Raw logs: `raw/run4/` (`build-tests.log/.exit`, `ctest.log/.exit`, `g-ctest-N.log`, `g-inventory.diff`, `g-lint.log`, `g-build1.log`, `g-build2.log`). No source edited; run 3 files untouched.

| Command | Exit | Result |
|---|---|---|
| `cmake --build build/rewrite-debug --target planar_tests` | 0 | completed (it ran past 120 s and was backgrounded; exit read from file) |
| `ctest --test-dir build/rewrite-debug --no-tests=error --output-on-failure` | 0 | `100% tests passed out of 987` (unit 980, acceptance 7), 327.71 s, 0 failed, 0 not run |
| `ctest -N`, sorted diff vs C02 `inventory.txt` | 0 / diff 1 | 987 names; only two additions, both the C03 names listed above; no removals; `inventory.txt` regenerated, identical set |
| `python3 scripts/rewrite-check.py lint --build-dir build/rewrite-debug` | 0 | `Formatting and Doxygen checked 136 active source files` |
| `cmake --build build/rewrite-debug` (first) | 0 | `ninja: no work to do.` |
| `cmake --build build/rewrite-debug` (second) | 0 | `ninja: no work to do.` (zero steps) |
| sha256 prefixes | n/a | `runtime.cpp` ab70f5441bea, `runtime.cppm` 331bd1b629df, `profile.cpp` 91ac81f3d466, `profile.cppm` 7bb805634449: all equal |

Limits: the first full build reported no work because the planar_tests target step had just built the tree; the two "all" builds are both no-work, so the zero-work check follows a successful build but not a fresh full rebuild. The red-first and acceptance sections above are unchanged: red evidence for the two C03 tests is claimed, not shown. Two "skipped|disabled" grep hits in ctest.log are test names/lines containing the word, not skips.
