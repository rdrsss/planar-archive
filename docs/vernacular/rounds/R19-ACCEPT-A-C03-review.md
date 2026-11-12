# R19-ACCEPT-A-C03 independent review

Disposition: **request-changes**. The C03 candidate closes the repository half of C02-R1 cleanly and closes C02-R3:

- The schema fixture moved, byte-identical, to a tracked path. Only line 41 of `src/engine/ledger/CMakeLists.txt` changed.
- The clean-export red and green runs are behavioural and hashed.
- All four sites that let the Linux gate include the ignored schema file are gone.
- `run_heavy.py` now exits 3 on a stopped scenario, and that is tested and probed.
- The F11-b/L17 fixture now reaches `finish()` and kills its mutant by name.
- Advisories A1 and A3 are resolved.

The round still does not meet its acceptance criteria, and the record says so honestly. The record states that the round
is NOT closed and does not claim these items:

- The heavy faulted CMake run fails the run-level counter check of `rerun-index`.
- `tests/native/r19-accept-evidence/runs/cmake-rerun/index.json` (L20) does not exist.
- The L21 steps (`limits`, `packet`, `index`, `scan`, `verify-redaction`) were not run.
- The C03 `COMMIT-LIST.txt` and its redaction check do not exist.
- A `.pyc` file remains in the support tree.
- The saved Python after-list is stale.

These gaps need a harness code change with a red-first test, a new heavy run, the L20 and L21 steps, and the commit
list. Record edits alone cannot close them, so `recordsOnly` is false. They do not need an operator decision. The
counter-formula choice belongs to the coordinator, so the disposition is request-changes, not blocked.

The reviewer was not the coder, coordinator or prober. Writes were limited to this file. No checkout file was mutated or
staged. No build, ctest, ledger command, installed or legacy binary, Docker command, or script under `tests/native/` or
`scripts/` was run. The only executed tool was the workflow identity tool, copied to the scratchpad and run from the
repository root. Everything else was read-only:

- `ls`, `find`, `grep`, `sed`, `cmp` and `shasum`
- `git diff`, `git check-ignore`
- inline `python3 -I` that parsed `result.json` and the record

## Candidate identity

Re-run by this reviewer. Both pathspecs used by the record give output equal to the record's "Final candidate identity"
tables byte for byte, including every per-file hash:

| pathspec | tracked sha256 | combined sha256 |
|---|---|---|
| `src` | `d9ca75a13fcf1c3aa11b79ee38fdb5d24d733501deb4caf24c42d1830a6726c0` | `d9ca75a13fcf1c3aa11b79ee38fdb5d24d733501deb4caf24c42d1830a6726c0` |
| `src tests/native/r19-accept-support fixtures scripts tests/acceptance docs/testing.md` | `2daacd67636901cb57039c3c41aa8b083cbbf2bf4761a1fd127439d8c6c8463c` | `5988c6d810da9c9b35b46a3fd57e44ee2d75a8ed23ac984d6b1dbb86d4a98bb2` |

HEAD is `91e0df24f2167471e49ec057c7159bce5dff3608`. Two further checks show the candidate has not moved since the final
gate:

- The identity tool with pathspec `src tests/native` gives output identical to
  `tests/native/r19-accept-a-c03-evidence/raw/run3/identity-after.txt`.
- The whole-tree `git diff --binary HEAD` sha256 equals `raw/run3/whole-diff-sha.txt`.

## What was verified and holds

- **Fixture move (C3-R1a, R1e).**
  - `cmp` of `fixtures/ledger/expected-schema.sql` against `tests/native/r12b-evidence/expected-schema.sql` exits 0, and
    both are 3895 bytes.
  - `git check-ignore` exits 1 for the new path, so it is not ignored. The old copy is still ignored by `.gitignore:37`.
  - The old copy's sha256 today equals `r1/raw/old-copy-before.sha256` and `old-copy-after.sha256`, so it is untouched.
- **`src` scope (C3-R1d, G2).**
  - `git diff --numstat HEAD -- src` gives `CMakeLists.txt 1 1`, `execution.t.cpp 177 0` and `storage.cpp 8 4`.
  - The CMake hunk changes only the `PLANAR_LEDGER_SCHEMA` path.
  - The hashes of `storage.cpp` and `execution.t.cpp` equal the C02 candidate. No diagnostic text under `src` changed.
