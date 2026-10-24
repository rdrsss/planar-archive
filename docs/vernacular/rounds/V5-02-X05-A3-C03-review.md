# V5-02-X05-A3+C01+C02+C03 independent review

Disposition: **request-changes**. C03 corrects A3-C02-R5 in module sealing and
preserves the earlier corrections. A narrow audit of the analogous allocating
field updates found the same diagnostic defect at three earlier registration
sites. These sites are unchanged by C03; A3-C03-R6 below is a pre-existing
combined-candidate defect, not a C03 regression. Preserve this candidate and all
historical evidence. Use a fresh corrective search/coder/reviewer round before
the integrated X05-A gate.

Review used the shared `dev/planar-v2-vernacular` checkout at HEAD
`3171c49e7b92d3abda01bfbb8bfb0c6b29d4c67a`. Writes are confined to this report
and `tests/native/v5-02-x05-a3-c03-review-evidence/`. Product, current references,
coordinator records and author/historical evidence remained read-only. Other
work was preserved. No legacy Planar tools/skills/roles/queue, installed binary,
operator data, commit, installation, publication or merge was used. Review held
sole serialized build ownership, using active CLAUDE.md, implementation/roadmap,
v5 direction, exact X-REG/X-LIMIT/X-STORE, bounded implementation path, A3/C03
scope authorizations and the three previous independent combined reviews.

## A3-C03-R6 — Entry-field allocation failures still identify another valid entry

The combined constructor still uses allocating string updates before it retains
the selected original location in three earlier phases:

| Tested production site | Fixture | Actual result after one real 32-byte allocation refusal |
| --- | --- | --- |
| `register_external`, runtime.cpp:833 | Ten valid projections; projection1 in module1 and projections2–10 in module2 | `resource_limit / External profile resource limit exceeded / catalog.projections[9]` |
| `collect_final_descriptors`, runtime.cpp:2010 | The same ten valid projections | `resource_limit / External profile resource limit exceeded / catalog.projections[9]` |
| `validate_profile_catalog` original-index remapping, runtime.cpp:1983 | Eleven projections; only projection10 has invalid ABI, projection11 is valid | `resource_limit / External profile resource limit exceeded / catalog.projections[11]` |

The first two fixtures contain only valid descriptors and harmless Lua functions.
Native code already knows that projection10 is the attempted/selected entry, but
`registration_field = std::format(...)` throws before replacing projection9's
string. The existing bad_alloc adapters correctly refuse registration as a
resource error while reporting the preceding valid descriptor.

In the third fixture, pure validation identifies the invalid projection10, whose
canonical position differs from original order. The allocation in
`field = std::format("{}{}]", prefix, ordinal)` fails before the remapped field
can reach `registration_error` and then `registration_field`. The latter still
names the last collected projection11. The disabled-allocation control returns
exactly `internal / Invalid external profile protocol / catalog.projections[10]`.
There is one invalid descriptor; this assumes no precedence between competing
faults.

The independent probes use reversible copies of the exact frozen runtime and
unchanged production dependencies. Replacement global `operator new` is armed
immediately before each existing production format operation and disarmed after
success. It refuses the next actual allocation, records **32 bytes**, clears its
armed flag, and throws bad_alloc. No direct exception is inserted at a
nonallocating selector. The ten-valid-projection no-fault twin succeeds; the
invalid-ABI no-fault twin reports original projection10. Every path checks zero
tracked Lua allocation after destruction. The detectors return **1** when the
stale diagnostic is reproduced, preserved as expected failing product controls.
This is controlled actual native allocation refusal, not natural host exhaustion
or a capability/sealing escape. Registration refuses safely.

[Register/collection generator](../../../tests/native/v5-02-x05-a3-c03-review-evidence/field-allocation.py),
[reversible delta](../../../tests/native/v5-02-x05-a3-c03-review-evidence/field-allocation/delta.json),
[argv/status](../../../tests/native/v5-02-x05-a3-c03-review-evidence/field-allocation/commands.json),
[register output](../../../tests/native/v5-02-x05-a3-c03-review-evidence/field-allocation/run-register.log),
[collection output](../../../tests/native/v5-02-x05-a3-c03-review-evidence/field-allocation/run-collect.log),
[valid control](../../../tests/native/v5-02-x05-a3-c03-review-evidence/field-allocation/run-control.log).
[Remap generator](../../../tests/native/v5-02-x05-a3-c03-review-evidence/field-remap-allocation.py),
[delta](../../../tests/native/v5-02-x05-a3-c03-review-evidence/field-remap-allocation/delta.json),
[argv/status](../../../tests/native/v5-02-x05-a3-c03-review-evidence/field-remap-allocation/commands.json),
[refusal output](../../../tests/native/v5-02-x05-a3-c03-review-evidence/field-remap-allocation/run-remap.log),
[semantic control](../../../tests/native/v5-02-x05-a3-c03-review-evidence/field-remap-allocation/run-remap-control.log).

