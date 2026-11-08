# R17/R18 command integration draft

Implementation checkpoint, 2026-11-08. The operator requested all feature/code implementation before review,
verification and validation cycles. This is development evidence, not review approval or a release gate.

The production catalog now includes `check`, `check-recover` and `verdict` (17 production commands); the authoring
composition additionally exposes six authoring commands, including `orchestrator --ledger` (23 commands).
CLI11 owns each family. The retained command VM exposes only the command's finite native services; check retries
reuse the original native execution result and cannot rerun children. Native request framing excludes run secrets,
normalizes private paths, performs early replay and retains publication uncertainty after complete commits.

R17 wiring joins accepted-snapshot planning, descriptor-based preparation, reservation, bounded child execution,
boundary evidence and recapture with trusted completion/prerequisite policy and ledger commits. Reserve and
completion expectation lists are checked against invocation-owned snapshots. `check-recover` interrupts an explicit
open reservation and saves the interrupted response for both request IDs. R18 joins normalized verdict input,
current target/policy derivation and ledger verdict commits. Status projects open reservations, output-vector
mismatches and derived review acceptance. Recovery packets include recorded evidence and carried findings.

The live orchestrator reads the recorded ledger without a fresh declaration capture, includes current state,
runs, blockers and available commands, and never exposes run secrets. Native profile framing and metadata admission
cover the execution command families. The Lua policy uses metered sorting and the shipped restricted standard
library; development exposed and fixed typed empty-list and next-state `run: none` integration errors.

Development checks:

- Direct debug build of `planar`, `planar_authoring_stack`, `planar_execution_recovery_capture_tests`,
  `planar_ledger_execution_tests` and `planar_execution_change_value_tests` (serialized per build directory).
- `lua tests/native/r17-checks-policy.lua`: applicability, vector staleness, identity restoration, readiness and
  same-commit approval derivation scenarios passed.
- `lua tests/native/orchestrator-live-policy.lua`: live recorded-state projection passed.
- `python3 tests/native/r17-r18-cli-draft.py`: isolated source-built CLI workflow: initialize/load/activate,
  worker claim, real child check, exact check replay, completed worker/readiness, reviewer claim, approval verdict,
  exact verdict replay and accepted status.
- Isolated interrupted-check exercise: check recovery, byte-identical recovery replay, and original reserved check
  request replaying the same interruption response. This used a development-only open reservation left by a refused
  pre-fix command; no operator ledger was accessed.
- Adjacent agents exercised the R16 lifecycle, live authoring CLI, native recovery capture and ledger/decoder cases;
  their counts and evidence remain in their round records.

No full test gate, independent review, Linux verification, installation or publication was performed. The new
colocated CLI grammar cases are registered for the later consolidated verification phase. The source-built draft
workflow and focused checks above do not replace that phase.
