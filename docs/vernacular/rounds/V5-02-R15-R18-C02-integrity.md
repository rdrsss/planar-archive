# R15–R18 C02 native retained-check integrity

Candidate correction for C01 native review N2 and N3. Not self-approved; integrated
full gates and independent review belong to the coordinator. No commit, install,
publication, legacy binary or operator database access.

The execution reader reconstructs each saved `check_v1` from retained reservation,
run/generation, terminal event, evidence, observation, completion and publication
rows. Canonical value equality checks every response field and rejects extra or
missing fields, wrong identities and references. Interrupted check and recovery
request IDs must retain the same reconstructed response naming the recovery
request. Withheld reasons are checked for vocabulary, ordering and applicability
to interrupted/completed responses; historical policy is not rerun.

For every recorded reservation, expected evidence membership comes from its last
preceding retained dispatch W: one boundary gate per unit plus every effective
required check. This check includes unsuccessful recorded checks and historical
reservations, not only current completions. Evidence reasons obey the closed
outcome-dependent R17 vocabulary. Definitions must identify the same work/check,
use the appropriate gate and exact definition keys; registry definition policy
fields match the retained W check declaration. Boundary and missing-executable
exit-status constraints are enforced. Existing completion/read verification and
C01 fencing/lineage work are retained.

The ledger completion test previously reported `alpha_work` although its retained
W named `unit_one`; the fixture now consistently uses `unit_one`. Boundary fixture
definitions now include `cwd.root`, as the real native adapter does. Existing
history assertions still execute before corruption; their read handles close
before the mutation helper runs. No test assertion or verifier was weakened.

## Tests-first evidence and verification

Evidence: `tests/native/r15-r18-c02-evidence/integrity/`.

- Before the verifier change, the explicit source-built Debug CLI accepted all
  six mutations (`before-cli.log`, probe exit 1): minimal response, invalid
  reason, missing required registry evidence, missing required evidence together
  with its saved-response entry, missing boundary evidence, and incorrect saved
  request identity. Each mutation used a real reserve/complete transaction in a
  temporary fixture database, restored exact immutable trigger SQL, and started
  from a backup of the unmodified fixture. The coordinated deletion independently
  demonstrates the need to derive membership from W.
- `cmake --build build/rewrite-debug --target planar_ledger_execution_tests planar -j 3`
  passed (`build.log`, subsequent fixture-correction rebuild `build-2.log`). An
  initial attempted extra `planar_ledger_execution_review_tests` target did not
  exist; review tests belong to the common ledger executable.
- First complete ledger run: 162 cases, 160 passed, 2 failed, 102013 assertions
  (`tests.log`). New corruption sections encountered the still-open history
  reader. Scoping that reader fixed the test harness. The first strict real CLI
  run also exposed the boundary `cwd` schema/fixture omission described above.
- `build/rewrite-debug/src/engine/planar_ledger_execution_tests`: **162 cases,
  102732 assertions, all passed**, exit 0 (`tests-2.log`), no skips.
- `python3 tests/native/r15-r18-c02-evidence/integrity/probe.py`: real
  init/load/activate/claim/check/replay/status fixture passes; all six mutations
  now return `ledger_corrupt` through both status and identical check retry:
  **12 corruption assertions pass**, exit 0 (`after-cli.log`). Each refusal exits 4.
- Pinned `/opt/homebrew/opt/llvm/bin/clang-format --dry-run --Werror` on
  `execution.cpp`, `execution.t.cpp`, `execution_review.t.cpp`: exit 0.

No Release/Linux/full-suite claim is made by this focused lane. No new fault,
concurrency or kill-at-commit experiment was performed. Registry executable
resolution remains host-dependent; this correction compares retained check
policy fields, not a re-execution of executable lookup. Independent review must
assess the integrated candidate and any remaining row-validation gaps.

Candidate identity at lane handoff is recorded in `candidate-sha256.txt`
(relative to the evidence directory above): SHA256 for all three touched ledger
source/test files and the tested Debug `planar` binary. `base-head.txt`
records HEAD. These identify this focused run; coordinator changes outside this
lane require a fresh integrated manifest before review.
