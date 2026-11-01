# V5-02-RA01-C-C02 independent review (evidence-only corrective for RA01-C-C01)

Disposition: **request-changes, records only**. The substance of the round is right. The source is byte-identical to the
C01 frozen candidate. `red-first.log` and the C01 `plan/DECISIONS.md` no longer contain the repository path or the home
path. The script did the redaction, and `tests/native/path-redaction.json` gained exactly the two entries that redaction
requires. The corrected check passes one argument per file, prints every path and exits 0. Its positive controls exit 1.
`raw/g2-redact.log` is kept. A1 to A4 are applied correctly. Lint passes.

What blocks the commit is the close procedure the record hands to P5. It was written against the tree as P4 left it.
Three later writes changed that tree: the gate runner rewrote the C02 `REPORT.md` and added the C02 `inventory.txt`, and
the coordinator sync then edited eight more lines of the C01 record. Three of the record's own close guards (D27 step 1 and
I22, D31 and I25, D32 and I26) will therefore stop P5. The staged list also leaves out the new C02 `inventory.txt`. One
invariant row reports a P5 check as passed although it has not run. All of this is fixed by editing the C02 round record;
no source, test or evidence data needs to change (R1 below).

I reviewed the uncommitted C01 plus C02 candidate on HEAD `93053c42` against the [round record](V5-02-RA01-C-C02.md),
the C02 `plan/INVARIANTS.md` and `plan/DECISIONS.md` (revision 4), the gate `REPORT.md`, `raw/coder-REPORT.md`, every
`raw/c02-*` and `raw/gate-*` file in both evidence folders, the [C01 review](V5-02-RA01-C-C01-review.md) whose findings
this round closes, the [C01 record](V5-02-RA01-C-C01.md) and the C01 `REPORT.md`, `probes/PROBES.md` and `red/RED.md`. I
also read `scripts/redact-local-paths.py` in full. I ran no script found under `tests/native/`.

## Exact candidate

Output of the workflow `identity.py`, run from the repository root with pathspec `src`, pasted verbatim (also saved as
`tests/native/v5-02-ra01-c-c02-evidence/review/identity.txt`):

| identity | value |
|---|---|
| HEAD | `93053c42a4c68fd3ecc8911edab6464e3d1307f3` |
| sha256(git diff --binary HEAD -- src), tracked only | `6b23117bfb40136186fabf5515d97e0ac1da99c0b69b6d00584033cab0f4aeda` |
| combined sha256 (diff bytes, then each untracked path NUL bytes, sorted), pathspec src, evidence excluded | `6b23117bfb40136186fabf5515d97e0ac1da99c0b69b6d00584033cab0f4aeda` |

| file | state | sha256 |
|---|---|---|
| `src/cmd/planar/authoring_orchestrator.t.cpp` | modified | `38d79424ab104ac3be416588bcc58d33715976fd781a6128e173b51c6a06a46f` |
| `src/stdlib/planar/orchestrator.lua` | modified | `e7d9f570ef70b014e6b3facac55709c59ea8736958bb5ef7d5147eb227bc87a1` |

All three values equal the frozen values in the round record's Acceptance. `git diff HEAD -- src | shasum -a 256` gives
the same `6b23117b...4aeda`, and `git diff --stat HEAD -- src` is still 273 insertions in the test file and 4 insertions
and 3 deletions in `orchestrator.lua`. `git status` shows no other change under `src/` and no change to any test file. No
src or test byte changed, so the full code gate is not triggered and no rebuild is required. `ctest --test-dir
build/rewrite-debug -N` lists 1439 names, and the sorted list is byte-identical to the C01 `inventory.txt`. No name was
added, removed or renamed. The per-file hashes in the record's "Final candidate and gate" table match the files on disk,
all eleven of them.

## R1 from the C01 review: closed in substance

- **The two files are clean.** `grep -cF` of the repository path and of the home path is 0 in `red-first.log` and in the
  C01 `plan/DECISIONS.md`. It is also 0 in every other file of the check list, and no other local-path form (`/Users/`,
  `/private/`) occurs in any of them.
- **The script did the redaction, and nothing touched the files afterwards.** The on-disk sha256 values are `7c69cb0a...`
  and `3cf30cd5...`, equal to `redacted_sha256` in the manifest. I rebuilt both originals myself by substituting the paths
  back into the tokens: the rebuilt hashes are `f138a4b9...` and `aa8e7d9c...`. They equal `original_sha256` and the
  pre-redaction hashes that P0 recorded in `raw/c02-redact-pre.txt`. The token counts (33 and 1) match P0's pre-redaction
  counts.
