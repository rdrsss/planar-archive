# V5-02-X07-C02 independent review (lost-CAS race in the two-process load)

Disposition: **approve**. The five findings below are minor and affect only the records. None blocks the
candidate, and all can be fixed in the commit step by editing the round record, the plan files or the evidence
README. No source, test or evidence data needs to change.

I reviewed the uncommitted candidate against HEAD `f78f8eb4`. The fix does what the round set out to do. A loser
whose ledger was committed by a winner after admission now passes `prepare_load`, reaches the CAS and refuses
`revision_conflict` with exit 3. It no longer fails verification with the non-table message that
`registration_failed` turns into exit 4 `internal`. Real input changes still refuse `capture_changed`, every other
receipt stays strict, and no diagnostic byte changed. The round's red-first evidence did not follow its own D15
recipe, so I ran that recipe on a pristine HEAD copy. It gives behavioural reds for I6, I7 and I8, and the I9b and
I12b guards pass on HEAD. I also ran the I13 repeat, which the round had not run, and four probe spot-checks. All of
them agree with the record.

## Exact candidate

identity.py, run from the repository root on `src tests/acceptance` (`tests/native/v5-02-x07-c02-evidence/review/identity-candidate.txt`):

| identity | value |
|---|---|
| HEAD | `f78f8eb47267ab9c72efacc07708e7c0c2669840` |
| sha256(git diff --binary HEAD -- src tests/acceptance), tracked only | `b4a0bf1878d52940a43d27b5b37216cf71aedd51d33324055d8a6ac3d38d590e` |
| combined sha256 (diff bytes, then each untracked path NUL bytes, sorted), pathspec src tests/acceptance, evidence excluded | `b4a0bf1878d52940a43d27b5b37216cf71aedd51d33324055d8a6ac3d38d590e` |

| file | state | sha256 |
|---|---|---|
| `src/cmd/planar/CMakeLists.txt` | modified | `3b1406b28211503de9fec888d84b75fb4b125eea79c91eba9116d024ebae200a` |
| `src/engine/capture/admission.cpp` | modified | `98bf208515e3d5bcd04c838325e1fd71233143f6e25f02da9d0d3cd829e6e887` |
| `src/engine/capture/admission.cppm` | modified | `1a771957a5347f2410aea5a043ef98463acd70f1dfe14a6eadf275beb8588aa9` |
| `src/engine/capture/admission.t.cpp` | modified | `54908ae5ffefd7e9c0a53e7eee5315d2319392ad36d648f2287124809999b215` |
| `src/engine/ledger/profile_acceptance.t.cpp` | modified | `0d2778efddc3424e7dc12d06b92234e2a8ba461f1b02ec0a9ff2f466e3a438db` |
| `src/engine/runtime/external_registration.t.cpp` | modified | `e1f094d19785937767faa0b2a20de917da130a27f87bc2bc71e5f822519e5523` |
| `src/engine/runtime/runtime.cpp` | modified | `d329c26b6450a9f4a13cd696e6157a0f1e84c0cc60f58cfe5d13567038833ad9` |
| `src/engine/runtime/runtime.t.cpp` | modified | `e3897da21bb6b914807c0f6e7660789304264c4accc2f4131de871631b122c50` |
| `tests/acceptance/x07_replay.py` | modified | `badfdbb60eb779975f15e30e29e88d82ce9011f8784f547c1551be1e761646d2` |

These values equal the round record's "Final candidate identity" table, the gate (`raw/gate2/00-identity.log`, which
uses the pathspec `src tests/native`: I reran that form too and got `5d4a1139...`, as recorded) and the prober
(`probes/raw/00-identity.log`). The Linux host manifest (`raw/gate2/linux/host-manifest.txt`) has the same hashes for
`admission.cpp`, `admission.cppm`, `runtime.cpp`, `external_registration.t.cpp` and `x07_replay.py`, and its context
manifest is equal. So the gate, the probes and the container ran on exactly these bytes. Outside `src` and `tests`, the
only tracked changes are the status lines of `V5-02-X07.md` and `V5-02-X07-C01.md` (see R5).

## What I checked and found sound

