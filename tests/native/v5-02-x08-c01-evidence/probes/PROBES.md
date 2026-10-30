# V5-02-X08-C01 mutation probes

Prober run 2026-10-29, macOS Debug only (`build/rewrite-debug` of the warm scratch copy). No earlier `probes/` existed under this evidence
directory, so nothing was moved to `superseded-n/`. Nothing in the checkout or in `build/` was edited; the only checkout writes are under
`tests/native/v5-02-x08-c01-evidence/probes/`.

## Headline

- Killed with a named failing test: the fixed-root pin test (`BUD-I9`), both budget tests (`BUD-X13`, `BUD-I15`; the X06 D C02 test and the X07 N30
  test fail together under both), the C03 role test for tree enumeration (`BUD-X11`), and the new host path tests T1 and T2.
- **COVERAGE FINDING (I13 halt rule, for the coordinator).** Three guarded charges survive every focused and every wide unit run (all X05..X08 unit
  tests, `-LE acceptance`): `BUD-X12` (snapshot/directories VM charge deleted), `BUD-EXCL` (exclusion-list charges deleted) and `BUD-CONV` (the
  `6 * 320 + sizeof(value)` request-record charge deleted). `BUD-X14` survives the focused set but is killed in the wide run by three other tests.
  Measured boundaries (`raw/MEASURE-boundaries.log`, Debug, fixture root N = 64): baseline gap 228586 - 223844 = **4742**; X12 gap 4740 (-2);
  EXCL gap 4740 (-2); CONV gap 4742 (shift 0: the charge is paid on both the outputs-off and the outputs-on side, so no gap test can see it);
  unpadded root (BUD-I9) gap 228916 - 224391 = 4525. The pin `c02_outputs_gap = 4737` is the Linux value (Linux measures 4737, macOS 4742), so on
  macOS it leaves **5 padding bytes of slack**. Before C01 the macOS pin was 4524 against a measured 4525 (slack 1), and X12 (-2) was killed by the
  X06 D C02 test (`tests/native/v5-02-x06-d-c02-evidence/probes/PROBES.md`, X12 KILLED). C01 therefore lost the macOS kill of X12 and EXCL, and the gap
  fixture never could kill CONV. `BUD-I15` (pin 4743, the macOS measurement + 1) is killed; `BUD-I15b` (4738) survives because 4742 >= 4738. On Linux the
  baseline gap equals the pin (4737, slack 0), so X12 and EXCL would fail there (4735), but no Linux run applied them: not demonstrated here. The I13
  rule says a conversion-charge mutant that survives every test halts the round as a coverage finding for a fresh round, without strengthening the gap
  fixture. The prober changed no test.
- Linux-only kills I1b, I3, I24 and I6 are not demonstrated: I3 and I24 survive macOS by construction; I1b is killed on macOS only through the
  `alias/file/..` and `physical/file/../file` rows (not the Linux spellings); nothing mutated the container. The unmutated Linux runs (`raw/linux4/`,
  `raw/linux5/`) pass `replacement.host path maps a regular file ...` (linux4 debug-ctest.log line 105) and the budget tests (lines 1237, 1283, 1303). The
  Linux failures at HEAD behaviour are cited in `red/README.md`.
- Survivors recorded honestly: `HP-I2a` (equivalent: `status` with `ec` set returns `file_type::none`, so `exists` is already false), `HP-I3`, `HP-I24`
  (Linux-only by construction), `X-I02`, `X-I04` (the mutated text is not in the golden jobs; `X-I04b` on an exercised literal is killed), `X-I08`,
  `X-I08b` (the e09 test asserts INT64_MAX+1 is usage, yet the `decimal_id` mutation survives, so another check yields the same usage output; not
  investigated), `X-I16b` (module byte bound raised; not investigated), `X-I21` (redundant: `Skipped` is also caught by the per-test result parse).

## Candidate identity (identity.py verbatim)

Whole-tree run (`identity.py` with no pathspec), `raw/00-identity-full.txt`:

