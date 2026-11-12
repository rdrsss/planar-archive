# R19-ACCEPT-A evidence (package P6)

**A1 superseding note (R19-ACCEPT-A-C03 final heavy result; first written in C02).** Section 4 row L21 (line 102) of
`tests/native/r19-accept-a-c01-evidence/REPORT.md`, and the statements at its lines 153 and 194, say that the mini-fault
and heavy faulted runs are kept as evidence under this directory and that `limits`, `packet`, `index`, `scan` and
`verify-redaction` are recorded. Those statements are superseded; REPORT.md is not edited. The mini-fault run is recorded
under `runs/mini-fault/` (`rerun-index` exit 0) with raw output under `raw/mini-fault-c02/`. The C03 heavy faulted run completed
(exit 0, `trajectory.valid` true, no ABORTED.json; raw and `result.json` under `raw/cmake-rerun-heavy/`), but its run-level counter
check is NOT MET (`counter_lines` 7, `intended_executions` 8; `tests/native/r19-accept-a-c03-evidence/runs/STOPPED-HARNESS-HEAVY.md`),
so `runs/cmake-rerun/index.json` was NOT written and the post-run steps (`limits`, `packet`, index dry run, `index`, `scan`,
`verify-redaction`) were NOT executed: `limits/` is empty and no `index/` exists. Final status, identity, gate counts, probe results
and the met/not-met tables are in the C02 and C03 sections of `docs/vernacular/rounds/R19-ACCEPT-A.md`; any statement below about those steps is superseded.
`gate/test-names/after/py-worktree.txt` (132 ids) predates the C02 repairs and lacks two tests; the C02 regenerated list had 134 ids.
For C03 the Python test count is 149 (15 added, none removed; see the C03 section); the C03 lists are under
`tests/native/r19-accept-a-c03-evidence/gate/test-names/` and `tests/native/r19-accept-a-c03-evidence/README.md` explains which are stale.

Status of this README: it describes the P6 stop point of R19-ACCEPT-A on the PRE-FIX candidate. Every
identity hash, candidate hash, test count and "not executed" statement below belongs to P6 and is superseded
by the final "R19-ACCEPT-A-C03 result and record sync" section of `docs/vernacular/rounds/R19-ACCEPT-A.md` (the C01 and C02 sections hold
the figures of their own rounds), which has the final candidate identity, gate counts, probe results and the met/not-met table. Since P6: the defect was fixed in
`src/engine/ledger/storage.cpp`, the candidate was rebuilt and re-recorded (`gate/candidate/candidate.json`;
the P6 record is kept under `gate/candidate/superseded-1/`), and the quota combined, split and regroup runs
completed (`raw/quota-*-c01/`, one `result.json` each). Still not run, and not claimed: mini-fault, the heavy
faulted CMake rerun, `limits`, `packet`, `index`, `scan` and `verify-redaction`; `runs/` holds only the
defect-run index and `limits/` is empty.

P6 status (historical): STOPPED. A product defect was exposed by the quota combined run (D17). The wave stops here: no
later run was executed, no acceptance row is closed, and nothing in this directory claims acceptance.
Round record: `docs/vernacular/rounds/R19-ACCEPT-A.md`. Plan: `plan/DECISIONS.md` and `plan/INVARIANTS.md`
(copied byte-for-byte from `tests/native/r19-accept-a-evidence/plan/`, hash list verified).

## Provenance and identity

Starting revision: `91e0df24f2167471e49ec057c7159bce5dff3608` (HEAD at the start of P6; `git status` showed only
the untracked round record and `tests/native/r19-accept-support/`).

Candidate: P0 record `gate/candidate/candidate.json` (D28, scratch export of `git archive 91e0df24`, target
`planar_authoring_stack` only). Recomputed in P6: `shasum -a 256` of the recorded executable is
`98ec8419e7a32993de29019cc02f9a696895aff19316c9d842c41737eec0d706`, equal to the record. The quota run passed
`accept_lib.require_recorded_candidate` before it created its run directory.

