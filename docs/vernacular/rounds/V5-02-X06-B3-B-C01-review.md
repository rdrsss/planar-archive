# V5-02-X06-B3-B-C01 independent review (corrective round for bound capture and all-job validation)

Disposition: **request-changes**.

The combined B3-B plus C01 candidate fixes most of what the B3-B review asked
for. `prepare_load` now takes the shape baseline before phase 8. A frame cleanup
releases nested temporaries on success (before staging) and on every failure
exit. Owner allocation is charged before `make_unique` in both factories, and a
final `catch (...)` is present. Five new tests were added, all 977 earlier names
are kept, no diagnostic text changed, and the full Debug gate passes 982/982.
In my own scratch probes, release removal on success, on failure and on both is
killed by named tests, and so are every-job, consumed-before-copy, N3 and the
native `same_selection` check.

The round cannot be approved yet, for three reasons:

- **X06B3BC01-R1.** Acceptance criteria 1 and 4 require red-first records and
  raw probe logs. Neither exists for the five new tests or for the repair's
  probes, and the repair applied its mutants inside the shared checkout and
  `build/`.
- **X06B3BC01-R2.** R3 is still open. An instruction reset at the route to
  validation boundary survives all 34 `X06 B3` tests when it is placed just after
  the test's own `mark(2)`.
- **X06B3BC01-R3.** The R2 test does not assert what its name says. It counts
  registry reads, not declaration reads. The companion read-count test claims an
  ordering ("before source binding") that a reordering mutant shows it does not
  check. The reason the behavioural test cannot kill the reverify mutant (the
  native receipt check masks it) is not written down in the round record.

I reviewed in the shared checkout on `dev/planar-v2-vernacular` at HEAD
`0cc29923383b39508e31cbf0cb62651185963480`, against the uncommitted candidate.
I wrote only this file and new files under
`tests/native/v5-02-x06-b3-b-c01-review-evidence/`. Mutation probes ran in a
`git archive HEAD` copy, with the six candidate source files copied in, under my
session scratchpad (`rvc01/wt`) with its own Debug build. That is outside the
checkout and `build/`. I never mutated the shared checkout. I did not commit,
install, publish or merge, and I used no old Planar tool, queue, skill, role or
installed binary. Builds, ctest and lint in `build/rewrite-debug` ran one at a
time, after the probes had finished and with no other ninja or ctest process
running.

## Exact candidate

- `git diff HEAD` sha256 at review start and end:
  `04fda68d8a48cb57728554ef5b43bd0d9c72bf0818e5cdc3157ca0345f748bf5`. This equals
  the gate report and the prober's PROBES.md. The patch is preserved as
  [`candidate.patch`](../../../tests/native/v5-02-x06-b3-b-c01-review-evidence/candidate.patch).
- Tracked changes: the six B3-B files and the pre-existing
  `docs/vernacular/roadmap.md` status edit. Untracked: the round record and the
  B3-B review. The C01 changes touch only `src/engine/runtime/runtime.{cppm,cpp,t.cpp}`.
  The three `profile.*` files still hash to their pre-C01 values (`7bb80563…`,
  `91ac81f3…`, `d94976e9…`). `source_capture.*`, CMake, `ledger/*` and handlers
  are unchanged.
- I compared the C01-only changes against the pre-fix snapshot in the
  scratchpad (`b3b-c01-prefix/`, hashes `7783183f…`, `f30b48ea…`, `af373673…`, equal
  to the round record). The diffs are kept as `c01-only-runtime.{cpp,cppm,t.cpp}.diff`.

## Required change: X06B3BC01-R1 — no red-first records, no raw logs for the repaired candidate, and in-place mutation

Criterion 1 requires a red-first record for each R2, R3 and R4 test. Criterion 4
requires raw logs, named failing tests and equal restored hashes. The brief and
the standing rules require the mutation to happen in scratch copies outside the
checkout and `build/`. What the evidence actually holds:

- **Red-first: none, for any of the five new tests.** The gate report section (b)
  and `raw/coder-REPORT.md.orig` both say package A's record was never delivered.
  The record's verification item still says "none is fabricated". A mutant run is
  not a red-first record. The rule asks for the new test, run against the pre-fix
  snapshot (hash recorded), failing behaviourally with the exact command.
