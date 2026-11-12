# R19-ACCEPT-A-C03 mutation probes (revision 2, focused re-probe after the C3-R1m-c repair)

Run 2026-11-12 by the mutation prober. Mutants were applied only in the persistent scratch copy `<scratch-root>/probe-warm` (synced from the checkout with `rsync -a --delete --exclude build/ --exclude .git/ --exclude 'tests/native/*-evidence/'`; its build directory kept). Each mutant was restored by byte copy and the sha256 verified equal (every raw log carries the line `restored byte copy, sha256 equal to original: True`); the one C++-affecting mutant (`C3-R1b`, CMake path) was rebuilt after restore and SCHEMA612 passes again. The old evidence copy `tests/native/r12b-evidence/expected-schema.sql` is absent in the scratch copy (evidence trees are excluded from the sync), which is exactly a clean export. Nothing in the shared checkout or its build tree was edited; no PROBES.md, raw/ or results existed under `probes/` before this run, so nothing was moved to `superseded-<n>/`.

Live tests ran against the recorded candidate `planar_authoring_stack` sha256 `774bcd69809ead9e4cd7b6e06c74aa045ea9272a2321cd36f91bc9c76a99ab0c` (re-hashed before the run, equal to the recorded value; executed only through the live tests) with scratch HOME and PLANAR_DB. Control: `raw/BASELINE.log` (all observers pass on the unmodified scratch: 35 unittest cases OK, ctest x08 and SCHEMA612 pass, ignored-file count 0, docs greps exit 1, four record greps 4).

Revision-2 note: the identity below is the revision-2 identity.py output (run 2026-11-12 before the re-probe; only accept_test.py differs from revision 1). Rows marked 'carried from superseded-1' were run in revision 1; rows C3-R1l, R1m, R1n, R1o, R1p were additionally re-run in revision 2 (see Focused re-probe, logs in `raw/`).

## Identity (identity.py output, verbatim, run before the first mutant; pathspecs src tests/native/r19-accept-support fixtures scripts tests/acceptance docs/testing.md)

| identity | value |
|---|---|
| HEAD | `91e0df24f2167471e49ec057c7159bce5dff3608` |
| sha256(git diff --binary HEAD -- src tests/native/r19-accept-support fixtures scripts tests/acceptance docs/testing.md), tracked only | `2daacd67636901cb57039c3c41aa8b083cbbf2bf4761a1fd127439d8c6c8463c` |
| combined sha256 (diff bytes, then each untracked path NUL bytes, sorted), pathspec src tests/native/r19-accept-support fixtures scripts tests/acceptance docs/testing.md, evidence excluded | `5988c6d810da9c9b35b46a3fd57e44ee2d75a8ed23ac984d6b1dbb86d4a98bb2` |

| file | state | sha256 |
|---|---|---|
| `docs/testing.md` | modified | `703735b636090c685d7ad09430c5c7d4714df6fbdb135ada276aec7f8956769c` |
| `scripts/linux-gate.Dockerfile` | modified | `9c7b0eb08d883e997b67777647936589af180dada76d41c2798d3a7285d938d0` |
| `scripts/linux-gate.Dockerfile.dockerignore` | modified | `45caa66afac655fbf7daeba3025bf1fa16be2ca1ac08c2875f002397464db407` |
| `scripts/linux-gate.sh` | modified | `121d0df5998644b3b6ed2ebf2ef9eb4b807b77525b8f4c01855a1764fcd45cea` |
| `src/engine/ledger/CMakeLists.txt` | modified | `8141a8d56f504208eafd8f4ae6145e49f3f71e34f1300970bf7b678056b7779d` |
| `src/engine/ledger/execution.t.cpp` | modified | `250867b8bd74b86d718cbe17e5fa9983fda6a60a2b5e7c127829a3fec6f98950` |
| `src/engine/ledger/storage.cpp` | modified | `e4cf41dcb8cf44cc8004d4ba011543a557c742c623e20ede631eb975f237d822` |
| `tests/acceptance/x08_recipe.py` | modified | `cc04a0fbc557759a6e912596488969ce7308a507b8fde9b19f001a9c4be71014` |
| `fixtures/ledger/expected-schema.sql` | untracked | `730e38be112f8024a8a8eea232a2c9728ea1f23aae0f5ea90e14e200347d5395` |
| `tests/native/r19-accept-support/accept_lib.py` | untracked | `6ef4e069c2da203d508f3695482b5ea9883b71fc47ee8519a6f910f99e223e68` |
| `tests/native/r19-accept-support/accept_test.py` | untracked | `56e8a2a85c0c5032ec5461d81e45620b4c591de7ce214437be161b0b1978ad02` |
| `tests/native/r19-accept-support/acceptance_cli_test.py` | untracked | `d0d5f519d21e5910e4fa0981ff3d3bd43d2dfcdd240eebb5df6739b596d1842e` |
| `tests/native/r19-accept-support/check_runner.py` | untracked | `b71e939a226415c0aeb43083dffcf263618d2c88804500300e585bfafec1d5f1` |
| `tests/native/r19-accept-support/cmake_check.py` | untracked | `ba8f20625bda0f3c8fc3c613529e16f6a1f8e0a38b0b261f8341b2fdcbb3f360` |
| `tests/native/r19-accept-support/fault/cmake_rerun/run_heavy.py` | untracked | `c1751e0d6858337b6cbd1fa2ddef07cabc8caa083ab2eca38f86f18566c45e1e` |
| `tests/native/r19-accept-support/fault/delayed_marker_check.py` | untracked | `6ca4c0b4283083a8241a5edbe71c1912d0d8fa908ac6d602de29f453ed728141` |
| `tests/native/r19-accept-support/fault/mini/base/src/unit.txt` | untracked | `d73d5ccc52b8747e2bc253b7267b6110d41021eacd54ff33695d96c0a47fe56a` |
| `tests/native/r19-accept-support/fault/mini/jobs/combined-max1.lua` | untracked | `c7d8b3a33fd7940b6ef56e4ac4923836f527644d8b6952aaa8ae5d22f54e8e39` |
| `tests/native/r19-accept-support/fault/mini/jobs/combined-max2.lua` | untracked | `6d0df6cf79f148664587dcf43e774243cad211e4d26420ce8fe4f955c1281e0e` |
| `tests/native/r19-accept-support/fault/mini/registry.json` | untracked | `eca5ac671b6929f042a839591ff56da72d0afef8f4e9aca5433df7b1119490ca` |
| `tests/native/r19-accept-support/fault/mini/source/src/unit.txt` | untracked | `753f8fe079abf1160eaa98cc40257a00da60a033b2d3730ab679fa0f62a0778c` |
| `tests/native/r19-accept-support/fault/mini/workspace/spec.lua` | untracked | `d35b07e34032202e830cedb66b4d45b5774621f087acc9f8012b1d8edacf07da` |
| `tests/native/r19-accept-support/fault/mini/workspace/workspace.lua` | untracked | `21ac5c6da0e2d27bb4911202d0a364acacc49d39b052fa71d8ca44b68e2d453d` |
| `tests/native/r19-accept-support/fault/sleep_check.py` | untracked | `751414911570978faf4f82d0ee79631ceb41c76dfd2bc4e66d35c41faa51ce57` |
| `tests/native/r19-accept-support/fault_driver.py` | untracked | `be61aeb60fe5cbeb49ee2521ec95b2ce08c124441293fb20dc2aad1e0b7eebcf` |
| `tests/native/r19-accept-support/fault_live_test.py` | untracked | `9cd192a86eccdcdf652f99416bfc6415bd91f943aa60184d7cdbfa5438685621` |
| `tests/native/r19-accept-support/ignored_context_check.sh` | untracked | `648e19466cdc4736f99a0817a0b9c19f5cfb59697868d471c84c94685d58f039` |
| `tests/native/r19-accept-support/quota_driver.py` | untracked | `3c45a76571439a528e3cf1aee3803308964349da6edb247235cc656e8a4a3420` |
| `tests/native/r19-accept-support/quota_live_test.py` | untracked | `bedad9a306b98b535c33f2a27ea4224575bc6f2a81120019815d3d119117f861` |
| `tests/native/r19-accept-support/registry_gen.py` | untracked | `26752307e7257a99722b713342c790ae3eb146bd99c93ea410b6b4312109d172` |
| `tests/native/r19-accept-support/run_acceptance.py` | untracked | `2ff48c14f2bb7daff546583f1e58888220378a3f26e70255201cf17ec038c1a8` |