Identity tool output (`python3 .../workflows/scripts/identity.py src`, run from the repository root, pasted verbatim):

| identity | value |
|---|---|
| HEAD | `91e0df24f2167471e49ec057c7159bce5dff3608` |
| sha256(git diff --binary HEAD -- src), tracked only | `e3b0c44298fc1c149afbf4c8996fb92427ae41e4649b934ca495991b7852b855` |
| combined sha256 (diff bytes, then each untracked path NUL bytes, sorted), pathspec src, evidence excluded | `e3b0c44298fc1c149afbf4c8996fb92427ae41e4649b934ca495991b7852b855` |

| file | state | sha256 |
|---|---|---|

Identity tool output (`python3 .../workflows/scripts/identity.py tests/native/r19-accept-support`, run from the
repository root, pasted verbatim; the support tree is untracked and is covered by its untracked-file hashes):

| identity | value |
|---|---|
| HEAD | `91e0df24f2167471e49ec057c7159bce5dff3608` |
| sha256(git diff --binary HEAD -- tests/native/r19-accept-support), tracked only | `e3b0c44298fc1c149afbf4c8996fb92427ae41e4649b934ca495991b7852b855` |
| combined sha256 (diff bytes, then each untracked path NUL bytes, sorted), pathspec tests/native/r19-accept-support, evidence excluded | `9ee1c9579677aec7dcbeebe5d81ed74dfb4ee66e362a23f821ea7639b5f317db` |

| file | state | sha256 |
|---|---|---|
| `tests/native/r19-accept-support/accept_lib.py` | untracked | `9809217494dc27ebcfdf4728df83076a303aefb473de79ba6488b8d0099f6f16` |
| `tests/native/r19-accept-support/accept_test.py` | untracked | `ed4d8dd0b4675d8471d6871a5570b0cee340ec89c383150b51f260d765c63459` |
| `tests/native/r19-accept-support/acceptance_cli_test.py` | untracked | `5eb98e86d53bfe3a3bbb6545021db180bc0ccd61eddbf4102588e4c84509505a` |
| `tests/native/r19-accept-support/check_runner.py` | untracked | `1c05d3473f4951d660cd7c1033ba4c0dacf6c12871502c4d066a6cba4b7b9b65` |
| `tests/native/r19-accept-support/cmake_check.py` | untracked | `ba8f20625bda0f3c8fc3c613529e16f6a1f8e0a38b0b261f8341b2fdcbb3f360` |
| `tests/native/r19-accept-support/fault/cmake_rerun/run_heavy.py` | untracked | `d30d1faa5f0c82d1a57eee69b7727dcb6d5bb8e352607506dab2fd729ef5a9ed` |
| `tests/native/r19-accept-support/fault/mini/base/src/unit.txt` | untracked | `d73d5ccc52b8747e2bc253b7267b6110d41021eacd54ff33695d96c0a47fe56a` |
| `tests/native/r19-accept-support/fault/mini/jobs/combined-max1.lua` | untracked | `c7d8b3a33fd7940b6ef56e4ac4923836f527644d8b6952aaa8ae5d22f54e8e39` |
| `tests/native/r19-accept-support/fault/mini/jobs/combined-max2.lua` | untracked | `6d0df6cf79f148664587dcf43e774243cad211e4d26420ce8fe4f955c1281e0e` |
| `tests/native/r19-accept-support/fault/mini/registry.json` | untracked | `eca5ac671b6929f042a839591ff56da72d0afef8f4e9aca5433df7b1119490ca` |
| `tests/native/r19-accept-support/fault/mini/source/src/unit.txt` | untracked | `753f8fe079abf1160eaa98cc40257a00da60a033b2d3730ab679fa0f62a0778c` |
| `tests/native/r19-accept-support/fault/mini/workspace/spec.lua` | untracked | `d35b07e34032202e830cedb66b4d45b5774621f087acc9f8012b1d8edacf07da` |
| `tests/native/r19-accept-support/fault/mini/workspace/workspace.lua` | untracked | `21ac5c6da0e2d27bb4911202d0a364acacc49d39b052fa71d8ca44b68e2d453d` |
| `tests/native/r19-accept-support/fault/sleep_check.py` | untracked | `751414911570978faf4f82d0ee79631ceb41c76dfd2bc4e66d35c41faa51ce57` |
| `tests/native/r19-accept-support/fault_driver.py` | untracked | `4e12a07086ddbf773d8a88c45727d5a89b425bdf0874de7eaed76dd058c8509f` |
| `tests/native/r19-accept-support/fault_live_test.py` | untracked | `5f43287b412be23b8036318c0b85588d436eca1e8b87d38be12e4ff7e0c3b016` |
| `tests/native/r19-accept-support/quota_driver.py` | untracked | `87eef12106c5b3cdd46d16174fd8f0e58b7f25f817267ee14530ac8e388294f0` |
| `tests/native/r19-accept-support/quota_live_test.py` | untracked | `1bb70239dfe4968fc780e775c472b6cefa59873551d30b79389ac5acf07ccecb` |
| `tests/native/r19-accept-support/registry_gen.py` | untracked | `15b79f6704c72e5f7342e45c5185ba042588b03972af590d3580b613314c37ca` |
| `tests/native/r19-accept-support/run_acceptance.py` | untracked | `17de0ed1223fb75efab68598e25634afbdcb0ba543c3124ce899e4b67b26e148` |

