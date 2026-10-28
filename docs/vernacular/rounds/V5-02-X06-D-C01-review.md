# V5-02-X06-D-C01 independent review (corrective for the X06-D review)

Disposition: **request-changes** (not records-only).

The corrective round has done most of what the X06-D review asked. One factory budget (`factory_budget`, a
`budget_sink`) is now created by both profile factories. It is threaded into profile capture, admission metadata,
native reads, discovery, ancestor and verification staging, the declaration loader VM, the retained declaration values
and the source-capture bundle copy. The capture charge now comes before the read and the allocation. G1 ran first. The
metric-ownership inventory is in the record. The evidence index exists, and the `FATAL_ERROR` coupling is gone. The
candidate bytes are the ones recorded, and the gates hold. All 1056 earlier names are kept and identical in both
configurations. The closed message table, the public runtime surface and the X06-C seam are unchanged. The red checks
and the probe kills I spot-checked are genuine.

The round still cannot be approved, for three reasons. First, R1 is not closed: one later stage of the same factory,
extension role preparation, still builds an unbudgeted capture session. That session loads the workspace, spec and job
declarations and enumerates the trees. The inventory does not mention this stage (C01-R1). Second, the record itself
reports two open test gaps. The sink's own instruction ceiling is not observed by any test (C01-R2), and the B3 stage
tests T8 and T9 were never written (C01-R3). Third, the B4 semantic half (C01-R4) and the ceiling-headroom constraint
(C01-R5) are not met. Two further items concern documentation and records (C01-R6, C01-R7).

I reviewed in the shared checkout on `dev/planar-v2-vernacular` at HEAD `a0a8b3fb7b038cba98e77f1f80e0008353279cf3`,
against the uncommitted candidate. I wrote only this file and new files under
`tests/native/v5-02-x06-d-evidence/review-c01/`. I applied no mutants and edited no source, test, record or existing
evidence. With no other ninja or ctest process running, I ran two commands on `build/rewrite-debug`, one at a time:

- `cmake --build build/rewrite-debug --target planar_tests`: exit 0, `ninja: no work to do.`
- `ctest --test-dir build/rewrite-debug -R 'X06' --no-tests=error --output-on-failure`: exit 0, 149 of 149 passed,
  0 skipped, 67.45 s. All 42 `X06 D C01` names ran.

I did not commit, push, merge, install or publish. I used no old Planar tool, queue, skill, role or installed binary.

## Exact candidate

- `git diff HEAD -- src` sha256 is `3b45bac8e00f835a1ceb2249d3beab166100e7e13da423fd3bfcf379c4b03e4a`. `git diff HEAD`
  sha256 is `1424dc80e9e44bcf357f6c930e8fe60896179728602cffc1db9d1ec70ff01e9f`. Both equal the record, gate run 3 and
  probe run 2, and both were the same before and after my run (`review-c01/identity-before.txt`, `identity-after.txt`).
- The diffstat is 22 files, 2160 insertions and 63 deletions, as recorded. All 22 per-file sha256 values in the record
  equal the working tree.
- The evidence hash prefixes quoted in the record also match: `REPORT.md` `2c41656f...`, `PROBES.md` `672cf498...`,
  `RED.md` `dfb157e5...` and `inventory.txt` `8e6ac432...`.
- `runtime.cppm` is unmodified. The ledger product sources and `src/cmd/` are unmodified; only
  `ledger/acceptance.t.cpp` changed.

## Independent verification

All evidence paths below are under `tests/native/v5-02-x06-d-c01-evidence/` unless they say otherwise.

