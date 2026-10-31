# V5-02-RA01-B-W1-C01 independent review (corrective for RA01-B wave 1)

Disposition: **approve**. The corrective closes all nine findings of the
[wave 1 review](V5-02-RA01-B-W1-review.md). The product change is small and
matches the dispatch:

- `runtime.cpp` gains three non-throwing skip statements for D14.
- `authoring.cpp` gains a `remap_missing_leaf` parameter, so the missing-leaf remap applies only to `--diagnostic` (and
  `--from` keeps its directory mapping).

The new and changed tests fail under their named mutants. The wave 1 plus C01 candidate passes the full gate on
macOS and Linux. All 1339 wave 1 names are kept. Production help and the production binary path are unchanged.

I leave four advisory findings and one item that must be done at the commit step. None needs a new candidate.

## Exact candidate

I ran the workflow `identity.py` from the repository root with the pathspec `src tests/native`. The output is saved
verbatim at `tests/native/v5-02-ra01-b-w1-evidence/review/identity-c01-candidate.txt`:

| identity | value |
|---|---|
| HEAD | `307026288615e317d145ba6846c0692929c50a17` |
| sha256(git diff --binary HEAD -- src tests/native), tracked only | `eb11d7a210bfb7036f08ed98b900ef3953fffd6e11c5e4e4b399f0f4602d48eb` |
| combined sha256 (diff bytes, then each untracked path NUL bytes, sorted), pathspec src tests/native, evidence excluded | `de65ea1fe7bab915d77bf3e9329b9e2a226f64ac04e3ddad5bdd409e2dabb3d5` |

The full per-file table is in the saved file. It is equal, file for file, to four other tables:

- the C01 record's "Candidate identity (final)" table;
- the identity table of `probes/PROBES.md`;
- the identity table of gate run 4 (`raw/run4/identity.txt`);
- the hashes in the Linux container context manifest (`raw/run4/linux/container/context-manifest.txt`) for
  `runtime.cpp` `3911b25e...`, `authoring.cpp` `0bd890d8...`, `authoring.t.cpp` `bfa67729...`,
  `external_registration.t.cpp` `5386b9d9...` and `commands.lua` `23b597fa...`.

So the probes, the macOS gate and the container all ran on these bytes.

Against the wave 1 candidate (combined `588df167...`), exactly four files differ: `runtime.cpp`, `authoring.cpp`,
`authoring.t.cpp` and `external_registration.t.cpp`. I compared the pre-fix bytes in `red/snapshot/` with the
checkout:

- The `runtime.cpp` diff adds only `reserved_string_present` and the three `continue` statements. One hunk differs
  only in how the diff tool aligned it.
- The `authoring.cpp` diff adds only the parameter, the three call-site arguments and a comment.
- `commands.lua`, every `.cppm`, `bootstrap.*`, `response.*`, `runtime.t.cpp` and the handlers are byte-identical to
  the wave 1 candidate.

## Findings R1 to R9, row by row

