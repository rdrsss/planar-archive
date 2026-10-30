# V5-02-X08-C01-R7 independent review (whole X08 candidate with C01, at the R7 records and validator repair)

Disposition: **request-changes (records only)**.

The substance of R7 is sound. The product, test and acceptance bytes are identical to the R6 gate. The validator now covers the final
1260-name tree and its G06 check is real. The I16b re-probe was run against the right test and killed the mutant, and the C01-R6
findings R1, R2, R4, R6 and R7 are closed with evidence.

Two problems block the commit, and both are records:

- Several R7-lineage records that would be committed still say R3 is open or that the I16b re-probe is "still owed". This includes the
  roadmap opening, the R7 gate report and the R7 invariant table. The dispatch's own acceptance clause "records and roadmap agree" is
  therefore not met. The round record admits this in its acceptance table and leaves the fix to "the owner".
- The R7 `INVARIANTS.md` has no row for several closed findings and acceptance criteria.

Nothing in source, tests, CMake, the validator or the raw evidence needs to change. I did not commit.

## Candidate identity

I ran `identity.py src CMakeLists.txt tests/acceptance tests/native/v5-02-x08-support` from the repository root
(`tests/native/v5-02-x08-c01-r7-evidence/review/identity.txt`). Its output, verbatim:

| identity | value |
|---|---|
| HEAD | `48b8e526764a76de1392d397bb21659bf63e8b8d` |
| sha256(git diff --binary HEAD -- src CMakeLists.txt tests/acceptance tests/native/v5-02-x08-support), tracked only | `98c72551a86730f93f5c8b23464ef4c885f382464efd7d6aed4847c6cea88571` |
| combined sha256 (diff bytes, then each untracked path NUL bytes, sorted), pathspec src CMakeLists.txt tests/acceptance tests/native/v5-02-x08-support, evidence excluded | `6721925ef8a3c6002ddb16dba2352b4a65d8eed0da2df786b47602aa59ad0973` |

| file | state | sha256 |
|---|---|---|
| `CMakeLists.txt` | modified | `402c84f35ac0df395d5e3ff1d132b443fab4e9e6b90c56cc00edfacf7534f348` |
| `src/cmd/planar/bootstrap.t.cpp` | modified | `558c811d5fe670e874a5b446f1a10625f2f7ec997f95d75ed72f6b5720291b6d` |
| `src/engine/capture/host_path.cpp` | modified | `9207c2b79ecc9fce3064c865bb8e253931795a503244304efcc172a5eb8ca7d3` |
| `src/engine/capture/host_path.t.cpp` | modified | `ddafdd85acc47e0b0c400e9f19158709029fc4a988e06edf235c89c0137308e8` |
| `src/engine/runtime/commands.t.cpp` | modified | `f36e6a3cf2f52ab54ac24a0a7284109a832668d063516e959bc3592159ffa072` |
| `src/engine/runtime/external_registration.t.cpp` | modified | `22afc6e0c8f15a9dd705538c898b8bd263604f3c5da6fed673d04ba3f556b605` |
| `tests/acceptance/x08_adversarial.py` | untracked | `fe368c77f4671c102df2baaa53bcd66ad295bdd46b9a228c87702d5acb87d1be` |
| `tests/acceptance/x08_cli.py` | untracked | `c7f3bfad84b758eff8ae89de63889b15fb27517c46685a44356cdefffda52e7c` |
| `tests/acceptance/x08_e12.py` | untracked | `029fdd1cf0e6e4d37de9a78914248267a9b7fa98c507fd08afa32a11a5beffe4` |
| `tests/acceptance/x08_recipe.py` | untracked | `bb763c443c87f95a9a184eed480d742db1d9ec93ce5e22562f9ef027572b5804` |
| `tests/acceptance/x08_timeout.py` | untracked | `337e8c0ee230728c9f725e105c93456fc2c054c3cb58fe1f959d04f396c0c5c9` |
| `tests/native/v5-02-x08-support/generate.py` | untracked | `0211ae98cd8064385b628d8f89e744bbfb314de95f8489032efbc8bc364b7420` |
| `tests/native/v5-02-x08-support/validate.py` | untracked | `4aa57a9dc1e0ab32f25fa8c65ec0be309a650ec3756bc0ff9fd17f2ea0e60a4c` |
| `tests/native/v5-02-x08-support/x08.cmake` | untracked | `5c6d671fac9f016be46ccd136c529e0d3be55b8c866aa234247deb868948c7ea` |

