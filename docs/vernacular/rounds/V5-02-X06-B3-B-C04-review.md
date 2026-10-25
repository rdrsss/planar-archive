# V5-02-X06-B3-B-C04 independent review (fourth corrective round: record accuracy only)

Disposition: **approve**. The records now describe the final candidate accurately, and they
close X06B3BC03-R1, R2 and advisories N-a to N-d. The source is byte-identical to the
C03-reviewed candidate. No raw log was edited or deleted. The commit step is ready. The
advisories below are wording points. None of them blocks the commit.

I reviewed in the shared checkout on `dev/planar-v2-vernacular` at HEAD
`a2aa1cc0ed4ad8c5b9e879a04231591839e7dc48`, against the uncommitted candidate. I wrote only
this file and new files under `tests/native/v5-02-x06-b3-b-c04-review-evidence/`. I applied no
mutants, ran no build and edited no source, test, record or raw evidence. My only build-tree
command was a read-only `ctest -N`, run with no other ctest or ninja process active. I did not
commit, push, merge, install or publish, and I used no old Planar tool, queue, skill, role or
installed binary.

## Exact candidate (unchanged from the C03 review)

- `git diff HEAD -- src` sha256 is
  `96e44f54086469fe3ee0ccfa431ff4e3fadd12a15278834e5d6714613990531b` (`src-diff.sha256`). This
  equals the C04 dispatch value, the C04 gate files (`src-diff.start/end.sha256`,
  `src-diff.compare.exit` = 0) and the gate runner's three runs.
- The full sha256 values of all six files (`files.sha256`) equal the C03 reviewer's
  `gate/07-files-end.sha256` byte for byte (the diff exits 0). First 12 hex:

  | File | sha256 (12) |
  | --- | --- |
  | `runtime.cpp` | `ab70f5441bea` |
  | `runtime.cppm` | `331bd1b629df` |
  | `profile.cpp` | `91ac81f3d466` |
  | `profile.cppm` | `7bb805634449` |
  | `runtime.t.cpp` | `3b22d137816e` |
  | `profile.t.cpp` | `d94976e9c9ea` |

- **Prefix correction confirmed.** `runtime.t.cpp` hashes to
  `3b22d137816e942795336d6f37a2a44dd33ca8e021cbecef2fc492f1a29a669f`. The value `3b22d1378165`
  in the C03 review, in C03 `REPORT.md` line 11 and in earlier record text was a
  transcription error. C04 states the correction in the C04 dispatch, in the C03 candidate
  identity line and in the new C03 evidence `README.md`.
- No file under `src/` is newer than the C03 review file. `git status --short` (`status.txt`)
  is byte-identical to the C04 gate's `status.txt`. It lists the six source files, the
  roadmap and the eight untracked B3-B round and review records.
- Test inventory: `ctest --test-dir build/rewrite-debug -N` exits 0 (`ctest-N.exit`) with
  `Total Tests: 987`. That is 987 unique names, and 41 of them contain `X06 B3`. The sorted
  inventory equals the C03 `inventory.txt` (`inventory.diff` is empty, `inventory.diff.exit`
  = 0), so no name was added, removed or renamed. No test was executed. Because all six
  hashes are equal, the C03 review's 987/987 execution result still applies.

## Closure of the C03 findings

