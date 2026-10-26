# V5-02-X06-B3-C-C02 independent review (comment accuracy in runtime.t.cpp)

Disposition: **approve**. C01B3C-R1 is closed. The three comments in `src/engine/runtime/runtime.t.cpp` now say what
the C01 probes measured. Code apart from comments is byte-identical to the C01 candidate. The other ten source files
are unchanged. My own run found 1003 test names, 1003 of 1003 passing with zero skips, lint exit 0, and no work in a
second build. The record items N2 and N3 are present, and N1 and N4 are carried. Three records-only advisories follow,
for the coordinator to apply before committing. None of them blocks approval.

I reviewed in the shared checkout on `dev/planar-v2-vernacular` at HEAD `2af5f742498181a0f390c97f71db708bbe4b1fb3`,
against the uncommitted candidate. I wrote only this file and new files under
`tests/native/v5-02-x06-b3-c-c02-evidence/review/`. I applied no mutants. I edited no source, test, record or existing
evidence. I ran the builds, lint, `ctest -N` and one full Debug ctest one at a time, with no other ctest or ninja
process running. To reconstruct the C01 diff I used a scratch copy of `src` and a copied git index, with
`GIT_OPTIONAL_LOCKS=0`, so the checkout's index was not touched. I did not commit, push, merge, install or publish. I
used no old Planar tool, queue, skill, role or installed binary.

## Exact candidate

- `git diff HEAD -- src | shasum -a 256` is `d7dcc96c6e2552f7e60d62979e859d0a3cb35f212bd8f62909473238a67a9f09`. I
  checked it before and after my ctest run, and it equals the record and the gate's `raw/diff-src.sha`.
- `src/engine/runtime/runtime.t.cpp` hashes to `ec153c6953e8add5133e6df59841feeaea8546a11aa58e41e99c595a80674273`,
  as recorded.
- The ten other source files hash to the values in the gate's `raw/ref10.txt` and the B3-C table
  (`review/00-identity.txt`).
- The unrestricted `git diff HEAD` hashes to `74fdf145969065ebb39b1eb22c818811a583869b9315620a6ec96aea0899c762`. See
  C02-N1 below.

### C01 baseline: proof that the snapshot is the C01 file

The gate took its "pre-edit" snapshot after the edit was already applied. It recovered the C01 bytes from the prober's
scratch copy (`raw/runtime.t.cpp.c01-snapshot`, sha256 `8fedecbc...`). A hash recorded by the same lineage does not
prove much on its own, so I checked the snapshot a second way. I put the snapshot in place of `runtime.t.cpp` in a
scratch copy of `src`, next to the current ten other files, and ran `git diff HEAD -- src` against that work tree with a
copied index. The result hashes to `ea5f45376e9cd9473a57060f99a1b7adc41c38ddb165265693ba30a6c9c5daae`, which is the
value the C01 review confirmed and the C02 dispatch recorded.

This one reconstruction proves three things at once:

- the snapshot is exactly the C01 `runtime.t.cpp`;
- the other ten files are unchanged since C01;
- the whole C02 source change is the snapshot-to-current difference in `runtime.t.cpp`.

I ran the same reconstruction with the two modified tracked docs added (`roadmap.md`, `V5-02-X06-B3.md`). It gives the
unrestricted dispatch value `810b52f55f5ceac406fe9478fbd62e60dd030d812d6147780270a8299492b29c`. So since dispatch, the
only change to a tracked file is the comment edit in `runtime.t.cpp`.

### Comment-only proof

