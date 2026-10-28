# V5-02-X06-D-C03 independent review (corrective for the X06-D-C02 review)

Disposition: **request-changes** (not records-only).

This is the fourth request-changes review in the X06-D lineage. Under the operator's decision for this round, the chain
stops here and the operator decides how to continue.

C03 closes most of the C02 review:

- **R2 is closed properly.** A new role test with non-empty current and base trees and padded registry and observation
  reads is killed by name by X11, X11b, M_REG and M_OBS. The I1b row and comment now name X12 and X13, its real killers.
- **R3 is closed.** X4 is no longer called equivalent: a new direct shape test kills it. Restore-order mutants at
  `call_retained_value` and the shape path of `invoke_shape_impl` are killed by name. `call_retained_assembly`, the
  `prepare_load` route, X2 and X3 carry source-based equivalence arguments that I spot-checked. The misleading test
  comment is corrected, and clauses `:396` and `:455` are cited.
- **The R5 stale sentences named by the C02 review are gone.**
- **The gates hold on the exact bytes**, with all 1110 prior names preserved.

The finding that matters most is still open. The round record says so itself:

- **C02-R1 is not met** (C03-R1). No margin table was measured in either configuration. The P0 sizing package wrote
  "NOT MEASURED". The ceilings were then raised to values that no evidence derives (native 41,943,040, instructions
  18,000,000, nodes 25,500). Two source comments now point at a margin record that does not exist.

Two record findings and two documentation findings come on top of that:

- **R4 is incomplete** (C03-R3). The record links `plan/DECISIONS.md` instead of copying it, and it leaves out the
  equivalence arguments of `plan/INVARIANTS.md`.
- **The roadmap does not carry the named carried limits** CL-1 to CL-6 (C03-R2).
- **One new R5 sentence cites a file that does not exist** (C03-R4).
- **Two test comments are inaccurate** (C03-R5).

## Exact candidate

I reviewed in the shared checkout on `dev/planar-v2-vernacular` at HEAD `c875e08acdddf7b255ee0ff4c9533a59c872f81c`. The
candidate is uncommitted. The `identity.py` output is saved verbatim under `tests/native/v5-02-x06-d-c03-evidence/review/`:

- `identity-src-before.txt` and `identity-src-after.txt` (pathspec `src`) are byte-identical.
- `identity-before.txt` covers pathspec `src docs`.

| pathspec | tracked-only sha256 | combined sha256 (untracked included) |
|---|---|---|
| `src` | `c740bd2bac8bd94eb5eb216c10a46f7c59a63b446a5e137441928879582ad276` | `81972b34782baf628f2a63029ac5987a0605e92946246a5a138995456b485d49` |
| `src docs` (before this file existed) | `83f49b06618c33d9eacf9dd23bc9e5b860ab918092210293252f8a7835d4555a` | `100bf8d5b8c5205fd7dfe9ab6aec3dd420b39a17d59fe91b939435f4f21aeada` |

The `src` values equal the final record, gate run 3 and probe run 2. So does every per-file hash in the record's table,
including `src/engine/runtime/external_registration.t.cpp` (`feb5b215...`), `src/engine/runtime/runtime.t.cpp`
(`a444106a...`), `src/engine/CMakeLists.txt` (`d0952563...`) and the untracked `src/engine/budget/budget.cppm`
(`af85103b...`).

The `src docs` combined value covers the untracked round records, so it changes with every record edit, including this
review file.

Both documents also match the record: `docs/architecture.md` (`0869e6f4...`) and
`docs/vernacular/profile-preparation-architecture.md` (`16e9fc21...`).

`git diff --stat HEAD -- src/engine/runtime/runtime.cppm src/cmd` is empty. `runtime.cpp` keeps its C02 hash
`4100290b...`, so no product behaviour changed in C03, and diagnostics are byte-identical to the C02 candidate.