- **The fix, and its scope.** `admission.cpp` adds `identity_only` to the receipt and adds one helper,
  `same_receipt`, which compares a directory or an identity-only file with `same_directory` (device, inode and mode)
  and every other file with `same_file`. `check()` uses it for both the descriptor stat and the by-name `fstatat`
  stat, so a replaced, removed or chmodded load ledger still refuses. Only the leaf can be identity-only
  (`last && !is_directory && role == load_ledger`). `retained_file_receipt::verify`, `verify_file`, `verify_root` and
  discovery keep strict comparisons (D11). `open_known_load` retains only `workspace`, `declaration` and `registry`
  (`runtime.cpp:5299-5310`), so no strict descriptor check is ever made on the load ledger. `grep` finds
  `load_ledger` at one product call site only, `runtime.cpp:5297`, which is the plan's `:5294` after the three added
  comment lines. The built-in `admit_ledger` (`:5163`) and the extension host (`:6066-6068`) still use role `ledger`.
- **Gap 1: the role is set before `check()`.** `open()` now takes the role as a parameter, builds
  `selection result{..., role, ...}` from it, and computes the leaf's identity flag from it before `retain()`.
  `admit_file` no longer assigns `value.role` after `open()` returns. So both the role and the flag exist before
  `append()` runs `relationships()` and `check()`. I16 is a sound explained row: the remaining window between `fstatat`
  and `fstat` inside `open()` cannot be reached without a hook added only for observability.
