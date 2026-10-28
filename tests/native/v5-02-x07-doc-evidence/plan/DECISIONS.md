# V5-02-X07-DOC decisions table (search output, revision 2, 2026-10-28)

Round: [V5-02-X07-DOC](../../../../docs/vernacular/rounds/V5-02-X07-DOC.md). Docs-only. This revision supersedes
`superseded-1/` (kept unmodified). It applies the 2026-10-28 operator answer Q-INIT A and the eleven plan-review gaps.
Search read: the round record, `remaining-steps-design.md` (X07 section, D7-1 to D7-13, Operator decisions 2026-10-27),
`external-profile-contract.md` (X-CLI, X-CAP, X-REG, X-PROJ, X-LIMIT, X-STORE), `command-contract.md`,
`profile-preparation-architecture.md`, `external-profile-implementation.md`, the envelope spike and the sources named in
each row. Nothing was run; nothing outside `plan/` was written.

Status vocabulary (plan side): **CONTRACT** (contract-decided, with citation), **DESIGN** (decided in
`remaining-steps-design.md` or an engineering selection among existing things; cited), **OPERATOR** (explicit operator
decision, with the question id), **NEW** (not settled; halts). The addendum's Status cell uses the lowercase forms
`contract-decided`, `design`, `operator`, `new`. **No row in this revision is NEW; nothing halts package D1.**

## 0. Reviewer-visible items (read first)