This output is byte-identical to the round record's "Final candidate: V5-02-X08-C01-R7" table and to the prober's
`probes/raw/00-identity.txt`. I also ran the narrower pathspec `src CMakeLists.txt tests/acceptance`. It gives combined
`92891b72bf602b03cddaf44f85d8aef357fbf86d5d31d5a14134f1b8509c494b`, equal to the R7 dispatch, `raw/gate-identity.txt` and
`raw/g2-identity.txt`.

The five C++ files match the R6 gate run 2 identity (`c01-r6-evidence/raw/run2/identity.txt`) file for file: `external_registration.t.cpp`
`22afc6e0...`, `host_path.cpp` `9207c2b7...`, `bootstrap.t.cpp` `558c811d...`, `commands.t.cpp` `f36e6a3c...` and `host_path.t.cpp`
`ddafdd85...`. The R6 run 2 manifest records the acceptance and CMake hashes, and they are unchanged. The light gate is therefore valid,
and the R6 run 2 logs remain the final product test logs.

The default pathspec (`src tests/native`) gives tracked `505630cc...` and combined `6b8e55d2...`, as the record states. The only
differences from R6 are `validate.py` and one prober-added row in `tests/native/path-redaction.json` (for
`c01-r7-evidence/probes/tools/driver.py`). I confirmed that no CMake or test code reads `path-redaction.json`: only
`scripts/redact-local-paths.py` reads it.

## What I checked and found sound

- **R1, the validator tree.** I compared each assembled file with `c01-r6-evidence/raw/run2` by name:
  - The macOS ctest logs, inventories, build and no-op logs, lint, and redaction exits and file list are byte-equal.
  - `diff -rq` of `linux/` against `run2/linux` differs only by the host-side df, docker and builder-prune files, which the validator
    does not read, and by `identity-before.txt` and `identity-after.txt`. Both of those are byte-equal to `run2/identity.txt`, as the
    record states.
  - The run-4 tree is intact under `raw/superseded-3/{raw,linux}`. It has the same 24 raw names and 32 Linux files, and its ctest
    summaries say 1258.
  - `superseded-1` and `superseded-2` are still present (163 files). The files their reports cite exist, apart from glob and
    relative-sibling references.
- **The validator.** `R6_ADDED_NAMES` holds exactly the two R6 names. `EXPECTED_TOTAL` is derived as 1227 + 28 + 3 + 2 = 1260, and the
  self-test guard asserts 1260. G01 requires every X08, C01 and R6 name.
- **Validator reruns** (`review/selftest.{log,exit}` and `review/validate.{log,exit}`):
  - `--self-test` exits 0. The new rows are each refused by the intended gate, and the one accepted variant (the dpkg-list fallback) is
    accepted.
  - `--evidence tests/native/v5-02-x08-evidence --repo .` exits 0, with G01 to G09 PASS. G01, G02, G04 and G05 each report
    "1260 tests, 0 failed, 0 skipped", and G06 reports "9 platform rows". My self-test log is byte-equal to `raw/g2-selftest.log`.
- **The CAS rule.** `ctest_problems` refuses any per-test status that is not Passed, as well as any Skipped or Not Run marker.
  So a `x07.two_process_cas_one_winner` failure in any of the four logs fails the run, which meets that acceptance clause.
