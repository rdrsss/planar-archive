# R20-CARRY-A mutation probes (focused re-probe after survivor repair)

Candidate identity (verbatim from the identity tool, run this round; E6 to E9 re-run, rest carried from superseded-2/PROBES.md):

| identity | value |
|---|---|
| HEAD | `dec6f178daf10e2a1219ae56dc71485d42d839ca` |
| sha256(git diff --binary HEAD -- src tests/native), tracked only | `9a891be59df3f260098733b10c70b240542b09fa3b05f923f6dfb3bf66635355` |
| combined sha256 (diff bytes, then each untracked path NUL bytes, sorted), pathspec src tests/native, evidence excluded | `c80c07a863fbca7c2cc0cae22d11356f61791d3881cc0c0dd3b6b4120428c253` |

| file | state | sha256 |
|---|---|---|
| `src/cmd/planar/CMakeLists.txt` | modified | `d38fb946166260d3af1c728b7f9170dd695d64f4c8290a1a2594cbd81005c3c7` |
| `src/engine/history/CMakeLists.txt` | modified | `6c9ced935b2bcaaf70835068c68771dc6806c8b2a05c1eb0b6ec09413e0eecfe` |
| `src/engine/ledger/CMakeLists.txt` | modified | `91b82ac0dc63be317c69c134a84f8ee4b0cbd6a51a6cd8c5bab881936d30a565` |
| `src/engine/runtime/CMakeLists.txt` | modified | `4b33303aca17b8f168422a6cae153aa9e1e8a402424503ee795e929f83d64464` |
| `tests/native/r12c2-support/generate.py` | modified | `1f5f14dcaf795ffde05d8a924be120c4140406473b1c08109961816d457a82f7` |
| `tests/native/r14-d6-c01-support/generate.py` | modified | `29f92ea411196da51f86333627ce1d6fcc51af520a42151b7c463d2d705594a9` |
| `tests/native/r14-d6-support/generate.py` | modified | `c94a47722d519e45e712b27cffa49dc3cd4b737bef52f6d6395e1e48769ea77b` |
| `tests/native/r14-d7-b-support/generate.py` | modified | `bd8f983eef9899c0748bff43dffdbb8bfe7959846683800c407703552ccec2d5` |
| `tests/native/r14f1-support/generate.py` | modified | `919a57209ad0c64adb5f807beac9e2b23b22a29c331bc167d668b8791c773755` |
| `tests/native/r14f2-support/generate.py` | modified | `82b7ae2c4f0a833e69eb391e6548f4688be70dba9b5f59c8c524275d67332a91` |
| `tests/native/v5-02-b1-support/generate.py` | modified | `461b41d6f6dd500a72198653c49584d61080f96ff4fdca904cd90cb14608ce77` |
| `tests/native/v5-02-b2-support/generate.py` | modified | `bdbc71446ed0d291c9baec6fdbee605722edfdd9b4358f724325d41b14dadd83` |
| `tests/native/v5-02-x05-a0-support/generate.py` | modified | `bf09f69c3c1289af7e7ffae4a0a42e9f35706bcd322b03dda7afaf5af3f887ac` |
| `tests/native/v5-02-x07-support/generate.py` | modified | `90870785a31dcb94e66e39c792f33e0a979b2492fbdb6706bd2e83c82a10648d` |
| `tests/native/v5-02-x08-support/x08.cmake` | modified | `cb8c31806021e022da5e9bfb373d588db1de368336583b1ddd6194fccbfba1cd` |
| `tests/native/r20-carry-a-support/classify_cmake_diff.py` | untracked | `d14fe0c09e10c3625893407ac887cb198f4a9404b85a12bd72e64dc24fb8396c` |
| `tests/native/r20-carry-a-support/common.py` | untracked | `5ef5e85f767963a19df5cdfa019926a822a550d0ce44ed9edd88529df6f176fd` |
| `tests/native/r20-carry-a-support/configure_ninja.py` | untracked | `9adff379ae84dbddf44648a8aa497c84959143cde65e5a14784f2f2419083249` |
| `tests/native/r20-carry-a-support/embed_stability.py` | untracked | `7b36f3bc9133bd98ad135f360267a4eb4f372eac501bc346a3df2ea56d5159db` |
| `tests/native/r20-carry-a-support/generator_stability.py` | untracked | `4424b1b97febd5b50f4b6bcc871b64e2d3eabc726fdff39a66925f91004c4bb1` |
| `tests/native/r20-carry-a-support/mtime_snapshot.py` | untracked | `4103a1e6a3148b2325219417c2fe481f4f06ec3cccfdb6017852aae33427deff` |
| `tests/native/r20-carry-a-support/nearest_limit.py` | untracked | `b3f224d12b01e8170169ad99be95ea65f823d8404ed7ecde28899fe52d97bd84` |
| `tests/native/r20-carry-a-support/timeout_headroom.py` | untracked | `219b327e7a447bc9d5ada3ebc2eb3a4e4b2120a2861f2a1b0b741489927b93b1` |