- **Gap 3: the two strict sites are pinned.** I reran the I12b mutant (`:5163` admitted as `load_ledger`) in my own
  scratch copy. Only `X07 C02 history family ledger receipt stays strict at the built-in admission site` fails
  (`review/probe-R1-I12b.log`). I12a's carried log shows that only the new `C4 B2 appended admitted ledger refuses
  verify before any ledger read` fails.
- **Real input changes still refuse `capture_changed`.** `x07.receipt_races_refuse_before_authorization` and
  `x07.final_race_rolls_back_before_commit` pass on the candidate and on HEAD (`review/c2.log`, `review/cand-focused.log`).
  The per-receipt unit test (`content changes still refuse for every other supplied file role`) grows a strict
  registry, observation, legacy `ledger` or declaration file in the same admission as a `load_ledger` receipt. Verify
  still refuses, whether or not the load ledger also grew. I4, E3 and E4 kill it.
- **Gap 5: the post-preparation interleaving (D7 case 2) is unchanged from HEAD.** The seam still runs the CAS
  (`acceptance_seam.cpp:50-55`) before `candidate.verify()`, and `acceptance_seam.cpp` has no diff. On my pristine-HEAD
  build, `X07 C02 post-preparation input change plus winner commit still refuses revision_conflict` passes: "All tests
  passed (34 assertions in 1 test case)" (`review/p3.log`). That is the HEAD trace the gap asked for. The I9b mutant,
  which moves verify before the CAS, kills it (carried log).
- **Diagnostics and the closed table.** The product diff adds no string literal. It changes comments, the enumerator
  documentation and logic only. `response.cpp`, `invocation.cpp` and `bootstrap.t.cpp` have no diff. I15 and I15b are
  killed (carried logs).
- **Fencing (gap 7).** The D16 directory fences exit 0 for `src/engine/ledger` (except `profile_acceptance.t.cpp`),
  `src/engine/capture` (except the three admission files), `src/cmd/planar` (except `CMakeLists.txt`) and
  `tests/native/v5-02-x07-support`. `src/engine/runtime` also exits 0 once `external_registration.t.cpp` is added to
  its exclusions (see R2). `acceptance_seam.cpp`, `storage.*`, `sqlite.*`, `format.*`, `native.*`, `profile.*` and
  `runtime.cppm` are untouched, so the ledger lock and the busy wait are unchanged. So are the accepted X07 limits.
- **Gap 2 and gap 6.** D1 and the derived rows D2-D11 and D14-D20 are labelled NWC. They cite ledger-contract load
  steps 3-5, including step 5's "selected file/root identities and normalized file modes remain checked". D7 cases 2
  to 4 are labelled CD because they describe ordering that is unchanged from HEAD, which is fair. D13 and I20 record
  the generator-anchor alternative, its cost (a new anchor in `invocation.cpp`, a change to `generate.py` and an
  anchor inside phase 6 of `runtime.cpp`) and its rejection on operator option A. They no longer cite XD3 as a
  prohibition.
- **Tests: none removed, none renamed, and every new one can fail.** The test diff has no removed line. Sorted
  `ctest -N` from my own runs of both macOS builds is identical (1273 names). Against the committed 1260-name list
  (`v5-02-x08-c01-r7-evidence/inventory.txt` at HEAD), 0 names are missing and 13 are new, exactly `new-names.txt`.
  The Linux sorted inventory is byte-identical to macOS once the ordinals are removed. Every new test has a killing
  mutant in PROBES.md. I confirmed I2, I6/I7/I8, I9 and I12b myself.
- **Gates.** Every `.exit` file under `raw/gate2/` and `raw/gate2/linux/container/` is 0. All four ctest logs say
  "100% tests passed out of 1273". In each log, the only "skipped" match is the name of the test `command load refuses
  forged family order skipped duplicate and malformed arguments`. All four macOS no-op logs and both Linux no-work
  logs say `ninja: no work to do.` Lint checked 147 files. Redaction on the explicit file list exits 0 with empty
  output, and the builder prune exits 0. Before running anything in the shared build directories, I confirmed with
  `ninja -n planar_tests` that both are current for these bytes. The only planned step was the glob re-check.
- **Ownership.** Every edited file is listed in the record's "Files edited in this round". `runtime.t.cpp` and
  `external_registration.t.cpp` are recorded as amendments 1 and 2. No public field or method was added; `healthy()`
  already existed at HEAD (`admission.cppm:103`).

## Findings

### X07-C02-R1 (minor, records): the red run did not follow D15, and RED.md does not record the commands

D15 and the INVARIANTS.md "red-evidence rule" require P1, P2 and the CLI test to be red on a pristine HEAD archive
with exactly three files overlaid, each command recorded exactly, and identity.py output for the overlay. The round
instead ran in the shared `build/rewrite-debug` against the T1 stub snapshot. RED.md gives the hashes of the snapshot
diff and its `SHA256SUMS`, but no identity.py table and no command lines. The stub leaves `runtime.cpp` and every
load-path comparison at HEAD behaviour (the snapshot diff touches only the enumerator, the `admit_file` whitelist,
tests and registration). So the reds are behavioural and equivalent, but they are not the recorded recipe.

I ran D15 as written. I extracted `git archive f78f8eb4` into a scratch directory outside the checkout, overlaid
`profile_acceptance.t.cpp`, `x07_replay.py` and `CMakeLists.txt`, configured from cold with `cmake --preset debug`,
and built `planar_ledger_acceptance_tests planar planar_x07_variant_3 planar_x07_variant_4` (exit 0,
`review/head-overlay-build.log`). The overlay's identity.py output is in `review/identity-head-overlay.txt`, with
tracked hash `b96f914d40abac35be5b9d0e521c2507524f53941dd223466b0b41ed24e6a80c`. Results:

| command (binary in the scratch `build/rewrite-debug`) | exit | outcome |
|---|---|---|
| `planar_ledger_acceptance_tests "X07 C02 loser prepared after a winner commit refuses revision_conflict and not capture_changed"` | 42 | red: `prepare_load refused` with `capture_changed` / `Captured external profile changed` (`review/p1.log`) |
| `planar_ledger_acceptance_tests "X07 C02 prepared profile native receipts ignore a committed ledger change"` | 42 | red: `CHECK(verified)` with `Captured inputs changed during verification`, the non-table message (`review/p2.log`) |
| `planar_ledger_acceptance_tests "X07 C02 post-preparation input change plus winner commit still refuses revision_conflict"` | 0 | green guard on HEAD, the gap-5 trace (`review/p3.log`) |
| `planar_ledger_acceptance_tests "X07 C02 history family ledger receipt stays strict at the built-in admission site"` | 0 | green guard on HEAD (`review/p4.log`) |
| `ctest --test-dir <scratch>/build/rewrite-debug -R '^x07.ledger_commit_after_admission_reports_revision_conflict$' --no-tests=error --output-on-failure` | 8 | red: `AssertionError: (3, b'', ...capture_changed...)` at `x07_replay.py:134` (`review/c1.log`) |
| same, `-R '^x07.(receipt_races_refuse_before_authorization\|final_race_rolls_back_before_commit\|two_process_cas_one_winner)$'` | 0 | 3 of 3 pass on HEAD (`review/c2.log`) |

Each red is an assertion failure, not a compile failure. Fix: in RED.md and the record's red-first summary, point to
these logs as the D15 execution, and note that the stub-snapshot run was a deviation.

### X07-C02-R2 (minor, records): the I18 and D16 fence check text is stale

I18 check (1) says the changed set under `src` and `tests` must equal eight named files. Check (2) excludes only
`runtime.cpp` and `runtime.t.cpp` under `src/engine/runtime`. The final candidate also changes
`src/engine/runtime/external_registration.t.cpp` (amendment 2). So both checks, as written, fail: the set has nine
files, and `git diff --quiet HEAD -- src/engine/runtime ':(exclude)...runtime.cpp' ':(exclude)...runtime.t.cpp'` exits
1. Gate command 11 ran a narrower file-list fence instead. The record marks I18 MET with "file set check below", but
no such check follows. With `external_registration.t.cpp` added to the exclusions, every directory fence exits 0, and
the changed set is exactly the record's list of nine files. So the invariant holds. Fix: amend the I18 row (and D16
or a note) to include `external_registration.t.cpp`, and record the exact commands with exit 0.

### X07-C02-R3 (minor, records): the plan files are revision 4 and do not match the final candidate

`plan/INVARIANTS.md` and `plan/DECISIONS.md` still say 11 new names and 1271 tests (I14, I21d, D17). They have no rows
for E1 and E7. The I9 row's fallback names the existing unit test `capture admission ancestor replacement preserving
selected inode refuses` as the kill that is kept, but `probes/raw/I9.log` shows that test passes under the I9 mutant.
I confirmed this in `review/probe-R2-I9.log`. The kill comes from four other unit tests: `exited traversal remains
verified without false final nesting`, `root binding checks original ancestors leaf names and modes`, `root binding
retains exited physical parent branches` and `load ledger receipt refuses a replaced or removed file`. The CLI
subcases are masked by an unidentified layer. I reproduced that too: planar and the variants were rebuilt with the
mutant, and both x07 race cases still pass. The round record's table and limits state all of this correctly. Fix: add
a one-line note to the top of each plan file that the round record's "Complete invariant table" and "Amendments"
govern, or sync the rows.

### X07-C02-R4 (minor, records): the I13 repeat evidence was missing; it is now in the review folder

The I13 row requires 20 consecutive passes with `--repeat until-fail:20` in macOS Debug and RelWithDebInfo. The round
did not run this, and the record lists that as a limit. I ran it on the shared builds after confirming they are
current:

- `ctest --test-dir build/rewrite-debug -R '^x07.two_process_cas_one_winner$' --repeat until-fail:20 --no-tests=error --output-on-failure`
  passed 20 of 20, exit 0 (`review/i13-rewrite-debug.log`).
- The same command on `build/rewrite-release` passed 20 of 20, exit 0 (`review/i13-rewrite-release.log`).
- With 12 busy loops (`perl` spin, `alarm 600`), `build/rewrite-release` with `until-fail:30` passed 30 of 30, exit 0
  (`review/i13-release-busy12.log`).

For balance: in my scratch Debug copy, `runtime.cpp` restored to HEAD (so the load ledger is admitted as `ledger`
again) also passed 40 of 40 under 12 busy loops (`review/i13-headruntime-busy12.log`). The flake does not reproduce
reliably on this machine, so the repeat runs show that no new flake appeared, not that the old one is gone. The
evidence that the race is closed is the deterministic chain: I7 shows the HEAD message, I15b shows the redaction to
exit 4, and I8 shows HEAD `capture_changed` against fixed `revision_conflict`. Fix: cite these logs in the I13 row
and in acceptance criterion 2. The supplementary 12-busy-loop reproduction of exit 4 on HEAD is still not evidenced.
It was non-gating.

### X07-C02-R5 (minor, records): status-line edits to two committed round records were outside the owned list

The candidate edits the status lines of `docs/vernacular/rounds/V5-02-X07.md` and `V5-02-X07-C01.md`. Neither file is
in the Dispatch revision 2 owned list, and the "Amendments to earlier constraints" section does not record them,
although "Files edited in this round" does. The new sentences are accurate. However, the edited C01 line still says
"awaiting Opus review. Nothing is committed.", which has been false since `f0cbc374`. Fix: add an amendment row for
both files. Optionally, correct the stale C01 clause in the same edit, or revert both edits and keep the cross-reference
only in this round's record.

## Invariant table, red and probes

I checked every row of the record's "Complete invariant table" against `plan/INVARIANTS.md`, `red/RED.md` and
`probes/PROBES.md`. Each acceptance criterion has a row: I6, I7, I8, I15b and I20 for criterion 1; I13 for 2; I2 to
I5, I9, I9b, I12a, I12b and I15 for 3; the table itself for 4; I14 for 5; and I21a to I22 for 6. Each of the nine
plan-review gaps maps to a visible row (DECISIONS.md "Closure of the nine plan-review gaps"). Each mutant breaks the
invariant it is attached to, and the named test is among the failures in the cited log. The exceptions are I9 (R3)
and the explained or process rows I10, I13, I14, I16 and I18 to I22, which have no mutant by design. I10 is a sound
explanation: the per-receipt flag is killed at unit level by I4, and E6 kills the registry variant in process.

Spot-checks in my own scratch copy (the candidate overlaid on the HEAD archive, Debug, 52 focused tests green before
probing in `review/cand-focused.log`). The driver is `review/probe.py`: an exact single-occurrence replace, a build,
`ctest` under `perl -e 'alarm 900'`, then a byte restore with a hash check.

| probe | mutant | failing tests | matches PROBES.md |
|---|---|---|---|
| R1 | `runtime.cpp:5163` admits as `load_ledger` | only `X07 C02 history family ledger receipt stays strict at the built-in admission site` | yes (I12b) |
| R2 | delete the by-name `fstatat` block in `check()`; planar and variants rebuilt | the four unit tests in R3; both x07 race cases pass | yes (I9, masked CLI) |
| R3 | skip the by-name recheck for `identity_only` receipts | only `capture admission load ledger receipt refuses a replaced or removed file` | yes (I2) |
| R4 | `runtime.cpp:5297` back to role `ledger` | `X07 C02 loser prepared...`, `X07 C02 prepared profile native receipts...`, `x07.ledger_commit_after_admission_reports_revision_conflict` | yes (I6, I7, I8) |

The E5 and E5c survivors are sound equivalents. The `S_ISLNK` clause, the `!S_ISREG` clause and `O_NOFOLLOW` each
refuse a symlink final component on their own, and E5d (all three removed) is killed by the alias and link test.

## Commands I ran (from the repository root unless stated, unpiped; logs under `tests/native/v5-02-x07-c02-evidence/review/`)

- `python3 <workflow>/identity.py src tests/acceptance`, `... src tests/native`, and `... src/engine/ledger/profile_acceptance.t.cpp tests/acceptance/x07_replay.py src/cmd/planar/CMakeLists.txt`
- `git diff HEAD -- src tests/acceptance docs`. The four D16 `git diff --quiet` fences, plus the runtime fence with
  `external_registration.t.cpp` excluded, and `tests/native/v5-02-x07-support`.
- `ninja -C build/rewrite-debug -n planar_tests` and the same for `build/rewrite-release`: only the glob re-check was
  planned.
- `ctest -N` for both macOS builds, sorted and compared with the HEAD 1260-name list and the Linux inventories.
- The D15 recipe in a scratch copy outside the checkout (R1 table above), then the candidate overlaid in the same copy
  and the four probes (R1 to R4 above).
- The I13 repeat runs and the busy-loop runs (R4).

## Limits

- I did not rerun the full macOS or Linux ctest. I relied on the gate's raw logs and exit files, which I read and
  checked against the candidate identity and the Linux manifest.
- My probes ran in Debug on macOS only. I spot-checked four of the 14 invariant mutants and did not rerun E1 or E7. Their
  logs show the new tests as the only failures.
- The masking layer that keeps the x07 race cases refusing under the I9 mutant is still unidentified. That behaviour is
  pre-existing and was not changed by this round.
- D7 case 2 is pinned with a closure-file change, not the registry change named in gap 5. The CAS-before-verify order
  in the seam does not depend on which input changed, and the I9b mutant kills the test, so I accepted it.
- Commit, the roadmap update and the review-record links are for the coordinator. I edited only this file and the new
  files under `tests/native/v5-02-x07-c02-evidence/review/`.
