# R20-CARRY-A review (uncommitted tree on top of dec6f178)

Disposition: **request-changes**.

Most of the candidate is sound:

- The generator and embed-helper work does what the dispatch asked.
- The 18 new tests are wired into the gate and run on macOS and Linux.
- Every invariant-table mutant was killed by the listed test.
- No existing test name changed.
- Non-test `src` is untouched.

The round cannot be approved yet for one reason. One binding acceptance criterion is recorded as NOT MET: the two named slow tests must pass under a load much heavier than the earlier failure. The round's own approved plan already says what happens next (decision D32, conditional wave W6), but that step was not run. The record leaves it as an open option instead.

There is also one smaller blocking item, which is a records fix. The evidence `REPORT.md` that is going to be committed contains a machine-local home path, and the redaction check fails on it.

The reviewer was not the coder, the gate runner, the coordinator or the prober. My writes were limited to:

- this file;
- the gitignored folder `tests/native/r20-carry-a-review-evidence/`, which holds my identity, ctest, name-diff, TIMEOUT-property and redaction outputs.

I applied no mutant anywhere. Nothing was staged or committed. I did not build, configure, run a gate or use Docker. I did run two focused `ctest` commands against the existing `build/rewrite-debug`, after checking that no other ctest, ninja or build process was running (see Evidence). No installed or legacy binary ran. The real `~/.planar/planar.db` still has its 2026-11-07 modification time.

## Findings

### R20-CARRY-A-R1 (blocking): the loaded-host acceptance criterion is not met, and the plan's prescribed next step was not run

The dispatch's first acceptance criterion requires that both named tests "pass under a load much heavier than the previous failure". The record states it is NOT MET (acceptance table, row 2), and the raw evidence agrees.

- `raw/timings.txt` has the C0 and C1 runs. The C0 runs are direct Catch2 runs with no ctest timeout. They took 813 s (v2 allocation) and 406 s (above4096) at load 345 to 400. The C1 runs used ctest at the registered 120 s limit. Both timed out (exit 8, at 121 s and 122 s).
- `raw/load/c1-t1.log` and `c1-t2.log` show the timeouts.

Plan decision D32 (`plan/DECISIONS.md`, revision 4) and invariant I14 already decide what follows a `LOAD-MARGIN-FAIL`:

1. The coordinator re-dispatches PKG-I1 with `TEST_TIMEOUT = max(120, ceil60(2 x loaded direct time))`. That gives 1,680 s and 840 s, both under the 1,800 s design-question bound.
2. PKG-V, PKG-E and PKG-R then re-run.

The round record's "Open decision for the coordinator" offers a second option instead: accept the idle margin and call the loaded criterion a named limit. That option is not in the approved plan. It would also leave the dispatch's first objective (robust under load) undemonstrated.

A rough scaling shows why this matters. On this 8-core host the slowdown was about 34 to 37 times idle at load 345 to 400. At the plan's own target load of 148, the same scaling suggests roughly 13 to 18 times idle: about 300 to 400 s for the v2 test and 150 to 190 s for above4096. Both are above 120 s. So 120 s is very unlikely to meet "much heavier than 74" even at the intended load, never mind the overshoot load. This is an estimate, not a measurement. It is why I do not treat the overshoot as a reason to waive the criterion.

What closes this finding (not records-only):

- Preferred: run W6 as D32 prescribes. Set the two targets' `TEST_TIMEOUT` values, rebuild, run `replacement.r20` (18 tests), redo the name diff and I16, re-measure idle and loaded times, and re-report. If the coordinator thinks the 345 to 400 load is unrepresentative, a second loaded measurement near the 148 target with the D32 formula applied to it is equally acceptable. Either way the new limit must be backed by a recorded C1 pass.
- Also raise `MINIMUM` in `tests/native/r20-carry-a-support/timeout_headroom.py` to the new value. At `>= 120` the three headroom guards would not catch a revert from 1,680 to 120, so the guard would no longer protect the value the round actually depends on.
- Re-run the final gate once on the new candidate per the 2026-11-11 rule. The two CMakeLists change, so the earlier run1 does not cover it.
- Changing the acceptance criterion instead would be a change to the dispatch. The record cannot do that on its own authority.

