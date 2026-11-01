# V5-02-RA01-B-W2-C02 mutation probes (focused re-probe after survivor repair)

Note (review R2): carried rows cite log paths such as `raw/C02-01.log`; those logs now live in `probes/superseded-1/raw/`, so carried-row paths are relative to `probes/superseded-1/`.

Prober run of 2026-10-31 (focused re-probe). The previous `PROBES.md`, `raw/` and `results.jsonl` moved whole to `superseded-1/` (nothing deleted). The repair changed records only: `identity.py src tests/native` before the first mutant (`raw/identity-before.txt`) and after the last restore (`raw/identity-after.txt`) are byte-identical to each other AND to the superseded run (`diff` empty; combined `d56c7fbb18172967f34260855e78b9f0411c620c42ef74d717564044dd49a0ff`, tracked-only diff `d07472721b1f0bbedbaa49f363000ebebeda98ec8d9c348ccccafd161ae327ac`, HEAD `3343ad38304db57388bc7ac2438a40043eba1b7d`). Because no mutated file or test file changed, every other row of the superseded run is carried forward unchanged (tables below, copied verbatim from `superseded-1/PROBES.md`).

Scratch: warm copy `probe-warm` (outside the checkout, debug preset, kept) re-synced with `rsync -a --delete --exclude build/ --exclude .git/ --exclude 'tests/native/*-evidence/'`; incremental build had no work (`raw/baseline-build.txt`). Baseline `ctest -R "C01|RA01-B|X08 C01"` 228/228 before (`raw/BASELINE.ctest.txt`) and 228/228 after the final `planar_tests` rebuild (`raw/FINAL-BASELINE.ctest.txt`). Driver `raw/driver/drv3.py` with `m.json` (E02, E02b, E02c, C01-37r) and `m2.json` (C02-15a/b/c): exact-anchor patch (anchor occurs once, else patch-failed), build of the test executable that holds the named test, `ctest -R` of the exact name (matched 1), byte-copy restore with sha256 verified (`verified=True` in each log), and the broad fallback run when the named test survives. Outputs `raw/driver/run.out`. Platform: macOS Debug, non-root.

## Focused re-probe results (run)

| id | invariant row | mutant diff sha256 (16) | result | killing test | failing assertion lines | run or carried | log |
|---|---|---|---|---|---|---|---|
| E02 | C02-09 limit (E02: fstatat in scan_directory after listing becomes denied_name) | `57f5722b20ae62f7` | survived (named test and broad 228 pass): stated limit | - | - | run | `raw/E02.log` |
| E02b | C02-09 limit (E02b: fstatat in scan_root after listing) | `f3cd3f85d73512fd` | survived (named and broad 228 pass): stated limit | - | - | run | `raw/E02b.log` |
| E02c | C02-09 | `44cead6ef0a41350` | killed | RA01-B2 C01 observe maps a failed entry inspection after listing to cannot inspect | directory.t.cpp:355, :356, :370, :371 | run | `raw/E02c.log` |
| C01-37r | C02-37 source-guard limit (early `return error;` before the condition) | `edfb89c5d489430d` | survived (named and broad 228 pass): stated limit | - | - | run | `raw/C01-37r.log` |
| C02-15a | C02-15 (one byte appended to semantic.lua) | `ed52806a1b849cbc` | survived the pin test; hash comparison reports DIFF (`8f582987` vs pin `82bfe3dc`) | caught only by hash comparison | - | run | `raw/C02-15a.log`, `raw/C02-15-hash-comparison.log` |
| C02-15b | C02-15 (comment line in runtime.cpp) | `dd45dadf764cbefd` | survived the pin test; hash comparison reports DIFF (`b9f5bc85` vs pin `9c8c15cf`) | caught only by hash comparison | - | run | `raw/C02-15b.log`, `raw/C02-15-hash-comparison.log` |
| C02-15c | C02-15 (64 bytes Lua comment in semantic.lua) | `68c01aa6247a4404` | killed | X08 C01 role preparation outputs-off boundary pins the request-record conversion charge | external_registration.t.cpp:4595 | run | `raw/C02-15c.log` |

Notes: E02, E02b, C01-37r are the stated limits of the round record (seam forbidden by D-C01-13a and D-C02-07); the run confirms the repair's claim that they survive and that nothing else catches them. E02c remains killed (non-root, so the root-aware branches ran). C02-15 hash comparison of the four pinned files against `c02-pin.md:34-37`: all MATCH (`raw/C02-15-hash-comparison.log`, runtime.cpp `9c8c15cf`, commands.lua `f32c5be4`, schema.lua `e71fa6d9`, semantic.lua `82bfe3dc`). The four pre-listed rows were all re-run; no exploratory mutants were added in this focused pass (0 of 20 used).

Skipped list: none of the four requested rows skipped; all other rows carried from the superseded run on identical bytes.

## Carried forward from superseded-1/PROBES.md (verbatim; every row below is "carried", identical candidate bytes, except the seven rows re-run above)


Prober run of 2026-10-31 on the C02 candidate below. No `probes/` file existed before this run, so nothing was moved to `superseded-<n>/`. Only `tests/native/v5-02-ra01-b-w2-c02-evidence/probes/` was written in the checkout (the C02-06 diff-check and doc-grep logs also live under `probes/raw/`). The warm scratch copy `probe-warm` (outside the checkout, debug preset, kept in place) was synced with `rsync -a --delete --exclude build/ --exclude .git/ --exclude 'tests/native/*-evidence/'`, rebuilt incrementally (`raw/baseline-build.txt`, 470 steps), and baselined: `ctest --test-dir build/rewrite-debug --no-tests=error -R "C01|RA01-B|X08 C01" -j8` gave 228/228 passed before the first mutant (`raw/BASELINE.ctest.txt`) and 228/228 after the last restore and full `planar_tests` rebuild (`raw/FINAL-REBUILD.txt`, `raw/FINAL-BASELINE.ctest.txt`). `ctest -N -R "RA01-B2 C01"` matches 21 tests. Each mutant was run with a named-test `ctest -R`, which matched exactly 1 test (2 for `RA01-B2 capture_diagnostic`, 11 for `RA01-B2 observe`), as the `matched count` line of each log shows. A kill needs the named (or, marked `killed-by-other`, another) test in the ctest failure list.

Driver and definitions: `raw/driver/drv2.py` (C01 and C02 rows; builds the four test executables the named tests live in) and `raw/driver/drv3.py` (wave 2 rows; builds only the executable of the named test, and the full `planar_tests` aggregate before the broad fallback run when the named test survives). Mutant definitions: `raw/driver/m.json` (generator `gen2.py`, which reuses the C01 generator `gen_c01_orig.py` and rebuilds the build-clean forms against the C02 bytes), `m2.json` (C02-06, C02-15a to c), `mW2.json` (wave 2 rows from the wave 2 `m*.json`, plus `W2-78n`). Per mutant: exact-anchor replacement (anchor occurs exactly once, else `patch-failed`, not a kill; the run is chained on a successful apply), `ninja -C ... <targets>` then `ctest ... -R '<name>' --output-on-failure`, file restored by byte copy and sha256 verified (`verified=True` in every mutant log), targets rebuilt at the next mutant and fully after the last restore. Each mutant log holds the unified diff, its sha256, exact build and test commands, ctest exit code, failing test names and the failing assertion lines. Outputs: `raw/driver/run.out` (68 C01/C02 rows), `run2.out` (4), `runW2.out` (121). Platform: macOS Debug, non-root (root-aware branches ran for real).

## Candidate identity (identity.py, verbatim; identical before the first mutant and after the last restore: `raw/identity-before.txt`, `raw/identity-after.txt`, diff empty)

| identity | value |
|---|---|
| HEAD | `3343ad38304db57388bc7ac2438a40043eba1b7d` |
| sha256(git diff --binary HEAD -- src tests/native), tracked only | `d07472721b1f0bbedbaa49f363000ebebeda98ec8d9c348ccccafd161ae327ac` |
| combined sha256 (diff bytes, then each untracked path NUL bytes, sorted), pathspec src tests/native, evidence excluded | `d56c7fbb18172967f34260855e78b9f0411c620c42ef74d717564044dd49a0ff` |

Per-file table: `raw/identity-after.txt`. Against the C01 PROBES identity (combined `1f30a26e...`), the changed files are `capture.cpp`, `capture.cppm`, `invocation.cpp` (I1), `capture.t.cpp`, `authoring_observe.t.cpp`, `authoring_prompts.t.cpp`, `authoring_orchestrator.t.cpp`, `directory.t.cpp` (T1). `runtime.cpp`, `commands.lua`, `schema.lua`, `semantic.lua` are byte-identical to the C01 identity and to the `c02-pin.md:34-37` values (see C02-15).

## Result summary

193 mutants run on this candidate: 153 killed by the named test, 17 killed by another test (named below), 23 survived. Zero patch-failed (the only anchor that no longer applied, `W2-78`, was re-expressed as `W2-78n` for the C01 `seal_failure` layout). Survivors are the C02-15a and C02-15b rows that the design says only the hash comparison catches, plus rows that were already documented survivors in the C01 and wave 2 PROBES (analysis below). No new survivor appeared in the rows tied to the C02 acceptance.