- **The manifest changed only as the script requires.** `git diff HEAD -- tests/native/path-redaction.json` adds the two
  entries and nothing else, going from 513 to 515 entries. `verify` reports 515 entries and 0 mismatches. That result is
  supporting only, because `verify` ignores file arguments, as the record says.
- **The check is non-vacuous.** `raw/c02-redact-check.sh` refuses any entry that is not a regular non-symlink file and
  passes the list through `xargs -0`, so no argument can be silently dropped. `raw/c02-redact-check.log` prints
  `files: 42` and then 42 paths for the check list, and `files: 37` and 37 paths for the staged list, each ending in
  `exit: 0`. The script prints nothing when there is no leak, so the empty check output is expected. The log's sha256
  equals the value P4 saved in `raw/c02-check-log-sha256.txt`, and the C01 copy and the C02 mirror are identical.
- **The positive controls really exit 1.** In `raw/c02-redact-control.log`, the scratch file outside the repository
  exits 1, and the rebuilt `red-first.log` and the rebuilt `plan/DECISIONS.md` each exit 1 alone. I ran my own check over
  43 files: the 42-file list plus the C02 `inventory.txt`, at the current bytes, including the C01 record after the sync.
  It exits 0. A one-line control file exits 1. The same 42 files plus one leaking copy, in a single `xargs` call, also exit
  1, so a single leak among many arguments is detected
  (`tests/native/v5-02-ra01-c-c02-evidence/review/redaction-check.txt`).
- **The raw logs are kept.** `raw/g2-redact.log` still exists (0 bytes, unchanged). `raw/c02-raw-compare.log` shows the C01
  raw manifest with every old line unchanged and only `c02-*` additions.
- **The reports are corrected.** C01 `REPORT.md` command row 13 (:40) and the Redaction section (:95) now say the gate-run
  check was vacuous, name the redact step and the manifest update, and point to `raw/c02-redact-check.log` and
  `raw/c02-redact-control.log`. They do not repeat the old "23 files" claim. In the C01 record, the Redaction row (:159),
  G6 (:189), the Limits bullet (:201) and the R1 outcome (:232) say the same. `diff` against
  `raw/c01-record-pre.md` shows exactly the eight P2 lines plus the eight lines from the coordinator sync, and the line count
  is unchanged (238).

`red-first.log` is redacted and checked but not staged. That follows D7 and the evidence precedent (93053c42, afb927dc and
f78f8eb4 stage no `red-first.log`, `results.jsonl` or snapshot). `git ls-files tests/native` holds no `red-first.log` from
any round.

## A1 to A4 from the C01 review: closed

- **A1.** `inventory-debug.txt` now has 1439 lines. `LC_ALL=C sort raw/g2-names-debug.txt | cmp - inventory-debug.txt`
  exits 0, and it is byte-identical to `inventory.txt` (both `28bd1674...`). Both C01 names are present. The old file is
  kept as `raw/c02-inventory-debug-old.txt`, which equals the sorted gate run 1 list `raw/gate-names-debug.txt` (1438
  lines), so nothing is lost. REPORT.md :48 and record :85, :144 and :233 label it as the gate run 2 list.
- **A2.** REPORT.md :87 records that the red-time identity was given in prose and reconstructed by the C01 review, and
  that the reconstruction was accepted as non-blocking. The two full hashes it quotes come from `red/RED.md`. The red test
  file hash appears only in the abbreviated form `379d82d7...2a3d`, which is how C01 review :108 gives it. No 64-hex
  string beginning `379d82d7` appears anywhere.
- **A3.** `diff` against the pre-edit copies shows `acceptance-matrix.md` changed on line 29 only (A07) and
  `acceptance-rows.md` on lines 48 (C-15r) and 68 (RA01-C-F1) only. All three now state gate run 2, 1439 of 1439 on macOS
  and Linux, Debug and RelWithDebInfo, and cite the C01 review. No "gate pending" remains on those lines. The statuses, the
  `closed-by-corrective` cell, the D22 limit and the F2 to F8 rows are unchanged.
