# R19-AUTHORING-C01 — corrective accounting map

Search baseline: the uncommitted R19-AUTHORING candidate on
`dev/planar-v2-vernacular`, HEAD `d6c989ed9e196bf9b357019ea946e6ff18c47f18`.
The inherited 305-file source/support aggregate is
`161c575fc532e61ef6debd2183e019bd6d9357b17e98f255c8fce57a52d29b3b`.
This is a read-only search and proposed plan, not implementation or approval.
The ordinary available model substitutes for the unavailable prescribed Claude
search model. Only this map and the two assigned canonical plan tables were
written. No build, candidate CLI, ledger, operator data or installed binary was
invoked. The shared checkout and all prior evidence were preserved.

Canonical [decisions](../../../tests/native/r19-authoring-c01-evidence/plan/DECISIONS.md)
and [invariants](../../../tests/native/r19-authoring-c01-evidence/plan/INVARIANTS.md)
are part of this proposal. The controlling new finding is R1 of the
[independent review](R19-AUTHORING-review.md); A1 is records-only.

## Confirmed defect and authority

`command-contract.md:203–213` requires cumulative native allocation accounting
across all transfers, with terminal exhaustion. The budget interface explicitly
charges before allocation and never refunds (`src/engine/budget/budget.cppm:7–18`).
`runtime.cpp:149–155` performs checked cumulative additions and retains the
native-bytes failure for command VMs.

The new `bind_embedded_execution` (`runtime.cpp:6019–6053`) prepares the correct
VM-owned provenance identity but charges only assembly plus originals once,
65,536 bytes and a small per-original allowance before `prepare_stdlib`.
This does not cover the actual library call. Let:

- `A` be the retained assembly byte length;
- `O` be the sum of retained original payload lengths;
- `P = A + O`;
- `n` be the number of originals;
- `M` be the canonical assembly-manifest byte length.

`history/provenance_codec.cpp:71–113` builds the manifest graph, encodes its
canonical bytes and passes originals, assembly and manifest to `own`. That
function first hashes the tree and only then makes the owned tree:

| Allocation or transfer | Source | Payload cost |
| --- | --- | --- |
| File-record payload copies for one tree digest | `tree_core.cpp:163–165` | `P + M` |
| Independently encoded record buffers passed to XXH3 | `record.cpp:121–157`, `:256–258` | `P + M`, plus record framing |
| Final owned tree payloads | `provenance_codec.cpp:60–63` | `P + M` |
| Initial manifest encoding | `provenance_codec.cpp:86–90` | `M` |

Thus **at least `3P + 4M`** is required before graph, path, tree, directory,
framing, vector/object and digest-text work. Destroying a temporary does not
refund a cumulative metric. The initial review independently measured authoring
`P=536709` and production `P=463972`; even `3P` exceeds the old reservation by
far more than its 65,536-byte allowance.

`register_profile` already explains the same two digest copies plus one owned
copy (`runtime.cpp:4614–4673`). Its eleven-pass model applies to its larger call
chain, not directly to this single preparation. That model is a useful audited
scratch-bound vocabulary; the individual allocations above establish this fix.

## Proposed bounded source repair

Keep the fix in the new binder's private accounting block, optionally a private
helper in the same `.cpp` implementation. No interface/API or digest recipe
change is needed. Validate the provenance input count/size bounds before
arithmetic; then reserve a conservative complete cost before the first call to
`history::prepare_stdlib`:

1. Three passes of each original payload and assembly, with checked multiply.
2. Four passes of exact canonical metadata `M`, plus its constructed value graph
   and codec traversal scratch. The existing profile expression for `M` is
   `125 + separator.size() + sum(9 + path.size())`; derive/check it against the
   codec schema rather than treating 65,536 as sufficient for arbitrary input.
3. One digest's file framing for `n+2` files. The current record framing is
   `string_view("PLANAR-RECORD\x01").size() + 1 + 4 + 8`.
4. A per-path tree/directory reservation for every original and the actual
   synthetic paths `__planar__/assembly.lua` and
   `__planar__/assembly.canonical`. `tree_core.cpp:122–181` constructs folded/name
   strings, nodes/children maps, directory children, sorting and encoded directory
   records. The profile's component-bound model reserves per component plus
   path/name storage; use the real path spellings and one digest pass here.
