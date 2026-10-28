# V5-02-X06-D-C04 independent review (corrective for the X06-D-C03 review)

Disposition: **request-changes (records only)**.

The substance of C04 is sound. Both blocking findings concern records: the C04 record and the roadmap must describe the
reviewed state accurately. No source, test or evidence data has to change.

What C04 closes:

- **C03-R1 is closed by a real measurement.** Pass 2 measured the frozen candidate in scratch copies, in Debug and
  RelWithDebInfo. Its `src` identity is byte-identical before, during and after the measurement, and equal to the
  checkout as I reviewed it. I re-ran `check_margins.py` on the raw logs for both configurations: it exits 0, and its
  output is byte-identical to the recorded output. Every non-bracketing cell has at least 25 percent free in both
  configurations, on all five columns. The tightest cells are native 25.1, nodes 25.1, instructions 25.6, live Lua 59.2
  and registration 99.8 percent.
- **The ceilings follow from the table.** The bracketing fixtures are listed per column with their reasons. The CMake
  comment and the quarter-free test comment point at the real Margin record. The instruction equality pin is now a
  derivation that can fail: it was RED on the C03 ceilings, with a behavioural failure and a snapshot hash, and killed by
  name in the probes.
- **C03-R2 is closed for the carried-limit rows.** CL-1 to CL-6 are in the roadmap with verdict, untrusted caller and
  both clauses. Each line number matches the source.
- **C03-R3, C03-R4 and C03-R5 are closed.**
- **The gates hold on the exact bytes.** I re-ran the full Debug and RelWithDebInfo suites: 1114 of 1114 each, 0 skipped.
  All 1114 C03 names are preserved, and the Debug and RelWithDebInfo name sets are identical.

What is still open:

- **C04-R1 (required, records).** The roadmap is in its pre-C04 state. The plan's own row C4-18 required a review-time
  grep result of 0, and the record marks it NOT MET.
- **C04-R2 (required, records).** The record omits five of the calibration constants that C04 changed in
  `runtime.t.cpp`. Two record tables claim to list what changed.
- Two minor, non-blocking notes: C04-R3 (a parenthesis in `docs/architecture.md`) and C04-R4 (one calibration number in
  a test comment).

## Exact candidate

I reviewed in the shared checkout on `dev/planar-v2-vernacular` at HEAD `c875e08acdddf7b255ee0ff4c9533a59c872f81c`. The
candidate is uncommitted.

`identity.py` output for pathspec `src` (saved verbatim as `review/identity-src-before.txt` and
`review/identity-src-after.txt`):

| identity | value |
|---|---|
| HEAD | `c875e08acdddf7b255ee0ff4c9533a59c872f81c` |
| sha256(git diff --binary HEAD -- src), tracked only | `ca2d1ba5b1d714914d326f0b0f1c5c40918526cc27d57d2cb71b637c2e4853dd` |
| combined sha256 (diff bytes, then each untracked path NUL bytes, sorted), pathspec src, evidence excluded | `d8a2cab0b7a2a699158189aa1592c9c134d14b862de7e59dbd0968a7ee548aca` |

This output is byte-identical to:

- `sizing/pass2/identity-before.txt` and `identity-after.txt`;
- the probe run 1 and run 2 identities;
- the record's "Final candidate record".

Every per-file hash in the record's table also matches. The gate pathspec
`src docs/architecture.md docs/vernacular/profile-preparation-architecture.md docs/vernacular/orchestration.md` gives the
tracked-only hash `55dee86ae40e8c83433dd666cc9d0293e4facecfc4d5b665beedac69e2224cdb`, as the record states
(`review/identity-gate-pathspec.txt`).

Against the C03 record's per-file table, exactly three `src` files differ:

- `src/engine/CMakeLists.txt` (`d0952563...` to `4f263d74...`);
- `src/engine/runtime/runtime.t.cpp` (`a444106a...` to `1d7ecf32...`);
- `src/engine/runtime/external_registration.t.cpp` (`feb5b215...` to `281ed935...`).

