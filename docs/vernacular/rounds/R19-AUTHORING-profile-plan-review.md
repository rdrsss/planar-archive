# R19-AUTHORING profile wiring — plan amendment review

Disposition: **approve the amended engineering plan with the corrected scope
below**. The proposed claim that every handler implementation can remain
unchanged is rejected. This is plan approval only; implementation, tests, gates,
probes and the commit candidate still require their independent checks.

Fresh ordinary reviewer, substituting for the unavailable prescribed Claude
model. Read the repository instructions, implementation/protocol and roadmap
openings, v5 scope, R19-AUTHORING brief/map/initial plan review and canonical
decision/invariant tables. Inspected bootstrap's ordinary and with-profile
branches, invocation's selected-profile path and protected `rt_api` load owner,
the twelve execution interfaces, direct-intent handler implementations, runtime
option declarations and authoring source transformation. Reviewed the retained
focused profile-parity failure. Only this report was written; no source, build,
CLI, ledger or operator state was changed by this reviewer.

## Authority and finding

The focused candidate's authoring with-profile init/load succeeds, then its
fresh with-profile activation fails `interpretation_mismatch`. The blanket
qualification rewrite selects the authoring runtime for execution while load
deliberately uses the protected original profile owner. This is a candidate
composition regression, not authority to relax EX-6 or to admit authoring under
profiles.

The command contract's same-VM rule (`command-contract.md:175–189`) continues to
govern ordinary embedded dispatch. The external-profile contract's X-CLI
retained parser state/admission rules, authoring RA-9, execution D134 and EX-6
require the closed profile owner, exact effective bytes and early replay to
remain coherent. Routing authoring's with-profile calls through the existing
original closed profile handlers restores that behavior. It does not route the
ordinary authoring workflow through production or expose authoring in profiles.
No new product or authority choice is necessary.

## Approved corrected engineering scope

1. Add a runtime-free module below the handlers, such as
   `src/cmd/planar/execution_options.cppm`, owning invocation-local CLI option
   storage. Preserve every field, optional value, default and lifetime. Its
   dependencies should be limited to standard value types; it owns no parser,
   normalization, runtime, services or dispatch policy. Existing handler
   interfaces retain their public family option names as exported aliases.
   Shared run/recovery storage may back multiple family aliases; twelve families
   need not imply twelve distinct storage shapes.
2. Include all twelve execution handler interfaces in ownership, plus **six
   implementation files**: claim, takeover, refresh, checkpoint, release and
   recover. Today these interfaces alias runtime intent types and their bodies
   pass `o` directly to `execute`/`execute_with_profile`. A runtime-free shared
   CLI storage type cannot implicitly become each generated runtime's distinct
   intent. Add explicit typed field construction in those handlers, shared by
   their embedded/profile entry points. Preserve all run fields and all recovery
   fields, including `operation`, `run_id`, `reason`, `expect_fp`, observations
   and provenance where applicable. This is a mechanical boundary adapter, not a
   second semantic implementation. The other six handler bodies already own
   typed intent construction and need no semantic change.
3. Authoring's generated bootstrap retains composition-local embedded imports,
   option binding and twelve `&*vm` calls. It additionally imports and calls the
   original twelve `*_with_profile` functions using the same parsed shared
   storage. Protect these qualifications during the ordered transformation;
   reverse restoration must recover exact source bytes. Link the shared option
   module to every relevant handler and original profile modules explicitly to
   authoring bootstrap. Keep dependencies downward, test copies excluded from
   default `all`, and emit generated sources only when bytes change.
4. Preserve the four other test generators' intended local profile behavior;
   they still copy exact handler source and use local embedded runtime types.
   No cast, layout reinterpretation, field-dropping conversion, digest exception,
   runtime bundle switch, shadow parser, authoring macro site or public mode is
   included. Engine/runtime ownership is limited to the additional identity
   repair below; it is not needed for shared CLI storage itself.

This data-only option module plus six explicit intent adapters is a bounded,
typed repair. It avoids changing the profile factory's accepted bundle identity
or adding special cases to digest validation.

## Additional embedded identity repair

The coordinator supplied a second focused finding during this review: even
after correct typed VM forwarding, ordinary authoring activation still refuses.
`invocation.cpp`'s `embedded_stdlib_digest()` calls
`history::prepare_embedded_stdlib()`, whose separate compiled module always uses
the original runtime source. `native_services::execution_binding()` falls back
to that original digest. Thus correct selected Lua alone is insufficient: the
native EX-6 comparison is still given the wrong bundle identity.