| identity | value |
|---|---|
| HEAD | `48b8e526764a76de1392d397bb21659bf63e8b8d` |
| sha256(git diff --binary HEAD -- src tests/native), tracked only | `67384e8674ed073152d2719fce60d0b857528e1baee50e55b069722a640e549c` |
| combined sha256 (diff bytes, then each untracked path NUL bytes, sorted), pathspec src tests/native, evidence excluded | `5cdb43323d7f9b5a865c68e0bef71715f9ee5512126f1cc8f892a284d7ef9ff0` |

| file | state | sha256 |
|---|---|---|
| `src/cmd/planar/bootstrap.t.cpp` | modified | `0a4d0b4da795c8e3bf8b57c31b3446870ae5f1f951aa20608b57237bacddf6c9` |
| `src/engine/capture/host_path.cpp` | modified | `9207c2b79ecc9fce3064c865bb8e253931795a503244304efcc172a5eb8ca7d3` |
| `src/engine/capture/host_path.t.cpp` | modified | `ddafdd85acc47e0b0c400e9f19158709029fc4a988e06edf235c89c0137308e8` |
| `src/engine/runtime/external_registration.t.cpp` | modified | `33121b6ed35cfe39db9b3b4da4007404f722da0e9a16847ce43b97d5caec2346` |
| `tests/native/v5-02-x08-support/generate.py` | untracked | `0211ae98cd8064385b628d8f89e744bbfb314de95f8489032efbc8bc364b7420` |
| `tests/native/v5-02-x08-support/validate.py` | untracked | `967a41e01eae5fffd3135a6f442ef1bc32a3912c249cebfee8ffdae70e90461f` |
| `tests/native/v5-02-x08-support/x08.cmake` | untracked | `5c6d671fac9f016be46ccd136c529e0d3be55b8c866aa234247deb868948c7ea` |


Probed files (`identity.py` with the pathspecs of the C01 mutated and killing files), `raw/00-identity-probed-files.txt`:

| identity | value |
|---|---|
| HEAD | `48b8e526764a76de1392d397bb21659bf63e8b8d` |
| sha256(git diff --binary HEAD -- src/engine/capture/host_path.cpp src/engine/capture/host_path.t.cpp src/engine/runtime/external_registration.t.cpp tests/native/v5-02-x08-support), tracked only | `7e6a9378f3d05d6b3e0e7ad9cb21bab1028363294d4ee70d73e6a54256738557` |
| combined sha256 (diff bytes, then each untracked path NUL bytes, sorted), pathspec src/engine/capture/host_path.cpp src/engine/capture/host_path.t.cpp src/engine/runtime/external_registration.t.cpp tests/native/v5-02-x08-support, evidence excluded | `b6eeed464ff74c44724dea00e33d84d776bc0ccda07b2b2d095cd2ef998bd0d2` |

| file | state | sha256 |
|---|---|---|
| `src/engine/capture/host_path.cpp` | modified | `9207c2b79ecc9fce3064c865bb8e253931795a503244304efcc172a5eb8ca7d3` |
| `src/engine/capture/host_path.t.cpp` | modified | `ddafdd85acc47e0b0c400e9f19158709029fc4a988e06edf235c89c0137308e8` |
| `src/engine/runtime/external_registration.t.cpp` | modified | `33121b6ed35cfe39db9b3b4da4007404f722da0e9a16847ce43b97d5caec2346` |
| `tests/native/v5-02-x08-support/generate.py` | untracked | `0211ae98cd8064385b628d8f89e744bbfb314de95f8489032efbc8bc364b7420` |
| `tests/native/v5-02-x08-support/validate.py` | untracked | `967a41e01eae5fffd3135a6f442ef1bc32a3912c249cebfee8ffdae70e90461f` |
| `tests/native/v5-02-x08-support/x08.cmake` | untracked | `5c6d671fac9f016be46ccd136c529e0d3be55b8c866aa234247deb868948c7ea` |


## Method

