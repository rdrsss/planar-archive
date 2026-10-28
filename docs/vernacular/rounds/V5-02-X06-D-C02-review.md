# V5-02-X06-D-C02 independent review (corrective for the X06-D-C01 review)

Disposition: **request-changes** (not records-only).

This is the third request-changes review in the X06-D lineage, so under the round's own rule the chain stops here and
the operator decides how to continue.

C02 closes most of the C01 review. The factory budget now reaches extension role preparation, and the role test kills
the role mutant by name. The sink's own instruction refusal is observed: T3, T3c, T3d and T16 are each killed by a
named C02 test. The B3 stage tests T8 and T9 exist, and each is killed by a stage-specific mutant. A real engine
protocol and semantic failure caught by `pcall` in a profile-owned frame is pinned by a named test. `budget_sink` now
lives in a new `planar.engine.budget` module beneath the loader and admission. The C01 record corrections are in place.
The bytes are the recorded ones, and the gates hold.

Four findings still block approval:

- **R5 headroom is not demonstrated** (C02-R1). The record says so itself. No table covers the raised ceilings, and
  RelWithDebInfo was never measured. The projected native margins are under 25 percent, and the node ceiling was left
  with no slack.
- **R1's enumeration half is not pinned** (C02-R2). Because of that missing node headroom, the survivor X11 stands, and
  R1 is recorded as "met with a limit". The acceptance requires "met".
- **The first-failure repair is unproven** (C02-R3). It changed five product sites. Its test kills none of the mutants
  its comment names, and no probe covers the four reordered checks.
- **The invariant table is incomplete** (C02-R4). The dispatch table reached the round truncated, for the second
  round running. It has no row for C01-R7, and the search decisions table is not in the repository.

## Exact candidate

I reviewed in the shared checkout on `dev/planar-v2-vernacular` at HEAD `c875e08acdddf7b255ee0ff4c9533a59c872f81c`. The
candidate is uncommitted. The `identity.py` output was the same before and after my work. It is saved verbatim in
`tests/native/v5-02-x06-d-evidence/review-c02/identity-before.txt` and `identity-after.txt` (pathspec `src docs`).

| pathspec | tracked-only sha256 | combined sha256 (untracked included) |
|---|---|---|
| `src` | `f5961de92f66cfd686fb4f78dadd8355e7cfd98f466540edc10d714c8d4e1451` | `c45946d4f117610e31f7af1509698aa073ebbcab8b58af7f3e858aa72c68c2c9` |
| `src docs/architecture.md docs/vernacular/profile-preparation-architecture.md` | `ae2edabaf70beeb4d25b68304a6a98d31505889b3396e9406ec7aee59e8bb18e` | `401c718b7f4e02f21b25e3b034b60776ef5ad32da9d0416d5e4b5f22fd95706e` |
| `src docs` | `81faf2de9016ba1927ded78abbd909e6d5267b775a2a4fc7cf9918aee300994a` | `3819e36802237b19ccd03f611da2eb36cb2e74e1f7e703c46fb260c3c1ddf1d8` |

The `src` and `src`-plus-R6-docs values equal the final record, gate run 3 (`raw/run3-identity-before.txt` and
`run3-identity-after.txt`) and probe run 2 (`probes/raw/identity.txt`). Every per-file hash in the record's table matches
the working tree, including the untracked `src/engine/budget/budget.cppm`. The `src docs` value covers the round records
too, so it changes when they are edited. In `identity-after.txt`, the tracked-only `src docs` hash is unchanged, but the combined value is `ea7fb31f...`. The only cause is this review file, a new untracked path under `docs/`. The `src` hashes are identical before and after. `runtime.cppm` is unmodified. `src/cmd/` is unmodified. Under
`src/engine/ledger/`, only `acceptance.t.cpp` changed, and it is byte-identical to the C01 candidate.

I wrote only this file and new files under `tests/native/v5-02-x06-d-evidence/review-c02/`. I applied no mutant and
edited no source, test, record or existing evidence. I did not commit, push, merge, install or publish. I used no old
Planar tool, queue, skill, role or installed binary. With no other ninja or ctest process running, I ran these
commands one at a time on `build/rewrite-debug` (plus `ctest -N` on `build/rewrite-release`):

