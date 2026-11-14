# R20-CARRY-A-C01 report

Closed directly by the orchestrator on 2026-11-14 at the operator's direction; see the "C01 close" section of `docs/vernacular/rounds/R20-CARRY-A.md`. Placeholders: `<home>` is the home directory, `<repo>` the repository root.

## Change

- `src/engine/history/CMakeLists.txt`: `TEST_TIMEOUT 1680` on `planar_history_interpretation_v2_codec` (16 test cases).
- `src/engine/ledger/CMakeLists.txt`: `TEST_TIMEOUT 840` on `planar_ledger_format` (104 test cases).
- `tests/native/r20-carry-a-support/timeout_headroom.py`: per-test minima 1680 and 840.
- Records: R20-CARRY-A `REPORT.md` redaction, `docs/testing.md` wording.

## Verification (targeted, not the full gate)

| command | result |
|---|---|
| `cmake --build build/rewrite-debug --target all planar_tests` | `ninja: no work to do.` |
| `ctest --test-dir build/rewrite-debug -j8 -R 'replacement\.r20\.\|v2 every native allocation failure\|selects wider fixed tree profile for valid closure above4096'` | 20 of 20 passed, 24.5 s wall |
| `ctest --show-only=json-v1` TIMEOUT histogram | 1,872 tests; 16 at 1680 s, 104 at 840 s |
| `python3 scripts/rewrite-check.py lint --build-dir build/rewrite-debug` | format and Doxygen clean, 251 files |

Earlier Debug-only gate attempts on this candidate (prelim-1 passed 1,872 of 1,872 after one rerun; prelim-2 failed twice on unrelated load flakes) are in local `raw/prelim-1/` and `raw/prelim-2/`. The final full gate was stopped by the operator.

## Limits

- Release and Linux not re-run on the C01 candidate; last full gate is R20-CARRY-A run1 at limit 120.
- Limits derived from recorded loaded times, not re-measured under heavy load.
- Load flakes seen: `replacement.runtime makes caught quota terminal before protected calls return` (non-timeout, failure text not captured) and `replacement.cli` (30 s timeout).