Warm scratch copy `.../scratchpad/probe-warm` (outside the checkout, kept): synced with `rsync -a --delete --exclude build/ --exclude .git/ --exclude 'tests/native/*-evidence/'`,
`cmake --preset debug`, `ninja planar_runtime_tests planar_capture_host_path_tests` (1256 of 1258 names registered at baseline: the `planar_command_tests` names are discovered only
after that target is built, which the platform rows did). Unmutated baselines before and after all rows: `raw/BASELINE-initial-premutation.log`, `raw/BASELINE-final.log` (host path 10/10,
budget set 5/5, `x08.validator_selftest` 1/1). Driver `tools/driver.py` per mutant: exact-text patch (anchor count asserted; a failed anchor is PATCH-FAILED and not a kill, as happened
on the first HP-I1a attempt, re-run with a unique anchor), `ninja -C build/rewrite-debug <targets>`, `ctest -R <regex> --no-tests=error -j8 --output-on-failure`, restore by byte copy from the
checkout with sha256 compared, rebuild of the same targets. Every row restored and rebuilt (`restored` true, `rebuild_exit` 0 in each `raw/<id>.json`); the final `cmp` of every mutated file
against the checkout is clean. A row is KILLED only if a test in its named list fails (no FAILED-OTHER-TEST occurred). Definitions: `tools/mutants.py`, `tools/mutants_x.py`, `tools/mutants_w.py`
(wide re-runs, regex `X0[5-8]` with `-LE acceptance`). Per mutant: `raw/<id>.log` (unified diff, diff sha256, commands, exit codes, ctest output) and `raw/<id>.json`. Mutant hash = sha256 of the
unified diff text in its log (first 16 hex shown). Debug only. `raw/_run3-extras-aborted-bad-id.out` records X-I17 and an aborted run (an id containing `/`); the remaining rows were re-run.

## Table A: invariants (C01 plan) and findings

