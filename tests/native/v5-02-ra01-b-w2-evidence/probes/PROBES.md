# V5-02-RA01-B-W2 mutation probes (focused re-probe after survivor repair)

Prober run of 2026-10-31. Warm scratch copy `probe-warm` (outside the checkout, debug preset) synced from the checkout with `rsync -a --delete --exclude build/ --exclude .git/ --exclude 'tests/native/*-evidence/'`, rebuilt incrementally, baseline `raw/BASELINE.ctest.txt` 146/146 passed for `-R "RA01-B|X08 C01"`. Every mutant was applied by exact-anchor replacement (driver `raw/driver/drv.py`, mutant definitions `raw/driver/m6.json`), the named test run with `ctest -R`, the file restored by byte copy with sha256 verified, and the targets rebuilt after the last restore (`raw/FINAL-REBUILD.txt`, `raw/FINAL-BASELINE.ctest.txt`). Nothing in the checkout outside `tests/native/v5-02-ra01-b-w2-evidence/probes/` was written; previous results are under `superseded-1/` and nothing was deleted.

What the repair changed (product bytes unchanged, tests only): `authoring_prompts.t.cpp` (U+0086 coverage escape; decoder message/details assertions), `authoring_orchestrator.t.cpp` (exact heading assertion; inputs.files size and file digest assertions; `import planar.engine.hash`), `tests/native/path-redaction.json`. Re-run set = the requested rows plus rows whose test case the repair changed (W2-24, W2-68, X20).

## Candidate identity (identity.py, verbatim; identical before the first mutant and after the last restore, `raw/identity-before.txt`, `raw/identity-after.txt`)

| identity | value |
|---|---|
| HEAD | `3343ad38304db57388bc7ac2438a40043eba1b7d` |
| sha256(git diff --binary HEAD -- src tests/native), tracked only | `cdd3a0e14585381d3e361c5b0ba49a5e75dd8e77a7ef0412d9359aaa3bee9dfe` |
| combined sha256 (diff bytes, then each untracked path NUL bytes, sorted), pathspec src tests/native, evidence excluded | `62b5fec2dd8fcbac2bf61ad8c1913886a6361f0fb583654ddd243b015d647c36` |

| file | state | sha256 |
|---|---|---|
| `src/cmd/planar/CMakeLists.txt` | modified | `6d2f00a2d392c56c645b1e01bd382deffce3a569c48132403cfd27ce6d6cc8d1` |
| `src/cmd/planar/authoring.cpp` | modified | `80c899781f0f6efffa89f3171ac7759d07205b71d543b5496d2ec23650888cb7` |
| `src/cmd/planar/authoring.cppm` | modified | `efffaecc1b269a0753efe5a0e80afe67d85a60049bce2d0783e74dddbeaf9028` |
| `src/cmd/planar/authoring.t.cpp` | modified | `8c179675609936067d990819b6f1f905b98dd0497456468c0257e836e41eb3fb` |
| `src/cmd/planar/authoring_catalog.t.cpp` | modified | `2f652de5cc47a924c77d8e70e450f8005d219b60341bd01521f2e13dfb34a5b7` |
| `src/cmd/planar/bootstrap.cpp` | modified | `a39426cd3b52c3d5dbd1bd15a743ec74f04aeafb58eb2c9b79f60eb36d027519` |
| `src/cmd/planar/bootstrap.t.cpp` | modified | `ee7dc832a61bdc0c1a354ec7d489506f523a9b0ce3de1b720ab6e0bc4bceb610` |
| `src/cmd/planar/handlers/critique/command.cpp` | modified | `815f686ec770b57f2dd5e50120f130d2afb273a0b69737a34ca89fbe1ad04dc9` |
| `src/cmd/planar/handlers/critique/command.cppm` | modified | `96c27f9b0ddc9d4bbbac3c131d9ace4f555e25ffc699f36afaa4ebfba0fda123` |
| `src/cmd/planar/handlers/explain-rejection/command.cpp` | modified | `e0de2df4c73e8ed1678050f9daf62ce506b6a409460edf3d7e8c9d3468c13ba8` |
| `src/cmd/planar/handlers/explain-rejection/command.cppm` | modified | `ac908ca3e9bfd7442cb6c5ff08cc483e3e84420c6f8f9c3bc2e5e34fa1fa9ce4` |
| `src/cmd/planar/handlers/ingest/command.cpp` | modified | `e68549c4f7dacc6d051a56a516ed42f57cd893eb7debbeabda00c41f18f5cc5c` |
| `src/cmd/planar/handlers/ingest/command.cppm` | modified | `5a89c4a2b987f28d284a48363ccb50d9d5b3ecef0310caf968ade910b7c4a899` |
| `src/cmd/planar/handlers/observe/command.cpp` | modified | `7d5134da66b5bc27d26848cbacf2c69048a82d1dcf5a014ac721d3c89c7dd1e5` |
| `src/cmd/planar/handlers/observe/command.cppm` | modified | `c787a7dbb85a4fc38b0cdda90cc0ec1c363f88775e2bdbf1286ed2b06a1c2113` |
| `src/cmd/planar/handlers/orchestrator/command.cpp` | modified | `22335e37e48c7e3bdd618924181eef5f982ba9a07f0c97147c3e558d14ef1581` |
| `src/cmd/planar/handlers/orchestrator/command.cppm` | modified | `9db7075ff095a3fc4eaa968927c53a1ceb382ccd3accf4f12cbb70a686412a36` |
| `src/cmd/planar/handlers/spec-prompt/command.cpp` | modified | `c25bc6d7e7a77a252bde534c7c1778e24113389acaf3f1dd58a5d2fc1bb96dbf` |
| `src/cmd/planar/handlers/spec-prompt/command.cppm` | modified | `e73b33a95e13704446d26785086639503ab80310cdaab387d50e362bcb720c82` |
| `src/cmd/planar/invocation.cpp` | modified | `2c0d8de4e3120dda78d5ab62ec56da3fef6f13e773d2517da4e585aedb90c910` |
| `src/cmd/planar/invocation.cppm` | modified | `a2a02bdc22f406049c2c62bb49d916d95c2daf22c988fa1d02a5ec6c3b542b8b` |
| `src/engine/CMakeLists.txt` | modified | `d7364fb1cd3678f5a9f1a7f32bcb9fd708f0afc4b9e2afeef72fa15d3796e3c8` |
| `src/engine/capture/capture.cpp` | modified | `84d7f19367b6a4672e1488167b73b05045d367f121b581fa7db7e2f2eebc612f` |
| `src/engine/capture/capture.cppm` | modified | `6e0cf4750ed98446e25f0caf100441684c2674e705a667a732f695a81bda60c9` |
| `src/engine/capture/capture.t.cpp` | modified | `e365737883db6a2c9d22ff2f73454e8fdcc9c450e82614d5c167c3a586ac51a6` |
| `src/engine/runtime/external_registration.t.cpp` | modified | `48ded2a062d1eac66c4b59abc9b6c8f1baff367606569c0a9cabac845771f8b0` |
| `src/engine/runtime/runtime.cpp` | modified | `9c8c15cff3bc42eaddb4066f26f32e8b9464be2426fef3b57ebee5f94448a71d` |
| `src/engine/runtime/runtime.cppm` | modified | `610f9404b9dad5648b0059daa3b736e0296226daf60b3f9a49bef1d0f049bb94` |
| `src/stdlib/planar/commands.lua` | modified | `f32c5be4b8ae3de2a36ded7b51081813bfe3c63ca3d3cfa0e58e702274cf10b7` |
| `src/stdlib/planar/schema.lua` | modified | `e71fa6d969602e23ab9ee61c8838ccf3d7f8a0118ad27b35e1d0504927d86fe9` |
| `src/stdlib/planar/semantic.lua` | modified | `82bfe3dcad643d9eff7eb7d4c75108b40d2dcfd93dee69be711ba5e5f65f89aa` |
| `tests/native/path-redaction.json` | modified | `330e8586ff8aa7bf2ec7dcaa4fdfba8a6797dc33ccdc374d839cec5103849c98` |
| `src/cmd/planar/authoring_observe.t.cpp` | untracked | `3e01e63e7e26086a5aeeafba81251c067f9d2be9cb4bec79bcbb408a3e04f8c0` |
| `src/cmd/planar/authoring_orchestrator.t.cpp` | untracked | `0966d32b09332169be6ae006c33a07669c1e53573cc929bf3a8d4f72bfbc851d` |
| `src/cmd/planar/authoring_orchestrator_variants.t.cpp` | untracked | `740fff5e1ca80d624cf98ef52f965f647860493b25ef6e3669c39c703f88bd02` |
| `src/cmd/planar/authoring_prompts.t.cpp` | untracked | `e0fa761f480a4ac4c4471bf2195352a54a84826ecd051c92a6f291dd13d28231` |
| `src/engine/capture/directory.cpp` | untracked | `000c8cc3092fc5f6eaf414c25638e4d1eef7e96d3247c085fc34afe81bb8331c` |
| `src/engine/capture/directory.cppm` | untracked | `baa70fcca43c512227aed2b64e2c4a36231c9af411184fb57baaedc3263e90aa` |
| `src/engine/capture/directory.t.cpp` | untracked | `94789156423ee9e2e2d8ce0584b6429cd00ff7947650c60b0b298d8318591e67` |
| `src/engine/capture/directory_recheck.cppm` | untracked | `fc3a77c9194e237b3f440edbed88e7d5c07efab8d69a8db953939025111be81f` |
| `src/stdlib/planar/observe.lua` | untracked | `00c852affb77a240922f746087602aa5296707ef1e5adce04f382fb491e15ca8` |
| `src/stdlib/planar/orchestrator.lua` | untracked | `a7f5ebb4e3b0b531656bb7dca97536b2a61524f9f5dacc68cae25bf30d147238` |
| `src/stdlib/planar/prompts.lua` | untracked | `b1524300da9bced721e329a5ebd64f8a68f3f902285afa8a2e1a1399b2ac9bed` |

