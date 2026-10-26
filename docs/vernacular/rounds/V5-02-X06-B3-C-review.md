# V5-02-X06-B3-C independent review (native v2 derivation, `prepared_profile`, sweep-timeout and probe-gap repair)

Disposition: **request-changes**. The product work is sound, and every gate result I
re-checked matches the raw logs. All 987 baseline names are kept and 15 names are added. I
found no change to any existing diagnostic message. The probe-gap tests do kill the mutants
they were written for, and the known limits are recorded with reasons. One required item is
missing. The sweep-timeout repair made the test fast partly by thinning the original
whole-call injection stride, and the round record does not show which allocation sites or
phases are still covered. The dispatch asked for the opposite on both points (R1). One
statement about the candidate hash in the authoritative record section is also wrong (R2).
Both fixes are small, and neither needs a product change.

I reviewed in the shared checkout on `dev/planar-v2-vernacular` at HEAD
`7110196d87c473439d33c9174a8c90218477a805`, against the uncommitted candidate. I wrote only
this file and new files under `tests/native/v5-02-x06-b3-c-evidence/review/`. I applied no
mutants, edited no source, test, record or existing evidence, and ran no build. My only
`cmake --build` calls were two ninja dry runs (`-n`). I ran `ctest -N`, the formerly
timing-out test through ctest, and four focused test executables directly, one at a time,
with no other ctest or ninja process running. I did not commit, push, merge, install or
publish, and I used no old Planar tool, queue, skill, role or installed binary.

## Exact candidate

- `git diff HEAD -- src` sha256 is
  `78b1deb4ea855d6bbfca6648048c941af59c09db58f68f6b616714249251517f`
  (`review/00-diff-src.sha256`). This equals the gate (`raw/g3/diff-sha.txt`) and the prober's
  run 3.
- All eleven per-file sha256 values in the record's "Candidate identity" table equal the
  working tree (`review/00-files.sha256`).
- `git status --short` (`review/00-status.txt`) lists the eleven source files, the parent
  record `V5-02-X06-B3.md` and the untracked round record. The gate's `raw/g3/status.txt`
  does not yet list `V5-02-X06-B3.md`, because the coordinator edited it at sync time (see R2).
- No product source changed after the survivor repair. `runtime.cpp` and
  `interpretation_v2_codec.cpp` were last modified at 11:10:59. The test files were last
  modified between 12:49 and 13:06, before the 13:07 gate-2b start and the g3 gate.

## Independent verification

| Check | Result (raw file under `tests/native/v5-02-x06-b3-c-evidence/`) |
| --- | --- |
| Gate raw exits | `raw/g3/01-build`, `02-ctest`, `04-lint`, `05a`, `05b` exit files all read 0. `02-ctest.log` reads "100% tests passed out of 1002" with 1002 `Passed` lines. The only "skipped" text is test #559's name. `05b.log` reads "ninja: no work to do". The lint log shows the Doxygen SIGBUS flake on attempt 1 and success on attempt 2 through the script's own retry. |
| Inventory | `ctest -N` exits 0 with "Total Tests: 1002" (`review/02-ctest-N.*`). Compared as sets with `p4-inventory-base-sorted.txt`: 0 removed, 15 new (`review/02-removed.txt`, `review/02-new.txt`). I checked that baseline against the B3-B C04 review's `inventory.sorted.txt` and the C03 `inventory.txt`: all three are the same 987 names. My inventory equals the gate's `inventory.txt` as a set. The line-order difference is locale collation only. |
| Formerly timing-out test | ctest #531 passed in **12.88 s** (`review/03-sweep-ctest.*`, exit 0). The gate measured 13.24 s and the prober 12.6 s. The ctest timeout is unchanged: neither CMake diff touches `TIMEOUT`. |
| Focused executables | `planar_runtime_tests '[x06b3a],[x06b3b],[x06b3c]'`: 7106 assertions in 51 cases. `planar_acceptance_source_capture_tests`: 735 in 19. `planar_history_interpretation_v2_codec_tests`: 49587 in 16. `planar_acceptance_load_tests`: 533 in 7. All exit 0 (`review/04`–`07`), and the counts equal the prober's scratch baseline. |
| Diagnostics | Every message-like string literal on a removed product line is still present on an added line. The private helpers moved from `load.cpp` into `source_capture.cpp` unchanged, and `load.cpp` still maps `mismatch` to its unchanged `invalid_interpretation` refusal (mutant L1 is killed by the existing load test). The only new message-like literal is `"Captured inputs changed."` in `prepared_profile::verify_native_receipts` (`review/09-diagnostic-literals.txt`); see N2. No existing message changed. |
| Dependency direction | The new edge is runtime → `interpretation_v2_codec`. The codec imports only `effective_provenance`, `value`, `effective_catalog`, `hash` and `canonical`. `source_capture` imports `capture`, `capture.host_path` and `canonical`. No new import reaches `runtime`, `ledger.acceptance` or `acceptance.load`, so there is no cycle. `runtime.cpp` does not import `ledger.acceptance`. |
| Public surface | `runtime.cppm` adds `profile_acceptance_data`, which holds product data fields, and the move-only `prepared_profile`, which has a private constructor and offers `data()`, `bound_intent()`, `verify_native_receipts()` and `observe()`. `observe()` is the one recorded amendment. `source_capture.cppm` adds the shared `verify_load_correspondence` and its error type, with Doxygen. The diff adds no new `assert(`. |
| Red-first | `raw/redfirst-prefix-runtime.cpp.snapshot` hashes to `b0af42c7…8c3`, as recorded. Diffing it against the current `runtime.cpp` shows exactly the semantic-model fix. In `probes/raw/R0.log`, 26 of 51 cases fail on the pre-fix file and the file is restored to `a82d1abf…`. |
| Probe spot checks | I read the raw logs for P08b, P10c, P11a, P13c, S16, C2 and T1ciso. Each mutant diff is the one described. Each run names exactly the intended killer (`…published workspace id…`, `…each derivation charge scales exactly…`, `source capture correspondence refuses every single bundle tamper as mismatch`, `v2 custom shipped classification…`, `X06 B3 A into_extension…`), and each log reads `RESTORE_VERIFIED True`. |