- **Raw logs.** `raw/probes/` holds logs only for the *pre-repair* candidate
  (`05c3f070…`). On that candidate `reverify`, `reset-c-instr` and `baseline`
  survived. The PROBES.md for the repaired candidate says its "raw per-probe logs
  lived there (excerpts below)" in a scratch directory it then removed. So the
  `reverify-*`, `reset-e-instr`, `sf-*`, `single-frame`, `reset-c-diag` and
  `baseline` results on the current candidate exist only as excerpts.
- **In-place mutation.** The repair section of
  `raw/prior-REPORT-before-regate.md.orig` says: "Mutants were applied in place in
  the checkout (not a scratch copy) … `cmake --build build/rewrite-debug`". This
  breaks R1's "mutate only scratch copies outside the shared checkout and
  `build/`". The restored hash `1ff83396…` matches the current file, so I see no
  damage. But the rule exists because the checkout is shared.

My reviewer probes below (raw logs kept) cover the six invariants for this
candidate. They do not replace the coder's red-first records or probe set.

**Fix:** after R2 and R3 are fixed, produce a behavioural red-first record for
each new or changed test against the hashed pre-fix snapshot, with the command
and failing assertion. Re-run the probe set in a scratch copy and keep its raw
logs in `tests/native/v5-02-x06-b3-b-c01-evidence/`.

## Required change: X06B3BC01-R2 — an instruction reset between route and validation still survives (R3 open)

The new test "one cumulative instruction and native byte budget spans route
validation and preparation" compares four counter marks that `prepare_load`
records itself. `mark(2)` is taken after the validation input is built, and
immediately before `validate_profile_phases`. A reset placed after `mark(2)` is
therefore invisible:

- Probe `reset-f-instr` inserts `s.instructions = 0;` right after `mark(2);`
  (mutant `d9b54d0579c3…`). It survives all 34 `X06 B3` tests
  (`probes/probe-reset-f-instr.log`). That is exactly the phase 8 to phase 0
  boundary that R3 names. `i[2] < i[3]` still holds, because validation alone
  (about 122k instructions) exceeds route (about 9k). The heavy check
  `marks[3] >= marks[2] + 30000000` also holds, because validation alone exceeds
  30M.
- The mutant is not equivalent: it forgives every route instruction, so a profile
  whose route plus validation just exceeds the 100M budget would succeed.
- The native-byte twin `reset-f-bytes` is killed (`n[2] < n[3]` fails), as are
  resets before `mark(1)` and between `mark(1)` and `mark(2)` (`reset-e-*`,
  `reset-a-instr`). The test catches only resets that land between two of its own
  marks.

**Fix:** add an observable that does not depend on where `prepare_load` happens
to take its marks. The B3-B review suggested one: a fixture whose exhaustion
depends on route consumption. Its route phase 8 should burn enough instructions
that route plus validation exceeds 100M while validation alone does not. Assert
`resource_limit`/`instructions` on it, plus success on a variant with a light
route. Then show `reset-f-instr` killed, with a raw log.

## Required change: X06B3BC01-R3 — the R2 tests overclaim, and the masking is not recorded

1. **"capture change before prepare_load refuses capture_changed with zero
   declaration reads" counts registry reads only.** It calls
   `watch_path(f.declaration); watch_path(f.registry);`. The interposer
   (`tests/native/v5-02-x04-a1-support/syscalls.cpp`) holds a single watched inode
   and `planar_admission_watch` resets the counter. So the second call replaces
   the first, and `planar_admission_read_attempts() == 0` says nothing about the
   declaration. The same pattern occurs at `runtime.t.cpp:1528–1529` (the N2
   adoption test) and `:1637–1638`.

   I checked that the claim itself is true. A positive control shows declaration
   reads are counted: watching the declaration in a successful `prepare_load`
   gives 10 reads (`probe-control-declaration-reads.log`). With the test edited to
   watch only the declaration, it passes on the candidate
   (`probe-watch-decl-only.log`). So only the assertion is wrong, not the
   behaviour.
