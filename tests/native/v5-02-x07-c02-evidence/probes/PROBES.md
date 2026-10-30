# V5-02-X07-C02 mutation probes, focused re-probe after survivor repair (Debug, macOS)

Prober: scratch copy only (`.../scratchpad/probe-warm`, outside the checkout, kept, synced by rsync, build directory kept). The first-pass probes moved to `superseded-1/` (PROBES.md and raw/, nothing deleted).
Re-probed: the three named survivors E1, E5, E7, plus every row whose mutated file or whose killing test file changed in the repair. The repair changed only test files: `src/engine/capture/admission.t.cpp` (new TEST_CASE `capture admission verify refusal poisons the capability even when the change reverts`) and `src/engine/runtime/external_registration.t.cpp` (new TEST_CASE `C3 B2 appended observations file refuses verify and stays refused`). No product file changed (`admission.cpp`, `runtime.cpp` hashes identical to the first pass). Rows that depend on admission.t.cpp were therefore re-run (I1 to I5, I9, I17, I23, E2, E3, E4); rows whose mutated file and killing tests are in unchanged files were carried forward (I6 to I8, I9b, I12a, I12b, I15, I15b, E6).
Method (driver `raw/run.py`): exact-text single-occurrence patch (a miss is NOT-APPLIED, never a kill); `cmake --build build/rewrite-debug --target planar_capture_admission_tests planar_ledger_acceptance_tests planar_runtime_tests` (CLI rows also `planar_command_tests planar planar_x07_variant_3 planar_x07_variant_4`); `ctest -R 'C3 B2 appended observations|capture admission|X07 C02|C4 B2|X07 adapter redacts|X07 command sources|x07.ledger_commit|x07.receipt_races|x07.final_race|x07.two_process' --no-tests=error --timeout 300 -j4`; byte-copy restore with sha256 check (all 16 matched, `98bf2085...e6e887` for admission.cpp) and rebuild of the same targets after each row.
Baseline on the synced candidate: 58 of 58 pass (`raw/baseline-build.log`, `raw/baseline-ctest.log`); final run after the last restore: 58 of 58 pass (`raw/final-ctest.log`). Limit: Debug, macOS only; RelWithDebInfo and Linux not probed. Note: ctest names in the logs carry a `replacement.` prefix for the unit tests.

## Candidate identity (identity.py verbatim, `raw/00-identity.log`)

| identity | value |
|---|---|
| HEAD | `f78f8eb47267ab9c72efacc07708e7c0c2669840` |
| sha256(git diff --binary HEAD -- src tests/acceptance), tracked only | `b4a0bf1878d52940a43d27b5b37216cf71aedd51d33324055d8a6ac3d38d590e` |
| combined sha256 (diff bytes, then each untracked path NUL bytes, sorted), pathspec src tests/acceptance, evidence excluded | `b4a0bf1878d52940a43d27b5b37216cf71aedd51d33324055d8a6ac3d38d590e` |

Per-file sha256 table is in the log (admission.t.cpp `54908ae5...b215`, external_registration.t.cpp `e1f094d1...5523` are the two files that differ from the first pass).

## Invariant table results

All run rows killed; every carried row was killed in the first pass. Basis: run = this pass, carried = `superseded-1/PROBES.md` and `superseded-1/raw/<id>.log`.

