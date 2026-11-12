# R19-ACCEPT-A-C01 independent review

Disposition: **request-changes**. The history defect is diagnosed and fixed correctly. The fix has a
behavioural red-first regression and is minimal, and closed-table diagnostics are unchanged. Gates, the
22-mutant re-probe and the quota live runs hold up when checked against the raw bytes. The round still does not meet its
own acceptance criteria. The faulted CMake rerun, the mini-fault run and the `limits`, `packet`, `index`,
`scan` and `verify-redaction` steps were never executed. The evidence set marked for commit contains real
host paths that the redaction gate never checked. Most invariant rows for the fault path have no probe
outcome. The first and third findings need new runs and new evidence, so they cannot be fixed by editing
records alone.

The reviewer was not the coder, coordinator or prober. Writes were limited to this file. No checkout file
was mutated. No build, ledger, installed binary or legacy binary was run, and no script under `tests/native/`
was run. The only tool executed was the workflow identity tool, copied to the scratchpad and run from the
repository root.

## Candidate identity

Re-run by this reviewer with pathspec `src tests/native/r19-accept-support`. The output matches the
round record, `probes/PROBES.md` and the gate run 4 `identity-before.txt`/`identity-after.txt` byte for byte:

| identity | value |
|---|---|
| HEAD | `91e0df24f2167471e49ec057c7159bce5dff3608` |
| sha256(git diff --binary HEAD -- src tests/native/r19-accept-support), tracked only | `83a16fdee033fce5a1a792d8d65f4090fba8dacffc2d38013684cacb4daeba19` |
| combined sha256, pathspec src tests/native/r19-accept-support, evidence excluded | `6612206823f3350ea5e9ce36bf9f39750a553894ca6624941379f2b200dd9891` |

Product diff: `src/engine/ledger/storage.cpp` (8 insertions, 4 deletions) and
`src/engine/ledger/execution.t.cpp` (177 pure insertions). `runtime.cpp`, production Lua and
`external_registration.t.cpp` are unmodified. The dispatch's candidate list names `runtime.cpp`, but the
defect is in `storage.cpp`. The record documents this as an owned-file amendment (D1, D31), and the
amendment is justified.

## What was verified and holds

- **History defect and fix.** `history_rows` set both `integer` and `boolean` on every `n`/`b` column.
  `runtime.cpp` `validate()` refuses any non-boolean node that has `boolean` set and any non-integer node
  that has `integer` set, and raises `Unsupported trusted structural value`. The fix sets only the member
  for the node's kind. Before the fix, nodes with a stored zero already encoded identically, and any nonzero
  value failed. So no output that worked before changes bytes, and no diagnostic text changed. The fix is
  minimal and correct.
- **Red-first.** `red-c01/history/raw/history-red-combined.log` shows ctest exit 8, with 2 of 3 failing on
  `CHECK` assertions at `execution.t.cpp:4002/4004/4101/4105/4125`. There is no compile or fixture
  `REQUIRE` failure. The pre-fix snapshot `b5b9a3fb…` is verbatim identity output. The red-time hash of
  `execution.t.cpp` (`250867b8…`) equals today's, so the tests did not change after red. H3 is honestly
  marked as a guard that passes unfixed and is proven by the H3a/H3b mutant kills.
- **`c02_off_boundary`.** No re-pin is required because `runtime.cpp` and the embedded Lua are untouched.
- **Gates (run 4).** Every `.exit` is 0. ctest reports `100% tests passed out of 1854` for macOS Debug and
  Release and for Linux arm64 Debug and Release. `rewrite-check.py gate` reports "Executed 1854 registered
  tests; zero skips" for both presets. The no-change builds print `ninja: no work to do.` The Python suites
  ran `Ran 108 tests … OK`, reported separately from ctest. The src diff has zero deleted lines, so no C++
  test was removed. Against the RED-stage support snapshot, `comm -23` of `def test_*` names is empty for
  all four Python modules.
- **Quota live runs.** `raw/quota-{combined,split,regroup}-c01/result.json` all report
  `status: accepted`. The reviewer compared the original and replayed stdout byte for byte for the
  combined run: `activate` 296 B, `claim` 9131 B, `check` 1692 B and `verdict` 408 B, all identical with
  exit 0. The check counter has 3 lines for 3 check runs. The verdict actor is `review_session` and the
  builder actor is `builder_session`. The verdict targets equal `final_targets`. `history --event 6`
  (`check_recorded`) now exits 0, and its integer/boolean evidence fields are typed correctly. The driver
  has no SQLite access. The wrong-target control (exit 0, `historical`, one event appended) matches
  `execution-contract-r18.md` R18-3: "Mismatched targets or policy are not refusals". The corrected
  expectation is contract-backed.
