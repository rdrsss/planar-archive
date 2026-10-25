# V5-02-X06-B3-A-C02 independent review (profile invocation owner, second correction)

Disposition: **approve**. This review covers the combined B3-A+C01+C02 candidate.
C02 closes both required changes of the [B3-A+C01 review](V5-02-X06-B3-A-C01-review.md):

- **X06B3AC01-R1 is closed.** A real per-owner registration count now changes on
  every successful snapshot inside `register_vm`, and `profile_invocation_observation`
  exposes it. Tests assert exactly one registration for each factory and none
  across `into_extension`. I reran the three double-`register_vm` mutants myself.
  Each fails the new assertions with `2 == 1`. Two extra mutants of my own also
  fail: one adds a registration inside `into_extension`, and one turns the counter
  into a no-op. The misleading test comment is corrected, and a new C02 note
  corrects the C01 NOTES claim.
- **X06B3AC01-R2 is closed.** The name `source capture N1 unreadable declaration is
  not reported as missing` is back. The inventory has 963 names. It keeps all 957
  B3-A names and all 961 C01 names, with nothing removed.

The optional N2 and N3 changes are applied. Apart from the observable, C01's
source behaviour is unchanged. The full Debug gate passes 963/963. I found three
small advisory points, none blocking.

I reviewed in the shared checkout on `dev/planar-v2-vernacular` at HEAD
`d783c0b9514c50c8a2ad060281c4e75400d5b2dd`, against the uncommitted candidate. I
wrote only this file and new files under
`tests/native/v5-02-x06-b3-a-c02-review-evidence/`. The mutation probes ran in a
temporary detached worktree and build directory in my scratchpad. Both were
removed afterwards, and the shared checkout was never mutated. I did not commit,
install, publish or merge. I used no old Planar tool, queue, skill, role or
installed binary. Builds, ctest and lint in `build/rewrite-debug` ran one at a
time, and no other ninja or ctest process was running.

## Exact candidate

- `git diff HEAD -- src` sha256 at review start and end:
  `76d1b84fdd1c2a6ec09705b20aaad5ae3ce5ba94e006475c2d6c8a19f37d68f6`. This equals
  the round record, the gate report and the coder's `candidate-c02.patch`.
