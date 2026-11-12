# RED: FaultLiveTest.test_scenario_driver_is_config_parameterized

Round R19-ACCEPT-A-C01, package PKG-F, stage RED-driver (D39). Recorded before the fix on the pre-fix support tree.

- Command (raw/command.txt): `PYTHONDONTWRITEBYTECODE=1 PLANAR_CANDIDATE=<scratch-root>/r19-candidate-c01-rev2/export/build/rewrite-debug/src/cmd/planar/planar_authoring_stack HOME=<scratch-root>/r19-c01-wave4/red/home-16 PLANAR_DB=<scratch-root>/r19-c01-wave4/red/home-16/planar.db python3.15 -B -m unittest -v fault_live_test.FaultLiveTest.test_scenario_driver_is_config_parameterized`
- Exit: 1
- Failure marker: FAIL: test_scenario_driver_is_config_parameterized
- Assertion: `AssertionError: Lists differ: ['act[88 chars]ck', 'verdict', 'load', 'claim', 'check', 'verdict', 'verdict'] != ['act[88 chars]ck', 'verdict', 'load', 'check', 'verdict']`
- Summary: `Ran 1 test in 14.272s`
- Support tree combined sha256 (raw/identity-support.txt, verbatim row): `7cbadf5e190627978a86ac4610030977d66b5d4f91a228e2a6ddd1bc2524b16f`
- validate_red_record (raw/validate_red_record.txt): `validate_red_record: accepted`
- Compile or import error in raw log: none (grep of ImportError, ModuleNotFoundError, SyntaxError and `error: ` found no line).

## Diagnosis

After the load the builder is re-claimed (claim-builder-3, the load made it stale) and the historical stale verdict is recorded and traced as a state change (R18-3). expected_trace omitted both. Defect class (b): test expectation.

Product code: none changed (no src edit). Raw logs and identity output are kept under raw/ and never deleted.
