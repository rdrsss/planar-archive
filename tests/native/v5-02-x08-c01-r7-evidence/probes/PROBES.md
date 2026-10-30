# V5-02-X08-C01-R7 mutation probes (focused re-probe after the survivor repair)

Prober run 2026-10-29, macOS Debug. The first R7 pass (six validator survivors) is preserved unchanged in `probes/superseded-1/` (PROBES.md, raw/, tools/).
This pass re-probes after the repair changed only `tests/native/v5-02-x08-support/validate.py` (sha256 `4aa57a9d...`, was `626acf10...`); no C++, CMake or acceptance file changed.
Method: persistent warm scratch copy `.../scratchpad/probe-warm` (outside the checkout, kept) synced with `rsync -a --delete --exclude build/ --exclude .git/ --exclude 'tests/native/*-evidence/'`
(checksum dry run afterwards lists no differing file; validate.py byte-equal to the checkout). Driver `tools/driver.py`, mutant list `tools/mutants8.py`: exact-text patch with anchor count asserted
(a failed anchor would read PATCH-FAILED, not killed), `ctest -R '^x08\.validator_selftest$' --no-tests=error`, byte-copy restore with sha256 check, rebuild step (no targets: Python-only rows).
Baseline before: 1 of 1 passed (`raw/BASELINE.log`); after all mutants and restores: 100% passed (`raw/POSTRUN-BASELINE.log`). Every `raw/<id>.json` has `restored` true and `rebuild_exit` 0 (19 of 19).
Each log shows the diff, its sha256, the commands, exit code 8, `x08.validator_selftest ***Failed` and the self-test line `'<mutation row>' was not refused by <gate>` (or the 1260 count line for R7-TOT-R6). Only the prober wrote into the checkout, only under `probes/`.

## Headline

- Every focused row and every validator row killed: 19 run, 19 KILLED, killing test `x08.validator_selftest` in each. All six survivors of the first pass are now killed, each by the intended new self-test row
  (stale pass_line on the last row, release-only line, extra row, repeated row, empty fields, images key empty or absent, R6 name renamed with count intact, dpkg-list fallback and unrelated-list rows).
- No survivor remains among rows run in this pass. Real-tree data checks from the first pass (stale G06i line rejected, real tree G01 to G09 PASS) are not re-run here.
- R3 (R6-I16b) stays closed as KILLED, carried from `superseded-1` (`R7-I16b`: `replacement.profile capture module ceiling checked before observed module read`; lower bound killed by it and `x08.adv_limits_cli`); profile.cpp is unchanged (sha256 below), so the carry is valid.

## Table: rows run in this pass (all in validate.py, killing test `x08.validator_selftest`)

