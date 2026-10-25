# V5-02-X06-B3-B-C02 independent review (second corrective round for bound capture and all-job validation)

Disposition: **request-changes**.

The test work in C02 closes the two behavioural findings from the C01 review.

- The new route-consumption fixture kills the reviewer mutant `d9b54d0579c3`, which
  survived C01. I confirmed the kill in my own scratch probe.
- All three paired watches now watch the declaration alone, and a positive control shows
  that declaration reads are counted.
- The read-count test has been renamed to the count it actually checks. The rename is
  recorded, and so is the reason the native receipt check masks the pre-capture verify.
- No diagnostic literal changed, no `assert(` was added, 981 of the 982 C01 names are kept
  (the one rename is allowed), and my independent Debug gate passes 985/985.

The round still cannot be approved, for three reasons:

- **X06B3BC02-R1.** The round record no longer describes the candidate on disk. After
  probing, a repair changed `runtime.cppm` and `runtime.cpp` and added a fourth test. The
  main "Coder results" section still states the opposite on hashes, test counts, N-b, N-c
  and the `n1-both` survivor. The B3-B record repeats two of those claims.
- **X06B3BC02-R2.** The repair adds a public observation field,
  `profile_invocation_observation::owner_charge`. Advisory N-c in the brief rules that out,
  and no coordinator decision records an exception. The first package had recorded N-b as
  infeasible for exactly this reason.
- **X06B3BC02-R3.** The provenance problems that C01-R1 raised happened again. The repair
  applied its mutants in the shared checkout. The re-run of the probe set on the final
  candidate deleted its raw logs with its scratch directory. The raw logs that are kept
  belong to the pre-repair candidate.

I reviewed in the shared checkout on `dev/planar-v2-vernacular` at HEAD
`0cc29923383b39508e31cbf0cb62651185963480`, against the uncommitted candidate. I wrote only
this file and new files under `tests/native/v5-02-x06-b3-b-c02-review-evidence/`. All of my
mutation ran in my session-scratch copy `rvc01/wt`, which is outside the repository and has
its own `build/rewrite-debug`. Before probing, I copied the three current runtime files into
it (`scratch-start.sha256`). After the last probe, all four probed files compare equal to
the checkout (`probes/scratch-end-cmp.txt`). I never mutated the shared checkout or its
`build/rewrite-debug`. My gate there ran only after the probes had finished, one command at
a time, with no other ninja or ctest process running. I did not commit, install, publish or
merge. I used no old Planar tool, queue, skill, role or installed binary.

## Exact candidate

- `git diff HEAD` sha256 at review start and end:
  `2b214ec3eb1e140e38ec1511d77188fa64842b5d56e4de792083b0faa33825d9`
  (`gate/00-diff-start.sha256`, `gate/07-diff-end.sha256`). This equals the gate runner's
  `raw/gate-run/diff.sha256`. The patch is kept as `candidate.patch`.
- Per-file sha256 (first 12): `runtime.cpp` `ab70f5441bea`, `runtime.cppm` `331bd1b629df`,
  `runtime.t.cpp` `162997debb60`, `profile.cpp` `91ac81f3d466`, `profile.cppm`
  `7bb805634449`, `profile.t.cpp` `d94976e9c9ea`. The three `profile.*` files are unchanged
  since B3-B. Apart from these six files and the coordinator's pre-existing
  `docs/vernacular/roadmap.md` edit, nothing under `src`, `tests`, `cmake` or
  `CMakeLists.txt` differs from HEAD.
- Two candidate states exist within the round:
  - The state that the coder's probes, coder gate and "Coder results" section describe:
    `runtime.cpp` `1ff83396d768` and `runtime.cppm` `91f866dc8ff6`, both unchanged from C01,
    plus `runtime.t.cpp` `5fbcbade1f32`. The `git diff HEAD` hash was `848de5c1fbca…`.
  - The final state above, produced by the "Repair after mutation probes" section.

  I rebuilt the pre-repair state from the coder's `candidate.patch` and diffed it against the
  final files (`repair-src.diff`). The repair adds the `owner_charge` field and its
  doc comment to `runtime.cppm`. In `runtime.cpp` it adds the matching member, measures the
  delta around the existing charge in both factories, and copies it out in `observe()`. It
  adds no other product change.