## Invariant table rows (C02-01 to C02-06, C02-09, C02-15, exploratory rows tied to R1)

| id | invariant row | mutant diff sha256 (16) | result | killing test(s) | failing assertion lines | run or carried | log |
|---|---|---|---|---|---|---|---|
| C02-01 | C02-01 | `d2e2863209e7d4d3` | killed | RA01-B2 C01 capture_diagnostic post-gate failures are remapped at every native call | capture.t.cpp:1097, capture.t.cpp:1100 | run | `raw/C02-01.log` |
| C02-02 | C02-02 | `8ca92fc580711129` | killed | RA01-B2 C01 observe digest allocation failure is a bound refusal | authoring_observe.t.cpp:785, authoring_observe.t.cpp:789 | run | `raw/C02-02.log` |
| C02-03 | C02-03 | `30b424af4c87ac85` | killed | RA01-B2 C01 capture_diagnostic post-gate failures are remapped at every native call | capture.t.cpp:1100 | run | `raw/C02-03.log` |
| C02-04 | C02-04 | `77690ef146571cb7` | killed | RA01-B2 C01 observe digest allocation failure is a bound refusal | authoring_observe.t.cpp:789 | run | `raw/C02-04.log` |
| C02-35 | C02-35 | `5109460f6806d9ae` | killed | RA01-B2 C01 capture_diagnostic post-gate failures are remapped at every native call | capture.t.cpp:1101 | run | `raw/C02-35.log` |
| C02-36 | C02-36 | `af57c7dbcd2d61e6` | killed | RA01-B2 C01 observe digest allocation failure is a bound refusal | authoring_observe.t.cpp:790 | run | `raw/C02-36.log` |
| C02-05a | C02-05 | `a455a03657a139cb` | killed | RA01-B2 C01 capture_diagnostic post-gate failures are remapped at every native call | capture.t.cpp:1100 | run | `raw/C02-05a.log` |
| C02-05b | C02-05 | `bf5e2082df5cc740` | killed | RA01-B2 C01 capture_diagnostic post-gate failures are remapped at every native call | capture.t.cpp:1100 | run | `raw/C02-05b.log` |
| C02-05c | C02-05 | `b2c62f52faf80061` | killed | RA01-B2 C01 observe digest allocation failure is a bound refusal | authoring_observe.t.cpp:786, authoring_observe.t.cpp:789 | run | `raw/C02-05c.log` |
| C02-05d | C02-05 | `6c2a43f2077fd3df` | killed | RA01-B2 C01 observe digest allocation failure is a bound refusal | authoring_observe.t.cpp:786, authoring_observe.t.cpp:789 | run | `raw/C02-05d.log` |
| X-C02-a | extra | `614c105004ce154c` | killed | RA01-B2 C01 capture_diagnostic post-gate failures are remapped at every native call | capture.t.cpp:1096, capture.t.cpp:1100 | run | `raw/X-C02-a.log` |
| X-C02-b | extra | `3781dac59218487c` | killed | RA01-B2 C01 observe digest allocation failure is a bound refusal | authoring_observe.t.cpp:784, authoring_observe.t.cpp:789 | run | `raw/X-C02-b.log` |
| X-C02-c | extra | `2f2bcd623226942d` | killed | RA01-B2 C01 capture_diagnostic post-gate failures are remapped at every native call | capture.t.cpp:1100 | run | `raw/X-C02-c.log` |
| X-C02-d | extra | `843dbde0755c611c` | killed | RA01-B2 C01 observe digest allocation failure is a bound refusal | authoring_observe.t.cpp:789 | run | `raw/X-C02-d.log` |
| X-C02-e | extra | `6cd016e0be196006` | killed | RA01-B2 C01 capture_diagnostic bad_alloc refusal carries diagnostic_bytes | capture.t.cpp:1060 | run | `raw/X-C02-e.log` |
| C02-06 | C02-06 | `0258b08d54e65fa9` | killed | RA01-B2 C01 capture_diagnostic maps a post-gate add_input failure to file changed | capture.t.cpp:1050 | run | `raw/C02-06.log` |
| C02-15a | C02-15 | `ed52806a1b849cbc` | survived | - | - | run | `raw/C02-15a.log` |
| C02-15b | C02-15 | `dd45dadf764cbefd` | survived | - | - | run | `raw/C02-15b.log` |
| C02-15c | C02-15 | `68c01aa6247a4404` | killed | X08 C01 role preparation outputs-off boundary pins the request-record conversion charge | external_registration.t.cpp:4595 | run | `raw/C02-15c.log` |
| E02c | extra | `44cead6ef0a41350` | killed | RA01-B2 C01 observe maps a failed entry inspection after listing to cannot inspect | directory.t.cpp:355, directory.t.cpp:356, directory.t.cpp:370, directory.t.cpp:371 | run | `raw/E02c.log` |


Per-row notes:

- C02-01, C02-02: both digest helpers lose the Canonical alternative; T37 fails at `capture.t.cpp:1097` and `:1100`, T44 at `authoring_observe.t.cpp:785` and `:789`. Killed by the intended test.
- C02-03, C02-04 (`||` to `&&`): the literal checks still pass (both strings stay); the exact-spelling `occurrences(...) == 1` check fails at `capture.t.cpp:1100` and `authoring_observe.t.cpp:789`. Killed by the intended test.
- C02-35, C02-36 (separate `canonical_bytes` branch before the Hash branch): both literals and the old condition spelling stay, so only the exactly-one `capture_limit(` check fails (`capture.t.cpp:1101`, `authoring_observe.t.cpp:790`). Killed by the intended test.
- C02-05 ten mutants: the six table mutants (C02-01, 02, 03, 04, 35, 36) plus the four wrong-limit forms: C02-05a (capture returns `canonical_bytes`), C02-05b (capture returns `maximum + 1`), C02-05c (observe returns `diagnostic_bytes`), C02-05d (observe returns maximum 4194305). All ten killed by T37 or T44, each with the failing assertion line named above; matched count 1, equal to baseline. Pre-fix confirmation that each probe applied: every log shows a non-empty diff, a successful build and `verified=True`; a failed apply would have logged `PATCH FAILED`.
- X-C02-a, X-C02-b (Hash alternative dropped, Canonical only), X-C02-c, X-C02-d (the `error.code == "resource_limit"` conjunct dropped from the capture and observe helper) and X-C02-e (the `bad_alloc` catch of `capture_diagnostic` returns `canonical_bytes`): all killed. X-C02-e is killed by `capture_diagnostic bad_alloc refusal carries diagnostic_bytes` at `capture.t.cpp:1060`, a behavioural test, not by the source guard.
- C02-06: editing the existing literal `File changed during read` in `post_gate_failure` to `File changed during reads` is killed by `RA01-B2 C01 capture_diagnostic maps a post-gate add_input failure to file changed` (`capture.t.cpp:1050`). The literal diff check is `raw/C02-06-diff-check.log`: `diff -U0` of `capture.cpp`, `capture.cppm` and `invocation.cpp` against `base/` shows only the two predicates, their doc comments and the `capture.cppm` `@note`; the message literals that differ are only `Hash allocation failed` and `Canonical allocation failed` appearing in the predicates (no new literal text), and `resource_limit` in the predicates. Note: the `base/` copies of the five T1 test files are not pre-C02 bytes (finding 1 below).
- C02-09 (R2): E02c re-run, killed by `RA01-B2 C01 observe maps a failed entry inspection after listing to cannot inspect` (`directory.t.cpp:355`, `:356`, `:370`, `:371`). Root-aware, ran as non-root.
- C02-15 boundary pin: the candidate hashes of `runtime.cpp` (`9c8c15cf`), `commands.lua` (`f32c5be4`), `schema.lua` (`e71fa6d9`), `semantic.lua` (`82bfe3dc`) equal `c02-pin.md:34-37` (`raw/C02-15-hash-comparison.log`). (a) one byte appended to `semantic.lua` and (b) one comment line in `runtime.cpp` are not caught by the pin test (both survive; the pin window is plus or minus 8) but the comparison reports a different sha256 (`8f582987` and `b9f5bc85` against `82bfe3dc` and `9c8c15cf`), so only the gate hash comparison catches them, as designed. (c) 64 bytes of Lua comment appended to `semantic.lua` is killed by the pin test `X08 C01 role preparation outputs-off boundary pins the request-record conversion charge` at `external_registration.t.cpp:4595`.
- C02-07, C02-10 to C02-14, C02-16 to C02-19, C02-32, C02-33: records and comments, not mutation rows. Informational greps are in `raw/C02-doc-greps.log` (the phrase `only other post-gate` has no hit; `Canonical allocation failed` appears in `plan/DECISIONS.md` lines 25, 35, 40, 83, 87, 88 and in the C01 round record; `shows equality` remains only in the quoted R4 finding row, exempt class E2; no placeholder text in `c02-pin.md`). Final grep verdicts belong to the gate stage.

