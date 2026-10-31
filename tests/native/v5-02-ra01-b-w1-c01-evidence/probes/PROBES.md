# V5-02-RA01-B-W1-C01 mutation probes (focused re-probe after the survivor repair, 2026-10-31)

The previous full probe set is preserved unchanged in `probes/superseded-1/` (PROBES.md, raw/ with 393 logs, results.jsonl). Nothing was deleted.

## Identity (identity.py output, verbatim; byte-identical before the first mutant and after the last restore)

| identity | value |
|---|---|
| HEAD | `307026288615e317d145ba6846c0692929c50a17` |
| sha256(git diff --binary HEAD -- src tests/native), tracked only | `eb11d7a210bfb7036f08ed98b900ef3953fffd6e11c5e4e4b399f0f4602d48eb` |
| combined sha256 (diff bytes, then each untracked path NUL bytes, sorted), pathspec src tests/native, evidence excluded | `de65ea1fe7bab915d77bf3e9329b9e2a226f64ac04e3ddad5bdd409e2dabb3d5` |

| file | state | sha256 |
|---|---|---|
| `src/cmd/planar/CMakeLists.txt` | modified | `a500e63c899b433cf8d253fb5f8dbe80061074b4d728c151940aeb5e9d75a823` |
| `src/cmd/planar/bootstrap.cpp` | modified | `9db6993381e4cbea3cbc2cb432504321b245dd7b2d341dbd2320917aa74f05a8` |
| `src/cmd/planar/bootstrap.t.cpp` | modified | `44684743e0b8f2b92955641dc8400baa8fb8f03da82bdcda72de05d3cc583bf9` |
| `src/cmd/planar/response.cpp` | modified | `233af0d52232e51855d0dfb00000fe20b5981e36c399aaf252cc6ee3ead74cb6` |
| `src/cmd/planar/response.cppm` | modified | `c007ebbfe83bd2b9f88234294b29fe9f704e279982a5eac61323d7befbf1cf31` |
| `src/engine/runtime/external_registration.t.cpp` | modified | `5386b9d9c3ea23590efb6bc9266f9f69ba42c7a2fbf17580e219a2cd6c54a5e7` |
| `src/engine/runtime/runtime.cpp` | modified | `3911b25effcf890db0924fe9959907c3e563e5f0e9b47cc939831764e282c61e` |
| `src/engine/runtime/runtime.t.cpp` | modified | `f8d3a1fde07a32c472f200f4c6807f0608bcccf982ccce0cc6144b69855216bc` |
| `src/stdlib/planar/commands.lua` | modified | `23b597fa889f5d05869169f9ba9980e2cb242efd6a0316ada8ab8df08988072f` |
| `src/cmd/planar/authoring.cpp` | untracked | `0bd890d8d0a04a5aad9e51ccfa01d22b71e7431b527712ca669e928f27b9286f` |
| `src/cmd/planar/authoring.cppm` | untracked | `4e56274a57a410a00fda867ea1474ce1c02a892a12cce0b0e26b762a075b2d09` |
| `src/cmd/planar/authoring.t.cpp` | untracked | `bfa677299157c694eec594c4f94582f082fde74ff66c85003ce6de63a936f2a8` |
| `src/cmd/planar/authoring_catalog.t.cpp` | untracked | `110674c27e1a34764eb710f85213590444a5e18263ee61bea3a2d6b205ae52c8` |
| `src/cmd/planar/handlers/critique/command.cpp` | untracked | `7b153955067b45b52769dae6c79b2d34656a6dd30a125a7c3ce2f883c3c2d8cb` |
| `src/cmd/planar/handlers/critique/command.cppm` | untracked | `379fb8be986b24d469de67e4853d7d76af90ce16b74a15e6834d9c6b28354a69` |
| `src/cmd/planar/handlers/explain-rejection/command.cpp` | untracked | `7af9d33c599e3557677c836075a6b917ce59c7a20e618f35675bb520a95760e3` |
| `src/cmd/planar/handlers/explain-rejection/command.cppm` | untracked | `dbfe243381fbc299c1014049b1f7103b96a60a6f214381efa3c591756103a58c` |
| `src/cmd/planar/handlers/ingest/command.cpp` | untracked | `ae1935f5e0f177aa374b84cec4f4027d7c1e4857c63e1a27bbdf87c21f3ac0dd` |
| `src/cmd/planar/handlers/ingest/command.cppm` | untracked | `e061a72ff6b6e3bc3487cb6b459791ad2d4b0beea81bc8f6a52128c4dc4e27ec` |
| `src/cmd/planar/handlers/observe/command.cpp` | untracked | `ef5485db8381cbc7d600eeebeeb2d140944dd7f21bc0fb4e0a4bbc0b42ec272d` |
| `src/cmd/planar/handlers/observe/command.cppm` | untracked | `4a7c212beec81cfb1c66e68e15e3f6864cdec001afeb8bf0961e0fc341023883` |
| `src/cmd/planar/handlers/orchestrator/command.cpp` | untracked | `f903ec66255b5d6e853ca0788660affc006cdf7cd176bc47e2d626c36e1cbc94` |
| `src/cmd/planar/handlers/orchestrator/command.cppm` | untracked | `59f63fa6e29478543b1a652cb993d4c11935408ae92935632d21765ff9626dc7` |
| `src/cmd/planar/handlers/spec-prompt/command.cpp` | untracked | `0a836e7a89bcf76ddc3c996086f0c4a2563c0589be39e553acbb43356cac515a` |
| `src/cmd/planar/handlers/spec-prompt/command.cppm` | untracked | `a603095ed5196e7169d49d3ab0e9762835a3e9186d2c1ed3200cf0055266ebb1` |

