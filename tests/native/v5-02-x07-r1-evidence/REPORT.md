# V5-02-X07-R1 gate report (second gate run)

Gate runner output only; no source file was edited. Raw logs of this run are raw/g2-*; the first gate run's logs (raw/01 to raw/15) and its report (raw/REPORT-r0-superseded.md) are kept. The candidate changed between the two runs (storage.t.cpp gained two tests), so this report supersedes the first one.

## Candidate identity

Output of identity.py (verbatim, raw/g2-01-identity.log):

| identity | value |
|---|---|
| HEAD | `7a81b0bcb8c85c3d8f75a6b9ea1dc9a1c66f19a4` |
| sha256(git diff --binary HEAD -- src tests/native), tracked only | `a96814439fc55317332d89beb73b88f5f28bf46d73c2299ec4d83b3663658f6b` |
| combined sha256 (diff bytes, then untracked paths), pathspec src tests/native, evidence excluded | `0568aea2a156a3bbacbc2eae9c711a527a93fc7cb062fa3b9a539aaf6168870a` |

The only per-file difference from the first run is src/engine/ledger/storage.t.cpp (now sha256 `ce1dea1b61fc8dd80c1123f2582ee15b19cde9269d2aeda0e2a66954b1ba674b`). The whole X07 candidate is uncommitted on top of HEAD.

## Commands and results

| # | Command | Exit | Result |
|---|---|---|---|
| 1 | identity.py | 0 | identity above |
| 2 | cmake --build build/rewrite-debug --target planar_tests | 0 | ninja: no work to do (tree was already current) |
| 3 | ctest build/rewrite-debug --no-tests=error -R 'x07\|X07\|X06' | 0 | 100% passed, 260 of 260, 0 failed, 0 skipped, 210.98 s |
| 4 | ctest build/rewrite-debug --no-tests=error (full) | 0 | 100% passed, 1213 of 1213, 0 failed, 0 skipped, 652.47 s |
| 5 | cmake --preset release | 0 | configured |
| 6 | cmake --build build/rewrite-release --target planar_tests | 0 | built (635 steps) |
| 7 | ctest build/rewrite-release --no-tests=error (full) | 0 | 100% passed, 1213 of 1213, 0 failed, 0 skipped, 305.81 s |
| 8 | python3 scripts/rewrite-check.py lint --build-dir build/rewrite-debug | 0 | clang-format --Werror and Doxygen passed first time: "Formatting and Doxygen checked 147 active source files" (no SIGBUS this run) |
| 9 | cmake --build build/rewrite-debug (first) | 0 | ninja: no work to do |
| 10 | cmake --build build/rewrite-debug (second) | 0 | ninja: no work to do, zero steps |
| 11 | scripts/redact-local-paths.py check --repo-path "$PWD" --home "$HOME" on the 42 explicit files (git diff --name-only HEAD plus untracked, excluding this evidence directory; list raw/g2-13-files.txt) | 0 | no output |
| 11b | same script with no file arguments | 1 | reports local paths only in ignored build output and .claude/settings.local.json (raw/g2-15); not meaningful, reported for completeness |

Skips: none (ctest summaries show no Skipped or Not Run lines). No run matched zero tests. Exit codes were read directly, unpiped, except the Release ctest, which ran under a nohup wrapper that wrote its exit code to raw/g2-07-exit.txt (0).

Zero-work check: commands 9 and 10 both report "no work to do". Command 9 was already a no-op because command 2 and the earlier build left the default targets current, so a "first build did real work, second did none" sequence was not exercised. Zero steps is confirmed on the Debug tree only.

## Inventory

Source: ctest -N for both configurations (raw/g2-11, raw/g2-12). inventory.txt is the Debug listing, inventory-names.txt its sorted names. Debug and Release lists are identical, 1213 names.

Compared against:
- The first gate run's inventory of this round (raw/old-inventory-names-r0-1211.txt, 1211 names, untracked and gitignored): 0 removed, 2 added:
  - replacement.storage begin_write sidecar refusal message is byte exact
  - replacement.storage begin_write surfaces a non-ENOENT sidecar stat failure
