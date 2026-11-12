# RED: FaultLiveTest.test_run_scenario_aborts_when_counter_increases_on_replay

Round R19-ACCEPT-A-C01, package PKG-F, stage RED-driver (D39). Recorded before the fix on the pre-fix support tree.

- Command (raw/command.txt): `PYTHONDONTWRITEBYTECODE=1 PLANAR_CANDIDATE=<scratch-root>/r19-candidate-c01-rev2/export/build/rewrite-debug/src/cmd/planar/planar_authoring_stack HOME=<scratch-root>/r19-c01-wave4/red/home-13 PLANAR_DB=<scratch-root>/r19-c01-wave4/red/home-13/planar.db python3.15 -B -m unittest -v fault_live_test.FaultLiveTest.test_run_scenario_aborts_when_counter_increases_on_replay`
- Exit: 1
- Failure marker: FAIL: test_run_scenario_aborts_when_counter_increases_on_replay
- Assertion: `AssertionError: AcceptError not raised`
- Summary: `Ran 1 test in 4.101s`
- Support tree combined sha256 (raw/identity-support.txt, verbatim row): `7cbadf5e190627978a86ac4610030977d66b5d4f91a228e2a6ddd1bc2524b16f`
- validate_red_record (raw/validate_red_record.txt): `validate_red_record: accepted`
- Compile or import error in raw log: none (grep of ImportError, ModuleNotFoundError, SyntaxError and `error: ` found no line).

## Diagnosis

Same cause: plain base returns before the counter check in finish(). Defect class (b): test configuration.

Product code: none changed (no src edit). Raw logs and identity output are kept under raw/ and never deleted.
