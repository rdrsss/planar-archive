# R19-ACCEPT-A close review (closing fix of C03)

Disposition: **request-changes** (records and commit list only; no build, run, gate or support-code change is needed).

The closing fix answers the substance of C03-R1, R2 and R3:

- The counter formula is fixed and tested red-first.
- `run_heavy` exits 4 on a refused result.
- A new heavy run passed validation, `index.json` (L20) exists, and every L21 step has command, exit and output.
- The `.pyc` is gone, and the 153-id Python list is saved.
- The product candidate is byte-identical to the C03 full gate.

The two `commit-list` rule changes are sound and narrow. Two record defects stop a clean commit. The commit list departs from
a recorded decision, and its retained check outputs do not match the record. The roadmap is stale and drops status text
that its own rule says must go to history. Both are fixed by editing records and re-running the list checks.

The reviewer was not the coder or the coordinator. Writes were limited to this file. Nothing was staged or committed. No
build, ctest, Docker, Linux gate or installed or legacy binary ran, and `~/.planar` was not touched. In the checkout the
reviewer ran only read-only commands: `git diff`, `git status`, `shasum`, `cmp`, `grep`, `find`, `stat`, inline
`python3 -I` parsers, and `run_acceptance.py commit-list`, which exited 0 with `commit-list: 4847 entries`. The reviewer
copied the support tree and `r19-cmake-support` to `<scratch-root>`, then ran `HeavyDriverTest` and four `commit-list`
tests there with `-B`, scratch HOME and PLANAR_DB: 12 run, OK. No `.pyc` appeared in the checkout.

## What was verified and holds

- **C03-R1 formula.**
  - `fault_driver._Scenario.intended_executions()` returns `(check_rounds - interrupted) * checks_per_round + interrupted`.
    Both call sites use it (`stopped_result` near line 960, `finish` near line 991).
  - `test_intended_executions_counts_one_check_for_an_interrupted_round` expects 7 for the C03 shape, 8 with no interrupt,
    and 4 with one check per round.
  - `closing/red/intended-red.out` is `AssertionError: 8 != 7`, a behavioural red. `probe-formula.out` kills the reverted
    formula by that test name.
- **C03-R1 exit status.**
  - `run_heavy.main` writes `result.json`, returns 3 when the trajectory is not valid, then calls
    `run_acceptance.validate_rerun_result` and returns 4 on `AcceptError`.
  - `exit-red.out` is `AssertionError: 0 != 4`, and `probe-exit.out` kills the `return 0` mutant by name.
  - The completed-scenario test now patches the validator. That is the only change to it.
- **Heavy close run.**
  - `run-markers/` holds start, DONE, exit `0`, and empty stdout and stderr.
  - `SHASUMS-source.sha256` is byte-equal to `SHASUMS-copy.sha256`, 903 lines. `shasum -c` of the copy reports no
    mismatch. The tree holds 911 files, which equals 903 plus the two sums files, five markers and the export list.
  - In `result.json`, `counter_lines` is 7, `intended_executions` is 7 and `trajectory.valid` is true.
  - The attempt outcomes are interrupted, completed, completed, completed. `evidence/checks/debug` holds 4 attempt
    folders and `release` holds 3, which matches 7.
- **L20.**
  - `runs/cmake-rerun/rerun-index.exit` is 0, and stdout is `rerun index written: index.json`.
  - `index.json` carries every L20 field:
    - `faults.interrupt`: CLI exit -9, child dead, reservation in progress. `faults.recover` is `check-recover`, exit 0,
      outcome interrupted.
    - `history`: matched `check_reserved`, `check_interrupted` and `run_taken_over` events.
    - `reads.after_restart`: fresh processes.
    - `takeover`: generation 1 to 2. `fenced`: `fenced`, exit 3, ledger before equal to after.
    - `verdicts.reject1.repair_admitted` is true. `reject2` is capped with `max_rounds` open.
    - `load`: max_rounds 1 to 2, `blockers_cleared` `max_rounds`.
    - `verdicts.final`: accepted, `targets_equal_latest_output` true.
    - Six replays, all response- and ledger-identical. `ledger` before replays equals after.
    - `counters` 7 = 7, and the trajectory has `valid` true.