Previous identity for carried rows had combined sha256 `865c246f...b071ab` and `embed_stability.py` `58fc9909...`; only that file changed.

## Method

- Scratch trees outside the checkout (kept, never deleted): `<scratchpad>/probe-warm` (full Debug build of `planar_tests`, 1,872 registered tests, used for the configure/build-dependent rows I1, I2, I3, E14, E14b) and `<scratchpad>/probe-warm2` (configured only; the 15 Python/CMake-script tests in it need no C++ build; used for every other row). Each synced with `rsync -a --delete --exclude build/ --exclude .git/ --exclude 'tests/native/*-evidence/'` from the checkout.
- Driver `probes/probe-driver.py` (copy of the scratch driver). Per mutant: assert the scratch file equals the checkout file, apply the text edit (assert exactly one occurrence and a changed result; a failed apply is not a kill), record the unified diff and its sha256, for I1, I2, I3, E14, E14b run `cmake --preset debug` then `cmake --build build/rewrite-debug --target <the test executable>`, run `ctest --test-dir build/rewrite-debug -R <anchored regex> --output-on-failure`, restore the file by byte copy from the checkout and verify sha256 equality, and for build rows reconfigure and rebuild the same target. Kill = ctest exit non-zero with at least one named failing test.
- Baselines before probing: all 18 `replacement.r20.*` pass in probe-warm; 15 pass in probe-warm2. After all probes both trees were byte-compared to the checkout with `rsync -rcn` (empty) and the same suites passed again.
- The matched count was 3 (timeout rows) or 1 (single-row), shown in each raw log. Limit: the two named slow Catch2 tests were not executed, only their registration (TIMEOUT property and name) is under test by design.

## Table rows (invariant table)

37 of 37 invariant-table mutants run were killed by a named test; every kill was by the row's listed test (I3 was also killed by `timeout_headroom_above4096`, because the renamed case leaves that name unregistered; this is the distinct NOT REGISTERED message and the listed `timeout_names_registered` also fails). I12 uses the same patch as I5; the run shows `configure_zero_work` fails under it. Evidence-only rows I14 to I18, I22, I23, I25 to I28 have no mutant; I22's byte-identity was cross-checked by the E6 run.

