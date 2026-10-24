# Planar rewrite agent guide

X05-C4-B1 and C4-B2 are independently approved native read foundations.
The C4-B3 candidate runs an admitted history-only retained handler with lazy
`context.read_history({})` bound synchronously to its original moved capture,
ledger receipt and private exact selector. No ledger contents are read if the
handler returns a semantic error without calling the service. The original B2
helper closes every native snapshot, transaction, connection and descriptor
adapter before any Lua result allocation or execution resumes.

The immutable invocation exposes an opaque ledger slot and exact ID strings,
scalar defaults and presence flags. The service is once-only; failed service,
invalid arguments, repeated calls and missing declared-service access poison the
invocation even when caught. Execute preserves original typed registration,
format and history errors. History success must match the retained native facts
exactly; raw comparison precedes owned response copying. Original receipts are
reverified before release. Pure handlers retain their summary/error protocol.
Manual native history preparation cannot bypass handler service sequencing.
The B3 review requested C4B3-R1: proxy-aware rawget bypassed sticky missing-service
refusal. The corrective B3+C01 candidate binds both lookup paths to the exact live
context and one registry backing, so history exposes one method consistently to
indexing/rawget/next/pairs and pure contexts enumerate none. Missing known methods
still return nil and poison even without a call; unrelated tables keep ordinary
lookup semantics. The extra pointer adds exactly eight compiled/charged bytes;
cleanup clears its identity before releasing rooted context storage.
Combined B3+C01 awaits fresh independent review. Corrective focused Debug evidence
is in `tests/native/v5-02-x05-c4-b3-c01-evidence/REPORT.md`; original B3 and failed
review evidence remain preserved.

Role read, projection service and capability combinations beyond history-only
remain closed. X06 recorded profile interpretation/acceptance and shared
invocation accounting, X07 public cutover, X08 mandatory external-profile
acceptance, execution/RA01 and Linux remain open. Public init creates format1.
The latest complete approved gate remains 676 per configuration; no new full or
release gate is claimed. Earlier evidence retains its historical scope.

Current integrated shape checkpoint: [X05-B4-C evidence](docs/vernacular/x05-shape-evidence.md) records one full
Debug **676/676** gate (268.37 seconds) and one release-preset RelWithDebInfo
**676/676** gate (126.24 seconds), each with zero failures/errors/skips/disabled
cases and all 676 candidate plus 582 A4 names preserved. Six common vectors,
per-configuration source/runtime audits, pinned formatting/Doxygen over 118 active
sources and no-work builds pass. This verification-only candidate awaits fresh
independent review; the latest independently approved full gate remains A4's
582 per configuration. The B2-C+C01+C02, B3-A, B3-B+R01, B3-C, B4-A+C01 and
B4-B+C01 slices are independently approved; B4B-R1 is closed. Historical private
fault/retained-callable probes remain approved Debug evidence, without release
rerun claims. Public init still creates format1. C projection/read services, X06
recorded interpretation/history/acceptance and receipt/whole-invocation orchestration,
X07 public cutover, X08 mandatory external-profile acceptance, execution/RA01 and
Linux remain open. Detailed later historical checkpoints retain their original
limits; this current checkpoint supersedes their stale future-slice wording.

X03-A, X03-B1, X03-B2-A and the combined X03-B2-B+C01 native implementation
are independently approved; C01 closes B2B-R1. The independently approved [X03-B3 verification gate](docs/vernacular/x03-format-evidence.md)
runs full debug and RelWithDebInfo gates: 430/430 each, zero failures/errors/skips,
pinned lint/Doxygen and no-work builds. [Independent B3 review](docs/vernacular/rounds/V5-02-X03-B3-review.md) closes X03-B. Native format2 acceptance uses immutable request-v2 embedded
intent, codec-aware exact interning and explicit recovery under retained limits.
Public init still creates format1. Fresh format2 request-v1 and profile acceptance
refuse; profile accepted history remains unsupported until X06 provenance/
interpretation support. B2-A SQL reader fixtures provide no conversion tooling.
The mandatory external-profile gate, public cutover, Linux and execution remain open.


The [X01 CLI11 spike](docs/vernacular/rounds/V5-02-X01-review.md) is independently approved as
test-only feasibility; approved baseline inventory is 405 tests. X02-C01 independently approves the
[external-profile exact amendment](docs/vernacular/external-profile-contract.md) and
[bounded implementation path](docs/vernacular/external-profile-implementation.md), with operator-selected P1 A/P2 A recorded in X02-C01. These documents do not
implement external loading or close its mandatory gate. Current v1 contracts,
R14 evidence, execution/RA01 order and Linux limits remain authoritative.

R14 is independently approved through D8, including D7-B public load and the applicable acceptance portions. Final D8 debug/release gates each pass 402 tests with zero skips. The mandatory external-profile gate is next; execution, RA01 and Linux acceptance remain future work.

`AGENTS.md` is a symlink to this file. Edit this target and preserve the symlink.
These are the active repository instructions for the approved breaking rewrite.

## Authority and scope

Work in the current checkout on `dev/planar-v2-vernacular`. The operator approved
replacing Planar completely, with no migration, upgrade path, compatibility CLI
or permanent legacy source tree. Git preserves the previous implementation.
This approval does not authorize deleting installed binaries or operator data,
installing the replacement, publishing or merging it.

Read [the implementation plan](docs/vernacular/implementation.md),
[roadmap](docs/vernacular/roadmap.md), the assigned round under
[rounds](docs/vernacular/rounds/), and the
[v5 design](docs/vernacular/planar-design-spec-v5.md) before working.
V5 is the product-direction baseline; explicit operator decisions remain higher
priority. Approved exact declaration/render/ledger contracts remain authoritative
until scoped reviewed amendments. See [v5 alignment](docs/vernacular/v5-alignment.md)
and [acceptance traceability](docs/vernacular/acceptance-matrix.md). V5-02 A/B/C
are operator accepted. The [exact command/workspace target](docs/vernacular/command-contract.md)
is independently approved in V5-02-A; B1 sealed registry/runtime is approved with
C01. B2 CLI family composition and C workspace naming/invocation snapshots are
independently approved. The [R14-D3 load amendment](docs/vernacular/load-command-contract.md)
is independently approved in [D3 review](docs/vernacular/rounds/R14-D3-review.md).
R14-D4 typed native diagnostics are independently approved in [D4 review](docs/vernacular/rounds/R14-D4-review.md).
R14-D5 bounded native load capture/preparation is independently approved.
The external-profile gate is mandatory after full R14 and before dependent jobs/RA01.
C adds workspace names to render, and D7-B supports them in load. External profile loading remains a target. Resolve
unspecified product choices
before implementing dependent behavior. Historical v4 remains an archival reference.