I wrote only this file and new files under `tests/native/v5-02-x06-d-c03-evidence/review/`. I applied no mutant, and I
edited no source, test, record or existing evidence. I did not commit, push, merge, install or publish. I used no old
Planar tool, queue, skill, role or installed binary.

With no other ninja or ctest process running, I ran these one at a time:

- `cmake --build build/rewrite-debug --target planar_tests`: exit 0, `ninja: no work to do.`
- `ctest --test-dir build/rewrite-debug -R 'X06 B3 C a file replaced at any allocation inside prepare_load is never published as verified' --no-tests=error`:
  run three times; exit 0 each time; 17.49 s, 17.99 s and 17.32 s.
- The same command for sibling test #561 (`... an in place same size edit ...`): exit 0, 11.85 s.
- `ctest --test-dir build/rewrite-debug -R X06 --no-tests=error --output-on-failure -j4`: exit 0, 165 of 165 passed,
  0 skipped.
- `ctest -N` in `build/rewrite-debug` and `build/rewrite-release`: exit 0, 1114 names each.

## Independent verification

Evidence paths are under `tests/native/v5-02-x06-d-c03-evidence/` unless they say otherwise.

| Check | Result |
| --- | --- |
| Gate exits | Every `raw/run3/*.exit` reads 0, except `g9-nofiles.exit`, which reads 1. That is the no-argument redaction form; it scans ignored build output and is not a gate. The explicit-file form `g9-files` passed. |
| Gate order | By modification time, `g3-build` (18:09:01, `ninja: no work to do`, so the binaries were frozen) comes first. G1 follows: `g1-x06` at 18:09:44 (165 of 165) and `g1-budget` at 18:10:15 (36 of 36). Then come the full Debug run, the release configure, build and full run, the inventories, lint, the two no-change builds and the redaction check. G1 is the first test run, as in the accepted C02 order. |
| Full runs | Debug: 1114 of 1114 passed. RelWithDebInfo: 1114 of 1114 passed. The only "skip" text in either log is in the names of two passing tests (#626 and #1086). |
| Names | My own comparison (`review/removed-vs-c02.txt` is empty; `review/added-vs-c02.txt` lists 4 names). None of the 1110 C02 names was removed or renamed. The 4 added names are the ones the record lists. The Debug and RelWithDebInfo sets are byte-identical. |
| Guarded test #559 | Gate run 3: 16.28 s in the `-j4` Debug run. My serial runs took 17.3 to 18.0 s, at a load average of 7.8 on 8 cores. Sibling #561 (unchanged in C03) slowed in the same proportion. The #559/#561 ratio is 1.49 in the C02 gate (15.35/10.28) and in C03 gate runs 1 and 3 (17.42/11.66 and 16.28/10.92). So the extra seconds are host load, not the candidate. C03 changed no product code on this path, and the probe scratch measured 15.47 s alone. I accept G-2 on that basis. The 16 s figure itself was not reproduced on a quiet host. |
| Lint and no-work build | `g7-lint.log` ends "Formatting and Doxygen checked 143 active source files". `g8-build2` and my build both say `ninja: no work to do.` |
| R2 kills | `probes/superseded-1/raw/C3-6a.json` to `C3-6d.json`: X11 (current-tree `require(session.tree(current, exclusions))` deleted), X11b, M_REG (`r.name != "registry"` added to the read-charge condition in `native.cpp`) and M_OBS. Each was applied (build exit 0), its restore hash checked out, and the C03 role test failed by name (M_OBS also fails the C02 enumeration test). I read the diffs; they are the right edits. The test asserts strictly positive drops and calibrated floors on real bisected boundaries; it is not hollow. |
| R3 kills | C3-9a (M_O2, the swap at `call_retained_value`) fails only the C02 known-load prepare-frame test. C3-9b (M_O4, the swap on the shape path of `invoke_shape_impl`) fails only the new direct-shape test. C3-9e (X4, the guard `&& !exhausted(context(l))` removed) fails only the new recovered-refusal test. Each kill is named, with a correct diff. |
| R3 equivalence | M_O3, M_O5, X2 and X3 survive. I checked E3 against the source: `call_retained_assembly` has one caller (`runtime.cpp:6403`), and that callback returns early on `exhausted(s) \|\| c.bridge_error` (`:6301`). The arguments rest on what runs, not on the trusted-Lua fact F, and the acceptance allows them. |
| R3 comment | The comment above the known-load test (`runtime.t.cpp:4202-4208`) names M_O2 as its kill, names M_O3, M_O5, X2 and X3 as recorded equivalent, and says X4 is killed by the direct-shape test. It no longer mentions M_R4c or M_R4d. Accurate. |
| Red-first | `red/RED.md` records two behavioural assertion failures on snapshot `ed58f909...`. One is the quarter-free test (`runtime.t.cpp:3824`). The other is the role test (`external_registration.t.cpp:4636`, `REQUIRE(both_trees.ok)` failing on nodes), but in its N=40 form. For the delivered N=1 test, probe C3-1 (`NODE_MAX` 18000) fails it by name. That log has no `--output-on-failure` detail, so the failing line is not shown. Product code did not change, so this is an acceptable stand-in. |
| No mutants in the checkout | The `src` identity is equal before and after both probe runs and my review. Every mutant JSON records a scratch build and a verified restore. Accepted. |

## What is sound

- **C02-R2 / C01-R1.** These are met, and recorded as met.
- **C02-R3.** This is met: every site is either killed by a named test or carries an equivalence argument. X4 was
  correctly reclassified as not equivalent after a reachability reading (fact F does not hold for untrusted Lua after a
  recovered allocator refusal), and it is now killed.
- **CL-1 to CL-6.** These exhaustion-first sites outside the five C02 sites are recorded in the round record with file
  and line, verdict, untrusted caller and both clauses. They are operator-approved (2026-10-27, option A), and I accept
  them as carried limits, not open findings.
- **C02-R5 as requested.** "Registration ... remain future integration" and "X06 whole-invocation accounting remains
  open" are gone. The 512 MiB live Lua ceiling is named in both documents. "Each owner keeps its original accounting
  storage" is qualified (`profile-preparation-architecture.md:416-419`).
- **C02-R6.** The C02 record is corrected (clause citation, timing as a spread with history), and the gate REPORT note
  is kept.
- **Process discipline.** It improved: the decisions table exists, the plan file revisions are kept, raw probe and gate
  logs are kept, and the record states its own unmet criteria plainly.

## Findings

### C03-R1 (required; evidence, likely `src/engine/CMakeLists.txt` and test comments): C02-R1 is still not met

The acceptance requires:

- a margin table per fixture and configuration, measured on the final candidate after the last ceiling change;
- at least 25 percent margin on native, live Lua, instructions and nodes for every non-bracketing fixture, in both
  configurations;
- every bracketing fixture listed with its reason.

None of that exists:

- `sizing/SIZING.md` opens with "NOT MEASURED". No file in the C03 evidence holds a margin table (`find -iname
  '*margin*'` finds nothing).
- The record's own acceptance table marks R1 **NOT MET**, and its amendment 1 says the reason for the final ceilings is
  "not recorded". They are native 41,943,040, instructions 18,000,000 and nodes 25,500. None of them is a planning value
  of DECISIONS D8/D14 (36,700,160 / 16,000,000 / 30,000).
- The correction loop of DECISIONS D15 (measure the frozen candidate in both configurations, one repair pass, re-measure)
  never ran.
- Two source comments now state something untrue:
  - `src/engine/CMakeLists.txt:122-125` says bracketing fixtures are "listed in the C03 margins record", which does not
    exist.
  - The comment above `X06 D C03 ordinary fixtures keep a quarter of every observable ceiling free`
    (`runtime.t.cpp:3824-3826`) says the unobservable margins "are covered by the measurement table". It also quotes the
    stale figure 24,963,619, where RED.md measured 24,503,619.
- What the quarter-free test shows is real but partial. It covers the ordinary control's native, instruction and open
  margins. It does not cover the role flows, the I1/I1b/I2/T8/T9/R2/R4 fixtures, live Lua or nodes. The role-test
  comment itself records 346 nodes and 218 KB native per entry per tree. So the role flows' node and native margins at
  25,500 and 41.9 MB are unknown: this is exactly the fragility C02-R1 named.
- The two equality pins added by the repair (`CHECK(budget_instruction_max == 18000000)` and
  `CHECK(budget_registration_instruction_max == 1000000)`) are configuration change-detectors, not margin properties.
  Their reason is honest: the margin checks are upper bounds and cannot notice a lowered ceiling. But they fail on any
  deliberate re-sizing, which R1 will probably require.

Required:

1. Run the D15 measurement in a scratch copy with the `margins.md` instrumentation, in Debug and RelWithDebInfo, on the
   final candidate. Cover every row that INVARIANTS C3-3 lists.
2. Set the ceilings from that measurement, or confirm the current ones, so that every non-bracketing fixture has at
   least 25 percent margin.
3. List every bracketing fixture per column, with its D9 reason.
4. If a ceiling changes, update the two pins and re-run the ceiling-dependent probe rows. Then measure again, after the
   last change.
5. Make the CMake comment and the quarter-free test comment point at the real table.

### C03-R2 (required, records): the roadmap does not carry the named carried limits CL-1 to CL-6

The record's row C3-16 and DECISIONS D5b/D13 make CL-1 to CL-6 operator-approved limits **only if** they are named in
both the round record and the roadmap's `Next work`. Each entry needs its verdict, untrusted caller, clauses `:396` and
`:455`, and the statement that a later round repairs it.

`docs/vernacular/roadmap.md` has zero hits for `call_retained_projection`, `4713`, `4887`, `4298`, `6182`, `3505`,
`:396`, `:455` and `CL-1` (`review/audit.txt`). The record marks the roadmap half NOT MET and defers it to the
coordinator. Because the review is the last gate before commit, the roadmap entry must exist in the reviewed state, or
the record must say explicitly that the operator accepted a post-review roadmap edit. Records only.

### C03-R3 (required, records): R4's "full copies" are not in the record

The dispatch says the coordinator copies `plan/INVARIANTS.md` and `plan/DECISIONS.md` "in full into this record at
sync", and acceptance R4 requires the record to hold both. The record does three things:

- It copies the invariant table rows and adds outcome columns.
- It **links** `plan/DECISIONS.md` (D1 to D23, including the operator answer D5b and the sizing loop D15) instead of
  copying it.
- It omits the non-table parts of `plan/INVARIANTS.md`: the package list, the equivalence arguments (fact F, E1 withdrawn,
  E2, E3, E5, E3b) and the probe dispatch (section D). `grep -c 'Fact F'` on the record is 0.

The R3 equivalence dispositions rest entirely on those arguments, and the evidence folder is gitignored except for its
README. So once committed, the record would cite arguments that are not in the repository. Copy both files verbatim
(or the full argument and decision text) into the record, and correct the R4 row, which now says MET. Records only.

### C03-R4 (minor, documentation): a new R5 sentence cites a file that does not exist; one stale sentence remains

- `docs/architecture.md:88-89` says the 512 MiB live Lua ceiling is "`memory_max` in `src/engine/lua/runtime.cpp`".
  That file does not exist. The 512 MiB constant is `src/engine/runtime/runtime.cpp:32`. The Lua loader has its own
  64 MiB `memory_max` (`src/engine/lua/loader.cpp:12`), so the wrong path also invites confusion between the two.
- `docs/architecture.md:1510-1512` still says, in the present tense, "whole-invocation shared capture/VM accounting
  stays open for X06". It sits in an X05-stage paragraph, but it now contradicts the composed-budget paragraph at
  `:78-95`. The C03 record flagged it for the reviewer. Qualify it as historical (for example "stayed open at X05;
  composed by X06") or remove the clause.

Both are description-only edits.

### C03-R5 (minor, test comments): calibration and margin comments are inaccurate

- The calibration comment of the C03 role test (`external_registration.t.cpp:4575-4580`) still describes the floors as
  calibrated at "N = 40 ... boundary b0 = 694446". The delivered constants are for `c03_entries = 1` (tree floor 3391
  from M = 3768). The record lists this as a known inconsistency, but standing lesson (1) asks for comments that say
  truthfully what each test measures and kills.
- See C03-R1 for the two margin-record comments.

Comment-only edits in test files and `CMakeLists.txt`.

## Records-only status

Not records-only. C03-R1 needs a new measurement in both configurations, probably a ceiling change in
`src/engine/CMakeLists.txt`, and comment edits in `CMakeLists.txt` and `runtime.t.cpp`. If a ceiling moves, the test
pins and ceiling-dependent probes must be re-run. C03-R5 needs test-comment edits.

C03-R2 and C03-R3 are records only, and C03-R4 is documentation only.

## Limits carried and observed

- I accept the limits accepted by the C01 and C02 reviews: L6d, L7d, N2, N2d, N6c, I4e, I4f, E2, E3, E6b, E11, E12,
  E22, E25a to E25c, I3b, I10d, the dead pre-attach paths (T8 survives as I25 requires), the verification reread
  transient buffers, P01 masked, and Linux not run.
- I accept CL-1 to CL-6 as operator-approved, subject to C03-R2.
- C03 survivors I accept as recorded:
  - C3-4e and C3-4e2: single-file role padding is not load-bearing at the current ceilings.
  - C3-4f: its premise does not hold at 25,500; C3-4f2 kills it with the premise made true.
  - C3-4h, C3-4d3 and I17c: killed, but not by the row's named test.
  - C3-6e and C3-6f: no independent mutant.
  - The role test runs at one entry per tree (amendment 2). It meets the acceptance's "non-empty trees", but it shows
    node headroom for only one entry. That is part of why C03-R1 matters.
- I did not re-run mutants. I read the diffs, JSON results and failing-test lists for C3-1, C3-6a to C3-6d, C3-9a to
  C3-9g and the run-2 rows. Probes ran in Debug only; the C02 runtime-test rows listed as "not probed" were not re-run.
- The guarded-test timing was not reproduced at about 16 s on a quiet host (see the verification table).
- No RSS or capacity claim is made.

## Review evidence

The files are under `tests/native/v5-02-x06-d-c03-evidence/review/`:

- `identity-before.txt` and `identity-after.txt` (pathspec `src docs`). The tracked-only hash is unchanged. The combined
  value is `e724a285...` after, because this review file is a new untracked path under `docs/`.
- `identity-src-before.txt` and `identity-src-after.txt` (pathspec `src`; identical).
- `rv1-build-tests.log` and `.exit`: the `planar_tests` build, with no work done.
- `rv2-guarded-alone.log`, `rv3-guarded-repeat1.log`, `rv3-guarded-repeat2.log` and their `.exit` files, `rv3-load.txt`,
  `rv4-sibling-561.log` and `.exit`: the #559 and #561 timings and the host load.
- `rv5-x06-focused.log` and `.exit`: the focused X06 run, 165 of 165 passed.
- `rv6-N-debug.log`, `rv6-N-release.log` and their `.exit` files, `names-debug.txt`, `names-release.txt`,
  `c02-sorted.txt`, `removed-vs-c02.txt` (empty) and `added-vs-c02.txt` (4 names).
- `audit.txt`: the greps behind the findings.
