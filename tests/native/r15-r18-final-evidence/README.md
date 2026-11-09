# R15–R18 corrective review evidence

C01–C15 records live under `docs/vernacular/rounds/`. This directory retains the
local raw gate, mutation, calibration and independent audit evidence. Only compact
records and indexes are published; bulky logs, copied builds, databases and private
mutants stay local. Earlier failed, setup-invalid, masked and superseded attempts
remain recorded and are not counted as acceptance passes.

`COMPACT-COPIES.json` identifies byte-identical publication copies and the original
relative base for paths inside each manifest. Any later path redaction is separately
recorded with original/redacted hashes; the original local artifacts remain intact.
The compact C11 calibration and C12 identity records live in their own evidence
folders. The explicitly required `plan/DECISIONS.md` and `plan/INVARIANTS.md` tables
are published alongside these compact records under the orchestration rule.

Key compact files:

- `PROBES-C09-MANIFEST.json`, `PROBES-REQUIRED58.json` and
  `PROBES-ATTEMPT-SUMMARY.json`: execution mutation attempts and scoped dispositions.
- `BUDGET-PROBES-MANIFEST.json`: budget fault campaign and private checker evidence.
- `C11-EXPERIMENTS-MANIFEST.json`, `C11-RERUN-VALIDATED.json` and
  `C11-CARRY-SCOPE.json`: actual old-tuple build, instruction isolation, section
  discovery sensitivity and exact carry scope.
- `INDEPENDENT-RECORD-AUDIT.json`, `MANIFEST-SENSITIVITY-AUDIT.json` and
  `C12-RECORD-AUDIT.json`: independent recomputation and hash checks.

Current C14 host and Linux aarch64 Debug/Release gates each pass 1,844 tests
with zero skips and actual no-work repeats. Host pinned formatting/Doxygen covers
250 active files. The Linux before/after/container manifests match. Exact raw
results and independent audits are bound by `FINAL-GATE-AUDIT.json` and the final
manifest; earlier failing/incomplete attempts remain separate.

Current source identity is `../r15-r18-c14-evidence/SOURCE-FROZEN.json`. Fresh C13
normal/raised measurement records remain component-scoped under the C14 source
overlay and explicit post-build identity qualification: 112 ar timestamp-only
changes and two executable UUID/debug-time/derived-signature changes, with all
code/data payloads identical. `CURRENT-ARTIFACT-RECONCILIATION.json` records 8,927
raw matches, 114 metadata-only differences and zero unexplained changes.

C15 removes exactly three whitespace-only blank-line indentations in two Python
verification scripts. The full platform gates retain their tested-original byte
scope, qualified by independently reproduced syntax, token and compiled-code
identity for the exact old/new source pairs. Both affected workflows passed in
both host configurations (four executions, ten scenarios, zero skips). Compact
C15 proof and independent review records are indexed here.

Original raw records remain local. Published path-redaction layers retain
original/redacted hashes and are checked by inverse restoration before staging.
Final integrated disposition is the C14 acceptance report, with actual staged
publication verification performed separately from product gates.

Limits remain explicit: Linux covers aarch64 only; the real operator project/spec
for R19 remains unselected; fixture success does not certify that exercise. No
installed binary or operator database was used. Installation, publication, pushing
and merging are not authorized by these records.
