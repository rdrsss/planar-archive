# V5-02-RA01-B-W2 gate report (third gate run, after the probe repair)

Verdict: **GATE PASSED (passed=true).** Every command met its criterion. No source file was edited by the gate runner.
This run follows the probe repair (test-only edits to `authoring_orchestrator.t.cpp` and `authoring_prompts.t.cpp`), so the second gate report
(candidate hash 9b215481...) no longer describes the tree; it is kept verbatim as `raw/run2/REPORT-superseded-gate2.md`.
Raw logs of this run are under `raw/run3/`.

## Candidate identity

Output of the workflow `identity.py` for `src tests/native` (raw/run3/identity.txt, which holds the full per-file table):

| identity | value |
|---|---|
| HEAD | `3343ad38304db57388bc7ac2438a40043eba1b7d` |
| sha256(git diff --binary HEAD -- src tests/native), tracked only | `cdd3a0e14585381d3e361c5b0ba49a5e75dd8e77a7ef0412d9359aaa3bee9dfe` |
| combined sha256 (diff bytes, then each untracked path NUL bytes, sorted), pathspec src tests/native, evidence excluded | `62b5fec2dd8fcbac2bf61ad8c1913886a6361f0fb583654ddd243b015d647c36` |


## Commands, exit codes, counts

| # | Command | Exit | Result |
|---|---|---|---|
| 1 | `python3 scripts/rewrite-check.py lint --build-dir build/rewrite-debug` | 0 | clang-format --Werror clean; "Formatting and Doxygen checked 171 active source files" (raw/run3/lint.log) |
| 2 | `cmake --build build/rewrite-debug --target planar_tests` | 0 | `ninja: no work to do.` (an earlier build, by the probe repair, already held these exact bytes; build-debug.log) |
| 3 | `cmake --build build/rewrite-release --target planar_tests` | 0 | compiled the stale steps (832 planned), zero FAILED (build-release.log) |
| 4 | `ctest --test-dir build/rewrite-debug --no-tests=error --output-on-failure -j4` | 0 | **100% tests passed out of 1416**, 0 failed, 0 skipped (ctest-debug.log) |
| 5 | `ctest --test-dir build/rewrite-release --no-tests=error --output-on-failure -j4` | 0 | **100% tests passed out of 1416**, 0 failed, 0 skipped (ctest-release.log) |
| 6 | no-work builds of `planar_tests`, twice per config, after the successful builds | 0 each | each ended `ninja: no work to do.` (nowork-{debug,release}-{1,2}.log); `all` in debug also no work (nowork-all-debug.log) |
| 7 | `scripts/redact-local-paths.py --repo-path <repo> --home $HOME check <explicit files>` | see Redaction | see Redaction |
| 8 | `scripts/linux-gate.sh <scratch>` (one container build runs Debug and RelWithDebInfo; no -t, one build, `docker builder prune -f` after) | 0 | docker-build.exit 0, builder-prune.exit 0, see Linux |

Skip check: the only log lines matching "skipped" are the passing test named `replacement.command load refuses forged family order skipped duplicate and malformed arguments`; ctest reported no Skipped, Not Run or Failed entries.

## Linux (container, linux/arm64)

Free disk before the build: 62518760 512-byte blocks (about 30 GiB, above the 15 GiB limit); after: 39901016 blocks (about 19 GiB) with the builder prune done. Host manifests before and after equal; host manifest equals the container context manifest. Scratch dir removed afterwards.

| config | configure | build | ctest | no-work build | inventory |
|---|---|---|---|---|---|
| Debug | 0 | 0 | exit 0, 100% passed out of 1416, 0 skipped, 0 failed | 0, `ninja: no work to do.` | 1416 names, equal to macOS |
| RelWithDebInfo | 0 | 0 | exit 0, 100% passed out of 1416, 0 skipped, 0 failed | 0, `ninja: no work to do.` | 1416 names, equal to macOS |

