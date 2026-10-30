# V5-02-X07-C02 red check

Red-time identity (covers tracked diff and untracked bytes; no untracked files under src or tests/native/*-support existed):
- HEAD: f78f8eb47267ab9c72efacc07708e7c0c2669840
- sha256 snapshot/candidate.diff: 99c8183a25e1ffe790ebd14c4a8a9ffd7904a7d270e186749769400349e0b1e3
- sha256 snapshot/SHA256SUMS: 9b82beb07e9e893ea312dbc0819252952f4fd631cd9ada22f72dd44c901f72f5

Build: cmake --build build/rewrite-debug --target planar_tests, rc 0 (raw/build.log).
Catch2 tests register in ctest as `replacement.<name>`; the first run without the prefix (raw/t*.log overwritten) found no tests, then was rerun with the prefix. x07.* tests are registered unprefixed.

| test | inv | expectRed | outcome | failing line |
|---|---|---|---|---|
| tolerates committed content changes | I1,I17,I23 | true (I23 false) | failed-assertion | admission.t.cpp:1019 CHECK(result) |
| refuses a replaced or removed file | I2 | false | passed | |
| refuses a mode change | I3 | false | passed | |
| content changes still refuse for every other supplied file role | I4,I10 | false | failed-assertion (UNEXPECTED for a guard) | admission.t.cpp:1019 CHECK(result), case at 1068 |
| load ledger role refuses trusted aliases and links | I5 | false | passed | |
| X07 C02 loser prepared after a winner commit ... | I6 | true | failed-assertion | profile_acceptance.t.cpp:808 |
| X07 C02 prepared profile native receipts ignore a committed ledger change | I7 | true | failed-assertion | profile_acceptance.t.cpp:831 CHECK(verified) |
| x07.ledger_commit_after_admission_reports_revision_conflict | I8 | true | failed-assertion | x07_replay.py:134 assert status==3 ... revision_conflict |
| x07.receipt_races_refuse_before_authorization | I9 | false | passed | |
| X07 C02 post-preparation input change plus winner commit ... | I9b | false | passed | |
| C4 B2 appended admitted ledger refuses verify ... | I12a | false | passed | |
| X07 C02 history family ledger receipt stays strict ... | I12b | false | passed | |
| X07 command sources add no diagnostic outside the closed table | I15 | false | passed | |
| X07 adapter redacts malformed trusted payloads ... | I15b | false | passed | |
| x07.two_process_cas_one_winner | I13 | false | passed | |

Non-test invariants (I14, I16, I18-I22) not run by this step. Raw logs: raw/t1..t15.log (index order of the brief list, t8/t9/t15 are x07.*).

## Reviewer D15 run (2026-10-30, finding V5-02-X07-C02-R1)

The runs above used the stub snapshot in the shared build, not the D15 pristine-HEAD overlay, and did not record command lines or identity output. The reviewer ran D15 on a pristine HEAD copy with the three test files overlaid: identity in `review/identity-head-overlay.txt`, configure and build in `review/head-overlay-configure.log` and `review/head-overlay-build.log`, results in `review/p1.log` to `review/p4.log`, `review/c1.log` and `review/c2.log`. I6, I7 and I8 fail on assertions on HEAD; the I9b and I12b guards pass on HEAD. These logs are the behavioural red evidence of record.
