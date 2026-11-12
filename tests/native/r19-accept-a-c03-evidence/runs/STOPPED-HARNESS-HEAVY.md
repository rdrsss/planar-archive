# C03 heavy run: completed, but run-level counter equality NOT MET (harness)

The heavy faulted CMake run finished: exit 0 (c03-heavy.DONE), `result.json` has `trajectory.valid` true,
no ABORTED.json, no STOPPED file from the driver, first completed check passed its boundary row,
replays byte identical, takeover/fence/verdict/load steps as scripted. Raw logs are retained under
`tests/native/r19-accept-evidence/raw/cmake-rerun-heavy/`. `runs/cmake-rerun/index.json` is NOT written.

## Finding (harness, not product)

`result.json`: `counter_lines` 7, `intended_executions` 8. `run_acceptance.py rerun-index` requires
`counter_lines == intended_executions` (run_acceptance.py:709-711), so L20 would refuse this result.

Cause: `fault_driver.py:982` computes `intended_executions = check_rounds * checks_per_round` (4 x 2 = 8),
but check-1 is the interrupted round: the debug check was killed mid-run and the release check never
started. Attempt folders in `evidence/checks`: debug 4 (check-1 killed, check-2, 3, 4), release 3
(check-2, 3, 4) = 7, which is exactly what ran. The mini scenario has one check per round, so the
formula was never exercised against an interrupted multi-check round.

## Proposal (red-first, support code, not edited here)

Test: a cmake-mode (2 checks) driver unit test where check_rounds counts an interrupted round whose
second check never ran; expect intended_executions == 7 (completed rounds x checks_per_round + attempts
of the interrupted round actually started), or have the validator accept the interrupted-round shortfall.
Needs a coordinator decision on which side changes; fault_driver.py/run_heavy.py are otherwise final (C3-H11).
