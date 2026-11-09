# R15–R18 integrated final mutation verification

Status: in progress; no approval or full gate claim. Owned artifacts are limited
to this record and `tests/native/r15-r18-final-evidence/probes/`.

Actual-Lua policy baseline passes 15 cases and planner baseline passes 3 assertions.
Ten required mutants are assertion-killed: seven existing policy rows and three
per-consumer, complete-phase and grouped-producer planner rows. Raw commands,
patches, input digests and results are in `probes/lua-results.json` and `probes/raw/`.
An initial whitespace-invalid planner mutant is preserved in `superseded-1`,
excluded from the ten valid results; its corrected form fails the expected assertion.

Native rows are planned in `probes/INVARIANTS.md`; both build lanes currently
belong to calibration. No native mutant compile has run and no candidate build
artifact has been changed by the prober. An immutable artifact snapshot and lane
grant are required before scratch object/archive/PCM substitution starts.

Coordinator granted external native lane 1 after the copy-only snapshot. The first
native batch exposed stale Debug artifacts: invocation archive still referenced
the superseded two-argument preparation method and the ledger test executable
predated current publication controls. All provisional native kills were marked
`INVALID_STALE_BASELINE`, excluded, and retained in
`native-stale-baseline-results.json`; link failures remain invalid. The prober
stopped the batch and is rebuilding frozen implementation/test objects in external
scratch, preserving originals, before rerunning every native row. A passing stale
executable is insufficient baseline evidence for a different current source object.


## Handoff after coordinator stop

Native campaign stopped pending a coherent post-C06 build snapshot. Recompiling
invocation against the mixed PCM set spun in Clang ASTReader type desugaring;
the sample is preserved. Coordinator requested a fresh final prober after C06.
Lane 1 is released. Valid totals are **10 actual-Lua kills and 4 byte-identical
native carry rows; 0 new valid native kills/survivors**. Native table remains pending.
`probes/PROBES.md` contains the exact handoff and limitations; `manifest.json`
hashes scripts, logs and records. No candidate product/build writes or approval.
