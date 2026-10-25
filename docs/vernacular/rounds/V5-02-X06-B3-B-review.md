# V5-02-X06-B3-B independent review (bound capture, route, seal and all-job validation)

Disposition: **request-changes**.

The candidate adds a private consuming `profile_invocation::prepare_load` that
checks the final intent against the bound intent, reverifies the original
capture, opens bound source capture with the original root, declaration and
registry tokens, begins one profile frame, routes through the retained phase 8
callable, captures and seals the declaration closure, and runs phases 0, 1 and 2
for every job. It retains B and S in an opaque `profile_load_stage`. The design
follows architecture phases 1 to 3, no `prepared_profile` or v2 derivation
leaked in, and the full Debug gate passes 977/977 with all 963 prior names kept.

It cannot be approved yet, for four reasons:

- **X06B3B-R1.** Acceptance criterion 6 is not met. The candidate ships no
  mutation probes and no red-first evidence at all. The gate report says so
  itself.
- **X06B3B-R2.** I ran the probes myself. Removing the receipt reverification
  that happens before capture survives every B3-B test.
- **X06B3B-R3.** A counter reset between phase 8 and all-job validation also
  survives every B3-B test.
- **X06B3B-R4.** Nested temporaries from route, phases and F are never released
  on the success path. The test whose name claims "cleans nested roots" asserts
  nothing about roots.

Four other probes are killed: N3 refusal, consumed-before-copy, every-job
validation, and the native `same_selection` check (killed by the existing
D2-B2-C01 test). So the tests can fail, but two of the six invariants the round
names are not protected.

I reviewed in the shared checkout on `dev/planar-v2-vernacular` at HEAD
`4b79334929965b5e93ec4c790f866f7b9762b791`, against the uncommitted candidate.
I wrote only this file and new files under
`tests/native/v5-02-x06-b3-b-review-evidence/`. Mutation probes ran in a
temporary detached worktree, with its own build directory, in my scratchpad. I
removed it afterwards and never mutated the shared checkout. I did not commit,
install, publish or merge, and I used no old Planar tool, queue, skill, role or
installed binary. Builds, ctest and lint in `build/rewrite-debug` ran one at a
time, with no other ninja or ctest process running.

## Exact candidate

- `git diff HEAD` sha256 at review start and end:
  `db2969f6cd5f66bfdcbbdd13416091f63ba8896319993d828344aa34e3f76d3a`. This equals
  the gate report. The patch is preserved as
  [`candidate.patch`](../../../tests/native/v5-02-x06-b3-b-review-evidence/candidate.patch).
- Six tracked files changed: `src/engine/runtime/runtime.{cppm,cpp,t.cpp}` and
  `src/engine/capture/profile.{cppm,cpp,t.cpp}`. The untracked round record is
  the only other change. The evidence directory is gitignored, as for earlier
  rounds. All six files are owned by the round's packages. `source_capture.*` and
  both CMakeLists files are unchanged, and the round record says why.
- No file outside the round's ownership changed.

## Required change: X06B3B-R1 — mutation probes and red-first evidence are missing

Criterion 6 requires probes in a scratch copy for six invariants: single frame
and no reset, receipt reverification before capture, `same_selection`, every-job
validation, consumed-before-copy and N3 refusal. Each needs a raw log that names
a failing test, plus equal restored hashes. The evidence folder has none.
`tests/native/v5-02-x06-b3-b-evidence/raw/` contains only the five gate logs and
`ctest -N`. The round record's verification item 6 still reads "PENDING". The
gate report's section (b) also states that no red-first record exists for any of
the 14 new tests.

My probes below are reviewer evidence, not a replacement for the coder's. The
corrective round must produce its own probe set covering all six invariants, with
raw logs, after R2 to R4 are fixed.

## Required change: X06B3B-R2 — receipt reverification before capture is not tested

Architecture phase 1 requires the original capture and the initial metadata
receipts to be reverified before bound source capture. The candidate does call
`impl->capture.verify()` before `open_bound` (`runtime.cpp:5066–5067`). But
removing that call (probe `reverify`) leaves all 12 `[x06b3b]` cases passing
(469 assertions). The replaced-leaf test is still refused, because `open_bound`'s
own leaf verification catches the replacement. Later `capture.verify()` calls
after capture and after staging would also catch other changes, but only after
declaration content has been read.

