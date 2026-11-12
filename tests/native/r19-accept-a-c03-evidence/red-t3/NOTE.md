# red-t3: R3 extended test body (PKG-T3)

Test: FaultLiveTest.test_trace_is_derived_from_executed_steps_in_plain_cmake_run (name and plain-run assertions kept; docstring updated, 'Kills F11-b' count 1).
Command (cwd tests/native/r19-accept-support): PLANAR_CANDIDATE=<build/rewrite-debug/src/cmd/planar/planar_authoring_stack, local debug build, NOT the sha256-recorded D28 candidate> python3 -B -m unittest -v fault_live_test.FaultLiveTest.test_trace_is_derived_from_executed_steps_in_plain_cmake_run
Output: test_trace_run.log (Ran 1, OK) against the current unmutated fault_driver.
Mutant it should kill (C02 F11-b / L17): fault_driver._Scenario.finish sets result["trace"] to the hard-coded 15-entry list. Then the mini run's result trace lacks 'repair-sentinel' and differs from instance trace; the assertEqual(mini_trace, instances[0].trace) / assertIn sentinel fail. No mutation was applied here (prober only).
Limit: no red run was produced by this coder (never mutates); the pre-existing finish() implementation is already correct, so this test is green-first by design.
