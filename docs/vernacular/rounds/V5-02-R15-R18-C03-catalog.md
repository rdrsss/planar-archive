# C03 profile catalog integration correction

Status: implementation and focused validation in progress; not approval.
Shared checkout changes retained. Own native effective-catalog validator and
catalog/runtime/authoring/bootstrap regression fixtures only.

The native profile validator admitted fourteen shipped commands while the Lua
bundle registered seventeen. The independent valid seventeen-row fixture failed
its first assertion before the fix (`catalog/red.log`); an empty-profile CLI
also failed before correction (gate fixture map). The validator now admits exact
check, check-recover and verdict descriptors, their closed capabilities and
option tables, including the exceptional underscore check-recover handler and
check/verdict response kinds. Check-recover has six required options. Shipped
history gains the decimal event selector.

External `history_read_v1` signatures remain unchanged: external-profile-contract
line61 explicitly illustrates the amended *embedded* history probe with
`with-profile P history --event ID`, while its capability row422 and history_id
row270 still enumerate revision/at-event/after-event. The known shipped history
signature therefore gains event independently of the extension capability ABI.

The catalog fixture is independently assembled and exercises every shipped row's
handler, response, capability and option metadata. Missing/extra capability and
option controls are added. Existing catalog limits stay64 entries and1MiB;
positive exact-boundary and plus-one checks remain. Runtime test offsets use17
shipped rows, authoring uses23, and profile authoring exclusion remains explicit.
The coordinator owns bootstrap.cpp root-help corrections; this lane maintains
independent expected bytes.

Initial post-fix pure catalog suite: 11 cases, 899 assertions pass. More controls
were added afterward and await the complete focused build. Empty-profile Release
CLI selected help now exits0 (`catalog/green-profile-help.log`). A separate
workflow verifier subsequently found the pre-existing format3 profile-load
refusal; that lies outside this lane and is assigned by the coordinator.

Raw evidence: `tests/native/r15-r18-c03-evidence/catalog/`. No commit or
self-approval. Full gates and independent review remain coordinator-owned.

## Final lane handoff

The coordinator extended ownership to the orchestrator parser and authoring
fixtures. A genuine CLI11 defect caused `orchestrator --workspace ... --registry
... --job ...` to refuse: `require_option(1)` means exactly one option. It is now
`require_option(1,3)`; the existing mutual needs and exclusions enforce either the
complete three-option declaration form or one ledger option. Added positive
ledger/triple and negative empty/partial/mixed regression controls. Existing
failures are preserved in `command-tests.log`.

The coordinator changed the CMake synthetic catalog variant from already-shipped
status to `catalogprobe`. The independent variant tests now prove the sealed
catalog adds this arbitrary24th action beyond the23 real commands, retaining
anti-hardcoding, sorted names and provider-decoy refusal controls. Graph-only
orchestrator fixtures now expect23 available/zero future actions. Path-stage
fixtures supply the newly required job so their intended filesystem refusals
remain observable. Direct source pins account for both declared/live consumers
and the conditional state section. Closed diagnostic scanning retains its
external-registration golden table, adds the four exact native execution
messages separately, and excludes two explicitly identified non-diagnostic
string arrays by their surrounding source syntax.

Executed Release evidence:

- `planar_history_effective_catalog_tests`: **11 cases,1041 assertions pass**.
- `planar_authoring_catalog_tests`: **24 cases,333 assertions pass**, including
  the generic catalogprobe and updated orchestrator descriptor.
- First complete command run:212 cases,40 failures; parser and stale fixtures
  addressed. Second:213 cases,203 pass/10 fail,10670 assertions/48 failures.
  Some sources changed after Ninja's initial dependency scan, so old action
  assertions still ran. Final path-stage, diagnostic scanner and source-pin
  fixture edits have **not yet been rebuilt/retested**; no passing command-suite
  claim is made.
- First complete runtime run:484 cases,463 pass/21 fail. Second:484 cases,
  465 pass/19 fail,83249 assertions/20 failures. Near1MiB catalog control now
  passes with its original bounds unchanged. The near-miss `status-extra` fixture
  was invalid because its generated handler contains a hyphen; final source uses
  `statusx`, pending rebuild. Other residual failures below are deferred to the
  coordinator, not suppressed.
- Pinned formatter over owned C++ files and `git diff --check` passed at handoff.

Deferred runtime failures (`runtime-tests-2.log`): load-protocol generated VMs fail
creation before their intended assertions (`commands.t.cpp`784,806,829,840,855,
870,887,918,932,942,967,978); stale production counts in `runtime.t.cpp`4333/4358;
quarter-free native budget assertion at3849 measured31,009,820 against29,884,416;
source-size output-off pin145332 vs227275; C03 padded observation configuration
cannot prepare at0; N30 projected baseline is-1; reconcile fixture457 expects
three transitions but sees one. No ceiling, padding positive control, pin window
or failure assertion was relaxed. The coordinator reserved further correction.

All lane builds/tests have finished and Release lane2 is released. Final shared
rebuild/full gates, profile-format3 workflow work and independent review remain.
No code is committed or approved by this lane.
