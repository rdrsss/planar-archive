# X06-C-C03 evidence index

This folder describes the C03 candidate (X06-C plus C01 plus C02 plus C03), not the final candidate: the C03 review returned request-changes (C03-R1) and C04 followed. The final candidate is
X06-C plus C01 to C04, in [the C04 round record](../../../docs/vernacular/rounds/V5-02-X06-C-C04.md#c04-final-record-coordinator-sync-before-review-2026-10-26) and [tests/native/v5-02-x06-c-c04-evidence](../v5-02-x06-c-c04-evidence/README.md). The C03 candidate, gate, red-first and probe results are in
[the C03 round record, C03 final record](../../../docs/vernacular/rounds/V5-02-X06-C-C03.md#c03-final-record-coordinator-sync-before-review-2026-10-26).
HEAD `caaf0396ae90a7f7d2d511b9a9a6e185e4362d52` (uncommitted). `git diff HEAD -- src` sha256
`daced2fd0c1c6ae9f50237373a30c15f5b32c31a234f894b431edcb93e981ee8`; `git diff HEAD -- src` then the five untracked sources
(`acceptance_seam.cpp`, `acceptance_seam.cppm`, `profile_acceptance.cpp`, `profile_acceptance.cppm`, `profile_acceptance.t.cpp`)
`5a9214f17d39dfe65fdef554c3efc658d4f54c7a5ca2fb388e3a3fcbccc1a1fb`; per-file hashes in the round record. Raw logs are never deleted.

Files here: `REPORT.md` (gate run of the C03 candidate, 1038 of 1038), `raw/` (this gate run `01-*` to `08-*`; the earlier gate run `1-*` to `7-*`, superseded and kept),
`inventory.txt`, `red/RED.md` with `red/raw/`, `probes/PROBES.md` (run 2, the C03 candidate) with `probes/raw/` and `probes/superseded-1/`
(run 1, an earlier tree; it also holds `raw/dependency-audit.txt`).

Statements in this folder that the round record corrects:

- `REPORT.md` says the probes and red logs were taken on earlier trees (`3d9c87aa...`, `df3828ed...`) and that the three newest tests (message pins and the
  runtime layering test) have no mutation evidence. For the probes this is superseded: `probes/PROBES.md` run 2 is on the final tree (its hashes
  `daced2fd...` and `5a9214f1...` equal the gate's) and kills D2 to D7, D9, E1 (`shared body refusal message bytes are pinned`), D4 (`workspace mismatch message
  bytes are pinned`) and CY2 (`runtime product sources import only ledger completion from the ledger`). The red-first gap stays: those three tests have no red record
  (they are guards added after the red wave). The red logs still describe the red-time tree.
- `REPORT.md` and its criterion 7 row cite `probes/raw/dependency-audit.txt`; that file does not exist. The audit is `probes/superseded-1/raw/dependency-audit.txt`
  (run 1); the coordinator re-checked its content read-only on the final tree (runtime imports only `planar.engine.ledger.completion`; the `:seam` and `:profile` importers are as listed).
- `REPORT.md` says the prober's negative-compile content was not re-verified by the gate and the probe table has "88 rows, 66 killed, 15 survived, 7 build-failed": that
  count belongs to run 1. Run 2 has 91 rows (76 KILLED, 7 SURVIVED, 7 BUILD-FAILED, 1 TEST-ERROR that is a kill) and the negative-compile probes n1 to n12, c1, c2, l1 in
  `probes/raw/negative-compile/`.
- `REPORT.md` criterion 9 ("coordinator work, not checked here") is done by the coordinator sync of the C03 record, which also relabels the C02, C01 and X06-C records.
- `red/RED.md` hash `df3828ed...` is `git diff HEAD -- src` at the red time (tracked files only), not the final candidate (`daced2fd...`). The coder's untracked-inclusive pre-edit
  hash `9376aab7765590b7c22e2f6028795b442c43f3a635a179d75e5f7295d4687198` (pathspec `git ls-files -m -o --exclude-standard -- src tests/native docs`) is only in the coder's report and is recorded in the round record.
- `REPORT.md` gate commands 1 and 5 found no work to do (the tree had been built by earlier runs); that limit is stated there and in the round record.
- `probes/superseded-1/` is the first probe run on an earlier tree (`3d9c87aa...`); not final.
- Corrections from the C03 review, made by the C04 sync (the gate `REPORT.md` itself is unedited, so its text below is still as written):
  - `REPORT.md` criterion 7 names `planar_atomic_acceptance_seam_test` and `planar_metadata_acceptance_seam_test`; those targets no longer exist (folded into
    `planar_atomic_acceptance_test` and `planar_metadata_acceptance_test`). The consumers of the `planar_runtime` link edge through `planar_ledger_acceptance` are listed completely in the C04 record, including
    `planar_cli_invocation_0` to `_5`, `planar_load_cli_probe_invocation` and the `planar_cli_variant_*` and `planar_load_cli_probe` executables; no runtime symbols are pulled in today.
  - The `9376aab7...` hash (coder report; cited in the C03 record and the entry above as the untracked-inclusive snapshot hash) is the sha256 of a `git ls-files` file-name list; it does not identify contents.
  - `probes/PROBES.md` rows I5b-r (tests 160, 168) and P7b (test 168) show extra failures; they were stale mutated binaries left by the previous rows (S3 and E4b: only two variant test targets were
    rebuilt), not flakiness. The named kills stand; the C04 prober rebuilds every variant test target on every row.
  - Criteria 1, 2 and 3 are recorded as met in this folder's `REPORT.md` and in the C03 record; the C03 review found them not met (C03-R1, argument-dependent lookup). See the C04 record.
