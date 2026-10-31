# X07 design addendum: closed factory, production outcomes and carried limits

Status (2026-10-28): records only. This is the design addendum that the [remaining-steps design](remaining-steps-design.md#x07-public-cutover-one-round-two-waves) requires before any X07 code, under operator decision Q1 B and Q4 A. It is written for round [V5-02-X07-DOC](rounds/V5-02-X07-DOC.md). It changes no contract, source, test, script or build file. Where this addendum and an approved contract differ, the contract wins and this addendum is wrong.

Every choice below is marked as `contract-decided` (with its contract citation), `design` (decided in the remaining-steps design or an engineering selection among existing things, with its citation) or `operator` (an explicit operator decision, with its question). The choices are listed in section 3. None is `new`. No operator question is open. Q-G35, the exit of a profile-preparation semantic error whose code `failed()` maps to 2 or 4 while D7-7 says exit 3, was decided by the operator on 2026-10-28 as option A, map by code (note under the 2.5 table; row A-13; round record).

## 1. Closed factory over the five shipped families

### 1.1 Shape and placement

The closed factory is a static member family of `profile_invocation` in `planar.engine.runtime` (`runtime.cppm`, class block from line 455). It adds no module, no exported constructor and no exported type. It is the single creation site for profile VMs as far as `src/cmd/` can reach (D7-1, D7-12, A-01). The sketch below is design only and is not compiled.

```text
// Not compiled. Design sketch for the class profile_invocation in planar.engine.runtime.
class profile_invocation {
public:
  // Unchanged: open_known_load, open_opaque, prepare_load (&&), into_extension (&&), observe.
  // Proposed: one closed static entry point with two overloads over the five-member command_options variant.
  // Render, init, history and export arm: the envelope-admitted selector plus that family's parsed options.
  // A command_load_options alternative is refused here before capture (redacted protocol error, G30); load
  // never enters through a selector.
  static std::expected<profile_invocation, registration_error>
  open_with_profile(std::string_view profile_selector, const command_options& family);
  // Load arm: the one request_v2 intent framed at X-CLI step 4 and used for the step-5 early lookup, plus the
  // native ledger path taken from command_load_options::ledger. No selector argument: open_known_load
  // extracts the selector from the intent. No second framing. Forwards to open_known_load unchanged.
  static std::expected<profile_invocation, registration_error>
  open_with_profile(const acceptance::request_intent& request_intent, std::string_view ledger_path);

private:
  // Proposed private helper, not exported: capture, metadata-admit the family's slots, register once, reverify.
  static std::expected<profile_invocation, registration_error>
  open_admitted(std::string_view profile_selector, const command_options& family);
};
```

Routing inside the factory:

- **Load** (`command_load_options`) enters only through the load overload, whose inputs are the single `request_v2` intent framed at X-CLI step 4 and used for the step-5 early lookup, and the native ledger path from `command_load_options::ledger`. It has no selector argument and does no second framing: `open_known_load` requires the codec2 request intent and extracts the profile selector itself, so no independent profile argument can disagree (`profile-preparation-architecture.md:205-208`; status `design`, row A-01). This is a plan for X07, not the current types: today `command_load_options` (`runtime.cppm:58-66`) holds the ledger, workspace, registry, expect_revision, request_id, actor and expected_head fields and no request intent, and `open_known_load(const acceptance::request_intent&, std::string_view ledger_path)` (`runtime.cppm:466-467`) is the only existing entry that takes the intent. X07 adds the load overload with that same parameter list; it does not add an intent field to `command_load_options`. The selector overload refuses a `command_load_options` alternative before capture, so load cannot be opened from a selector. It routes to `open_known_load` (`runtime.cpp:5039`), then `prepare_load`, then the exported `complete_acceptance(prepared_profile)` overload (`src/engine/ledger/profile_acceptance.cppm:21`). Load keeps its existing sequence (operator decision Q1 B).
- **Render, init, history and export** use the private admitted path. It captures the profile closure, metadata-admits that family's known slots from 1.2 before registration, registers once and reverifies. This is the same order `open_known_load` already follows (`runtime.cppm`, the `open_known_load` declaration comment). The admission position is X-CLI step 6.
- **Render** runs the approved `render_profile` phases (`runtime.cppm:281`) in the retained VM, so `render_external_v1` and custom shapes apply (Q1 B).
- **Init, history and export** run their sealed shipped callables in the same profile VM after admission, with the existing native services. This needs an invoke path on profile VMs. The exported `command_vm::invoke` keeps its refusal of profile owners (`runtime.cpp:4844-4845`), as the architecture requires (`profile-preparation-architecture.md:46`, generic `invoke` explicitly refuses profiles) and as the exported `command_vm::create(profile_snapshot)` note states (`runtime.cppm:221-225`, generic command invocation refuses). The invoke path is a module-private function of `planar.engine.runtime`, reachable only through the closed factory and not exported. Lifting the refusal in the exported `invoke` is not chosen: with `command_vm::create(profile_snapshot)` also exported, it would let in-tree callers skip admission. Choosing that option instead would be a design change that halts the round. Section 4 records the effect on CL-5.
- No caller supplies a profile value, catalog, snapshot or VM. The inputs are per arm: render, init, history and export take the admitted envelope selector and that family's parsed options; load takes the step-4 `request_v2` intent and the native ledger path, with no selector. The factory captures the profile itself in both arms.

### 1.2 Per-family admission table

Known slots are the native root, file and target slots that each family supplies. Each check runs after profile capture, before registration (X-CLI step 6), unless the row says otherwise. Missing supplied paths keep the existing family capture errors (X-CAP `:172`).

| Family | Known slots | Check and phase | Citation |
| --- | --- | --- | --- |
| render | Workspace declaration root and selected declaration file; registry file; supplied source root; supplied base root; observations file. | Root overlap with the trusted profile root is refused as `unsafe_path` in either direction for the declaration root, source root and base root, even when render omits output capture. The observations file is refused if it is inside the trusted root or aliases a trusted selected file. The registry file is checked for selected-file aliases with the profile and its modules only. The selected declaration file is also checked for aliases (shared device/inode, for example a hard link) against the profile and its modules, and an alias is refused as `unsafe_path` (G26) before registration. Phase: after capture, before registration. | [X-CAP](external-profile-contract.md#x-cap--strict-capture-and-trust) `:154-155`, `:159-168`; architecture hard-link gap; X-CLI step 6 |
| init | Ledger target only (the new explicit target). The profile and its modules are the trusted selected files. | Alias-only. Operator Q-INIT A (2026-10-28): the init ledger target gets only the selected-file alias check against the profile and its modules (X-CAP, external-profile-contract.md:159-175; X-STORE :647-650); no containment refusal inside the trusted root and no contract amendment. The target does not exist yet, so the alias check is vacuous for a new target. Phase: after capture, before registration (X-CLI step 6). Format2 is created only through `initialize_new_v2` after admission. | [X-CAP](external-profile-contract.md#x-cap--strict-capture-and-trust) `:159-175`; X-STORE `:647-650`; Q-INIT A |
| history | Ledger file. There is no inferred workspace root (X-CAP `:173-174`). | Selected-file alias with the profile and its modules. Phase: after capture, before registration (X-CLI step 6). | [X-CAP](external-profile-contract.md#x-cap--strict-capture-and-trust) `:169-170`, `:173-174`; X-CLI step 6 |
| export | Ledger file (selected-file alias) and the proposed target tree (overlap with the trusted root and selected files). | Ledger alias and target-tree overlap are both refused before publication. Phase: after capture, before registration (X-CLI step 6). | [X-CAP](external-profile-contract.md#x-cap--strict-capture-and-trust) `:170-171`; X-STORE `:650-655`; X-CLI step 6 |
| load | Workspace root, declaration file, registry file and ledger (the existing load set). | Unchanged. The early saved-body lookup runs before capture (X-CLI step 5). Then capture, metadata admission, registration and reverification in `open_known_load`. | X-CLI steps 4 to 6; [load admission](command-contract.md#load-admission-and-exact-provenance) |
| spec-prompt | None. Embedded-only authoring family (RA01-B wave 1). | Not admitted under with-profile: no known slots exist and no profile is captured for it. A with-profile tail naming `spec-prompt` stays usage after profile admission (X-CLI step 7), like any unknown verb. | [X-CLI](external-profile-contract.md#x-cli--envelope-and-admission-order) step 7; RA01-B wave 1 plan D23 |
| critique | None. Embedded-only authoring family (RA01-B wave 1). | Not admitted under with-profile: no known slots exist and no profile is captured for it. A with-profile tail naming `critique` stays usage after profile admission (X-CLI step 7), like any unknown verb. | [X-CLI](external-profile-contract.md#x-cli--envelope-and-admission-order) step 7; RA01-B wave 1 plan D23 |
| ingest | None. Embedded-only authoring family (RA01-B wave 1). | Not admitted under with-profile: no known slots exist and no profile is captured for it. A with-profile tail naming `ingest` stays usage after profile admission (X-CLI step 7), like any unknown verb. | [X-CLI](external-profile-contract.md#x-cli--envelope-and-admission-order) step 7; RA01-B wave 1 plan D23 |
| explain-rejection | None. Embedded-only authoring family (RA01-B wave 1). | Not admitted under with-profile: no known slots exist and no profile is captured for it. A with-profile tail naming `explain-rejection` stays usage after profile admission (X-CLI step 7), like any unknown verb. | [X-CLI](external-profile-contract.md#x-cli--envelope-and-admission-order) step 7; RA01-B wave 1 plan D23 |
| observe | None. Embedded-only authoring family (RA01-B wave 1). | Not admitted under with-profile: no known slots exist and no profile is captured for it. A with-profile tail naming `observe` stays usage after profile admission (X-CLI step 7), like any unknown verb. | [X-CLI](external-profile-contract.md#x-cli--envelope-and-admission-order) step 7; RA01-B wave 1 plan D23 |
| orchestrator | None. Embedded-only authoring family (RA01-B wave 1). | Not admitted under with-profile: no known slots exist and no profile is captured for it. A with-profile tail naming `orchestrator` stays usage after profile admission (X-CLI step 7), like any unknown verb. | [X-CLI](external-profile-contract.md#x-cli--envelope-and-admission-order) step 7; RA01-B wave 1 plan D23 |
| selected help | None. Selected help supplies no untrusted roots. | Trusted capture only, then registration and deterministic help. No known slots are checked. | X-CAP `:172-175`; [X-CLI help paragraph](external-profile-contract.md#x-cli--envelope-and-admission-order) |

### 1.3 Family registry

The family set is `command_options` (`runtime.cppm:68`), a `std::variant` of five members: `command_render_options`, `command_init_options`, `command_history_options`, `command_export_options` and `command_load_options`. The variant is closed at five. A sixth member needs an architecture revision, a new admission row in 1.2 and a reviewed amendment of the contract that defines the family. The six authoring families (spec-prompt, critique, ingest, explain-rejection, observe, orchestrator) are embedded-only: the `command_options` variant stays closed at five and the six families live outside it, so none of them has a with-profile arm in the factory. There is no generic service factory. The factory takes no caller-supplied profile argument. Its only inputs are per arm (1.1): for render, init, history and export, the envelope-admitted selector and that family's parsed options; for load, the single `request_v2` intent framed at X-CLI step 4 and used at step 5, plus the native ledger path from `command_load_options::ledger`, with no selector argument (operator decision Q1 B; D7-1; load-arm input `design`, `profile-preparation-architecture.md:205-208`, row A-01).

### 1.4 Dispatch per family and X-CLI step 8

| Family | Dispatch entry | Citation |
| --- | --- | --- |
| render | X-CLI step 8: the admitted render intent is bound in the same retained VM and captured bundle. The sealed `render_profile` phase set is selected by native registry reference. Dispatched once. All state is destroyed at the terminal result. Render uses retained phase callables. | [X-CLI step 8](external-profile-contract.md#x-cli--envelope-and-admission-order) |
| init | X-CLI step 8: the sealed shipped init callable is bound by native registry reference in the same retained VM and captured bundle, after admission. Dispatched once. All state is destroyed at the terminal result. | [X-CLI step 8](external-profile-contract.md#x-cli--envelope-and-admission-order); `command-contract.md:117-121` |
| history | X-CLI step 8: the sealed shipped history callable is bound by native registry reference in the same retained VM and captured bundle. The history decode into the VM is charged to the factory budget owner (clause 30). Dispatched once. All state is destroyed at the terminal result. | [X-CLI step 8](external-profile-contract.md#x-cli--envelope-and-admission-order); D7-8 |
| export | X-CLI step 8: the sealed shipped export callable is bound by native registry reference in the same retained VM and captured bundle. Target-tree overlap is refused at admission, before publication. Dispatched once. All state is destroyed at the terminal result. | [X-CLI step 8](external-profile-contract.md#x-cli--envelope-and-admission-order) |
| load | X-CLI step 8: `prepare_load` consumes the retained owner once, and `complete_acceptance(prepared_profile)` commits. The retained phase callables are the dispatch. Dispatched once. All state is destroyed at the terminal result. | [X-CLI step 8](external-profile-contract.md#x-cli--envelope-and-admission-order); [load admission](command-contract.md#load-admission-and-exact-provenance) |

### 1.5 Command-contract substitution evidence

The command contract requires that tests replace an embedded handler in a test bundle and prove that its semantic result is used after identical CLI11 parsing (`command-contract.md:117-121`). Under a profile, render and load run retained phase callables instead of the shipped handler, so that test form does not carry over as written. The substitute evidence per family is:

| Family | Substitution evidence | Citation |
| --- | --- | --- |
| render | Retained phase callables: under a profile, render runs the approved `render_profile` phases on the retained VM. The substitute evidence is the custom-shape effect on W and on the render output, observed through the real CLI with `render_external_v1`. | `command-contract.md:117-121`; [design risks](remaining-steps-design.md#risks-of-late-discovery) |
| load | Retained phase callables: `prepare_load` and the exported `complete_acceptance(prepared_profile)` run the load phases on the retained VM. The substitute evidence is the committed or replayed load result observed through the CLI. | `command-contract.md:117-121`; [design risks](remaining-steps-design.md#risks-of-late-discovery) |
| init | Test-bundle handler replacement driven through the closed factory under with-profile: a test-only embedded bundle replaces the init handler, CLI11 parsing is identical, and the substituted semantic result is observed. | `command-contract.md:117-121`; A-11 |
| history | Test-bundle handler replacement driven through the closed factory under with-profile: a test-only embedded bundle replaces the history handler, CLI11 parsing is identical, and the substituted semantic result is observed. | `command-contract.md:117-121`; A-11 |
| export | Test-bundle handler replacement driven through the closed factory under with-profile: a test-only embedded bundle replaces the export handler, CLI11 parsing is identical, and the substituted semantic result is observed. | `command-contract.md:117-121`; A-11 |

Ordinary no-profile substitution tests are unchanged. They replace the embedded handler without with-profile, and the factory is not on their path.

### 1.6 Creation sites and source-scan audit

The table lists every construction of a command VM or a profile owner in product source under `src/` at the starting revision. Profile VMs are built at one call, `command_vm::create(*snapshot)` in `extension_invocation::register_vm` (`runtime.cpp:4963-4982`, call at 4969). After X07 that call is reached only through the closed factory, so the factory is the single creation site in `planar.engine.runtime`.

| Site | Construction | Treatment after X07 |
| --- | --- | --- |
| `src/cmd/planar/bootstrap.cpp:103` | `engine::command_vm::create()`, embedded catalog, no profile | Unchanged. Not a profile creation site. |
| `src/cmd/planar/invocation.cpp:404` | `engine::command_vm::create()`, embedded catalog, no profile | Unchanged. Not a profile creation site. |
| `src/engine/runtime/runtime.cpp:3670` | Definition of `command_vm::create()`, no profile | Unchanged. |
| `src/engine/runtime/runtime.cpp:3676` | Definition of `command_vm::create(profile_snapshot)`, the profile VM constructor | Engine-internal. No `src/cmd/` caller. |
| `src/engine/runtime/runtime.cpp:4969` | `command_vm::create(*snapshot)` in `register_vm` (`4963-4982`), the one profile VM build call | Reached only through the closed factory (single creation site). |
| `src/engine/runtime/runtime.cpp:4983` | `extension_invocation::create(profile_capture)` | Engine-internal. No `src/cmd/` caller. |
| `src/engine/runtime/runtime.cpp:5039` | `profile_invocation::open_known_load` | Load route of the closed factory only. |
| `src/engine/runtime/runtime.cpp:5095` | `profile_invocation::open_opaque` | Opaque route for selected help and unknown extension tails only. |

The D7-12 audit is a Catch2 source-scan test in the X06-C style, under `src/cmd/`. It fails if any file there calls `extension_invocation::create` or `command_vm::create(` with a profile argument. Threat model XD1 applies unchanged: trusted in-tree code that uses exported names, guaranteed by language visibility plus this source scan. Deliberate adversarial C++ is a recorded limit under XD1, not a request-changes finding.

The renamed authoring stack and the malformed-bundle variants are generated test targets compiled with `PLANAR_AUTHORING_BUNDLE`. Their `command_vm::create()` call is the same embedded no-profile site, renamed. The production targets `planar`, `planar_command` and `planar_runtime` never receive the definition. The source-scan wording is unchanged because no new creation call exists.

### 1.7 Ownership boundaries

The factory owns the native profile capture with its factory budget, the original root and file receipts, the dormant VM with every native receipt, and the whole-invocation budget owner (D7-8, Q2 A). It must not expose `command_vm`, a mutable catalog, receipts, a generic service factory, or any caller-supplied profile argument. There is no reverse conversion from an owner back to a capture or selector. There is no mutable `command_vm` extraction. There is no new exported constructor. `profile_invocation`'s constructor stays private. The architecture's private same-runtime adoption constructor (`profile-preparation-architecture.md`, constructor, parse and consumer protocol, `:233-247`) is the only new constructor the extension path relies on, and it stays private too.

The factory's relation to the existing conversions is:

- `open_opaque` serves selected help and unknown external tails. It has no upgrade to known load, and the class block says there is intentionally no upgrade API (`runtime.cppm:455-460`).
- `into_extension` adopts opaque owners only and refuses known-load owners, consuming nothing and leaving the owner usable (`runtime.cppm:496`). Init, history and export are opened by the closed factory, never by `into_extension` on a known-load owner.
- Extension dispatch for external commands keeps using `into_extension` on opaque owners only.
- The profile invoke path is module-private and reachable only through the closed factory after admission. The exported `command_vm::invoke` keeps refusing profile owners (`runtime.cpp:4844-4845`; architecture `:46`; `runtime.cppm:221-225`). No exported name lets a caller invoke a profile VM that skipped admission.

### 1.8 Replacement architecture sentence

The sentence below replaces the architecture sentence that begins "Other known shipped families require their own finite admission path" in [the architecture doc](profile-preparation-architecture.md#constructor-parse-and-consumer-protocol). It is one line here, and it is byte-identical in the architecture doc, where the same paragraph links this addendum.

> Render, init, history and export are opened by the closed factory defined in the X07 design addendum; load is opened by open_known_load; no generic service factory exists, and a sixth with-profile family requires an amendment.

### 1.9 Attachment of X07-A and X07-B by wave

Each X07 item attaches to one wave. Wave A must compile as stubs before wave B's test coders start (D7-13).

| ID | Wave | Item | Clauses or notes |
| --- | --- | --- | --- |
| WA-1 | A | Envelope module `src/cmd/planar/envelope.{cppm,cpp}`, imported only by bootstrap; handlers never import it | D7-4; A-06 |
| WA-2 | A | CLI11 envelope: `prefix_command()`, scalar `--stdlib-profile` with Throw multiplicity, nonempty validator, counted `--help` | Clauses 1, 2 (X-CLI 1, 2) |
| WA-3 | A | Embedded and opaque probes; reverse-order passthrough copied unchanged to both probes | Clause 3 (X-CLI 3) |
| WA-4 | A | Final parser built from the sealed catalog, with option storage alive through dispatch | Clauses 7, 8 (X-CLI 7) |
| WA-5 | A | Selected help: exact format, sorting, escaping, 1 MiB buffer | Clause 11 |
| WA-6 | A | Error adapter in `src/cmd/planar/response.*` (one adapter) | D7-7; A-04; A-05; clause 10 |
| WA-7 | A | Closed factory with admission (section 1.1 to 1.3), including the known-slot checks before registration | Clauses 6, 12, 13; D7-1 |
| WA-C | A | Clauses 1 to 13, 27 and 28 | Q-WAVE reconciliation in the paragraph below (A-08) |
| WB-1 | B | Extension handler module `src/cmd/planar/handlers/extension/` | Clauses 18, 19 |
| WB-2 | B | Load and early replay wiring for profile load, including the replay and relookup races | Clauses 20, 21, 22 |
| WB-3 | B | Format2 init through `initialize_new_v2` (today `invocation.cpp:234` uses `initialize_new`) | Clause 24; A-08 |
| WB-4 | B | Request codec by opened format: v1 on format1, request_v2 with the embedded selector on format2 | Clauses 25, 26; D7-6 |
| WB-5 | B | History and extension services, including the `history_id` matrix, decimal and limit vectors, and role-read overlap refusals | Clauses 14 to 17 |
| WB-6 | B | Races and faults after commit through generated CLI variants and one two-process CAS race; no product hooks | Clause 23; D7-9; Q3 A |
| WB-7 | B | Budget threading: late-slot admission, role capture, history decode into the VM and projection transport charge the factory owner | Clause 30; D7-8; Q2 A |
| WB-C | B | Clauses 14 to 26, 29 and 30 | Wave B runs after wave A stubs compile (D7-13) |

Q-WAVE reconciliation with D7-13. D7-13 in the [design](remaining-steps-design.md#design-decisions-implied) lists clauses 24 to 28 under wave A. The round record's objective (part 1) and the X07-B row of the [implementation path](external-profile-implementation.md#source-seams-and-bounded-implementation-rounds) place format2 init and the request codec by opened format (clauses 24 to 26) in X07-B. This addendum follows the round record for clauses 24 to 26 (wave B), keeps clauses 27 and 28 in wave A, because they are factory admission rules, and records the choice as A-08. This is a wave-order choice, not a product or authority choice.

## 2. X01 production outcome table

### 2.1 Legend and fixture

Every row runs against the production bootstrap, not the spike fixture (D7-3). Each row runs in a fresh scratch tree with the layout below. The trusted root is the sibling directory `trusted/` of the invocation cwd `work/`, so no cwd-relative path lies inside the trusted root (fixture layout, row A-10).

- `trusted/` holds `p.json`, a valid profile that registers one external command `inspect` with the option `--text` (a required nonempty text value that does not begin with `--`) and the flag `--flag`. It also holds `q.json`, a second valid profile, and their modules.
- `work/` is the invocation cwd and starts empty.
- The selector spellings are `../trusted/p.json` (SP) and `../trusted/q.json` (SQ). Every selector token in the tables is one of these two spellings.
- All other path names, `a`, `out`, `w`, `r`, `src` and `missing`, are relative to `work/` and name nonexistent paths.
- `PLANAR_WORKSPACE_DIR` is unset in every counted row.
- The opaque prefix is `with-profile --stdlib-profile ../trusted/p.json`. The opaque rows in 2.3 are that prefix followed by the tail.
- Token rule: argv cells omit the executable name. Tokens are separated by single spaces. An empty token is written `""` and maps to the empty string. An empty tail is the empty string.

Class vocabulary, with the X-CLI step that decides each class:

- SYN: syntax guard (step 1). The profile is not accessed.
- PFX: prefix usage (step 2). The profile is not accessed.
- PRB: probe usage (step 3). The profile is not accessed.
- NRM: known-slot normalization usage (step 4; `command-contract.md:289-316`). The profile is not accessed.
- ERL: early lookup (step 5; load only). The profile is not accessed.
- KSA: known-slot admission refusal (step 6). It happens after profile capture and the early replay miss, before registration. The family's capture error is exit 3 or 4. The profile is accessed; registration is not run.
- PRF: profile error (step 6). The profile is accessed.
- FIN: final usage after capture and registration (step 7). The profile is accessed.
- LAT: late overlap (step 7, `unsafe_path`). The profile is accessed.
- HND: handler (step 8). The profile is accessed.
- HLP: selected help (the help paragraph of X-CLI, and step 6). The profile is accessed.

"Accessed" means the trusted profile files were opened and captured.

Variants are non-counted notes. E36 with the environment set to an absolute scratch directory and ledger `a` missing is ERL, exit 3 `ledger_not_found`, with the profile not accessed. E37 with `--workspace w/workspace.lua`, a missing explicit file, reaches the known tail and is a KSA refusal with the existing native capture error (G36).

### 2.2 Envelope rows

Each row gives the production argv that the spike row must now be run with, the production exit, the class, whether the profile was accessed, and the X-CLI steps that govern it. The Spike case column is the only place where spike case names appear. Classes and clauses are translated from the spike labels, not copied.

| ID | Spike case | Production argv | Exit | Class | Accessed | Clause | Note |
| --- | --- | --- | --- | --- | --- | --- | --- |
| E01 | ordered-extension | `with-profile --stdlib-profile ../trusted/p.json inspect --text value --flag` | 0 | HND | yes | steps 2, 3, 6, 7, 8 | extension summary success |
| E02 | shipped-history | `with-profile --stdlib-profile ../trusted/p.json history --ledger missing` | 3 | KSA | yes | steps 3, 6 | known shipped tail: ledger slot missing is a step-6 admission refusal `ledger_not_found`, after capture, before registration (derived from X-CAP :172, :179-182, :195) |
| E03 | no-selector | `with-profile inspect --text value` | 2 | PFX | no | step 2 | selector required |
| E04 | no-value | `with-profile --stdlib-profile` | 2 | PFX | no | step 2 |  |
| E05 | empty-value | `with-profile --stdlib-profile "" inspect --text value` | 2 | PFX | no | step 2 | validator nonempty |
| E06 | verb-as-value | `with-profile --stdlib-profile inspect --text value` | 2 | PRB | no | steps 2, 3 | selector value is `inspect`; tail `--text value` begins with an unknown flag: probe refusal before profile access (CHANGES from spike) |
| E07 | duplicate-selector | `with-profile --stdlib-profile ../trusted/p.json --stdlib-profile ../trusted/q.json inspect --text v` | 2 | PFX | no | step 2 | Throw multiplicity |
| E08 | unknown-option-value | `with-profile --stdlib-profile --bogus inspect --text v` | 2 | PFX | no | step 2 | selector value `--bogus` fails the validator (spike fixture lacked it) |
| E09 | known-option-value | `with-profile --stdlib-profile --help inspect --text v` | 2 | PFX | no | step 2 | selector value `--help` fails the validator |
| E10 | missing-help-value | `with-profile --stdlib-profile --help` | 2 | PFX | no | step 2 |  |
| E11 | extra-prefix-option | `with-profile --stdlib-profile ../trusted/p.json --bogus inspect --text v` | 2 | PRB | no | steps 2, 3 | tail begins `--bogus`: probe refusal (CHANGES from spike) |
| E12 | option-before-selector | `with-profile --bogus --stdlib-profile ../trusted/p.json inspect --text v` | 2 | PFX | no | step 2 |  |
| E13 | unknown-verb | `with-profile --stdlib-profile ../trusted/p.json absent --text v` | 2 | FIN | yes | steps 3, 6, 7 | profile captured and registered first; unknown command is usage after admission |
| E14 | profile-as-tail-value | `with-profile --stdlib-profile ../trusted/p.json inspect --text --stdlib-profile` | 2 | FIN | yes | step 7 | extension text value beginning `--` refused |
| E15 | tail-selector | `with-profile --stdlib-profile ../trusted/p.json inspect --stdlib-profile ../trusted/q.json --text v` | 2 | FIN | yes | step 7 | `--stdlib-profile` after a verb has no selector role |
| E16 | tail-option-value | `with-profile --stdlib-profile ../trusted/p.json inspect --text --bogus` | 2 | FIN | yes | step 7 | text value `--bogus` refused |
| E17 | tail-known-option-value | `with-profile --stdlib-profile ../trusted/p.json inspect --text --flag` | 2 | FIN | yes | step 7 | text value `--flag` refused |
| E18 | no-tail | `with-profile --stdlib-profile ../trusted/p.json` | 2 | PFX | no | step 2 | empty tail without help |
| E19 | prefix-terminator | `with-profile --stdlib-profile ../trusted/p.json -- inspect --text v` | 2 | PRB | no | steps 2, 3 | leading terminator in the tail: probe refusal (CHANGES from spike) |
| E20 | tail-terminator | `with-profile --stdlib-profile ../trusted/p.json inspect --text v --` | 2 | FIN | yes | step 7 | discarded terminator rejected by token count |
| E21 | terminator-value | `with-profile --stdlib-profile -- inspect --text v` | 2 | PFX | no | step 2 | selector value `--` fails the validator |
| E22 | selected-help | `with-profile --stdlib-profile ../trusted/p.json --help` | 0 | HLP | yes | steps 2, 6; help | capture and register, deterministic help |
| E23 | help-before-selector | `with-profile --help --stdlib-profile ../trusted/p.json` | 0 | HLP | yes | steps 2, 6; help | both orders valid |
| E24 | help-with-tail | `with-profile --stdlib-profile ../trusted/p.json --help inspect --text v` | 2 | PFX | no | step 2 |  |
| E25 | help-after-verb | `with-profile --stdlib-profile ../trusted/p.json inspect --help` | 2 | FIN | yes | step 7 | no per-command help |
| E26 | duplicate-help | `with-profile --stdlib-profile ../trusted/p.json --help --help` | 2 | PFX | no | step 2 |  |
| E27 | shipped-missing | `with-profile --stdlib-profile ../trusted/p.json history` | 2 | PRB | no | step 3 | known shipped signature failure is usage, no fallback |
| E28 | shipped-duplicate | `with-profile --stdlib-profile ../trusted/p.json history --ledger a --ledger b` | 2 | PRB | no | step 3 |  |
| E29 | shipped-extra | `with-profile --stdlib-profile ../trusted/p.json history --ledger a --bogus b` | 2 | PRB | no | step 3 |  |
| E30 | shipped-terminator | `with-profile --stdlib-profile ../trusted/p.json history --ledger a --` | 2 | PRB | no | step 3 |  |
| E31 | name-equals | `with-profile --stdlib-profile=../trusted/p.json inspect --text v` | 2 | SYN | no | step 1 |  |
| E32 | tail-name-equals | `with-profile --stdlib-profile ../trusted/p.json inspect --text=v` | 2 | SYN | no | step 1 |  |
| E33 | root-terminator | `-- with-profile --stdlib-profile ../trusted/p.json inspect --text v` | 2 | PFX | no | step 2 |  |
| E34 | shipped-init | `with-profile --stdlib-profile ../trusted/p.json init --ledger a --workspace-id w` | 0 | HND | yes | steps 3, 4, 6, 7, 8 | creates format2 at new target `a` in `work/`; alias check vacuous (Q-INIT A); response bytes unchanged |
| E35 | shipped-export | `with-profile --stdlib-profile ../trusted/p.json export --ledger a --revision 1 --target out` | 3 | KSA | yes | steps 3, 6 | target `out` is in `work/`, outside the trusted root, so no overlap refusal; ledger `a` missing is a step-6 admission refusal `ledger_not_found` before registration (derived from X-CAP :172, :179-182, :195) |
| E36 | shipped-load | `with-profile --stdlib-profile ../trusted/p.json load --ledger a --workspace w --registry r --expect-revision none --request-id k --actor a` | 2 | NRM | no | step 4 | bare name `w` with environment unset is usage before early lookup (variant in legend) |
| E37 | shipped-render | `with-profile --stdlib-profile ../trusted/p.json render --workspace w --registry r --root s=src --job j --role r` | 2 | NRM | no | step 4 | same |
| E38 | init-missing | `with-profile --stdlib-profile ../trusted/p.json init --ledger a` | 2 | PRB | no | step 3 |  |
| E39 | export-malformed | `with-profile --stdlib-profile ../trusted/p.json export --ledger a --revision bad --target out` | 2 | PRB | no | step 3 |  |
| E40 | load-missing | `with-profile --stdlib-profile ../trusted/p.json load --ledger a` | 2 | PRB | no | step 3 |  |
| E41 | render-missing | `with-profile --stdlib-profile ../trusted/p.json render --workspace w` | 2 | PRB | no | step 3 |  |
| E42 | help-terminator | `with-profile --stdlib-profile ../trusted/p.json --help --` | 2 | PFX | no | step 2 |  |
| E43 | empty-tail-value | `with-profile --stdlib-profile ../trusted/p.json inspect --text ""` | 2 | FIN | yes | step 7 | extension text must be nonempty |
| E44 | second-command | `with-profile --stdlib-profile ../trusted/p.json history --ledger a inspect --text v` | 2 | PRB | no | step 3 |  |

### 2.3 Opaque-probe rows

Each row is the opaque prefix followed by the tail. Two rows do not reach the opaque probe through the envelope. O05 (`--help`) is consumed by the envelope as the counted selected help. O12 (an empty tail) is prefix usage. O06 reaches the probe: its leading terminator is refused by the probe, which is the same argv as E19. O07 is a KSA refusal (see 2.4).

| ID | Tail | Reaches probe | Exit | Class | Accessed | Clause |
| --- | --- | --- | --- | --- | --- | --- |
| O01 | `inspect --text v --flag` | yes | 0 | HND | yes | steps 3, 6, 7, 8 |
| O02 | `inspect history --text --stdlib-profile` | yes | 2 | FIN | yes | steps 3, 7 |
| O03 | `absent --unknown -- history` | yes | 2 | FIN | yes | steps 3, 7 |
| O04 | `--bogus inspect --text v` | yes | 2 | PRB | no | step 3 |
| O05 | `--help` | no (envelope consumes it: selected help) | 0 | HLP | yes | step 2; help |
| O06 | `-- inspect --text v` | yes | 2 | PRB | no | step 3 |
| O07 | `history --ledger a` | yes | 3 | KSA | yes | steps 3, 6 |
| O08 | `history` | yes | 2 | PRB | no | step 3 |
| O09 | `history --ledger a --bogus b` | yes | 2 | PRB | no | step 3 |
| O10 | `history --ledger a --` | yes | 2 | PRB | no | step 3 |
| O11 | `history --ledger a inspect --text v` | yes | 2 | PRB | no | step 3 |
| O12 | `` | no (empty tail is prefix usage) | 2 | PFX | no | step 2 |

### 2.4 Class changes from the spike

E06 (spike case verb-as-value) changes class. Its selector value is `inspect`, and its tail `--text value` begins with an unknown flag. The probe refuses a leading unknown flag before any profile access, as X-CLI step 3 requires, so the row is PRB, exit 2, with the profile not accessed. The spike labelled it as a final-stage refusal because its fixture always ran the final parser.

E11 (extra prefix option, tail beginning `--bogus`) and E19 (prefix terminator, tail beginning `--`) change class for the same reason. Both are PRB refusals before profile access under X-CLI step 3, not final-stage refusals.

Some rows are translated because the production validators refuse them earlier or later than the spike fixture did. The spike fixture omitted those validators. E08 and E09 (selector values `--bogus` and `--help`) and E21 (selector value `--`) fail the nonempty-and-not-starting-with-`--` validator and are PFX usage at step 2. E14, E16, E17 and E43 (extension text values beginning `--` or empty) are FIN usage after capture and registration at step 7, because the X-REG text rule refuses them there (`external-profile-contract.md:242-243`, X-CLI 7).

E13 (unknown verb after a valid profile) is FIN, exit 2, with the profile accessed. The unknown final command is usage after profile admission, which X-CLI step 7 requires.

O05 and O12 do not reach the opaque probe through the envelope, and O06 does reach it and is refused there. Section 2.3 gives the reach column for each row.

KSA is a known-slot admission refusal, separate from handler results. E02 (`history --ledger missing`), E35 (`export` with ledger `a` missing) and O07 (`history --ledger a`, ledger missing) are step-6 admission refusals, not handler results. X-CLI step 6 requires every supplied slot of a known shipped tail to be checked after capture and the early replay miss, and before registration. X-CAP `:172` (missing supplied paths keep the existing family capture errors), `:179-182` (the known shipped tail checks every supplied slot before external Lua) and `:195` (all supplied slots require initial checks) make the missing ledger a `ledger_not_found` refusal of exit 3 at that point. The profile is accessed, and the result is registration not run. The D7-5 marker shows that: a profile module with unbounded chunk-level work returns `resource_limit` only if registration runs, and these rows return `ledger_not_found`, so registration did not run. E34 (init at new target `a`) stays HND, exit 0, because the alias check is vacuous for a target that does not exist (Q-INIT A).

E36 and E37 (bare workspace name `w`) are NRM usage at step 4. With `PLANAR_WORKSPACE_DIR` unset, the current C rule makes a bare name a usage error before profile access and before the load early lookup (`command-contract.md:289-316`). Their environment variants are non-counted notes in 2.1.

Class names in this addendum are the production names only. The spike's final-stage labels map to the production class in 2.2 (FIN, LAT or HND), or to PRB and PFX where X-CLI steps 2 and 3 refuse first.

### 2.5 Production outcome and diagnostic table

This table has exactly the 69 rows G01 to G69 of the approved diagnostic set, and no other row. Every row cites an existing message or code from the closed table, the X-LIMIT table or the source named in its Source column. No row is new. Exit values follow `failed()` (`cmd/planar/response.cpp:162-170`): usage 2; io, internal, ledger_corrupt, ledger_busy and storage_full 4; every other code 3. The forced exit 4 for export_incomplete is at `cmd/planar/invocation.cpp:479-481`, and the unrecoverable allocation case is `{4, "", ""}`.

The runtime string Invalid extension invocation (`src/engine/runtime/runtime.cpp:4016`) is never printed by the public CLI. It is not a row of this table, and no X07 response may print it. Every exit-2 usage, including `extension_preparation_failure::usage`, prints the existing public bytes `Expected command options or --help; see --help` (row A-04).

| ID | Outcome class | Exit | Code | Message | Source |
| --- | --- | --- | --- | --- | --- |
| G01 | syntax-and-usage, extension_preparation_usage, unknown-final-command | 2 | `usage` | `Expected command options or --help; see --help` | `cmd/planar/response.cpp:175` (detail::usage); adapter maps `extension_preparation_failure::usage` here (A-04) |
| G02 | syntax-and-usage, path-normalization | 2 | `usage` | `Expected command options or --help; see --help` | `engine/capture/host_path.cpp:71`, `:121` |
| G03 | path-normalization | 3 | `path_denied` | `Invalid explicit input path` | `engine/capture/host_path.cpp:64` |
| G04 | path-normalization | 3 | `path_denied` | `Invalid explicit input cwd` | `engine/capture/host_path.cpp:64` |
| G05 | path-normalization | 3 | `resource_limit` | `Explicit input path limit exceeded` | `engine/capture/host_path.cpp:67` |
| G06 | path-normalization | 4 | `io` | `Cannot resolve explicit input path` | `engine/capture/host_path.cpp:83`, `:92`; `cmd/planar/invocation.cpp:111` |
| G07 | path-normalization | 4 | `io` | `Cannot obtain invocation cwd` | `cmd/planar/invocation.cpp:367`, `:416` |
| G08 | path-normalization | 3 | `path_denied` | `Explicit input file is a symbolic link` | `engine/capture/host_path.cpp:12` |
| G09 | path-normalization | 4 | `io` | `Explicit input must be a regular file` | `engine/capture/host_path.cpp:14` |
| G10 | path-normalization | 4 | `io` | `Explicit root must be a directory` | `engine/capture/host_path.cpp:21` |
| G11 | path-normalization | 3 | `path_denied` | `Too many symbolic links in explicit root` | `engine/capture/host_path.cpp:45` |
| G12 | path-normalization | 3 | `path_denied` | `Explicit root is a symbolic link target` | `engine/capture/host_path.cpp:47` |
| G13 | path-normalization | 4 | `io` | `Cannot resolve explicit root directory` | `engine/capture/host_path.cpp:56` |
| G14 | early-replay-and-load-prelude | 3 | `path_denied` | `Cannot resolve explicit input path` | `cmd/planar/invocation.cpp:43` |
| G15 | early-replay-and-load-prelude | 3 | `resource_limit` | `Load exceeds native resource limits` | `cmd/planar/invocation.cpp:45` |
| G16 | early-replay-and-load-prelude, internal-trusted-api | 4 | `internal` | `Invalid native load preparation` | `cmd/planar/invocation.cpp:46` |
| G17 | early-replay-and-load-prelude | 3 | `resource_limit` | `Load request exceeds native resource limits` | `cmd/planar/invocation.cpp:446` |
| G18 | early-replay-and-load-prelude | 3 | `schema` | `Invalid load arguments` | `cmd/planar/invocation.cpp:447` |
| G19 | early-replay-and-load-prelude, internal-trusted-api | 4 | `internal` | `Missing native load capture` | `cmd/planar/invocation.cpp:135` |
| G20 | early-replay-and-load-prelude, internal-trusted-api | 4 | `internal` | `Missing native load intent` | `cmd/planar/invocation.cpp:146` |
| G21 | early-replay-and-load-prelude, internal-trusted-api | 4 | `internal` | `Invalid saved load response` | `cmd/planar/invocation.cpp:155` |
| G22 | recovery-fallback, internal-trusted-api | 4 | `internal` | `Missing load completion` | `cmd/planar/invocation.cpp:488` |
| G23 | profile-capture-and-schema, registration_error, extension_execution_registration | 3 | `schema` | `Invalid external profile structure` | `external-profile-contract.md:531` |
| G24 | profile-capture-and-schema, registration_error, extension_execution_registration | 3 | `unsupported_version` | `Unsupported external profile version` | `external-profile-contract.md:532` |
| G25 | profile-capture-and-schema, registration_error, extension_execution_registration | 3 | `path` | `Invalid external module path` | `external-profile-contract.md:533` |
| G26 | profile-capture-and-schema, registration_error, extension_execution_registration | 3 | `unsafe_path` | `External profile overlaps a supplied input` | `external-profile-contract.md:534` |
| G27 | registration_error, extension_execution_registration | 3 | `reference` | `External registration identity conflict` | `external-profile-contract.md:535` |
| G28 | profile-capture-and-schema, registration_error, extension_execution_registration | 3 | `capture_changed` | `Captured external profile changed` | `external-profile-contract.md:536` |
| G29 | registration_error, extension_preparation_resource, profile_preparation_failure, extension_execution_registration | 3 | `resource_limit` | `External profile resource limit exceeded` | `external-profile-contract.md:537` |
| G30 | registration_error, extension_preparation_protocol, profile_preparation_failure, internal-trusted-api, extension_execution_registration | 4 | `internal` | `Invalid external profile protocol` | `external-profile-contract.md:538`; `engine/runtime/runtime.cpp:1248` |
| G31 | internal-trusted-api | 4 | `internal` | `Invalid embedded command protocol` | `cmd/planar/response.cpp:41` |
| G32 | extension_preparation_resource, profile_preparation_failure, extension_execution_registration | 3 | `resource_limit` | `Lua stack exceeded` | `engine/runtime/runtime.cpp:1196` |
| G33 | extension_preparation_resource, profile_preparation_failure, extension_execution_registration | 3 | `resource_limit` | `Lua instruction limit exceeded` | `engine/runtime/runtime.cpp:1198` |
| G34 | extension_preparation_resource, profile_preparation_failure, extension_execution_registration | 3 | `resource_limit` | `Lua allocation exceeded` | `engine/runtime/runtime.cpp:1199` |
| G35 | profile_preparation_semantic, extension_execution_assembly | by code (failed()) | `*` | `*` (the exact trusted inner E, raw-validated; no Planar-authored message) | `engine/runtime/runtime.cppm:137`, `:417-419` |
| G36 | native-capture-passthrough | by code (failed()) | `*` | `*` (existing propagated native capture and render errors keep their exact messages) | `external-profile-contract.md` X-LIMIT preamble (existing propagated native errors keep their exact messages); `engine/capture/native.cpp` |
| G37 | ledger-format-error, extension_execution_ledger | 3 | `schema` | `Invalid ledger arguments` | `cmd/planar/response.cpp:88` |
| G38 | ledger-format-error, extension_execution_ledger | 3 | `path_denied` | `Ledger path requires existing real parent directories and a valid target` | `cmd/planar/response.cpp:92` |
| G39 | ledger-format-error, extension_execution_ledger | 3 | `ledger_exists` | `Ledger target already exists` | `cmd/planar/response.cpp:96` |
| G40 | ledger-format-error, extension_execution_ledger, early-replay-and-load-prelude | 3 | `ledger_not_found` | `Ledger was not found` | `cmd/planar/response.cpp:100` |
| G41 | ledger-format-error, extension_execution_ledger | 3 | `foreign_ledger` | `File is not a Planar ledger` | `cmd/planar/response.cpp:104` |
| G42 | ledger-format-error, extension_execution_ledger | 3 | `unsupported_ledger_version` | `Ledger version or state is unsupported` | `cmd/planar/response.cpp:108`, `:112` |
| G43 | ledger-format-error, extension_execution_ledger | 4 | `ledger_corrupt` | `Ledger integrity verification failed` | `cmd/planar/response.cpp:116` |
| G44 | ledger-format-error, extension_execution_ledger | 4 | `ledger_busy` | `Ledger is busy` | `cmd/planar/response.cpp:120` |
| G45 | ledger-format-error, extension_execution_ledger | 4 | `storage_full` | `Ledger storage is full` | `cmd/planar/response.cpp:124` |
| G46 | ledger-format-error, extension_execution_ledger | 4 | `io` | `Ledger operation failed` | `cmd/planar/response.cpp:128` |
| G47 | ledger-format-error, recovery-fallback | 4 | `io` | `Ledger may be published; inspect the explicit target before retrying` | `cmd/planar/response.cpp:132` |
| G48 | ledger-format-error, recovery-fallback | 4 | `io` | `Explicit ledger recovery is required` | `cmd/planar/response.cpp:136` |
| G49 | ledger-format-error, extension_execution_ledger | 3 | `hash_collision` | `Ledger record identity collision` | `cmd/planar/response.cpp:140` |
| G50 | ledger-format-error, early-replay-and-load-prelude | 3 | `request_conflict` | `Ledger request conflicts with its saved intent` | `cmd/planar/response.cpp:144`; early conflict `cmd/planar/invocation.cpp:461` |
| G51 | ledger-format-error, extension_execution_ledger | 3 | `revision_conflict` | `Ledger revision does not match the expected revision` | `cmd/planar/response.cpp:148` |
| G52 | ledger-format-error, extension_execution_ledger | 3 | `capture_changed` | `Captured declaration inputs changed` | `cmd/planar/response.cpp:152` |
| G53 | extension_execution_history | 3 | `revision_not_found` | `Recorded history view is unavailable` | `cmd/planar/invocation.cpp:277` |
| G54 | extension_execution_history | 3 | `request_not_found` | `Recorded history view is unavailable` | `cmd/planar/invocation.cpp:277` |
| G55 | extension_execution_history | 3 | `event_not_found` | `Recorded history view is unavailable` | `cmd/planar/invocation.cpp:277` |
| G56 | extension_execution_history | 3 | `unsupported_interpretation` | `Recorded history view is unavailable` | `cmd/planar/invocation.cpp:277` |
| G57 | export-error-map | 3 | `revision_not_found` | `Recorded revision was not found` | `cmd/planar/invocation.cpp:65` |
| G58 | export-error-map | 3 | `schema` | `Invalid export arguments` | `cmd/planar/invocation.cpp:69` |
| G59 | export-error-map | 3 | `path_denied` | `Export path requires existing real parent directories and a valid target` | `cmd/planar/invocation.cpp:73` |
| G60 | export-error-map | 3 | `target_exists` | `Export target already exists` | `cmd/planar/invocation.cpp:77` |
| G61 | export-error-map | 4 | `export_incomplete` | `Export staging cleanup is incomplete; inspect the sibling location in the explicit target parent before taking action` | `cmd/planar/invocation.cpp:81-82`; exit forced 4 at `:479-481` |
| G62 | export-error-map | 4 | `io` | `Export operation failed` | `cmd/planar/invocation.cpp:86` |
| G63 | export-error-map, internal-trusted-api | 4 | `internal` | `Invalid export cleanup location` | `cmd/planar/invocation.cpp:92` |
| G64 | recovery-fallback | 4 | `io` | `Export may be complete; inspect the explicit target before retrying` | `cmd/planar/invocation.cpp:29` |
| G65 | recovery-fallback | 4 | `io` | `Load may be complete; retry the same request ID or inspect request history` | `cmd/planar/invocation.cpp:35` |
| G66 | stdout-failure | 4 | `io` | `Cannot write stdout` | `cmd/planar/response.cpp:183` |
| G67 | recovery-fallback | 4 | `-` | `-` (empty stderr; unrecoverable allocation `{4, "", ""}`) | `cmd/planar/response.cpp:171`, `:187`; `cmd/planar/invocation.cpp:430`, `:437`, `:505` |
| G68 | help-ceiling | 3 | `resource_limit` | `External profile resource limit exceeded` | `external-profile-contract.md:537` (new bound; selected help buffered within 1 MiB, X-CLI help paragraph) |
| G69 | response-ceiling | 3 | `resource_limit` | `JSON codec refused input` | `cmd/planar/invocation.cpp:494-499` (maximum rewritten to 16777216); `engine/json_codec/codec.cpp:359` |

G35 and D7-7 differ where `failed()` (`cmd/planar/response.cpp:162-170`) does not give exit 3: D7-7 (`remaining-steps-design.md:261`) says `profile_preparation_error.semantic` "maps to the existing typed E with exit 3", while `failed()` gives a semantic E exit 2 when its code is `usage` and exit 4 when it is `io`, `internal`, `ledger_corrupt`, `ledger_busy` or `storage_full`. By reading only (nothing was run), a semantic E can carry such codes: the trusted `check_shape` returns `internal` for an invalid shape result (`src/stdlib/planar/semantic.lua:374`), which the contract makes exit 4 (`external-profile-contract.md:521`), and it passes through any nonempty code from a shape handler's `ok=false` error (`semantic.lua:384-400`), which under a profile can be a custom shape, so a semantic `usage` or `io` E appears possible. Operator decision Q-G35 A (2026-10-28; row A-13; round record): map by code. A profile-preparation semantic E keeps today's `failed()` mapping: `usage` exits 2; `io`, `internal`, `ledger_corrupt`, `ledger_busy` and `storage_full` exit 4; every other code exits 3. This is the existing exit policy for user semantic refusals (`command-contract.md:147`). D7-7's literal "exit 3" (`remaining-steps-design.md:261`) is superseded on this point by the operator decision and holds only for the codes `failed()` maps to 3. X07 implements no other exit for these codes. X07 search still confirms which codes are reachable, for its tests; that does not change the mapping.

## 3. Decisions

Each Status is exactly one of `contract-decided`, `design`, `operator` or `new`. No row is `new`. The first table restates design rows D7-1 to D7-13 of the [remaining-steps design](remaining-steps-design.md#design-decisions-implied). The second table lists the addendum's own choices, A-01 to A-13. Q-INIT is recorded in D7-2, A-07 and the 1.2 init row. Q-G35 is recorded in A-13 and the note under the 2.5 table.

| ID | Choice | Status | Citation |
| --- | --- | --- | --- |
| D7-1 | One closed factory over the five-member `command_options` variant admits and dispatches render, init, history, export and load under a profile, rather than one factory per family or refusal (sections 1.1 to 1.5). | operator | Q1 B; [remaining-steps design, D7-1](remaining-steps-design.md#design-decisions-implied); option C (refuse) rejected |
| D7-2 | Known slots per family, as in 1.2. Render, load, history, export and selected help are contract-decided by X-CAP. The init cell is decided by Q-INIT A (2026-10-28), stated in full in the 1.2 init row: alias-only, no containment refusal. | operator | [X-CAP](external-profile-contract.md#x-cap--strict-capture-and-trust) (`:159-175`); X-STORE `:647-650`; Q-INIT A, 2026-10-28. The init cell has no containment refusal: the ledger target gets the selected-file alias check only. |
| D7-3 | Expected production outcome per X01 row: production exit, precedence class, profile-accessed flag and governing clause, for all 44 envelope rows (2.2) and 12 opaque rows (2.3). Spike labels are translated. | contract-decided | [X-CLI steps 1 to 8 and the exit paragraph](external-profile-contract.md#x-cli--envelope-and-admission-order); the table requirement is design |
| D7-4 | The envelope code lives in a new `src/cmd/planar/envelope.{cppm,cpp}` module, imported only by bootstrap. | design | D7-4; [implementation path](external-profile-implementation.md#source-seams-and-bounded-implementation-rounds) (`Do not broaden ownership ad hoc`) |
| D7-5 | "External Lua did not run" is observed without hooks: an unbounded chunk-level profile module gives `resource_limit` if it runs. Registration counts come from `profile_invocation::observe().registrations` (`runtime.cppm:409`). | design | D7-5; [XD3](remaining-steps-design.md#cross-cutting-decisions) |
| D7-6 | The embedded load request codec is chosen by the ledger format opened in the prelude: format1 uses request_v1, format2 uses request_v2 with the embedded selector. | contract-decided | [X-STORE format table](external-profile-contract.md#x-store--byte-codecs-exact-provenance-and-supported-formats) rows 1 and 3 |
| D7-7 | Error mapping goes through one adapter in `src/cmd/planar/response.*`. Codes and exit codes are as in 2.5. Message selection for usage is row A-04. | contract-decided | [X-CLI exit paragraph](external-profile-contract.md#x-cli--envelope-and-admission-order); [X-LIMIT](external-profile-contract.md#x-limit--cumulative-ceilings-and-errors) |
| D7-8 | The budget owner charges late-slot admission, role capture, history decode into the VM and projection transport. CLI11 objects, the cwd and environment snapshot, request framing, the early replay read, selected help (1 MiB) and response encoding (16 MiB) are inherited own-bound stages, checked before allocation. | operator | Q2 A; D7-8; section 5 (N30-1 to N30-4) |
| D7-9 | Races and failures after commit are tested through generated CLI variant binaries with compile-time pause and fault points, plus one real two-process CAS race. There are no product hooks. | operator | Q3 A; D7-9; [XD3](remaining-steps-design.md#cross-cutting-decisions) |
| D7-10 | Test placement. Catch2 parser matrix in `src/cmd/planar/bootstrap.t.cpp`; Python process suites in `tests/acceptance/`; fixture generators in `tests/native/v5-02-x07-support`; evidence in `tests/native/v5-02-x07-evidence` (gitignored). Confirmed reading: `tests/native/` holds gitignored `*-evidence/` folders and `*-support/` harness folders that CMake registers (`src/cmd/planar/CMakeLists.txt:107-111`, `tests/native/v5-02-c-support/cli.py`); the older per-family suites are `tests/acceptance/bootstrap_*.py`, with `bootstrap_load.py` registered at `:156-158`. | design | Confirmed reading (`src/cmd/planar/CMakeLists.txt:107-111` and `:156-158`); [remaining-steps design, D7-10](remaining-steps-design.md#design-decisions-implied); tests/native and tests/acceptance placement |
| D7-11 | Test names are kept. Existing tests whose assertions the cutover changes, found by reading only, are candidates that X07 search must confirm, each with clause 24 or 25. The exported `command_vm::invoke` refusal stays (1.1), so the X05 A3 profile-owner invoke refusal test (`external_registration.t.cpp` line 308) is not a candidate. The registration-time refusal assertions at lines 177, 461, 477 and 543 of that file are not changed by clause 26 or by any invoke path and are dropped from the candidate list. Line 2763 is also dropped: it is not in `X05 C2 B2 maximum section text and shared noncyclic sections remain valid` (that test runs from 2703 to 2718) but in the anonymous-namespace helper `extension_usage` (2758-2765), which asserts the runtime-level `prepare_extension_invocation` refusal `usage` / `Invalid extension invocation` (a runtime string that is never printed by the public CLI); X07 maps that refusal to the public usage bytes in the CLI adapter (A-04) and does not change the runtime value, so no clause changes that assertion. Remaining candidates: `tests/acceptance/bootstrap_init.py` (public init assertions that assume format1, for example the ledger_initialized event version near line 109), candidate, clause 24; `tests/acceptance/bootstrap_load.py` and the `planar_load_cli_probe` fixtures (v1 embedded request bytes through the public prelude), candidate, clause 25. Every name is kept (XD5). | design | D7-11; XD5; candidate; X07 search confirms |
| D7-12 | Creation-site audit: one creation site for profile VMs (the closed factory), the pre-X07 sites in 1.6, and a Catch2 source-scan test under `src/cmd/` that forbids `extension_invocation::create` and `command_vm::create(` with a profile argument, in the X06-C style. | design | D7-12; [XD1](remaining-steps-design.md#cross-cutting-decisions) |
| D7-13 | Two waves. Wave A covers the envelope, probes, final parser, selected help, the error adapter and the closed factory with admission (clauses 1 to 13, 27, 28). Wave B covers the extension handler, load and replay wiring, format2 init, the request codec, races, faults and budget threading (clauses 14 to 26, 29, 30). Wave B test coders start after wave A stubs compile. | design | D7-13; Q-WAVE reconciliation (A-08); round record objective (part 1) |

| ID | Choice | Status | Citation |
| --- | --- | --- | --- |
| A-01 | The closed factory is a static member family of `profile_invocation` in `planar.engine.runtime`: no new module, no new exported constructor, one creation site. Its inputs are per arm (1.1, 1.3): a selector overload for render, init, history and export, which refuses a `command_load_options` alternative before capture, and a load overload that takes the one `request_v2` intent framed at X-CLI step 4 and used at step 5 plus the native ledger path, with no selector argument and no second framing. Init, history and export invoke their shipped callables through a module-private invoke path of `planar.engine.runtime`, reachable only through the closed factory; the exported `command_vm::invoke` keeps refusing profile owners and no exported signature changes (1.1, 1.7). | design | [architecture constructor protocol](profile-preparation-architecture.md#constructor-parse-and-consumer-protocol) (`:157-161`, `:233-247`; load-arm input `:205-208`); invoke path: architecture `:46` (generic `invoke` explicitly refuses profiles) and `runtime.cppm:221-225` (generic command invocation refuses); D7-1; D7-12 |
| A-02 | The load arm routes to `open_known_load`, then `prepare_load`, then the exported `complete_acceptance(prepared_profile)` overload. | operator | Q1 B ("Load keeps"); `src/engine/ledger/profile_acceptance.cppm:21` |
| A-03 | Class vocabulary SYN, PFX, PRB, NRM, ERL, KSA, PRF, FIN, LAT, HND, HLP. NRM, ERL, KSA and HLP name X-CLI steps 4, 5, 6 and the help paragraph. | design | D7-3 extension; X-CLI steps 4, 5, 6 and the help paragraph |
| A-04 | Usage message: every exit-2 usage, including `extension_preparation_failure::usage`, prints the public `detail::usage` bytes `Expected command options or --help; see --help` (`response.cpp:175`). The runtime string `Invalid extension invocation` (`runtime.cpp:4016`) is never printed. X07 must not add a public message: the X07 must-not rule forbids a new public error code or message, and round acceptance 3 requires it. | design | `response.cpp:175`; X07 must-not rule in the [remaining-steps design](remaining-steps-design.md#x07-public-cutover-one-round-two-waves); round record acceptance 3 |
| A-05 | One error adapter in `src/cmd/planar/response.*`. | design | D7-7 placement |
| A-06 | Envelope module `src/cmd/planar/envelope.{cppm,cpp}`, imported only by bootstrap; handlers never import it, so there is no handler-to-bootstrap cycle. | design | D7-4; [implementation path](external-profile-implementation.md#source-seams-and-bounded-implementation-rounds) |
| A-07 | Init ledger target: selected-file alias check only, with no containment refusal (Q-INIT A, 2026-10-28; full statement in the 1.2 init row and D7-2). | operator | Q-INIT A (2026-10-28); [X-CAP](external-profile-contract.md#x-cap--strict-capture-and-trust); X-STORE `:647-650` |
| A-08 | Q-WAVE reconciliation: clauses 24 to 26 in wave B, clauses 27 and 28 in wave A (section 1.9). | design | D7-13; round record objective (part 1); [implementation path X07-B row](external-profile-implementation.md#source-seams-and-bounded-implementation-rounds) (`external-profile-implementation.md:55`) |
| A-09 | KSA derivation for E02, E35 and O07: step-6 known-slot admission refusals with `ledger_not_found`, exit 3, profile accessed, registration not run (section 2.4). | contract-decided | [X-CLI step 6](external-profile-contract.md#x-cli--envelope-and-admission-order); [X-CAP](external-profile-contract.md#x-cap--strict-capture-and-trust) `:172`, `:179-182`, `:195` |
| A-10 | Fixture layout: trusted sibling directory `trusted/`, invocation cwd `work/`, selectors `../trusted/p.json` and `../trusted/q.json`, other paths nonexistent relative to `work/` (section 2.1). | design | Fixture-layout fix of 2026-10-28 made by the round author; no operator decision is recorded for it (records fix V5-02-X07-DOC-R4); [X-CAP](external-profile-contract.md#x-cap--strict-capture-and-trust) `:169-171` |
| A-11 | Substitution evidence per family (section 1.5): retained phase callables for render and load; test-bundle handler replacement under with-profile through the closed factory for init, history and export. | design | [command contract, bootstrap section](command-contract.md#bootstrap-errors-and-retained-lifetime) (`command-contract.md:117-121`) |
| A-12 | Diagnostic table granularity: one row per existing code and message pair, G01 to G69 (section 2.5). | design | D7-7; [X-LIMIT](external-profile-contract.md#x-limit--cumulative-ceilings-and-errors) |
| A-13 | Exit of a profile-preparation semantic error (G35): map by code through `failed()` (`cmd/planar/response.cpp:162-170`): `usage` exits 2; `io`, `internal`, `ledger_corrupt`, `ledger_busy` and `storage_full` exit 4; other codes exit 3. D7-7's literal "exit 3" (`remaining-steps-design.md:261`) is superseded on this point. | operator | Q-G35 A, 2026-10-28; [command contract](command-contract.md#bootstrap-errors-and-retained-lifetime) (`command-contract.md:147`, semantic refusals keep the existing exit policy) |

## 4. Carried limits

The "Roadmap classification" cells are quoted byte for byte from the Carried limits row of the [roadmap](roadmap.md#next-work). X07 repairs none of these limits and reclassifies none of them. The relation cells say how the X07 path touches each one.

| ID | Roadmap classification (unchanged) | X07 relation |
| --- | --- | --- |
| CL-1 | `call_retained_projection` (`runtime.cpp:2016-2022`, exhaustion test at 2019, `bridge_error` at 2021): REACHABLE; untrusted callers `invoke_projection` (4498) and the extension projection callback (6335-6338). | X07 repairs none. Added reach: X07 may make `call_retained_projection` reachable from more entry points, namely the extension dispatch of wave B (clauses 18 and 19) and render under a profile (1.4). Classification unchanged; no exhaustion-first order changes. |
| CL-2 | registration module loop in `register_profile` (`runtime.cpp:3503-3508`, exhaustion at 3505): REACHABLE; untrusted caller is the profile module chunk. | X07 repairs none. The factory registers through the existing registration path, so the classification applies unchanged to the module loop. |
| CL-3 | `setup_commands`/`seal_environment` frames (`runtime.cpp:4219-4222` and `4235-4238`): NOT REACHABLE by untrusted callers (trusted environment-setup natives only). | X07 repairs none. X07 does not change the `setup_commands` or `seal_environment` frames. |
| CL-4 | non-nested `phase()` pcall in `validate_profile_phases` (`runtime.cpp:4713`): PROBABLY REACHABLE, not exercised; untrusted caller is a public `validate_profile`, `select_profile` or `render_profile` call on a profile-snapshot command VM. | X07 repairs none. Added reach: the factory calls `render_profile` on the retained profile VM under with-profile (1.4), which is one of the public calls the roadmap names as the untrusted caller. Classification PROBABLY REACHABLE and not exercised is unchanged. |
| CL-5 | command-service response in `command_vm::invoke` (`runtime.cpp:4888-4891`): NOT REACHABLE by untrusted callers (profile VMs refuse invoke at 4844-4845). | X07 repairs none. Added reach: the roadmap basis is that profile VMs refuse invoke at 4844-4845. Under Q1 B that basis changes. The exported `command_vm::invoke` keeps its refusal, but profile VMs gain a module-private invoke path behind the closed factory for init, history and export (1.1). If that path shares the command-service response handling at `runtime.cpp:4888-4891`, that code becomes reachable on profile VMs. This is recorded as a fact without reclassifying: the classification is not changed and not repaired here, and any reclassification is for the coordinator. |
| CL-6 | catch-all fallbacks (`runtime.cpp:4298`, 4442, 4549, 4632, 6182): reachable only on host `bad_alloc` or `length_error`, not injectable. | X07 repairs none. The design adds no catch-all fallback; the code round must record any new site it adds. |
| L-P01 | P01 survives (masked) | X07 repairs none. The X07 brief carries P01 as a carried limit; the gate does not rely on it. |
| L-N1 | N1 phase coverage unmeasured | X07 repairs none. The phase-coverage measurement is not part of the X07 gate unless the code round adds it. |
| L-531 | #531 timing test about 15.5 s of 30 s | X07 repairs none and must not slow #531 (brief input P12). A new test never joins #531. The code round records the #531 time. |
| L-RED | red snapshot tracked-only | X07 repairs none. The X07 code round saves `git diff HEAD` plus untracked bytes before its red run (brief input P5). This addendum round has no product red snapshot. |
| L-LINUX | Linux not run | X07 repairs none and claims no Linux acceptance. X08 owns the Linux gate. The optional non-gating Linux probe (Q6 A) is evidence only. |
| L-CEIL | ceiling pins: the instruction-ceiling equality pin was replaced by a derivation in C04 | X07 repairs none. X07 threads the existing budget owner (clause 30) and changes no ceiling. The registration pin `budget_registration_instruction_max == 1000000` must change only with a deliberate ceiling change recorded in the round. |

## 5. X07 brief inputs

Each input is a requirement the X07 code brief must carry. The standing pre-emptions P1 to P12 are the rows of the [standing pre-emptions table](remaining-steps-design.md#standing-pre-emptions-from-past-reviews).

| ID | Requirement | Source |
| --- | --- | --- |
| P1 | Narrowing risk: every clause marked narrowing risk gets one invariant row per stage it covers, and a coder never narrows scope. | [P1](remaining-steps-design.md#standing-pre-emptions-from-past-reviews) |
| P2 | Ordering clauses each get their own row; the mutant moves the check after the action, and the test observes the order, not only the outcome. | [P2](remaining-steps-design.md#standing-pre-emptions-from-past-reviews) |
| P3 | The gate runner writes the gate order into `REPORT.md`, and every step has a raw log and an exit file. | [P3](remaining-steps-design.md#standing-pre-emptions-from-past-reviews) |
| P4 | Process gates carry "mutant: not applicable" with their evidence file; the prober's named killer equals the table's test. | [P4](remaining-steps-design.md#standing-pre-emptions-from-past-reviews) |
| P5 | The red runner saves `git diff HEAD` plus the bytes or per-file hashes of every untracked candidate file into `red/` before it runs. | [P5](remaining-steps-design.md#standing-pre-emptions-from-past-reviews) |
| P6 | The prober rebuilds every generated variant test target on each row, or after each restore. | [P6](remaining-steps-design.md#standing-pre-emptions-from-past-reviews) |
| P7 | No new friend, public field or hook exists only for tests unless the round record allows it; otherwise the limit is reported. | [P7](remaining-steps-design.md#standing-pre-emptions-from-past-reviews) |
| P8 | Any "cannot be called or named from X" claim needs negative-compile probes with the exact flags of the consuming CLI target; deliberate adversarial C++ is a recorded limit under XD1. | [P8](remaining-steps-design.md#standing-pre-emptions-from-past-reviews) |
| P9 | Test-only mechanisms stay in test targets; product configures never depend on test text. | [P9](remaining-steps-design.md#standing-pre-emptions-from-past-reviews) |
| P10 | Each evidence folder has a `README.md` that names the final identity and lists every superseded statement. | [P10](remaining-steps-design.md#standing-pre-emptions-from-past-reviews) |
| P11 | Machine-local paths are redacted before the commit step, and all committed files are checked. | [P11](remaining-steps-design.md#standing-pre-emptions-from-past-reviews) |
| P12 | Budget-sensitive tests use small injected ceilings; a new test never joins #531; the round records the #531 time. | [P12](remaining-steps-design.md#standing-pre-emptions-from-past-reviews) |
| N30-1 | Clause 30 narrowing risk, late-slot admission: `extension_invocation::admit` late slots must charge the factory budget owner, not their own bound. | [clause 30](remaining-steps-design.md#x07-public-cutover-one-round-two-waves); D7-8; Q2 A |
| N30-2 | Clause 30 narrowing risk, role capture: workspace, source, base and observation capture must charge the same owner. | [clause 30](remaining-steps-design.md#x07-public-cutover-one-round-two-waves); D7-8; Q2 A |
| N30-3 | Clause 30 narrowing risk, history decode into the VM: the decode must charge the same owner. | [clause 30](remaining-steps-design.md#x07-public-cutover-one-round-two-waves); D7-8; Q2 A |
| N30-4 | Clause 30 narrowing risk, projection after `into_extension`: projection calls must charge the same owner. | [clause 30](remaining-steps-design.md#x07-public-cutover-one-round-two-waves); D7-8; Q2 A |
| RH | Root `--help` bytes are unchanged. Reviewers must not ask root help to advertise `with-profile`. | [X-CLI help paragraph](external-profile-contract.md#x-cli--envelope-and-admission-order); X07 must-not list in the [remaining-steps design](remaining-steps-design.md#x07-public-cutover-one-round-two-waves) |
| RS | Round shape: one addendum run (this round), then one code round of two waves, A then B. Split before coding if the candidate is too large. | [suggested round shape](remaining-steps-design.md#x07-public-cutover-one-round-two-waves); Q4 A |

The inherited own-bound stages are listed separately from the owner's stages. CLI11 parser objects, the cwd and environment snapshot, request framing, the early read-only replay, selected-help rendering (1 MiB) and final response encoding (16 MiB) keep their own bounds, checked before allocation (D7-8; Q2 A).

The non-goals for X07 are: init stays exact (Q10 B: no prompts and no scaffolding); no format3 is introduced by X07 (Q13 A); no new public code or message; root help is unchanged; and Linux is not claimed, because X08 owns the Linux gate.