The shared checkout was never edited; the only files written there are under `tests/native/v5-02-ra01-b-w1-c01-evidence/probes/`.

## What the repair changed (why these rows)

Against the candidate probed in superseded-1, only two files differ: `src/engine/runtime/external_registration.t.cpp` (23b7e4fa... to 5386b9d9...: one NEW test `RA01-B1 reserved pre check continues past a malformed entry sorted before a reserved entry`) and `src/cmd/planar/authoring.t.cpp` (bfa67729...; additions only: a leaf-below-a-regular-file assertion in the explicit-path-stage test and `--from` text-validation plus valid-relative-`--from` cwd assertions in the cwd test). `diff` of the scratch (previous) copy against the checkout shows only added lines (no `<` lines), and every product file hash (`runtime.cpp` 3911b25e..., `authoring.cpp` 0bd890d8..., `commands.lua` 23b597fa..., `bootstrap.cpp`, handlers) is identical to superseded-1. Inventory 1343 plus 1 new test name = 1344 (selection 74 vs 73 before).

## Method

Warm scratch `scratchpad/probe-warm` (outside the checkout, kept), synced with `rsync -a --delete --exclude build/ --exclude .git/ --exclude 'tests/native/*-evidence/'`, `ninja -C build/rewrite-debug planar_runtime_tests planar_authoring_catalog_tests planar_command_tests` (exit 0). Baseline on the unmutated candidate: `-R 'RA01-B1|X08 C01'` matches 74, 74 pass. Driver `scratchpad/prober/c01drv.py`: exact one-occurrence anchor patch (a failed apply is `patch-failed`, never a kill), build, `ctest -R <selection> -j8 --output-on-failure`, restore by byte copy with sha256 verification (every raw log ends `verified=True`). Selection matched count must equal baseline for the same selection (74 for the default selection; 71 for the older wave 1 rows E07, E08, E09, E10, E11, E15, I35, I42, whose regex `RA01-B1` matches 71 on the baseline); no run was `invalid-selection`. A kill is an assertion failure (kind `Failed`) by a named test. SANITY (rebuild from restored files plus the 74-test selection) after run 12 and at the end of each batch: every line reads `100% tests passed out of 74`, so no stale mutated binary survived. Raw logs: `probes/raw/<id>.log` (diff, diff sha256, pre-apply sha256, build and test commands with exit codes, matched/baseline/failed, failing test names and excerpts, restore proof) plus `.build.txt`/`.ctest.txt`; `probes/results.jsonl` is append-only (it keeps the first C-SKIP-ALL attempt, `build-failed`: the driver's first definition was the superseded-1 one without `static_cast<void>(&reserved_string_present);`, a compile break under `-Werror`, never a kill; rerun with the corrected mutant from `c01-rerun.json`, same diff sha256 `5a9d6e52a6f6ea78` as superseded-1, killed). `raw/focus-driver-output.txt` is the driver console output of the 21-row batch.

## Part 1: rows RUN in this re-probe (22)

Result column: `killed` needs a named failing test. `named killer` = Y when the test named for the row is among the failing tests. For the X rows the named test is the one the repair added or changed.

| row | diff sha256 (16) | result | failing tests | matched/baseline | named killer failed |
|---|---|---|---|---|---|
| C-SKIP-N | `3fcaac9ee1b8db85` | survived | - | 74/74, fail 0 | N |
| C-PROJ-EQ | `32dc0957a05b0bc6` | survived | - | 74/74, fail 0 | N |
| C-NOAT | `7f26868e3b63e15c` | survived | - | 74/74, fail 0 | N |
| E07 | `b70fd59fa12bec2b` | survived | - | 71/71, fail 0 | N |
| E15 | `277c2af5bc983705` | survived | - | 71/71, fail 0 | N |
| I35 | `93c7d3e1a0e4c0d6` | survived | - | 71/71, fail 0 | N |
| X-C01-04 | `51ed959df2ff527b` | killed | RA01-B1 path text validation precedes the cwd read | 74/74, fail 1 | Y |
| X-C01-06 | `61f5275ae5262d39` | killed | RA01-B1 reserved pre check continues past a malformed entry sorted before a reserved entry | 74/74, fail 1 | Y |
| X-C01-09 | `8ef0523d175a18ae` | survived | - | 74/74, fail 0 | N |
| X-C01-10 | `ed601c8e2e07819b` | survived | - | 74/74, fail 0 | N |
| C-R3-REG | `5c8eafa12e24850c` | killed | RA01-B1 explicit path stage precedes the stub refusal; RA01-B1 selector stage precedes the explicit path stage; RA01-B1 missing leaf remap applies only to diagnostic and from; RA01-B1 explicit path stage keeps the existing messages | 74/74, fail 4 | Y |
| C-R3-DIAG | `433d70f0f61d593c` | killed | RA01-B1 explicit path stage keeps the existing messages; RA01-B1 missing leaf remap applies only to diagnostic and from; RA01-B1 explicit path stage precedes the stub refusal | 74/74, fail 3 | Y |
| C-R3-FROM | `2e3bc9ef5b7fdb78` | killed | RA01-B1 explicit path stage precedes the stub refusal; RA01-B1 missing leaf remap applies only to diagnostic and from; RA01-B1 explicit path stage keeps the existing messages | 74/74, fail 3 | Y |
| C-R3-NARROW | `f9bb6a50a72b5842` | killed | RA01-B1 explicit path stage keeps the existing messages; RA01-B1 missing leaf remap applies only to diagnostic and from | 74/74, fail 2 | Y |
| E08 | `7a62b359c60742ef` | killed | RA01-B1 path text validation precedes the cwd read | 71/71, fail 1 | Y |
| E09 | `eb2ce189560cac45` | killed | RA01-B1 path text validation precedes the cwd read; RA01-B1 stages read the cwd at most once and only for relative paths; RA01-B1 only workspace verbs read the workspace environment | 71/71, fail 3 | Y |
| E10 | `ddf7a8894633ceaf` | killed | RA01-B1 explicit path stage precedes the stub refusal; RA01-B1 missing leaf remap applies only to diagnostic and from; RA01-B1 explicit path stage keeps the existing messages | 71/71, fail 3 | Y |
| E11 | `482eade8183fc52b` | killed | RA01-B1 explicit path stage keeps the existing messages; RA01-B1 missing leaf remap applies only to diagnostic and from; RA01-B1 explicit path stage precedes the stub refusal | 71/71, fail 3 | Y |
| I42 | `d2018be3ea961a7c` | killed | RA01-B1 explicit path stage keeps the existing messages; RA01-B1 missing leaf remap applies only to diagnostic and from; RA01-B1 explicit path stage precedes the stub refusal; RA01-B1 each authoring verb parses its signature through the bundle to the stub refusal (+13 more) | 71/71, fail 17 | Y |
| C-SKIP-H | `b948fff3e27b2081` | killed | RA01-B1 reserved pre check skips an entry with any ill typed identity field | 74/74, fail 1 | Y |
| C-SKIP-F | `d9d8bcc37a4c3d9a` | killed | RA01-B1 reserved pre check skips an entry with any ill typed identity field | 74/74, fail 1 | Y |
| C-SKIP-ALL | `5a9d6e52a6f6ea78` | killed | RA01-B1 reserved pre check skips an entry with any ill typed identity field | 74/74, fail 1 | Y |

Verdicts for the 10 requested rows:

| row | verdict | evidence |
|---|---|---|
| C-SKIP-N | survived, EQUIVALENT (carried reason confirmed by a fresh run) | 74/74 pass. `text_field(d, "name")` in `collect_final_descriptors` refuses a non-string command name before `reserved_authoring_identity` runs, so the name skip is unobservable. |
| C-PROJ-EQ | survived, EQUIVALENT (confirmed by a fresh run) | 74/74 pass. An ill-typed projection id is refused by the same early `text_field` call. |
| C-NOAT | survived, REVIEWED LIMIT (confirmed by a fresh run) | 74/74 pass. A missing descriptor field cannot be built from Lua (`exact()`), so `.at("descriptor")` is unobservable; no test invented. |
| C-E07 (E07) | survived, EQUIVALENT (confirmed by a fresh run) | 71/71 pass. `expand_workspace` already validates an explicit selector (`host_path.cpp` 201-204); the `authoring.cpp` statement is redundant. |
| C-E15 (E15) | survived, EQUIVALENT (confirmed by a fresh run) | 71/71 pass. `help` alone already forces usage at `bootstrap.cpp` 424. |
| C-I35 (I35, single edit) | survived, EQUIVALENT (confirmed by a fresh run) | 71/71 pass. The parsed-token count already refuses equals syntax; the double-skip I35b (carried, killed) is the killer. |
| X-C01-04 | KILLED (was a survivor) | `RA01-B1 path text validation precedes the cwd read` fails at `authoring.t.cpp:1015` `CHECK( counts.cwd == 0 )`: `1 == 0`. Named killer Y. Repair verified. |
| X-C01-06 | KILLED (was a survivor) | `RA01-B1 reserved pre check continues past a malformed entry sorted before a reserved entry` fails at `external_registration.t.cpp:5428` `CHECK( vm.error().detail.code == "reference" )`: `"internal" == "reference"`. Named killer Y. Repair verified. |
| X-C01-09 | SURVIVED, equivalent mutant (defensive arm; review RA01-B-W1-C01-R1 supersedes the earlier open-gap verdict) | 74/74 pass under the mutant. `symlink_status("<regular file>/leaf", ec)` sets ENOTDIR but returns `not_found`, so the other arm fires; `host_file` uses the throwing form, which throws for ENAMETOOLONG, so the guard is false and no fixture reaches the `ignored ||` arm. The assertion lives in `RA01-B1 missing leaf remap applies only to diagnostic and from` and is a behaviour pin. |
| X-C01-10 | survived, EQUIVALENT (reason confirmed by a fresh run) | 74/74 pass. A symlink leaf is reported by `resolve_explicit_file` as `path_denied` / `Explicit input file is a symbolic link`, so the regular-file-message guard is false before the absent-leaf test. |

Rows run because the repair changed their test file, all killed by their named test (no regression): C-R3-REG (4 failing), C-R3-DIAG (3), C-R3-FROM (3), C-R3-NARROW (2), E08, E09 (3), E10 (3), E11 (3), I42 (17), C-SKIP-H, C-SKIP-F, C-SKIP-ALL (1 each, rerun after the first build-failed attempt).

## Part 2: rows CARRIED FORWARD from `probes/superseded-1/PROBES.md` (not re-run)

Proof: every product file (all of `runtime.cpp`, `authoring.cpp`, `commands.lua`, `bootstrap.cpp`, `response.*`, `CMakeLists.txt`, handlers, `authoring.cppm`) has the same sha256 as in superseded-1, and the two test files that changed contain only ADDED lines (a new TEST_CASE and extra assertions in existing ones; `diff` shows no removed or edited line), so a test that failed under a mutant before still fails under it. Carried rows (all results as recorded in superseded-1/PROBES.md and raw/): C-E17a to d, C-SKIP-CTRL, C-I53, C-I06, C-I14, C-R5-PIN, C-I12a/g, C-I14, C-I15 and C-count-only (I24 contrast, masking by design), E01 to E06, E12 to E14, E16 to E19, I01 to I05, I07 to I11, I12b to I12f, I13 to I23, I25 to I33, I35b, I36 to I41c, I43a to I58, I65, I66, I40b, I40c, I52. Exploratory rows carried: X-C01-01 to X-C01-03, X-C01-05, X-C01-07, X-C01-08 (all killed in superseded-1; their killer tests were not edited except by additions). No wave 1 row lacks a run or this carry-forward proof. Rows with no table entry (documentation rows I59 to I63, I64, I44 strong form, I24, I34) are unchanged from superseded-1 (not mutation testable).

C-I06 on this candidate: carried with the diff and no `patch-failed` in `superseded-1/raw/C-I06.log`; commands.lua and runtime.cpp are byte-identical, so it still applies.

## Red-first table (every test added in this round lineage)

| test (added or changed) | mutant that fails it | row | run or carried |
|---|---|---|---|
| RA01-B1 reserved strings in non reserved fields keep their HEAD diagnostic | commands loop reserves response_kind / help / options name / capabilities[1] | C-E17a, b, c, d (E17) | carried (superseded-1: killed, named Y, 1 failing test each) |
| RA01-B1 reserved pre check skips an entry with any ill typed identity field | delete the handler_id skip; delete the family skip; delete all three skips | C-SKIP-H, C-SKIP-F, C-SKIP-ALL | run (killed, named Y) |
| RA01-B1 reserved pre check continues past a malformed entry sorted before a reserved entry (added by the repair) | a skip that ends the pre-check at the first malformed entry (`return std::nullopt`) | X-C01-06 | run (killed, named Y) |
| RA01-B1 missing leaf remap applies only to diagnostic and from | registry leak (remap true at the registry call); diagnostic remap off; --from mapping deleted | C-R3-REG, C-R3-DIAG, C-R3-FROM | run (killed, named Y) |
| RA01-B1 path text validation precedes the cwd read | delete the `validate_host_path` loop for `--registry` and `--diagnostic` (E08); skip text validation for `--from` (X-C01-04, added assertion) | E08, X-C01-04 | run (killed, named Y) |
| RA01-B1 explicit path stage keeps the existing messages (changed body) | absent-leaf condition dropped (C-R3-NARROW); `--from` resolved as a file (I42); conversion removed (E10, E11) | C-R3-NARROW, I42, E10, E11 | run (killed, named Y) |
| RA01-B1 missing leaf remap applies only to diagnostic and from, `<regular file>/leaf` assertion | dropping the `ignored ||` arm | X-C01-09 | run: SURVIVED, the assertion cannot fail under that mutant on this platform (see verdict); it is a behaviour pin, not a red-first test |
| RA01-B1 selector stage precedes the explicit path stage; RA01-B1 explicit path stage precedes the stub refusal (changed bodies) | registry leak; explicit paths resolved before the selector (I40b); stub refusal before the stage function (I40c) | C-R3-REG, I40b, I40c | carried (superseded-1: killed, named Y) |

## Survivors and skipped

Survivors: C-SKIP-N, C-PROJ-EQ, E07, E15, I35 (single edit) and X-C01-10 are equivalent mutants with reasons above; C-NOAT is a reviewed limit; X-C01-09 is a defensive arm, an equivalent mutant (review R1). Skipped: no exploratory mutants beyond the ten requested X and equivalence rows were added (0 of the 20 allowed used in this pass); the full table rows were not re-run where the carry-forward proof applies. Not run at all: nothing else.

Name notes (review R4): in Part 2 the name C-I14 appears for two rows. The plain I14 row (`validator refuses a duplicated authoring row`) was run and killed; the masking-contrast row C-I14 in `probes/superseded-1/` (I14 plus the re-added HEAD-style count refusal) survives by design. The plain I12a and I12g rows were also run and killed (superseded-1) and are carried.