| invariant | mutant | mutant hash | result | killing test (name) | run or carried | note |
|---|---|---|---|---|---|---|
| I1a | `HP-I1a` pre-check throws "Explicit root must be a directory" instead of "Cannot resolve explicit root directory" | `eca257654f2313dd` | KILLED | host path maps a regular file followed by parent traversal to the root directory error | run |  |
| I1b | `HP-I1b` delete the pre-check call in host_directory (Linux-only kill for physical/file/.., file-link/.., physical/file/./..; macOS kills via alias and ../file rows) | `26ba54394d7206ac` | KILLED | host path maps a regular file followed by parent traversal to the root directory error | run | Linux-only mechanism not isolated; the macOS kill comes from the alias and `../file` rows |
| I2 | `HP-I2a` drop the !ec condition in the pre-check | `6457a7d5dc4c653b` | SURVIVED | - | run | equivalent mutant, see headline |
| I2 | `HP-I2b` widen the pre-check trigger to any non-directory prefix (ENOENT prefixes throw the root directory error) | `2aad895602f6506e` | KILLED | host path keeps missing and unresolvable directory spellings as input path errors; host path sanitizes invalid file traversal and missing parents | run |  |
| I3 | `HP-I3` skip symlink prefixes in the pre-check (file-link/.. then reaches canonical; Linux-only kill) | `4217fcc09a011018` | SURVIVED | - | run | Linux-only by construction; no Linux mutation was run |
| I4 | `HP-I4` call the pre-check from host_file as well (resolve_explicit_file must be unchanged) | `6802d84078d0dccb` | KILLED | host path sanitizes invalid file traversal and missing parents | run |  |
| I5 | `HP-I5` pre-check also throws when the ".." prefix is a symlink to a directory | `d933f8326facef5b` | KILLED | host path preserves physical parent traversal | run |  |
| I7 | `HP-I7` edit an existing literal: "Explicit root is a symbolic link target" gains a suffix | `d985756384afc855` | KILLED | host path preserves physical parent traversal; host path refuses final root aliases with all suffixes | run |  |
| I24 | `HP-I24` pre-check stops skipping "." components (prefix file/. makes status fail with ENOTDIR; Linux-only kill) | `fdcfd88174ae60f6` | SURVIVED | - | run | Linux-only by construction; no Linux mutation was run |
| I25 | `HP-I25` move the pre-check after canonical and the is_directory check (precedence) | `cb62a2bb471f1ab9` | KILLED | host path maps a regular file followed by parent traversal to the root directory error | run |  |
| I8 (platform token) | `X-I18-HP-I8` host_path.cpp pre-check call wrapped in #ifdef __linux__ (adds a platform token and removes the pre-check on macOS) | `83e2dcf2073f8616` | KILLED | X08 platform branches are exactly the audited set | run | the row regex anchored the T1 name out; T1 is red here too by construction, the audit test is the named kill |
| I9 (fixed-root pin) | `BUD-I9` remove the mkdtemp-name padding in extension_fixture (root stays at the platform default 36 bytes) | `0d6b4aa028b7b9d4` | KILLED | X06 D C02 role preparation charges enumeration and registry reads to the factory budget; X07 N30 role capture charges the factory budget owner; X08 C01 the C02 budget fixture root has a fixed canonical byte length | run | kills the pin test, and both gap tests under the unpadded root (4525 < 4737) |
| I10/I11 snapshot charge (X12) | `BUD-X12` delete the prepare_role snapshot/directories VM charge | `163d7a7af9b73d21` | SURVIVED | - | run | SURVIVED, gap 4740 vs pin 4737, see headline |
| I10/I11 snapshot charge (X12), wide | `BUD-X12-wide` delete the prepare_role snapshot/directories VM charge [wide: all unit tests matching X05..X08] | `163d7a7af9b73d21` | SURVIVED | - | run | SURVIVED |
| I12 capture-outputs ignored (X13) | `BUD-X13` capture-outputs ignored (outputs forced off) in prepare_role | `b53be4a416c2db40` | KILLED | X06 D C02 role preparation charges enumeration and registry reads to the factory budget; X07 N30 role capture charges the factory budget owner | run | both gap tests |
| I12 factory charge (X14) | `BUD-X14` factory_budget::charge_native attached sink charges nothing to the VM counter | `992324186669ee9a` | SURVIVED | - | run | survives the focused set |
| I12 factory charge (X14), wide | `BUD-X14-wide` factory_budget::charge_native attached sink charges nothing to the VM counter [wide: all unit tests matching X05..X08] | `992324186669ee9a` | KILLED | X06 D C01 retained declaration values exhaust the composed native ceiling in the declaration stage; X06 D C03 ordinary fixtures keep a quarter of every observable ceiling free; X06 D capture stage charges every sealed byte to the one native counter | run | killed by tests other than the gap tests |
| I13 request-record conversion charge | `BUD-CONV` delete the role request record-conversion charge (6 * 320 + sizeof(value)) | `b63ee4bf0ae51d1b` | SURVIVED | - | run | SURVIVED focused and wide (gap shift 0): I13 halt rule |
| I13 request-record conversion charge, wide | `BUD-CONV-wide` delete the role request record-conversion charge (6 * 320 + sizeof(value)) [wide: all unit tests matching X05..X08] | `b63ee4bf0ae51d1b` | SURVIVED | - | run | SURVIVED |
| I13 exclusion-list charges | `BUD-EXCL` delete the exclusion-list charges in the capture-outputs arm | `0a2690e65b24e856` | SURVIVED | - | run | SURVIVED, gap 4740 |
| I13 exclusion-list charges, wide | `BUD-EXCL-wide` delete the exclusion-list charges in the capture-outputs arm [wide: all unit tests matching X05..X08] | `0a2690e65b24e856` | SURVIVED | - | run | SURVIVED |
| I14 tree enumeration (X11, X11b) | `BUD-X11` delete the current-tree and base-tree enumeration charges (X11, X11b); attributed to the C03 test | `35bb096d9ab4d5a6` | KILLED | X06 D C03 role preparation charges tree enumeration and registry and observation reads to the factory budget | run | killed by the C03 test by name, not by the gap tests |
| I15 pin raised to measurement + 1 | `BUD-I15` c02_outputs_gap raised to the macOS measurement plus one (4743 versus measured 4742) | `fc74e4ad9d708886` | KILLED | X06 D C02 role preparation charges enumeration and registry reads to the factory budget; X07 N30 role capture charges the factory budget owner | run | both gap tests |
| I15 pin raised by one | `BUD-I15b` c02_outputs_gap raised by one above the pin (4738; still below the macOS measurement 4742, so expected to survive on macOS) | `2096b9b8e8ba3080` | SURVIVED | - | run | survives: macOS measures 4742 |
| I26 | `VAL-I26` validate.py EXPECTED_TOTAL left at 1255 | `673b487b5de7c885` | KILLED | x08.validator_selftest | run |  |
| I18 (G06c row) | `VAL-G06` validate.py accepts a missing platform row (G06c deleted) | `aded5945d189e770` | KILLED | x08.validator_selftest | run |  |
| I6 | public binary `replacement.cli` on Linux | n/a | not run | - | not run | Linux-only; not killable on macOS |
| I16 | report-only (measurement) | n/a | n/a | `raw/MEASURE-boundaries.log` | run | Debug at root 64 and the unpadded root only |
| I17, I19 to I23, I27 to I30 | gate or records invariants | n/a | not mutable | gate logs, validator | n/a | not probed |

