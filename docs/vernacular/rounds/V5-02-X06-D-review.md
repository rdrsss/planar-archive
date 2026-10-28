# V5-02-X06-D independent review (whole-invocation budgets and the integrated X06 gate)

Disposition: **request-changes** (not records-only).

The evidence for what the candidate does is sound. The candidate bytes are the ones recorded. The one
red test failed on assertions against a saved pre-fix snapshot. Every probe kill I spot-checked names
the intended test. The Debug and RelWithDebInfo runs are clean. All 1041 baseline names are kept and
15 are added. The closed message table, the public surface and the X06-C seam are unchanged.

The round still cannot be approved, because the budget row's main obligation is not delivered. The
dispatch, criterion B1 and the architecture ask for one factory-owned budget injected into capture,
admission metadata and declarations. The candidate's only product change charges the sealed source
capture's file bytes to the existing VM counter, after those bytes have already been allocated. Three
things stay separate owners that never feed the composed counters: admission metadata, the
declaration loader VM (a fresh state per chunk), and the retained declaration values. The required
inventory of metric ownership, and the statement of which X-LIMIT ceilings are composed and which stay
inherited, is not recorded anywhere (R1). The capture charge also comes after allocation, which
breaks B2's "checked addition before allocation" and the X-LIMIT preamble (R2). Both need source
changes or a recorded operator amendment. G1 was not run in the required order (R3). The invariant
table also has gaps (R4) and several records are inaccurate (R5).

I reviewed in the shared checkout on `dev/planar-v2-vernacular` at HEAD
`a0a8b3fb7b038cba98e77f1f80e0008353279cf3`, against the uncommitted candidate. I wrote only this
file and new files under `tests/native/v5-02-x06-d-evidence/review/`. I applied no mutants and edited
no source, test, record or existing evidence. I ran two commands on `build/rewrite-debug`, one at a
time, with no other ninja or ctest process running:

- `cmake --build build/rewrite-debug --target planar_tests`, which exited 0 with `ninja: no work to do.`
- `ctest --test-dir build/rewrite-debug -R 'X06' --no-tests=error --output-on-failure`

I did not commit, push, merge, install or publish. I used no old Planar tool, queue, skill, role or
installed binary.

## Exact candidate

- `git diff HEAD` and `git diff HEAD -- src` both have sha256
  `125785336bdb7891d0600149f28728f4dd484cf7dd0aedf4e73c60eeadf18ca6`. This equals the round record,
  `raw/r3-candidate.sha256` and `probes/PROBES.md` generation 2. I took it before and after my ctest
  run, and it did not change (`review/identity.txt`, `review/candidate.sha256`).
- The four per-file sha256 values in the record equal the working tree. `runtime.cppm`,
  `source_capture.*`, `ledger/acceptance.cppm`, `ledger/acceptance.cpp` and
  `src/cmd/planar/CMakeLists.txt` are unmodified.
- `git status --short` shows exactly the four modified files and the untracked round record. The
  evidence folder is gitignored (`.gitignore:37`).
- The four evidence-file hash prefixes quoted in the record (`PROBES.md`, `REPORT.md`, `RED.md`,
  `inventory.txt`) match the files.

## Independent verification

