# V5-02-X06-B3-B-C03 independent review (third corrective round: survivor tests, records and evidence)

Disposition: **request-changes**. Only the records need fixing. No source change, test
change, gate re-run or probe re-run is needed.

The engineering work in C03 is sound, and I could reproduce or check every part of it from
raw bytes:

- The four product files are frozen at their C02-reviewed hashes. `runtime.t.cpp` changes
  by exactly 45 added lines, which are the two new tests and nothing else
  (`c03-test-vs-c02.diff`, built against `runtime.t.cpp` `162997debb60` reconstructed from
  the C02 review patch). `profile.t.cpp` is unchanged at `d94976e9c9ea`.
- The prober ran in a scratch copy outside the checkout and kept its raw logs. Those logs
  show standalone kills, each with an `N failed` count, the named killing test and the
  failing assertion, for `open-bound-refusal-protocol` (`febc237bd88c`),
  `source-error-mapping` (`664b8217a4bf`) and `semantic-routed-as-protocol`
  (`b7a8d1bc017a`).
- The remaining survivor, `fail-overwrites-shape-failure` (`939c80e88815`), is explained
  correctly. I checked the explanation against `runtime.cpp` myself.
- The full probe set the C02 review asked for now has raw logs on the final candidate.
- My independent gate passes 987/987 with no skips. The inventory keeps all 985 C02 names
  and adds exactly the two C03 names.

The round still cannot be approved, because its records do not yet describe the candidate
accurately. That was the core of X06B3BC02-R1, and this round's objective was to close it:

- **X06B3BC03-R1.** The authoritative "Final candidate record" in the C03 record states
  that the C02 and B3-B rewrites and the C02 evidence README are "not verified" and "still
  owed". It also leaves those three edited files out of its changed-file list. On disk the
  work exists, and most of it is correct (see the closure table below). As written, though,
  the record that would be committed says its own acceptance criteria R1, R2, R3, N-c and
  N-d are unmet. The owned-file list is also incomplete, which breaks standing rule 4.
- **X06B3BC03-R2.** The rewritten C02 and B3-B sections still carry statements that C03
  has made false or stale. The C02 "Masking reason" design fact and its red-first paragraph
  say that `open-bound-refusal-protocol` and `source-error-mapping` survive on their own
  because of receipt masking. C03 proved that wrong: `open_bound` also parses the
  declaration. Both records also label the 985-test, 39-`X06 B3`,
  `runtime.t.cpp` `162997debb60` state as the "final candidate". The candidate being
  committed has 987 tests, 41 `X06 B3` tests and `runtime.t.cpp` `3b22d1378165`.

I reviewed in the shared checkout on `dev/planar-v2-vernacular` at HEAD
`a2aa1cc0ed4ad8c5b9e879a04231591839e7dc48`, against the uncommitted candidate. I wrote only
this file and new files under `tests/native/v5-02-x06-b3-b-c03-review-evidence/`. I applied
no mutants and edited no source, test or build file. My gate commands ran one at a time,
with no other ninja or ctest process running. I did not commit, install, publish or merge,
and I used no old Planar tool, queue, skill, role or installed binary.

## Exact candidate

- `git diff HEAD -- src` sha256 at review start and end:
  `96e44f54086469fe3ee0ccfa431ff4e3fadd12a15278834e5d6714613990531b`
  (`gate/00-diff-src-start.sha256`, `gate/07-diff-src-end.sha256`). This equals the C03
  record, the gate runner's run 3 and the prober's `git-diff-src.before/after.sha256`.
- Per-file sha256 (first 12), equal at start and end (`gate/00-files-start.sha256`,
  `gate/07-files-end.sha256`):

  | File | sha256 (12) | Status |
  | --- | --- | --- |
  | `runtime.cpp` | `ab70f5441bea` | frozen at C02 |
  | `runtime.cppm` | `331bd1b629df` | frozen at C02 |
  | `profile.cpp` | `91ac81f3d466` | frozen at C02 |
  | `profile.cppm` | `7bb805634449` | frozen at C02 |
  | `runtime.t.cpp` | `3b22d1378165` | changed in C03 |
  | `profile.t.cpp` | `d94976e9c9ea` | unchanged |

- Because the four product files hash-equal their C02-reviewed values, no diagnostic, public
  field or public method can have changed. The new test asserts the existing loader
  diagnostic `lua_syntax` / `Invalid Lua text chunk`, defined in
  `src/engine/lua/loader.cpp`.