`runtime.cpp` (`4100290b...`), `runtime.cppm`, `src/cmd`, `budget.cppm`, `ledger/acceptance.t.cpp`, `native.cpp`,
`loader.*` and `capture/*` keep their C03 hashes. So C04 changes no product behaviour, and diagnostics, the closed
message table, the public surface and the X06-C seam are byte-identical to C03.

I wrote only this file and new files under `tests/native/v5-02-x06-d-c04-evidence/review/`. I applied no mutant. I
edited no source, test, record, roadmap or existing evidence. I did not commit, push, merge, install or publish. I used
no old Planar tool, queue, skill, role or installed binary.

## Independent verification

Paths are under `tests/native/v5-02-x06-d-c04-evidence/` unless they say otherwise. I ran every command one at a time,
wrote each exit code to a file and piped nothing.

| Check | Result |
| --- | --- |
| Build state | `cmake --build build/rewrite-debug --target planar_tests` and `cmake --build build/rewrite-release --target planar_tests` both print `ninja: no work to do.`, so the binaries match the reviewed bytes. Two plain `cmake --build build/rewrite-debug` runs also do no work (`review/b-*.log`). |
| Full Debug | `ctest --test-dir build/rewrite-debug --no-tests=error --output-on-failure -j4`: exit 0, 100 percent of 1114 passed, no skip and no "Not Run" (`review/ctest-dbg.log`). |
| Full RelWithDebInfo | Same command on `build/rewrite-release`: exit 0, 1114 of 1114 passed (`review/ctest-rel.log`). |
| Focused | `ctest -R X06 --no-tests=error`: 165 of 165. `-R '[Bb]udget'`: 36 of 36. Both exit 0 (`review/x06.log`, `review/budget.log`). |
| Names | `ctest -N` in both build directories gives 1114 names each, and the two sets are byte-identical. After stripping the `replacement.` prefix, `comm -3` against `v5-02-x06-d-c03-evidence/inventory.txt` is empty (`review/names-diff-vs-c03.txt`). No `TEST_CASE` name that exists at HEAD in a changed test file is missing. |
| Guarded test #559 | 16.26 s against sibling #561 at 10.92 s in my full Debug run, a ratio of 1.49, the same as the record (`review/ctest-dbg.log`). |
| Lint | `rewrite-check.py lint --build-dir build/rewrite-debug`: exit 0, "Formatting and Doxygen checked 143 active source files" (`review/lint.log`). |
| Redaction | `redact-local-paths.py check --repo-path "$PWD" --home "$HOME"` exits 0 on the X06-D lineage round records, `docs/architecture.md`, the roadmap, `orchestration.md`, `profile-preparation-architecture.md`, every lineage `plan/*.md`, the READMEs and the REPORT files. |
| Margin table recomputed | `python3 sizing/pass2/check_margins.py debug raw/dbg --items raw/items/items.log` and the `release` equivalent, run from `sizing/pass2/`: exit 0 and 0, output byte-identical to `check-debug.txt` and `check-release.txt` (`review/check-margins-*.txt`). I read the script: it reads the ceilings from the checkout's `CMakeLists.txt`, keeps only runs whose logged ceilings equal them, applies the D9 per-test, per-column exclusion with one named manual exclusion, and judges the hidden native-boundary instruction cell (D10). |
| Raw logs are genuine | Run logs carry `memmax=39845888 instrmax=19900000 nodemax=25000 regmax=1000000`. The quarter-free test's two runs (`raw/dbg/runtime/t203.runlog`) give exactly 24,503,619 / 131,200 and 24,963,619 / 571,200, the values that the record and the test comment cite. The node test's tagged runs reach 29,828,959 native at 24,998 nodes, the binding native cell. In `raw/dbg` and `raw/rel`, 426 of 426 runtime and 19 of 19 loader tests exit 0. |
| Scratch copies match | `diff -rq src` between the checkout and the pass 2 copies `c04-p2-debug` and `c04-p2-release` lists only `runtime.cpp`, `runtime.t.cpp` and `external_registration.t.cpp`. `c04-p2-raised` also lists `CMakeLists.txt`, with exactly the four raised `set()` lines. The `runtime.cpp` difference equals `raw/instrumentation-runtime.cpp.diff`. The test-file differences only add scratch probe cases and wrap `last_passing` with the measure tag, and the tag affects only the exclusion label. The instrumentation records peaks and refusal bits and changes no limit comparison. |
| Required rows (C4-1e) | All 14 exact names are in SIZING.md. The record says the clause "every C02 `margins.md` section 1 row" was not matched row by row. I matched it: all 21 test names of C02 `margins.md` section 1 are in SIZING.md. |
| Plan copies (C4-11, C4-12, C4-19) | After undoing the one-level heading demotion, the C03 record's embedded INVARIANTS (166 lines) and DECISIONS (35 lines) are byte-identical to `v5-02-x06-d-c03-evidence/plan/*.md`. The C04 record's embedded INVARIANTS and DECISIONS are byte-identical to `plan/*.md`. The Margin record is byte-identical to `sizing/SIZING.md` without its title. C03 has `Fact F`, `E3b` (13 hits), "Probe dispatch" and D1 to D23. Its R4 row reads "NOT MET at the C03 review ... MET after the C04 copy". |
| Invariant table | The record's table holds all 45 plan section B ids. It adds the 5 C03 findings, C3-17 and the prober rows C4-5a-iii and C4-5b2. No row is missing. |
| Probe kills | I read the JSON and diffs of `probes/superseded-1/raw/` for C4-2b, C4-5a-i, C4-5a-ii, C4-5a-iii, C4-5b, C4-5b2, C4-6a, C4-6a2, C4-6b to C4-6f3, C4-6h-f2, C4-6h-g, C4-6h-h2, C4-6i1, C4-6i2 and C4-8 (X11, X11b, M_REG, M_OBS). Each mutant was applied, built with exit 0 and restored with a verified hash. In each, the row's named test is in `failed_names`. C4-5a-iii (instructions 19,700,000, just under 4/3 of E) fails only the quarter-free test. |
| Red check (C4-5r) | `raw/C4-5r-ctest-red.log`: `runtime.t.cpp:3859: FAILED: CHECK( budget_instruction_max * 3 >= 4 * extrapolated )` with `extrapolated boundary instructions 16812370`. The configure and build exits are 0, so this is a behavioural failure. The green run passes 1 of 1. The snapshot hash and the fallback command are recorded. |
| Survivors | C4-6g1 to C4-6g3 and C4-6h-f are recorded as survivors, not as kills. They have the same disposition as C3-4e, C3-4e2 and C3-4f, which the C03 review accepted. The equivalence survivors (E2, E3, E3b, E5, I25) are carried with `runtime.cpp` byte-identical. |
| CL-1 to CL-6 line numbers | In the current `runtime.cpp`, each cited line holds the code the roadmap row names: 2016-2022 (2019, 2021), 3503-3508 (3505), 4219-4222, 4235-4238, 4298, 4442, 4498, 4549, 4632, 4713, 4844-4845, 4888-4891, 6182 and 6335-6338. Line 396 of `profile-preparation-architecture.md` holds "one terminal first failure", and line 459 holds the "Semantic/protocol/resource failure caught by Lua" row (line 455 at HEAD). |
| architecture.md (C03-R4) | `runtime.cpp:32` is `constexpr std::size_t memory_max = 512 * 1024 * 1024;`, and `loader.cpp:12` is the 64 MiB `memory_max`. There are no hits for `src/engine/lua/runtime.cpp`. With newlines joined, "stays open for X06" has 0 hits, and the sentence now says "stayed open at X05 (historical: ...)". |
| Test comments (C03-R1, C03-R5) | `grep -rn SIZING src/` is empty. `24,963,619` and "C03 margins record" have no hits. "N = 40" and `694446` have no hits. The role-test comment names `c03_entries (= 1)`, the Debug configuration, the recalibration trigger and the Margin record. The tree floor 3391 is 0.90 of M = 3768, and the Margin record's item (d) bisect gives 224,394 - 220,626 = 3,768. |