| invariant | mutant | mutant hash | killed | killing self-test row | run or carried |
|---|---|---|---|---|---|
| I29-R7: pass_line check on every platform row, not only the first | `R7-G06-STALE-first` check only for G06a | `32eec888c0ce62c7` | YES | a stale pass_line on the last platform row | run (`raw/R7-G06-STALE-first.log`) |
| I29-R7: pass_line checked against the Linux debug log | `R7-G06-STALE-wronglog` log constant -> release log | `08e3b0de7cf68ca8` | YES | a pass_line present only in the Linux release log | run |
| G06 unexpected platform rows refused | `R7-G06-NOEXTRA` | `83ebce12b29739da` | YES | an unexpected extra platform row | run |
| G06 repeated platform rows refused | `R7-G06-NODUP` | `df37a73176453169` | YES | a repeated platform row | run |
| G06 empty branch/test_or_log/pass_line refused | `R7-G06-NOEMPTY` | `b9b9cc4ff745f4e6` | YES | an empty platform branch | run |
| G06 images record required | `R7-G06-NOIMAGES` | `5fff78492c919df6` | YES | an images key empty | run |
| I17/I26: G01 requires the R6 names | `R7-G01-R6names` | `27435a55a810ce3b` | YES | an R6 required name absent with the count intact (G01 did not refuse; only G02, G04 failed) | run |
| I31: installed clang-23 row required in dpkg-list.txt | `R7-TC-DPKG` any line accepted | `4692b50ca36bdcbb` | YES | no dpkg line and a dpkg-list without an installed clang-23 row | run |
| I29-R7: stale pass_line absent from log fails | `R7-G06-STALE` never reports | `7b5dc72b9470e65f` | YES | a platform pass_line absent from the Linux debug log | run (changed file) |
| I29-R7: self-test stale mutator not a no-op | `R7-SELF-noop` | `c945920be266acaf` | YES | a platform pass_line absent from the Linux debug log | run (changed file) |
| I29-R7: G06i row present | `R7-G06-NOROWI` | `826292fc9db9a863` | YES | platform row G06i missing | run (changed file) |
| I26-R7: 1260 total | `R7-TOT-R6` R6_ADDED_NAMES emptied (1258) | `c4c7445a8b694b76` | YES | self_test count guard (`expected PRIOR_COUNT 1227 and EXPECTED_TOTAL 1260, got 1227 and 1258`) | run (changed file) |
| I17-R7: no prior name removed | `R7-G01-NOPRIOR` | `dd4fa8f08a615bfd` | YES | a prior name missing from the inventory | run (changed file) |
| G09 stale round-record text | `R7-G09-NOSTALE` | `84061371dd22c77c` | YES | stale text present in the round record | run (changed file) |
| I29-R7: pass_line check at all | `R8-G06-NOPASSCHK` condition False | `26cd9305481c2f6b` | YES | a platform pass_line absent from the Linux debug log | run (exploratory 1) |
| G06 images object required | `R8-G06-IMG-ABSENT` | `f6d6360691824e4f` | YES | the images record absent | run (exploratory 2) |
| G06 images keys non-empty | `R8-G06-IMG-KEYS` | `41cc962891c69b3c` | YES | an images key empty | run (exploratory 3) |
| I31: only installed (ii) rows count | `R8-TC-DPKG-RC` `startswith("ii")` dropped | `0ae133eb7edea2a3` | YES | no dpkg line and a dpkg-list without an installed clang-23 row | run (exploratory 4) |
| I31: dpkg-list fallback exists | `R8-TC-DPKG-NOFALLBACK` fallback disabled | `947cee0dedd53a5e` | YES | accepted-variant check (dpkg-list.txt installed clang-23 row without a toolchain dpkg line) | run (exploratory 5) |

Exploratory mutants used: 5 of 20 (the eight brief rows and the six changed-file rows are not exploratory). The `R8-` ids are only a label for this pass.
Carried from `superseded-1/PROBES.md` (C++ rows, unchanged files): `R7-I16b` and `R7-I16b-lower` (KILLED, profile.cpp unchanged).

## Candidate identity (identity.py verbatim, `raw/00-identity.txt`; default pathspec `raw/00-identity-default.txt`)

| identity | value |
|---|---|
| HEAD | `48b8e526764a76de1392d397bb21659bf63e8b8d` |
| sha256(git diff --binary HEAD -- tests/native/v5-02-x08-support src CMakeLists.txt tests/acceptance), tracked only | `98c72551a86730f93f5c8b23464ef4c885f382464efd7d6aed4847c6cea88571` |
| combined sha256 (diff bytes, then each untracked path NUL bytes, sorted), pathspec tests/native/v5-02-x08-support src CMakeLists.txt tests/acceptance, evidence excluded | `6721925ef8a3c6002ddb16dba2352b4a65d8eed0da2df786b47602aa59ad0973` |

