# X04-B integrated effective provenance evidence

Status: the pure B1 profile decoder/tree-budget, B2 catalog and B3 effective
provenance slices are independently approved. B4 is a verification-only candidate
pending independent review; it does not yet close integrated X04-B. The mandatory
external-profile gate remains open.

The shared `dev/planar-v2-vernacular` candidate starts at
`3171c49e7b92d3abda01bfbb8bfb0c6b29d4c67a`. B4 adds no product code or tests.
Its inventory retains all 520 existing names: 489 approved A3 names, eight B1,
eleven B2 and twelve B3 cases. The [B3 review](rounds/V5-02-X04-B3-review.md)
explains why existing pure composition already covers this integration; no
redundant wrapper or fake interner is added.

## Pure composition and limits

Contracts remain [X-STORE](external-profile-contract.md#x-store--byte-codecs-exact-provenance-and-supported-formats)
and [X-LIMIT](external-profile-contract.md#x-limit--cumulative-ceilings-and-errors).
CTest registers Catch2 names with an additional `replacement.` prefix. Complete
actual names, including inherited cases, are in the per-configuration inventories
and JUnit, and `pure-composition-test-names.json` selects this pure module scope.

| Obligation | Source and concrete executed cases | Claim boundary |
| --- | --- | --- |
| Pure strict ordered profile decoding | `capture/profile_codec.t.cpp`: `profile codec decodes exact ordered paths in memory`, `keeps strict schema and arbitrary integer classification`, `preserves path policy and logical error fields`, `enforces bounded strings arrays nodes depth and input` | Extraction preserves the approved native profile schema and error vocabulary. No host access or Lua. |
| Distinct effective tree budget | `history/tree_core.t.cpp`: `tree core effective provenance counts expanded file occurrences`, `file boundary validates before callbacks`, `byte budgets preserve original ceilings`, `preserves lower path depth and file limits` | B1 checks exact outer 32 MiB/8192-file boundaries through real tree primitives, retaining lower path/depth/per-file limits. It does not establish a reachable complete effective provenance tree of that size. |
| Immutable exact catalog-v2 structural sealing | Eleven `replacement.effective catalog ...` cases in `history/effective_catalog.t.cpp`, including mandatory shipped signatures, static read capability signatures, history ID matrix, source ownership/ordinals, closed schemas, ordering/conflicts and canonical/count ceilings | B2 validates explicit original owner order, exact shipped native signatures, external static capabilities and bounded retained canonical values. The catalog-only 1 MiB bound is separate from the combined manifest bound. Callability is not proved. |
| Exact combined provenance reconstruction | `replacement.effective provenance independent exact bytes and digest`, `raw whitespace modes and source bytes affect identity`, `profile module order and correspondence`, `embedded source order assembly and catalog owners`, `external owners bind profile ordinals` | B3 prepares and independently structurally verifies exact `effective_stdlib_v1` trees from old embedded originals/assembly, raw profile and ordered modules, explicit owner ordinals and canonical catalog. Full file bytes/modes and portable record-v1 digest are compared. |
| Closed structure, complete membership and applicable bounds | Remaining B3 cases: `full membership and modes`, `closed exact manifest and framing`, `component and defensive outer resource refusals`, `reachable external byte and profile boundaries`, `prefixed path byte and depth limits`, `combined manifest one MiB ceiling`, `maximum reachable file membership` | Combined canonical manifest is at most 1 MiB. Valid complete inputs reach at most 4162 files and remain below 32 MiB under stricter component limits; the outer 8192/32 MiB budget is independently checked by B1. Ceilings do not guarantee capacity/RSS or invocation-wide accounting. |

The independent [B1](rounds/V5-02-X04-B1-review.md),
[B2](rounds/V5-02-X04-B2-review.md) and [B3](rounds/V5-02-X04-B3-review.md)
reviews bind exact approved candidates and retain prior focused evidence and its
limits. B4 executes their combined current product/test bytes in both full gates.
Old canonical, record-v1, embedded provenance and work-fingerprint fixtures remain
unchanged. Structural verification accepts internally consistent changed bytes
with their corresponding new digest; later storage must compare the expected
recorded digest. This round introduces no digest-based reuse implementation.

## Actual integrated gates

Serialized Debug and release-preset **RelWithDebInfo** each execute exactly one
full CTest suite: **520/520**, zero failures, errors, skips or disabled cases,
every testcase status `run`. Actual JUnit names exactly match each fresh
520-name inventory and all prior 520 B3 names. The immediate `LastTest.log` copy
contains the same 520 names and 520 `Test Passed.` results; `CTestCostData.txt`
is retained immediately with it. Cost data is supporting history, not a separate
execution count. No helper reruns either full suite.

Commands in each configuration are `cmake --preset debug`/`release`,
`cmake --build build/rewrite-<configuration> --target all planar_tests`, fresh
`ctest --show-only=json-v1`, and `ctest --output-on-failure --output-junit` with an
absolute result path. Both configurations then pass:

- Canonical vectors: 12 hand byte, 20 upstream hash and 41 canonical/domain checks;
  10 independent record-v1 and two independent request-v2 vectors.
- The independent effective-provenance generator `--check`: 12 established
  canonical known-byte and four portable-record byte/digest oracle controls,
  followed by five fixed effective-provenance artifacts.
- Four exact current runtime/CLI audits: historical B2, R14-D6, D6-C01 and D7-B.
- All five real storage/export generated families (`r12c2`, `r13b1`, `r14c3b2`,
  `r14f1`, `v5-02-x03-b2-b-c01`): current source/generated SHA256, declared
  substitution counts, exact forward generation and exact reverse source recovery.
  These inherited production-interner variants are distinct from pure B3 tests;
  no fake collision proof is claimed.
- Pinned formatting/Doxygen for 113 active C++ sources and a final
  `all planar_tests` build reporting `ninja: no work to do.`.

Raw commands, argv, UTC times, exits, durations, JUnit, immediate CTest temporary
copies, vectors/audits, inventories and per-pass manifests live under
[`tests/native/v5-02-x04-b4-evidence/attempt01/`](../../tests/native/v5-02-x04-b4-evidence/attempt01/).
Both build configurations, executable/object/module hashes and build graph
identities are retained. Gates run on macOS ARM64; no Linux result is inferred.

## Candidate integrity and remaining work

The pre-edit full filesystem manifest excludes only root `.git/`, root `build/`
and this new evidence directory. Nested historical build paths remain included;
original owned docs, git status/diff/HEAD and exclusions are saved. A complete
pre-command freeze and source archive bind the support and current product/test/
configuration candidate. Before and after every gate command, complete filesystem
equality with the freeze is required. Both gates passed unchanged.

The seven existing status documents and this evidence map are finalized afterward.
`post-gate-doc-stability.json` proves these are the only differences from the
tested freeze, with product, tests, support and configuration unchanged.
`ownership-audit.json`, `candidate-manifest.json`, exact `owned.diff`, final-owned
copies, final filesystem and artifact manifests bind the finished candidate.
`AGENTS.md -> CLAUDE.md` remains intact. B4 does not regenerate historical evidence,
fixtures or goldens. No commit, installation, publishing, merge or operator data
access is performed. Both build directories are idle on handoff.

This gate proves pure structural composition only. Live external registration and
Lua execution, public profile CLI, effective provenance storage/profile acceptance,
recorded interpretation-v2/history dispatch, current-policy/callability/semantic
honesty, whole-invocation shared budgets, public format2 cutover, X08 acceptance,
execution/RA01 and Linux remain open. Public init still creates format1. Independent
B4 review and a current-candidate identity check are required before integrated
X04-B closure or advancement to dependent work.