### R20-CARRY-A-R2 (blocking, records-only): the evidence `REPORT.md` fails the redaction check

`tests/native/r20-carry-a-evidence/REPORT.md` line 45 contains the literal a literal absolute home-directory path after `--home`. It is in the commit set (`git add -f`). I ran:

```
python3 -B scripts/redact-local-paths.py check --repo-path "$PWD" --home "$HOME" tests/native/r20-carry-a-evidence/REPORT.md
```

It printed `local path in tests/native/r20-carry-a-evidence/REPORT.md` and exited 1 (`review evidence/redact-report.txt`). The gate's command 12 checked 26 round files but not this report, so its "exit 0" does not cover the committed report.

`raw/commit-list.txt` and `raw/redaction-check.txt` are also stale. They describe the earlier 68,702-byte coder report and list `roadmap.md` and `history.md`, which are not edited yet. The record already says this.

Fix: replace the path with `<home>`, regenerate the commit list, and run the check on the staged set until it exits 0. If R1 is closed through W6, PKG-R regenerates all of this anyway.

### R20-CARRY-A-R3 (non-blocking): the per-target limit raises 120 tests, not two, and the record does not say so

`TEST_TIMEOUT` on `planar_rewrite_module` passes the limit to `catch_discover_tests` for the whole test executable (`cmake/rewrite.cmake:30-33`). `ctest --show-only=json-v1` on the current build shows TIMEOUT 120 for:

- 104 tests in `planar_ledger_format_tests`;
- 16 tests in `planar_history_interpretation_v2_codec_tests`.

Before the change, all 120 had 30 s. The dispatch allowed a per-target limit, so this is permitted. It does mean a hang anywhere in `format.t.cpp`, `snapshot.t.cpp`, `history_verification.t.cpp`, `acceptance.t.cpp` or the v2 codec tests now takes longer to detect. After W6 that would be 840 s or 1,680 s.

Fix: state the affected test counts and the hang-detection trade-off in the round record. The same counts should appear in the W6 brief.

### R20-CARRY-A-R4 (non-blocking): `docs/testing.md` points at local-only evidence and contains a statement that is not true

The new section has three problems:

- It cites `tests/native/r20-carry-a-evidence/raw/timings.txt` and `raw/nearest-limit/table.txt`. These are raw files that the commit rule keeps local, so they will not exist in a clone. Earlier sections cite evidence folders, which have committed READMEs.
- It copies round-specific loaded-failure numbers into a product reference document. Those numbers go stale as soon as W6 changes the limit.
- It says the 18 drivers "never write to the checkout". They import `common.py`, so CPython writes `tests/native/r20-carry-a-support/__pycache__/` into the source tree. It is gitignored, and other `*-support` folders have the same pattern, but the sentence is not true as written.

Fix: keep the measurements in the round record and evidence README. Cite the evidence folder, not raw files. Either reword the sentence or run the drivers with `-B` (or `PYTHONDONTWRITEBYTECODE=1`).

### R20-CARRY-A-R5 (non-blocking, recorded limit): on the real tree, the first build after an unchanged reconfigure still runs dyndep steps

The dispatch's acceptance wording is "the next build does zero work". On the real tree, `raw/real-tree-zero-work/build-2.log` shows 607 `Generating CXX dyndep file` steps and no compile or link step. `build-3.log` then prints `ninja: no work to do.`.

I checked the cause against the mtime snapshots:

- The candidate's `mtimes-diff.txt` lists only CMake-managed build-system files: 607 `CXXDependInfo.json`, plus `CTestTestfile.cmake`, `cmake_install.cmake`, Catch2 `_include.cmake`, `build.ninja` and similar.
- No generated `.cppm`, `.lua`, `.cpp` or `generation.json` changed.
- The baseline at dec6f178 rewrites the same `CXXDependInfo.json` files and also 131 generated sources.

So R20 removed everything the round owns, and the remaining dyndep work comes from stock CMake. The scratch-project test `configure_zero_work` and the gate's no-change builds meet the literal wording.

