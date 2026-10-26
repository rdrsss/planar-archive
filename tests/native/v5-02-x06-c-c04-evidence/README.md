# X06-C-C04 evidence index

Final candidate (X06-C plus C01 to C04), gate, red-first and probe results are in
[the C04 round record, C04 final record](../../../docs/vernacular/rounds/V5-02-X06-C-C04.md#c04-final-record-coordinator-sync-before-review-2026-10-26).
HEAD `caaf0396ae90a7f7d2d511b9a9a6e185e4362d52` (uncommitted). `git diff HEAD -- src` (tracked only) sha256
`179c4a8a292384e3fbf4affe4aa3410a0bf76ea850a641d3fac8ff142e81f8ba`; `git diff HEAD -- src` then the bytes of the six untracked src files (list in the round record)
`6243274522f393e2a423ebc62ee896e6f81fc9662ed1d44b64fcd4eff95693fb`; per-file hashes in the round record. Raw logs are never deleted.

Files here: `REPORT.md` (gate run, 1041 of 1041; its logs are `raw/build-tests.log` and `raw/gr2` to `gr7`), `raw/` (the earlier `1-*` to `7-*`, `build1`, `g2` to `g7` logs describe earlier tree
states and are superseded and kept), `inventory.txt` (1041 names), `red/RED.md` with `red/raw/`, `probes/PROBES.md` (final prober run, 131 rows) with `probes/raw/` and `probes/superseded-1/`
(the first prober run on an earlier `acceptance.t.cpp`, superseded).

Statements in this folder that the round record corrects:

- `REPORT.md` says the probes (`PROBES.md`) and red logs were taken on an earlier `acceptance.t.cpp` (probe-time hash `abf22e8d...`, gate identity `diff HEAD` `618c6ceb...`, `src` `179c4a8a...`) and
  quotes `9af1cc57...` and `20260273...` for the probes. That was true of the first prober run (`probes/superseded-1/`). `probes/PROBES.md` (16:55) is a full re-run after the gate (15:46) on the same
  bytes: its identity equals the gate's (`179c4a8a...`, `618c6ceb...`, `acceptance.t.cpp` `0188afa2...`). The red logs still describe the earlier file (13:45).
- `REPORT.md` says the probe table has "131 rows, 91 killed, 27 build-failed, 13 survived". The final run is 131 rows: 27 BUILD-FAILED, 93 KILLED, 11 SURVIVED (`probes/PROBES.md`).
- `REPORT.md` criteria 2 and 6 are "not evidenced by this gate": they are prober deliverables, met by `probes/PROBES.md` and `probes/raw/negative-compile/final-negative.log`. Criterion 9 (records) is the
  coordinator's and is done by the C04 sync, except that the gate `REPORT.md` and `red/RED.md` themselves are not edited. Criterion 7's link-graph consumer list is in the round record.
- `REPORT.md` and `red/RED.md` name the red snapshot `fe6980a9...` (`git diff HEAD -- src`, tracked files only). No saved diff, untracked bytes or per-file hashes exist in `red/`; criterion 10 is partly met.
- `REPORT.md` gate commands 1 and 5 found no work to do (the tree had been built by earlier runs); that limit is stated there and in the round record.
- `REPORT.md` quotes the failing line numbers of `RED.md` for the older `acceptance.t.cpp`; the current file shifted by about 3 lines.
- `probes/PROBES.md` row A1p's table mutant ("r7 then compiles") is inexact: r7 compiles under A1x (the C03 shape), A2d and A1i, and still fails under A1 itself (see the round record's amendments).