## Table B: extra exploratory mutants (15 of the 20 allowed, plus X-I18-HP-I8 above)

| X08 row | mutant | mutant hash | result | killing test | run or carried | note |
|---|---|---|---|---|---|---|
| I17 | `X-I17` src/engine/CMakeLists.txt planar_runtime TEST_TIMEOUT 120 changed to 30 (reconfigure) | `818fb96e2ad52016` | KILLED | x08.timeout_headroom_531 | run |  |
| I19 | `X-I19` source_export.cpp Linux publish call loses RENAME_NOREPLACE (inactive arm on macOS) | `f61b32d7e4b567c1` | KILLED | X08 Linux publish and metadata branches keep their no-replace and path-only flags | run | inactive Linux arm on macOS; text-only mutation |
| I20 | `X-I20` Dockerfile clang-23 loses its =${LLVM_APT_VERSION} pin | `9966e66413b498f7` | KILLED | x08.linux_recipe_pins | run |  |
| I21 | `X-I21` validate.py ignores the Skipped marker | `7f8724ea3725ac84` | SURVIVED | - | run | redundant check, see headline |
| I02 | `X-I02` envelope.cpp --stdlib-profile multi_option_policy Throw changed to TakeLast | `9ed3c1170497f7ed` | SURVIVED | - | run |  |
| I08 | `X-I08` runtime.cpp decimal_id maximum raised from INT64_MAX to UINT64_MAX | `db44429dac7c1768` | SURVIVED | - | run |  |
| I08 | `X-I08b` runtime.cpp decimal_id maximum raised by one (INT64_MAX+1 accepted) | `9167fefdb18405ef` | SURVIVED | - | run |  |
| I15B | `X-I15B` semantic.lua obligation quota check raised from 10000 to 10001 | `7aae8d18ebe77b26` | KILLED | x08.adv_job_mix_and_phase_failures | run |  |
| I14 | `X-I14` admission.cpp relationships(): the same-inode refusal against other selected files dropped | `bccfbc18622b43bd` | KILLED | x08.adv_hardlink_module_refuses_before_chunk | run |  |
| I03B | `X-I03B` admission.cpp relationships(): trusted-contains-root overlap refusal dropped | `e5eeb1332de2bfe2` | KILLED | x08.e03_closure_separation_binary | run |  |
| I04 | `X-I04` render.lua requirements absent text changed by one byte | `306ff5a84a473689` | SURVIVED | - | run | mutated literal is not exercised by the six goldens |
| I04 | `X-I04b` render.lua exercised text "Questions: none. Question blockers: none." changed by one byte | `271e53d6fae3b396` | KILLED | x08.e07_old_goldens_unselected_profile | run |  |
| I15C | `X-I15C` admission.cpp relationships(): registry-role files are also refused when colocated with the trusted directory | `e062d28bb9cc54ad` | KILLED | x08.adv_registry_colocation_binary | run |  |
| I16(b) | `X-I16b` profile.cpp: per-module byte bound raised from 1048576 to 16777216 (oversize module accepted) | `9d19b2f7fb2be363` | SURVIVED | - | run |  |
| I12 | `X-I12` generate.py: poison chunk emits no assignments (poisoning a no-op); reconfigure and rebuild the variant | `917b2a72897908f2` | KILLED | x08.e12_variant_poison_is_live | run | reconfigure, variant rebuilt |