## Carry-forward decision

The previous PROBES.md in this lineage is `tests/native/r19-accept-a-c02-evidence/probes/PROBES.md`. Its identity listed `fault_driver.py` `ced606f4...`, `fault_live_test.py` `d675691f...`, `accept_test.py` `c6ae7b28...`, `acceptance_cli_test.py` `1b71cf8c...`, `run_acceptance.py` `3c04f00e...`, `run_heavy.py` `316fa2ab...`. Every one of these files has a different hash now (identity above), so no C02 row qualifies for carry-forward: every C02 row on those files was re-run (rows `C3-F*`, `C3-L14` and `C3L-*`, using the diffs from the C02 raw logs, applied with `patch --fuzz=3`; none failed to apply). C02 rows on files unchanged now (`accept_lib.py` `6ef4e069...`, `check_runner.py`, `cmake_check.py`, `quota_*`) are not tied to this round's findings and were not re-run (see Skipped).

## Table 1. Invariant, mutant, outcome (all rows run this round)

Raw log per mutant: `raw/<mutant id>.log` (unified diff, diff sha256, original/mutated/restored sha256, exact commands, exit codes, failing test names). Mutants `C3L-*` are the C02 L20/L21 and H rows re-run on the changed `run_acceptance.py`.

| invariant | mutant id | diff sha256 (first 16) | mutated file | outcome | killing test or observer output (expected observer) | run or carried-from |
|---|---|---|---|---|---|---|
| C3-R1a | `C3-R1a2-nl` | `1b9e4be157c24d51` | `fixtures/ledger/expected-schema.sql` | KILLED by RECORD observer | none (observer shell exits [0, 1]) (expected: RECORD cmp -s: drift gives cmp exit 1 (SCHEMA612 still passes, as the table says)) | carried from superseded-1 (`superseded-1/raw/C3-R1a2-nl.log`) |
| C3-R1a2 | `C3-R1a2-int` | `9b39e542c27df62a` | `fixtures/ledger/expected-schema.sql` | KILLED | replacement.ledger format initializes exact metadata schema and Glaze event, replacement.ledger format initializes exact metadata schema and Glaze event (expected: SCHEMA612) | carried from superseded-1 (`superseded-1/raw/C3-R1a2-int.log`) |
| C3-R1a2 | `C3-R1a2-idx` | `469c86c79214729e` | `fixtures/ledger/expected-schema.sql` | KILLED | replacement.ledger format initializes exact metadata schema and Glaze event, replacement.ledger format initializes exact metadata schema and Glaze event (expected: SCHEMA612) | carried from superseded-1 (`superseded-1/raw/C3-R1a2-idx.log`) |
| C3-R1b | `C3-R1b` | `07419ce6d82055a5` | `src/engine/ledger/CMakeLists.txt` | KILLED | replacement.ledger format initializes exact metadata schema and Glaze event, replacement.ledger format initializes exact metadata schema and Glaze event (expected: SCHEMA612) | carried from superseded-1 (`superseded-1/raw/C3-R1b.log`) |
| C3-R1f | `C3-R1f-a` | `895e0968eb1ea017` | `scripts/linux-gate.sh` | KILLED | x08.linux_recipe_pins, x08.linux_recipe_pins (expected: x08.linux_recipe_pins) | carried from superseded-1 (`superseded-1/raw/C3-R1f-a.log`) |
| C3-R1f | `C3-R1f-b` | `922a333c46e46b09` | `scripts/linux-gate.sh` | KILLED | x08.linux_recipe_pins, x08.linux_recipe_pins (expected: x08.linux_recipe_pins) | carried from superseded-1 (`superseded-1/raw/C3-R1f-b.log`) |
| C3-R1g | `C3-R1g` | `8a41578057006e2c` | `scripts/linux-gate.sh` | KILLED | x08.linux_recipe_pins, x08.linux_recipe_pins (expected: x08.linux_recipe_pins) | carried from superseded-1 (`superseded-1/raw/C3-R1g.log`) |
| C3-R1g | `C3-R1g-b` | `92a8686a5463fe3f` | `scripts/linux-gate.sh` | KILLED | x08.linux_recipe_pins, x08.linux_recipe_pins (expected: x08.linux_recipe_pins) | carried from superseded-1 (`superseded-1/raw/C3-R1g-b.log`) |
| C3-R1h | `C3-R1h-a` | `656c1a196f7d6cf8` | `scripts/linux-gate.Dockerfile` | KILLED | x08.linux_recipe_pins, x08.linux_recipe_pins (expected: x08.linux_recipe_pins) | carried from superseded-1 (`superseded-1/raw/C3-R1h-a.log`) |
| C3-R1h | `C3-R1h-b` | `1f05c46bdb91fbc1` | `scripts/linux-gate.Dockerfile` | KILLED | x08.linux_recipe_pins, x08.linux_recipe_pins (expected: x08.linux_recipe_pins) | carried from superseded-1 (`superseded-1/raw/C3-R1h-b.log`) |
| C3-R1h | `C3-R1h-c` | `5ecd21168922b725` | `scripts/linux-gate.Dockerfile` | KILLED | x08.linux_recipe_pins, x08.linux_recipe_pins (expected: x08.linux_recipe_pins) | carried from superseded-1 (`superseded-1/raw/C3-R1h-c.log`) |
| C3-R1i | `C3-R1i-a` | `83aacd4a412bcbc5` | `scripts/linux-gate.Dockerfile.dockerignore` | KILLED | x08.linux_recipe_pins, x08.linux_recipe_pins (expected: x08.linux_recipe_pins) | carried from superseded-1 (`superseded-1/raw/C3-R1i-a.log`) |
| C3-R1i | `C3-R1i-b` | `1ff89d0dd693af77` | `scripts/linux-gate.Dockerfile.dockerignore` | KILLED | x08.linux_recipe_pins, x08.linux_recipe_pins (expected: x08.linux_recipe_pins) | carried from superseded-1 (`superseded-1/raw/C3-R1i-b.log`) |
| C3-R1i | `C3-R1i-c` | `8f631bed3d62fb79` | `scripts/linux-gate.Dockerfile.dockerignore` | KILLED | x08.linux_recipe_pins, x08.linux_recipe_pins (expected: x08.linux_recipe_pins) | carried from superseded-1 (`superseded-1/raw/C3-R1i-c.log`) |
| C3-R1i | `C3-R1i-d` | `56fcd767516665ab` | `scripts/linux-gate.Dockerfile.dockerignore` | KILLED | x08.linux_recipe_pins, x08.linux_recipe_pins (expected: x08.linux_recipe_pins) | carried from superseded-1 (`superseded-1/raw/C3-R1i-d.log`) |
| C3-R1j | `C3-R1j-wm` | `05968ba12a43b05b` | `docs/vernacular/rounds/R19-ACCEPT-A.md` | KILLED by RECORD observer | none (observer shell exits [1]) (expected: RECORD four greps (each mutant removes one site; its own grep reports MISSING)) | carried from superseded-1 (`superseded-1/raw/C3-R1j-wm.log`) |
| C3-R1j | `C3-R1j-ex` | `d8b10ad00094882c` | `docs/vernacular/rounds/R19-ACCEPT-A.md` | KILLED by RECORD observer | none (observer shell exits [1]) (expected: RECORD four greps (each mutant removes one site; its own grep reports MISSING)) | carried from superseded-1 (`superseded-1/raw/C3-R1j-ex.log`) |
| C3-R1j | `C3-R1j-di` | `0c1b03309f066390` | `docs/vernacular/rounds/R19-ACCEPT-A.md` | KILLED by RECORD observer | none (observer shell exits [1]) (expected: RECORD four greps (each mutant removes one site; its own grep reports MISSING)) | carried from superseded-1 (`superseded-1/raw/C3-R1j-di.log`) |
| C3-R1j | `C3-R1j-df` | `ba644104c9e90a17` | `docs/vernacular/rounds/R19-ACCEPT-A.md` | KILLED by RECORD observer | none (observer shell exits [1]) (expected: RECORD four greps (each mutant removes one site; its own grep reports MISSING)) | carried from superseded-1 (`superseded-1/raw/C3-R1j-df.log`) |
| C3-R1k | `C3-R1k-a` | `a8901395c18eab80` | `docs/testing.md` | KILLED by RECORD observer | none (observer shell exits [0, 0]) (expected: RECORD grep (exit 0 = stale text present, the record expects exit 1)) | carried from superseded-1 (`superseded-1/raw/C3-R1k-a.log`) |
| C3-R1k | `C3-R1k-b` | `4e482514578d3cc8` | `docs/testing.md` | KILLED by RECORD observer | none (observer shell exits [0]) (expected: RECORD grep (exit 0 = stale text present, the record expects exit 1)) | carried from superseded-1 (`superseded-1/raw/C3-R1k-b.log`) |
| C3-R1q | `C3-R1q-a` | `8a41578057006e2c` | `scripts/linux-gate.sh` | KILLED by RECORD observer | none (observer shell exits [0]) (expected: RECORD ignored_context_check.sh (count 1 and the old path; count 105633)) | carried from superseded-1 (`superseded-1/raw/C3-R1q-a.log`) |
| C3-R1q | `C3-R1q-b` | `92a8686a5463fe3f` | `scripts/linux-gate.sh` | KILLED by RECORD observer | none (observer shell exits [0]) (expected: RECORD ignored_context_check.sh (count 1 and the old path; count 105633)) | carried from superseded-1 (`superseded-1/raw/C3-R1q-b.log`) |
| C3-R1l | `C3-R1l-a` | `fd4a6dc43d7507d0` | `tests/native/r19-accept-support/fault/cmake_rerun/run_heavy.py` | KILLED | test_run_heavy_exits_nonzero_when_the_scenario_stopped (expected: HeavyDriverTest.test_run_heavy_exits_nonzero_when_the_scenario_stopped) | carried from superseded-1 (`superseded-1/raw/C3-R1l-a.log`) |
| C3-R1l | `C3-R1l-b` | `afc79976a445b75f` | `tests/native/r19-accept-support/fault/cmake_rerun/run_heavy.py` | KILLED | test_run_heavy_exits_nonzero_when_the_scenario_stopped (expected: HeavyDriverTest.test_run_heavy_exits_nonzero_when_the_scenario_stopped) | carried from superseded-1 (`superseded-1/raw/C3-R1l-b.log`) |
| C3-R1l | `C3-R1l-c` | `bbc18a9c2612defc` | `tests/native/r19-accept-support/fault/cmake_rerun/run_heavy.py` | KILLED | test_run_heavy_exits_nonzero_when_the_scenario_stopped (expected: HeavyDriverTest.test_run_heavy_exits_nonzero_when_the_scenario_stopped) | carried from superseded-1 (`superseded-1/raw/C3-R1l-c.log`) |
| C3-R1l | `C3-R1l-d` | `ad3e8930c5919cc1` | `tests/native/r19-accept-support/fault/cmake_rerun/run_heavy.py` | KILLED | test_run_heavy_exits_nonzero_when_the_scenario_stopped (expected: HeavyDriverTest.test_run_heavy_exits_nonzero_when_the_scenario_stopped) | carried from superseded-1 (`superseded-1/raw/C3-R1l-d.log`) |
| C3-R1m | `C3-R1m` | `a5e642e3771817d7` | `tests/native/r19-accept-support/fault/cmake_rerun/run_heavy.py` | KILLED | test_run_heavy_exits_zero_and_writes_result_for_a_completed_scenario (expected: HeavyDriverTest.test_run_heavy_exits_zero_and_writes_result_for_a_completed_scenario) | carried from superseded-1 (`superseded-1/raw/C3-R1m.log`) |
| C3-R1n | `C3-R1n-a` | `6889d157436b75b3` | `tests/native/r19-accept-support/fault/cmake_rerun/run_heavy.py` | KILLED | test_run_heavy_config_overlays_the_fixture_and_ledger_cmake (expected: HeavyDriverTest.test_run_heavy_config_overlays_the_fixture_and_ledger_cmake) | carried from superseded-1 (`superseded-1/raw/C3-R1n-a.log`) |
| C3-R1n | `C3-R1n-b` | `ac5e4d3f9e67ea1f` | `tests/native/r19-accept-support/fault/cmake_rerun/run_heavy.py` | KILLED | test_run_heavy_config_overlays_the_fixture_and_ledger_cmake (expected: HeavyDriverTest.test_run_heavy_config_overlays_the_fixture_and_ledger_cmake) | carried from superseded-1 (`superseded-1/raw/C3-R1n-b.log`) |
| C3-R1n | `C3-R1n-c` | `304c764ec1a0a798` | `tests/native/r19-accept-support/fault/cmake_rerun/run_heavy.py` | KILLED | test_run_heavy_config_overlays_the_fixture_and_ledger_cmake (expected: HeavyDriverTest.test_run_heavy_config_overlays_the_fixture_and_ledger_cmake) | carried from superseded-1 (`superseded-1/raw/C3-R1n-c.log`) |
| C3-R1o | `C3-R1o-a` | `bf2a50da02529ffa` | `tests/native/r19-accept-support/fault_driver.py` | KILLED | test_export_overlay_copies_working_tree_bytes_of_the_named_paths (expected: HeavyDriverTest.test_export_overlay_copies_working_tree_bytes_of_the_named_paths) | carried from superseded-1 (`superseded-1/raw/C3-R1o-a.log`) |
| C3-R1o | `C3-R1o-b` | `289d279a99f0fbbe` | `tests/native/r19-accept-support/fault_driver.py` | KILLED | test_export_overlay_copies_working_tree_bytes_of_the_named_paths (expected: HeavyDriverTest.test_export_overlay_copies_working_tree_bytes_of_the_named_paths) | carried from superseded-1 (`superseded-1/raw/C3-R1o-b.log`) |
| C3-R1o | `C3-R1o-c` | `848271bb22019fe2` | `tests/native/r19-accept-support/fault_driver.py` | KILLED | test_export_overlay_copies_working_tree_bytes_of_the_named_paths (expected: HeavyDriverTest.test_export_overlay_copies_working_tree_bytes_of_the_named_paths) | carried from superseded-1 (`superseded-1/raw/C3-R1o-c.log`) |
| C3-R1p | `C3-R1p` | `8e88afef6219b519` | `tests/native/r19-accept-support/fault_driver.py` | KILLED | test_export_overlay_refuses_a_path_missing_from_the_working_tree (expected: HeavyDriverTest.test_export_overlay_refuses_a_path_missing_from_the_working_tree) | carried from superseded-1 (`superseded-1/raw/C3-R1p.log`) |
| C3-H0 | `C3-H0-b` | `68e0ef2e1365e5fe` | `tests/native/r19-accept-support/fault/cmake_rerun/run_heavy.py` | KILLED | test_run_heavy_aborts_on_unrecorded_candidate (expected: FaultLiveTest.test_run_heavy_aborts_on_unrecorded_candidate; AcceptanceCliTest.test_live_commands_abort_before_first_action_on_candidate_mismatch) | carried from superseded-1 (`superseded-1/raw/C3-H0-b.log`) |
| C3-H0 | `C3L-R1a-a` | `81db59f75b957c56` | `tests/native/r19-accept-support/run_acceptance.py` | KILLED | test_live_commands_abort_before_first_action_on_candidate_mismatch (expected: FaultLiveTest.test_run_heavy_aborts_on_unrecorded_candidate; AcceptanceCliTest.test_live_commands_abort_before_first_action_on_candidate_mismatch) | carried from superseded-1 (`superseded-1/raw/C3L-R1a-a.log`) |
| C3-H2 | `C3-F1-a` | `eb10a321c67e8f10` | `tests/native/r19-accept-support/fault_driver.py` | KILLED | test_interrupted_check_is_recovered_by_check_recover (expected: interrupted-check recover and restart tests) | carried from superseded-1 (`superseded-1/raw/C3-F1-a.log`) |
| C3-H2 | `C3-F1-b` | `530f9b9d23a64bbd` | `tests/native/r19-accept-support/fault_driver.py` | KILLED | test_interrupted_check_is_recovered_by_check_recover (expected: interrupted-check recover and restart tests) | carried from superseded-1 (`superseded-1/raw/C3-F1-b.log`) |
| C3-H2 | `C3-F1-c` | `f36296f8d4baa224` | `tests/native/r19-accept-support/fault_driver.py` | KILLED | test_interrupted_check_is_recovered_by_check_recover (expected: interrupted-check recover and restart tests) | carried from superseded-1 (`superseded-1/raw/C3-F1-c.log`) |
| C3-H2 | `C3-F2` | `ab239524b86bfae7` | `tests/native/r19-accept-support/fault_driver.py` | KILLED | test_state_survives_process_restart_between_phases (expected: interrupted-check recover and restart tests) | carried from superseded-1 (`superseded-1/raw/C3-F2.log`) |
| C3-H3 | `C3-F3` | `fb2ce8b9efe2c1e3` | `tests/native/r19-accept-support/fault_driver.py` | KILLED | test_takeover_fences_old_run_token (expected: test_takeover_fences_old_run_token) | carried from superseded-1 (`superseded-1/raw/C3-F3.log`) |
| C3-H4 | `C3-F4` | `cdfec4a71529bc23` | `tests/native/r19-accept-support/fault_driver.py` | KILLED | test_rejecting_verdict_admits_repair_and_worker_returns (expected: test_rejecting_verdict_admits_repair_and_worker_returns) | carried from superseded-1 (`superseded-1/raw/C3-F4.log`) |
| C3-H5 | `C3-F5` | `e8b38189a0755acb` | `tests/native/r19-accept-support/fault_driver.py` | KILLED | test_second_rejection_is_capped_and_opens_max_rounds_blocker (expected: capped second rejection; load raising max_rounds) | carried from superseded-1 (`superseded-1/raw/C3-F5.log`) |
| C3-H5 | `C3-F6` | `eab7f385518af7ee` | `tests/native/r19-accept-support/fault_driver.py` | KILLED | test_load_raising_max_rounds_clears_blocker (expected: capped second rejection; load raising max_rounds) | carried from superseded-1 (`superseded-1/raw/C3-F6.log`) |
| C3-H6 | `C3-F7` | `8cf77ee2fb29d9f3` | `tests/native/r19-accept-support/fault_driver.py` | KILLED | test_final_approval_after_faults_is_exact_target (expected: test_final_approval_after_faults_is_exact_target) | carried from superseded-1 (`superseded-1/raw/C3-F7.log`) |
| C3-H7 | `C3-F8` | `fe42ece75abccfe9` | `tests/native/r19-accept-support/fault_driver.py` | KILLED | test_replays_and_inspections_do_not_reexecute_checks_or_write (expected: replay, inspection, tamper and counter tests) | carried from superseded-1 (`superseded-1/raw/C3-F8.log`) |
| C3-H7 | `C3-F15` | `daff5b26d304247a` | `tests/native/r19-accept-support/fault_driver.py` | KILLED | test_run_scenario_aborts_when_inspection_changes_ledger (expected: replay, inspection, tamper and counter tests) | carried from superseded-1 (`superseded-1/raw/C3-F15.log`) |
| C3-H7 | `C3-F16` | `ddea8976a100e7ac` | `tests/native/r19-accept-support/fault_driver.py` | KILLED | test_run_scenario_aborts_on_tampered_replay (expected: replay, inspection, tamper and counter tests) | carried from superseded-1 (`superseded-1/raw/C3-F16.log`) |
| C3-H7 | `C3-F17` | `4b021b2cb2701d76` | `tests/native/r19-accept-support/fault_driver.py` | KILLED | test_run_scenario_aborts_when_counter_increases_on_replay (expected: replay, inspection, tamper and counter tests) | carried from superseded-1 (`superseded-1/raw/C3-F17.log`) |
| C3-H8 | `C3-F9` | `71e8e1051bcd37d3` | `tests/native/r19-accept-support/fault_driver.py` | KILLED | test_history_references_through_faults (expected: test_history_references_through_faults) | carried from superseded-1 (`superseded-1/raw/C3-F9.log`) |
| C3-H8 | `C3-L14` | `687dcfa0bdb7c07e` | `tests/native/r19-accept-support/fault_driver.py` | KILLED | test_history_references_through_faults (expected: test_history_references_through_faults) | carried from superseded-1 (`superseded-1/raw/C3-L14.log`) |
| C3-H9 | `C3-F13` | `02281d7de5f7776a` | `tests/native/r19-accept-support/fault_driver.py` | KILLED | test_first_completed_check_passes_boundary_rows (expected: boundary rows; retained attempt folders) | carried from superseded-1 (`superseded-1/raw/C3-F13.log`) |
| C3-H9 | `C3-F10` | `8013bbcf5ab2b22a` | `tests/native/r19-accept-support/fault_driver.py` | KILLED | test_failed_and_interrupted_attempts_remain_retained (expected: boundary rows; retained attempt folders) | carried from superseded-1 (`superseded-1/raw/C3-F10.log`) |
| C3-L20 | `C3L-H1` | `1dd2588fa05fd93e` | `tests/native/r19-accept-support/run_acceptance.py` | KILLED | test_rerun_index_normalizes_real_shaped_absolute_argv (expected: test_rerun_index_* tests) | carried from superseded-1 (`superseded-1/raw/C3L-H1.log`) |
| C3-L20 | `C3L-R1j-a` | `bf5e527ae3ebddf0` | `tests/native/r19-accept-support/run_acceptance.py` | KILLED | test_rerun_index_names_the_missing_top_level_key_itself (expected: test_rerun_index_* tests) | carried from superseded-1 (`superseded-1/raw/C3L-R1j-a.log`) |
| C3-L20 | `C3L-R1j-b` | `e4135172b2093b6f` | `tests/native/r19-accept-support/run_acceptance.py` | KILLED | test_rerun_index_fails_when_any_required_field_is_missing (expected: test_rerun_index_* tests) | carried from superseded-1 (`superseded-1/raw/C3L-R1j-b.log`) |
| C3-L20 | `C3L-H3` | `0499ff55f12bb6ee` | `tests/native/r19-accept-support/run_acceptance.py` | KILLED | test_rerun_index_still_refuses_host_path_outside_argv (expected: test_rerun_index_* tests) | carried from superseded-1 (`superseded-1/raw/C3L-H3.log`) |
| C3-L21a | `C3L-R1k` | `cf91e33ac2138816` | `tests/native/r19-accept-support/run_acceptance.py` | KILLED | test_limits_command_requires_exact_closed_diagnostic (expected: test_limits_command_*) | carried from superseded-1 (`superseded-1/raw/C3L-R1k.log`) |
| C3-L21a | `C3L-H4` | `481f5dd1b2ff754e` | `tests/native/r19-accept-support/run_acceptance.py` | KILLED | test_limits_command_accepts_real_error_envelope (expected: test_limits_command_*) | carried from superseded-1 (`superseded-1/raw/C3L-H4.log`) |
| C3-L21b | `C3L-R1l-old` | `9e0f66dce3f186c3` | `tests/native/r19-accept-support/run_acceptance.py` | KILLED | test_packet_command_records_literal_source_lines (expected: test_packet_command_records_literal_source_lines) | carried from superseded-1 (`superseded-1/raw/C3L-R1l-old.log`) |
| C3-L21c | `C3L-R1m-a` | `775347457a093b2a` | `tests/native/r19-accept-support/run_acceptance.py` | KILLED | test_index_lists_only_host_lines_and_refuses_out_dir_other_than_work_cwd (expected: test_index_* tests) | carried from superseded-1 (`superseded-1/raw/C3L-R1m-a.log`) |
| C3-L21c | `C3L-R1m-b` | `ec01ebcec7c6208b` | `tests/native/r19-accept-support/run_acceptance.py` | KILLED | test_index_lists_only_host_lines_and_refuses_out_dir_other_than_work_cwd (expected: test_index_* tests) | carried from superseded-1 (`superseded-1/raw/C3L-R1m-b.log`) |
| C3-L21c | `C3L-H8` | `c75399dbbac3794a` | `tests/native/r19-accept-support/run_acceptance.py` | KILLED | test_index_escapes_nul_next_to_host_path_in_binary_raw_file (expected: test_index_* tests) | carried from superseded-1 (`superseded-1/raw/C3L-H8.log`) |
| C3-L21c | `C3L-H9` | `dea160c8885d41b7` | `tests/native/r19-accept-support/run_acceptance.py` | KILLED | test_index_dry_run_refuses_unredactable_raw_file_and_writes_nothing (expected: test_index_* tests) | carried from superseded-1 (`superseded-1/raw/C3L-H9.log`) |
| C3-L21c | `C3L-H10` | `3160d4914b903dfb` | `tests/native/r19-accept-support/run_acceptance.py` | KILLED | test_index_dry_run_reports_expected_count_and_writes_nothing (expected: test_index_* tests) | carried from superseded-1 (`superseded-1/raw/C3L-H10.log`) |
| C3-L21d | `C3L-R1n` | `557787903ddb49a6` | `tests/native/r19-accept-support/run_acceptance.py` | KILLED | test_scan_command_fails_on_host_path_in_committed_evidence (expected: test_scan_command_fails_on_host_path_in_committed_evidence) | carried from superseded-1 (`superseded-1/raw/C3L-R1n.log`) |
| C3-L21e | `C3L-R1o` | `f40fba4614d3d420` | `tests/native/r19-accept-support/run_acceptance.py` | KILLED | test_verify_redaction_runs_standard_tool_and_rejects_scratch_tokens_in_manifest (expected: test_verify_redaction_runs_standard_tool_and_rejects_scratch_tokens_in_manifest) | carried from superseded-1 (`superseded-1/raw/C3L-R1o.log`) |
| C3-R2a | `C3-R2a` | `4a1f3bf7d3b3e6c7` | `tests/native/r19-accept-support/run_acceptance.py` | KILLED | test_commit_list_check_fails_naming_a_missing_path (expected: test_commit_list_check_fails_naming_a_missing_path; test_commit_list_check_accepts_a_list_of_existing_sorted_files) | carried from superseded-1 (`superseded-1/raw/C3-R2a.log`) |
| C3-R2a | `C3-R2a-acc` | `5ea34c1cf83c61f4` | `tests/native/r19-accept-support/run_acceptance.py` | KILLED | test_commit_list_check_accepts_a_list_of_existing_sorted_files (expected: test_commit_list_check_fails_naming_a_missing_path; test_commit_list_check_accepts_a_list_of_existing_sorted_files) | carried from superseded-1 (`superseded-1/raw/C3-R2a-acc.log`) |
| C3-R2b | `C3-R2b-a` | `d33dde4ec6b1b06b` | `tests/native/r19-accept-support/run_acceptance.py` | KILLED | test_commit_list_check_refuses_unsorted_duplicate_raw_and_bytecode_paths (expected: commit-list rule tests) | carried from superseded-1 (`superseded-1/raw/C3-R2b-a.log`) |
| C3-R2b | `C3-R2b-b` | `b46b3cad344f6105` | `tests/native/r19-accept-support/run_acceptance.py` | KILLED | test_commit_list_check_refuses_unsorted_duplicate_raw_and_bytecode_paths (expected: commit-list rule tests) | carried from superseded-1 (`superseded-1/raw/C3-R2b-b.log`) |
| C3-R2b | `C3-R2b-c` | `b8a8e560d669cc9b` | `tests/native/r19-accept-support/run_acceptance.py` | KILLED | test_commit_list_check_refuses_unsorted_duplicate_raw_and_bytecode_paths (expected: commit-list rule tests) | carried from superseded-1 (`superseded-1/raw/C3-R2b-c.log`) |
| C3-R2b | `C3-R2b-d` | `77f4d2da43a1c8c8` | `tests/native/r19-accept-support/run_acceptance.py` | KILLED | test_commit_list_check_refuses_unsorted_duplicate_raw_and_bytecode_paths (expected: commit-list rule tests) | carried from superseded-1 (`superseded-1/raw/C3-R2b-d.log`) |
| C3-R2b | `C3-R2b-e` | `b163010328e1b804` | `tests/native/r19-accept-support/run_acceptance.py` | KILLED | test_commit_list_check_refuses_unsorted_duplicate_raw_and_bytecode_paths (expected: commit-list rule tests) | carried from superseded-1 (`superseded-1/raw/C3-R2b-e.log`) |
| C3-R2b | `C3-R2b-f` | `73fd24a590ade922` | `tests/native/r19-accept-support/run_acceptance.py` | KILLED | test_commit_list_check_refuses_snapshot_original_and_unlisted_superseded_paths (expected: commit-list rule tests) | carried from superseded-1 (`superseded-1/raw/C3-R2b-f.log`) |
| C3-R2b | `C3-R2b-g` | `3b0651ec62d1cc79` | `tests/native/r19-accept-support/run_acceptance.py` | KILLED | test_commit_list_check_refuses_snapshot_original_and_unlisted_superseded_paths (expected: commit-list rule tests) | carried from superseded-1 (`superseded-1/raw/C3-R2b-g.log`) |
| C3-R2c, R2f | `C3-R2c` | `ca8b741634828b72` | `tests/native/r19-accept-support/run_acceptance.py` | KILLED | test_commit_list_check_refuses_files_that_pin_report_hashes (expected: test_commit_list_check_refuses_files_that_pin_report_hashes) | carried from superseded-1 (`superseded-1/raw/C3-R2c.log`) |
| C3-R2c, R2f | `C3-R2c-b` | `1d94c82cbd4f1d28` | `tests/native/r19-accept-support/run_acceptance.py` | KILLED | test_commit_list_check_refuses_files_that_pin_report_hashes (expected: test_commit_list_check_refuses_files_that_pin_report_hashes) | carried from superseded-1 (`superseded-1/raw/C3-R2c-b.log`) |
| C3-R3a, R3c, L17 | `C3-F11-b` | `2883873368fa0c9d` | `tests/native/r19-accept-support/fault_driver.py` | KILLED | test_trace_is_derived_from_executed_steps_in_plain_cmake_run (expected: test_trace_is_derived_from_executed_steps_in_plain_cmake_run) | carried from superseded-1 (`superseded-1/raw/C3-F11-b.log`) |
| C3-G6 | `C3L-A3-a` | `3d030dad506c8603` | `tests/native/r19-accept-support/run_acceptance.py` | KILLED | test_suites_command_refuses_skips_failures_and_zero_matched (expected: test_suites_command_refuses_skips_failures_and_zero_matched) | carried from superseded-1 (`superseded-1/raw/C3L-A3-a.log`) |