| Check | Result (files under `tests/native/v5-02-x06-d-evidence/`) |
| --- | --- |
| Gate exits | Every `raw/r3-*.exit` reads 0, except `r3-11-redact.exit`, which reads 2. That run is the documented usage error with no `--repo-path`/`--home`; `r3-11b` reads 0. |
| Debug ctest | `raw/r3-2-ctest-debug.log` reads "100% tests passed out of 1056" in 379.53 s. The only "skip" text is in the names of tests #573 and #1028. Test #531 took 15.47 s, the same as the C04 baseline log `v5-02-x06-c-c04-evidence/raw/2-ctest.log` (15.47 s), so it is not slowed. The slowest X06 D test took 2.42 s. |
| RelWithDebInfo ctest | `raw/r3-9-ctest-release.log` reads "100% tests passed out of 1056" in 153.01 s. There are no skip lines, and #531 took 4.54 s. `r3-7` configured the release preset. |
| Inventory | Compared as sets with `v5-02-x06-c-c04-evidence/inventory.txt`, which has 1041 names: 0 removed and 15 added, all `replacement.X06 D ...` (`review/removed-vs-c04.txt`, `review/added-vs-c04.txt`). The names in the Debug and Release `ctest -N` logs are identical to each other and to `inventory.txt`. |
| Lint and no-work build | `raw/r3-4-lint.log` ends "Formatting and Doxygen checked 142 active source files". `r3-5` and `r3-6` both say `ninja: no work to do.`, and so did my own `planar_tests` build. |
| Focused run (mine) | `ctest -R 'X06' --no-tests=error`: 107 of 107 passed, 0 skipped, exit 0, 59.56 s. All 15 X06 D names ran (`review/rv2-ctest-x06-focused.*`). The candidate hash was the same before and after. |
| Red-first | `red/snapshot/candidate.diff` hashes to `07414a33...9b69e`, as RED.md says. It contains the tests and the CMake variant but not the `runtime.cpp` charge loop. `red/raw/t1.log` is a behavioural failure at `runtime.t.cpp:3189` (1952 >= 5851), 3195 and 3196: five failed assertions out of 45, not a compile error. The snapshot differs from the final candidate only in the charge loop and the C03 vocabulary rewording (`review/red-snapshot-vs-final.diff`). The other 16 RED rows are guards that already passed, and RED.md says so. |
| Probes | `PROBES.md` has 73 result rows and 17 survivors, which matches the record. I read the raw diffs and failing-test lists for I1, I4a, I5b, I6, I10, I17c, E2 and E26. I1 fails only the capture test, I4a only the lower-limits test, and I5b the two native-ceiling tests. I17c fails the construction-path and surface tests. I6 fails only two existing B3 B tests, and I10, E2 and E26 pass 144 of 144, all as recorded. The static_asserts cited for I17, I17b and I17d are at `runtime.t.cpp` lines 1028, 1044 and 1054. |
| Small-ceiling variant | The generated `build/rewrite-debug/src/engine/runtime_budget_test.cppm` equals `runtime.cppm` with only the module and namespace renamed. The generated `.cpp` differs from `runtime.cpp` in only the four ceiling lines (27 to 29 and 3135). So the variant compiles the exact runtime. |
| Surface | `X06 D runtime public surface and resource messages stay closed` pins the public sections and access grants of `prepared_profile`, `profile_invocation` and `profile_load_stage`, both observation structs, and the set of seven `resource_limit` literals in `runtime.cpp`. The candidate adds no literal. The seam source-scan guard and the C03 seam test pass in both full runs. |

## What is sound

- **Factory counter carried into the frame.** `begin_invocation` keeps `native_bytes`. It resets only
  the instruction counter, from the registration limit to the invocation limit, which is pre-existing
  behaviour and pinned by I7. `begin_frame` refuses a second frame, and preparation does not
  re-register (I2a and I2b are killed by the named test).
- **Cumulative native and instruction ceilings across registration, route, shape dispatch and
  preparation.** These are shown at small ceilings with adjacent pass and fail, to within one hook
  quantum (instructions) or two charge steps (native bytes). Exhaustion gives `resource_limit` with
  the existing message, cleans nested roots and leaves the owner refusing a retry (I5b, I9a, I9b and
  E1 are killed).
- **Moves and wrapper destruction.** Covered for `prepared_profile` and the consumed owner (I11a to
  I11c, E21 and I12 are killed). Owner moves are covered by the existing X06 B3 A test.
- **The red test is real.** It measures the native-counter gap between marks 1 and 2 and fails without
  the loop. It fails for a double count (I1c) and for counting the first file only (I1d).

## Findings

### X06D-R1 (required, source and records): the budget owner is not composed into admission, declarations or retained declaration values, and the required inventory is missing

The dispatch, Objective 1, says: "Today the content, admission metadata, declaration-VM and retained
trusted-VM metrics are distinct, each with its own storage. The architecture says later composition
must inject a common budget owner from the initial factory into these stages." Criterion B1 says the
factory's budget is "the only source of cumulative charges for capture, registration, declarations,
...", and that "no stage creates a second budget". The architecture
(`profile-preparation-architecture.md`, lines 414 to 428) repeats that the common owner must be
injected into these stages. It also says the first slice "must inventory metric ownership/sizes".

The candidate's single product change is the loop in `prepare_load`, at `runtime.cpp` lines 5286 to
5295. After the bundle is captured, it charges each sealed capture file's bytes, path, root, mode and
`sizeof(captured_file)` to the VM's `state::native_bytes`. Nothing else is threaded through
(`review/accounting-owner-audit.txt`):