## What is sound

- **C03-R1, measurement and derivation.** Pass 1 found two failing columns at the C03 ceilings: instructions at 6.6
  percent on the native sweeps, and nodes at 20.0 percent on the stages family. D10 options A and B are shown to be
  infeasible from the measured model. Option C changes the ceilings jointly (39,845,888 / 19,900,000 / 25,000, with
  registration unchanged), was trial-measured, and was then re-measured in pass 2 after the last `src` edit. Every
  exclusion is per test and per column, with its class:
  - one manual exclusion: the I2 instruction cell, as the confirmation partner of I1;
  - each must-succeed run hidden by a test-level exclusion is listed separately, and all are above 25 percent.
- **The quarter-free test is no longer a change detector.** It derives the native-sweep boundary cost from two padded
  probes and requires a quarter free. It failed behaviourally on the C03 ceilings, and the sharp mutant 19,700,000 kills
  it alone. The kept registration equality pin is allowed by the plan row (C4-5b). The record states it as a limit:
  the derived margin check already kills the in-interval mutant 2400.
- **The records state their own gaps honestly.** The record marks C4-18 NOT MET. It records the C4-21 pre-existing grep
  hit and C4-6h-h's "killed, but not by the named test". It labels survivors as survivors.

## Findings

### C04-R1 (required, records): the roadmap still shows the pre-C04 state