- **Clean-export red-first (C3-R1b, R1c).** `r1/RED.md` pastes the pre-fix identity, combined sha256
  `2cb15eb83efa6c00d393d1e339deac84d30378231c9bc8c304909e32cec6f418`, for pathspec
  `src fixtures scripts tests/acceptance docs/testing.md`.
  - The export lists are hashed. `grep -zc` for the fixture gives 0 before the fix and 1 after.
  - `raw/red-ctest.exit` is 8. `raw/red-ctest-output.log` shows `format.t.cpp:115: FAILED: REQUIRE_FALSE( expected_sql.empty() )`.
    That is a behavioural failure, not a compile failure.
  - `raw/green-ctest.exit` is 0, with 1 of 1 passed.
  - The final gate repeats the green run in a fresh export: `raw/run3/cleanexport.exit` is 0.
- **Linux context (C3-R1f to R1j, R1q).** In `scripts/linux-gate.sh`:
  - the `write_manifest` append of the old path is removed, so the list is now only `git ls-files -z --cached --others --exclude-standard`;
  - the `is_excluded` special case is removed.

  The dockerignore re-include is removed. The Dockerfile `test -s` now names the tracked fixture. No `r12b-evidence`
  text remains in `scripts/`, `src/`, `tests/acceptance/` or the edited `docs/testing.md` passages. The one hit at
  `docs/testing.md:196` is an unrelated R12 evidence-directory mention. The new `x08_recipe.py` pins check these sites
  and fail on a re-include, a stale mention or a lost evidence exclusion. The record's four-site explanation of the
  earlier pass matches the pre-fix diff lines.
- **`run_heavy.py` exit code (C3-R1l, R1m, R1m-c).**
  - `main` writes `result.json` and then returns 3 unless `trajectory.valid is True`.
  - `HeavyDriverTest` stubs the scenario and asserts `rc == 3`, the written result and the stderr word for a stopped
    result. It asserts 0 for a completed result, and 3 for three invalid-trajectory shapes.
  - Revision 2 probe logs show `C3-R1l-a` (`return 0`) and `C3-R1m-c` (`stopped = not isinstance(trajectory, dict)`)
    each killed by the named test, with byte-restore verified.
  - The test can fail and was red before the fix (`red/raw-2/...stopped.log`, `0 == 0`).
- **Overlay (C3-R1n to R1p).** The overlay tests use a real temporary git repository whose HEAD differs from the
  working tree. `C3-R1p` (`continue` on a missing file) is killed by
  `test_export_overlay_refuses_a_path_missing_from_the_working_tree`.
- **Commit-list check (C3-R2a to R2c).** `cmd_commit_list` in `run_acceptance.py`:
  - refuses missing, unsorted, duplicate, raw, bytecode, log, snapshot, original and unlisted-superseded paths, and
    REPORT pins;
  - adds only a new subcommand.

  `test_commit_list_check_fails_naming_a_missing_path` asserts exit 1 and the path on stderr. Probe `C3-R2a` (`if False:`
  in place of the existence test) is killed by exactly that test (`probes/superseded-1/raw/C3-R2a.log`). The
  `run_acceptance.py` hash in that probe equals the candidate.
- **R3, L17 (C3-R3a, R3c).** The fixture now runs a mini scenario with `apply_repair` wrapped to append a sentinel to
  the instance trace. It asserts `trajectory` is present (so `finish()` ran), that the result trace equals the instance
  trace, and that the trace contains the sentinel.
  - The prober applied the C02 diff, hard-coding the trace in `finish()` (`probes/superseded-1/raw/C3-F11-b.log`). The
    run failed with the sentinel diff, and `killed_by_named` is `test_trace_is_derived_from_executed_steps_in_plain_cmake_run`.
  - The mutated file's original hash `be61aeb6...` and the test file hash `9cd192a8...` equal the candidate, so carrying
    the result into revision 2 is valid.
- **Advisories.**
  - A1: both corrected paths exist (`test -e` of `tests/native/r19-accept-a-evidence/red-c01/history/RED.md`,
    `test -d` of `gate/suites/summary.txt.raw`). The A3 row and line 773 cite them.
  - A3: 25 of 26 F rows in the C02 table carry a "fires through" entry. The 26th, `F11-b/L17-nf`, is killed directly by
    a test assertion. F24 is honestly marked "not named" (a test error, no `AcceptError`).