Unassigned runs: none.

## Survivors and findings

- `C3-R1m-c` was a SURVIVOR in the superseded probe run (`superseded-1/`); the repair added `HeavyDriverTest.test_run_heavy_exits_nonzero_when_the_trajectory_is_present_but_invalid` and the focused re-probe now KILLS it (see Focused re-probe). Superseded finding kept in `superseded-1/PROBES.md`.

- **`C3-R1a2-nl`**: SCHEMA612 passes with the trailing newline dropped (exit 0) and `cmp -s` against the old evidence copy exits 1, so the one-byte drift is observed ONLY by the RECORD `cmp -s`/shasum check, as the table states. The tracked `SchemaFixtureContextTest.test_schema_fixture_is_listed_not_ignored_and_matches_the_evidence_copy` compares bytes only while the old copy exists, so once the old copy is deleted nothing automatic observes drift. Limit, not a defect of this round.

- **RECORD kills** (`C3-R1a2-nl`, `C3-R1j-*`, `C3-R1k-*`, `C3-R1q-*`) have no failing test name: the kill is the shell observer's exit or count differing from `raw/BASELINE.log` (grep exit 1, count 0, four record greps all 4). They are marked RECORD kills as the invariant table defines them, not test kills.

- **Run-level finding (not a mutant)**: the record (section 'Heavy run status') and `runs/STOPPED-HARNESS-HEAVY.md` state that the C03 heavy run completed with `trajectory.valid` true but the counter check is NOT MET (`counter_lines` 7 against `intended_executions` 8), so `rerun-index` refuses it and `runs/cmake-rerun/index.json` (L20) and the L21 post-run steps are NOT MET. The acceptance clause 'runs/cmake-rerun/index.json exists and L20/L21 hold' is therefore not satisfied at probe time. The unit observers detect a counter increase on replay (`C3-F17` killed) but no unit test pins the counter formula that produced 7 against 8. Red-first proposal: after the coordinator decision on the formula (cause in STOPPED-HARNESS-HEAVY.md), a `FaultLiveTest` or `accept_test` case that feeds the scenario counter computation the observed 7 and 8 and expects the corrected result.