Plan row C4-18 requires the coordinator to update `docs/vernacular/roadmap.md` before review. Its check is
`grep -c "pending C04 outcome" docs/vernacular/roadmap.md` = 0 at review time. The record marks it NOT MET and hands it to
the orchestrator. At review time (`review/audit.txt`):

- `grep -c "pending C04 outcome"` is **1** ("Other carried limits ... ceiling pins: pending C04 outcome"). The real
  outcome is known: the instruction pin is replaced by a derivation, and the registration pin is kept at 1,000,000.
- The opening's "In review" bullet cites the C03 candidate hash `76092574...`. It says the candidate "is unchanged since
  review and not approved" and that "Corrective round X06-D-C04 is next". None of that is true any more.
- The `X06-D-C04` Next-work row still describes the plan ("ceilings 41943040, 18000000, 25500 confirmed or changed").

The C03 review (C03-R2) already said the roadmap must be right in the reviewed state, because review is the last gate
before the commit. The CLAUDE.md instructions also say that rounds update the roadmap opening.

Required:

- Replace the placeholder with the real pin outcome.
- Update the opening "In review" bullet: the C04 candidate, its `src` hash `ca2d1ba5...`, 1114 tests, and the review
  outcome.
- Rewrite the `X06-D-C04` and `X06-D` rows to state the delivered ceilings.
- Then set C4-18 to MET in the record.

Records only.

### C04-R2 (required, records): five calibration constants changed by C04 are missing from the record

I compared the C03 test bytes with the candidate. The pass 1 scratch copy `c04-meas-debug` holds the C03
`CMakeLists.txt` (`d0952563...`) plus additive instrumentation, and the diff is in `review/c03-scratch-to-c04-src.diff`.
Besides the extra-job counts, C04 changed these constants in `src/engine/runtime/runtime.t.cpp`:

- the stages-family shape loop, `for i=1,600000` to `for i=1,3670000` (`:3693`);
- the sink-family prefixes, `spin(950000)` to `spin(985000)`, twice (`:3874`, `:3876`);
- the C02 first-failure family prefix, `prefix = 900000` to `prefix = 995000` (`:3990`);
- the native-family prefixes, `spin(900000)` to `spin(985000)`, twice (`:4153`, `:4155`).

Pass 1 `RECOMMENDATION.md` section 5 derives these values, and they are measured in pass 2. They are what makes the
knife-edge cells hold: the binding node cell of 18,720 comes from the sink and first families. But the record does not
mention any of them:

- `grep -c "3670000\|985000\|995000"` on the record is 0.
- The "Changed files" row for `runtime.t.cpp` lists only the `c01_extra_jobs_*` counts.
- Margin record section 7, "Dependent constants as found in the source at measurement time", lists only the four
  ceilings, `c02_tail`, the four counts and the `c03_*` constants.

