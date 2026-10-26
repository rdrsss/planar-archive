# X06-C evidence index

Candidate, gate, red-first and probe results of the X06-C candidate as reviewed (request-changes) are recorded in
[the round record's reviewed-candidate section](../../../docs/vernacular/rounds/V5-02-X06-C.md#reviewed-x06-c-candidate-record-coordinator-sync-before-the-x06-c-review-2026-10-26).
The C01 candidate is in [tests/native/v5-02-x06-c-c01-evidence](../v5-02-x06-c-c01-evidence/README.md); the C02 candidate is in [tests/native/v5-02-x06-c-c02-evidence](../v5-02-x06-c-c02-evidence/README.md); the C03 candidate is in [tests/native/v5-02-x06-c-c03-evidence](../v5-02-x06-c-c03-evidence/README.md); the final candidate (X06-C plus C01 to C04) is in [tests/native/v5-02-x06-c-c04-evidence](../v5-02-x06-c-c04-evidence/README.md).
Reviewed candidate: HEAD `caaf0396ae90a7f7d2d511b9a9a6e185e4362d52`, `git diff HEAD -- src` sha256
`e13ef3b13f6ee5303ea3aa3ae480ee3f0fbf82e941053ca49292c8916057076b` (tracked files only), per-file hashes in the round record.

Raw logs are never deleted. Statements in this folder that the reviewed-candidate record supersedes (all concern the reviewed
candidate; C01 to C04 supersede the whole folder for the final candidate; C04 is the final one):

- `REPORT.md` "Limitations" and section (a)/(b): "the 25th test has no ... probe record" is superseded. `probes/PROBES.md`
  (run 2) kills the bound-intent encoded-body mutant (X16) with that test. It still has no red-first record.
- `REPORT.md` "Mutation probes belong to the prober and were not part of this gate" is true of the gate; probe results are in
  `probes/PROBES.md` and all 19 table rows are killed.
- `REPORT.md` "The seam test ... Only the prober can confirm the kill": confirmed, I1i is killed by that test.
- `REPORT.md` notes that the redact check is stale: still true (last run 22:21, before the 22:59:49 test edit).
- `red/RED.md` hash `fb54336b...` is the stub snapshot, not the final candidate; the t07 row is the original seam body.
- `probes/superseded-1/` is the first probe run (40 tests, earlier test file); not final.
- `REPORT.md` "criterion 11 runtime import audit was not run by the gate runner": the coordinator checked at sync that
  `src/engine/runtime` imports neither `ledger.acceptance` nor `ledger.storage`.