- All `C3-F*`, `C3-L14` and `C3L-*` rows are killed by the named test. The C02 survivors F11-b and L17 (same diff) are now KILLED by `test_trace_is_derived_from_executed_steps_in_plain_cmake_run` (`C3-F11-b`).

## Table 2. Red-first (every test added in this round lineage and the mutant that makes it fail)

| test added | mutant that makes it fail | outcome |
|---|---|---|
| HeavyDriverTest.test_run_heavy_exits_nonzero_when_the_scenario_stopped | C3-R1l-a, C3-R1l-b, C3-R1l-c, C3-R1l-d | KILLED |
| HeavyDriverTest.test_run_heavy_exits_zero_and_writes_result_for_a_completed_scenario | C3-R1m | KILLED (re-run) |
| HeavyDriverTest.test_run_heavy_exits_nonzero_when_the_trajectory_is_present_but_invalid (new, repair) | C3-R1m-c, C3-R1m-d | KILLED (run, `raw/C3-R1m-c.log`, `raw/C3-R1m-d.log`) |
| HeavyDriverTest.test_run_heavy_config_overlays_the_fixture_and_ledger_cmake | C3-R1n-a, C3-R1n-b, C3-R1n-c | KILLED |
| HeavyDriverTest.test_export_overlay_copies_working_tree_bytes_of_the_named_paths | C3-R1o-a, C3-R1o-b, C3-R1o-c | KILLED |
| HeavyDriverTest.test_export_overlay_refuses_a_path_missing_from_the_working_tree | C3-R1p | KILLED |
| AcceptanceCliTest.test_commit_list_check_accepts_a_list_of_existing_sorted_files | C3-R2a-acc (success path prints no entry count); note C3-R2b-b does NOT fail this test | KILLED |
| AcceptanceCliTest.test_commit_list_check_fails_naming_a_missing_path | C3-R2a | KILLED |
| AcceptanceCliTest.test_commit_list_check_refuses_unsorted_duplicate_raw_and_bytecode_paths | C3-R2b-a, -b, -c, -d, -e | KILLED |
| AcceptanceCliTest.test_commit_list_check_refuses_snapshot_original_and_unlisted_superseded_paths | C3-R2b-f, C3-R2b-g | KILLED |
| AcceptanceCliTest.test_commit_list_check_refuses_files_that_pin_report_hashes | C3-R2c, C3-R2c-b | KILLED |
| FaultLiveTest.test_trace_is_derived_from_executed_steps_in_plain_cmake_run (C02 survivor, repaired) | C3-F11-b | KILLED |
| x08.linux_recipe_pins extensions (check_runner, check_schema_assertion, check_dockerignore) | C3-R1f-a/b, C3-R1g, C3-R1g-b, C3-R1h-a/b/c, C3-R1i-a/b/c/d | KILLED |
| SchemaFixtureContextTest.* in accept_test.py (clean-export, fixture-listed, testing-doc, ignored-context mirror) | not probed in the scratch copy: they read git state of the scratch checkout, whose .git is not a faithful export. Their invariants are probed by the shell observers C3-R1b, C3-R1a2-nl, C3-R1k-a/b, C3-R1q-a/b | NOT PROBED (listed) |

