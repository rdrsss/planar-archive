# V5-02-X06-C-C02 gate report (run 3)

Result: passed = true. All five gate commands and the redact check met their criteria. This run gates the candidate after the coder's C14 fixture repair (two 9 MiB registry files instead of one 17 MiB file; the C02 test line numbers moved by +2 versus run 2). Earlier reports are kept as raw/REPORT-run1-superseded.md and raw/REPORT-run2-superseded.md; run 3 logs are raw/g3-*.

## Candidate identity (2026-10-26)

HEAD caaf0396ae90a7f7d2d511b9a9a6e185e4362d52 plus the uncommitted X06-C, C01 and C02 candidate.

| Pathspec | sha256 |
|---|---|
| `git diff HEAD -- src` (tracked only) | e9f10163530691bc2a3f8b2666448ba7b5db1383f53ac01b6b67f2a4040cb582 |
| `git diff HEAD` whole tree (tracked only) | 5bcd9cc3c2d02ec6b3a49df8b79189f1c1bd6b92e19603fa8bdad7f9391b4b4e |
| tracked `git diff HEAD -- src`, then cat of acceptance_seam.cpp, acceptance_seam.cppm, profile_acceptance.cpp, profile_acceptance.cppm, profile_acceptance.t.cpp (untracked, in that order) | fff9ee6408f8b3e86f9fd874dcf1b047e3a46380fe3af7882e69bcb74c336de4 |

The two tracked hashes equal run 2 (the repair touched only the untracked profile_acceptance.t.cpp). Run 2's combined hash e2d50ed1... differs from the combined hash above because of the repair and possibly a different concatenation method; compare only against hashes taken with the stated method.

## Commands (raw logs raw/g3-NN-*.log, exit codes raw/g3-NN.exit)

| # | Command | Exit | Result |
|---|---|---|---|
| 1 | `cmake --build build/rewrite-debug --target planar_tests` | 0 | ninja: no work to do (binary already current) |
| 2 | `ctest --test-dir build/rewrite-debug --no-tests=error --output-on-failure` | 0 | 100% tests passed, 0 failed out of 1036; no skipped or not-run lines (grep for `***` and Not Run: 0); Total Test time (real) 378.31 sec. #531 passed in 15.67 s |
| 3 | `python3 scripts/rewrite-check.py lint --build-dir build/rewrite-debug` | 0 | "Formatting and Doxygen checked 141 active source files" |
| 4 | `cmake --build build/rewrite-debug` (first) | 0 | ninja: no work to do |
| 5 | `cmake --build build/rewrite-debug` (second) | 0 | ninja: no work to do; zero steps, pass |
| 6 | `python3 scripts/redact-local-paths.py --repo-path <repo> --home <home> check <17 files>` | 0 | no output. Files: roadmap.md, five round records (X06-C, X06-C-review, X06-C-C01, X06-C-C01-review, X06-C-C02), src/engine/CMakeLists.txt, acceptance.{cpp,cppm,t.cpp}, acceptance_seam.{cpp,cppm}, profile_acceptance.{cpp,cppm,t.cpp}, the two generate.py scripts. No file-less invocation was run. Evidence files were not included. |

Matched tests 1036 (nonzero), failures 0, skips 0 reported.

Caveat on commands 1, 4, 5: all three reported no work, so no build that does work was observed in this run; the relink happened in the coder's earlier build. The no-change check is nevertheless valid (successful build 4 then no-work build 5).

## Inventory

Saved as inventory.txt (1036 sorted names, identical to the run 2 file by diff) and raw/g3-ctest-N.log (ctest -N, Total Tests: 1036). inventory-full.txt is the unsorted run 2 listing. Most recent committed inventory found: tests/native/v5-02-x06-c-c01-evidence/inventory.txt (1031 names; comm against the current list shows 0 removed). The b3-c-c02 committed list (1003 names) is a subset of the C01 list per the round's own count; I did not compare it separately in this run.

New names versus C01 (5), all in src/engine/ledger/profile_acceptance.t.cpp:
- replacement.X06 C C02 seam key has no public constructor (line 981)
- replacement.X06 C C02 complete_native_candidate requires the passkey (line 989)
- replacement.X06 C C02 candidate_values requires the passkey (line 996)
- replacement.X06 C C02 native_candidate callables and constructor require the passkey (line 1002)
- replacement.X06 C C02 passkey grants are confined to the adapters and the test probe (line 1013)

Removed names: none.

## (a) Acceptance criteria to tests (current line numbers)

Files under src/engine/ledger/.