- **R2, G06.** `gate_g06` now requires rows G06a to G06i, refuses extra, repeated and empty rows, and checks each `pass_line` against
  `linux/container/debug-ctest.log`. In my check (`review/g06-passline-check.txt`), all nine pass_lines occur as a whole line, exactly
  once, both in the validator tree's Linux debug log and in `run2/linux/container/debug-ctest.log`. Each one also names the test in its
  row's `test_or_log`.
  - G06i cites `external_registration.t.cpp:4557`, which is the `#if defined(__APPLE__)` that selects 4742/228586 against 4737/228612.
  - Self-test mutants for a stale first line, a stale last line, a line that occurs only in the release log, and a missing G06i row are
    each refused by G06. The prober's `R7-G06-STALE`, `R7-SELF-noop`, `R8-G06-NOPASSCHK` and `R7-G06-STALE-first` logs show that these
    rows can fail.
- **R3, I16b.** I read `probes/superseded-1/raw/R7-I16b.log`:
  - The diff raises the `profile.cpp` module bound from 1048576 to 16777216 (diff sha256 `9d19b2f7...`, the same mutant as `R6-I16b`).
  - The ctest filter was `profile capture module ceiling|x08\.adv_limits_cli`, and 2 tests matched.
  - `replacement.profile capture module ceiling checked before observed module read` fails on the assertion `REQUIRE_FALSE( result )`
    at `profile.t.cpp:88`, while `x08.adv_limits_cli` passes. The restore and rebuild are recorded.
  - The honest result is KILLED, and the record's final section, `probes/PROBES.md` and the R6 README now say so.
- **R4, `docs/testing.md`.** The platform table has the test-only `external_registration.t.cpp:4557` row, and the C01 paragraph covers:
  - per-platform pins 4742/4737;
  - the outputs-off boundary 228586/228612 with window 8 and an explicit re-measure rule (both configurations, each platform, never
    widen and never take the smaller value);
  - the withdrawal of the "keep the smaller value" rule;
  - the 1260-name breakdown;
  - the G06 occurrence check.
- **R6, redaction.** Both evidence `REPORT.md` files now quote `<home>`, and the R6 REPORT says 23 files (`redact-files.txt` has 23
  lines).
- **R7, run numbering.** The repair report's numbering note matches the evidence: `REPORT-run5.md` says "GATE PASSED", and `REPORT.md`
  is titled "gate run 6" and says "GATE FAILED". The CAS history is also accurate: the only `x07.two_process_cas_one_winner` failure in
  any X08 evidence is `c01-evidence/raw/05-ctest-release.log:2499` (C01 gate run 1, Release, 0.18 s). It passed in all four R6 run 2
  logs (1.21, 0.51, 0.97 and 0.43 s) and in the R7 `gate-ctest-debug.log`.
- **Names.** The live `ctest -N` inventory has 1260 names and equals the R7 `inventory.txt` after sorting. Against the committed X07-C01
  inventory (1227), no name is removed and 33 are added. Against the C01 inventory (1258), no name is removed and exactly the two R6
  names are added. `x08.validator_selftest` keeps its name.