The first replacement slice compiled restricted Lua declarations and captured
observations into scoped role context. Trusted Lua owns schemas, shapes,
selection and projections; a small native engine owns isolation, captured file
access, canonical encoding, hashing and output. An external driver owns sessions
and scheduling. That initial render-only slice had no ledger; operator packet
evaluation preceded the implemented native ledger/history foundation. Current
commands are `render`, `init`, `history`, `export` and `load`. Render remains database-free.
The approved load service/CLI composes native acceptance transactions with
declaration-closure-only membership; D8 closes the applicable R14 acceptance
portions under the documented recovery limits. Claims, check execution,
model calls, scheduling and worktree management remain outside the current product.

## Coordination and records

Use repository files under `docs/vernacular/` for planning, decisions and evidence.
Do not invoke existing Planar tools for planning, claims, heartbeats, queues,
artifacts, documentation bookkeeping or finalization. Do not use the old
Planar-managed orchestrator, agent roles or skills for this rewrite. Historical references in `docs/vernacular/` describe removed legacy surfaces;
their operational requirements do not govern this work.

The coordinator records each round's objective, dependencies, owned files,
acceptance criteria, starting revision, pre-existing changes and verification.
Use fresh ordinary subagents for each round, sequentially by default:

| Role | Model | Reasoning | Responsibility |
| --- | --- | --- | --- |
| Search | `gpt-6-luna` | high | Read-only map of the exact scope, with source citations |
| Coder | `gpt-6.1-sol` | medium | Implement only assigned files and report verification |
| Reviewer | `gpt-6.1-sol` | high | Independently review candidate and raw evidence |

The operator approved the search model above in place of unavailable
`gpt-6.1-luna`; do not silently substitute models. Every brief must state that
the checkout is shared, assign explicit file ownership, and forbid reverting
others' work. Parallel coding needs a documented reason and disjoint ownership.

Coders never approve their own work. Review returns `approve`, `request-changes`
or `blocked`. Requested changes start a fresh corrective round with new search,
coder and reviewer agents, preserving the existing findings and candidate.
Approval lets the coordinator advance ordinary rounds without repeated operator
permission after checking that the reviewed candidate is still current. Record
an exact candidate snapshot or diff identity until commits exist. Edits affecting
approved scope need renewed review. Retire completed agents from dispatch; do
not reuse them in later rounds or claim termination when only idle completion
is available. Unresolved product choices return to the operator.

## Current tree and database safety

The active checkout builds one `planar` binary with render/init/history/export/load,
native capture and acceptance primitives, trusted Lua validation/selection/
projections, verified recorded-history reads, exact source export and canonical
JSON output. Ledger commands use explicit paths to the new format. Legacy
sources, migrations, operational surfaces and distribution scripts have been
removed. [The repository map](docs/vernacular/repository-map.md) is a historical
inventory; [architecture](docs/architecture.md) describes the active graph.

Never run a from-source legacy binary against the real database. Legacy runtime
code resolves `PLANAR_DB`, falls back to `~/.planar/planar.db`, and can apply
migrations automatically. If a scoped test must invoke legacy code, set both
`PLANAR_DB` and `HOME` to temporary scratch paths and use an explicit binary
path; keep subprocesses in that scratch environment. `PLANAR_HOME` alone does
not isolate the database. Do not use `make run`, which targets real data. Do not
invoke installed Planar binaries as a substitute for testing the candidate.
The replacement must have no dependency on the old database. Do not run the
legacy installer or release commands during rewrite work.

## Build and verification

Run builds and tests directly, serialized per build directory. Never use the
old host queue. Agree commands in the round brief and report executed commands,
results, matched test counts and limitations. Zero matching tests is not success;
expected unexplained skips are zero. Run focused checks for the changed scope.
An instruction-only round does not require a product build.

The direct replacement commands are:

```sh
make build
make test
make test-all
```

CMake is at least 4.3; presets use the pinned LLVM and `libc++` with Ninja.
The debug/release presets use fresh `build/rewrite-debug` and
`build/rewrite-release` directories. Test binaries stay outside default `all`;
`planar_tests` builds them. The complete gate builds, runs a nonzero count with
zero skips, checks active source formatting/Doxygen, and requires a no-change
build to do no work. See [testing](docs/testing.md) for direct equivalents and
current coverage. R09 records the integrated renderer gate and its platform limits.
Do not use the legacy installer, queue or release scripts.
Read [toolchain details](docs/toolchain-parity.md) when changing build tooling.

## Engineering conventions

- Keep first-party product C++ and Lua implementation under `src/`, including
  future product workflows. Build and integrity tooling, test-only Lua/harnesses,
  and fixtures may live outside. Fixture workspaces, source snapshots, observations
  and expected packets are verification inputs rather than runtime implementation.
- Use C++26 named modules: `.cppm` interfaces and `.cpp` implementations,
  `import planar.<dotted.path>;`, and `import std;` for the standard library.
  First-party headers are limited to test harnesses under `src/cmd/`.
- Keep dependencies downward: thin command entry points compose lower-level
  modules. Avoid cycles and edges between command binaries. Review the
  engine/Lua boundary explicitly rather than copying the old domain graph.
- Use `snake_case` for functions, variables, namespaces and types; leading
  underscores for private/protected members, none for public struct fields;
  CamelCase template parameters.
- Use `std::expected<T, E>` for expected boundary failures. Wrap library errors
  in the owning module's error vocabulary. Exceptions represent unrecoverable
  conditions. For Lua bindings account for C `longjmp` across C++ destructors.
- Do not use runtime `assert()` for required boundary validation; release builds
  define `NDEBUG`. Use explicit typed failures; `static_assert` remains valid.
- Append formatted text with `out += std::format(...)`. Avoid
  `std::format_to(std::back_inserter(out), ...)`: the pinned `libc++` has a known
  stack-buffer failure for some argument byte lengths divisible by 256.