| Check | Result |
| --- | --- |
| Gate exits | Every `raw/r3-*.exit` file reads 0. |
| G1 order | By file modification time, `r3-g1-focused.log` (03:37:36) comes first. Then come `r3-g2-build.log` (03:37:42, `ninja: no work to do.`), `r3-g1b` (03:38:48) and the Debug full run `r3-g3` (03:45:10), the release configure and build (`r3-g4`, `r3-g5`, 552 steps), the RelWithDebInfo full run `r3-g6` (03:48:48), and finally lint, the no-work build and the redaction check. G1 passed 149 of 149 with no skip text. Because the build that followed did no work, G1 ran on the final binaries. |
| Full runs | Debug passed 1098 of 1098 in 381.60 s. RelWithDebInfo passed 1098 of 1098 in 154.33 s. The only "skip" text in either log is inside the names of tests #610 and #1070, both of which passed. |
| Names | I compared the sets myself against the X06-D inventory (`../v5-02-x06-d-evidence/inventory.txt`, 1056 names). None were removed (`review-c01/removed-vs-x06d.txt` is empty). 42 were added, all `X06 D C01` (`review-c01/added-vs-x06d.txt`). The Debug and Release name sets are byte-identical and equal `inventory.txt`. |
| #531 timing | The test is now #556, `X06 B3 C a file replaced at any allocation inside prepare_load is never published as verified`. It took 15.29 s on Debug, against 15.47 s in the X06-D gate, and 4.51 s on RelWithDebInfo. It is not slower. |
| Lint and no-work build | `r3-g8-lint.log` ends "Formatting and Doxygen checked 142 active source files". `r3-g9` and `r3-g10` both say `ninja: no work to do.`, and so did my own build. |
| Red-first | `red/snapshot-2/candidate.diff` hashes to `dbec1665...`, as RED.md says. I read the A1, N1, T1 and S2 logs. Each is an assertion failure, not a compile error: A1 `0 > 0` at `admission.t.cpp:932`, N1 `"schema" == "resource_limit"` at `native.t.cpp:924`, T1 `0 >= 99000` at `runtime.t.cpp:3608`, and S2 `0 == 16000` at `source_capture.t.cpp:1051`. |
| Probes | The kills I read in the raw diffs and failing-test lists match the record. N1 fails both N1 tests, and N1b and N1c fail only `refused capture allocates nothing near the file size`. I10 fails only `runtime makes caught quota terminal before protected calls return`. I6 fails the two X06 B3 B tests, the C01 caught-error test and both T3 tests. I3a fails the shape-dispatch test. V1, S1, A1, L4, T1, N4, T5e, I3g, T14, N2e and S2c each fail the test the record names. I also confirmed the survivors: T3, T3c, T3d and T16 each read "100% tests passed out of 1091". |
| Variant | The generated `build/rewrite-debug/src/engine/runtime_budget_test.{cpp,cppm}` differ from `runtime.{cpp,cppm}` only in the module and namespace lines. The ceilings come from compile definitions, and `src/engine/CMakeLists.txt` contains no `FATAL_ERROR` or `REGEX`. |
| Surface and seam | No new `resource_limit` literal, public code or message was added. The closed-message test and the seam guard pass in both full runs. B8 does not forbid the new internal engine interfaces: the exported `budget_sink` and the defaulted budget parameters on `load_declaration`, `capture_admission::create`, `capture_session`, `native_capture_session::set_budget`, `profile_capture::open` and `source_capture::open_*`. They are functional, not added for observability. |

## What is sound

- **Factory budget lifetime.** `profile_invocation::implementation` declares `budget` first, so the budget outlives the
  capture and the receipts. `attach_budget` folds the charges made before the VM existed. `detach_budget` runs in
  `~command_vm::implementation`, so a receipt that outlives the VM never reaches freed state.
- **Loader composition.** Every native charge reaches the sink after the inherited 64 MiB check, so the stricter limit
  wins. Each hook tick charges 100 instructions, and the inherited 1,000,000-instruction message wins on a tie. Live Lua
  memory is acquired before `realloc` and released on free, on shrink and on a failed `realloc`; `old` is zeroed when
  `p` is null, so Lua type tags are never counted. A null sink keeps the loader byte-identical (L8, L8b).
- **R2 is closed at the native layer.** `read_file` charges before `std::string data(...)`. The test that a refused
  capture allocates nothing near the file size uses a test-local `operator new` probe with an accepting control, and it
  kills both late-charge mutants. The bundle copy in `source_capture::capture` is also charged before it is allocated.
- **R3 and R5 are closed.** G1 ran first on the final binaries. Both evidence folders now have a README that lists the
  corrections to `REPORT.md`. The variant no longer rewrites ceiling text.
- **R4 is partly closed.** G1 to G5 have rows. The B6 handle, discovery and verification rows (A3, N3, N5, N6) are each
  killed by their named tests. I3 and I6 now name tests that kill those mutants, and I10 is killed.

### The confirmations the record asks for

- **Halt candidate (a): registration versus invocation instructions.** I confirm that no amendment is needed.
  Registration (10 million) and Invocation (100 million) are separate X-LIMIT rows. The reset at `begin_invocation` is
  existing behaviour, pinned by I7. Carrying the pre-frame declaration instructions into the invocation counter applies
  the X-LIMIT rule that stages do not reset budgets.
- **Halt candidate (b): declaration Lua memory is charged live, not cumulatively.** I confirm this too. The Invocation
  row bounds "512 MiB live Lua", so acquiring and releasing live bytes is the contract's own measure.
- **R2 ordering: `resource_limit` before `schema`.** I confirm this. X-LIMIT opens with "All bounds are checked before
  allocation/copy/action". UTF-8 validation needs the bytes, so it must come after the check. The inherited
  `retained_bytes` check already preceded the read, so an oversized invalid file already refused as `resource_limit`.
  No contract clause orders `schema` before resource bounds.

## Findings

### C01-R1 (required, source, tests and records): extension role preparation still creates an unbudgeted capture session