- **22 mutants.** Every listed mutant is KILLED in `probes/PROBES.md`, and each `failing tests` entry names
  the new fixture. Spot checks of the raw logs (M-I12c, M-E17, superseded-1 M-E6) show the applied diff,
  the source hash equal to the current file, `restored … True` and the named killing test. The mutants of
  the re-probe match the first-probe survivors in meaning (I10, I3b and I24 have identical hunks, and the
  I13b/I30c variants are equivalent). No survivor was relabelled. The three new survivors (M-I13b2, S2b/S2c
  and L3b) were closed by named fixtures and re-killed. A6 is green with `PLANAR_CANDIDATE` and fails,
  without skipping, when it is unset.
- **Driver red-first (D39).** All 16 `red-c01/driver/*` records show exit 1 with an `AssertionError`, and
  `validate_red_record` accepted each one.
- **Hash-pin rule.** The record and the evidence README pin only `src` and support files plus the
  candidate binary. No pin of a record, review, REPORT or the roadmap was found.
- **A24 and matrix closure** are explicitly left to R19-ACCEPT-B.

## Findings

### R19-ACCEPT-A-C01-R1 (blocking): the faulted CMake rerun and the remaining live steps were not executed

The C01 acceptance criteria require the following. The faulted CMake rerun must show each injected fault
with its observed recovery, then a rejecting verdict followed by repair, then a max_rounds/blocker exercise,
then final approval, with no re-execution on replay and no ledger writes from inspection. Scope item 2 also
requires the mini-fault run and the `limits`, `packet`, `index` and `scan` steps.

The round's own acceptance table marks the faulted rerun and `limits/packet/index/scan/verify-redaction`
as **not met**. `tests/native/r19-accept-evidence/runs/` holds only the defect-run index, `limits/` is
empty, and no `runs/cmake-rerun/index.json` exists. That file is the proof that invariant L20 requires.
The 25 `FaultLiveTest` tests drive the mini fixture inside the gate, but they leave no retained per-action
raw output, replay comparison or rerun index. They do not substitute for the run the criterion names.
Carrying the work forward is an honest choice, but the criterion is unmet, so this corrective cannot be
approved as meeting R19-ACCEPT-A.

Fix: run the mini-fault run and the heavy faulted CMake rerun (`run_heavy.py`, then
`run_acceptance.py rerun-index`) against fresh scratch ledgers with the recorded candidate. Then run
`limits`, `packet`, `index`, `scan` and `verify-redaction`, keep raw output per action, and record the results.
Alternatively, the coordinator can obtain an explicit operator decision that moves these items to
R19-ACCEPT-B, and amend the acceptance criteria to match.

### R19-ACCEPT-A-C01-R2 (blocking): committable evidence contains host paths, and the redaction gate never checked it

Gate command 13 ran `redact-local-paths.py check` on exactly 23 paths (`raw/run4/13-redact-files.txt`):
the two `src` files, the round record and the 20 support files. It covered no evidence file. The record
nevertheless marks "no host paths in committed evidence" as met. The evidence the record lists for commit
(line 253 onward) contains real host paths:

- `tests/native/r19-accept-a-evidence/red-c01/history/RED.md:9` and `red-c01/support/RED.md:10` contain
  the absolute path of the workflow identity script under the user's home directory.
- `red-c01/driver/FaultLiveTest.test_run_scenario_aborts_before_action_not_listed_as_legal/RED.md:8`
  contains a `/var/folders/...` temporary path.
- `tests/native/r19-accept-a-c01-evidence/inventory.txt:1` contains the absolute repository build path.
- `tests/native/r19-accept-a-c01-evidence/red/run_red.sh:3` contains `cd` to the absolute repository
  path.
- The record lists `r19-accept-a-c01-evidence/raw/run1 to run4` for commit. That contradicts its own rule
  "never raw logs", and 81 files under that `raw/` tree contain host paths.
- The P6 README (`r19-accept-evidence/README.md`, "Exact commit path list") says the whole
  `tests/native/r19-accept-a-evidence/` tree is never committed. The C01 record lists
  `r19-accept-a-evidence/red-c01/` for commit. The two commit lists conflict.