The source comments name the Margin record as the authority. Its section 8 says that "any change to a family constant, a
spin value or a ceiling needs a new measurement". A reader therefore cannot tell from the committed record which spin
values the 25 percent result depends on.

Required:

- Add the five spin and prefix changes, with their C03 and C04 values and the pass 1 section 5 citation, to the
  "Changed files" row and to the "Repairs, amendments and why" list of the final record.
- Leave the Margin record body as it is. It must stay equal to SIZING.md (C4-19), so add the list beside it, in the
  final record, not inside it.

Records only.

### C04-R3 (minor, documentation, not blocking): unbalanced parenthesis in the new architecture sentence

`docs/architecture.md:88-93` now reads "... ceiling (`memory_max = ...` in `src/engine/runtime/runtime.cpp:32`; the
declaration loader has its own separate 64 MiB `memory_max` in `src/engine/lua/loader.cpp:12`), enforced ... decision Q9
in `docs/vernacular/remaining-steps-design.md`), the cumulative ...". The C04 insertion closes the parenthesis early, so
the ")" after `remaining-steps-design.md` has no opening partner. The citations themselves are correct. Moving the
first ")" to after the Q9 reference fixes it. This can go with the C04-R1 edit, or be carried.

### C04-R4 (minor, test comment, not blocking): one calibration figure does not match the cited record

`external_registration.t.cpp:4585` says `c03_obs_floor = 174140; // measured 175864`. The comment above now says
"Measurements: the Margin record section". That section's item (d) gives a base boundary of 224,394 and an
observations-padded boundary of 48,532, which is a drop of 175,862. The 2-byte difference has no effect on the test: the
floor is 174,140, and the predicted mutated drop is 172,416. The figure dates from C03, but the C04 comment now points at
a record that does not contain it. Correct the figure or the pointer at the next source edit. Do not open a source
round for it.

## Records-only status

Records only. C04-R1 is fixed by editing `docs/vernacular/roadmap.md`, a planning record, and the C04 record. C04-R2 is
fixed in the C04 record. Neither needs a change to source, tests, ceilings or evidence data, and neither invalidates the
gates, the measurement or the probes: the `src` identity does not move. C04-R3 and C04-R4 do not block.

## Limits carried and observed

- I accept CL-1 to CL-6 as operator-approved carried limits (option A, 2026-10-27), not findings.
- I accept the limits accepted by the C01, C02 and C03 reviews, including P01 masked, N1 phase coverage unmeasured,
  Linux not run, and no RSS or capacity claim.
- Accepted as recorded:
  - live Lua, node and role-flow native peaks have no public observer (D6);
  - the registration equality pin is kept;
  - the instruction ceiling 19,900,000 is not a multiple of 1,000,000, because 20,000,000 is shown to be infeasible;
  - the knife-edge cells hold only for exactly these constants;
  - the raised scaling build is Debug only.
- Survivors accepted as recorded:
  - C4-6g1 and C4-6g2: the two-file padding split is a reach safeguard, guarded at `external_registration.t.cpp:4626`.
  - C4-6g3: the bracket's `maximum == PLANAR_BUDGET_NATIVE_MAX` check rules out a `file_bytes` refusal. It would not
    rule out a `lua_bytes` refusal, which carries the same number, but this fixture's live Lua is far below the ceiling.
  - C4-6h-f: the literal 6000 still brackets 3,565 entries, and C4-6h-f2 kills the row with its premise made true.
  - C4-6h-h: killed, but not by the named test; C4-6h-h2 kills it by name.
  - The equivalence rows C3-9c, C3-9d, C3-9f, C3-9g and T8.
- Probes ran in Debug only. The C02 runtime-test rows listed as "not probed" were not re-run. I re-ran no mutant: I read
  the diffs, the JSON and the failing-test names.
- I re-ran `check_margins.py` on the recorded raw logs, but I did not rebuild the instrumented scratch copies. The claim
  that the logs come from the final bytes rests on the scratch `diff -rq` above and on the byte-identical pass 2
  identities.

## Review evidence

The files are under `tests/native/v5-02-x06-d-c04-evidence/review/`:

