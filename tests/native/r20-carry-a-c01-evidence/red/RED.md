# R20-CARRY-A-C01 red check

Red-time identity (covers tracked and untracked bytes):
- HEAD: dec6f178daf10e2a1219ae56dc71485d42d839ca
- sha256 candidate.diff (git diff HEAD -- src tests/native): 9a891be59df3f260098733b10c70b240542b09fa3b05f923f6dfb3bf66635355
- sha256 SHA256SUMS: 2eda29c05269992df0b3e17affa68237378b1d8c8d8e1f556ed3673de3fad943
- Snapshot: red/snapshot/ (candidate.diff, untracked/ copies of src and tests/native/*-support untracked files, SHA256SUMS)

Build: cmake --build build/rewrite-debug --target planar_tests -> exit 0 (raw/build.log).
Each test: ctest --test-dir build/rewrite-debug -R '^replacement\.r20\.<name>$' --no-tests=error --output-on-failure

| test | invariant | expectRed | outcome | failing line |
|---|---|---|---|---|
| replacement.r20.timeout_headroom_v2_allocation | C1 | true | failed-assertion | AssertionError: TIMEOUT 120 < 1680 (timeout_headroom.py:37) |
| replacement.r20.timeout_headroom_v2_allocation | C1b | false | failed-assertion (same run) | same |
| replacement.r20.timeout_headroom_above4096 | C2 | true | failed-assertion | AssertionError: TIMEOUT 120 < 840 (timeout_headroom.py:37) |
| replacement.r20.timeout_headroom_above4096 | C2b | false | failed-assertion (same run) | same |
| replacement.r20.timeout_names_registered | C3 | false | passed | |
| replacement.r20.configure_rebuilds_on_content_change | C13 | false | passed | |
| C17 | | true | failed-assertion (the two headroom tests above) | |

Other invariants (C4-C12, C14-C16, C18-C22) are evidence or gate rows with no test in this red run; not executed.
Raw logs: red/raw/{build,timeout_headroom_v2_allocation,timeout_headroom_above4096,timeout_names_registered,configure_rebuilds_on_content_change}.log