X-LIMIT requires one-based original-order logical fields. A truthful selected
entry or truthful module/catalog boundary fallback must not name an unrelated
valid descriptor. Correct this class across registration-field mutation and
error adaptation, retaining the selected logical location independently of
allocating string preparation/propagation. Preserve C03's successful seal-token
fix and existing diagnostic vocabulary, cumulative budgets, protected frames,
shared traversal and terminal semantics. The correction should audit all such
field preparation/propagation paths together; only the three sites above are
claimed as independently demonstrated allocation failures.

[Scope audit](../../../tests/native/v5-02-x05-a3-c03-review-evidence/scope-audit.json)
compares all three enclosing functions byte-for-byte with the preserved C02
starting runtime: all unchanged. This is an earlier residual allocation-boundary
attribution defect discovered during combined review. It does not reopen the
ordinary malformed-key R1 correction or invalidate C03's R5 correction.

## C03 correction and preserved combined boundary

**A3-C02-R5 closes for sealing.** Root selection now writes only a trivial
kind/index token. Both D/F roots retain the same original per-kind index, counting
prior modules and the five shipped commands. No allocation or formatting occurs
in the selector. Before-root and identity-union failures select the module token.
The detailed guard still uses one inspector and identity/prototype memo, the same
root order/depth and all-root audit before any freezing. The enum-only API remains
a delegate. Initial embedded/environment and repeated-root seams keep their old
field semantics.

Before registration modules run, two independent module-field strings per module
are staged while the public field is `profile`. The checked, already bounded
module count is at most 64, so multiplication by
`sizeof(module_fields) + 2*32` cannot overflow; cumulative `charge()` runs before
reserve/copy. Each field is at most 19 bytes plus NUL, below its 32-byte logical
payload reservation. This accounts for both string objects and independent
payload capacities without relying on SSO. It is the existing logical charged
native metric, not an RSS or allocator-overhead bound. There is no refund/reset
or new quota. The completed vector publishes by move.

An active module string installs by swap. A separately staged fallback restores
the active module after descriptor collection and before protected sealing.
The catch-all protected-call adapter handles exhaustion, Lua memory/status,
stack, bridge diagnostics, terminal state and native exceptions, rendering only
outside Lua. A temporary string publishes by swap only after successful rendering;
actual rendering bad_alloc preserves the preinstalled module fallback and becomes
the existing external resource diagnostic. Outer constructor adapters move the
owned field into the result. Terminal retries check failure state before changing
the first token. The public constructor discards failed owners.

Independent C03 allocation probes pass all ten scenarios: valid success, both
D/F token selections with zero allocation requests, actual 32-byte outer-render
refusal with `profile.modules[2]`, actual 96-byte staging reserve refusal at
`profile`, exact projection10 writer control, actual Lua-growth refusal, active
frame stack check, controlled partial-freeze Lua status, and controlled throwing
allocator adaptations. The latter are explicitly controlled adapter tests, not
claims that production realloc throws C++ exceptions. Failed protected returns
retain terminal token and published identity-set identity; every mode cleans up.
Ordinary index10/64 controls cover all three kinds and mixed module/kind inputs.

R1's original-field pre-sort key checks and canonical remapping still work in
ordinary execution. R2 preserves typed initial provenance resource refusal. R3's
six canonical/payload slots, separate framing/digest room, independently derived
13 metadata byte passes/four graph passes, and eleven embedded/three external
payload passes remain unchanged. Retained R1/R2 and high/exact/one-byte-short
budget probes pass. The high catalog is 1,041,459 bytes and the combined manifest
is 1,041,840 bytes. R4's ordinary unsafe-callable location controls remain correct.