- `identity-src-before.txt` and `identity-src-after.txt` (pathspec `src`; byte-identical), and
  `identity-gate-pathspec.txt`.
- `b-dbg-tests.log` and `.exit`, `b-rel-tests.log`, `b-dbg-1.log`, `b-dbg-2.log`: the builds, with no work done.
- `ctest-dbg.log` and `.exit`, `ctest-rel.log` and `.exit`: the full suites, 1114 of 1114 each.
- `x06.log`, `budget.log`: the focused runs. `lint.log`: the lint.
- `names-debug.txt`, `names-release.txt` and `names-diff-vs-c03.txt` (empty).
- `check-margins-debug.txt` and `check-margins-release.txt`: my re-runs, byte-identical to the recorded outputs.
- `c03-scratch-to-c04-src.diff`: the C03 test and CMake bytes (from the pass 1 scratch copy, plus its additive
  instrumentation) against the candidate, the basis of C04-R2.
- `audit.txt`: the greps behind C04-R1 to C04-R4.

## Records re-check (2026-10-27)

Decision: **request-changes, records only.** C04-R1 and C04-R2 are closed in substance, and the candidate bytes have not
moved. But the fix updated only some of the record's statements about the roadmap. Three passages still describe the
pre-fix state, and one of them contradicts its own row. No source, test or evidence-data change is needed.

### Candidate unchanged

I re-ran `identity.py` from the repository root at HEAD `c875e08acdddf7b255ee0ff4c9533a59c872f81c`.

- Pathspec `src`: the output is byte-identical to `review/identity-src-after.txt`. The tracked diff sha256 is
  `ca2d1ba5b1d714914d326f0b0f1c5c40918526cc27d57d2cb71b637c2e4853dd` and the combined sha256 is
  `d8a2cab0b7a2a699158189aa1592c9c134d14b862de7e59dbd0968a7ee548aca` (saved as `review/recheck-identity-src.txt`).
- Gate pathspec `src docs/architecture.md docs/vernacular/profile-preparation-architecture.md docs/vernacular/orchestration.md`:
  the output is byte-identical to `review/identity-gate-pathspec.txt`. The tracked diff sha256 is
  `55dee86ae40e8c83433dd666cc9d0293e4facecfc4d5b665beedac69e2224cdb` (`review/recheck-identity-gate-pathspec.txt`).
- Pathspec `docs/vernacular/roadmap.md`: the tracked diff sha256 is
  `83cc4660760b8c4851d8179f6f1c5b8c61e8eac90f2868e05fa7d1fdca80d8d1`, as the record's "Records fix after review" states
  (`review/recheck-identity-roadmap.txt`).
- Since my review file was written, only three files under `src`, `docs` and `tests/native` are newer:
  `docs/vernacular/roadmap.md`, the C04 record and my own `review/identity-src-after.txt` (`review/recheck-greps.txt`).
  No source, test or evidence-data file changed.

I ran no build, test or mutant.

### Findings

- **C04-R1: closed.**
  - `grep -c 'pending C04 outcome' docs/vernacular/roadmap.md` is 0.
  - The roadmap no longer contains `76092574`, "X06-D-C04 is next", `41943040`, `18000000` or `25500`.
  - The opening's "In review" bullet names X06-D-C04 with the final ceilings, the `src` combined hash `d8a2cab0...` and
    the gate hash `55dee86a...`.
  - The X06-D-C04 row cites 39845888 / 19900000 / 25000 with registration 1000000 unchanged. The X06-D row notes that
    C04 is in review.
  - The "Other carried limits" item says the instruction pin was replaced by a derivation and the registration pin stays
    at 1,000,000.
  - I checked the bullet's claim that each earlier round was reviewed with request-changes against the four review files.
    It is true.
- **C04-R2: closed.** The new table in "Records fix after review" lists all five constants. Their lines and values match
  `runtime.t.cpp`: 3670000 at `:3693`, 985000 at `:3874` and `:3876`, 995000 at `:3990`, and 985000 at `:4153` and
  `:4155`. The C03 values (600000, 950000, 900000, 900000) match `review/c03-scratch-to-c04-src.diff`. The Margin record
  was left equal to `SIZING.md`, as required.
