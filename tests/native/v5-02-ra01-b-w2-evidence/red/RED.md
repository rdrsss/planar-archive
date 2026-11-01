# RED check, round V5-02-RA01-B-W2

Red-time identity (covers tracked and untracked bytes):
- HEAD: 3343ad38304db57388bc7ac2438a40043eba1b7d
- sha256 candidate.diff (git diff HEAD -- src tests/native): 53cb5d25f2902d57e8b28b430a516f222e38e4b50c56654384f244206df12e02
- sha256 SHA256SUMS (candidate.diff plus 11 untracked files): 638128c2e248df19facd2ac68b8062b2d40565a0505e456256a8c2f20028ecca
- Snapshot: red/snapshot/

Build: cmake --build build/rewrite-debug --target planar_tests, exit 0 (raw/build.log).
Command per test: ctest --test-dir build/rewrite-debug -R '^replacement\.<escaped name>$' --no-tests=error --output-on-failure (ctest names carry a replacement. prefix; a first attempt without it matched nothing and was rerun). Raw logs: raw/W2-NN.log.
Not run as TEST_CASEs: W2-75, W2-81, W2-82 (coordinator/other steps).

Result: 69 expectRed tests failed-assertion; 10 guards passed.

| test | invariant | expectRed | outcome | failing line |
|---|---|---|---|---|
| RA01-B1 explicit path stage precedes the stub refusal | W2-01 | false | passed |  |
| RA01-B1 each authoring verb parses its signature through the bundle to the stub refusal | W2-02 | true | failed-assertion | src/cmd/planar/authoring.t.cpp:527 |
| RA01-B1 stub invocations write nothing and print no success document | W2-03 | false | passed |  |
| RA01-B1 default invoke and production binary refuse all six authoring verbs as usage | W2-04 | false | passed |  |
| RA01-B2 native refuses an authoring capture whose include does not match the verb | W2-05 | true | failed-assertion | src/cmd/planar/authoring_catalog.t.cpp:516 |
| RA01-B2 diagnostic and from reach Lua only as opaque slots | W2-06 | true | failed-assertion | src/cmd/planar/authoring_orchestrator.t.cpp:216 |
| RA01-B2 an oversize prompt is refused as prompt_bytes | W2-07 | true | failed-assertion | src/cmd/planar/authoring_prompts.t.cpp:707 |
| RA01-B2 spec-prompt captures the workspace without the spec while critique and ingest capture the spec | W2-08 | true | failed-assertion | src/cmd/planar/authoring_prompts.t.cpp:753 |
| RA01-B2 authoring capture succeeds when a listed job file is absent for all three prompts | W2-09 | true | failed-assertion | src/cmd/planar/authoring_prompts.t.cpp:783 |
| RA01-B2 a present job file is neither read nor listed | W2-10 | true | failed-assertion | src/cmd/planar/authoring_prompts.t.cpp:805 |
| RA01-B2 ingest and critique with an invalid spec fail with the typed error load gives | W2-11 | true | failed-assertion | src/cmd/planar/authoring_prompts.t.cpp:846 |
| RA01-B2 spec-prompt validates the workspace and registry only | W2-12 | true | failed-assertion | src/cmd/planar/authoring_prompts.t.cpp:863 |
| RA01-B2 authoring reports no job file errors | W2-13 | true | failed-assertion | src/cmd/planar/authoring_prompts.t.cpp:910 |
| RA01-B2 prompt envelope fields digest bytes and kind per verb | W2-14 | true | failed-assertion | src/cmd/planar/authoring_prompts.t.cpp:937 |
| RA01-B2 prompt inputs list sorted file entries and registry header | W2-15 | true | failed-assertion | src/cmd/planar/authoring_prompts.t.cpp:980 |
| RA01-B2 A35 five prompt kinds are byte-equal across two fresh processes and reordered declarations | W2-16 | true | failed-assertion | src/cmd/planar/authoring_prompts.t.cpp:1075 |
| RA01-B2 prompts carry their sections in contract order | W2-17 | true | failed-assertion | src/cmd/planar/authoring_prompts.t.cpp:1130 |
| RA01-B2 schema text lists spec job and workspace fields generated from the schema tables | W2-18 | true | failed-assertion | src/cmd/planar/authoring_prompts.t.cpp:1168 |
| RA01-B2 schema text format sorts fields and nests records | W2-19 | true | failed-assertion | src/cmd/planar/authoring_prompts.t.cpp:1233 |
| RA01-B2 checks section is the sorted allow-list intersected with the registry | W2-20 | true | failed-assertion | src/cmd/planar/authoring_prompts.t.cpp:1304 |
| RA01-B2 shape catalog lists pipeline then fanout_review | W2-21 | true | failed-assertion | src/cmd/planar/authoring_prompts.t.cpp:1327 |
| RA01-B2 spec prompt ends with the critique step line and embeds no host path | W2-22 | true | failed-assertion | src/cmd/planar/authoring_prompts.t.cpp:1359 |
| RA01-B2 spec summary lists work checks and paths sorted by id | W2-23 | true | failed-assertion | src/cmd/planar/authoring_prompts.t.cpp:1414 |
| RA01-B2 captured text is escaped as data in prompts | W2-24 | true | failed-assertion | src/cmd/planar/authoring_prompts.t.cpp:1444 |
| RA01-B2 critique rule prescription_unchecked fires only without a nonempty check | W2-25 | true | failed-assertion | src/cmd/planar/authoring_prompts.t.cpp:1480 |
| RA01-B2 critique rule blocking_question_unanswered covers start and complete | W2-26 | true | failed-assertion | src/cmd/planar/authoring_prompts.t.cpp:1500 |
| RA01-B2 critique rule facet_dependency_without_output_vector needs an output_vector check | W2-27 | true | failed-assertion | src/cmd/planar/authoring_prompts.t.cpp:1522 |
| RA01-B2 critique findings are sorted advisory and exit zero | W2-28 | true | failed-assertion | src/cmd/planar/authoring_prompts.t.cpp:1553 |
| RA01-B2 critique prints the fixed line when no rule fires | W2-29 | true | failed-assertion | src/cmd/planar/authoring_prompts.t.cpp:1575 |
| RA01-B2 job prompt lists sorted constructors and the job tier statement | W2-30 | true | failed-assertion | src/cmd/planar/authoring_prompts.t.cpp:1592 |
| RA01-B2 explain-rejection refuses malformed diagnostic JSON as schema | W2-31 | true | failed-assertion | src/cmd/planar/authoring_prompts.t.cpp:1627 |
| RA01-B2 explain-rejection validates the diagnostic envelope and code grammar | W2-32 | true | failed-assertion | src/cmd/planar/authoring_prompts.t.cpp:1627 |
| RA01-B2 explain-rejection accepts 65536 bytes and refuses 65537 as diagnostic_bytes | W2-33 | true | failed-assertion | src/cmd/planar/authoring_prompts.t.cpp:1723 |
| RA01-B2 read_diagnostic refuses links and special files with the existing messages | W2-34 | false | passed |  |
| RA01-B2 rejection inputs carry the diagnostic size and file digest | W2-35 | true | failed-assertion | src/cmd/planar/authoring_prompts.t.cpp:1784 |
| RA01-B2 rejection excerpt resolves job.roles[1].work to the work field | W2-36 | true | failed-assertion | src/cmd/planar/authoring_prompts.t.cpp:1808 |
| RA01-B2 rejection excerpt for an unresolvable path is the fixed line | W2-37 | true | failed-assertion | src/cmd/planar/authoring_prompts.t.cpp:1842 |
| RA01-B2 raw_error block equals the diagnostic bytes with DEL and C1 raw | W2-38 | true | failed-assertion | src/cmd/planar/authoring_prompts.t.cpp:1864 |
| RA01-B2 raw_error fence outgrows backtick runs and adds one LF only when missing | W2-39 | true | failed-assertion | src/cmd/planar/authoring_prompts.t.cpp:1898 |
| RA01-B2 A36 rejected load stderr round-trips through explain-rejection | W2-40 | true | failed-assertion | src/cmd/planar/authoring_prompts.t.cpp:1938 |
| RA01-B1 explain-rejection and observe ignore an invalid workspace environment | W2-41 | false | passed |  |
| RA01-B2 observe refuses a symlink entry | W2-42 | true | failed-assertion | src/cmd/planar/authoring_observe.t.cpp:140 |
| RA01-B2 observe refuses a FIFO entry | W2-43 | true | failed-assertion | src/cmd/planar/authoring_observe.t.cpp:180 |
| RA01-B2 observe refuses a hard-link alias | W2-44 | true | failed-assertion | src/cmd/planar/authoring_observe.t.cpp:195 |
| RA01-B2 observe refuses layout violations without echoing names | W2-45 | true | failed-assertion | src/cmd/planar/authoring_observe.t.cpp:228 |
| RA01-B2 observe ignores dot names | W2-46 | true | failed-assertion | src/cmd/planar/authoring_observe.t.cpp:123 |
| RA01-B2 observe bounds listed names at 1024 counting dot names | W2-47 | true | failed-assertion | src/engine/capture/directory.t.cpp:52 |
| RA01-B2 observe refuses an oversize file as observation_file_bytes | W2-48 | true | failed-assertion | src/engine/capture/directory.t.cpp:105 |
| RA01-B2 observe refuses more than four MiB as observation_total_bytes | W2-49 | true | failed-assertion | src/engine/capture/directory.t.cpp:125 |
| RA01-B2 observe reports an unsafe entry before a later size limit | W2-50 | true | failed-assertion | src/engine/capture/directory.t.cpp:145 |
| RA01-B2 observe lists entries sorted by category and name | W2-51 | true | failed-assertion | src/engine/capture/directory.t.cpp:174 |
| RA01-B2 observe refuses a directory changed before the recheck | W2-52 | true | failed-assertion | src/engine/capture/directory.t.cpp:218 |
| RA01-B2 observe output over one MiB is refused as observation_bytes | W2-53 | true | failed-assertion | src/cmd/planar/authoring_observe.t.cpp:281 |
| RA01-B2 observe refuses duplicate ids within checks and within findings | W2-54 | true | failed-assertion | src/cmd/planar/authoring_observe.t.cpp:114 |
| RA01-B2 observe errors carry the logical file and object-relative field | W2-55 | true | failed-assertion | src/cmd/planar/authoring_observe.t.cpp:114 |
| RA01-B2 observe applies structural checks only | W2-56 | true | failed-assertion | src/cmd/planar/authoring_observe.t.cpp:114 |
| RA01-B2 observe enforces the checkpoint bounds | W2-57 | true | failed-assertion | src/cmd/planar/authoring_observe.t.cpp:486 |
| RA01-B2 observe reports the first error in checks, findings, checkpoint order | W2-58 | true | failed-assertion | src/cmd/planar/authoring_observe.t.cpp:114 |
| RA01-B2 observe prints sorted bare observations | W2-59 | true | failed-assertion | src/cmd/planar/authoring_observe.t.cpp:123 |
| RA01-B2 observe empty directory prints canonical empty observations | W2-60 | true | failed-assertion | src/cmd/planar/authoring_observe.t.cpp:123 |
| RA01-B2 observe output is independent of creation order | W2-61 | true | failed-assertion | src/cmd/planar/authoring_observe.t.cpp:582 |
| RA01-B2 observe output is structurally accepted by render observations | W2-62 | true | failed-assertion | src/cmd/planar/authoring_observe.t.cpp:603 |
| RA01-B1 explicit path stage keeps the existing messages | W2-63 | false | passed |  |
| RA01-B2 orchestrator refuses an unlisted job as reference selection.job | W2-64 | true | failed-assertion | src/cmd/planar/authoring_orchestrator.t.cpp:244 |
| RA01-B2 orchestrator reads only the selected job | W2-65 | true | failed-assertion | src/cmd/planar/authoring_orchestrator.t.cpp:154 |
| RA01-B2 orchestrator sections render shape roles and graph | W2-66 | true | failed-assertion | src/cmd/planar/authoring_orchestrator.t.cpp:154 |
| RA01-B2 orchestrator lists only the prescriptions that drive the job | W2-67 | true | failed-assertion | src/cmd/planar/authoring_orchestrator.t.cpp:154 |
| RA01-B2 orchestrator prompt carries five sections in order | W2-68 | true | failed-assertion | src/cmd/planar/authoring_orchestrator.t.cpp:154 |
| RA01-B2 orchestrator labels every catalog verb available and every future verb future | W2-69 | true | failed-assertion | src/cmd/planar/authoring_orchestrator.t.cpp:154 |
| RA01-B2 orchestrator label flips to available for a catalog stub verb | W2-70 | true | failed-assertion | src/cmd/planar/authoring_orchestrator_variants.t.cpp:247 |
| RA01-B2 orchestrator prompt is graph only | W2-71 | true | failed-assertion | src/cmd/planar/authoring_orchestrator.t.cpp:154 |
| RA01-B1 orchestrator requires all three options | W2-72 | false | passed |  |
| RA01-B2 orchestrator_context_v1 is refused as an external projection input | W2-73 | false | passed |  |
| X08 C01 role preparation outputs-off boundary pins the request-record conversion charge | W2-74 | false | passed |  |
| RA01-B2 render and load keep Listed job not captured outside the authoring modes | W2-76 | false | passed |  |
| RA01-B2 native appends the sorted sealed command names to the orchestrator capture | W2-77 | true | failed-assertion | src/cmd/planar/authoring_orchestrator_variants.t.cpp:278 |
| RA01-B2 capture_diagnostic refuses a file changed before verification | W2-78 | true | failed-assertion | src/engine/capture/capture.t.cpp:797 |
| RA01-B2 capture_diagnostic bounds the file at 65536 bytes from the descriptor | W2-79 | true | failed-assertion | src/engine/capture/capture.t.cpp:864 |
| RA01-B2 orchestrator selected-job mode still checks the listed job identity | W2-80 | true | failed-assertion | src/cmd/planar/authoring_orchestrator.t.cpp:154 |
