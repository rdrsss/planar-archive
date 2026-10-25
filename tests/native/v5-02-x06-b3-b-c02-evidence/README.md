# V5-02-X06-B3-B-C02 evidence folder index

Written in C03 Dispatch 1, when C03 was records-only. C03 then became test-only after
Dispatch 2 (two tests added to `runtime.t.cpp`), so the candidate this folder's Generation 2
describes (985 tests, 39 `X06 B3`) is the C02 candidate, superseded by the C03 final candidate
(987 tests, 41 `X06 B3`, `runtime.t.cpp` `3b22d137816e`). Nothing in this folder was deleted or edited; this file only
classifies what is here, by timestamp, file size and content checks (for example
`candidate.patch` contains no `owner_charge`, so it predates the repair).

**Authoritative evidence for the final candidate is
`tests/native/v5-02-x06-b3-b-c03-evidence/` (gate in `gate/`, probes with raw logs in
`probes/`).** Where this folder disagrees with it, C03 wins.

## Generation 1: pre-repair candidate (984 tests, 36 `X06 B3` tests)

Candidate hashes (first 12): `runtime.cppm` `91f866dc8ff6`, `runtime.cpp` `1ff83396d768`,
`runtime.t.cpp` `5fbcbade1f32`; profile files as in the final candidate. Superseded.

- `candidate-files.before.sha256`, `candidate-files.after-probes.sha256`,
  `git-diff-head.before.sha256`, `candidate-files-edited.txt`, `candidate.patch`,
  `manifest.txt` (its listing predates the repair).
- `probes/results.md`, `probes/results.tsv`, `probes/raw/` (the probe-run raw logs, 36-test
  runs), and the probe drivers and inputs `probes/*.json`, `probes/*.sh`, `probes/*.py`,
  `probes/mutations/`. In `probes/results.md` the `n1-both` row (`9b2c43a0e71d`) survives
  by design of that generation.
- `gate/` (all of it, including `run-gate.sh`; 984 tests, `Total Tests: 984`).
- `raw/01-*` to `raw/06-*` (`.log` and `.exit`; byte copies of the `gate/` outputs),
  `raw/scratch-baseline-build.log`, `raw/scratch-baseline-ctest.log`,
  `raw/scratch-configure.log`.
- `raw/REPORT.previous-stale.md`: the stale gate report that described 984 tests.

## Generation 2: C02 post-repair candidate (985 tests, 39 `X06 B3` tests; superseded by C03)

C02 candidate hashes (first 12): `runtime.cpp` `ab70f5441bea`, `runtime.cppm` `331bd1b629df`,
`runtime.t.cpp` `162997debb60`, `profile.cpp` `91ac81f3d466`, `profile.cppm` `7bb805634449`,
`profile.t.cpp` `d94976e9c9ea`.

- `REPORT.md` (the final gate report of the C02 gate runner).
- `raw/gate-run/` (`01`-`06` `.log` and `.exit`, `base.txt`, `diff.sha256`, `head.txt`,
  `files.start.sha256`, `files.end.sha256`; the end hashes equal the values above).
- `inventory.txt` (985 names), `new-names.txt` (4), `removed-names.txt` (1).
- `probes/PROBES.md`: the independent prober's second pass on this candidate (39 tests;
  `n1-both` `1adbea375f61` killed). Its raw logs were deleted with the scratch directory, so
  this file has no raw logs beside it; this is part of the recorded process breach.

## Generation 3: review

The C02 reviewer's own evidence is in `tests/native/v5-02-x06-b3-b-c02-review-evidence/`
(`candidate.patch`, `repair-src.diff`, `gate/`, `probes/`), which is separate and not
edited here.

## Status

Generations 1 and 2 are kept as raw history. Generation 2's hashes differ from the C03 final
candidate only in `runtime.t.cpp` (`162997debb60` here, `3b22d137816e` final). Only the C03 evidence folder reflects the
rerun of the full probe set in a scratch copy and the final gate.
