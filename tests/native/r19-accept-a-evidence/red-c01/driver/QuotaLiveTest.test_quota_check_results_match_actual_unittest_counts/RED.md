# RED: QuotaLiveTest.test_quota_check_results_match_actual_unittest_counts

Round R19-ACCEPT-A-C01, package PKG-F, stage RED-driver (D39). Recorded before the fix on the pre-fix support tree.

- Command (raw/command.txt): `PYTHONDONTWRITEBYTECODE=1 PLANAR_CANDIDATE=<scratch-root>/r19-candidate-c01-rev2/export/build/rewrite-debug/src/cmd/planar/planar_authoring_stack HOME=<scratch-root>/r19-c01-wave4/red/home-01 PLANAR_DB=<scratch-root>/r19-c01-wave4/red/home-01/planar.db python3.15 -B -m unittest -v quota_live_test.QuotaLiveTest.test_quota_check_results_match_actual_unittest_counts`
- Exit: 1
- Failure marker: FAIL: test_quota_check_results_match_actual_unittest_counts
- Assertion: `AssertionError: unexpectedly None : the raw unittest output must contain a Ran line`
- Summary: `Ran 1 test in 11.077s`
- Support tree combined sha256 (raw/identity-support.txt, verbatim row): `7cbadf5e190627978a86ac4610030977d66b5d4f91a228e2a6ddd1bc2524b16f`
- validate_red_record (raw/validate_red_record.txt): `validate_red_record: accepted`
- Compile or import error in raw log: none (grep of ImportError, ModuleNotFoundError, SyntaxError and `error: ` found no line).

## Diagnosis

The driver stores a placeholder b'' for unittest_output (quota_driver.py check(), unittest_output_retained False). The candidate check document carries only an output record id and byte count (execution-contract-r17.md:408), so the unittest Ran line is never retained. Defect class (b): driver. Fix planned: check_runner --transcript retains each counted execution's output; the driver records it per execution.

Product code: none changed (no src edit). Raw logs and identity output are kept under raw/ and never deleted.