- **Heavy run content.** All raw logs are retained under `tests/native/r19-accept-evidence/raw/cmake-rerun-heavy/`:
  145 steps, 585 raw files, and `SHASUMS-copy.sha256` byte-equal to `SHASUMS-source.sha256`. This reviewer evaluated
  every condition of `validate_rerun_result` independently against `result.json`.
  - The interrupt, `check-recover`, restart reads and takeover (generation 1 to 2) hold.
  - The fenced old token (exit 3, ledger unchanged) holds.
  - `reject1` admitted a repair, and `reject2` was capped with a `max_rounds` blocker.
  - The load raised `max_rounds` from 1 to 2 and cleared the blocker.
  - The final approve was accepted with targets equal to the latest output.
  - Six replays are byte-identical with an unchanged ledger, and the ledger digest is equal before and after the replays.
  - The recover history entry is a matched `check_interrupted`, and the trajectory is valid.

  **Every condition holds except `counter_lines == intended_executions` (7 against 8).**
- **Gate (spot-checked, not re-run).**
  - Every `.exit` file in `raw/run3/` is 0 except `suites.exit`, which is 1: the refused in-tree candidate path, as
    reported.
  - The logs hold four `100% tests passed out of 1854` lines, two `zero skips` lines, six `ninja: no work to do.` lines
    and `Ran 149 tests`. `ALLDONE` is present.
  - The ctest inventory is unchanged, and no Python test was renamed or removed.

## Findings

### R19-ACCEPT-A-C03-R1 (blocking): the heavy run fails `rerun-index`; L20 and L21 are not done

The C03 heavy run reached every scripted phase, but `result.json` has `counter_lines` 7 and `intended_executions` 8.
`run_acceptance.py:709-711` refuses it, so `runs/cmake-rerun/index.json` was never written. `runs/cmake-rerun/` holds
only the two timeout config files, and none of the L21 steps ran. The acceptance criteria "`index.json` exists (L20)"
and "`limits`, `packet`, `index`, `scan`, `verify-redaction` pass (L21)" are NOT MET. C01-R1 and C02-R1 therefore stay
open for their run-level part.

The reviewer confirms the record's diagnosis from the bytes:

- `fault_driver.py:578` increments `check_rounds` once per `check` request.
- `fault_driver.py:982` multiplies it by `checks_per_round`, which is 2 in cmake mode.
- `evidence/checks/debug/` holds 4 attempt folders. The first is the killed attempt, with no `ctest.xml`, `lint` or
  `source-after.json`.
- `evidence/checks/release/` holds 3 attempt folders, because the interrupted round never started its release check.

Seven executions ran. The formula overstates the interrupted round. The mini scenario has one check per round, so it
cannot exercise this case.

A second point the next run should fix: `run_heavy.py` exited 0 on a result that the acceptance validator refuses. Its
"stopped" predicate is only `trajectory.valid`. The C02 stop-rule requirement ("nonzero on `stopped_result`") is met,
but a zero exit still does not mean the run is acceptable, which is the shape of the C02 defect at a lower level. The
exit status and DONE marker of this heavy run are not retained in the evidence tree either. `STOPPED-HARNESS-HEAVY.md`
cites `c03-heavy.DONE` and exit 0 from the scratch directory, and neither file was copied.

Required to close:

- A coordinator decision on the side that changes: count only the attempts started in the interrupted round (preferred,
  because it keeps the validator strict), or relax the validator.
- A red-first unit test that fails on the current formula, for example a cmake-mode interrupted round whose second check
  never started, expecting 7.
- Preferably, `run_heavy.py` should also run the rerun validation, or at least the counter equality, before it returns
  0. Add a test that fails on a zero exit for a counter mismatch.
- A new heavy run under a new run name. `fault_driver.py` is a heavy-run input (C3-H11), and the failed raw stays.
  Retain its `.exit` and DONE marker in the evidence tree.
- `rerun-index` (L20), then `limits`, `packet`, `index`, `scan` and `verify-redaction` (L21), with command, exit and
  output files.

### R19-ACCEPT-A-C03-R2 (blocking): no C03 `COMMIT-LIST.txt`, so the redaction and pin checks are not on the final list

`tests/native/r19-accept-a-c03-evidence/COMMIT-LIST.txt` and `redaction/` do not exist. The gate's redaction run checked
an ad-hoc `redact-files.txt`, which the record correctly does not count. The tool (`commit-list`) and its missing-path
test are sound and can fail (see above). The criterion is about the list itself: it names exactly existing files, it
passes `commit-list` with no REPORT pins, and `scripts/redact-local-paths.py check` exits 0 on exactly that list. That
criterion is NOT MET.

Build the list after C03-R1, because the new heavy run adds evidence. It must name:

- the moved fixture, the CMake, recipe and doc edits, and `ignored_context_check.sh`;
- the C02 and C03 `REPORT.md` and `PROBES.md`.

It must exclude:

- raw trees;
- the hidden 95 MB export copy at `tests/native/r19-accept-evidence/.raw/cmake-rerun-heavy-export/`. Its `.raw`
  component is already refused by the `raw` rule.

Run `commit-list` and then the redaction check on that exact file. Record command, exit and list.

### R19-ACCEPT-A-C03-R3 (blocking, small): `.pyc` in the support tree, and a stale Python after-list

The dispatch acceptance includes "no `.pyc` in the support tree". `tests/native/r19-accept-support/__pycache__/accept_test.cpython-315.pyc`
exists, with an mtime equal to the last `accept_test.py` edit. A2 asked for the regenerated after-list to be saved.
`gate/test-names/after/py-worktree-c03.txt` holds 148 ids, but the candidate has 149: the repair test is missing. The
record's "computed in this sync, not saved" leaves G1 unreproducible from retained files. Both are cheap, but both are
stated criteria. Remove the bytecode (the coder owns the support tree) and save the 149-id list with the stage-0
generator.

### R19-ACCEPT-A-C03-A1 (advisory): gate scope deviation needs a ruling, not a block

The final gate ran Release ctest and the Docker Linux gate, and pruned with `docker builder prune -af --filter
'description~=home/planar'`. That goes against C3-D17, which reads the 2026-11-11 operator answer as Debug-only before the
final gate, with plain `prune -f` only. The record flags this as a deviation and does not hide it. The extra runs passed,
1854 of 1854 each, with no-work builds, equal manifests and 0 B freed by the prune. They are additive evidence and do
not replace the R19-ACCEPT-B final gate. The coordinator should record whether the operator accepts them. They do not
affect this disposition.

### R19-ACCEPT-A-C03-A2 (advisory): the REPORT-pin rule of `commit-list` is narrower than the hash-pin rule

`cmd_commit_list` flags a 64-hex string only on a line that also contains `REPORT`. The hash-pin rule covers records,
reviews, REPORTs and the roadmap. The check would pass:

- a pin of a review or of the roadmap;
- a REPORT name and its hash on adjacent lines.

The reviewer grepped the current record and found no such pins: every 64-hex string there is a candidate, identity or
`src` hash. Either widen the rule, or state the limit next to C3-R2c and keep a manual grep step in PKG-LIST.

### R19-ACCEPT-A-C03-A3 (advisory): the edited-files list omits the hidden heavy export copy

"Files edited in this round" names `raw/cmake-rerun-heavy/` and the timeout files under `tests/native/r19-accept-evidence/`.
It does not name `.raw/cmake-rerun-heavy-export/` (95 MB, written at 05:05 with the heavy copy), and the record does not
say why it is kept. Name it, and keep it out of the commit list. The dispatch asked to keep the scratch small, and 127 MB
of heavy-run data now sit in the evidence tree.

## Invariant table check

`plan/INVARIANTS.md` revision 4 has 67 `C3-` rows. The record copies all of them and adds seven rows that the plan lacks:
`C3-R1m-c`, `C3-SF1` to `C3-SF4`, `C3-HC` and `C3-PYC`. Every C02 finding and every C03 acceptance criterion has a row:

- the C02 findings R1, R2, R3 and A1 to A3;
- the heavy counter, as `C3-HC` in the record only;
- the bytecode, as `C3-G5` and `C3-PYC`.

The mutants the reviewer checked really break their invariant and are killed by the listed test, with byte-restore
logged: R1l-a, R1m-c, R1p, R2a and F11-b. The reviewer found no hollow row among the probed ones.

The heavy-run rows have limits:

- `C3-H1` and `C3-S1` cite a stubbed unit test. They are proven for the exit code, but not for a live run.
- `C3-H2` to `C3-H9` are observed by mini or plain `FaultLiveTest` runs, not by the heavy run. Their heavy-run proof was
  meant to be `rerun-index`, which refused.

So these rows are not hollow, but they are not met for the heavy run (C03-R1). The red-first evidence is behavioural
where it is claimed. `SCHEMA612` is correctly declared not red in the checkout, because the ignored copy existed, and red
only in the clean export.

## Limits of this review

- No build, ctest, Python suite, Linux gate or mutation was re-run. Gate and probe outcomes rest on the retained logs,
  spot-checked as described above.
- The heavy-run check evaluated the validator conditions with the reviewer's own read-only parser. It did not run
  `run_acceptance.py`.
- Diagnostics byte-identity rests on `src` having no change other than the CMake path, plus the C02 review of
  `storage.cpp`.
- x86_64 Linux is uncovered, as recorded.
