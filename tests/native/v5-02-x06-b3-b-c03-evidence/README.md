# V5-02-X06-B3-B-C03 evidence folder index (written in C04)

Nothing in this folder was deleted or edited by C04. This README only classifies the existing files and records
corrections to them.

## Superseded and authoritative files

- `README.txt` ("PREPARED, NOT RUN") is SUPERSEDED raw history from Dispatch 1 preparation, written before any run. The
  real results are `REPORT.md` (runs 3 and 4) and `probes/PROBES.md`.

## Classification

- `raw/00-*` to `raw/07-*`, `raw/REPORT.run1.md`, `raw/inventory.run1.txt`: run 1 (C02-candidate era, 985 tests,
  superseded).
- `raw/run2/`, `raw/REPORT.run2.md`, `raw/inventory.run2.txt`: run 2 (986 tests, one route test added, superseded).
- `raw/run3/` and `raw/run4/`: the final-candidate gate (987 tests).
- `probes/` (`PROBES.md`, `results.md`, `results.tsv`, `mutations/`, `raw/`): run 4, the final probes.
  `probes/superseded-1`, `superseded-2` and `superseded-3`: earlier probe generations, kept as history.
- `candidate-files.*.sha256`: hash snapshots of the candidate files around the gate batches.
- `gate/run-gate.sh` (and `raw/run-gate.sh`): the gate runner scripts.
- `inventory.txt`: final test-name inventory (987 names).

## Correction notes for `REPORT.md` (raw, not edited)

- (a) Run 4 exit files: `raw/run4/` has `.exit` files only for the test build (`build-tests.exit`), ctest
  (`ctest.exit`) and the `ctest -N` step (`g-ctestN.exit`). Lint (`g-lint.log`) and the two full builds
  (`g-build1.log`, `g-build2.log`) rest on log text only. The C03 reviewer's gate
  (`tests/native/v5-02-x06-b3-b-c03-review-evidence/gate/`, `01.exit` to `06.exit`) covers lint and builds with exit
  files. Read `REPORT.md`'s run 4 'Exit 0' rows in that light.
- (b) The run 3 acceptance row 'N-c known limit' says line numbers 'have shifted by the added tests'. That is false: the
  known-limit lines (zero-declaration-read check in the `into_extension` adoption test) are unchanged at
  `runtime.t.cpp` lines 1528 to 1539.
- (c) `REPORT.md` still carries stale statements listed in the C03 record's Gate subsection, and the `3b22d1378165`
  prefix on its line 11 is a typo for `3b22d137816e`.
- (d) The `REPORT.md` section (b) Red-first "claimed, not shown", the R3 row and the probe limitation were written
  before the prober ran and are overtaken by `probes/PROBES.md` run 4: the route test killed its mutant (1 failed of
  41); the open-bound test kills `open-bound-refusal-protocol` and `source-error-mapping`, each 1 failed of 41.
- (e) The row R1 'not verified here' is closed by C04.

## Final candidate figures

- 987 tests, 41 `X06 B3` tests.
- `src/engine/runtime/runtime.t.cpp` sha256 prefix `3b22d137816e`.
- `git diff HEAD -- src` sha256 `96e44f54086469fe3ee0ccfa431ff4e3fadd12a15278834e5d6714613990531b`.