| invariant | mutant (id: description) | diff sha256 (first 16; full in raw log) | killed | killing test | run or carried-from |
|---|---|---|---|---|---|
| I1 | I1: history CMakeLists: delete TEST_TIMEOUT 120 (planar_history_interpretation_v2_codec) | `d1ccd3887e0daada` | yes | `replacement.r20.timeout_headroom_v2_allocation` | carried from superseded-2 |
| I2 | I2: ledger CMakeLists: delete TEST_TIMEOUT 120 (planar_ledger_format) | `76748f6e47a14fb3` | yes | `replacement.r20.timeout_headroom_above4096` | carried from superseded-2 |
| I3 | I3: history_verification.t.cpp:1325 TEST_CASE string renamed by one character | `43a84de2b274272f` | yes | `replacement.r20.timeout_headroom_above4096`, `replacement.r20.timeout_names_registered` | carried from superseded-2 |
| I4 | I4: B1 generator: unconditional write for commands/runtime outputs | `65f6775e9170b4ae` | yes | `replacement.r20.generator_unchanged_mtime_b1` | carried from superseded-2 |
| I5 | I5: D6 generator: unconditional write for the three outputs | `cec46f81c42a4134` | yes | `replacement.r20.generator_unchanged_mtime_d6` | carried from superseded-2 |
| I6 | I6: D6-C01 generator: unconditional (out/name).write_text(data) | `8e9344de415f9e7c` | yes | `replacement.r20.generator_unchanged_mtime_d6_c01` | carried from superseded-2 |
| I7 | I7a: X05-A0 generator: unconditional target.write_text(generated) | `bb5edc47efd9a1d3` | yes | `replacement.r20.generator_unchanged_mtime_x05_a0` | carried from superseded-2 |
| I7 | I7b: X05-A0 generator: unconditional generation.json write | `3d8d38573aed57ea` | yes | `replacement.r20.generator_unchanged_mtime_x05_a0` | carried from superseded-2 |
| I8 | I8: B1 generator: write-if-missing | `f31adeb405d7e9a4` | yes | `replacement.r20.generator_content_change_rewrites_b1` | carried from superseded-2 |
| I9 | I9: D6 generator: write-if-missing | `5b6bf2f1ec77f71f` | yes | `replacement.r20.generator_content_change_rewrites_d6` | carried from superseded-2 |
| I10 | I10: D6-C01 generator: write-if-missing | `619fa1b7377a164f` | yes | `replacement.r20.generator_content_change_rewrites_d6_c01` | carried from superseded-2 |
| I11 | I11: X05-A0 generator: write-if-missing | `20b7be416c717ebf` | yes | `replacement.r20.generator_content_change_rewrites_x05_a0` | carried from superseded-2 |
| I12 | I12: D6 generator unconditional write (configure_zero_work) | `cec46f81c42a4134` | yes | `replacement.r20.configure_zero_work` | carried from superseded-2 |
| I13 | I13: B1 generator: write-if-missing (configure_rebuilds_on_content_change) | `f31adeb405d7e9a4` | yes | `replacement.r20.configure_rebuilds_on_content_change` | carried from superseded-2 |
| I19 | I19: embed helper: embed first with generic file(CONFIGURE), then read back, REPLACE, second file(CONFIGURE) | `a6b4167daebc3306` | yes | `replacement.r20.embed_unchanged_mtime` | re-run this round |
| I20 | I20m1: embed helper: write-if-missing | `6fe83e929d864c26` | yes | `replacement.r20.embed_content_change_rewrites` | re-run this round |
| I20 | I20m2: embed helper: ignore REMAP | `a25987b831aef88e` | yes | `replacement.r20.embed_content_change_rewrites` | re-run this round |
| I20 | I20m3: embed helper: ignore LITERAL | `cf29c00def84d5e0` | yes | `replacement.r20.embed_content_change_rewrites` | re-run this round |
| I20 | I20m4: embed helper: apply LITERAL before REMAP | `82d6539b4ce8440e` | yes | `replacement.r20.embed_content_change_rewrites` | re-run this round |
| I20 | I20m5a: embed helper: bare REMAP is an error | `a2b29dabf9a8a54b` | yes | `replacement.r20.embed_content_change_rewrites` | re-run this round |
| I20 | I20m5b: embed helper: token after REMAP is consumed as a pair even when it is LITERAL | `75d57210c25c8f0d` | yes | `replacement.r20.embed_content_change_rewrites` | re-run this round |
| I21 | I21a: audit: restore read/replace/configure triplet at runtime D6 site | `88a0d4ae7f8816bc` | yes | `replacement.r20.embed_callsite_audit` | re-run this round |
| I21 | I21b: audit: restore triplet at cmd/planar d7b site | `9ba1521e562569b5` | yes | `replacement.r20.embed_callsite_audit` | re-run this round |
| I21 | I21c: audit: restore triplet at x08.cmake site (HEAD version of x08.cmake) | `df7ac85b6bb85973` | yes | `replacement.r20.embed_callsite_audit` | re-run this round |
| I21 | I21d: audit: drop REMAP from the X05 call | `b4ae73f04caef78d` | yes | `replacement.r20.embed_callsite_audit` | re-run this round |
| I24 | I24-r12c2: manifest generator r12c2: unconditional generation.json write | `497cea61b1fee850` | yes | `replacement.r20.manifest_unchanged_mtime` | carried from superseded-2 |
| I24b | I24b-r12c2: manifest generator r12c2: generation.json write-if-missing | `d416444019620089` | yes | `replacement.r20.manifest_content_change_rewrites` | carried from superseded-2 |
| I24 | I24-r14f1: manifest generator r14f1: unconditional generation.json write | `039a90eae7600e82` | yes | `replacement.r20.manifest_unchanged_mtime` | carried from superseded-2 |
| I24b | I24b-r14f1: manifest generator r14f1: generation.json write-if-missing | `eaed773ff2d22c37` | yes | `replacement.r20.manifest_content_change_rewrites` | carried from superseded-2 |
| I24 | I24-r14f2: manifest generator r14f2: unconditional generation.json write | `dad82aef15e927c5` | yes | `replacement.r20.manifest_unchanged_mtime` | carried from superseded-2 |
| I24b | I24b-r14f2: manifest generator r14f2: generation.json write-if-missing | `6df8afa05fd5995d` | yes | `replacement.r20.manifest_content_change_rewrites` | carried from superseded-2 |
| I24 | I24-v5-02-b2: manifest generator v5-02-b2: unconditional generation.json write | `53375d55317eee9b` | yes | `replacement.r20.manifest_unchanged_mtime` | carried from superseded-2 |
| I24b | I24b-v5-02-b2: manifest generator v5-02-b2: generation.json write-if-missing | `632ffc13afcdaf4c` | yes | `replacement.r20.manifest_content_change_rewrites` | carried from superseded-2 |
| I24 | I24-r14-d7-b: manifest generator r14-d7-b: unconditional generation.json write | `7c9ddce4e4dc74b6` | yes | `replacement.r20.manifest_unchanged_mtime` | carried from superseded-2 |
| I24b | I24b-r14-d7-b: manifest generator r14-d7-b: generation.json write-if-missing | `d04b26f75cee516c` | yes | `replacement.r20.manifest_content_change_rewrites` | carried from superseded-2 |
| I24 | I24-v5-02-x07: manifest generator v5-02-x07: unconditional generation.json write | `47b2e654bdda02a1` | yes | `replacement.r20.manifest_unchanged_mtime` | carried from superseded-2 |
| I24b | I24b-v5-02-x07: manifest generator v5-02-x07: generation.json write-if-missing | `5ea0ac90271c4b86` | yes | `replacement.r20.manifest_content_change_rewrites` | carried from superseded-2 |

