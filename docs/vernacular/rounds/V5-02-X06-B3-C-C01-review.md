# V5-02-X06-B3-C-C01 independent review (sweep stride restoration and record corrections)

Disposition: **request-changes**. R1 and R2 of the B3-C review are closed. The whole-call stride is back to
`total / 40`, the dense tail stays at step 40, the test name is unchanged, and the measurements the review asked for
are recorded and match the raw prober logs. The two hashes are now named by pathspec. N1 to N4 are recorded. No
product source changed after B3-C, no diagnostic text changed, and all 1002 earlier test names are still present. My
own ctest and gate checks agree with the gate report.

One required item is new in this round. The survivor-repair test added in C01, "X06 B3 C an in place same size edit at
any allocation inside prepare_load is never published as verified", opens with a comment that this round's own prober
measurement disproves (C01B3C-R1). The rename sweep, whose body R1 owns, carries two comments with the same false
premise. The fix is a comment-only edit to `runtime.t.cpp`, so it cannot be done in records alone.

I reviewed in the shared checkout on `dev/planar-v2-vernacular` at HEAD `2af5f742498181a0f390c97f71db708bbe4b1fb3`,
against the uncommitted candidate. I wrote only this file and new files under
`tests/native/v5-02-x06-b3-c-c01-evidence/review/`. I applied no mutants, edited no source, test, record or existing
evidence, and ran no compiling build. I ran `ctest -N`, tests #531 and #533 through ctest, the x06b3 runtime cases
directly, the pinned `clang-format --dry-run --Werror` on `runtime.t.cpp`, one ninja dry run, and one full Debug
ctest, one at a time with no other ctest or ninja process running. To get the B3-C bytes of `runtime.t.cpp`, I only
read and copied the B3-C prober's scratch copy. I did not commit, push, merge, install or publish, and I used no old
Planar tool, queue, skill, role or installed binary.

## Exact candidate

- `git diff HEAD -- src` sha256 is `ea5f45376e9cd9473a57060f99a1b7adc41c38ddb165265693ba30a6c9c5daae`, and
  `git diff 7110196d -- src` hashes to the same value (`review/00-diff-src.sha256`,
  `review/00-diff-src-vs-7110196d.sha256`). This equals the gate's `raw/hash-src-r2.txt`, the prober's
  `probes/raw/candidate-identity.txt` and the round record.
- Per-file sha256 (`review/00-files.sha256`): ten of the eleven files match the B3-C candidate table in
  `V5-02-X06-B3-C.md` exactly. `src/engine/runtime/runtime.t.cpp` is
  `8fedecbc2874f3280dbac17c8c6609a8e0b4e668fb5b9ec7f4b621834416433e`, as recorded. So the only source change since
  the B3-C dispatch hash `78b1deb4…` is in `runtime.t.cpp`.
- What changed in `runtime.t.cpp`: I took the B3-C file from the B3-C prober scratch copy
  (`scratch3/repo/src/engine/runtime/runtime.t.cpp`). It hashes to `c61c53c5…`, the value in the B3-C table
  (`review/07-b3c-runtime.t.cpp.sha256`). Its diff against the current file (`review/07-runtime.t.cpp-b3c-to-final.diff`)
  has nine removed lines and contains only:
  - the sweep comment and stride (`total / 24` to `total / 40`);
  - the `<fcntl.h>` include;
  - the in-place edit fields of `alloc_probe` and their branch in `operator new`;
  - the defaulted `arm_edit` parameter of `run_with_hook`;
  - the new test.

  I also rebuilt the intermediate post-P1 file by applying `raw/post-c01-src.diff` to `7110196d`. It hashes to the
  recorded `75a03994…` and differs from the B3-C file only in the stride and its comment.
- The executable `build/rewrite-debug/src/engine/planar_runtime_tests` (17:41:19) is newer than `runtime.t.cpp`
  (17:41:11) and `runtime.cpp` (11:10:59) (`review/02-mtimes.txt`), so the gate and my runs used this candidate.
- The unrestricted `git diff HEAD` currently hashes to `7ce7cdee…` (`review/00-diff-all.sha256`). The coordinator's
  sync touched only untracked records, so this is still the prober's value (see N2 below).

## Independent verification

