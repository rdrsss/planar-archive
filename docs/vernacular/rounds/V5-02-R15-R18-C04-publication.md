# C04 publication integrity and event-operation correction

In progress; coder evidence, not approval. Shared candidate and all C01–C03
changes preserved. No installation, legacy binaries/data, commit or self-review.

## Mapping and decisions

N8 validates every publication, including historical rows, against the declaration
at its completion's dispatch revision. The retained work object binds spec, root
and owned work. Its facet list contains consumed facets only, so own publication
definitions must come from the retained interpretation's `bundle.spec.facets`.
This does not change work fingerprints or add runtime authority. Facet paths are
exact file paths (declarations.md185–188), with no directory expansion. The
normalized payload entries and approved `facet_v1` hash are recomputed using the
existing tree reader, file digest and canonical hash adapters. No live checkout
or check execution participates in verification.

The previous C03 native publication fixture supplied an undeclared facet and an
arbitrary semantic digest. Its scoped replacement records a facet declaration in
the test revision and computes its semantic independently; other seeded fixtures
retain their existing default interpretation.

Parent additionally assigned the full-gate failure in `R18-B verdict_recorded
payload references`: the pure codec accepted a different valid operation under
an event name. The bounded fix maps all supported carrying events to their one
operation, preserving load/noop, release/interrupted and check-recover mappings.

## Tests-first evidence

Evidence root: `tests/native/r15-r18-c04-evidence/publication/`.
`r15-r18-c04-publication-cli.py` creates three real grouped completions, first
changing facet bytes and then reusing the exact latest payload. The valid history
and duplicate-tree control passes. Before the product fix all eight corruption
assertions fail because status and exact check retry accept: historical unrelated
tree, current unrelated tree, current same-path old bytes, and paired publication
semantic plus matching saved-response corruption. `red-cli.log` preserves this.

The first attempted codec red build stopped before compilation on an unrelated
profile-edit generator anchor (`using namespace engine;` appeared twice). The
profile coder removed the redundant directive; the second attempt is retained.
No compile failure is counted as a mutation/test assertion failure.

## Verification

- Codec tests-first: one case, 65 assertions, 26 intended failures (both encode
  and decode for each of 13 event names); `codec-red.log`.
- Focused Release build: `cmake --build build/rewrite-release --target
  planar_history_execution_codec_tests planar_ledger_execution_tests -j 3`.
  First attempt found a test-only unqualified `file_digest`; the qualified
  call correction rebuilt successfully (`green-build-2.log`).
- Full codec executable: **31 cases / 1555 assertions**, all pass, exit0
  (`codec-green.log`).
- Full ledger executable: **163 cases / 102531 assertions**, all pass, exit0
  (`ledger-green.log`). Native cases include unrelated complete-tree payload
  and paired semantic/saved-response corruption.
- Pinned formatter on four changed C++ files passes (`format.log`).
- Exact candidate file hashes and HEAD are generated in `candidate.json`.

Release build lane returned to the profile coder. Real CLI green rerun awaits
their freshly linked Planar; the current raw CLI record is intentionally red.
Full gates, independent review and mutations remain coordinator-owned.