- **L21.**
  - `limits/limits.json` records engine exit 3, empty stdout and the closed `resource_limit` `instructions` envelope.
  - `limits/packet.json` records lines 163 and 218. They are byte-equal to the current `src/stdlib/planar/render.lua`, and
    `source_sha256` equals that file's sha256.
  - The `raw/limits-close/*.exit` files are all 0. `runs/post-index/index.exit` and `verify-redaction.exit` are 0.
  - The index count difference checks out:
    - `expected-index-count.txt` is 4463, and `index` reported 4466.
    - The three extra files, `index/limits-close/index-dry.{stdout,stderr,exit}.index`, exist.
    - `index/` holds 4466 files.
    - `raw/` has 4474 files. The 8 not indexed are the later redact and verify outputs and the two originals, all local only.
  - `scan-whole-tree` hits only `raw/`, `.raw/` and `gate/candidate`. No such path is on the list except `candidate.json`
    and its superseded copy, which carry no host path.
  - The repository manifest `tests/native/path-redaction.json` has no diff against HEAD.
- **Rule change 1: `COMMIT_LIST_INDEX_ALLOWED`.**
  - It is sound and narrow. The regular expression accepts only an `.index` file under
    `tests/native/r19-accept-evidence/index/`.
  - An index file is a digest: name, size, sha256 and the redacted host-bearing lines only (`compact_index`). It holds no raw
    log body.
  - The test refuses a non-index file, an index in another tree and an index under `raw/`. The permissive probe is killed
    3 times.
  - C02-D28 had already put `index/**` on the list, so this removes a conflict between C02-D28 and C3-D16. It does not
    weaken what C3-D16 refuses.
- **Rule change 2: `snapshot` on directories only.**
  - It is sound. The only snapshot-named directories in the round trees are the `red/snapshot*` folders, and they are
    still refused (test and probe).
  - The only snapshot-named files outside `index/` and `raw/` are the closing red and green logs for this test, which are
    legitimate.
  - The scratch copy of the C03 `run_acceptance.py`, whose hash equals the C03 record, differs from the current file only
    in these two hunks.
- **C03-R2 content.**
  - The list has 4847 sorted entries.
  - It names the moved fixture, the CMake, recipe and doc edits, `ignored_context_check.sh`, the C02 and C03 `REPORT.md`
    and `PROBES.md`, and the three tree-local manifests.
  - Outside the five evidence trees, the 36 changed or untracked paths from `git status --porcelain -uall` equal the list's
    non-evidence entries exactly (`comm -3` empty).
  - A grep of every listed file for the three host-path prefixes finds no real host path. The hits are pattern
    descriptions in the C01 and C02 reviews and the record, and test data in `execution.t.cpp` and the support files.
  - No listed file contains the current sha256 of any of 43 REPORT, PROBES, README, review, record or roadmap files.
  - The C02 review exception (lines 71-72 describe the grep pattern and hold no host path) is acceptable.
- **C03-R3.** `find` shows no `__pycache__` and no `.pyc` under the support tree.
  `gate/test-names/after/py-worktree-closing.txt` holds 153 ids. `comm -23` against `before/py-worktree-c03.txt` is empty.
  The `def test_` counts in the four test files are 63, 29, 28 and 33, which equals 153.
- **Gate citation.**
  - `git diff --binary HEAD -- src | shasum -a 256` gives `d9ca75a13fcf1c3aa11b79ee38fdb5d24d733501deb4caf24c42d1830a6726c0`.
    That equals the record and `closing/identity/src.txt`.
  - `closing/identity/acceptance-scripts-fixtures-testing.txt` is byte-identical to the C03
    `gate/identity/acceptance-scripts-fixtures-testing.txt`. A recomputed tracked diff and the per-file hashes over that
    pathspec equal both.
  - No ctest or CMake file references the support tree, so the support edits are not gate inputs.
  - Citing the C03 full gate under the 2026-11-11 operator rule is valid.
  - `closing/suites/summary.txt` reports 153 run, 0 failures, 0 errors, 0 skipped, exit 0. The logs it points to say `OK`.
- **Product.** The `history_rows` fix keeps integer and boolean nodes apart. Three Catch2 tests were added, and the CMake
  change is the schema path only. All are unchanged since the C02 and C03 reviews.

## Findings

### R19-ACCEPT-A-close-R1 (blocking): the commit list departs from C3-D16, and its retained checks do not match the record