| Item | Kind | What search found | Resolution |
| --- | --- | --- | --- |
| **Q-INIT** | OPERATOR Q-INIT A (2026-10-28), applied | The D7-2 reading that an init ledger target gets a "not inside the trusted root" refusal is not in the contract: X-CAP (`external-profile-contract.md:159-175`) lists root overlap only for workspace declaration root, supplied render source root, supplied base root and the observation file; ledger gets only a selected-file alias refusal, export gets target-tree overlap, and "history/init have no inferred workspace root" (`:174-175`). X-STORE (`:647-650`) says init creates format2 at a new explicit target with the old publication and no-overwrite rules. | Operator chose option A (contract-literal): the init ledger target gets only the selected-file alias check against the profile and its modules; **no containment refusal inside the trusted root and no contract amendment**. The addendum init admission row says alias-only and "no containment refusal"; decisions row D7-2 (Status `operator`) and addendum row A-07 cite Q-INIT A and the X-CAP anchor. The coordinator also records the decision in the round record (D1 does not own the record). |
| **Q-WAVE** | DESIGN conflict, engineering, non-blocking | D7-13 puts clause 24 to 28 tests in wave A; the round record objective part 1 and the implementation-path X07-B row (`external-profile-implementation.md:55`) put format2 init and the request codec by opened format (clauses 24 to 26) in X07-B. Clauses 27 and 28 are factory admission (wave A under both readings). | Addendum follows the round record for 24 to 26 (wave B), keeps 27 and 28 in wave A, states the reconciliation (row A-08). Wave order, not a product or authority choice. |
| **F-CLASS** | Finding | By X-CLI step 3 (`:76-81`) E06 (verb-as-value), E11 (extra prefix option) and E19 (prefix terminator) are probe refusals before profile access in production; the spike labelled all three `final:*` because its fixture always ran the final parser. | Addendum section 2.4 calls out E06 (required) and E11, E19 with the X-CLI step 3 citation. |
| **F-VALIDATOR** | Finding | E08, E09, E21 (selector value starting `--`: prefix usage, never reaches the probe) and E14, E16, E17, E43 (extension text value starting `--` or empty: final usage after capture and registration, X-CLI 7, X-REG text rule `:240-243`) differ from their spike labels because the spike fixture omitted the production validators. | Translate, never copy, spike labels; section 2.4 lists them. |
| **F-NRM** | Finding | E36 and E37 use bare workspace name `w`; with `PLANAR_WORKSPACE_DIR` unset the current C rule (`command-contract.md:298-300`) gives usage exit 2 at X-CLI step 4, before profile access and before the load early lookup. | Class NRM. Variants (environment set to an absolute scratch directory) are non-counted notes. |
| **F-OPAQUE-REACH** | Finding | Opaque-probe rows **O05** (`--help`) and **O12** (`[]`) cannot reach the opaque probe through the envelope: the envelope consumes `--help` as its counted flag (selected help) and an empty tail is a prefix usage. O06 (`--` leading terminator) does reach the probe, which refuses it (same argv as E19). | Table states "reaches probe" per row (O05 no, O12 no, all other O rows yes). |
| **F-KSA** (gap 3) | Finding, derived from X-CAP | E02 (`history --ledger missing`), E35 (`export ... --ledger a ...`, ledger missing) and O07 (`history --ledger a`, ledger missing) were classed HND (handler returns `ledger_not_found`). X-CLI step 6 (`:99-105`) and X-CAP's known-shipped-tail phase (`:179-182`: "check every supplied native root/file/target slot before external Lua evaluation"), together with `:172` "Missing supplied paths retain existing family capture errors" and `:195` "All supplied slots require initial checks", put the missing-ledger refusal at step-6 admission, after profile capture and **before registration**. It is observable through the D7-5 marker: a profile module with unbounded chunk-level work gives `resource_limit` only if registration ran; for these rows the result is `ledger_not_found` exit 3, so registration did not run. | New class **KSA** (known-slot admission refusal, X-CLI 6, exit 3 or 4 by the family's existing capture error, profile accessed yes, registration not run). E02, E35, O07 become KSA exit 3; I4, I5, I6 reference data updated. E34 (init at a new target `a`) stays HND exit 0: the alias check is vacuous for a target that does not exist. Derivation recorded in addendum 2.4 (row A-09). |
| **F-FIXTURE** (gap 2) | Finding, operator-requested fix | The earlier Appendix A resolved profile, ledger, export target and workspace paths against one cwd, making cwd the trusted profile root; under X-CAP `:169-171` E35's export target `out` would then be refused `unsafe_path`, contradicting its `ledger_not_found` note, and E34 would depend on the Q-INIT reading. | Profile in a sibling trusted directory: scratch tree `trusted/` (profile `p.json`, `q.json`, modules) and invocation cwd `work/`. Every selector token is `../trusted/p.json` (or `../trusted/q.json`). All other path names (`a`, `out`, `w`, `r`, `src`, `missing`) are relative to `work/` and name nonexistent paths. Applied to E01 to E44 and to the opaque prefix. Production argv is stored in `reference.json` and compared by I6. Row A-10. |
| **F-USAGE** (gap 4) | Finding | The runtime string `Invalid extension invocation` (`runtime.cpp:4016`) is in no contract and the public CLI never printed it; only a native test asserts it (`external_registration.t.cpp:2763`). Printing it from X07 would add a public message, which the design's X07 must-not list ("No new public error code or message") and round-record acceptance 3 forbid. D7-7 decides only codes and exit codes, not messages. | Every exit-2 usage, including `extension_preparation_failure::usage`, maps to the existing public `detail::usage` bytes `Expected command options or --help; see --help` (`response.cpp:175`). The runtime message is never printed and is absent from Appendix B. Row A-04 (design, not D7-7). If a coder insists on the runtime message it becomes an operator question and D1 halts. |
| **F-CL5** | Finding for section 4 | CL-5's roadmap basis "profile VMs refuse invoke at 4844-4845" (`runtime.cpp:4844-4845`) no longer holds once Q1 B opens an invoke path on profile VMs; CL-1 and CL-4 become more reachable through render and projection. | Addendum records the facts; it does not reclassify or repair any limit. Whether the roadmap reclassifies CL-5 is the coordinator's call. |
| **F-ANCHOR** (gap 8) | Finding | No heading has the slug `x-cap`; the real slug of `## X-CAP — Strict capture and trust` is `x-cap--strict-capture-and-trust` (em dash removed, double hyphen kept). All other required anchors were checked against heading text the same way (Appendix D of `INVARIANTS.md`). | I17 and `reference.json` use the real slugs. |
| **D7-10 qualification** | Reading confirmed with correction | `tests/native/` holds gitignored `*-evidence/` folders and `*-support/` harness directories that CMake registers (`src/cmd/planar/CMakeLists.txt:107-111` runs `tests/native/v5-02-c-support/cli.py`); `tests/acceptance/bootstrap_*.py` hold the older per-family CLI suites (`bootstrap_load.py` registered at `:156-158`). | Row D7-10 `design`, confirmed, with the placement split: Catch2 matrix in `bootstrap.t.cpp`, Python process suites in `tests/acceptance`, fixture generators in `tests/native/v5-02-x07-support`, evidence in `tests/native/v5-02-x07-evidence`. No halt. |