- **Declarations.** `capture_session::load` calls `load_declaration(input->bytes)`
  (`capture/capture.cpp:45`). That function takes no budget, creates its own `state` with its own
  64 MiB, instruction and node counters (`lua/loader.cpp`, lines 11 to 13 and 45), and reports
  nothing back. `source_capture::open_bound` and `capture()` evaluate the workspace, spec and job
  declarations this way. They run before and inside the frame, and none of their instructions, Lua
  bytes or native bytes reach the owner's counters. The candidate's own I4 test shows this: a
  declaration loop refuses with `maximum == 1000000`, the loader's separate counter.
- **Retained declaration values.** The `declaration` trees kept in `source_capture`'s `declarations`
  map are not charged to the owner. The new loop charges only the file bytes of the sealed snapshot.
- **Admission metadata and discovery staging.** These keep their own budgets
  (`capture/admission.cpp`, lines 10 to 11 and 463, "<=64 MiB charged storage";
  `capture/native.cpp` lines 131 to 144, `discovery_budget` and `ancestor_budget`). The owner's own
  comment says so: "this excludes receipts and capture, whose storage is charged by the
  admission/native layers" (`runtime.cpp:4868`).

The objective also asks search to state "which ceilings in X-LIMIT (Registration, Invocation,
Declarations, Shape, Module capture) are composed and which stay lower inherited limits". That
statement does not appear in the round record, `REPORT.md`, `RED.md` or `PROBES.md`. The record
marks B1 "met" on the strength of I1, I2 and I18 alone, and B6 "met with limits" without mentioning
the declaration-file handle or the comparison and discovery staging that B6 names.

Required:

- Record the inventory: each accounting store, its owner, what it counts and its ceiling. For each
  X-LIMIT row, say whether it is composed into the factory budget or kept as an inherited lower
  limit, and why.
- Then do one of the following:
  - Thread the factory owner into the stages B1 names, so that every charge they make is also a
    checked charge on the one owner, while their stricter per-chunk or per-stage limits keep winning.
    This covers the declaration loader (instructions, Lua bytes, native bytes and retained values)
    and the admission and source-capture storage. Add tests and probes that show the composed
    ceiling refusing in those stages at small ceilings.
  - Obtain a recorded operator amendment that narrows B1, B6 and the architecture paragraph to what
    is composed. Choosing between these is the operator's decision, not the reviewer's.

### X06D-R2 (required, source): the capture charge comes after the allocation it accounts for

X-LIMIT opens with "All bounds are checked before allocation/copy/action, with checked addition".
B2 requires the composed ceilings to be enforced "with checked addition before allocation". The
architecture adds that "retained data, copies and fallback allocations require checked charges
before allocation".

The new loop runs only after `source->capture(...)` (line 5279) and after both `verify()` calls.
By then every captured file is already read and stored. The composed 512 MiB native ceiling
therefore cannot prevent the capture allocation; it can only refuse afterwards. The capture's own
64 MiB retained bound does stay a before-allocation check, and the overshoot is bounded by it.

The record does not disclose this. `REPORT.md` notes only that the "checked-addition-before-allocation
clause of B2" has no test. The record's B2 row says "met with limits" and cites I1b, which is about
the loop's addition, not about when the charge happens.

Fix this together with R1 by charging inside the capture stage before each read (for example through
the injected owner), with a red test showing a refusal before the read at a small ceiling.
Alternatively, record an operator-approved exception.

### X06D-R3 (required, gates): G1 was not run on the frozen candidate before the full runs

G1 requires focused, nonzero, zero-skip runs of every X06 test and the new budget tests before the
full runs. The gate runner did not run them; `REPORT.md` and the record's G1 row say "partly met".
The record cites three runs instead:

- `raw/p2-ctest.log`: 245 runtime tests on the earlier `d1779fbf...` candidate, before the C03
  rewording.
- `raw/red/p1-focused-x06d.log`: before the fix, with one failure.
- The probe baseline in a scratch copy.

My own focused run (107 of 107, 0 skipped, on `125785336b...`) shows that the X06 set passes on this
candidate, but it ran after the full runs. Because R1 and R2 change the candidate, the next gate run
must do the focused X06 and budget runs first, then the full Debug and RelWithDebInfo runs, as G1
requires.

### X06D-R4 (required, records, and tests where noted): invariant-table gaps

The dispatch requires every acceptance criterion to have its own row, mutant and killing test.

- **G1 to G5 have no rows.** They are process gates, so the "mutant" can be "not applicable". Each
  row should still name its evidence file and exit code.
