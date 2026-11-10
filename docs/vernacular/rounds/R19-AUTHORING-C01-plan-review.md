# R19-AUTHORING-C01 plan review

Disposition: **approve the bounded corrective plan**, with the execution
requirements below. This is not implementation, gate, probe or commit approval.
No new product or authority choice is required.

Fresh ordinary reviewer, substituting for the unavailable prescribed Claude
model. The shared checkout was read only except for this owned report. No
product/test source, build, candidate CLI, ledger or operator data was changed
or invoked by this review.

## Reviewed authority and finding

Read the repository instructions, implementation/orchestration protocol, roadmap
opening and v5 scope; the initial final review R1/A1; the C01 brief and source
map; and the canonical C01 decision/invariant tables. Inspected the actual
runtime charge/binder/profile code, provenance preparation, tree construction,
record encoding, existing finite execution providers and the historical private
remaining-budget probe.

The command contract's cumulative native allocation rule
(`command-contract.md:203–213`) and budget interface require checked charges
before the corresponding allocation/copy, with no refund and terminal
exhaustion. The new binder's one-payload reservation cannot cover its library
call. `prepare_stdlib` constructs canonical metadata, `own` calls `digest_tree`,
the tree digest constructs file-record payloads and independently encodes those
records, and `own` then copies the final file payloads. For original/assembly
payload total P and encoded assembly-manifest length M, that establishes a
minimum **3P + 4M**, before graph/path/directory/framing/object work.

The map correctly uses the existing profile model as a vocabulary for a derived
bound, not as proof by copying another caller. It also correctly preserves the
VM-owned provenance recipe and the existing profile effective identity. The
correction changes neither digest format nor interpretation admission policy.

## Accounting design requirements

The proposed binder-local repair is appropriate. The implementation must make
the following parts explicit in code/comments or its allocation trace:

1. Reserve three transfers of each original/assembly payload, four transfers of
   the exact canonical manifest length, one digest's record framing, and owned
   file/view/digest-text storage before `prepare_stdlib` is entered. Derive the
   fixed metadata expression from its actual canonical schema; the map's
   `125 + separator bytes + sum(9 + path bytes)` expression agrees with the
   current four-field manifest encoding.
2. Metadata encoding bytes do not themselves cover construction of the
   metadata value graph. In particular, its separator string has a separate
   owned graph copy; reserve that copy explicitly. The per-original graph bound
   must also cover each path string's initial graph ownership, shared-value/
   list/map/traversal structures and bounded capacities. State what a generous
   per-input allowance covers instead of naming it unexplained scratch.
3. Cover one tree digest's folded/name strings, component nodes/child maps,
   directory children/sort/encoded records and the final owned paths. Use the
   real synthetic names `__planar__/assembly.lua` and
   `__planar__/assembly.canonical`, not historical illustrative profile names.
   A per-component/path reservation is valid when its count/length limits and
   allocation coverage are documented. Allocator headers/capacity growth must
   not be described as a measured RSS guarantee.
4. Check count, path, separator, assembly and cumulative provenance limits
   before arithmetic that relies on them. Check multiplication/addition before
   allocation; do not allow a multiplication to wrap before calling `charge`.
   Cumulative/native-capacity refusal must retain the native-byte failure
   category, maximum and terminal failure behavior of `charge`. Keep existing
   library structural-bound refusals distinct; a numerical reservation guard
   must not silently disguise the intended native-budget failure as another
   resource category.
5. Preserve the current exemption checks and call placement. No reservation
   moves ahead of saved-response/body-conflict preludes, no exempt bare status
   or check-recover is charged, and a failed/consumed owner is rejected before
   preconsumption or binding. The provider receives the same exact digest after
   successful reservation; no production-source fallback or ceiling increase
   is admitted.

These requirements refine C01-D2/D3 and I1/I3 without changing their scope.
Source review and the final prober must trace the complete model; a runtime
lower-bound assertion alone cannot prove every metadata/scratch term.

## Private regression design

Approve the additional verification-only runtime composition with the declared
module/namespace renames and **one audited checked-preconsumption insertion**
immediately after `begin_invocation` and before the unchanged real binder call.
It is a private test binary input transformation, not a new product hook.
The production source, interface and existing fourteen semantic command
compositions keep their current exact-source guarantees.

