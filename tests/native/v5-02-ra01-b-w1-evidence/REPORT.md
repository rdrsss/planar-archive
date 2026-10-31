# REPORT V5-02-RA01-B-W1 gate run 6 (2026-10-30)

Gate runner report. No source file was edited by the runner. Raw logs: `tests/native/v5-02-ra01-b-w1-evidence/raw/run6/` (earlier runs untouched; the run 5 report is kept as `raw/run6/prev-REPORT-run5.md`).
Run 6 is the second gate after the probe repair and the operator-decided re-pin (2026-10-30), measured by the coordinator of the X08 C01 outputs-off boundary (macOS 227525, Linux 227551).

Verdict: **passed = true**. Every command met its criterion. Test #674 (`X08 C01 role preparation outputs-off boundary pins the request-record conversion charge`), the single run 5 failure, now passes in all four full runs.
The only non-zero exit is the no-argument `redact-local-paths.py check`, which the brief declares not meaningful (it scans ignored build output and older evidence); the explicit-file check exits 0.

Gate order: identity, lint, build, focused ctest, full ctest, inventory, no-work builds, redaction, Linux gate.

## Candidate identity

HEAD `307026288615e317d145ba6846c0692929c50a17`. `sha256(git diff --binary HEAD)` (all tracked changes, including docs): `fc0a6cd731c4482305fb420e2bdb7890f4c9338bc74f43b590cd781526b885df` (`raw/run6/diff-sha.txt`).
`identity.py` output verbatim (`raw/run6/identity.txt`):

| identity | value |
|---|---|
| HEAD | `307026288615e317d145ba6846c0692929c50a17` |
| sha256(git diff --binary HEAD -- src tests/native), tracked only | `519dec7b4aa45564700f2c0b1f7d7197a63b6382d2e9eb223898c1ae0acdcc85` |
| combined sha256 (diff bytes, then each untracked path NUL bytes, sorted), pathspec src tests/native, evidence excluded | `588df1672b82a46079a07c54d5f171144e9a697229d7401bcf6c1e79bb5922ff` |

| file | state | sha256 |
|---|---|---|
| `src/cmd/planar/CMakeLists.txt` | modified | `a500e63c899b433cf8d253fb5f8dbe80061074b4d728c151940aeb5e9d75a823` |
| `src/cmd/planar/bootstrap.cpp` | modified | `9db6993381e4cbea3cbc2cb432504321b245dd7b2d341dbd2320917aa74f05a8` |
| `src/cmd/planar/bootstrap.t.cpp` | modified | `44684743e0b8f2b92955641dc8400baa8fb8f03da82bdcda72de05d3cc583bf9` |
| `src/cmd/planar/response.cpp` | modified | `233af0d52232e51855d0dfb00000fe20b5981e36c399aaf252cc6ee3ead74cb6` |
| `src/cmd/planar/response.cppm` | modified | `c007ebbfe83bd2b9f88234294b29fe9f704e279982a5eac61323d7befbf1cf31` |
| `src/engine/runtime/external_registration.t.cpp` | modified | `076de8e73b8bd3cf93e2b442ff573e7a656a590820914bfd2d0899349af0da26` |
| `src/engine/runtime/runtime.cpp` | modified | `50c6c0efb0f917125c7e87bd4a0ba752d0e3b888d39b9ed2b8bd37ac51069c3f` |
| `src/engine/runtime/runtime.t.cpp` | modified | `f8d3a1fde07a32c472f200f4c6807f0608bcccf982ccce0cc6144b69855216bc` |
| `src/stdlib/planar/commands.lua` | modified | `23b597fa889f5d05869169f9ba9980e2cb242efd6a0316ada8ab8df08988072f` |
| `src/cmd/planar/authoring.cpp` | untracked | `e0e0eb11a5654e4492727918c843cb350d4f0d142ecd37e38afc423a1a9eb506` |
| `src/cmd/planar/authoring.cppm` | untracked | `4e56274a57a410a00fda867ea1474ce1c02a892a12cce0b0e26b762a075b2d09` |
| `src/cmd/planar/authoring.t.cpp` | untracked | `22ce075df56edf3b21e616c6489a93d98da89b824b332d57a6814f8875593a6c` |
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

Only `src/engine/runtime/external_registration.t.cpp` differs from run 5 (sha256 `ed458bd5...` to `076de8e7...`), which is the re-pin.

## Commands and results

All commands ran serially from the repository root, unpiped; exit codes are read from `raw/run6/*.exit`.

| command | exit | result |
| --- | --- | --- |
| `python3 scripts/rewrite-check.py lint --build-dir build/rewrite-debug` | 0 | "Formatting and Doxygen checked 163 active source files". `raw/run6/lint.log` |
| `cmake --build build/rewrite-debug --target planar_tests`, then `cmake --build build/rewrite-debug` | 0, 0 | both `ninja: no work to do.` (the Debug tree was already current when the runner began; it had been rebuilt after the re-pin, and the focused and full Debug runs below exercise that binary) |
| `cmake --build build/rewrite-release --target planar_tests`, then `cmake --build build/rewrite-release` | 0, 0 | planar_tests rebuilt (5 steps: recompile external_registration.t.cpp, relink `planar_runtime_tests`, `raw/run6/release.build-tests.log`); full build then `ninja: no work to do.` |
| `ctest --test-dir build/rewrite-debug -R RA01-B1 --no-tests=error` (focused) | 0 | 66 matched, 100% passed, 0 failed, 0 skipped (11.04 s) |
| `ctest --test-dir build/rewrite-release -R RA01-B1 --no-tests=error` (focused) | 0 | 66 matched, 100% passed, 0 failed, 0 skipped (3.88 s) |
| `ctest --test-dir build/rewrite-debug --no-tests=error` (full) | 0 | 1339 of 1339 passed, 0 failed, 0 skipped (602.39 s) |
| `ctest --test-dir build/rewrite-release --no-tests=error` (full) | 0 | 1339 of 1339 passed, 0 failed, 0 skipped (249.72 s) |
| `ctest -N` both configurations | 0 | 1339 names each, identical to each other and to run 5 and to both Linux inventories (same sha256 `0f4eff71...fc4b` after `LC_ALL=C sort`) |
| no-work rebuild `cmake --build build/rewrite-debug --target planar_tests`, twice, then the full target | 0, 0, 0 | all `ninja: no work to do.` PASS (each follows a successful build) |
| no-work rebuild `cmake --build build/rewrite-release --target planar_tests`, twice, then the full target | 0, 0, 0 | all `ninja: no work to do.` PASS |
| `python3 scripts/redact-local-paths.py --repo-path <repo> --home <home> check <32 explicit files>` | 0 | PASS. File list `raw/run6/redact-files.txt`: the 32 edited or untracked files from `git status` (src, docs, round record), excluding the evidence directory and ignored build output |
| `python3 scripts/redact-local-paths.py --repo-path <repo> --home <home> check` (no files) | 1 | Not meaningful: 9031 lines scanning ignored build output and older evidence (`raw/run6/redact-noargs.log`). Reported alongside, as instructed |
| `df -h /System/Volumes/Data` before Linux | n/a | 41 GiB free (threshold 15), proceeded (`raw/run6/df-pre-linux.txt`); 34 GiB free after the run |
| `scripts/linux-gate.sh <scratch dir>` with the docker PATH exported | 0 | One docker build, no `-t`, Debug then RelWithDebInfo. configure, build, inventory and no-work exits all 0 in both; container ctest 1339 of 1339 passed in both (Debug 561.09 s, RelWithDebInfo 252.88 s); in-container no-work rebuilds `ninja: no work to do.`; `x07.two_process_cas_one_winner` present and passed. `docker builder prune -f` ran inside the script, exit 0. Manifests: before and after equal, host and container context equal. Logs in `raw/run6/linux/` |

