# V5-02-X05-C2-A independent review

Disposition: **approve**. No blocking finding was identified in this bounded
projection-definition identity slice. Retained projection execution and native
result admission remain C2-B.

Review used shared `dev/planar-v2-vernacular` at HEAD
`3171c49e7b92d3abda01bfbb8bfb0c6b29d4c67a`, preserving the inherited dirty
rewrite. Product, author support/evidence, historical records and current reference
prose were read-only. Reviewer writes are confined to this report,
`tests/native/v5-02-x05-c2-a-review-evidence/`, and exclusive serialized ordinary
Debug verification. No product fix, Planar tools/skills/roles/queues, subagents,
commits, installation, publication, merge, release invocation or operator-data
access occurred. Applicable guide, implementation/roadmap/v5 direction, full
C2-A brief, C2 search disposition, X-REG/X-PROJ/X-LIMIT and approved shape terminal
and B3-C hash-accounting contracts govern this review.

## Identity and ownership

The [owned result type and internal API](../../../src/engine/runtime/runtime.cppm#L133)
return copied ID/revision and a moved digest, without escaping VM references.
The [resolver](../../../src/engine/runtime/runtime.cpp#L2759) selects retained
kind2 entries only and borrows immutable, final, validated D. The native ordinal
locates privately retained original selected-module bytes; it does not enter
identity. The exact recipe is canonical-v1/seed-zero XXH3-128 over
`["planar","canonical_v1","projection_definition_v1",{descriptor:D,owner:{tier:"external",module:"external/<original path>"},module_bytes:HEX}]`.
D has exactly id/abi/revision/input/help, and HEX is lowercase original source
bytes. No selector, module ordinal, unselected source, profile bytes or mode is
introduced. There is no callable dispatch, provider, invocation transition or
budget reset in this method. The approved shape resolver is byte-identical to its
actual pre-edit copy.

An independent reviewer encoder, separate from runtime and the author's Python
encoder, reproduces the full **806-byte** pinned preimage and
`xxh3_128:11f11b5d6a3fd4c65e8b7cb163a73f1f` using upstream vendored XXH3. A second
**935-byte** oracle covers UTF-8 selected-source/comment/help bytes, a nested
original module path, and final mutation to history_snapshot_v1/r2. The production
resolver matches it. Both complete preimages and descriptors are preserved in
[first-oracle.json](../../../tests/native/v5-02-x05-c2-a-review-evidence/first-oracle.json)
and [second-oracle.json](../../../tests/native/v5-02-x05-c2-a-review-evidence/second-oracle.json).

Production tests cover every variable descriptor field, source/path/comment
materiality, reorder without ordinal leakage, unselected/profile/selector
independence, caller capture and filesystem mutation, repeated lookup, owned
lifetime after destruction and moves. All selected projection callbacks contain
execution-failure sentinels. Additional [reviewer production cases](../../../tests/native/v5-02-x05-c2-a-review-evidence/extra.cpp)
interleave lookup with healthy actual shape invocation, semantic validation,
selection and rendering on the same owner three times. Same-ID shape/projection
namespaces resolve independently, semantic/render bytes equal a healthy control,
and identity remains exact before/after all calls. The UTF-8/history-input case
also survives destruction of caller snapshot contents and owner move.

## Reservation and failure boundary

The [prospective reservation](../../../src/engine/runtime/runtime.cpp#L2808)
finishes before allocating staging strings, new graph objects, canonical storage
or result metadata. Path addition and source doubling are checked; later byte
terms enter existing checked charge calls. The final flat descriptor is borrowed,
so it is traversed by allocation-free planning rather than copied into a
pre-reservation graph. The 25 logical visits account for descriptor values/keys,
new wrapper values/keys and the hash envelope. Extra borrowed-D and envelope
scratch reservations add bytes without adding converted nodes; conservative
overlap is explicit.

Source tracing through command_record/command_text, hash.cpp, canonical.cpp and
the approved pinned libc++ derivation confirms coverage of map/value/control
blocks, initializer-list ownership, staging and graph strings, key/tier/domain
copies, encoder active-set nodes/buckets and sorted-map pointer storage, cumulative
canonical string growth, digest allocation and returned ID/revision copies.
Canonical bytes use the approved **4*N+4*A** growth bound and owned string copies
use **2*n+A**, rather than final live byte counts. This is cumulative charged
work, not RSS accounting or a new quota. Existing shape accounting is unchanged.

The author's reversible full-operation observer reconstructs exactly to current
runtime bytes. All seven final private compile/link/run commands pass; recursive
response expansion and private object/PCM/dependency output paths are recorded.
Eleven fixtures span source **207–1,048,207 bytes**, path **6–816 bytes**, and
64-byte ID/revision plus 4096-byte help. Observed **45–48 allocation requests**
and **5,880–10,487,448 requested bytes** remain below source-derived reservations
**26,632–16,801,112 bytes**, with exactly 25 node visits. The independent flat
schema formula matches every planned delta. The reviewer reran the frozen
observer binary, including denial of **all 47 actual allocation positions** of
its nontrivial descriptor lookup. Both cases pass. The raw no-remaining/late-native/
node controls refuse before the first prospective allocation hook; their XML and
transformations were independently checked rather than rerun.

Unknown/shipped/shape IDs refuse nonterminally at catalog.projections. The shared
shape_failure slot retains the first owner error before fallible response copying;
lookup, shape, validation, selection, render and moves consult it before new
admission. Inner and outer allocation adapters preserve this ownership and existing
conversion failures. Natural cumulative-counter exhaustion tests cover both
projection-first and shape-first resource errors, including exact code/message/
limit/maximum/field. This is deterministic charged-work exhaustion, not host-heap
exhaustion.

Additional [reviewer diagnostic allocation cases](../../../tests/native/v5-02-x05-c2-a-review-evidence/diagnostic.cpp)
deny each actual early protocol-refusal allocation for healthy embedded,
moved-from and selected-profile owners. Each returns typed redacted resource
failure at catalog.projections. Embedded/moved-from retry retains ordinary
protocol behavior; a moved-to owner remains healthy; selected-profile failures
stay terminal. A prior actual shape execution-terminal error also survives each
projection response-copy allocation refusal and owner move with all five fields
unchanged. These are controlled one-shot failures, without a persistent exhausted
heap guarantee. No source bytes, selector, traceback or substituted message enter
new diagnostics.

## Verification and preservation

Independent serialized Debug verification passes:

- Direct all/planar_tests build and final no-work build.
- **691** registered names, preserving all **682** pre-edit names.
- **15/15** projection/shape identity cases and **46/46** render/profile-render cases.
- **2/2** new interleaving/second-oracle cases, **2/2** full-operation allocation
  cases, and **2/2** new diagnostic allocation cases.

All executed test cases have zero failures/errors/skips. Exact argv, cwd, exit
status, timings and JUnit are in the [command records](../../../tests/native/v5-02-x05-c2-a-review-evidence/commands.json),
[diagnostic commands](../../../tests/native/v5-02-x05-c2-a-review-evidence/diagnostic-commands.json)
and [summary](../../../tests/native/v5-02-x05-c2-a-review-evidence/summary.json).
The first reviewer summary assertion expected 14 identity cases instead of actual
15; all commands and tests had exited zero. raw.py corrects the summary without
redundant reruns. One diagnostic harness compile-only attempt mixed the distinct
create() expected error types. Its source/log/commands are retained; only the
reviewer helper was corrected, and the final diagnostic compile/link/run passes.
Neither event was a product failure or candidate edit.

Author raw evidence independently verifies **425/425** focused and **11/11**
supplemental cases, direct identity9/render46, six common vectors, the new
projection oracle, six source/runtime audits, pinned formatting/Doxygen over118
active sources and no-work builds. All23 outer commands exit zero. These broader
checks were inspected, not redundantly rerun. This review adds no full C or
release gate; the recorded integrated shape gate remains676 per configuration.
Earlier approval limits remain unchanged. Public init creates format1. Read
services, capability/receipt/whole-invocation orchestration, public profile
admission, X06–X08, execution/RA01 and Linux remain open.

The [independent history audit](../../../tests/native/v5-02-x05-c2-a-review-evidence/history-check.json)
rechecks all **30,295** entries of the prior approved C1 reviewer checkpoint,
plus its **40** final artifacts, with only explicit candidate/coordinator paths
permitted. This checkpoint is correctly labeled prior approved evidence, not a
contemporaneous C2-A pre-edit whole-tree snapshot. The actual C2-A owned-file,
inventory/status and release baselines are separately preserved. All **30,347**
non-author-evidence source entries match both author freeze/final membership,
bytes, modes and symlinks. Reviewer before/after audits preserve all source/history
outside this report, all **3,492** release files and all **888** shared Debug
root/src/vendor object/PCM/archive members. The author's separately frozen enclosing
wrapper covers all888 members around its seven private commands; the inherited
narrower748-src-member driver claim is truthful and is not mistaken for the
complete enclosing proof. AGENTS.md remains the CLAUDE.md symlink.

## Reviewed candidate

| Identity | SHA256 |
| --- | --- |
| 233-member candidate manifest | `5f381b822ce94d5f3585052362525ffc2d4e9323469a545cf3f3a00d79d371a8` |
| Candidate diff | `525225af6a809c7d99628716d5aef26bfcb54e6b3a4ae9094093ebd4b6b3f65a` |
| 380-member author artifact manifest | `483f40062b10318027ee3fe2b2b967f4df31d08d6a72183b5e6ad8d076d6faad` |
| Author report | `fac8cd9e334f411abee0afcc3d96575ad42452cdb2f8287ee774644ce12ed19b` |

[Reviewer finalization](../../../tests/native/v5-02-x05-c2-a-review-evidence/finalization.json)
binds this report, reviewer artifact manifest, protected identities and exact
candidate. **Debug and all reviewer edits are released after final hashing; no
subsequent reviewer edits.** The coordinator must recheck this exact candidate
before recording approval or advancing C2-B.
