# C04 profile acceptance and history corrective

Status: implemented with focused Release verification passing. This is coder
evidence, not independent approval or a full gate claim.

The approved C04 plan and independent profile map govern this change. The shared
checkout retains all earlier corrective changes; no installed Planar or operator
ledger is used.

## Implementation

`profile_invocation::prepare_load` accepts an optional bounded structural execution
snapshot. It validates and deep-copies that input using the original cumulative
owner, invokes the fixed retained trusted `reconcile_execution` export in the
existing preparation frame, and freezes the change and snapshot binding in the
opaque prepared product before cleanup. No dormant VM API or second factory was
added. Format2 callers omit the snapshot and retain their prior path.

The profile load entry selects format3 before capture, uses the already-framed
profile-bearing request_v2 for early lookup, and captures a verified native
snapshot with matching identity. Native completion uses the prepared source,
registry, effective-provenance tree and interpretation_v2 through the existing
execution staging transaction. Exact five-record no-op comparison is retained.
The staged replay disposition returns before checking changed receipts or
performing commit effects. Fresh commits verify the original prepared receipts
immediately before commit and preserve completion uncertainty. Load never
internally retries or reenters preparation.

Shipped profile history now maps both the `event` selector and its ID. The separate
external history capability ABI is unchanged. Initialization also restores the
existing syntax/schema refusal for empty, dot or parent final path components,
before filesystem traversal; unsafe parent paths retain their existing refusal.

## Verification assets and initial red evidence

`tests/native/r15-r18-c04-profile-cli.py` exercises a nonempty captured profile:
initial load, exact no-op, effective provenance bytes and export, expanded history
without tokens, material load drift, changed-profile binding/refreshed load, and
saved load replay/conflict after input deletion. With the existing X07 variant3,
it also pauses after preparation for equal late replay with changed receipts,
unequal request conflict, receipt refusal and CAS loss. It adds no production
fault hooks. The independent C03 grouped/split/profile lifecycle remains a separate
integration regression.

Initial executions against the pre-fix source-built Debug binary:

- `python3 tests/native/r15-r18-c03-vectors-cli.py build/rewrite-debug/bin/planar`
  exits1: grouped and split pass; profile load refuses unsupported ledger version.
  Raw: `profile/red-cli.log` under C04 evidence.
- `python3 tests/native/r15-r18-c04-profile-cli.py build/rewrite-debug/bin/planar`
  exits1: history event selector assertion fails because the response is a page.
  Raw: `profile/red-focused.log`.
- Existing `replacement.init_cli` supplies the initialization red evidence in the
  coordinator's C03 `debug-gate-1.log`: `.` returns path_denied instead of schema.

All subprocesses use explicit source-built binaries, temporary roots and bounded
timeouts. C04 focused, complete platform gates, mutation probes and fresh review
must be recorded before any completion claim.

## Focused verification outcome

Evidence lives under `tests/native/r15-r18-c04-evidence/profile/`.

- Release `planar` and `planar_x07_variant_3` build passed (`build-1.log`).
- New profile CLI: **5 scenarios, zero failures/skips** (`green-cli-3.log`).
  Both embedded and changed-profile binding refusals are checked before a fresh
  profile load; exact equal late replay skips changed receipt checks. Receipt and
  CAS negative cases leave no partial revision.
- Independent C03 vectors: **3 scenarios, zero failures/skips**, grouped, split and
  grouped-profile, each with 10 exact vector assertions and 5 accepted units;
  retained facet trees/bytes and check/verdict replay pass (`vectors-cli.log`).
- New native preparation checks: **2 cases, 34 assertions passed**
  (`native-tests.log`), including nested alias mutation after preparation, one
  registration/frame, cleanup, cumulative counters and unchanged default
  preparation shape. Initial native-test compilation exposed two fixture errors
  (missing POSIX declaration include and a string instead of optional expected
  head); both were corrected, with logs retained.
- Existing format2 profile acceptance suite: **29 cases, 3494 assertions passed**
  (`format2-tests.log`), rebuilt against the current runtime (`build-format2.log`).
- Pinned formatting on four owned C++ files and Python syntax checks passed.

The first new profile fixture placed its profile beside supplied inputs; once
load reached actual capture, that correctly refused overlap. The fixture was
corrected to a disjoint trusted directory, with the failed log retained
(`green-cli-1.log`). The independent C03 profile fixture received the same narrow
confinement correction with coordinator authorization; product confinement was
not weakened.

The initialization acceptance script now passes the previously failing syntax
cases and stops later at its stale assertion that new ledgers have format2
(`init-cli.log`, line113 of the test). That expectation is coordinator-owned;
this lane does not claim the whole initialization suite passed.

The Release build lane is released. Exact owned source/test and binary hashes
are in `candidate.json`. Full Debug/Release/Linux gates, mutation probes and fresh
independent review remain coordinator-owned. No commit, install or operator-data
access occurred.