## Inventory

`ctest -N` lists 1339 tests in each configuration; the sorted list is `inventory.txt` (also `raw/run6/inv-debug.txt`, `inv-release.txt`, and the Linux lists under `raw/run6/linux/container/`).
Compared with the most recent committed inventory list, `tests/native/v5-02-ra01-a-evidence/inventory.txt` (1273 names, committed in 30702628; same count as the X07-C02 list): 0 removed names (`removed-names.txt` is empty), 66 new names (`new-names.txt`), 1273 + 66 = 1339. All 66 new names contain `RA01-B1`. New names:

```
replacement.RA01-B1 a broken profile reports its own error before a reserved name collision
replacement.RA01-B1 authoring bundle runs shipped verbs like the default entry
replacement.RA01-B1 authoring capabilities are the exact per row sets
replacement.RA01-B1 authoring catalog keeps the five shipped descriptors identical
replacement.RA01-B1 authoring catalog seals the eleven RA-1 rows in sorted order
replacement.RA01-B1 authoring families refuse empty and dash prefixed values
replacement.RA01-B1 authoring families refuse positionals unknown options and help
replacement.RA01-B1 authoring families refuse repeated options
replacement.RA01-B1 authoring families require every option
replacement.RA01-B1 authoring handler ids and response kinds use underscores
replacement.RA01-B1 authoring option signatures are exactly the RA-1 table
replacement.RA01-B1 authoring root help inserts the six RA-1 lines before the final line
replacement.RA01-B1 authoring verbs do not accept each others options
replacement.RA01-B1 authoring verbs refuse missing required options as usage
replacement.RA01-B1 authoring verbs refuse positionals unknown options and equals syntax as usage
replacement.RA01-B1 authoring verbs refuse repeats empty and dash prefixed values as usage
replacement.RA01-B1 command options variant stays closed at five members
replacement.RA01-B1 commands.lua keeps one handler function and one registration line per authoring verb
replacement.RA01-B1 control bundle seals and a refused bundle maps to internal exit 4
replacement.RA01-B1 default catalog refuses authoring rows registered without the request
replacement.RA01-B1 default embedded catalog stays the five HEAD rows
replacement.RA01-B1 default invoke and production binary refuse all six authoring verbs as usage
replacement.RA01-B1 defined runtime keeps profile VMs at five rows
replacement.RA01-B1 each authoring family binds exactly its RA-1 option names
replacement.RA01-B1 each authoring verb parses its signature through the bundle to the stub refusal
replacement.RA01-B1 explain-rejection and observe ignore an invalid workspace environment
replacement.RA01-B1 explicit path stage keeps the existing messages
replacement.RA01-B1 explicit path stage precedes the stub refusal
replacement.RA01-B1 help splice inserts six lines before the final line only
replacement.RA01-B1 non reserved near miss names stay registrable
replacement.RA01-B1 only workspace verbs read the workspace environment
replacement.RA01-B1 orchestrator job value is text
replacement.RA01-B1 orchestrator requires all three options
replacement.RA01-B1 production targets never receive the authoring definition
replacement.RA01-B1 profile registering a reserved authoring command name is refused as identity conflict
replacement.RA01-B1 profile registering a reserved authoring family name is refused as identity conflict
replacement.RA01-B1 profile registering a reserved authoring handler id is refused as identity conflict
replacement.RA01-B1 profile registering a reserved projection id is refused as identity conflict
replacement.RA01-B1 projection response refuses the five orchestrator section ids for both input kinds
replacement.RA01-B1 reserved command collision field follows registration order
replacement.RA01-B1 reserved command is refused when the profile also registers a projection
replacement.RA01-B1 reserved pre check skips malformed descriptors and keeps their diagnostic
replacement.RA01-B1 reserved projection collision field follows registration order
replacement.RA01-B1 response kinds map to envelope kinds
replacement.RA01-B1 selector stage precedes the explicit path stage
replacement.RA01-B1 stages read the cwd at most once and only for relative paths
replacement.RA01-B1 stub invocations write nothing and print no success document
replacement.RA01-B1 stub refusal is the closed internal envelope on stderr only
replacement.RA01-B1 usage precedes the workspace selector stage
replacement.RA01-B1 validator refuses a catalog missing one row
replacement.RA01-B1 validator refuses a duplicated authoring row
replacement.RA01-B1 validator refuses a mispaired capability
replacement.RA01-B1 validator refuses a mispaired family
replacement.RA01-B1 validator refuses a mispaired handler id
replacement.RA01-B1 validator refuses a mispaired response kind
replacement.RA01-B1 validator refuses a missing option
replacement.RA01-B1 validator refuses a renamed option
replacement.RA01-B1 validator refuses a wrong option type
replacement.RA01-B1 validator refuses a wrong required flag
replacement.RA01-B1 validator refuses an extra twelfth row
replacement.RA01-B1 validator refuses the five shipped rows when eleven are required
replacement.RA01-B1 with-profile after admission an authoring tail is usage and a broken profile reports its own error
replacement.RA01-B1 with-profile outputs are identical under the authoring bundle
replacement.RA01-B1 with-profile probe stays five and a malformed authoring tail reaches profile capture
replacement.RA01-B1 with-profile reserved command registration prints the existing identity conflict
replacement.RA01-B1 workspace selector stage expands names and keeps files lexical
```

