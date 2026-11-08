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
  3 test cases, 519 assertions passed before the final secret-free reader
  refinement. The final reader refinement compiled successfully in the shared
  CLI build; its focused test rerun is pending that build finishing.
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