| Finding | Status | Evidence (read from the bytes) |
| --- | --- | --- |
| X06B3BC03-R2: stale "final candidate" labels | **Closed** | C02 record: the heading now reads "Coder results (C02 candidate, superseded by C03 ...)". It opens with the final figures (987 tests, 985 C02 names plus 2, 41 `X06 B3`, `runtime.t.cpp` `3b22d137816e`, src diff `96e44f54...`) and points to the C03 "Final candidate record". The `162997debb60` and `8b2f73ea` values carry "C02 candidate" labels with the C03 final value beside them (lines 155, 162-164). The inventory, red-first and gate subsections carry "(C02 candidate)" and add the 987/41 figures (lines 241-243, 269-272). B3-B record: "C01 verification results (C02 candidate; final candidate figures added in C04)" now has a separate "Final candidate (C03)" bullet with 987 / 41 / `3b22d137816e` / `96e44f54...` and both C03 test names (lines 371-390). "Verification commands to run" item 2 reads "C02 candidate 985 of 985 ... final candidate (C03) 987 of 987" (line 215). In my reading of `grep-final-candidate.txt` and `grep-c02-figures.txt`, every "final candidate" hit names C03 or 987/41. Every 985, 39, `162997debb60` or `8b2f73ea` hit is labelled as the C02 candidate, as history, or as part of the coordinator's dated C02-outcome text. |
| X06B3BC03-R2: masking reason | **Closed, matches raw logs** | The C02 "Masking reason (corrected in C04)" design fact and the C02 red-first paragraph now say the following. On the C02 candidate, `open-bound-refusal-protocol` (`febc237bd88c`) and `source-error-mapping` (`664b8217a4bf`) survived standalone because no C02 test reached `open_bound`'s declaration parse. `open_bound` also parses the declaration. C03 kills each one standalone with `X06 B3 B a declaration the bound source cannot open refuses with its native diagnostic`, 1 failed of 41. I checked both halves against raw bytes. Survival: `probes/superseded-2/` ran on the C02 candidate (`candidate-files.scratch-before.sha256` has `runtime.t.cpp` `162997debb60...`), and its `raw/open-bound-refusal-protocol.ctest.0.txt` and `raw/source-error-mapping.ctest.0.txt` read "100% tests passed out of 37". That supports the record's "0 failed of 37". Kill: `probes/raw/probe-open-bound-refusal-protocol.log` and `probe-source-error-mapping.log` each give mutant sha256 `febc237b...` / `664b8217...`, build exit 0, ctest exit 8, "1 tests failed out of 41", and test #353 (the open-bound test) as the only failure. Each restored its file equal to the original. The test at `runtime.t.cpp:1718` asserts `lua_syntax` / `Invalid Lua text chunk` under field `profile`, which shows the parse claim from product behaviour. |
| X06B3BC03-R1: C03 final record understates the round | **Closed** | The "Final candidate record" no longer says "not verified" or "still owed". The only remaining hits for those words are in the first-dispatch Results section, which is labelled SUPERSEDED, and in line 415, which states the correction. "Changed files in C03" (lines 357-365) and the coordinator's "Complete changed-file list" (lines 405-413) both list `runtime.t.cpp`, the C03 record, the C02 record, the B3-B record, the C02 evidence `README.md`, the C03 evidence folder and the coordinator's roadmap edit. That matches the fix the review asked for. The acceptance table (lines 417-431) has one row per Dispatch 1 and Dispatch 2 criterion: R1, R2, R3 and probes, N-c, N-d, test names, diagnostics and product hashes, the gate, survivors, record consistency and the authoritative section. Its met and partly-met values agree with the C03 review's closure table. The two NOT MET rows are kept as the review left them, and line 433 states that C04 closed them. I accept that, because it keeps the review-time status accurate and the closure is recorded in the C04 acceptance table. |
| N-a: Repair 2 wording | **Closed** | Repair 2 now says the invalid declaration is written before `f.open()`, and marks the wording as corrected in C04. The test (`runtime.t.cpp` 1722-1725) does exactly that: `put(f.declaration, ...)`, then `f.open()`, then `prepare_load`. |
| N-b: C02 evidence README opening | **Closed** | The opening now says the README was written in C03 Dispatch 1, while C03 was records-only. It adds that C03 became test-only after Dispatch 2, and that Generation 2 (985 / 39) is the C02 candidate, superseded by 987 / 41 / `3b22d137816e`. |
| N-c: C03 `REPORT.md` run 3 and run 4 wording | **Closed by notes, raw report untouched** | The C03 record's "Correction notes (C04, N-c)" and the new C03 `README.md` note (a) agree with the folder. `raw/run4/` has exactly `build-tests.exit`, `ctest.exit` and `g-ctestN.exit`, while lint and both full builds have only `.log` files. Both texts point to the C03 reviewer's `01.exit` to `06.exit`. Note (b) withdraws the "line numbers have shifted" remark at `REPORT.md` line 49. I confirmed that `runtime.t.cpp` lines 1528-1539 are still the opaque-owner watch, from `b3b_fixture g;` through `CHECK(planar_admission_read_attempts() == 0);`. `REPORT.md` mtime is 09:23:29, before the C03 review. |
| N-d: C03 `README.txt` | **Closed** | The new C03 `README.md` marks `README.txt` ("PREPARED, NOT RUN") as SUPERSEDED raw history and names `REPORT.md` runs 3 and 4 and `probes/PROBES.md` as the real results. `README.txt` is still present and unchanged (mtime 07:56:14). |

## Raw evidence integrity

In the B3-B, C01, C02 and C03 evidence and review-evidence folders, only two files are newer
than the C03 review file. Both are owned READMEs:
`tests/native/v5-02-x06-b3-b-c02-evidence/README.md` and
`tests/native/v5-02-x06-b3-b-c03-evidence/README.md`. The only directory in those folders
newer than the review is the C03 evidence top level, where the new `README.md` was created.
`probes/`, `probes/raw/`, `raw/` and all three `superseded-*` folders keep their pre-review
mtimes, so no file inside them was added or deleted. The C03 evidence folder holds 1194 files.