**Approve extending ownership to `src/engine/runtime/runtime.cpp`,
`runtime.cppm` and `src/cmd/planar/invocation.cpp` for this identity binding only.**
Derive the embedded stdlib digest from the invoked VM's retained immutable
`command_state.originals`, `separator` and `assembly`, using the same
`history::prepare_stdlib` provenance recipe and
`history::digest_tree(..., tree_profile::provenance)` used by accepted revisions.
Bind this verified identity to the native services before a fresh bound
execution reaches its Lua/native service calls. The existing profile owner
already binds its verified effective digest in `run_known`; a neutral
`bind_execution_stdlib` name may replace the profile-specific binder at its
declaration, default implementation, profile call and native override. Preserve
the exact profile value and owner verification sequence.

The following boundaries are mandatory:

- Do not use `command_vm::effective_digest()` as an embedded identity: its
  documented current value is empty for embedded VMs. The registration
  `command_bundle_v1` seal is also a different recipe, not the ledger stdlib
  provenance digest. Do not manufacture equality or substitute production bytes.
- Keep format/early-replay/body-conflict preludes before this fallible derivation
  and invocation, so a saved result still replays through a different bundle
  without reinterpreting it. The current service comparisons occur after those
  preludes; bind there rather than moving interpretation validation ahead of
  replay. Preserve consumed-owner/failed-seal validation and terminal release.
- Derivation belongs to the needed execution alternatives, not unrelated
  render/init/load/history/export paths. Preserve exact exemptions: bare status
  (including request lookup), history/export, authoring orchestrator ledger reads
  and `check-recover` remain available without EX-6 interpretation binding.
  In particular, discriminate `status.role` and `check.operation` instead of
  treating every instance of those two option alternatives as bound.
- Charge owned provenance copies and hash scratch through existing cumulative
  native accounting before allocation, following the load provenance boundary;
  preserve protected error vocabulary and resource failures. Do not put new
  global mutable state or an unbounded cache in the identity path. Returned
  digest views must not outlive their owner or temporary preparation buffers.
- Remove the production-digest fallback from native execution services. A bound
  service without the required VM/profile identity must fail closed with the
  existing internal failure, never silently switch interpretations. No EX-6
  comparison or no-write rule is removed.

This extension restores the exact accepted/current interpretation contract. It
changes neither digest format nor product policy and needs no operator choice.

## Verification requirements before final review

- Keep the frozen lifecycle assertions unchanged, including actual fresh
  authoring/production profile activation from cloned equal-identity state,
  representative profile reads/claim/check, changed-profile no-write refusal and
  authoring-tail refusal. Preserve the failing candidate's raw parity attempt;
  passing baseline controls must not be relabeled as new red regressions.
- Exercise the converted run/recovery option shapes through existing real CLI
  tests and the authoring lifecycle/recovery scenarios. Retain existing profile
  lifecycle, saved-response, error precedence and no-write controls. Source
  review must compare every shared-storage field and adapter assignment with
  the prior runtime aliases, not infer completeness from compilation alone.
- Correct the new structural wiring guard's scope: its blanket prohibition on
  original imports/qualifications is overbroad for authoring's intentional
  profile path. Require all twelve local embedded routes and retained-VM
  arguments, and all twelve original authoring profile routes separately. Do
  not permit original embedded fallback. Preserve the exact HEAD source red
  and family-specific omitted-VM kills, rerunning the guard after amendment.
- Rerun reversible generator audits, representative typed composition builds,
  focused nonzero/zero-skip tests, then the actual host Debug/Release complete
  gates with retained JUnit, formatting/Doxygen and no-target-work repeat.
  Document the new module and aliases, update canonical decisions/invariants
  and the real task's declared file boundaries before checking its output.
  Final source/evidence review and current-candidate identity are still owed.
- The actual authoring lifecycle must now pass both selected-callable and
  selected-digest boundaries. Preserve fresh production-vs-authoring mismatch
  negatives and exact cross-bundle replay; add or reuse focused tests for binder
  timing, fail-closed missing identity and exempt-read behavior where observable.
  Review cumulative budget accounting and unchanged native/profile diagnostic
  precedence explicitly. Runtime scope invokes the protocol's more capable
  coder rule; the unavailable prescribed model limitation remains disclosed.

The coordinator may dispatch this corrected scope without another operator
permission request. An implementation that instead changes profile admission,
effective digest policy or authoring exposure is outside this approval.