## Required change: X06B3BC02-R1 — the round record describes a different candidate

The brief requires every agent to list every file it edits, and N-e asks for the
verification text to match the repaired candidate. The C02 record's main section
("Coder results (package P3, records)") was written before the repair and was not updated.
The repair's own section was appended at the end and contradicts it:

- "Owned files edited" says the implementation edited `runtime.t.cpp` only. It also says
  the other five files "keep their C01-review hashes … Confirmed again when this record was
  written" and "No product source changed". Both `runtime.cppm` and `runtime.cpp` changed
  (see above). The repair's edit of those two files is not listed as an owned-file edit.
- **N-b** reads "Not done, and cannot be done in this test unit … `n1-both` … survives all
  36 tests". **N-c** reads "C02 added no observation fields to public types". The repair
  then added a public field and a test that kills `n1-both`. I confirmed that kill below.
- "Test inventory" gives 984 names, three new, and 36 tests matching `X06 B3`. The
  candidate has 985 names, four new (including
  `X06 B3 B owner allocation charge is observable and covers the owner's own storage`), and
  39 tests matching `X06 B3`.
- The gate table reports `100% … out of 984` and diff hash `848de5c1fbca…`. Neither
  matches the candidate. Only the gate runner's `REPORT.md` reflects the final tree.
- The "Surviving mutants" paragraph still lists `n1-both`.
- In [V5-02-X06-B3-B.md](V5-02-X06-B3-B.md), the C01 verification section says that
  `n1-both` "cannot be detected from the test unit and is recorded". It also says that C02
  re-ran the probe set on the final candidate "with raw logs kept in
  `tests/native/v5-02-x06-b3-b-c02-evidence/`". The first claim is now false. The second
  is false for the final candidate (see R3). The B3-B "Verification commands to run" list
  (items 2, 3 and 6) still says "C01 candidate 981/981", "4 new" and "three required probes
  survive". N-e targeted this kind of stale text.
- The zero-custom-job `{false,false}` success-release case was added in C02. The diff from
  the C01 candidate to the final `runtime.t.cpp` shows it, together with the
  `if (f.custom_jobs > 0)` guard. The record says it "does not attribute the change to a
  specific package". It should say C02 added it.

**Fix:** rewrite the C02 "Coder results" section so it describes the final candidate:
hashes, the owned-file edits (including the `runtime.cppm`/`runtime.cpp` repair), 985
names with four new, N-b/N-c as finally decided under R2, the survivors and the gate.
Correct the two stale statements and the verification list in the B3-B record. Records
only; no product change is needed for R1.

## Required change: X06B3BC02-R2 — `owner_charge` contradicts N-c without a decision

N-c in the brief reads: "Add no further observation-only fields to public product types
beyond what R2 and R3 strictly need. B3-C must not grow them." N-b asked for a test that
detects the charge removed from both factories "if feasible". It gave a same-module
`sizeof(implementation)` comparison as the example, which adds no public field. The first
package recorded N-b as infeasible because a hook would break N-c. The repair resolved the
conflict the other way, without recording an exception.

- The field is public: `profile_invocation_observation` is exported from
  `planar.engine.runtime`. Every product run records it, which is the cost N-c exists to cap.
- The test works. In my scratch probe `n1-both` (mutant `1adbea375f61`, the reviewer's C01
  edit applied to the final file) fails only the new test, at lines 1623 and 1624. It is
  close to a tautology, though: the field is the `native_bytes` delta measured around the
  `charge` call it guards.
- It is not new *stage* surface. `profile_load_stage` is unchanged, so criterion 4's
  wording is met. Criterion 5 ("N-a through N-e handled as described") is not.

**Fix (coordinator decision required):** either remove `owner_charge`, its plumbing and the
new test, and restore the infeasibility record for N-b; or record an explicit,
coordinator-approved amendment to N-c that names this one field and keeps the "B3-C must
not grow them" rule. Both choices are acceptable to me. The current silent contradiction is
not. Removing the field and test would change the inventory back to 984 names, which is
allowed because the test is new in C02.

## Required change: X06B3BC02-R3 — in-place mutation again, and no raw logs for the final candidate