No survivor among the tests added after the focused re-probe. Added since revision 1: `HeavyDriverTest.test_run_heavy_exits_nonzero_when_the_trajectory_is_present_but_invalid` killed by C3-R1m-c, C3-R1m-d (and also fails under C3-R1l-a/b/c/d).

## Skipped and not probed

- Exploratory mutants run beyond the table rows (limit 20): `C3-R1m-c`, `C3-R1h-c`, `C3-R1i-c`, `C3-R1i-d`, `C3-R1o-c`, `C3-R1n-c`, `C3-R1l-c`, `C3-R1l-d`, `C3-R1g-b`, `C3-R1q-b`, `C3-R2b-b`..`C3-R2b-g`, `C3-R2c-b`, `C3-H0-b`, `C3-R2a-acc`. Slightly above 20 only if the R2b/R2c sub-rows are counted as exploratory; they are rows of the C3-R2b and C3-R2c table entries.

- Not mutated (RECORD, run-level or guard rows; no code to mutate or the writing package is not in the tree): C3-R1c (r1/RED.md: its export-list hashes equal `r1/raw/export-list-before.sha256` and `export-list-after.sha256`, failure at `format.t.cpp:115`, post-fix pass; the red run was not re-executed), C3-R1d (`git diff --numstat HEAD -- src`: CMakeLists 1/1, execution.t.cpp 177/0, storage.cpp 8/4; storage.cpp and execution.t.cpp hashes equal the pre-fix values in `r1/RESULT.md`), C3-R1e (old copy hash `730e38be...` unchanged, `cmp` exit 0 with the fixture), C3-H1 (the exit-code half is killed by C3-R1l; the run itself is NOT MET, see findings), C3-H10, C3-S1, C3-D1, C3-H11, C3-L0, C3-L21f, C3-R2d, C3-R2e, C3-R2g, C3-R2h, C3-R2i, C3-R2j (`COMMIT-LIST.txt`, `redaction/` and PROBES-listing files not present at probe time; the commit-list tool itself is probed by R2a-R2c), C3-A1..A3, C3-G1..G5, G7..G11.

