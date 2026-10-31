# RED check, round V5-02-RA01-B-W1-C01

Red-time identity (covers tracked diff and untracked bytes under src and tests/native/*-support):
- HEAD: 307026288615e317d145ba6846c0692929c50a17
- sha256 candidate.diff (git diff HEAD -- src tests/native): 7b880280948d74e70409b4b72e7a75dd319dfe45d05f99d564a308775d3da324
- sha256 SHA256SUMS: 16f5ec126387d1ab0f9b7b547c404c558d577df6b8f1791c4f190236c03b8f90
- Snapshot: red/snapshot/ (candidate.diff, untracked/ 16 files, list.txt, SHA256SUMS)

Build: cmake --build build/rewrite-debug --target planar_tests -> success (raw/build.log).
Command per test: ctest --test-dir build/rewrite-debug -R '^replacement\.<escaped name>$' --no-tests=error --output-on-failure (ctest names carry the "replacement." prefix). Logs raw/t1..t15.log (first attempt without prefix found no tests; those logs were replaced, see raw/NOTE.txt).

| log | test | invariants | expectRed | outcome | failing lines |
|---|---|---|---|---|---|
| t1 | reserved strings in non reserved fields keep their HEAD diagnostic | C-E17a-d | false | passed | |
| t2 | reserved pre check skips an entry with any ill typed identity field | C-SKIP-H, C-SKIP-F, C-SKIP-ALL (true); C-SKIP-N (false) | true | failed-assertion | external_registration.t.cpp:5411,5412,5413 (and more, see log) |
| t3 | reserved pre check skips malformed descriptors and keeps their diagnostic | C-I53 | false | passed | |
| t4 | profile registering a reserved authoring command name is refused as identity conflict | C-SKIP-CTRL | false | passed | |
| t5 | missing leaf remap applies only to diagnostic and from | C-R3-REG (true); C-R3-DIAG, C-R3-FROM (false) | true | failed-assertion | authoring.t.cpp:886 |
| t6 | explicit path stage keeps the existing messages | C-I42 (true); C-R3-NARROW (false) | true | failed-assertion | authoring.t.cpp:855 |
| t7 | selector stage precedes the explicit path stage | C-I40b | true | failed-assertion | authoring.t.cpp:672 |
| t8 | explicit path stage precedes the stub refusal | C-I40c | true | failed-assertion | authoring.t.cpp:693 |
| t9 | path text validation precedes the cwd read | C-E07, C-E08 | false | passed | |
| t10 | authoring verbs refuse positionals unknown options and equals syntax as usage | C-I35 | false | passed | |
| t11 | non reserved near miss names stay registrable | C-I52 | false | passed | |
| t12 | X08 C01 role preparation outputs-off boundary pins the request-record conversion charge | C-R5-PIN | false | passed | |
| t13 | authoring option signatures are exactly the RA-1 table | C-I06 | false | passed | |
| t14 | validator refuses a duplicated authoring row | C-I14 | false | passed | |
| t15 | X07 command sources add no diagnostic outside the closed table | C-DIAG | false | passed | |

Rows with no test (C-PROJ-EQ, C-NOAT, C-R5-CMT, C-R5-REC, C-R3-REC, C-R1-*, C-R4, C-R6-R9b, C-G12, C-NAMES, C-E15): not applicable, none registered; expectRed=false.
Limitation: several invariants share one test case, so per-invariant red is attributed at test-case granularity.

Note (review R5): the first red logs matched zero tests because the regex lacked the `replacement.` prefix; they were replaced by the prefixed runs rather than kept (against the never-delete-raw-logs rule). No result was lost; see `red/raw/NOTE.txt`.