Environment for every test and run: `PYTHONDONTWRITEBYTECODE=1`; `HOME` and `PLANAR_DB` in a scratch area
(never `~/.planar`); interpreter Homebrew Python 3.15.2. Driver processes set `HOME` and `PLANAR_DB` themselves.

## Commands executed (P6) and results

Full logs are retained under `raw/unittest/` and `raw/quota-combined-defect1/` (never committed). Compact results are in
`unittest-run/summary.txt`, `gate/prerequisite-status.txt` and `runs/quota-combined-defect1.txt`.

1. `git status --short`, `git rev-parse HEAD`, `git log --oneline -3`: HEAD `91e0df24`.
2. `shasum -a 256` of the recorded candidate executable: matches `candidate.json` (see above).
3. `python3.15 -m unittest -v accept_test` (AcceptLibTest): Ran 35, failures 2 (I25 and I35 tests, see Concerns), exit 1.
4. `python3.15 -m unittest -v acceptance_cli_test` (AcceptanceCliTest): Ran 7, OK, exit 0.
5. `python3.15 -m unittest -v quota_live_test` (QuotaLiveTest, PLANAR_CANDIDATE set): Ran 19, failures 1, errors 17, exit 1.
6. `python3.15 run_acceptance.py quota --candidate <candidate> --record gate/candidate/candidate.json --repo-path <repo>
   --raw-root raw --grouping combined --run-name quota-combined-defect1`: exit 1, `CandidateFault` at
   `history --event 6` (exit 4, `Unsupported trusted structural value`). The run directory is retained.
7. `python3.15 -m unittest -v fault_live_test` (FaultLiveTest, PLANAR_CANDIDATE set): Ran 25, failures 11, exit 1.
8. `cp -R tests/native/r19-accept-a-evidence/plan/. tests/native/r19-accept-evidence/plan/` and `shasum -a 256` of both
   trees: the two hash lists are identical (8 files: DECISIONS.md, INVARIANTS.md, superseded-1/2/3 copies).
9. Control, outside the repository: an init-only ledger (scratch) returned exit 0 for `init`, `history`,
   `history --event 1` and `history --at-event 1`, so the failure is specific to a later event.
10. `python3 .../identity.py` for `src` and `tests/native/r19-accept-support` (outputs above).

## Product defect exposed (D17): `history --event` fails for `check_recorded`