- C02 rows not re-run because their mutated files are unchanged (`accept_lib.py`, `check_runner.py`, `cmake_check.py`, `quota_*`) and not tied to a C03 finding: C02 H2-a (accept_lib capture-limit predicate; also observed through the re-run `C3L-H4`), H5-H7 and the Q/L fix-wave negative fixtures; carried by citation to `tests/native/r19-accept-a-c02-evidence/probes/superseded-3/PROBES.md`, not re-verified.

- C++ product source: `storage.cpp` and `execution.t.cpp` are unchanged since C02 and have no C03 row; the only product-tree mutation probed is the CMake schema path (`C3-R1b`).

## Focused re-probe (this revision; only revision-2 raw logs are in `raw/`)

Trigger: repair of the `C3-R1m-c` survivor. The only changed file is `tests/native/r19-accept-support/accept_test.py` (sha256 `d77c7ff7...` -> `56e8a2a8...`, one test added; every other identity row equals revision 1, verified by diffing the file table). Synced into `probe-warm` with the standard rsync; unittest `accept_test` ran 61 tests OK on the unmodified scratch. Python-only mutants, so no C++ rebuild was needed. Because the test file changed, every HeavyDriverTest row was re-run in addition to `C3-R1m-c` (command `python3 -B -m unittest -v accept_test.HeavyDriverTest`, cwd `tests/native/r19-accept-support` of the scratch; each file restored by byte copy and sha256 verified). Scripts: `scratchpad/c03probe2.py`.

