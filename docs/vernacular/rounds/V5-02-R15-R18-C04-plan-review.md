# C04 plan review

Disposition: **approve — revised plan**.

The initial review requested one bounded replay-coverage clarification. The
coordinator added explicit late-replay and no-internal-retry decisions and
regressions; the recheck closes R1.
No product choice or redesign is required. This is a plan review only; no code,
build, mutation or full-gate approval is claimed. Source was read-only.

The proposed single-owner preparation is consistent with the sealed profile
architecture: retain the fixed trusted reconcile export, execute it in the
existing active preparation frame before cleanup, charge input/output conversion
and execution to the same cumulative owner, freeze owned results bound to the
exact snapshot/intent, and publish only opaque native prepared authority. The
existing dormant VM must remain inaccessible. Native ledger types need not be
introduced into runtime; bounded structural values suffice.

Using source/registry/effective-provenance trees and the derived interpretation_v2
with the existing format3 staging transaction is appropriate. Format2 retains its
existing completion path and authority; merely removing its format guard is not
an implementation. The profile history selector correction is also required and
does not expand the separate external history capability ABI.

## R1 resolved: late replay behavior and its regression

The initial invariant table covered early replay before capture but omitted the
new adapter's critical behavior after `stage_execution` returns a late replay.
The revised tables now explicitly exempt replay from receipt/CAS checks and new
completion effects, and prohibit internal load retry.

The approved decision and regression require that an equal late replay return its original
saved bytes and replay receipt **without checking prepared capture receipts or
CAS and without new effects**. Use a valid frozen prepared candidate, arrange a
same-ID winner, then alter its captured inputs and/or current head before the
loser stages. The loser must replay; an unequal body conflicts before CAS. A
faithful fault is unconditional `verify_native_receipts()` after a replayed stage.
This follows execution-contract.md EX-3 steps 6–7 and the profile preparation
architecture's native transaction ordering. `execution_write::result().replayed`
is the existing disposition; no duplicate transaction algorithm is needed.

Do not overextend this request: format3 EX-3 permits structural native validation
before its lock. This review does not require arbitrary invalid or moved-from
candidate acceptance on format3. Preserve the unchanged format2 path's stronger
late candidate-inspection ordering.

The revised plan also pins that load does not internally retry by recapturing, reconstructing an
owner or reentering a consumed VM. EX-3 permits internal retries only for check
completion; stale load CAS is returned to the caller for a fresh invocation.

The revised plan states these controls and is approved. Exact API and
implementation review must still check immutable alias severing, snapshot and
request binding, cumulative budget ownership/first-error preservation, no Lua
under the write lock, identity/CAS, original precommit receipt authority, no-op
handling, completion uncertainty and format2 compatibility. Existing history and
full profile grouped/split workflows provide useful coverage but do not replace
these boundary controls.

## Reviewed revised plan identities

- `tests/native/r15-r18-c04-evidence/plan/DECISIONS.md`: `e18ec93cdac48ef46b789e3309ef841bc2015fa9424972972b81a1b6b8144838`
- `tests/native/r15-r18-c04-evidence/plan/INVARIANTS.md`: `5ef1b2b91b7600efc72ced58b43dc2286907d6d16319ac698da3e900ac162035`