- `cmake --build build/rewrite-debug --target planar_tests`: exit 0, `ninja: no work to do.`
- `ctest --test-dir build/rewrite-debug -R 'X06' --no-tests=error --output-on-failure -j4`: exit 0, 161 of 161
  passed, 0 skipped.
- `ctest --test-dir build/rewrite-debug -R 'X06 B3 C a file replaced at any allocation inside prepare_load is never published as verified' --no-tests=error`:
  exit 0, test #559, 15.47 s.
- `ctest -N` in both build trees: exit 0, 1110 names each.

## Independent verification

Evidence paths in this table are under `tests/native/v5-02-x06-d-c02-evidence/` unless they say otherwise.

| Check | Result |
| --- | --- |
| Gate exits | Every `raw/run3-*.exit` file reads 0, except `run3-g11b-redact-noargs.exit`, which reads 1. G11b is the no-argument redaction run. It fails only on ignored build output and `.claude/settings.local.json`, and it is not a gate. G11 on the 32 explicit files passed. |
| G1 order | By modification time, `run3-g0-build-tests` (12:19:13, no work) comes first. `run3-g1-focused` (12:20:45, 161 of 161, 91.97 s) follows. Then come the full Debug run (12:27:35), the release configure, build and full run (12:31:18), lint, the two no-change builds and the redaction check. G1 therefore ran first, on frozen binaries. |
| Full runs | Debug passed 1110 of 1110 in 409.92 s. RelWithDebInfo passed 1110 of 1110 in 162.58 s. The only "skip" text in either log is inside the names of two tests that passed. |
| Names | I compared the sets myself (`review-c02/removed-vs-c01.txt`, `added-vs-c01.txt`). None of the 1098 C01 names was removed or renamed, and 12 were added; they are listed in the record. The Debug and RelWithDebInfo sets are byte-identical and equal `inventory.txt`. |
| Guarded test timing | Test #559 (formerly #531 and #556) took 15.35 s in the gate and 15.47 s in my run alone. Earlier history: 15.29 (C01), 15.39, 15.46, 15.47 (X06-D), 15.52 and 15.97 s. These figures are within the run-to-run spread around "about 15.3 s", so I do not treat this as a regression. |
| Lint and no-work build | `run3-g8-lint.log` ends "Formatting and Doxygen checked 143 active source files". `run3-g9b` and my own build both say `ninja: no work to do.` |
| Diagnostics | No message literal was changed or removed. The two removed lines in the diff are both re-added: `Lua allocation exceeded` stays as the fallback branch in `loader.cpp`, and the route `command_resource` refusal moved below the `bridge_error` check. No new code, message or public member was added. The closed-message test and the seam guard pass in both full runs. |
| R2 kills | `probes/superseded-1/raw/T3.log`, `T3d.log` and `T16.log` each fail `X06 D C02 the composed sink refusal is the first failure on a declaration chunk that crosses the invocation ceiling`; T16 also fails three other tests. `T3c.log` fails `X06 D C02 the composed sink refuses a pre-frame declaration chunk at the carried ceiling` alone. Each was applied (exit 0 build), and its restore hash checked out. The product file, `CMakeLists.txt` and these test bodies are byte-identical between probe runs 1 and 2. Only the T9 slope pin changed in `runtime.t.cpp`. So carrying these rows forward is valid. |
| R3 kills | rowT8 deletes the post-route exhaustion check and maps route failures to protocol. It fails only `X06 D C01 route stage exhaustion refuses before any declaration is captured`. rowT9a maps the capture-stage bundle failure to protocol. rowT9b deletes the phase-6 charge. Both fail `X06 D C01 native exhaustion after all-job validation refuses in preparation and cleans nested roots`, and so does X6 (`* 2`). These mutants are stage-specific. Both tests add observation to existing C01 behaviour, so the dispatch allowed proof by probe instead of a red snapshot. |
| R1 kill | I1 (`capture_session session(s.budget);` changed to `capture_session session;`) fails only `X06 D C02 role preparation charges declaration chunks to the factory budget`. That test was red-first on an assertion (`red/raw/t1.log`, `external_registration.t.cpp:4462`). I2 fails the no-factory test by name. |
| R4 test | `X06 D C02 a protocol or semantic failure caught by pcall in a profile-owned frame stays terminal` runs on `profile_owner()`, the factory-adopted extension frame. It raises a real engine protocol failure under `pcall`: `ctx.read_history()` misuse, and a second read. It also raises a typed semantic failure (`history_read_error` `revision_not_found`). It then checks that both survive a later success, semantic, `error('later')` and exhausting tail. M_R4a and M_R4b fail it by name (`superseded-1/raw/M_R4a.log` and `M_R4b.log`). This meets C01-R4's requirement. The T10 row text in the C01 record is corrected. |
| R6 graph | `planar_budget` (`budget/budget.cppm`) depends only on `planar_value`. `planar_lua_loader` depends on `planar_value`, `planar_lua` and `planar_budget`, not on admission. `planar_capture_admission` depends on `planar_value` and `planar_budget`, and it `export import`s the budget module, so existing consumers still see `budget_sink`. There is no cycle. The only loader-to-admission edge left is test-only: `planar_lua_loader_tests` links admission, and `loader.t.cpp` imports it. That is acceptable. The graph test was red-first (`red/raw/t12.log`). |
| Inventory 7a to 7c | The cited sources are correct. `acceptance/load.cpp:54` calls `source_capture::open_embedded(workspace, registry)` without a budget, and `src/cmd/planar/invocation.cpp:100` holds `capture_session _session`. The remaining product session sites take the budget: `profile.cpp:73`/`74` uses `set_budget`, and `runtime.cpp:5327` passes `impl->budget` to `open_bound`. No pre-attach declaration stage was added, because `prepare_role` runs inside an admitted, executed extension owner. |
| R7 | In the C01 record: the R4 row now says that the table was not complete, R1 now reads "not met at C01", V1 records the cosmetic `file(READ)` clause and the V2b 300-character window gap, T9b stays a GAP, and the full probe dispatch table is restored. |

