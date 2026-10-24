# V5-02-X05-B3-A independent review

Disposition: **approve**. No blocking finding. This approves the bounded pure
trusted Lua prepare/check-one/finish factoring and its compatibility evidence.
B3-B native composition and authenticity, B3-C work identity and B4 rendering
remain separate required rounds.

Review used shared `dev/planar-v2-vernacular` at HEAD
`3171c49e7b92d3abda01bfbb8bfb0c6b29d4c67a`. The intentional dirty rewrite and
others' changes were preserved. Product, candidate support/current prose and
historical evidence were read-only. Writes are confined to this report,
`tests/native/v5-02-x05-b3-a-review-evidence/` and serialized ordinary Debug
verification outputs. No Planar tools, old skills/roles/queue, installed binaries,
operator data, commits, installation, publication or merge were used. No fixes
were made. Active instructions, implementation/roadmap/v5 direction, the exact
external-profile contracts, B3 search disposition and B1/B2 approvals informed
this review.

## Phase semantics and compatibility

[Preparation](../../../src/stdlib/planar/semantic.lua#L57) retains the original
normalization and mandatory validation pass. All global and job assignment,
reviewer, ownership, effective-check, registry, subscription/question and base
phase-cycle checks finish before any custom request is returned. Requests follow
normalized job order, omit shipped shapes and carry exactly the five normalized
F input fields plus logical native orchestration association. The trusted
structural context is a separate parameter and needs no callable; declaration
and envelope fields cannot select its dialect. Existing shipped schemas and
stage/worker-coverage rules remain intact.

[Check-one](../../../src/stdlib/planar/semantic.lua#L370) shares the previous B1
exact success/refusal, UTF-8/no-NUL guidance, obligation and count policies.
Per-job 10000 and cumulative 100000 guards precede expansion. Synthetic IDs,
assignment-derived roles and sorting remain the same. Malformed or refused
results stop the wrapper before a later callback. Cycle diagnostics still
accumulate in finish; a later malformed/refused callback retains the previous
fail-fast priority over an earlier additive cycle. The prior-total arithmetic
case proves 100000/100001 guard behavior, while the inherited real workload hits
the unchanged instruction ceiling before an exact full-bundle boundary. Neither
this review nor the author claims that full-bundle test was reached.

[Finish](../../../src/stdlib/planar/semantic.lua#L435) copies base/job records,
retains every original graph edge, adds unit-level obligations and reruns the
same deterministic complete-cycle algorithm in job order. Grouping does not
substitute a role self-wait. Exact result/request and definition index/job/shape
correspondence is checked. The output's selected_shape has exactly
`{id,revision,definition,obligations,guidance}` and no legacy sidecar. Supplied
metadata is explicitly trusted; these pure checks do not authenticate a native
owner or prove same-call provenance. Structural checked values, counts and
metadata must remain native-owned between phases, as documented in the
[phase ABI](../../../tests/native/v5-02-x05-b3-a-support/phase-abi.md).

The old wrapper composes the common helpers and keeps embedded output and its
B1 callback adapter/shape_results behavior. No shipped semantic or W recipe was
changed. Product Lua bytes and effective source provenance do change. The added
exports are internal product-selected operations; public profile invocation,
CLI admission and native retained-phase orchestration are not introduced.

## Independent behavioral verification

The [recorded Debug commands](../../../tests/native/v5-02-x05-b3-a-review-evidence/commands.json)
configure/build `all planar_tests`, register **627** names preserving all prior
**620**, and pass **361/361** focused cases in **203.65 seconds**, plus **11/11**
supplemental history/codec cases. There are zero failures, errors, skips or
disabled cases. [Raw evidence checks](../../../tests/native/v5-02-x05-b3-a-review-evidence/raw-evidence-audit.json)
match selected unique names, JUnit and immediately saved LastTest records for
both author and reviewer. All seven new production cases and inherited B1
semantic/golden regressions pass. Inventory is not a full 627-case run.

Four additional [production phase cases](../../../tests/native/v5-02-x05-b3-a-review-evidence/adversarial.cpp)
pass **305 assertions**. They cover multiple custom IDs/order, dialect injection,
all obligation scalar-field types, malformed refusal fields, invalid UTF-8/NUL
and oversized guidance, accumulated cycles versus later refusal, fail-fast call
counts, repeated finish with independent metadata, empty-array tags and wrong
metadata correspondence. Their [JUnit](../../../tests/native/v5-02-x05-b3-a-review-evidence/adversarial.xml)
and exact compile/link commands are retained.

A separate [same-VM observer](../../../tests/native/v5-02-x05-b3-a-review-evidence/identity.lua)
uses exact product helpers in the distinct private native adapter. It passes
**one case / 54 assertions** for both nonempty and empty obligations, checking
that finish leaves the original prepared bytes and graph unchanged, copies
base/job records, preserves shared bundle identity, retains graph
edges and keeps array tags exact. Its only source transformation replaces the
private helper's final export with an added observer; [reversal](../../../tests/native/v5-02-x05-b3-a-review-evidence/identity-transform.json)
is exact. This is pure helper evidence, not retained external F or native
metadata-authenticity evidence.

The independent [prechange/current wrapper comparison](../../../tests/native/v5-02-x05-b3-a-review-evidence/wrapper-comparison.json)
compiles the preserved original schema/semantic bytes into an owned source
module, retaining the unchanged exact native adapter. All **43** complete
encoded observations match byte-for-byte, including normalized F inputs,
callback counts, mixed shipped/custom jobs, mandatory errors, malformed Lua
values, refusal order and additive-cycle accumulation. The author's separate
ten-case production embedded comparison and existing W/render goldens also
pass in inspected raw records.

All five vector families, six exact source/variant audits and pinned
formatting/Doxygen (**118 active C++ sources**) pass independently. The final
`all planar_tests` build reports `ninja: no work to do.`. No full milestone,
release or Linux gate was run.

## Build restoration, isolation and candidate identity

The author preserves its first baseline reconstruction failure and intermediate
shared-PCM overwrite. Independent [restoration checks](../../../tests/native/v5-02-x05-b3-a-review-evidence/restoration-identities.json)
match all four recorded generated source/PCM/object/library identities.
Actual Clang module inspection names the production generated source, without a
private evidence source path. A separate [embedding audit](../../../tests/native/v5-02-x05-b3-a-review-evidence/production-embedding-audit.json)
compares every original embedded array with current product files and verifies
exact ordered assembly. The restored focused gate, these checks and no-work
build establish the current artifacts; the discarded intermediate PCM is not
represented as retained evidence.

The corrected author observer expands response files and redirects object and
PCM outputs. Reviewer observers use the same isolation discipline without
running obsolete unsafe versions. Every one of the four private compilations
preserves exact membership and bytes of all **748** shared object/PCM/library
files. Restored production identities remain unchanged afterward. Private
executables never replace product libraries. Initial reviewer path-resolution
and missing-std inspection setup errors are recorded; they stopped before their
intended check and required no candidate change.

[Scope and diff checks](../../../tests/native/v5-02-x05-b3-a-review-evidence/diff-and-history-audit.json)
reconstruct the exact candidate diff. All **11** archived originals match the
approved preceding snapshot. Relative to that snapshot, only ten owned existing
files change, four support files are added and no scoped member disappears;
the existing test-only helper remains unchanged. All **15** candidate members,
**168** author artifacts and **411** frozen source/configuration/fixture/support/
current-prose identities match. The predecessor's **179** author and **116**
reviewer artifacts remain intact. AGENTS.md remains a symlink to CLAUDE.md.

| Reviewed author identity | SHA256 |
| --- | --- |
| Candidate manifest | `0f2fc02dfb28b51e9b6002c4a7063724350b9c3fd2221502f466bae6e0ff4ff1` |
| Candidate diff | `a8c11635befbecafd8dd922968c16aaeeb37ab49ef55dd7bcad34bf557f92e91` |
| Artifact manifest | `f66e53a09a1b2b82df196bc4c3a30c985e30b8eb681f232ef1ffdd4109d9e0fe` |
| Author report | `7194b5b2b77c74729dd1cf43193cacb45a34d93db263bf481a06a406b0ecf7ca` |

[Finalization](../../../tests/native/v5-02-x05-b3-a-review-evidence/finalization.json)
binds this report, reviewer artifacts and final unchanged candidate/source
identities. Debug ownership is released after final hashing; no reviewer edits
follow release. The coordinator must verify this candidate is still current
before recording approval or advancing.

B3-B still owns same-owner retained F composition, immutable input/private root
sealing, original definition authenticity, lifetimes and cumulative phase
conversions. B3-C W, B4 rendering, X06 history/interpretation/acceptance and
receipt/whole-invocation orchestration, X07 public cutover, X08 mandatory external
profile acceptance, execution/RA01 and Linux remain open. Public init creates
format1. Latest full approved gate remains A4's 582 per configuration.
