# R17/R18 CLI and live orchestrator draft surface

2026-11-08 resumed draft implementation. This record tracks the staged surface work;
shared runtime/native wiring follows integration of the R16 lane.

New CLI11 handler modules bind `check`, `check-recover` and `verdict`, with canonical
positive IDs, run-token grammar, request and actor bounds, separate required values,
and explicit `--confirm-stopped true`. Their command forwarding is completed when the
new runtime option alternatives enter the integrated tree; these modules are not yet
registered in CMake or bootstrap.

The authoring orchestrator now accepts exactly one form: the existing three declaration
options together, or `--ledger` alone. Ledger input is normalized through the existing
explicit-file path boundary. The Lua live branch consumes `read_execution`, recorded
interpretation and sealed command names, emits `orchestrator_state` after actions and
lists legal actions, blockers and the check execution isolation limit. The declaration
projection remains unchanged when the live context is absent.

Pending shared integration: authoring native ledger option, bootstrap copy, runtime
optional descriptor and execution-read capability, native service sealed command names,
run request IDs in snapshots, handler forwarding and CMake/test registration. The new
CLI tests cover all execution-path requirements, recovery confirmation, run-token
refusal and orchestrator form exclusivity. Tests not run yet; Lua syntax parsed.

No full gates, reviews, mutation probes or Linux runs, per draft-mode instruction.

`lua tests/native/orchestrator-live-policy.lua .` passed the standalone live projection
scenario: recorded state and check action are rendered, and a supplied token/secret is
not exposed. This exercises the pure projection only, not the pending native adapter.

## Integration update (2026-11-08)

The integrated branch now contains command registration and finite native services
for checks, reservation recovery and verdicts. Live orchestrator plumbing now carries
the ledger intent through CLI normalization, bootstrap and the runtime, reads the
accepted snapshot through a read-only native service and appends sealed command names.
The declaration and ledger forms have separate service-call paths. The catalog and
malformed-bundle generation anchors reflect the optional orchestrator option set and
its two capabilities. `orchestrator_state` is reserved alongside the prior sections.

The standalone live projection scenario still passes. The shared development build
and targeted command exercises are in progress; no final gate or review is claimed.
