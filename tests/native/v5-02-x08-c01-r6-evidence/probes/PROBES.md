# V5-02-X08-C01-R6 mutation probes (focused re-probe, run 2)

Prober run 2026-10-29. The earlier PROBES.md, raw/ and tools/ were moved unchanged to `probes/superseded-1/` (nothing deleted).
Candidate identity is byte-identical to the superseded run (same HEAD, same diff hash, same combined hash, same per-file hashes; `raw/00-identity-full.txt` is identity.py verbatim),
so the only rows re-run are the three the brief named; every other row is carried from `superseded-1/PROBES.md` (its logs are under `superseded-1/raw/`).
Method: the warm scratch copy `.../scratchpad/probe-warm` (outside the checkout, kept) was synced with `rsync -a --delete --exclude build/ --exclude .git/ --exclude 'tests/native/*-evidence/'`;
`rsync -anc` afterwards listed no content difference. Driver `tools/driver.py` (exact-text patch with anchor count asserted, ninja of only `planar` for the Python acceptance rows and
`planar planar_runtime_tests ...` for the budget row, `ctest -R ... --no-tests=error`, byte-copy restore with sha256 check, rebuild). Unmutated baseline of the four focused test groups
after sync: 16 of 16 passed (`raw/BASELINE-refocus.log`, exit 0). Nothing in the checkout or `build/rewrite-*` was edited; the only checkout writes are under `probes/`.

## Headline

- Re-run: `R6-I16b`, `R6-I02`, `R6-BUD-I15b`. All three SURVIVED again, with the same mutant hashes as before and `restored` true, `rebuild_exit` 0 in each `raw/<id>.json`.
  They are the three honest limits, unchanged and recorded, none a closure:
  - I16b (profile.cpp:91 bound 1048576 -> 16777216 alone): equivalent in observable behaviour. A second independent 1 MiB bound in `runtime.cpp` (profile.modules registration loop, `f.bytes.size() > 1048576`) refuses the same module with the same closed resource_limit row. Proof carried from superseded-1: raising both (`R6-I16b-both-runtime`) is KILLED by `x08.adv_limits_cli`; the lower bound (`R6-I16b-lower`, 1048575) is killed by it too.
  - I02 (`--stdlib-profile` Throw -> TakeLast): equivalent. `classify_prefix` also refuses `selector_option->count() != 1` and CLI11 counts both occurrences under TakeLast, so the usage error is identical.
  - BUD-I15b (macOS pin 4742 -> 4741, looser): not detectable while the measured gap (4742) stays above the pin; the stricter 4743 is killed (carried `R6-BUD-I15`).
- All other R6 survivors remain killed by named tests as recorded in Table A (carried): BUD-CONV, I08, I08b, BUD-X12, BUD-EXCL, platform audit rows, HP-I3 and HP-I24 on Linux.

## Table R: rows run in this pass (macOS Debug)

| invariant | mutant | mutant hash | killed | killing test | run or carried |
|---|---|---|---|---|---|
| I16b | `R6-I16b` profile.cpp module bound 1048576 -> 16777216 | `9d19b2f7fb2be363` | NO (1 of 1 passed) | - | run (`raw/R6-I16b.log`); equivalent, redundant runtime.cpp guard |
| I02 | `R6-I02` --stdlib-profile Throw -> TakeLast | `9ed3c1170497f7ed` | NO (1 of 1 passed) | - | run (`raw/R6-I02.log`); equivalent, classify_prefix count check |
| BUD-I15b | `R6-BUD-I15b` macOS pin 4742 -> 4741 | `64013f08da85daa1` | NO (7 of 7 passed) | - | run (`raw/R6-BUD-I15b.log`); expected, looser pin undetectable |

## Candidate identity (identity.py verbatim, `raw/00-identity-full.txt`)

| identity | value |
|---|---|
| HEAD | `48b8e526764a76de1392d397bb21659bf63e8b8d` |
| sha256(git diff --binary HEAD -- src tests/native), tracked only | `4eca3b80e9b318a64b32064f2758a240c603f8416fe955b15be59aad83467724` |
| combined sha256 (diff bytes, then each untracked path NUL bytes, sorted), pathspec src tests/native, evidence excluded | `8177d388939ecd09ee6498002906184dd22f5c5519d13d525ec3c5d0702dedc8` |

