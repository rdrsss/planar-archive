# V5-02-X07 gate report (gate runner, run 2 after gate repair, 2026-10-28)

## Verdict

NOT PASSED, one failure. Everything else passes. RelWithDebInfo full ctest: 1208 of 1209 passed; x07.two_process_cas_one_winner fails (exit 8). It failed again when re-run alone and serially (non-gating diagnostic, raw/diag-release-cas-rerun.log): the two racing loads produced a loser that exited 4 with code io, message "Explicit ledger recovery is required", instead of exit 3 revision_conflict. It passes in the Debug configuration (full run and alone). The repair record (docs/vernacular/rounds/V5-02-X07.md, Gate repair) names this test as a known race-prone limit under load, but here it failed serially in release, so it is not only a parallel-load effect. Debug G1, G2, G3, G5 to G7 and the redaction gate all pass. No source file was edited by the gate runner. Run 1 (before the repair) is archived under raw/run1-prerepair/ (logs, REPORT-run1.md, scripts); nothing was deleted.

Deviation from the specified order: an extra build (G0, cmake --build build/rewrite-debug --target planar_tests, exit 0) ran before G1, so G1 exercised the candidate binary instead of a stale one. The repaired CMake now makes planar_tests depend on planar.

## Candidate identity (identity.py output, verbatim, raw/identity-final.txt)

| identity | value |
|---|---|
| HEAD | `7a81b0bcb8c85c3d8f75a6b9ea1dc9a1c66f19a4` |
| sha256(git diff --binary HEAD -- src tests/native docs scripts tests/acceptance CMakeLists.txt), tracked only | `6bacc9985407419f3a07313afa93718ce1e00dbadeb09a15ad2090a8ffb28cc1` |
| combined sha256 (diff bytes, then each untracked path NUL bytes, sorted), pathspec src tests/native docs scripts tests/acceptance CMakeLists.txt, evidence excluded | `20e711dbcb4a3bcaf057a9144028bafbce5f6751cd17645c09d052c2cb939d7a` |

