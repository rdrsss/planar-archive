# C13 independent source and verification review

Disposition: **approve the bounded C13 test-fixture correction**. No blocking
finding or code advisory remains in this scope. Actual calibration closeout,
complete host/final-source Linux gates and final integrated acceptance remain
separate. This approval grants no installation, publication or merge authority.
An available ordinary agent substituted for the unavailable prescribed model.

The reviewer read the active instructions, implementation/roadmap/v5 authority,
R17-2 rule 2 and the descriptor test obligation, C13 brief/map/decision/invariant
tables and approved conditional plan, the candidate additions and affected
tests, surrounding spawn/owned-descriptor setup, exact Linux provenance, coder
results and private fault records. Review executed only read-only Python
comparisons, hashing and XML/log parsing. No build, candidate mutation, installed
Planar or operator data was used. Only this report and owned ignored audits
were written.

## Source and fixture correctness

The reviewed `runner.t.cpp` SHA256 is
`70ea1208b8a70020523a94e1e94a1c6d52b2aee23cbe67263f6c96bdf804c65f`.
Independent comparison with the diagnostic's 272-source snapshot finds only
that test changed; the other 271 files match. Its 145 insertions preserve every
original line and all 28 test names. Product implementation and CMake are
unchanged.

R17-2 requires close-on-exec for Planar-owned Linux descriptors; macOS additionally
uses `POSIX_SPAWN_CLOEXEC_DEFAULT`. The exact diagnostic independently matches
CTest/native/check parentage, descriptor-3 target/inode/mount and absent CLOEXEC,
while owned cwd/pipe descriptors have CLOEXEC and are absent from the child.
That evidence satisfies the approved fixture-only precondition. The repair
introduces no runtime close-all policy.

The helper is constructed before mutating flags and cannot be copied or moved.
It records original flags and file identity before changing a descriptor.
Partial setup failure rolls recorded changes back; destructor restoration
handles unwinding. Explicit restoration returns an error on failed cleanup.
Enumeration excludes its own directory descriptor and closes the observer before
isolation. Cleanup checks saved identity before restoring flags, within the
documented single-threaded fixture and retained lifetime of pre-existing owners.
It therefore does not restore the enumeration handle onto a later test subject.

Both tests capture before cwd and original held subjects exist. A dynamically
allocated inheritable marker above 3 and within the probe's 3..63 range checks
normalization, validity and original flags afterward. Separate controls exercise
partial-failure rollback and exception unwinding. The macOS inheritable subject
is created after capture and checked immediately before spawn. The original
exact empty output and `9\n` assertions remain intact, without an fd3 exception
or output filtering. Later Planar-owned descriptors remain outside the captured
set and observable by the original probe.

## Independently verified evidence

The owned audit scripts verify all 53 coder evidence files, 51 diagnostic files,
their report bindings, all current source hashes and both current host binaries.
XML reproduces **28 cases / 252 assertions** in each host configuration, zero
failures, expected failures or skips, with names matching the preserved source.
Both no-change build records report no work. The affected registered entries
retain 27 and 13 actual assertions on macOS.

The final proof manifest, its 394-file raw index and report hashes all match.
Independent parsing of source diffs, build/link logs, XML and artifact hashes
confirms the following actual Debug faults, each with a green same-scope
baseline:

| Fault | Platform | Verified failure |
| --- | --- | --- |
| I02: omit pipe `O_CLOEXEC` | Linux | Newly owned descriptors 8/9 are reported; exact empty output fails once. |
| I03: omit devnull `O_CLOEXEC` | Linux | Newly owned descriptor 7 is reported; exact empty output fails once. |
| I05: suppress both probe reports | Linux | Empty output violates the original exact `9\n` control once. |
| I06: retain CLOEXEC instead of original flags | Linux | Three original-flag checks fail after rollback, unwinding and final restoration. |
| I04: omit macOS default flag | macOS | The newly created inheritable subject leaks; exact empty output fails once. |
| I06: recapture after subject creation | macOS | Immediate pre-spawn inheritable-subject flag check fails once, preventing masked default-flag coverage. |

These are **six compiled faults / eight intended assertion failures**. Native
fault exits are 42; successful baseline, compile/link and driver results are
separately checked. No Release mutation execution is claimed. The Linux healthy
and restored suites each pass 28 cases / 248 assertions and registered 28/28;
the macOS no-op/restored suites each pass 28 / 252. Four macOS-only subject
assertions explain the difference, with zero skips. Each platform's restored
binary matches its no-op baseline hash.

All four Linux ambient-marker runs pass with marker 3 or 23. The negative probe
observes marker 23's range; the positive control still observes only 3 and 9,
so its marker-23 pass receives no detection credit. The separate owned-resource
faults prove that fixture isolation preserves genuine leak sensitivity.

The macOS audit also verifies 11 unchanged baseline inputs, 35 link-input
bindings and 1,934 dependency bindings, including the exact fault source edits.
Prober setup attempts remain preserved and excluded from credit. Read-only
reviewer checker filename/comment-matcher setup corrections are recorded under
`review/SETUP-EXCLUSIONS.md`; no candidate or proof bytes changed.

## Remaining acceptance boundary

This bounded approval closes source and required descriptor-fault sensitivity
for C13. It does not replace independent D15/D27 record verification, complete
platform gates, publication/redaction checks or final integrated review. Earlier
Linux failures remain valid preserved red evidence. R19's real-project exercise
remains separate and unselected.

Owned audit scripts/results and their manifest are under
`tests/native/r15-r18-c13-evidence/review/`. The coordinator must check that the
reviewed source remains current before advancing.