| id | verdict | what I verified |
|---|---|---|
| R1 | closed | See "Probe evidence" below. `probes/PROBES.md` exists on the final identity, with identity before and after the probes equal. The 22 rows run on this candidate each have a raw log with matched/baseline counts (74/74 or 71/71). I06 applies on identical product bytes and is killed. The carried rows rest on an additions-only argument, which I checked independently. |
| R2 | closed | `reserved_authoring_identity` (`runtime.cpp:3448-3491`) skips any command entry whose `name`, `family` or `handler_id` is missing or not a string. It uses only `find` and kind tests, with no `.at()` and no `throw`. `family` is a required descriptor key (`exact()` at :1405 and :3583), so the skip cannot swallow a well-formed entry. E17 is killed in four variants (`C-E17a` to `d`): in each, the named test `RA01-B1 reserved strings in non reserved fields keep their HEAD diagnostic` fails 1 of 73. Do-not-skip-malformed (`C-SKIP-ALL`, and the per-field `C-SKIP-H` and `C-SKIP-F`) is RUN on the final candidate and kills `RA01-B1 reserved pre check skips an entry with any ill typed identity field`. Its variant `{name='observe', handler_id=5}` asserts `internal` / `Invalid external profile protocol`, equal to a `'mine'` control. |
| R3 | closed | `prepare_inputs` passes `false` for `--registry`, `true` for `--diagnostic` and `false` for `observe`. The `--from` directory mapping to `Cannot resolve explicit root directory` does not depend on the flag. A missing `--registry` now reports `io` / `Explicit input must be a regular file`, the unmapped `resolve_explicit_file` message that render gives. `--workspace` (selector) and `--job` (text) never enter the path stage. The killers are C-R3-REG (registry leak: 4 failing tests, named Y) and C-R3-DIAG and C-R3-FROM (remap missing: 3 failing each, named Y), all RUN. The precedence sentence is in the wave 1 record at :72 and in the `authoring.cpp` comment. Both new mapped messages are rows of the authoring-contract Messages table (:489, :492), and `Cannot resolve explicit root directory` is an existing message (`host_path.cpp:30`, `response.cpp:196`). |
| R4 | closed | Wave 1 record row I35 (:480) names the single-edit mutant as equivalent and I35b as the killer. `superseded-1/raw/I35b.log` shows it killed, and `probes/raw/I35.log` shows the single edit surviving at 71/71. Row I52 (:504) says the uppercase `Orchestrator` projection is refused with a non-collision diagnostic, and that the test body changed after red. |
| R5 | closed | Both moves are named in four places, each with numbers, measuring run and authority (the operator decision of 2026-10-30 as carried by the dispatch): the wave 1 record note at :80, row I68, the `c02_off_boundary` comment (`external_registration.t.cpp:4575-4582`) and wave 1 plan D20. The first move is macOS 228586 to 227593 and Linux 228612 to 227619, measured by gate run 1. The second is macOS 227593 to 227525 and Linux 227619 to 227551, after the 68 bytes of `fresh_options` padding. The `git diff` of the constants shows exactly 228586 to 227525 and 228612 to 227551. `commands.lua` is unchanged by C01, so no new re-pin is needed. `X08 C01 role preparation outputs-off boundary pins the request-record conversion charge` passes in all four gate configurations and in my focused runs. C-R5-PIN shows that the pin still bites. "orchestrator's re-pin" no longer appears in either `REPORT.md`. |
| R6 | closed | The wave 1 record contains the D13 row and the wave 2 note at :59-78. Work content item 4 carries the "SUPERSEDED by D13" marker (:48). `V5-02-RA01-A.md` is in the owned list (:190) with its hash (:291). |
| R7 | closed, with a commit-step item | `REPORT.md:68-69` and `plan/INVARIANTS.md:142` of the wave 1 evidence now use placeholders. I ran `redact-local-paths.py --repo-path "$PWD" --home "$HOME" check` on 68 files: every changed or untracked source and doc, the three round records and the earlier review, and every `.md`, `.txt` and `.jsonl` file outside `raw/` and `snapshot/` in both evidence folders. 67 are clean. The one failure, `tests/native/v5-02-ra01-b-w1-evidence/red/summary.txt`, is not on the R7 list and not in the staging precedent; see C01-R3. The list and output are saved under `review/redaction-check-*.txt`. |
| R8 | closed | `docs/architecture.md:774-778` states the live identity-conflict refusal: reference, with the field being the registration ordinal. `:1625-1626` states that the union grows from 18 to 23 and is live in the production binary. |
| R9 | closed | Wave 1 plan D7, D8, D13 and D14 are tagged `GAP`. All six `D23` occurrences in `x07-addendum.md` read "RA01-B wave 1 plan D23". |

## Probe evidence

I did not apply mutants. I read every final raw log named in `PROBES.md` Part 1 and the superseded-1 logs for the
carried rows that the acceptance names.

- **Run rows.** For C-SKIP-H, C-SKIP-F, C-SKIP-ALL, C-R3-REG, C-R3-DIAG, C-R3-FROM, C-R3-NARROW, I42, E08 to E11,
  X-C01-04 and X-C01-06, the named killer is among the `Failed` tests. Each run's matched count equals its baseline
  for the same selection, and each log ends with a verified restore. The first C-SKIP-ALL attempt was `build-failed`.
  It stays in `results.jsonl` and was not counted.
- **Survivors.** C-SKIP-N and C-PROJ-EQ survive because `collect_final_descriptors` refuses a non-string name or id
  through `text_field` before the pre-check runs. E07, E15, the single-edit I35 and X-C01-10 survive with reasons I
  checked against the code. C-NOAT is a stated limit.