5. Owned `tree_file` and view vectors, metadata/value nodes and remaining bounded
   path/digest text. A conservative per-input graph/object reservation is valid
   when its derivation is stated. The existing `(n+16)*4096` graph and
   component `2048`/path-string bounds are reusable only after checking their
   coverage for this call; they are not an RSS or allocator-growth promise.

Use an overflow-safe reserve helper analogous to the existing profile helper:
check `bytes <= memory_max / passes` before multiplication, then call `charge`.
Check sums against the relevant 16 MiB provenance bound before additions.
Do not raise production ceilings or refund/reset a native counter. Preserve
the existing native resource vocabulary and the outer terminal release path.
No fallible derivation moves ahead of invocation's saved-response preludes.

The selected digest computation and `bind_execution_stdlib` call remain exactly
as in the candidate. Bare status/request lookup, check-recover, history/export
and authoring ledger reads retain their exemptions; profiles retain their
verified effective identity. No production-source fallback returns.

## Runtime regression without a public product hook

The embedded `command_vm` exposes no observational native counter. Profile
owners do, but profiles bypass this binder and cannot serve as the regression.
Blindly selecting a low fixed native ceiling would couple the assertion to
registration/closure costs and ABI changes. Use a **private verification-only
compiled runtime composition**, with one audited preconsumption insertion:

- Copy the exact runtime implementation/interface under a new module/namespace,
  using the original compiled `planar.engine.runtime_source`.
- Immediately before the real `bind_embedded_execution` call, a test-only block
  reads a decimal remainder from activate `request_id` or status `request_id`.
  These are normal typed fixture inputs; no product CLI or environment flag is
  added. Validate the remainder against current available room, then use the
  real `charge(s, available - remainder)` to preconsume. Never assign/reset or
  refund `s.native_bytes`.
- Product runtime source is unchanged by that insertion. The generator must
  record exact substitutions, require the insertion anchor once, reverse them
  to original bytes, and emit only changed bytes. This is an explicit private
  test composition, not another product observation API. Historical
  `tests/native/v5-02-x05-a3-c02-support/budget.py` demonstrates private
  remaining-budget probes; its direct counter assignment should **not** be
  copied when checked `charge` preconsumption suffices.
- Reuse `commands.t.cpp`'s finite fake execution provider. Both shipped activate
  and exempt status handlers first call `read_execution`; return the same exact
  `ledger_busy` native failure there. No ledger/filesystem access occurs. Status
  with a request ID and no role remains identity-exempt.

After fixture preconsumption, registration and option preparation costs have
already happened. Binary-search the minimum remaining native room that lets
each of these two operations finish with the normal native-failure envelope.
Record actual boundaries and require the bound-minus-exempt gap to be at least
`3P`, deriving `P` from the same existing embedded source module's exported
original byte views and assembly (or an independent prepared provenance oracle).
The activation invocation carries additional option fields and both early native
failures have the same shape; audit this side-cost assumption during test review.
No helper reservation formula or registration-peak constant is copied into the
test. A generous fixed upper bracket (for example 64 MiB, below the unchanged
512 MiB owner ceiling) and zero-room lower bracket must be proved, not skipped.

Additionally leave `2P` room: the corrected reservation must refuse with
`resource_limit/native_bytes` **before any binder/provider call**, and a second
invoke must refuse as consumed. A generous-room control must bind the exact
digest and reach the expected native failure. The present one-pass code is
expected to reach the provider or fail the boundary-gap assertion; **the test
coder must demonstrate actual compiling assertion-red before implementation**.
If an unforeseen side cost prevents that red, report it and adjust the private
fixture transparently; do not weaken the quantity criterion or count setup/
compile failure as red.

This test directly observes charge quantity and terminal failure. A resource
failure before provider calls alone cannot prove the pure codec was never
entered. Preserve an independent source/order audit, and optionally a private
preparation-entry poison control in the mutation checkout: low room must refuse
before the poison, whereas moving reservations after preparation must expose it.
Report this distinction rather than calling a source-order check a runtime
allocation observation.

## Other callers and scope decision