| file | state | sha256 |
|---|---|---|
| `src/cmd/planar/bootstrap.t.cpp` | modified | `558c811d5fe670e874a5b446f1a10625f2f7ec997f95d75ed72f6b5720291b6d` |
| `src/engine/capture/host_path.cpp` | modified | `9207c2b79ecc9fce3064c865bb8e253931795a503244304efcc172a5eb8ca7d3` |
| `src/engine/capture/host_path.t.cpp` | modified | `ddafdd85acc47e0b0c400e9f19158709029fc4a988e06edf235c89c0137308e8` |
| `src/engine/runtime/commands.t.cpp` | modified | `f36e6a3cf2f52ab54ac24a0a7284109a832668d063516e959bc3592159ffa072` |
| `src/engine/runtime/external_registration.t.cpp` | modified | `22afc6e0c8f15a9dd705538c898b8bd263604f3c5da6fed673d04ba3f556b605` |
| `tests/native/v5-02-x08-support/generate.py` | untracked | `0211ae98cd8064385b628d8f89e744bbfb314de95f8489032efbc8bc364b7420` |
| `tests/native/v5-02-x08-support/validate.py` | untracked | `02f55552d639da2f2ed08dc0ecb27296ae0d59eb9461d53f6066e7ee3e427b94` |
| `tests/native/v5-02-x08-support/x08.cmake` | untracked | `5c6d671fac9f016be46ccd136c529e0d3be55b8c866aa234247deb868948c7ea` |

## Table A (carried from superseded-1; logs in superseded-1/raw/): R6 survivors, pins and the audit (macOS Debug unless stated)

| invariant | mutant | mutant hash | killed | killing test | run or carried |
|---|---|---|---|---|---|
| BUD-CONV | `R6-BUD-CONV` delete runtime.cpp `charge(s, 6 * 320 + sizeof(value))` | `b63ee4bf0ae51d1b` | yes | X08 C01 role preparation outputs-off boundary pins the request-record conversion charge | run |
| BUD-CONV | `R6-BUD-CONV-half` charge reduced to `3 * 320 + sizeof(value)` | `a2fbe0c7bd098646` | yes | same test | run |
| BUD-CONV | `R6-BUD-CONV-up` charge inflated to `9 * 320 + sizeof(value)` | `d7ea75812eeff4b6` | yes | same test | run |
| BUD-CONV (Linux 228612) | `R6-BUD-CONV` in the container | `b63ee4bf0ae51d1b` | yes | same test | run, Linux |
| I08 | `R6-DEC-max+1` decimal_id maximum raised by one | `9167fefdb18405ef` | yes | X08 C01 native decimal id facts are bounded at INT64_MAX exactly | run |
| I08b | `R6-DEC-uint64` decimal_id maximum raised to UINT64_MAX | `db44429dac7c1768` | yes | same test | run |
| I08 | `R6-DEC-max-1` decimal_id maximum lowered by one (INT64_MAX refused) | `de20ecf11d0827ff` | yes | same test | run |
| I08 | `R6-DEC-len20` length bound raised to 20 digits, compare dropped | `c7848b32bd096c88` | yes | same test | run |
| I16b | `R6-I16b` profile.cpp module bound 1048576 -> 16777216 | `9d19b2f7fb2be363` | NO | - | run; equivalent, see headline |
| I16b | `R6-I16b-lower` bound lowered to 1048575 | `c5e9ec45b75ec5d3` | yes | x08.adv_limits_cli | run |
| I16b | `R6-I16b-both` profile.cpp plus native.cpp text bounds raised | `b28783e37fcff44d` | NO | - | run; runtime.cpp still refuses |
| I16b | `R6-I16b-both-runtime` profile.cpp plus runtime.cpp:3288 raised | `0078a8d16fdebd40` | yes | x08.adv_limits_cli | run; proves the redundant guard |
| I02 | `R6-I02` --stdlib-profile Throw -> TakeLast | `9ed3c1170497f7ed` | NO | - | run; equivalent, see headline |
| BUD-X12 (macOS slack closed by the 4742 pin) | `R6-BUD-X12` delete snapshot/directories VM charge | `163d7a7af9b73d21` | yes | X06 D C02 role preparation charges enumeration and registry reads to the factory budget; X07 N30 role capture charges the factory budget owner; X08 C01 role preparation outputs-off boundary pins the request-record conversion charge | run |
| BUD-EXCL | `R6-BUD-EXCL` delete exclusion-list charges | `0a2690e65b24e856` | yes | X06 D C02 ...; X07 N30 ... | run |
| BUD-X13 | `R6-BUD-X13` capture-outputs forced off | `b53be4a416c2db40` | yes | X06 D C02 ...; X07 N30 ... | run |
| BUD-X14 | `R6-BUD-X14` charge_native charges nothing | `992324186669ee9a` | yes | X08 C01 role preparation outputs-off boundary pins the request-record conversion charge | run (the focused set now kills it; before only the wide run did) |
| BUD-X11 | `R6-BUD-X11` delete tree enumeration charges | `35bb096d9ab4d5a6` | yes | X06 D C03 role preparation charges tree enumeration and registry and observation reads to the factory budget | run |
| BUD-I9 (fixed-root pin) | `R6-BUD-I9` remove the mkdtemp-name padding | `0d6b4aa028b7b9d4` | yes | X08 C01 the C02 budget fixture root has a fixed canonical byte length; X06 D C02 ...; X07 N30 ...; X08 C01 role preparation outputs-off boundary ... | run |
| BUD-I15 | `R6-BUD-I15` macOS pin 4742 -> 4743 | `dfed696c97dc053e` | yes | X06 D C02 ...; X07 N30 ... | run |
| BUD-I15b | `R6-BUD-I15b` macOS pin 4742 -> 4741 (looser) | `64013f08da85daa1` | NO | - | run; expected, not detectable |
| platform audit | `R6-AUD-extra-branch` second `#if defined(__APPLE__)` in external_registration.t.cpp | `15e4a6f4faa05bcd` | yes | X08 platform branches are exactly the audited set | run |
| platform audit | `R6-AUD-row-count` audit row 1 -> 2 | `8f1757626f934ec0` | yes | same test | run |
| platform audit | `R6-AUD-row-missing` audit row removed (the run 5 failure) | `619c9fdedccacc15` | yes | same test | run |
| platform audit | `R6-AUD-HP` `#ifdef __linux__` around the host_path.cpp pre-check call | `83e2dcf2073f8616` | yes | same test | run |
| x08 adversarial (file touched) | `R6-I15B` quota 10000 -> 10001 | `7aae8d18ebe77b26` | yes | x08.adv_job_mix_and_phase_failures | run |
| x08 adversarial | `R6-I14` same-inode refusal dropped | `bccfbc18622b43bd` | yes | x08.adv_hardlink_module_refuses_before_chunk | run |
| x08 adversarial | `R6-I15C` registry-role colocation refusal widened | `e062d28bb9cc54ad` | yes | x08.adv_registry_colocation_binary | run |
| I26 (validate.py changed) | `R6-VAL-I26` EXPECTED_TOTAL pinned at 1255 | `673b487b5de7c885` | yes | x08.validator_selftest | run |