**Fix:** add a test that changes something only the original profile capture
covers. Examples are the admitted ledger file, a profile module, or an admitted
ancestor relationship, changed between registration and `prepare_load`. The test
should assert a terminal `capture_changed` with zero declaration content reads
(using the existing `watch_path` and `planar_admission_read_attempts` seam). The
reverify probe must then kill that test.

## Required change: X06B3B-R3 — a counter reset inside the frame is not detected

Inserting `s.instructions = 0;` just before the validation input is built
(probe `reset`) leaves all 12 `[x06b3b]` cases passing. This is the boundary
between phase 8 and phase 0. The existing tests cannot see it, for three reasons:

- "one frame cumulative counters never reset" compares stage counters with the
  pre-`prepare_load` owner. By design, `begin_invocation` already moves from
  registration instructions to invocation instructions once, so that comparison
  says nothing about resets after the frame begins.
- The exhaustion test spends its budget in two custom F calls. It detects a reset
  between F calls, but not one between route and validation, or between
  validation and staging.
- The same applies to native bytes: the test only checks that they grow.

Criterion 3 requires exhaustion "across phase 8, all jobs and preparation".

**Fix:** add a check that fails when any counter decreases or restarts at any
phase boundary inside the frame. One option is a same-module observation of the
instruction and native counters taken after phase 8 and after validation,
asserting they never decrease and that the final stage value includes the phase 8
contribution. Another is a fixture whose exhaustion depends on route consumption.
The reset probe, and a native-bytes variant, must kill it.

## Required change: X06B3B-R4 — nested temporaries are not released and the cleanup test asserts nothing

`prepare_load` runs every nested Lua call through `call_retained_value`: phase 8
at `runtime.cpp:5106`, and phases 0, 1 and 2 plus every F through
`validate_profile_phases` and `invoke_shape_impl` with a profile frame. The
nested paths deliberately leave temporary root release to the enclosing handler:

- the comment above `call_retained_value` (`runtime.cpp:1982–1983`)
- the nested branch of the `phase` lambda (`runtime.cpp:4512–4521`)
- the nested branch of `invoke_shape_impl` (`runtime.cpp:4174–4191`)

The extension path has that outer handler. Its cleanup calls
`release_shape_temporaries` (`runtime.cpp:~5930–5939`). `prepare_load` has no
such cleanup. On success, the dormant VM inside `profile_load_stage` therefore
keeps every invocation-temporary tag and backing entry until the stage is
destroyed. B3-C plans to move that VM into `prepared_profile`. That conflicts
with the established invariant that invocation scratch and result tags are
released, while exact pre-invocation membership survives. It also conflicts with
the architecture's split of duties: helpers "never ... release outer roots", so
the outer owner must. On failure, cleanup happens only because the consumed owner
and its VM are destroyed.

