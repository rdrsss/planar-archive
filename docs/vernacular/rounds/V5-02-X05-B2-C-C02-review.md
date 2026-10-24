# V5-02-X05-B2-C+C01+C02 independent review

Disposition: **approve**. C02 closes **B2C-R2**; C01's closure of **B2C-R1** remains valid. The combined retained shape bridge meets this bounded native slice's acceptance criteria. No further blocking finding was identified.

Review used the shared `dev/planar-v2-vernacular` checkout at HEAD `3171c49e7b92d3abda01bfbb8bfb0c6b29d4c67a`, preserving its intentional dirty rewrite. Product, candidate support/current prose and historical evidence were read-only. Writes are confined to this report, `tests/native/v5-02-x05-b2-c-c02-review-evidence/`, and ordinary serialized Debug verification outputs. Private copies compile to owned objects and never replace product source/libraries. No Planar tools, old skills/roles/queue, installed binaries, operator data, commits, installation, publication or merge were used. No implementation fix was made.

## B2C-R2 closed — Shared first terminal diagnostic

The [lookup failure adapter](../../../src/engine/runtime/runtime.cpp#L2414) moves the complete first profile resource error into native owner `shape_failure` before returning a fallible copy. Both API entries consult that owned error before fresh selection/admission. Lookup's exhausted-state guard still precedes ID selection. The [shape failure and allocation adapters](../../../src/engine/runtime/runtime.cpp#L2602) also prefer the owned error, so an allocation refusal cannot replace its code, message, limit, maximum or logical field. The error is never moved out of the owner. A definition failure first reached during invocation keeps `catalog.shapes`; actual execution/codec failures keep the selected shape's original ordinal.

Both operations' [outer allocation boundaries](../../../src/engine/runtime/runtime.cpp#L2518) cover construction/copying performed inside the inner catch adapters. Under the scoped one-shot refusal model, a failed copy retries from existing owned state. Allocation failure before original diagnostic ownership can use the existing generic resource diagnostic. This does not claim typed return under persistent global host-memory exhaustion.

The independent [original production regression](../../../tests/native/v5-02-x05-b2-c-c02-review-evidence/prior-resource.py) is compiled against an owned copy of the exact preserved C01 runtime and then against the current production library. Both reach the existing cumulative native ceiling after **35,421** successful lookups, without quota changes, fault injection or F execution. The preserved-baseline [red run](../../../tests/native/v5-02-x05-b2-c-c02-review-evidence/prior-resource/red.log) deliberately exits **42**: missing ID returns protocol and valid ID changes the location. The current [green run](../../../tests/native/v5-02-x05-b2-c-c02-review-evidence/prior-resource/green.log) exits 0: both retain `resource_limit`, `Native conversion allocation exceeded`, and `catalog.shapes`. This is natural exhaustion of the deterministic cumulative work counter, not host-heap/RSS exhaustion.

The new production regression in the focused suite independently passes the full missing/wrong-kind/valid ID, null/valid input, repeated retry and later lookup matrix, comparing all five diagnostic fields. Additional [reviewer production probes](../../../tests/native/v5-02-x05-b2-c-c02-review-evidence/allocation.cpp) verify:

- A fresh owner's first lookup exhaustion occurs **inside `invoke_shape`**, before F; it keeps the lookup field and all original details. Moving that terminal owner preserves the same error. Every observed response-copy allocation is denied once for subsequent lookup and invocation.
- A prior execution protocol failure retains all fields and the selected original ordinal through subsequent lookup/invocation copy allocation refusals.
- Healthy embedded and moved-from operations retain the exact ordinary protocol code/message, empty limit, zero maximum and catalog field. Healthy profile missing/wrong-kind and malformed-input refusals remain nonconsuming; the moved-to owner subsequently executes its real F successfully.

The augmented production allocation runner passes **five cases / 137,578 assertions**, including the inherited **67-request** actual native allocation sweep and first-ownership/copy controls. The separate exact ordinary-admission case passes. These are one-shot controlled native refusals, not persistent exhausted-heap promises.

## B2C-R1 remains closed — Retained references and transient ownership

The [private immutable address keys](../../../src/engine/runtime/runtime.cpp#L29) remain disjoint from every integer `luaL_ref`. Source inspection confirms protected snapshot construction, strong registry snapshot ownership, exact membership cleanup, and response conversion before backing removal. Partial snapshot failure makes the owner terminal; destruction releases partial snapshots. Temporary input proxies are not published into the permanent sealed-identity set.

Independent owned copies recompile the exact original observer and the expanded reference matrix against the candidate. Their transformation records reverse exactly to runtime SHA256 `798e032cd31944ced27d8f476cfabd392fbedd1c03c5f70064707ea4b04338e8`. The [original observer](../../../tests/native/v5-02-x05-b2-c-c02-review-evidence/exact-green/registry.log) passes. The [expanded observer](../../../tests/native/v5-02-x05-b2-c-c02-review-evidence/registry/transform.json) preserves all **15** exact reference number/type/object identities: five shipped F and ten external D/F over two modules, including unselected command/shape/projection entries. Embedded setup, first/25 repeated real shape calls with GC, execution failure, controlled codec failure and terminal retry pass. Native destruction checks zero live Lua accounting.

The independent [persistent controlled Lua growth refusal](../../../tests/native/v5-02-x05-b2-c-c02-review-evidence/partial-snapshot/snapshot.log) starts after the first pointer snapshot publication and produces `ready=false`, `exhausted=true`; all 15 exact bindings survive cleanup/retry and destruction. It is a private allocator adapter, not natural host-memory exhaustion evidence.

## Combined boundary and verification

Source review and production tests confirm final owned definition binding before actual retained F dispatch in the original selected-profile VM; exact five-field structural admission; nested input alias preservation and immutable proxies; retained D/global immutability; writable local scratch; one actual return; and unsupported/cyclic return refusal. Raw Lua conversion charges precede allocation and remain distinct from reserved Glaze encoding, which enforces the complete **16 MiB** transport ceiling. Padding fixtures establish transport only, not B1 shape semantics. Registration transitions once to invocation instructions; repeated instructions, native bytes and converted nodes stay cumulative. Caught resource/bridge failures remain sticky before status/return adaptation. Generic profile command invocation remains closed before provider effects.

The independent Debug graph registers **620** names, preserving all prior 619. Focused verification passes **354/354** in **187.89 seconds** (347 unit/native and seven acceptance cases); supplemental history/codec passes **11/11**. Both have zero failures/errors/skips/disabled cases. Exact commands, JUnit and timings are in the [command record](../../../tests/native/v5-02-x05-b2-c-c02-review-evidence/commands.json).

Six independent reversible lifetime/resource modes pass: success, codec, instructions, Lua allocation, nodes and bridge diagnostic allocation. Across 100 successful calls, post-GC tag/backing/sealed counts stay **42/262/311**, live Lua stays **396607** bytes, and cumulative conversion work increases without refunds. Pinned formatting/Doxygen passes **118 active C++ sources**; the final product build reports `ninja: no work to do.`. Author vector families and six exact source audits were inspected, not independently rerun. Author development failures remain retained with truthful labels. The reviewer baseline exit42 is an expected regression result, not a failed current-candidate gate.

B1 semantic integration, B3 selected identity/W, B4 rendering, X06–X08, public profile admission/cutover, mandatory external-profile acceptance, execution/RA01 and Linux remain open. Public init creates format1. This approval does not claim a full 620-case milestone or release gate; the latest full approved gate remains A4's 582 per configuration.

## Candidate and evidence identity

Before/after [scope audits](../../../tests/native/v5-02-x05-b2-c-c02-review-evidence/scope-after-probes.json) match all **17** C02 candidate members, **179** author artifacts and **407** frozen source/configuration/fixture/support/current-prose identities, including the AGENTS symlink. A separate [historical audit](../../../tests/native/v5-02-x05-b2-c-c02-review-evidence/historical-audit.json) matches all 153 original B2-C author, 36 original reviewer, 137 C01 author and 145 C01 reviewer artifacts.

| Reviewed C02 identity | SHA256 |
| --- | --- |
| Candidate manifest | `38f773fc444870a3a601532585874ef775947e30a9506f56d1cb1a7e9a46a0aa` |
| Candidate diff | `8c8dd6edfcb550948dceef7e2484453aa587e5695815e2dbbbed690d96aca85a` |
| Artifact manifest | `75146ca56d00679f4576552fd9c5d399896b354d6f45cf34778caa9c3e6aa90a` |
| Author report | `2ef4cb20b172027f31ae847dea9b3ec94ebd4acb83248478629cb6fcdaa6518f` |

[Finalization](../../../tests/native/v5-02-x05-b2-c-c02-review-evidence/finalization.json) binds this report, reviewer artifact manifest, exact candidate identities and final source rehash. Debug build ownership is released after final hashing; no reviewer edits follow release. The coordinator must verify this exact candidate is current before recording combined approval or advancing dependent work.