Carried-forward HP rows, Linux run in the container (the HP-I3 and HP-I24 requirement; Linux Debug):

| invariant | mutant | mutant hash | killed | killing test | run or carried |
|---|---|---|---|---|---|
| I3 | `HP-I3` skip symlink prefixes in the pre-check | `4217fcc09a011018` | **yes (Linux)** | replacement.host path maps a regular file followed by parent traversal to the root directory error | run, Linux |
| I24 | `HP-I24` pre-check stops skipping "." components | `fdcfd88174ae60f6` | **yes (Linux)** | same test | run, Linux |
| I1a | `HP-I1a` wrong message | `eca257654f2313dd` | yes (Linux) | same test | run, Linux (macOS carried from C01 probes) |
| I1b | `HP-I1b` pre-check call deleted | `26ba54394d7206ac` | yes (Linux) | same test; host path sanitizes invalid file traversal and missing parents | run, Linux |
| I2b | `HP-I2b` pre-check widened | `2aad895602f6506e` | yes (Linux) | host path keeps missing and unresolvable directory spellings as input path errors; host path sanitizes invalid file traversal and missing parents | run, Linux |
| I25 | `HP-I25` pre-check after canonical | `cb62a2bb471f1ab9` | yes (Linux) | same test as I1a | run, Linux |
| I1a, I1b, I2a, I2b, I4, I5, I7, I25 on macOS | see `tests/native/v5-02-x08-c01-evidence/probes/PROBES.md` | - | yes (I2a equivalent) | - | carried from the C01 probes (host_path.cpp and host_path.t.cpp unchanged) |


Rows in Table A for `R6-I16b`, `R6-I02` and `R6-BUD-I15b` are superseded by Table R above (same result). Every other row is carried; none of its mutated files or test files changed (identity identical).

## Red-first: every test added in this lineage and the mutant that makes it fail