| id | mutant sha256 (mutated file) | killed | killing test (named test hit?) | basis | log |
|---|---|---|---|---|---|
| I1 | `6cb4e2fac239a23f222a766d62ee1f5fdd248b50be6840f7aba9d29434f36d96` | yes | named: `capture admission load ledger receipt tolerates committed content changes`; also `...every other supplied file role`, X07 C02 loser prepared, X07 C02 native receipts | run | raw/I1.log |
| I2 | `48486e24fb437335e1c11c22b13d8fccf8b9c35aeb51af85e7c56af0fc5be67e` | yes | named: `...refuses a replaced or removed file` (only failure) | run | raw/I2.log |
| I3 | `451d74588be49e2a721c31adf67530d700b1587170cd8857e53e8d84d079c3f1` | yes | named: `...refuses a mode change`; also new `capture admission verify refusal poisons the capability even when the change reverts` | run | raw/I3.log |
| I4 | `fa5c3ce5807c003a41121905fadee54ae23ca5dc9016736e069f5d51872e7fb7` | yes | named: `...every other supplied file role`; also C3 B2 appended observations (new), C4 B2 appended, X07 C02 history family, `capture admission retained modes and file metadata reject changes` | run | raw/I4.log |
| I5 | `cf270e1ee2d047b3d79b0f84c2bdaba48f3f1f1717de127dba563e27603f9d9b` | yes | named: `...load ledger role refuses trusted aliases and links` (only failure) | run | raw/I5.log |
| I6, I7, I8 | carried mutant sha `d401b88f09a9d74b5a460324201c1ad94e3311cbe1e6797e56e2f14dd802daff` (shared edit, hash of runtime.cpp there) | yes | named, one each: X07 C02 loser prepared..., X07 C02 prepared profile native receipts..., `x07.ledger_commit_after_admission_reports_revision_conflict` | carried (runtime.cpp, profile_acceptance.t.cpp, x07_replay.py unchanged) | superseded-1/raw/I6-I7-I8.log |
| I9 | `cc4f2e573d7fc6750da967d25087cc9a8b74a3a5c3eeead1f0bbc6c473155f03` | yes, NOT by the named CLI tests | killed by unit tests `capture admission exited traversal remains verified without false final nesting`, `capture admission root binding checks original ancestors leaf names and modes`, `capture admission root binding retains exited physical parent branches`, `capture admission load ledger receipt refuses a replaced or removed file`. `x07.receipt_races_refuse_before_authorization`, `x07.final_race_rolls_back_before_commit` and the unit `ancestor replacement preserving selected inode` still pass (planar and variants rebuilt). Table fallback applies: CLI subcases are masked by another layer, not identified (limit). | run | raw/I9.log |
| I9b | carried mutant sha `decf5723b7418a769fea7062a601344da3d682b656dbbb16fb36a74a3839a9cc` | yes | named: `X07 C02 post-preparation input change plus winner commit still refuses revision_conflict` | carried (acceptance_seam.cpp and profile_acceptance.t.cpp unchanged) | superseded-1/raw/I9b.log |
| I12a | carried mutant sha `52a80fa2efbfc80108730404d6c69c0b0ddcd6e1844ac2f10c5ae0fe88bb1832` | yes | named: `C4 B2 appended admitted ledger refuses verify before any ledger read` | carried (runtime.cpp, runtime.t.cpp unchanged) | superseded-1/raw/I12a.log |
| I12b | carried mutant sha `f31738438fcd4fb4af420b8a6f51a3303dd297e5a410505a95170949a1a5ab04` | yes | named: `X07 C02 history family ledger receipt stays strict at the built-in admission site` | carried | superseded-1/raw/I12b.log |
| I15 | carried mutant sha `20de2f393857876c7bc9bb073ab942bd1f20a287411816cde489fa22c24fe875` | yes | named: `X07 command sources add no diagnostic outside the closed table` | carried (response.cpp, bootstrap.t.cpp unchanged) | superseded-1/raw/I15.log |
| I15b | carried mutant sha `fbf4c0117b7d68fab41ea90bcd73d740ee936729cc15ca2fc42eccb75c3a3e2d` | yes | named: `X07 adapter redacts malformed trusted payloads as internal protocol` | carried | superseded-1/raw/I15b.log |
| I17 | `028d1992de9bf5c7bff918076822ff52fdbaa37b4f8ba915be8a1691b5320bf6` | yes | named: `...tolerates committed content changes`; also loser prepared, native receipts, every other supplied file role | run | raw/I17.log |
| I23 | `9e98bc2cadd6d9279d34537ba22a1f794c9e86c9cbe24ceac45de59efa2b3aa0` | yes | named: `...tolerates committed content changes`; 8 failures in total (also loser prepared, native receipts, post-preparation, mode, replaced) | run | raw/I23.log |
| I10 | none (explained row) | n/a | I4 is the unit kill; E6 (carried) kills the registry variant in-process only, so no hollow test is needed | explained + E6 | superseded-1/raw/E6.log |
| I13 | not a mutant row (PROC) | n/a | `x07.two_process_cas_one_winner` passed in baseline, all 16 mutant runs and the final run (Debug); the repeat evidence is the orchestrator's. Deterministic kills are I6, I7, I8. | not run | raw/baseline-ctest.log |
| I14, I16, I18 to I22 | process, explained or documentation rows with no mutant | n/a | not run, per table | skipped by table | |

## Exploratory mutants

First pass used 7 of 20 (E1 to E7). This pass re-ran E1 to E5 and E7 and added E5c and E5d (9 distinct ids, 2 new).

