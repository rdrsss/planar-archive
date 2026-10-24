# V5-02-X04-B2 independent review

Verdict: **approve**. No blocking findings for the pure catalog-v2 structural
validation and immutable sealing slice. This review does not close X04-B3 or
the mandatory external-profile gate.

Reviewed the shared `dev/planar-v2-vernacular` checkout at
`3171c49e7b92d3abda01bfbb8bfb0c6b29d4c67a`, the complete B2 brief including
coordinator corrections, parent B/B1 records, implementation plan, roadmap, v5
direction, X-REG/X-PROJ/X-LIMIT/X-STORE, exact command contract and D3 load
amendment. Reviewer ownership is this report and
`tests/native/v5-02-x04-b2-review-evidence/` only. Product sources, author evidence
and unrelated rewrite changes were read-only. No Planar tool/skill/queue,
installed binary, operator database, commit, install, publish or merge was used.

## Frozen candidate and scope

- Candidate manifest: `186f33f0101da7774325e0c5f835077b78d2340be37d3add81fd99b79fd7f4d1`.
- Owned diff: `ae7611cf1602ee85af23c6ab3e738fa69a2d3ef091fcf5572ccb08b54578740d`.
- Author artifacts: `e90f479d7ca2a215d2ec6c4f14d0ae849e8f9bd61fd3ba4e57c218c51b45ccba`.

Independently verified all17 candidate paths, all663 author artifacts,721 debug
build identities,16692 protected final filesystem entries and the16 baseline
changes. `protect.py` already existed at the baseline; it is the seventeenth
candidate path. Reconstructed the exact eleven-file owned diff from originals
and current bytes. `AGENTS.md -> CLAUDE.md` remains intact. Checked141/141/142/143
archived sources for attempts01/02/03/04 against their individual freezes, and
confirmed final product/tests/current documentation equal attempt04. Later
finalization support is separately bound. Reproduced forward/reverse substitutions
and current source/generated hashes for all five storage/export variant families.
Initial and final identity results are in reviewer evidence.

## Contract assessment

The new module imports only standard/value/canonical layers; no runtime, Lua,
filesystem, CLI or ledger path enters it. CMake adds one module and colocated
tests using the existing helper, outside default `all` tests. Existing runtime,
shipped command Lua, profile codec, canonical codec and provenance codec remain
byte-identical to the baseline.

`effective_catalog.cpp:119` checks exact owner fields, positive integer ordinal,
tier, prefix, explicit ordered manifest membership and command ownership in
`shipped/planar/commands.lua`. Lines144–170 and235–240 preserve all five native
signatures, including shipped history/export decimal and all six required load
options. Mandatory membership is checked after the complete catalog. The old
runtime signature/sealing source was independently inspected at
`runtime.cpp:777` and `runtime.cpp:807`.

External descriptors use the exact extension family/response and permitted
capability vocabulary. Lines248–291 enforce mutually exclusive reads, projection
requiring exactly one read, complete required/optional option signatures, distinct
optional history_id names, extra nonhost options and refusal of incompatible
recognized host options. Pure text options grant no IO. Projection selection and
input matching are later invocation checks; a catalog may contain both projection
input kinds. Reserved shape/projection IDs require shipped ownership if present;
the validator correctly avoids fixed help/revision/source-module inventions or
forbidding additional shipped shapes/projections. Separate ID namespaces, all
exact fieldsets/kinds, sorting, duplicates and reserved command/option/native
handler identities are enforced.

The canonical boundary at lines311–331 refuses input over1048576 bytes before
decode and bounds temporary canonical traversal to depth7/nodes22921. The node
bound is the schema derivation `9 + 64*(25 + 32*9 + 3) + 128*21`, including map
keys. Descriptor/count/help/path checks finish before the successful sealed result
retains canonical bytes and its privately decoded tree. Bounded transient canonical
decode is not a capacity/RSS guarantee or the B3 aggregate manifest proof.
The successful object has no caller-owned mutable aliases; independent byte-buffer
and manifest mutation probes confirm retained value/bytes agreement. Null native
descendants are refused by canonical validation before unchecked schema access.

## Raw and independent verification

The author record is accurately bounded. Attempt01 failed test compilation on
the `history` name collision. Attempt02 ran98 focused cases:97 passed, including
all87 pre-existing cases; the single failing new fixture assigned the wrong
shipped ordinal. Attempt03 then demonstrated the malformed-handler diagnostic
regression failing before its fix. Independently verified the sole product change
after attempt02 is splitting malformed grammar into invalid_catalog while genuine
handler duplication remains identity_conflict. Final attempt04 passes11/11 catalog
cases with zero failures/errors/skips. Its508-name inventory preserves all497
prior names. Raw final LastTest log contains11 passes; all final commands are
serialized and successful. Pinned110-source lint/Doxygen, vectors, four inherited
source audits and no-work results are bound and inspected.

The reviewer executed `verify.py` under a complete filesystem freeze, excluding
only root `.git/`, root `build/` and reviewer-owned outputs. Commands are retained
in `commands.json`:

- `cmake --build build/rewrite-debug --target all planar_tests`: no work.
- CTest inventory:508 names, exactly matching the final author inventory.
- `ctest --test-dir build/rewrite-debug -R 'effective catalog' --output-on-failure`
  with retained JUnit and immediate LastTest/CTestCostData:11/11, zero skips.
- Independently compiled/linked reviewer `probes.cpp` with the candidate's pinned
  compile flags/module map and unchanged static libraries; executed only the
  seven `reviewer.*` cases:7/7, zero failures/errors/skips.
- Final `all planar_tests` build: no work; final candidate, author artifact and
  debug binary identity verification passes.

Additional probes cover option64/65 and handler128/129 byte grammar, shape/
projection revision64/65 and help4096/4097, reordered shipped/external owner
ordinals, separate same-spelling namespaces,4094/4095 original manifests, every
external read-option type mutation, all seven recognized host-option capability
combinations, null descendants/unused scalar payloads, and retention after input
bytes/manifest buffers change. No source changed during verification. Two
reviewer-runner assumptions failed before product commands and were corrected;
`runner-corrections.md` records them without representing them as product failures.

This is focused macOS ARM64 Debug verification. It is not a full508-case run,
RelWithDebInfo or Linux result. The last full integrated gates remain approved
X04-A3 at489/489 each. Catalog structural validation proves no callability or
semantic honesty. Effective tree byte/mode/profile correspondence and combined
manifest1MiB, live registration/CLI/acceptance integration, whole-invocation budget
sharing, public cutover, execution/RA01 and Linux remain open. Public init still
creates format1. Approval is limited to the exact candidate identities above;
changes affecting that scope need renewed review.