Fix: produce one exact committable path list that excludes every `raw/` tree and every superseded copy
that contains host paths. Redact or regenerate the listed prose and data files through the standard tool,
and run the redaction check on exactly that list. Record the command, its exit status and the file list.

### R19-ACCEPT-A-C01-R3 (blocking): the fault-path invariant rows have no probe outcome

The reviewer was asked to confirm that each row's listed test is killed by its mutant. That cannot be
confirmed for most of the table. `PROBES.md` and the record mark these rows "not probed (Skipped: live
driver row needs a live scenario)": L2, L6, L7, L9, L10, L12 to L19, Q1 to Q19 (apart from the quota rows
re-run as L1a to L5 and X-Q14a) and F1 to F25. L11 is the only fault row that was probed.

The skip reason does not hold. The gate itself ran `FaultLiveTest` live in about 8 minutes (Python suite
total 495 s), so these rows are probeable on the same harness. The D39 red records show that 11 of the
`FaultLiveTest` tests can fail on a defective driver. F1 to F6, F8 to F10 and the remaining fault rows have
neither a red record nor a kill. With R1 open, these unprobed tests are the only coverage of the
faulted-rerun behaviours (check-recover, restart, takeover fencing, rejection then repair, max_rounds
blocker, no re-execution on replay, no inspection writes), and none of it is shown able to fail.

Fix: run the prober over the F, L and Q rows that are still unprobed, in a scratch copy, and record each
row's kill with its named test. A survivor stays a survivor and gets a negative fixture. Probe the heavy-run
rows after R1.

### R19-ACCEPT-A-C01-A1 (advisory): the committable REPORT contains false statements

`r19-accept-a-c01-evidence/REPORT.md` says three things that are false. Section 4 still says redaction
"fails on one test fixture", which was the run 1 outcome. Line 153 and line 194 say the mini-fault and
heavy CMake runs "are kept as evidence", and no such directory exists. The round record flags both
statements and supersedes them. If the REPORT is committed as it stands, add a superseding note to the
evidence README so that the committed tree does not carry the false claims without correction.

### R19-ACCEPT-A-C01-A2 (advisory): the G1 capture is not where the invariant puts it

Invariant G1 requires before and after name lists under
`tests/native/r19-accept-evidence/gate/test-names/{before,after}/`. That directory does not exist, and the
record says the `comm -23` check was done manually against run 1. This reviewer's own check found no
removed or renamed test (see above). The missing capture is a record gap, not a defect.

### R19-ACCEPT-A-C01-A3 (advisory): the gate did not use the suites command built for it

Invariant L22 specifies `run_acceptance.py suites` as the gate's Python step. Gate run 4 used
`python3 -m unittest discover` instead. The counts were reported (108 tests, five classes), so scope item 4
is met through "explicitly run". The purpose-built per-module refusal command was implemented and probed
but never used in the gate.

### R19-ACCEPT-A-C01-A4 (advisory): bytecode was written into the support tree

`tests/native/r19-accept-support/__pycache__/` exists with five `.pyc` files written between 09:11 and
15:03. Invariant I35 and the gate environment say this should not happen. The directory is ignored by
`.gitignore`, so nothing reaches a commit. A run without `-B` or `PYTHONDONTWRITEBYTECODE` wrote it, which
indicates the "never writes bytecode" property was not upheld during the round.

### R19-ACCEPT-A-C01-A5 (advisory): the record's text and fixture reuse disagree

Objective item 1 still says the coder must author a quota fixture. Decision D2 instead reused
`fixtures/render/quota/` unmodified, and rows Q1 and Q11 reflect that reuse. The P6 README asked the
coordinator to amend the objective, and the amendment has not been made. In addition, the D4 deviation
(the R19-CMAKE run was scripted rather than a Claude Code session) and the `execution-design.md`
"fresh corrective round of the owning step" reading both go to B for confirmation, as the record says.

## Limits of this review

- The reviewer did not re-run builds, ctest, the Python suites, the Linux container gate or any mutant.
  Those results were read from the retained raw logs.
- Replay byte-identity was checked directly only for the four combined-run replays. The split and regroup
  runs were checked through `result.json` flags.
- The probe raw logs were spot-checked (3 of 37 re-run logs plus the first-probe comparison of 5
  mutants), not read in full.
- The contract citation was verified for the wrong-target change only. The other REPORT section 6
  citations, including D39's driver corrections, were checked only as far as their red records.