- **Diagnostics.** No C++ file changed since R6 (see the hashes above), so every diagnostic is byte-identical.
- **Lint.** `python3 scripts/rewrite-check.py lint --build-dir build/rewrite-debug` exits 0 ("Formatting and Doxygen checked 147 active
  source files"; `review/lint.{log,exit}`).
- **Probe table.** All 19 R7 validator rows in `probes/PROBES.md` have a `.json` with `result` KILLED, `restored` true and
  `rebuild_exit` 0. I spot-checked `R7-G06-STALE-first`, `R7-TOT-R6` and `R8-TC-DPKG-NOFALLBACK` against their logs.

## Findings

### V5-02-X08-C01-R7-R1 (blocking, records): R7-lineage records still say R3 is open; records and roadmap do not agree

The R7 dispatch's acceptance requires that "records and roadmap agree". The round record's own acceptance table marks R5 and R8
"met" with stale text. The following text, all of which would be committed, contradicts the final section of the record:

- `docs/vernacular/roadmap.md:35` (the X08 bullet in the opening) says "the prober re-run of `R6-I16b` is still owed" and labels the
  candidate "request-changes 2026-10-29" with "Next: C01-R7 review". The next-work row at `:49` still lists "prober re-run of R6-I16b"
  as outstanding work.
- `tests/native/v5-02-x08-c01-r7-evidence/REPORT.md:44` marks "R3 I16b prober re-run" as "OPEN, not claimed closed here", and `:63` says
  "R3 (I16b) is still open pending the prober."
- `tests/native/v5-02-x08-c01-r7-evidence/plan/INVARIANTS.md` row I33 has status "OPEN: prober owed", and its check column still names
  the pre-probe command. This is the round's own invariant table, and it shows a closed finding as open.
- `tests/native/v5-02-x08-c01-r6-evidence/probes/PROBES.md:120-126` ("R7 note") ends by saying that the `R6-I16b` and `R6-I16b-both`
  survivor rows "are NOT accepted as equivalent" until the re-run. It has no closing sentence. The R6 README carries a superseding note,
  which is enough for `REPORT.md:56,76,91`, but the note inside PROBES.md should state the outcome. It should also say that
  `R6-I16b-both` was not re-run and stands as a survivor because of the `runtime.cpp` guard, which is what the final record says.

Fix, records only: correct these four places to the final outcome. `R7-I16b` was KILLED by
`replacement.profile capture module ceiling checked before observed module read`. `R7-I16b-lower` was killed by that test and by
`x08.adv_limits_cli`. `R6-I16b-both` remains a survivor because of the `runtime.cpp` guard.

At approval time, update the roadmap opening X08 bullet to the approved state and remove the R7 next-work row, keeping X07-C02 and then
RA01. A gate report written before the probe can keep its original text if it carries a dated superseding note at the top, and the
same applies to I33.

### V5-02-X08-C01-R7-R2 (blocking, records): the R7 invariant table lacks rows for several closed findings and acceptance criteria

`plan/INVARIANTS.md` has rows I17-R7, I26-R7 and I29-R7, and I30 to I33. They cover R1, R2, R3 and R8, plus the dpkg fallback. There is
no row for:

- R4, the `docs/testing.md` content: pins, boundary, re-measure rule, 1260 names and the G06i table row.
- R5, the roadmap figures.
- R6, the redaction of both `REPORT.md` files and the count of 23.
- R7, the run numbering and the CAS history statement.
- The acceptance clause that `src/`, CMake and `tests/acceptance` are byte-identical to R6, which is what makes the light gate valid.
- The acceptance clause that a `x07.two_process_cas_one_winner` failure in any run fails that run. This one has a real check
  (`ctest_problems` refuses any result that is not Passed) and an existing self-test row ("failed=1 in the macOS debug log", G01), so it
  can cite a mutant.

Most of these are prose invariants with no code mutant. In that case the row should say so: give the check (the reviewer reading, the
redaction tool's exit, identity per-file hashes) and write "mutant: none, records only". The table should not omit them.

Fix, records only: add the rows, and correct I33 as in R1.

### V5-02-X08-C01-R7-R3 (required at commit time): one evidence inventory still holds the local home path

The R7 gate's redaction list (`raw/g2-redact-files.txt`, 25 files) covers source, docs, records and the R7 REPORT, but none of the other
evidence files that earlier X-round commits included (`REPORT*.md`, `inventory.txt`, `plan/*.md`, `README.md`, `probes/PROBES.md`). It
also leaves out `tests/native/path-redaction.json`, which the X07 commit did include.

I ran the check over that wider set of 50 files (`review/redact-files.txt`). It exits 1 (`review/redact.{log,exit}`) with a single
finding: `tests/native/v5-02-x08-c01-r6-evidence/inventory.txt` line 1 is `Test project <home>/...`, with the real home path. The R7
and validator-tree inventories already use `<repo>` or have no header.

Fix: either leave this file out of the staged set, or redact it with `scripts/redact-local-paths.py redact` so that the manifest records
both hashes. Then run `check` on the exact staged set before committing, as the acceptance already requires.

### V5-02-X08-C01-R7-R4 (advisory): the G06 occurrence check is a substring test and is not tied to the row's test

`gate_g06` accepts a `pass_line` if its stripped text occurs anywhere in `linux/container/debug-ctest.log`. A pass_line reduced to
`Passed`, or quoting a different test's line, would pass. Every current row is a whole, unique line that names its own test, so the
evidence is honest today. A stronger check would require a whole-line match, a `Passed` status and the `test_or_log` name inside the
line, with one self-test mutant. Not blocking: the acceptance asks only that the lines "occur verbatim", and they do.

### V5-02-X08-C01-R7-R5 (minor, records): small wording slips

- `probes/PROBES.md` ("Skipped") calls the CAS test `x08.two_process_cas_one_winner`. Its name is `x07.two_process_cas_one_winner`.
- `docs/testing.md` says the outputs-off boundary has the "same value in Debug and RelWithDebInfo" on macOS. The pin comment at
  `external_registration.t.cpp:4558` says "measured on macOS in Debug", and I found no separate macOS RelWithDebInfo measurement in the
  evidence. The RelWithDebInfo ctest passing shows only that the value lies within the window of 8. Either cite the measurement or say
  "within the window in RelWithDebInfo".

## Invariant table cross-check

| row | mutant really breaks it | listed test killed (PROBES.md) | result |
|---|---|---|---|
| I17-R7 (no name removed, 1260) | `R7-G01-NOPRIOR`, `R7-G01-R6names` | `x08.validator_selftest` (prior name missing; R6 name renamed with the count intact) | real |
| I26-R7 (total 1260) | `R7-TOT-R6` (`R6_ADDED_NAMES` emptied) | count guard in the self-test | real |
| I29-R7 (pass_line in the debug log, G06i present) | `R7-G06-STALE`, `-STALE-first`, `-STALE-wronglog`, `R8-G06-NOPASSCHK`, `R7-G06-NOROWI`, `R7-SELF-noop` | `x08.validator_selftest` | real |
| I30 (tree is run 2; superseded kept) | none (data) | directory comparison, done in this review | holds |
| I31 (dpkg-list fallback) | `R7-TC-DPKG`, `R8-TC-DPKG-RC`, `R8-TC-DPKG-NOFALLBACK` | `x08.validator_selftest` | real |
| I32 (re-measure rule) | none (prose) | `docs/testing.md` read | holds |
| I33 (R6 survivors, I16b) | `R7-I16b` | `profile capture module ceiling ...` killed | the row's status is stale (R1) |
| R4, R5, R6, R7, light-gate identity, CAS rule | no row | n/a | missing (R2) |

This round adds no regression test, so no red-first snapshot applies. The validator rows were proven by the prober's mutants in a
scratch copy.

## Limits of this review

- I applied no mutant. Mutation results are the prober's logs, which I spot-checked: `R7-I16b`, `R7-G06-STALE-first`, `R7-TOT-R6` and
  `R8-TC-DPKG-NOFALLBACK`.
- I did not re-run ctest, the release configuration or the Linux container. The product conclusions rest on the R6 run 2 logs, whose
  identity matches the candidate, as the R6 review accepted.
- I re-ran identity, lint, both validator commands and a wider redaction check, plus the inventory comparisons above.
- The carried limits stand: RelWithDebInfo and x86_64 Linux mutants were not run, and G08 and G09 check anchors only.

## Evidence written by this review

`tests/native/v5-02-x08-c01-r7-evidence/review/`:

- `identity.txt`
- `selftest.log` and `.exit` (exit 0)
- `validate.log` and `.exit` (exit 0, 1260 names)
- `g06-passline-check.txt`
- `lint.log` and `.exit` (exit 0; home path replaced with `<home>`)
- `redact-files.txt`, `redact.log` and `.exit` (exit 1, one finding, R3)

## Records re-check (2026-10-29)

Disposition: **request-changes (records only)**.

I re-checked the "Records fix after review" section of `docs/vernacular/rounds/V5-02-X08.md` against the bytes. The fixes for R1, R2 and
R5 are correct. The R3 decision, however, rests on a premise that is false: the file it leaves out is cited by a record that will be
committed. That leaves a new false claim and a dangling evidence citation, and both are fixed by editing records.

### Nothing outside records changed

- `identity.py src CMakeLists.txt tests/acceptance tests/native/v5-02-x08-support`
  (`tests/native/v5-02-x08-c01-r7-evidence/review/recheck-identity.txt`) is byte-equal to my review's `review/identity.txt`. Combined
  `6721925ef8a3c6002ddb16dba2352b4a65d8eed0da2df786b47602aa59ad0973`, tracked `98c72551...`, and all fourteen per-file hashes are
  unchanged.
- The narrow pathspec `src CMakeLists.txt tests/acceptance` still gives combined `92891b72bf602b03cddaf44f85d8aef357fbf86d5d31d5a14134f1b8509c494b`
  (`review/recheck-identity-narrow.txt`). The default `src tests/native` still gives tracked `505630cc...` and combined `6b8e55d2...`
  (`review/recheck-identity-default.txt`). All three exits are 0.
- Files under `tests/native`, `docs`, `scripts`, `src` and `CMakeLists.txt` that are newer than my review's last evidence file
  (`review/recheck-changed-files.txt`) are exactly the seven files the fix section names, plus this review file. No validator, raw log,
  inventory or probe JSON changed.
- `docs/testing.md` and `docs/vernacular/roadmap.md` have the file sha256 values the fix section states. The tracked diff over its six
  pathspecs hashes to the stated `11dec424...`.

### Earlier findings

| finding | re-check | status |
|---|---|---|
| R1 | Roadmap candidate bullet (line 35) and next-work row (line 49) now say `R7-I16b` KILLED by `profile capture module ceiling checked before observed module read`, and that `R6-I16b-both` stays a survivor because of the `runtime.cpp` guard. R7 `REPORT.md` row R3 (line 44) and the Limitations bullet (line 63) say the same; its citation `profile.t.cpp:368` is that `TEST_CASE`. INVARIANTS I33 status is KILLED. The R6 `probes/PROBES.md` "R7 note" now records the outcome. A grep for "owed", "OPEN" and "still open" over these files finds no stale I16b text. The historical R6 `REPORT.md` still calls I16b an equivalent survivor, but the R6 `README.md:13` marks that verdict as superseded, which I accepted in the review | closed |
| R2 | INVARIANTS rows I34 to I39 exist; the prose-only rows say "mutant: none, records only". I38's combined hash matches `raw/gate-identity.txt:5` and `raw/g2-identity.txt:5`. I39's check is real: `ctest_problems` (`validate.py:177-211`) refuses failed counts, a percent other than 100, Skipped or Not Run, and any non-Passed result. The self-test rows "skipped=1" and "failed=1 in the macOS debug log" (`validate.py:864`, `:867`) exist, and `scripts/linux-gate.sh` has no exclusion filter | closed (one wording slip, RC2) |
| R3 | See RC1 | not closed |
| R4 | Advisory, accepted unchanged | n/a |
| R5 | R7 `probes/PROBES.md:91` names `x07.two_process_cas_one_winner`. The only remaining `x08.two_process...` is in `probes/superseded-1/PROBES.md`, a superseded record that stays verbatim. `docs/testing.md:1838` now says the macOS outputs-off value was measured in Debug only and that the RelWithDebInfo pass shows only that it is within the window of 8 | closed |

### New findings

**RC1 (blocking, records): the R3 decision says the inventory "is not referenced by ... any record", but the R7 gate report cites it.**
`tests/native/v5-02-x08-c01-r7-evidence/REPORT.md:34` cites `../v5-02-x08-c01-r6-evidence/inventory.txt` as the basis of its R7
inventory comparison ("the most recent committed list"). If that file is left out of the staged set as decided, the committed report
cites evidence that is not in the commit. The fix-section row R3 (`V5-02-X08.md:909`) and the roadmap's "these are fixed" then
overstate the result. Evidence: `review/recheck-r3-inventory.txt`.

Either remedy closes this finding:

- (records only, preferred) Repoint `REPORT.md:34` to `../v5-02-x08-c01-r6-evidence/raw/run2/inv-debug-sorted.txt`, and correct
  the R3 row. That file has 1260 names and no home path. Under `LC_ALL=C`, its name set equals both the R6 `inventory.txt` names and
  the R7 `inventory.txt` (zero differing lines each). The same edit can drop "committed", because the R6 evidence is not committed yet.
- Redact the file with `scripts/redact-local-paths.py redact` and stage it. This matches the committed precedent
  (`v5-02-x07-c01-evidence/inventory.txt` line 1 reads `Test project <repo>/build/rewrite-debug`), but it changes an evidence file.

In both cases, the commit-time `check` over the exact staged set, including `tests/native/path-redaction.json`, still applies.

**RC2 (minor, records): I36 misstates the 23.** INVARIANTS row I36 says "the count of redacted reports is 23". In fact, 23 is the
number of files in the R6 redaction check list (`c01-r6-evidence/REPORT.md:27`, `redact-files.txt`), while the reports that were
redacted are the two `REPORT.md` files (R6 finding R6, `V5-02-X08.md:666`). Reword it to "the R6 redaction file count is 23".

### Limits of this re-check

I ran no build, ctest, validator, lint or mutant. The product and validator bytes are unchanged, so the review's runs stand. I read
the edited bytes directly and grepped for stale wording. I did not re-review prose beyond the fix section's claims.

### Evidence written by this re-check

`tests/native/v5-02-x08-c01-r7-evidence/review/`: `recheck-identity.txt`, `recheck-identity-narrow.txt`, `recheck-identity-default.txt`,
`recheck-identity.exit` (0, 0, 0), `recheck-changed-files.txt` and `recheck-r3-inventory.txt`. None contains a home path.

## Records re-check 2 (C01-R8, 2026-10-30)

Disposition: **approve (records only)**, with one status note for commit time (RR1).

I re-checked the "C01-R8 records fix (2026-10-30)" section of `docs/vernacular/rounds/V5-02-X08.md` against the bytes. Both open
findings are closed, and nothing outside the three named records changed.

### Nothing outside records changed

- `identity.py src CMakeLists.txt tests/acceptance tests/native/v5-02-x08-support` (`review/recheck2-identity.txt`) is byte-equal to
  `review/recheck-identity.txt` and `review/identity.txt`. HEAD `48b8e526...`, tracked `98c72551...`, combined
  `6721925ef8a3c6002ddb16dba2352b4a65d8eed0da2df786b47602aa59ad0973`, and all fourteen per-file hashes are unchanged.
- The narrow pathspec `src CMakeLists.txt tests/acceptance` (`review/recheck2-identity-narrow.txt`) and the default `src tests/native`
  (`review/recheck2-identity-default.txt`) are byte-equal to the first re-check's files (combined `92891b72...` and `6b8e55d2...`).
  All three exits are 0 (`review/recheck2-identity.exit`).
- Files newer than the first re-check's `recheck-identity.txt` (`review/recheck2-changed-files.txt`): the R8 edits
  `tests/native/v5-02-x08-c01-r7-evidence/REPORT.md`, `.../plan/INVARIANTS.md` and `docs/vernacular/rounds/V5-02-X08.md` (all
  05:20:11). There are also two earlier files from the end of the first re-check: this review file (my append) and
  `docs/vernacular/roadmap.md` (22:18:34, the coordinator's "C01-R7 review outcome" status edit, which that section records). No file
  under `build/` changed. The other files listed there are my own side effect (see "Reviewer incident"), not the fix's.
- `git status --short` over the whole tree equals the status at the start of this session.

### Reviewer incident (disclosed, repaired)

To find the identity script, I first ran every `tests/native/*-review-evidence/identity.py` with the pathspec and compared the output.
Some of these are not the identity printer. `v5-02-x05-b3-a-review-evidence/identity.py` compiles a probe and rewrites its own
evidence, and two others write a `src.json`. This modified three tracked, committed files in
`tests/native/v5-02-x05-b3-a-review-evidence/` (`identity-commands.json`, `identity-compile-isolation.json`,
`identity-source-isolation.json`, now holding home paths). It also overwrote ignored outputs in that folder (`identity`, `identity-*.o`,
`.pcm`, `.cppm`, `.log`), `n03b-review-evidence/identity-src.json` and `v5-02-x05-a3-c05-review-evidence/src.json`, and created two
`__pycache__` folders. I restored the three tracked files with `git checkout HEAD --` and removed the two `__pycache__` folders. After
that, the default-pathspec identity matches the first re-check again; before the repair its tracked hash had moved to `4ff787eb...`.
I cannot restore the previous bytes of the ignored outputs, which are not committed. `identity-transform.json` (tracked) was rewritten
with identical bytes. The script I compared with was the workflow `scripts/identity.py`, as before. Nothing in the X08 candidate was
affected.

### Earlier findings

| finding | re-check | status |
|---|---|---|
| RC1 | `REPORT.md:34` now cites `../v5-02-x08-c01-r6-evidence/raw/run2/inv-debug-sorted.txt` and calls it "the R6 gate run 2 sorted name list". It no longer says "committed". The file exists, has 1260 lines, all unique, already sorted under `LC_ALL=C`, and holds no home or user path. With ordinals stripped (R7 `inventory.txt` has none), its sorted set has 0 differing lines against the R7 `inventory.txt` (`review/recheck2-rc1-inventory.txt`). The grep over `docs/vernacular` and `tests/native/v5-02-x08*` for `c01-r6-evidence/inventory.txt` finds no evidence citation. The remaining hits name the file as the subject of a finding or of the staging rule: the R3 row, the RC1 outcome row, `V5-02-X08.md:948`, the roadmap's RC1 description, this review, and review/raw redaction logs. The R3 row (`V5-02-X08.md:909`) is now true | closed |
| RC2 | Row I36 (`INVARIANTS.md:16`) says "the R6 redaction file count is 23 (the files in the R6 redaction check list; the two REPORT.md files are the reports that were redacted)". That matches `c01-r6-evidence/REPORT.md:27`, `redact-files.txt` and R6 finding R6 | closed |

### New findings

No new false claim is introduced by the edits. The rest of the R8 changes are accurate: the other parenthetical at `REPORT.md:34` (C01 list, 1258 names, no home path) and the R8 section's statement that no source, CMake, acceptance, validator or evidence-data file changed (true of the fix; my incident above came after it and is repaired).

**RR1 (minor, records, at commit time): the roadmap status is stale.** `docs/vernacular/roadmap.md:35` still says the R7 review "leaves
two records findings open" (RC1, RC2) and "Next: records-only corrective C01-R8, then a records re-check". The R8 edit set does not
include the roadmap, so the opening no longer matches the round record. The next-work row (line 49) still lists "X08-C01-R8 (next)". Update both to record C01-R8 and this re-check's outcome when the commit is prepared. This does not block approval.

### Limits of this re-check

I ran no build, ctest, validator, lint or mutant. The product and validator bytes are unchanged, so the review's runs stand. I did not
run `scripts/redact-local-paths.py check`; the commit-time check over the exact staged set, including `tests/native/path-redaction.json`,
still applies. That staged set must not include `c01-r6-evidence/inventory.txt` unless it is redacted first.

### Evidence written by this re-check

`tests/native/v5-02-x08-c01-r7-evidence/review/`: `recheck2-identity.txt`, `recheck2-identity-narrow.txt`, `recheck2-identity-default.txt`,
`recheck2-identity.exit` (0, 0, 0), `recheck2-changed-files.txt` and `recheck2-rc1-inventory.txt`. None contains a home path.