| Check | Result (raw file under `tests/native/v5-02-x06-b3-c-c01-evidence/`) |
| --- | --- |
| Gate raw exits | `raw/build-r2`, `ctest-r2`, `ctest-N-r2`, `lint-r2`, `all1-r2` and `all2-r2` exit files all read 0. `ctest-r2.log` reads "100% tests passed out of 1003", with 1003 `Passed` lines and a total of 357.59 s. #531 took 15.40 s and #533 took 10.27 s. "skipped" appears only inside test names (#560, #975). Both `all*-r2.log` files read "ninja: no work to do". The lint log shows the Doxygen SIGBUS on attempt 1, success on attempt 2 through the script's retry, and 136 files checked. |
| Full Debug ctest (mine) | `review/08-ctest-full.log`: exit 0, "100% tests passed out of 1003", 1003 `Passed` lines, no failed, skipped or not-run tests, total 358.98 s. #531 took 15.40 s and #533 took 10.29 s. |
| Inventory | `ctest -N` exits 0 with "Total Tests: 1003" (`review/01-ctest-N.*`). As sets against the B3-C gate inventory (`v5-02-x06-b3-c-evidence/raw/g3/inventory-now.txt`, 1002 names), 0 are removed and 1 is new: the in-place test (`review/01-removed.txt`, `review/01-new.txt`). |
| Test #531 | Passed in **15.45 s** through ctest (`review/04-ctest-531.*`), well under the 30 s timeout. #533 passed in 10.26 s (`review/05-ctest-533.*`). |
| Focused run | `planar_runtime_tests '[x06b3a],[x06b3b],[x06b3c]'`: 11400 assertions in 52 test cases, exit 0 (`review/06-runtime-x06b3.*`). This equals the prober's scratch baseline. |
| Formatting | The pinned `clang-format --dry-run --Werror src/engine/runtime/runtime.t.cpp` exits 0 with no output (`review/03-clang-format.*`). |
| No-change build | My `ninja -n` prints "Re-running CMake" (`review/02-ninja-dry.log`). This is expected under `-n` with globbed `CONFIGURE_DEPENDS` and proves nothing either way. The gate's two "no work to do" builds stand. As the record says, they followed no compiling build. |
| Diagnostics | No product file changed (per-file hashes above). In the test diff, every string literal is either a test name, tag, fixture path or label, or the existing `"capture_changed"` / `"profile"` expectations (`review/07-…diff`). No message changed. |
| Measurements (R1) | `probes/raw/MEAS-sweep.full` prints, per target, `total=18863 stride=471 positions=58`, and `sampled=121` for the two closure members. Refused/fresh/late/never-fired are 56/0/1/1 and 116/3/1/1, with `earliest_late=18863`. The log restores `runtime.t.cpp` to `8fedecbc…` with `RESTORE_VERIFIED True`. 41 whole-call positions (1 + 471k < 18863) plus 16 tail positions plus 1 past the end make 58, and the dense tail (16 to 2499, step 40) adds 63 for 121. All of this matches the record's table. |
| Probe spot checks | I read the raw logs for P03s, PB1s, V3bs, SW-window, IP-hook-norename, IP-P01 and MEAS-inplace-P01. Each mutant diff is the one described. P03s, PB1s, V3bs, SW-window and IP-hook-norename each fail the sweep test, which is named as the single killer (exit 42, with `N failed` counts). IP-P01 passes. Every log reads `RESTORE_VERIFIED True`. The prober's scratch copy is `cmp`-identical to the checkout for all eleven files. |

## Prior findings

- **X06B3C-R1: closed.** `runtime.t.cpp:2668` reads `n += total / 40`. The dense tail is `back += 40`, and the test
  name and assertions are unchanged. The unmeasured "crosses every phase" wording is gone. The record states the total
  allocations (18863), the positions per target kind, the #531 wall time (15.40 s in the gate, 15.45 s in my run) and
  the probe sensitivity rows. Phases are not instrumented, and the record says so plainly instead of making a claim.
  The review's main concern was coverage lost by thinning the stride. Restoring the pre-repair density answers that,
  and since no phase claim is made, I accept this limit (see N1 below). The prober confirms that the sweep still fails
  under receipt-skip (P03s, V3bs) and publish-before-verify (PB1s) mutants in its scratch copy. R1-stride-only shows
  that no product mutant's verdict depends on the stride, and the record states that honestly.
- **X06B3C-R2: closed.** The "Candidate identity" section of `V5-02-X06-B3-C.md` names `git diff HEAD -- src` for
  `78b1deb4…`, explains the unrestricted hash `cddc3c24…`, and the new "Final candidate after C01" section gives
  `ea5f4537…` with per-file hashes.
- **N1: recorded.** The codec item cites X-REG ("Shipped IDs `pipeline` and `fanout_review` are reserved…") and X-STORE
  ("…except registered job.shape IDs permitted"). I confirmed both clauses at lines 267 and 607 of
  `external-profile-contract.md`. The vector follow-up is left open.
- **N2: recorded** as "OPEN ITEM (N2)" in the C01 record. The message at `runtime.cpp:5101` is byte-identical to
  `acceptance/prepared.cpp:112` and was not changed.
- **N3: recorded and, with reason, amended.** The dispatched wording "not reachable with the rename-only allocator
  hook" was written into the B3-C `probes/PROBES.md` and the B3-C record. C01 then added an in-place hook, and P01
  still survives. I checked the measured reason against `MEAS-inplace-P01.full`. Under P01, every in-place edit of the
  text reference at allocations 18841 to 18855 is refused. Those edits fall after the point where the elided source
  reverification would have run, so only the profile capture reverification remains to refuse them. That supports
  "masked by the profile capture reverification". I accept the amendment.