## C02-37 carry-forward ledger

No earlier row is carried on this candidate. All five T1 test files and `capture.cpp`, `capture.cppm`, `invocation.cpp` changed since the C01 identity, so by the D-C02-25 rule every row whose named test lives in one of them is RE-RUN; every C01 and wave 2 row has a named test in `capture.t.cpp`, `directory.t.cpp`, `authoring_observe.t.cpp`, `authoring_prompts.t.cpp`, `authoring_orchestrator.t.cpp`, or a file that holds one of them, and was re-run. The required names were all re-run and are in the tables: C01-35 (`C01-35a`, `C01-35br`, `C01-35b-call`, `C01-35b-body`), C01-36 (`C01-36ar`, `C01-36br`), C01-37 (`C01-37r`, `C01-37-call`, `C01-37-body`), C01-44 (`C01-44r`), E06r, E08r, C01-20, C01-21, C01-45 (a to c), W2-06n, and the directory rows (E02c killed; E02 and E02b are stated limits, survived).

Forms not re-run, with reason: the original build-failed forms (`C01-35b`, `C01-36a`, `C01-36b`, `C01-37`, `C01-44`, `E06`, `E08` in C01; `W2-04`, `W2-45`, `W2-49` in wave 2) were replaced by their build-clean forms in the previous lineage and are not re-created; `W2-51` and `W2-51b` have the same diff hash as `C01-07` (`8a9de6f6c4a1911e`) and are covered by the `C01-07` and `C01-08` runs; `W2-78` (anchor changed by C01) is covered by `W2-78n`; `W2-81` (cmake configure refusal, process row, not a test kill) and `W2-82`, `W2-75` (container run, inventory process check) are not mutation rows of this stage.

### C01 lineage rows (re-run)


| id | invariant row | mutant diff sha256 (16) | result | killing test(s) | failing assertion lines | run or carried | log |
|---|---|---|---|---|---|---|---|
| C01-01 | C01-01 | `799924acf8a0e434` | killed | RA01-B2 C01 observe maps an unreadable entry to cannot inspect | directory.t.cpp:304, directory.t.cpp:305 | run | `raw/C01-01.log` |
| C01-02 | C01-02 | `855db61ed68bdb5c` | killed | RA01-B2 C01 observe maps an unreadable entry to cannot inspect | directory.t.cpp:319, directory.t.cpp:320 | run | `raw/C01-02.log` |
| C01-03 | C01-03 | `909cd755208716a3` | killed | RA01-B2 C01 observe maps an unreadable entry to cannot inspect | directory.t.cpp:334, directory.t.cpp:335 | run | `raw/C01-03.log` |
| C01-04 | C01-04 | `08cd3a8ecb264267` | killed | RA01-B2 C01 observe refuses an unreadable entry as io exit 4 | authoring_observe.t.cpp:678, authoring_observe.t.cpp:692 | run | `raw/C01-04.log` |
| C01-05 | C01-05 | `1a5a5cc31ceb8267` | killed | RA01-B2 C01 observe refuses a symlink root | directory.t.cpp:385 | run | `raw/C01-05.log` |
| C01-06 | C01-06 | `f4750820a6e6bf92` | killed | RA01-B2 C01 observe bad_alloc refusal carries observation_total_bytes | directory.t.cpp:406, directory.t.cpp:407, directory.t.cpp:408 | run | `raw/C01-06.log` |
| C01-07 | C01-07 | `8a9de6f6c4a1911e` | killed | RA01-B2 C01 observe applies precedence in sorted name order | directory.t.cpp:458, directory.t.cpp:459, directory.t.cpp:470, directory.t.cpp:471 | run | `raw/C01-07.log` |
| C01-08 | C01-08 | `49880f62c0dd9462` | killed | RA01-B2 C01 observe applies precedence in sorted name order | directory.t.cpp:458, directory.t.cpp:459, directory.t.cpp:470, directory.t.cpp:471 | run | `raw/C01-08.log` |
| C01-09 | C01-09 | `0e4531c011776ddc` | killed | RA01-B2 C01 capture_diagnostic refuses a symlink as a symbolic link | capture.t.cpp:961, capture.t.cpp:962, capture.t.cpp:971, capture.t.cpp:972 | run | `raw/C01-09.log` |
| C01-10 | C01-10 | `fbddcf74da52370d` | killed | RA01-B2 C01 capture_diagnostic refuses a symlink as a symbolic link | capture.t.cpp:962, capture.t.cpp:972, capture.t.cpp:982 | run | `raw/C01-10.log` |
| C01-11a | C01-11 | `8f2dac63118a7c8f` | killed | RA01-B2 C01 capture_diagnostic refuses a non-regular file as io | capture.t.cpp:996, capture.t.cpp:997, capture.t.cpp:1005, capture.t.cpp:1006 | run | `raw/C01-11a.log` |
| C01-11b | C01-11 | `eb8525bb9a77e9c2` | killed | RA01-B2 C01 capture_diagnostic refuses a non-regular file as io | capture.t.cpp:996, capture.t.cpp:997, capture.t.cpp:1005, capture.t.cpp:1006 | run | `raw/C01-11b.log` |
| C01-12 | C01-12 | `a5fed990d380c02c` | killed | RA01-B2 C01 capture_diagnostic maps a failed open to cannot resolve | capture.t.cpp:1019, capture.t.cpp:1033 | run | `raw/C01-12.log` |
| C01-13 | C01-13 | `78ebbf0945651c95` | killed | RA01-B2 C01 capture_diagnostic bad_alloc refusal carries diagnostic_bytes | capture.t.cpp:1060, capture.t.cpp:1061 | run | `raw/C01-13.log` |
| C01-14 | C01-14 | `deb80d727e47412e` | killed | RA01-B2 capture_diagnostic refuses a file changed before verification | capture.t.cpp:894, capture.t.cpp:902, capture.t.cpp:903 | run | `raw/C01-14.log` |
| C01-18 | C01-18 | `8e726fc6af99fa9c` | killed | RA01-B2 C01 observe checkpoint canonical size is bounded at exactly 8192 bytes | authoring_observe.t.cpp:114, authoring_observe.t.cpp:115, authoring_observe.t.cpp:116, authoring_observe.t.cpp:117 | run | `raw/C01-18.log` |
| C01-19 | C01-19 | `ea05458ed4c00eda` | killed | RA01-B2 C01 observe checkpoint canonical size is bounded at exactly 8192 bytes | authoring_observe.t.cpp:749, authoring_observe.t.cpp:750 | run | `raw/C01-19.log` |
| C01-20 | C01-20 | `127bd304bad3821b` | killed | RA01-B2 C01 prompt of exactly 1048576 bytes is accepted and one more byte is refused | authoring_prompts.t.cpp:2088 | run | `raw/C01-20.log` |
| C01-21 | C01-21 | `411d506a17272a94` | killed | RA01-B2 C01 prompt of exactly 1048576 bytes is accepted and one more byte is refused | authoring_prompts.t.cpp:2100, authoring_prompts.t.cpp:2101, authoring_prompts.t.cpp:2102, authoring_prompts.t.cpp:2104 | run | `raw/C01-21.log` |
| C01-22 | C01-22 | `f2fe51604fe2aabe` | killed | RA01-B2 C01 critique global prescription needs a nonempty check | authoring_prompts.t.cpp:2125, authoring_prompts.t.cpp:2126 | run | `raw/C01-22.log` |
| C01-23 | C01-23 | `b90c30bb8fdbbca1` | killed | RA01-B2 C01 rejection excerpt with a non-digit index is the fixed line | authoring_prompts.t.cpp:2154 | run | `raw/C01-23.log` |
| C01-24 | C01-24 | `8dbcefc6e6d01b0b` | killed | RA01-B2 diagnostic and from reach Lua only as opaque slots | authoring_orchestrator.t.cpp:221 | run | `raw/C01-24.log` |
| C01-25 | C01-25 | `843b4e3043b230e3` | killed | RA01-B2 diagnostic and from reach Lua only as opaque slots | authoring_orchestrator.t.cpp:230 | run | `raw/C01-25.log` |
| C01-33 | C01-33 | `909cd755208716a3` | killed | RA01-B2 C01 observe refuses an unreadable root as io exit 4 | authoring_observe.t.cpp:710, authoring_observe.t.cpp:712 | run | `raw/C01-33.log` |
| C01-34 | C01-34 | `c0c1bdc878d7a56e` | killed | RA01-B2 C01 capture_diagnostic maps a post-gate add_input failure to file changed | capture.t.cpp:1049, capture.t.cpp:1050 | run | `raw/C01-34.log` |
| C01-35a | C01-35 | `93794636e1ba1d87` | killed | RA01-B2 C01 capture_diagnostic post-gate failures are remapped at every native call | capture.t.cpp:1077 | run | `raw/C01-35a.log` |
| C01-38 | C01-38 | `3d029392d654182b` | killed | RA01-B2 C01 observe recheck bad_alloc is a limit and not a membership change | directory.t.cpp:419 | run | `raw/C01-38.log` |
| C01-39 | C01-39 | `5e5e953214564e91` | killed | RA01-B2 C01 observe recheck growth past the total bound stays membership changed | directory.t.cpp:438, directory.t.cpp:439, directory.t.cpp:440 | run | `raw/C01-39.log` |
| C01-45a | C01-45 | `c1dd1320c47fb1df` | killed | RA01-B2 C01 slot type checks are pinned in the Lua handlers | authoring_orchestrator.t.cpp:622, authoring_orchestrator.t.cpp:628 | run | `raw/C01-45a.log` |
| C01-45b | C01-45 | `a19aae7c75ea9dd3` | killed | RA01-B2 C01 slot type checks are pinned in the Lua handlers | authoring_orchestrator.t.cpp:635, authoring_orchestrator.t.cpp:639 | run | `raw/C01-45b.log` |
| C01-45c | C01-45 | `af9d3d8a8a37fcb9` | killed | RA01-B2 C01 slot type checks are pinned in the Lua handlers | authoring_orchestrator.t.cpp:635, authoring_orchestrator.t.cpp:639 | run | `raw/C01-45c.log` |
| E01 | extra | `fe41e685f1414b66` | survived | - | - | run | `raw/E01.log` |
| E02 | extra | `57f5722b20ae62f7` | survived | - | - | run | `raw/E02.log` |
| E03 | extra | `21fa0acbd7ba0a40` | survived | - | - | run | `raw/E03.log` |
| E04 | extra | `431b5f5bab7f03b9` | killed | RA01-B2 capture_diagnostic bounds the file at 65536 bytes from the descriptor | capture.t.cpp:915, capture.t.cpp:945 | run | `raw/E04.log` |
| E05 | extra | `bdd5a9fce9062519` | survived | - | - | run | `raw/E05.log` |
| E07 | extra | `4a34ff879648bb98` | killed-by-other | RA01-B2 capture_diagnostic refuses a file changed before verification | capture.t.cpp:863 | run | `raw/E07.log` |
| E09 | extra | `a80ee1fbf5c50b31` | killed-by-other | RA01-B2 observe enforces the checkpoint bounds | authoring_observe.t.cpp:114, authoring_observe.t.cpp:115, authoring_observe.t.cpp:116, authoring_observe.t.cpp:117 | run | `raw/E09.log` |
| E10 | extra | `45ebe2a7e09716c3` | killed | RA01-B2 C01 observe recheck bad_alloc is a limit and not a membership change | directory.t.cpp:418 | run | `raw/E10.log` |
| E11 | extra | `b2504f0714f20915` | survived | - | - | run | `raw/E11.log` |
| C01-35br | C01-35 | `2f18d57e03e9c639` | killed-by-other | RA01-B2 capture_diagnostic refuses a file changed before verification | capture.t.cpp:863 | run | `raw/C01-35br.log` |
| C01-36ar | C01-36 | `6317c571ed5bb16f` | killed | RA01-B2 C01 capture_diagnostic post-gate failures are remapped at every native call | capture.t.cpp:1088, capture.t.cpp:1089 | run | `raw/C01-36ar.log` |
| C01-36br | C01-36 | `eca30ee1f0217fea` | killed | RA01-B2 C01 capture_diagnostic post-gate failures are remapped at every native call | capture.t.cpp:1088, capture.t.cpp:1089 | run | `raw/C01-36br.log` |
| C01-37r | C01-37 | `edfb89c5d489430d` | survived | - | - | run | `raw/C01-37r.log` |
| C01-44r | C01-44 | `1db449c8bb3ef3e8` | killed | RA01-B2 C01 observe digest allocation failure is a bound refusal | authoring_observe.t.cpp:773, authoring_observe.t.cpp:774 | run | `raw/C01-44r.log` |
| E06r | extra | `dcd9dd845c083924` | killed | RA01-B2 C01 capture_diagnostic post-gate failures are remapped at every native call | capture.t.cpp:1096, capture.t.cpp:1097, capture.t.cpp:1100 | run | `raw/E06r.log` |
| E08r | extra | `0f1dcff816f4bba3` | killed | RA01-B2 C01 observe digest allocation failure is a bound refusal | authoring_observe.t.cpp:784, authoring_observe.t.cpp:785, authoring_observe.t.cpp:789 | run | `raw/E08r.log` |
| C01-35b-call | C01-35 | `611745e88e65e854` | killed | RA01-B2 C01 capture_diagnostic post-gate failures are remapped at every native call | capture.t.cpp:1080 | run | `raw/C01-35b-call.log` |
| C01-37-call | C01-37 | `4d416b50da376aad` | killed | RA01-B2 C01 capture_diagnostic post-gate failures are remapped at every native call | capture.t.cpp:1093 | run | `raw/C01-37-call.log` |
| C01-37-body | C01-37 | `fd3017b83c5b17e4` | killed | RA01-B2 C01 capture_diagnostic post-gate failures are remapped at every native call | capture.t.cpp:1096, capture.t.cpp:1097, capture.t.cpp:1100, capture.t.cpp:1101 | run | `raw/C01-37-body.log` |
| C01-35b-body | C01-35 | `71ec8302be8586e1` | killed | RA01-B2 C01 capture_diagnostic post-gate failures are remapped at every native call | capture.t.cpp:1088, capture.t.cpp:1089 | run | `raw/C01-35b-body.log` |
| E02b | extra | `f3cd3f85d73512fd` | survived | - | - | run | `raw/E02b.log` |
| E02c | extra | `44cead6ef0a41350` | killed | RA01-B2 C01 observe maps a failed entry inspection after listing to cannot inspect | directory.t.cpp:355, directory.t.cpp:356, directory.t.cpp:370, directory.t.cpp:371 | run | `raw/E02c.log` |