1. **The stale C02 list is listed.** `tests/native/r19-accept-a-c02-evidence/COMMIT-LIST.txt` is entry 16 of the C02 list
   and is now on the C03 list.
   - C3-D16 says that list is "LEFT OUT of the C03 list and treated as superseded". The R2d exclusion grep names it.
   - It names a missing file (`probes/PROBES-reprobe.md`) and lists itself.
   - `closing/list/scripts/make-list.sh` does not exclude it, and the closing record does not mention the departure.
   - Fix: add it to the exclusions, or record a decision that replaces C3-D16 on this point.
2. **The record understates the check hits.** The final check outputs (`closing/list/raw/`, written 16:24 to 16:25 local
   time) show:
   - two `scan` hits: the C02 review and `docs/vernacular/rounds/R19-ACCEPT-A.md` (`users`);
   - two `grep-c2` hits: C02 review line 72 and record line 1649.

   The record says "one hit" for each. The record hit comes from the record's own description of the grep pattern
   (line 1648 quotes the users prefix, and line 1649 quotes the per-user slug). It is not a host path, but the record is a
   second, unrecorded exception.

   The record was also edited after the checks ran. Its mtime is 16:25:15, and the saved grep output quotes line 1648 as
   "4818 paths", while the record now says 4822. So the retained outputs were taken on different record bytes.
3. **The list must change again anyway.** `make-list.sh` already names this review, and C3-D16 has the coordinator add
   the review at commit time.

Required:

- Exclude the C02 list.
- Make the last record edit, then regenerate the list with `make-list.sh`, and run `list-checks.sh` once on the final bytes.
- Record the real hit set from that run, and extend the recorded exception to the record's own pattern lines, or reword
  those lines so they do not quote the patterns.
- This review quotes no pattern literal, so it should add no hit.

### R19-ACCEPT-A-close-R2 (blocking, records only): the roadmap is stale and drops status without preserving it

- The "R19 acceptance" row still begins "Corrective R19-ACCEPT-A-C04 comes first (C03 review R1-R3)". No C04 exists: the
  operator asked for the closing fix instead.
- The opening deletes the R19-AUTHORING + C01 and R19-CMAKE status paragraphs. Roadmap line 8 says superseded status
  checkpoints are preserved verbatim in history. `docs/vernacular/history.md` is unchanged and does not contain them
  (`grep -c` is 0).
- Those paragraphs carried open items: the authoring-loaded activation mismatch, the captured-preview instruction limit
  and the reviewer-packet wording. L21 now measures two of them, but the third has no other place in the status.

Fix: move the removed paragraphs verbatim to a new history checkpoint, add `history.md` to the list, and correct the
R19 acceptance row. Then re-run the list checks (close-R1).

### R19-ACCEPT-A-close-A1 (advisory): the identity of the heavy-run inputs is not retained in the tree

The heavy close run ended at 15:45 local time. `fault_driver.py` and `run_heavy.py` both have mtime 16:03:00, which fits
the restore after probes P1 and P2. `run_acceptance.py` was edited at 16:06, after the run. C4-D2 makes `run_heavy` import
`run_acceptance`, which C3-H11 had ruled out.

The reviewer confirmed from the coordinator's scratch copies:

- `fd.fixed` and `rh.fixed`, saved 46 and 40 seconds before the run started, are byte-equal to the current files.
- The C03 `run_acceptance.py` copy differs from the current one only in the two commit-list hunks.

So `validate_rerun_result` and the formula that the run used equal the committed bytes. The record should state this, as
a C3-H11 and C3-L0 successor row, with the identities saved in the tree. Scratch is not evidence.

### R19-ACCEPT-A-close-A2 (advisory): small record and comment drift

- The `run_heavy.py` header comment still lists exit statuses 0 and 3 only. Add 4.
- The exit-4 test's stub result is refused by the validator for missing fields before the counter clause. That is valid
  for "a refused result exits 4", but it does not isolate the counter mismatch.
- The record calls `closing/suites/refused-1/` "local only", but its three files are on the list. They are harmless, but
  the record and the list should agree.
- The record says the two identity files are equal "apart from the HEAD row". They are byte-identical.
- The record should state the 8 post-index files in `raw/limits-close/` that have no index.
- The 4463 against 4466 count departs from C02-D28's "must equal". The explanation is correct and verified, and it should
  stay recorded as a departure.

### R19-ACCEPT-A-close-A3 (advisory): commit size against the evidence convention

