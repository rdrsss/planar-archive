# C04 decisions

Existing contract authority and architecture from the independent map:


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


Plan-review clarification: exact late replay under stage_execution precedes
receipt checks and CAS for a structurally valid frozen candidate. Existing native
pre-lock structural validation remains unchanged; no moved-from candidate
acceptance or new transaction algorithm is required. If staging returns replay, return
the saved bytes and do not verify prepared receipts or perform completion side
effects. A different body conflicts before CAS. Load has no internal retry;
EX-3's bounded retries belong to checks, not a consumed profile preparation.
