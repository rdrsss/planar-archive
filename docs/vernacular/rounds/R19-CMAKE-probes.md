# R19-CMAKE verification-input sensitivity controls

Completed: **12 faulty input views rejected as intended, bracketed by two healthy
full-verifier controls with zero errors**. This is a bounded sensitivity result,
not code approval or native compiled-fault evidence.

A fresh ordinary prober substitutes for the unavailable prescribed Claude
model. The checkout is shared. This prober wrote only this record and ignored
`tests/native/r19-cmake-evidence/probes/` outputs. No candidate CMake, product
source, build output, historical evidence, ledger or operator data was changed.

## Actual execution and input identity

The executed command is retained in `probes/attempt-1794267151560956000/EXECUTION.json`
and exited 0. It invokes the actual `verify.py` function fourteen times against
the frozen new-layout Debug target metadata, generated sources, Ninja graph and
source files. One input at a time is replaced only inside the prober process.
The healthy-before call took 74.20 seconds; all fourteen calls totaled 994.86
seconds. The unchanged verifier source SHA-256 is
`f989b6a1f083857856be43f7303453b328ff7b54bbb831916f479881a8ff144a`.

CTest discovery is the exact raw listing retained by the actual green Debug
check beside `checks/debug/1794266799601479000-58896/EQUIVALENCE.json`. That
listing has 1,844 registered names and SHA-256
`d6632efb5eced665f97173cc51b42636c1abebc40c1aa6beb95b588fe52c3a3b`; the
actual green equivalence result SHA is
`1aa061e8cc6e1864d09c34e9a6e29f6d1b9e4e24d1da323b8887bc9422064dda`.
No CTest, configure, build or native test subprocess was launched by these
probes. Supplying the retained actual listing prevents interference with the
real gate's active Testing logs. The registered-name count is discovery
metadata here, not a claim that this prober executed those native tests.

Before and after, all **1,550** bound files have identical hashes: source/CMake
files, checker sources, current/baseline File API data, generated controls,
Ninja files, fixed owner allowlist, and actual green discovery/result. The two
complete input indexes have the same SHA-256:
`9ddd628bcaa393e6c88a98e78603470a295dd22ad5059f01fd70a0d6e038d918`.

## Observed rejection controls

Each faulty view produced its specific intended error with no unexplained
additional error. Dropping a target also changes the two expected closures.
Every full result, exact semantic difference and discovery view is retained
in its named raw folder under the attempt directory.

| Criterion | Input fault observed by the full checker | Intended rejection |
| --- | --- | --- |
| I1 | Return runtime CMake text without `add_subdirectory(closure_guard)`. | Immediate child reachability differs for runtime. |
| I2 | Remove `planar_capture_directory` from current target metadata. | Target-name set differs, plus expected default-all and planar_tests closure differences. |
| I3 | Remove `execution_review.cpp` from ledger execution target source membership. | Ledger execution target semantics differ. |
| I4 | Remove runtime's direct `planar_closure_guard` compile dependency. | Runtime target semantics differ. |
| I5 | Change the runtime test instruction ceiling from 29,000,000 to 29,000,001. | Runtime test target semantics differ. |
| I6 | Append unexpected bytes to the observed generated runtime budget interface. | Generated payload differs. |
| I7 | Drop `replacement.execution_saved_response_c08_regressions` from the retained discovery input. | Registered test-name set differs. |
| I7 | Change the first runtime test's timeout from 120 to 119 seconds. | Registered command/properties differ. |
| I8 | Add the snapshot-probe executable to the observed default-all graph. | Default-all target closure differs. |
| I8 | Remove the snapshot-probe edge from the observed planar_tests graph. | Required-test target closure differs; setup confirms that exact target is lost. |
| I6 / I9 static portion | Remove the runtime implementation from observed configure dependencies. | Configure-dependency set differs beyond the exact new CMake inputs. |
| I10 | Return altered loader source bytes to the existing source-hash guard. | Non-CMake loader source changed. |

The altered timeout belongs to
`replacement.C3 A earlier retained callable failure dominates preparation without entering handler`.
The final healthy control restored the unmodified actual inputs and again
reported zero errors. No failed setup or surviving input fault occurred in
this attempt. No native source/build mutant was applied or compiled.

## Limits and handoff

I9's actual deterministic rebuild and formatting/Doxygen claims rest on the
real host gates, not these input controls. The configure-dependency control
checks the strict static graph policy only. I10's control observes the source
hash boundary; no historical-record or operator-data fault was injected.
This is not an exhaustive fault matrix, a new platform run, or closure of R19.
The authoring/execution interpretation mismatch remains outside this task's
checker sensitivity scope. Independent final review remains required.

`probes/MANIFEST.json` binds the local harness, this report, the execution
record, summary, input indexes and all fourteen raw result/detail/discovery
sets. The compact summary is
`probes/attempt-1794267151560956000/SUMMARY.json`; bulky raw records remain local.