The list adds 4847 files and about 17.6 MB, all force-added under the `.gitignore` evidence rule. That includes 24 index
files over 100 KB, the 1.4 MB `TEMP-PATH-REDACTION.json` and the 439 KB list itself. The `.gitignore` comment describes
committed evidence as REPORT and README files plus small manifests of 100 KB or less. C02-D28 chose to commit `index/**`,
so this is not a defect of the closing fix. It is a repository-size choice the operator should see before the commit.

### R19-ACCEPT-A-close-A4 (advisory): roadmap gate and limit bullets

"Latest complete gates" and "Open limits" still say Linux has not run on the per-directory layout. The C03 final gate ran
Linux arm64 on it, 1854 of 1854. Update these bullets when the round is approved.

## Limits of this review

- No build, ctest, Linux gate, live suite or mutation ran here. Gate and probe outcomes rest on retained logs and on the
  identity checks above.
- Unit tests ran only for `HeavyDriverTest` and four `commit-list` tests, from a scratch copy.
- The `scan`, redaction `check` and slug greps were not re-run. Their results rest on the retained outputs, and close-R1
  shows those outputs are stale.
- The counter log of the heavy run is not retained. The count of 7 rests on `result.json` and the attempt folders.
- The heavy-run input identity rests on scratch copies outside the evidence tree (close-A1).
- REPORT-hash pins were checked only against current document hashes, plus the tool rule and the coordinator's adjacent-line
  grep. Hashes of older document versions were not enumerated.
- x86_64 Linux remains uncovered, as recorded.

## Re-review (after the coordinator's fixes)

Disposition: **approve**. The lineage is ready to commit as one local commit, on one condition. After this section is
written, the coordinator re-runs the list checks on the final bytes and the results match those recorded below.

Same rules as before. The checkout was read-only apart from this file. The only command run in place was the read-only
`commit-list`, with `-B`: exit 0, `commit-list: 4852 entries`.

- **close-R1: fixed.**
  - The C02 `COMMIT-LIST.txt` and `closing/suites/refused-1/` are off the list. `docs/vernacular/history.md`, this review
    and `closing/identity/heavy-run-inputs.md` are on it.
  - The non-evidence entries still equal the changed and untracked paths from `git status` outside the evidence trees
    (`comm -3` empty).
  - The order is right. The last record edit was at 16:34 local time, history and roadmap at 16:33, the list at 16:36:02,
    and every check output from 16:36:03 to 16:36:55.
  - The exit files show:
    - `commit-list` 0 and redaction `check` 0;
    - `scan` 1, with `scan-hits.txt` naming only the C02 review and the record;
    - c1 1 and c3 1, with no match;
    - c2 0, with `grep-c2-files.txt` naming the same two files.
  - The committed outputs hold paths only, so they cannot match themselves.
  - The record now states the exception as "documents that describe the patterns in words". It says any other hit path
    blocks the commit. That is the right form.
  - A fresh grep of every listed file for the host-path prefixes finds the same pattern-description and test-data files as
    before, and nothing new in `history.md` or the roadmap.
  - `pin-grep.txt` names record lines 983 and 1441. They still hold a src identity hash and a probe diff hash, not a
    document pin.
- **close-R2: fixed.**
  - The new `history.md` section is byte-equal to lines 12-16 of the HEAD roadmap (`diff` empty).
  - The roadmap links to it, and the anchor matches the heading.
  - The R19 row no longer names C04.
- **close-A4: fixed.** The gate bullet states the C03 macOS gate and the first per-directory Linux aarch64 gate, as an
  uncommitted candidate. The stale "Linux not run on the per-directory layout" limit is gone.
- **close-A1: fixed.** The heavy-run input table in `heavy-run-inputs.md` matches the reviewer's own check:
  `fault_driver.py` and `run_heavy.py` are equal, and `run_acceptance.py` differs only by the two commit-list rule edits,
  which the heavy run does not call.
- **close-A2: accepted.** The identity text, the eight unindexed `limits-close` files and `refused-1` are corrected. The
  `run_heavy.py` header comment and the exit-4 stub result are kept as stated limits. Both are minor, and neither changes
  a result.
- **close-A3: stated for the operator.** It is not a defect of this round.

No new findings.

### Limits of the re-review

- `scan`, redaction `check` and the slug greps were not re-run by the reviewer. They rest on the retained exit files and
  path lists, plus the reviewer's own host-path grep.
- Appending this section changes this file's bytes after the 16:36 checks. Because of that, the coordinator's final
  re-run of the list checks is a condition of this approval.
- The limits of the first review still apply.
