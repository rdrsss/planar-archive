# C03 profile execution cutover map

Read-only source investigation; no product/test edits, builds, installed binaries
or operator ledgers. The only written repository file is this report. Current
shared changes remain intact. Source line numbers are this investigation's
checkpoint and may move during correction.

## Findings and authority

1. `execute_with_profile` (`src/cmd/planar/invocation.cpp:2355`) frames the correct
   profile-bearing `request_v2` and routes load to `execute_profile_load`.
   That function explicitly refuses format3 at 1861–1865, before replay, and later
   accepts only format2. This is an unfinished cutover, not an intended product
   restriction: execution-contract.md:497 and 1275 require format3 load to keep
   request_v2, profile selection and the effective-stdlib path. Lines728–730
   require execution under the accepting profile and support profile changes by
   fresh reconciled load. The independent vectors CLI already reproduces init
   succeeding followed by profile load refusing `unsupported_ledger_version`.
2. Live independent reproduction: a physically resolved temporary empty profile,
   source-built `build/rewrite-release/bin/planar`, and a new temporary ledger:
   `with-profile --stdlib-profile PROFILE init --ledger LEDGER --workspace-id x`
   exits0; `with-profile --stdlib-profile PROFILE history --ledger LEDGER --event 1`
   also exits0 but returns `data.selector:"page"` with an events array. The
   contract requires selector `event` and expanded event data. The root cause is
   `native_services::read_history` at1450: it recognizes revision/request/state
   only, and its id expression omits `options.event`. This affects shipped
   profile history, not the separate external `history_read_v1` capability ABI.

No new user-facing product choice is required. Existing contracts already require
both behaviors. Internal ownership/API additions require scoped architecture and
code review because the profile preparation owner is intentionally sealed.

## Existing paths and reuse boundary

| Symbol | Current responsibility / implication |
| --- | --- |
| `execute_profile_load`, invocation.cpp1840 onward | Early request lookup, one `open_with_profile(intent, ledger)`, prepare, format2 acceptance, receipt/fallback response handling. Preserve its fault anchors and format2 behavior. |
| `format3_load_prelude`, invocation.cpp1969 | Embedded-only format3 selection, identity/workspace capture, request_v2 framing, execution request lookup. Its current create_v2 omits profile; do not call it unchanged for profile requests. Reuse/refactor the framing-independent verified lookup logic while retaining the exact already-framed profile intent. |
| `native_services::read_execution(command_load_options)` and `accept_load`, invocation.cpp860–952 | Embedded format3 snapshot, prepared records, exact no-op detection, execution-change decoding, `stage_execution`, precommit source receipt verification and completion receipt. This is the commit model to share; its `source_capture::prepare` and embedded `provenance` profile are not valid substitutes for prepared effective-profile artifacts. |
| `profile_invocation::open_known_load`, runtime.cpp6541 | Captures profile once; admits workspace/declaration/registry and identity-only load ledger; retains original receipts; registers exactly one VM; binds exact immutable intent. Reuse unchanged ownership semantics. |
| `profile_invocation::prepare_load`, runtime.cpp6865–7206 | Consumes owner, opens source through retained receipts, begins one frame, routes/captures/validates, closes nested frame, derives exact effective tree and interpretation_v2, publishes opaque prepared_profile. |
| `profile_load_stage::implementation`, runtime.cpp6755 | Retains owner, sealed source, normalized bundle and validated semantic model. These are available during preparation to build the candidate for trusted reconciliation. |
| `prepared_profile`, runtime.cppm821 onward | Published product exposes immutable data and native receipt checks only. Dormant VM is explicitly never callable. **Do not add a method that revives it or invoke a second VM via run_trusted.** |
| `profile_acceptance_data`, runtime.cppm809 | Already contains source, registry, effective tree, interpretation_v2, workspace and bound intent. Sufficient native record inputs for execution_prepared_revision. Add only the closed frozen reconcile result/input-binding needed by the format3 path. |
| `ledger/profile_acceptance.cpp::complete_acceptance` | Adapter into old `complete_native_candidate` seam. Do not delete the format3 refusal and feed format3 into this format2 writer. Preserve this path for format2. |
| `execution.cpp::stage_execution` | Existing atomic format3 writer with identity/CAS, all-role changes, record interning, exact validation and request replay. Correct destination for profile format3 acceptance after native reconstruction. |

## Recommended bounded architecture

1. Select ledger format before profile capture, retain verified identity and
   workspace, and use the one profile-bearing intent framed by execute_with_profile
   for both early execution lookup and every later stage. Exact replay and conflict
   must resolve before touching an absent or changed profile/declaration/registry.
   Keep format2 on its existing path and format1 refused as before.
