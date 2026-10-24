# V5-02-X05-A3+C01+C02 independent review

Disposition: **request-changes**. C02 corrects the ordinary R4 writer/native
location failures and preserves the approved C01 corrections. One concrete
allocation-failure path in the new location mapping still attributes a failure
to a different valid descriptor. A3-C02-R5 below blocks combined approval.
Preserve this candidate and all evidence; use a fresh corrective
search/coder/reviewer round before the integrated X05-A gate.

Review used shared `dev/planar-v2-vernacular` at HEAD
`3171c49e7b92d3abda01bfbb8bfb0c6b29d4c67a`. Writes are confined to this
report and `tests/native/v5-02-x05-a3-c02-review-evidence/`. Product, current
references, coordinator records, author and historical evidence remained
read-only. Other work was preserved. No legacy Planar tools/roles/skills/queue,
installed binaries, operator data, commit, installation, publication or merge
were used. Review held sole serialized build ownership and used the active
rewrite instructions, implementation/roadmap/v5 direction, exact
X-REG/X-LIMIT/X-STORE, bounded implementation path, C02 authorization and both
previous independent A3 reviews.

## A3-C02-R5 — Allocation failure while selecting a root retains the previous entry

[New location mapping](../../../src/engine/runtime/runtime.cpp):1149 constructs
and assigns the formatted catalog field before freezing each selected root.
Its bad_alloc catch at line 1159 retains the correct resource category/message,
but leaves `registration_field` at the preceding root. [The freezer loop](../../../src/engine/runtime/runtime.cpp):1224
then returns without freezing the selected root; constructor error adaptation
reads that stale string.

A narrow independent production-copy probe registers **ten valid projections**,
all with harmless Lua callables. It arms a replacement global `operator new`
immediately before the original production format/assignment for projection 10
and disarms it immediately after a successful assignment. The allocator refuses
its next real allocation, recording its requested size, clears its armed flag,
and throws `std::bad_alloc`. The production location catch executes. No exception
is inserted directly into the mapping body, no guard/freezer refusal policy is
changed, and no descriptor is invalid.

On the pinned libc++, formatting `catalog.projections[10]` requests **32 bytes**.
The actual result is:

```
resource_limit / External profile resource limit exceeded / catalog.projections[9]
```

The selected D root belongs to original projection 10. Projection9 is valid and
its D/F roots already completed freezing. This is the same incorrect attribution
class as R4 on a new handled native allocation boundary. A truthful current-root
location, or a truthful module-boundary fallback when location preparation itself
fails, must not identify projection 9. The detector expects projection 10 and
exits1, preserving the failure rather than presenting it as a passing product
assertion. The identical instrumented production copy with allocation refusal
disabled succeeds, records zero refused bytes, and both paths verify
`Lua cleanup live=0` after destruction.

[Exact reversible delta](../../../tests/native/v5-02-x05-a3-c02-review-evidence/actual-location-allocation/delta.json),
[probe source](../../../tests/native/v5-02-x05-a3-c02-review-evidence/actual-location-allocation/main.cpp),
[compile/link/run argv and statuses](../../../tests/native/v5-02-x05-a3-c02-review-evidence/actual-location-allocation/commands.json),
[actual allocation output](../../../tests/native/v5-02-x05-a3-c02-review-evidence/actual-location-allocation/actual.log),
[success control](../../../tests/native/v5-02-x05-a3-c02-review-evidence/actual-location-allocation/success.log).
This is controlled refusal of an actual allocation, not natural host-memory
exhaustion or a security escape. Registration refuses safely.

Correction should retain the selected logical location independently of a
potentially allocating string update, with truthful fallback if rendering fails.
A trivial native location token rendered at the outer error boundary, or an
equivalent allocation-safe mapping, can preserve the existing shared traversal,
protected Lua frames and diagnostic vocabulary. Implementation choice belongs
to the fresh corrective round. Cover the reachable two-digit projection boundary
and preserve original-order, mixed-kind/module, terminal and cleanup controls.

An earlier exploratory probe directly threw bad_alloc before short
`profile.modules[N]` formatting. It also retained the previous string, but it
**is not a separate finding**: those bounded short fields fit pinned libc++'s
short-string and format buffers, so direct exception insertion there does not
establish an actual reachable allocation failure. Its complete
[source/delta/outputs](../../../tests/native/v5-02-x05-a3-c02-review-evidence/location-allocation.py)
remain preserved. R5 depends on the actual 32-byte allocation probe above.

## Preserved corrections and combined boundary

The detailed guard selects the original zero-based root immediately before the
existing `audit.graph(root,1,true)` call, under one inspector and the same
identity/prototype memo. It retains no root for metadata/stack preflight
failures. The enum-only wrapper delegates to this traversal, preserving order,
depth, typed categories, PMR cumulative charges and stack restoration; unknown
exceptions still restore the top and propagate. No per-entry re-audit or inferred
shared-graph owner policy was introduced.

Module-only mapping counts original per-kind entries across prior modules and
the five shipped commands, maps both D/F roots to the same entry, and uses the
module boundary before root selection and before identity union. The root span
is native-owned and bounded: at most64 entries of each kind including shipped
commands, below the384-root array; callback kind values are native-installed.
The map refuses an unmatched pair. All-root audit still precedes any freezing;
only trivial mapping state crosses allocating Lua calls. Terminal checks precede
module retry location changes. Identity union is private and publishes once only
after complete success. Initial environment and repeated-root seams retain
previous field semantics. These successful paths do not resolve R5's allocating
field update.

