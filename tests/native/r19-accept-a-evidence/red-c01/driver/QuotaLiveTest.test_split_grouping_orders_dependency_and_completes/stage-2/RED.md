# RED stage 2: QuotaLiveTest.test_split_grouping_orders_dependency_and_completes

Second red for the same existing test (D39). Stage 1 (the early-root count, "2 != 1") was fixed in the test
file; this stage is recorded before the driver edit it motivates (quota_driver.py check(), output_vector).

- Command (raw/command.txt): `PYTHONDONTWRITEBYTECODE=1 PLANAR_CANDIDATE=<scratch-root>/r19-candidate-c01-rev2/export/build/rewrite-debug/src/cmd/planar/planar_authoring_stack HOME=<scratch-root>/red/home-stage2 PLANAR_DB=<scratch-root>/red/home-stage2/planar.db python3.15 -B -m unittest -v quota_live_test.QuotaLiveTest.test_split_grouping_orders_dependency_and_completes` (run from tests/native/r19-accept-support)
- Exit: 1
- Failure marker: FAIL: test_split_grouping_orders_dependency_and_completes
- Assertion: `AssertionError: 'library' not found in ['command']`
- Summary: `Ran 1 test in 11.405s`, `FAILED (failures=1)`
- Support tree combined sha256 (raw/identity-support.txt, run from the repository root): `b727b375d396b80ef44893559c2cd66856cba923fcc831339a546dca59b13060`
- validate_red_record (raw/validate_red_record.txt): `validate_red_record: accepted`
- Compile or import error in raw log: none.
- Identity mis-run kept: raw/identity-support-run-from-support-dir-cwd-error.txt was produced with the support directory as the working directory, so its pathspec matched nothing (combined row e3b0c4...). It is kept and superseded by raw/identity-support.txt.

## Diagnosis

The integration check lists output_vector among its inputs (fixture registry: inputs own_output, output_vector), so its
consumed vector must hold every unit observed in the root (R17 observation units, execution-contract-r17.md:407).
quota_driver.py built output_vector from the owned units only, so the root_cli vector lacked the library unit.
Defect class (b): driver. Fix: output_vector holds every observed unit; targets stay the owned units.

Product code: none changed (no src edit). Raw logs kept under raw/ and never deleted.