- Delegate command and option parsing to pinned CLI11 through `import cli11;`.
  Keep entry-point I/O and bootstrap registration/dispatch thin. Command families
  own `handlers/<family>/command.cppm` and `command.cpp`, with CLI11 option state
  alive through parsing and dispatch. Shared response helpers sit below handlers
  and bootstrap; never introduce a handler-to-bootstrap cycle or shadow argv parser.
  Preserve the separate-token CLI contract with only its documented syntax guard
  and checks based on CLI11 parsed metadata. Keep semantic validation, fixed help,
  exit policy and Glaze-encoded diagnostics in Planar.
- Delegate JSON grammar, string escaping and serialization to the pinned Glaze
  library; adapters enforce bounds and structural policy, trusted Lua owns schemas.
  Do not add handwritten product JSON parsers or serializers.
- Avoid global mutable state; use explicit cancellation signals or atomics.
  Keep machine output stable and undecorated. Support macOS and Linux.
- Review reusable libraries and dependencies individually. Existing sandbox,
  process, serialization or hashing code does not prove the new contract.
  Review storage formats before application code when persistence is in scope;
  never reinterpret the legacy database as the new ledger.
- Pin third-party release archives with URL and SHA256 through
  `cmake/dependencies.cmake`, and commit their sources under `vendor/`.
  For new GitHub pins use `codeload.github.com/.../tar.gz/refs/tags/<tag>`.
  Do not add application dependencies through submodules, `GIT_REPOSITORY`,
  `FetchContent` or `find_package`, or depend on private repositories. A public
  clone must configure without network access or tokens. Check references before
  removing dependencies. Installation/runtime prerequisites must match the
  shipped product when distribution is redesigned.
- Keep Catch2 tests in colocated `*.t.cpp` files with per-test temporary roots.
  Test files begin with plain `//` comments. Add meaningful regression tests for
  bugs and realistic workflow tests for new CLI behavior. Demonstrate the failing
  regression before the fix; preserve evidence in the round record.
- Use the reviewed CMake module/test helpers while applicable. Test executables
  must stay outside default `all`. A no-change build should run zero steps.
  Version metadata is opt-in; do not invalidate the graph with live Git metadata.
- Use the pinned formatter. `clang-format --Werror` and Doxygen gate applicable
  C++ changes; `clang-tidy` is advisory. Every `.cppm` opens with `/// @file` and
  `/// @brief` describing purpose, entry points and error boundaries. Document
  exported declarations with summary sentences and useful `@param`/`@return`;
  state invariants and thread safety for shared state.
- Update affected reference documentation with behavior or architecture changes.
  Keep planning and round evidence in `docs/vernacular/`, product references in
  `docs/`, and generic gate helpers in `scripts/`. Do not author generated
  vendor instruction copies in `.claude/`, `.codex/` or `.github/`.
- Make only scoped edits; do not revert unrelated changes. Every commit, when
  authorized, carries `Signed-off-by` (`git commit -s`). Do not install, publish
  or merge without authorization for that action.

The [X03-B2-B-C01 correction candidate](docs/vernacular/rounds/V5-02-X03-B2-B-C01.md) reuses complete metadata equality,
including the selected ledger format, at locked transaction entry and both native
precommit boundaries. Controlled verified-metadata fixtures exercise refusal and
rollback in both formats; they do not reproduce an external format change.
C01 independently approves the correction and combined B2-B candidate.
The [X03-B3 review](docs/vernacular/rounds/V5-02-X03-B3-review.md) independently approves the full
430-case debug/release gate and closes X03-B; the external-profile gate remains open.

The [X04-A1 review](docs/vernacular/rounds/V5-02-X04-A1-review.md) independently approves native metadata
admission and descriptor binding; its inventory is 451 (not a full 451-test run).
The [X04-A2 review](docs/vernacular/rounds/V5-02-X04-A2-review.md) independently approves strict bounded
profile/ordered module capture and retained staged input receipts. The
[X04-A3 integrated native gate](docs/vernacular/x04-capture-evidence.md) is independently approved by the X04-A3 review: debug and RelWithDebInfo each pass 489/489 with zero
failures/errors/skips, preserving all 488 prior names. No external Lua, public
profile CLI, provenance acceptance or whole-invocation budget sharing is inferred;
the mandatory external-profile gate remains open.

The [X04-B1 prerequisite candidate](docs/vernacular/rounds/V5-02-X04-B1.md) extracts
the unchanged bounded profile decoder into the pure `capture/profile_codec` module
and adds the distinct 32 MiB/8192-file `effective_provenance` tree budget.
Focused debug verification passes 177/177 with zero failures/errors/skips; the
497-name inventory preserves all 489 approved A3 names. The independent
[X04-B1 review](docs/vernacular/rounds/V5-02-X04-B1-review.md) approves these prerequisites.

The pure X04-B1/B2/B3 slices are independently approved. B2 validates and
immutably seals catalog-v2, including source owner ordinals, exact shipped native
signatures and external static capabilities. B3 assembles and structurally verifies
exact `effective_stdlib_v1` bytes, modes, ordered modules, old embedded assembly,
catalog owners and full membership. The catalog-only 1 MiB ceiling and combined
canonical manifest 1 MiB ceiling are separate checks. B1 tests the outer
32 MiB/8192-file tree budget; valid complete B3 inputs reach at most 4162 files
and remain below 32 MiB under the stricter component bounds.

The [X04-B4 integrated provenance gate](docs/vernacular/x04-provenance-evidence.md)
is independently approved by the [B4 review](docs/vernacular/rounds/V5-02-X04-B4-review.md): full Debug and
RelWithDebInfo each pass 520/520 with zero failures/errors/skips/disabled cases,
preserving all 520 prior names. Pinned formatting/Doxygen checks 113 active
sources; vectors, exact source audits and no-work builds pass. No product or test
bytes changed in B4, and tested sources remained fixed through both gates.
This pure structural gate adds no live external registration/Lua, public profile
CLI, storage/profile-history acceptance or history dispatch, whole-invocation
shared budgets, or callability/semantic-honesty proof. The mandatory external-profile
gate, public cutover, execution/RA01 and Linux remain open; public init creates
format1. The independent B4 review closes integrated pure X04-B.