## Probe-gap tests (repair item 2)

- **P11a to P11d.** `X06 B3 C each derivation charge scales exactly with the bytes it covers`
  (`runtime.t.cpp:2825`) compares the charge after `native_byte_marks[3]` across three
  same-file-count padding variants. It requires an exact equality with the per-byte
  coefficients of reassembly plus encoder (6 per effective byte), source tree (3) and
  post-encode (4). Each variant moves at least one term, so removing any one charge or
  resetting the counter to the validation mark (P10c) changes the difference. The probe logs
  confirm single-killer kills. This is a real pin, not a lower bound.
- **P08b, P13c.** `…published workspace id follows the declaration and source modes are
  normalised` (`:2859`) uses workspace id `ledgerq`, a 0755 closure file, a 0664 file
  normalised to 0644, and an uncaptured extra file. It also checks that clearing the
  executable bit changes the source digest. Both mutants are killed.
- **S10, S14, S15, S16.** The correspondence tamper list (`source_capture.t.cpp:806`) now
  includes a route table with swapped files (still valid and sorted), reversed routes, root
  id and root base, the workspace spec reference, and an extra registry header field. Each
  case is built so that only the target check can refuse it, and every one of them is
  killed.
- **C2.** The new codec block renames the shipped job (index 1, asserted to have no
  `selected_shape`) to `unknown` and expects `invalid_correspondence`. It kills the "accept
  any shape" mutant.
- **T1 / criterion 5a.** In the kept-name test, the positive control now resets the shared
  read counter through an unread file, then arms the declaration watch inside the test. A
  fresh count above zero is required before the opaque zero assertion. T1c and T1ciso (run
  in isolation) kill it, so the control no longer depends on test order.

## Findings

### X06B3C-R1 (required): the timeout repair thinned the original whole-call injection stride and does not record coverage per site or phase

The dispatch said: if the test is slow, "make it fast without weakening it", "keep failure
injection at every distinct allocation site or category (a smaller fixture or a stride that
still covers each site), and record exactly what is covered". The diagnosis is credible.
About 910 runs at roughly 38 ms each is consistent with the recorded 35 s, so the test was
slow, not hung. (The 35 s standalone figure itself is not in any raw log. The logs show
only the 30.02 s timeouts.)

The repair changed two constants (`runtime.t.cpp:2668` and `:2692`):

- The dense closure tail went from step 8 to step 40. That tail was the survivor repair's
  own addition, aimed at P01, which survives either way. Its cost is what caused the timeout.
- The whole-call stride went from `total / 40` to `total / 24`. This stride was part of the
  test before the survivor repair. In that form the test passed in 10.8 to 11.0 s
  (`p4-ctest.log`, `raw/prev-run-1049/`). The change cuts the stride samples by about 40%
  and widens the largest unsampled gap from about 2.5% to about 4% of the call, so a
  stale-publication window that falls between samples is now easier to miss.

Restoring `/ 40` with the step-40 tail would be about 3 × 57 + 2 × (57 + 62) ≈ 409 runs,
roughly 15 to 16 s at the measured rate. That is still well under 30 s.

The record also does not say which allocation sites or phases the remaining positions reach.
It says only "stride total/24 and tail step 40 (about 330 runs, 14 s standalone)". The test
comment's claim that the stride "crosses every phase" has no supporting measurement (the
total allocation count and the phase boundaries are not recorded). The prober confirmed that
the test still kills P03 and V3b. That shows the assertions still bite, not that every
category is still injected.

Required:

1. Restore the whole-call stride to at least `total / 40`, or show that the coarser stride
   still covers every phase.
2. Record the resulting wall time.
3. Record what is covered: the measured `total` for the fixture, the position set per target
   kind, and which phases receive at least one injection. Phase boundaries can be measured
   by the prober in its scratch copy, without touching the checkout.

The test name and its assertions stay as they are.