### Wave 2 lineage rows (re-run on this candidate; `drv3.py`)


| id | invariant row | mutant diff sha256 (16) | result | killing test(s) | failing assertion lines | run or carried | log |
|---|---|---|---|---|---|---|---|
| W2-01 | W2-01 | `b3d75ebc2e529b84` | killed | RA01-B1 explicit path stage precedes the stub refusal | authoring.t.cpp:780 | run | `raw/W2-01.log` |
| W2-02 | W2-02 | `09bac0c9da5ccc58` | killed | RA01-B1 each authoring verb parses its signature through the bundle to the stub refusal | authoring.t.cpp:527, authoring.t.cpp:529 | run | `raw/W2-02.log` |
| W2-03 | W2-03 | `63a77d924967deef` | killed | RA01-B1 stub invocations write nothing and print no success document | authoring.t.cpp:681 | run | `raw/W2-03.log` |
| W2-05 | W2-05 | `ccec63d83f52e955` | killed | RA01-B2 native refuses an authoring capture whose include does not match the verb | authoring_catalog.t.cpp:525, authoring_catalog.t.cpp:526, authoring_catalog.t.cpp:527, authoring_catalog.t.cpp:528 | run | `raw/W2-05.log` |
| W2-06 | W2-06 | `c704fe51d02da685` | killed | RA01-B2 diagnostic and from reach Lua only as opaque slots | authoring_orchestrator.t.cpp:221 | run | `raw/W2-06.log` |
| W2-07 | W2-07 | `ce17b607dbe4d4bd` | killed | RA01-B2 an oversize prompt is refused as prompt_bytes | authoring_prompts.t.cpp:727, authoring_prompts.t.cpp:728, authoring_prompts.t.cpp:729, authoring_prompts.t.cpp:731 | run | `raw/W2-07.log` |
| W2-08 | W2-08 | `6648c50364412ba7` | killed | RA01-B2 spec-prompt captures the workspace without the spec while critique and ingest capture the spec | authoring_prompts.t.cpp:754, authoring_prompts.t.cpp:755 | run | `raw/W2-08.log` |
| W2-09 | W2-09 | `fbaba7edab02fa42` | killed | RA01-B2 authoring capture succeeds when a listed job file is absent for all three prompts | authoring_prompts.t.cpp:783, authoring_prompts.t.cpp:784, authoring_prompts.t.cpp:785 | run | `raw/W2-09.log` |
| W2-10 | W2-10 | `f69843a8001407df` | killed | RA01-B2 a present job file is neither read nor listed | authoring_prompts.t.cpp:807, authoring_prompts.t.cpp:809, authoring_prompts.t.cpp:810 | run | `raw/W2-10.log` |
| W2-11 | W2-11 | `4cb6e710c3eae71b` | killed | RA01-B2 ingest and critique with an invalid spec fail with the typed error load gives | authoring_prompts.t.cpp:848 | run | `raw/W2-11.log` |
| W2-12 | W2-12 | `9742427ed497e7ef` | killed | RA01-B2 spec-prompt validates the workspace and registry only | authoring_prompts.t.cpp:863, authoring_prompts.t.cpp:864, authoring_prompts.t.cpp:865, authoring_prompts.t.cpp:872 | run | `raw/W2-12.log` |
| W2-13 | W2-13 | `d2da54de77dcc488` | killed | RA01-B2 authoring reports no job file errors | authoring_prompts.t.cpp:910, authoring_prompts.t.cpp:911 | run | `raw/W2-13.log` |
| W2-14 | W2-14 | `a7c3a106a341e4cf` | killed | RA01-B2 prompt envelope fields digest bytes and kind per verb | authoring_prompts.t.cpp:960, authoring_prompts.t.cpp:961 | run | `raw/W2-14.log` |
| W2-15 | W2-15 | `3ac884080b761ee4` | survived | - | - | run | `raw/W2-15.log` |
| W2-15b | W2-15 | `600b75fbedd2c4fe` | killed | RA01-B2 prompt inputs list sorted file entries and registry header | authoring_prompts.t.cpp:994, authoring_prompts.t.cpp:1014 | run | `raw/W2-15b.log` |
| W2-16 | W2-16 | `fea978250380d647` | killed-by-other | RA01-B2 an oversize prompt is refused as prompt_bytes; RA01-B2 C01 prompt of exactly 1048576 bytes is accepted and one more byte is refused (+98) | authoring.t.cpp:527, authoring.t.cpp:529, authoring.t.cpp:545, authoring.t.cpp:548 | run | `raw/W2-16.log` |
| W2-17 | W2-17 | `2922dc6f3da2f7c1` | killed | RA01-B2 prompts carry their sections in contract order | authoring_prompts.t.cpp:1132 | run | `raw/W2-17.log` |
| W2-17b | W2-17 | `242024cffb14717e` | killed | RA01-B2 prompts carry their sections in contract order | authoring_prompts.t.cpp:1132 | run | `raw/W2-17b.log` |
| W2-18 | W2-18 | `082a4ece65bbec71` | killed | RA01-B2 schema text lists spec job and workspace fields generated from the schema tables | authoring_prompts.t.cpp:1190 | run | `raw/W2-18.log` |
| W2-19 | W2-19 | `df6642cfaccda293` | killed | RA01-B2 schema text format sorts fields and nests records | authoring_prompts.t.cpp:1263, authoring_prompts.t.cpp:1277, authoring_prompts.t.cpp:1278 | run | `raw/W2-19.log` |
| W2-19b | W2-19 | `bada121bcdb7213e` | killed | RA01-B2 schema text format sorts fields and nests records | authoring_prompts.t.cpp:1251, authoring_prompts.t.cpp:1276, authoring_prompts.t.cpp:1277, authoring_prompts.t.cpp:1278 | run | `raw/W2-19b.log` |
| W2-19c | W2-19 | `8c371e539b4ba492` | killed | RA01-B2 schema text format sorts fields and nests records | authoring_prompts.t.cpp:1279 | run | `raw/W2-19c.log` |
| W2-20 | W2-20 | `81aebc5f55b34416` | killed | RA01-B2 checks section is the sorted allow-list intersected with the registry | authoring_prompts.t.cpp:1315, authoring_prompts.t.cpp:1316, authoring_prompts.t.cpp:1317 | run | `raw/W2-20.log` |
| W2-20b | W2-20 | `c2031dc68b998393` | killed | RA01-B2 checks section is the sorted allow-list intersected with the registry | authoring_prompts.t.cpp:1315 | run | `raw/W2-20b.log` |
| W2-21 | W2-21 | `afde321be0f4532e` | killed | RA01-B2 shape catalog lists pipeline then fanout_review | authoring_prompts.t.cpp:1327 | run | `raw/W2-21.log` |
| W2-22 | W2-22 | `2f58b3faea9c5649` | killed | RA01-B2 spec prompt ends with the critique step line and embeds no host path | authoring_prompts.t.cpp:1362, authoring_prompts.t.cpp:1367 | run | `raw/W2-22.log` |
| W2-23 | W2-23 | `b917323642862f4a` | survived | - | - | run | `raw/W2-23.log` |
| W2-24 | W2-24 | `7f1650037400ba27` | killed | RA01-B2 captured text is escaped as data in prompts | authoring_prompts.t.cpp:1445, authoring_prompts.t.cpp:1446, authoring_prompts.t.cpp:1447, authoring_prompts.t.cpp:1448 | run | `raw/W2-24.log` |
| W2-25 | W2-25 | `18238bdf495c3c59` | killed | RA01-B2 critique rule prescription_unchecked fires only without a nonempty check | authoring_prompts.t.cpp:1485, authoring_prompts.t.cpp:1486 | run | `raw/W2-25.log` |
| W2-26 | W2-26 | `a3a98e0840d8c047` | killed | RA01-B2 critique rule blocking_question_unanswered covers start and complete | authoring_prompts.t.cpp:1504 | run | `raw/W2-26.log` |
| W2-27 | W2-27 | `c499836035dcf8fb` | killed | RA01-B2 critique rule facet_dependency_without_output_vector needs an output_vector check | authoring_prompts.t.cpp:1527 | run | `raw/W2-27.log` |
| W2-28 | W2-28 | `02744a26ef90a5ca` | killed | RA01-B2 critique findings are sorted advisory and exit zero | authoring_prompts.t.cpp:1567 | run | `raw/W2-28.log` |
| W2-28b | W2-28 | `e9d55fc8abb8b8c3` | killed | RA01-B2 critique findings are sorted advisory and exit zero | authoring_prompts.t.cpp:1555 | run | `raw/W2-28b.log` |
| W2-29 | W2-29 | `1946031629651fc7` | killed | RA01-B2 critique prints the fixed line when no rule fires | authoring_prompts.t.cpp:1580, authoring_prompts.t.cpp:1581 | run | `raw/W2-29.log` |
| W2-30 | W2-30 | `3d3e111eef2e58c8` | killed | RA01-B2 job prompt lists sorted constructors and the job tier statement | authoring_prompts.t.cpp:1601 | run | `raw/W2-30.log` |
| W2-30b | W2-30 | `c8481f0825187839` | killed | RA01-B2 job prompt lists sorted constructors and the job tier statement | authoring_prompts.t.cpp:1602, authoring_prompts.t.cpp:1603 | run | `raw/W2-30b.log` |
| W2-31 | W2-31 | `4fede4e5ba1c65e1` | killed | RA01-B2 explain-rejection refuses malformed diagnostic JSON as schema | authoring_prompts.t.cpp:1637 | run | `raw/W2-31.log` |
| W2-32 | W2-32 | `c1861f7fcde129ff` | killed | RA01-B2 explain-rejection validates the diagnostic envelope and code grammar | authoring_prompts.t.cpp:1617, authoring_prompts.t.cpp:1618, authoring_prompts.t.cpp:1619, authoring_prompts.t.cpp:1621 | run | `raw/W2-32.log` |
| W2-32b | W2-32 | `5ed871b07f9689e8` | killed | RA01-B2 explain-rejection validates the diagnostic envelope and code grammar | authoring_prompts.t.cpp:1617, authoring_prompts.t.cpp:1618, authoring_prompts.t.cpp:1619, authoring_prompts.t.cpp:1621 | run | `raw/W2-32b.log` |
| W2-33 | W2-33 | `1f956ff4bc7523ff` | survived | - | - | run | `raw/W2-33.log` |
| W2-34 | W2-34 | `81b45307ab7fe874` | killed-by-other | RA01-B1 missing leaf remap applies only to diagnostic and from; RA01-B1 explicit path stage keeps the existing messages | authoring.t.cpp:946, authoring.t.cpp:1015 | run | `raw/W2-34.log` |
| W2-35 | W2-35 | `c24e0dddf3186f40` | killed | RA01-B2 rejection inputs carry the diagnostic size and file digest | authoring_prompts.t.cpp:1819, authoring_prompts.t.cpp:1820 | run | `raw/W2-35.log` |
| W2-36 | W2-36 | `9fb0e32d211d534b` | killed | RA01-B2 rejection excerpt resolves job.roles[1].work to the work field | authoring_prompts.t.cpp:1835, authoring_prompts.t.cpp:1836, authoring_prompts.t.cpp:1842 | run | `raw/W2-36.log` |
| W2-37 | W2-37 | `ff79dd26775d1744` | killed | RA01-B2 rejection excerpt for an unresolvable path is the fixed line | authoring_prompts.t.cpp:1875 | run | `raw/W2-37.log` |
| W2-38 | W2-38 | `d6039d48402505eb` | killed | RA01-B2 raw_error block equals the diagnostic bytes with DEL and C1 raw | authoring_prompts.t.cpp:1889, authoring_prompts.t.cpp:1890, authoring_prompts.t.cpp:1891 | run | `raw/W2-38.log` |
| W2-39 | W2-39 | `e6a08117834bb9e0` | killed | RA01-B2 raw_error fence outgrows backtick runs and adds one LF only when missing | authoring_prompts.t.cpp:1923 | run | `raw/W2-39.log` |
| W2-39b | W2-39 | `5fe2b643585df512` | killed | RA01-B2 raw_error fence outgrows backtick runs and adds one LF only when missing | authoring_prompts.t.cpp:1924 | run | `raw/W2-39b.log` |
| W2-40 | W2-40 | `0211e64b67aed8d7` | killed | RA01-B2 A36 rejected load stderr round-trips through explain-rejection | authoring_prompts.t.cpp:1963 | run | `raw/W2-40.log` |
| W2-41 | W2-41 | `af07a5017989d8fe` | killed-by-other | RA01-B1 with-profile outputs are identical under the authoring bundle; RA01-B1 with-profile probe stays five and a malformed authoring tail reaches profile capture (+28) | authoring.t.cpp:527, authoring.t.cpp:528, authoring.t.cpp:561, authoring.t.cpp:574 | run | `raw/W2-41.log` |
| W2-42 | W2-42 | `dbcf47402d21032e` | killed | RA01-B2 observe refuses a symlink entry | authoring_observe.t.cpp:142, authoring_observe.t.cpp:151, authoring_observe.t.cpp:166, authoring_observe.t.cpp:167 | run | `raw/W2-42.log` |
| W2-43 | W2-43 | `15e48210369198ee` | killed | RA01-B2 observe refuses a FIFO entry | authoring_observe.t.cpp:182 | run | `raw/W2-43.log` |
| W2-44 | W2-44 | `7d7808e88b2f987c` | killed | RA01-B2 observe refuses a hard-link alias | authoring_observe.t.cpp:197, authoring_observe.t.cpp:205, authoring_observe.t.cpp:213 | run | `raw/W2-44.log` |
| W2-46 | W2-46 | `cb681f0488fc923e` | killed | RA01-B2 observe ignores dot names | authoring_observe.t.cpp:123, authoring_observe.t.cpp:124, authoring_observe.t.cpp:125 | run | `raw/W2-46.log` |
| W2-46b | W2-46 | `4e137b161ca2cf7f` | killed-by-other | RA01-B2 observe bounds listed names at 1024 counting dot names | directory.t.cpp:107 | run | `raw/W2-46b.log` |
| W2-47 | W2-47 | `1b5c063a8cabae04` | killed | RA01-B2 observe bounds listed names at 1024 counting dot names | directory.t.cpp:84, directory.t.cpp:103, directory.t.cpp:118 | run | `raw/W2-47.log` |
| W2-47b | W2-47 | `20689402b6683021` | killed | RA01-B2 observe bounds listed names at 1024 counting dot names | directory.t.cpp:88, directory.t.cpp:107, directory.t.cpp:125 | run | `raw/W2-47b.log` |
| W2-48 | W2-48 | `18badd1bc4837de0` | killed | RA01-B2 observe refuses an oversize file as observation_file_bytes | directory.t.cpp:143 | run | `raw/W2-48.log` |
| W2-50 | W2-50 | `4c851a22ad1c83b0` | killed | RA01-B2 observe reports an unsafe entry before a later size limit | directory.t.cpp:177, directory.t.cpp:178 | run | `raw/W2-50.log` |
| W2-52 | W2-52 | `becc07e1c4889660` | killed | RA01-B2 observe refuses a directory changed before the recheck | directory.t.cpp:279 | run | `raw/W2-52.log` |
| W2-53 | W2-53 | `a48f1291254a6925` | killed | RA01-B2 observe output over one MiB is refused as observation_bytes | authoring_observe.t.cpp:294, authoring_observe.t.cpp:295, authoring_observe.t.cpp:296, authoring_observe.t.cpp:297 | run | `raw/W2-53.log` |
| W2-54 | W2-54 | `e8d8de8e1c07d86f` | killed | RA01-B2 observe refuses duplicate ids within checks and within findings | authoring_observe.t.cpp:123, authoring_observe.t.cpp:124, authoring_observe.t.cpp:125 | run | `raw/W2-54.log` |
| W2-55 | W2-55 | `90be7a80d88b1268` | killed | RA01-B2 observe errors carry the logical file and object-relative field | authoring_observe.t.cpp:118 | run | `raw/W2-55.log` |
| W2-56 | W2-56 | `e2d9838ce655bb3b` | killed | RA01-B2 observe applies structural checks only | authoring_observe.t.cpp:466, authoring_observe.t.cpp:467, authoring_observe.t.cpp:469, authoring_observe.t.cpp:470 | run | `raw/W2-56.log` |
| W2-57 | W2-57 | `a80ee1fbf5c50b31` | killed | RA01-B2 observe enforces the checkpoint bounds | authoring_observe.t.cpp:114, authoring_observe.t.cpp:115, authoring_observe.t.cpp:116, authoring_observe.t.cpp:117 | run | `raw/W2-57.log` |
| W2-58 | W2-58 | `d89e95e896e59477` | killed | RA01-B2 observe reports the first error in checks, findings, checkpoint order | authoring_observe.t.cpp:117, authoring_observe.t.cpp:118 | run | `raw/W2-58.log` |
| W2-59 | W2-59 | `460148013b9d3f24` | killed | RA01-B2 observe prints sorted bare observations | authoring_observe.t.cpp:123, authoring_observe.t.cpp:124, authoring_observe.t.cpp:125 | run | `raw/W2-59.log` |
| W2-60 | W2-60 | `4507d606a136380c` | killed | RA01-B2 observe empty directory prints canonical empty observations | authoring_observe.t.cpp:123, authoring_observe.t.cpp:124, authoring_observe.t.cpp:125 | run | `raw/W2-60.log` |
| W2-61 | W2-61 | `a3a3354ec2fa950f` | killed | RA01-B2 observe output is independent of creation order | authoring_observe.t.cpp:596 | run | `raw/W2-61.log` |
| W2-62 | W2-62 | `16050077e43393fa` | killed | RA01-B2 observe output is structurally accepted by render observations | authoring_observe.t.cpp:610, authoring_observe.t.cpp:619 | run | `raw/W2-62.log` |
| W2-63 | W2-63 | `f7c8c1596ca84582` | killed | RA01-B1 explicit path stage keeps the existing messages | authoring.t.cpp:946, authoring.t.cpp:948 | run | `raw/W2-63.log` |
| W2-64 | W2-64 | `2a15a83a1ff9ce8a` | killed | RA01-B2 orchestrator refuses an unlisted job as reference selection.job | authoring_orchestrator.t.cpp:254, authoring_orchestrator.t.cpp:256, authoring_orchestrator.t.cpp:264 | run | `raw/W2-64.log` |
| W2-65 | W2-65 | `e4748845d89441b2` | killed | RA01-B2 orchestrator reads only the selected job | authoring_orchestrator.t.cpp:280, authoring_orchestrator.t.cpp:281, authoring_orchestrator.t.cpp:282, authoring_orchestrator.t.cpp:286 | run | `raw/W2-65.log` |
| W2-66 | W2-66 | `587bacbd02072fc3` | killed | RA01-B2 orchestrator sections render shape roles and graph | authoring_orchestrator.t.cpp:357, authoring_orchestrator.t.cpp:371 | run | `raw/W2-66.log` |
| W2-66b | W2-66 | `19f39d1083e83a4b` | survived | - | - | run | `raw/W2-66b.log` |
| W2-67 | W2-67 | `431c29c25b1585e9` | survived | - | - | run | `raw/W2-67.log` |
| W2-68 | W2-68 | `fb92abacf930f0f4` | killed | RA01-B2 orchestrator prompt carries five sections in order | authoring_orchestrator.t.cpp:422 | run | `raw/W2-68.log` |
| W2-69 | W2-69 | `59c13d30de4e7505` | killed | RA01-B2 orchestrator labels every catalog verb available and every future verb future | authoring_orchestrator.t.cpp:514, authoring_orchestrator.t.cpp:517, authoring_orchestrator.t.cpp:518 | run | `raw/W2-69.log` |
| W2-69b | W2-69 | `35ee40c8cd016d47` | killed | RA01-B2 orchestrator labels every catalog verb available and every future verb future | authoring_orchestrator.t.cpp:506 | run | `raw/W2-69b.log` |
| W2-70 | W2-70 | `9e755ecbb43ec265` | killed | RA01-B2 orchestrator label flips to available for a catalog stub verb | authoring_orchestrator_variants.t.cpp:264, authoring_orchestrator_variants.t.cpp:265 | run | `raw/W2-70.log` |
| W2-71 | W2-71 | `d8f472b95113eb0c` | killed-by-other | RA01-B2 orchestrator sections render shape roles and graph | authoring_orchestrator.t.cpp:321, authoring_orchestrator.t.cpp:327, authoring_orchestrator.t.cpp:368 | run | `raw/W2-71.log` |
| W2-72 | W2-72 | `275b2a3220136597` | killed-by-other | RA01-B1 selector stage precedes the explicit path stage; RA01-B1 missing leaf remap applies only to diagnostic and from (+4) | authoring.t.cpp:759, authoring.t.cpp:760, authoring.t.cpp:797, authoring.t.cpp:798 | run | `raw/W2-72.log` |
| W2-73 | W2-73 | `f354c42388f00f63` | killed | RA01-B2 orchestrator_context_v1 is refused as an external projection input | external_registration.t.cpp:271 | run | `raw/W2-73.log` |
| W2-74 | W2-74 | `0bdd7545ce8faf65` | killed | X08 C01 role preparation outputs-off boundary pins the request-record conversion charge | external_registration.t.cpp:4595 | run | `raw/W2-74.log` |
| W2-76 | W2-76 | `e7cb9523b303ba92` | killed | RA01-B2 render and load keep Listed job not captured outside the authoring modes | authoring_prompts.t.cpp:2012, authoring_prompts.t.cpp:2015, authoring_prompts.t.cpp:2016, authoring_prompts.t.cpp:2017 | run | `raw/W2-76.log` |
| W2-77 | W2-77 | `2dc41bee582b1667` | killed | RA01-B2 native appends the sorted sealed command names to the orchestrator capture | authoring_orchestrator_variants.t.cpp:284, authoring_orchestrator_variants.t.cpp:285 | run | `raw/W2-77.log` |
| W2-77c | W2-77 | `21d786771c3aaa65` | survived | - | - | run | `raw/W2-77c.log` |
| W2-79 | W2-79 | `1f956ff4bc7523ff` | survived | - | - | run | `raw/W2-79.log` |
| W2-79b | W2-79 | `bdd5a9fce9062519` | survived | - | - | run | `raw/W2-79b.log` |
| W2-80 | W2-80 | `20230dd93e3a1af3` | killed | RA01-B2 orchestrator selected-job mode still checks the listed job identity | authoring_orchestrator.t.cpp:155 | run | `raw/W2-80.log` |
| X01 | X-ORCH | `ae7f06b493b642cf` | survived | - | - | run | `raw/X01.log` |
| X02 | X-OBS | `0aeee8073ec64bbe` | killed | RA01-B2 observe applies structural checks only | authoring_observe.t.cpp:450, authoring_observe.t.cpp:451, authoring_observe.t.cpp:452, authoring_observe.t.cpp:453 | run | `raw/X02.log` |
| X03 | X-OBS | `8e726fc6af99fa9c` | killed-by-other | RA01-B2 C01 observe checkpoint canonical size is bounded at exactly 8192 bytes | authoring_observe.t.cpp:114, authoring_observe.t.cpp:115, authoring_observe.t.cpp:116, authoring_observe.t.cpp:117 | run | `raw/X03.log` |
| X04 | X-PRM | `127bd304bad3821b` | killed-by-other | RA01-B2 C01 prompt of exactly 1048576 bytes is accepted and one more byte is refused | authoring_prompts.t.cpp:2088 | run | `raw/X04.log` |
| X05 | X-CRIT | `f2fe51604fe2aabe` | killed-by-other | RA01-B2 C01 critique global prescription needs a nonempty check | authoring_prompts.t.cpp:2125, authoring_prompts.t.cpp:2126 | run | `raw/X05.log` |
| X06 | X-CRIT | `123ab5ccf43b4794` | killed-by-other | RA01-B2 critique rule prescription_unchecked fires only without a nonempty check | authoring_prompts.t.cpp:1484 | run | `raw/X06.log` |
| X07 | X-RJ | `b90c30bb8fdbbca1` | killed-by-other | RA01-B2 C01 rejection excerpt with a non-digit index is the fixed line | authoring_prompts.t.cpp:2154 | run | `raw/X07.log` |
| X08 | X-RJ | `0b1f16ade6bc5959` | killed | RA01-B2 explain-rejection validates the diagnostic envelope and code grammar | authoring_prompts.t.cpp:1617, authoring_prompts.t.cpp:1618, authoring_prompts.t.cpp:1619, authoring_prompts.t.cpp:1621 | run | `raw/X08.log` |
| X09 | X-ESC | `247b9fa59320f572` | killed | RA01-B2 captured text is escaped as data in prompts | authoring_prompts.t.cpp:1445, authoring_prompts.t.cpp:1450, authoring_prompts.t.cpp:1461 | run | `raw/X09.log` |
| X10 | X-DIR | `de0a84cd00cc3dc7` | killed | RA01-B2 observe ignores dot names | authoring_observe.t.cpp:123, authoring_observe.t.cpp:124, authoring_observe.t.cpp:125 | run | `raw/X10.log` |
| X11 | X-RT | `3fb4eeb617850601` | survived | - | - | run | `raw/X11.log` |
| X12 | X-PRM | `4173cb7200fdc441` | survived | - | - | run | `raw/X12.log` |
| X13 | X-ORCH | `60189e029987b561` | survived | - | - | run | `raw/X13.log` |
| X14 | X-ORCH | `93759a272e1f8115` | survived | - | - | run | `raw/X14.log` |
| X15 | X-ORCH | `4dda7ed8f5d1619c` | survived | - | - | run | `raw/X15.log` |
| X16 | X-CAP | `0e4531c011776ddc` | killed-by-other | RA01-B2 capture_diagnostic refuses a file changed before verification; RA01-B2 C01 capture_diagnostic refuses a symlink as a symbolic link | capture.t.cpp:893, capture.t.cpp:894, capture.t.cpp:961, capture.t.cpp:962 | run | `raw/X16.log` |
| X17 | X-DIR | `1a5a5cc31ceb8267` | killed-by-other | RA01-B2 C01 observe refuses a symlink root | directory.t.cpp:385 | run | `raw/X17.log` |
| X18 | X-PRM | `908d21ed3c2da61b` | killed | RA01-B2 prompt inputs list sorted file entries and registry header | authoring_prompts.t.cpp:983, authoring_prompts.t.cpp:985 | run | `raw/X18.log` |
| X19 | X-ORCH | `52df9a4b201e72fc` | killed | RA01-B2 orchestrator prompt carries five sections in order | authoring_orchestrator.t.cpp:471 | run | `raw/X19.log` |
| W2-06n | W2-06 | `40c758a77bef3b14` | killed | RA01-B2 diagnostic and from reach Lua only as opaque slots | authoring_orchestrator.t.cpp:221 | run | `raw/W2-06n.log` |
| W2-16n | W2-16 | `5335ec4688004ad9` | killed-by-other | RA01-B2 C01 prompt of exactly 1048576 bytes is accepted and one more byte is refused; RA01-B2 an oversize prompt is refused as prompt_bytes (+48) | authoring.t.cpp:527, authoring.t.cpp:887, authoring.t.cpp:888, authoring_observe.t.cpp:125 | run | `raw/W2-16n.log` |
| W2-45r | W2-45 | `7fe04a36235e9b5d` | killed | RA01-B2 observe refuses layout violations without echoing names | authoring_observe.t.cpp:227, authoring_observe.t.cpp:228, authoring_observe.t.cpp:229, authoring_observe.t.cpp:230 | run | `raw/W2-45r.log` |
| W2-49r | W2-49 | `e75a01717c6a9353` | killed | RA01-B2 observe refuses more than four MiB as observation_total_bytes | directory.t.cpp:161 | run | `raw/W2-49r.log` |
| W2-33c | W2-33 | `8f2604c93ad553db` | killed | RA01-B2 explain-rejection accepts 65536 bytes and refuses 65537 as diagnostic_bytes | authoring_prompts.t.cpp:1751, authoring_prompts.t.cpp:1752, authoring_prompts.t.cpp:1753, authoring_prompts.t.cpp:1755 | run | `raw/W2-33c.log` |
| W2-79c | W2-79 | `8f2604c93ad553db` | killed | RA01-B2 capture_diagnostic bounds the file at 65536 bytes from the descriptor | capture.t.cpp:924, capture.t.cpp:934 | run | `raw/W2-79c.log` |
| X20 | X-RJ | `2820e319412c1d95` | killed | RA01-B2 explain-rejection refuses malformed diagnostic JSON as schema | authoring_prompts.t.cpp:1637 | run | `raw/X20.log` |
| W2-41r | W2-41 | `af07a5017989d8fe` | killed | RA01-B1 explain-rejection and observe ignore an invalid workspace environment | authoring.t.cpp:885 | run | `raw/W2-41r.log` |
| W2-77d | W2-77 | `34c3c0959255757f` | killed | RA01-B2 native appends the sorted sealed command names to the orchestrator capture | authoring_orchestrator_variants.t.cpp:278 | run | `raw/W2-77d.log` |
| W2-04b | W2-04 | `f03cf525a1074ed0` | killed | RA01-B1 default invoke and production binary refuse all six authoring verbs as usage | bootstrap.t.cpp:3304, bootstrap.t.cpp:3338 | run | `raw/W2-04b.log` |
| W2-16s | W2-16 | `fea978250380d647` | killed | RA01-B2 A35 five prompt kinds are byte-equal across two fresh processes and reordered declarations | authoring_prompts.t.cpp:1078, authoring_prompts.t.cpp:1080, authoring_prompts.t.cpp:1085, authoring_prompts.t.cpp:1096 | run | `raw/W2-16s.log` |
| W2-16ns | W2-16 | `5335ec4688004ad9` | killed | RA01-B2 A35 five prompt kinds are byte-equal across two fresh processes and reordered declarations | authoring_prompts.t.cpp:1078, authoring_prompts.t.cpp:1080, authoring_prompts.t.cpp:1085, authoring_prompts.t.cpp:1096 | run | `raw/W2-16ns.log` |
| W2-78n | W2-78 | `f15e4b5daaee7acc` | killed | RA01-B2 capture_diagnostic refuses a file changed before verification | capture.t.cpp:861, capture.t.cpp:874, capture.t.cpp:883 | run | `raw/W2-78n.log` |

