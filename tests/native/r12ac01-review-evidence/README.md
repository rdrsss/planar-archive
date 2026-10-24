# R12-A-C01 independent review evidence

Disposition: approve; report is `docs/vernacular/rounds/R12-A-C01-review.md`.
Product files stayed read-only. Shared debug directory was exclusively held
while running the direct serial commands recorded in the report.

- `candidate-before.log`, `candidate-after.log`: all 14 frozen candidate entries match.
- `evidence-before.log`, `evidence-after.log`: all 23 frozen implementation-evidence entries match.
- `focused.log`, `focused.xml`: 13 matched, 13 passed, zero skipped.
- `probe.cpp`, `probe-command.sh`, `probe.log`: independent scratch-only correction matrix, compiler and executable exit zero.
- `no-work.log`: all+planar_tests build reports no work.
- `protected-audit.json`: 3248 baseline files rehashed, 3246 unchanged and only the two authorized reference edits.
- `vendor-audit.json`: all seven SQLite vendor/cache files exactly match Git HEAD.

The local probe binary is a reproducible build artifact, not product source.
No broad milestone gate was repeated. Existing final 32-source format/Doxygen
logs and earlier independent transaction probes were inspected in place without
altering them. See the report for explicit remaining platform and R12 limits.