B1 says the factory budget is "the only source of cumulative charges for capture, registration, declarations, shapes,
conversion, preparation and result. No stage creates a second budget". The C01 objective restates this as "registration
and the later stages".

`profile_invocation::open_opaque` attaches the factory budget to the VM state (`state::budget`).
`into_extension` then moves that VM into an `extension_invocation`. When role history is enabled,
`extension_invocation::prepare_role` (`runtime.cpp:5954`) runs inside that invocation, and at `runtime.cpp:5964` it
builds `capture_session session;` with the default null budget (`capture.cppm:13`). Through that session it does three
things:

- It loads the workspace, spec and every job declaration (`session.load` at lines 6026, 6041 and 6047). Each load
  calls `load_declaration(bytes, nullptr)`, a fresh declaration VM whose instructions, native bytes and live Lua memory
  never reach the factory counters.
- It reads the registry and observations (lines 6052 and 6056). These reads charge no composed native bytes.
- It enumerates the current and base trees (lines 6073 and 6074). This discovery staging is not composed.

The factory budget is reachable at that point as `vm.runtime.budget`. The record contains no mention of
`prepare_role` or the extension path. The inventory therefore omits a stage that owns declaration-VM and staging
metrics, and R1 is marked "met with a limit" without it (`review-c01/audit.txt`).

The other unbudgeted sites are the embedded load path (`acceptance/load.cpp:54`, `open_embedded` without a budget) and
the CLI render session (`src/cmd/planar/invocation.cpp:100`). Neither is created by a profile factory, so B1 does not
reach them. The inventory should still say so explicitly.

Required:

- Pass the factory budget into the role session, for example as `capture_session session(s.budget)`; it stays null for
  `extension_invocation::create`.
- Add a test, with a probe, that shows role-stage declarations and enumeration charging the one owner and refusing at a
  small ceiling.
- Add an inventory row for the stage, and inventory rows that place the embedded and render sessions outside the factory.

If the coordinator believes extension role preparation is outside X06-D, that is a scope decision for the operator. It
must be recorded; it cannot simply be left out.

### C01-R2 (required, tests): the sink's own instruction ceiling is not observed (row T3)

Probe mutants T3 (skip the post-frame comparison), T3c (skip the carried comparison), T3d (`>` instead of `>=`) and T16
(wrong maximum in the sink diagnostic) each pass the full 1091-test unit suite. I read their logs. The repair test
`the composed sink alone enforces the invocation instruction ceiling on declaration chunks` reaches both branches (the
R3a and R3c reach probes kill it). However, the refusal it observes comes from the runtime's own check downstream, so
deleting or loosening the sink's comparison changes nothing it measures. This leaves several criteria unproven for the
declaration stage:

- B1/B3: the declaration stage refuses at the owner.
- B7: the exact boundary of the sink.
- The B1 inventory claim in row 6b.

The record reports this honestly as OPEN. It still cannot be accepted as a limit, because the counter is reachable at
small ceilings. B7 allows a limit only for a counter that cannot be observed without a new public field.

Required: add a test in which the sink's refusal is the first observable failure. One option is a declaration chunk
that crosses the ceiling and is followed by a stage that would fail differently, or would never run, if the declaration
VM continued to its own 1,000,000 limit. The test must kill T3, T3c, T3d and T16 by name.

### C01-R3 (required, tests): the B3 stage tests T8 and T9 do not exist

R4 required rows for B3's stage clauses: exhaustion between stages, after phase 8, and after the all-job phases. Rows
T8 and T9 say "NOT REGISTERED; never written". The record's own acceptance table marks R4 and B3 "not met in part". The
existing `X06 D native exhaustion is resource_limit terminal and cleans nested roots` test does not show which stage
refuses.

Required: write both tests, register them, red-check them where they are test-first, and probe each with a mutant that
breaks that stage's refusal.

### C01-R4 (required, records or tests): B4's semantic half is still covered by an ordinary Lua error

B4 and the architecture's adversarial-matrix row "Semantic/protocol/resource failure caught by Lua" require that the
first terminal failure survives. Row T10 instead describes "B4 semantic failure caught by Lua is not terminal". Its test,
`a caught Lua error still leaves the budget counters monotone`, raises `error('x')` and `error({})`. These are ordinary
Lua errors, which the engine deliberately lets recover (see the existing `X05 B2 C caught illegal registration stays
sticky while ordinary caught Lua errors recover`). They are not engine semantic or protocol failures. The test is a
useful budget-monotonicity guard (it kills I10b and I6), but it does not cover B4's semantic half. Row T11, together
with I10, covers only the resource half.

Required: name an existing test that raises an engine semantic or protocol failure caught by `pcall` inside a
profile-owned frame and checks that the first failure stays terminal, together with a mutant it kills. The history and
projection tests around `runtime.t.cpp` lines 664 to 983 may qualify if they run under a profile factory; this should
be checked. If none qualifies, add such a test. Correct the T10 row text either way.