The 36-combination ordinary-runtime matrix passes: early/middle/late writer or
direct-native F, all three kinds, interleaved entries and a second module with
prior entries. Exact existing category, message and one-based original field
are required. The original R4 ordinary cases are represented by this matrix;
author immutable failing binaries and recompiled passing detector outputs remain
preserved. The controlled real pre-root, partial-freezer and union paths also
pass exact location, same-owner terminal retry, unchanged published identity-set
identity and zero Lua allocation after destruction. Those injected Lua/protocol
faults are distinct from R5's actual native allocation refusal.

Original A3-R1/R2/R3 remain corrected. Pre-sort key structure is checked while
the original entry field is current; canonical-order validator diagnostics still
remap to original entries. Initial pure provenance resource results preserve
`resource_limit` before protocol adaptation. The six final canonical/payload
slots, separate framing/digest bytes, independently derived 13 metadata byte
passes/four graph passes, and 11 embedded/three external payload passes remain
unchanged. Checked cumulative charging precedes helper allocation, with no
refund/reset or RSS claim. Independent retained R1/R2 allocation/typed-result
probes and high/exact/one-byte-short budget controls pass; the high catalog is
1,041,459 bytes and its combined manifest1,041,840 bytes.

The combined A3 constructor still checks and privately copies bounded captured
profile/module bytes, order, roots and modes before evaluation. Source-only,
zero-result chunks execute in one cumulative bounded VM under native owner
ordinals. D/F references remain strong; final D conversion/copy and sealing occur
before the next module. The private external environment is created before
shipped evaluation from pristine curated globals, with self `_G`, constructor-only
engine and exactly three registration APIs. Exact native identities enroll before
module execution. Original embedded handlers retain their engine. Catalog and
exact effective provenance bind original bytes/modes/order; real B/C adapters
are not fabricated. Profile-owner invocation refuses before provider work.
Caller receipt verification and whole-invocation orchestration remain later work.

## Independent verification and author evidence

[Command record](../../../tests/native/v5-02-x05-a3-c02-review-evidence/commands.json)
independently builds Debug `all` plus `planar_tests`, inventories, runs the exact
focused selector and all six retained private runners, checks the high catalog,
pinned lint/Doxygen and a no-work build. Both builds report no work.
Inventory is **577 unique names**, preserving all 573 starting names.
Focused execution is **204/204**, zero failures/errors/skips/disabled cases,
83.35 seconds. [JUnit](../../../tests/native/v5-02-x05-a3-c02-review-evidence/focused.xml),
[matched names/counts](../../../tests/native/v5-02-x05-a3-c02-review-evidence/test-summary.json),
[raw output](../../../tests/native/v5-02-x05-a3-c02-review-evidence/focused.log).
Pinned formatting/Doxygen checks 117 active sources.

The repeated-root runner passes ten scenarios and its deliberate red control;
production and reachable-native runners each pass eight scenarios and their red
control. Corrective R1/R2 and budget runners pass. Their private copies exercise
retained dispatch/GC, exact iterator/proxy aliases, scratch, writers/natives,
cumulative exhaustion, native exceptions, terminal refusal and cleanup. Review
runner copies change only the output root; [reverse equality and hashes](../../../tests/native/v5-02-x05-a3-c02-review-evidence/runner-adaptations.json)
are recorded. These passing probes do not cover or supersede the additional R5
failure.

The author's 21-command final gate and raw vector/historical/lint/no-work logs
were inspected. Four vector checks and historical B2/D6/D6-C01/D7-B plus A0
exact-source checks pass; they are inspected author evidence, not independently
rerun claims. Final1's two-line indentation failure and the final2 leading-space-only
correction/refrozen full scope rerun are preserved. Earlier location retry and
obsolete fixture compile failures, and the disclosed development compile overlap,
remain recorded. Independent verification here was serialized.

[Scope audit](../../../tests/native/v5-02-x05-a3-c02-review-evidence/scope-audit.json)
reconstructs the exact owned diff against C01 starting copies and matches the
frozen diff. Shipped Lua, vendor and historical evidence remain unchanged;
AGENTS.md remains the CLAUDE.md symlink. C02 adds no CMake, codec or public runtime
interface change.

## Identity and release

[Before](../../../tests/native/v5-02-x05-a3-c02-review-evidence/before.json),
[after tests](../../../tests/native/v5-02-x05-a3-c02-review-evidence/after-tests.json)
and [final release](../../../tests/native/v5-02-x05-a3-c02-review-evidence/final-source.json)
independently compare 20,440 source entries including modes/symlinks,514 author
artifacts and 931 build identities: zero mismatches. Complete entry-set checks
exclude only root .git/build, C02 author evidence, this review evidence/report.
Author source freeze/after/release remain identical.

Author identities:

- Source freeze/release: `8beb5181756f4fc31f603c3eceda489f835b736c91b076abbc6ffd6bebfeac46`.
- Owned diff: `a749522841f12e0e5eb90b99023807eefe18ebcc3055dba47d5f5615033cbece`.
- Report: `774c02a56360314259ba2a1515dc96cb5b359985cd41fa7a5892577af0cf3114`.
- Artifact manifest: `124c42fed44f8c2a80aa9cb091787d8694bb265e593a47ba468ce7c69d267ed0`.
- Build identities: `8fb391b2dd86317d7e80a801962334126752f9d098479ab47c5400950766da8c`.

Review [finalization](../../../tests/native/v5-02-x05-a3-c02-review-evidence/finalization.json)
binds this report and review artifact manifest. Serialized build ownership is
released after final hashing; no subsequent reviewer edits follow release.

No full 577 suite, RelWithDebInfo/release or Linux gate was run. Latest full gate
remains 520 per configuration. Combined A3+C01+C02 and integrated X05-A approval
remain open because of R5. Public profile CLI, B/C semantics/adapters, X06 profile
history/storage acceptance and interpretation, caller orchestration, mandatory
external-profile gate, public cutover, execution/RA01 and Linux remain open.
Public init creates format1.
