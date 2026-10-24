# R14-C3-B2 coder evidence

This is a candidate for fresh independent review, not a coder approval. The shared
checkout and all protected prior sources/helpers/evidence were preserved. No
Planar tools, installed binary, operator database, commit, installation or
publication was used.

The native `complete_acceptance` transaction resolves exact saved-body replay
before candidate/CAS inspection, then checks unknown-request CAS, prepared owner
and workspace. It shares the private record interner, builds trees through
`tree_core`, compares complete encoded root/value records, guards signed ID
allocation, and publishes revision/event/request/result/projection atomically.
The already-owned complete response precedes pending-state verification. The last
operations are complete verification, metadata comparison, native receipt verify,
SQLite commit and response return, with no intervening SQL after receipt verify.

Executed checks and actual counts:

- `cmake --preset debug`, followed by serialized direct target builds: success.
- `planar_ledger_format_tests '[atomic_acceptance]'`: 9 cases, 670 assertions,
  zero skips. Initial/no-op/change/revert, exact parent/timestamp relationships,
  replay after later head/missing files/moved capture, conflicting bodies, CAS
  ordering, workspace refusal, changed receipt, real SQL failures, concurrency,
  valid maximum record ID and sparse later maximum revision are covered.
- `planar_atomic_format_test_tests`: 3 cases, 136 assertions, zero skips.
  Selective file/directory/value/request collisions reach each interning phase.
  Every other kind retains real hashes, and all provenance/history validators
  remain active. The exact private shared allocator is also checked at signed
  maximum; this is helper-only event exhaustion evidence, not a fictitious
  gap-free maximum event history.
- The generated `--mutate-final-receipt` build succeeded; its single receipt
  regression failed with exit 42 (15 passed assertions, 1 failed). Exact normal
  generation was restored and rebuilt before the final gate. Both manifests and
  raw failing output are retained.
- `python3 scripts/rewrite-check.py gate --build-dir build/rewrite-debug`:
  301 registered cases passed, zero skips, 71 active source files passed pinned
  formatting/Doxygen, and the final build performed zero steps. This includes
  all 151 prior history-focused cases, 25 capture and 12 prepared cases plus
  12 new atomic cases. The CLI test covers 247 real processes.
- A dedicated verbose CLI rerun preserved the explicit 247-process stdout after
  CTest inventory inspection had overwritten its temporary LastTest log. This
  passed one CTest case, zero skips; no baseline history/capture suite was rerun.
- Both vector generators in `--check` mode passed: 12 hand byte vectors,
  20 upstream hashes, 41 canonical/domain vectors, 10 independent record vectors.
- Supplementary pinned formatting/Doxygen passed all 6 existing/new helper C++
  files. Python generator/audit source uses only the standard library.
- `audit.py` verified 4,701 protected files and the instruction symlink, exact
  generated files plus byte-reversible manifests for all three variants, original
  baseline name inventories, full gate counts and CLI247. It actually linked the
  production acceptance object/archive against the native dependency set and ran
  all 9 cases (670 assertions), excluding Lua, loader, runtime and embedded source.
  The first audit's missing temporary CLI stdout assertion is preserved separately;
  final audit uses the dedicated preserved CLI log and passes.
- Final `cmake --build build/rewrite-debug --target all planar_tests`:
  `ninja: no work to do.`

Fault coverage is actual SQLite statement trace interruption at records,
tree_edges, revisions, events, requests, current_state and pending verification,
and actual SQLite commit-hook refusal. Each compares original database bytes,
fully reopens original state, and successfully retries. No VFS fault, process
kill, storage-device exhaustion or power-loss claim is made by these probes.
The prior storage primitive/reliability tests remain included in the full gate.
Concurrency uses two separate native prepared owners, finite retry of busy/live
journal refusals, and the existing bounded SQLite wait. This is macOS Apple
Silicon debug evidence; Linux and integrated R14 release gates remain outstanding.

`candidate.json` identifies frozen source snapshots. `report.json` separately
identifies raw evidence. Coordinator-owned round/status documents are excluded.