## Red-first table: every test added in this lineage and the mutant that makes it fail

| test added | mutant that makes it fail | result |
|---|---|---|
| `replacement.host path maps a regular file followed by parent traversal to the root directory error` | HP-I1a, HP-I1b, HP-I25 | killed by name |
| `replacement.host path keeps missing and unresolvable directory spellings as input path errors` | HP-I2b | killed by name (HP-I2a survives, equivalent) |
| `replacement.X08 C01 the C02 budget fixture root has a fixed canonical byte length` | BUD-I9 | killed by name |
| `replacement.X08 platform branches are exactly the audited set` | X-I18-HP-I8 | killed by name |
| `replacement.X08 Linux publish and metadata branches keep their no-replace and path-only flags` | X-I19 | killed by name |
| `x08.timeout_headroom_531` | X-I17 | killed by name |
| `x08.linux_recipe_pins` | X-I20 | killed by name |
| `x08.validator_selftest` | VAL-I26, VAL-G06 | killed by name (X-I21 survives) |
| `x08.e03_closure_separation_binary` | X-I03B | killed by name |
| `x08.adv_hardlink_module_refuses_before_chunk` | X-I14 | killed by name |
| `x08.adv_job_mix_and_phase_failures` | X-I15B | killed by name |
| `x08.adv_registry_colocation_binary` | X-I15C | killed by name |
| `x08.e07_old_goldens_unselected_profile` | X-I04b | killed by name |
| `x08.e12_variant_poison_is_live` | X-I12 | killed by name |
| `x08.e02_envelope_rows_binary` | X-I02 | SURVIVED (no kill found) |
| `x08.e09_history_id_binary` | X-I08, X-I08b | SURVIVED (no kill found) |
| `x08.adv_limits_cli` | X-I16b | SURVIVED (no kill found) |
| `x08.e01_no_discovery`, `x08.e01_registration_policy`, `x08.e07_old_w_vectors_binary`, `x08.e08_projection_vector_binary`, `x08.e10_removed_profile_replay_binary`, `x08.adv_missing_declaration_initial_admission`, `x08.adv_opaque_ordering_binary`, `x08.adv_invalid_unselected_job_refuses_all`, `x08.e12_variant_reads_identically`, `x08.e12_variant_exports_identically`, `x08.e12_variant_poison_is_live_load`, `x08.e12_variant_poison_is_live_profile_render`, `x08.e12_variant_audit`, `x08.e12_sources_and_profile_removed` | none run (extra cap and cost) | not probed |

The two budget tests named by the brief, `X06 D C02 role preparation charges enumeration and registry reads to the factory budget` and `X07 N30 role capture charges the factory budget owner`, are killed by name under BUD-I9, BUD-X13 and BUD-I15 (each of these rows lists both).
Carried-forward rows: none. The previous probes in this lineage (`tests/native/v5-02-x07-c01-evidence/probes/`) concern `storage.cpp` and `storage.t.cpp`, which this round does not modify
(neither is in `git status`); they were neither re-run nor re-attributed here. The X06 D C02 rows X12 and X13 target a test whose constant and fixture changed, so they were re-run (BUD-X12 now survives, BUD-X13 is killed).

## Skipped

- Linux-only mutants I1b (Linux spellings), I3, I24, I6: no container mutation; see headline.
- X08 plan rows I01, I01B, I03, I05, I06, I07, I09, I10, I11, I12B, I12C, I12D, I13, I14B, I14C, I15 and the G rows: not run (extra cap of 20 and cost).
- RelWithDebInfo: no probe run.
- `x07.two_process_cas_one_winner` was not part of any probe run.