2. Capture a verified execution snapshot (identity checked against the prelude)
   outside any write transaction. Pass its structural snapshot value into a
   format3-specific preparation overload/input. Native ledger snapshot types need
   not become a runtime dependency: the existing execution snapshot converter
   already provides bounded structural values.
3. In the existing owner and **before `cleanup.run()`** at runtime.cpp7023, execute
   the fixed trusted `reconcile_execution` export on `{operation="load",snapshot,
   candidate={bundle,semantic}}`, using the validated bundle/model already in that
   frame. Retain this fixed export during registration, analogous to the retained
   shape-phase export references; `call_retained_value` is the existing bounded
   invocation helper. Validate its closed envelope, preserve first-error and
   cumulative budget rules, copy the result into immutable native values, then
   close the frame normally. Existing no-snapshot format2 preparation remains
   unchanged. Do not expose arbitrary export names, callable dormant owners,
   recapture or callback authority.
4. Continue native effective-tree/interpretation_v2 derivation exactly as today.
   Freeze the reconciliation result and its snapshot binding in the prepared
   format3 product. The future writer must not accept caller-substituted prepared
   data as authority. A narrowly typed overload can preserve the existing opaque
   `prepared_profile` ownership and one-shot protocol.
5. Construct execution_prepared_revision with source profile `source`, registry
   `provenance`, and stdlib **`effective_provenance`**, plus the derived v2
   interpretation and versions. Reuse exact five-record no-op comparison from
   embedded acceptance. Decode the frozen reconciliation using the captured
   snapshot and the exact profile-bearing request_v2. Stage via `stage_execution`,
   verify the original prepared profile's native receipts immediately before
   commit, abort on refusal, preserve uncertainty and replay receipts. No Lua runs
   under the write lock. Share the embedded commit helper where practical rather
   than copying a second transaction protocol.
6. For shipped profile history, map selector `event` to history_selector::event
   and select options.event as q.id (see the existing standalone history handler
   for exact successful/error behavior). Keep the external capability selector
   signature separate; external-profile-contract.md61 versus its capability table
   distinguish these contracts.

The frozen reconciliation must correspond to the snapshot actually used for
stage_execution. Normal CAS failure should refuse or follow the contract's
existing retry policy; it must not silently recapture or reenter the consumed VM.
No new schema or legacy database migration is needed.

## Ownership and regression matrix

Suggested product ownership: `src/cmd/planar/invocation.cpp` and its focused
format3/profile CLI tests; `src/engine/runtime/runtime.cpp/.cppm` for bounded
preparation input/result and retained trusted export; a helper below invocation
if needed for native acceptance composition. Avoid ledger acceptance-module to
runtime dependency cycles; preserve the existing downward graph. Catalog coder
already owns effective_catalog and runtime/external_registration/bootstrap test
fixtures: coordinate before touching those tests. This map does not authorize
editing another agent's owned files.

| Invariant | Required regression / faithful fault |
| --- | --- |
| Empty profile creates/loads format3 and binds execution | Fresh profile init/load/activate/claim/check/verdict; both grouped and split fixture behavior; reject embedded or changed-profile mutation until fresh load. Existing 3-scenario vectors CLI supplies a strong lifecycle. |
| Saved profile load replay precedes capture | Complete load, remove/alter profile and declaration inputs, repeat exact request: saved bytes; changed parameters same ID: conflict. Fault: move lookup after profile capture. |
| Effective provenance survives | Inspect stored family/effective tree/profile/module bytes and export accepted source; nonempty extension profile works, not just empty profile. Fault: use embedded provenance profile. |
| Profile load reconciles active roles | Active claimed worker, material profile/declaration load, then drift/refresh behavior; exact no-op and immaterial load preserve appropriate state. Fault: omit reconcile or use unrelated snapshot. |
| Identity and CAS remain atomic | Competing acceptance and replaced ledger refuse without partial revision/transitions/request; receipt change before commit aborts. Fault: skip identity, expected revision or original native receipt check. |
| Single owner/frame and bounds | Registration remains once; native/instruction counters retain cumulative values; cleanup succeeds/refuses as existing tests require; no call after preparation. Fault: second VM/factory or reopened frame. |
| History event is actual expansion | Profile init then history --event1 has selector event; after claim, expansion has retained work/manifest/packet and no token/secret. Fault: omit event selector or ID mapping. |
| Format2 compatibility is retained where explicitly contracted | Existing format2 profile acceptance/replay/receipts pass; format1 remains refused. This is supported new-engine ledger formats, not legacy Planar migration. |

The live history reproduction above establishes a second concrete bug. No native
build, race injection, full gate, or product approval is claimed by this map.