| file | state | sha256 |
|---|---|---|
| `docs/architecture.md` | modified | `8fc636308f9289532f9073877610041352b5105e9925e8e610d0d3725731217c` |
| `docs/cli-reference.md` | modified | `04a7f05089052b5cd2890790e56c32658a852b90ac2feda52d6877caeedeb6fc` |
| `docs/testing.md` | modified | `74820d1c1389f93a1ea1f97ad5e579ffa906608ef2dbfaf66c12d68549ef2d54` |
| `src/cmd/planar/CMakeLists.txt` | modified | `8d20e64bc86ffc28c8fb1e1cb65ab097b711ffce69a6f3ae20f4a55bd80422b8` |
| `src/cmd/planar/bootstrap.cpp` | modified | `d380b76389870792db8c59c2b7c2219436555b55ea8686d006fcb2c36fd1ac8d` |
| `src/cmd/planar/bootstrap.t.cpp` | modified | `0029403b30bc72ad659a5251bbc26effdd2d45d274027089d4ba307d80bfaf18` |
| `src/cmd/planar/handlers/export/command.cpp` | modified | `1272a5b49433295ea7f295e769eeb147b117999f89b48c0e3ed9303ada6b4671` |
| `src/cmd/planar/handlers/export/command.cppm` | modified | `d3c69c9ef36d12f538ff0bfcbe3832432bb791921b24b1cfdfc32ed85a9f1870` |
| `src/cmd/planar/handlers/history/command.cpp` | modified | `236e3a4549f5cf5608f4f36d771ec0eef8d72f3112e3a0483ca5c7b4670933bd` |
| `src/cmd/planar/handlers/history/command.cppm` | modified | `93ddab95073d46d63a08aec2555d89de163979b463b79788f891cadce9135ffc` |
| `src/cmd/planar/handlers/init/command.cpp` | modified | `c0ed435204edd73180b6f51ba66f4827a15b74815138b456a3fe2372e9340c1b` |
| `src/cmd/planar/handlers/init/command.cppm` | modified | `8e98af5224a708e922e79c2dfb9841403ca3fd4b618c66100d4642e781cac739` |
| `src/cmd/planar/handlers/load/command.cpp` | modified | `43b322e0a8d172407cb924dda9aff9bbc3f9637a7a027994858a2ff811b794fb` |
| `src/cmd/planar/handlers/load/command.cppm` | modified | `31a0ee75359dc047889c4ea9ab971aba4c390bea92925a446de998ea06e8afae` |
| `src/cmd/planar/handlers/render/command.cpp` | modified | `7102bcca81a1c17e34aa9a55682b4aa4dc72b9feff98ebd4ed478f2c6a101c30` |
| `src/cmd/planar/handlers/render/command.cppm` | modified | `2c73b72f905e5c2a947efafb62408f8d5533dd28dd30277ffd59a8e45159fe65` |
| `src/cmd/planar/invocation.cpp` | modified | `47aade4bddd05425d5a051150664cd3a8d33090c409bb3fd5b9b6f72c37805e6` |
| `src/cmd/planar/invocation.cppm` | modified | `78d96ceef69355f1889ac458fc315f02a5ba805a095b2550d29153c87c486453` |
| `src/cmd/planar/response.cpp` | modified | `33b900c1fb7489dc30e9fca9b9a6dc7054c804c5fae6c47b2f2ca59e385331ee` |
| `src/cmd/planar/response.cppm` | modified | `245fd9accce5551bfc496cb81ba2b79f1dd4b3978b54a33d54c53509129ef1d8` |
| `src/engine/ledger/acceptance.t.cpp` | modified | `94adb99c2677551047302f89be22516b46c4352ce21023df814261e217198aba` |
| `src/engine/runtime/external_registration.t.cpp` | modified | `7c582f031bb200668030ef32948a9fb35517c9f56ff8209aea92765e468f2041` |
| `src/engine/runtime/runtime.cpp` | modified | `e0b6404c58ce85e40909f9e7de89506544b36e245581419a0678acb65d99842b` |
| `src/engine/runtime/runtime.cppm` | modified | `2750777349030c3608e21ba405c82fff96827b8b14d18699d3965400bb64b991` |
| `tests/acceptance/bootstrap_init.py` | modified | `fb5e65631869a7ae39df9bc309c6b25374139917c8030d2dfeb928e51875787d` |
| `tests/acceptance/bootstrap_load.py` | modified | `48391318bed6526ec09a1c5e7431641db597c85ec460c4ba11d7801d4e40d22a` |
| `tests/native/r14-d7-b-support/generate.py` | modified | `418125151a88e18025a2505ede1198686911bc1f49cd442f84e0347ed895d5d7` |
| `tests/native/v5-02-b2-support/generate.py` | modified | `13b19ac63ab12101e9f75267738db95d96b569858d9b3cbf314110532449334a` |
| `docs/vernacular/execution-design-review.md` | untracked | `587dc38b5992a77dd5fe9adb6af202aa5e6ef22486cd930d33715bbecc1d42e6` |
| `docs/vernacular/execution-design.md` | untracked | `fd2cf72a234e46406b087452b2bf595f139ed231b1655589824d98580caef93a` |
| `docs/vernacular/rounds/V5-02-X07.md` | untracked | `0c614dd018fd8ae5894d3b3a807b90c047d625cb249c4bd05620a1e68bc17605` |
| `src/cmd/planar/envelope.cpp` | untracked | `a2834e1024406ff9ed9b1f3c689f8a79fbfcd2102dfb357af57a9e838e7bc325` |
| `src/cmd/planar/envelope.cppm` | untracked | `8ada4ad6ae4c3ec9b4371127275da10fd193d5917baf909add1230bafdf3ba76` |
| `src/cmd/planar/handlers/extension/command.cpp` | untracked | `5f931e66a318d7bac7365fc1321486abf86c47219d0e641aeb3503c40c2840be` |
| `src/cmd/planar/handlers/extension/command.cppm` | untracked | `5c5c2f72b0c4cd527f99bc34ff1bba07f99588453ca50c09df0672b98b6466ea` |
| `tests/acceptance/x07_replay.py` | untracked | `b951cf0f78361b5bb0754644dd9b9b41dc6b99100f799e42c101edeb1cc15bef` |
| `tests/acceptance/x07_services.py` | untracked | `7aa04ba4ead5ca30c62f51b5f438950e53eb767377907eb7e4ae503d1cd1a276` |
| `tests/acceptance/x07_substitution.py` | untracked | `d6009d9954a891e31dd247f301e4de8e0fdff7212856c49cced21edbfe89a216` |
| `tests/acceptance/x07_workflow.py` | untracked | `bd4ce189a18e3ae8a87eceb917a814214cfb302372b4a7c5f07b939d601b206b` |
| `tests/native/v5-02-x07-support/generate.py` | untracked | `66ee281d909b2288b4eac983df05d551dc4023fc292c4fb3a5a230ac1564dc53` |

