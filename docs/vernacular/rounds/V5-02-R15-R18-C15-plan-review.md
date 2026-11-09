# C15 plan review

**Disposition: approve the bounded plan.** This approves proceeding to the
fresh coder; it does not approve an edited candidate, focused execution results
or final integrated publication. No blocking finding was identified.

The reviewed inputs are the implementation-plan authority/protocol, roadmap
opening, v5 direction, C15 brief, C15 source map, mapping `DECISIONS.md` and
`INVARIANTS.md`, `MAP-PROPOSED-PAIRS.json`, both complete Python scripts and their
CTest registrations in `src/cmd/planar/CMakeLists.txt`. The ordinary available
model substitutes for the unavailable historically prescribed model. This is
an independent plan review, not a legacy Planar workflow.

The scope is concrete and sufficient: remove exactly four spaces from C02
lines 24 and 74 and C03 line 25, preserving every LF and all other bytes;
remove exactly one terminal LF from the historical code-review record. The
proposed old/new hashes are explicit. Reading both scripts confirmed that the
blank lines lie outside string literals and that neither script reads its own
source. The only `read_text()` calls read temporary workspace `spec.lua` files.
CTest supplies the source-built command explicitly. The three C02 scenarios
and two C03 release reasons remain meaningful workflow coverage.

Acceptance must retain all of these conditions:

- A fresh coder proves the actual edited bytes match the two exact proposed
  SHA256 pairs and the three permitted blank-line positions, with every other
  byte and line number unchanged. Reproduce AST identity including attributes,
  whole-module and recursively nested marshaled code identity with identical
  filename/flags/inheritance/optimization settings, and semantic token strings
  and positions. Record the interpreter used and the registered commands.
- Run exactly the two anchored CTest registrations from the map in both
  existing host configurations, serially per build directory, with
  `--no-tests=error`, verbose raw logs and JUnit accounting. Each configuration
  must select and pass two tests, skip zero, and include the unchanged harness
  summaries `tests=3 failures=0 skips=0` and
  `tests=2 failures=0 skips=0`. These are four registration executions and ten
  harness scenarios across both configurations. An empty selection is failure.
- Preserve the historical prose original and its `ORIGINALS.json` binding.
  Record the exact one-LF normalization separately and restore that LF when
  proving the inverse against the original backup. Do not widen path-redaction
  allowances or rewrite original evidence.
- A fresh independent reviewer checks the actual candidate, exact proof and
  focused results. Any additional source change, identity discrepancy,
  unexplained skip or failed workflow invalidates this narrow route and returns
  the candidate to the coordinator for renewed scope and review.
- Publication checking admits only the hardcoded exact two source pairs and
  their proof. Every other non-record source retains its existing raw binding;
  all 272 frozen `src/` files, engine/product inputs and measured components
  remain unchanged. Intentional later prose changes require a separate closing
  revision record rather than modified original backups.

The four complete C14 gates remain evidence for their tested-original bytes.
The exact interpreted-code equivalence proof and focused host execution are a
separate C15 qualification. They do not establish raw current/Linux context
equality or a newly executed full platform gate. With all the above conditions
met, no new native build, full gate or calibration is necessary for this
three-blank-line cleanup. This is a bounded source round, not a records-only
exception or a blanket gate waiver. Final status and evidence records must
state the qualification before final staged review.

No candidate source, historical record, staging entry or original backup was
changed by this reviewer. No test, build, workflow, measurement, installed
binary, old Planar tool or operator database was invoked. Only this report and
the reviewer-owned evidence note were written. No commit, installation,
publication, push or merge occurred. R19 remains unselected.
