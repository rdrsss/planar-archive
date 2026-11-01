# RED-CHECK V5-02-RA01-B-W2-C01

Red-time identity (covers tracked and untracked bytes):
- HEAD: 3343ad38304db57388bc7ac2438a40043eba1b7d
- snapshot: red/snapshot-2/ (snapshot/ already existed; never overwritten)
- sha256 candidate.diff (git diff HEAD -- src tests/native): 316a0581affa8fc08c6bfb90378bcf8d831f099472973a19a2d874c6c619a9ca
- sha256 SHA256SUMS (candidate.diff + 11 untracked files under src): 954f2b5112a1bb93316bc4224ae8c377cd02daea1c83df82fd3314f7bd5ef228

Build: cmake --build build/rewrite-debug --target planar_tests succeeded (raw/build.log).
Command per test: ctest --test-dir build/rewrite-debug -R '^replacement\.<escaped name>$' --no-tests=error --output-on-failure
(ctest names carry the `replacement.` prefix; an unprefixed first attempt matched nothing and its logs were replaced.)

| raw log | test | invariants | expectRed | outcome | failing line |
|---|---|---|---|---|---|
| 01 | observe maps an unreadable entry to cannot inspect | C01-01,02,03 | false | passed | |
| 02 | observe refuses an unreadable entry as io exit 4 | C01-04 | false | passed | |
| 03 | observe refuses a symlink root | C01-05 | false | passed | |
| 04 | observe bad_alloc refusal carries observation_total_bytes | C01-06 | true | failed-assertion | directory.t.cpp:370 |
| 05 | observe applies precedence in sorted name order | C01-07,08 | false | passed | |
| 06 | capture_diagnostic refuses a symlink as a symbolic link | C01-09,10 | true | failed-assertion | capture.t.cpp:962 |
| 07 | capture_diagnostic refuses a non-regular file as io | C01-11 | true | failed-assertion | capture.t.cpp:996 |
| 08 | capture_diagnostic maps a failed open to cannot resolve | C01-12 | true | failed-assertion | capture.t.cpp:1019 |
| 09 | capture_diagnostic bad_alloc refusal carries diagnostic_bytes | C01-13 | true | failed-assertion | capture.t.cpp:1060 |
| 10 | RA01-B2 capture_diagnostic refuses a file changed before verification | C01-14 | true | failed-assertion | capture.t.cpp:894 |
| 11 | observe checkpoint canonical size bounded at 8192 | C01-18,19 | false | passed | |
| 12 | prompt of exactly 1048576 bytes accepted, one more refused | C01-20,21 | false | Timeout 30s under ctest (guard, not assertion) | |
| 13 | critique global prescription needs a nonempty check | C01-22 | false | passed | |
| 14 | rejection excerpt with a non-digit index | C01-23 | false | passed | |
| 15 | diagnostic and from reach Lua only as opaque slots | C01-24,25 | false | passed | |
| 16 | X08 C01 role preparation outputs-off boundary | C01-30 | false | passed | |
| 17 | observe refuses an unreadable root as io exit 4 | C01-33 | false | passed | |
| 18 | capture_diagnostic maps a post-gate add_input failure | C01-34 | true | failed-assertion | capture.t.cpp:1049 |
| 19 | capture_diagnostic post-gate failures remapped at every native call | C01-35,36,37 | true | failed-assertion | capture.t.cpp:1072 |
| 20 | observe recheck bad_alloc is a limit | C01-38 | true | failed-assertion | directory.t.cpp:382 |
| 21 | observe recheck growth past total bound stays membership changed | C01-39 | false | passed | |
| 22 | observe digest allocation failure is a bound refusal | C01-44 | true | failed-assertion | authoring_observe.t.cpp:771 |
| 23 | slot type checks are pinned in the Lua handlers | C01-45 | true | failed-assertion | authoring_orchestrator.t.cpp:621 |

Invariants C01-15,16,17,26-29,31,32,40-43 are n/a (document/record/process rows), not run.
All expectRed tests failed on assertions. Guard 12 timed out at 30 s (expectRed=false), noted as a limitation.

## Limits (added by C02, R5)

The red base snapshot is identified by the sha256 of `candidate.diff` and of `SHA256SUMS` in this folder, not by `identity.py` output. The snapshot is byte-equal to the reviewed wave 2 bytes, so the red base is sound; the limit is only that its identity is not the workflow `identity.py` value (C01-42). C01-46 (`RA01-B2 C01 observe maps a failed entry inspection after listing to cannot inspect`) has no red-first record: the test was added after this red-first run.
