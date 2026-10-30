# V5-02-X07-C02 gate report (gate run 2)

This replaces the first gate report, which is kept unchanged at `superseded-gate1/REPORT.md` (with its inventory and new-names list).
It was superseded because three test files changed after gate run 1 (`admission.t.cpp`, `runtime.t.cpp`, `external_registration.t.cpp`
gained two new tests, bringing the count from 1271 to 1273). The gate runner edited no source file. Raw logs of this run are under
`raw/gate2/` (container logs under `raw/gate2/linux/`); gate-1 logs under `raw/` are kept. Every gate command met its criterion.

## Candidate identity (identity.py, verbatim from `raw/gate2/00-identity.log`)

| identity | value |
|---|---|
| HEAD | `f78f8eb47267ab9c72efacc07708e7c0c2669840` |
| sha256(git diff --binary HEAD -- src tests/native), tracked only | `5d4a1139cbb22809e7faa34780c85887735db0ed4f274a07cf13a1f7aae46c71` |
| combined sha256 (diff bytes, then each untracked path NUL bytes, sorted), pathspec src tests/native, evidence excluded | `5d4a1139cbb22809e7faa34780c85887735db0ed4f274a07cf13a1f7aae46c71` |

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

`tests/acceptance/x07_replay.py` is also modified (outside identity.py's pathspec). Extra, not from identity.py: `git diff --binary HEAD`
over the whole tree hashes to `b4a0bf1878d52940a43d27b5b37216cf71aedd51d33324055d8a6ac3d38d590e` (tracked changes only).

Files edited this round: `src/cmd/planar/CMakeLists.txt`, `src/engine/capture/admission.cpp`, `src/engine/capture/admission.cppm`,
`src/engine/capture/admission.t.cpp`, `src/engine/ledger/profile_acceptance.t.cpp`, `src/engine/runtime/external_registration.t.cpp`,
`src/engine/runtime/runtime.cpp`, `src/engine/runtime/runtime.t.cpp`, `tests/acceptance/x07_replay.py`, plus the untracked round record
`docs/vernacular/rounds/V5-02-X07-C02.md`. Diff stat against HEAD: 9 files, 311 insertions, 17 deletions (`raw/gate2/09-diffstat.log`).

## Commands and results

| # | command | exit | result |
|---|---|---|---|
| 1 | `python3 scripts/rewrite-check.py lint --build-dir build/rewrite-debug` | 0 | "Formatting and Doxygen checked 147 active source files" (`01-lint.log`) |
| 2 | `cmake --build build/rewrite-debug --target planar_tests` | 0 | `ninja: no work to do.` (tree already built with the current bytes; `02-build-debug.log`) |
| 3 | `ctest --test-dir build/rewrite-debug --no-tests=error --output-on-failure` | 0 | 100% passed, 1273 of 1273, 0 failed, 587.28 s (`03-ctest-debug.log`) |
| 4 | `cmake --build build/rewrite-release --target planar_tests` | 0 | 11 steps, rebuilt the changed test objects, linked (`04-build-release.log`) |
| 5 | `ctest --test-dir build/rewrite-release --no-tests=error --output-on-failure` | 0 | 100% passed, 1273 of 1273, 0 failed, 234.39 s (`05-ctest-release.log`) |
| 6 | no-op build x2, Debug | 0, 0 | both `ninja: no work to do.` (zero steps), run after the successful build (`06-noop-debug-1.log`, `-2.log`) |
| 7 | no-op build x2, RelWithDebInfo | 0, 0 | both `ninja: no work to do.` (`06-noop-release-1.log`, `-2.log`) |
| 8 | `ctest -N` Debug vs RelWithDebInfo | 0, 0 | 1273 each, identical except the build-directory header line (`07-inv-debug.txt`, `07-inv-release.txt`) |
| 9 | `scripts/redact-local-paths.py check` with no file arguments | 2 | not meaningful: requires `--repo-path` and `--home` (`08-redact-noargs.log`) |
| 10 | `scripts/redact-local-paths.py --repo-path <repo> --home <home> check` on the 9 edited files and the round record | 0 | no findings, empty output (`08-redact.log`) |
| 11 | `git diff --stat HEAD` fence over `acceptance_seam.cpp`, `sqlite.*`, `storage.*` (ledger lock and busy wait), `native.*`, `profile.cpp`, `invocation.cpp`, `runtime.cppm` | 0 | empty output: no diff to any fenced file (`09-fence.log`) |
| 12 | `df -h /System/Volumes/Data` before the container run | 0 | 98 GiB available (limit 15 GiB); `linux/df-before.txt` 204877352 blocks of 512 bytes (about 97.7 GiB), after 166850160 (about 79.6 GiB) |
| 13 | `scripts/linux-gate.sh <scratch dir outside checkout>` (docker PATH exported) | 0 | see below |

Zero matched tests did not occur; every ctest run matched 1273. Skips: a case-insensitive grep for "skipped" in each ctest log matches only
two lines, the Start and Passed lines of the test named `command load refuses forged family order skipped duplicate and malformed arguments`.
There were no skipped tests. The release build did work only in command 4; every no-change build reported zero steps.

### Linux container gate (linux/arm64, one `docker build`, both presets, no `-t`)

All exit files in `raw/gate2/linux/container/*.exit` are 0 (the earlier `wc -l` of 1 was the line count of each one-line file, not an exit value).
Script exit 0, `docker-build.exit` 0, host manifest before and after equal, host manifest equal to the container context manifest,
`docker builder prune -f` exit 0.

| step | Debug | RelWithDebInfo |
|---|---|---|
| configure | 0 | 0 |
| build | 0 | 0 |
| ctest | 0, 100% passed, 1273 of 1273, 241.79 s | 0, 100% passed, 1273 of 1273, 538.62 s |
| no-work rebuild | 0, `ninja: no work to do.` | 0, `ninja: no work to do.` |
| `ctest -N` | 1273 names | 1273 names |

The Linux sorted inventories are byte-identical to each other and to the macOS inventory (`LC_ALL=C` sort).

## Inventory

`inventory.txt`: the 1273 registered names from macOS Debug `ctest -N`, ordinals stripped, in ctest order. Debug, RelWithDebInfo and both
Linux configs hold the same set. Comparison list: the most recent committed list is
`tests/native/v5-02-x08-c01-r7-evidence/inventory.txt` (1260 names, last changed in commit f78f8eb4), which matches the 1260-name baseline.
Against it: 13 new names (`new-names.txt`), 0 removed (`removed-names.txt` empty), 1260 + 13 = 1273. The 13 new names:

1. `replacement.capture admission load ledger receipt tolerates committed content changes`
2. `replacement.capture admission load ledger receipt refuses a replaced or removed file`
3. `replacement.capture admission load ledger receipt refuses a mode change`
4. `replacement.capture admission verify refusal poisons the capability even when the change reverts` (new since gate 1)
5. `replacement.capture admission content changes still refuse for every other supplied file role`
6. `replacement.capture admission load ledger role refuses trusted aliases and links`
7. `replacement.X07 C02 loser prepared after a winner commit refuses revision_conflict and not capture_changed`
8. `replacement.X07 C02 prepared profile native receipts ignore a committed ledger change`
9. `replacement.X07 C02 post-preparation input change plus winner commit still refuses revision_conflict`
10. `replacement.X07 C02 history family ledger receipt stays strict at the built-in admission site`
11. `replacement.C4 B2 appended admitted ledger refuses verify before any ledger read`
12. `replacement.C3 B2 appended observations file refuses verify and stays refused` (new since gate 1)
13. `x07.ledger_commit_after_admission_reports_revision_conflict`

## (a) Acceptance criteria to tests

Source: round record "Acceptance criteria (criterion 1 amended)" and `plan/INVARIANTS.md`. No corrective-round findings exist for this
round. Line numbers are from the current files.

| criterion | tests (file:line) |
|---|---|
| 1. Deterministic chain: verify failure message at HEAD, redaction to exit 4, CLI interleaving shows `capture_changed` at HEAD vs `revision_conflict` exit 3, one winner, rows (1,1,2), no loser row | step 1: `X07 C02 prepared profile native receipts ignore a committed ledger change` (`src/engine/ledger/profile_acceptance.t.cpp:823`) and `X07 C02 loser prepared after a winner commit refuses revision_conflict and not capture_changed` (`:797`); step 2: `X07 adapter redacts malformed trusted payloads as internal protocol` (`src/cmd/planar/bootstrap.t.cpp:2062`, pre-existing); step 3: `x07.ledger_commit_after_admission_reports_revision_conflict` (`tests/acceptance/x07_replay.py:130`, registered `src/cmd/planar/CMakeLists.txt:235`). The supplementary non-gating 12-busy-loop release reproduction was not run by the gate runner. |
| 2. `x07.two_process_cas_one_winner` passes everywhere | `tests/acceptance/x07_replay.py:107`, registered `CMakeLists.txt:232`; passed in macOS Debug, macOS RelWithDebInfo, Linux Debug, Linux RelWithDebInfo. The plan's I13 repeat run (`--repeat until-fail:20`) is not in the gate command list and was not run here (the probes note says the orchestrator owns it). |
| 3. Real input changes still refuse `capture_changed`; table unchanged | `x07.receipt_races_refuse_before_authorization` and `x07.final_race_rolls_back_before_commit` (`x07_replay.py:115`); `capture admission content changes still refuse for every other supplied file role` (`src/engine/capture/admission.t.cpp:1081`); `capture admission load ledger receipt refuses a replaced or removed file` (`:1039`); `capture admission load ledger receipt refuses a mode change` (`:1056`); `capture admission verify refusal poisons the capability even when the change reverts` (`:1067`); `capture admission load ledger role refuses trusted aliases and links` (`:1142`); `X07 C02 post-preparation input change plus winner commit still refuses revision_conflict` (`profile_acceptance.t.cpp:835`); `X07 command sources add no diagnostic outside the closed table` (`bootstrap.t.cpp:2094`) |
| 4. Invariant and decision tables, one mutant per invariant | tables `plan/INVARIANTS.md`, `plan/DECISIONS.md`; results in `probes/PROBES.md` (14 invariant rows run, 14 killed; exploratory E1, E5 and E7 survived). Row to test map: I1/I17/I23 `admission.t.cpp:1022`; I2 `:1039`; I3 `:1056`; I4/I10 `:1081`; I5 `:1142`; I6 `profile_acceptance.t.cpp:797`; I7 `:823`; I8 `x07_replay.py:130`; I9 `x07_replay.py:115` (see probe note: killed only by unit tests); I9b `profile_acceptance.t.cpp:835`; I12a `src/engine/runtime/runtime.t.cpp:527`; I12b `profile_acceptance.t.cpp:849`; I15 `bootstrap.t.cpp:2094`; I15b `bootstrap.t.cpp:2062`; I13 `x07_replay.py:107`. The two tests added after the probes target the survivors: E1 (verify poisoning) by `admission.t.cpp:1067`, and E7 (extension observations slot strictness) by `external_registration.t.cpp:3199`. They were not re-probed in the evidence I could see. |
| 5. Inventory identical in Debug and RelWithDebInfo, 1260 names kept | commands 8 and 13 above: identical, 0 removed |
| 6. Process gates | commands 1 to 13 |
| Fencing (gap 7, I18) | command 11: no diff to any fenced file |

## (b) Red-first

Source: `red/RED.md`, `red/raw/t1.log` to `t15.log`, `red/snapshot/`. Red-time identity from RED.md: HEAD
`f78f8eb47267ab9c72efacc07708e7c0c2669840`; sha256 of `red/snapshot/candidate.diff` `99c8183a25e1ffe790ebd14c4a8a9ffd7904a7d270e186749769400349e0b1e3`;
sha256 of `red/snapshot/SHA256SUMS` `9b82beb07e9e893ea312dbc0819252952f4fd631cd9ada22f72dd44c901f72f5`. Build command (RED.md): `cmake --build build/rewrite-debug
--target planar_tests`, rc 0. The tests ran against the test-package snapshot (tests plus a one-line `load_ledger` stub, product fix absent), not a pristine
HEAD overlay. The literal per-test ctest command lines are not recorded in RED.md: it says only that tests were run by name with the `replacement.` prefix
(x07.* unprefixed). The D15 pristine-HEAD overlay recipe is not evidenced as executed.

| test | outcome | failing line | real red or guard |
|---|---|---|---|
| `capture admission load ledger receipt tolerates committed content changes` (`admission.t.cpp:1022`) | failed | `admission.t.cpp:1019 CHECK(result)` | real red |
| `capture admission load ledger receipt refuses a replaced or removed file` | passed | none | guard, already passed |
| `capture admission load ledger receipt refuses a mode change` | passed | none | guard, already passed |
| `capture admission content changes still refuse for every other supplied file role` | failed | `admission.t.cpp:1019 CHECK(result)` (setup assertion against the stub, not the intended clause) | RED.md flags this as unexpected for a guard; passes in the final candidate |
| `capture admission load ledger role refuses trusted aliases and links` | passed | none | guard, already passed |
| `X07 C02 loser prepared after a winner commit ...` (`profile_acceptance.t.cpp:797`) | failed | `profile_acceptance.t.cpp:808` | real red |
| `X07 C02 prepared profile native receipts ignore a committed ledger change` (`:823`) | failed | `profile_acceptance.t.cpp:831 CHECK(verified)` | real red |
| `x07.ledger_commit_after_admission_reports_revision_conflict` (`x07_replay.py:130`) | failed | `x07_replay.py:134` assert status==3 / `revision_conflict` | real red |
| `x07.receipt_races_refuse_before_authorization`, `X07 C02 post-preparation input change ...`, `C4 B2 appended admitted ledger refuses verify before any ledger read`, `X07 C02 history family ledger receipt stays strict ...`, `X07 command sources add no diagnostic outside the closed table`, `X07 adapter redacts malformed trusted payloads ...`, `x07.two_process_cas_one_winner` | passed | none | guards, already passed (HEAD behaviour is green by design); their ability to fail is shown by the probe kills in `probes/PROBES.md` |
| `capture admission verify refusal poisons the capability even when the change reverts` (`admission.t.cpp:1067`) | no red evidence | none recorded | MISSING: added after the red run; it is aimed at probe survivor E1 and was not red-checked or re-probed in the evidence |
| `C3 B2 appended observations file refuses verify and stays refused` (`external_registration.t.cpp:3199`) | no red evidence | none recorded | MISSING: added after the red run; aimed at probe survivor E7; not re-probed in the evidence |

No red run shows exit 4 `internal`. By operator option A (2026-10-30) the deterministic chain stands in for it.

## (c) Deliberate behaviour differences

Fix shape (a'): new `admission_file_role::load_ledger`, used only by `open_known_load`, whose receipt compares device, inode and mode (plus descriptor and
name) and not size or times. A committed winner no longer fails the loser's verify, so the loser reaches the CAS and refuses `revision_conflict` instead of
`capture_changed`. Permitting clauses: ledger contract load step 3 (relookup), step 4 (CAS mismatch gives `revision_conflict` even if content equals the head),
step 5 ("selected file/root identities and normalized file modes remain checked"); labeled "new within contract" in `plan/DECISIONS.md`. The built-in
history-family site (`admit_ledger`) and the extension host ledger site keep strict receipts (`X07 C02 history family ledger receipt stays strict at the built-in
admission site`, `C4 B2 appended admitted ledger refuses verify before any ledger read`). No diagnostic message changed (`X07 command sources add no diagnostic outside the
closed table` passes).

## Limitations

- Mutation probes (`probes/PROBES.md`) ran before the final two tests were added and in Debug on macOS only; E1, E5 and E7 survived there, and the two new tests have no recorded red or probe run.
- I9: the named CLI subcases pass under the mutant; the invariant is held by unit tests (probe note).
- The I13 `--repeat until-fail:20` run and the supplementary 12-busy-loop release reproduction were not part of this gate and were not run.
- `scripts/linux-gate.sh` builds both presets in one docker build, so the builder prune ran once after both; linux/arm64 only.
- The identity pathspec is `src tests/native`, so `x07_replay.py` is covered only by the whole-tree `git diff` hash.
- Red-first evidence was gathered against a stub snapshot, not a pristine HEAD overlay, and per-test command lines are not recorded.