- **I06.** `superseded-1/raw/C-I06.log` shows the diff applied to `runtime.cpp` and `commands.lua` with no
  `PATCH FAILED`. The named `RA01-B1 authoring option signatures are exactly the RA-1 table` is among 21 failing
  tests. Both mutated files and the killer file `authoring_catalog.t.cpp` are byte-identical in the final candidate.
- **Carry-forward.** Superseded-1 ran on combined `b15bb717...`. Its per-file table differs from the final one only
  in `external_registration.t.cpp` (`23b7e4fa...`) and `authoring.t.cpp` (`50e9333b...`). I found both earlier
  files in the session scratch copy, confirmed their hashes and diffed them against the checkout. The diff is saved
  at `review/c01-superseded1-to-final-test-diff.txt`. It has no removed lines:
  - In `external_registration.t.cpp`, the only change is the new TEST_CASE appended at :5419. The bytes of the E17
    killer test case, and of every other existing test case in the file, are therefore identical.
  - In `authoring.t.cpp`, the changes are three added scoped blocks. Each builds fresh `inputs` and `ra01_counts`,
    creates no file and only reads.

  An added assertion can only add failures, so every carried kill holds on the final bytes. I accept this as the
  byte-identity proof that acceptance 2 asks for: the E17 killer test case is byte-identical, though its file is
  not. Every wave 1 invariant row (I01 to I66 with letter variants, E01 to E19) appears in `superseded-1/results.jsonl`
  or in the final run. The exceptions are the documentation rows I24, I34 and I59 to I64, which are not
  mutation-testable.
- **Red-first.** `red/snapshot/` hashes match `RED.md` (`candidate.diff` `7b880280...`, `SHA256SUMS`
  `16f5ec12...`). The snapshot's `authoring.cpp` is the wave 1 `e0e0eb11...`, without `remap_missing_leaf`. t2 fails
  at `external_registration.t.cpp:5411-5414` (identity conflict instead of `internal`). t5 to t8 fail on
  `authoring.t.cpp` assertions. These are behavioural failures, not compile breaks. The test without a red record,
  `...continues past a malformed entry sorted before a reserved entry`, is a guard and not a regression test:
  - On the wave 1 code it would pass. With no skip at all, an ill-typed `family` reads as empty, so the loop moves
    on and still reaches `observe`.
  - Its evidence is the kill of X-C01-06.

  The same holds for the added `--from` text-validation assertions (X-C01-04). I accept both without red runs.

## Gate (verified from raw logs and my own runs)

- **macOS (`raw/run4/`).** Every `.exit` is 0. `ctest-debug.log` and `ctest-release.log` show
  `100% tests passed out of 1344`. The only "skipped" hits are two RA01-B1 test names and
  `command load refuses forged family order skipped ...`, which passed. All four no-work logs read
  `ninja: no work to do.`, and `inv-debug.txt` equals `inv-release.txt`.