Result changes against the earlier PROBES of this lineage (all other rows reproduce their recorded status):

- W2-34 (`authoring.cpp` skips path resolution for the missing-leaf remap): was killed by the named test `read_diagnostic refuses links and special files with the existing messages`; now the named test passes and the mutant is killed by `RA01-B1 missing leaf remap applies only to diagnostic and from` and `RA01-B1 explicit path stage keeps the existing messages`. Cause: C01 made `capture_diagnostic` refuse a symlink with `path_denied` at its own descriptor gate, so the named symlink case no longer depends on the path-stage guard. Still a kill, by a different test. Not a defect of C02.
- W2-06 (killed-by-other in wave 2) and W2-06n (survived in wave 2) are now killed by `RA01-B2 diagnostic and from reach Lua only as opaque slots`, as the C01 repair intended. W2-31, X09, X19, X20 reproduce their post-repair kills.
- killed-by-other rows (named test passes, another test fails): E07, E09, C01-35br, W2-16, W2-16n, W2-34, W2-41, W2-46b, W2-71, W2-72, X03, X04, X05, X06, X07, X16, X17; each failing test is named in its log and in the table.

## Survivors and their status

- C02-15a, C02-15b: by design, caught only by the gate hash comparison (shown above).
- C01-37r: `digest_failure` gets `(void)maximum; return error;` before the original body, so the helper text still contains both literals and the exact spelling (dead code after the early return). The source guard cannot see an early return; the call-site form `C01-37-call` and the body-replacement form `C01-37-body` are killed by the T37 test, and `C01-35br` is likewise killed only by another test. This is the documented source-guard limit of T37 (no allocator fault seam), unchanged from C01 and unchanged by C02. No C02 row depends on the early-return form.
- E01, E03, E05, E11, E02, E02b: fault or race sites with no fixture (read() error, readdir errno after open, file growth after the gate, `fstatat` after a successful listing); unchanged limits, no seam may be added (D-C01-13a). E02 and E02b stay stated limits under R2; E02c is the killed row for the new directory test.
- W2-15, W2-23, W2-33, W2-66b, W2-67, W2-77c, W2-79, W2-79b, X01, X11, X12, X13, X14, X15: wave 2 documented survivors (equivalent mutants or probable redundant guards, explanations in `tests/native/v5-02-ra01-b-w2-evidence/probes/PROBES.md`), same status now.