- The quota combined run (retained as `raw/quota-combined-defect1/`, index `runs/quota-combined-defect1.txt`) recorded
  68 actions (step 068 is the failing `history --event 6`). Ledger events 1 to 5 (`ledger_initialized`, `declaration_accepted`, `job_activated`,
  `run_claimed`, `check_reserved`) expand under `history --event`, and events 6 to 10 were not reached because the
  driver stops at the first failing expansion.
- `history --ledger <ledger> --event 6` (event 6 is `check_recorded`, references reservation 1, observation 44,
  completion 1) exits 4 with stdout empty and stderr
  `{"error":{"code":"internal","field":"","message":"Unsupported trusted structural value"},"kind":"error","version":1}`.
  `history --at-event 6` on the same ledger succeeds (exit 0), so the state replay is not the failing path.
- The message is the one raised by `refuse()` in `src/engine/runtime/runtime.cpp` (line 146; `validate()` refuses at
  lines 190 to 216). The root cause is NOT diagnosed here; P6 does not edit `src/**` (D17).
- The same defect explains the 17 `QuotaLiveTest` errors. The `FaultLiveTest` failures are NOT diagnosed; six of them
  expect an `AcceptError` on tampered expansions and see none.
- Per D17, the coordinator decides between a narrow in-round fix (behavioural red-first regression, closed
  diagnostics byte-identical, full Debug gate, candidate rebuilt and re-recorded, and every run repeated) or a
  corrective round.

## Deviations from the dispatch text

- D2 (quota fixture): the round record says no quota fixture exists and the coder must author one. This checkout has
  one: `fixtures/render/quota/` (R03; `implementation.md:163-165`). It is REUSED unmodified and copied into scratch per run.
  The live tests and the one run above read it from the checkout (no historical v4 fixture exists).
  COORDINATOR NOTE: amend the round record's objective item 1 and owned-files wording to say the fixture is reused.
- D4 (live model-driven run): the recorded R19-CMAKE task run (round record `R19-CMAKE.md`, commit `d6c989ed`,
  driver map `rounds/R19-CMAKE-driver-map.md`) is proposed as the one non-gating live run. It was agent-driven through a
  scripted driver whose provenance names a Codex external driver, not a Claude Code session. This deviates from the E13 A
  wording. It is carried to B's review for confirmation; P6 did not re-cite or re-run it.
- Standard redaction tool interpreter: `scripts/redact-local-paths.py` rejects file arguments that follow its options
  under Python 3.9 (`/usr/bin/python3`) and accepts them under Python 3.15. Live and index commands therefore need a
  Python of 3.12 or later on this host. Not in the dispatch text.

## Concerns for the coordinator (test files are read-only for P6; none edited)

- `accept_test.py` (I25 `test_cmake_adapter_rendezvous_after_inventory_before_ctest`, I35
  `test_cmake_adapter_never_writes_bytecode_into_support_tree`): `AcceptLibTest.support_copy()` copies
  `tests/native/r19-cmake-support`, which has no `cmake_check.py`. D1 places that adapter in `r19-accept-support/`, so
  the child exits 2. The test expects the adapter inside the copy. Contract clauses: D1 layout, D36, D31.
- `quota_live_test.py::test_run_grouping_aborts_before_action_not_listed_as_legal`: expects `ABORTED.json` verb `check`
  but observes `orchestrator` (the guidance capture that precedes the check). Contract clause: D37 and the Q19 row.
- Overwritten logs: the first `accept_test` and `acceptance_cli_test` runs under Python 3.9 wrote the same `raw/unittest/`
  names and were overwritten by the Python 3.15 runs. Their results: `accept_test` Ran 35, failures 2 (same two tests);
  `acceptance_cli_test` Ran 7, failures 2 (the index and verify-redaction tests, the 3.9 argparse issue above).

## Carried to R19-ACCEPT-B

- D15 (reviewer-packet wording, objective item 5): carried, not edited. Source lines, quoted from
  `src/stdlib/planar/render.lua` (the `packet` command has not run, so no observed packet is recorded here):
  line 163: `Not executed by render. Commands describe checks; coverage is a driver assertion, unverified. No check receipt exists.`
  line 218: `section("observations",absent(body,"No applicable check or finding driver assertions supplied. No check receipt or approval exists."),true)`
  The observed-packet lines (`limits/` packet record) are NOT captured; they stay open for the packet run.