## Redaction and commit readiness

- The coder's check, over the records and READMEs, exited 0 with no output
  (`tests/native/v5-02-x06-b3-b-c04-evidence/redact-check.exit`). The gate runner's
  no-argument run exits 1. Its 8225 hits are all in git-ignored `build/`, evidence output and
  `.claude/` paths. The gate runner's file-argument run over the records, READMEs, the six
  source files and the roadmap exited 0 (`raw/run3-redact-files.exit`). I agree that the
  no-argument form has no meaning here.
- I listed the likely commit set (`commit-candidate-files.txt`): the eight B3-B round and
  review records, the roadmap, the six source files, and every top-level `.md` or `.json`
  of 100 KB or less in the nine B3-B evidence and review-evidence folders. The checker over
  that set exits 0 with no output (`redact-check.exit`, `redact-check.log`). A grep for
  `/private/tmp` and `/Users/` over the same files finds nothing. The C01 round has no
  separate record: its dispatch and results live in `V5-02-X06-B3-B.md`, so "C01" in the
  subject is covered by that file and `V5-02-X06-B3-B-C01-review.md`.
- The commit step in the C04 record matches `docs/vernacular/orchestration.md#commits`.
  It stages explicit paths only and uses `git add -f` for evidence reports. It runs
  `redact` and then `check` on the staged files, which must exit 0. It uses `git commit -s`
  with subject `V5-02-X06-B3-B+C01..C04: authentic runtime preparation` and no Claude
  co-author trailer. This review file and the C04 evidence `REPORT.md` should be added to
  the explicit path list. The coordinator still has to update the roadmap opening, which
  describes C03 as request-changes and C04 as pending, before staging. That is the normal
  coordinator outcome step and outside C04's scope.

## Advisory notes (non-blocking)

- **X06B3BC04-N-a.** C03 record line 437 ("Findings that were open at review") still gives the
  final figures as "987 / 41 / `3b22d1378165` / `96e44f54`". It repeats the review's typo in
  a list headed as the final figures. The C04 acceptance table calls this hit a "quoted
  correction note", but it is a paraphrase of the finding, not a correction. The same record
  gives the correct full hash and names the typo at line 351, so a reader is not misled
  about the candidate. An inline "[sic; `3b22d137816e`]" would make the line self-contained.
- **X06B3BC04-N-b.** The C04 evidence `README.txt` indexes only the coder's gate files. It
  does not list `REPORT.md`, `inventory.txt` or `raw/`, which the gate runner added later.
  The C04 acceptance table also says `status.txt` "equals the starting status", but there is
  no starting status file. It equals the start state described in the dispatch, and my own
  `status.txt` is identical to it.
- **X06B3BC04-N-c.** The labelled "History (superseded)" part of the C02 record keeps the old
  masking explanation verbatim ("masked by the pre-capture verify", lines 360-370 and
  455-457), with no inline pointer to the corrected design fact. This is allowed under the
  labelled-history rule, and the current section says "corrected in C04". A one-line pointer
  would still save a reader from following the history text.
- **X06B3BC04-N-d.** The C03 "Changed files in C03" list includes the C03 evidence
  `README.md`, which C04 added. The entry is labelled "added in C04", so it is accurate, but
  the file belongs in C04's list, where it also appears.

## Limits

- This was a records-only review. I ran no build, no test execution, no lint and no
  mutants. The 987/987 execution result, the zero skips and the lint result come from the
  C03 review gate on the same six hashes. The kill and survivor facts come from the
  prober's raw logs, which I read directly.
- I can show that no raw evidence changed after the C03 review only from mtimes and
  directory mtimes. No file-level hash manifest of those folders was taken at C03 review
  time.
- The redaction check is report-only. I did not run `redact`. My commit-set list is my
  reconstruction of what orchestration.md allows. The coordinator's explicit path list is
  what counts.
- macOS only. No Linux or RelWithDebInfo run.

## Evidence files (`tests/native/v5-02-x06-b3-b-c04-review-evidence/`)

`src-diff.sha256`, `files.sha256`, `status.txt`, `ctest-N.log`, `ctest-N.exit`,
`inventory.sorted.txt`, `c03-inventory.sorted.txt`, `inventory.diff`, `inventory.diff.exit`,
`commit-candidate-files.txt`, `redact-check.log`, `redact-check.exit`.