## 1. Decisions the round implies

Marks as in the status vocabulary. IDs DC-nn are plan-side; the addendum carries the addendum-choice rows A-01 to A-12
(section 3 of this file maps them).

| ID | Choice | Status | Citation and notes |
| --- | --- | --- | --- |
| DC-01 | Shipped families admitted under `with-profile` by one closed factory over the five-member `command_options` variant (not per-family factories, not refusal) | OPERATOR Q1 B; DESIGN D7-1 | `remaining-steps-design.md` Operator decisions table row Q1; D7-1 row. Option C refused. |
| DC-02 | Factory is a static member family of `profile_invocation` in `planar.engine.runtime` (same-module friendship; no new module, no new exported constructor), one creation site for profile VMs | DESIGN (addendum row A-01); consistent with D7-1 and D7-12 | `profile-preparation-architecture.md:157-161`, `:233-247`. Member names are a "not compiled" sketch. |
| DC-03 | The factory's load alternative routes to `open_known_load` (then `prepare_load`, then exported `complete_acceptance(prepared_profile)`); still one site | OPERATOR Q1 B ("Load keeps ..."); addendum row A-02 | Round record, Governing inputs; `src/engine/ledger/profile_acceptance.cppm:21`. |
| DC-04 | Render under a profile runs `render_profile` phases in the retained VM; init, history and export run their sealed shipped callables in the profile VM after admission with existing native services | OPERATOR Q1 B | `runtime.cppm:281` (`render_profile`), `:310` (`invoke`, refused for profile owners today at `runtime.cpp:4844-4845`). |
| DC-05 | How X-CLI step 8 is satisfied for render and load (retained phase callables) | DESIGN D7-1 ("The record must explain") | Same VM, same captured bundle, native registry reference binding, one dispatch, destroy at terminal result. |
| DC-06 | Substitution evidence under `command-contract.md:117-121` for every family: render and load by retained phase callables and the custom-shape effect on W and render output; **init, history and export by a test-bundle handler replacement driven through the closed factory under with-profile** (test-only embedded bundle with a substituted handler, identical CLI11 parsing, the substituted semantic result observed); ordinary no-profile substitution tests unchanged | DESIGN (addendum row A-11) on a contract requirement | `command-contract.md:117-121` (requirement); `remaining-steps-design.md` Risks of late discovery (retained phase callables and custom shapes). The test-bundle form is the engineering choice recorded so the X07 coder does not decide it alone. |
| DC-07 | Family registry closed at five; a sixth member needs an architecture revision, a new admission row and a reviewed amendment; no generic service factory; no caller-supplied profile argument | OPERATOR Q1 B; DESIGN D7-1 | `runtime.cppm:68` (`command_options`). |
| DC-08 | Replacement architecture sentence at `profile-preparation-architecture.md:230-231` | DESIGN (Q4 A scoped edit) | One line starting `> ` in addendum 1.8, byte-identical in the architecture doc. |
| DC-09 | Known slots per family | CONTRACT (D7-2) | Render: X-CAP `:159-168`; architecture `:213-221`. Load: X-CLI 4 to 6, architecture `:209-221`. History: X-CAP `:169-170`, `:174-175`. Export: X-CAP `:170-171`, X-STORE `:650-655`. Selected help: `:172-175`. |
| DC-10 | **Init ledger target: selected-file alias check against the selected profile and its modules only; no containment refusal inside the trusted root; no amendment** | OPERATOR Q-INIT A (2026-10-28); alias part CONTRACT | X-CAP `external-profile-contract.md:159-175` and X-STORE `:647-650`. Alias is vacuous for a target that does not yet exist, real if the path already names the profile or a module. Section 0 row Q-INIT. |
| DC-11 | Threat model for creation-site audit: trusted in-tree code using exported names; language visibility plus a source scan; adversarial C++ is a recorded limit | DESIGN XD1 | `remaining-steps-design.md` XD1. |
| DC-12 | Creation-site audit (D7-12): Catch2 source-scan test over `src/cmd/` plus the creation-site table | DESIGN D7-12 | Pre-X07 baseline: `bootstrap.cpp:103`, `invocation.cpp:404`; runtime `runtime.cpp:3670`, `3676`, `4969`, `4983`, `5039`, `5095`. |
| DC-13 | Enforcement placement: known shipped tail checks every supplied slot after the early replay miss and profile capture, before registration; opaque tail checks late; selected help checks only the trusted capture | CONTRACT | X-CLI 6, 7 (`:99-118`), X-CAP `:177-192`, architecture `:454`. |
| DC-14 | Ordering clauses (early replay before capture; malformed profile before unknown command; final syntax before late overlap; overlap before handler) each get a row | DESIGN P2; CONTRACT X-CLI 5 to 7 | |
| DC-15 | Production outcome per X01 row: exit, class, accessed flag, clause, production argv | CONTRACT outcomes, DESIGN D7-3 | X-CLI 1 to 8 and exit paragraph (`:122-127`). Reference values in `INVARIANTS.md` Appendix A. |
| DC-16 | Class vocabulary SYN PFX PRB NRM ERL KSA PRF FIN LAT HND HLP: D7-3's seven named classes plus labels NRM (step 4), ERL (step 5, load only), **KSA (step 6 known-slot admission refusal, gap 3)** and HLP (selected help) | DESIGN D7-3 extension; addendum row A-03 | Labels only; they name X-CLI steps 4, 5, 6 and the help paragraph. |
| DC-17 | Spike labels translated, not copied | DESIGN D7-3 | |
| DC-18 | E06, E11, E19 become probe refusals before profile access | CONTRACT X-CLI 3 via D7-3 | F-CLASS. |
| DC-19 | Outcome and diagnostic mapping uses only existing codes and messages, enumerated exactly in Appendix B (G01 to G69) | CONTRACT X-LIMIT `:517-541`, X-CLI exit paragraph; DESIGN D7-7 (codes) | Row A-12 (table granularity: one row per code/message pair). |
| DC-20 | One error adapter in `src/cmd/planar/response.*` | DESIGN D7-7 (placement); addendum row A-05 | |
| DC-21 | **Usage message selection: every exit-2 usage, including `extension_preparation_failure::usage`, prints the existing public `detail::usage` bytes; the runtime `Invalid extension invocation` is never printed** | DESIGN (addendum row A-04, engineering selection); **not D7-7** (D7-7 decides codes and exit codes only) | `response.cpp:175`; X07 must-not list (no new public message); round-record acceptance 3. F-USAGE. |
| DC-22 | Envelope module `src/cmd/planar/envelope.{cppm,cpp}`, imported only by bootstrap | DESIGN D7-4; addendum row A-06 | "Do not broaden ownership ad hoc", `external-profile-implementation.md:58-63`. |
| DC-23 | "External Lua did not run" observed without hooks: unbounded chunk-level profile module gives `resource_limit` if it runs; registration count via `profile_invocation::observe().registrations` | DESIGN D7-5; XD3 | `runtime.cppm:409`. |
| DC-24 | Request codec for embedded load by the format of the ledger opened in the prelude | CONTRACT X-STORE `:639-642`; DESIGN D7-6 | `invocation.cpp:442` builds v1 only today. |
| DC-25 | Public init creates format2 only through `initialize_new_v2`; native fixtures keep explicit format1 | CONTRACT X-CLI (`:50-55`), X-STORE (`:647-650`) | `invocation.cpp:234`; `format.cppm:125-132`. |
| DC-26 | Budget: late-slot admission, role capture, history decode into the VM and projection transport charge the factory owner; CLI11 objects, cwd and environment snapshot, request framing, early replay read, selected help (1 MiB) and response encoding (16 MiB) are inherited own-bound stages | OPERATOR Q2 A; DESIGN D7-8 | |
| DC-27 | Registration to invocation instruction transition and declaration-VM memory composition (background only) | OPERATOR Q8 A, Q9 A | X06-D landed it. |
| DC-28 | Races and faults after commit: generated CLI variant binaries with compile-time pause and fault points, plus one real two-process CAS race; no product hooks | OPERATOR Q3 A; DESIGN D7-9; XD3 | `src/cmd/planar/CMakeLists.txt:95-100,116-158`. |
| DC-29 | Test placement | DESIGN D7-10 | Section 0 row D7-10. |
| DC-30 | Test names preserved; candidate tests whose assertions cutover changes, found by reading only, each "candidate; X07 search confirms" with clause 24, 25, 26 or the profile-invoke refusal | DESIGN D7-11, XD5 | Candidates: `external_registration.t.cpp` around `:177`, `:308`, `:461`, `:477`, `:543`, `:2763` (profile-owner invoke refusal and the usage string); `tests/acceptance/bootstrap_init.py` (public init assuming format1); `tests/acceptance/bootstrap_load.py` and `planar_load_cli_probe` fixtures (v1 request bytes through the public prelude). If reading cannot decide, the entry says so. |
| DC-31 | Wave order and attachment | DESIGN D7-13; Q-WAVE (row A-08) | |
| DC-32 | Wave A attaches: envelope module, CLI11 envelope, both probes, final parser, selected help, error adapter, closed factory with admission (clauses 1 to 13, 27, 28). Wave B attaches: extension handler module, load and early replay wiring, format2 init and request codec by opened format (clauses 24 to 26), history and extension services, races, faults, budget threading (clauses 14 to 23, 29, 30). Wave B test coders start after wave A stubs compile | DESIGN D7-13; round record objective part 1 | The wave-attachment invariant row is **I21** in `INVARIANTS.md`. |
| DC-33 | macOS Debug and RelWithDebInfo only; no Linux claim; non-gating probe allowed | OPERATOR Q5 A, Q6 A; DESIGN XD6 | |
| DC-34 | Carried limits CL-1 to CL-6 and other limits described only, quoted byte-for-byte from the roadmap | OPERATOR option A of 2026-10-27; round record part 4 | Roadmap Next work row; exact substrings in Appendix D of `INVARIANTS.md`. F-CL5. |
| DC-35 | Root `--help` bytes unchanged; selected help format per the X-CLI help paragraph | CONTRACT X-CLI `:129-134` | |
| DC-36 | Q10 (init exact) and Q13 (no format3) are non-goals; Q16 not addressed | OPERATOR Q10 B, Q13 A | |
| DC-37 | Addendum at `docs/vernacular/x07-addendum.md`; pointer edits only in the other two docs | Round record "Owned files" | |
| DC-38 | Architecture doc edits: one pointer, the one sentence at `:230-231`, one line in the bounded delivery sequence near `:488` | Round record "Owned files" | |
| DC-39 | Ownership boundaries and relations (addendum 1.7): the factory owns capture, receipts, VM and budget owner; exposes none of command_vm, mutable catalog, receipts, a generic service factory or a caller-supplied profile argument; no reverse conversion, no mutable `command_vm` extraction, no new exported constructor; `open_opaque` serves selected help and unknown external tails (no upgrade API); `into_extension` adopts opaque owners only and refuses known-load owners | CONTRACT architecture `:181-257`, runtime.cppm class docs at `:455-500` | Covered by I30 and I31. |
| DC-40 | Closed-variant statement (addendum 1.3): `command_options` (`runtime.cppm:68`) members `command_render_options`, `command_init_options`, `command_history_options`, `command_export_options`, `command_load_options`; sixth member needs architecture revision plus new admission row plus reviewed amendment; no generic service factory | OPERATOR Q1 B; DESIGN D7-1 | Covered by I29. |
| DC-41 | Per-family X-CLI step 8 and substitution statements for init, history and export (not only render_profile and retained phases) | DESIGN D7-1; command-contract `:117-121` | Covered by I12 and I28. |