The constructor still structurally checks and privately copies bounded captured
profile/module bytes, order, roots and modes before evaluation. Source-only,
zero-result modules run in one bounded VM under native owner ordinals. Final D
is validated/copied, and D/F seal before the next module. The private external
_ENV is created before shipped evaluation with constructor-only engine, self _G,
shared pure libraries and exactly three registration functions; exact native
identities enroll before module execution. Embedded handlers retain their original
engine. Original captured bytes/modes/order and catalog bind effective provenance.
Profile-owner invocation refuses before provider work. No shipped Lua or real B/C
adapter/semantic behavior is added or inferred.

## Independent verification and identity

[Command record](../../../tests/native/v5-02-x05-a3-c03-review-evidence/commands.json)
builds Debug all+planar_tests, inventories, executes the exact focused selector,
recompiles/runs the seven C03/retained private runners, checks high catalog,
pinned formatting/Doxygen and a no-work build. Both builds report no work.
Inventory is **578 unique names**, preserving all 577 prior names.
Focused execution is **205/205**, zero failures/errors/skips/disabled cases,
83.05 seconds. [JUnit](../../../tests/native/v5-02-x05-a3-c03-review-evidence/focused.xml),
[matched names/counts](../../../tests/native/v5-02-x05-a3-c03-review-evidence/test-summary.json),
[raw output](../../../tests/native/v5-02-x05-a3-c03-review-evidence/focused.log).
Pinned formatting/Doxygen checks 117 active sources.

[Runner adaptations](../../../tests/native/v5-02-x05-a3-c03-review-evidence/runner-adaptations.json)
prove exact reverse equality after output-root-only changes. Repeated roots,
locations, production dispatch, reachable natives, R1/R2 and capacity probes pass,
including their deliberate red controls. These passing author-designed probes do
not supersede R6's additional independent failing detectors. Verification was
serialized; private probe objects/executables are confined to this review evidence.

The author's 22-command gate, raw historical/vector/A0 audits and red R5
reproduction were inspected. Four vectors, historical B2/D6/D6-C01/D7-B and A0
source assertions are inspected author evidence, not independently rerun claims.
The full C03 owned diff reconstructs exactly from eight original files and all
new support files, including the red generator created before the author baseline.
An initial review-only reconstruction used `a/path` instead of `/dev/null` for
new-file diff headers; its discrepancy remains preserved. Correcting that diff
header convention yields an exact match; no source or author evidence changed.

[Before](../../../tests/native/v5-02-x05-a3-c03-review-evidence/before.json),
[after tests](../../../tests/native/v5-02-x05-a3-c03-review-evidence/after-tests.json)
and [final release](../../../tests/native/v5-02-x05-a3-c03-review-evidence/final-source.json)
verify **21,172 source entries** including modes/symlinks, **284 author artifacts**
and **931 build identities**, with zero mismatches. Complete source entry-set
checks exclude only root .git/build, C03 author evidence, this report and this
review evidence. AGENTS.md remains the CLAUDE.md symlink. Shipped Lua, vendor,
historical evidence and C03 guard/API/build/codec scope remain unchanged.

Author identities:

- Source freeze/release: `00aedef73482d57d51b3578faf51edb2f0702bc3fb5d66d76bfdf03430416fdb`.
- Complete owned diff: `d55974b8d82846851b58d76dcf71c761ed5b4b3492076960ade7d35ab8f61532`.
- Report: `c61cbff4c5be4e199dac64783ec36eb4ee733c746e9651ff54bba720c7362205`.
- Artifact manifest: `b428790057cccda22dd869b3b2f515a1c989a953b1bf32d3b8762bac8a3d13f0`.
- Build identities: `68d4f3375c744266173946a66ba16becb645fa8e3c4e486e9785b0289be8fe37`.

Review [finalization](../../../tests/native/v5-02-x05-a3-c03-review-evidence/finalization.json)
binds this report and review artifact manifest. Serialized build ownership is
released after final hashing; no subsequent reviewer edits follow release.

No full 578 suite, RelWithDebInfo/release or Linux gate was run. Latest full gate
remains 520 per configuration. Combined A3 approval and integrated X05-A gate
remain open because of R6. Public profile CLI, B/C semantics/adapters, X06 profile
history/storage interpretation and acceptance, caller receipt/budget orchestration,
mandatory external-profile gate, public cutover, execution/RA01 and Linux remain
open. Public init creates format1.