| row | mutant | mutant diff sha256 (16) | verdict | failing tests | log |
|---|---|---|---|---|---|
| C3-R1m (exploratory) | `C3-R1m-c` stopped = not isinstance(trajectory, dict) | see log | KILLED by named test | test_run_heavy_exits_nonzero_when_the_trajectory_is_present_but_invalid | raw/C3-R1m-c.log |
| C3-R1m (exploratory) | `C3-R1m-d` stopped accepts any non-None valid (string "true") | see log | KILLED by named test | same | raw/C3-R1m-d.log |
| C3-R1l | `C3-R1l-a` stopped returns 0 | see log | KILLED | ..._when_the_scenario_stopped, ..._trajectory_is_present_but_invalid | raw/C3-R1l-a.log |
| C3-R1l | `C3-R1l-b` result.json written only on completion | see log | KILLED | same two | raw/C3-R1l-b.log (first attempt malformed, `raw/C3-R1l-b-malformed-attempt.log`: IndentationError, not a kill, redone) |
| C3-R1l | `C3-R1l-c` stop line removed | see log | KILLED | same two | raw/C3-R1l-c.log |
| C3-R1l | `C3-R1l-d` exit 1 not 3 | see log | KILLED | same two | raw/C3-R1l-d.log |
| C3-R1m | `C3-R1m` stopped always true | see log | KILLED | test_run_heavy_exits_zero_and_writes_result_for_a_completed_scenario | raw/C3-R1m.log |
| C3-R1n | `C3-R1n-a/b/c` overlay missing / extra / lacks fixture | see logs | KILLED x3 | test_run_heavy_config_overlays_the_fixture_and_ledger_cmake | raw/C3-R1n-*.log |
| C3-R1o | `C3-R1o-a` overlay loop skipped | see log | KILLED | ..._copies_working_tree_bytes_of_the_named_paths, ..._refuses_a_path_missing_from_the_working_tree | raw/C3-R1o-a.log |
| C3-R1o | `C3-R1o-b` HEAD blob; `C3-R1o-c` fixture not written | see logs | KILLED x2 | test_export_overlay_copies_working_tree_bytes_of_the_named_paths | raw/C3-R1o-b.log, C3-R1o-c.log |
| C3-R1p | `C3-R1p` missing overlay file skipped | see log | KILLED | test_export_overlay_refuses_a_path_missing_from_the_working_tree | raw/C3-R1p.log |

Results: 14 mutants run, 0 survivors, 0 killed by an unexpected test only. Exploratory count this revision: 2 (`C3-R1m-c`, `C3-R1m-d`), within the limit of 20.

Carried forward unchanged from `superseded-1/PROBES.md` (their mutated files and test files are identical to revision 1; their raw logs are in `superseded-1/raw/`): every other row in Table 1 above, including the Catch2/ctest SCHEMA612 row `C3-R1b`, the x08 rows, the R2 commit-list rows, the live `C3-H*` and `C3L-*` rows, and the RECORD rows. Note: `accept_test.py` also hosts the AcceptanceCli/SchemaFixtureContext classes' neighbours; the other classes in it were not re-mutated because the only diff in that file is the added test (revision-1 vs revision-2 test count 65 -> 66 `def test_` lines is not directly comparable to the 35-case baseline subset; a full 61-test run passed). That is a carry-forward by judgment, listed here as a limit.

Open items from revision 1 that this re-probe does not change: the counter-formula run-level finding (heavy run NOT MET, `counter_lines` 7 vs `intended_executions` 8), the `C3-R1a2-nl` drift-detection limit, and the NOT PROBED SchemaFixtureContextTest class.