## Run 6 notes on (a) and (b)

- Test file:line values in table (a) were refreshed by script against the current sources (TEST_CASE line); 10 values shifted since run 5 because of the re-pin edit. Red-first assertion lines in (b) are lines in the red snapshot, not the current files.
- The 66th test, `RA01-B1 reserved command is refused when the profile also registers a projection` (`src/engine/runtime/external_registration.t.cpp:5281`, mutant comment: skip the reserved command pre-check whenever the catalog also lists a projection), has no row in `plan/INVARIANTS.md`, no red record in `red/RED.md`, `red/summary.txt` or `red/names.txt`, and no entry in `probes/PROBES.md`. It maps to criterion 4 (with-profile collisions) alongside I48 and I50. It was added after the red snapshot; its red-first status is missing from the evidence and is not invented here.
- The X08 C01 outputs-off boundary pin test (#674) maps to no RA01-B1 criterion. It is an existing X08 budget pin; it passes after the operator-decided re-pin (2026-10-30), measured by the coordinator (macOS 227525, Linux 227551, constants at `src/engine/runtime/external_registration.t.cpp:4570` and `:4573`). Acceptance criterion 5 (no regression of existing behaviour) is met: all 1273 earlier tests pass.
- No corrective round findings are closed by this round; the table has no corrective-finding rows.


## (a) Acceptance mapping

The round record lists seven acceptance criteria; the plan's invariant table (`plan/INVARIANTS.md`, revision 3) refines them into rows I01 to I66. Test names are from that table, file:line from the current sources (TEST_CASE line). Rows with test `none` are documentation or records rows with no test.

| criterion | rows | summary |
| --- | --- | --- |
| 1 eleven-row validator, five-row default byte-identical | I01-I07, I10-I24, I26 | catalog, validator refusals, default bytes |
| 2 six verbs parse the RA-1 signature, help lines | I27-I30f, I33-I37 | CLI11 families, help |
| 3 stubs open no ledger, write nothing, precedence of rule 7 | I31, I32, I38a-I42 | stubs and stages |
| 4 with-profile | I45-I55, I65, I66 | probe stays five, collisions |
| 5 no name removed, no new public message or field | I56-I58, I64 and the inventory comparison above | |
| 6 tests can fail, name a mutant | every row's mutant column in `plan/INVARIANTS.md` | mutation prober evidence is not part of this gate run |
| 7 documentation | I59-I63 | architecture, x07-addendum, records |

In run 6 all 66 new tests and all 1273 earlier tests pass in every configuration. Mutation-prober evidence is outside this runner.

| row | statement (abridged) | test name | file:line |
| --- | --- | --- | --- |
| I1 | impl | `src/engine/runtime/runtime.cpp` (flag, definition-gated selection, validator, D14 pre-check, projection id extension) | n/a |
| I2 | impl | `authoring.cppm/.cpp`, six `handlers/<verb>/command.cppm/.cpp`, `bootstrap.cpp`, `response.cppm/.cpp` | n/a |
| I01 | The production `command_vm::create()` (compiled without the definition) seals exactly the five HEAD rows (render, init, history, export, loa | RA01-B1 default embedded catalog stays the five HEAD rows | src/engine/runtime/runtime.t.cpp:4297 |
| I02 | Under the definition (control variant 0, `create()`) the catalog seals exactly the eleven rows, names sorted ascending (critique, explain-re | RA01-B1 authoring catalog seals the eleven RA-1 rows in sorted order | src/cmd/planar/authoring_catalog.t.cpp:179 |
| I03 | Handler ids are `shipped.spec_prompt.v1`, `shipped.critique.v1`, `shipped.ingest.v1`, `shipped.explain_rejection.v1`, `shipped.observe.v1`,  | RA01-B1 authoring handler ids and response kinds use underscores | src/cmd/planar/authoring_catalog.t.cpp:191 |
| I04 | Response kinds are `spec_prompt_v1`, `critique_v1`, `ingest_v1`, `explain_rejection_v1`, `observe_v1`, `orchestrator_v1` | RA01-B1 authoring handler ids and response kinds use underscores | src/cmd/planar/authoring_catalog.t.cpp:191 |
| I05 | Capabilities per row are exactly one: `authoring_capture_v1` for spec-prompt, critique, ingest; `diagnostic_read_v1` for explain-rejection;  | RA01-B1 authoring capabilities are the exact per row sets | src/cmd/planar/authoring_catalog.t.cpp:203 |
| I06 | Option signatures are exactly RA-1, all type `text`, required, sorted by name: workspace+registry (spec-prompt, critique, ingest), `--diagno | RA01-B1 authoring option signatures are exactly the RA-1 table | src/cmd/planar/authoring_catalog.t.cpp:216 |
| I07 | The five shipped descriptors in the control (eleven-row) catalog are identical to the production default catalog's five descriptors (the aut | RA01-B1 authoring catalog keeps the five shipped descriptors identical | src/cmd/planar/authoring_catalog.t.cpp:235 |
| I08 | `command_options` stays a closed five-member variant (the six families are an embedded-only set outside it); observed with a RUNTIME `CHECK( | RA01-B1 command options variant stays closed at five members | src/engine/runtime/runtime.t.cpp:4357 |
| I09 | commands.lua keeps the frozen layout: six `local function <name>_command(` definitions (spec_prompt, critique, ingest, explain_rejection, ob | RA01-B1 commands.lua keeps one handler function and one registration line per authoring verb | src/engine/runtime/runtime.t.cpp:4360 |
| I10 | Harness validity: the unedited control variant 0 (definition attached) seals eleven rows through `create()`, and a refused bundle error (`in | RA01-B1 control bundle seals and a refused bundle maps to internal exit 4 | src/cmd/planar/authoring_catalog.t.cpp:252 |
| I11 | The default validator (variant 18, built WITHOUT the definition, so `create()` selects five rows) refuses a commands.lua whose gate is `if t | RA01-B1 default catalog refuses authoring rows registered without the request | src/cmd/planar/authoring_catalog.t.cpp:265 |
| I12a | Authoring bundle missing `spec-prompt` (variant 1, defined) is `internal` exit 4 | RA01-B1 validator refuses a catalog missing one row | src/cmd/planar/authoring_catalog.t.cpp:272 |
| I12b | missing `critique` (variant 2) | RA01-B1 validator refuses a catalog missing one row | src/cmd/planar/authoring_catalog.t.cpp:272 |
| I12c | missing `ingest` (variant 3) | RA01-B1 validator refuses a catalog missing one row | src/cmd/planar/authoring_catalog.t.cpp:272 |
| I12d | missing `explain-rejection` (variant 4) | RA01-B1 validator refuses a catalog missing one row | src/cmd/planar/authoring_catalog.t.cpp:272 |
| I12e | missing `observe` (variant 5) | RA01-B1 validator refuses a catalog missing one row | src/cmd/planar/authoring_catalog.t.cpp:272 |
| I12f | missing `orchestrator` (variant 6) | RA01-B1 validator refuses a catalog missing one row | src/cmd/planar/authoring_catalog.t.cpp:272 |
| I12g | missing shipped `load` (variant 7) in the authoring bundle | RA01-B1 validator refuses a catalog missing one row | src/cmd/planar/authoring_catalog.t.cpp:272 |
| I13 | The expected row set follows the selected bundle: the five-row catalog (variant 19, gate `if false then`, DEFINED so eleven rows are expecte | RA01-B1 validator refuses the five shipped rows when eleven are required | src/cmd/planar/authoring_catalog.t.cpp:278 |
| I14 | A duplicated row (variant 9: critique twice, twelve entries, all eleven names present) is refused | RA01-B1 validator refuses a duplicated authoring row | src/cmd/planar/authoring_catalog.t.cpp:283 |
| I15 | An extra twelfth row (variant 8: all eleven plus `status`) is refused | RA01-B1 validator refuses an extra twelfth row | src/cmd/planar/authoring_catalog.t.cpp:288 |
| I16 | A mispaired family (variant 10: observe family `ingest`) is refused | RA01-B1 validator refuses a mispaired family | src/cmd/planar/authoring_catalog.t.cpp:293 |
| I17 | A mispaired handler id (variant 11) is refused | RA01-B1 validator refuses a mispaired handler id | src/cmd/planar/authoring_catalog.t.cpp:298 |
| I18 | A mispaired response kind (variant 12: `observe_v2`) is refused | RA01-B1 validator refuses a mispaired response kind | src/cmd/planar/authoring_catalog.t.cpp:303 |
| I19 | A mispaired capability (variant 13: observe declares `authoring_capture_v1`) is refused | RA01-B1 validator refuses a mispaired capability | src/cmd/planar/authoring_catalog.t.cpp:308 |
| I20 | A row with a missing option (variant 14: orchestrator without `--job`) is refused | RA01-B1 validator refuses a missing option | src/cmd/planar/authoring_catalog.t.cpp:313 |
| I21 | A renamed option of equal count (variant 15: `--jobs` for `--job`) is refused. Unmasked and defined: the mutant substitutes a valid iterator | RA01-B1 validator refuses a renamed option | src/cmd/planar/authoring_catalog.t.cpp:318 |
| I22 | A wrong option type (variant 16: spec-prompt `--registry` decimal) is refused | RA01-B1 validator refuses a wrong option type | src/cmd/planar/authoring_catalog.t.cpp:323 |
| I23 | A wrong required flag (variant 17: explain-rejection `--diagnostic` optional) is refused | RA01-B1 validator refuses a wrong required flag | src/cmd/planar/authoring_catalog.t.cpp:328 |
| I24 | Unmasking proof, prober-run meta row: each mutant of I11 to I23 is applied alone in a scratch copy; the failing AC test output shows `expect | all AC tests (prober log, not a TEST_CASE) | n/a |
| I25 | Production never selects an authoring verb: DEFAULT_INVOKE refuses all six as `usage` exit 2 with the existing usage bytes (`detail::usage() | RA01-B1 default invoke and production binary refuse all six authoring verbs as usage | src/cmd/planar/bootstrap.t.cpp:3287 |
| I26 | Default catalog (five) and default root help bytes are byte-identical to HEAD (existing tests) | RA01-B1 default embedded catalog stays the five HEAD rows | src/engine/runtime/runtime.t.cpp:4297 |
| I27 | Authoring root help = HEAD five-line text with the six RA-1 lines in exact order and bytes inserted immediately before the final `planar --h | RA01-B1 authoring root help inserts the six RA-1 lines before the final line | src/cmd/planar/authoring.t.cpp:353 |
| I28 | `splice_root_help` inserts only before the final line, leaves every other byte, and returns nullopt when the final line is absent | RA01-B1 help splice inserts six lines before the final line only | src/cmd/planar/authoring.t.cpp:337 |
| I29 | `authoring_envelope_kind`: five prompt kinds give `prompt`, `observe_v1` gives the engaged empty string, the five shipped kinds give `render | RA01-B1 response kinds map to envelope kinds | src/cmd/planar/authoring.t.cpp:362 |
| I30a | Each family binds exactly its RA-1 option names (long names only, no short names, no positional), a valid parse stores each value | RA01-B1 each authoring family binds exactly its RA-1 option names | src/cmd/planar/authoring.t.cpp:387 |
| I30b | Every RA-1 option of every family is required (omitting any one throws `CLI::ParseError`) | RA01-B1 authoring families require every option | src/cmd/planar/authoring.t.cpp:407 |
| I30c | Repeating any option throws | RA01-B1 authoring families refuse repeated options | src/cmd/planar/authoring.t.cpp:426 |
| I30d | Empty and `--`-prefixed values throw for every option | RA01-B1 authoring families refuse empty and dash prefixed values | src/cmd/planar/authoring.t.cpp:439 |
| I30e | A positional token, an unknown option and `--help` on the verb throw (no extras) | RA01-B1 authoring families refuse positionals unknown options and help | src/cmd/planar/authoring.t.cpp:456 |
| I30f | `orchestrator --job` is text: a decimal-looking value and a non-numeric value both parse and are stored | RA01-B1 orchestrator job value is text | src/cmd/planar/authoring.t.cpp:471 |
| I31 | Through STACK_INVOKE each verb with its exact signature and existing files reaches the stub refusal: exit 4 and the stub bytes (proves catal | RA01-B1 each authoring verb parses its signature through the bundle to the stub refusal | src/cmd/planar/authoring.t.cpp:486 |
| I32 | STACK_INVOKE runs the shipped verbs exactly as DEFAULT_INVOKE (same bytes for `history --ledger <missing>` and a usage case) | RA01-B1 authoring bundle runs shipped verbs like the default entry | src/cmd/planar/authoring.t.cpp:495 |
| I33 | Every missing required option of the six verbs (each option alone omitted, and no options) is `usage` exit 2 with the existing bytes | RA01-B1 authoring verbs refuse missing required options as usage | src/cmd/planar/authoring.t.cpp:518 |
| I34 | Repeated option, empty value and `--`-prefixed value are `usage` for the six verbs | RA01-B1 authoring verbs refuse repeats empty and dash prefixed values as usage | src/cmd/planar/authoring.t.cpp:531 |
| I35 | Positionals, unknown options, `--opt=value` spelling and `--help` after the verb are `usage` | RA01-B1 authoring verbs refuse positionals unknown options and equals syntax as usage | src/cmd/planar/authoring.t.cpp:548 |
| I36 | `orchestrator` needs all three of `--workspace`, `--registry`, `--job`; omitting any one is `usage`; a full set reaches the stub | RA01-B1 orchestrator requires all three options | src/cmd/planar/authoring.t.cpp:575 |
| I37 | Options are not interchangeable between verbs (`observe --workspace`, `spec-prompt --job`, `explain-rejection --from`, `critique --diagnosti | RA01-B1 authoring verbs do not accept each others options | src/cmd/planar/authoring.t.cpp:587 |
| I38a | A stub invocation writes no file and changes no file: the scratch tree listing and every file's bytes (workspace, registry, diagnostic, obse | RA01-B1 stub invocations write nothing and print no success document | src/cmd/planar/authoring.t.cpp:612 |
| I38b | A stub invocation prints no success document: stdout empty, status 4 | RA01-B1 stub invocations write nothing and print no success document | src/cmd/planar/authoring.t.cpp:612 |
| I39 | The stub refusal is exactly `{"error":{"code":"internal","field":"","message":"Invalid embedded command protocol"},"kind":"error","version": | RA01-B1 stub refusal is the closed internal envelope on stderr only | src/cmd/planar/authoring.t.cpp:626 |
| I40a | P2: usage beats selector. `--help spec-prompt --workspace <bad selector> --registry R` (post-parse usage check) is usage, not the selector's | RA01-B1 usage precedes the workspace selector stage | src/cmd/planar/authoring.t.cpp:648 |
| I40b | P2: selector beats path. A bad workspace selector (`path_denied` / `Invalid explicit input path`, exit 3) wins over a missing registry (`io` | RA01-B1 selector stage precedes the explicit path stage | src/cmd/planar/authoring.t.cpp:660 |
| I40c | P2: path beats stub. A valid selector and a missing registry/diagnostic/from reports the path stage's `io` error, never the stub bytes; with | RA01-B1 explicit path stage precedes the stub refusal | src/cmd/planar/authoring.t.cpp:678 |
| I41a | Only `spec-prompt`, `critique`, `ingest`, `orchestrator` read the workspace environment, only for a bare workspace name, exactly once; `expl | RA01-B1 only workspace verbs read the workspace environment | src/cmd/planar/authoring.t.cpp:695 |
| I41b | The cwd is read at most once per invocation and only when a selected path is relative (counting reader; zero reads when all paths are absolu | RA01-B1 stages read the cwd at most once and only for relative paths | src/cmd/planar/authoring.t.cpp:742 |
| I41c | `explain-rejection` and `observe` through STACK_INVOKE ignore an invalid `PLANAR_WORKSPACE_DIR` (relative value), while `spec-prompt` with a | RA01-B1 explain-rejection and observe ignore an invalid workspace environment | src/cmd/planar/authoring.t.cpp:787 |
| I42 | The explicit-path stage keeps the existing messages, per contract Messages table: `--diagnostic` missing `io`/`Cannot resolve explicit input | RA01-B1 explicit path stage keeps the existing messages | src/cmd/planar/authoring.t.cpp:805 |
| I43 | The selector stage expands a bare name to `<PLANAR_WORKSPACE_DIR>/<name>/workspace.lua`, keeps an explicit workspace file lexical (relative  | RA01-B1 workspace selector stage expands names and keeps files lexical | src/cmd/planar/authoring.t.cpp:863 |
| I44 | The definition reaches only test variant targets and no production path names an authoring selector: (a) source scan through `__FILE__` of ` | RA01-B1 production targets never receive the authoring definition | src/cmd/planar/bootstrap.t.cpp:3331 |
| I45 | With-profile probe membership stays five: a malformed authoring-looking tail (`<verb> --from`, `<verb> --bogus`) under the MALFORMED profile | RA01-B1 with-profile probe stays five and a malformed authoring tail reaches profile capture | src/cmd/planar/bootstrap.t.cpp:3390 |
| I46 | After a successful admission a well-formed RA01 tail is `usage` with the existing message bytes (`o.usage`) for all six verbs; a broken prof | RA01-B1 with-profile after admission an authoring tail is usage and a broken profile reports its own error | src/cmd/planar/bootstrap.t.cpp:3407 |
| I47 | The with-profile outputs of STACK_INVOKE are byte-equal to DEFAULT_INVOKE (valid, malformed, spinning profile; verb tails; selected help `o. | RA01-B1 with-profile outputs are identical under the authoring bundle | src/cmd/planar/bootstrap.t.cpp:3422 |
| I48 | A profile registering a command named any of the six verbs is refused `reference` / `External registration identity conflict` with the REGIS | RA01-B1 profile registering a reserved authoring command name is refused as identity conflict | src/engine/runtime/external_registration.t.cpp:5238 |
| I48b | The reported command field follows registration order, not sort order: (i) two external commands, the non-reserved `zzz_first` registered fi | RA01-B1 reserved command collision field follows registration order | src/engine/runtime/external_registration.t.cpp:5243 |
| I49 | A profile registering a command `mine` whose family is any of the six verb names is refused `reference` / `External registration identity co | RA01-B1 profile registering a reserved authoring family name is refused as identity conflict | src/engine/runtime/external_registration.t.cpp:5249 |
| I50 | A profile registering a command `mine` whose handler id is any of the six `shipped.<id>.v1` ids is refused with the same error and field `ca | RA01-B1 profile registering a reserved authoring handler id is refused as identity conflict | src/engine/runtime/external_registration.t.cpp:5259 |
| I51 | A profile registering projection id `orchestrator` or any of the five `orchestrator_*` ids is refused `reference` / `External registration i | RA01-B1 profile registering a reserved projection id is refused as identity conflict | src/engine/runtime/external_registration.t.cpp:5269 |
| I51b | The reported projection field follows registration order, not sort order: two projections, non-reserved `zzz` registered first and a reserve | RA01-B1 reserved projection collision field follows registration order | src/engine/runtime/external_registration.t.cpp:5274 |
| I52 | Reservation is exact: commands `observer`, `status`, `orchestrate`; handler `custom.observe`; projections `orchestrate`, `orchestrator_extra | RA01-B1 non reserved near miss names stay registrable | src/engine/runtime/external_registration.t.cpp:5295 |
| I53 | The pre-check never throws and skips malformed entries: a command whose `handler_id` is a number or whose `family` is a table keeps the HEAD | RA01-B1 reserved pre check skips malformed descriptors and keeps their diagnostic | src/engine/runtime/external_registration.t.cpp:5318 |
| I54 | A broken profile reports its own error first: modules `[command("observe"), "error('boom')"]` give exactly the error of `["error('boom')"]`  | RA01-B1 a broken profile reports its own error before a reserved name collision | src/engine/runtime/external_registration.t.cpp:5331 |
| I55 | Through the real CLI (DEFAULT_INVOKE), `with-profile --stdlib-profile P observe --from X` where P registers command `observe` prints the exi | RA01-B1 with-profile reserved command registration prints the existing identity conflict | src/cmd/planar/bootstrap.t.cpp:3448 |
| I56 | No new public code or message (XD4): every message literal in new and old `src/cmd/planar` sources is in the closed table (existing test sca | existing: X07 command sources add no diagnostic outside the closed table | n/a |
| I57 | No profile VM creation site in the new cmd sources (`command_vm::create(` followed by `)`, no `extension_invocation::create`) | existing: X07 src cmd contains no profile VM creation site | n/a |
| I58 | The factory-call scan is unchanged by the new sources (one opaque open, one adoption, two closed-factory calls) | existing: X07 cmd sources open the profile owner through one factory call per branch | n/a |
| I59 | `docs/architecture.md` describes the embedded-only six families, the compile-time bundle selection (definition `PLANAR_AUTHORING_BUNDLE`, in | none (documentation; G13 checks the changed sections exist) | n/a |
| I60 | x07-addendum 1.2 has six admission rows, 1.3 the embedded-only family set paragraph, 1.6 the compile-time variant paragraph WITHOUT any `cre | none (documentation; reviewer checklist; G13) | n/a |
| I61 | x07-addendum 1.8 and `profile-preparation-architecture.md:232` carry the byte-identical sentence ending `and a sixth with-profile family req | none (G12 byte comparison of the two lines) | n/a |
| I62 | D13 opaque-slot deferral with the corrected `prepare_invocation` reason is recorded in the plan, the round record scope and the wave-2 note | none (reviewer checklist) | n/a |
| I63 | Records-only carry-over applied at the records step: status words `authoring-contract.md:4`, `v5-alignment.md:48`, `acceptance-matrix.md:57- | none (records step) | n/a |
| I64 | No public field or method is added only to make something observable and no new public code or message. The complete new export list is: `pl | none (G14 export diff) | n/a |
| I65 | Under the definition the PROFILE VM stays five rows (D3 non-profile clause): in the control variant 0, `create(profile)` of a valid profile  | RA01-B1 defined runtime keeps profile VMs at five rows | src/cmd/planar/authoring_catalog.t.cpp:333 |
| I66 | Projection RESPONSES carrying any of the five section ids `orchestrator_authority`, `orchestrator_graph`, `orchestrator_roles`, `orchestrato | RA01-B1 projection response refuses the five orchestrator section ids for both input kinds | src/engine/runtime/external_registration.t.cpp:5342 |

Rows I59 to I64 have no test (documentation, scans or records). No evidence exists in this folder for the mutation prober run (I24) or for the records-step carry-over (I63); the round record's status-word changes at `authoring-contract.md:4`, `v5-alignment.md:48`, `acceptance-matrix.md:57-58` do not appear in `git status` (these files are not modified), (stale run 4 statement: in run 5 these files do appear as modified in git status, content not verified).
G12 (byte identity of x07-addendum 1.8 and `profile-preparation-architecture.md:232`), G13, G14 and G15 were not in the runner's command list and were not run.

## (b) Red-first

Source of truth: `red/RED.md` and `red/summary.txt` (coders' evidence). RED.md records: HEAD `307026288615e317d145ba6846c0692929c50a17`, snapshot `red/snapshot-2`, `candidate.diff` sha256 `1e6dfd3aeb7204ba4809b56bbdd304031effe188c4fbcb7c975895b16a996326`, `SHA256SUMS` sha256 `d23c1470893a405b1981ef677e0802b18486c2f695ab47b30e3c5a89e3ac0b02` (the earlier `red/snapshot` was superseded; its candidate.diff hash is the same). Build command recorded: `cmake --build build/rewrite-debug --target planar_tests`. Per-test command recorded: `ctest --test-dir build/rewrite-debug -R '^replacement\.<escaped name>$' --no-tests=error --output-on-failure` with logs in `red/raw/prefixed/NN.log` (NN = line in `red/raw/names.txt`). 69 tests were run: 55 expected-red tests all failed on an assertion (not a compile failure) against the stubs; guards that already passed on the stubs: I01, I08, I09, I26, I53, I56, I57, I58. RED.md names I59 to I64 as having no test. The red command was run by the coders; I did not re-run it. No red evidence exists for any behaviour added after the red snapshot (the snapshot hash is of the stub-era candidate, not of the current candidate: the current diff hash differs).

The table lists every new test with its red state taken from `red/summary.txt` (exit code 8 plus first assertion line = real red; exit code 0 = guard that already passed on the stubs):

| test name | red state | row |
| --- | --- | --- |
| RA01-B1 a broken profile reports its own error before a reserved name collision | real red (assertion at src/engine/runtime/external_registration.t.cpp:5313) | I54 |
| RA01-B1 authoring bundle runs shipped verbs like the default entry | real red (assertion at src/cmd/planar/authoring.t.cpp:514) | I32 |
| RA01-B1 authoring capabilities are the exact per row sets | real red (assertion at src/cmd/planar/authoring_catalog.t.cpp:209) | I05 |
| RA01-B1 authoring catalog keeps the five shipped descriptors identical | guard, already passed | I07 |
| RA01-B1 authoring catalog seals the eleven RA-1 rows in sorted order | real red (assertion at src/cmd/planar/authoring_catalog.t.cpp:183) | I02 |
| RA01-B1 authoring families refuse empty and dash prefixed values | real red (assertion at src/cmd/planar/authoring.t.cpp:442) | I30d |
| RA01-B1 authoring families refuse positionals unknown options and help | real red (assertion at src/cmd/planar/authoring.t.cpp:460) | I30e |
| RA01-B1 authoring families refuse repeated options | real red (assertion at src/cmd/planar/authoring.t.cpp:429) | I30c |
| RA01-B1 authoring families require every option | real red (assertion at src/cmd/planar/authoring.t.cpp:410) | I30b |
| RA01-B1 authoring handler ids and response kinds use underscores | real red (assertion at src/cmd/planar/authoring_catalog.t.cpp:197) | I03 |
| RA01-B1 authoring handler ids and response kinds use underscores | real red (assertion at src/cmd/planar/authoring_catalog.t.cpp:197) | I04 |
| RA01-B1 authoring option signatures are exactly the RA-1 table | real red (assertion at src/cmd/planar/authoring_catalog.t.cpp:222) | I06 |
| RA01-B1 authoring root help inserts the six RA-1 lines before the final line | real red (assertion at src/cmd/planar/authoring.t.cpp:358) | I27 |
| RA01-B1 authoring verbs do not accept each others options | real red (assertion at src/cmd/planar/authoring.t.cpp:609) | I37 |
| RA01-B1 authoring verbs refuse missing required options as usage | real red (assertion at src/cmd/planar/authoring.t.cpp:522) | I33 |
| RA01-B1 authoring verbs refuse positionals unknown options and equals syntax as usage | real red (assertion at src/cmd/planar/authoring.t.cpp:553) | I35 |
| RA01-B1 authoring verbs refuse repeats empty and dash prefixed values as usage | real red (assertion at src/cmd/planar/authoring.t.cpp:535) | I34 |
| RA01-B1 command options variant stays closed at five members | guard, already passed | I08 |
| RA01-B1 commands.lua keeps one handler function and one registration line per authoring verb | guard, already passed | I09 |
| RA01-B1 control bundle seals and a refused bundle maps to internal exit 4 | real red (assertion at src/cmd/planar/authoring_catalog.t.cpp:255) | I10 |
| RA01-B1 default catalog refuses authoring rows registered without the request | guard, already passed | I11 |
| RA01-B1 default embedded catalog stays the five HEAD rows | guard, already passed | I01 |
| RA01-B1 default embedded catalog stays the five HEAD rows) | no red record | I26 |
| RA01-B1 default invoke and production binary refuse all six authoring verbs as usage | real red (assertion at src/cmd/planar/bootstrap.t.cpp:3301) | I25 |
| RA01-B1 defined runtime keeps profile VMs at five rows | real red (assertion at src/cmd/planar/authoring_catalog.t.cpp:333) | I65 |
| RA01-B1 each authoring family binds exactly its RA-1 option names | real red (assertion at src/cmd/planar/authoring.t.cpp:393) | I30a |
| RA01-B1 each authoring verb parses its signature through the bundle to the stub refusal | real red (assertion at src/cmd/planar/authoring.t.cpp:490) | I31 |
| RA01-B1 explain-rejection and observe ignore an invalid workspace environment | real red (assertion at src/cmd/planar/authoring.t.cpp:792) | I41c |
| RA01-B1 explicit path stage keeps the existing messages | real red (assertion at src/cmd/planar/authoring.t.cpp:850) | I42 |
| RA01-B1 explicit path stage precedes the stub refusal | real red (assertion at src/cmd/planar/authoring.t.cpp:685) | I40c |
| RA01-B1 help splice inserts six lines before the final line only | real red (assertion at src/cmd/planar/authoring.t.cpp:340) | I28 |
| RA01-B1 non reserved near miss names stay registrable | real red (assertion at src/engine/runtime/external_registration.t.cpp:5290) | I52 |
| RA01-B1 only workspace verbs read the workspace environment | real red (assertion at src/cmd/planar/authoring.t.cpp:707) | I41a |
| RA01-B1 orchestrator job value is text | real red (assertion at src/cmd/planar/authoring.t.cpp:476) | I30f |
| RA01-B1 orchestrator requires all three options | real red (assertion at src/cmd/planar/authoring.t.cpp:579) | I36 |
| RA01-B1 production targets never receive the authoring definition | real red (assertion at src/cmd/planar/bootstrap.t.cpp:3368) | I44 |
| RA01-B1 profile registering a reserved authoring command name is refused as identity conflict | real red (assertion at src/engine/runtime/external_registration.t.cpp:5225) | I48 |
| RA01-B1 profile registering a reserved authoring family name is refused as identity conflict | real red (assertion at src/engine/runtime/external_registration.t.cpp:5225) | I49 |
| RA01-B1 profile registering a reserved authoring handler id is refused as identity conflict | real red (assertion at src/engine/runtime/external_registration.t.cpp:5224) | I50 |
| RA01-B1 profile registering a reserved projection id is refused as identity conflict | real red (assertion at src/engine/runtime/external_registration.t.cpp:5224) | I51 |
| RA01-B1 projection response refuses the five orchestrator section ids for both input kinds | real red (assertion at src/engine/runtime/external_registration.t.cpp:5328) | I66 |
| RA01-B1 reserved command collision field follows registration order | real red (assertion at src/engine/runtime/external_registration.t.cpp:5225) | I48b |
| RA01-B1 reserved pre check skips malformed descriptors and keeps their diagnostic | guard, already passed | I53 |
| RA01-B1 reserved projection collision field follows registration order | real red (assertion at src/engine/runtime/external_registration.t.cpp:5224) | I51b |
| RA01-B1 response kinds map to envelope kinds | real red (assertion at src/cmd/planar/authoring.t.cpp:367) | I29 |
| RA01-B1 selector stage precedes the explicit path stage | real red (assertion at src/cmd/planar/authoring.t.cpp:670) | I40b |
| RA01-B1 stages read the cwd at most once and only for relative paths | real red (assertion at src/cmd/planar/authoring.t.cpp:762) | I41b |
| RA01-B1 stub invocations write nothing and print no success document | real red (assertion at src/cmd/planar/authoring.t.cpp:618) | I38a |
| RA01-B1 stub invocations write nothing and print no success document | real red (assertion at src/cmd/planar/authoring.t.cpp:618) | I38b |
| RA01-B1 stub refusal is the closed internal envelope on stderr only | real red (assertion at src/cmd/planar/authoring.t.cpp:628) | I39 |
| RA01-B1 usage precedes the workspace selector stage | real red (assertion at src/cmd/planar/authoring.t.cpp:655) | I40a |
| RA01-B1 validator refuses a catalog missing one row | real red (assertion at src/cmd/planar/authoring_catalog.t.cpp:144) | I12a |
| RA01-B1 validator refuses a catalog missing one row | real red (assertion at src/cmd/planar/authoring_catalog.t.cpp:144) | I12b |
| RA01-B1 validator refuses a catalog missing one row | real red (assertion at src/cmd/planar/authoring_catalog.t.cpp:144) | I12c |
| RA01-B1 validator refuses a catalog missing one row | real red (assertion at src/cmd/planar/authoring_catalog.t.cpp:144) | I12d |
| RA01-B1 validator refuses a catalog missing one row | real red (assertion at src/cmd/planar/authoring_catalog.t.cpp:144) | I12e |
| RA01-B1 validator refuses a catalog missing one row | real red (assertion at src/cmd/planar/authoring_catalog.t.cpp:144) | I12f |
| RA01-B1 validator refuses a catalog missing one row | real red (assertion at src/cmd/planar/authoring_catalog.t.cpp:144) | I12g |
| RA01-B1 validator refuses a duplicated authoring row | real red (assertion at src/cmd/planar/authoring_catalog.t.cpp:144) | I14 |
| RA01-B1 validator refuses a mispaired capability | real red (assertion at src/cmd/planar/authoring_catalog.t.cpp:144) | I19 |
| RA01-B1 validator refuses a mispaired family | real red (assertion at src/cmd/planar/authoring_catalog.t.cpp:144) | I16 |
| RA01-B1 validator refuses a mispaired handler id | real red (assertion at src/cmd/planar/authoring_catalog.t.cpp:144) | I17 |
| RA01-B1 validator refuses a mispaired response kind | real red (assertion at src/cmd/planar/authoring_catalog.t.cpp:144) | I18 |
| RA01-B1 validator refuses a missing option | real red (assertion at src/cmd/planar/authoring_catalog.t.cpp:144) | I20 |
| RA01-B1 validator refuses a renamed option | real red (assertion at src/cmd/planar/authoring_catalog.t.cpp:144) | I21 |
| RA01-B1 validator refuses a wrong option type | real red (assertion at src/cmd/planar/authoring_catalog.t.cpp:144) | I22 |
| RA01-B1 validator refuses a wrong required flag | real red (assertion at src/cmd/planar/authoring_catalog.t.cpp:144) | I23 |
| RA01-B1 validator refuses an extra twelfth row | real red (assertion at src/cmd/planar/authoring_catalog.t.cpp:144) | I15 |
| RA01-B1 validator refuses the five shipped rows when eleven are required | real red (assertion at src/cmd/planar/authoring_catalog.t.cpp:144) | I13 |
| RA01-B1 with-profile after admission an authoring tail is usage and a broken profile reports its own error | real red (assertion at src/cmd/planar/bootstrap.t.cpp:3223) | I46 |
| RA01-B1 with-profile outputs are identical under the authoring bundle | real red (assertion at src/cmd/planar/bootstrap.t.cpp:3223) | I47 |
| RA01-B1 with-profile probe stays five and a malformed authoring tail reaches profile capture | real red (assertion at src/cmd/planar/bootstrap.t.cpp:3223) | I45 |
| RA01-B1 with-profile reserved command registration prints the existing identity conflict | real red (assertion at src/cmd/planar/bootstrap.t.cpp:1000) | I55 |
| RA01-B1 workspace selector stage expands names and keeps files lexical | real red (assertion at src/cmd/planar/authoring.t.cpp:873) | I43 |

In the current run every one of these tests passes (66 of 66 focused; full suite 1339 of 1339 in run 6), so each real red above is now green. The red evidence was produced by the coders on the stub snapshot and was not re-executed by the gate runner.

## (c) Deliberate behaviour differences

The runner found none beyond those the plan records, each tied to a contract clause by the plan (not re-verified by the runner): the stub refusal is `internal` exit 4 with the existing text `Invalid embedded command protocol` (plan I39, authoring contract Common rules and Messages, no new public message); the production default catalog and root help stay at five rows and five lines until RA01-C (RA-1); D13 opaque-slot creation deferred to wave 2 (`prepare_invocation`); the with-profile family set stays five (RA-9). The X08 C01 outputs-off boundary pin changed by operator decision of 2026-10-30 (68 bytes of padding growth in `commands.lua`); the pin and its +-8 window are the test's own, no diagnostic message changed. Documentation rows I59 to I62 were not independently verified by this run.

## Limitations

- Mutation prober, review and the records step are outside this runner's commands and have no evidence here. G12 to G15 (byte identity of x07-addendum 1.8 with `profile-preparation-architecture.md:232`, documentation sections, export diff, and the like) were not in the runner's command list and were not run. The records-only status words show as modified in git status but their content was not verified.
- The coders' red evidence was not re-executed; red snapshot hashes (`candidate.diff` sha256 `1e6dfd3a...6326`, `SHA256SUMS` sha256 `d23c1470...0ac3`, HEAD `30702628...`) are for the stub-era candidate (`red/snapshot-2`), not the current candidate. The red command per test is the one recorded in `red/RED.md`: `ctest --test-dir build/rewrite-debug -R '^replacement\.<escaped name>$' --no-tests=error --output-on-failure`.
- The Debug tree was already current at the start of the gate (no-work), so Debug compilation after the re-pin is attested only by the passing Debug runs, not by a build log; Release recompiled; the Linux image compiled from scratch.
- Full-tree `redact-local-paths.py check` fails on ignored build output and older evidence; only the explicit-file check is meaningful.
- Linux covers linux/arm64 only (script limit). The #531 timing check was covered only through the full ctest.
