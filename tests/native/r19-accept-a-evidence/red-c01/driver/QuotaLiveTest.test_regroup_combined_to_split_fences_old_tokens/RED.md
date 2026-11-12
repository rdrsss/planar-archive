# RED: QuotaLiveTest.test_regroup_combined_to_split_fences_old_tokens

Round R19-ACCEPT-A-C01, package PKG-F, stage RED-driver (D39). Recorded before the fix on the pre-fix support tree.

- Command (raw/command.txt): `PYTHONDONTWRITEBYTECODE=1 PLANAR_CANDIDATE=<scratch-root>/r19-candidate-c01-rev2/export/build/rewrite-debug/src/cmd/planar/planar_authoring_stack HOME=<scratch-root>/r19-c01-wave4/red/home-05 PLANAR_DB=<scratch-root>/r19-c01-wave4/red/home-05/planar.db python3.15 -B -m unittest -v quota_live_test.QuotaLiveTest.test_regroup_combined_to_split_fences_old_tokens`
- Exit: 1
- Failure marker: FAIL: test_regroup_combined_to_split_fences_old_tokens
- Assertion: `AssertionError: 1 not greater than 1`
- Summary: `Ran 1 test in 12.255s`
- Support tree combined sha256 (raw/identity-support.txt, verbatim row): `7cbadf5e190627978a86ac4610030977d66b5d4f91a228e2a6ddd1bc2524b16f`
- validate_red_record (raw/validate_red_record.txt): `validate_red_record: accepted`
- Compile or import error in raw log: none (grep of ImportError, ModuleNotFoundError, SyntaxError and `error: ` found no line).

## Diagnosis

After regroup the split reviewer is the first generation of its own (job, role) lineage (execution-contract.md:820, M1), so generation ordinal 1; the combined reviewer also had ordinal 1, so the test compared ordinals across jobs. quota_driver.py regroup() also sets activated True as a constant. Defect class (b): test expectation and driver field.

Product code: none changed (no src edit). Raw logs and identity output are kept under raw/ and never deleted.