2. **That behavioural test cannot kill `reverify-pre`, and the record does not
   say why.** With the pre-capture `capture.verify()` removed (mutant
   `62086754001d…`, identical to the coder's), the test still passes, even when it
   watches only the declaration (`probe-watch-decl-reverify-pre.log`). The cause
   is the native retained read: `native.cpp` `read_file` calls
   `r.authority->verify(fd)` before any read, so `open_bound` refuses before the
   frame begins (the test asserts `cleanup_ran` false). This is the same masking
   as N3 for `same_selection`. The repair note in the raw prior report says this,
   but the round record does not.
3. **"captured profile content is reverified before source binding, after
   capture and after sealing" checks a count, not an order.** It pins six reads of
   the module, so it kills the removal of any one of the three verifies. My probe
   `reverify-moved` removes the pre-binding verify and adds one after source
   capture (mutant `52b29026261a…`), and all 34 tests pass. The name's "before
   source binding" is not protected. This test is new in this round, so it can
   still be renamed or rewritten.

**Fix:** watch only the declaration in the R2 test (or watch and assert each
inode separately), and fix the other two paired watches. Either make the
read-count test observe ordering, for example by asserting two module reads have
happened by the time `open_bound` runs, or give it a name that claims only the
count. In the round record, state that the pre-capture reverification is masked
by the native receipt verification, so that only the count pins it.

## Closure of the B3-B findings

| Finding | Status | Evidence |
| --- | --- | --- |
| X06B3B-R1 probes and red-first | **Open** (X06B3BC01-R1) | Pre-repair raw logs only; no red-first record |
| X06B3B-R2 pre-capture reverification | **Partly closed** (X06B3BC01-R3) | Removal killed only by the read-count test (`4 == 6`); ordering unprotected; behavioural test masked by native receipts; registry-only watch |
| X06B3B-R3 no counter reset | **Open** (X06B3BC01-R2) | `reset-e-instr`, `reset-e-bytes`, `reset-a-instr` and `reset-f-bytes` killed; `reset-f-instr` survives |
| X06B3B-R4 release of nested temporaries | **Closed** | See below |

**R4 detail.** `frame_cleanup` is declared after `impl`, so it runs before the
VM is destroyed on every exit, including exceptions. It runs explicitly after
staging on success. It resets `preserving_aliases`, `push_memo` and the input,
clears the stack, and calls `release_shape_temporaries`, matching the extension
path. The baseline is taken before phase 8, through a protected call.

The success test asserts that tag and backing counts equal the pre-invocation
counts, that the baseline holds no remaining temporaries, and that the peak was
above zero. Probe `release-success` fails it with real membership:
`1095 == 42` tags, `907 == 262` backings, and `1698` leftover temporaries.
`release-failure` fails both "failure paths release nested temporaries" and the
kept "cumulative instruction exhaustion preserves the first error and cleans
nested roots", which now asserts `cleanup_ran`, `peak > 0` and `after == 0`. That
name is now truthful. `release-both` fails all three.

Two smaller caveats on R4:

- On failure the VM is destroyed with the consumed owner. The only observer is
  the cleanup report, not the VM's membership. "Exact membership" is
  count-equality plus a subset-of-baseline check, not element identity. I accept
  this.
- `before.temporary_entries == 0` is constant, since no baseline exists before
  `prepare_load`. The `baseline` mutant is killed only through the
  observation-only `baseline_before_route` flag. The repair note says a
  zero-custom-job variant was added to the success test to pin this
  behaviourally. That test iterates only over 2 and 1 custom jobs, so no such
  variant is present.

## Acceptance criteria

| # | Criterion | Finding |
| --- | --- | --- |
| 1 | R2, R3, R4 tests exist, can fail, each with red-first and a killing mutant | **Not met.** R4 met. R2 is killed only by count (R3 above). R3 survives at one boundary (R2 above). No red-first records (R1 above). |
| 2 | All 977 names preserved, new names listed | **Met.** 982 registered, all unique. 0 removed against the 977-name B3-B inventory. The 5 new names are byte-equal to the coder's list. |
| 3 | Diagnostics byte-identical; no `assert()`; typed `std::expected`; docs; no new public stage surface | **Met.** The diff adds no message literal: the only new strings are field names and the `"not_captured"` status value. No `assert(`. Lint and Doxygen pass over 136 sources. `profile_load_stage` still exposes only `observe()` and moves, and the static asserts hold. New fields are observation-only (see advisory N-c). |
| 4 | Probe evidence for all six invariants with raw logs, named failing tests and equal restored hashes | **Not met by the coder** (R1 above). My reviewer probes give raw logs for all six. Single-frame/no-reset is only partial, because `reset-f-instr` survives. |
| 5 | B3-B criteria 1, 2, 4, 5, 7 remain met | **Met.** The tests pass. `everyjob` kills 10, `consumed` kills 3, `n3` kills 3. No ledger, handler or CMake change, and the `runtime.cppm` imports are unchanged. |
| 6 | Gate | **Met** (see below). |

## Advisory notes (non-blocking)

- **X06B3BC01-N-a (N5 mapping).** The final `catch (...)` maps to
  `external_resource`, but the brief asked for protocol. The choice matches
  `begin_frame`, `into_extension` and the other `catch (...)` arms in the file.
  Record that decision, with its reason, in the round record. No probe or test can
  reach this branch.
- **X06B3BC01-N-b (N1).** The charge now comes before `make_unique` in both
  factories. Removing it from either one is caught by the cross-factory
  comparison. Removing it from both survives (`probe-n1-both.log`), so the charge
  test still cannot detect the charge disappearing everywhere. The record calls
  this "partial", which is fair. A same-module `sizeof(implementation)` check
  would close it.
- **X06B3BC01-N-c (observation fields in product types).** The mark arrays,
  `baseline_before_route`, the cleanup report and the registry counts are now
  fields of `profile_load_stage_observation`, `profile_invocation_observation` and
  `profile_preparation_error`. They are documented as observation-only and add no
  method or authority. But they are recorded in every product run, and B3-C should
  not grow them further.
- **X06B3BC01-N-d (N2).** "begin_frame is once only and blocks adoption" now
  exercises the private frame through `prepare_load`, and is killed by `consumed`
  and `everyjob`. "into_extension adopts the same VM and receipts without
  recapture" uses opaque owners. Its receipt evidence is
  `extension->verify()` after adoption, and its zero-read check watches only the
  registry (R3 above). Both are acceptable once the watch is fixed.
- **X06B3BC01-N-e (record hygiene: N4 and N6).** The reasons for N4's unreachable
  branches are only in `raw/coder-REPORT.md.orig`, which is gitignored and will not
  be committed. Move them into the record. The record's "C01 verification results"
  are stale: they say 981 tests, 4 new, "three required probes survive" and "a
  fresh coder round must strengthen those tests". They also do not describe the
  repair (the read-count test, `baseline_before_route`, the marks), its in-place
  probing, or the resulting 982.
- **X06B3BC01-N-f (unchanged shared path).** `execute_shape` was refactored to
  call the new `ensure_shape_baseline`, with identical behaviour. The extension
  path's tests are still green in the full run.

## Independent gate (macOS Debug, `build/rewrite-debug`)

| # | Command | Result | Log |
| --- | --- | --- | --- |
| 1 | `cmake --build build/rewrite-debug --target planar_tests` | exit 0, `ninja: no work to do.` (already built by the gate runner) | `gate/01-build-planar_tests.log` |
| 2 | `ctest --test-dir build/rewrite-debug --no-tests=error -j 4 --timeout 300` | exit 0, `100% tests passed out of 982`; no failed, skipped, disabled or not-run entries (grep hits only test names) | `gate/02-ctest.log` |
| 3 | `ctest -N` against the B3-B review inventory | 982 names, 977 kept, 0 removed, 5 new | `gate/03-ctest-N.log`, `inventory.txt`, `removed-names.txt`, `new-names.txt` |
| 4 | `python3 scripts/rewrite-check.py lint --build-dir build/rewrite-debug` | exit 0, "Formatting and Doxygen checked 136 active source files" | `gate/04-lint.log` |
| 5 | `cmake --build build/rewrite-debug`, run twice | both exit 0, `ninja: no work to do.`; the second ran zero steps | `gate/05-build-all.log`, `gate/06-build-nochange.log` |

Each exit code was written directly to its own `gate/NN.exit` file, with no
pipe. The diff hash was equal before and after (`gate/00-diff-start.sha256`,
`gate/07-diff-end.sha256`). I ran ctest at `-j 4` with a 300 s per-test timeout.
The gate runner's serial run (`raw/2-ctest.log`, 982/982) is the default-timeout
evidence.

## Reviewer mutation probes

Each probe ran in the scratch copy (`git archive HEAD` plus the six candidate
files; `cmp`-equal to the checkout before and after, as logged in `probes/driver.log`).
`probes/probe.py` applies each exact edit only when it occurs exactly once,
touches the file, and rebuilds `planar_runtime_tests` (plus
`planar_capture_native_tests` for `samesel`). It then runs
`ctest -R 'X06 B3' --no-tests=error -j 4` under `perl -e 'alarm 1200'`, restores
from backup, touches again and compares sha256. The baseline matched 34 tests
and all passed. Every probe built with exit 0 and logged `restored equal
original … True`. The final rebuilds (`probe-final.log`, `probe-final2.log`) pass
34/34 and the native test 1/1. Mutation specs are in `probes/mutations/*.json`.

| Probe | Mutation (runtime.cpp unless noted) | Mutant sha256 (12) | Result |
| --- | --- | --- | --- |
| `reverify-pre` | remove the verify before `open_bound` | `62086754001d` | **killed** only by "captured profile content is reverified …" (`4 == 6`) |
| `reverify-moved` | remove that verify and add one after source capture | `52b29026261a` | **survives** (R3 above) |
| `watch-decl-only` | test only: R2 test watches the declaration alone | t.cpp `c7ab3f643846` | passes (the claim holds) |
| `watch-decl-reverify-pre` | both of the above | as above | R2 test still passes (masked); killed only by the count test |
| `control-declaration-reads` | test only: watch the declaration on success, expect a sentinel | t.cpp `c8de3e2a7e0b` | fails `10 == 999999`: declaration reads are counted |
| `reset-e-instr` | `s.instructions = 0;` after `mark(1)` | `8d241449497f` | **killed**: cumulative test, `9000 <= 0` |
| `reset-e-bytes` | `s.native_bytes = 0;` after `mark(1)` | `692c3165df60` | **killed**: cumulative test; "one frame cumulative counters never reset" |
| `reset-a-instr` | `s.instructions = 0;` before the validation input | `85b4f5383237` | **killed**: cumulative test |
| `reset-f-instr` | `s.instructions = 0;` after `mark(2)`, before validation | `d9b54d0579c3` | **survives** (R2 above) |
| `reset-f-bytes` | `s.native_bytes = 0;` after `mark(2)` | `cad9bc2eb5ce` | **killed**: cumulative test; counters-never-reset |
| `release-success` | `cleanup.run()` becomes `cleanup.done = true` | `dfa8ec077558` | **killed**: "success releases nested temporaries …" (tags `1095 == 42`) |
| `release-failure` | empty `~frame_cleanup()` | `b13512efc6c7` | **killed**: "failure paths release nested temporaries"; "… cleans nested roots" |
| `release-both` | both of the above | `55172c300d4b` | **killed**: all three |
| `n1-both` | drop the charge in both factories | `9b2c43a0e71d` | **survives** (advisory N-b) |
| `everyjob` | skip F for request index > 1 in profile frames | `332d4145ede0` | **killed**: 10 tests including "every job is validated exactly once" |
| `consumed` | put the owner back on intent mismatch | `539aa2f2a2a0` | **killed**: 3 tests including "final intent must equal the bound intent" |
| `n3` | drop `\|\| _owner->known_load` | `a72dda1d37a2` | **killed**: 3 tests including "known-load owner refuses into_extension …" |
| `samesel` | native.cpp `same_selection` refusal becomes `if (false)` | native `0a828b2d1703` | survives `X06 B3` (N3, by design); **killed** by "D2 B2 C01 first retained reads require original root selection" |

## Limits

- macOS Debug only. No RelWithDebInfo, Linux, RSS or whole-invocation budget
  claim, matching the round's limits.
- I did not re-run the prober's `sf-invoked`, `sf-guard`, `single-frame` or
  `reset-c-diag` probes. Their equivalence claims are plausible from source
  (`begin_invocation` zeroes the counter once, and `c.invoked` blocks re-entry),
  but they rest on excerpts only.
- The R2 analysis of `reset-f-instr` uses the coder's observed marks (about
  9k for route and about 122k for validation). I did not build the route-heavy
  fixture myself.
- I did not try to reach the `catch (...)` arm. No reachable trigger exists.
