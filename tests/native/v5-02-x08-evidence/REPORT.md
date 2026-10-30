# V5-02-X08 with C01: interim report

Status: INTERIM, written by PK-REC before the gate stage. No gate result is claimed here; every result below the scope section
is filled after the gate from the final raw logs by PK-POST.

## Scope of C01

C01 follows the second failed gate. The second gate's report is preserved byte-identical at
`raw/superseded-2/REPORT-second-gate.md` and the first gate's at `raw/superseded-1/REPORT-first-gate.md`; their raw trees are
`raw/superseded-2/` and `raw/superseded-1/` and are not rewritten.

C01 covers: (1) a platform-neutral host_path pre-check so a regular file followed by `..` maps to "Cannot resolve explicit root
directory" on macOS and Linux; (2) a fixed-length fixture root for the C02 budget-gap tests so the pinned gap does not depend on
the platform temp path; (3) the validator total derived as 1227 + 28 + 3 = 1258; (4) the X08 records: `requirements.json`,
`README.md`, this report, the A08 matrix row, the round record and the three reference documents. Details are in
`docs/vernacular/rounds/V5-02-X08.md` (C01 result).

## Records state at this point

- Evidence tree moved: all former `raw/` entries except `superseded-1` are now under `raw/superseded-2/` (163 files including
  the report copy; 162 before). `raw/` and `linux/` for the final gate do not exist yet.
- `requirements.json` is written with E01 to E14, matrix rows 1 to 12 (sub-clauses 5, 6, 4, 4 on rows 3, 5, 6, 12) and platform
  rows G06a to G06h (G06i added in C01-R7). Every listed test name was checked against `ctest -N` of `build/rewrite-debug`. The G06 `pass_line` values
  are PROVISIONAL, quoted from `raw/superseded-2/linux/container/debug-ctest.log`; PK-POST replaces them from the final Linux log.
  `logs` paths name the final assembled tree and do not exist yet, so the validator is not expected to pass at this point.

## Results

To be filled by PK-POST from the final raw logs (macOS Debug and RelWithDebInfo ctest, lint, no-op builds, redaction, Linux gate,
mutation prober, validator). Nothing is claimed before then.

C01-R7 update: the tree is refreshed from the R6 final gate (1260 names) and the validator exits 0; see `README.md` and the R7 section of the round record.