### X06B3C-R2 (required, record accuracy): the candidate-hash statement no longer holds for the tree it describes

The "Final candidate" section says the coordinator re-verified at sync time and that
"`git diff HEAD | shasum -a 256` equals the gate's and prober's hash" `78b1deb4…`. The same
sync edited the tracked parent record `docs/vernacular/rounds/V5-02-X06-B3.md`, which was
modified at 15:56:39, after the gate. The full `git diff HEAD` now hashes to
`cddc3c2461a4a715855d3f2dc89c068dacc228f898dd73c658801f4b32a4df31`
(`review/00-diff-all.sha256`). Only `git diff HEAD -- src` gives `78b1deb4…`. The identity
of the source candidate is unaffected, but the authoritative section should name the hashed
pathspec (`-- src`), or add a note that the full-tree hash changed with the parent-record
edit.

## Advisories (not blocking)

- **N1: codec relaxation confirmed against the contract; add the clause and a realistic
  vector.** The record asks the reviewer to confirm. `verify_correspondence` now treats a job
  whose shape is `pipeline` or `fanout_review` and has no catalog entry as a shipped job with
  no selection, and it still refuses unknown shapes. This matches the external-profile
  contract on three points:
  - X-REG: "Shipped IDs `pipeline` and `fanout_review` are reserved; adapter registration
    retains their old behavior."
  - X-STORE: "B is existing complete normalized bundle except registered job.shape IDs
    permitted", and E has one `selected_shapes` entry "for each custom-shape job".
  - The shipped stdlib registers no shape descriptors (`review/08-shipped-shape-sources.txt`),
    so a real catalog never lists the two reserved shapes. Requiring an entry would refuse
    every real profile that has a shipped job. The C1 mutant, which restores the refusal, is
    killed by 8 runtime tests.

  No ledger holds v2 records yet, because the fresh profile acceptance refusal remains, so
  there is no replay compatibility issue. Two follow-ups:
  - Cite these clauses in the record.
  - The reviewed vector `tests/vectors/interpretation-v2/interpretation.json` still carries
    shipped-tier catalog entries for both shapes, which the runtime never produces
    (`review/08-codec-vector-catalog.txt`). The codec accepts both forms because C must
    equal the manifest. A later round should add a vector without shipped entries, or record
    why both forms stay accepted.
- **N2: the moved-from message of `verify_native_receipts` is outside the closed X-LIMIT
  table.** On a moved-from or stage-less object, the method returns `capture_changed` /
  `"Captured inputs changed."`. That message is copied from the embedded
  `prepared_acceptance::verify()` (`acceptance/prepared.cpp:112`), so it is an existing
  adapter message, not new vocabulary. However, the external-profile table says new native
  sites use `Captured external profile changed`, and `data()` on the same moved-from object
  returns the protocol refusal. No CLI path reaches this yet, because ledger completion is
  out of scope. Settle it, or record the operator's choice, before
  `complete_profile_acceptance` consumes the method.
- **N3: the P01 reason should be narrowed.** "Redundant with profile-capture reverify" holds
  for the sweep's rename replacement, which the profile capture evidently detects (P01
  survives with refusals). It is not shown for an in-place, same-size edit of a closure
  member with mtime restored, made between seal and the final source reverification. Under
  P01 that edit would probably be published and caught only later by
  `verify_native_receipts`. The allocator hook can only rename. State the limit as "not
  reachable with the rename-only hook" rather than "redundant".
- **N4: the prober's own finding stands.** `X06 B3 C replacement before publication refuses
  terminally and publishes nothing` kills no receipt-skip mutant. P02, X2, X5 and X5b are
  caught only by the B3-B call-count test, and P03 by the sweep. Criterion 3 is still
  covered, but the B3-C test name says more than it proves.
- **N5: gate limits I could not close.** The g3 gate compiled nothing, so the evidence that
  the second build does no work comes from the earlier builds. I added no compile. My ninja
  dry runs print "Re-running CMake", which is expected under `-n` with globbed
  `CONFIGURE_DEPENDS` and proves nothing either way. No RelWithDebInfo, Linux, RSS or
  whole-invocation budget evidence exists, as the record states.

## Known limits recorded in the round record

The record classifies every listed item with a reason: P01, P04, P05, P06c, P07b, P12b, V5, F1
and L2, plus run-3 P09c, P11e, P11f, P13a, P13b, P14, S1, T1d, X1 and N1a. P10c, which the
dispatch listed as a possible limit, is now killed. N4 (the unreachable branches) and N5
(the final catch) are listed. N3 is cited in every probe log, and N6 has no PENDING entries.
I accept these classifications, with N3 above as a wording correction.

## What the corrective round must do

1. Fix R1: restore the stride, or show phase coverage, then record the coverage and the
   wall time. Gate it with a full Debug ctest, zero failures and skips, 1002 names, and #531
   well under 30 s. The prober re-runs P03, V3b and the R0 replay on the new test text.
2. Fix R2 in the round record.
3. Optionally address N1 to N3 in the record.

Commit only after re-review approval, with `git commit -s` and no Claude co-author trailer.
