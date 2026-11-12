# RED: QuotaLiveTest.test_split_grouping_orders_dependency_and_completes

Round R19-ACCEPT-A-C01, package PKG-F, stage RED-driver (D39). Recorded before the fix on the pre-fix support tree.

- Command (raw/command.txt): `PYTHONDONTWRITEBYTECODE=1 PLANAR_CANDIDATE=<scratch-root>/r19-candidate-c01-rev2/export/build/rewrite-debug/src/cmd/planar/planar_authoring_stack HOME=<scratch-root>/r19-c01-wave4/red/home-03 PLANAR_DB=<scratch-root>/r19-c01-wave4/red/home-03/planar.db python3.15 -B -m unittest -v quota_live_test.QuotaLiveTest.test_split_grouping_orders_dependency_and_completes`
- Exit: 1
- Failure marker: FAIL: test_split_grouping_orders_dependency_and_completes
- Assertion: `AssertionError: 2 != 1`
- Summary: `Ran 1 test in 10.948s`
- Support tree combined sha256 (raw/identity-support.txt, verbatim row): `7cbadf5e190627978a86ac4610030977d66b5d4f91a228e2a6ddd1bc2524b16f`
- validate_red_record (raw/validate_red_record.txt): `validate_red_record: accepted`
- Compile or import error in raw log: none (grep of ImportError, ModuleNotFoundError, SyntaxError and `error: ` found no line).

## Diagnosis

The early root executes both registered checks (contract and integration), each withheld with complete_prerequisite, so the driver emits one entry per check name (execution-contract-r17.md:408 per work and check rows). The test assumed one entry per root. Defect class (b): test expectation. The root_cli entries are two per name for the same reason.

Product code: none changed (no src edit). Raw logs and identity output are kept under raw/ and never deleted.

Stage 2 (second defect in the same test, driver output_vector): see stage-2/RED.md in this directory.