The proposed bound-activation versus exempt-status comparison is meaningful:
both real shipped handlers first reach the same fake `read_execution` failure,
while only activation takes the real binder path. Registration and option
preparation happen before the insertion. The test must nevertheless audit the
remaining post-insertion side costs and use these safeguards:

- Encode every probed remainder with a **fixed-width decimal request ID** for
  both operations. Variable digit lengths can change later validation/transfer
  charges at powers of ten and undermine a strict binary-search monotonicity
  assumption. Fixed-width leading-zero IDs remain ordinary valid fixture IDs;
  the private decoder must consume the whole bounded numeric value.
- Use a fresh VM for every trial. Prove both zero-room resource refusal and
  the generous successful upper bracket before bisection; never skip an
  unbracketed case. Require normal completion to mean the exact expected native
  `ledger_busy` envelope and provider order, not merely a non-resource result.
- Derive P independently from actual original/assembly bytes or accepted
  provenance. Do not copy the implementation's reservation expression into
  the assertion. Record the two measured boundaries and the observed gap.
  The gap assertion must demonstrably fail with the current one-pass code.
- At 2P remaining room, require `resource_limit/native_bytes`, the expected
  unchanged maximum, no binder/provider call and a consumed second invocation.
  With generous room require the exact accepted digest and expected callback.
  Preserve a separate source/order check: no provider callback alone does not
  prove that provenance allocation had not already been entered.
- The mutation phase must actually kill both one-pass and two-pass quantity
  regressions through runtime assertions. If a legitimate metadata allowance
  masks the latter on the chosen corpus, return that observation for a scoped
  test adjustment; do not under-reserve real metadata to make the mutant die.

The generator must require the insertion anchor exactly once, record each
ordered substitution, reverse to byte-identical original source and write only
changed bytes. Missing/inert preconsumption must fail the bracket assertions.
The target remains excluded from default `all` and is linked through the normal
test aggregate. Its intentional test insertion must not be counted as another
unmodified semantic runtime copy or shipped to product targets.

Actual compiling assertion-red is still required before product implementation.
No setup error, source-only check, compile failure or zero matched tests closes
that requirement. Freeze the generator and assertions after the accepted red
run. Unexpected fixture behavior returns to the coordinator; an implementation
coder may not edit the test to make the product green.

## Scope, ownership and final verification

The proposed exclusive packages are approved:

- Test-first coder: new `tests/native/r19-authoring-c01-support/generate.py`,
  its wiring in `src/engine/runtime/CMakeLists.txt`, and colocated quantity/
  insufficient-room cases in `src/engine/runtime/commands.t.cpp`.
- Product coder: the new binder's accounting block/private helper in
  `src/engine/runtime/runtime.cpp`; no test edits. Necessary build dependency
  corrections require an explicit ownership transfer.
- Coordinator: actual repair declaration/claim, status/reference prose,
  identities, evidence and A1 record formatting. Fresh prober/reviewer keep
  scratch/report ownership and do not mutate the candidate checkout.

Older `accept_load` and profile-load phase-4 reservations are explicitly known
related follow-up work. This approval neither proves nor closes them. A shared
three-pass helper must not be applied blindly to the larger profile chain; a
broader repair needs its own complete reviewed derivation and tests.

Keep the parent 1,849-case gates and 31 probe groups attached to the rejected
candidate. Rerun affected identity/exemption/owner/quantity observations and
the complete host Debug/Release gates for the corrected exact candidate, with
retained nonzero inventories/JUnit, zero unexplained skips, pinned format/
Doxygen and strict idle repeats. Carry old observations only with the precise
unchanged footprint required by the protocol. Do not relax production/private
calibration limits or existing margin rules. Linux and remaining R19 matrix
obligations remain outside this bounded correction.

All eight C01 invariant rows have an observation route, including honest
source-only order/overflow coverage. The canonical tables currently render as
continuous tables; parent A1 is a records-only correction with original bytes
preserved. Final source/evidence review, actual approve verdict and the current
candidate check remain required before a signed-off local commit. Installation,
pushing, publishing and merging remain unauthorized.