## Re-run results (14 mutants, all run; none skipped)

| id | diff sha256 (16) | status | failing test(s) | assessment | log |
|---|---|---|---|---|---|
| W2-23 | `b917323642862f4a` | survived | - | survived: equivalent. Keyed lists reach prompts.lua already sorted by id (schema.lua:38), so the sort in `ids` is redundant; same explanation as superseded-1. | `raw/W2-23.log` |
| W2-24 | `7f1650037400ba27` | killed | RA01-B2 captured text is escaped as data in prompts | killed | `raw/W2-24.log` |
| W2-31 | `4fede4e5ba1c65e1` | killed | RA01-B2 explain-rejection refuses malformed diagnostic JSON as schema | killed now (was a survivor): the new w2p_expect_decoder_refusal asserts message, details {offset,reason} and field exactly `diagnostic`. | `raw/W2-31.log` |
| W2-67 | `431c29c25b1585e9` | survived | - | survived: equivalent. Every valid job targets all work units, so no prescription is out of scope; the row stays a documented limit, as in superseded-1. | `raw/W2-67.log` |
| W2-68 | `fb92abacf930f0f4` | killed | RA01-B2 orchestrator prompt carries five sections in order | killed (test file changed by the repair, re-run to confirm) | `raw/W2-68.log` |
| X01 | `ae7f06b493b642cf` | survived | - | survived; probable redundant guard (sealed names are produced and sorted natively), not proven. Carried explanation unchanged. | `raw/X01.log` |
| X09 | `247b9fa59320f572` | killed | RA01-B2 captured text is escaped as data in prompts | killed now (was a survivor): the test now puts U+0086 in a check coverage string and asserts the `\u0086` escape. | `raw/X09.log` |
| X11 | `3fb4eeb617850601` | survived | - | survived; probable redundant guard (shipped Lua always sends the invocation job), not proven. Unchanged. | `raw/X11.log` |
| X13 | `60189e029987b561` | survived | - | equivalent: fanout_review forbids stages, so the loop has nothing to render. | `raw/X13.log` |
| X14 | `93759a272e1f8115` | survived | - | survived despite the new exact-heading assertion: equivalent, scope is a schema set that arrives sorted (schema.lua:39), same reason as X15. | `raw/X14.log` |
| X15 | `4dda7ed8f5d1619c` | survived | - | equivalent by schema normalisation (set, schema.lua:39). | `raw/X15.log` |
| X16 | `789e344ab7312d9f` | survived | - | survived; probable defence in depth (the later add_input follows links), not proven. Unchanged. | `raw/X16.log` |
| X19 | `52df9a4b201e72fc` | killed | RA01-B2 orchestrator prompt carries five sections in order | killed now (was a test gap): the orchestrator inputs.files test recomputes size and file_digest of the captured bytes. | `raw/X19.log` |
| X20 | `2820e319412c1d95` | killed | RA01-B2 explain-rejection refuses malformed diagnostic JSON as schema | killed now (was a survivor): same assertions as W2-31. | `raw/X20.log` |

Status counts: killed 6 (W2-24, W2-31, W2-68, X09, X19, X20, each by its named test), survived 8 (W2-23, W2-67, X01, X11, X13, X14, X15, X16), all with an explanation above. Previous survivors now killed: W2-31, X09, X19, X20. X14 and X15 survivors are equivalent by schema normalisation. The remaining not-proven survivors (X01, X11, X16) are redundant-guard or defence-in-depth mutants that no public behaviour distinguishes; they are reported as limits, not kills.

## Red-first additions from the repair

| test (changed assertion) | mutant that fails it in this run |
|---|---|
| RA01-B2 captured text is escaped as data in prompts (C1 branch) | X09, W2-24 |
| RA01-B2 explain-rejection refuses malformed diagnostic JSON as schema (decoder message, details) | W2-31, X20 |
| RA01-B2 orchestrator prompt carries five sections in order (inputs.files size and digest) | X19, W2-68 |
| RA01-B2 orchestrator lists only the prescriptions that drive the job (exact heading) | none: X14 equivalent (schema set arrives sorted); assertion is a guard only |

## Carried forward from superseded-1 (rows not listed above are unchanged: their mutated file and test case are byte-identical to the earlier run; raw logs in `superseded-1/raw/`)

## Invariant table (rows W2-01 to W2-82)

`expectRed` (new behaviour must fail against the wave 1 stubs) is not re-measured here: the wave 1 stubs are not part of the candidate; the red-first evidence of the coders is under `red/`. The column `result` is the probe of the row's own mutant on the candidate.