The [combined X05-A0+C01 independent review](docs/vernacular/rounds/V5-02-X05-A0-C01-review.md) approves bounded
test-only lexical sealing feasibility and closes A0-R1. The
[X05-A1 production foundation](tests/native/v5-02-x05-a1-evidence/REPORT.md)
is independently approved in the X05-A1 review. The embedded five-command VM audits
original global/callable roots together before alias-preserving table freezing.
The isolated `runtime/closure_guard` module reads pinned Lua compiler metadata,
rejects retained upvalue writers and unknown reachable native identities, and
permits invocation-local scratch. Exact strong native identity capture precedes
module evaluation; the five native engine callbacks enroll immediately after
native installation, without intervening module execution. Proxy natives are
created only after this audit and are not claimed to pass a later raw audit.

The protected sealing frame reserves 2048 slots before root pushes/native RAII;
inspection entry and every traversal push use nonallocating active-frame checks.
A shared identity/prototype memo preserves existing graph depth256 semantics;
prototype entries include the inherited sealed-cell mask. PMR allocation requests
charge the existing checked cumulative 512 MiB native conversion accounting before
allocation, with no refund or root reset. Static metadata inspection has checked
reported counters and is not charged as Lua instructions or converted nodes.
The finite compiled input remains bounded by the registration Lua heap; no fresh
4 MiB audit arena or public scan quota is introduced. Existing embedded invocation
instruction reset remains unchanged.

A1 does not execute external modules or admit public profile flags. Later X05-A
rounds retain source-owner/ordered module admission, the three descriptor APIs,
module-end audit/seal before later chunks, frozen global/API state, persistent
frozen-identity memo, add-time snapshot versus retained mutation semantics, and
whole-registration cumulative gates. The mandatory external-profile gate, public
cutover, storage/profile history dispatch, execution/RA01 and Linux remain open;
public init creates format1. No full milestone or release gate is claimed for A1.


The [X05-A2 repeatable sealing foundation](tests/native/v5-02-x05-a2-evidence/REPORT.md)
is independently approved by the X05-A2 review. A private protected root-argument seam
seals new retained roots after the initial embedded environment boundary. Its
strong native-owned successful-identity set is distinct from each boundary's DFS
memo: the guard treats prior sealed tables/Lua closures as immutable atoms and
the freezer preserves their proxy backing and aliases. New identities publish
only after complete audit/freeze success; any failure leaves the owner terminal,
including partial freezing or publication failure. Direct native callable roots
and unknown reachable native closures still refuse. `pairs` returns the exact
stable global `next` closure captured before module execution; native snapshot
never traverses C upvalues. Repeated sealing never writes frozen globals.

The production public runtime interface and old invocation tail remain unchanged
except that a failed sealing owner cannot invoke. Synthetic root bookkeeping adds
no graph depth; every boundary shares existing cumulative native charges and
inspection totals, without resets. Real private production probes exercise three
successive boundaries, fresh retained roots, prior proxies/aliases, exact iterator
identity across GC, scratch, writer/native refusal, resource failures and same-owner
failed retry. A3 still owns captured ordered modules, owner frames, three exact
registration descriptors and final catalog; final D is validated/copied at module
end before sealing D/F. No external execution, public profile flags, profile
storage/history acceptance or full registration gate is delivered by A2. The latest
full gate candidate is A4 (582 per configuration); the mandatory external-profile gate,
public cutover, execution/RA01 and Linux remain open. Public init creates format1.


The [X05-A3 captured registration](tests/native/v5-02-x05-a3-evidence/REPORT.md) adds native
`command_vm::create(profile_snapshot)` with structurally checked, privately copied
captured bytes and module order. Source-only zero-result chunks execute in one
bounded VM with native owner ordinals. Three exact descriptor APIs retain D/F;
final descriptors validate and seal with their callables at each module boundary.
Catalog-v2 and exact effective provenance bind to the original captured bytes/modes.
Selected-profile errors retain redacted native logical fields separately from
existing embedded diagnostics. External `_ENV` is natively created before shipped
evaluation with constructor-only engine, self `_G`, shared pure libraries and
three exact registration functions; original globals and this environment freeze
together before external execution. Embedded handlers retain their original engine.

A3 is registration-only and independently approved with C01–C05 by the
[C05 review](docs/vernacular/rounds/V5-02-X05-A3-C05-review.md). Profile-owner invocation
refuses before any provider action; retained alternate callable execution is
verification through private reversible production copies. Shipped shape/projection
IDs remain reserved; real adapters and semantics belong to B/C. No shipped Lua,
public CLI admission, profile history/storage interpretation or acceptance changes
are delivered. Native conversion/audit charges and explicit conservative pure-helper
capacity reservations remain cumulative; these are not process RSS accounting.
The mandatory external-profile gate, public cutover, execution/RA01 and Linux
remain open. Public init creates format1; the latest full approved gate is A4
(582 per build).


The [X05-A3-C01 corrective](tests/native/v5-02-x05-a3-c01-evidence/REPORT.md)
preserves original entry fields before command-name/shape/projection sorting and
propagates initial provenance preparation's typed resource refusal. Its final
logical reservation enumerates six canonical/payload buffers with separate record
framing/digest room, independently derived embedded metadata encoding/payload and
graph passes, and unchanged eleven embedded/three external source payload passes.
The existing cumulative 512 MiB charged native metric remains; this is not RSS
accounting. A valid catalog near the 1 MiB ceiling and controlled remaining-budget
probes exercise these reservations. Combined A3 through C05 is independently
approved; A4 records the latest full approved gate. Profile CLI, B/C/X06, public
cutover, execution/RA01 and Linux remain open; public init creates format1.


The [X05-A3-C02 location correction](tests/native/v5-02-x05-a3-c02-evidence/REPORT.md)
retains the selected original root through the guard's single shared audit and
maps module D/F pairs to one-based original catalog fields. Actual root freezing
uses the same mapping; failures before root selection and during identity-set
union use the active `profile.modules[N]` boundary. The existing enum-only guard
API delegates to the detailed result. Initial embedded/environment sealing and
private repeated-root sealing do not acquire external entry locations. Terminal
module retries preserve the first location; the public constructor discards failed
owners rather than retrying. Original C01 fixes and cumulative budgets remain.
Controlled probes exercise actual pre-root, partial-freeze and union refusal,
terminal state and cleanup; they do not claim natural resource exhaustion.
Combined A3 through C05 is independently approved. A4 records the latest
full gate candidate; public profile CLI, B/C/X06, mandatory external-profile
gate, cutover, execution/RA01 and Linux remain open. Public init creates format1.


