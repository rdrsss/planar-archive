# C13 descriptor-fixture fault sensitivity

All six private compiled faults were caught at their intended assertions. This is
bounded Debug sensitivity evidence, not source approval or integrated acceptance.
An ordinary available agent substituted for the unavailable prescribed model.
No candidate product, test, CMake, dependency or limit byte was edited by this
prober. Private variants, commands, sources, objects, archives, binaries and raw
results remain under `tests/native/r15-r18-c13-evidence/probes/`.

## Candidate and build boundaries

The tested final `runner.t.cpp` SHA256 is
`70ea1208b8a70020523a94e1e94a1c6d52b2aee23cbe67263f6c96bdf804c65f`.
All 272 candidate source hashes match before/after/final snapshots. All 190
product inputs also match the earlier execution-campaign product identity;
the fixture repair changes test source only.

Linux uses the exact pinned base, LLVM 23.1.6 packages, CMake 4.4.5 SHA,
non-root source-copy recipe and toolchain footer from the official Dockerfile.
Only its combined full-gate command region is replaced with the focused driver;
the exact Dockerfile-specific ignore file accompanies the external recipe.
The private image builds the current runner target, then restores its own source
between variants and rebuilds. The candidate checkout is never mounted or
mutated. No builder prune was run, leaving its cache available to the official
gate. Configure, build, complete driver and no-work exits are all zero.

macOS uses the current Debug Ninja compiler/link commands and matching module
maps/PCMs. The no-op private link copies the current test object and libraries.
A separate unchanged-source private compile/link proves each mutation route
before its fault. I04 replaces only the runner implementation archive member;
the test object and remaining inputs stay unchanged. The late-capture variant
replaces only its private test object. Compiler-discovered dependency hashes,
link vectors and source/object/archive/binary identities are retained. No
shared build directory was built or modified by this prober.

## Healthy controls

| Platform/configuration | Complete native baseline and restored run | Other healthy controls |
| --- | --- | --- |
| Linux aarch64 Debug | Each 28 cases / 248 assertions, zero failures, expected failures or skips. Baseline/restored binary SHA256 is identically `5ed113bee533cd9e8ef1980f7a042a6714b797b85e1b7151914b1fd167da0b82`. | Complete registered runner selection passes 28/28 both before and after faults. Both exact descriptor cases also pass with explicit inheritable marker 3 and marker 23 (four one-case runs). |
| macOS arm64 Debug | Each 28 cases / 252 assertions, zero failures, expected failures or skips. No-op baseline/restored binary SHA256 is identically `355c7b60086009b2f420487e76fd6aeced80e217eb56b63abbc49ef0a13907c4`. | Unchanged private implementation and unchanged private test compilation/link routes each pass the one-case negative probe before their faults. |

The four extra macOS assertions arise from its existing intentional inheritable
subject; Linux's 248 is the actual platform count. Marker 23 lies in the negative
probe's 3..63 range. The positive control observes only 3 and 9, so its marker-23
pass receives no claim of detecting descriptor 23. All matched counts are
nonzero. Every fault has a green same-scope baseline.

## Actual fault observations

| Row / private fault | Platform | Baseline assertions | Actual fault result |
| --- | --- | --- | --- |
| I02: `pipe2(fds, O_CLOEXEC)` becomes `pipe2(fds, 0)` | Linux Debug | One case, 23 passed | Exit 42; one case, 22 passed / 1 failed. Newly owned pipe descriptors 8 and 9 are reported; `result.output.empty()` fails. |
| I03: `/dev/null` open omits `O_CLOEXEC` | Linux Debug | One case, 23 passed | Exit 42; one case, 22 passed / 1 failed. Newly owned `/dev/null` descriptor 7 is reported; `result.output.empty()` fails. |
| I05: both private probe reports replace `echo $n` with `:` | Linux Debug | Positive control: one case, 13 passed | Exit 42; one case, 12 passed / 1 failed. Empty output violates the original exact `9\n` expectation. |
| I06: original-flag restoration writes `saved.flags \| FD_CLOEXEC` | Linux Debug | One case, 23 passed | Exit 42; one case, 20 passed / 3 failed. The dynamic marker remains close-on-exec after partial-setup rollback, exception unwinding and explicit final restoration; all three original-flag checks fail. |
| I04: omit `POSIX_SPAWN_CLOEXEC_DEFAULT` | macOS Debug | One case, 27 passed | Exit 42; one case, 26 passed / 1 failed. The deliberately inheritable subject created after capture is descriptor 7; it is reported and the exact empty-output check fails. |
| I06: restore and recapture isolation after creating the macOS subjects | macOS Debug | One case, 27 passed | Exit 42; one case, 28 passed / 1 failed. The immediate pre-spawn inheritable-subject flag check fails (`1 == 0`). The two extra successful REQUIREs belong to the private fault setup. |

The late-recapture variant specifically tests marking new subjects after their
creation. Its flag assertion fails before that normalization could silently mask
the default-flag fault. The separate I04 failure proves the final fixture leaves
the real inheritable subject observable. Restoration faults leave the actual
child leak check green but fail the independent parent-flag checks, so parent
state preservation has its own observation.

These are six distinct compiled faults and eight intended assertion failures,
not six platform acceptance runs. Faults were run in Debug on the platform where
their relevant branch executes. No Release mutation rerun or x86_64 result is
claimed. Coder-focused Release results and complete platform gates are separate.

## Preserved setup exclusions

Two Mac proof-driver attempts remain under `setup-attempt1/` and
`setup-attempt2/`, with original drivers, logs, metadata and explicit exclusions:

1. The first read-only command selector matched `runner.cppm` as a substring of
   `runner.cpp`. It stopped before compilation; exact token matching repaired the
   selector. No fault/test credit is assigned to that attempt.
2. The second driver checked an unexpanded `FD_CLOEXEC` token in Catch XML.
   Catch expands it to `1`. Native I04 and late-recapture executions already had
   the intended failures, but the proof driver stopped before its final identity
   checks. A complete final rerun uses the exact normalized flag expression and
   supersedes that incomplete driver attempt. All credit above uses the final
   complete results.

The final drivers completed with exit zero after expected native fault exits 42.
All private Mac inputs/sources/objects/binaries have verified identical retained
copies under `retained-private/`; original scratch paths remain intact.
`PRIVATE-RETENTION.json` maps those paths and hashes. `FILE-HASHES.json` binds all
local raw files, and `MANIFEST.json` is the compact publication index.

No installed Planar binary, operator database, host installation, publication,
push, merge or commit was used. Actual D15 calibration, full host/Linux gates,
source review and integrated acceptance remain separate obligations. R19's real
project/spec remains unselected.