- **A4.** REPORT.md :71, :83 and :101 now carry the `probes/PROBES.md` pointer ("T2 probed, 9 of 9 I6 mutants killed by
  T2, no survivors"), alongside the existing pointers on :67 and :87. That matches PROBES.md, whose I6 rows show nine
  killed by T2, and matches the C01 review.

## Lint and build state

`python3 scripts/rewrite-check.py lint --build-dir build/rewrite-debug`, run by me, exits 0 with "Formatting and Doxygen
checked 171 active source files" (`review/raw/lint.log`, which is a raw log and is not staged). The gate runner's
`raw/gate-lint.log` agrees. The `ninja -n` dry run printing "Re-running CMake" is correctly reported as a limit and not
claimed as a no-work result. The last valid no-work results are the C01 gate run 2 logs, on identical source.

## Findings

### V5-02-RA01-C-C02-R1 (blocking, records only): the recorded close procedure no longer matches the tree, and the staged list omits the C02 inventory

The record's P5 procedure and its guards were fixed against the state at P4 (`raw/c02-final-sha256.txt` 05:40,
`raw/c02-status-pre-close.txt`, `raw/c02-set-derivation.log`). Three committable files changed after that. The gate runner
rewrote the C02 `REPORT.md` and created the C02 `inventory.txt` (05:41 to 05:42). Then the coordinator sync rewrote eight
C01 record lines (05:43). The record lists all three writes, but it amends only D24 and I9. It leaves the close guards as
they were:

1. **D27 step 1 and I22 will stop P5.** They require every staged path other than the roadmap, the C02 record and the
   review files to still match `raw/c02-final-sha256.txt`. `shasum -a 256 -c` over that file fails today for
   `docs/vernacular/rounds/V5-02-RA01-C-C01.md` (pinned `8cfed374...`, now `9813ee5c...`) and for
   `tests/native/v5-02-ra01-c-c02-evidence/REPORT.md` (pinned `f26ad991...`, now `e38d017d...`). The C02 record also fails,
   but it is exempt.
2. **D31 and I25 will stop P5.** The drift compare allows only the roadmap, the C02 record and new review paths to differ.
   `raw/c02-status-pre-close.txt` pins the C01 record at `8cfed374...`, so the drift compare reports a changed hash. The
   I25 row's observed cell says "PASS, no drift (raw/c02-status-pre-close.txt)", but the compare is a P5 step that has not
   run: there is no `raw/c02-drift-close.log`. It would fail if run now. That cell is false. (The status listing also
   cannot see the git-ignored evidence folders, so it would never notice an added evidence file there.)
3. **D32 and I26 will stop P5, and the staged list is incomplete.** The find rule now prints ten paths instead of nine,
   because `inventory.txt` is new. D32 says P5 must stop when its find differs from the P4 output. Independently of that
   stop, the staged list (`raw/c02-redact-staged.txt`, S = 37) does not contain
   `tests/native/v5-02-ra01-c-c02-evidence/inventory.txt`. The round's own staging rule ("reports, READMEs, manifests,
   inventory, plan and probe notes") and the precedents stage each round's `inventory.txt`, and the gate REPORT cites it.
4. **I5's ordering claim no longer holds.** I5 says the authoritative check ran "after the last byte edit of every
   committable file except the roadmap, the Outcome and the review files". The current bytes of the C01 record, the C02
   REPORT and the C02 inventory came later, and none of the round's saved checks covers them (see A1 below). The files are
   clean: my check in `review/redaction-check.txt` covers all three at their current bytes and exits 0. But the record's
   statement that the binding check followed the last edit is wrong as written.

Fix, in the C02 round record only (no source, test or evidence data):