- `git status --short` shows the same six source files, the coordinator's roadmap edit and
  the untracked round records. No other path under `src` differs from HEAD.

## Required change: X06B3BC03-R1 — the C03 final record contradicts the work it closes

The "Final candidate record (authoritative; supersedes ...)" section of
`V5-02-X06-B3-B-C03.md` has two problems:

- Under "Changed files in C03" it lists only `runtime.t.cpp`, the C03 record and the C03
  evidence folder. It then says: "Dispatch 1 record rewrites of `V5-02-X06-B3-B-C02.md`,
  `V5-02-X06-B3-B.md` and the C02 evidence `README.md` were not verified by this sync step;
  they are still owed per Dispatch 1 R1, R2, R3 (breach), N-c and N-d."
- All three files were in fact edited during C03. Both records and the README carry
  07:56–07:57 timestamps, after Dispatch 1, and their text says "rewritten in C03" or
  "Written in C03". I verified their content:
  - The N-c `owner_charge` amendment is recorded as a coordinator decision in both records.
  - The C01/C02 in-place probing breach is recorded in both records.
  - The opaque-owner known limit is recorded in both records. Lines 1528–1539 are still
    correct, because both C03 tests were inserted after line 1690.
  - `{false,false}` is attributed to C02.
  - The README classifies three generations of evidence and points to C03 as
    authoritative.

So the authoritative section understates the round, and its owned-file list is incomplete.
If this is committed, the history will say that C03's records criteria were never met.

**Fix:** in the final C03 section, list every file edited in C03 (`runtime.t.cpp`, this
record, `V5-02-X06-B3-B-C02.md`, `V5-02-X06-B3-B.md`,
`tests/native/v5-02-x06-b3-b-c02-evidence/README.md` and the C03 evidence folder). Replace
the "still owed" sentence with the actual status of R1, R2, R3 (breach), N-c and N-d, after
R2 below is fixed. Add an acceptance criteria table that states met or not met for each
Dispatch 1 and Dispatch 2 criterion, as the first-dispatch section did.

## Required change: X06B3BC03-R2 — rewritten C02 and B3-B sections are stale against the C03 candidate

These sections were rewritten before Repair 2. Repair 2 then changed two facts they state:

- **C02 "Design facts — Masking reason"** says: "`open-bound-refusal-protocol` and
  `source-error-mapping` alone survive for the same reason; combined with removal of the
  pre-capture verify they are killed." The C02 "Red-first and killing mutants" paragraph
  repeats "alone survive (masked)". C03 Repair 2 states that this reasoning was wrong, and
  the C03 raw logs show both mutants killed standalone (1 failed of 41, see below). The
  survival on the C02 candidate is true history. The reason is not. The text should say the
  two mutants were survivors because no test reached `open_bound`'s declaration parse, and
  that C03 kills them with `X06 B3 B a declaration the bound source cannot open refuses with
  its native diagnostic`.
- **"Final candidate" labels.** The C02 "Coder results (final candidate ...)" section and
  the B3-B "C01 verification results (final candidate ...)" and "Verification commands to
  run" sections give 985 tests, 39 `X06 B3` tests, `runtime.t.cpp` `162997debb60` and
  `git diff HEAD -- src` `8b2f73ea...` as the final candidate. Item 2 of the B3-B list says
  "final candidate 985 of 985". The candidate being committed has 987 tests, 41 `X06 B3`
  tests, `runtime.t.cpp` `3b22d1378165` and `git diff HEAD -- src` `96e44f54...`. Both
  sections do point to C03 as authoritative, and that helps. They still present superseded
  numbers as "final", which is the exact pattern X06B3BC02-R1 asked to remove.

**Fix:** relabel these sections as "C02 candidate" (or similar), with the C02 numbers kept
as history. Add one line in each that gives the final C03 figures (987 / 41 /
`3b22d1378165`, the two C03 test names) or points to the C03 final section. Correct the
masking statement as described above.

## Closure of the C02 findings