| id | mutant | result | killing test | basis | log |
|---|---|---|---|---|---|
| E1 | `verify()` failure does not set `failed` (no poisoning); sha `ddd19edc60a03946e696fcb64a116bb31623b870298d832a9d32c2be56157ca6` | KILLED (was SURVIVED) | new `capture admission verify refusal poisons the capability even when the change reverts` (only failure). The revert-then-verify sequence makes poisoning observable. | run | raw/E1.log |
| E2 | by-name recheck treats vanished name as unchanged; sha `8a613bda...887b` | killed | `...refuses a replaced or removed file`, `exited traversal remains verified...` | run | raw/E2.log |
| E3 | legacy `ledger` role identity-only; sha `5e47e7cb...d8e8` | killed | `...every other supplied file role`, C4 B2 appended, X07 C02 history family | run | raw/E3.log |
| E4 | `observation` role identity-only; sha `9023677e...99bc` | killed | `...every other supplied file role`, new C3 B2 appended observations | run | raw/E4.log |
| E5 | symlink final component allowed for `load_ledger` at the `S_ISLNK` clause; sha `3280f94d...7355` | SURVIVED, EXPLAINED equivalent | none. Three independent layers refuse a symlink final component: the `S_ISLNK` clause, the `!S_ISREG` clause of the same condition (a symlink `lstat` is not regular), and `O_NOFOLLOW` in `metadata_flags`. Removing one layer changes no behaviour. | run | raw/E5.log |
| E5c | skip the whole type check (`S_ISLNK`, `!S_ISREG`) for `load_ledger`; sha `9d49da29...9e29` | SURVIVED, EXPLAINED equivalent | none. `openat(O_NOFOLLOW)` still refuses with `path_denied`. | run | raw/E5c.log |
| E5d | E5c plus `O_NOFOLLOW` dropped from the non-directory open flags (all layers removed); sha `45a35b1a26aa9e965df258015100ce2db33ceef01e4fcb0709e2ccaf236fa4b2` | KILLED | `capture admission load ledger role refuses trusted aliases and links` (only failure). Shows the symlink half of the test is load-bearing once defence in depth is removed, which closes the E5 gap by explanation. | run | raw/E5d.log |
| E6 | load-site registry admitted as `load_ledger` | killed | X07 C02 loser prepared / native receipts / post-preparation (in-process only) | carried | superseded-1/raw/E6.log |
| E7 | extension host `observations` slot admitted as `load_ledger`; sha `4ff0f3a8b818b10a35d43dcd51baf771e68a7ab96043bd6b22a63d8bb36affd3` | KILLED (was SURVIVED) | new `C3 B2 appended observations file refuses verify and stays refused` (only failure) | run | raw/E7.log |

Skipped exploratory ideas (limit 20, 9 used): reordering of `role` assignment in `open()` (I16, equivalent), mutations in fenced files (`storage.*`, `sqlite.*`), a second masking-layer hunt for I9, RelWithDebInfo and Linux probes.

## Red-first: mutant that makes each added test fail

| added test | mutant | killed | basis |
|---|---|---|---|
| `capture admission load ledger receipt tolerates committed content changes` | I1 (also I17, I23) | yes | run |
| `capture admission load ledger receipt refuses a replaced or removed file` | I2 and E2 | yes | run |
| `capture admission load ledger receipt refuses a mode change` | I3 | yes | run |
| `capture admission content changes still refuse for every other supplied file role` | I4 (also E3, E4) | yes | run |
| `capture admission load ledger role refuses trusted aliases and links` | I5 (hard-link half), E5d (symlink half) | yes | run |
| `capture admission verify refusal poisons the capability even when the change reverts` | E1 (also I3) | yes | run |
| `C3 B2 appended observations file refuses verify and stays refused` | E7 (also E4, I4) | yes | run |
| `X07 C02 loser prepared after a winner commit refuses revision_conflict and not capture_changed` | I6 | yes | carried |
| `X07 C02 prepared profile native receipts ignore a committed ledger change` | I7 | yes | carried |
| `x07.ledger_commit_after_admission_reports_revision_conflict` | I8 | yes | carried |
| `X07 C02 post-preparation input change plus winner commit still refuses revision_conflict` | I9b | yes | carried |
| `X07 C02 history family ledger receipt stays strict at the built-in admission site` | I12b (also I4, E3) | yes | carried and run |
| `C4 B2 appended admitted ledger refuses verify before any ledger read` | I12a (also I4, E3) | yes | carried and run |

## Survivors and limits (for the reviewer)

1. No unexplained survivor remains. E5 and E5c survive and are equivalent (redundant symlink layers); E5d proves the test kills when all layers go.
2. I9: the named CLI subcases are still not the sole guard; the invariant is held by the four unit kills above (table fallback).
3. Debug configuration, macOS only; carried rows were not re-run because neither their mutated files nor their killing test files changed (all 58 focused tests, including them, pass on the candidate: `raw/baseline-ctest.log`).
