# RED: FaultLiveTest.test_final_approval_after_faults_is_exact_target

Round R19-ACCEPT-A-C01, package PKG-F, stage RED-driver (D39). Recorded before the fix on the pre-fix support tree.

- Command (raw/command.txt): `PYTHONDONTWRITEBYTECODE=1 PLANAR_CANDIDATE=<scratch-root>/r19-candidate-c01-rev2/export/build/rewrite-debug/src/cmd/planar/planar_authoring_stack HOME=<scratch-root>/r19-c01-wave4/red/home-06 PLANAR_DB=<scratch-root>/r19-c01-wave4/red/home-06/planar.db python3.15 -B -m unittest -v fault_live_test.FaultLiveTest.test_final_approval_after_faults_is_exact_target`
- Exit: 1
- Failure marker: FAIL: test_final_approval_after_faults_is_exact_target
- Assertion: `AssertionError: 0 == 0 : an approval built from an earlier output is refused`
- Summary: `Ran 1 test in 15.244s`
- Support tree combined sha256 (raw/identity-support.txt, verbatim row): `7cbadf5e190627978a86ac4610030977d66b5d4f91a228e2a6ddd1bc2524b16f`
- validate_red_record (raw/validate_red_record.txt): `validate_red_record: accepted`
- Compile or import error in raw log: none (grep of ImportError, ModuleNotFoundError, SyntaxError and `error: ` found no line).

## Diagnosis

A verdict built from an earlier output is recorded historical with exit 0 (R18-3, execution-contract-r18.md:144; the approval is not refused and approves nothing). The test expected a refusal and an unchanged ledger. Defect class (b): test expectation.

Product code: none changed (no src edit). Raw logs and identity output are kept under raw/ and never deleted.
