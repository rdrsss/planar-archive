# V5-02-X06-B3-B gate report

Gate runner output for the bound capture, route, seal and all-job validation candidate. Raw logs are under `tests/native/v5-02-x06-b3-b-evidence/raw/`. No source file was edited by the gate runner. This report replaces the P4 skeleton.

## Candidate identity

- HEAD: `4b79334929965b5e93ec4c790f866f7b9762b791` on `dev/planar-v2-vernacular`
- sha256 of `git diff` (tracked changes only, six files, 760 insertions, 95 deletions): `db2969f6cd5f66bfdcbbdd13416091f63ba8896319993d828344aa34e3f76d3a`. It was identical before and after the gate.
- Changed files: `src/engine/capture/profile.{cpp,cppm,t.cpp}`, `src/engine/runtime/runtime.{cpp,cppm,t.cpp}`. The round record is untracked.

## Commands (serial, unpiped, macOS Debug)

| # | Command | Exit | Result | Log |
| --- | --- | --- | --- | --- |
| 1 | `cmake --build build/rewrite-debug --target planar_tests` | 0 | built, last step 685/686 | raw/01-build-planar_tests.log |
| 2 | `ctest --test-dir build/rewrite-debug --no-tests=error` | 0 | 100% passed, 977 of 977 matched; 0 failed; no Skipped, Disabled or Not Run entries (the only "skipped" text is a test name); 331.03 s | raw/02-ctest.log |
| 3 | `python3 scripts/rewrite-check.py lint --build-dir build/rewrite-debug` | 0 | clang-format --Werror and Doxygen: "Formatting and Doxygen checked 136 active source files" | raw/03-lint.log |
| 4 | `cmake --build build/rewrite-debug` (first full build) | 0 | built, 17 steps ending with bin/planar | raw/04-build-all.log |
| 5 | `cmake --build build/rewrite-debug` (second) | 0 | "ninja: no work to do." (zero steps) | raw/05-build-nochange.log |

passed = true. All five criteria met. ctest -N is in raw/06-ctest-N.log (977 tests).

## Inventory

Compared against the most recent committed inventory, `tests/native/v5-02-x06-b3-a-c02-evidence/inventory.txt` (963 names). Current list: `inventory.txt` (977 names, sorted, number prefixes stripped).

- Preserved: all 963.
- Removed: 0 (`removed-names.txt` empty).
- New: 14 (`new-names.txt`), all prefixed `replacement.X06 B3 B`. 963 + 14 = 977.

## (a) Acceptance criteria to tests

Round record: `docs/vernacular/rounds/V5-02-X06-B3-B.md`. No corrective findings are closed by this round (it is a first candidate).

| Criterion | Test (file:line) |
| --- | --- |
| 1 success through phase 2, custom and mixed jobs | "X06 B3 B prepare_load succeeds through phase 2 for custom and mixed jobs" (src/engine/runtime/runtime.t.cpp:1587) |
| 1 invalid unselected job fails and publishes nothing | "X06 B3 B an invalid unselected job fails all-job validation and publishes nothing" (src/engine/runtime/runtime.t.cpp:1622); "a custom job refusal after earlier shape dispatch is ordinary semantic data" (src/engine/runtime/runtime.t.cpp:1638) |
| 1 every job validated exactly once | "X06 B3 B every job is validated exactly once" (src/engine/runtime/runtime.t.cpp:1649) |
| 2 replaced declaration leaf refuses terminally before content | "X06 B3 B declaration leaf replaced before prepare_load refuses terminally before content" (src/engine/runtime/runtime.t.cpp:1667) |
| 3 one frame, no counter reset | "X06 B3 B one frame cumulative counters never reset" (src/engine/runtime/runtime.t.cpp:1733) |
| 3 cumulative exhaustion, first error kept, nested roots cleaned | "X06 B3 B cumulative instruction exhaustion preserves the first error and cleans nested roots" (src/engine/runtime/runtime.t.cpp:1754) |
| 3 / N2 profile-owner frame branches | "X06 B3 B profile frame branches stay reachable and first terminal native failures are preserved" (src/engine/runtime/runtime.t.cpp:1780) |
| 4 final intent equality, no caller upgrade | "X06 B3 B final intent must equal the bound intent" (src/engine/runtime/runtime.t.cpp:1688) |
| 4 opaque, consumed, failed, moved-from refuse | "X06 B3 B opaque consumed failed and moved-from owners refuse prepare_load" (src/engine/runtime/runtime.t.cpp:1712) |
| 4 stage handle has no public accessor | no test; enforced by the interface (no accessor declared in runtime.cppm). Reviewer should inspect the interface. |
| 5 all 963 names preserved | Inventory comparison above |
| 5 byte-identical diagnostics, no new assert, typed failures, docs | Existing tests pass; lint/Doxygen pass; no dedicated grep audit was run by the gate runner |
| 6 mutation probes | NOT PRESENT, see (b) |
| 7 fresh profile acceptance refusal and CLI unchanged | Existing tests pass (all 963 preserved). No new test. |
| 7 dependency audit, no cycle | Not a test. Read-only P4 audit (record of the round and earlier skeleton) found no CMake edit needed; the gate runner did not re-run it. Both CMakeLists are unmodified per git status. |
| N3 known-load into_extension refusal | "X06 B3 B known-load owner refuses into_extension terminally-free and stays usable" (src/engine/runtime/runtime.t.cpp:1468) |
| N4 owner allocation charged | "X06 B3 B owner implementation allocation is charged identically by both factories" (src/engine/runtime/runtime.t.cpp:1493) |
| N1 single frame, private begin_frame | covered indirectly by the one-frame test (src/engine/runtime/runtime.t.cpp:1733); no separate test |
| N5 runtime @file brief | documentation only; no test |
| X06B3AC02-N3 note_registration | "X06 B3 B note_registration is a pure counter that never affects authority" (src/engine/capture/profile.t.cpp:876); "note_registration is a no-op on moved-from owners and the count moves with the capture" (src/engine/capture/profile.t.cpp:907) |

## (b) Red-first evidence

Missing. The round's evidence directory contained only the P4 skeleton REPORT.md when the gate ran; the coders left no red snapshot hash, red command, failing line or log for any of the 14 new tests. I looked for none elsewhere and invent nothing. For each of the 14 tests the status is therefore **unknown: not shown whether a real red or a guard that already passed**. Reviewers should require the coders' red-first record, or treat these as unproven regressions.

Likewise, criterion 6 mutation probes (six, with raw logs and equal restored hashes) have no evidence: no probe logs exist under this round's directory. The probe table in the skeleton was never filled.

## (c) Deliberate behaviour differences

- Known-load `into_extension` now refuses with typed `invalid_state` (X06B3A-N3). Permitted by the round record's decision: adoption would change mode after load binding, which the architecture forbids. Not a product choice per the record.
- Each owner and stage is charged `sizeof(implementation) + 64` bytes cumulatively with no refund (N4), per the round decisions.
- `begin_frame` is private (N1); public validation entry semantics unchanged per the record.
- `note_registration` is not restricted (cannot across named modules); observational only (AC02-N3).
- `acceptance/source_capture.*` unedited: `open_bound` already performs the same_selection and leaf checks (record's decision).

## Limitations

- macOS Debug only; no RelWithDebInfo, Linux, RSS or whole-invocation budget claim.
- Items marked missing above: red-first records, mutation probes, source-level audits of assert/diagnostic bytes.
- Some defensive branches are reachable only through same-module observers (per P2).
- No prepared_profile, native v2 derivation or transaction API (B3-C).