| file | state | sha256 |
|---|---|---|
| `CMakeLists.txt` | modified | `402c84f35ac0df395d5e3ff1d132b443fab4e9e6b90c56cc00edfacf7534f348` |
| `src/cmd/planar/bootstrap.t.cpp` | modified | `558c811d5fe670e874a5b446f1a10625f2f7ec997f95d75ed72f6b5720291b6d` |
| `src/engine/capture/host_path.cpp` | modified | `9207c2b79ecc9fce3064c865bb8e253931795a503244304efcc172a5eb8ca7d3` |
| `src/engine/capture/host_path.t.cpp` | modified | `ddafdd85acc47e0b0c400e9f19158709029fc4a988e06edf235c89c0137308e8` |
| `src/engine/runtime/commands.t.cpp` | modified | `f36e6a3cf2f52ab54ac24a0a7284109a832668d063516e959bc3592159ffa072` |
| `src/engine/runtime/external_registration.t.cpp` | modified | `22afc6e0c8f15a9dd705538c898b8bd263604f3c5da6fed673d04ba3f556b605` |
| `tests/acceptance/x08_adversarial.py` | untracked | `fe368c77f4671c102df2baaa53bcd66ad295bdd46b9a228c87702d5acb87d1be` |
| `tests/acceptance/x08_cli.py` | untracked | `c7f3bfad84b758eff8ae89de63889b15fb27517c46685a44356cdefffda52e7c` |
| `tests/acceptance/x08_e12.py` | untracked | `029fdd1cf0e6e4d37de9a78914248267a9b7fa98c507fd08afa32a11a5beffe4` |
| `tests/acceptance/x08_recipe.py` | untracked | `bb763c443c87f95a9a184eed480d742db1d9ec93ce5e22562f9ef027572b5804` |
| `tests/acceptance/x08_timeout.py` | untracked | `337e8c0ee230728c9f725e105c93456fc2c054c3cb58fe1f959d04f396c0c5c9` |
| `tests/native/v5-02-x08-support/generate.py` | untracked | `0211ae98cd8064385b628d8f89e744bbfb314de95f8489032efbc8bc364b7420` |
| `tests/native/v5-02-x08-support/validate.py` | untracked | `4aa57a9dc1e0ab32f25fa8c65ec0be309a650ec3756bc0ff9fd17f2ea0e60a4c` |
| `tests/native/v5-02-x08-support/x08.cmake` | untracked | `5c6d671fac9f016be46ccd136c529e0d3be55b8c866aa234247deb868948c7ea` |

Default pathspec (`src tests/native`): combined sha256 `6b8e55d204ebbca93f06c42001eef6983935172dc4395070d89baa8e3b3f4bd7`, tracked-diff `505630cc5d3502f763d348ddc3c80b9f38c388fe62cbd51f1e2dc2e0517b12a0`.
The first pass's src/CMake/acceptance per-file hashes are identical to the table above (only `validate.py` differs).

## Carried from `tests/native/v5-02-x08-c01-r6-evidence/probes/PROBES.md` and `superseded-1/PROBES.md`

R6 Table A rows (BUD-CONV, I08, I02, BUD-X11 to X14, BUD-I9, BUD-I15 and I15b, platform audit rows, x08 adversarial rows, R6-VAL-I26), the HP Linux rows and `R6-I16b-both` are carried unchanged: no C++, CMake or acceptance file changed in R7.
The first pass's data checks (`superseded-1/raw/DATA-real.log`, `DATA-stale-G06i.log`) were against the earlier validate.py; the gate runs under C01-R7 `raw/` cover the repaired one.

## Red-first: tests added in this lineage and the mutant that makes each fail

| test added | mutant that makes it fail | result |
|---|---|---|
| `x08.validator_selftest` (R7: G06i row, pass_line, 1260; repair rows: last-row stale, release-only, extra, repeated, empty fields, images, R6 rename, dpkg fallback) | the 19 rows above plus carried R7 rows | killed by name (run); each repaired row has its own killing mutant |
| `replacement.profile capture module ceiling checked before observed module read` | `R7-I16b`, `R7-I16b-lower` | killed by name (carried from superseded-1) |
| `x08.adv_limits_cli` | `R7-I16b-lower` | killed by name (carried); single raise masked by the redundant runtime guard, recorded earlier |
| R6 and earlier-lineage tests (`replacement.X08 C01 ...`, host_path C01, X06 D / X07 N30 budget, `x08.adv_*`, `x08.timeout_headroom_531`, `x08.linux_recipe_pins`, `x08.e*`) | carried from the R6 and X08 C01 PROBES.md | carried |

R7 adds no test name and no C++; no red-first snapshot was needed.

## Skipped

- Exploratory budget 5 of 20 used. Skipped: RelWithDebInfo and Linux container mutants (no C++ change); `x07.two_process_cas_one_winner` (X07-C02, never failed in any run here);
  mutants of docs/testing.md or roadmap numbers (G08 and G09 check anchors and stale phrases only, a recorded limit); G03/G07 validator mutants (unchanged logic);
  a mutant of the R7 gate-run evidence tree (not a code invariant).
- Not re-run: the two I16b C++ rows (profile.cpp, runtime.cpp, tests unchanged; carried).