The [X05-A3-C03 allocation-safe location](tests/native/v5-02-x05-a3-c03-evidence/REPORT.md)
selects seal locations with a trivial kind/index token. Root audit/freezing no
longer formats fields. Every protected seal failure adapter renders the token
outside Lua; a rendering allocation refusal returns the existing external resource
diagnostic with the preinstalled active module field. Two independent module-field
strings per module are staged before processing, charged cumulatively, and swapped
without allocation after descriptor collection. Staging failure identifies profile;
first-failure terminal retries retain their token. The shared audit, freeze order,
depth, publication and embedded paths remain unchanged.
Ordinary index10/64 and mixed-module controls, actual native allocation refusal,
controlled Lua allocation/stack/status and throwing-allocator adapters verify these
boundaries with cleanup. The throwing allocator is a controlled adapter probe,
not a claim that realloc throws C++ exceptions or natural host exhaustion occurred.
Combined A3 through C05 is independently approved. A4 records the latest
full gate candidate; public profile CLI, B/C/X06, mandatory external
profile gate, cutover, execution/RA01 and Linux remain open. Public init creates
format1.


The [X05-A3-C04 registration attribution](tests/native/v5-02-x05-a3-c04-evidence/REPORT.md)
extends the nonallocating original-location token across callback admission,
reference retention, final descriptor collection and pure catalog error remapping.
Successful callbacks clear their token; caught failures retain the first token,
and catalog prevalidation clears completed entry ownership. Error adaptation
renders only outside protected Lua calls. Rendering refusal retains the owned
module fallback; unmatched canonical entries retain the truthful caller boundary.
Pure global error fields move their owned storage. A separately charged/staged
catalog fallback installs by swap before final provenance work as preventative
consistency; no allocation failure at the previous short assignment is claimed.
C03 module staging/sealing, shared audit and cumulative budgets remain intact.
Controlled actual allocation probes distinguish removed zero-request operations,
remaining projection rendering refusal, sticky callback failures and module/global
phase work. Raw-reference status and unmatched pure errors use explicit controlled
adapters. No new quota or public interface is added. Combined A3 through C05 is
independently approved. A4 records the latest full approved gate; public profile
CLI, B/C/X06, mandatory external-profile gate, cutover, execution/RA01 and Linux remain open. Public init creates
format1.


The [X05-A3-C05 caught-memory](tests/native/v5-02-x05-a3-c05-evidence/REPORT.md) makes a completed
restricted `lua_pcall` memory failure sticky in native state. The bit is set
only after `LUA_ERRMEM`, preserving a successful emergency allocation retry;
logical quota refusal remains separate. Outer restricted calls rethrow and host
adapters retain the existing resource diagnostic and original entry token.
Actual descriptor-reference allocation probes cover normal/later-error tails,
three nested catches, direct host failure and successful retry after one null
return. The function-reference-only control has zero actual growth requests;
no function-reference allocation failure or natural host exhaustion is claimed.
Ordinary caught Lua errors remain recoverable. Shared accounting, protected
ownership and existing diagnostic priority are retained. The [C05 review](docs/vernacular/rounds/V5-02-X05-A3-C05-review.md)
independently approves combined A3 through C05 and closes R1–R7. A4 records the
latest full approved gate; public profile CLI, B/C/X06, mandatory external
profile gate, cutover, execution/RA01 and Linux remain open. Public init creates
format1.


The [X05-A4 integrated registration gate](docs/vernacular/x05-registration-evidence.md)
is independently approved by the X05-A4 review. Full Debug and
release-preset RelWithDebInfo each pass 582/582, with zero failures/errors/skips/
disabled cases, preserving all current 582 and prior 520 names. Pinned formatting/
Doxygen checks 117 active C++ sources; common source vectors, per-configuration
runtime/CLI and reversible variant audits, and no-work builds pass. Product/test
bytes remain identical across both gates; only current status documentation is
finalized afterward. Earlier private retained-callable/fault probes remain
approved Debug evidence and are not claimed as release reruns. Profile-owner
invocation refuses before provider effects. B/C semantics/adapters, X06 recorded
history/interpretation/acceptance and receipt/whole-invocation orchestration,
X07 public cutover, X08 mandatory external-profile acceptance, execution/RA01
and Linux remain open. Public init creates format1.


The [X05-B1 pure shape foundation](tests/native/v5-02-x05-b1-evidence/REPORT.md) is independently approved by the X05-B1 review. An optional trusted internal context selects profile-only shape IDs and
stages policy while retaining declaration version1 and the embedded default.
Every job's mandatory policy and base phase cycles finish before custom shape
callbacks; bounded additive unit obligations then join the original full graph.
A distinct test-only exact-runtime adapter exercises the actual product Lua
helpers. Its internal `shape_results` sidecar is not final selected-shape identity.
B1 does not prove retained native F dispatch, immutable F input, exact source
identity, the complete return's 16 MiB boundary, or whole-invocation accounting.
B2–B4 retain those native/identity/W/render duties. Public profile admission,
X06 history/acceptance, X08 mandatory acceptance, execution/RA01 and Linux remain
open; public init still creates format1. Focused verification is not a full B gate.


The [X05-B2-A native shape-definition foundation](tests/native/v5-02-x05-b2-a-evidence/REPORT.md)
received a request-changes review for early error adaptation; the combined
B2-A+C01 correction is independently approved by the X05-B2-A-C01 review. The internal selected-profile VM
resolves registered external shape IDs to owned ID, final revision and exact
`shape_definition_v1` digest. Identity includes the final descriptor, native
external logical module path and lowercase hex of that selected module's original
captured bytes; module ordinal and unrelated modules do not enter this identity.
Each lookup recomputes with cumulative checked native charges, without executing
Lua or starting/resetting invocation budgets. Resource failures remain terminal.
Generic profile invocation stays closed. Independently approved B2-B codec accounting precedes B2-C retained
F dispatch/immutable input/complete-return transport, B3 selected identity/W and
B4 rendering remain open, as do X06–X08, execution/RA01 and Linux. This focused
foundation does not close B2 or the B milestone; public init creates format1.