## Findings for the coordinator and reviewer

1. `base/` is not the pre-C02 copy for the five T1 test files (C02-38, owner S0). `base/src/engine/capture/capture.t.cpp`, `authoring_observe.t.cpp`, `authoring_prompts.t.cpp`, `authoring_orchestrator.t.cpp` and `directory.t.cpp` are byte-equal to the CURRENT checkout (`base/SHA256SUMS` lists `0de9b5d1...` for `capture.t.cpp`, `aa667a22...` for `authoring_observe.t.cpp`, `673a9c09...` for `authoring_prompts.t.cpp`, `16c6b2c8...` for `authoring_orchestrator.t.cpp`, `a2f51823...` for `directory.t.cpp`), while the C01 identity (`probes/raw/identity-after.txt` of the C01 evidence) has `d59633bb...`, `d85c08fc...`, `a96f8856...`, `468d35e6...`, `c992a209...`. The product `base/` files (`capture.cpp`, `capture.cppm`, `invocation.cpp`) do match the C01 identity, and `base/BASE.md` does not exist. Consequence: the C02-08 and C02-18 line-count checks and any comment-only claim for those five test files cannot be verified against `base/` (the diffs are vacuously empty). Whether T1 changed only comments in `directory.t.cpp`, `authoring_prompts.t.cpp` and `authoring_orchestrator.t.cpp` is therefore not shown by this stage; the behaviour of all three files is covered by the full re-run above. Not fixed here: `base/` is owned by S0.
2. The two C02 digest guards (T37, T44) are source guards. The ten R1 mutants are killed, but a mutant that returns early before the condition (C01-37r) survives, a limit stated in the test comment and unchanged.