- Whole-tree `git diff HEAD` (including the coordinator's `roadmap.md`) at start
  and end: `d99f01da977265ea97a12084292aeff1d169da422b04bd3bb286caa0abd8ed75`.
  This equals the gate report
  ([`diff-at-review-start.sha256`](../../../tests/native/v5-02-x06-b3-a-c02-review-evidence/diff-at-review-start.sha256),
  [`diff-at-review-end.sha256`](../../../tests/native/v5-02-x06-b3-a-c02-review-evidence/diff-at-review-end.sha256)).
  Excluding `roadmap.md`, it equals the `src` hash above.
- **The C02 delta is exactly what the evidence says.** I applied the candidate to
  a clean `d783c0b9` worktree, then reverse-applied the coder's `c02-vs-c01.diff`
  (`git apply -R -p1 --directory=src`). The resulting `git diff HEAD -- src` hash
  is `84c68c2e287a9d240bef76d5e183a5f3a4365768cb7216f13fd454449029f2b6`, the
  reviewed C01 candidate. Applying the delta forward gives `76d1b84f…` again.
  The delta therefore contains the whole difference from C01. It touches only
  seven files, all owned: `runtime.cppm/.cpp/.t.cpp`, `profile.cppm/.cpp`,
  `admission.cppm` and `source_capture.t.cpp`. The CMake lists, `admission.cpp`,
  `source_capture.cpp/.cppm` and every other C01 file are byte-identical to C01.
  `admission.cpp` (`7434866d…`) and `source_capture.cpp` (`a3e671cd…`) still
  match the hashes recorded at the C01 review.

## X06B3AC01-R1: registration observable

**Product code.** `profile_capture::implementation` gains `mutable std::size_t
registrations`. `note_registration()` increments it, and `registration_count()`
reads it. Moved-from owners make no change and report zero. The single
registration path, `extension_invocation::register_vm` (`runtime.cpp:4790`), calls
`capture.note_registration()` after the snapshot succeeds and before
`command_vm::create`. Both factories (`runtime.cpp:4889` known, `:4913` opaque)
and the test-only `extension_invocation::create` (`:4812`) go through this path.
No other source calls these methods.

`profile_invocation_observation::registrations` returns the live capture count
for an unconsumed owner. For a consumed owner, it returns the count taken at
adoption. `into_extension` reads that count from the adopted capture as its last
fallible step, just before `consumed = true`. So a registration added anywhere
earlier in the adoption body is visible on the consumed owner. The count lives on
the capture itself, so it is real state, not a constant: it changes on every
registration that reaches the snapshot step.

**Tests.** `X06 B3 A factory registration counters are deterministic across
factories` now also checks `registrations == 1` for the known, opaque and repeated
known owners (`runtime.t.cpp:1253–1255`). The new case `X06 B3 A registration
count stays one across into_extension` (`:1257`) checks 1 before and after
adoption for both factories (`:1260/1263` known, `:1267/1270` opaque). `f.clean()`
is `open_known_load` through the fixture's `open()`.

**Comment and note.** The comment above the counter test (`runtime.t.cpp:1240–1241`)
now says that equal instruction and native-byte counts "CANNOT detect a second
register_vm, because each call builds a fresh VM", and names the registrations
observable as the detector. This is accurate. The new
`NOTES-C01-r2-r3-correction.md` quotes both wrong C01 claims, explains why they
are false and points to the C02 reds. As the round requires, it leaves the
reviewed C01 `r2-r3/NOTES.txt` untouched.

**Independent probes.**
([`probes/INDEX.txt`](../../../tests/native/v5-02-x06-b3-a-c02-review-evidence/probes/INDEX.txt),
`run_probes.sh`, `extra_mutate.py`, raw `apply-/build-/run-*.log`.) I ran
`git worktree add --detach <scratch>/wt d783c0b9`, then applied the candidate patch
(hash verified as `76d1b84f…`). I configured a fresh Debug build there with the
pinned toolchain and `-DPLANAR_WARNINGS_AS_ERRORS=ON`. For each mode the script
mutates the file, builds `planar_runtime_tests`, runs `'[x06b3a]'` under a
600-second `perl` alarm and restores the saved copy. The double-registration
mutants use the reviewer's original
`tests/native/v5-02-x06-b3-a-c01-review-evidence/probes/mutate.py`.

| Mode | Mutated file sha256 | Build | Run | Failing assertions |
| --- | --- | --- | --- | --- |
| baseline (unmutated) | runtime.cpp `7393ba85…` | 0 | 0 | All passed, 312 assertions / 17 cases |
| opaque double `register_vm` | `4d67e5e1…d857` | 0 | 42 | 3: `runtime.t.cpp:1254, 1267, 1270` `CHECK( opaque->observe().registrations == 1 )` → `2 == 1` |
| known double `register_vm` | `5f93f71a…c761` | 0 | 42 | 4: `:1253` known, `:1255` again, `:1260, :1263` owner → `2 == 1` |
| both | `b8698327…afdc` | 0 | 42 | 7: `:1253, 1254, 1255, 1260, 1263, 1267, 1270` → `2 == 1` |
| adopt: extra `register_vm` on the adopted capture inside `into_extension` (mine) | `031f3a52…8b0b` | 0 | 42 | 2: `:1263` owner and `:1270` opaque (post-adoption checks) → `2 == 1` |
| noop: `note_registration` body removed (mine) | profile.cpp `bfafa9ba…eba5` | 0 | 42 | 7: all registration checks → `0 == 1` |
| restored | runtime.cpp `7393ba85…`, profile.cpp `1ff57e2d…` | 0 | 0 | All passed, 312 / 17 |

My three double-registration mutant hashes are byte-identical to the coder's
recorded hashes, and the failing lines match theirs. At C01 the same mutants
survived with 294 assertions in 16 cases. Here they are killed by the new
assertions alone; every failing line is a `registrations == 1` check. The
`adopt` mutant shows the "none across `into_extension`" assertions are live. The
`noop` mutant shows the assertions depend on the product counter. After restore,
the worktree hash returns to `76d1b84f…`.

## X06B3AC01-R2: restored name

`source capture N1 unreadable declaration is not reported as missing` is again a
`TEST_CASE` (`source_capture.t.cpp:587`). Its body is the C01 "unreadable mode 0000
declaration" section, moved out unchanged: on macOS it expects io/`Cannot open
confined input` and no absence, on Linux it expects admission and no absence, and
under root it expects admission. The forbidden `Cannot open admitted input` text
does not come back.

Inventory ([`inventory-review.txt`](../../../tests/native/v5-02-x06-b3-a-c02-review-evidence/inventory-review.txt),
[`inventory-compare.txt`](../../../tests/native/v5-02-x06-b3-a-c02-review-evidence/inventory-compare.txt)):
`ctest -N` lists 963 unique names. Compared with the 957-name B3-A list, none is
missing and six are added: the four C01 names plus the two C02 names. Compared
with the 961-name C01 list, none is missing and two are added: the restored name
and `X06 B3 A registration count stays one across into_extension`. My list is
identical to the evidence `inventory.txt`. The dispatch expected "at least 961";
963 is consistent with that.

## Optional N2 and N3

- **N2 applied.** The root-only branch at `runtime.t.cpp:1173–1176` now uses
  `SUCCEED(...); return;`. No `SKIP(` remains in `runtime.t.cpp`,
  `source_capture.t.cpp` or `admission.t.cpp`.
- **N3 applied as documentation.** `admission.cppm:89–92` adds that the value
  reflects only the most recent admission open attempt, and that `admit_file`
  clears it on entry. It also says retain, poison and healthy calls never write
  the value, so it persists across them on a poisoned handle. The code agrees
  (`admission.cpp:144, 191, 202, 350`). One clause is slightly loose; see
  X06B3AC02-N1.

## B3-A and C01 criteria re-check

| Criterion | Finding |
| --- | --- |
| Admission messages byte-identical to HEAD | **Met.** The admission/profile `.cpp`/`.cppm` diff against HEAD adds no line containing a string literal. `admission.cpp` is byte-identical to C01. |
| Absence signal unchanged | **Met.** `admission.cpp`, `source_capture.cpp/.cppm` and the factory call site are unchanged from C01. The C01 absence tests all pass. |
| No public CLI change | **Met.** No handler, `src/cmd` source or CMake list differs from C01. The new accessor and observation field are engine-internal. |
| No new cycle | **Met.** CMake lists are byte-identical to C01, which are byte-identical to B3-A; the B3-A DAG audit stands. `runtime` already imported `capture/profile`. |
| No runtime `assert()` | **Met.** The `src` diff against HEAD adds no `assert(`. |
| Owner semantics (one capture, one registration, adoption) | **Met.** A second capture is detected by the C01 read-count test. A second registration is now detected for each factory and across adoption. |
| Rest of C01 preserved; only owned files | **Met.** Proven by the reverse-apply hash identity above. |

## Advisory notes (non-blocking)

- **X06B3AC02-N1: N3 wording on non-file admissions.** The new doc says "Root,
  target and trusted admissions also begin by clearing it." They clear it only
  inside `open()`, and `admit_root`, `admit_target` and `select_trusted` call
  `check()` first (`admission.cpp:309–334, 371–376`). On a poisoned handle they
  refuse before clearing, so the value also persists across those refusals. The
  known factory reads the signal right after `admit_file`, so nothing is affected.
  Tighten the sentence when this file is next touched.
- **X06B3AC02-N2: what "attempt" counts.** `note_registration` runs after the
  snapshot succeeds, so a `register_vm` call that fails at the snapshot is not
  counted. The observation's comment, "Cumulative register_vm calls", is therefore
  slightly broad. A factory that fails there returns no owner, so the difference
  is not observable through the product API, and R1 is satisfied.
- **X06B3AC02-N3: public mutator.** `profile_capture::note_registration()` is an
  exported, public const mutator. Any holder of a capture could bump the count.
  The count carries no authority and has no other caller today. If later rounds
  rely on it beyond tests, consider limiting it to the registration path (a
  friend or private seam).
- Carried forward: X06B3AC01-N1 (embedded versus known message for an
  unsearchable directory) stays deferred to X07 public-cutover parity, as the
  round records. The B3-A carried duties N2 to N4 stay with B3-B/X07.

## Prior findings

| Finding | Status |
| --- | --- |
| X06B3AC01-R1 second registration not detected | **Closed** (real count, assertions, three mutant reds rerun independently, comment and note corrected) |
| X06B3AC01-R2 removed name | **Closed** (restored; 957/957 B3-A and 961/961 C01 names present) |
| X06B3AC01-N2 `SKIP` in root branch | **Closed** (`SUCCEED`) |
| X06B3AC01-N3 signal-scope wording | **Closed**, with a residual wording nit (X06B3AC02-N1) |
| X06B3AC01-N1 embedded/known parity | Deferred to X07 (recorded) |
| X06B3A-R1 to R4 | Remain closed; C02 does not touch the code they cover |

## Independent gate (macOS Debug, `build/rewrite-debug`)

| # | Command | Result |
| --- | --- | --- |
| 1 | `cmake --build build/rewrite-debug --target planar_tests` | exit 0, `ninja: no work to do.` ([log](../../../tests/native/v5-02-x06-b3-a-c02-review-evidence/01-build-tests.log)) |
| 2 | `ctest --test-dir build/rewrite-debug --no-tests=error --output-on-failure -j1` | exit 0, `100% tests passed out of 963`, 324.89 s; zero failed, not-run, skipped or disabled lines ([log](../../../tests/native/v5-02-x06-b3-a-c02-review-evidence/02-ctest.log)) |
| 3 | `ctest -N` inventory | 963 names; 0 missing against B3-A (957) and C01 (961) |
| 4 | `python3 scripts/rewrite-check.py lint --build-dir build/rewrite-debug` | exit 0, `Formatting and Doxygen checked 136 active source files` |
| 5 | `cmake --build build/rewrite-debug`, twice | both exit 0, `ninja: no work to do.` |

Exit codes are in [`exit-codes.txt`](../../../tests/native/v5-02-x06-b3-a-c02-review-evidence/exit-codes.txt).
The tree was already built, so command 1 did no work. Ninja reported nothing out
of date, and the diff hash was unchanged from start to end, so the binaries match
the hashed candidate. A separate clean configure and build of the same patch in my
scratch worktree, with warnings as errors, compiled without errors.

## Limits

macOS Debug only. No RelWithDebInfo or Linux run, so the Linux behaviour of the
restored N1 test (`O_PATH` branch) is reasoned from source, not executed. The
mutation probes ran against `planar_runtime_tests '[x06b3a]'` in a scratch
worktree built from the hashed candidate patch, not in the shared build directory.
Each mutant was restored, and the restored hashes are recorded. No RSS,
whole-invocation budget, preparation or transaction claim is made.