Notes on rows.

- I3 is expectRed=false: it is red only under its mutant (rename by one character), killed.
- I7 has the two mutants named by the table; both killed by `generator_unchanged_mtime_x05_a0`.
- I20 m5 was run in two forms: bare REMAP raises an error (m5a) and the token after REMAP is consumed as a pair even when it is LITERAL (m5b); both killed by `embed_content_change_rewrites`. m4 (LITERAL before REMAP) is killed, i.e. the order-sensitive shape S4 does its job.
- Ineffective first attempt: my first m5a patch tested `remap_pairs STREQUAL ""` unquoted, which CMake evaluates as the literal string, so the mutant never fired (`raw/I20m5a-first-attempt-ineffective-mutant.log`, `raw/I20m5a-second-attempt-ineffective-mutant.log`). That is a mutation-not-applied case, not a surviving test; the corrected mutant (`"${remap_pairs}"`) is in `raw/I20m5a.log` and is killed. Both ineffective logs are kept.
- First pass of the five build rows used a ctest regex that did not match `timeout_names_registered` (it said `timeout_headroom_...`), so only 2 of 3 tests ran; logs kept under `raw/first-pass-regex-gap/` and all five rows were rerun with the correct anchored regex.

## Extra exploratory mutants (15 of 20 allowed; E6 to E9 re-run this round)