The [X05-B2-A-C01 error-boundary correction](tests/native/v5-02-x05-b2-a-c01-evidence/REPORT.md)
includes embedded, moved-from and already-exhausted lookup refusal construction
inside the typed boundary. An actual one-shot native allocation refusal returns
`resource_limit` at `catalog.shapes`; existing sticky conversion diagnostics keep
their first message, limit and maximum. Profile owners remain terminal. Ordinary
embedded/moved-from refusals retain protocol-first behavior on retry; a moved-to
owner remains independent. This covers controlled one-shot failures, without a
persistent exhausted-heap guarantee. Identity, cumulative reservation policy and
public admission remain unchanged. The combined candidate is independently approved by the X05-B2-A-C01 review;
B2-C and the mandatory external-profile gate remain open.


The [X05-B2-B codec reservation candidate](tests/native/v5-02-x05-b2-b-evidence/REPORT.md)
adds optional native `codec_encode_reserved` without changing ordinary callers.
One shared validator derives checked logical graph, string, workspace and fixed
Glaze output capacity before the synchronous native charge hook. Successful
validation uses a bounded active path without heap allocation; each copied
container reserves its exact entry count. Input stability through the hook is a
trusted C++ precondition. The moved output buffer is counted once; conservative
workspace slots are not independent heap allocations or an RSS guarantee.
Focused Debug evidence and independent allocation phase probes accompany the
candidate; combined B2-B+C01 is independently approved by the X05-B2-B-C01 review. B2-C retains raw-return conversion
charges, retained F execution, immutable input and invocation integration. B2/B,
X06–X08, execution/RA01 and Linux remain open; public init creates format1.


The [X05-B2-B-C01 encode diagnostic correction](tests/native/v5-02-x05-b2-b-c01-evidence/REPORT.md)
is independently approved by the combined X05-B2-B-C01 review. A shared outer
allocation boundary protects the existing encode handlers' diagnostic construction,
including validation refusals and callback exception adaptation. Controlled one-shot
native diagnostic allocation refusal returns the existing `resource_limit` /
`output_bytes` diagnostic; normal precedence and moved typed callback refusals
remain unchanged. This is not a persistent exhausted-heap guarantee. Encoding,
reservation math, decode and limits are unchanged. B2-C retained F dispatch and
invocation integration, B3/B4, X06–X08, execution/RA01 and Linux remain open;
public init creates format1. Focused correction evidence does not close B2/B.


The [X05-B2-C retained shape bridge](tests/native/v5-02-x05-b2-c-evidence/REPORT.md) is independently approved with C01/C02; their reviews close B2C-R1 and B2C-R2.
Internal `command_vm::invoke_shape` binds owned final definition metadata before
calling the actual retained F in its original selected-profile VM. Exact normalized
five-field inputs preserve nested aliases and freeze transient proxies without
publishing permanent sealed identities. Exact pre-invocation tag/backing membership
survives cleanup after raw response conversion, including returned input aliases;
invocation scratch/result tags are released. One actual JSON-compatible return
passes separately charged raw conversion and reserved Glaze encoding with the
complete 16 MiB ceiling. Padding transport fixtures do not prove shape semantics.
Registration transitions once to cumulative invocation instructions; native/node
charges never reset. Admission protocol errors are nonconsuming; execution/resource
failures retain the first terminal original shape location. B1 Lua still owns shape
success/refusal/obligation semantics. B3 selected identity/W and B4 rendering,
X06–X08, public profile admission, execution/RA01 and Linux remain open. Generic
profile command invocation stays closed; public init creates format1. This focused
Debug slice is not a full B or release gate.

The [X05-B2-C-C01 correction candidate](tests/native/v5-02-x05-b2-c-c01-evidence/REPORT.md) stores transient
snapshots under private immutable address keys, separate from every integer
`luaL_ref`. Retained shipped F and external D/F bindings survive first/repeated
shape calls, GC and failure cleanup in the owned reversible observer. The original
reviewer regression is retained and reproduced. This correction preserves existing
cleanup, cumulative budgets and public boundaries; the combined C02 review independently approves the bridge. No full milestone or release gate is claimed.

The [X05-B2-C-C02 terminal diagnostic correction candidate](tests/native/v5-02-x05-b2-c-c02-evidence/REPORT.md) shares the profile owner's first terminal error across definition lookup and shape invocation.
A resource failure owns its original code, message, limit, maximum and field before
fallible response copying; later ID/input guards and allocation adaptation preserve
that error. Natural cumulative lookup exhaustion reproduces the prior masking and
location change, then verifies missing/wrong-kind/valid IDs, invalid inputs and
retries. Controlled one-shot native allocation probes cover ownership and copies;
no persistent host-memory guarantee is claimed. C01 private registry keys remain.
The combined independent B2-C+C01+C02 review approves this scope. The focused Debug gate adds
one regression (620 registered names, 354 focused plus 11 supplemental); no full
milestone or release gate, public profile admission, B3/B4, X06–X08, execution/RA01
or Linux acceptance is claimed. Public init creates format1.


The [X05-B3-A pure phase candidate](tests/native/v5-02-x05-b3-a-evidence/REPORT.md) factors existing trusted validation into structural prepare/check-one/finish exports.
Every mandatory/base-cycle check precedes custom requests; checked responses retain
B1 exact obligations, guidance and count policy. Finish retains original graph edges
and forms selected_shape only from explicitly supplied trusted request-corresponding
definition metadata. The old embedded validator and B1 shape_results adapter remain.
The [phase ABI](tests/native/v5-02-x05-b3-a-support/phase-abi.md) records handoff requirements.
The X05-B3-A review independently approves this pure slice. It does not prove native metadata
authenticity, immutable F input, same-VM dispatch or cumulative phase conversion;
B3-B owns those. B3-C W, B4 rendering, X06–X08, public profile admission/cutover,
mandatory external-profile acceptance, execution/RA01 and Linux remain open.
Public init creates format1; latest full approved gate remains A4 582/configuration.


The original X05-B3-B incomplete prerequisite checkpoint is preserved in
`tests/native/v5-02-x05-b3-b-evidence/REPORT.md`, including its actual request423
regression and scoped sticky correction. The R01 integration candidate below
supersedes that checkpoint's remaining accounting/lifetime status.