### C01-R5 (required, evidence or tests): the ceiling-headroom constraint is not demonstrated

The dispatch constraint and this review's brief both require "test ceilings with comfortable headroom (the coder
flagged 16000 nodes and 26738688 bytes as tight)". The record says the constraint is "NOT changed and not re-measured"
and "not demonstrated as met". Several of the new tests are calibrated close to the variant ceilings. For example, the
release-between-chunks test peaks near 12.6 MB per chunk against a 26.7 MB ceiling, and only the freeing keeps it under.

Required: measure and record, for each `PLANAR_BUDGET_*` ceiling, the peak that the fixtures relying on it consume
without exhausting it (both configurations), and state the margin. If the margin is not comfortable, raise the ceiling
and re-run the affected tests and probes.

### C01-R6 (minor, documentation and design): stale reference documentation and the loader-to-admission edge

- `docs/architecture.md:76` still says that "sharing 64 MiB/10000 capture limits with later services remain future
  integration". The accounting paragraph of `profile-preparation-architecture.md` (lines 414 to 428) still says that
  the metrics are "**distinct**" and that "no claim of already-composed 512 MiB follows". The repository instructions
  require affected reference documentation to change with the behaviour. Neither file is in the round's owned list, so
  the coordinator should assign the edits. The architecture paragraph is an authority document, so it may need a scoped
  amendment rather than a silent edit.
- `planar_lua_loader` now depends on `planar_capture_admission` only to obtain the `budget_sink` interface. That module
  is the filesystem metadata admission layer, which sits above the declaration loader in practice (`capture` already
  depends on the loader). There is no cycle, but the instructions ask for the engine/Lua boundary to be reviewed
  explicitly. A small interface module beneath both, for example one that depends only on `planar_value`, would keep
  dependencies pointing downward. Record the choice or move the interface.

### C01-R7 (minor, records): record accuracy

- The R4 row of the acceptance table says "table above is complete". Rows T8 and T9 have no test, and the
  extension-stage row (C01-R1) is missing.
- R1 is marked "met with a limit". With C01-R1 it is not met.
- The probe dispatch text was truncated inside row P1 (PROBES.md says rows after T3 were "not delivered"). The record
  maps those rows onto the parent's mutants after the fact. The next probe run should receive the full table.
- The V1 guard checks that no `file(READ ...)` line names `runtime.cpp`. The candidate satisfied this by changing the
  pre-existing test and shapes bridges from a literal path to `${planar_runtime_unit}`, so that clause is not a real
  guard. The real protection is the `FATAL_ERROR` and `REGEX REPLACE` clauses, and V2b shows a gap in the 300-character
  window. Record that the `file(READ)` clause is cosmetic.
- T9b (a detached sink returning success) is a GAP in PROBES.md, but the record lists T9 and T9b together as
  "unobserved". Keep the GAP classification for T9b.

## Limits carried and observed

- P01 is still masked. N1 phase coverage was not measured. Linux was not run (X08). The mutation probes ran on Debug
  only. All of these are recorded accurately.
- I accept the following as limits under B7 and the rule against new public fields: L6d (a failed `realloc` release),
  L7d (masked by ERRMEM), N2, N2d and N6c (equivalent alone), I4e and I4f (masked by the capture layer), E2, E3, E6b,
  E11, E12, E22, E25a to E25c, I3b and I10d.
- The pre-attach paths T3b, T4, T5, T5f, T7 and T8 (probe ids) are dead in the product, since the pre-VM charges are
  only the profile and module reads. I accept them, provided C01-R1's fix adds no pre-attach declaration stage.
- L7c (the diagnostic when the composed ceiling refuses Lua state creation) is an untested reachable path. Add a test
  for it when C01-R2's test is written.
- The verification reread allocates file-sized transient buffers without a composed charge. Only the inherited 64 MiB
  bound applies, and N2e pins this design. I accept it as transient staging, which is not retained.
- No claim of RSS or capacity guarantees is made. The trusted in-tree code threat model applies.

## Review evidence

The files are under `tests/native/v5-02-x06-d-evidence/review-c01/`:

- `identity-before.txt` and `identity-after.txt`: HEAD, the `src` and unrestricted diff hashes, and the status, taken
  before and after my runs.
- `rv1-build-tests.log` and `.exit`: the `planar_tests` build, with no work done.
- `rv2-ctest-x06-focused.log` and `.exit`: the focused X06 run, 149 of 149, 0 skipped.
- `removed-vs-x06d.txt` (empty) and `added-vs-x06d.txt` (42 names): the comparison with the X06-D inventory.
- `audit.txt`: the greps behind C01-R1 (budgeted-stage construction sites, `prepare_role` session use, `into_extension`),
  the loader edge and the stale reference text in C01-R6.