## Red-first (every test of this lineage, mutant that makes it fail)

C02 adds and renames no test (1437 names kept); T37 and T44 gained assertions. For every test added in the C01 lineage and for the strengthened T37 and T44, a mutant that fails it was run on this candidate:

| test | mutant ids that fail it (all run on this candidate) |
|---|---|
| RA01-B2 C01 capture_diagnostic post-gate failures are remapped at every native call (T37) | C01-35a, C01-35b-call, C01-35b-body, C01-36ar, C01-36br, C01-37-call, C01-37-body, E06r, C02-01, C02-03, C02-35, C02-05a, C02-05b, X-C02-a, X-C02-c |
| RA01-B2 C01 observe digest allocation failure is a bound refusal (T44) | C01-44r, E08r, C02-02, C02-04, C02-36, C02-05c, C02-05d, X-C02-b, X-C02-d |
| RA01-B2 C01 observe maps a failed entry inspection after listing to cannot inspect (T46) | E02c (E02 and E02b do not fail it: stated limits) |
| RA01-B2 C01 observe maps an unreadable entry to cannot inspect | C01-01, C01-02, C01-03 |
| RA01-B2 C01 observe refuses an unreadable entry as io exit 4 | C01-04 |
| RA01-B2 C01 observe refuses an unreadable root as io exit 4 | C01-33 |
| RA01-B2 C01 observe refuses a symlink root | C01-05 |
| RA01-B2 C01 observe bad_alloc refusal carries observation_total_bytes | C01-06 |
| RA01-B2 C01 observe applies precedence in sorted name order | C01-07, C01-08 |
| RA01-B2 C01 observe recheck bad_alloc is a limit and not a membership change | C01-38, E10 |
| RA01-B2 C01 observe recheck growth past the total bound stays membership changed | C01-39 |
| RA01-B2 C01 observe checkpoint canonical size is bounded at exactly 8192 bytes | C01-18, C01-19 |
| RA01-B2 C01 capture_diagnostic refuses a symlink as a symbolic link | C01-09, C01-10 |
| RA01-B2 C01 capture_diagnostic refuses a non-regular file as io | C01-11a, C01-11b |
| RA01-B2 C01 capture_diagnostic maps a failed open to cannot resolve | C01-12 |
| RA01-B2 C01 capture_diagnostic bad_alloc refusal carries diagnostic_bytes | C01-13, X-C02-e |
| RA01-B2 C01 capture_diagnostic maps a post-gate add_input failure to file changed | C01-34, C02-06 |
| RA01-B2 C01 prompt of exactly 1048576 bytes is accepted and one more byte is refused | C01-20, C01-21 |
| RA01-B2 C01 critique global prescription needs a nonempty check | C01-22 |
| RA01-B2 C01 rejection excerpt with a non-digit index is the fixed line | C01-23 |
| RA01-B2 C01 slot type checks are pinned in the Lua handlers | C01-45a, C01-45b, C01-45c |
| RA01-B2 diagnostic and from reach Lua only as opaque slots | C01-24, C01-25, W2-06n |
| X08 C01 role preparation outputs-off boundary pins the request-record conversion charge | C02-15c (C02-15a and C02-15b are caught only by the hash comparison) |

The wave 2 tests (`RA01-B1`/`RA01-B2 ...` outside the `C01` set) are each failed by the wave 2 rows in the table above (W2-01 to W2-80, X rows), all re-run here; the rows that survive are the documented equivalents listed under Survivors. The pre-fix (red-first) runs of T37 and T44 against the C01 product bytes are the RF stage's evidence under `red/` (`guard1.log`, `guard2.log`), not re-run here.

## Skipped

- Exploratory budget: 5 new extra mutants (X-C02-a to X-C02-e) plus 2 re-built forms of earlier extras (E06r, E08r) of 20; none skipped for budget. Earlier extras E01 to E11 re-run as carried rows of this lineage.
- Not run: Linux container re-run of the root-aware and Linux-only rows (W2-45 Linux branch, W2-51 Linux fallback; as root the root-aware rows SUCCEED), RelWithDebInfo (Debug scratch build only), `W2-81` (cmake configure process row), the first build-failed forms listed in the carry-forward ledger, and every non-mutation row (C02-07 to C02-14, C02-16 to C02-34, C02-38 to C02-41), which are grep, diff, process or gate rows owned by the gate, reviewer and coordinator stages.