- **N4: recorded,** and confirmed by the prober: no receipt-skip mutant lists the replacement-before-publication test.

## Findings

### C01B3C-R1 (required): the new in-place test, and the sweep it extends, describe a guard that this round measured not to exist

The new test opens with this comment (`runtime.t.cpp`, first lines of the test body):

> Unlike the rename sweep, the edit keeps the inode, the size and the modification time, so the profile capture
> cannot see it: only the source capture's final reverification stands between the edit and publication. A skipped
> source reverification widens the stale publication window from the trailing allocations to the whole derivation.

The prober's measurements in this round contradict each part of that comment:

- IP-P01 skips exactly that source reverification, and the test passes with zero failures. The window does not widen.
  Under P01 the only late publication is still the final allocation (`MEAS-inplace-P01.full`).
- The profile capture does see the edit. Under P01, edits made after the elided call are refused as `capture_changed`.
  The record itself now classifies P01 as "masked by the profile capture reverification".
- The modification-time restore is not load-bearing. IP-hook-nomtime passes, because the captures compare content.

The rename sweep, whose body R1 owns, carries the same premise twice:

- "Closure members are checked again by the final source reverification only." P01 survives the sweep with refusals.
- "A skipped final reverification would widen this to the whole derivation." This holds only when both
  reverifications are skipped (P03).

The round record states the truth, but these comments live in the test file and will outlast the record. They are the
only description of what the two tests guard. A reader will take the new test as the P01 guard, which is exactly the
gap this round showed it does not close. The B3-C review asked for an unmeasured comment claim to be dropped from this
same test. This new claim is stronger, and it is contradicted by a measurement, not merely unmeasured.

Required:

1. Reword the opening comment of "X06 B3 C an in place same size edit at any allocation inside prepare_load is never
   published as verified". It should say what the test was measured to guard: a skip of both pre-publication
   reverifications (P03), and ignoring their results (PB1, PB2). It should state that a single skipped source
   reverification is masked by the profile capture reverification, and it should not claim that the mtime restore
   matters.
2. Correct the two sweep comments the same way: closure members are also rechecked by the profile capture
   reverification, and the window widens only when both reverifications are skipped.
3. Keep the edit to comments only. Assertions, constants and test names stay byte-identical. List the edit in the
   round record, and give the new `runtime.t.cpp` and `git diff HEAD -- src` hashes.
4. Gate: `clang-format`, lint, the `planar_tests` build, a full Debug ctest with 1003 names, zero failures and zero
   skips, and a second build with no work. A comment-only change needs no new mutant run. The prober (or the gate)
   should still confirm that the token stream is unchanged, for example by diffing the file with comments stripped.

## Advisories (not blocking)

- **N1: phases are still not measured.** The dispatch's acceptance criterion 2 asks which phases are covered, and the
  record marks it "Met except phases". I accept this because the stride is back to its pre-repair density and the
  comment no longer claims phase coverage. Since C02 runs a gate anyway, the prober could cheaply record the
  allocation count at each phase marker in its scratch copy and close the gap.
- **N2: one stale sentence about the unrestricted hash.** The C01 record says that neither the gate's `33cff643…`
  nor the prober's `7ce7cdee…` is current. `git diff HEAD` hashes to `7ce7cdee…` now, because the coordinator's sync
  edited only untracked files. The record should say that `7ce7cdee…` is current as of the sync, or drop the
  statement.
- **N3: the repair went beyond the dispatched owned-file clause without an amendment on record.** The dispatch limited
  `runtime.t.cpp` edits to "the one test body named above only". The repair coder also extended the shared
  `alloc_probe` hook and the `run_with_hook` signature and added a test. Survivor repair is a normal workflow step
  (`orchestration.md`, step 5), and the record does describe the edits. The owned-file clause should still carry a
  one-line amendment naming them, so the clause matches what was done.
- **N4: limits carried forward.** These limits are recorded and I accept them:
  - macOS Debug only.
  - The gate builds compiled nothing.
  - P01 survives and is masked.
  - R1-stride-only cannot be killed.
  - The vector follow-up from N1.
  - The N2 message is open.
  - #531 timed out once at 30 s in the prober's scratch copy under heavy load. At about 15.4 s, that is a flake risk
    to watch.

## What the corrective round must do

1. Fix C01B3C-R1 with a comment-only edit to `runtime.t.cpp`. Record the new hashes, and gate as listed above.
2. Optionally take up N1 (phase markers), N2 (the hash sentence) and N3 (the owned-file amendment) in the records.

Commit only after re-review approval, with `git commit -s` and no Claude co-author trailer.