The test "cumulative instruction exhaustion preserves the first error and cleans
nested roots" asserts nothing about roots. `CHECK(owner.observe().instructions
== 0)` only restates that the caller's owner was moved from.

**Fix:** release the shape temporaries at the end of `prepare_load` on both
success and failure, with the same discipline as the extension outer cleanup.
Then observe exact tag and backing membership before and after, using the
reversible observer approach accepted in B2-C, B3-B/R01 and B4-B. Cover a success
case and an exhausting case. If the coordinator decides that retaining the
temporaries until VM destruction is correct, record and justify that decision in
the round record, and rename or rewrite the test so that it claims only what it
asserts.

## Acceptance criteria

| # | Criterion | Finding |
| --- | --- | --- |
| 1 | Success through phase 2, custom and mixed jobs; invalid job fails and publishes nothing; every job validated once | **Met.** The two fixture mixes succeed (2 jobs, 1 or 2 F dispatches). The built-in `nosuch` reference and the custom F refusal both return semantic errors, with the owner consumed. The every-job probe kills six cases, including "every job is validated exactly once". |
| 2 | Replaced declaration leaf refuses terminally before content, no retry | **Met behaviourally.** `capture_changed`, zero reads of the replacement inode, and the owner is consumed. The refusal comes from the profile capture or `open_bound` checks, not specifically from `same_selection` (see N3). |
| 3 | One frame, no reset, cumulative exhaustion across phase 8, all jobs and preparation, first error kept, nested roots cleaned, frame branches exercised | **Not met.** Positive profile-frame branches now run, and first-error preservation through `shape_failure` holds. The no-reset invariant is unprotected (R3) and nested root cleanup is neither done nor asserted (R4). |
| 4 | No caller upgrade; opaque, consumed and failed owners refuse; stage has no public accessor | **Met.** `prepare_load` accepts only an intent and compares it against the bound intent. Static asserts cover the stage's surface. The consumed and N3 probes are killed. |
| 5 | 963 names kept; diagnostics byte-identical; no `assert()`; `std::expected`; docs | **Met.** 977 = 963 + 14, nothing removed, and the new-name list matches the gate's byte for byte. The diff adds no diagnostic text (only existing `profile_owner_protocol_error`, `external_*`, `command_resource` and pass-through source and profile details) and no `assert(`. Lint and Doxygen pass over 136 sources. |
| 6 | Six mutation probes with raw logs and restored hashes | **Not met** (R1). Reviewer probes show two of the six survive (R2, R3). |
| 7 | Fresh profile acceptance refusal and CLI unchanged; no cycle; no runtime to ledger-acceptance edge | **Met.** No `ledger/*`, handler or CMake change, and `runtime.cppm` imports are unchanged. |

## Advisories N1 to N5 and X06B3AC02-N3

- **X06B3A-N1 resolved.** `begin_frame` is a private static. `prepare_load` is
  its only caller, before phase 8. A static assert proves the public method is
  gone.
- **X06B3A-N2 partly resolved.** Positive profile-frame branches in
  `validate_profile_phases` and `invoke_shape_impl` now run in product code. The
  refusal branches (`profile_frame_valid` false, and the exhausted check on entry
  at `runtime.cpp:4474`) remain unreached. The evidence says "same-module
  observers" exist for them, but there are none (see N4 below). The
  cumulative-exhaustion half is covered by R3 and R4.
- **X06B3A-N3 resolved.** Known-load `into_extension` is refused without
  consuming the owner. The adopted receipt fields are removed. The probe is killed
  by three tests.
- **X06B3A-N4 resolved with caveats (advisory N1 below).**
- **X06B3A-N5 resolved.** The `runtime.cppm` `@file` brief names
  `prepare_load`, the private frame and the `into_extension` rule.
- **X06B3AC02-N3 recorded.** `note_registration` stays public because
  `capture.profile` cannot name the runtime caller without an import cycle. It
  now saturates, and two characterization tests show it carries no authority. The
  reason is acceptable.

## Advisory notes (non-blocking)

- **X06B3B-N1: N4 test and charge order.** "owner implementation allocation is
  charged identically by both factories" passes whether or not the charge exists,
  because both factories would drop it equally. Its own comment admits this. Both
  factories also charge after `make_unique`, while the stage is charged before
  its allocation. Charge before allocating, and use a test that can detect
  removal: for example, compare against a same-module observation of
  `sizeof(implementation)`, or probe the removal.
- **X06B3B-N2: name and content drift in B3-A tests.** The names are kept, as
  required. But "begin_frame is once only and blocks adoption" no longer calls
  `begin_frame`, and "into_extension adopts the same VM and receipts without
  recapture" now uses an opaque owner that has no receipts. Note this in the
  record so later readers do not take these names as coverage.
- **X06B3B-N3: `same_selection` is protected only at the native layer.** Making
  the native check `native.cpp:251` permissive survives all B3-B tests, because
  the profile capture check refuses first. The existing "D2 B2 C01 first
  retained reads require original root selection" test kills it. This matches the
  round's decision. The corrective probe set should cite that test explicitly.
- **X06B3B-N4: unreached defensive branches.** List the unreachable refusal
  branches in the record, with reasons, instead of citing observers that do not
  exist.
- **X06B3B-N5: catch set.** `prepare_load` catches `diagnostic`, `bad_alloc` and
  `length_error`, but not `...`, unlike `begin_frame` and the factories. Nothing
  currently throws anything else, but an unexpected exception would escape after
  the owner was consumed. Consider a final catch that maps to protocol.
- **X06B3B-N6: record hygiene.** The round record's verification list is still
  all "PENDING". Fill it, or point it at the evidence report, in the corrective
  round.

## Independent gate (macOS Debug, `build/rewrite-debug`)

| # | Command | Result | Log |
| --- | --- | --- | --- |
| 1 | `cmake --build build/rewrite-debug --target planar_tests` | exit 0 | `01-build-planar_tests.log` |
| 2 | `ctest --test-dir build/rewrite-debug --no-tests=error -j 8` | exit 0, 100% passed out of 977; no failed, skipped, disabled or not-run entries | `02-ctest.log` |
| 3 | `ctest -N` inventory against the B3-A-C02 inventory | 977 names; 963 kept; 0 removed; 14 new, matching the gate list | `03-ctest-N.log`, `inventory.txt`, `new-names.txt`, `removed-names.txt` |
| 4 | `python3 scripts/rewrite-check.py lint --build-dir build/rewrite-debug` | exit 0, "Formatting and Doxygen checked 136 active source files" | `04-lint.log` |
| 5 | `cmake --build build/rewrite-debug`, run twice | both exit 0; the second reports "ninja: no work to do." | `05-build-all.log`, `06-build-nochange.log` |

I ran ctest in parallel (`-j 8`). The gate runner's serial run is in its own
`raw/02-ctest.log`, also 977/977.

## Reviewer mutation probes

These ran in a detached scratch worktree of `4b793349` with `candidate.patch`
applied (diff hash equal to the candidate), using its own Debug build. The
baseline `[x06b3b]` run passes 12 cases and 469 assertions. Each probe applies
exactly one textual mutation (occurrence count 1), rebuilds, runs, then restores
the file. Every log records `restored equal True`. Logs, mutation texts and
scripts are under `tests/native/v5-02-x06-b3-b-review-evidence/probes/`.

| Probe | Mutation | Result |
| --- | --- | --- |
| `n3` | drop `|| _owner->known_load` from `into_extension` | **killed**: "X06 B3 B known-load owner refuses into_extension terminally-free and stays usable", "X06 B3 A factories capture exactly once and adoption reads nothing", "X06 B3 A registration count stays one across into_extension" |
| `consumed` | on intent mismatch, put the owner back and return without consuming | **killed**: "X06 B3 B final intent must equal the bound intent", "X06 B3 B opaque consumed failed and moved-from owners refuse prepare_load" |
| `everyjob` | skip F dispatch for request index > 1 in profile frames | **killed**: six cases, including "X06 B3 B every job is validated exactly once" and "prepare_load succeeds through phase 2 for custom and mixed jobs" |
| `samesel-native` | `native.cpp:251` same-selection refusal changed to `if (false)` | **killed** by "D2 B2 C01 first retained reads require original root selection" (`planar_capture_native_tests`) |
| `samesel` | same mutation, `[x06b3b]` only | **survives** (N3, masked by the profile capture check) |
| `reverify` | remove the pre-capture `impl->capture.verify()` | **survives** (R2) |
| `reset` | `s.instructions = 0;` before the validation input | **survives** (R3) |

Probe-harness errors, preserved rather than hidden:

- The first `samesel` run (`probe-samesel-CONTAMINATED.log`) is invalid. My
  restore step kept the backup's older mtime, so ninja did not rebuild
  `runtime.cpp` after the `everyjob` probe, and the `samesel` binary still
  contained that mutation. The runtime probes before it are unaffected: each one
  rewrote `runtime.cpp` and rebuilt it (339-step rebuilds). I fixed the script to
  touch restored files, rebuilt, and re-confirmed the baseline
  (`rebaseline-x06b3b.log`) before rerunning `samesel`. The final rebuild after
  all probes passes again (`final-x06b3b.log`).
- `probe-samesel-native-WRONGBINARY.log` targeted the wrong test binary and
  matched zero tests. It does not count. The valid run is
  `probe-samesel-native.log`, with a zero-mutation baseline in
  `native-baseline-c01.log`.

## Limits

- macOS Debug only. No RelWithDebInfo, Linux, RSS or whole-invocation budget
  claim, matching the round's limits.
- R4 is reasoned from source; I did not instrument tag and backing membership.
  Release placement and the observer are the corrective round's job.
- Reviewer probes cover only the six named invariants. They do not replace the
  coder's required probe and red-first evidence (R1).
