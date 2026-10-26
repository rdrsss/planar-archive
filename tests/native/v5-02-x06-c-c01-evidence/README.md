# X06-C-C01 evidence index

This folder describes the C01 candidate, not the final candidate: the C01 review returned request-changes and C02 followed. The final
candidate (X06-C plus C01 to C04), gate, red-first and probe results are in the [C04 round record](../../../docs/vernacular/rounds/V5-02-X06-C-C04.md#c04-final-record-coordinator-sync-before-review-2026-10-26) and
[tests/native/v5-02-x06-c-c04-evidence](../v5-02-x06-c-c04-evidence/README.md); the C03 candidate is in the [C03 round record](../../../docs/vernacular/rounds/V5-02-X06-C-C03.md#c03-final-record-coordinator-sync-before-review-2026-10-26) and [tests/native/v5-02-x06-c-c03-evidence](../v5-02-x06-c-c03-evidence/README.md); the C02 candidate is in
[tests/native/v5-02-x06-c-c02-evidence](../v5-02-x06-c-c02-evidence/README.md). The C01 candidate figures are in
[the C01 round record, C01 candidate record](../../../docs/vernacular/rounds/V5-02-X06-C-C01.md#c01-candidate-record-coordinator-sync-before-the-c01-review-2026-10-26)
(cited by the reviews as its "Final record"). C01 candidate: HEAD `caaf0396ae90a7f7d2d511b9a9a6e185e4362d52` (uncommitted). `git diff HEAD` (tracked, no pathspec) sha256
`3ad35762c7dfab9f09c4fa005a0f68b8c8ce8c9ee8a8a4e37ef5a8eba14ab1e6`; `git diff HEAD -- src` sha256
`d8cec5718ee4a058f9aeb873e8a431246d3182184b0b8c4a4276ea051c5ba102`; per-file hashes in the round record.
Raw logs are never deleted.

Statements in this folder that the C01 candidate record supersedes (all of the folder is superseded by C04 for the final candidate):

- `REPORT.md` (gate run 3, written 02:11) says the probes ran on an earlier tree and that its hashes differ from `probes/PROBES.md`.
  That was true of probe run 1 (`probes/superseded-1/`). `probes/PROBES.md` is probe run 2 (03:10) and records the same tracked
  hash `3ad35762...`; no source or test file changed after 02:03:28, so gate run 3 and probe run 2 describe the same candidate.
  REPORT.md's lines that "no prober re-run on this tree exists" and "None re-probed" are superseded: run 2 re-probed everything
  and KILLS C01, C02-r2, C02-r3, C17, C22 and X28. Survivors in run 2: C14 (real gap), C05, C19, X12, X18, X20 and C09
  (the last six equivalent or boundary-unobservable).
- `REPORT.md` says "Decision B" still appears in the X06-C record and that records R3 to R5, the authority analysis and the
  dependency audit are not yet written. All of them were written by the coordinator sync (X06-C-C01 candidate record, then named "Final record"; X06-C record
  rows I1g, I3b, X16, carried item 1 and "Changes versus the dispatch").
- `REPORT.md` says "no separate no-change run of planar_tests" and the redact check ran on 15 files; both still hold. The redact
  check predates the record edits and must be re-run at commit.
- `REPORT.md` and `red/RED.md`: the red snapshot hash `9681ec4d...` is `git diff HEAD -- src` at the stub wave, tracked files only,
  not the final candidate. Only the stub form of the R1 test has a behavioural red.
- `probes/PROBES.md` lists "Gaps to coordinator: C14". The C01 text called it an open limit (refusal site not observable); that was withdrawn by the C01 review and C14 is killed in C02 (`../v5-02-x06-c-c02-evidence/probes/PROBES.md`, row I-C14).
- `probes/superseded-1/` is the first probe run on an earlier tree; not final.
- `REPORT.md` and `probes/PROBES.md` of this folder: "Final", "no passkey" and "link graph" wording describes the C01 candidate; see the C02 and C03 records.