- The plain diff from the snapshot to the current file (`review/08-runtime.t.cpp-c01-to-c02.diff`, byte-identical to
  the gate's `raw/runtime.t.cpp.c01-to-c02.diff`) has three hunks: 2705-2707, 2760-2761 and 2772-2775. Every removed
  and added line starts with optional whitespace and then `//`. The file has no line ending in a backslash, so no
  comment continues onto a code line. I also read the code around each hunk: all three sit in ordinary function
  bodies, not inside a raw string.
- I reran the gate's token-level stripper (`raw/strip_comments.py`, with `python3 -I`) on both files. `cmp` exits 0,
  2897 lines each. My stripped output of the snapshot equals the gate's `raw/snap.stripped`.
- So assertions, constants, helpers, the `alloc_probe` hook, `run_with_hook` and all test names are byte-identical.
  No diagnostic message can have changed, because no other source file changed and the `runtime.t.cpp` change is
  comments only.

## C01B3C-R1: the three comments, checked against the C01 probes

I compared each new comment with `tests/native/v5-02-x06-b3-c-c01-evidence/probes/PROBES.md` (findings 1 and 2, and the
rows for P01, P02, P03, PB1, PB2, IP-P01, IP-P01full, IP-PB1src, IP-P03, IP-PB1, IP-PB2 and IP-hook-nomtime).

1. **In-place test, opening comment (2772-2775).** It says the inode and size are kept and the mtime is restored, but
   the captures compare content, so the restore is not load-bearing. That matches IP-hook-nomtime, which survives. It
   says both reverifications refuse the edit, so a single skipped source reverification is masked by the profile
   capture reverification. That matches IP-P01 and IP-P01full surviving, and Finding 1's trace. It says the test was
   measured to guard a skip of both reverifications and the ignoring of their results. That matches IP-P03, IP-PB1 and
   IP-PB2, which are killed by this test. "Their results" is plural, which fits IP-PB1src (only the source result
   ignored) surviving. The old claims are gone: that the profile capture cannot see the edit, that the mtime matters,
   and that one skip widens the window.
2. **Rename sweep, "Closure members" comment (2705-2707).** It now says closure members are checked again by both the
   final source reverification and the profile capture reverification. The word "only" is gone. This is consistent
   with P01 surviving the sweep, where refusals still carry `failure_field == "profile"`.
3. **Rename sweep, window comment (2760-2761).** It now says the window widens to the whole derivation only when both
   reverifications are skipped, and that a single skipped one is masked by the other. The "by the other" direction
   needs a measurement too, and it has one. P02 (profile reverification skipped alone) is killed only by the B3-B
   call-count test, not by either sweep. So a single skip of either kind is masked as far as these sweeps can see.

Acceptance criterion 1 is met. No comment claims the mtime matters, and none claims a single skipped reverification
widens the window.

## Gate checks I ran (exit codes in files, nothing piped)

| Check | Result | Evidence |
|---|---|---|
| `cmake --build build/rewrite-debug --target planar_tests` | exit 0, "ninja: no work to do." | `review/01-build-tests.*` |
| `cmake --build build/rewrite-debug` (second build after the first) | exit 0, "ninja: no work to do." | `review/02-build-all.*` |
| `ninja -C build/rewrite-debug -n -d explain` | Only the always-dirty glob re-check (`VerifyGlobs`, `cmake.verify_globs`) is listed, as usual under `-n`. No compile or link edge. | `review/03-ninja-dry.*` |
| `python3 scripts/rewrite-check.py lint --build-dir build/rewrite-debug` | exit 0. Pinned `clang-format --dry-run --Werror` clean; "Formatting and Doxygen checked 136 active source files". | `review/04-lint.*` |
| `ctest --test-dir build/rewrite-debug -N` | 1003 tests. 1003 unique names, identical to the C01 inventory and to the gate's `inventory.txt` (no added, removed or renamed names). | `review/05-ctest-N.*`, `review/07-names.txt` |
| `ctest --test-dir build/rewrite-debug --no-tests=error` | exit 0. "100% tests passed out of 1003". Zero `Failed`, `Skipped` or `Not Run` lines. 357.86 s. #531 passed in 15.31 s, #533 (in-place test) in 10.24 s. | `review/06-ctest-full.*` |

Every check agrees with the gate report. As the gate noted, the builds compiled nothing in this round, because the
repair coder had already built the edited file. The no-work result is still the required property.

## Invariant table

The C02 record has no mutant-based invariant table. It is a repair-kind corrective round with no mapping step, and
both the C01 review and the C02 dispatch said a comment-only change needs no new mutant run. Each acceptance criterion
has a row in the record's acceptance table with a mechanical check:

- comment text: the diff;
- code identity: the stripped comparison and the hashes;
- names, failures and skips: ctest;
- lint and no-work build.

I verified each row directly above. A test mutant cannot express "only comments changed", so a mutant row here would
be hollow. The C01 probe results apply unchanged, because the code tokens are identical, and I do not count the missing
table as a finding. There is no red-first requirement, because no behavior or assertion changed.

## Record items

- **N2: applied, wording incomplete (see C02-N1).** The C01 record's review outcome now says `7ce7cdee...` was still
  current at the C01 review, and adds a C02 note on why it stopped being current. The sentence N2 targeted, in the C01
  identity section ("the gate recorded `33cff643...` and the prober `7ce7cdee...` at their own times; neither is
  current"), was left as it was. Read today it happens to be true, but it still says nothing of the review-time fact.
- **N3: present.** The C01 record's review outcome has "C01B3C-N3 (amendment to Owned files)". It names the
  `alloc_probe` extension, the `run_with_hook` signature change and the added test, all within `runtime.t.cpp`. The
  amendment sits in the review outcome rather than next to the Owned files clause. That is acceptable, because it is
  labelled as an amendment to that clause.
- **N1 and N4: carried** in the C02 record's "Remaining limits" and in the parent record's "Final candidate after C02":
  - phases not measured;
  - macOS Debug only;
  - P01 survives, masked;
  - R1-stride-only cannot be killed;
  - the N2 message "Captured inputs changed." is open;
  - the vector follow-up;
  - #531 at about 15.4 s against a 30 s timeout.
- **Owned files.** The record's "Files edited in this round" matches what I observed: `runtime.t.cpp` (comments), the
  three records and the C02 evidence directory. The tracked `roadmap.md` and `V5-02-X06-B3.md` did not change after
  dispatch (shown by the reconstruction above).

## Findings

No required findings.

### C02-N1 (advisory, records-only): the C02 record calls a current hash "stale by design"

The C02 record's "Final candidate identity" says the gate saw unrestricted `git diff HEAD` = `74fdf145...` "before this
sync's record edits; that value is stale by design". It is not stale. The sync edited only untracked records
(`V5-02-X06-B3-C.md`, `V5-02-X06-B3-C-C01.md`, `V5-02-X06-B3-C-C02.md`), and an unrestricted `git diff HEAD` leaves
untracked files out. The value is current at this review. It is the same kind of error C01B3C-N2 corrected.

Fix: say that `74fdf145...` was still current at the C02 review. Also say that the change from `810b52f5...` at
dispatch is due only to the `runtime.t.cpp` comment edit. In the same pass, reword the C01 identity sentence that N2
targeted, or point it at the review outcome correction.

### C02-N2 (advisory, records-only): this round's raw logs hold absolute local paths

The gate's `raw/ctest-N.log`, `raw/ctest.log` and `raw/lint.log` contain tool-printed absolute paths, as the gate
reported. So do my `review/03-ninja-dry.log`, `review/04-lint.log` and possibly `review/05-ctest-N.log` and
`review/06-ctest-full.log`. The commit step's `redact` then `check` on the staged files must cover both directories.
Redaction rewrites path text only, and no raw log may be deleted.

### C02-N3 (advisory): gate snapshot order

The gate's step 1 was out of order: the snapshot was taken after the edit. This does not weaken the result, because
the reconstruction above independently reproduces the C01 `src` identity from the snapshot. A future gate should
still snapshot before the edit, or record the recovery path the way this gate did.

## Limits

- macOS Debug only. The builds compiled nothing, because the edited file had already been built by the repair coder.
- I did not rerun any mutant. The comment claims are checked against the recorded C01 probe logs, which apply because
  the code tokens are identical.
- The comment-only proof relies on a purpose-written stripper, which I reran, plus a line-level check that every
  changed line is a `//` comment outside any string. No compiler-based token dump was produced.

## On approval

The coordinator may apply C02-N1 to the records. Then, after confirming that `git diff HEAD -- src` still hashes to
`d7dcc96c...`, the coordinator may commit B3-C, C01 and C02 as the dispatch describes:

- explicit paths, including the roadmap Next work edits, the parent `V5-02-X06-B3.md`, all B3-C, C01 and C02 records
  and reviews, and the three evidence directories;
- `redact` then `check` on the staged files;
- `git commit -s`, with no Claude co-author trailer;
- subject "V5-02-X06-B3-C+C01+C02: native v2 derivation and prepared_profile".

Pushing, merging and installing remain unauthorized.