## What is sound

- **R1 source change.** `extension_invocation::prepare_role` builds `capture_session session(s.budget)` (`runtime.cpp:5965`).
  `extension_invocation::create` keeps a null budget, and the no-factory test pins that.
- **R2 closed.** The sink's own refusal is now the first observable failure. The test runs an erroring tail under a
  two-sided window anchored on the valid control's stage marks. A quantum-window check rather than exact equality is
  a reasonable observable, given the loader's 100-instruction quantum, and it still separates `>` from `>=`. The L7c
  path is tested with a sink that refuses state creation, and L7c is killed by name in both the focused and full runs.
- **R3 closed.** See the kills above.
- **C01-R4 closed** by the profile-owned frame test.
- **The first-failure direction of the repair is consistent with the architecture.** The record says no contract clause
  governs it. Two do: `profile-preparation-architecture.md:396` ("one terminal first failure from any active stage")
  and its adversarial row at `:455` ("Semantic/protocol/resource failure caught by Lua | First terminal failure
  survives"). Letting an earlier `bridge_error` win over a later exhaustion follows them. The problem is that the
  change is unproven (C02-R3), not that it points the wrong way.
- **R6 module move.** Done, with no cycle.
- **R7 corrections.** Done.

## Findings

### C02-R1 (required, evidence and tests; possibly `CMakeLists.txt`): R5 headroom is not demonstrated

C01-R5 and this round's acceptance require "a peak and margin table per ceiling in both configurations, all margins at
least 25 percent or the ceiling raised and affected tests and probes re-run". The final record marks R5 **not met**, and
the bytes confirm it:

- `margins.md` measured a pre-raise snapshot, in Debug only: native 26738688, instructions 4000000, nodes 18000. Its
  own identity is a different candidate (`fc9e2126...`). There is no RelWithDebInfo measurement and no measurement of
  the raised ceilings.
- Its own projection for the raised ceilings gives native margins of 22.1 percent on the ordinary control and 20.6
  percent on the calibration peak. Both are below 25 percent.
- The node ceiling stayed at 18000 even though the role flows were measured at a 2.0 percent node margin (17,646 of
  18,000). The I1b test now says this in its own comment (`external_registration.t.cpp`): "Trees must stay empty: the
  node ceiling has no slack for even one more captured entry."

Fixtures that are deliberately bracketed against a ceiling, such as the role-flow native bracket, may be excluded from
the margin rule if the record says so. However, a must-succeed fixture that cannot add one tree entry is exactly the
fragility C01-R5 named, and it directly causes C02-R2.

Required:

1. Measure, in Debug and RelWithDebInfo, the peak native bytes, live Lua, instructions and nodes of every must-succeed
   fixture that relies on each `PLANAR_BUDGET_*` and `PLANAR_TIGHT_*` ceiling, on the final candidate. Use scratch
   instrumentation, never the checkout.
2. Raise the node ceiling, and the native ceiling if needed, to at least 25 percent headroom over those peaks, or
   record each deliberately bracketed fixture as excluded, with its reason.
3. Re-run the affected tests and probes, and confirm that the refusal tests still refuse.

### C02-R2 (required, tests and records): R1's tree-enumeration charge is not pinned, and R1 is recorded as "met with a limit"

C01-R1, as restated in this round's objective, requires a test that shows "role-stage declarations **and enumeration**
charging the one owner". It also says that the current and base tree enumeration, and the registry and observation
reads, charge the owner. The acceptance says "R1 is recorded as met". The record instead marks R1 "met with a limit",
and probe X11 (`prepare_role`: `require(session.tree(current, exclusions))` deleted when capture-outputs is set)
survives `ctest -R X06` (`probes/raw/X11.log`).

X11 survives because the I1b test must keep both trees empty: it has no node headroom (C02-R1). The registry and
observation read charges are not pinned by a role-stage test either. X1 is killed only by `X06 D capture stage charges
every sealed byte to the one native counter`, which is not a role test. Also, the table's own I1b row names the I1
mutant, and I1 does not kill that test (PROBES.md finding 2).

Required: once there is node headroom, give the I1b test (or a new test) non-empty current and base trees and a
registry or observation read whose charge moves the boundary. Show that X11 and a registry/observation-read-charge
mutant each fail it by name. Then record R1 as met, with the I1b row naming the mutant that actually kills it.

### C02-R3 (required, tests and records): the first-failure repair in `runtime.cpp` is unproven, and its test comment is false

After gate run 1, the repair coder changed product behaviour at five sites:

- `command_failure` records nothing once a ceiling is exhausted.
- In `call_retained_value`, `call_retained_assembly`, the shape path of `invoke_shape_impl` and the route phase of
  `prepare_load`, the `bridge_error` check now comes before the `exhausted` check.

The record lists only three of the four reordered sites; it omits `call_retained_assembly`.

`X06 D C02 a protocol failure caught by pcall in a known-load prepare frame stays terminal` failed on an assertion before
the repair (`red/raw/t13.log`: `"resource_limit" == "internal"` at `runtime.t.cpp:4156`), so some part of the repair is
needed. But no probe shows which part:

- X4 (the `command_failure` guard removed) survives both the focused run and the full suite. X2 and X3 (the
  pre-existing `shape_failure` precedence and post-route `bridge_error` check) survive as well.
- No mutant restores the original order at any of the four reordered sites. None of the four swaps has a killed
  mutant.
- The test's comment (`runtime.t.cpp`, above the test) says it is aimed at M_R4c and M_R4d. Probe run 2 shows it kills
  neither. Standing lesson (1) requires an accurate statement of which mutant each new test kills.

Required:

- Probe a restore-order mutant at each of the four reordered sites. Name the test that kills each one, or record each
  as equivalent with a reachability argument.
- Do the same for X4: name a reaching fixture, or record it as equivalent with a reason.
- Correct the test comment so it names the mutants it really kills (a comment-only test edit).
- Cite `profile-preparation-architecture.md:396` and `:455` in the record as the governing first-failure clauses.
- Add `call_retained_assembly` to the record's list of repaired sites.

### C02-R4 (required, records; probes for any recovered row without a kill): the invariant table is incomplete

The orchestration process requires search to write one row per acceptance criterion and closed finding, and its notes
to open with a decisions table. In this round:

- The dispatch table again reached the round truncated, the same defect C01-R7 reported. Rows I15 to I28 were rebuilt
  from `RED.md` and `PROBES.md`. Rows I19 to I22 and I24 to I28 have no recoverable text and no status.
- No row covers C01-R7. The acceptance requires one for each of C01-R1 to C01-R7, even if only as a record correction.
- The L7c test has no table row, although R2's acceptance names it.
- The search notes and their decisions table are not in the repository or the evidence folder.

Required:

- Recover or rewrite rows I19 to I28 with their invariant, mutant and test.
- Add an R7 row (record correction) and an L7c row.
- Store the search decisions table, or a faithful copy, in the record or the evidence.
- Probe any recovered row that names a mutant without a recorded kill.

### C02-R5 (minor, documentation): R6 text is accurate, but it leaves stale or contradictory sentences nearby

`docs/architecture.md` now describes the composed factory budget, the stages it covers, the outside-the-factory sessions
and the new module. Problems remain:

- The paragraph just before it still says "Registration ... remain future integration", while the new paragraph says
  the budget composes registration.
- Around line 1540, "X06 whole-invocation accounting remains open" is now stale.
- Neither document names the 512 MiB live Lua ceiling as enforced by the composed owner, which the dispatch objective
  asked for.
- In `profile-preparation-architecture.md`, the next sentence after the edit still says "Each owner keeps its original
  accounting storage". This reads as contradicting "composed into the one factory budget owner" unless it is qualified
  (for example, the stage objects keep their storage but charge the one owner).

Fix these with description-only edits.

### C02-R6 (minor, records): smaller record inaccuracies

- The record's limit "the first-failure precedence change cites no contract clause" is incorrect (see C02-R3).
- The gate REPORT's limitation about probes not being re-run is superseded, as the README says. Keep that note.
- Describe the guarded-test timing as within the observed spread, with the history figures, rather than "at the limit,
  0.3 percent over".

## Records-only status

Not records-only:

- C02-R1 needs new measurements, and probably a `CMakeLists.txt` ceiling change.
- C02-R2 needs a test change and probes.
- C02-R3 needs new probes and a test-comment edit.

C02-R4 is records-only except for any probes its recovered rows require. C02-R5 is documentation only, and C02-R6 is
records only.

## Limits carried and observed

- The limits accepted by the C01 review stand: L6d, L7d, N2, N2d, N6c, I4e, I4f, E2, E3, E6b, E11, E12, E22, E25a to
  E25c, I3b, I10d, the dead pre-attach paths (T8 survives as required) and the verification reread transient buffers.
  P01 is still masked.
- I1b has no red-first evidence (it passed at the snapshot). Its proof is the X12 and X13 kills, as the dispatch
  allowed for observation tests.
- I did not re-run mutants. I read the raw diffs, JSON results and failing-test lists for I1, I2, X11, X2, X3, X4,
  rowT8, rowT9a, rowT9b, T3, T3c, T3d, T16, M_R4a and M_R4b. That a coder applied no mutant in the checkout is accepted
  from the records: the prober and the margins package worked in scratch copies, and the checkout bytes match every
  recorded identity.
- I could not verify that no raw evidence was deleted beyond the fact that the superseded folders (`REPORT-pre-repair.md`,
  `REPORT-run2.md`, `probes/superseded-1/`, `raw/pre-repair/`) are present. The folders are gitignored.
- Linux was not run (X08). No RSS or capacity claim is made.

## Review evidence

The files are under `tests/native/v5-02-x06-d-evidence/review-c02/`:

- `identity-before.txt` and `identity-after.txt`: the `identity.py` output (pathspec `src docs`) before and after my
  runs.
- `rv1-build-tests.log` and `.exit`: the `planar_tests` build, with no work done.
- `rv2-ctest-x06-focused.log` and `.exit`: the focused X06 run, 161 of 161 passed, 0 skipped.
- `rv3-guarded-timing.log` and `.exit`: test #559 run alone, 15.47 s.
- `rv-N-debug.log`, `rv-N-release.log` and their `.exit` files, `names-debug.txt`, `names-release.txt`,
  `c01-sorted.txt`, `removed-vs-c01.txt` (empty) and `added-vs-c01.txt` (12 names): the name comparison.
- `audit.txt`: the greps behind the findings: session construction sites, ceilings, margins lines, the repair hunks,
  the test comment, the architecture first-failure clauses, module edges and the stale documentation text.
