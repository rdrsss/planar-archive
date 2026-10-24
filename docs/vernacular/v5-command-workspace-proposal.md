# V5-02 proposed command, trust and workspace contracts

**PROPOSED — pending operator decisions and independent review.** This document
changes no supported command, exact contract or implementation. The checkpoint is
approved R14-D2: render/init/history/export exist; load service/CLI do not.
[V5 alignment](v5-alignment.md#authority-and-current-evidence) records authority.
[V5](planar-design-spec-v5.md#engine-surface-and-sandbox) supplies direction;
[declaration v1](declarations.md), [render v1](render-contract.md) and
[ledger v1](ledger-contract.md) remain authoritative until reviewed amendments.

## 1. Current behavior and recommended boundary

| Current evidence | Recommendation requiring amendment |
| --- | --- |
| [Bootstrap](../../src/cmd/planar/bootstrap.cpp) attaches four CLI11 handler families, retains option state and checks parsed metadata. | Preserve that pattern. Trusted Lua supplies validated command descriptors and semantic callables; native families own CLI11 attachment and capability orchestration. |
| [Runtime](../../src/engine/runtime/runtime.cpp) invokes fixed exports from embedded Lua in a fresh bounded VM. | Introduce a pure registration export and a sealed callable registry; no job-tier command registration. |
| [Provenance](../../src/engine/history/provenance.cpp) and [assembly codec](../../src/engine/history/provenance_codec.cpp) pin ordered originals, assembly and manifest. | Preserve embedded v1. External profiles require separately versioned effective-bundle provenance and interpretation. |
| [Request record](../../src/engine/history/record.cppm) carries operation/actor/expected/workspace/registry only. | Keep request_v1 unchanged. A later request_v2 carries explicit extension selector intent; never slip it into actor or workspace fields. |
| [Prepared acceptance](../../src/engine/acceptance/prepared.cppm) seals native capture receipts and immutable interpretation. | Retain opaque receipts and final native verification; Lua cannot fabricate acceptance evidence. |
| [Render invocation](render-contract.md#invocation-and-trust) accepts explicit workspace files, including cwd-relative paths. | Add unambiguous bare names, with no discovery; retain explicit relative files and existing confinement. |
| [Init](../../src/cmd/planar/handlers/init/command.cpp) only initializes a new explicit ledger. | Keep it. RA01 separately adds explicit scaffold/prompt mode; no implicit ledger creation. |

Lua must own real policy: required semantic fields, normalization, supported
shapes, dependency interpretation, selection and projections, plus which semantic
handler implements each registered verb. Native code owns argument grammar and
coercion, capability issuance, capture receipts, canonical codecs, exact-byte
collision refusal, immutable recorded reads and atomic conditional storage.
Native validation of transaction invariants does not prove Lua policy correct.
No generic SQL, database handle or unchecked `ledger.commit(expected, changes)`
is exposed. [V5's conceptual signatures](planar-design-spec-v5.md#engine-surface-and-sandbox)
are direction, not callable APIs introduced by this proposal.

## 2. Registration and dispatch contract

Proposed `command_catalog_v1` is a bounded data result from trusted registration.
Each entry has `name`, `family`, `handler_id`, `options`, `help`, `response_kind`
and `capabilities`. Unknown fields fail. Names use `[a-z][a-z0-9_-]{0,63}`;
handler IDs are namespaced IDs of at most 128 ASCII bytes. There are no aliases,
positionals, short flags, optional option values or repeated options in v1.
Options have `name`, `type` (`text`, `decimal`, `flag`), `required` and bounded
help text. Every scalar takes exactly one separate token; flags take none.
CLI11 performs parsing/coercion; Lua receives typed values plus presence bits,
never argv. Decimal lexical restrictions remain native CLI11 validators, matching
current canonical positive-decimal rules where required.

Illustrative registration data, not a supported Lua constructor:

```lua
{ version = 1, commands = {
  { name = "load", family = "load", handler_id = "shipped.load.v1",
    options = { {name="--workspace", type="text", required=true}, ... },
    help = "Accept the declaration closure", response_kind = "load_v1",
    capabilities = {"accept_declaration_v1"} }
} }
```

Registration is pure: it can create local tables/functions and register entries,
but cannot read captures, obtain capabilities, emit output or mutate storage.
The engine validates and freezes the entire registry atomically; one bad entry
rejects it all. Duplicate verb/option/handler/shape/projection IDs fail, including
collisions with shipped IDs. No overrides, file-order shadowing or last-wins
behavior. Bootstrap names `with-profile` and `--help` are reserved.

A descriptor's callable is resolved from the same captured bundle in the same
registration VM, held by a native-owned registry reference. Seal descriptor
canonical bytes, handler ID, owner module logical path and effective-bundle
digest together. Never look up an arbitrary current global by handler name after
validation. Execution uses this retained VM under a fresh execution budget and
read-only registration environment; no module reload or replacement between
parse and dispatch. Registration state is invocation-local, never global.

`handlers/<family>/command.cppm` and `command.cpp` remain real owners of retained
CLI11 option state and typed family orchestration. They attach validated
catalog options, convert parsed bindings to native typed invocation values,
invoke the sealed Lua semantic handler and service its declared operations.
For example, Lua's load handler selects normalization/validation and assembles
the interpretation; native acceptance orchestration performs replay, capture,
receipt verification and commit. A family is not permission to call arbitrary
other family services. Initial external verbs receive projection/read-only
capabilities only; the existing acceptance capability stays sealed to shipped
load. Later extension mutation/execution requires an explicit typed ABI gate,
not access to current native command functions. Native attach code must reject unsupported descriptor
families/capabilities. Lua registration is therefore consequential, not a help
catalog over a second native policy dispatcher.

Proposed native binding vocabulary (signatures need the scoped ABI amendment):

| Binding / values | Native boundary |
| --- | --- |
| `registration.add(descriptor, callable)` | Registration VM only; frozen after validation; no host services. |
| `context.capture_value(logical_root, portable_path)` | Read only an already sealed invocation capture; no host path/open/glob discovery. |
| `context.hash(value)` / `hash_bytes(bytes)` | Existing versioned canonical encoding/hash and bounds; no policy authority. |
| `context.read_recorded(selector)` | Read-only verified records from the family-issued ledger capability; no SQL. |
| `context.accept_declaration(interpretation)` | Load-family capability only, after native replay/capture prelude; verifies structured interpretation and retained receipts before native CAS/commit. |
| `context.project(projection_id, value)` | Sealed registered projection over bounded values; result returned to native response encoder. |

Native acceptance freezes Lua output before entering a write transaction. No
Lua callback, projection or handler runs while the native write lock is held;
replay lookup precedes candidate inspection as well as capture and CAS.

Capability objects are unforgeable invocation-local handles, never serializable
values or declaration fields. Native operations return typed success/failure to
Lua; errors cannot become unchecked receipts. No general read/write/exec binding
is added. Future finite execution services require their own typed ABI and
native evidence checks; an external verb cannot implement them with raw IO.

For shipped catalog v1, each command's structural option signature is pinned
by its native family ABI. Lua cannot remove request IDs or expected-head fields
from load. Adding a new family, capability or signature version needs a scoped
native implementation/review. Existing current semantic Lua exports can be
adapted incrementally, without duplicating their policy in C++.

Shapes and projections use separate versioned registries in the same sealed
bundle: shape ID plus bounded semantic callable; projection ID plus allowed
input kind and response kind. Job declarations select approved shape IDs and
receive curated constructors only. They never receive registry mutation,
trusted function references, capability objects or host paths. Future new shape
schemas/interpretation structures need their own contract review; the registry
does not silently widen declaration v1.

## 3. Explicit extension selection and CLI11 bootstrap

Recommend embedded-only first load integration, followed by an explicit external
extension gate. This stages [v5's stdlib.d requirement](planar-design-spec-v5.md#engine-surface-and-sandbox);
it does not remove user shapes/projections/verbs or permanently reduce their
trust. Shipped and selected external Lua ultimately use the same invariant-
enforced primitives. Finite execution bindings arrive with their execution
rounds; neither tier gets arbitrary subprocess or ledger mutation now.

No `HOME`, config, workspace-relative stdlib directory, executable search path
or ambient `stdlib.d/` scan selects code. Recommend an explicit driver-selected
profile file as the only external selector, through a reserved native envelope:

```sh
# Supported today; retains its spelling after the embedded bridge.
planar render --workspace ./workspace.lua --registry ./registry.json \
  --root source=/scratch/view --job combined --role builder
# Proposed later; not supported today.
planar with-profile --stdlib-profile /trusted/billing-profile.json \
  render --workspace billing --registry /trusted/registry.json \
  --root source=/scratch/view --job combined --role builder
planar with-profile --stdlib-profile /trusted/billing-profile.json \
  inspect-billing --workspace billing
```

This costs an explicit envelope for extensions; it avoids finding a profile
option inside an unknown verb's values or evaluating code to discover how to
select that code. Direct commands always use embedded Lua. A workspace cannot
opt itself into a profile. `--stdlib-profile` after the verb is not a selector.

Concrete two-stage parser recommendation:

1. The static native CLI11 root recognizes direct shipped commands and the
   `with-profile` bootstrap envelope. That envelope requires exactly one
   `--stdlib-profile FILE`, and uses CLI11 `prefix_command()` so the first
   unrecognized token starts an opaque tail. CLI11 retains that tail through
   `remaining_for_passthrough()`. No hand scan/split recognizes options.
2. Validate phase-one consumed-token counts from CLI11 metadata. Require a
   nonempty tail; duplicate/missing profile or extra prefix flags fail usage.
   The sole current `--name=value` syntax guard still applies to the whole
   invocation. No positional terminator or extra token acquires meaning.
3. Parse the tail using an embedded command catalog probe. A recognized shipped
   command must parse completely against its fixed signature; errors cannot
   fall back to extension parsing. For load, perform the early request lookup
   described below before opening/evaluating the profile. Shipped signatures
   and names cannot be overridden by extensions.
4. For a non-replay invocation, capture/evaluate the selected profile and seal
   its registry. Parse the original tail afresh with CLI11 against the sealed
   catalog, with extras/fallthrough disabled and exact metadata accounting.
   A genuinely unknown verb fails usage. Retain all option state through dispatch.

The probe uses CLI11 subcommand selection metadata; an unknown tail is opaque
until the final parse. It does not interpret unknown option values. The pinned
[CLI11 documentation](../../vendor/cli11/9d6a/README.md) describes prefix-command
and passthrough APIs. An implementation spike must prove ordered tail retention,
missing prefix values, duplicate prefix options, profile-looking option values,
unknown verbs and terminators before adopting this amendment. Failure of that
spike returns the bootstrap design for review, not a license for a shadow parser.

`planar --help` remains fixed embedded help with no file reads. Recommend
`planar with-profile --stdlib-profile FILE --help` for explicit selected-catalog
help: it captures/registers trusted extensions but never invokes a handler.
Native code renders validated help in stable verb/option order. Lua cannot print
raw help or replace usage/exit policy. This help exception belongs explicitly in
the new envelope parser contract; current per-command `--help` stays invalid.

## 4. Profile, trust, limits and output

Proposed `stdlib_profile_v1` JSON is parsed by Glaze with strict fields:
`version:1`, `modules:[<portable relative .lua file paths>]`. Paths are relative
to the profile's directory, not cwd; list order is the execution order after
shipped modules. Each path must satisfy declaration portable-path rules, with
no duplicates/case aliases/traversal/reserved synthetic namespace. No recursion,
glob, implicit directory enumeration, bytecode, dynamic require or imports.
An explicitly listed `stdlib.d/foo.lua` is allowed; the directory is not scanned.

The driver owns the trust decision and must keep profiles and modules outside
harness-editable authority. Native code rejects physical overlap/aliases with
captured untrusted declaration/source/base roots when those roots are supplied;
that check supplements driver trust and cannot prove OS ownership or prevent a
trusted driver from intentionally selecting hostile Lua. Trusted errors remain
product errors. Profile and module capture use native no-follow regular-file
reads, alias checks and retained stability receipts, never Lua host filesystem IO.
No external module receives clocks, environment, host paths or arbitrary IO.

| Proposed `extension_limits_v1` bound | Refusal |
| --- | --- |
| Profile JSON 64 KiB; 64 modules; each module 1 MiB; aggregate module bytes 8 MiB | `resource_limit`, no registration/dispatch |
| 64 commands total; 32 options per command; 64 shapes and 64 projections total | `resource_limit`, no partial registry |
| Help 4 KiB per entry; entire catalog canonical bytes 1 MiB | `resource_limit` |
| Registration: 64 MiB Lua allocation, 10 million instructions, stack 2048 | Terminal `resource_limit`; no retry inside same invocation |
| Execution: existing runtime ceilings (512 MiB, 100 million instructions), plus native service bounds | Terminal failure; transactions remain atomic |

Bounds are proposed versioned ceilings, not measured capacity promises. Aggregate
bounds apply across shipped plus external registration; the future implementation
must verify shipped catalog fits. Native value depth/node/string and service
bounds remain independently enforced. Lua C bindings must not longjmp across
live C++ destructors; use protected calls and owned boundary state.

A handler returns one bounded typed success/error value or asks for a typed
capability operation. Native code validates the response schema and emits through
Glaze and the selected response family. No `print`, direct stdout/stderr, arbitrary
exit code, raw JSON, raw host-path diagnostic or output during registration.
Projection text is bounded data within its response family, not an output handle.
Errors retain [render's envelope and exit policy](render-contract.md#errors-and-verification-boundary):
usage 2, validation/capture/resource 3, system/internal 4, success 0. Profile
schema/version/unsafe-path/capture errors reuse typed categories; registration
collisions use `reference` with a logical field path. Unexpected trusted Lua
failure is redacted `internal`; never expose traceback or host location. Existing
ledger typed errors and saved replay response bytes remain unchanged.

## 5. Request intent, exact provenance and historical identity

| Situation | Required behavior under proposed request_v2 |
| --- | --- |
| Same request ID, same normalized intent and profile selector; profile/modules changed or vanished | Return saved original response before profile capture/evaluation and before CAS. |
| Same ID, different selector, actor, expected head, workspace or registry | `request_conflict`, even if effective Lua bytes would be equal. |
| New ID, same selector, changed extension bytes | Capture new bytes; validate new interpretation; new exact provenance as applicable. |
| New ID, missing/invalid selected profile | Refuse without mutation; never silently use embedded policy. |
| History/export of old revision | Decode recorded versions and exact trees; do not load current profile or execute old Lua. |

Normalize selector paths lexically against invocation cwd without filesystem
access. request_v2 stores a discriminated `trusted_selector`: `embedded` or
`profile` plus normalized absolute profile argument. This selector is intent,
not the profile digest; adding that digest to equality would defeat replay after
file disappearance. Workspace and registry normalization follows section 6.
Open/verify the ledger and compare recorded body before capture, current-head
CAS or external registration. Repeat exact-body lookup inside the write
transaction before CAS, as ledger v1 requires. Request IDs remain ledger-scoped.

The embedded-only R14 continuation can retain request_v1 and interpretation_v1.
The external gate introduces explicit codecs/tags for request_v2 and effective
stdlib provenance, and interpretation_v2/versions_v2 where extension semantics
apply. Define mixed request-version equality as conflict, not reinterpretation.
Retain readers/verifiers for existing v1 records. A recorded format version
must explicitly support new tags; do not sneak them into the v1 schema or
promise in-place conversion. Review format coexistence before external code.

Effective provenance must retain exact profile bytes and ordered exact module
bytes, shipped originals/assembly, descriptor manifest, module logical paths,
assembly/version metadata and exact canonical effective-bundle manifest. Separate
selector host arguments stay private request metadata, outside portable identity.
Use versioned canonical bytes and XXH3-128 with exact-byte collision checks.
The effective trusted digest covers the complete sealed bundle and registration
ABI, not merely selected command names or a profile label. Bytes are retained,
not recoverable by digest alone. Any effective bundle change therefore changes
interpretation provenance; policy version changes are material only according
to reviewed fingerprint recipes. Do not automatically hash every extension into
work freshness or silently alter existing W recipes. Scope-specific semantic
versions and selected definitions need explicit reviewed identity amendments.

## 6. Workspace resolution and initialization

Recommend one unambiguous rule for `--workspace VALUE` on render and future load:

| VALUE | Proposed interpretation |
| --- | --- |
| `[A-Za-z0-9][A-Za-z0-9_-]{0,63}` | Bare workspace name; requires `PLANAR_WORKSPACE_DIR`. |
| Absolute path, or relative value containing `/`, or value ending `.lua` | Explicit workspace file; resolve relative values against invocation cwd. |
| Any other bare spelling (including `.`, `..`, spaces, backslash, colon) | Usage error; use an explicit `./...` file path where valid. |
| `workspace.lua` | Explicit file; `./billing` is an explicit file even though `billing` is a name. |
| Bare name with env unset/empty | Usage error; no HOME/cwd fallback, search or default. |

A name maps to `<PLANAR_WORKSPACE_DIR>/<name>/workspace.lua`. The environment
must be a nonempty absolute UTF-8 directory path without NUL; normalize lexical
`.`/`..`. Invalid configured env fails when name resolution uses it; explicit
file invocations ignore it. Do not expand `~`, `$VARS` or resolve shell syntax.
The shell may expand those before invocation. No default root/registry/ledger
is added. This reads cwd only as the explicit relative-path anchor; “no current
directory” means no discovery, remembered workspace or implicit root selection.

For request intent, normalize the resolved file to an absolute lexical path
before lookup, without stat/realpath/open. Equivalent name and explicit file
spellings have equal intent if they resolve to the same lexical target; changing
the env base changes intent. Parent `..` in explicit host paths is lexical,
not allowed in declaration references or workspace names. Physical aliases are
not collapsed into request equality. Two lexical paths to the same object remain
different request intents. Render's physical canonicalization/capture remains
later, and host paths do not enter portable packets or semantic identities.

Current [host resolution](../../src/engine/capture/host_path.cpp) physically
canonicalizes file parents and permits intermediate directory symlinks; it
preserves filesystem `symlink/..` meaning. File leaves are not followed, final
root aliases are refused, and captured traversal disallows symlinks/aliases.
Recommend preserving those rules for explicit render paths and name-expanded
paths, rather than silently forbidding all parent symlinks. Load first records
and uses its absolute lexically normalized target, so `symlink/..` can select a
different file than raw render spelling. This existing intent/capture distinction
must be explicit in the amendment and regression tests; replay never requires
physical resolution. Intermediate aliases do not merge request identities.

After an unknown request, load resolves that normalized target and applies
native capture confinement, physical tree-disjointness, regular-file/leaf-link
and captured hardlink/case-alias checks. Names gain no weaker capture policy.
Missing file/directory is typed IO refusal; unsafe leaf/root/captured alias is
`path_denied`. Retained native descriptors/receipts detect traversal or replacement
races. This proposal preserves [declaration path rules](declarations.md#paths-and-one-root)
and [capture exclusions](render-contract.md#capture-boundary); profile capture
and new scaffold publication have their separately stated stricter parent policy.

Current supported `init --ledger FILE --workspace-id ID` stays unchanged.
For RA01 recommend a disjoint explicit mode, for example:

```sh
# Proposed RA01 authoring syntax, not yet supported.
planar init --scaffold-workspace billing --workspace-id billing
planar init --scaffold-workspace /scratch/new-billing --workspace-id billing
```

Scaffold selector uses name versus explicit-directory rules (not file rules),
requires an existing safe parent, and creates a new directory only. Any existing
target, including empty directory/file/link, refuses. Prompt-only mode and its
required registry/schema/shape inputs need the RA01 contract; this example does
not make a usable authored spec without user review. No checkout, registry,
ledger, Git repository or inferred ledger path is created. Ledger initialization
remains a separate explicit invocation; scaffold options cannot mix with ledger
mode. No currently supported positional `init <name>` is implied.

RA01 must design exclusive staging/publication, race checks and owned cleanup
before code. Failed preparation publishes nothing; failed publication may leave
an explicitly reported owned staging directory if cleanup fails. Never remove
or overwrite an existing target, and never clean a directory whose identity no
longer matches the created object. Existing ledger/file publication primitives
are evidence to examine, not proof of an atomic portable directory transaction.

## 7. Bounded implementation and decisions

1. **Before R14 resumes:** operator resolves the grouped choices below; fresh
   scoped review amends render/workspace resolution and ledger normalization
   text, then defines command_catalog_v1 and family ABI. No code from this draft.
2. **Embedded bridge:** prove sealed registration, CLI11 descriptor attachment,
   retained state, help/diagnostics, capability refusal and pure registration.
   Preserve current command packet bytes and all native acceptance invariants.
3. **Remaining R14:** implement closure-only load service/CLI using embedded Lua,
   request_v1 replay-first ordering and native receipt/CAS boundaries. Complete
   focused plus integrated debug/release gates with honest platform limits.
4. **External gate before any dependent job:** review new record/interpretation/
   provenance codecs and selected shape identity rules, then implement profile
   capture, envelope parser spike, sealed external registry and exact retention.
   Prove disappeared-profile replay, selector conflicts, collisions, bounds,
   mutation refusal and historical reads without today's extensions. This is a
   required v5 delivery stage, not assumed complete by embedded-only load.
5. **RA01 and execution:** separately review scaffolding/prompt publication and
   observation contracts; later expose typed finite check/state services with
   generation/token/fencing invariants to both trusted tiers. Do not advertise
   unavailable commands in driver prompts. No scheduling/model/worktree service.

Exact amendments needed: render invocation/help/errors and any new identity
recipes; ledger request framing/replay, versions, provenance/interpretation and
format reader support; declaration shape/extension boundary only where new
registrations become selectable. Update product CLI/architecture references in
those implementation rounds. V5 input remains unchanged.

Operator decisions, with recommended answers:

- **A — Boundary and delivery:** adopt Lua semantic registration plus native
  CLI11 families/capability orchestration; finish embedded-only R14 first, then
  require an explicit external gate before jobs can depend on user extensions.
  Alternative: implement the full versioned external gate before resuming load,
  increasing the first integration scope. Neither alternative permits generic SQL.
- **B — Extension selection:** adopt the explicit profile envelope, ordered
  modules, no overrides/discovery and proposed bounds. Alternative: a separately
  designed explicit fixed global selector syntax; it must solve CLI11 bootstrap
  and replay before evaluation. Ambient stdlib.d discovery is not recommended.
- **C — Workspace convenience and init:** adopt the name/file/env table, preserve
  cwd-relative explicit paths, existing intermediate-parent symlink behavior
  and lexical replay identity (including the documented render/load distinction),
  and keep RA01 scaffold
  mode separate from existing explicit-ledger init. Alternative: explicit files
  only until RA01, deferring v5 naming convenience rather than guessing defaults.

Independent design review is required before treating this proposal as accepted.
Review approval alone does not resolve these operator choices or authorize code.
