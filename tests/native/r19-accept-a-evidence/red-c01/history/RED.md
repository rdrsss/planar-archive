# R19-ACCEPT-A-C01 RED-1 record: history defect (H1, H2, H3)

Stage RED-1, run after wave 1 and before any product fix. `src/engine/ledger/storage.cpp` is still the unfixed
source (PKG-C not applied). The only `src` change in the working tree is `src/engine/ledger/execution.t.cpp`, which
adds three TEST_CASEs (`git diff --stat`: 177 insertions, 0 deletions, no existing test removed or renamed).

## Pre-fix snapshot

Verbatim output of `python3 <workflows-scripts>/identity.py src`,
run from the repository root (also kept as `raw/identity-src.txt`). The combined row is the `snapshot_sha256` used by
the validator run below:

| identity | value |
|---|---|
| HEAD | `91e0df24f2167471e49ec057c7159bce5dff3608` |
| sha256(git diff --binary HEAD -- src), tracked only | `b5b9a3fb357c2ded89b7deb8ff5ba5550dc5d1fd359c22ba2e28099ca1bf4ec4` |
| combined sha256 (diff bytes, then each untracked path NUL bytes, sorted), pathspec src, evidence excluded | `b5b9a3fb357c2ded89b7deb8ff5ba5550dc5d1fd359c22ba2e28099ca1bf4ec4` |

| file | state | sha256 |
|---|---|---|
| `src/engine/ledger/execution.t.cpp` | modified | `250867b8bd74b86d718cbe17e5fa9983fda6a60a2b5e7c127829a3fec6f98950` |

## Commands and results

1. Build (serial, after wave 1): `cmake --build build/rewrite-debug --target planar_tests` exited 0; the log is
   `raw/build.log` (`ninja: no work to do`, so the binary already holds the three new TEST_CASEs).
2. Matched count: `ctest --test-dir build/rewrite-debug -N -R "R19-ACCEPT-A-C01 history event of a recorded check"`
   lists `Total Tests: 3` (ctest #725, #726, #727).
3. Combined run (the record the validator reads): `ctest --test-dir build/rewrite-debug -R "R19-ACCEPT-A-C01 history event of a recorded check" --no-tests=error --output-on-failure`
   exited 8: `33% tests passed, 2 tests failed out of 3`. Raw log: `raw/history-red-combined.log`.
4. Per-test raw logs (same command with each full test name): `raw/R19_ACCEPT_A_C01_*.log`.

| row | test (execution.t.cpp) | outcome | failing assertion | expectRed |
|---|---|---|---|---|
| H1 | `R19-ACCEPT-A-C01 history event of a recorded check keeps evidence integer nodes free of the boolean flag` | failed-assertion | `execution.t.cpp:4101` (`duration_ms` boolean == false) and `:4105` (`output_bytes` boolean == false); the shared walk also reports `:4002` and `:4004` | true |
| H2 | `R19-ACCEPT-A-C01 history event of a recorded check keeps evidence boolean nodes free of the integer value` | failed-assertion | `execution.t.cpp:4125` (`output_truncated` integer == 0) | true |
| H3 | `R19-ACCEPT-A-C01 history event of a recorded check preserves evidence integer and boolean values exactly` | passed (guard, D27) | none | false |

The `:4002` and `:4004` reports come from the shared `check_scalar_flags` walk, which checks both invariants over the
whole expansion. On the unfixed source the walk also trips on the `output_truncated` integer defect, so H1 also
fails there. No failure comes from a fixture `REQUIRE`.

## Validator (D36)

`raw/validate_red_record.txt` runs `accept_lib.validate_red_record` through `python3 -I -B -c` on the composed record
`{exit: 8, output: raw/history-red-combined.log, snapshot_sha256: b5b9a3fb357c2ded89b7deb8ff5ba5550dc5d1fd359c22ba2e28099ca1bf4ec4, command}`,
where the snapshot is the `combined sha256` row of `raw/identity-src.txt`. Result: `ACCEPTED`. The output contains
`FAILED:` markers and no compile or import error.

## Mutants each new test should kill (applied only by the prober, PROBE-1 and PROBE-2, in scratch)

- H1 test: `storage.cpp` `history_rows` keeps `node.boolean = **raw != 0` for the integer kind. Expected kill at
  `execution.t.cpp:4101` or `:4105`.
- H2 test: `storage.cpp` `history_rows` keeps `node.integer = **raw` for the boolean kind. Expected kill at
  `execution.t.cpp:4125`.
- H3 test (guard, expectRed false): clamp `node.integer = **raw != 0` and always-true `node.boolean = true` on the
  fixed `storage.cpp`. Expected kill at the exact-value checks (`duration_ms` 42, `output_bytes` 7, and the 17/3
  row) or the stored-zero-flag checks. H3 passes on the unfixed source, so this kill is the only proof for H3.

## Limits

- Red is against the unfixed source, so H1 and H2 turn green only after PKG-C. Their green is not shown in this stage.
- H3 is not red here (D27). Its guard value is shown only by the prober.
- No `rf/` records were written in this stage. Those belong to PKG-G (D18 and RF1/RF2), which needs prober kills.
- Nothing was mutated, stashed or reverted. `build/rewrite-debug` was only built, serially, with no changes.