| extra | mutant | diff sha256 | killed | killing test | note |
|---|---|---|---|---|---|
| E1 | audit: authoring call spells REMAP with a literal pair instead of ${authoring_remap} (D17a spelling) | `9eb1dfe14bcc6721` | yes | `replacement.r20.embed_callsite_audit` | re-run this round |
| E2 | audit: a LITERAL appears in a second call (d7b) | `2fe9a910809e76ca` | yes | `replacement.r20.embed_callsite_audit` | re-run this round |
| E3 | audit: drop REMAP from runtime B1 site (count 8 of 9) | `b3697af015998f9d` | yes | `replacement.r20.embed_callsite_audit` | re-run this round |
| E5 | embed helper: REMAP names files only relative to PROJECT_SOURCE_DIR (drop stdlib-relative rule) | `ea0a3e32376958f3` | yes | `replacement.r20.embed_content_change_rewrites` | re-run this round |
| E6 | embed helper: only the first REMAP pair is applied (S6 two-pair REMAP shape) | `d9eda5d2879c65e1` | yes | `replacement.r20.embed_content_change_rewrites` | RE-PROBED this run after repair (raw/E6.log) |
| E7 | embed helper: only the first LITERAL pair applied (S7 two-pair LITERAL shape) | `a7cc4ef0e8fd8a36` | yes | `replacement.r20.embed_content_change_rewrites` | RE-PROBED this run after repair (raw/E7.log) |
| E8 | embed helper: drop CMAKE_CONFIGURE_DEPENDS registration of embedded parts | `051d0af63fd9ffcf` | yes | `replacement.r20.embed_content_change_rewrites` | RE-PROBED this run after repair (raw/E8.log) |
| E9 | embed helper: planar_rewrite_module call removed | `bacad4092e9331a9` | yes | `replacement.r20.embed_content_change_rewrites` | RE-PROBED this run after repair (raw/E9.log) |
| E10 | embed helper: file(WRITE) unconditional instead of file(CONFIGURE) | `f16651a9ae6139c2` | yes | `replacement.r20.embed_unchanged_mtime` | re-run this round |
| E14 | ledger CMakeLists: TEST_TIMEOUT 119 (boundary) | `ab2b8d13e664b84e` | yes | `replacement.r20.timeout_headroom_above4096` |  |
| E14b | history CMakeLists: TEST_TIMEOUT 60 (halved) | `8697e0a527aa9c89` | yes | `replacement.r20.timeout_headroom_v2_allocation` |  |
| E16a | configure_zero_work: B1 generator unconditional | `65f6775e9170b4ae` | yes | `replacement.r20.configure_zero_work` |  |
| E16b | configure_zero_work: D6-C01 generator unconditional | `8e9344de415f9e7c` | yes | `replacement.r20.configure_zero_work` |  |
| E16c | configure_zero_work: X05-A0 generator unconditional | `5593dbb5bf8ac48b` | yes | `replacement.r20.configure_zero_work` |  |
| E17 | configure_rebuilds_on_content_change: all four generators write-if-missing | `737712c21acae542` | yes | `replacement.r20.configure_rebuilds_on_content_change` |  |

Survivor analysis (re-probe after repair).

E6, E7, E8 and E9 survived the previous run and are now killed by `replacement.r20.embed_content_change_rewrites` after the repair added shapes S6 (two-pair REMAP, failure text "S6: first configure differs from the legacy-idiom reference"), S7 (two-pair LITERAL, "S7: first configure differs ...") and the registration checks (E8 "embedded part ... is not in CMAKE_CONFIGURE_DEPENDS", E9 "embed helper did not register its module through planar_rewrite_module"). All four kills are by the named test only; the other two embed tests and the audit pass under each mutant. No survivors remain among the extra mutants. Prior notes that E6 to E9 survive are superseded (see `superseded-2/`).

## Red-first table (every test added in this round lineage)

Pre-fix red status comes from `tests/native/r20-carry-a-evidence/red/RED.md` (carried; all 18 names, 10 expectRed tests failed behaviourally pre-fix, 8 guards passed). The mutant column is the mutant that makes the test fail on the final tree (run here).

