# Remaining steps: up-front design pass

Written 2026-10-27, while X06-D-C01 was still in progress. This is a planning input for the
coordinator, search and reviewer of each remaining step. It is not a contract and does not amend
one. Where it says "contract decides", the cited exact contract wins over anything written here.
Where it says "operator question", nothing dependent should be coded until the operator answers.

Current status lives in the [roadmap opening](roadmap.md#current-status). This document covers the
"Next work" rows after X06-D: X07, X08, RA01 and R15 to R19.

## Why this document exists

Recent steps took three to six workflow runs each. In the review records the cause is the same each
time: a scope or design choice was made silently by a coder, and the final reviewer found it two or
three hours later.

- X06-C needed five runs because nobody decided up front how private the internal ledger seam had to
  be, or against which threat model. The design went from an exported seam, to a passkey, to an
  internal partition, to the ADL route, and finally to a nested namespace plus operator decision B.
  See the [C02 review](rounds/V5-02-X06-C-C02-review.md), the [C03 review](rounds/V5-02-X06-C-C03-review.md)
  and the [C04 record](rounds/V5-02-X06-C-C04.md#operator-decisions-2026-10-26-recorded-verbatim).
- X06-D's first run narrowed criterion B1 (one factory budget threaded through every stage) to a
  single capture charge made after allocation. The [X06-D review](rounds/V5-02-X06-D-review.md) found
  this as R1 and R2.

For each step, this document lists four things before any coding: the clauses that are easy to
narrow, the choices a coder would otherwise make alone, the operator questions, and the review
findings that have come up again and again.

How to use it:

- The coordinator cites the relevant section in the round record's dispatch.
- Search copies the numbered acceptance clauses into the invariant table, one row each.
- Search reports any new decision it finds, either as "contract decides" with a citation or as an
  escalation.
- The reviewer checks the candidate against the decision tables here as well as the contracts.

## Standing pre-emptions from past reviews

These findings came up in several X06 reviews. Every remaining code round's brief should include
them as explicit requirements, not leave them to the reviewer.

| # | Recurring finding | Brief requirement |
| --- | --- | --- |
| P1 | A criterion is narrowed to what was easy, for example B1 composed in one stage only ([X06-D R1](rounds/V5-02-X06-D-review.md)) | For every clause marked "narrowing risk" below, the invariant table lists each module and stage the clause covers, with one row per stage. A stage left out needs either a recorded operator amendment or a halt. A coder never narrows scope. |
| P2 | Ordering words are ignored: "before allocation", "before external Lua", "before CAS" (X06-D R2) | Each ordering clause gets its own row. The row's mutant moves the check after the action, and the test must observe the order, not only the outcome. |
| P3 | The gate order was not followed: focused runs before the full runs (X06-D R3) | The gate runner writes the order into `REPORT.md`, and every step has a raw log with an exit file. |
| P4 | The invariant table has no rows for process gates, or names a test that its own mutant does not kill (X06-D R4) | G rows carry "mutant: not applicable" plus their evidence file. The prober's named killer must equal the table's test. Otherwise the row is corrected before review. |
| P5 | The red-first snapshot is identified by a tracked-only hash (raised in four X06-C reviews) | The red runner saves `git diff HEAD` plus the bytes or per-file hashes of every untracked candidate file into `red/` before running. |
| P6 | Probe rows are run against stale variant binaries ([C03 review, C03-R5](rounds/V5-02-X06-C-C03-review.md)) | The prober rebuilds every generated variant test target on each row, or after each restore. |
| P7 | A product type gains a privilege only so that a test can reach it (the C02 `test_probe` friend) | No new friend, public field or hook only for tests unless the round record allows it. Otherwise report the limit. |
| P8 | Claims of exclusivity were never probed from a real consumer target (C02, C03) | Any "cannot be called or named from X" claim needs negative-compile probes that use the exact flags of the consuming CLI target. Operator decision B is the bar: deliberate adversarial C++ is a recorded limit, not request-changes. |
| P9 | A configure-time guard in product builds depends on source text ([X06-D R5](rounds/V5-02-X06-D-review.md)) | Test-only mechanisms stay in test targets. Product configures never depend on test text. |
| P10 | Evidence has no README index, and its statements go stale after repairs | Each evidence folder has a `README.md` that names the final identity and lists every superseded statement. |
| P11 | Machine-local paths appear in evidence logs (C03 limits) | Redact before the commit step and check all committed files, as [orchestration](orchestration.md#commits) requires. |
| P12 | New tests slow #531, which takes about 15.5 s of its 30 s timeout | Budget-sensitive tests use small injected ceilings, and a new test never joins #531. Each round records the #531 time. |

## Cross-cutting decisions

These choices recur in every remaining step. Deciding them once avoids rediscovering them per round.

| ID | Decision | Status and source |
| --- | --- | --- |
| XD1 | Threat model for module privacy and authority seams: trusted in-tree code that uses exported names. Language visibility plus a source scan is the guarantee. Deliberate adversarial C++ is a recorded limit. | Decided: operator decision B, [X06-C-C04](rounds/V5-02-X06-C-C04.md#threat-model-operator-decision-b-recorded-for-criterion-9). Apply it unchanged to the X07 creation-site audits. |
| XD2 | Threat model for files: trusted profile and registry versus untrusted workspace, source, base and observations. Hostile writers, change-and-revert and writes after the final receipt check are not covered. | Decided: [external-profile contract X-CAP](external-profile-contract.md#x-cap--strict-capture-and-trust), last paragraph. |
| XD3 | No public field, flag or fault hook only for observability. Faults go through generated variant targets (the existing `planar_cli_variant_*`, `planar_load_cli_probe` and runtime budget variants). Anything unobservable is a recorded limit. | Decided: [orchestration](orchestration.md#round-sequence) step 5 and the X06-D B7 criterion. |
| XD4 | Exhaustion uses the existing `resource_limit`, and every new native site uses the closed X-LIMIT message table. No new public code or message is added. | Decided: [X-LIMIT](external-profile-contract.md#x-limit--cumulative-ceilings-and-errors). |
| XD5 | Exact test-name preservation. When a contract-mandated behaviour change, such as format2 init, changes an existing test's expectation, the test keeps its name. The record lists each changed assertion with the clause that requires it. | Decided by the [round protocol](orchestration.md#round-sequence) and the X06-D-C01 practice. Briefs must say this explicitly, or the reviewer will flag "test weakened". |
| XD6 | "Full gate" means focused runs, then full macOS Debug, then RelWithDebInfo on the same frozen candidate. Each has an inventory, uses `--no-tests=error` and has zero skips, followed by lint and two no-work builds. From X08 it also includes Linux. | Decided: [testing](../testing.md) and the [implementation path](external-profile-implementation.md#verification-and-handoff-protocol). |

## X07: public cutover (one round, two waves)

### Scope

X07 is the only step that turns on `with-profile` in the shipped binary. The
[operator amendment of 2026-10-25](profile-preparation-architecture.md#bounded-delivery-sequence-and-independent-gates)
merges X07-A and X07-B into one round with two coder waves, because the public switch may ship only
in one frozen candidate together with its service and replay tests
([implementation path](external-profile-implementation.md#source-seams-and-bounded-implementation-rounds),
X07-A row and the paragraph after the table).

X07 must deliver:

- The production CLI11 envelope (`with-profile --stdlib-profile FILE ...`), including the
  embedded/opaque probe, the final catalog parser and selected-catalog help.
- Public dispatch for all five shipped families under a profile, plus external commands through
  `handlers/extension/`.
- Fresh profile load through `open_known_load`, then `prepare_load`, then the exported
  `complete_acceptance(prepared_profile)` overload.
- Early saved replay for `request_v2`.
- Format2 `init`, and embedded load on format2 ledgers with the `request_v2` embedded selector.
- Realistic scratch CLI suites.
- Reference documentation updates in `docs/cli-reference.md`, `docs/architecture.md` and
  `docs/testing.md`.

X07 must not do the following:

- No Linux run and no claim of full external-profile acceptance. Both belong to X08.
- No new public error code or message, no profile flag outside the envelope, no root or global
  profile flag, no environment selector, no `stdlib.d` discovery, and no change to the default root
  `--help` bytes. The last one is contract-mandated, so a reviewer must not ask for root help to
  advertise `with-profile`.
- No mutation, process or arbitrary I/O capability for extension verbs.
- No offline format conversion tool.
- No widening of the X06-C seam.
- No new budget categories beyond what X06-D-C01 lands. X07 only threads the existing owner (see the
  dependencies section).
- No authoring prompts or observe verb (RA01).

### Acceptance clauses

Each clause below needs its own invariant-table row. "Narrowing risk" marks clauses whose wording
invites partial delivery, followed by what full compliance means.

Envelope, parser and precedence ([X-CLI](external-profile-contract.md#x-cli--envelope-and-admission-order)
steps 1 to 8; [X01 spike](external-envelope-spike.md#exact-argv-and-results); E02):

1. Only the existing whole-invocation `--name=value` guard runs before CLI11. There is no manual
   split, option scan or shadow argv parser (X-CLI 1; CLAUDE.md engineering conventions).
2. The envelope uses `prefix_command()`, a scalar `--stdlib-profile` with Throw multiplicity, the
   nonempty-and-not-starting-with-`--` validator, valid UTF-8 with no NUL, and a counted `--help` with
   default help disabled. The prefix count is exactly 3, or 4 for selected help. `./--help` is a
   valid filename (X-CLI 2).
3. The reverse-order passthrough stack is copied unchanged to both probes. The embedded probe has an
   optional `opaque-tail` vector positional with `expected(-1)` and `positionals_at_end()`. A
   shipped-signature failure is usage immediately and never falls back to the extension path. A
   leading unknown flag or terminator fails before any profile access (X-CLI 3).
   **Narrowing risk:** "all X01 argv rows" means all 44 envelope rows and all 12 opaque-probe rows
   run against the production bootstrap, not the spike fixture. Each row asserts the production exit
   code, the precedence class and whether the profile was accessed (see D7-3).
4. One cwd snapshot and one workspace-environment snapshot are taken per invocation. The same
   snapshot is used for known slots, for the `extension_invocation::admit` late slots and for profile
   selector normalization. The profile selector is normalized lexically to an absolute path without
   requiring the file to exist. Path ceilings are 64 MiB, and the request ceiling still wins (X-CLI 4;
   architecture "Constructor, parse and consumer protocol").
5. Load only: the verified read-only lookup of the exact saved body happens before profile capture,
   registration, evaluation or CAS. An equal body returns the saved bytes. An unequal body returns
   `request_conflict`. On a format1 ledger, a saved v1 body under the same request ID conflicts and an
   unknown ID refuses `unsupported_ledger_version` before the profile is read. The read snapshot
   closes before capture (X-CLI 5; [X-STORE format table](external-profile-contract.md#x-store--byte-codecs-exact-provenance-and-supported-formats);
   architecture "One native transaction algorithm" step 1).
6. Known shipped tail: after the early replay miss, the profile closure is captured, and every
   supplied native root, file and target slot of that shipped family is checked before any external
   Lua runs (X-CLI 6; X-CAP "Known shipped tail"; E03).
   **Narrowing risk:** this applies to all five shipped families (render, init, history, export,
   load), not only load. Today only `open_known_load` exists (see D7-1).
7. Opaque tail: capture, then pure registration and sealing, then the final CLI11 parse, then the
   late-slot checks with retained receipts, all before the handler or any read service runs. A
   malformed profile takes precedence over an unknown command. Final syntax and selector errors take
   precedence over late-slot overlap. Overlap takes precedence over the handler, even when the handler
   would never call the service (X-CLI 7; X-CAP "Opaque extension tail"; E02/E03).
8. The final parser is built from the sealed catalog, and its option storage stays alive through
   dispatch. For shipped commands the final option state exactly equals the probe state, with no
   reinterpretation. A token that spells another command in a scalar position stays a value (X-CLI 7).
9. The callable is bound by sealed native registry reference in the same retained VM and captured
   bundle. It is dispatched once, and all state is destroyed at the terminal result (X-CLI 8).
10. Exit codes: syntax 2; profile capture, schema or resource 3; native I/O 4; malformed trusted
    API or response is redacted `internal` 4; collisions are `reference` 3. No code, host path or
    traceback is printed (X-CLI, the paragraph after step 8).
11. Selected help: UTF-8 LF text in the exact format; commands and options sorted by unsigned bytes;
    `\\n`, `\\t`, `\\u00XX` and `\\\\` escaping; both option orders accepted; buffered within 1 MiB;
    no Lua output. Default root help stays byte-for-byte unchanged (X-CLI help paragraph).

Owner continuity ([architecture](profile-preparation-architecture.md#constructor-parse-and-consumer-protocol)):

12. One `profile_invocation` lives from capture to dispatch. For known load, the final parameters
    must exactly equal the bound initial intent. Root and file receipts survive the parse. Extension
    dispatch uses `into_extension`. X07 never calls `extension_invocation::create` or
    `command_vm::create(profile_snapshot)` on an already registered profile. "Review must audit every
    actual creation site."
13. Exactly one capture and one registration happen per invocation (architecture adversarial matrix,
    "Parse continuation" row).

Services, responses and options ([X-REG](external-profile-contract.md#x-reg--registration-and-immutable-ownership),
[X-PROJ](external-profile-contract.md#x-proj--pure-projections-and-native-read-capabilities); E08, E09):

14. The `history_id` boundary vectors run through the real CLI for each of `revision`, `at-event`
    and `after-event`: absent, `1`, `2147483647`, `2147483648`, `9223372036854775807`, the twelve
    invalid spellings, and selector conflicts. Valid after-event plus limit paging is included.
    **Narrowing risk:** the contract says X07 "must prove Lua value type and presence, not merely
    successful native parsing". The handler must report `type()` and `present`, for example through a
    summary text, and the test asserts those values.
15. The general decimal vectors (`2147483647` accepted; `2147483648` and INT64_MAX refused) and the
    history limit 1/100 accepted, 0/101 refused, all through the real CLI.
16. Capability, type and name misuse is refused at registration through the real CLI: `history_id`
    on another name, on a command without `history_read_v1`, or marked required. This is `internal`
    exit 4 (X07-B row; X-REG).
17. Role-read workspace, source, base and observation overlaps and aliases refuse before the handler
    or read service, even when the handler skips the service. Initial late-slot receipts survive to
    the result seal (X07-B row).
18. The response is the closed `extension_v1` with summary, role, history or projection data, and it
    matches the retained services. The multi-omission projection vector (87, then 94, then 59 bytes)
    is reproduced through the CLI (E08, X07 portion).
19. Custom shape load and render (`render_external_v1`, with a mandatory `shape_policy` section),
    a custom verb with a projection, and the recorded read and export all work through the full
    profile (X07-B row).

Replay, races and transport (X07-B row; [X-STORE](external-profile-contract.md#x-store--byte-codecs-exact-provenance-and-supported-formats)
last paragraph; E10, E13):

20. Early saved replay still returns the saved bytes after the profile, modules, workspace and
    registry are deleted and the head has advanced. A changed selector, actor, head or target
    conflicts. The same selector with changed bytes under a fresh ID captures new provenance.
21. Under the lock, an exact relookup returns an equal replay before a stale CAS. An unequal body
    conflicts. With different IDs exactly one CAS wins.
22. Known and late root, file and ancestor receipt races refuse before result authorization. A final
    race on the profile root, a file or an ancestor rolls back before the load commits. No Lua runs
    under the lock.
23. Allocation, Lua, encoding and stdout failures after commit keep the completion receipt and the
    old recovery fallbacks.

Format cutover ([X-CLI](external-profile-contract.md#x-cli--envelope-and-admission-order) opening;
X-STORE format table and last paragraph; E11):

24. Public `init` creates format2 only, at a new explicit target. Its CLI signature and response
    bytes are unchanged, and there is no format selector. Native fixtures keep explicit format1
    creation.
25. Ordinary embedded load on a format2 ledger frames `request_v2` with the embedded selector and
    uses the old embedded provenance with v1 interpretation and versions. On format1 it keeps
    `request_v1`.
    **Narrowing risk:** today the public prelude always builds a v1 intent
    (`src/cmd/planar/invocation.cpp`, the `request_intent::create` call before the early lookup).
    After cutover, every public load on a fresh ledger is a format2 load. The intent codec must
    follow the format of the ledger that was opened.
26. A profile load on format2 uses `request_v2` with the profile selector, `effective_stdlib_v1`,
    `interpretation_v2` and `versions_v2`.
27. History and export under an explicit envelope validate the selected profile before handler
    dispatch, and the profile does not reinterpret recorded semantics. Ordinary history and export
    never need a profile. Export refuses a trusted root or selected file that overlaps the proposed
    target tree, before publication (X-STORE last paragraph; X-CAP ledger and export sentences).
28. The ledger selected file must not alias the profile or any module (X-CAP).

Output and limits:

29. One buffered Glaze JSON response plus LF on stdout. Errors are JSON plus LF on stderr. Nothing is
    printed during registration or services. The 16 MiB whole-response ceiling applies (X-PROJ last
    paragraph).
30. The whole-invocation budget owner is carried through X07's new stages (see D7-8 and operator
    question Q2). **Narrowing risk:** this is the B1 pattern again. Late-slot admission, role-read
    capture, history reads converted into the VM and projection calls after `into_extension` must
    charge the same owner.

Process:

31. Every row has a red-first test where the behaviour is new (P5). The gates follow XD6. The full
    current inventory is preserved: about 1098 names in the C01 working tree, not yet approved. The
    "405" in the implementation path's X08 row is a stale figure. #531 is not slowed.
32. `docs/cli-reference.md`, `docs/architecture.md` and `docs/testing.md` describe the envelope and
    the format2 init. The roadmap opening is updated by the coordinator.

### Design decisions implied

| ID | Choice a coder would otherwise make alone | Options | Recommendation and reasoning | Status |
| --- | --- | --- | --- | --- |
| D7-1 | How with-profile render, init, history and export are admitted and dispatched. The architecture opens only known load and says "Other known shipped families require their own finite admission path; this document does not open them". Today `command_vm::invoke` refuses profile owners. | A: one finite factory per family in `planar.engine.runtime` (`open_known_render`, and so on), each admitting that family's known slots before registration. B: one closed factory over the five-member `command_options` variant. C: refuse with-profile for shipped families other than load (this contradicts X01 rows such as shipped-history, shipped-init, shipped-export and shipped-render, which are accepted). | B. It keeps one creation site to audit and a finite variant. Per-family admission tables sit inside it, with no generic service factory. Render dispatches through the approved `render_profile` phases, so custom shapes and `render_external_v1` apply. Init, history and export dispatch their sealed shipped callables in the profile VM with the existing native services. The record must explain how this satisfies X-CLI step 8. Like load, which uses `prepare_load` and not the shipped load callable, profile render runs retained trusted phases. | **Operator question Q1**, plus a short reviewed architecture addendum, because the architecture requires revision "before dependent code when those alter constructor ... authority". |
| D7-2 | Known slots per family and what is checked before registration | Taken from X-CAP | Render: workspace root and selected declaration file, registry file, source root, base root, observations file. Load: the existing set. Init: the ledger target, as a selected-file alias check and a check that it is not inside the trusted root. Its parent is a native target, and nothing exists yet. History: the ledger file alias. Export: the ledger alias, and target-tree overlap with the trusted root and selected files before publication. Selected help: trusted capture only. | Contract decides (X-CAP paragraphs on root overlap, registry, ledger and export). For init the target-not-inside-trusted-root reading is mine; mark it uncertain and let search confirm it against X-CAP. |
| D7-3 | Expected production outcome for each X01 row | Keep the spike labels, or map each to a production exit and precedence | Add a mapping table to the round record before coding. For each of the 56 rows it gives the exit code, the precedence class (syntax guard, prefix usage, probe usage, profile error, final usage, late overlap, handler) and the profile-accessed flag. Spike labels such as `final:ExtrasError` become "usage exit 2, after profile capture and registration". The verb-as-value row changes from the spike's final-stage refusal to a probe refusal before profile access, because of the X-CLI step 3 leading-flag rule. Search must derive each row from X-CLI and record it. | Contract decides the outcomes. The table is engineering work, but it must exist before red tests are written. |
| D7-4 | Where the envelope code lives | Inside `bootstrap.cpp`; a new `src/cmd/planar/envelope.{cppm,cpp}` module below bootstrap; inside handlers | A new envelope module that only bootstrap imports. Handlers never import it, so there is no handler-to-bootstrap cycle. The X07-A owned list names `bootstrap.*`, `invocation.*` and "scoped response/extension family modules", so the coordinator must list the new module explicitly ("Do not broaden ownership ad hoc"). | Engineering. Record it in the dispatch. |
| D7-5 | How "external Lua did not run" is observed without hooks | A test hook; a counter observer; a module whose execution would change the error | The profile module does unbounded work at chunk level. If it runs, the result is `resource_limit`. The expected precedence result, such as `unsafe_path` or usage, therefore proves it did not run. Registration-count proofs (clause 13) stay native, using `profile_invocation::observe().registrations`. | Engineering, following XD3. |
| D7-6 | Choosing the request codec for embedded load after cutover | Always v1; choose by the ledger format opened in the prelude | Choose by the opened format. Format1 uses v1. Format2 uses v2 with the embedded selector. This needs no new public option. | Contract decides (X-STORE format table rows 1 and 3). |
| D7-7 | Mapping errors to responses | Ad hoc per handler; one adapter | One adapter in `src/cmd/planar/response.*`. `registration_error` maps to the closed table. `extension_preparation_failure::usage` is exit 2, `protocol` is `internal` exit 4, and `resource` is exit 3. `profile_preparation_error.semantic` maps to the existing typed E with exit 3. Ledger `format_error` keeps the existing mapping. | Contract decides the codes (X-CLI after step 8; X-LIMIT). Placement is engineering. |
| D7-8 | Which X07 stages charge the factory budget owner | Everything after factory creation; only VM and capture work; nothing new | Charge late-slot admission, role capture, history decode into the VM and projection transport to the owner, through the sink that X06-D-C01 lands. The following keep their own bounds, checked before allocation, and are recorded as inherited: CLI11 parser objects, the cwd and environment snapshot, request framing, the early read-only replay, selected-help rendering (1 MiB) and final response encoding (16 MiB). | **Operator question Q2.** The X-LIMIT preamble and X06-D B1 make this the next likely B1-style finding. |
| D7-9 | Testing races and failures after commit through the CLI | Product hooks (forbidden); native-only proof; generated CLI variant binaries with compile-time pause or fault points | Generated CLI variants, as in the existing `planar_load_cli_probe` pattern, for failures after commit and for deterministic receipt races. One real two-process race for "one CAS winner". Native tests remain the main evidence for lock-internal ordering. | **Operator question Q3.** The X07-B row asks for "realistic scratch CLI suites", while XD3 forbids observability hooks. |
| D7-10 | Where the CLI tests live | `src/cmd/planar/*.t.cpp`; Python suites in `tests/acceptance/` (existing); `tests/native/` | Use Catch2 in `bootstrap.t.cpp` for the parser matrix (no process spawn) and new Python suites in `tests/acceptance/` for process-level behaviour. Split the suites so each stays well under its 120 s or 240 s CTest timeout. The X07-B row's "`tests/native/` realistic scratch CLI suites" is read as evidence placement. | Engineering. My reading of that row is uncertain; search confirms it. |
| D7-11 | Existing tests that pin "profile owner invocation refuses" or "public init creates format1" | Edit the assertions; add new tests | Keep every name. Change only the assertions the cutover mandates, and list each with its clause (XD5). | Decided by XD5. Must be in the brief. |
| D7-12 | Audit that no product path creates a fresh profile VM | Reviewer reading only; a source-scan test | A source-scan Catch2 test in the X06-C style: under `src/cmd/`, no call to `extension_invocation::create` or `command_vm::create(` with a profile argument. Pair it with the creation-site audit table in the record. Threat model XD1. | Engineering, following XD1. |
| D7-13 | Order of the two waves | Parser first, or services first | Wave A covers the envelope, probes, final parser, help, D7-1 factories and the error adapter, with tests for clauses 1 to 13 and 24 to 28. Wave B covers the extension handler module, load and replay wiring, races, failures after commit and full-profile workflows (clauses 14 to 23 and 29 to 30). Wave B's test coders start only after wave A's stubs compile. | Engineering. Matches the amendment's two waves. |

### Risks of late discovery

- **D7-1 is the biggest risk.** The architecture explicitly leaves other shipped families
  unspecified, and X01 accepts with-profile render, init, history and export. If the coder improvises
  a generic path, or quietly refuses these families, the reviewer will flag it. Pre-empt it with Q1
  and the addendum.
- **The command contract requires a shipped-handler substitution test.**
  ([Bootstrap section](command-contract.md#bootstrap-errors-and-retained-lifetime): "Tests must replace
  an embedded handler in a test bundle and prove its semantic result is actually used".) Under a
  profile, load and render run retained phases rather than the shipped handler, so that test does not
  carry over. The record must say why and name the substitute evidence: the retained phase callables
  and the custom-shape effect on W and on the render output.
- **Budget narrowing (clause 30).** It is likely the reviewer will look for the sink in the late
  admission and role capture paths.
- **The embedded load codec after cutover (clause 25).** This is easy to miss, because the existing
  load suites keep passing on format2 ledgers unless the request record codec is asserted.
- **The X01 matrix run only against the fixture.** The reviewer will ask for production evidence for
  each row.
- **Help.** Do not change root help. Selected-help escaping and sorting need exact byte tests.
- **Ownership creep.** The new envelope module, the factories in the runtime and the response adapter
  must all be owned in the dispatch. The runtime and `ledger/acceptance*` trigger the Sonnet coder
  escalation ([orchestration](orchestration.md#roles)).
- **Size.** This is the largest remaining round. If the frozen candidate is too large to review
  well, the roadmap rule says split it before coding. Q4 asks whether to allow a docs-only addendum
  first.

### Dependencies

- **X06-D-C01 must be approved** before X07 starts. X07 needs three things from it: the sink API
  shape (how the owner is passed to `open_known_load` and `open_opaque` and onwards); whether the
  extension path (`into_extension`, `admit`, role capture, `prepare_history`, projection) already
  charges the sink, which C01's table does not show beyond T14's module reads; and the final status of
  the two halt candidates in the C01 record (see D-B below).
- X06-C with C01 to C04 provides the exported `complete_acceptance(prepared_profile)` overload and
  the seam threat model. X05-C provides the internal read and projection services. X03 provides the
  format2 initializer `initialize_new_v2` (already in `src/engine/ledger/format.cppm`).
- If Q1 is answered, the D7-1 factory needs a short architecture addendum, reviewed before code.

D-B, the C01 halt candidates. These matter only if C01 escalates. My reading of the contracts:

- (a) The registration-to-invocation instruction transition is contract-mandated. Registration has
  its own 10 million ceiling, and invocation "starts one fresh cumulative 100-million-instruction
  execution budget" ([command contract, Bootstrap section](command-contract.md#bootstrap-errors-and-retained-lifetime);
  X-LIMIT rows). That one transition is not a forbidden reset.
- (b) X-LIMIT's invocation row says "512 MiB live Lua", so composing declaration-VM Lua memory as
  live acquire and release matches the contract's word "live". Instructions and native bytes stay
  cumulative.

These are interpretations, not decisions. If C01 halts, they become operator questions.

### Suggested round shape

- **Before code:** a docs-only addendum (recommended in Q4) that records the D7-1 factory design and
  the D7-3 mapping table, reviewed records-only. This should cost about one short run and remove the
  two most likely request-changes causes.
- **Seed rows for the invariant table** (test-first, red expected unless noted):
  - **Parser and help:**
    - X01 envelope rows 1 to 44 and opaque rows 1 to 12 through production `invoke`, asserting exit,
      precedence and profile access.
    - Selected help bytes in both orders, with escaping and sorting.
    - Root `--help` bytes unchanged (a guard, not red).
  - **Admission and precedence:**
    - For each shipped family: an overlap between a known slot and the trusted root refuses
      `unsafe_path` before external Lua (D7-5 marker).
    - A hard link between the module and the workspace declaration refuses before registration.
    - Opaque precedence chain: broken profile, then unknown command; final syntax, then overlap; then
      the handler, using a handler that skips the read.
    - One registration per invocation.
  - **Options and responses:** the `history_id` matrix for three names, the decimal and limit
    vectors, registration misuse with exit 4, and the projection omission vector through the CLI.
  - **Replay and format:**
    - Early replay with every input deleted and the head advanced.
    - The conflict matrix.
    - Format1 profile load refuses `unsupported_ledger_version` even with the profile deleted.
    - Init writes format2 (`user_version` 2) with response bytes unchanged.
    - Embedded load on format2 writes a codec2 embedded request.
  - **Workflow:** custom shape load, then render, then a custom verb projection, then history and
    export, then deletion of the profile, then ordinary history and export.
  - **Races and faults (per Q3):** equal replay before a stale CAS, one CAS winner, receipt races,
    and failures after commit returning the fallbacks.
  - **Guards:** a source scan that `src/cmd` contains no fresh profile VM creation, and no output
    during registration.
- **Gate additions:**
  - The full Debug and RelWithDebInfo runs (XD6), including every acceptance suite.
  - A Catch2 X01 matrix test.
  - A source-scan test.
  - #531 timing recorded.
  - Optionally, the non-gating Linux pre-flight from Q6.
- **Fit:** even with these decisions made up front, X07 is unlikely to pass in one review. Plan for
  one addendum run plus one or two code runs, and keep the scope fixed.

## X08: mandatory full external-profile acceptance gate, including Linux

### Scope

X08 delivers verification only, under the
[X08 row](external-profile-implementation.md#source-seams-and-bounded-implementation-rounds) and the
2026-10-25 amendment that moved the actual Linux gate here. It covers new native, CLI and adversarial
tests, fixtures and frozen evidence for every requirement E01 to E14; full serialized macOS Debug
and RelWithDebInfo gates; the actual Linux gate; and the documentation and testing references.
Approval closes the mandatory external-profile gate and lets RA01 start (E14;
[v5 alignment](v5-alignment.md#ordered-delivery-and-decision-gates) step 3).

X08 must not:

- Change product behaviour. Product fixes "require fresh corrective round"; see Q6 for Linux-only
  fixes.
- Change contracts.
- Create installers or distributions.
- Claim execution coverage. A08 through the external gate is the only matrix row it advances.

### Acceptance clauses

1. Every requirement E01 to E14 in the
   [requirement table](external-profile-implementation.md#requirement-to-evidence-obligations) maps
   to the actual test names and raw outputs that prove its X08 portion, through the public binary
   wherever the row names X07 or X08.
   **Narrowing risk:** a "map" that cites the native tests of earlier rounds for rows whose proof
   column names X07 or X08 (E02, E07, E08, E09, E10, E12, E13, E14) does not close them. Each needs
   at least one public-CLI test in X08 or X07.
2. Every row of the architecture's
   [adversarial matrix](profile-preparation-architecture.md#adversarial-acceptance-matrix-for-following-code-rounds)
   is mapped to its evidence, or recorded as satisfied by an approved earlier round with a citation.
3. Historical reads and export work after the profile is removed and after the current trusted
   runtime is removed (E12; X08 row).
   **Narrowing risk:** deleting only the profile files does not remove "the current trusted runtime".
   Full compliance reads and exports a ledger written by the normal binary using a binary whose
   embedded stdlib differs. See D8-2.
4. Old fixtures, goldens and W vectors stay byte-exact with and without an unselected profile (E07;
   X-SHAPE).
5. Full serialized macOS Debug and RelWithDebInfo runs: nonzero counts, zero unexplained skips, the
   complete current inventory preserved (not "405"), lint and Doxygen, no-work builds, and relevant
   source and variant audits (X08 row; XD6).
6. The actual Linux gate (amendment of 2026-10-25). **Narrowing risk:** "builds on Linux" or "unit
   tests only" is not the gate. Full compliance is configure, build, full CTest with zero failures
   and skips, the acceptance suites, and the no-work build, in the configurations Q5 fixes.
7. The A08 external portion is recorded in [acceptance-matrix.md](acceptance-matrix.md), and the
   limits are restated: no RSS guarantee, no hostile writer, and the X-CAP last paragraph.
8. The reference docs (`docs/testing.md`, `docs/architecture.md`, `docs/cli-reference.md`) describe
   the gate and the Linux status.

### Design decisions implied

| ID | Choice | Options | Recommendation | Status |
| --- | --- | --- | --- | --- |
| D8-1 | Evidence layout | One report; per-requirement files | A machine-readable `requirements.json` mapping E01 to E14 and the architecture matrix rows to test names, raw log files and configuration. `REPORT.md` and `README.md` follow P10. | Engineering |
| D8-2 | How to prove reads without the trusted runtime | Delete files only; a generated variant binary with altered embedded stdlib bytes; a native reader without the runtime | A generated variant whose embedded shipped Lua differs (for example a changed comment in one module). It reads and exports a ledger written by the normal binary, with the result compared byte for byte. A variant whose stdlib would fail if executed proves that no Lua runs. | Engineering (E12 wording decides the "what"). |
| D8-3 | Linux environment | Docker arm64 on this Mac; a real Linux host; x86_64 under emulation | See Q5 | **Operator question Q5** |
| D8-4 | Linux source and build placement | Bind-mount the shared checkout; copy the tree into the container filesystem | Copy the tree into the container filesystem, with the build directory on the container's own filesystem. A bind mount would expose macOS APFS semantics (case-insensitivity, inode behaviour) through the VM file share, which would invalidate the alias, case and hard-link tests. It could also write into the shared checkout's `build/` while other workflows use it. | Engineering, strongly recommended. Uncertainty: I have not measured the file-share semantics. |
| D8-5 | Docker disk use | Unmanaged; pruned | Build without a persistent tag, prune the builder cache after each run, never remove other images, and record free disk space before and after. | Engineering (prior operator practice). |
| D8-6 | Linux-specific expectations | Assume parity; audit the branches | Search lists every `__APPLE__`/`__linux__` branch, including `capture/admission.cpp` (Linux uses `O_PATH`), `capture/native.cpp`, `export/source_export.cpp` (`renameat2` with `RENAME_NOREPLACE`) and the `#ifdef __APPLE__` tests in `acceptance/source_capture.t.cpp` and `runtime/runtime.t.cpp`. Those Linux branches have never run. Each branch gets a row naming the Linux test that executes it. | Engineering. These must be found before X08 starts. |
| D8-7 | Lint on Linux | Run both; macOS only | Run the formatting and Doxygen lint once, on macOS. It is platform-independent, given the same pinned tools. Linux runs configure, build, full CTest and the no-work build. | Recommendation inside Q5 |

### Risks of late discovery

- **Linux failures turn up only in X08.** Linux code paths such as `O_PATH`, `renameat2`, error
  number mapping and `/tmp` filesystems have never executed. The X08 rule then forces a corrective
  round for each product fix. Q6 pre-empts this with an early non-gating Linux probe.
- **Timing.** #531 takes about 15.5 s of 30 s on macOS Debug. A Linux VM, or emulation, could time
  it out. See Q7.
- **No container recipe exists.** There is no Dockerfile in the tree; the old CI and Docker workflows
  were removed (see [testing](../testing.md)). Writing the pinned Linux toolchain recipe (LLVM 23
  with modules-enabled libc++, CMake 4.3 or later, Ninja, Python, Doxygen) is real work. If the base
  image or LLVM version is not pinned, the reviewer will flag it. Committed vendor sources keep
  configure offline, but building the image needs network access. Record that.
- **E-row mapping that leans on native tests** where the row demands CLI evidence (clause 1).
- **A stale "405" inventory figure** carried into the brief from the implementation path.

### Dependencies

- X07 approved, with the frozen public binary.
- An answer to Q5. Q6 and Q7 are ideally answered before X07 ends.
- No dependency on X06-D beyond what X07 carries.

### Suggested round shape

- **Seed rows:**
  - One row per E01 to E14 X08 portion.
  - The E12 runtime-removal variant.
  - Byte stability of old goldens and W with an unselected profile.
  - One Linux row for each platform branch in D8-6.
  - G rows for the four gate runs (macOS Debug, macOS RelWithDebInfo, Linux Debug, Linux
    RelWithDebInfo, if Q5 says both).
- **Test-first:** mostly guards that are not red, since the product already exists. New tests that
  fail reveal product defects, which go to a corrective round, not to an in-round fix. Q6 asks
  whether to relax this for Linux-only fixes.
- **Gate additions:** the Linux container gate under `scripts/`, which CLAUDE.md names as the place
  for generic gate helpers, with the image digest and toolchain versions recorded in the evidence.
- **Fit:** one round for macOS verification is realistic. Linux probably needs one corrective run
  unless the Q6 pre-flight runs during X07.

## RA01: authoring and driver prompts

### Scope

The [roadmap RA01](roadmap.md#ra01--authoring-and-driver-prompts) section and
[v5 open items 6 and 7](planar-design-spec-v5.md#open-items) ask for:

- Deterministic spec, critique and job-ingest prompts.
- Rejection prompts that name the field path while keeping the raw typed diagnostics and refusal
  exits.
- Coexistence between init scaffolding and the existing empty-ledger init, never overwriting user
  files.
- `observe --from DIR` assembly of observations, invoked by the driver.
- An initial reference-orchestrator graph and shape projection that labels available and future
  commands accurately.
- Local acceptance rows A35 and A36.

The roadmap says "Specify then implement", so RA01 must start with a reviewed contract round.

RA01 must not include:

- A watcher, autonomous scanning, model calls or checkout management.
- Execution state, claims, status or check commands (R15 to R18).
- Orchestrator actions beyond labels for the commands that exist.

### Acceptance clauses (for the RA01 contract round, then implementation)

1. A35: the same declarations and registry give byte-identical prompts across fresh processes
   ([acceptance map](acceptance-matrix.md), row A35).
2. A36: a rejected job's diagnostic prompt names the failing field path. The raw typed error and the
   exit code are kept (row A36).
3. Templates carry the exact v1 schemas, the workspace's allow-listed check IDs and the shape catalog
   (v5 open item 6).
   **Narrowing risk:** with a profile, "the shape catalog" includes the external shapes of the sealed
   catalog (see Q12).
4. Init scaffolding never overwrites user files, and the existing explicit empty-ledger init stays
   available ([ledger contract command surface](ledger-contract.md#proposed-command-surface);
   [command contract workspace section](command-contract.md#workspace-selector-and-environment)).
5. Observe reads only the explicitly named directory, when the driver invokes it. Its schema, file
   limits and trust and extension policy are specified (roadmap RA01; v5 open item 7).
6. The orchestrator projection is a pure Lua projection over graph and shape data. Unavailable
   commands are visibly unavailable (roadmap RA01; [v5 bootstrap section](planar-design-spec-v5.md#bootstrap-and-external-driver)).
7. New verbs follow the catalog and native-family rules of the
   [command contract](command-contract.md#catalog-and-structural-abi). Native ABI v1 "admits exactly
   the following pairings", so a new shipped verb needs a contract amendment.

### Design decisions implied

| ID | Choice | Recommendation | Status |
| --- | --- | --- | --- |
| DR-1 | How the init prompt coexists with the exact init response | See Q10 | **Operator question.** v5 (authority 3) says init emits the prompt. The ledger and X-CLI contracts (authority 2) fix the init response bytes. |
| DR-2 | Scaffold writes | Emit prompts only in RA01, and defer file scaffolding. If the operator wants scaffolding, reuse the export no-replace staging and publication mechanics. | Part of Q10 |
| DR-3 | Where observe lives | A shipped native family with a bounded, no-follow, explicit-directory read service, read-only, writing observations JSON to stdout. It is not an external verb, because external verbs are read/projection-only and forbid directory listing ([X-PROJ](external-profile-contract.md#x-proj--pure-projections-and-native-read-capabilities)). | **Operator question Q11** (it needs a new native capability) |
| DR-4 | The orchestrator projection's surface | A render-contract amendment adding a reserved `orchestrator` projection whose input is graph-only, never worker facets (v5 bootstrap section) | **Operator question Q12** |
| DR-5 | Prompt templates | Trusted Lua under `src/stdlib/planar/`, rendered with the existing presentation helpers. Natively, the Glaze-encoded diagnostic is passed through unchanged. | Engineering (CLAUDE.md product shape) |

### Risks of late discovery

- The init response conflict (DR-1) and the new native capability for observe (DR-3) are both
  contract amendments. If they are discovered in the implementation round, they halt it.
- Under the threat model, prompt text is untrusted data passed to a harness, and determinism tests
  need fresh processes (A01 discipline).
- External profile shapes in prompts need with-profile variants of the authoring verbs. That widens
  X-CLI's family list, which is an amendment.

### Dependencies

- X08 approved (E14).
- The answers to Q10, Q11 and Q12.

### Suggested round shape

- **RA01-A:** a docs-only contract (prompts, observe, orchestrator projection, amendments to the
  command catalog and render contract), reviewed.
- **RA01-B:** prompts and rejection wrappers, with A35 and A36 tests.
- **RA01-C:** observe.
- **RA01-D:** the orchestrator projection.
- Seed rows for each code round: fresh-process byte equality, the field path in the rejection prompt
  with raw error equality, no-overwrite refusal, refusal of unsupplied directories and symlinks in
  observe, and future commands labelled as unavailable.
- **Fit:** four rounds, probably 5 to 7 runs in total.

## R15 to R19: observed-state execution

Each of these rounds "begins with independently reviewed exact schema/protocol"
([roadmap Phase 2](roadmap.md#phase-2--ledger-foundation-then-observed-state-execution)). The
detailed clauses will come from those contracts. This section records only the decisions that
should be settled before each contract round, so the contracts are not reopened later.

| Step | Scope (roadmap) | Matrix rows | Decisions to settle first |
| --- | --- | --- | --- |
| R15 | Accepted-revision reads, draft status, active job, generations, retirement, claim and packet atomicity, fencing, takeover; minimal A34 provenance | A09, A10, A20, A22, A23 dispatch, A26 reconciliation, A33, A34 minimal | Storage format for execution state (Q13). Claim surface versus "render is database-free" (Q14). A34 record fields. Role reconciliation in the same transaction as acceptance (A26). |
| R16 | Same-run acknowledgment, `--since`, drift, checkpoints, interrupted and released runs, retained recovery | A11, A19, A29, plus recovery parts of A12, A20 and A23 | Recovery packet retention bounds. How drift is reported against the accepted revision. |
| R17 | Trusted check adapters, reservations, before and after observation, boundary attribution, evidence applicability, facets, prerequisite rederivation | A05, A12, A15 to A18, plus completion parts of A23 and A28 | Subprocess capability and isolation level (Q15). Timeouts and output bounds. Linux and macOS process behaviour. |
| R18 | Exact-target review, atomic verdict and repair, `max_rounds`, durable blockers | A06, A13, A14, A16, plus approval parts of A22, A33 and A24 | A24 policy (Q16) |
| R19 | Single-root external-driver demonstration under the full matrix | All applicable rows | The subject spec and driver (Q17). A 24-hour run is not the gate. |

Risks of late discovery common to R15 to R19:

- The v5 command names (`render --claim`, `status`, `check`, `release`, `explain`) clash with the
  CLAUDE.md product rule "render is database-free". That has to be settled before R15's contract.
- New tables require a new ledger format. The
  [ledger contract](ledger-contract.md#initialization-failure-safety-export-and-format-evolution)
  requires a separately reviewed offline tool for any new-ledger version migration. The format1 and
  format2 ledgers created by X07 will therefore not gain execution state unless that tool is built.
- Process execution (R17) is the first capability that runs untrusted code. Its isolation level must
  be an explicit operator choice, not a coder default.
- Each round grows the orchestrator projection "only as corresponding commands/state become real".
  The reviewer will check its labels.

Dependencies: RA01 approved, and R14 complete (it is). Each round depends on the previous one.

## Operator questions

Each question lists options, my recommendation and the governing source. Q1 to Q4 block X07; Q5 to
Q7 block X08; Q8 and Q9 are conditional; Q10 to Q12 block RA01; Q13 to Q17 block the R15 to R19
contracts; Q18 is process.

1. **Shipped families under `with-profile` (X07, D7-1).** The architecture opens only known load.
   - A: one finite per-family factory in the runtime.
   - B: one closed factory over the five-family option variant. Render runs `render_profile`; init,
     history and export run their sealed shipped callables in the profile VM after profile admission.
   - C: refuse shipped families other than load under a profile.

   Recommend B, written as a short reviewed architecture addendum before X07 code. C contradicts the
   accepted X01 rows. Source: [architecture constructor protocol](profile-preparation-architecture.md#constructor-parse-and-consumer-protocol)
   ("Other known shipped families require their own finite admission path") and
   [X-CLI](external-profile-contract.md#x-cli--envelope-and-admission-order) steps 6 to 8.

2. **Budget composition boundary for X07 stages (D7-8).**
   - A: compose late-slot admission, role capture, history decode into the VM and projection
     transport into the factory owner, and record CLI11 parser objects, the cwd and environment
     snapshot, request framing, the early replay read, help rendering (1 MiB) and response encoding
     (16 MiB) as inherited own-bound stages checked before allocation.
   - B: compose everything, including the CLI11 and response buffers.
   - C: compose only what X06-D-C01 already composes.

   Recommend A. Source: the [X-LIMIT preamble](external-profile-contract.md#x-limit--cumulative-ceilings-and-errors),
   the [architecture accounting paragraph](profile-preparation-architecture.md#failure-lifetime-authority-and-resource-accounting)
   and X06-D criterion B1.

3. **How X07-B proves races and failures after commit through the CLI (D7-9).**
   - A: generated CLI variant binaries with compile-time pause and fault points, plus one real
     two-process CAS race.
   - B: native-only proof with CLI composition tests.
   - C: product hooks.

   Recommend A. C is excluded by XD3. Source: the [X07-B row](external-profile-implementation.md#source-seams-and-bounded-implementation-rounds)
   and [orchestration](orchestration.md#round-sequence) step 5.

4. **X07 round shape.**
   - A: keep the amendment (one code round, two waves) and add a docs-only design addendum run
     first, for the Q1 design and the X01 production outcome table, reviewed records-only.
   - B: keep the amendment with no addendum.
   - C: split X07 into two reviewed code rounds, with the parser compiled only into a test variant
     until X07-B.

   Recommend A. Source: the [delivery sequence amendment](profile-preparation-architecture.md#bounded-delivery-sequence-and-independent-gates)
   and the roadmap rule to split before coding if a brief is too large.

5. **Linux gate environment (X08, D8-3 and D8-7).**
   - A: Docker on this Mac, arm64 native, with a committed recipe under `scripts/`, a pinned base
     image digest, a pinned LLVM 23.x with modules-enabled libc++, a pinned CMake 4.3 or later, the
     source copied into the container, and the builder cache pruned after each run. Run full Debug
     and RelWithDebInfo CTest plus the no-work build; lint once on macOS; x86_64 recorded as not
     covered.
   - B: as A, but Debug only.
   - C: a separate real Linux host.
   - D: also x86_64 under emulation.

   Recommend A. Source: roadmap "Next work" X08 row, CLAUDE.md ("Support macOS and Linux") and
   [testing](../testing.md), which notes there is no current Linux recipe.

6. **Linux pre-flight and Linux-only fixes.**
   - A: allow a non-gating Linux build and test probe during X07, with evidence only and no claim,
     and allow fixes confined to `__linux__` branches in an X08 corrective round without new search.
   - B: Linux only in X08, with every fix in a full fresh corrective round.

   Recommend A, because Linux code paths (`O_PATH`, `renameat2`) have never executed. Source: the
   X08 row ("product fixes require fresh corrective round") and the 2026-10-25 amendment.

7. **#531 near its timeout** (about 15.5 s of 30 s on macOS Debug).
   - A: before X08, split the test or raise its CTest timeout, recorded as test infrastructure, with
     no assertion change.
   - B: leave it, and accept a Linux timeout risk.

   Recommend A. Source: the X06-D carried limits in the [roadmap](roadmap.md#next-work).

8. **Conditional: X06-D-C01 halt candidate (a)**, if C01 escalates it. Registration has a 10 million
   instruction ceiling, and invocation starts a fresh 100 million cumulative budget.
   - A: treat this one transition as contract-mandated.
   - B: carry registration instructions into the invocation counter.

   Recommend A. Source: [command contract, Bootstrap section](command-contract.md#bootstrap-errors-and-retained-lifetime)
   and the X-LIMIT Registration and Invocation rows.

9. **Conditional: X06-D-C01 halt candidate (b).**
   - A: compose declaration-VM Lua memory as live acquire and release against the 512 MiB live
     ceiling.
   - B: compose it cumulatively.

   Recommend A, because X-LIMIT says "live". Source: X-LIMIT, Invocation and Declarations rows.

10. **Init prompts and scaffolding (RA01, DR-1 and DR-2).**
    - A: add a prompt to the init response, as a contract amendment that changes init bytes.
    - B: keep init exact; add read-only prompt verbs for spec and critique, with `ingest` for the job
      prompt, and no file scaffolding in RA01.
    - C: as B, plus no-overwrite scaffolding through export-style staging.

    Recommend B. Source: [v5 open item 6](planar-design-spec-v5.md#open-items) against the
    [ledger command surface](ledger-contract.md#proposed-command-surface) and the X-CLI "result bytes
    stay unchanged" sentence.

11. **Observe (RA01, DR-3).**
    - A: a shipped native family with an explicit-directory, bounded, no-follow read service, output
      to stdout only, as a command-contract amendment.
    - B: an external profile verb, which is impossible under X-PROJ because there is no directory
      listing.

    Recommend A. Source: [v5 open item 7](planar-design-spec-v5.md#open-items) and the
    [command contract catalog](command-contract.md#catalog-and-structural-abi).

12. **Orchestrator projection and profile shapes in prompts (RA01, DR-4).**
    - A: a render-contract amendment for a reserved graph-only `orchestrator` projection, with
      with-profile variants of the authoring verbs so external shapes appear in the shape catalog.
    - B: embedded shapes only in RA01.

    Recommend A for the projection, and B for prompt shapes in the first cut if the operator wants
    RA01 smaller. Source: [v5 bootstrap section](planar-design-spec-v5.md#bootstrap-and-external-driver)
    and the roadmap RA01 section.

13. **Execution-state storage (R15).**
    - A: a new format3 for new ledgers only, with execution commands refusing format1 and format2
      ledgers explicitly and no conversion tool.
    - B: build the separately reviewed offline conversion tool as well.
    - C: add tables to format2 (forbidden: no ALTER on open, exact schemas).

    Recommend A, with B deferred. Source: [ledger contract format evolution](ledger-contract.md#initialization-failure-safety-export-and-format-evolution)
    and the X-STORE last paragraph.

14. **Claim surface (R15).**
    - A: separate `claim`, `status` and `release` verbs, keeping render database-free.
    - B: v5's `render --claim`, amending the CLAUDE.md product rule.

    Recommend A. Source: CLAUDE.md product shape ("render is database-free") against the
    [v5 bootstrap section](planar-design-spec-v5.md#bootstrap-and-external-driver).

15. **Check execution isolation (R17).**
    - A: v5's minimum (registry-allow-listed IDs, bounded time, output, cwd and environment, every
      call logged), with no OS sandbox and the limits recorded.
    - B: an OS isolation adapter in R17.

    Recommend A. Source: [v5 engine surface](planar-design-spec-v5.md#engine-surface-and-sandbox) and
    the "Optional stronger execution" section.

16. **A24 approval policy (R18).**
    - A: explicit unsupported-policy refusal only.
    - B: an exact-target external assertion policy.

    Recommend A first. Source: [acceptance map](acceptance-matrix.md) row A24.

17. **R19 subject.**
    - A: the quota fixture in both groupings plus one real spec the operator names.
    - B: the quota fixture only.

    Recommend A, since v5 asks for "a spec previously run on old Planar". Source:
    [v5 cut 2 table](planar-design-spec-v5.md#cut-2-observed-state-execution-as-rounds), R19 row.

18. **Process: make this design pass binding.**
    - A: the coordinator cites this document in each dispatch. Search must output a decisions table
      that marks each choice as contract-decided (with citation), decided here, or new. Any new
      product or authority choice halts before coding.
    - B: advisory only.

    Recommend A, because it targets exactly the late-discovery pattern. Source:
    [orchestration](orchestration.md#round-sequence) step 2 (operator-owned process).

## Operator decisions (2026-10-27)

The operator accepted the recommended option for every question above, Q1 to Q18, on 2026-10-27. These
answers are explicit operator decisions and rank above the documents they settle until a scoped,
reviewed amendment changes them.

| Question | Decision |
| --- | --- |
| Q1 | B: one closed factory over the five shipped families, written as a reviewed architecture addendum before X07 code |
| Q2 | A: profile stages compose into the factory budget owner; CLI11 parser objects, cwd and environment snapshot, request framing, early replay read, help (1 MiB) and response encoding (16 MiB) are inherited own-bound stages checked before allocation |
| Q3 | A: generated CLI variant binaries with compile-time pause and fault points, plus one real two-process CAS race; no product hooks |
| Q4 | A: a docs-only design addendum round first, then X07 as one code round with two waves |
| Q5 | A: Docker on this Mac (arm64 native), committed recipe under `scripts/`, pinned base image digest, LLVM and CMake, source copied into the container, builder cache pruned; full Debug and RelWithDebInfo CTest plus the no-work build; x86_64 recorded as not covered |
| Q6 | A: a non-gating Linux probe during X07 (evidence only); fixes confined to `__linux__` branches may land in an X08 corrective without new search |
| Q7 | A: before X08, split #531 or raise its CTest timeout as test infrastructure, with no assertion change |
| Q8 | A (if raised): the fresh invocation budget after registration is contract-mandated |
| Q9 | A (if raised): declaration-VM Lua memory composes as live acquire and release against the 512 MiB live ceiling |
| Q10 | B: init stays exact; read-only prompt verbs; no file scaffolding in RA01 |
| Q11 | A: observe as a shipped native family with a bounded, no-follow, explicit-directory read service, stdout only, via a command-contract amendment |
| Q12 | A for the reserved graph-only `orchestrator` projection (render-contract amendment); embedded shapes only in RA01 prompts if RA01 needs to stay small |
| Q13 | A: format3 for new ledgers only; format1 and format2 refused explicitly; conversion tool deferred |
| Q14 | A: separate `claim`, `status` and `release` verbs; render stays database-free |
| Q15 | A: the v5 minimum check isolation, no OS sandbox, limit recorded |
| Q16 | A: explicit unsupported-policy refusal first |
| Q17 | A: the quota fixture in both groupings plus one real spec; the operator names the spec before R19 — **named 2026-11-11 (operator, live): the R19-CMAKE task**; v5's "previously run on old Planar" deviation accepted |
| Q18 | A: binding. Every dispatch cites this document; search outputs a decisions table marking each choice contract-decided (with citation), decided here, or new; any new product or authority choice halts before coding |