- **Linux container (`raw/run4/linux/`).** Every `.exit` is 0, including `docker-build` and `builder-prune`. Debug and
  RelWithDebInfo each show `100% tests passed out of 1344`. `x07.two_process_cas_one_winner` (#1335) passed in both.
  The no-work logs are clean, and the two Linux inventories are identical. `manifest-compare.txt` shows the host
  manifest equal before and after, and equal to the container context.
- **My own runs.**
  - Lint (`python3 scripts/rewrite-check.py lint --build-dir build/rewrite-debug`) exited 0 over 163 files.
  - `cmake --build` of `planar_tests` and of the default target printed `ninja: no work to do.` in both build trees.
  - `ctest -R 'RA01-B1|X08 C01' --no-tests=error` passed 74 of 74 in Debug and in RelWithDebInfo.
  - `ctest -N` gives 1344 identical names in both configurations. Against
    `tests/native/v5-02-ra01-b-w1-evidence/inventory.txt` (1339), 0 names were removed and 5 added, and the 5 equal
    `new-names.txt`.
- **Production unchanged.** With scratch `HOME` and `PLANAR_DB`, `planar --help` gives the same bytes in Debug and in
  RelWithDebInfo (sha256 `211083b3...`). Those bytes equal the HEAD literal in `bootstrap.cpp` once its two-part
  `load` string is joined. All six authoring verbs exit 2 with the existing usage envelope.
  `ninja -t commands planar` contains no `PLANAR_AUTHORING_BUNDLE` in either configuration.
- **Diagnostics.** No message text was added or changed. `X07 command sources add no diagnostic outside the closed
  table` passes.

## Findings

### RA01-B-W1-C01-R1 (advisory, records): X-C01-09 is an equivalent mutant, not an open coverage gap

The record leaves X-C01-09 (drop the `ignored ||` arm in `prepare_explicit`) as an open gap. It suggests an
ENAMETOOLONG leaf as a possible fixture. That fixture cannot work.

The remap guard first requires `resolve_explicit_file` to return `Explicit input must be a regular file`, and
`host_file` (`host_path.cpp:9-16`) gets there through the throwing `symlink_status`. I measured with the pinned libc++
(`review/x-c01-09-symlink-status.txt`):

- The throwing form throws for ENAMETOOLONG. The function then returns `Cannot resolve explicit input path` and the
  guard is false.
- For ENOTDIR and ENOENT, the throwing form returns `not_found` without throwing. The non-throwing call then sets the
  error code and also returns `not_found`, so both arms agree.

The arm therefore differs only if the filesystem changes between the two `symlink_status` calls on the same absolute
spelling. That is a race no single-threaded fixture can build.

At commit, restate the X-C01-09 row and the Remaining-limits bullet: the mutant is equivalent outside a concurrent
filesystem change, and the arm is defensive. Coordinator acceptance row 7 is then met.

### RA01-B-W1-C01-R2 (advisory, records): the leaf-below-a-regular-file assertion is in a different test than recorded

The C01 record and `PROBES.md` place the `<regular file>/leaf` assertion in
`RA01-B1 explicit path stage keeps the existing messages`. Those passages are Gate runs item 2, the red-first table,
row X-C01-09, and the PROBES.md verdict and red-first table. The bytes put it in
`RA01-B1 missing leaf remap applies only to diagnostic and from`, at `authoring.t.cpp:923-930`. This does not change
any result, because the assertion is a behaviour pin either way. Correct the test name in the record.

### RA01-B-W1-C01-R3 (required at the commit step, evidence): do not stage an unredacted `red/summary.txt`

`tests/native/v5-02-ra01-b-w1-evidence/red/summary.txt` contains absolute checkout paths in its failure-location
column, and the redaction check refuses it. It is not on the R7 list, and the f78f8eb4 precedent (reports, READMEs,
manifests, inventory, plan and probe notes) does not stage it. But the wave 1 record cites it as the source of its
red-check column (:431), and the wave 1 README lists it.

At commit, either leave it unstaged and say so in the README, or redact the paths before staging it. Re-run the
check on the exact staged list.

### RA01-B-W1-C01-R4 (advisory, records): the C-I14 row name is ambiguous

There are two rows with the same name:

- The C01 invariant table row C-I14 means plain I14 (`S2 names.insert(name)`), killed in `superseded-1/raw/I14.log`.
- `superseded-1/PROBES.md` uses C-I14 for the I24 masking-contrast row, which survives by design.

`PROBES.md` Part 2 also lists C-I14 twice, and its ranges leave out the plain I12a and I12g rows. Both were run and
killed in superseded-1. At commit, write "I14 (row C-I14)" in the carried column, and list I12a and I12g explicitly.

### RA01-B-W1-C01-R5 (advisory, evidence process): first red logs were replaced

`red/raw/NOTE.txt` says that the first `t*.log` runs, whose ctest regex lacked the `replacement.` prefix and matched
no test, were replaced by the corrected runs. They carried no result, so nothing material was lost. Still, standing
lesson 6 says to keep such logs, for example as `t<n>.attempt1.log`. Note this in `RED.md`. No re-run is needed.

## Limits of this review

- I did not apply mutants, and I re-ran neither the full ctest nor the Linux gate. The full-gate figures come from the
  run 4 raw logs, which I read rather than took from the summary. My own runs were the lint, the no-work builds, the
  74-test focused selection in both configurations, the inventories, and the production help and verb checks.
- The R5 authority is the operator decision of 2026-10-30 as recorded in the C01 dispatch. I have no other record of
  the operator's own words.
- The carry-forward proof uses the superseded-1 test files kept in the session scratch copy (`coder-T-AT`). I matched
  their hashes to the superseded-1 identity table before diffing.
- The libc++ measurement for R1 ran on macOS arm64 only. The Linux container uses the same pinned libc++ rule, but I
  did not measure it there.
- I ran the candidate binary only with a scratch `HOME` and `PLANAR_DB`. I ran no script under `tests/native/`, and
  I edited nothing outside this review file and `tests/native/v5-02-ra01-b-w1-evidence/review/`.