| Finding | Status | Evidence |
| --- | --- | --- |
| X06B3BC02-R1: records describe a different candidate | **Partly closed** | The C02 Coder results, the B3-B C01 results and the verification list now describe the post-repair C02 state: changed `runtime.cppm`/`runtime.cpp`, 985 names, 4 new, `n1-both` killed, `{false,false}` attributed to C02. They are stale again against the C03 additions (R2 above), and the C03 record misreports them (R1 above). |
| X06B3BC02-R2: `owner_charge` without a decision | **Closed** | The coordinator decision is recorded in the C03 Dispatch, the C02 record ("Coordinator decision: N-c amendment") and the B3-B record. It names exactly one field, keeps "B3-C must not grow" (in C02), and withdraws the N-b infeasibility claim. |
| X06B3BC02-R3: in-place probing, no final-candidate raw logs | **Closed** | The breach is stated in both records and in the C03 Dispatch. `probes/run-all.sh` copies the tree to a scratchpad path outside the repository, excluding `build/` and evidence folders. It checks that the scratch hashes equal the checkout hashes before building, and writes per-mutant raw logs into `probes/raw/`. Every probe shows `restored equal original ... True`, and `hash-equal.txt` is `0`. Earlier generations are kept in `superseded-1..3`, and nothing was deleted. |
| X06B3BC02-N-c: opaque-owner watch cannot fail | **Recorded** as a known limit, as Dispatch 1 directed | C02 and B3-B records. The lines are unchanged at 1528–1539. |
| X06B3BC02-N-d: mixed evidence generations | **Closed** | `tests/native/v5-02-x06-b3-b-c02-evidence/README.md` classifies the pre-repair, post-repair and review generations and names C03 as authoritative. |

## C03 survivor work

| Mutant | Result in raw log | Killing test and failing assertion |
| --- | --- | --- |
| `open-bound-refusal-protocol` (`febc237bd88c`) | build 0, ctest rc 8, `1 tests failed out of 41` | `X06 B3 B a declaration the bound source cannot open refuses with its native diagnostic`, lines 1729–1730: `"internal" == "lua_syntax"`, `"Invalid external profile protocol" == "Invalid Lua text chunk"` |
| `source-error-mapping` (`664b8217a4bf`) | build 0, rc 8, `1 tests failed out of 41` | same test, same two lines; the `capture` code no longer maps to the native diagnostic |
| `semantic-routed-as-protocol` (`b7a8d1bc017a`) | build 0, rc 8, `1 tests failed out of 41` | `X06 B3 B a route phase refusal is ordinary semantic data before any job is captured`, line 1708: `REQUIRE( r.error().semantic )` expands to `0` |
| `fail-overwrites-shape-failure` (`939c80e88815`) | build 0, rc 0, `100% tests passed out of 41` | survivor; unreachability accepted (below) |

Each log records the old and new text, the mutant sha256, the build exit code and the
restored-equal check. The baseline and final control runs both show
`100% tests passed out of 41`. So the scratch build ran all 41 `X06 B3` tests, including
the two `profile.t.cpp` tests that the C02 review's scratch build had missed.

**`fail-overwrites-shape-failure` unreachability.** I checked the blocking predicate against
`runtime.cpp`. `profile_invocation::fail` is called from three places:

- `begin_frame`. Its `c.shape_failure` branch needs a stored shape failure before the frame
  begins. `register_vm`/`command_vm::create` cannot return a VM in that state, and the
  factories never execute phases.
- `prepare_load`'s `refuse`. During `prepare_load`, the only writers of
  `command.shape_failure` are `validate_profile_phases` and its callees
  `invoke_shape_impl`/`resolve_shape_definition_impl`. Each of them returns
  `*shape_failure` itself, so the argument passed to `refuse` always equals the stored
  value.
- The two staged `verify()` failures. These run only after validation succeeded, so no
  shape failure is stored.

The route phase is a direct `call_retained_value` with no native shape bridge, and the
owner's VM is private, so a test cannot invoke shapes on it. The one remaining difference
under the mutant is that the VM's optional is moved out of rather than copied. That is
unobservable, because the consumed owner is destroyed. I agree that this is equivalent at
the public surface. The existing test `profile frame branches stay reachable and first
terminal native failures are preserved` covers first-failure preservation where it is
observable.

**Test quality.** The route-phase test includes a positive control for its zero-read watch
in the same test case. The open-bound test asserts the exact code, message and field, no
semantic data, no cleanup, a dead owner and a protocol refusal on reuse. Neither test adds
a public field or method, and no existing test name changed.

## Full probe set on the final candidate (criterion R3)

`probes/PROBES.md` (run 4) lists 48 rows (44 mutants plus four baseline and final control runs). The raw logs match the table for every row I
sampled: the three C03 kills above, `n1-both`, `fail-overwrites-shape-failure`, and the
baseline and final controls. Coverage includes the items the C02 review listed as missing:

- reset a/e/f for both counters;
- release success, failure and both;
- `everyjob`, `consumed`, `n3`;
- the one-factory charge removals (`n1-known-only`, `n1-opaque-only`);
- `samesel`;
- the `profile.cpp` note-registration mutants;
- reviewer mutants `d9b54d0579c3` (`reset-f-instr`, killed by the route-consumption test),
  `52b29026261a` (`reverify-moved`, file hash `cda20705630b`, survives as the documented
  equivalent) and `1adbea375f61` (`n1-both`, killed only by the owner-charge test, 1 of 41).

The red-first table maps all 24 tests added across B3-B, C01, C02 and C03 to at least one
killing mutant. For the two C03 tests, the pre-fix product is the frozen product. A
behavioural red against that product therefore means a mutant of it, and each mutant is
identified by its hash. I accept this, as the C02 review did for earlier tests. It replaces
the "claimed, not shown" statement in the gate runner's `REPORT.md`, and the C03 record
labels that statement as stale without editing the raw report. Accepted.

## Independent gate (macOS Debug, `build/rewrite-debug`)

| # | Command | Result | Log |
| --- | --- | --- | --- |
| 1 | `cmake --build build/rewrite-debug --target planar_tests` | exit 0, `ninja: no work to do.` | `gate/01-build-planar_tests.log` |
| 2 | `ctest --test-dir build/rewrite-debug --no-tests=error -j 4 --timeout 600 --output-on-failure` | exit 0, `100% tests passed out of 987`. The only two "skipped" grep hits are the start and pass lines of a test whose name contains the word. No failed, not-run or disabled entries. | `gate/02-ctest.log` |
| 3 | `ctest -N`, sorted diff against `tests/native/v5-02-x06-b3-b-c02-evidence/inventory.txt` | `Total Tests: 987`, 987 unique names. The diff contains only two `>` lines (the C03 names), so all 985 C02 names are kept. 41 names match `X06 B3`. | `gate/03-ctest-N.log`, `gate/03-inventory.diff`, `inventory.txt` |
| 4 | `python3 scripts/rewrite-check.py lint --build-dir build/rewrite-debug` | exit 0, `Formatting and Doxygen checked 136 active source files` | `gate/04-lint.log` |
| 5 | `cmake --build build/rewrite-debug`, run twice | both exit 0; the second reports `ninja: no work to do.` | `gate/05-build-all.log`, `gate/06-build-nochange.log` |
| 6 | Product and test file hashes, and the `git diff HEAD -- src` hash | equal before and after | `gate/00-*`, `gate/07-*` |

Each exit code was written directly to its own `gate/NN.exit` file, never through a pipe.

## Advisory notes (non-blocking)

- **X06B3BC03-N-a.** Repair 2 says the declaration "is replaced with invalid Lua after
  `open_known_load`". The test writes it before `f.open()`. The test is still correct,
  because `open_known_load` only admits metadata and reads no declaration content, but the
  record should describe what the code does.
- **X06B3BC03-N-b.** The C02 evidence README begins "Written in C03 (records only)". C03
  stopped being records-only after Dispatch 2. Consider rewording.
- **X06B3BC03-N-c.** `REPORT.md` run 4 says exit codes were read from files, but
  `raw/run4/` has `.exit` files only for the test build, ctest and `ctest -N`. Lint and the
  two full builds rely on their log text. My gate covers those commands with exit files.
  The run 3 section also says the known-limit line numbers "have shifted", but they have
  not.
- **X06B3BC03-N-d.** The C03 evidence `README.txt` still says "PREPARED, NOT RUN". It is
  raw history and should not be deleted, but the final C03 section or a new README should
  say it is superseded.

## Limits

- macOS Debug only. No RelWithDebInfo or Linux run.
- I did not re-run mutants myself. The prober's scratch build directory no longer exists,
  and a cold scratch build was not justified for a records-only disposition. My kill and
  survivor judgements rest on the prober's raw logs, which include the mutant text, the
  hashes, the `N failed` counts and the failing assertion expansions, and on my own reading
  of `runtime.cpp` for the survivor.
- Steps 1 and 5 were incremental no-work builds over a tree that was already built. They
  show the tree is current and that a no-change build does no work, not that a fresh
  rebuild succeeds.