| Caller | Existing code | Classification |
| --- | --- | --- |
| Embedded `accept_load`, `runtime.cpp:3570–3573` | One payload pass before the same three-pass preparation | Confirmed preexisting related undercharge; not evidence for the new binder. |
| Profile registration, `:4614–4674` | Eleven embedded payload passes for initial preparation plus effective preparation | Separate, explicitly derived model; preserve it. |
| Profile-load phase 4, `:7117–7150` | `total*4` before prepare, effective prepare and effective verify | Preexisting related accounting requires its own complete chain audit; embedded payload lower bound is 22 passes, not four. |

For phase 4: initial preparation costs three embedded payload passes;
`prepare_effective_stdlib` performs `verify_stdlib` (two digest plus three nested
prepare passes) and final digest/owner (three), totaling eight; subsequent
`verify_effective_stdlib` performs initial digest (two), reconstructed embedded
ownership (one) and another effective preparation (eight), totaling eleven.
The full chain is therefore `3+8+11=22`, before metadata and directory work.
See `effective_provenance.cpp:123–215` and `:218–301`.

**Recommended C01 scope is the new binder only**, matching formal R1's explicit
scope instruction. Record both older sites as known follow-up accounting work;
do not claim they are correct or silently share a three-pass helper across a
twenty-two-pass chain. A broader family repair is legitimate only after a scoped
plan amendment with full chain accounting and tests. It could affect C07's
private 51 MiB native/29M instruction/37,500-node calibration: its 25 percent
headroom and coupled smallest-ceiling rule cannot be relaxed to hide a charge.
This map does not close those older sites.

A separate preexisting performance lead explains some configure-time rebuild
work without source-byte changes: `v5-02-b1-support/generate.py` unconditionally
writes its generated command/runtime/interface files. Its output timestamps can
therefore invalidate native targets on explicit reconfiguration even when their
content hashes stay equal. This does not establish every retry build's cause,
and no performance fix is included here. The new private generator must use
write-only-if-different behavior from the outset.

## Packages, ownership and checks

1. **Test-first coder:** new
   `tests/native/r19-authoring-c01-support/generate.py`, the private harness
   wiring in `src/engine/runtime/CMakeLists.txt`, and new colocated quantity/
   insufficient-room tests in `src/engine/runtime/commands.t.cpp`. Add no product
   hook or Lua API. Generator and tests freeze after real assertion-red.
2. **Implementation coder:** only the new binder accounting/private helper in
   `src/engine/runtime/runtime.cpp`; receive CMake ownership only if a necessary
   explicit dependency adjustment remains. Test assertions and injection
   generator are read-only. No existing load/profile reservation rewrite without
   the reviewed scope amendment above.
3. **Coordinator:** corrective brief, actual task declarations/repair dispatch,
   roadmap/reference status, evidence identities and A1 table formatting. Keep
   old successful gates/probes attached to their rejected candidate, not this fix.
4. **Fresh prober/reviewer:** own separate reports and scratch verification only.
   Restore one-pass and two-pass quantity mutants; deletion/order/overflow and
   preconsume-no-op mutants; rerun affected runtime identity/exemption/owner rows.
   Carry only byte-identical unaffected source footprints with explicit proof.

Proposed focused commands, after coordinator lane authorization:

```sh
cmake --preset debug
cmake --build build/rewrite-debug --parallel 4 --target planar_runtime_tests
ctest --test-dir build/rewrite-debug --output-on-failure \
  --output-junit <absolute-evidence>/quantity.xml -R 'command embedded binding native budget'
ctest --test-dir build/rewrite-debug --output-on-failure \
  --output-junit <absolute-evidence>/binding.xml -R 'command execution (binds|leaves|refuses)'
python3 tests/native/r19-authoring-support/lifecycle.py \
  --authoring build/rewrite-debug/src/cmd/planar/planar_authoring_stack \
  --production build/rewrite-debug/bin/planar --case all
```

The lifecycle binaries must first be rebuilt for the implementation candidate;
the first test-only build above does not establish that. Register actual proposed
Catch names with the stated prefix, require a nonzero inventory/zero skips and
retain raw red/green JUnit. Then run the complete host Debug/Release gates,
formatting/Doxygen and strict idle repeats after code is frozen. No new Linux or
remaining full R19 claim is inferred. Plan review must approve the private
composition and accounting recipe before code; unresolved product choices stop
dependent work, but none is necessary for this contract repair.
