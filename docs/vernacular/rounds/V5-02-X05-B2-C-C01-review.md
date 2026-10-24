# V5-02-X05-B2-C+C01 independent review

Disposition: **request-changes**. C01 closes **B2C-R1**: snapshot storage no longer
corrupts retained registry references. A separate combined-boundary finding,
**B2C-R2**, blocks approval: an unknown-ID shape call suppresses a prior terminal
native resource failure from the definition lookup on the same owner.

Review used the shared `dev/planar-v2-vernacular` checkout at
`3171c49e7b92d3abda01bfbb8bfb0c6b29d4c67a`. Product, candidate support/prose,
contracts and historical evidence remained read-only. Reviewer writes are confined
to this report, `tests/native/v5-02-x05-b2-c-c01-review-evidence/`, and ordinary
serialized Debug verification outputs. Private copies compile to reviewer-owned
objects without replacing product source or libraries. No Planar tools, old
skills/roles/queue, installed binaries, operator data, commits, installation,
publication or merge were used. No implementation correction was made.

## B2C-R2 — Missing-ID admission suppresses an already terminal resource failure

The [round](V5-02-X05-B2-C.md) requires resource failures to remain terminal and
later calls not to suppress the first terminal failure. The approved B2-A lookup
likewise promises cumulative charges and terminal resource refusal. Its
[lookup entry](../../../src/engine/runtime/runtime.cpp#L2434) checks exhaustion
before selecting an ID. The new [shape entry](../../../src/engine/runtime/runtime.cpp#L2613)
checks only the newer `shape_failure` optional before selection. If the requested
ID is absent, [its early protocol return](../../../src/engine/runtime/runtime.cpp#L2627)
precedes `resolve_shape_definition`, so it never sees the pre-existing exhausted
native state. A lookup resource failure does not itself populate `shape_failure`.

The independent [production API regression](../../../tests/native/v5-02-x05-b2-c-c01-review-evidence/prior_resource_regression.cpp)
creates an ordinary selected-profile owner, repeatedly calls its actual public
native `resolve_shape_definition("custom")` operation until the existing cumulative
native ceiling is reached, then calls `invoke_shape` with missing and present IDs.
It uses the unchanged production runtime library, no fault injection, private
runtime substitutions, quota changes or shape F invocation. Its
[raw result](../../../tests/native/v5-02-x05-b2-c-c01-review-evidence/prior-resource-regression.log)
is:

| Operation | Code | Message | Field |
| --- | --- | --- | --- |
| Lookup after 35,421 successful lookups | `resource_limit` | `Native conversion allocation exceeded` | `catalog.shapes` |
| `invoke_shape("missing", {})` | `internal` | `Invalid external profile protocol` | `catalog.shapes` |
| `invoke_shape("custom", {})` | `resource_limit` | `Native conversion allocation exceeded` | `catalog.shapes[1]` |

Compilation/linking succeed; the regression deliberately exits **42** when the
missing-ID result fails to preserve the prior resource code/message. The earlier
observation-only run exits 0 and preserves the same sequence separately. This is
ordinary deterministic exhaustion of the logical cumulative work counter, not
natural host-heap exhaustion or RSS measurement. No F/provider execution after
exhaustion or public CLI exposure is claimed. The defect is observable native
error precedence and terminal diagnostic consistency.

**Required correction:** honor pre-existing terminal resource state before fresh
ID/admission protocol refusal, preserve the first terminal diagnostic, and cover
this cross-operation sequence and later retries. Preserve the ordinary
nonconsuming protocol behavior of healthy owners and the reviewed shape-failure
priority. Use a fresh corrective search/coder/reviewer round; do not alter this
candidate or failing evidence in place.

## B2C-R1 closed — Collision-free storage and exact retained identity

The [two private constexpr key objects](../../../src/engine/runtime/runtime.cpp#L29)
have distinct static address identities. Pinned Lua's
[`lua_rawsetp`](../../../vendor/lua/0b10/src/lapi.c#L945) and
[`lua_rawgetp`](../../../vendor/lua/0b10/src/lapi.c#L782) use light-userdata keys;
[`luaL_ref`](../../../vendor/lua/0b10/src/lauxlib.c#L710) uses integer keys from
its freelist/raw length. The snapshot keys cannot overlap any allocated reference.
All registry uses and setup entry points were inspected. No renumbering, reserved
integer range, mutable global state or new public hook was introduced.

Snapshot construction/publication remains inside the protected `execute_shape`
frame, with trivial native locals. The VM registry strongly owns successful
snapshots until destruction. Existing raw cleanup only reads snapshot membership
and deletes transient tag/backing entries; it retains mappings through response
conversion. A failure before the ready flag leaves no transient input graph to
clean and the owner becomes terminal. Destruction releases partial snapshots.

Independent owned copies of the inspected author observers are recompiled against
the current native dependencies; their transformation records reverse exactly to
the candidate source. The exact original observer reproduces the old baseline
[red exit 42](../../../tests/native/v5-02-x05-b2-c-c01-review-evidence/exact-red/registry.log)
(reference 10/11 become tables), then passes on the corrected
[green source](../../../tests/native/v5-02-x05-b2-c-c01-review-evidence/exact-green/registry.log).
The expanded observer passes embedded-only setup and all **15** retained bindings:
five shipped F and ten external D/F from two modules, covering a command, three
shapes and a projection. It compares reference number, type and exact original
`lua_topointer` identity, including unselected entries, across first/25 repeated
actual calls with explicit GC, execution failure, controlled codec resource
failure and terminal retry. Each owner checks zero live Lua accounting after close.

An additional reviewer-controlled persistent Lua growth refusal starts after the
first pointer snapshot publication. Its
[partial-state log](../../../tests/native/v5-02-x05-b2-c-c01-review-evidence/partial-snapshot-persistent/snapshot.log)
shows `ready=false`, `exhausted=true` after cleanup with all 15 exact bindings
preserved; retry and final destruction also pass. This is an explicit controlled
allocator adapter, not natural exhaustion evidence. A preliminary memory-limit
adapter also preserved all bindings and returned a typed resource error, but Lua
emergency GC allowed snapshot completion. The stricter preliminary observer
incorrectly asserted `ready=false`, aborted, and is retained as
`partial-snapshot-verified/` with its raw commands. The corrected persistent
adapter forces the intended partial state. This failed reviewer assumption is
not a product finding.

## Combined bridge checks and verification

Source review confirms definition binding precedes actual retained-F dispatch;
normalized five-field inputs preserve aliases and freeze transient proxies;
returned aliases convert before cleanup. Scratch remains writable. One actual
return is required; unsupported/cyclic values refuse. Raw conversion is separately
charged before reserved Glaze encoding enforces the complete 16 MiB transport
ceiling. Padding fixtures establish transport only, not shape semantics. The
registration-to-invocation transition occurs once; repeated invocation instructions,
native bytes and nodes remain cumulative. Shape-execution failures preserve their
first diagnostic and original shape ordinal. R2 concerns terminal state created
before that new optional is populated. Generic/public profile command invocation
remains closed before provider effects.

The independent Debug graph builds and registers the same **619** names. The
focused selection passes **353/353** in **201.93 seconds** (346 unit/native and
seven acceptance cases), and supplemental history/codec passes **11/11**; all
have zero failures/errors/skips/disabled cases. The independently compiled
production allocation sweep passes two cases/**482 assertions**, refusing each
of **67** observed actual native allocation requests, with embedded/moved controls.
Those denials are one-shot controls, not persistent exhausted-heap promises.
Six owned reversible lifetime/resource modes pass: 100 successful calls, codec,
instructions, Lua allocator, nodes and bridge diagnostic allocation. Successful
calls retain tags/backings/sealed counts **42/262/311**, live Lua **396607** bytes
after GC, and increasing cumulative work without refunds/resets. Pinned
formatting/Doxygen passes **118 active C++ sources**; final `all planar_tests`
reports `ninja: no work to do.`. Exact commands, timings, JUnit and failed probe
attempts remain in reviewer evidence. A first finalization link preflight stopped before writing manifests because the
linked finalization record had not yet been created; the corrected finalizer defers
that one forward reference. Its failed attempt is preserved. The author's vector families and six exact
source audits were inspected; they are not presented as independent reruns.

B1 semantic integration, B3 selected identity/W, B4 rendering, X06–X08,
public profile admission/cutover, mandatory external-profile acceptance,
execution/RA01 and Linux remain open. Public init still creates format1. No full
619-case milestone or release gate is claimed.

## Candidate and evidence identity

Before/after scope audits match all **15** C01 candidate members, **137** author
artifacts and **399** frozen source/configuration/fixture/support/current-prose
identities, including the AGENTS symlink. Separate audits match all **153** original
B2-C author artifacts and **36** original review artifacts. Historical evidence is
unchanged; the combined B2-C implementation was reviewed with its exact C01 delta.

| Reviewed C01 identity | SHA256 |
| --- | --- |
| Candidate manifest | `1f907aad8583fb4f1bcb0680659d8c7e37dcbdf7d5768ade54c2f069a4702130` |
| Candidate diff | `eda0f0c996b61fe795f76bd448fdedb91660b964f0af8d65122198f3f2def150` |
| Artifact manifest | `236919076118e249fb916f9dac3d96499787144df79bc4d2da2ba3d289cf9217` |
| Author report | `34c3edc31d9be5761f97ad84701de04a0b81ec80f7c0a147a0827619fc30684c` |

[Finalization](../../../tests/native/v5-02-x05-b2-c-c01-review-evidence/finalization.json)
binds this report, the reviewer artifact manifest and unchanged final candidate/
source audits. Debug ownership is released after hashing, with no subsequent
reviewer edits. B2C-R1 is closed locally; combined B2-C+C01 remains unapproved
because B2C-R2 requires a fresh corrective round.