`x07.two_process_cas_one_winner` passed in both (Test #1407). `c02_off_boundary` (`X08 C01 role preparation outputs-off boundary pins the request-record conversion charge`, pin 227275 macOS / 227301 Linux) passed on macOS Debug and RelWithDebInfo and Linux Debug and RelWithDebInfo. Logs: raw/run3/linux/container/.

## Registered inventory

`ctest -N`: 1416 tests in each macOS configuration; sorted lists identical to each other and to both Linux lists. Full list: `inventory.txt`
(C-locale sorted). Compared against the most recent committed inventory, `tests/native/v5-02-ra01-b-w1-c01-evidence/inventory.txt`
(commit 3343ad38, 1344 names): **0 removed, 72 added** (`removed-names.txt` empty, `new-names.txt` the 72). 1344 + 72 = 1416. The probe repair added no test names.

## (a) Acceptance mapping

Test names omit the `replacement.` ctest prefix. Line numbers are current for this candidate. All listed tests passed in all four runs.

| Criterion | Tests (file:line) |
|---|---|
| A35: byte-equal envelopes, five kinds, two fresh processes, reordered declarations | RA01-B2 A35 five prompt kinds are byte-equal across two fresh processes and reordered declarations (src/cmd/planar/authoring_prompts.t.cpp:1052); RA01-B2 prompts carry their sections in contract order (src/cmd/planar/authoring_prompts.t.cpp:1101) |
| A36: rejected load round trip, raw_error bytes incl. DEL and C1, job.roles[1].work excerpt, load exit 3 | RA01-B2 A36 rejected load stderr round-trips through explain-rejection (src/cmd/planar/authoring_prompts.t.cpp:1933); RA01-B2 rejection excerpt resolves job.roles[1].work to the work field (src/cmd/planar/authoring_prompts.t.cpp:1825); RA01-B2 raw_error block equals the diagnostic bytes with DEL and C1 raw (src/cmd/planar/authoring_prompts.t.cpp:1881); RA01-B2 raw_error fence outgrows backtick runs and adds one LF only when missing (src/cmd/planar/authoring_prompts.t.cpp:1903) |
| Critique: three rules, advisory exit 0, no-findings line | RA01-B2 critique rule prescription_unchecked fires only without a nonempty check (src/cmd/planar/authoring_prompts.t.cpp:1470); RA01-B2 critique rule blocking_question_unanswered covers start and complete (src/cmd/planar/authoring_prompts.t.cpp:1490); RA01-B2 critique rule facet_dependency_without_output_vector needs an output_vector check (src/cmd/planar/authoring_prompts.t.cpp:1512); RA01-B2 critique findings are sorted advisory and exit zero (src/cmd/planar/authoring_prompts.t.cpp:1540); RA01-B2 critique prints the fixed line when no rule fires (src/cmd/planar/authoring_prompts.t.cpp:1573) |
| Schema drift guard (no test with a drift name; schema text generated from schema tables) | RA01-B2 schema text lists spec job and workspace fields generated from the schema tables (src/cmd/planar/authoring_prompts.t.cpp:1161); RA01-B2 schema text format sorts fields and nests records (src/cmd/planar/authoring_prompts.t.cpp:1228)  [the mutation itself belongs to the prober] |
| Observe: symlink, FIFO, hard-link alias | RA01-B2 observe refuses a symlink entry (src/cmd/planar/authoring_observe.t.cpp:132); RA01-B2 observe refuses a FIFO entry (src/cmd/planar/authoring_observe.t.cpp:171); RA01-B2 observe refuses a hard-link alias (src/cmd/planar/authoring_observe.t.cpp:187) |
| Observe: bounds (oversize file, entries, total bytes, output bytes) | RA01-B2 observe refuses an oversize file as observation_file_bytes (src/engine/capture/directory.t.cpp:99); RA01-B2 observe bounds listed names at 1024 counting dot names (src/engine/capture/directory.t.cpp:43); RA01-B2 observe refuses more than four MiB as observation_total_bytes (src/engine/capture/directory.t.cpp:118); RA01-B2 observe output over one MiB is refused as observation_bytes (src/cmd/planar/authoring_observe.t.cpp:275) |
| Observe: duplicate IDs, unknown fields, structural only, logical file and field | RA01-B2 observe refuses duplicate ids within checks and within findings (src/cmd/planar/authoring_observe.t.cpp:301); RA01-B2 observe applies structural checks only (src/cmd/planar/authoring_observe.t.cpp:426); RA01-B2 observe errors carry the logical file and object-relative field (src/cmd/planar/authoring_observe.t.cpp:329) |
| Observe: file replaced between listing and read (capture_changed) | RA01-B2 observe refuses a directory changed before the recheck (src/engine/capture/directory.t.cpp:198) |
| Observe: empty directory, creation-order independence, dot names | RA01-B2 observe empty directory prints canonical empty observations (src/cmd/planar/authoring_observe.t.cpp:556); RA01-B2 observe output is independent of creation order (src/cmd/planar/authoring_observe.t.cpp:573); RA01-B2 observe ignores dot names (src/cmd/planar/authoring_observe.t.cpp:259) |
| Observe: sorted listing, layout, unsafe-entry precedence, first-error order, checkpoint bounds, bare output, render acceptance | RA01-B2 observe lists entries sorted by category and name (src/engine/capture/directory.t.cpp:161); RA01-B2 observe reports an unsafe entry before a later size limit (src/engine/capture/directory.t.cpp:136); RA01-B2 observe refuses layout violations without echoing names (src/cmd/planar/authoring_observe.t.cpp:217); RA01-B2 observe reports the first error in checks, findings, checkpoint order (src/cmd/planar/authoring_observe.t.cpp:512); RA01-B2 observe enforces the checkpoint bounds (src/cmd/planar/authoring_observe.t.cpp:475); RA01-B2 observe prints sorted bare observations (src/cmd/planar/authoring_observe.t.cpp:533); RA01-B2 observe output is structurally accepted by render observations (src/cmd/planar/authoring_observe.t.cpp:599) |
| Orchestrator: every available label in the sealed catalog, every future verb future, stub verb flips label | RA01-B2 orchestrator labels every catalog verb available and every future verb future (src/cmd/planar/authoring_orchestrator.t.cpp:480); RA01-B2 orchestrator label flips to available for a catalog stub verb (src/cmd/planar/authoring_orchestrator_variants.t.cpp:241) |
| Orchestrator: five sections in order, graph only, selected job only, unlisted job refused, sealed names, context_v1 not admissible | RA01-B2 orchestrator prompt carries five sections in order (src/cmd/planar/authoring_orchestrator.t.cpp:412); RA01-B2 orchestrator prompt is graph only (src/cmd/planar/authoring_orchestrator.t.cpp:521); RA01-B2 orchestrator reads only the selected job (src/cmd/planar/authoring_orchestrator.t.cpp:268); RA01-B2 orchestrator refuses an unlisted job as reference selection.job (src/cmd/planar/authoring_orchestrator.t.cpp:234); RA01-B2 orchestrator sections render shape roles and graph (src/cmd/planar/authoring_orchestrator.t.cpp:310); RA01-B2 orchestrator lists only the prescriptions that drive the job (src/cmd/planar/authoring_orchestrator.t.cpp:373); RA01-B2 orchestrator selected-job mode still checks the listed job identity (src/cmd/planar/authoring_orchestrator.t.cpp:554); RA01-B2 orchestrator selected-job mode refuses an unlisted job declaration (src/cmd/planar/authoring_orchestrator_variants.t.cpp:309); RA01-B2 native appends the sorted sealed command names to the orchestrator capture (src/cmd/planar/authoring_orchestrator_variants.t.cpp:271); RA01-B2 orchestrator_context_v1 is refused as an external projection input (src/engine/runtime/external_registration.t.cpp:5285) |
| Authoring capture: listed job absent succeeds for all three; invalid spec gives load's typed error; workspace vs spec capture | RA01-B2 authoring capture succeeds when a listed job file is absent for all three prompts (src/cmd/planar/authoring_prompts.t.cpp:773); RA01-B2 ingest and critique with an invalid spec fail with the typed error load gives (src/cmd/planar/authoring_prompts.t.cpp:819); RA01-B2 spec-prompt captures the workspace without the spec while critique and ingest capture the spec (src/cmd/planar/authoring_prompts.t.cpp:739); RA01-B2 a present job file is neither read nor listed (src/cmd/planar/authoring_prompts.t.cpp:793); RA01-B2 spec-prompt validates the workspace and registry only (src/cmd/planar/authoring_prompts.t.cpp:856); RA01-B2 authoring reports no job file errors (src/cmd/planar/authoring_prompts.t.cpp:900) |
| Prompt envelope, content and escaping | RA01-B2 prompt envelope fields digest bytes and kind per verb (src/cmd/planar/authoring_prompts.t.cpp:919); RA01-B2 prompt inputs list sorted file entries and registry header (src/cmd/planar/authoring_prompts.t.cpp:968); RA01-B2 checks section is the sorted allow-list intersected with the registry (src/cmd/planar/authoring_prompts.t.cpp:1287); RA01-B2 shape catalog lists pipeline then fanout_review (src/cmd/planar/authoring_prompts.t.cpp:1322); RA01-B2 spec prompt ends with the critique step line and embeds no host path (src/cmd/planar/authoring_prompts.t.cpp:1354); RA01-B2 spec summary lists work checks and paths sorted by id (src/cmd/planar/authoring_prompts.t.cpp:1383); RA01-B2 captured text is escaped as data in prompts (src/cmd/planar/authoring_prompts.t.cpp:1436); RA01-B2 job prompt lists sorted constructors and the job tier statement (src/cmd/planar/authoring_prompts.t.cpp:1588); RA01-B2 an oversize prompt is refused as prompt_bytes (src/cmd/planar/authoring_prompts.t.cpp:703) |
| explain-rejection: bounds, grammar, malformed, special files, inputs | RA01-B2 explain-rejection accepts 65536 bytes and refuses 65537 as diagnostic_bytes (src/cmd/planar/authoring_prompts.t.cpp:1737); RA01-B2 explain-rejection validates the diagnostic envelope and code grammar (src/cmd/planar/authoring_prompts.t.cpp:1678); RA01-B2 explain-rejection refuses malformed diagnostic JSON as schema (src/cmd/planar/authoring_prompts.t.cpp:1653); RA01-B2 read_diagnostic refuses links and special files with the existing messages (src/cmd/planar/authoring_prompts.t.cpp:1769); RA01-B2 rejection inputs carry the diagnostic size and file digest (src/cmd/planar/authoring_prompts.t.cpp:1801); RA01-B2 rejection excerpt for an unresolvable path is the fixed line (src/cmd/planar/authoring_prompts.t.cpp:1859); RA01-B2 capture_diagnostic refuses a file changed before verification (src/engine/capture/capture.t.cpp:787); RA01-B2 capture_diagnostic bounds the file at 65536 bytes from the descriptor (src/engine/capture/capture.t.cpp:856) |
| Opaque slots; include/verb mismatch | RA01-B2 diagnostic and from reach Lua only as opaque slots (src/cmd/planar/authoring_orchestrator.t.cpp:207); RA01-B2 native refuses an authoring capture whose include does not match the verb (src/cmd/planar/authoring_catalog.t.cpp:507) |
| RA-8: render and load unchanged outside authoring modes | RA01-B2 render and load keep Listed job not captured outside the authoring modes (src/cmd/planar/authoring_prompts.t.cpp:1988); existing render, init, load, history, export suites (1344 earlier names, all passed) |
| Round criterion 1: checklist demonstrable; A35 and A36 pass; Messages-table messages | rows above; all 72 new tests passed in all four runs. No separate Messages-table test exists; conformance is asserted inside the failure-path tests. |
| Round criterion 2: disjoint hunks, production binary has no authoring path (I25) | existing name `RA01-B1 default invoke and production binary refuse all six authoring verbs as usage` (src/cmd/planar/bootstrap.t.cpp) passed; disjoint hunks are a reviewer item, not a test. |
| Round criterion 3: 1344 kept, new listed, identical inventory, diagnostics byte-identical | 0 removed, 72 added; macOS Debug = RelWithDebInfo = Linux Debug = Linux RelWithDebInfo; existing diagnostic suites pass unchanged. |
| Round criterion 4: c02_off_boundary re-pinned both platforms | `X08 C01 role preparation outputs-off boundary pins the request-record conversion charge` (src/engine/runtime/external_registration.t.cpp) passed on macOS Debug and Release and Linux Debug and Release (pin 227275 macOS, 227301 Linux; c02-pin.md). Linux RelWithDebInfo was not measured separately by the coder; this gate shows it passing. |
| Round criteria 5 and 6 (probes, Opus review) | outside this gate (probe results: probes/PROBES.md) |

No corrective round closes findings in this record. The repair table of the round record (build dependency, formatting, redaction, orchestrator `already_typed`, `inputs.files`, variant 20, X06 seam scan, X08 branch audit) and the probe repair are verified by this run: lint exit 0, all builds and ctest runs green.

### All 72 new test cases with current locations

| Test (prefix replacement. omitted) | file:line |
|---|---|
| RA01-B2 A35 five prompt kinds are byte-equal across two fresh processes and reordered declarations | src/cmd/planar/authoring_prompts.t.cpp:1052 |
| RA01-B2 A36 rejected load stderr round-trips through explain-rejection | src/cmd/planar/authoring_prompts.t.cpp:1933 |
| RA01-B2 a present job file is neither read nor listed | src/cmd/planar/authoring_prompts.t.cpp:793 |
| RA01-B2 an oversize prompt is refused as prompt_bytes | src/cmd/planar/authoring_prompts.t.cpp:703 |
| RA01-B2 authoring capture succeeds when a listed job file is absent for all three prompts | src/cmd/planar/authoring_prompts.t.cpp:773 |
| RA01-B2 authoring reports no job file errors | src/cmd/planar/authoring_prompts.t.cpp:900 |
| RA01-B2 capture_diagnostic bounds the file at 65536 bytes from the descriptor | src/engine/capture/capture.t.cpp:856 |
| RA01-B2 capture_diagnostic refuses a file changed before verification | src/engine/capture/capture.t.cpp:787 |
| RA01-B2 captured text is escaped as data in prompts | src/cmd/planar/authoring_prompts.t.cpp:1436 |
| RA01-B2 checks section is the sorted allow-list intersected with the registry | src/cmd/planar/authoring_prompts.t.cpp:1287 |
| RA01-B2 critique findings are sorted advisory and exit zero | src/cmd/planar/authoring_prompts.t.cpp:1540 |
| RA01-B2 critique prints the fixed line when no rule fires | src/cmd/planar/authoring_prompts.t.cpp:1573 |
| RA01-B2 critique rule blocking_question_unanswered covers start and complete | src/cmd/planar/authoring_prompts.t.cpp:1490 |
| RA01-B2 critique rule facet_dependency_without_output_vector needs an output_vector check | src/cmd/planar/authoring_prompts.t.cpp:1512 |
| RA01-B2 critique rule prescription_unchecked fires only without a nonempty check | src/cmd/planar/authoring_prompts.t.cpp:1470 |
| RA01-B2 diagnostic and from reach Lua only as opaque slots | src/cmd/planar/authoring_orchestrator.t.cpp:207 |
| RA01-B2 explain-rejection accepts 65536 bytes and refuses 65537 as diagnostic_bytes | src/cmd/planar/authoring_prompts.t.cpp:1737 |
| RA01-B2 explain-rejection refuses malformed diagnostic JSON as schema | src/cmd/planar/authoring_prompts.t.cpp:1653 |
| RA01-B2 explain-rejection validates the diagnostic envelope and code grammar | src/cmd/planar/authoring_prompts.t.cpp:1678 |
| RA01-B2 ingest and critique with an invalid spec fail with the typed error load gives | src/cmd/planar/authoring_prompts.t.cpp:819 |
| RA01-B2 job prompt lists sorted constructors and the job tier statement | src/cmd/planar/authoring_prompts.t.cpp:1588 |
| RA01-B2 native appends the sorted sealed command names to the orchestrator capture | src/cmd/planar/authoring_orchestrator_variants.t.cpp:271 |
| RA01-B2 native refuses an authoring capture whose include does not match the verb | src/cmd/planar/authoring_catalog.t.cpp:507 |
| RA01-B2 observe applies structural checks only | src/cmd/planar/authoring_observe.t.cpp:426 |
| RA01-B2 observe bounds listed names at 1024 counting dot names | src/engine/capture/directory.t.cpp:43 |
| RA01-B2 observe empty directory prints canonical empty observations | src/cmd/planar/authoring_observe.t.cpp:556 |
| RA01-B2 observe enforces the checkpoint bounds | src/cmd/planar/authoring_observe.t.cpp:475 |
| RA01-B2 observe errors carry the logical file and object-relative field | src/cmd/planar/authoring_observe.t.cpp:329 |
| RA01-B2 observe ignores dot names | src/cmd/planar/authoring_observe.t.cpp:259 |
| RA01-B2 observe lists entries sorted by category and name | src/engine/capture/directory.t.cpp:161 |
| RA01-B2 observe output is independent of creation order | src/cmd/planar/authoring_observe.t.cpp:573 |
| RA01-B2 observe output is structurally accepted by render observations | src/cmd/planar/authoring_observe.t.cpp:599 |
| RA01-B2 observe output over one MiB is refused as observation_bytes | src/cmd/planar/authoring_observe.t.cpp:275 |
| RA01-B2 observe prints sorted bare observations | src/cmd/planar/authoring_observe.t.cpp:533 |
| RA01-B2 observe refuses a FIFO entry | src/cmd/planar/authoring_observe.t.cpp:171 |
| RA01-B2 observe refuses a directory changed before the recheck | src/engine/capture/directory.t.cpp:198 |
| RA01-B2 observe refuses a hard-link alias | src/cmd/planar/authoring_observe.t.cpp:187 |
| RA01-B2 observe refuses a symlink entry | src/cmd/planar/authoring_observe.t.cpp:132 |
| RA01-B2 observe refuses an oversize file as observation_file_bytes | src/engine/capture/directory.t.cpp:99 |
| RA01-B2 observe refuses duplicate ids within checks and within findings | src/cmd/planar/authoring_observe.t.cpp:301 |
| RA01-B2 observe refuses layout violations without echoing names | src/cmd/planar/authoring_observe.t.cpp:217 |
| RA01-B2 observe refuses more than four MiB as observation_total_bytes | src/engine/capture/directory.t.cpp:118 |
| RA01-B2 observe reports an unsafe entry before a later size limit | src/engine/capture/directory.t.cpp:136 |
| RA01-B2 observe reports the first error in checks, findings, checkpoint order | src/cmd/planar/authoring_observe.t.cpp:512 |
| RA01-B2 orchestrator label flips to available for a catalog stub verb | src/cmd/planar/authoring_orchestrator_variants.t.cpp:241 |
| RA01-B2 orchestrator labels every catalog verb available and every future verb future | src/cmd/planar/authoring_orchestrator.t.cpp:480 |
| RA01-B2 orchestrator lists only the prescriptions that drive the job | src/cmd/planar/authoring_orchestrator.t.cpp:373 |
| RA01-B2 orchestrator prompt carries five sections in order | src/cmd/planar/authoring_orchestrator.t.cpp:412 |
| RA01-B2 orchestrator prompt is graph only | src/cmd/planar/authoring_orchestrator.t.cpp:521 |
| RA01-B2 orchestrator reads only the selected job | src/cmd/planar/authoring_orchestrator.t.cpp:268 |
| RA01-B2 orchestrator refuses an unlisted job as reference selection.job | src/cmd/planar/authoring_orchestrator.t.cpp:234 |
| RA01-B2 orchestrator sections render shape roles and graph | src/cmd/planar/authoring_orchestrator.t.cpp:310 |
| RA01-B2 orchestrator selected-job mode refuses an unlisted job declaration | src/cmd/planar/authoring_orchestrator_variants.t.cpp:309 |
| RA01-B2 orchestrator selected-job mode still checks the listed job identity | src/cmd/planar/authoring_orchestrator.t.cpp:554 |
| RA01-B2 orchestrator_context_v1 is refused as an external projection input | src/engine/runtime/external_registration.t.cpp:5285 |
| RA01-B2 prompt envelope fields digest bytes and kind per verb | src/cmd/planar/authoring_prompts.t.cpp:919 |
| RA01-B2 prompt inputs list sorted file entries and registry header | src/cmd/planar/authoring_prompts.t.cpp:968 |
| RA01-B2 prompts carry their sections in contract order | src/cmd/planar/authoring_prompts.t.cpp:1101 |
| RA01-B2 raw_error block equals the diagnostic bytes with DEL and C1 raw | src/cmd/planar/authoring_prompts.t.cpp:1881 |
| RA01-B2 raw_error fence outgrows backtick runs and adds one LF only when missing | src/cmd/planar/authoring_prompts.t.cpp:1903 |
| RA01-B2 read_diagnostic refuses links and special files with the existing messages | src/cmd/planar/authoring_prompts.t.cpp:1769 |
| RA01-B2 rejection excerpt for an unresolvable path is the fixed line | src/cmd/planar/authoring_prompts.t.cpp:1859 |
| RA01-B2 rejection excerpt resolves job.roles[1].work to the work field | src/cmd/planar/authoring_prompts.t.cpp:1825 |
| RA01-B2 rejection inputs carry the diagnostic size and file digest | src/cmd/planar/authoring_prompts.t.cpp:1801 |
| RA01-B2 render and load keep Listed job not captured outside the authoring modes | src/cmd/planar/authoring_prompts.t.cpp:1988 |
| RA01-B2 schema text format sorts fields and nests records | src/cmd/planar/authoring_prompts.t.cpp:1228 |
| RA01-B2 schema text lists spec job and workspace fields generated from the schema tables | src/cmd/planar/authoring_prompts.t.cpp:1161 |
| RA01-B2 shape catalog lists pipeline then fanout_review | src/cmd/planar/authoring_prompts.t.cpp:1322 |
| RA01-B2 spec prompt ends with the critique step line and embeds no host path | src/cmd/planar/authoring_prompts.t.cpp:1354 |
| RA01-B2 spec summary lists work checks and paths sorted by id | src/cmd/planar/authoring_prompts.t.cpp:1383 |
| RA01-B2 spec-prompt captures the workspace without the spec while critique and ingest capture the spec | src/cmd/planar/authoring_prompts.t.cpp:739 |
| RA01-B2 spec-prompt validates the workspace and registry only | src/cmd/planar/authoring_prompts.t.cpp:856 |

## (b) Red-first

Source: `red/RED.md` and `red/rows.tsv` (coder evidence). Red-time identity: HEAD 3343ad38304db57388bc7ac2438a40043eba1b7d;
sha256 of `candidate.diff` (`git diff HEAD -- src tests/native`) `53cb5d25f2902d57e8b28b430a516f222e38e4b50c56654384f244206df12e02`;
sha256 of SHA256SUMS (candidate.diff plus 11 untracked files) `638128c2e248df19facd2ac68b8062b2d40565a0505e456256a8c2f20028ecca`;
snapshot in `red/snapshot/`. Build: `cmake --build build/rewrite-debug --target planar_tests`, exit 0.
Command per test, from RED.md: `ctest --test-dir build/rewrite-debug -R '^replacement\.<escaped name>$' --no-tests=error --output-on-failure`,
raw logs `red/raw/W2-NN.log`. Result: **69 expectRed rows failed an assertion (real behavioural reds against the wave 1 stubs); 10 guards passed
before the change (W2-01, 03, 04, 34, 41, 63, 72, 73, 74, 76) and are not reds.** W2-75, 81 and 82 were not run as TEST_CASEs.
Per-test failing lines (red-time numbering; the files changed afterwards) are the `failing line` column of `red/rows.tsv` (absolute paths redacted to
`<repo>`), not copied here to avoid transcription errors. Gaps in the evidence: no distinct red record for the schema drift guard; no Linux red run;
W2-31 and W2-32 share failing line authoring_prompts.t.cpp:1627, so they may not be independently red. The repair coder's test-first edits are
described in the round record's repair table; no separate red snapshot hash for them exists in the evidence. I did not re-run any red.

## (c) Deliberate behaviour differences

None found by the gate. Items the repair coder recorded as contract-grounded test changes: null and non-integer observation sections answer `schema`, field `$`,
message `JSON codec refused input` exactly as render does (authoring contract, Assembly by Lua); ids must satisfy render's `id` grammar; the
`c02_off_boundary` re-pin is authorised by the operator decision of 2026-10-30 ("re-pin and resume"). Contract-listed NEW engineering choices
(envelope inputs shape, three critique rules, required orchestrator options, observe bounds) are decisions-table entries, not deviations.

## Limitations

- Linux is arm64 only; the container ran Debug and RelWithDebInfo in a single build.
- The Debug build (command 2) had no work, so its log holds no compile steps; Release compiled everything stale. Ctest and no-work checks followed successful builds in each config.
- Probes, Opus review and the schema drift guard mutation are not part of this gate. Several probe survivors are recorded in the round record as equivalent or unkillable without a product change.
- Red-first evidence predates the probe repair; the probe repair's test strengthening has no red snapshot hash (mutation-kill evidence is in probes/PROBES.md).

## Redaction result

`python3 scripts/redact-local-paths.py --repo-path <repo> --home $HOME check <files>` on the explicit list in raw/run3/redact-files.txt
(59 files: every file changed or untracked per git status excluding build output, plus every non-raw evidence file including this REPORT.md,
inventory.txt, plan/, probes/PROBES.md, red/RED.md and red/rows.tsv): **exit 0**, no output (raw/run3/redact.log). The no-argument form
errors on required options and is not a check. Raw logs and snapshot directories are not committable and were not checked.