Criterion 3 requires raw logs under the C02 evidence folder and a re-run of the C01 probe
set on the final candidate. Criterion 5 and the hard rule require that probes never mutate
the shared checkout or `build/rewrite-debug`. The evidence shows:

- **In-place probing (criterion 5 not met).** The record's repair section says: "Mutants
  run in place on a backup (restored, sha256 12 `1a402d5995ff` before and after)". No such
  hash matches any current candidate file. The files were reformatted afterwards, so the
  in-place probes ran on yet another intermediate state. No command or raw log for them
  exists. C01-R1 named exactly this violation. I see no damage now: the checkout hashes
  are stable across my review, and only the six owned files differ from HEAD. The breach
  cannot be undone, but it must be stated as a breach and not presented as evidence.
- **Final-candidate raw logs deleted (criterion 3 not met).** The second-pass
  `probes/PROBES.md` re-ran the full set (39 tests, final hashes) in a scratch copy, which
  is good. It then says "Raw logs lived in the scratch directory and are removed with it".
  C01-R1 named this too. `probes/raw/`, `results.md` and `results.tsv` hold only the
  pre-repair run (36 tests, `runtime.cpp` `1ff83396d768`). The scratch directory no longer
  exists.
- **Red-first.** I accept killing mutants as red-first, as the brief allows ("against a
  mutant or the pre-fix snapshot"). I also accept the record's statement that the C01
  pre-fix snapshot does not compile against the C02 tests, so it cannot give a behavioural
  red. For the owner-charge test, however, the only raw logs are my own.

My own probes below cover the invariants this round had to close, with raw logs. They do
not cover the rest of the C01 set on the final candidate: reset a/e/f for both counters
(I ran only reset-f for instructions), release success/failure/both, every-job, consumed,
n3, the one-factory charge removals and `samesel`.

**Fix:** after R2 is settled, re-run the C01+C02 probe set once on the final candidate in a
scratch copy outside the repository and keep every raw log in
`tests/native/v5-02-x06-b3-b-c02-evidence/`. Supersede the pre-repair `results.*` rather
than presenting them as final. State the in-place repair probing in the record as a breach
of the hard rule.

## Closure of the C01 findings

| Finding | Status | Evidence |
| --- | --- | --- |
| X06B3BC01-R1 red-first, raw logs, scratch-only | **Open** (R1, R3 above) | Killing-mutant reds accepted. Pre-repair raw logs exist. Final-candidate logs were deleted. The repair probed in place. |
| X06B3BC01-R2 instruction reset at route/validation boundary | **Closed** | `reset-f-instr` (final-file mutant `3f77818edcd8`, the reviewer's `d9b54d0579c3` edit) fails "exhaustion depends on route consumption …" at lines 2015 and 2021 (1 failed of 37). The light variant passes on the candidate. `light-overcharge-route` fails it at 2005. |
| X06B3BC01-R3 watches, ordering claim, masking record | **Closed** | Watches fixed at 1529, 1651/1654 and 1859. The positive control fails under `control-watches-unread-file` (`1 failed`, line 1673). `decl-read-in-prepare` fails both zero-read tests (lines 1737, 1867). Rename recorded. Masking reason recorded. |

**R2 detail (route-consumption fixture).** The heavy variant (`N = 49937000`) refuses with
`resource_limit`, limit `instructions`, maximum 100000000, not semantic. The light variant
(`N = 49927000`) succeeds. The test derives the heavy validation cost from the light run's
own marks: below budget alone, above budget with the route added. So "each phase alone
under 100M" is asserted and not just stated in a comment. Under the reset mutant the heavy
variant succeeds, which is the behavioural kill. The margins are tight (about 4k
instructions above budget and 16k below it), so any calibration drift fails loudly rather
than silently. That is acceptable.

**R3 detail (ordering).** I tried to refute the "equivalent at the observable surface"
claim for `reverify-moved` with a scratch-only test, `ordering-test`. It replaces the ledger
or the declaration by rename, watches the profile module, and expects the two module reads
of the pre-capture verify before `open_bound` refuses. On the unmodified candidate it reads
**0** modules (`0 == 2` for both variants), and the refusal is `capture_changed`/`profile`.
So the pre-capture `capture.verify()` itself refuses those changes before reading any
content, and source-side refusal never follows a successful pre-capture verify in these
fixtures. That agrees with the repair section's argument. `reverify-moved` (final-file
mutant `cda20705630b`) passes all 37 runtime `X06 B3` tests. I accept the rename to
"prepare_load reverifies captured profile content exactly three times". `reverify-pre`
(`185ac66a000a`) still fails it at line 1984 (`4 == 6`).

## Acceptance criteria

| # | Criterion | Finding |
| --- | --- | --- |
| 1 | `d9b54d0579c3` killed; light variant succeeds; each phase alone under 100M | **Met** (reviewer probe, raw log) |
| 2 | Declaration-only watches, positive control, `52b29026261a` killed or rename recorded, masking recorded | **Met** |
| 3 | Red-first and killing mutant per C01/C02 test with raw logs; C01 set re-run on final candidate; equal restored hashes | **Not met** (R3: final-candidate logs deleted; owner-charge test has none of the coder's) |
| 4 | 982 names kept (allowed rename excepted), new names listed, diagnostics byte-identical, no `assert()`, typed errors, no new stage surface | **Met, except that the record's name list is stale** (R1). 981 kept + 1 allowed rename; 4 new. The string-literal multiset of `runtime.cpp` is identical to the C01 candidate's. The repair diff adds no literal. No `assert(` in the four product files. `profile_load_stage` is unchanged. |
| 5 | N-a..N-e handled; checkout and `build/rewrite-debug` never mutated by probes | **Not met.** N-a met. N-b met only through the field that breaks N-c (R2). N-e partly (R1). In-place probing (R3). |
| 6 | Gate | **Met** (below) |

## Advisory notes (non-blocking)

- **X06B3BC02-N-a.** The N-a decision (`catch (...)` maps to `external_resource`, matching
  the other arms) is recorded with its reason. Accepted.
- **X06B3BC02-N-b.** The N4 unreachable-branch reasons are now in the record. The
  `{false,false}` case adds a zero-custom-job run. `baseline_before_route` is still pinned
  only by its observation flag (`baseline-false`), not behaviourally. That is unchanged from
  C01 and acceptable.
- **X06B3BC02-N-c.** The zero-read check in "X06 B3 A into_extension adopts the same VM
  and receipts without recapture" watches the declaration of an *opaque* owner, which never
  reads a declaration on any path. It is correct, but it cannot fail for any plausible
  mutation. The behavioural adoption evidence is the catalog-pointer equality and the
  registration count.
- **X06B3BC02-N-d.** The coder's evidence folder mixes three generations: pre-repair
  `results.*` and `probes/raw/`, the stale `raw/REPORT.previous-stale.md`, and the
  final-candidate `PROBES.md` with no logs. When R3 is fixed, make the final generation
  self-contained.

## Independent gate (macOS Debug, `build/rewrite-debug`)

| # | Command | Result | Log |
| --- | --- | --- | --- |
| 1 | `cmake --build build/rewrite-debug --target planar_tests` | exit 0, `ninja: no work to do.` | `gate/01-build-planar_tests.log` |
| 2 | `ctest --test-dir build/rewrite-debug --no-tests=error -j 4 --timeout 300` | exit 0, `100% tests passed out of 985`. No failed, skipped, disabled or not-run entries; the only grep hit is a test name containing "skipped". | `gate/02-ctest.log` |
| 3 | `ctest --test-dir build/rewrite-debug -N` against the C01 review inventory | 985 names, all unique. 981 kept, 1 removed (the allowed C01 rename), 4 new. | `gate/03-ctest-N.log`, `inventory.txt`, `removed-names.txt`, `new-names.txt` |
| 4 | `python3 scripts/rewrite-check.py lint --build-dir build/rewrite-debug` | exit 0, "Formatting and Doxygen checked 136 active source files" | `gate/04-lint.log` |
| 5 | `cmake --build build/rewrite-debug`, run twice | both exit 0, `ninja: no work to do.`; the second ran zero steps | `gate/05-build-all.log`, `gate/06-build-nochange.log` |

Each exit code was written directly to its own `gate/NN.exit` file, never through a pipe.
The diff hash and the six file hashes were equal before and after the gate.

New names: `X06 B3 B exhaustion depends on route consumption because the instruction budget
is one cumulative counter`; `X06 B3 B owner allocation charge is observable and covers the
owner's own storage`; `X06 B3 B prepare_load reads the declaration through the watched
interposer`; `X06 B3 B prepare_load reverifies captured profile content exactly three times`
(the rename target). Removed: `X06 B3 B captured profile content is reverified before source
binding, after capture and after sealing` (allowed, C01-introduced).

## Reviewer mutation probes

Driver: `probes/probe.py`, run serially by `probes/run.sh`. It refuses a scratch path
inside the checkout. Each edit must occur exactly once, or nothing is built. The driver
rebuilds `planar_runtime_tests` and `planar_capture_native_tests`, then runs
`perl -e 'alarm 1500; exec @ARGV' ctest -R 'X06 B3' --no-tests=error -j 4 --timeout 300
--output-on-failure`, and the same for `D2 B2 C01 first retained reads`. It restores from an
in-memory copy and compares sha256. Specs are in `probes/mutations/*.json`, and raw ctest
output and build logs are in `probes/raw/`.

The scratch build lists 37 runtime `X06 B3` tests, against 39 in the checkout inventory. The
two missing are the `note_registration` tests in `profile.t.cpp`. They live in another test
binary that my probes did not rebuild, and no probed edit touches profile code. Baseline and
final runs: 37/37 and 1/1 pass. Every probe built with exit 0 and restored byte-equal.

| Probe | Edit | Mutant sha256 (12) | Result |
| --- | --- | --- | --- |
| `reset-f-instr` | `s.instructions = 0;` after `mark(2)` (reviewer `d9b54d0579c3`) | runtime.cpp `3f77818edcd8` | **killed**: route-consumption test, lines 2015, 2021 (1 of 37) |
| `light-overcharge-route` | `s.instructions += 60000000;` after `mark(1)` | `73bd92db2799` | **killed**: route-consumption (2005), cumulative exhaustion (1833), one budget (1897) (3 of 37) |
| `reverify-pre` | remove pre-capture verify | `185ac66a000a` | **killed**: "reverifies … exactly three times", `4 == 6` (1 of 37) |
| `reverify-moved` | move that verify after source capture (reviewer `52b29026261a`) | `cda20705630b` | survives 0 of 37 (equivalent here; see R3 detail) |
| `n1-both` | drop the owner charge in both factories | `1adbea375f61` | **killed**: owner-charge test, lines 1623, 1624 (1 of 37) |
| `decl-read-in-prepare` | read the declaration before the pre-capture verify | `456cd463cc37` | **killed**: capture change (1867), declaration leaf replaced (1737) (2 of 37) |
| `control-watches-unread-file` | test only: control watches the ledger | t.cpp `c3ac8b3bd61a` | **killed**: positive control, line 1673 |
| `ordering-test` | test only: added ordering probe (see R3 detail) | t.cpp `a7744c6edb75` | the added test fails on the candidate (`0 == 2`), so it cannot observe ordering |
| `ordering-test` + `reverify-moved` / + `reverify-pre` | combined | as above | same `0 == 2`; plus `4 == 6` under `reverify-pre` |

The mutant hashes for `reset-f-instr`, `reverify-moved`, `reverify-pre`, `n1-both`,
`decl-read-in-prepare`, `light-overcharge-route` and `control-watches-unread-file` equal the
second-pass `PROBES.md` hashes for the same edits. That is consistent with its table having
been produced on this exact candidate.

## Limits

- macOS Debug only. No RelWithDebInfo, Linux, RSS or whole-invocation budget claim.
- I did not re-run release success/failure/both, every-job, consumed, n3, the reset a/e
  variants, the byte resets, the one-factory charge removals or `samesel` on the final
  candidate. The repair touched only the factories and `observe()`, so I expect the
  pre-repair results to carry over, but that is the evidence R3 asks the coder to produce.
- I did not reach the `catch (...)` arm. No reachable trigger exists.
- My finding of an in-place violation rests on the round record's own statement. No log of
  those probes exists to inspect.
