# V5-02-X07-DOC evidence

Documentation-only round. Final candidate identity: HEAD 25704378472f3120baa6755c922894188fa94db5; sha256 of
`git diff --binary HEAD -- docs/vernacular` (tracked) 9f88be5b2ebfda87bf4e77bf8b0298cb220c6c606fa3c4fdc274c6989653d10f;
`git diff HEAD -- src tests scripts cmake` empty. Per-file sha256: x07-addendum.md 5e87c2ad...d175,
profile-preparation-architecture.md 0cc2d7ee...8c35, external-profile-implementation.md 1a76f1dc...a5af
(full values and the changed-file list are in docs/vernacular/rounds/V5-02-X07-DOC.md).

- check/: 35-check document checker, reference.json, red.log (Run A absent addendum, Run B stub), red.exit.
- plan/: INVARIANTS.md and DECISIONS.md (revision 2); superseded-1/ is revision 1.
- red/: ctest not-registered runs (not a behavioural red; see check/red.log).
- raw/: gate logs. REPORT.md: gate report. inventory.txt: ctest -N names (build predates the round).
- No probes folder: mutation probing was not run (probe false).
