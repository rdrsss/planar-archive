# R15–R18 C05 independent source review

Disposition: **approve the scoped historical catalog and profile API correction source**. No new blocking source finding in the reviewed scope. This is not integrated round approval: full host/Linux gates, calibration verification and final probes remain pending. No source/test corrections, builds or commits were performed by this reviewer.

## Candidate and reviewed boundaries

Exact source hashes are in `tests/native/r15-r18-c05-evidence/review/source-sha256.json`. Reviewed the complete historical catalog implementation/interface, historical tests, profile preparation correction and its surrounding C04 preparation/commit path, the exact acceptance header pin, and focused prior C04 publication and event-operation integrity code. Surrounding unrelated C01–C03 runtime/ledger changes and test calibration are not implicitly approved.

Authority read: repository instructions, implementation/roadmap/design openings, C05 round and historical plan/handoff, external-profile-contract historical/effective provenance and reader clauses, C04 API finding, and execution R17-7 applicability/publication definitions.

Historical catalog admission retains separate policy selection: `validate_catalog` requires current seventeen-command registration; only `decode_catalog` selects a supported historical epoch. Epoch discovery requires an exact shipped-name set, not just a count. Complete cuts 5/10/14/16/17 select exact shipped capability/family/handler/response/options signatures; arbitrary subsets and mixed history/preview option epochs refuse. External collisions use the selected epoch while current admission reserves all current names. Owner/module/ordinal correspondence, sorted uniqueness, exact schema, byte/node/count bounds and owned decoding remain active for both paths. The external history capability keeps its unchanged ABI. Public API signatures remain unchanged.

C04-R1 is closed at source: original one-argument `prepare_load(intent)` is restored, the generic second-value compile-negative assertions remain, and the exact whole-class source pin adds only the new `prepare_execution_load(intent, snapshot)` declaration. The private shared helper consumes the owner before null-input refusal. Non-null snapshots are bounded/validated then deeply copied before the retained reconciliation phase. Preparation still binds the original request, source receipts and captured registry/profile; the new method does not accept caller bundles, records or results. Invocation chooses the method using native ledger format. Commit compares the frozen snapshot against the native snapshot, derives native revision records, stages before verifying original receipts, and keeps exact late replay ahead of receipt/CAS side effects. No second profile factory, dormant-VM reentry or Lua execution under the lock was introduced.

Focused residual C04 review: publication verification resolves each own facet through its completion dispatch's retained interpretation, binds producer ownership and exact paths, and recomputes the semantic from retained bytes/modes. Current and historical rows both pass this verification. Event payload validation now pairs all thirteen carrying event names with the required operation, rather than accepting an unrelated allowed operation.

## Independent evidence

- Ran `build/rewrite-release/src/engine/planar_history_effective_catalog_tests`: **13 cases, 1747 assertions, zero failures/skips**; `review/catalog-tests.log`.
- Independently compared all twenty listed immutable historical fixture files against HEAD and the historical handoff hashes: **20/20 identical**; `review/immutable-fixtures.json`.
- Ran `python3 tests/native/r15-r18-c05-evidence/review/producer-facet-probe.py`: isolated five-unit split arithmetic workflow, ten vector assertions, five accepted units, then producer-only facet definition change and consumer preview; exit zero. Script/log retained. The initial exploratory preview command included unsupported `--job`, returned usage, and was corrected to the actual preview signature before the recorded final run.
- Inspected coder evidence `profile-api/preparation.log`: **3 cases, 57 assertions**, and `profile-api/profile-cli.log`: **5 scenarios, zero failures/skips**, including late equal replay, conflict, changed receipt and CAS loss. These are coder executions, not independent reruns.
- Inspected corrected `profile-api/surface-2.log`: **1 case, 53 assertions** passes in `planar_ledger_format_tests`. Earlier `surface.log` was a wrong-binary zero-match attempt, retained as failed evidence and not counted. Runtime compile-negative and integrated gate completion remain coordinator responsibilities.

## Producer-only facet change observation

The isolated probe changes `s_files` from `s/value` to `s/new` after successful checks and approval, without changing producer work/checks/output. Loading revision 2 leaves `s_worker` done, makes `b_worker` stale, and blocks the reviewer. Consumer preview still lists the original `s_files` publication and the start prerequisite as available.

This is reproducible but not a demonstrated violation of the approved exact contract. R17-7 explicitly defines current publication as the facet in the producer role's current completion; completion applicability compares W, observed own outputs and producer-vector outputs. Dispatch W includes consumed facet declarations, not producer-only facet declarations. Thus the consumer W changes while the producer's does not, and publication integrity correctly uses the original dispatch revision. Any policy requiring the latest facet definition to invalidate the producer/publication needs a scoped contract decision; adding a new fingerprint field during this correction would exceed the reviewed recipe. This observation is retained for product consideration, not used as a blocker or hidden by weakening historical verification.

## Limits

Source approval is limited to the pinned files and paths described above. Historical focused handoff reported 67 cases/73658 assertions; only the catalog suite was independently rerun here. No full Debug/Release/Linux gate, new mutation campaign, resource calibration, or R19 real-project acceptance is claimed. Re-review changed source hashes and evaluate final gate/probe evidence before integrated approval.