The [X05-B3-B-D01-A test-only decode feasibility](tests/native/v5-02-x05-b3-b-d01-a-evidence/REPORT.md) demonstrates
request-local allocation interception with pinned Glaze readers, including variant
rewind, duplicate-check storage, malformed partial cleanup and typed refusal
adaptation. Independent D01-A review approves the test-only boundary. No product, vendor, codec API or runtime
bytes change. Production conversion reservation is now the separate D01-B candidate; B3-B accounting/request423 and its final gate remain open. The
637-name product inventory includes WIP; last approved inventory remains 627.
No full milestone, release or Linux gate is claimed; public init creates format1.


The [X05-B3-B-D01-B production reserved decode candidate](tests/native/v5-02-x05-b3-b-d01-b-evidence/REPORT.md) adds optional
`codec_decode_reserved`: every actual parser/duplicate-check request charges before
allocation, followed by one checked allocation-free immutable-conversion sizing
pass and reservation before persistent ownership. Request-local Glaze wrappers
retain original grammar/error ordering; returned values use ordinary allocators.
Exact pinned control-block/tree-node/list/string recipes and the actual bounded
workspace determine capacity. Typed supplied errors move unchanged, and outer
allocation protection covers early/catch diagnostic construction in both decode
entry points. Ordinary decode API/defaults and approved encoding remain unchanged.

Independent D01-B review approves the reserved decoder prerequisite. The
X05-B3-B-R01 native integration candidate consumes both decoder hooks only while
profile semantic composition is active. Actual parser/checker allocation requests
and immutable conversion capacity charge the existing cumulative native budget
before allocation, without refunds. The inherited node and graph/transport charges
remain deliberately conservative overlapping accounting. Embedded decode keeps
its ordinary path. Semantic error-data staging also charges concrete pinned
control blocks, map nodes and copied string buffers before allocation; successful
encoded text separately charges its native wrapper while moving the already
reserved output buffer. Direct in-place construction avoids initializer-list
copies. Two narrowly scoped global-fragment libc++ headers expose only the pinned
allocation type identities; product operations retain `import std`. Reserved
encoding and first typed terminal diagnostics remain.

The combined original B3-B plus R01 candidate retains audited private phase roots
and native-owned request/result/definition values across sequential protected
same-VM prepare/F/check/finish calls. Production coverage includes recoverable JSON
refusal followed by healthy calls. Private reversible observers cover exact tag/
backing key-value membership and all16 retained references in a mixed fixture
(five shipped, three phases, four external descriptor/callable pairs), stable live
Lua heap after warmup/GC, partial baseline setup and terminal cleanup. Controlled
parser/conversion/node/Lua refusals prove no F dispatch; repeated real operations
exercise cumulative instruction/native limits. Near-budget staging controls
verify exact native error-graph requests and deny node/map/string/wrapper storage
before allocation, retaining ordinary healthy schema diagnostics. These probes supplement actual
production F and semantic execution.

The frozen candidate gate targets642 registered names (all641 prior names
preserved),376 focused Debug cases and11 supplemental history cases, production
semantics, actual allocation sampling, request423 checkpoint/current controls,
vectors, six exact source audits, pinned formatting/Doxygen and no-work builds.
See `tests/native/v5-02-x05-b3-b-r01-evidence/REPORT.md` for executed results and
limitations. The independent combined B3-B/R01 review approves this integration. No full B,
release or Linux gate is claimed; latest full approval remains A4's582/config.
B3-C W, B4 render, X06–X08, public profile admission/cutover, mandatory external
profile acceptance, execution/RA01 and Linux remain open. Public init creates
format1. Codec and original checkpoint artifacts remain unchanged.


The independently approved X05-B3-C selection adds internal native `select_profile` over original
bundle/capture/files/job/role intent. Its semantic phases run once, then a fourth
privately retained trusted phase selects from that exact finalized same-call model.
All four phase roots join the original pre-module audit/freeze; phase transport,
codec work and repeated operations share cumulative accounting and cleanup. No
caller model, shape definition or export selector is admitted. Generic profile
invocation remains closed.

Embedded `select_role` retains its original normalization and whole-bundle
validation. Shipped selection remains exact with profiles. Custom W preserves old
fields, appends `shape_selection_v1` and shape definition/revision plus synthetic-ID
sorted unit obligations where selected work is either endpoint, and hashes
`work_external_shape_v1`. Guidance is excluded. Selected source comments/revisions
change every custom-job role; separate unselected/projection modules affect only
provenance. Same-module visual source changes conservatively affect definition/W.
The independently authored minimal-fixture W and complete canonical preimage live
under `tests/native/v5-02-x05-b3-c-support/`; the candidate verification report is
`tests/native/v5-02-x05-b3-c-evidence/REPORT.md`; the B3-C review independently approves this selection scope.
This is selection infrastructure: B4 presentation, X06 history/acceptance, X07
public cutover, X08 mandatory external-profile acceptance, execution/RA01 and Linux
remain open. Public init creates format1; no full B/release gate is inferred.

Selected-profile hash work additionally reserves conservative pinned-library
cumulative string growth, owned source/domain copies, encoder scratch and digest
storage before allocation. These byte charges do not add converted-node counts.
Actual allocation probes retain typed first resource refusal; embedded digest
behavior remains unchanged. The derivation and private aggregate measurements are
recorded in `tests/native/v5-02-x05-b3-c-support/hash-accounting.md` and the C report.

The X05-B4-A candidate factors the shared trusted render body and adds the private
`render_profile_material({request,model,selected})` helper. Its caller must supply
authentic finalized semantic and selection results; logical job/role/shape identity
checks do not authenticate arbitrary tables. The helper normalizes render intent,
reads retained observations, and renders without validation, selection, F dispatch
or mutation of model/selection. Custom shapes add exact three-field identity,
full scoped unverified prerequisites and mandatory `shape_policy` after dependencies;
shipped shapes preserve the old envelope and presentation. B4-A verifies pure helper
composition only. Native same-call retention, immutable transport, cumulative full
render accounting and lifecycle proof are delivered by the B4-B candidate below; the full Debug/release B gate
remains B4-C. Public profile CLI, X06–X08, execution/RA01 and Linux remain open;
public init creates format1. Candidate evidence is
`tests/native/v5-02-x05-b4-a-evidence/REPORT.md`; independent B4-A review requested
the correspondence correction recorded below.