I accept this as a named limit. The coordinator's roadmap sync should carry it, so that nobody later reads "configure timestamp follow-up closed" as "reconfigure is free".

### R20-CARRY-A-R6 (non-blocking, probe quality): three kills come from the generators' own self-audits, not from the test's assertions

The kills for I8 (B1 write-if-missing), I11 (X05-A0 write-if-missing), I13 (`configure_rebuilds_on_content_change` under B1 write-if-missing) and E17 do not come from the tests' own checks. In each case the generator's read-back audit fails (`assert restored==impl`, or the X05 `altered` check), the generator exits 1, and that fails the configure or the generator run (`probes/superseded-2/raw/I8.log`, `I11.log`, `I13.log`, `E17.log`).

The kills are real and name the listed test. The mutant does break the invariant, because a stale B1 or X05 output can never pass silently. But they do not show that the tests' own assertions can fail:

- For `generator_content_change_rewrites_*`, the shared assertion is shown to work by I9 and I10 (D6 and D6-C01). Those have no read-back audit and fail on `stale commands_0.lua not rewritten to the exact first-run bytes`.
- For `configure_rebuilds_on_content_change`, no probe shows that the "ninja does work after a content change" assertion can fail. A D6-only or D6-C01-only write-if-missing mutant would survive it, because the other generators still rewrite on the same input change. The per-generator content-change tests do cover each generator, and the test header calls the check "necessary, not sufficient".

Fix: no code change is required. Add a sentence to the record's I13 row saying the kill is through the B1 self-audit and that the ninja-work assertion is a coarse aggregate.

### R20-CARRY-A-R7 (non-blocking, roadmap sync): slice B and the stale rows

Slice B is carried, not dropped:

- the round record names it ("Remaining limits");
- roadmap Next work already has the row "R19 carried: reviewer-packet wording and captured-preview limit".

At the post-approval sync the coordinator still needs to:

- rename that row to R20-CARRY-B;
- replace the stale row "R19 carried: load-sensitive tests #1321 and #1030" (roadmap line 42) with the real test names and the final limits;
- close or rewrite the "Configure timestamp follow-up" row (line 35) with the R5 residual;
- add the D28 near-limit item (`replacement.cli` 18.03 s of 30 s, and RA01-B2 C01 1048576-byte prompt 16.40 s of 30 s).

These are already the coordinator's sync duties under the dispatch. I list them so none is missed.

## What was verified and holds

### Candidate identity

From the repository root, `identity.py src tests/native` printed the following (saved in `tests/native/r20-carry-a-review-evidence/identity-src-tests.txt`):

| identity | value |
|---|---|
| HEAD | `dec6f178daf10e2a1219ae56dc71485d42d839ca` |
| sha256(git diff --binary HEAD -- src tests/native), tracked only | `9a891be59df3f260098733b10c70b240542b09fa3b05f923f6dfb3bf66635355` |
| combined sha256 (diff bytes, then each untracked path NUL bytes, sorted), pathspec src tests/native, evidence excluded | `c80c07a863fbca7c2cc0cae22d11356f61791d3881cc0c0dd3b6b4120428c253` |

`identity.py cmake docs/testing.md` printed tracked and combined `1d24a8b8e5d80a6e077681b69d9e97178d6a5408eba4c9469478adfe2c7b3ac0`, with:

- `cmake/embed_lua.cmake` `9f712ff41efca52919213620a42c40992ad51f236c0b11d0858a5dab14d92f1d`;
- `docs/testing.md` `689b990cea5134a6f89facdc29a4123ae5561272167e64e0000342261e8a1c2d`.

These values, and every per-file hash, equal the round record's final-candidate section, the gate `REPORT.md` and `probes/PROBES.md`. So the reviewed, gated and probed trees are the same tree.

### (a) The right tests, and nothing weakened

- The stale numbers resolve by name:
  - `replacement.v2 every native allocation failure remains a redacted typed resource refusal` is now #442;
  - `replacement.recorded effective history selects wider fixed tree profile for valid closure above4096 files` is now #658.
  - Each is registered exactly once, with TIMEOUT 120 (`show-only.json`).
