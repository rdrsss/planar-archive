# RED: QuotaLiveTest.test_wrong_target_verdict_is_refused_and_ledger_unchanged

Round R19-ACCEPT-A-C01, package PKG-F, stage RED-driver (D39). Recorded before the fix on the pre-fix support tree.

- Command (raw/command.txt): `PYTHONDONTWRITEBYTECODE=1 PLANAR_CANDIDATE=<scratch-root>/r19-candidate-c01-rev2/export/build/rewrite-debug/src/cmd/planar/planar_authoring_stack HOME=<scratch-root>/r19-c01-wave4/red/home-04 PLANAR_DB=<scratch-root>/r19-c01-wave4/red/home-04/planar.db python3.15 -B -m unittest -v quota_live_test.QuotaLiveTest.test_wrong_target_verdict_is_refused_and_ledger_unchanged`
- Exit: 1
- Failure marker: FAIL: test_wrong_target_verdict_is_refused_and_ledger_unchanged
- Assertion: `AssertionError: 0 == 0`
- Summary: `Ran 1 test in 10.947s`
- Support tree combined sha256 (raw/identity-support.txt, verbatim row): `7cbadf5e190627978a86ac4610030977d66b5d4f91a228e2a6ddd1bc2524b16f`
- validate_red_record (raw/validate_red_record.txt): `validate_red_record: accepted`
- Compile or import error in raw log: none (grep of ImportError, ModuleNotFoundError, SyntaxError and `error: ` found no line).

## Diagnosis

The candidate records a wrong-target approval as historical with exit 0 and appends one verdict event (R18-3, execution-contract-r18.md:144: mismatched targets are not refusals; execution-design.md A06). The test expected a refusal and an unchanged ledger. Defect class (b): test expectation. The worker-token refusal (fenced, exit 3) is unaffected.

Product code: none changed (no src edit). Raw logs and identity output are kept under raw/ and never deleted.
