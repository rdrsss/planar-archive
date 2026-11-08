# R15–R18 integrated implementation draft

2026-11-08. The operator requested completion of feature and code implementation
before review, verification and validation cycles. This record is an implementation
checkpoint, not approval or a full acceptance result.

## Recovered starting point

The prior Claude session was inspected at
`~/.claude/projects/-Users-mn-projects-github-rdrsss-planar/aa194b3a-a2f1-48b9-a031-343fdb418741.jsonl`.
It stopped at the usage limit. Lane A contained committed R15-C and partial R15-D;
lane B contained the runner and R18 core. Those exact drafts were resumed and
integrated into `dev/planar-v2-vernacular`.

## Implemented scope

- Execution command registration, native services and CLI11 handlers for activate,
  status, preview, claim, takeover, refresh, checkpoint, release and recovery.
- Retained packets and checkpoints, exact request replay, recovery output capture,
  evidence/findings, history expansion and structural-lineage filtering.
- Check preparation/capture, bounded process execution, durable reservations,
  evidence, completions, publications, downstream readiness and check recovery.
- Reviewer verdicts, repair identity, blockers, acceptance, carried findings and
  downstream derivation against the facts being committed.
- Profile request framing, effective implementation binding and supplied-path
  admission for the execution families.
- Live ledger-form orchestrator services, run/reservation state and review guidance
  in the separate authoring bundle. The production/authoring split is preserved.

Native code owns capture, identity/token checks, transactions and retained records;
trusted Lua owns work material, prerequisite qualities and rendering. Fresh refresh
checkpoint IDs are predicted from the snapshot and checked under the transaction,
so Lua renders the actual IDs without native Markdown rewriting.

## Development checks

The production binary and authoring bundle build with the pinned toolchain.
Focused tests passed: ledger lifecycle/check/history 7 cases and 1216 assertions;
change decoder 9 cases and 153 assertions; recovery capture 2 cases and 16 assertions.
Focused Lua policy tests cover completion applicability, output vectors, lineage,
approval-dependent readiness, same-commit approval derivation and live orchestration.

Isolated CLI exercises passed:

- Lifecycle: 19 calls including exact replay, retained checkpoint IDs, refresh,
  preview since a retained packet, output recovery, history and release.
- Worker check completion and exact replay; reviewer readiness, claim, approval,
  exact verdict replay and accepted status.
- Interrupted reservation recovery, exact recovery retry and replay of the original
  check request as interrupted.
- Production init/load/activate with the quota fixture, followed by the authoring
  bundle's read-only live orchestrator prompt.

These are development checks of the new paths. No full test suite, release build,
Linux gate, mutation probes or independent review was run in this resumed draft.
Temporary diagnostic tracing was removed before finalization. Installed binaries
and operator databases were not used or changed; nothing was published or merged
upstream.

## Remaining phase

The integrated draft is ready for the final review, verification and fixing phase.
Existing carried review findings and the full contract/adversarial matrix remain
open until that phase. R19's real project/spec was never selected in the recovered
session; the operator does not know its location. The quota fixture is development
input and is not presented as that real-project exercise. Later product extensions
such as multi-root composition and distribution remain outside this draft's scope.