- **C04-R3 and C04-R4:** deferred with reasons, as non-blocking findings may be.

### New finding

**C04-R5 (required, records only).** The fix left three passages in `docs/vernacular/rounds/V5-02-X06-D-C04.md` that are
now false.

1. **Acceptance table, row C03-R2 (`:704`).** The status cell now reads "MET (roadmap status updated in the records fix
   after review)". The evidence cell of the same row still says:
   - ``grep -c "pending C04 outcome" docs/vernacular/roadmap.md` is 1``;
   - "the roadmap opening still says X06-D-C04 is next";
   - "the orchestrator must update it before review".

   It also cites `roadmap.md:44` for the CL row, which is now at `:47`. The row contradicts itself and the C4-18 row
   (`:713`, "is 0").
2. **Invariant table, row C03-R2 (`:793`).** The outcome column still says "roadmap status placeholder still open
   (C4-18)". This column was written by the record, not copied from `plan/INVARIANTS.md` (the plan file has no such
   text), so it can be corrected.
3. **"Next" (`:803`).** It still says the orchestrator updates the roadmap placeholder, opening and rows, and *then* the
   independent review follows. Both have now happened.

Fix: in row `:704`, replace the stale sentence with the current grep (0) and `roadmap.md:47`. In row `:793`, drop the
"placeholder still open" clause or mark it closed by the records fix. Rewrite "Next" as the post-review step (re-check,
then the single commit on approval).

### Minor, not blocking

The C04-R2 table is followed by a line labelled "Killing mutant per constant". It names no mutant and no test. It only
says that a change would move a knife-edge cell, and no probe covers these five constants. Relabel it, for example
"Dependency", or name the test whose cell each constant calibrates.

### Commands

- `identity.py src`
- `identity.py src docs/architecture.md docs/vernacular/profile-preparation-architecture.md docs/vernacular/orchestration.md`
- `identity.py docs/vernacular/roadmap.md`

Each output was compared with `diff` against the saved review identities. The greps and the newer-file check are in
`review/recheck-greps.txt`.

New evidence under `tests/native/v5-02-x06-d-c04-evidence/review/`:

- `recheck-identity-src.txt`
- `recheck-identity-gate-pathspec.txt`
- `recheck-identity-roadmap.txt`
- `recheck-greps.txt`

I edited only this section of this file and created only those four evidence files.

## Records re-check 2 (operator session, 2026-10-27)

Decision: **approve.** C04-R5 and C04-R6 are fixed in the record. The operator-session fixes of C04-R3 and C04-R4 are
correct, and they are the only byte changes since the identity this review recorded. The notes below are not blocking.
I read the bytes myself and edited only this section.

### C04-R5 and C04-R6

- Acceptance row C03-R2 (`V5-02-X06-D-C04.md:704`) no longer contradicts itself. It says MET, gives the grep as 0, and
  says the roadmap opening names X06-D-C04 as in review. The stale "is 1" and "still says X06-D-C04 is next" text is
  gone. It cites `roadmap.md:47` for the CL row. That was the right line at the first re-check, but the coordinator
  later added an X06-D-C05 row, and the CL row is now at `roadmap.md:48`. Line 47 is the X06-D row (note N1).
- Invariant row C03-R2 (`:793`) now reads "roadmap status placeholder closed by the records fix (C4-18)".
- "### Next" (`:803`) now describes the roadmap update and the review as done. It still names C05 as the next step and
  the commit as "X06-D, C01 to C05" (note N2).
- `grep -n "Killing mutant"` finds the phrase only in the Outcome table, which quotes the old label. The line after the
  C04-R2 table is now labelled "Dependency" and claims no mutant. C04-R6 is closed.

### C04-R3 and C04-R4, and the identity

- `identity.py src` (exit 0) prints tracked `9409517d...d8d6` and combined `2b7b5ac5...9838`. Both equal the values in
  the record's appended section. Compared with `review/recheck-identity-src.txt` (tracked `ca2d1ba5...`, combined
  `d8a2cab0...`), only one per-file row differs: `src/engine/runtime/external_registration.t.cpp`, which went from
  `281ed935...` to `243c4cd4...`. The other 23 rows, including untracked `budget.cppm`, are unchanged.
- In that file, replacing "measured 175862; f = 1/51" with "measured 175864; f = 1/51" gives sha256 `281ed935...`, the
  recorded pre-fix hash. So the one-digit comment change is the only byte change. The line is now `:4586`
  (`c03_obs_floor = 174140; // measured 175862`), and the constant is unchanged.