| Criterion | Tests (file:line) | Status |
|---|---|---|
| 1 Every seam callable and constructor requires the passkey; private key constructor; only adapters befriend | profile_acceptance.t.cpp: "X06 C C02 seam key has no public constructor" :981; "complete_native_candidate requires the passkey" :989; "candidate_values requires the passkey" :996; "native_candidate callables and constructor require the passkey" :1002; "passkey grants are confined to the adapters and the test probe" :1013 | Runtime tests pass. The negative-compile probe (non-adapter target, raw log naming the private constructor) is the prober's deliverable; probes/PROBES.md and probes/raw exist but this gate did not verify their content, so I cannot confirm that part. |
| 2 R1 scan test stays; all 1031 names kept | acceptance.t.cpp:770 "X06 C C01 seam is unreachable from a plain acceptance importer" | Met: 0 removed, 1036 pass. |
| 3 Registry-limit test asserts no staging ran; C14 kill names this test | profile_acceptance.t.cpp:878 "X06 C C01 seam interns the registry tree with the provenance limits", `CHECK_FALSE(installed.probe.pending)` at :900 (verified in source), with the two-9-MiB-file repair | Assertion present and passing. The C14 re-probe belongs to the prober; not run here. |
| 4 No-Lua test comment rewritten, agrees with X27 | profile_acceptance.t.cpp:574 "X06 C no Lua runs while the profile write transaction is open" (comment at :575-578 per run 2) | Comment rewritten; record corrections are the coordinator's. |
| 5 Records and hashes | not a test | Not checked by this gate; fresh hashes above name their pathspecs. |
| 6 Passkey and C14 invariant rows | not a test | Prober/coordinator item. |
| 7 Dependency audit | not a test | Not run by this gate. |
| 8 Diagnostics byte-identical; #531 not slower | full ctest | Diagnostic tests pass. #531 took 15.67 s here (16.05 s in run 2) versus about 15.3 s noted in the round record: about 2 to 5 percent above, from one full-suite measurement, within the noise of a loaded machine but not proven equal. |

Findings closed by this round: C01-R1 maps to :878/:900; C01-R2 to the passkey tests plus the unchanged :770 scan; C01-R3 to :574; C01-R4 is record-only.

## (b) Red-first

Source: red/RED.md. Pre-fix snapshot hash recorded by the coders: sha256(git diff HEAD -- src) = 769c5a2cf803a2605c8f00da53bcfa7d7cff14545db995927be51f6c68e058ab (HEAD caaf0396; tracked files only). Build command: `cmake --build build/rewrite-debug --target planar_tests` (raw red/raw/build.log). Per-test command: `ctest --test-dir build/rewrite-debug -R '^replacement\.<escaped name>$' --no-tests=error --output-on-failure`.

| Test | Failing line (red snapshot, per RED.md) | Real red? | Log |
|---|---|---|---|
| X06 C C02 complete_native_candidate requires the passkey | 986 | real, failed assertion | red/raw/t1.log |
| X06 C C02 seam key has no public constructor | 977, 978 | real | red/raw/t2.log |
| X06 C C02 candidate_values requires the passkey | 994 | real | red/raw/t3.log |
| X06 C C02 native_candidate callables and constructor require the passkey | 998, 999, 1000, 1004 | real | red/raw/t4.log |
| X06 C C02 passkey grants are confined to the adapters and the test probe | 1023 | real | red/raw/t5.log |
| X06 C profile completion stores the derived trees and values from profile_acceptance_data | none | guard, already passed | red/raw/t6.log |
| X06 C C01 seam interns the registry tree with the provenance limits (C14) | none | guard, already passed; no red-first exists for C14, the kill depends on the prober | red/raw/t7.log |
| X06 C no Lua runs while the profile write transaction is open | none | guard, already passed (comment-only) | red/raw/t8.log |
| X06 C C01 seam is unreachable from a plain acceptance importer | none | guard, already passed | red/raw/t9.log |
| X06 C seam interns an effective stdlib closure beyond the original tree limits | none | guard, already passed | red/raw/t10.log |

Caveats: RED.md line numbers describe the red snapshot, before the Doxygen fix (+3) and the C14 repair (+2); current starts are 981, 989, 996, 1002, 1013. I did not re-run the red snapshot. Its hash covers tracked files only and predates both fixes, so it is not the current candidate. RED.md calls the reds failed assertions, which I take as stated (behavioural, not compile failures) without re-verifying. The C14 repair itself (fixture change) has no recorded red-first command.

## (c) Deliberate behaviour differences

None found in the evidence. The passkey is an operator-decided product addition (round record, operator decision of 2026-10-26), not a diagnostic change.

## Limitations

- The negative-compile probe, C14 re-probe and X27 probe results were not verified by this gate.
- No build that does work was observed in run 3.
- ctest prints no skip count in this version; none were seen in the log.
- #531 timing is a single full-suite measurement, not isolated.
- Combined hash method differs from run 2's, see identity section.