| row | invariant | mutant(s) run (diff sha256, first 16) | result | killing test | run / carried | note |
|---|---|---|---|---|---|---|
| W2-01 | Error precedence for the six verbs: usage, then selector, then explicit paths, then capture, then Lua; a path-stage refusal is never replace | W2-01 `b3d75ebc2e529b84` killed | killed | W2-01: RA01-B1 explicit path stage precedes the stub refusal | run |  |
| W2-02 | Stub refusals of the six verbs are gone for valid inputs: spec-prompt, critique, ingest, explain-rejection, observe, orchestrator over valid | W2-02 `09bac0c9da5ccc58` killed | killed | W2-02: RA01-B1 each authoring verb parses its signature through the bundle to the stub refusal | run | first batch; killed |
| W2-03 | Wave 1 invocation properties survive: authoring verbs write nothing, print no success document on failure, refusals are the closed envelope  | W2-03 `dc37e276b7f6eaea` killed | killed | W2-03: RA01-B1 stub invocations write nothing and print no success document | run |  |
| W2-04 | Production binary keeps no authoring path: the six verbs are usage in the shipped binary, default help bytes unchanged (I25) | W2-04 `bc7a767621ca6ad0` build-failed; W2-04b `f03cf525a1074ed0` killed | killed (W2-04b); W2-04 itself does not compile | W2-04b: RA01-B1 default invoke and production binary refuse all six authoring verbs as usage | run |  |
| W2-05 | Native service protocol per verb: spec-prompt, critique, ingest open the workspace then capture_authoring exactly once with the verb-bound i | W2-05 `ccec63d83f52e955` killed | killed | W2-05: RA01-B2 native refuses an authoring capture whose include does not match the verb | run |  |
| W2-06 | Opaque slots diagnostic and from exist per family and cross into Lua as userdata, never as host text (D13); the handler assertion refuses in | W2-06 `c704fe51d02da685` killed-by-other; W2-06n `40c758a77bef3b14` survived | named test cannot kill (W2-06n survives); whole-slot mutant killed by 37 other tests | W2-06 (other): RA01-B2 an oversize prompt is refused as prompt_bytes (+36) | run | finding 1 |
| W2-07 | prompt_bytes: markdown over 1048576 bytes is resource_limit, limit prompt_bytes, maximum 1048576, exit 3, message Capture limit exceeded, st | W2-07 `d75a5ba2b58dadee` killed | killed | W2-07: RA01-B2 an oversize prompt is refused as prompt_bytes | run |  |
| W2-08 | include workspace (spec-prompt): inputs.files lists workspace.lua, its referenced text files and the registry, and NOT the spec file; includ | W2-08 `5308e77769f4b487` killed | killed | W2-08: RA01-B2 spec-prompt captures the workspace without the spec while critique and ingest capture the spec | run |  |
| W2-09 | Jobs are never captured: a listed job file that does not exist does not fail spec-prompt, critique or ingest (all three succeed). Needs both | W2-09 `fbaba7edab02fa42` killed | killed | W2-09: RA01-B2 authoring capture succeeds when a listed job file is absent for all three prompts | run |  |
| W2-10 | A job file that is present is neither read nor listed: a syntactically invalid present job file does not change the prompt and does not appe | W2-10 `37f9e02a6edd71b1` killed | killed | W2-10: RA01-B2 a present job file is neither read nor listed | run |  |
| W2-11 | ingest and critique with an invalid spec fail with exactly the typed error load gives for the same bytes (code, field, file, message; exit 3 | W2-11 `4cb6e710c3eae71b` killed | killed | W2-11: RA01-B2 ingest and critique with an invalid spec fail with the typed error load gives | run |  |
| W2-12 | spec-prompt validates only workspace and registry (mode workspace; the spec envelope entry is optional because spec-prompt has no spec): an  | W2-12 `9742427ed497e7ef` killed | killed | W2-12: RA01-B2 spec-prompt validates the workspace and registry only | run |  |
| W2-13 | A job-file error that load would report first is not reported by authoring (jobs unread): fixture with an invalid listed job and a valid spe | W2-13 `d2da54de77dcc488` killed | killed | W2-13: RA01-B2 authoring reports no job file errors | run |  |
| W2-14 | Envelope fields: kind per verb (spec, critique, job, rejection, orchestrator), markdown ends with exactly one LF and has no CR, bytes equals | W2-14 `a7c3a106a341e4cf` killed | killed | W2-14: RA01-B2 prompt envelope fields digest bytes and kind per verb | run |  |
| W2-15 | inputs.files sorted by (root,path) in unsigned byte order, each entry exactly {root,path,kind,mode,size,digest} with no bytes_hex; registry  | W2-15 `3ac884080b761ee4` survived; W2-15b `600b75fbedd2c4fe` killed | sort drop equivalent; bytes_hex killed (W2-15b) | W2-15b: RA01-B2 prompt inputs list sorted file entries and registry header | run |  |
| W2-16 | A35 determinism: for the SAME files, the FULL prompt envelope of each of the five kinds is byte-equal across two fresh planar_authoring_stac | W2-16 `fea978250380d647` killed-by-other; W2-16n `4a63d6ea5f39cc7b` killed-by-other; W2-16ns `4a63d6ea5f39cc7b` killed; W2-16s `fea978250380d647` killed | killed (W2-16s, W2-16ns); first runs W2-16 and W2-16n superseded (stale spawned binary) | W2-16ns: RA01-B2 A35 five prompt kinds are byte-equal across two fresh processes and reordered declarations; W2-16s: RA01-B2 A35 five prompt kinds are byte-equal across two fresh processes and reordered declarations; W2-16 (other): RA01-B2 an oversize prompt is refused as prompt_bytes (+92); W2-16n (other): RA01-B2 explain-rejection validates the diagnostic envelope and code grammar (+45) | run | Lua `tostring({})` address and a native steady-clock suffix are both nondeterministic across processes; the A35 test kills both once the spawned `planar_authoring_stack` is rebuilt |
| W2-17 | Section order per kind: spec = authority, task, workspace, spec_schema, checks, shapes, critique_step; critique = authority, task, workspace | W2-17 `2922dc6f3da2f7c1` killed; W2-17b `242024cffb14717e` killed | killed | W2-17: RA01-B2 prompts carry their sections in contract order; W2-17b: RA01-B2 prompts carry their sections in contract order | run |  |
| W2-18 | Schema text is generated from the schema tables: golden lines for the spec, job and workspace record tables (field lines sorted by name, nes | W2-18 `082a4ece65bbec71` killed | killed | W2-18: RA01-B2 schema text lists spec job and workspace fields generated from the schema tables | run |  |
| W2-19 | Schema text format details: fields sorted by unsigned byte name, nested records and list-of-record fields indented two spaces per level, req | W2-19 `df6642cfaccda293` killed; W2-19b `bada121bcdb7213e` killed; W2-19c `8c371e539b4ba492` killed | killed | W2-19: RA01-B2 schema text format sorts fields and nests records; W2-19b: RA01-B2 schema text format sorts fields and nests records; W2-19c: RA01-B2 schema text format sorts fields and nests records | run |  |
| W2-20 | checks section: workspace checks allow-list intersected with registry check IDs, sorted by unsigned bytes; an allow-listed check missing fro | W2-20 `81aebc5f55b34416` killed; W2-20b `c2031dc68b998393` killed | killed | W2-20: RA01-B2 checks section is the sorted allow-list intersected with the registry; W2-20b: RA01-B2 checks section is the sorted allow-list intersected with the registry | run |  |
| W2-21 | shapes section lists pipeline then fanout_review (embedded shapes only, no external shapes) with the job rules of declarations.md | W2-21 `afde321be0f4532e` killed | killed | W2-21: RA01-B2 shape catalog lists pipeline then fanout_review | run |  |
| W2-22 | spec prompt last section critique_step ends with exactly: Run planar critique --workspace <workspace> --registry <registry> and resolve ever | W2-22 `2f58b3faea9c5649` killed | killed | W2-22: RA01-B2 spec prompt ends with the critique step line and embeds no host path | run |  |
| W2-23 | workspace and spec_summary facts sorted by ID: workspace ID, spec path, job list, registry id and revision; spec ID, goal, work units with c | W2-23 `b917323642862f4a` survived | survived: equivalent (keyed lists normalised sorted) | - | run |  |
| W2-24 | Captured text (goal, prescription text, coverage strings) is rendered as labelled data with control characters escaped (u00xx) and a fence l | W2-24 `7f1650037400ba27` killed | killed | W2-24: RA01-B2 captured text is escaped as data in prompts | run |  |
| W2-25 | critique rule prescription_unchecked fires for a prescription whose scope (work set or global) contains no unit with a check (own checks or  | W2-25 `18238bdf495c3c59` killed | killed | W2-25: RA01-B2 critique rule prescription_unchecked fires only without a nonempty check | run |  |
| W2-26 | critique rule blocking_question_unanswered fires for a question with blocks start or complete and no answer; none-blocking or answered quest | W2-26 `a3a98e0840d8c047` killed | killed | W2-26: RA01-B2 critique rule blocking_question_unanswered covers start and complete | run |  |
| W2-27 | critique rule facet_dependency_without_output_vector fires for a dependency with source facet and phase complete whose consumer has no check | W2-27 `c499836035dcf8fb` killed | killed | W2-27: RA01-B2 critique rule facet_dependency_without_output_vector needs an output_vector check | run |  |
| W2-28 | Findings lines are exactly - RULE SUBJECT: TEXT, sorted by (rule, subject) unsigned bytes, all three rules together; none renders the fixed  | W2-28 `02744a26ef90a5ca` killed; W2-28b `e9d55fc8abb8b8c3` killed | killed | W2-28: RA01-B2 critique findings are sorted advisory and exit zero; W2-28b: RA01-B2 critique findings are sorted advisory and exit zero | run |  |
| W2-29 | No mechanical findings.: a spec triggering none prints exactly that fixed line in the findings section | W2-29 `1946031629651fc7` killed | killed | W2-29: RA01-B2 critique prints the fixed line when no rule fires | run |  |
| W2-30 | job prompt: constructors section lists exactly the sorted names - file, - immaterial, - job, - list, - md (D-20; loader.cpp:305, declaration | W2-30 `3d3e111eef2e58c8` killed; W2-30b `c8481f0825187839` killed | killed | W2-30: RA01-B2 job prompt lists sorted constructors and the job tier statement; W2-30b: RA01-B2 job prompt lists sorted constructors and the job tier statement | run |  |
| W2-31 | Diagnostic parser level: duplicate keys, invalid UTF-8, NUL and trailing non-whitespace are schema exit 3 with file absent and field startin | W2-31 `4fede4e5ba1c65e1` survived | survived: finding 3 | - | run |  |
| W2-32 | Diagnostic envelope level: exactly {version:1,kind:"error",error:E}; E has string code and message, string field (may be empty), optional st | W2-32 `c1861f7fcde129ff` killed; W2-32b `5ed871b07f9689e8` killed | killed | W2-32: RA01-B2 explain-rejection validates the diagnostic envelope and code grammar; W2-32b: RA01-B2 explain-rejection validates the diagnostic envelope and code grammar | run |  |
| W2-33 | read_diagnostic size bound: exactly 65536 bytes accepted, 65537 refused as resource_limit, limit diagnostic_bytes, maximum 65536, checked fr | W2-33 `f80f4bd1d52e2047` survived; W2-33c `a2f53536e7093029` killed | killed only by the double mutant W2-33c; single off-by-one survives (redundant bound) | W2-33c: RA01-B2 explain-rejection accepts 65536 bytes and refuses 65537 as diagnostic_bytes | run |  |
| W2-34 | GUARD of the existing path stage: prepare_inputs runs host_file (host_path.cpp:10-14) which refuses a symlink leaf (path_denied Explicit inp | W2-34 `81b45307ab7fe874` killed | killed | W2-34: RA01-B2 read_diagnostic refuses links and special files with the existing messages | run |  |
| W2-35 | Rejection inputs form: inputs is exactly {files:[],diagnostic:{bytes:size,digest:H(file_bytes_v1, file bytes)}} and has no registry key; dig | W2-35 `dc615aaec4966843` killed | killed | W2-35: RA01-B2 rejection inputs carry the diagnostic size and file digest | run |  |
| W2-36 | Excerpt for job.roles[1].work resolves to the work field line of the role record with its subtree (index suffix dropped); a path of only the | W2-36 `9fb0e32d211d534b` killed | killed | W2-36: RA01-B2 rejection excerpt resolves job.roles[1].work to the work field | run |  |
| W2-37 | Unresolvable paths give exactly No schema excerpt applies to this field.: first segment registry, observations, bundle, $, empty or unknown; | W2-37 `ff79dd26775d1744` killed | killed | W2-37: RA01-B2 rejection excerpt for an unresolvable path is the fixed line | run |  |
| W2-38 | raw_error block is verbatim: bytes between the LF ending the opening fence line and the closing fence line equal the file bytes, DEL and C1  | W2-38 `dde07872891dc19f` killed | killed | W2-38: RA01-B2 raw_error block equals the diagnostic bytes with DEL and C1 raw | run |  |
| W2-39 | raw_error fence is max(3, L+1) backticks where L is the longest backtick run in the file bytes (runs of 2, 3 and 5 tested); one LF is added  | W2-39 `cf2ad367ce16ab01` killed; W2-39b `fb78751a51cbfa88` killed | killed | W2-39: RA01-B2 raw_error fence outgrows backtick runs and adds one LF only when missing; W2-39b: RA01-B2 raw_error fence outgrows backtick runs and adds one LF only when missing | run |  |
| W2-40 | A36: a rejected load in a scratch ledger; its stderr as the diagnostic file; the prompt names the exact field path and its raw_error block b | W2-40 `ab3b3a0dbcf5478b` killed | killed | W2-40: RA01-B2 A36 rejected load stderr round-trips through explain-rejection | run |  |
| W2-41 | explain-rejection is pure: no workspace, registry, ledger or environment read, no file created, an invalid PLANAR_WORKSPACE_DIR is ignored ( | W2-41 `af07a5017989d8fe` killed-by-other; W2-41r `af07a5017989d8fe` killed | killed (W2-41r); the first run W2-41 carried a wrong test name | W2-41r: RA01-B1 explain-rejection and observe ignore an invalid workspace environment; W2-41 (other): RA01-B1 default invoke and production binary refuse all six authoring verbs as usage (+27) | run | first run named the test with a stray prefix and ran no test; the broad run failed 28 other tests |
| W2-42 | Observe entry kinds: a symlink under checks, findings or as checkpoint.json is path_denied Links and special files forbidden; a FIFO entry i | W2-42 `dbcf47402d21032e` killed | killed | W2-42: RA01-B2 observe refuses a symlink entry | run |  |
| W2-43 | Observe FIFO entry is path_denied Links and special files forbidden without blocking (O_NONBLOCK) and without reading | W2-43 `15e48210369198ee` killed | killed | W2-43: RA01-B2 observe refuses a FIFO entry | run |  |
| W2-44 | Hard-link alias: two listed entries with the same device and inode (checks/a.json and findings/b.json) are path_denied Hard-link input alias | W2-44 `7d7808e88b2f987c` killed | killed | W2-44: RA01-B2 observe refuses a hard-link alias | run |  |
| W2-45 | Layout violations are path_denied Invalid captured path with no file field: extra top-level file, subdirectory other than checks and finding | W2-45 `9e95b4bb40470a63` build-failed; W2-45r `7fe04a36235e9b5d` killed | macOS part killed (W2-45r); Linux invalid-UTF-8 branch killed in the container by the named test (W2-45L) | W2-45r: RA01-B2 observe refuses layout violations without echoing names | run |  |
| W2-46 | Dot names are ignored (not opened, not inspected: a dot symlink and a dot FIFO cause no error) but count toward the 1024 bound | W2-46 `cb681f0488fc923e` killed; W2-46b `4e137b161ca2cf7f` killed-by-other | killed | W2-46: RA01-B2 observe ignores dot names; W2-46b (other): RA01-B2 observe bounds listed names at 1024 counting dot names | run | W2-46b (dot names not counted) killed by the bound test, which the named test's comment assigns it to |
| W2-47 | Entry bound: 1024 listed names in total across root, checks and findings (dot names and subdirectories counted) accepted, 1025 refused as re | W2-47 `1b5c063a8cabae04` killed; W2-47b `20689402b6683021` killed | killed | W2-47: RA01-B2 observe bounds listed names at 1024 counting dot names; W2-47b: RA01-B2 observe bounds listed names at 1024 counting dot names | run |  |
| W2-48 | File bound: 65536 bytes accepted, 65537 refused as observation_file_bytes maximum 65536, decided from the descriptor size before allocation | W2-48 `18badd1bc4837de0` killed | killed | W2-48: RA01-B2 observe refuses an oversize file as observation_file_bytes | run |  |
| W2-49 | Total bound: 4194304 bytes read in total accepted, one byte over refused as observation_total_bytes maximum 4194304 (64 files of 65536 accep | W2-49 `a5e7f1423886fe04` build-failed; W2-49r `e75a01717c6a9353` killed | killed (W2-49r); first build failed | W2-49r: RA01-B2 observe refuses more than four MiB as observation_total_bytes | run |  |
| W2-50 | An unsafe entry takes precedence over a later size limit: a symlink sorted before an oversize file reports path_denied | W2-50 `4c851a22ad1c83b0` killed | killed | W2-50: RA01-B2 observe reports an unsafe entry before a later size limit | run |  |
| W2-51 | Listing is sorted by unsigned name bytes before use and independent of readdir order: duplicate-ID errors blame the later file in name order | W2-51 `8a9de6f6c4a1911e` killed-by-other; W2-51b `8a9de6f6c4a1911e` killed-by-other | killed-by-other on macOS and in the Linux container (symlink-entry test); the named sorted-listing test survives on Linux too (finding 7) | W2-51 (other): RA01-B2 observe refuses a symlink entry; W2-51b (other): RA01-B2 observe refuses a symlink entry | run | the removal of the listing sort is killed only by the symlink-entry test (order of the first unsafe entry), on macOS and in the container; the named sorted-listing test does not fail on either |
| W2-52 | Recheck, step 4: a file replaced (different bytes: File changed during read; new inode: Directory membership or identity changed), added or  | W2-52 `d7b49edb5fee032d` killed | killed | W2-52: RA01-B2 observe refuses a directory changed before the recheck | run |  |
| W2-53 | Output observation_bytes: canonical observations over 1048576 bytes (about 260 checks with 4096-byte summaries) is resource_limit observatio | W2-53 `d1269db970c09d81` killed | killed | W2-53: RA01-B2 observe output over one MiB is refused as observation_bytes | run |  |
| W2-54 | Duplicate IDs: a duplicate check id among checks (and finding id among findings) is reference Duplicate value with file the later file in na | W2-54 `261408eca7884d4c` killed | killed | W2-54: RA01-B2 observe refuses duplicate ids within checks and within findings | run |  |
| W2-55 | Structural errors: unknown fields, nulls, non-integers, duplicate keys, invalid UTF-8 and NUL are schema with the shared normalizer or parse | W2-55 `332f0e0b502d3780` killed | killed | W2-55: RA01-B2 observe errors carry the logical file and object-relative field | run |  |
| W2-56 | Structural checks only: a malformed digest is schema Malformed output digest (field target.output or vector[N].output); a well-formed digest | W2-56 `df17a8c0986344b0` killed | killed | W2-56: RA01-B2 observe applies structural checks only | run |  |
| W2-57 | Checkpoint bounds: more than 32 files is resource_limit {checkpoint_files,32} Too many checkpoint paths; canonical checkpoint over 8192 byte | W2-57 `3a1a83ff582f3aae` killed | killed | W2-57: RA01-B2 observe enforces the checkpoint bounds | run |  |
| W2-58 | Validation order: files visited in unsigned name order, checks then findings then checkpoint, first error wins | W2-58 `1453490aa644cfc7` killed | killed | W2-58: RA01-B2 observe reports the first error in checks, findings, checkpoint order | run |  |
| W2-59 | Output is the bare canonical observations value plus LF: {version,checks,findings} sorted by id (not by file name), checkpoint key only when | W2-59 `c05ee5268a026765` killed | killed | W2-59: RA01-B2 observe prints sorted bare observations | run |  |
| W2-60 | Empty directory prints exactly {"version":1,"checks":[],"findings":[]} and LF; a missing checks or findings directory is empty | W2-60 `b955774e75a44f60` killed | killed | W2-60: RA01-B2 observe empty directory prints canonical empty observations | run |  |
| W2-61 | Output is independent of creation order: the same files created in two different orders in two directories print byte-equal stdout; the dige | W2-61 `dc5a1bb874ec7eb1` killed | killed | W2-61: RA01-B2 observe output is independent of creation order | run |  |
| W2-62 | Observe output is structurally accepted by render --observations: feeding it to render gives observation_target_mismatch (not schema) for a  | W2-62 `dda13889dd1a48d6` killed | killed | W2-62: RA01-B2 observe output is structurally accepted by render observations | run |  |
| W2-63 | Observe path stage keeps existing messages: missing --from io Cannot resolve explicit root directory, non-directory io Explicit root must be | W2-63 `f7c8c1596ca84582` killed | killed | W2-63: RA01-B1 explicit path stage keeps the existing messages | run |  |
| W2-64 | Orchestrator capture: a job ID the workspace does not list (including an ID that is not a valid declaration ID) is reference exit 3 field se | W2-64 `2b7186d5003f793a` killed | killed | W2-64: RA01-B2 orchestrator refuses an unlisted job as reference selection.job | run |  |
| W2-65 | Orchestrator reads only the selected job (mode selected_job, D-15): other listed job files absent or malformed are ignored and produce no Li | W2-65 `87eeb0f9e1a2c146` killed | killed | W2-65: RA01-B2 orchestrator reads only the selected job | run |  |
| W2-66 | Orchestrator context: shape line, stages (pipeline stage lists in order, fanout_review none), roles sorted by id with sorted work or targets | W2-66 `587bacbd02072fc3` killed; W2-66b `19f39d1083e83a4b` survived | killed (W2-66); W2-66b equivalent (keyed roles sorted) | W2-66: RA01-B2 orchestrator sections render shape roles and graph | run |  |
| W2-67 | Prescriptions drive the job: scope global or intersecting the job work units, sorted by id, each id scope and escaped text | W2-67 `431c29c25b1585e9` survived | survived: equivalent (documented limit of the test) | - | run |  |
| W2-68 | Five mandatory sections in order orchestrator_authority, graph, roles, prescriptions, actions under one # title; kind orchestrator in the en | W2-68 `fb92abacf930f0f4` killed | killed | W2-68: RA01-B2 orchestrator prompt carries five sections in order | run |  |
| W2-69 | Action labels: the union of the sealed catalog names (eleven in the stack) and the twelve future names activate, status, preview, claim, tak | W2-69 `59c13d30de4e7505` killed; W2-69b `35ee40c8cd016d47` killed | killed | W2-69: RA01-B2 orchestrator labels every catalog verb available and every future verb future; W2-69b: RA01-B2 orchestrator labels every catalog verb available and every future verb future | run |  |
| W2-70 | Label flip: in variant 20 (sealed catalog also contains status; D-25) status is labelled available and the other eleven future labels are un | W2-70 `9e755ecbb43ec265` killed | killed | W2-70: RA01-B2 orchestrator label flips to available for a catalog stub verb | run | killed through the catalog variant 20 |
| W2-71 | Orchestrator is graph-only: orchestrator.lua builds exactly the RA-7 orchestrator_context_v1 keys and the projection function receives only  | W2-71 `d8f472b95113eb0c` killed-by-other | named test survives; killed by 'orchestrator sections render shape roles and graph' | W2-71 (other): RA01-B2 orchestrator sections render shape roles and graph | run |  |
| W2-72 | orchestrator requires --workspace --registry --job together and writes nothing; an explicit-path failure precedes capture (existing usage an | W2-72 `275b2a3220136597` killed-by-other | named test survives; killed by six other RA01-B1 tests | W2-72 (other): RA01-B1 selector stage precedes the explicit path stage (+5) | run |  |
| W2-73 | orchestrator_context_v1 is not an admissible external projection input: an external projection registered with that input is refused by regi | W2-73 `f354c42388f00f63` killed | killed | W2-73: RA01-B2 orchestrator_context_v1 is refused as an external projection input | run |  |
| W2-74 | c02_off_boundary is re-measured after the round's LAST edit of production-embedded Lua or runtime.cpp (waves 1 and 3: commands.lua, schema.l | W2-74 `0bdd7545ce8faf65` killed | killed | W2-74: X08 C01 role preparation outputs-off boundary pins the request-record conversion charge | run |  |
| W2-76 | Mode guard (D-15): render and load behave exactly as before; a workspace listing a job whose file is absent is still refused reference Liste | W2-76 `e7cb9523b303ba92` killed | killed | W2-76: RA01-B2 render and load keep Listed job not captured outside the authoring modes | run |  |
| W2-77 | The capture_orchestrator result Lua sees has exactly five keys {bundle,capture,commands,files,job}: the provider returns exactly four, nativ | W2-77 `2dc41bee582b1667` killed; W2-77c `21d786771c3aaa65` survived; W2-77d `34c3c0959255757f` killed | killed (W2-77, W2-77d); W2-77c equivalent (entries sorted at seal) | W2-77: RA01-B2 native appends the sorted sealed command names to the orchestrator capture; W2-77d: RA01-B2 native appends the sorted sealed command names to the orchestrator capture | run |  |
| W2-78 | capture_diagnostic (D-40) on a capture_session: a file replaced between registration and verification (session.before_verification callback) | W2-78 `4a80e3025dd49649` killed | killed | W2-78: RA01-B2 capture_diagnostic refuses a file changed before verification | run |  |
| W2-79 | capture_diagnostic size gate: exactly 65536 bytes accepted, 65537 refused resource_limit limit diagnostic_bytes maximum 65536 decided from t | W2-79 `f80f4bd1d52e2047` survived; W2-79b `783ea0483d198e08` survived; W2-79c `a2f53536e7093029` killed | killed only by the double mutant W2-79c; single off-by-one survives (redundant bound) | W2-79c: RA01-B2 capture_diagnostic bounds the file at 65536 bytes from the descriptor | run |  |
| W2-80 | Selected-job mode keeps the identity checks: a listed job whose returned id differs from the workspace listing is refused reference Returned | W2-80 `20230dd93e3a1af3` killed | killed | W2-80: RA01-B2 orchestrator selected-job mode still checks the listed job identity | run |  |
| W2-81 | Variant 21 anchor preserved (process row): prompts.lua contains the exact line local prompts_spec_prompt_include="workspace" once and the sp | W2-81 `d716238e8144aab0` killed | killed (cmake configure fails: authoring variant anchor is absent) | W2-81:  | run | process mutant: a configure failure, no TEST_CASE |
| W2-82 | Linux probes (D-41): the W2-51 no-sort mutant and the W2-45 Linux-branch mutant are applied by the prober in a scratch copy inside the Linux | - | container run done (linux/arm64, Debug): baseline 21/21 passed; W2-51 and W2-45L results above; raw/linux/ | - | run |  |
| W2-75 | Existing tests unchanged in name: all 1344 earlier test names present in Debug and RelWithDebInfo inventories, none removed or renamed; new  | - | process check done: 0 removed, 72 added (raw/W2-75.log) | - | run |  |

## Extra exploratory mutants (limit 20; 20 run, none skipped)

| id | invariant attacked | mutant | diff sha256 (16) | result | failing test(s) | assessment |
|---|---|---|---|---|---|---|
| X01 | X-ORCH | orchestrator names_valid accepts any commands value | `ae7f06b493b642cf` | survived | - | survived; probable redundant guard (the sealed names are produced and sorted natively), not proven |
| X02 | X-OBS | observe digest_valid always true | `e35bfb2426580c7b` | killed | RA01-B2 observe applies structural checks only | killed |
| X03 | X-OBS | checkpoint canonical size bound 8193 | `48e765201a8b7d2c` | survived | - | test gap: no boundary assertion at 8192/8193 canonical bytes |
| X04 | X-PRM | prompt ceiling uses >= (a prompt of exactly 1048576 bytes refused) | `959eef56950b1693` | survived | - | test gap: a prompt of exactly 1048576 bytes is not asserted |
| X05 | X-CRIT | critique global prescription counts any check, not only nonempty | `f2fe51604fe2aabe` | survived | - | test gap: global scope with a non-nonempty check is not asserted |
| X06 | X-CRIT | critique ignores contract consumers when collecting unit checks | `123ab5ccf43b4794` | killed-by-other | RA01-B2 critique rule prescription_unchecked fires only without a nonempty check | killed by the prescription_unchecked test, not the facet test I guessed (contract consumers feed both rules) |
| X07 | X-RJ | excerpt strip_index accepts non-digit index text | `b90c30bb8fdbbca1` | survived | - | test gap: a non-digit index inside [ ] in a field path is not asserted |
| X08 | X-RJ | diagnostic details non-object check dropped | `0b1f16ade6bc5959` | killed | RA01-B2 explain-rejection validates the diagnostic envelope and code grammar | killed |
| X09 | X-ESC | escape ignores the C1 control branch | `247b9fa59320f572` | survived | - | survived, not proven equivalent: no fixture carries a C1 control inside a check coverage string |
| X10 | X-DIR | scan_root inspects dot names | `de0a84cd00cc3dc7` | killed | RA01-B2 observe ignores dot names | killed |
| X11 | X-RT | native capture_orchestrator accepts a job text different from the invocation | `3fb4eeb617850601` | survived | - | survived; probable redundant guard (the shipped Lua always sends the invocation job), not proven |
| X12 | X-PRM | workspace facts list jobs unsorted | `4173cb7200fdc441` | survived | - | equivalent by schema normalisation (keyed list sorted by id, schema.lua:38) |
| X13 | X-ORCH | orchestrator renders stages for any shape | `60189e029987b561` | survived | - | equivalent: fanout_review forbids stages, so the loop has nothing to render |
| X14 | X-ORCH | orchestrator prescription scope listed unsorted | `93759a272e1f8115` | survived | - | survived, not proven equivalent |
| X15 | X-ORCH | orchestrator worker work list unsorted | `4dda7ed8f5d1619c` | survived | - | equivalent by schema normalisation (set, schema.lua:39) |
| X16 | X-CAP | capture_diagnostic gate descriptor opened without no-follow | `789e344ab7312d9f` | survived | - | survived; probable defence in depth (the later add_input follows), not proven |
| X17 | X-DIR | observation root opened without no-follow | `a976fbdbd45d77c0` | survived | - | survived; the CLI prepare stage resolves the directory first and no engine-level root-symlink test exists, not proven equivalent |
| X18 | X-PRM | prompt inputs registry header carries the id only | `908d21ed3c2da61b` | killed | RA01-B2 prompt inputs list sorted file entries and registry header | killed |
| X19 | X-ORCH | orchestrator markdown lacks the file inputs digest (digest of file bytes via fragment digest) | `52df9a4b201e72fc` | survived | - | test gap: orchestrator inputs.files digests are not asserted |
| X20 | X-RJ | diagnostic JSON refusal details dropped when mapping the decoder error | `2820e319412c1d95` | survived | - | finding 3 |

## Red-first table

### Tests added in this round (72 new names, `new-names.txt`)

Every new test with the mutant of this run that makes it fail; the first column of each cell is the mutant id (rows of the table above).

| new test | mutants that fail it in this run | count |
|---|---|---|
| RA01-B2 A35 five prompt kinds are byte-equal across two fresh processes and reordered declarations | W2-16ns, W2-16s | 2 |
| RA01-B2 A36 rejected load stderr round-trips through explain-rejection | W2-16, W2-16n, W2-40, W2-41 | 4 |
| RA01-B2 a present job file is neither read nor listed | W2-06, W2-10, W2-16, W2-16n | 4 |
| RA01-B2 an oversize prompt is refused as prompt_bytes | W2-06, W2-07, W2-16, W2-16n | 4 |
| RA01-B2 authoring capture succeeds when a listed job file is absent for all three prompts | W2-06, W2-09, W2-16, W2-16n | 4 |
| RA01-B2 authoring reports no job file errors | W2-06, W2-13, W2-16 | 3 |
| RA01-B2 capture_diagnostic bounds the file at 65536 bytes from the descriptor | W2-79c | 1 |
| RA01-B2 capture_diagnostic refuses a file changed before verification | W2-78 | 1 |
| RA01-B2 captured text is escaped as data in prompts | W2-06, W2-16, W2-16n, W2-24 | 4 |
| RA01-B2 checks section is the sorted allow-list intersected with the registry | W2-06, W2-16, W2-16n, W2-20, W2-20b | 5 |
| RA01-B2 critique findings are sorted advisory and exit zero | W2-06, W2-16, W2-16n, W2-28, W2-28b | 5 |
| RA01-B2 critique prints the fixed line when no rule fires | W2-06, W2-16, W2-16n, W2-29 | 4 |
| RA01-B2 critique rule blocking_question_unanswered covers start and complete | W2-06, W2-16, W2-16n, W2-26 | 4 |
| RA01-B2 critique rule facet_dependency_without_output_vector needs an output_vector check | W2-06, W2-16, W2-16n, W2-27 | 4 |
| RA01-B2 critique rule prescription_unchecked fires only without a nonempty check | W2-06, W2-16, W2-16n, W2-25, X06 | 5 |
| RA01-B2 diagnostic and from reach Lua only as opaque slots | W2-16, W2-16n, W2-41 | 3 |
| RA01-B2 explain-rejection accepts 65536 bytes and refuses 65537 as diagnostic_bytes | W2-16, W2-16n, W2-33c, W2-41 | 4 |
| RA01-B2 explain-rejection refuses malformed diagnostic JSON as schema | W2-16, W2-16n, W2-41 | 3 |
| RA01-B2 explain-rejection validates the diagnostic envelope and code grammar | W2-16, W2-16n, W2-32, W2-32b, W2-41, X08 | 6 |
| RA01-B2 ingest and critique with an invalid spec fail with the typed error load gives | W2-06, W2-11, W2-16 | 3 |
| RA01-B2 job prompt lists sorted constructors and the job tier statement | W2-06, W2-16, W2-16n, W2-30, W2-30b | 5 |
| RA01-B2 native appends the sorted sealed command names to the orchestrator capture | W2-06, W2-16, W2-77, W2-77d | 4 |
| RA01-B2 native refuses an authoring capture whose include does not match the verb | W2-05, W2-06, W2-16 | 3 |
| RA01-B2 observe applies structural checks only | W2-16, W2-56, X02 | 3 |
| RA01-B2 observe bounds listed names at 1024 counting dot names | W2-46b, W2-47, W2-47b | 3 |
| RA01-B2 observe empty directory prints canonical empty observations | W2-16, W2-16n, W2-60 | 3 |
| RA01-B2 observe enforces the checkpoint bounds | W2-16, W2-57 | 2 |
| RA01-B2 observe errors carry the logical file and object-relative field | W2-16, W2-55 | 2 |
| RA01-B2 observe ignores dot names | W2-16, W2-16n, W2-46, X10 | 4 |
| RA01-B2 observe lists entries sorted by category and name | none in this run or in the container (finding 7) | 0 |
| RA01-B2 observe output is independent of creation order | W2-16, W2-16n, W2-61 | 3 |
| RA01-B2 observe output is structurally accepted by render observations | W2-16, W2-16n, W2-62 | 3 |
| RA01-B2 observe output over one MiB is refused as observation_bytes | W2-16, W2-53 | 2 |
| RA01-B2 observe prints sorted bare observations | W2-16, W2-16n, W2-59 | 3 |
| RA01-B2 observe refuses a FIFO entry | W2-16, W2-43 | 2 |
| RA01-B2 observe refuses a directory changed before the recheck | W2-52 | 1 |
| RA01-B2 observe refuses a hard-link alias | W2-16, W2-44 | 2 |
| RA01-B2 observe refuses a symlink entry | W2-16, W2-42, W2-51, W2-51b | 4 |
| RA01-B2 observe refuses an oversize file as observation_file_bytes | W2-48 | 1 |
| RA01-B2 observe refuses duplicate ids within checks and within findings | W2-16, W2-16n, W2-54 | 3 |
| RA01-B2 observe refuses layout violations without echoing names | W2-16, W2-45r | 2 |
| RA01-B2 observe refuses more than four MiB as observation_total_bytes | W2-49r | 1 |
| RA01-B2 observe reports an unsafe entry before a later size limit | W2-50 | 1 |
| RA01-B2 observe reports the first error in checks, findings, checkpoint order | W2-16, W2-58 | 2 |
| RA01-B2 orchestrator label flips to available for a catalog stub verb | W2-06, W2-16, W2-70 | 3 |
| RA01-B2 orchestrator labels every catalog verb available and every future verb future | W2-06, W2-16, W2-16n, W2-69, W2-69b | 5 |
| RA01-B2 orchestrator lists only the prescriptions that drive the job | W2-06, W2-16, W2-16n | 3 |
| RA01-B2 orchestrator prompt carries five sections in order | W2-06, W2-16, W2-16n, W2-68 | 4 |
| RA01-B2 orchestrator prompt is graph only | W2-06, W2-16, W2-16n | 3 |
| RA01-B2 orchestrator reads only the selected job | W2-06, W2-16, W2-16n, W2-65 | 4 |
| RA01-B2 orchestrator refuses an unlisted job as reference selection.job | W2-06, W2-16, W2-64 | 3 |
| RA01-B2 orchestrator sections render shape roles and graph | W2-06, W2-16, W2-16n, W2-66, W2-71 | 5 |
| RA01-B2 orchestrator selected-job mode refuses an unlisted job declaration | W2-06, W2-16 | 2 |
| RA01-B2 orchestrator selected-job mode still checks the listed job identity | W2-06, W2-16, W2-16n, W2-80 | 4 |
| RA01-B2 orchestrator_context_v1 is refused as an external projection input | W2-73 | 1 |
| RA01-B2 prompt envelope fields digest bytes and kind per verb | W2-06, W2-14, W2-16, W2-16n, W2-41 | 5 |
| RA01-B2 prompt inputs list sorted file entries and registry header | W2-06, W2-15b, W2-16, W2-16n, X18 | 5 |
| RA01-B2 prompts carry their sections in contract order | W2-06, W2-16, W2-16n, W2-17, W2-17b, W2-41 | 6 |
| RA01-B2 raw_error block equals the diagnostic bytes with DEL and C1 raw | W2-16, W2-16n, W2-38, W2-41 | 4 |
| RA01-B2 raw_error fence outgrows backtick runs and adds one LF only when missing | W2-16, W2-16n, W2-39, W2-39b, W2-41 | 5 |
| RA01-B2 read_diagnostic refuses links and special files with the existing messages | W2-16, W2-34, W2-41 | 3 |
| RA01-B2 rejection excerpt for an unresolvable path is the fixed line | W2-16, W2-16n, W2-37, W2-41 | 4 |
| RA01-B2 rejection excerpt resolves job.roles[1].work to the work field | W2-06, W2-16, W2-16n, W2-36, W2-41 | 5 |
| RA01-B2 rejection inputs carry the diagnostic size and file digest | W2-16, W2-16n, W2-35, W2-41 | 4 |
| RA01-B2 render and load keep Listed job not captured outside the authoring modes | W2-16, W2-76 | 2 |
| RA01-B2 schema text format sorts fields and nests records | W2-06, W2-16, W2-16n, W2-19, W2-19b, W2-19c | 6 |
| RA01-B2 schema text lists spec job and workspace fields generated from the schema tables | W2-06, W2-16, W2-16n, W2-18, W2-41 | 5 |
| RA01-B2 shape catalog lists pipeline then fanout_review | W2-06, W2-16, W2-16n, W2-21 | 4 |
| RA01-B2 spec prompt ends with the critique step line and embeds no host path | W2-06, W2-16, W2-16n, W2-22, W2-41 | 5 |
| RA01-B2 spec summary lists work checks and paths sorted by id | W2-06, W2-16, W2-16n | 3 |
| RA01-B2 spec-prompt captures the workspace without the spec while critique and ingest capture the spec | W2-06, W2-08, W2-16, W2-16n | 4 |
| RA01-B2 spec-prompt validates the workspace and registry only | W2-06, W2-12, W2-16, W2-16n | 4 |

### Carried forward: tests of the wave 1 lineage (RA01-B1 names kept by this round)

These 71 names were added in wave 1 and its corrective; this round keeps them (some assertions rewritten from the stub refusal to the real outcome, D-33). Their red-first mutants are the wave 1 / C01 probe results (`tests/native/v5-02-ra01-b-w1-c01-evidence/probes/results.jsonl` and its `superseded-1/results.jsonl`, same test names). Table: first three wave 1 mutant ids that fail each test, plus the mutants of this run that fail it where any.

| carried test | wave 1 mutants (first 3) | this run (first 3) |
|---|---|---|
| RA01-B1 a broken profile reports its own error before a reserved name collision | I54r | - |
| RA01-B1 authoring bundle runs shipped verbs like the default entry | C-I06, E06, E12 | W2-16 |
| RA01-B1 authoring capabilities are the exact per row sets | E12, I02, I05 | W2-16 |
| RA01-B1 authoring catalog keeps the five shipped descriptors identical | E12, I01, I02 | W2-16 |
| RA01-B1 authoring catalog seals the eleven RA-1 rows in sorted order | E12, I02, I10 | W2-16 |
| RA01-B1 authoring families refuse empty and dash prefixed values | I30a, I30d, I30f | - |
| RA01-B1 authoring families refuse positionals unknown options and help | I30a, I30e, I30f | - |
| RA01-B1 authoring families refuse repeated options | I30a, I30c, I30f | - |
| RA01-B1 authoring families require every option | I30a, I30b, I30f | - |
| RA01-B1 authoring handler ids and response kinds use underscores | E12, I02, I03 | W2-16 |
| RA01-B1 authoring option signatures are exactly the RA-1 table | C-I06, E12, I02 | W2-16 |
| RA01-B1 authoring root help inserts the six RA-1 lines before the final line | E01, E02, E16 | - |
| RA01-B1 authoring verbs do not accept each others options | C-I06, E06, E19 | - |
| RA01-B1 authoring verbs refuse missing required options as usage | C-I06, E05, E06 | W2-41 |
| RA01-B1 authoring verbs refuse positionals unknown options and equals syntax as usage | C-I06, E05, E06 | W2-41 |
| RA01-B1 authoring verbs refuse repeats empty and dash prefixed values as usage | C-I06, E05, E06 | W2-41 |
| RA01-B1 command options variant stays closed at five members | I08 | - |
| RA01-B1 commands.lua keeps one handler function and one registration line per authoring verb | I02, I09a, I09b | - |
| RA01-B1 control bundle seals and a refused bundle maps to internal exit 4 | E12, I10, I12a | W2-16 |
| RA01-B1 default catalog refuses authoring rows registered without the request | E12, I01, I02 | W2-16 |
| RA01-B1 default embedded catalog stays the five HEAD rows | I01, I09c | - |
| RA01-B1 default invoke and production binary refuse all six authoring verbs as usage | E05, E06, E16 | W2-04b, W2-41 |
| RA01-B1 defined runtime keeps profile VMs at five rows | E12, I09c, I10 | W2-16 |
| RA01-B1 each authoring family binds exactly its RA-1 option names | I30a, I30f, I37 | - |
| RA01-B1 each authoring verb parses its signature through the bundle to the stub refusal | C-I06, E05, E06 | W2-02, W2-06, W2-16 |
| RA01-B1 explain-rejection and observe ignore an invalid workspace environment | C-I06, E04, E06 | W2-16, W2-16n, W2-41 |
| RA01-B1 explicit path stage keeps the existing messages | C-R3-DIAG, C-R3-FROM, C-R3-NARROW | W2-16, W2-63, W2-72 |
| RA01-B1 explicit path stage precedes the stub refusal | C-I06, C-R3-DIAG, C-R3-FROM | W2-01, W2-16, W2-41 |
| RA01-B1 help splice inserts six lines before the final line only | E01, E02, I27 | - |
| RA01-B1 missing leaf remap applies only to diagnostic and from | C-I06, C-R3-DIAG, C-R3-FROM | W2-16, W2-41, W2-72 |
| RA01-B1 non reserved near miss names stay registrable | E13, E14, I09c | - |
| RA01-B1 only workspace verbs read the workspace environment | E09, I41a, I41c | W2-41, W2-72 |
| RA01-B1 orchestrator job value is text | I30f | - |
| RA01-B1 orchestrator requires all three options | C-I06, E06, E19 | W2-16 |
| RA01-B1 path text validation precedes the cwd read | E08, E09, I41b | W2-72 |
| RA01-B1 production targets never receive the authoring definition | I25, I44, I57 | - |
| RA01-B1 profile registering a reserved authoring command name is refused as identity conflict | C-SKIP-CTRL, I09c, I48 | - |
| RA01-B1 profile registering a reserved authoring family name is refused as identity conflict | C-SKIP-CTRL, I09c, I48 | - |
| RA01-B1 profile registering a reserved authoring handler id is refused as identity conflict | C-SKIP-CTRL, I09c, I48b | - |
| RA01-B1 profile registering a reserved projection id is refused as identity conflict | I09c, I51, I51b | - |
| RA01-B1 projection response refuses the five orchestrator section ids for both input kinds | I09c, I66 | - |
| RA01-B1 reserved command collision field follows registration order | C-SKIP-CTRL, I09c, I48 | - |
| RA01-B1 reserved command is refused when the profile also registers a projection | C-SKIP-CTRL, E18, I09c | - |
| RA01-B1 reserved pre check continues past a malformed entry sorted before a reserved entry | X-C01-06 | - |
| RA01-B1 reserved pre check skips an entry with any ill typed identity field | C-I53, C-SKIP-ALL, C-SKIP-F | - |
| RA01-B1 reserved pre check skips malformed descriptors and keeps their diagnostic | C-I53 | - |
| RA01-B1 reserved projection collision field follows registration order | I09c, I51, I51b | - |
| RA01-B1 reserved strings in non reserved fields keep their HEAD diagnostic | C-E17a, C-E17b, C-E17c | - |
| RA01-B1 response kinds map to envelope kinds | E03, I29 | - |
| RA01-B1 selector stage precedes the explicit path stage | C-I06, C-R3-REG, E05 | W2-16, W2-72 |
| RA01-B1 stages read the cwd at most once and only for relative paths | E09, I41a, I41b | W2-72 |
| RA01-B1 stub invocations write nothing and print no success document | C-I06, E05, E06 | W2-03 |
| RA01-B1 stub refusal is the closed internal envelope on stderr only | C-I06, E05, E06 | W2-16, W2-41 |
| RA01-B1 usage precedes the workspace selector stage | C-I06, E06, E12 | W2-16 |
| RA01-B1 validator refuses a catalog missing one row | E12, I02, I10 | W2-16 |
| RA01-B1 validator refuses a duplicated authoring row | E12, I10, I14 | W2-16 |
| RA01-B1 validator refuses a mispaired capability | E12, I10, I19 | W2-16 |
| RA01-B1 validator refuses a mispaired family | E12, I10, I16 | W2-16 |
| RA01-B1 validator refuses a mispaired handler id | E12, I03, I10 | W2-16 |
| RA01-B1 validator refuses a mispaired response kind | E12, I04, I10 | W2-16 |
| RA01-B1 validator refuses a missing option | C-I06, E12, I02 | W2-16 |
| RA01-B1 validator refuses a renamed option | C-I06, E12, I02 | W2-16 |
| RA01-B1 validator refuses a wrong option type | E12, I10, I22 | W2-16 |
| RA01-B1 validator refuses a wrong required flag | E12, I10, I23 | W2-16 |
| RA01-B1 validator refuses an extra twelfth row | E12, I10, I15 | W2-16 |
| RA01-B1 validator refuses the five shipped rows when eleven are required | E12, I10, I13 | W2-16 |
| RA01-B1 with-profile after admission an authoring tail is usage and a broken profile reports its own error | C-I06, E05, E06 | W2-41 |
| RA01-B1 with-profile outputs are identical under the authoring bundle | C-I06, E05, E06 | W2-16, W2-41 |
| RA01-B1 with-profile probe stays five and a malformed authoring tail reaches profile capture | C-I06, E05, E06 | W2-41 |
| RA01-B1 with-profile reserved command registration prints the existing identity conflict | C-SKIP-CTRL, I09c, I45 | - |
| RA01-B1 workspace selector stage expands names and keeps files lexical | I41a, I41c, I43a | - |

## Linux container probe (W2-82, D-41)

Command: `docker build -f ctx/probe.Dockerfile --target evidence --output type=local,dest=out --build-arg JOBS=6 ctx` (no image tag, no bind mount; context = the candidate copied from the checkout, `raw/linux/probe.Dockerfile`, patch scripts in `raw/linux/probe-container/`), exit 0, 2 minutes 23 seconds (`raw/linux/docker-times.txt`, `docker-build.exit`); `docker builder prune -f` afterwards (`raw/linux/builder-prune.log`, 1.822 GB reclaimed), disk 28 GiB free after. Kernel `Linux buildkitsandbox 7.0.14-linuxkit aarch64` (`raw/linux/uname.txt`). One build ran the Debug preset: configure exit 0, build of `planar_capture_directory_tests` and `planar_command_tests` exit 0, baseline `ctest -R 'RA01-B2 observe'` 21 of 21 passed (`raw/linux/ctest-base.log`). Then each mutant was applied to `src/engine/capture/directory.cpp` by a patch script, rebuilt, run, and restored by byte copy (restored sha256 `000c8cc3...` equals the original and the checkout's, `raw/linux/w2-*-restored.sha`).

| mutant | diff (raw/linux/) | sha256 of the mutated file (16) | result | failing test(s) |
|---|---|---|---|---|
| W2-51 listing sort removed | `w2-51.diff` | `20290ea87aadc4bf` | 21 run, 1 failed (killed by other) | observe refuses a symlink entry; the named test `observe lists entries sorted by category and name` passed |
| W2-45L `valid_text` dropped from `name_valid` (invalid UTF-8 names accepted; Linux branch of the layout test) | `w2-45l.diff` | `2e04e19d3588e8c8` | 21 run, 1 failed (killed by the named test) | observe refuses layout violations without echoing names |

## Skipped, not run, or run elsewhere

- **Extra exploratory mutants:** 20 run (X01 to X20), none skipped.
- **W2-77(b)** (take the sealed commands from the provider result instead of native): not run. The provider capture result is checked by `exact(copied, {"bundle","capture","files","job"})` before the native append, so the mutant needs a Lua provider that adds a `commands` key; the variants of the catalog test already exercise that path through W2-70/W2-77/W2-77d.
- **W2-09 alternative form** (capture_authoring iterating route.jobs like capture_render) and **W2-11 alternative form** (Listed-job diagnostic first): not run; the schema-mode form of W2-09 and the generic-error form of W2-11 were run and killed.
- **W2-18 second mutant** (hard-copied schema text): not run as a separate patch; the row's detection is the first mutant (schema.lua goal req to opt), which was killed, meaning the text follows the schema tables.
- **W2-20 and W2-23 pairs-order mutants:** realised for W2-20 (registry iterated with `pairs`, W2-20b, killed); for W2-23 no Lua `pairs` exists to mutate, the unsorted-collection mutant is equivalent (finding 5).
- **W2-45 Linux branch, W2-51, W2-82:** run in the Linux container (section above); nothing skipped.
- **Release configuration:** probes ran in Debug only (the inventory is identical in both).