- `git status` shows neither `interpretation_v2_codec.t.cpp` nor `history_verification.t.cpp` modified. No assertion, allocation sweep count or code path changed. The only change is the `TEST_TIMEOUT 120` token on the two `planar_rewrite_module` calls.
- Idle evidence: medians of 23.66 s and 10.88 s give limit/idle ratios of 5.07 and 11.03, at or above the required 4. They were measured at load 35 to 52, so they are pessimistic.
- My own focused run at load 47.7 passed: #442 in 20.25 s and #658 in 11.79 s (`ctest-named.log`, exit 0).
- The loaded criterion is R1.

### (b) Write-if-different generators, and tests that can fail both ways

- All ten generators compare the text before writing. The comparison covers both bytes and mtime, because no write happens when the text is equal.
- `generator_stability.py` checks the result by backdating every output with `os.utime` to a fixed old timestamp and then requiring `st_mtime_ns` to be unchanged. It does not sleep. It compares exact output sets: 63, 18, 3 and 7 files, and 3, 3, 3, 265, 45 and 221 for the manifest generators.
- The content-change cases require exact first-run bytes with a newer mtime, and the marker text after an input change.
- The embed helper builds the module in memory and calls `file(CONFIGURE)` once. The nine former read-back and rewrite sites now pass REMAP (D6-C01 also passes LITERAL).
- `raw/embed-bytes-before.txt` and `-after.txt` are identical: 69 `*_source*.cppm` files, including `planar_runtime_source.cppm`. That shows the product embed is byte-identical.
- The relative-name rule (relative to `src/stdlib`, else to the project root) matches the old per-site `file(RELATIVE_PATH ... PROJECT_SOURCE_DIR ...)` for build-directory files, because the `src/stdlib`-relative form starts with `..` and falls back. Linux, where the build directory is laid out differently, passed 1,872 of 1,872.
- Wiring: the 18 cases are registered with `add_test` in `src/engine/runtime/CMakeLists.txt`. They ran in the macOS Debug and Release gates (`raw/run1/02-gate-debug.log`, `08-gate-release.log`, 100 percent of 1,872) and in the Linux Debug ctest (`raw/run1/linux/debug-ctest.log`, 18 `replacement.r20.*` entries).
- My own `ctest -R 'replacement\.r20\.'` run gave 18 of 18 passed, exit 0 (`ctest-r20.log`).

### (c) No rename or removal; non-test `src` and diagnostics unchanged

- I stripped the numbers from `raw/ctest-names-before.txt` (1,854) and `raw/ctest-names-after.txt` (1,872) and sorted both lists. The diff is exactly the 18 `replacement.r20.*` additions, with no deletions (`names-diff.txt`).
- `ctest -N` on the current build reports 1,872.
- Under `src`, only the four `CMakeLists.txt` files differ from HEAD:
  - two add a `TEST_TIMEOUT` token;
  - runtime adds the registration block and the REMAP and LITERAL arguments;
  - `cmd/planar` adds REMAP arguments.
- No product C++ or Lua changed, so every diagnostic is byte-identical.
- `cmake/embed_lua.cmake` is product build tooling, not `src`. The dispatch did not list it, but the plan (D19'', PKG-I2) does, and the record lists it as amendment (1). The dispatch's premise that `file(CONFIGURE)` already leaves these modules untouched was wrong for the nine double-write sites, so the change was needed to meet item 2. Its byte identity is shown above.

### (d) Mutation probes

- `probes/PROBES.md` runs every invariant-table mutant in scratch trees outside the checkout. The two ineffective first attempts at m5a are labelled and kept, and a regex gap in an early run was re-run.
- 37 of 37 table mutants and 15 extra mutants were killed by the named test. E6 to E9 survived the first run and were killed after the `embed_stability.py` repair (`probes/raw/E6.log` to `E9.log`).
- The carried rows' mutated files and tests are byte-identical to this candidate. Only `embed_stability.py` changed, and every row it hosts was re-run.
- R6 notes three kills that come from the generators' self-audits.

### (e) Red-first

`red/RED.md` with `red/raw/*.log` shows ten expect-red tests failing on the pre-fix tree. Each failure is behavioural, with an assertion message:

- `TIMEOUT 30 < 120` for both headroom tests;
- 63 of 63, 18 of 18, 3 of 3 and 7 of 7 files rewritten;
- six named manifest generators;
- `[1/1] Generating stamp`;
- a moved module mtime;
- the offender list for the call-site audit.

The eight guards passed. The red-time snapshot is `red/snapshot-2`, with the `candidate.diff` hash recorded from the identity tool.

The record says explicitly that the limit change itself has no deterministic red beyond the headroom checks. That meets the dispatch.

### (f) Evidence commit set

- `REPORT.md` is 14.5 KB and `README.md` 2.5 KB. `CODER-REPORT.md`, if committed, is 68.7 KB. All are under 100 KB.
- No committed file pins the hash of a record, review, report or roadmap. The hashes that do appear (`inventory.txt`, `ctest-names-before.txt`, `candidate.diff`, `SHA256SUMS`) are of local evidence data, produced by `shasum` or the identity tool.
- Machine-local paths: R2.

### (g) Slice B

Carried. See R7.

### (h) Owned files against `git status`

`git status --short` lists:

- the 17 modified files named in the record's "Files edited in this round";
- the untracked `docs/vernacular/rounds/R20-CARRY-A.md` and `tests/native/r20-carry-a-support/`. That folder's only ignored content is `__pycache__/`.

That matches the record exactly, plus this review file. `cmake/rewrite.cmake`, the two `.t.cpp` files, `roadmap.md` and `history.md` are unmodified, as the record says.

### Invariant table

The 29 rows (I1 to I28 and I24b) in the round record match `plan/INVARIANTS.md` revision 4 row for row:

- Every acceptance criterion has a row.
- Each test row's mutant breaks its invariant and is killed by the listed test (with the R6 caveat about how three of them are killed).
- Each expect-red row failed on an assertion before the fix.
- The evidence rows match their raw evidence.
- I14(b) is honestly marked NOT MET (R1), and I17 PARTIAL (R5).

## Evidence

These files are in `tests/native/r20-carry-a-review-evidence/` (gitignored, local):

- `identity-src-tests.txt` and `identity-cmake-docs.txt`: identity tool output.
- `ctest-N.txt`: 1,872 registered.
- `show-only.json`: the TIMEOUT properties, 120 for the 104 plus 16 target tests.
- `names-before-sorted.txt`, `names-after-sorted.txt`, `names-diff.txt`: the name comparison, additions only.
- `ctest-r20.log`: 18 of 18 passed, exit 0, load about 47.
- `ctest-named.log`: 2 of 2 passed, 20.25 s and 11.79 s, exit 0.
- `redact-report.txt`: exit 1 on `REPORT.md`.

Commands I ran from the repository root:

- `identity.py src tests/native`
- `identity.py cmake docs/testing.md`
- `ctest --test-dir build/rewrite-debug -N`
- `ctest --test-dir build/rewrite-debug --show-only=json-v1`
- `ctest --test-dir build/rewrite-debug -R 'replacement\.r20\.' --output-on-failure`
- `ctest --test-dir build/rewrite-debug -R 'v2 every native allocation failure|recorded effective history selects wider fixed tree profile' --output-on-failure`
- `python3 -B scripts/redact-local-paths.py check --repo-path "$PWD" --home "$HOME" <files>`

Before the two focused ctest runs I confirmed that no ctest, ninja or build process was running.

## Limits of this review

- I did not re-run the Debug, Release or Linux gates, the probes, the loaded measurement or the real-tree zero-work sequence. I read their raw logs and exit files (`raw/run1/*.exit`: all 0 except the documented command 11, which the copied-binary command 11b replaced with 198 of 198 OK).
- The gate's support-suite step as prescribed (command 11) is refused by the suites' own guard against `build/rewrite-*` candidate paths. That is a problem with the gate prescription, not with the candidate. Future gate briefs should point `PLANAR_CANDIDATE` at a copy.
- Timings are macOS only (8 cores). The "idle" host was busy. Linux covers aarch64 only.
- The two slow Catch2 tests were not run under mutation. Only their registration and TIMEOUT property are under test.
- My load-scaling estimate in R1 is an extrapolation, not a measurement.