- **B3's stage clauses are not mapped.** B3 requires exhaustion "in any stage, including between
  stages and in preparation after phase8 and the all-job phases". The native exhaustion test's
  `edge` and `far` probes do not show which stage refuses. No row shows a composed-ceiling refusal
  in phases 4 to 6. The existing X06 B3 C allocation-failure test may cover the post-validation
  stage; if so, name it in the row. If not, add a test (test change).
- **B4's semantic half is untested.** B4 says "a resource or semantic failure caught by Lua". The
  new test covers only an instruction exhaustion caught by `pcall`. The row's listed mutant (I10)
  and E25a to E25c survive. E27, the only kill, changes the ceiling constant, not the
  caught-failure behaviour. Either name an existing test that covers a semantic or protocol failure
  caught inside the profile frame, or add one (test change). Also record why E2 and E3 are
  unobservable: the returned error is identical, and a retry is refused as protocol anyway.
- **Rows I3 and I6 list tests their own mutants do not kill.** I6 (instruction reset between route
  and validation) fails only `X06 B3 B exhaustion depends on route consumption ...` and
  `X06 B3 B one cumulative instruction and native byte budget spans ...`. The X06 D test "invocation
  instruction ceiling is shared by every stage" passes under it, so the test's "every stage" claim
  does not hold for the route stage. I3a, I3c and I3e are likewise killed only by other tests. The
  rows should name the tests that actually kill these mutants, and the I6 test's comment, which
  says "Mutant I6: reset the instruction counter between route and validation", should be corrected
  the next time the source is touched.
- **B6 lists only shape-dispatch and phase-4 refunds.** It needs rows, or an R1 disposition, for the
  declaration-file handle and the comparison and discovery staging.

### X06D-R5 (minor, records): evidence index and stale statements

- The folder has no `README.md` index. The X06-C rounds each have one, and the orchestration rule
  commits `REPORT.md`/`README.md` and top-level manifests. Add one that names the final identity and
  lists the statements below that the record corrects.
- `REPORT.md` has three stale statements:
  - Under Limitations it says the probes "ran ... on a ctest filter that excludes #531 and
    capture/profile tests". Generation 2 included `profile capture`.
  - Its Mutation probes section lists I4b to I4h as limits. Generation 2 kills I4b, I4c, I4g and I4h.
  - Its "Missing: no red run exists for the post-red candidate" is not wrong, but it should point to
    the record's explanation.
- The record's B2 row should list R2's timing gap alongside I1b.
- The variant's `FATAL_ERROR` guard runs in every configure, product builds included. Reformatting a
  ceiling line in `runtime.cpp` would therefore stop the whole project from configuring. Record this
  coupling, or limit the guard to test builds. The I14 kills were also measured with the guard
  relaxed; with the guard in place, those mutants are configure-time failures.

## Limits carried and observed

- P01 is still masked. N1 phase coverage was not measured. Linux was not run (X08). Test #531 is
  carried at 15.47 s of its 30 s timeout on Debug, the same as the C04 baseline. All of these are
  recorded accurately.
- The 512 MiB composed Lua ceiling (E7, E26) and the 64 MiB declaration Lua ceiling (I4d) are not
  observed at small ceilings. Node-ceiling exactness (L2), the fixed `sizeof` charges (E11, E12), the
  nodes reset in phase 4 (E6b) and `profile_load_stage` move assignment (E22) are recorded limits.
  I accept them as limits under B7, given the rule against new public fields.
- I4e and I4f, the registration-only raises, are masked by the capture layer. I accept that as
  redundant layering.
- The probes excluded #531. The full gate runs cover it.
- No claim of RSS or capacity guarantees. The trusted in-tree code threat model applies.

## Review evidence

The files are under `tests/native/v5-02-x06-d-evidence/review/`:

- `identity.txt` and `candidate.sha256`: HEAD, diff and per-file hashes, the red snapshot hash and
  status.
- `head.txt`: HEAD.
- `rv1-build-tests.*`: the `planar_tests` build, with no work done.
- `rv2-ctest-x06-focused.*`: the focused X06 run, 107 of 107.
- `removed-vs-c04.txt` (empty) and `added-vs-c04.txt` (15 names): the inventory comparison.
- `red-snapshot-vs-final.diff`: the red snapshot against the final candidate, with index lines
  ignored.
- `accounting-owner-audit.txt`: the greps behind R1 and R2.