| test added | mutant that makes it fail | result |
|---|---|---|
| `replacement.X08 C01 role preparation outputs-off boundary pins the request-record conversion charge` (R6) | R6-BUD-CONV, -half, -up (macOS), R6-BUD-CONV (Linux); also X12, X14, I9 | killed by name |
| `replacement.X08 C01 native decimal id facts are bounded at INT64_MAX exactly` (R6) | R6-DEC-max+1, R6-DEC-uint64, R6-DEC-max-1, R6-DEC-len20 | killed by name |
| `replacement.X08 platform branches are exactly the audited set` (R6 row for external_registration.t.cpp) | R6-AUD-extra-branch, R6-AUD-row-count, R6-AUD-row-missing, R6-AUD-HP | killed by name |
| `x08.adv_limits_cli` (R6 I16b fixture) | R6-I16b-lower, R6-I16b-both-runtime | killed by name; the single profile.cpp raise survives (redundant runtime.cpp guard) |
| `x08.e02_envelope_rows_binary` (R6 duplicate selector block) | R6-I02 | SURVIVED, equivalent (honest limit) |
| `replacement.X08 C01 the C02 budget fixture root has a fixed canonical byte length` | R6-BUD-I9 | killed by name |
| `X06 D C02 ...`, `X07 N30 ...` budget tests with the per-platform pin | R6-BUD-I15 (4743), R6-BUD-X12, R6-BUD-EXCL, R6-BUD-X13 | killed by name |
| `replacement.host path maps a regular file followed by parent traversal to the root directory error` | HP-I1a, HP-I1b, HP-I25 (macOS and Linux), HP-I3 and HP-I24 (Linux) | killed by name |
| `replacement.host path keeps missing and unresolvable directory spellings as input path errors` | HP-I2b (macOS carried, Linux run) | killed by name |
| `x08.validator_selftest` | R6-VAL-I26 (VAL-G06 carried) | killed by name |
| `x08.adv_job_mix_and_phase_failures`, `x08.adv_hardlink_module_refuses_before_chunk`, `x08.adv_registry_colocation_binary` | R6-I15B, R6-I14, R6-I15C | killed by name |
| earlier-lineage tests (`x08.timeout_headroom_531`, `x08.linux_recipe_pins`, `x08.e03_closure_separation_binary`, `x08.e07_old_goldens_unselected_profile`, `x08.e12_variant_poison_is_live`, `replacement.X08 Linux publish and metadata branches ...`, `x08.e09_history_id_binary` and the rest) | carried from `tests/native/v5-02-x08-c01-evidence/probes/PROBES.md` (X-I17, X-I20, X-I03B, X-I04b, X-I12, X-I19) | carried; `x08.e09_history_id_binary` stays unable to reach the decimal_id mutants (the CLI refuses first); the new native test covers them |

## Skipped

- Exploratory mutants in this pass: 0 extra (the three re-run rows are the brief's focused set). The earlier exploratory budget (17 of 20) is recorded in `superseded-1/PROBES.md`.
- Carried, not re-run: every row in Table A except the three above, and the HP Linux rows (container not re-run in this pass; files unchanged).
- Not run: RelWithDebInfo mutants (Debug only); `x07.two_process_cas_one_winner` is not part of any probe run (corrective X07-C02).
- Note: `tests/native/v5-02-x08-support/validate.py` counts 1258 names for the committed X08 evidence; the candidate has 1260.

## R7 note (2026-10-29): R6-I16b was disputed; re-probe done, KILLED

The C01-R6 review (finding R3) disputes the "equivalent survivor" verdicts for `R6-I16b` above. The mutant (profile.cpp:91 module byte bound 1048576 -> 16777216) was run only against
`ctest -R ^x08\.adv_limits_cli$`, the filter-miss pattern. The native test `profile capture module ceiling checked before observed module read` (`src/engine/capture/profile.t.cpp:368-377`)
calls `profile_capture::open` directly with a 1048576-byte module (accepted) and a 1048577-byte module (refused, `resource_limit`, `profile.modules[1]`, zero read attempts); `runtime.cpp`
is not on that path and `native.cpp:312` allows 16777216 for non-text reads, so the test should fail under the mutant. Until the prober re-runs `R6-I16b` with
`-R 'profile capture module ceiling|x08\.adv_limits_cli'` (or the whole capture label) in a scratch copy, the survivor rows above for `R6-I16b` and `R6-I16b-both` were NOT accepted as equivalent at that time.
Outcome (added after the R7 prober, see `tests/native/v5-02-x08-c01-r7-evidence/probes/PROBES.md`): the prober re-ran the mutant as `R7-I16b` and it was KILLED by `profile capture module ceiling checked before observed module read`; the dispute is resolved and the survivor verdict for `R6-I16b` is withdrawn. `R6-I16b-both` (profile.cpp plus native.cpp raised) remains a survivor because the redundant `runtime.cpp` guard still refuses the module.