- The evidence supports 175862. `grep -rh "obs 17586[0-9]"` under `tests/native/v5-02-x06-d-c04-evidence` finds only
  `obs 175862` (9 lines, in the probe logs). No log shows 175864.
- `identity.py` on the gate pathspec (`src docs/architecture.md docs/vernacular/profile-preparation-architecture.md
  docs/vernacular/orchestration.md`, exit 0) differs from `review/recheck-identity-gate-pathspec.txt` only in the
  aggregate hashes and in two per-file rows: `docs/architecture.md` (`5672180a...` to `97ea3b05...`) and the test file
  above. The two other documents are unchanged.
- In `docs/architecture.md`, replacing "as live acquire and release (decision Q9 in" with the old "...; decision Q9 in"
  gives sha256 `5672180a...`, the recorded pre-fix hash. So that substitution is the only change. The paragraph at
  `:78-96` now has 8 opening and 8 closing parentheses, and the text reads correctly.

### Commands re-run (serialized, exit codes read unpiped)

- `cmake --build build/rewrite-debug --target planar_tests`: exit 0, `ninja: no work to do.`
- `ctest --test-dir build/rewrite-debug -R X06 --no-tests=error -j4`: exit 0, 100% passed out of 165, no skipped or
  not-run lines.
- `python3 scripts/rewrite-check.py lint --build-dir build/rewrite-debug`: exit 0, 143 active source files.
- `identity.py src` and `identity.py` on the gate pathspec: exit 0, outputs compared with `diff` against the saved
  re-check identities.

I ran no full Debug or RelWithDebInfo suite and no mutant. Gate run 2 (1114 tests in both configurations) and the probe
runs were made on the bytes before this fix. The only change since then is a comment, plus a documentation edit
outside `src`. So those results still apply in substance, but they are not a run on the current bytes.

### Appended section

The section "Operator-session fix of C04-R3 and C04-R4" makes no false claim that I could find. Its description of both
edits, its identity, its build/ctest/lint results and its claim that nothing else changed all match what I measured.

### Notes (not blocking)

- **N1** (`V5-02-X06-D-C04.md:704`): the C03-R2 acceptance row cites `roadmap.md:47`, but the CL-1 to CL-6 row is now at
  `:48`. The citation will move again when the roadmap is rewritten at commit time, so it is better to cite the row by
  name.
- **N2** (`V5-02-X06-D-C04.md:3`, `:803`, `:616`): the status line and "### Next" still say the next round is C05 and
  that the commit is "C01 to C05". The appended section explicitly supersedes only the Outcome's "Next round" line.
  The Final candidate record's gate-pathspec hash `55dee86a...` and its `src` hashes are now superseded too. The
  appended section records the new `src` identity, but not the new gate-pathspec identity (tracked
  `328943fc3d2362f3edcfad47083d7d0c590bb2553c6887daf88ebd44d77fc12f`).
- **Roadmap after approval:** apart from the status words "not approved" and C05 being next, the following roadmap
  text will be false once the round is approved:
  - the opening bullet (`roadmap.md:25-33`) says "R3 ... and R4 ... deferred to X06-D-C05" and calls the candidate
    current at `d8a2cab0...` / `ca2d1ba5...`. The current values are `2b7b5ac5...` / `9409517d...`;
  - its "Gate run 2 passed" refers to the pre-fix bytes (see above);
  - row `:45` says "R3/R4 deferred to C05";
  - the whole X06-D-C05 row (`:46`);
  - row `:47` says "reopened until C05 approved".
