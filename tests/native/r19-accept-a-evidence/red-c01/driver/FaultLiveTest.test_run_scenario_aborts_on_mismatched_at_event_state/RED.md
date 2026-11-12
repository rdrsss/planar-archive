# RED: FaultLiveTest.test_run_scenario_aborts_on_mismatched_at_event_state

Round R19-ACCEPT-A-C01, package PKG-F, stage RED-driver (D39). Recorded before the fix on the pre-fix support tree.

- Command (raw/command.txt): `PYTHONDONTWRITEBYTECODE=1 PLANAR_CANDIDATE=<scratch-root>/r19-candidate-c01-rev2/export/build/rewrite-debug/src/cmd/planar/planar_authoring_stack HOME=<scratch-root>/r19-c01-wave4/red/home-10 PLANAR_DB=<scratch-root>/r19-c01-wave4/red/home-10/planar.db python3.15 -B -m unittest -v fault_live_test.FaultLiveTest.test_run_scenario_aborts_on_mismatched_at_event_state`
- Exit: 1
- Failure marker: FAIL: test_run_scenario_aborts_on_mismatched_at_event_state
- Assertion: `AssertionError: AcceptError not raised`
- Summary: `Ran 1 test in 3.903s`
- Support tree combined sha256 (raw/identity-support.txt, verbatim row): `7cbadf5e190627978a86ac4610030977d66b5d4f91a228e2a6ddd1bc2524b16f`
- validate_red_record (raw/validate_red_record.txt): `validate_red_record: accepted`
- Compile or import error in raw log: none (grep of ImportError, ModuleNotFoundError, SyntaxError and `error: ` found no line).

## Diagnosis

scenario() defaults base to plain. The first completed check is withheld, so fault_driver.py run() returns at stopped_result before the at-event call site (fault_driver.py, the response2 outcome check). The cmake configuration is composed (fault/cmake_rerun/run_heavy.py build_config). Defect class (b): test configuration.

Product code: none changed (no src edit). Raw logs and identity output are kept under raw/ and never deleted.