- tests/native/v5-02-x07-evidence/inventory.txt (1209 names, earlier gate, untracked): 0 removed, 4 added (the two above plus replacement.storage begin_write waits out a transient live writer journal and replacement.storage begin_write still refuses a persistent journal).
- Pre-X07 baseline tests/native/v5-02-x07-doc-evidence/inventory.txt (1114 names, untracked): all 1114 preserved, 99 added. No committed inventory list exists for X07-DOC; the tracked inventory-preservation.json files in X06 evidence were not used.

## Acceptance traceability

The round record docs/vernacular/rounds/V5-02-X07.md has two parts. Original acceptance criteria 1 to 5 and the Dispatch 2 (sidecar repair) criteria 1 to 6.

| Criterion | Evidence |
|---|---|
| Orig 1: clauses 1 to 32 each with invariant row, mutant and killing test | Clause table below, taken from tests/native/v5-02-x07-evidence/plan/INVARIANTS.md (118 test-bearing rows, I01 to I125). Mutant column lives in that file; mutants have not been applied by this gate (prober's job). Clauses 31 and 32 and some "acc"/"P8"/"CL" rows have no ctest test (they are evidence or docs gates), listed after the table. |
| Orig 2: 1114 names preserved, assertion changes only D7-11 | inventory comparison above (0 removed). D7-11 assertion edits are listed in the round record "Gate repair" section; not independently audited here. |
| Orig 3: source scan proves one profile-VM creation site; exported invoke refuses profile owners | X07 runtime has exactly one profile VM creation call; X07 src cmd contains no profile VM creation site; X07 exported command_vm invoke still refuses profile owners (passed in focused run). |
| Orig 4: count rises identically in Debug and Release; #531 time recorded | 1211 in both, identical names; #531 times above. |
| Orig 5: docs updated, root help bytes unchanged | root help test: X07 root help bytes are unchanged and omit with-profile (passed). docs/architecture.md, cli-reference.md, testing.md are modified in the tree; content not audited by the gate runner. |
| Repair 1: CAS test passes 20 consecutive runs in release and in Debug | x07.two_process_cas_one_winner passed once in each full run (replacement.x07.two_process_cas_one_winner, src/cmd/planar/CMakeLists.txt:232, body tests/acceptance/x07_replay.py:107). The 20-run evidence is NOT in the evidence directory and was not part of this gate; missing. |
| Repair 2: ledger regression test red-first, command and hash recorded | New tests: "storage begin_write waits out a transient live writer journal" (src/engine/ledger/storage.t.cpp:185) and "storage begin_write still refuses a persistent journal" (src/engine/ledger/storage.t.cpp:201). Red-first evidence is MISSING (see red-first section). The brief asked for two connections or processes with a live journal; the test uses a plain file and a remover thread, so it does not exercise a real concurrent writer. |
| Repair 3: genuine-sidecar refusal tests remain, no name removed, 1114 preserved | 0 removed; the persistent-journal test above plus the existing sidecar tests pass. |
| Repair 4: diagnostics byte-identical | The refusal message in no_sidecars is unchanged ("Ledger sidecar requires separately audited recovery"); git diff shows no other message change in storage.cpp. |
| Repair 5: no new public fields or methods | storage.cpp change is inside the anonymous or native helper no_sidecars; no header changed by the repair. The X07 candidate as a whole adds exported factory and run_known per the round design (round record). |
| Repair 6: coders did not mutate; mutant named | Mutant named in the brief: move the sidecar check back before the lock. Not checked by this gate. |

### Clause to test table

Test names are registered in ctest with the prefix "replacement."; x07.* cases are Python acceptance cases registered through src/cmd/planar/CMakeLists.txt.

| Clause | Invariant rows | Test (exact registered name, without the replacement. prefix) | file:line | expectRed |
|---|---|---|---|---|
| 1 | I01 | X07 production envelope rows refused by syntax and prefix usage never access the profile | src/cmd/planar/bootstrap.t.cpp:1200 | true |
| 1 | I02 | X07 envelope and bootstrap source has no manual argv scan | src/cmd/planar/bootstrap.t.cpp:1302 | false |
| 1.5/A-11 | I113 | x07.substitute_init | tests/acceptance/x07_substitution.py:8 | true |
| 1.5/A-11 | I114 | x07.substitute_history | tests/acceptance/x07_substitution.py:8 | true |
| 1.5/A-11 | I115 | x07.substitute_export | tests/acceptance/x07_substitution.py:8 | true |
| 2 | I03,I04,I08 | X07 production envelope rows refused by syntax and prefix usage never access the profile | src/cmd/planar/bootstrap.t.cpp:1200 | true |
| 2 | I05 | X07 selector with invalid UTF-8 or NUL is prefix usage before profile access | src/cmd/planar/bootstrap.t.cpp:1313 | true |
| 2 | I06 | X07 prefix count is three or four and dot slash help is a valid selector | src/cmd/planar/bootstrap.t.cpp:1331 | true |
| 2 | I07 | X07 production selected help rows capture and register the profile | src/cmd/planar/bootstrap.t.cpp:1233 | true |
| 3 | I09,I10 | X07 production opaque probe rows refuse or admit at their step | src/cmd/planar/bootstrap.t.cpp:1245 | true |
| 3 | I11,I12 | X07 production envelope rows refused by the probe never access the profile | src/cmd/planar/bootstrap.t.cpp:1205 | true |
| 3 | I13 | X07 production matrices enumerate every X01 row | src/cmd/planar/bootstrap.t.cpp:1281 | false |
| 3 | I14 | X07 production envelope rows refused by syntax and prefix usage never access the profile | src/cmd/planar/bootstrap.t.cpp:1200 | true |
| 4 | I15 | X07 profile invocation reads cwd and environment once | src/cmd/planar/bootstrap.t.cpp:1460 | true |
| 4 | I16 | X07 profile selector normalizes lexically without existence | src/cmd/planar/bootstrap.t.cpp:1505 | true |
| 4 | I17 | X07 profile selector honours the path ceiling and the request ceiling wins | src/cmd/planar/bootstrap.t.cpp:1535 | true |
| 5 | I18 | X07 profile load replays a saved request before touching the profile | src/cmd/planar/bootstrap.t.cpp:1581 | true |
| 5 | I19 | X07 profile load with a different body conflicts before profile access | src/cmd/planar/bootstrap.t.cpp:1592 | true |
| 5 | I20 | X07 profile load on a format1 ledger refuses before profile access | src/cmd/planar/bootstrap.t.cpp:1605 | true |
| 5 | I21 | X07 early lookup scope closes before profile capture | src/cmd/planar/bootstrap.t.cpp:1618 | false |
| 6 | I22 | X07 render known slots refuse overlap before registration | src/cmd/planar/bootstrap.t.cpp:1678 | true |
| 6 | I23 | X07 init known slot is alias only and runs before registration | src/cmd/planar/bootstrap.t.cpp:1719 | true |
| 6 | I24 | X07 history known slot refuses a ledger alias before registration | src/cmd/planar/bootstrap.t.cpp:1739 | true |
| 6 | I25 | X07 export known slots refuse before registration | src/cmd/planar/bootstrap.t.cpp:1750 | true |
| 6 | I26 | X07 load known slots refuse before registration | src/cmd/planar/bootstrap.t.cpp:1773 | true |
| 6 | I27,I28 | X07 production normalization and known slot rows refuse at their step | src/cmd/planar/bootstrap.t.cpp:1218 | true |
| 6/12 | I107 | X07 run_known refuses opaque and known load owners | src/engine/runtime/external_registration.t.cpp:4913 | true |
| 7 | I29,I30 | X07 production envelope rows refused after capture and registration are final usage | src/cmd/planar/bootstrap.t.cpp:1210 | true |
| 7 | I105 | X07 final syntax error in an extension tail precedes late slot overlap | src/cmd/planar/bootstrap.t.cpp:2587 | true |
| 8 | I31 | X07 production envelope rows refused after capture and registration are final usage | src/cmd/planar/bootstrap.t.cpp:1210 | true |
| 8 | I32 | X07 shipped tail under a profile keeps probe option state and scalar command words stay values | src/cmd/planar/bootstrap.t.cpp:1810 | true |
| 8/12 | I106 | X07 run_known refuses options that differ from the admitted options | src/engine/runtime/external_registration.t.cpp:4895 | true |
| 9 | I33 | X07 profile owner is consumed by its single dispatch | src/engine/runtime/external_registration.t.cpp:4800 | true |
| 10 | I34 | X07 adapter maps every registration refusal to its closed code message and exit | src/cmd/planar/bootstrap.t.cpp:1982 | true |
| 10 | I35 | X07 adapter never prints fields host paths or tracebacks | src/cmd/planar/bootstrap.t.cpp:1992 | true |
| 10 | I36 | X07 extension preparation usage prints the public usage bytes | src/cmd/planar/bootstrap.t.cpp:2014 | true |
| 10 | I37 | X07 profile preparation semantic error exits by code | src/cmd/planar/bootstrap.t.cpp:2034 | true |
| 10 | I38 | X07 adapter redacts malformed trusted payloads as internal protocol | src/cmd/planar/bootstrap.t.cpp:2062 | true |
| 10/acc4 | I39 | X07 command sources add no diagnostic outside the closed table | src/cmd/planar/bootstrap.t.cpp:2094 | true |
| 10/acc4 | I125 | X07 new runtime refusals use only closed table messages | src/engine/runtime/external_registration.t.cpp:4937 | false |
| 11 | I40 | X07 selected help bytes are exact sorted and escaped | src/cmd/planar/bootstrap.t.cpp:2129 | true |
| 11 | I41 | X07 production selected help rows capture and register the profile | src/cmd/planar/bootstrap.t.cpp:1233 | true |
| 11 | I42 | X07 selected help refuses above the help ceiling | src/cmd/planar/bootstrap.t.cpp:2155 | true |
| 11 | I124 | X07 selected help ceiling is one mebibyte by default and bootstrap passes none | src/cmd/planar/bootstrap.t.cpp:2292 | true |
| 11/acc4 | I43 | X07 root help bytes are unchanged and omit with-profile | src/cmd/planar/bootstrap.t.cpp:1355 | false |
| 12 | I44 | X07 closed factory keeps the original receipts through dispatch | src/engine/runtime/external_registration.t.cpp:4815 | true |
| 12 | I49 | X07 selector overload refuses the load alternative before capture | src/engine/runtime/external_registration.t.cpp:4880 | true |
| 12 | I111 | X07 profile load binds the early lookup intent through prepare_load | src/cmd/planar/bootstrap.t.cpp:2886 | true |
| 12/acc3 | I45 | X07 src cmd contains no profile VM creation site | src/cmd/planar/bootstrap.t.cpp:2172 | false |
| 12/acc3 | I46 | X07 runtime has exactly one profile VM creation call | src/engine/runtime/external_registration.t.cpp:4828 | false |
| 12/acc3 | I47 | X07 exported command_vm invoke still refuses profile owners | src/engine/runtime/external_registration.t.cpp:4841 | false |
| 13 | I48 | X07 closed factory registers exactly once for every family | src/engine/runtime/external_registration.t.cpp:4850 | true |
| 13/12 | I112 | X07 cmd sources open the profile owner through one factory call per branch | src/cmd/planar/bootstrap.t.cpp:2260 | false |
| 14 | I60 | X07 history_id vectors report Lua type and presence through the real CLI | src/cmd/planar/bootstrap.t.cpp:2398 | true |
| 14 | I61 | X07 history_id invalid spellings refuse before the handler | src/cmd/planar/bootstrap.t.cpp:2413 | true |
| 14 | I62 | X07 history_id selector conflicts are usage regardless of magnitude | src/cmd/planar/bootstrap.t.cpp:2432 | true |
| 14 | I63 | X07 history_id after-event with limit pages through the real CLI | src/cmd/planar/bootstrap.t.cpp:2456 | true |
| 15 | I64 | X07 decimal and limit boundary vectors through the real CLI | src/cmd/planar/bootstrap.t.cpp:2466 | true |
| 16 | I65 | X07 history_id misuse is refused at registration through the real CLI | src/cmd/planar/bootstrap.t.cpp:2489 | false |
| 17 | I66 | X07 role read overlaps and aliases refuse before the handler runs | src/cmd/planar/bootstrap.t.cpp:2529 | true |
| 17 | I67 | x07.late_receipt_survives_to_seal | tests/acceptance/x07_services.py:28 | true |
| 18 | I68 | X07 extension responses are the closed extension_v1 and match retained services | src/cmd/planar/bootstrap.t.cpp:2655 | true |
| 18 | I69 | X07 projection omission vector reproduces 87 94 and 59 bytes through the CLI | src/cmd/planar/bootstrap.t.cpp:2692 | true |
| 19 | I70 | x07.workflow_shape_load | tests/acceptance/x07_workflow.py:42 | true |
| 19 | I71 | x07.workflow_render_external | tests/acceptance/x07_workflow.py:54 | true |
| 19 | I72 | x07.workflow_verb_history_export | tests/acceptance/x07_workflow.py:62 | true |
| 20 | I73 | x07.replay_after_inputs_deleted | tests/acceptance/x07_replay.py:67 | true |
| 20 | I74 | x07.replay_conflict_matrix | tests/acceptance/x07_replay.py:75 | true |
| 20 | I75 | x07.replay_same_selector_changed_bytes_new_provenance | tests/acceptance/x07_replay.py:84 | true |
| 21 | I76 | x07.relookup_equal_replay_before_stale_cas | tests/acceptance/x07_replay.py:95 | true |
| 21 | I77 | x07.relookup_unequal_conflicts | tests/acceptance/x07_replay.py:95 | true |
| 21 | I78 | x07.two_process_cas_one_winner | tests/acceptance/x07_replay.py:107 | true |
| 22 | I79 | x07.receipt_races_refuse_before_authorization | tests/acceptance/x07_replay.py:115 | true |
| 22 | I80 | x07.final_race_rolls_back_before_commit | tests/acceptance/x07_replay.py:115 | true |
| 22 | I81 | X06 C no Lua runs while the profile write transaction is open | src/engine/ledger/profile_acceptance.t.cpp:561 | false |
| 23 | I82 | x07.post_commit_allocation_failure_keeps_receipt | tests/acceptance/x07_replay.py:130 | true |
| 23 | I83 | x07.post_commit_encoding_failure_keeps_receipt | tests/acceptance/x07_replay.py:137 | true |
| 23 | I84 | x07.post_commit_stdout_failure_keeps_receipt | tests/acceptance/x07_replay.py:142 | true |
| 23 | I85 | replacement.load_cli | LOCATION NOT FOUND | false |
| 24 | I86 | X07 public init creates a format2 ledger with unchanged response | src/cmd/planar/bootstrap.t.cpp:2763 | true |
| 24 | I87 | X07 init refuses a format selector option | src/cmd/planar/bootstrap.t.cpp:2777 | false |
| 24 | I88 | X07 native format1 initializer remains available to fixtures | src/cmd/planar/bootstrap.t.cpp:2785 | false |
| 25 | I89 | X07 embedded load on format2 frames request_v2 with embedded selector | src/cmd/planar/bootstrap.t.cpp:2819 | true |
| 25 | I90 | X07 embedded load on format1 keeps request_v1 | src/cmd/planar/bootstrap.t.cpp:2827 | true |
| 25 | I91 | X07 embedded format2 load keeps v1 interpretation and versions | src/cmd/planar/bootstrap.t.cpp:2842 | true |
| 26 | I92 | X07 profile load records request_v2 selector effective stdlib and v2 interpretation | src/cmd/planar/bootstrap.t.cpp:2861 | true |
| 27 | I50 | X07 history and export under a profile validate the profile before dispatch | src/cmd/planar/bootstrap.t.cpp:2185 | true |
| 27 | I51 | X07 history and export responses are identical with and without a profile | src/cmd/planar/bootstrap.t.cpp:2206 | true |
| 27 | I52 | X07 export refuses a target overlapping the trusted root before publication | src/cmd/planar/bootstrap.t.cpp:2230 | true |
| 28 | I53 | X07 ledger aliasing the profile or a module refuses for history export and load | src/cmd/planar/bootstrap.t.cpp:2244 | true |
| 29 | I93 | X07 extension and profile rows print one JSON line and nothing during registration | src/cmd/planar/bootstrap.t.cpp:2741 | true |
| 29 | I94 | x07.response_ceiling_extension | tests/acceptance/x07_services.py:42 | true |
| 30 | I95 | X07 N30 late slot admission charges the factory budget owner | src/engine/runtime/external_registration.t.cpp:5048 | false |
| 30 | I96 | X07 N30 role capture charges the factory budget owner | src/engine/runtime/external_registration.t.cpp:5059 | false |
| 30 | I97 | X07 N30 history decode charges the factory budget owner | src/engine/runtime/external_registration.t.cpp:5139 | true |
| 30 | I98 | X07 N30 projection transport charges the factory budget owner | src/engine/runtime/external_registration.t.cpp:5074 | false |
| CL-5/CL-6 | I121 | catch-all diff (evidence/gates/catch-all-diff.txt) | LOCATION NOT FOUND | false |
| P8 | I108 | negative-compile probe open_admitted (evidence/negative-compile/open_admitted.log) | LOCATION NOT FOUND | false |
| P8 | I109 | negative-compile probe invoke_known (evidence/negative-compile/invoke_known.log) | LOCATION NOT FOUND | false |
| P8 | I110 | negative-compile probe profile_invocation constructor (evidence/negative-compile/constructor.log) | LOCATION NOT FOUND | false |
| acc2 | I100 | inventory comparison script output (evidence/inventory) | LOCATION NOT FOUND | false |
| acc3 | I116 | X06 C C01 seam is unreachable from a plain acceptance importer | src/engine/ledger/acceptance.t.cpp:770 | false |
| acc3 | I117 | git diff 7a81b0bc plus git status porcelain on the seam pathspecs (evidence/gates/seam-diff.txt) | LOCATION NOT FOUND | false |
| acc5 | I101 | ctest logs of both configurations (evidence/gates) | LOCATION NOT FOUND | false |
| acc6/c32 | I102 | docs gate output (evidence/gates/docs) | LOCATION NOT FOUND | false |
| acc7 | I104 | docker probe log (evidence/linux) | LOCATION NOT FOUND | false |
| c31/acc1 | I103 | red runner log (evidence/red) | LOCATION NOT FOUND | false |
| gate 6 | I118 | lint log (evidence/gates/lint.log) | LOCATION NOT FOUND | false |
| gate 7 | I119 | second build log (evidence/gates/nochange-build.log) | LOCATION NOT FOUND | false |
| gate 8 | I120 | redaction log (evidence/gates/redaction.log) | LOCATION NOT FOUND | false |
| must-not | I122 | X07 root level stdlib-profile flag is usage | src/cmd/planar/bootstrap.t.cpp:1348 | false |
| must-not | I123 | X07 profile environment variables are ignored without with-profile | src/cmd/planar/bootstrap.t.cpp:2273 | false |

Rows without a ctest test (evidence artifacts, produced by the earlier gate under tests/native/v5-02-x07-evidence/): I100, I101, I102, I104, I108 to I110, I117 to I121 and the CL-5/CL-6 catch-all diff. Clauses 31 and 32 appear only as these evidence or docs rows (acc6/c32 is the docs gate); this gate did not re-run the negative-compile probes, the Linux Docker probe or the catch-all diff.

## Red-first

Coders' evidence for the X07 lineage: tests/native/v5-02-x07-evidence/red/RED.md (HEAD 7a81b0bc; sha256 of snapshot/candidate.diff ff619c029b3385ff0ad1c571a53034b6a85dd447770900fd20fd2fca71c2aa03; SHA256SUMS bc041618e3aeb852b804f86b8040d2a9b1160cbc9e0a0086dbec1e18aad07367; identity.py combined sha256 cd2d73a34a6786222c4f03de67cefbf972d3c82b7b35c4bc2dc61d329be851b1).
Command (from RED.md): cmake --build build/rewrite-debug --target planar_tests, then per test: ctest --test-dir build/rewrite-debug -R '^<escaped full name>$' --no-tests=error --output-on-failure, with the replacement. prefix. Raw per-test logs: red/raw/001.log onward.
Result stated there: 96 distinct names run; 79 failed on an assertion (real red, first failing file:line per test in the RED.md table, for example bootstrap.t.cpp:998, :1093, external_registration.t.cpp:4803); 17 passed and were all expectRed=false guards (source-scan rows, root help bytes, X06 C no-Lua guard, init refuses a format selector option, root level stdlib-profile flag). Two guards were allowed to fail: "X07 history_id misuse is refused at registration through the real CLI" and replacement.load_cli. The per-test table is in RED.md and is not repeated.

Sidecar repair tests, all four in src/engine/ledger/storage.t.cpp:
| Test | file:line | Red-first evidence |
|---|---|---|
| storage begin_write waits out a transient live writer journal | storage.t.cpp:185 | MISSING. Expected real red (pre-fix code refuses at once). Not demonstrated. |
| storage begin_write still refuses a persistent journal | storage.t.cpp:201 | MISSING. Expected guard that passed before the fix. Not demonstrated. |
| storage begin_write sidecar refusal message is byte exact | storage.t.cpp:212 | MISSING. Likely a guard (message unchanged). Not demonstrated. |
| storage begin_write surfaces a non-ENOENT sidecar stat failure | storage.t.cpp:224 | MISSING. Added after the first gate run (apparently to kill probe mutant M08, which survived the focused set, see probes/PROBES.md). No command, snapshot hash or failing line found. |

No command, pre-fix snapshot hash or failing line for these four exists in tests/native/v5-02-x07-evidence or this directory. The repair coder or prober must produce it. The prober's mutants for them (probes/PROBES.md: M01, M02, M04 kill the transient test; M03, M06 kill the persistent test) were applied in a scratch copy and are mutation evidence, not red-first evidence.

## Deliberate behaviour differences

- The sidecar repair (src/engine/ledger/storage.cpp, no_sidecars) does NOT move the sidecar check after BEGIN IMMEDIATE as Dispatch 2 prescribes. It keeps the early check and adds a bounded poll: 200 attempts at 5 ms (about 1 s) before refusing with recovery_required. The cited authority is docs/vernacular/ledger-contract.md (busy wait 5000 ms; hot-journal recovery only for a recognized owned database). Whether a 1 s grace is within that clause is not established by this gate and needs reviewer judgment. A genuine crash leftover now costs about 1 s extra before refusing; the refusal message is byte-identical.
- Gate-repair test edits and the X06 D public-surface text change are recorded in the round record "Gate repair" section; not re-audited here.

## Limitations

- No-change build verified on the Debug tree only; the Release tree was built for planar_tests only.
- Linux probe, negative-compile probes and the catch-all diff were not run here.
- Repair criterion 1 (CAS test 20 consecutive Release runs): not in the evidence directory and not part of this gate; the test passed once in each full run here.
- Red-first evidence for the four storage tests is missing (above).
- The gate runner did not recheck the D7-11 assertion list.
- Mutation probes cover only the storage lineage (probes/PROBES.md); the other X07 invariant rows were not probed.
