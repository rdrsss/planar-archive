# X06-C-C02 evidence index

This folder describes the C02 candidate (X06-C plus C01 plus C02, passkey), not the final candidate: the C02 review returned request-changes (the passkey was
forgeable, C02-R1) and the operator withdrew it. The final candidate is X06-C plus C01 to C04, in the [C04 round record](../../../docs/vernacular/rounds/V5-02-X06-C-C04.md#c04-final-record-coordinator-sync-before-review-2026-10-26) and
[tests/native/v5-02-x06-c-c04-evidence](../v5-02-x06-c-c04-evidence/README.md); the C03 candidate is in the [C03 round record](../../../docs/vernacular/rounds/V5-02-X06-C-C03.md#c03-final-record-coordinator-sync-before-review-2026-10-26). The C02 candidate, gate, red-first and probe results are in
[the C02 round record, C02 final record](../../../docs/vernacular/rounds/V5-02-X06-C-C02.md#c02-final-record-coordinator-sync-before-review-2026-10-26).
HEAD `caaf0396ae90a7f7d2d511b9a9a6e185e4362d52` (uncommitted). `git diff HEAD -- src` sha256
`e9f10163530691bc2a3f8b2666448ba7b5db1383f53ac01b6b67f2a4040cb582`; `git diff HEAD -- src` plus the five untracked sources
`fff9ee6408f8b3e86f9fd874dcf1b047e3a46380fe3af7882e69bcb74c336de4`; per-file hashes in the round record. Raw logs are never deleted.

Files here: `REPORT.md` (gate run 3), `raw/` (gate runs 1 to 3 and the two superseded reports), `inventory.txt` and `inventory-full.txt`,
`red/RED.md` with `red/raw/`, `probes/PROBES.md` (run 2) with `probes/raw/` and `probes/superseded-1/` (run 1, C14 survived on the
original fixture).

Statements in this folder that the round record corrects:

- `REPORT.md` criterion 1 says the prober's negative-compile content was not verified by the gate and cannot be confirmed. The prober
  delivered it: `probes/raw/negative-compile/` (N1, N3, N4 name the private `seam_key` constructor; N2 is a missing-key error; control compiles).
- `REPORT.md` criterion 3 places `CHECK_FALSE(installed.probe.pending)` at `profile_acceptance.t.cpp:900`; the line is 901.
- `REPORT.md` says the C14 re-probe and X27 were not verified by the gate. Both belong to the prober: `probes/PROBES.md` run 2 kills I-C14
  and X27 (I-R3-X27) by the named tests alone.
- `REPORT.md` says "criterion 4 comment rewritten, record corrections are the coordinator's": done by the record sync.
- `REPORT.md` "Limitations" about hashes: the combined hash in its identity table equals the prober's row D (`fff9ee64...`); the earlier run 2
  value `e2d50ed1...` (and `probes/superseded-1/`) describes the tree before the C14 fixture repair.
- `PROBES.md` notes that the dispatch's per-file hash of `tests/native/r14c3b2-support/generate.py` (`d9731f25...`) does not equal the file:
  the C02 coder edited that file (line 65) after dispatch, as the round record lists.
- `red/RED.md` line numbers and snapshot hash `769c5a2c...` describe the stub snapshot (tracked files only), before the Doxygen fix (+3 lines)
  and the C14 fixture repair (+2 lines); current starts of the five C02 tests are 981, 989, 996, 1002, 1013. It is not the final candidate.
- `raw/REPORT-run1-superseded.md` (Doxygen failure) and `raw/REPORT-run2-superseded.md` (before the C14 repair) are superseded by `REPORT.md`.