The X05-B4-A-C01 corrective candidate requires complete positional one-to-one
bundle/semantic job correspondence before reading any selected shape. Both arrays
must have equal lengths, each row must carry a valid matching job ID, and IDs
must be unique across the complete mapping. Reordered, missing, extra, duplicate,
ambiguous and malformed identities refuse with the existing schema diagnostic at
`render.selected`; there is no ID lookup fallback, revalidation or input mutation.
Two new production cases reproduce same-shape/different-guidance substitution and
exercise selected/unselected rows and malformed containers. The actual pre-edit
baseline and red regression are retained in
`tests/native/v5-02-x05-b4-a-c01-evidence/REPORT.md`. B4-A's independent review
requested this correction (B4A-R1); the combined B4-A+C01 review independently approves the correction and closes B4A-R1.
Pure helper correspondence does not establish native same-call authenticity;
Combined B4-B+C01 native composition is independently approved; B4-C integrated evidence awaits independent review. Public init
still creates format1; X06–X08, public cutover, execution/RA01 and Linux remain open.


The X05-B4-B candidate adds internal native `render_profile` over original
bundle/capture/files/job/role intent and optional budget. A trusted normalization-only
preflight preserves full render schema then budget-stripped selection schema
precedence before any semantics or F call. Six fixed trusted roots seal before
external modules. Native-owned normalized intents, same-call finalized model and
selection flow through sequential protected calls to the approved renderer; each
custom job's F runs once. Existing selection behavior and public generic profile
refusal remain. All phases, observation/presentation codecs, hashes and a final
reserved complete-response encoding share cumulative limits; final output is
bounded by the existing 16 MiB codec ceiling and lower role/render budget.

B3-C and combined B4-A+C01 are independently approved. combined B4-B+C01 is independently approved; B4-C records the complete Debug/RelWithDebInfo gate candidate. Focused Debug
coverage, exact root/reference and frozen-membership GC observations, preflight
zero-F and valid once-per-job dispatch, actual native allocation refusals and
controlled transport failures are recorded in
`tests/native/v5-02-x05-b4-b-evidence/REPORT.md`; allocation derivation is in
`tests/native/v5-02-x05-b4-b-support/transport-accounting.md`. This candidate adds
no provider or public profile CLI, projection dispatch or storage interpretation.
X06–X08, mandatory external-profile acceptance, public cutover, execution/RA01 and
Linux remain open; public init creates format1. Latest full approved gate remains
A4's 582 per configuration.


The X05-B4-B-C01 correction candidate routes every ordinary native render
response, including normalization, preparation, shape-check, finalization and
selection errors, through the same existing reserved complete-response encoder.
The unchanged 16 MiB output ceiling and cumulative native charges apply before
returning the original native response pointer. Validation-only/selection-only
transport and native unexpected failures retain their prior paths. The genuine
oversized original unknown-key schema regression now returns terminal typed
`output_bytes`/16777216; small ordinary errors retain their bytes and recovery.
Production phase/lifetime controls and reversible exact writer/phase/F observers
supplement focused Debug verification. The combined B4-B+C01 review independently approves this correction and closes
B4B-R1. B4-C records the separate integrated macOS gate; Linux remains open.
Public profile admission, X06–X08, mandatory external-profile acceptance,
execution/RA01 and Linux remain open. Public init creates format1; latest full
approved gate remains A4's582 cases per configuration.


The X05-C1 candidate extracts private pure section assembly in the existing
`render.lua`; ordinary rendering supplies no custom records. It copies prepared
base/custom bookkeeping, appends custom headed bytes, removes custom optionals in
reverse result order before shipped priorities, and keeps exact notice/join counts.
A separate private adapter emits projection `{id,bytes,included}` metadata while
ordinary manifests retain `mandatory`. Native admission/limits/reserved IDs,
retained projection F dispatch, read capabilities and receipt orchestration remain
later work. See the [C1 report](tests/native/v5-02-x05-c1-evidence/REPORT.md). This
is a focused Debug candidate awaiting independent review, not a full C gate.


The X05-C2-A candidate adds internal native projection-definition identity lookup
without executing the retained projection callable. It hashes the exact final
five-string descriptor, logical external owner without ordinal, and original
privately captured selected module bytes under `projection_definition_v1`.
Owned ID/revision/digest results survive VM destruction. Unknown IDs refuse
nonterminally; resource failures retain the shared first owner error across shape,
projection, validation, selection, render and moves. Prospective graph, string
copies, cumulative canonical growth and encoder scratch charge existing native
limits before allocation; no invocation starts or budget resets.
Focused Debug evidence is in `tests/native/v5-02-x05-c2-a-evidence/REPORT.md`;
fresh independent review is pending. Retained projection execution/native result
admission remains C2-B. Read services, capability/receipt orchestration, public
profile admission, X06–X08, execution/RA01 and Linux remain open. No full C or
release gate is claimed; public init creates format1.


The X05-C2-B1 candidate adds only a native exact projection JSON size prerequisite
in `engine/json_codec`: stable borrowed section views, complete input preflight,
one synchronous charge for fixed 6,325,248-byte scratch and actual fixed writer
workspace, and pinned Glaze serialization without an AST copy or retained output.
Encoded overflow is sticky and nonthrowing; bounded writing completes before
refusal. Native invalid input, encoded ceiling, resource and scratch/internal
failures remain distinct typed outcomes. Callback diagnostics move unchanged;
independent nested/concurrent owners share no routing state. The operator's global
18-name built-in reservation is now explicit in X-PROJ for both input kinds;
semantic reserved-name admission remains the later native scanner's duty.
Evidence is `tests/native/v5-02-x05-c2-b1-evidence/REPORT.md`; fresh independent
review is pending. This prerequisite does not execute F, scan/root Lua results,
open profile invocation or implement read services, public CLI, X06–X08,
whole-invocation orchestration, execution/RA01 or Linux. Public init creates
format1; no full or release gate is claimed.


C2-B2 adds the internal typed retained-projection bridge and native raw-result
admission candidate. Descriptor kind and immutable structural transport are checked
before one retained F call; callers own authenticated selected-role/history data.
The protected scanner enforces exact version1 section schema, the shared GLOBAL18
reserved union and raw limits before borrowed-view exact encoded counting and
native result copying. Original projection indices and shared first terminal errors
survive retries and moves. This candidate awaits fresh independent review; native
read services, rendering composition, CLI/provider/receipt integration and the
mandatory external-profile gate remain open. See
[the C2-B2 evidence](tests/native/v5-02-x05-c2-b2-evidence/REPORT.md).
