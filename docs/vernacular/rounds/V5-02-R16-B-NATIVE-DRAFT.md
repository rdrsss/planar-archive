# R16 native recovery and R15-D history draft

Status: unreviewed implementation draft, resumed 2026-11-08 under the operator's
implementation-first instruction. No full gate, review, Linux run, mutation probes
or acceptance closure was attempted.

## Implementation

The execution ledger now commits refresh, checkpoint and release operations with
native token authorization under the write lock, exact operation preconditions,
checkpoint records, same-run dispatches and released/interrupted run states.
Refresh and release refuse an open check reservation. The event replay verifier
accepts and checks these operations and multiple dispatches per run. Checkpoints
are exposed on active-run snapshots. Recovery and dispatch reads use their own
read transaction and never select the run secret; they retain exact packet bytes,
manifest/work values, lineage checkpoints and current run identity.

The execution codec adds checkpoint values, recovery result shapes and R16 event
payloads. The change decoder builds recovery request framing from native options,
keeps token bytes out of request bodies, and attaches native authorization to the
ledger change.

History gains the `event` selector and CLI11 `--event` exclusions, event transition
and execution expansion, native/driver provenance, and packet/manifest assembly
for claim, takeover and refresh request results. Storage and format add a narrow
read-only event-metadata interface within their pinned snapshot. Execution value
records are decoded with the shared codec. At-event replay retains the original
claim through a refresh transition.

## Focused checks

- `cmake --build build/rewrite-debug --target planar_ledger_execution_tests planar_execution_change_value`
  compiled the ledger, codec, decoder, storage and history changes.
- `build/rewrite-debug/src/engine/planar_ledger_execution_tests 'R16 *'`:
  3 test cases, 519 assertions passed after the final secret-free reader
  refinement. The final focused run used the lane A draft before integration.
- The first native test run had one incorrect oracle: equal retained render
  digests must select the latest dispatch, not the original claim. The oracle was
  corrected to the contract's latest-dispatch rule. No product behavior was
  changed to satisfy that assertion.

The tests cover checkpoint/refresh/release commits and replay, token refusal
without a write, exact old and new retained dispatch reads, history event
expansion and saved request packet assembly. Existing broad suites and all
adversarial verification are deferred to the final fixing/validation phase.

## Integration notes

The recovery API is exported by `ledger.execution`; `ledger.recovery` reexports it
so the verifier and retained-reader implementation share record decoding without
a cyclic module edge. R17/R18 extend recovery with evidence and findings after
integration. Their history expansion and final cross-row acceptance remain part
of that integration, not a claim made by this native slice.

## R17/R18 integration implementation

The retained recovery reader now exposes latest evidence by work/check in the
current structural lineage and carried rejection findings. Snapshot roles expose
their structural lineage, and observed units exclude unrelated prior role
recreations. Pinned history snapshots expand reserved, recorded and interrupted
checks, and verdict bodies, repairs and blocker changes. Saved check and verdict
requests are decoded without exposing execution secrets or check output bytes.

Focused test cases were extended for these read paths. The integrated ledger,
history and decoder targets compile. Final focused commands:

- `build/rewrite-debug/src/engine/planar_ledger_execution_tests 'R16 *,R17 ledger*,R18-B history*'`:
  7 cases and 1216 assertions pass, zero skips.
- `build/rewrite-debug/src/engine/planar_execution_change_value_tests`:
  9 cases and 153 assertions pass, zero skips.

The R18 history fixture initially omitted the interpretation's embedded versions
and workspace identity, then used a hand-written completion shortcut invalid
under R17. The setup now carries those required relations and uses the native
reservation/completion transaction. No product verifier was relaxed. No review or
full validation gate is claimed.

Fresh refresh checkpoint rendering uses the snapshot's predicted checkpoint ID
and next event sequence. The locked native writer compares both allocations to
those predictions and refuses a raced snapshot with `state_conflict`, preserving
Lua's exact packet rendering and budget policy. A focused race test passes.
Recovery findings also exclude verdicts predating a worker's structural lineage.

Live snapshot run values now retain their original claim/takeover request ID for
orchestrator context. The R18 history setup uses an actual native check reservation
and completion, replacing its old hand-written completion shortcut in that path.

The integrated rejection history case exposed a replay mismatch: verdict rejection
completes the reviewer run while its role can become blocked. The merged replay
incorrectly derived a fenced run unless the role became done. Replay now derives
completed for verdicts independently of the next role state; check completion
still requires done. The failing focused case and debugger comparison showed
stored `completed` against replayed `fenced` at the verdict event.

The actual command smoke exposed a duplicated `xxh3_128:` prefix in the check
response observation digest. Native response assembly now uses the already
prefixed `hash_value` result; the focused R17 case compares the saved digest to
that exact value. The final focused run above includes this assertion and the
rejection replay fix.