The redaction step changed 0 files ("redacted 0 files"), so the identity is the same before and after the gates (raw/identity-before-redact.txt used the tool's default pathspec src tests/native).
Note: the table above lists each hash once; docs/vernacular/rounds/V5-02-X07.md changed between run 1 and run 2 only by the repair record.

## Gate order and results (script raw/run-gates2.sh; each command NAME.log and NAME.rc; nothing piped)

| # | Command | Exit | Result |
|---|---|---|---|
| G0 | cmake --build build/rewrite-debug --target planar_tests (raw/g0-prebuild-debug) | 0 | PASS (added, see above) |
| G1 | ctest --test-dir build/rewrite-debug -R 'X07\|X06\|x07\|x06' --no-tests=error (raw/g1-focused) | 0 | 260 matched, 100% passed, 0 failed, 0 skipped or not-run lines (unit 241, acceptance 19). PASS |
| G2 | cmake --build build/rewrite-debug --target planar_tests (raw/g2-build-debug) | 0 | PASS |
| G3 | ctest --test-dir build/rewrite-debug --no-tests=error (raw/g3-ctest-debug) | 0 | 1209 matched, 100% passed, 0 failed, 0 skipped. PASS |
| G4a | cmake --preset release (raw/g4a-preset-release) | 0 | PASS |
| G4b | cmake --build build/rewrite-release --target planar_tests (raw/g4b-build-release) | 0 | PASS |
| G4c | ctest --test-dir build/rewrite-release --no-tests=error (raw/g4c-ctest-release) | 8 | 1209 matched, 1208 passed, 1 failed (#1201 x07.two_process_cas_one_winner), 0 skipped. FAIL |
| G5 | ctest -N in both configurations (raw/g5a-inv-debug, raw/g5b-inv-release; names-*.txt) | 0, 0 | 1209 names in both, name lists identical (diff empty); all 1114 baseline names preserved, none removed or renamed; 95 new. PASS |
| G6 | python3 scripts/rewrite-check.py lint --build-dir build/rewrite-debug (raw/g6-lint) | 0 | clang-format --Werror and Doxygen: "Formatting and Doxygen checked 147 active source files". PASS |
| G7a | cmake --build build/rewrite-debug (raw/g7a-build1) | 0 | "ninja: no work to do." (already built by G0 and G2). PASS |
| G7b | cmake --build build/rewrite-debug again (raw/g7b-build2) | 0 | "ninja: no work to do." (zero steps). PASS |
| G8a | redact-local-paths.py redact --repo-path "$PWD" --home "$HOME" FILES (raw/g8a-redact) | 0 | "redacted 0 files" |
| G8b | same, check FILES (raw/g8b-check; 40 files in raw/g8-files.txt, from git status, evidence and build output excluded) | 0 | no output. PASS |
| G8c | check with no file arguments (raw/g8c-check-nofiles) | 1 | reports local paths in ignored build output and gitignored evidence (for example build/rewrite-debug/CMakeCache.txt, .claude/settings.local.json). Not meaningful without files; reported as required. |

Order run: G0, G1, G2, G3, G4a, G4b, G4c, G5 (debug, release), G6, G7a, G7b, G8. Every command ran regardless of earlier failures.

Note on G7: the first build check had no work to do because G0, G2 and the release steps had already built the debug tree. A separate successful build preceded both checks, so the zero-work result is valid: both reported zero steps.

Skips: none reported in any log (grep for Skipped, Not Run, Disabled found nothing).

## #531 timing (acceptance criterion 4)

Test #531 (replacement.X06 B3 A owner moves preserve address and counters and moved-from owners refuse) passed in 0.09 sec Debug and 0.02 sec RelWithDebInfo. No new test joined #531 (all 95 new names are numbered after the baseline set; see new-names.txt). The pre-round time was not measured here, so "not slowed" is unverified beyond these absolute values.

## Inventory (G5)

Baseline: tests/native/v5-02-x07-doc-evidence/raw/ctest-N.log (ctest -N at the X07-DOC commit, 1114 names; the committed evidence directories keep no name list in git, this is the most recent on-disk ctest -N, extracted to raw/names-baseline-x07-doc.txt). New inventory: tests/native/v5-02-x07-evidence/inventory.txt (ctest -N names in ctest order, 1209 names). Removed or renamed names: none. New names: 95, listed sorted in tests/native/v5-02-x07-evidence/new-names.txt, identical in Debug and RelWithDebInfo. D7-11 assertion changes: tests/acceptance/bootstrap_init.py, bootstrap_load.py and the generators tests/native/r14-d7-b-support/generate.py and tests/native/v5-02-b2-support/generate.py are modified (clause 24 init format2, clause 25 v1 embedded request bytes) with names kept; the per-assertion records were not in the evidence I read, so that part is not independently verified here. Also modified: src/engine/ledger/acceptance.t.cpp (X06 D public-surface text now lists the X07 factory and run_known, per the repair record) and src/engine/runtime/external_registration.t.cpp.

## Acceptance criteria

| Criterion | Evidence |
|---|---|
| 1 Clauses 1 to 32 each have a row, mutant and killing test, new tests red-first | Table (a) maps clauses 1 to 30 to tests; clause 31 (red runner log) and 32 (docs) are process rows. All mapped tests pass in Debug (G3) and all but x07.two_process_cas_one_winner in RelWithDebInfo (G4c). Mutants are applied only by the prober, not run here. Not claimed satisfied while G4c fails. |
| 2 All 1114 names preserved, assertion changes only on D7-11 list | Names: PASS (G5). D7-11 per-assertion records: not verified (see Inventory). |
| 3 Source-scan proves one profile-VM creation site, exported invoke refuses profile owners | X07 src cmd contains no profile VM creation site, X07 runtime has exactly one profile VM creation call, X07 exported command_vm invoke still refuses profile owners: passed in G1 and G3 |
| 4 Count rises by new tests identically in both configs, #531 not slowed | 1114 to 1209 in both (G5); #531 times above |
| 5 Docs updated, root help bytes unchanged | X07 root help bytes are unchanged and omit with-profile passed (G3). docs/architecture.md, cli-reference.md and testing.md are modified; lint and redaction check passed. Doc content not judged here. |

## (a) Table of invariants to test names

Clause column is the design's X07 clause. File:line is the TEST_CASE (or case branch) location found by name search at the frozen candidate; "Red outcome" is from the coders' red run (red/table.md), not from the gates. Rows I100 to I104 and I108 to I121 are process or evidence rows and not ctest tests.

| ID | Clause | Test | File:line | Red outcome | First failing line in the red run |
|---|---|---|---|---|---|
| I01 | 1 | X07 production envelope rows refused by syntax and prefix usage never access the profile | src/cmd/planar/bootstrap.t.cpp:1200 | failed-assertion rc8 | src/cmd/planar/bootstrap.t.cpp:1093 |
| I02 | 1 | X07 envelope and bootstrap source has no manual argv scan | src/cmd/planar/bootstrap.t.cpp:1302 | passed rc0 |  |
| I03 | 2 | X07 production envelope rows refused by syntax and prefix usage never access the profile | src/cmd/planar/bootstrap.t.cpp:1200 | failed-assertion rc8 | src/cmd/planar/bootstrap.t.cpp:1093 |
| I04 | 2 | X07 production envelope rows refused by syntax and prefix usage never access the profile | src/cmd/planar/bootstrap.t.cpp:1200 | failed-assertion rc8 | src/cmd/planar/bootstrap.t.cpp:1093 |
| I05 | 2 | X07 selector with invalid UTF-8 or NUL is prefix usage before profile access | src/cmd/planar/bootstrap.t.cpp:1313 | failed-assertion rc8 | src/cmd/planar/bootstrap.t.cpp:1093 |
| I06 | 2 | X07 prefix count is three or four and dot slash help is a valid selector | src/cmd/planar/bootstrap.t.cpp:1331 | failed-assertion rc8 | src/cmd/planar/bootstrap.t.cpp:998 |
| I07 | 2 | X07 production selected help rows capture and register the profile | src/cmd/planar/bootstrap.t.cpp:1233 | failed-assertion rc8 | src/cmd/planar/bootstrap.t.cpp:998 |
| I08 | 2 | X07 production envelope rows refused by syntax and prefix usage never access the profile | src/cmd/planar/bootstrap.t.cpp:1200 | failed-assertion rc8 | src/cmd/planar/bootstrap.t.cpp:1093 |
| I09 | 3 | X07 production opaque probe rows refuse or admit at their step | src/cmd/planar/bootstrap.t.cpp:1245 | failed-assertion rc8 | src/cmd/planar/bootstrap.t.cpp:1093 |
| I10 | 3 | X07 production opaque probe rows refuse or admit at their step | src/cmd/planar/bootstrap.t.cpp:1245 | failed-assertion rc8 | src/cmd/planar/bootstrap.t.cpp:1093 |
| I11 | 3 | X07 production envelope rows refused by the probe never access the profile | src/cmd/planar/bootstrap.t.cpp:1205 | failed-assertion rc8 | src/cmd/planar/bootstrap.t.cpp:1093 |
| I12 | 3 | X07 production envelope rows refused by the probe never access the profile | src/cmd/planar/bootstrap.t.cpp:1205 | failed-assertion rc8 | src/cmd/planar/bootstrap.t.cpp:1093 |
| I13 | 3 | X07 production matrices enumerate every X01 row | src/cmd/planar/bootstrap.t.cpp:1281 | passed rc0 |  |
| I14 | 3 | X07 production envelope rows refused by syntax and prefix usage never access the profile | src/cmd/planar/bootstrap.t.cpp:1200 | failed-assertion rc8 | src/cmd/planar/bootstrap.t.cpp:1093 |
| I15 | 4 | X07 profile invocation reads cwd and environment once | src/cmd/planar/bootstrap.t.cpp:1460 | failed-assertion rc8 | src/cmd/planar/bootstrap.t.cpp:1472 |
| I16 | 4 | X07 profile selector normalizes lexically without existence | src/cmd/planar/bootstrap.t.cpp:1505 | failed-assertion rc8 | src/cmd/planar/bootstrap.t.cpp:1519 |
| I17 | 4 | X07 profile selector honours the path ceiling and the request ceiling wins | src/cmd/planar/bootstrap.t.cpp:1535 | failed-assertion rc8 | src/cmd/planar/bootstrap.t.cpp:1548 |
| I18 | 5 | X07 profile load replays a saved request before touching the profile | src/cmd/planar/bootstrap.t.cpp:1581 | failed-assertion rc8 | src/cmd/planar/bootstrap.t.cpp:1586 |
| I19 | 5 | X07 profile load with a different body conflicts before profile access | src/cmd/planar/bootstrap.t.cpp:1592 | failed-assertion rc8 | src/cmd/planar/bootstrap.t.cpp:998 |
| I20 | 5 | X07 profile load on a format1 ledger refuses before profile access | src/cmd/planar/bootstrap.t.cpp:1605 | failed-assertion rc8 | src/cmd/planar/bootstrap.t.cpp:998 |
| I21 | 5 | X07 early lookup scope closes before profile capture | src/cmd/planar/bootstrap.t.cpp:1618 | passed rc0 |  |
| I22 | 6 | X07 render known slots refuse overlap before registration | src/cmd/planar/bootstrap.t.cpp:1678 | failed-assertion rc8 | src/cmd/planar/bootstrap.t.cpp:998 |
| I23 | 6 | X07 init known slot is alias only and runs before registration | src/cmd/planar/bootstrap.t.cpp:1719 | failed-assertion rc8 | src/cmd/planar/bootstrap.t.cpp:998 |
| I24 | 6 | X07 history known slot refuses a ledger alias before registration | src/cmd/planar/bootstrap.t.cpp:1739 | failed-assertion rc8 | src/cmd/planar/bootstrap.t.cpp:998 |
| I25 | 6 | X07 export known slots refuse before registration | src/cmd/planar/bootstrap.t.cpp:1750 | failed-assertion rc8 | src/cmd/planar/bootstrap.t.cpp:998 |
| I26 | 6 | X07 load known slots refuse before registration | src/cmd/planar/bootstrap.t.cpp:1773 | failed-assertion rc8 | src/cmd/planar/bootstrap.t.cpp:998 |
| I27 | 6 | X07 production normalization and known slot rows refuse at their step | src/cmd/planar/bootstrap.t.cpp:1218 | failed-assertion rc8 | src/cmd/planar/bootstrap.t.cpp:998 |
| I28 | 6 | X07 production normalization and known slot rows refuse at their step | src/cmd/planar/bootstrap.t.cpp:1218 | failed-assertion rc8 | src/cmd/planar/bootstrap.t.cpp:998 |
| I29 | 7 | X07 production envelope rows refused after capture and registration are final usage | src/cmd/planar/bootstrap.t.cpp:1210 | failed-assertion rc8 | src/cmd/planar/bootstrap.t.cpp:998 |
| I30 | 7 | X07 production envelope rows refused after capture and registration are final usage | src/cmd/planar/bootstrap.t.cpp:1210 | failed-assertion rc8 | src/cmd/planar/bootstrap.t.cpp:998 |
| I31 | 8 | X07 production envelope rows refused after capture and registration are final usage | src/cmd/planar/bootstrap.t.cpp:1210 | failed-assertion rc8 | src/cmd/planar/bootstrap.t.cpp:998 |
| I32 | 8 | X07 shipped tail under a profile keeps probe option state and scalar command words stay values | src/cmd/planar/bootstrap.t.cpp:1810 | failed-assertion rc8 | src/cmd/planar/bootstrap.t.cpp:998 |
| I33 | 9 | X07 profile owner is consumed by its single dispatch | src/engine/runtime/external_registration.t.cpp:4800 | failed-assertion rc8 | src/engine/runtime/external_registration.t.cpp:4803 |
| I34 | 10 | X07 adapter maps every registration refusal to its closed code message and exit | src/cmd/planar/bootstrap.t.cpp:1982 | failed-assertion rc8 | src/cmd/planar/bootstrap.t.cpp:1979 |
| I35 | 10 | X07 adapter never prints fields host paths or tracebacks | src/cmd/planar/bootstrap.t.cpp:1992 | failed-assertion rc8 | src/cmd/planar/bootstrap.t.cpp:998 |
| I36 | 10 | X07 extension preparation usage prints the public usage bytes | src/cmd/planar/bootstrap.t.cpp:2014 | failed-assertion rc8 | src/cmd/planar/bootstrap.t.cpp:998 |
| I37 | 10 | X07 profile preparation semantic error exits by code | src/cmd/planar/bootstrap.t.cpp:2034 | failed-assertion rc8 | src/cmd/planar/bootstrap.t.cpp:2049 |
| I38 | 10 | X07 adapter redacts malformed trusted payloads as internal protocol | src/cmd/planar/bootstrap.t.cpp:2062 | failed-assertion rc8 | src/cmd/planar/bootstrap.t.cpp:2074 |
| I39 | 10/acc4 | X07 command sources add no diagnostic outside the closed table | src/cmd/planar/bootstrap.t.cpp:2094 | failed-assertion rc8 | src/cmd/planar/bootstrap.t.cpp:2099 |
| I40 | 11 | X07 selected help bytes are exact sorted and escaped | src/cmd/planar/bootstrap.t.cpp:2129 | failed-assertion rc8 | src/cmd/planar/bootstrap.t.cpp:2139 |
| I41 | 11 | X07 production selected help rows capture and register the profile | src/cmd/planar/bootstrap.t.cpp:1233 | failed-assertion rc8 | src/cmd/planar/bootstrap.t.cpp:998 |
| I42 | 11 | X07 selected help refuses above the help ceiling | src/cmd/planar/bootstrap.t.cpp:2155 | failed-assertion rc8 | src/cmd/planar/bootstrap.t.cpp:2152 |
| I43 | 11/acc4 | X07 root help bytes are unchanged and omit with-profile | src/cmd/planar/bootstrap.t.cpp:1355 | passed rc0 |  |
| I44 | 12 | X07 closed factory keeps the original receipts through dispatch | src/engine/runtime/external_registration.t.cpp:4815 | failed-assertion rc8 | src/engine/runtime/external_registration.t.cpp:4818 |
| I45 | 12/acc3 | X07 src cmd contains no profile VM creation site | src/cmd/planar/bootstrap.t.cpp:2172 | passed rc0 |  |
| I46 | 12/acc3 | X07 runtime has exactly one profile VM creation call | src/engine/runtime/external_registration.t.cpp:4828 | passed rc0 |  |
| I47 | 12/acc3 | X07 exported command_vm invoke still refuses profile owners | src/engine/runtime/external_registration.t.cpp:4841 | passed rc0 |  |
| I48 | 13 | X07 closed factory registers exactly once for every family | src/engine/runtime/external_registration.t.cpp:4850 | failed-assertion rc8 | src/engine/runtime/external_registration.t.cpp:4863 |
| I49 | 12 | X07 selector overload refuses the load alternative before capture | src/engine/runtime/external_registration.t.cpp:4880 | failed-assertion rc8 | src/engine/runtime/external_registration.t.cpp:4884 |
| I50 | 27 | X07 history and export under a profile validate the profile before dispatch | src/cmd/planar/bootstrap.t.cpp:2185 | failed-assertion rc8 | src/cmd/planar/bootstrap.t.cpp:2188 |
| I51 | 27 | X07 history and export responses are identical with and without a profile | src/cmd/planar/bootstrap.t.cpp:2206 | failed-assertion rc8 | src/cmd/planar/bootstrap.t.cpp:2204 |
| I52 | 27 | X07 export refuses a target overlapping the trusted root before publication | src/cmd/planar/bootstrap.t.cpp:2230 | failed-assertion rc8 | src/cmd/planar/bootstrap.t.cpp:998 |
| I53 | 28 | X07 ledger aliasing the profile or a module refuses for history export and load | src/cmd/planar/bootstrap.t.cpp:2244 | failed-assertion rc8 | src/cmd/planar/bootstrap.t.cpp:998 |
| I60 | 14 | X07 history_id vectors report Lua type and presence through the real CLI | src/cmd/planar/bootstrap.t.cpp:2398 | failed-assertion rc8 | src/cmd/planar/bootstrap.t.cpp:998 |
| I61 | 14 | X07 history_id invalid spellings refuse before the handler | src/cmd/planar/bootstrap.t.cpp:2413 | failed-assertion rc8 | src/cmd/planar/bootstrap.t.cpp:998 |
| I62 | 14 | X07 history_id selector conflicts are usage regardless of magnitude | src/cmd/planar/bootstrap.t.cpp:2432 | failed-assertion rc8 | src/cmd/planar/bootstrap.t.cpp:998 |
| I63 | 14 | X07 history_id after-event with limit pages through the real CLI | src/cmd/planar/bootstrap.t.cpp:2456 | failed-assertion rc8 | src/cmd/planar/bootstrap.t.cpp:998 |
| I64 | 15 | X07 decimal and limit boundary vectors through the real CLI | src/cmd/planar/bootstrap.t.cpp:2466 | failed-assertion rc8 | src/cmd/planar/bootstrap.t.cpp:998 |
| I65 | 16 | X07 history_id misuse is refused at registration through the real CLI | src/cmd/planar/bootstrap.t.cpp:2489 | failed-assertion rc8 | src/cmd/planar/bootstrap.t.cpp:998 |
| I66 | 17 | X07 role read overlaps and aliases refuse before the handler runs | src/cmd/planar/bootstrap.t.cpp:2529 | failed-assertion rc8 | src/cmd/planar/bootstrap.t.cpp:2527 |
| I67 | 17 | x07.late_receipt_survives_to_seal | tests/acceptance/x07_services.py:28 | failed-assertion rc8 | AssertionError: CompletedProcess(args=['build/rewrite- |
| I68 | 18 | X07 extension responses are the closed extension_v1 and match retained services | src/cmd/planar/bootstrap.t.cpp:2655 | failed-assertion rc8 | src/cmd/planar/bootstrap.t.cpp:998 |
| I69 | 18 | X07 projection omission vector reproduces 87 94 and 59 bytes through the CLI | src/cmd/planar/bootstrap.t.cpp:2692 | failed-assertion rc8 | src/cmd/planar/bootstrap.t.cpp:2695 |
| I70 | 19 | x07.workflow_shape_load | tests/acceptance/x07_workflow.py:42 | failed-assertion rc8 | AssertionError: (['with-profile', '--stdlib-profile', '../trusted/p.json', 'load', '--ledger', |
| I71 | 19 | x07.workflow_render_external | tests/acceptance/x07_workflow.py:54 | failed-assertion rc8 | AssertionError: (['with-profile', '--stdlib-profile', '../trusted/p.json', 'render', '--worksp |
| I72 | 19 | x07.workflow_verb_history_export | tests/acceptance/x07_workflow.py:62 | failed-assertion rc8 | AssertionError: (['with-profile', '--stdlib-profile', '../trusted/p.json', 'histp', '--ledger' |
| I73 | 20 | x07.replay_after_inputs_deleted | tests/acceptance/x07_replay.py:67 | failed-assertion rc8 | AssertionError: CompletedProcess(args=['build/rewrite- |
| I74 | 20 | x07.replay_conflict_matrix | tests/acceptance/x07_replay.py:75 | failed-assertion rc8 | AssertionError: CompletedProcess(args=['build/rewrite- |
| I75 | 20 | x07.replay_same_selector_changed_bytes_new_provenance | tests/acceptance/x07_replay.py:84 | failed-assertion rc8 | AssertionError |
| I76 | 21 | x07.relookup_equal_replay_before_stale_cas | tests/acceptance/x07_replay.py:95 | failed-assertion rc8 | AssertionError: ('AFTER_EARLY_MISS', 2, (b'', b'{"error":{"code":"usage","field":"","message": |
| I77 | 21 | x07.relookup_unequal_conflicts | tests/acceptance/x07_replay.py:95 | failed-assertion rc8 | AssertionError: ('AFTER_EARLY_MISS', 2, (b'', b'{"error":{"code":"usage","field":"","message": |
| I78 | 21 | x07.two_process_cas_one_winner | tests/acceptance/x07_replay.py:107 | failed-assertion rc8 | AssertionError: (0, [(2, b'', b'{"error":{"code":"usage","field":"","message":"Expected comman |
| I79 | 22 | x07.receipt_races_refuse_before_authorization | tests/acceptance/x07_replay.py:115 | failed-assertion rc8 | AssertionError: ('AFTER_FACTORY_OPEN', 2, (b'', b'{"error":{"code":"usage","field":"","message |
| I80 | 22 | x07.final_race_rolls_back_before_commit | tests/acceptance/x07_replay.py:115 | failed-assertion rc8 | AssertionError: ('AFTER_PREPARE_LOAD', 2, (b'', b'{"error":{"code":"usage","field":"","message |
| I81 | 22 | X06 C no Lua runs while the profile write transaction is open | src/engine/ledger/profile_acceptance.t.cpp:561 | passed rc0 |  |
| I82 | 23 | x07.post_commit_allocation_failure_keeps_receipt | tests/acceptance/x07_replay.py:130 | failed-assertion rc8 | AssertionError: CompletedProcess(args=['build/rewrite- |
| I83 | 23 | x07.post_commit_encoding_failure_keeps_receipt | tests/acceptance/x07_replay.py:137 | failed-assertion rc8 | AssertionError: CompletedProcess(args=['build/rewrite- |
| I84 | 23 | x07.post_commit_stdout_failure_keeps_receipt | tests/acceptance/x07_replay.py:142 | failed-assertion rc8 | AssertionError: CompletedProcess(args=['build/rewrite- |
| I85 | 23 | replacement.load_cli | tests/acceptance/bootstrap_load.py (whole script, ctest-registered) | failed-assertion rc8 | AssertionError |
| I86 | 24 | X07 public init creates a format2 ledger with unchanged response | src/cmd/planar/bootstrap.t.cpp:2763 | failed-assertion rc8 | src/cmd/planar/bootstrap.t.cpp:2762 |
| I87 | 24 | X07 init refuses a format selector option | src/cmd/planar/bootstrap.t.cpp:2777 | passed rc0 |  |
| I88 | 24 | X07 native format1 initializer remains available to fixtures | src/cmd/planar/bootstrap.t.cpp:2785 | passed rc0 |  |
| I89 | 25 | X07 embedded load on format2 frames request_v2 with embedded selector | src/cmd/planar/bootstrap.t.cpp:2819 | failed-assertion rc8 | src/cmd/planar/bootstrap.t.cpp:2814 |
| I90 | 25 | X07 embedded load on format1 keeps request_v1 | src/cmd/planar/bootstrap.t.cpp:2827 | failed-assertion rc8 | src/cmd/planar/bootstrap.t.cpp:2830 |
| I91 | 25 | X07 embedded format2 load keeps v1 interpretation and versions | src/cmd/planar/bootstrap.t.cpp:2842 | failed-assertion rc8 | src/cmd/planar/bootstrap.t.cpp:2840 |
| I92 | 26 | X07 profile load records request_v2 selector effective stdlib and v2 interpretation | src/cmd/planar/bootstrap.t.cpp:2861 | failed-assertion rc8 | src/cmd/planar/bootstrap.t.cpp:2858 |
| I93 | 29 | X07 extension and profile rows print one JSON line and nothing during registration | src/cmd/planar/bootstrap.t.cpp:2741 | failed-assertion rc8 | src/cmd/planar/bootstrap.t.cpp:2738 |
| I94 | 29 | x07.response_ceiling_extension | tests/acceptance/x07_services.py:42 | failed-assertion rc8 | AssertionError: CompletedProcess(args=['build/rewrite- |
| I95 | 30 | X07 N30 late slot admission charges the factory budget owner | src/engine/runtime/external_registration.t.cpp:5048 | passed rc0 |  |
| I96 | 30 | X07 N30 role capture charges the factory budget owner | src/engine/runtime/external_registration.t.cpp:5059 | passed rc0 |  |
| I97 | 30 | X07 N30 history decode charges the factory budget owner | src/engine/runtime/external_registration.t.cpp:5139 | failed-assertion rc8 | src/engine/runtime/external_registration.t.cpp:5130 |
| I98 | 30 | X07 N30 projection transport charges the factory budget owner | src/engine/runtime/external_registration.t.cpp:5074 | passed rc0 |  |
| IM6 | 1-9 | X07 production handler rows succeed | src/cmd/planar/bootstrap.t.cpp:1250 | failed-assertion rc8 | src/cmd/planar/bootstrap.t.cpp:1258 |
| I105 | 7 | X07 final syntax error in an extension tail precedes late slot overlap | src/cmd/planar/bootstrap.t.cpp:2587 | failed-assertion rc8 | src/cmd/planar/bootstrap.t.cpp:998 |
| I106 | 8/12 | X07 run_known refuses options that differ from the admitted options | src/engine/runtime/external_registration.t.cpp:4895 | failed-assertion rc8 | src/engine/runtime/external_registration.t.cpp:4899 |
| I107 | 6/12 | X07 run_known refuses opaque and known load owners | src/engine/runtime/external_registration.t.cpp:4913 | failed-assertion rc8 | src/engine/runtime/external_registration.t.cpp:4930 |
| I111 | 12 | X07 profile load binds the early lookup intent through prepare_load | src/cmd/planar/bootstrap.t.cpp:2886 | failed-assertion rc8 | src/cmd/planar/bootstrap.t.cpp:2882 |
| I112 | 13/12 | X07 cmd sources open the profile owner through one factory call per branch | src/cmd/planar/bootstrap.t.cpp:2260 | failed-assertion rc8 | src/cmd/planar/bootstrap.t.cpp:2260 |
| I113 | 1.5/A-11 | x07.substitute_init | tests/acceptance/x07_substitution.py:8 | failed-assertion rc8 | AssertionError: (2, b'', b'{"error":{"code":"usage","field":"","message":"Expected command opt |
| I114 | 1.5/A-11 | x07.substitute_history | tests/acceptance/x07_substitution.py:8 | failed-assertion rc8 | AssertionError: (2, b'', b'{"error":{"code":"usage","field":"","message":"Expected command opt |
| I115 | 1.5/A-11 | x07.substitute_export | tests/acceptance/x07_substitution.py:8 | failed-assertion rc8 | AssertionError: (2, b'', b'{"error":{"code":"usage","field":"","message":"Expected command opt |
| I116 | acc3 | X06 C C01 seam is unreachable from a plain acceptance importer | src/engine/ledger/acceptance.t.cpp:770 | passed rc0 |  |
| I122 | must-not | X07 root level stdlib-profile flag is usage | src/cmd/planar/bootstrap.t.cpp:1348 | passed rc0 |  |
| I123 | must-not | X07 profile environment variables are ignored without with-profile | src/cmd/planar/bootstrap.t.cpp:2273 | passed rc0 |  |
| I124 | 11 | X07 selected help ceiling is one mebibyte by default and bootstrap passes none | src/cmd/planar/bootstrap.t.cpp:2292 | failed-assertion rc8 | src/cmd/planar/bootstrap.t.cpp:2296 |
| I125 | 10/acc4 | X07 new runtime refusals use only closed table messages | src/engine/runtime/external_registration.t.cpp:4937 | passed rc0 |  |

IM6 (clause 1 to 9, handler rows E01, E34, O01): X07 production handler rows succeed, in src/cmd/planar/bootstrap.t.cpp, failed-assertion at bootstrap.t.cpp:1258 in the red run.

No corrective-round findings exist for this round (it is the first X07 round), so there are no finding rows to add.

Line numbers in table (a) were recomputed at this frozen candidate by name search (the repair coder moved lines in bootstrap.t.cpp); the "Red outcome" columns are from the coders' red run and are unchanged. IM6 (clauses 1 to 9): X07 production handler rows succeed, in src/cmd/planar/bootstrap.t.cpp. No corrective-round findings exist for this round (it is the first X07 round).

## (b) Red-first

Source: tests/native/v5-02-x07-evidence/red/RED.md and red/table.md (per-test raw logs 001 to 096 and build.log in red/raw).
- Pre-fix snapshot: HEAD 7a81b0bcb8c85c3d8f75a6b9ea1dc9a1c66f19a4; sha256(red/snapshot/candidate.diff) = ff619c029b3385ff0ad1c571a53034b6a85dd447770900fd20fd2fca71c2aa03; sha256(red/snapshot/SHA256SUMS) = bc041618e3aeb852b804f86b8040d2a9b1160cbc9e0a0086dbec1e18aad07367; identity.py combined (src tests/native) cd2d73a34a6786222c4f03de67cefbf972d3c82b7b35c4bc2dc61d329be851b1.
- Commands: cmake --build build/rewrite-debug --target planar_tests (rc 0, red/raw/build.log), then per test ctest --test-dir build/rewrite-debug -R '^<escaped full name>$' --no-tests=error --output-on-failure (each log's second line records the exact -R; registered names carry the replacement. prefix; a first attempt without the prefix matched no tests, logs in red/raw-noprefix-attempt).
- Results: 96 distinct names run, 79 real reds on assertions (C++ CHECK or Python AssertionError), none crashed, none unregistered; 17 passed and were all expectRed=false guards (rows marked "passed" in table (a)). Two guards failed at red time because the product was absent (allowed): I65 history_id misuse at registration (the table row for it shows the failing CHECK) and I85 replacement.load_cli.
- Per-test outcome and failing line: the last two columns of table (a), and red/table.md.
- Missing: the red snapshot is a mid-round candidate, not a pure pre-fix tree; red/raw has no per-test command beyond the ctest -R line. Rows I100 to I104 and I108 to I121 have no red run. The repair round's changes (see below) were not red-first: they fixed tests or product after the gates in run 1 and have no separate red snapshot in the evidence I read. No mutation evidence is recorded (mutants belong to the prober).

## (c) Deliberate behaviour differences

- Public init now creates a format2 ledger (clause 24; Q13 A, contract X-STORE rows 1 and 3, addendum D7-6): test X07 public init creates a format2 ledger with unchanged response; bootstrap_init.py assertions changed (D7-11).
- Embedded load on a format2 ledger frames request_v2 with the embedded selector, format1 keeps request_v1 (clause 25; DC17, D7-6, X-STORE): tests X07 embedded load on format2 frames request_v2 with embedded selector, X07 embedded load on format1 keeps request_v1; bootstrap_load.py assertions changed (D7-11).
- with-profile is now accepted (previously usage): the purpose of the round (X-CLI).
- Repair record (V5-02-X07.md, Gate repair): render under a profile with an external shape now returns kind render_external with a shape record in validate_response (external-profile contract render_external_v1); the extension handler's before-result-seal anchor was reordered; the X06 D public-surface text test was updated to list the X07 factory and run_known. Whether each assertion edit in the repair is covered by a contract clause is recorded in that file, not re-verified here.
Diagnostics byte-identity is covered by X07 command sources add no diagnostic outside the closed table, which passes in G3.

## Non-gating Linux probe (Q6 A)

Not run. docker info reports "Cannot connect to the Docker daemon at unix://<home>/.docker/run/docker.sock. Is the docker daemon running?" and no container recipe or Dockerfile exists in the tree (remaining-steps-design.md line 429). I did not start Docker Desktop or write a recipe. Nothing was pulled and nothing pruned, so docker builder prune -f was not needed.

## Limitations

- x07.two_process_cas_one_winner fails in RelWithDebInfo, consistently, even alone; see Verdict. It is a ledger lock-contention outcome (io, exit 4) rather than revision_conflict, which the repair record attributes to ledger behaviour outside the round. Whether that is acceptable or needs the test relaxed is a decision for the coordinator.
- G1 ran after an extra G0 build, not as the first command.
- ctest runs were not captured with --output-on-failure (as specified); the diagnostic rerun is the only failure text.
- Mutants, review and Linux probe are outside this runner. Rows I100 to I104 and I108 to I121 are not mapped to file:line.