- Add an amendment beside the D24/I9 one. It should state that the gate-stage writes (C02 `REPORT.md`, C02
  `inventory.txt`) and the sync edit of the C01 record supersede their entries in `raw/c02-final-sha256.txt` and
  `raw/c02-status-pre-close.txt` and the P4 find output, and it should give their current sha256 values (already in the
  record's per-file table) as the values P5 compares against.
- Decide the staging of the C02 `inventory.txt`. The convention says stage it, which gives M = 10, S = 38 and N = 39 + K.
  State that P5 re-derives the find output and the counts rather than requiring equality with the P4 log.
- Change the I25 observed cell to "not yet run: P5 close step". Correct I5's observed cell to say which files were edited
  after the P4 check, and that the P5 commit check (I22, `raw/c02-redact-check-commit.log`) is the binding check over the
  final bytes of the whole close set, with the reviewer's 43-file check as the interim one.

### Advisories (non-blocking)

- **V5-02-RA01-C-C02-A1: `raw/gate-redact-check.log` is the same shape as the log R1 condemned.** It is 9 bytes long and
  holds only the heading `== final`, with no command line, no file count and no exit code. The record (I5 observed cell,
  Acceptance row) and the gate REPORT cite it as "exit 0". Nothing in the bytes shows that. The gate summary's claim of 43
  files exiting 0 is consistent with `raw/gate-final-list.txt` and with my own check, but the log itself proves nothing.
  Label it in the record as an unrecorded exit, and cite `raw/c02-redact-check.log` and `review/redaction-check.txt`
  instead.
- **V5-02-RA01-C-C02-A2: C01 record :222** says the hashes were "re-pinned at the C02 gate in
  `raw/c02-status-pre-close.txt`". P4 wrote that file, not the gate runner. It still holds the pre-sync C01 record hash, but
  the two hashes the sentence quotes (`f09a7531...`, `48c842dd...`) are right. A wording fix at the next sync is enough.
- **V5-02-RA01-C-C02-A3: C01 REPORT.md :48** joins the A1 label to the preceding sentence with a colon after "none removed
  or renamed", which reads as if the label were the list of removed names. A full stop would fix it.
- **V5-02-RA01-C-C02-A4: the I9 observer** prints `pre=239 post=239`, while `wc -l` gives 238 for both files (it counts the
  empty string after the final newline). The equality it tests still holds, so the result stands.

## Invariant table

The record reproduces all 26 rows of `plan/INVARIANTS.md` revision 4. I compared the id, invariant, mutant and test cells
programmatically, and they are identical. expectRed is false in every row. The record adds a testFile column ("see test
column") and drops the plan's owning-package column. No content is lost.

Every acceptance criterion and every closed finding has a row. Identity is I1, R1 is I2 to I7 (redaction, script
provenance, non-vacuous check, positive controls, raw logs kept), A1 is I10, A2 is I11, A3 is I12 and I13, A4 is I14, lint
is I16, ownership and the file list are I15 and I17, and the commit, roadmap, close check and Outcome are I18, I19, I22
and I23. Each row's stated mutant would break its invariant. The round has no test and no probes by design, so no probe
kill can be checked. The rows are observers, and I re-ran the observers that matter (I1 to I7, I10 to I14 and I16)
independently, with the results above. One row is hollow, I25: its observed cell reports a pass for a compare that has
not run (R1, item 2). I5's observed cell is stale (R1, item 4). I18, I19, I22 and I23 are correctly marked as not yet run.
There is no red-first wave and no `red/` folder, which is correct for an evidence-only round.

## Commit staging list

The staged list is explicit (37 enumerated paths plus, at close, this review file and the files under
`tests/native/v5-02-ra01-c-c02-evidence/review/` outside `raw/`). It uses no `git add -A`. It contains no `/raw/` path and
none of the five checked-but-unstaged files. Every listed path passes redaction. Nothing is staged yet
(`git diff --cached` is empty), and the branch is `dev/planar-v2-vernacular`. The planned commit (`git commit -s`, no
co-author trailer, a subject naming V5-02-RA01-C-C02 and C01) matches the brief. The one gap is the missing C02
`inventory.txt` (R1, item 3).

## Reviewer evidence files

Written by this review, all free of absolute paths and in the close set: `tests/native/v5-02-ra01-c-c02-evidence/review/identity.txt`
and `review/redaction-check.txt`. Raw, not staged: `review/raw/lint.log` (holds absolute paths in the lint trace).

## Limits

- I did not rebuild or run ctest. No src or test byte differs from the C01 candidate, whose gate run 2 (1439 of 1439,
  macOS and Linux) the C01 review verified from raw logs. My `ctest -N` confirms the registered names only.
- I applied no mutants. The round has none to probe.
- The roadmap close, the Outcome append, the staging and the commit are P5 steps after approval. I could not review them.

## Records re-check (2026-11-01)

Disposition: **request-changes, records only**. The records fix closes R1 items 1, 3 and 4 in substance and item 2 as
written. No source, test or evidence-data byte changed. But one close guard that R1 is about still stops P5 (D27 step 3).
That guard was already broken when I first reviewed, and I missed it then. The edited record also leaves one false
statement of state. Both are fixed by editing the C02 round record only (RR1 below). Evidence:
`tests/native/v5-02-ra01-c-c02-evidence/review/records-recheck.txt`, which is free of absolute paths and belongs in the
close set beside `review/identity.txt` and `review/redaction-check.txt`.

### Nothing outside the declared records changed

- `identity.py` over `src`, re-run and recorded verbatim in `review/records-recheck.txt`: HEAD `93053c42...`, tracked-only
  and combined `6b23117bfb40136186fabf5515d97e0ac1da99c0b69b6d00584033cab0f4aeda`, test file `38d79424...`,
  `orchestrator.lua` `e7d9f570...`. These equal the candidate in "Exact candidate" above, so no src or test byte changed.
- `find` (excluding `build/` and `.git/`) for files newer than my `review/redaction-check.txt` lists exactly the four files
  the records fix declares (the C01 record, the C02 record, the C01 `REPORT.md` and the C02 `REPORT.md`), plus this review
  file and my own `records-recheck.txt`. No raw log, plan file, manifest, inventory or other evidence file changed.
  `raw/g2-redact.log` is still there. `tests/native/path-redaction.json` is still `15fa3589...`.
- `shasum -a 256 -c raw/c02-final-sha256.txt` fails for exactly four paths: the three re-baselined files and the exempt C02
  record. Every other pinned file is unchanged.
- The C01 record still differs from `raw/c01-record-pre.md` only on the 15 lines declared in D23 and the Supersedes
  amendment (:83, :85, :144, :159, :189, :191, :192, :201, :220, :222, :226, :232, :233, :235, :236), and its line count
  is still 238. The C01 `REPORT.md` still differs from `raw/c01-report-pre.md` only on the seven P2 lines (:40, :48, :71,
  :83, :87, :95, :101). I kept no copy of the two files' bytes from my first review, so I cannot prove that the records
  fix touched only :222 and :48. Both edits do sit inside the declared line sets.
- A redaction check over the eight files that are new or changed since my first check (the four edited records, this
  review file, the C02 `inventory.txt` and my two earlier review files) passes one argument per file and prints
  `files: 8` and every path. It exits 0. The same call with a scratch control file added exits 1. (My first attempt passed
  the list as one unsplit zsh word and also exited 0: this is the R1 failure shape again. It is not evidence, and the
  recorded run uses an array.)

### The blocking findings, item by item

- **R1 (1), D27 step 1 and I22: closed.** The three replacement hashes in "Records fix after review" equal the files on disk
  (`75a8055b...`, `b09ba231...`, `83a8434e...`). The identity.py values quoted for those three pathspecs (empty tracked
  diff `e3b0c442...`, combined `7e3c180b...`) equal my re-run. They are a summary rather than the verbatim table, and
  identity.py excludes the two evidence `REPORT.md` files from the combined hash, so only the per-file hashes bind those
  two. That is enough.
- **R1 (2), D31 and I25: closed.** The I25 cell now says not yet run. `raw/c02-status-pre-close.txt` lists the C01 record as
  its only re-baselined path, because the evidence folders are git-ignored. "That one allowed difference" in the I25 cell
  is therefore right for the status compare. The broader three-file wording in the Records fix bullet is harmless.
- **R1 (3), D32 and I26: closed.** The find rule prints 10 paths (re-run). The record now requires
  `tests/native/v5-02-ra01-c-c02-evidence/inventory.txt` to be staged, gives M 10, S 38, C 43 and N 39 + K, and leaves the
  re-derivation to P5. The raw logs keep their P4 values and are not edited.
- **R1 (4), I5: closed in substance, wording incomplete.** The correction names the C01 record, the C02 `REPORT.md` and the
  C02 record as edited after the P4 check. It leaves out the C01 `REPORT.md` (edited by the records fix itself, for A3) and
  the C02 `inventory.txt` (created after the P4 check). It also says "reviewer check found them clean". My first check
  covered the earlier bytes of those files, not the bytes after the records fix. My re-check above now covers the current
  bytes, so the statement holds, but it should cite this check (RR1 item 3).

### V5-02-RA01-C-C02-RR1 (blocking, records only): D27 step 3 still stops P5, and the record leaves one false statement of state

1. **D27 step 3 is not superseded.** It says P5 must "copy the C02 record to raw/c02-record-pre-outcome.md (must equal its
   c02-final-sha256.txt hash)". That pinned hash is `54e05b92...`. The C02 record has changed twice since then, at the
   pre-review sync and in the records fix, so the guard fails today and a literal P5 stops at step 3. The records fix says
   that the C02 record "is exempt and hashed after its last edit into raw/c02-final-sha256-close.txt". That restates the
   exemption in steps 1 and 6. It does not address the separate equality in step 3. Fix: add a bullet to "Records fix after
   review" that supersedes the step 3 equality. For example: P5 records the C02 record's sha256 in step 1, before any P5
   edit, and the pre-outcome copy must equal that value. I said in the first review that the C02 record "is exempt" and did
   not check step 3. This item is my miss, not a regression from the fix.
2. **The Acceptance row "Opus review of this round" is now false.** It says "not met | not yet run; no review file exists".
   The review file exists and holds two passes. D28 has P5 append an Outcome but does not have it edit the Acceptance
   table, so this row would be committed as written. Fix: say the review ran (request-changes, records only, then this
   re-check) and that the approving pass is still owed. Alternatively, label the row as a snapshot taken before the review.
   Lines :118 and :163 ("checked at this sync") are labelled as snapshots and can stay.
3. **Complete the I5 correction:** add the C01 `REPORT.md` and the C02 `inventory.txt` to the list of files written after
   the P4 check, and cite `review/records-recheck.txt` as the check over their current bytes.

### Advisories carried (non-blocking)

- **A1 is partly applied.** C02 `REPORT.md` :43 and the record's I5 cell and Acceptance row now call
  `raw/gate-redact-check.log` unrecorded. C02 `REPORT.md` command row 2 (:33) still gives exit `0` and cites that log as
  its source, and :68 marks commands 2 and 3 as met. Adding "(log unrecorded, see Redaction)" to row 2 would resolve the
  contradiction.
- **A2 is partly applied.** C01 record :222 still says the snapshot was "written at the C02 gate". `raw/c02-status-pre-close.txt`
  has mtime 05:40:43, and the earliest gate-runner file has 05:41:43. P4 wrote it before the gate ran. Change "at the C02
  gate" to "by C02 P4". The parenthesis added by the fix is correct. The lineage table row for :222 (C02 record :175) still
  says "re-pinned".
- **A3 is closed.** C01 `REPORT.md` :48 now ends "none removed or renamed." with a full stop.
- **A4 is closed as recorded.** The raw log is left unedited, and the off-by-one is stated in the record.

### Re-check limits

- I did not rebuild or run ctest. No src or test byte changed.
- This pass approves nothing. After RR1 is fixed, a further records-only re-check can approve if it finds no new false
  statement. That re-check should again compare the four record files only, and use `review/records-recheck.txt` as its
  baseline.

## Closing re-check (2026-11-01)

Disposition: **approve**.

- **Candidate unchanged.** Workflow identity.py over `src` gives HEAD `93053c42`, tracked and combined diff sha256
  `6b23117bfb40136186fabf5515d97e0ac1da99c0b69b6d00584033cab0f4aeda`, `authoring_orchestrator.t.cpp` `38d79424...a46f`
  and `orchestrator.lua` `e7d9f570...87a1`, equal to the frozen values. No script under `tests/native/` was run.
- **RR1 (1) closed.** The record's Outcome supersedes the D27 step 3 equality: P5 records the C02 record's sha256 at step 1,
  before any P5 edit, in `raw/c02-final-sha256-close.txt`, and the pre-outcome copy must equal that value. The step can be
  executed as written. The pre-outcome copy is taken after the coordinator Outcome is in place, so the D28 "additions only"
  check applies to the P5 Outcome alone.
- **RR1 (2) closed.** The Acceptance row "Opus review of this round" now states that the review ran (request-changes, records
  only, then the records re-check) and that the approving pass was owed. It is no longer false. This section is that pass.
- **RR1 (3) closed.** The I5 cell lists the C01 `REPORT.md` and the C02 `inventory.txt` as written after the P4 check and
  cites `review/records-recheck.txt` as the check over their current bytes.
- **Advisories carried.** A1r (C02 `REPORT.md` command row 2 still cites `raw/gate-redact-check.log` for exit 0) and A2r
  (C01 record :222 says "at the C02 gate" where "by C02 P4" is meant) stay open as advisories and are not applied. Editing
  either file would change its pinned hash again. Both are wording faults, and neither misstates the candidate or any
  gate result.