| test | pre-fix (carried from RED.md) | mutant that fails it now | run here |
|---|---|---|---|
| `replacement.r20.timeout_headroom_v2_allocation` | red `TIMEOUT 30 < 120` | I1 (also E14b) | killed |
| `replacement.r20.timeout_headroom_above4096` | red `TIMEOUT 30 < 120` | I2 (also E14, I3) | killed |
| `replacement.r20.timeout_names_registered` | guard (passed pre-fix) | I3 | killed |
| `replacement.r20.generator_unchanged_mtime_b1` | red (63 of 63 rewritten) | I4 (also E16a via configure) | killed |
| `replacement.r20.generator_unchanged_mtime_d6` | red (18 of 18) | I5 | killed |
| `replacement.r20.generator_unchanged_mtime_d6_c01` | red (3 of 3) | I6 | killed |
| `replacement.r20.generator_unchanged_mtime_x05_a0` | red (7 of 7) | I7a, I7b | killed |
| `replacement.r20.generator_content_change_rewrites_b1` | guard | I8 | killed |
| `replacement.r20.generator_content_change_rewrites_d6` | guard | I9 | killed |
| `replacement.r20.generator_content_change_rewrites_d6_c01` | guard | I10 | killed |
| `replacement.r20.generator_content_change_rewrites_x05_a0` | guard | I11 | killed |
| `replacement.r20.manifest_unchanged_mtime` | red (six generators) | I24 x6 | killed (each names its generator) |
| `replacement.r20.manifest_content_change_rewrites` | guard | I24b x6 | killed (each names its generator) |
| `replacement.r20.configure_zero_work` | red | I12 (also E16a, E16b, E16c) | killed |
| `replacement.r20.configure_rebuilds_on_content_change` | guard | I13 (B1 alone), E17 | killed |
| `replacement.r20.embed_unchanged_mtime` | red (module mtime moved) | I19 (also E10) | killed |
| `replacement.r20.embed_content_change_rewrites` | guard | I20 m1 to m5b (also E5) | killed |
| `replacement.r20.embed_callsite_audit` | red | I21a to I21d (also E1, E2, E3) | killed |

Existing tests touched by this lineage keep their names; no other test is mutated here.

## Skipped list

- Re-probe note: only E6, E7, E8, E9 were run this round (tree `probe-warm2`, synced from the checkout with the usual rsync, `rsync -rcn` empty before and after; configure-only tree, the 3 timeout tests there fail by design because no C++ build registered them, the other 15 pass; 18/18 pass in the carried probe-warm run). Because `embed_stability.py` changed, every row it hosts was also re-run: I19, I20m1 to I20m5b, I21a to I21d, E1, E2, E3, E5, E10 (all killed by the same named tests as before; raw logs in `raw/`). The remaining rows (I1 to I13, I24 family, E14, E14b, E16a to E16c, E17) are carried forward unchanged from `superseded-2/PROBES.md`, since neither their mutated files nor their tests changed (cmake/embed_lua.cmake sha256 unchanged `9f712ff4...`).
- Evidence-only invariants have no mutant: I14, I15, I16, I17, I18, I22 (cross-checked by E6), I23, I25, I26, I27, I28.
- Extra candidates not run (budget 20, used 15, but low expected value or needing a product change): removing one `add_test` registration (caught only by I15 evidence, the ctest name diff); a generator emitting an extra or missing output file (exact-set checks are hard-coded, `set mismatch` is a test bug by the file header); lowering the 1,854 floor in `timeout_names_registered` (would need editing the test, not the product); x08 multi-pair LITERAL; D6 variant-4 marker handling.
- Not applicable here: running the two slow Catch2 tests under load (I14 evidence, PKG-E).

## Files

Raw logs: `probes/raw/<id>.log` (one per run mutant: diff, sha256, commands, exit codes, failing names, restore verification), plus `raw/E6-evidence-I22-embed-bytes.log`, `raw/first-pass-regex-gap/` and the two ineffective-mutant logs. Driver: `probes/probe-driver.py`. Nothing outside `tests/native/r20-carry-a-evidence/probes/` was written in the checkout.