## 2. Design rows D7-1 to D7-13, as they appear in addendum part 3 (one row each, one Status value each)

| Design row | Addendum Status cell | Citation basis |
| --- | --- | --- |
| D7-1 | `operator` | Q1 B; D7-1 |
| D7-2 | `operator` (single value). Render, load, history, export and selected help are contract-decided by X-CAP; the init cell is decided by **Q-INIT A (2026-10-28)**. The row's Choice cell says so; Status carries the single value `operator` because an operator decision is the highest authority involved | `external-profile-contract.md#x-cap--strict-capture-and-trust` (`:159-175`), X-STORE `:647-650`, Q-INIT A |
| D7-3 | `contract-decided` | X-CLI 1 to 8 and exit paragraph; the table requirement is design |
| D7-4 | `design` | D7-4 |
| D7-5 | `design` | D7-5, XD3 |
| D7-6 | `contract-decided` | X-STORE format table rows 1 to 4 |
| D7-7 | `contract-decided` | X-CLI exit paragraph; X-LIMIT (codes and exit codes only; message selection is A-04) |
| D7-8 | `operator` | Q2 A |
| D7-9 | `operator` | Q3 A |
| D7-10 | `design` | section 0 qualification; `tests/native` and `tests/acceptance` |
| D7-11 | `design` | D7-11, XD5 |
| D7-12 | `design` | D7-12, XD1 |
| D7-13 | `design` | D7-13 with the Q-WAVE reconciliation |