- D16 (whole-engine captured-preview limit): NOT run in P6 (`limits` subcommand not executed). Carried with its closed
  diagnostic (exit 3, empty stdout, `closed_capture_limit_ok`).
- D23 faults not run in A: claim response loss, takeover while the old session still checks, crash around verdict
  commit, contract change with refresh, disk-full during completion. Each goes to B's matrix map as "covered by focused
  evidence" (C06, C07, C08 and R15-R18 native rounds, acceptance-matrix A09, A10, A22, A23, A26, A29, A33) or "carried
  limit" with the exact citation.
- Heavy run (D35, P7): the CMake rerun (1.5-2 h, D12) was not started; it is not probed beyond F11, F13 and F14-F22.
- UNGUIDED_VERBS (D37): `accept_lib.UNGUIDED_VERBS = frozenset({"load", "activate"})`. The driver also exempts `init`
  (no ledger exists before it), saved-request replays and the fenced probe (see `quota_driver.py` header).
- Quota regroup (D20, Q10), mini-fault (F-rows acceptance run), A24 applicability, matrix closure and the Linux gate: not
  run in A; B owns the closure.

## Limitations

- P6 stopped at the first D17 exposure. Not executed: quota split, quota regroup, mini-fault, `limits`, `packet`,
  `index`, the committed-evidence `scan` gate, and `verify-redaction` of the indexes. No `runs/**` index beyond the
  defect run exists; `TEMP-PATH-REDACTION.json` is the empty D19 form (no temporary redaction was applied).
- `tests/native/path-redaction.json` was not written by P6 (no standard `redact` ran).
- The defect run's raw directory holds real paths (raw is local only and never committed).

## Exact commit path list (superseded by C02 D17 and D19, then by C03)

The single authority for what is committed is the C03 list `tests/native/r19-accept-a-c03-evidence/COMMIT-LIST.txt`; the C02 list is superseded and left unedited. It names: explicit
regular-file paths, one per line, sorted, checked by the D17 exclusion expression and an existence loop. The list this
section used to hold named only P6 outputs and is superseded by the C03 list. Staging is done by the coordinator, each
path once, with `git add -f` for gitignored evidence and `git commit -s`, never `git add -A`. Raw trees, build logs,
PATH files and whole evidence trees are not committed. At commit (2026-11-12) the operator also kept `index/`, `TEMP-PATH-REDACTION.json`
and every other file over 100 KB local; `tests/native/r19-accept-a-c03-evidence/closing/LOCAL-ONLY.txt` records their hashes.

## Files written by P6

- `tests/native/r19-accept-evidence/README.md`
- `tests/native/r19-accept-evidence/plan/` (8 files, copied from `r19-accept-a-evidence/plan/`)
- `tests/native/r19-accept-evidence/gate/prerequisite-status.txt`
- `tests/native/r19-accept-evidence/unittest-run/summary.txt`
- `tests/native/r19-accept-evidence/runs/quota-combined-defect1.txt`
- `tests/native/r19-accept-evidence/limits/` (empty directory)
- `tests/native/r19-accept-evidence/TEMP-PATH-REDACTION.json`
- `tests/native/r19-accept-evidence/raw/unittest/{accept_test,acceptance_cli_test,quota_live_test,fault_live_test}.log`
- `tests/native/r19-accept-evidence/raw/quota-combined-defect1.run-acceptance.{stdout,stderr}`
- `tests/native/r19-accept-evidence/raw/quota-combined-defect1/` (run output created by `run_acceptance.py quota`)
- Not written or modified by P6: `gate/candidate/**`, `tests/native/path-redaction.json`, `tests/native/r19-accept-support/**`,
  `tests/native/r19-cmake-support/**`, `src/**`, `docs/**`.
