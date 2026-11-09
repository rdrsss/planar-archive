# R15–R18 C02 independent policy/runtime review

Disposition: **request-changes**. Candidate HEAD is
`ae02a12bf77c6b72d3060253dd47800c645796a7` plus the working changes. Scope and
binary hashes are in `tests/native/r15-r18-c02-evidence/review-policy/candidate.json`.
No product edits, builds, installed binaries, operator data or legacy tools were
used. Probes create fresh temporary ledgers, workspace declarations and roots.

## P1 — P2: rederive reviewer readiness when a worker releases to done

Locations: `src/stdlib/planar/commands.lua:288`–317, especially the vector at
298 and direct commit at 317. The new actual-state prerequisite is
`src/stdlib/planar/checks.lua:103`–104. Native admission/replay also enforce
exactly one release transition (`src/engine/ledger/execution.cpp:1386` and 3153).

Both `release --reason released` and `release --reason interrupted` can restore a
worker to `done` using applicable historical completion. The implementation
updates only that worker, leaving its reviewer `blocked`. The reviewer then
cannot be claimed even though every worker is done. This prevents the ordinary
driver from advancing review after abandoning an unnecessary restored-work run.

Real CLI reproduction, separately for both release reasons:

1. Load A, activate, claim worker, and complete its passing check.
2. Change the material goal to B and load; claim the stale worker.
3. Restore A and load. Correct C02 behavior is worker `running, drifted=true`
   and reviewer `blocked`.
4. Release the worker using either reason. The release succeeds and the worker
   becomes `done`, but the reviewer remains `blocked`.
5. Claim reviewer: exit 3, `claim_conflict`, details `state=blocked`.

The probes assert reviewer `ready` and exit 1 on that assertion. Raw scripts and
logs: `review-policy/release_probe.py`, `release_probe.log`,
`interrupted_probe.py`, and `interrupted_probe.log` under C02 evidence.
The scripts derive from the native C02 lifecycle fixture; no SQL corruption or
mock snapshot is involved.

### Scoped contract correction

R18-1 item 1 requires the reviewer's start prerequisite to be every worker's
actual state `done`; EX-4 gives an active run precedence over applicable retained
completion. Those behaviors are correct. R16-4 lines 191–192 instead assert that
release cannot affect another role and requires exactly one transition; its
cross-row invariant at line 329 repeats that restriction. The rationale is
false once downstream readiness depends on the releasing worker's state.

Amend those R16 clauses to retain the target release transition first, with its
existing `released`/`interrupted` reason and cleared drift, followed by necessary
deterministic downstream reconciliation transitions. Derive the full next
vector from the hypothetical released snapshot and validate/replay those
additional transitions natively. Keep existing token fencing, request replay,
all-role fact-version preconditions, checkpoint/provenance behavior and the
no-fact-version-bump rule. Both release reasons need the restored-completion
regression, successful reviewer claim, replay and history verification. This
resolves the contradictory no-dependent-state assumption without introducing a
new product choice or relaxing the done-worker prerequisite.

## Assessed fixes and verification

Read the implementation/roadmap, C01/C02 policy reports, execution R15–R18
contracts and v5 direction. Inspected the complete checks/review/reconcile and
command lifecycle code, including recovery/status, repair identity, reviewer
lineage, hypothetical post-check state, load ordering, release and takeover.
The native reviewer owns independent ledger/check integrity assessment.

Independently executed against the current source-built Debug candidate:

- `lua tests/native/r15-r18-c01-policy.lua`: **15/15 passed**, zero skips.
- `python3 tests/native/r15-r18-c02-policy-cli.py build/rewrite-debug/bin/planar`:
  **3/3 scenarios passed**, zero skips, including both restoration modes and
  repaired preview/recover/claim/refresh/takeover manifest identities.
- Two additional native release lifecycles: **both reproduce P1**, each fails
  the intended readiness assertion after observing a successful release.

The C02 fixes for actual done-worker admission, post-load worker projection,
repair fingerprint/provenance and takeover's allocation placeholder are
consistent with the inspected contracts and these tests. No additional confirmed
policy defect is claimed. This bounded review does not certify every concurrency
interleaving, external-profile variant or authoring-orchestrator sequence.
Full Debug/Release/Linux gates and mutation evidence remain coordinator-owned
and were pending during review; focused success is not final acceptance.