Addendum-choice rows (second table in addendum section 3, IDs A-01 to A-12; each marked, none unmarked). Status per
`reference.json` key `decisions`:

| ID | Choice (non-D7) | Status | Citation |
| --- | --- | --- | --- |
| A-01 | Factory is a static member family of `profile_invocation` (DC-02) | `design` | architecture `:157-161`, `:233-247`; D7-1, D7-12 |
| A-02 | Load arm routes to `open_known_load` (DC-03) | `operator` | Q1 B |
| A-03 | Class vocabulary SYN PFX PRB NRM ERL KSA PRF FIN LAT HND HLP, including NRM, ERL, KSA, HLP (DC-16) | `design` | D7-3; X-CLI 4, 5, 6 and help paragraph |
| A-04 | Usage message: public `detail::usage` for every exit-2 usage; runtime `Invalid extension invocation` never printed (DC-21) | `design` | `response.cpp:175`; X07 must-not list; round acceptance 3 |
| A-05 | One error adapter in `src/cmd/planar/response.*` (DC-20) | `design` | D7-7 placement |
| A-06 | Envelope module `src/cmd/planar/envelope.{cppm,cpp}` imported only by bootstrap (DC-22) | `design` | D7-4 |
| A-07 | Init ledger target: alias-only, no containment refusal (Q-INIT, DC-10) | `operator` | Q-INIT A (2026-10-28); X-CAP `#x-cap--strict-capture-and-trust` |
| A-08 | Q-WAVE reconciliation: clauses 24 to 26 in wave B, 27 and 28 in wave A | `design` | D7-13; round record objective part 1; implementation path `:55` |
| A-09 | KSA derivation for E02, E35, O07 (F-KSA) | `contract-decided` | X-CLI 6; X-CAP `:172`, `:179-182`, `:195` |
| A-10 | Fixture layout: trusted sibling directory, cwd `work/`, selector `../trusted/p.json` (F-FIXTURE) | `operator` | Operator instruction of 2026-10-28 to apply the G1 fixture fix; X-CAP `:169-171` |
| A-11 | Substitution evidence per family incl. test-bundle handler replacement for init, history, export (DC-06) | `design` | `command-contract.md:117-121` |
| A-12 | Diagnostic table granularity: one row per existing code/message pair, G01 to G69 (DC-19) | `design` | D7-7; X-LIMIT |

## 3. What must go to the operator before the author runs

Nothing. Q-INIT was answered (option A). Optional, non-blocking, for the coordinator at review: Q-WAVE (A-08), the CL-5
reclassification question (F-CL5), and the A-04 usage-message selection (if a coder wants the runtime message, that
becomes an operator question and D1 halts). The coordinator records the Q-INIT decision in the round record.

## 4. Scope map (file:line)

Files the round may change (record "Owned files"): `docs/vernacular/x07-addendum.md` (new),
`docs/vernacular/profile-preparation-architecture.md` (sentence at `:230-231`, one delivery-sequence line near `:488`, one
pointer), `docs/vernacular/external-profile-implementation.md` (pointer at the X07-A and X07-B rows `:54-55` or the
paragraph at `:58-63`). Coordinator-owned after review: the round record, `docs/vernacular/roadmap.md` opening and Next
work rows, the review file. Untouchable: contracts, `remaining-steps-design.md`, `CLAUDE.md`, `src/`, `tests/` (except the
gitignored evidence folder), `scripts/`, `cmake/`.
