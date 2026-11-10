# R19-AUTHORING + C01 independent final review

Disposition: **approve** the exact combined candidate identified below. Parent
R1 is closed by this source correction and its verification; parent A1 is closed
by the canonical table formatting repair. No new blocking or advisory finding
was found. This approval does not claim that the older acceptance/profile
accounting sites or the remaining full R19 obligations are complete.

Fresh independent ordinary reviewer, using the available model in place of the
unavailable prescribed Claude reviewer. This is disclosed substitution, not
prescribed-model execution. The reviewer was not the coder, coordinator or
prober. Writes were limited to this report and ignored C01
`review-verification/` evidence. No source/build modification, native build,
ledger mutation, installed/legacy binary, commit, installation or publication
was performed by this reviewer.

## Candidate and actual review target

Reviewed HEAD: `d6c989ed9e196bf9b357019ea946e6ff18c47f18`, branch
`dev/planar-v2-vernacular`. Reviewed the complete combined `review/CODE.diff`,
including all **40** changed code/support paths and **eight** new untracked
files, rather than tracked Git output alone.

| Identity | SHA256 |
| --- | --- |
| Frozen 306-input aggregate | `847e5e23858170174a6e7c4a3d846f7ca016886ff4885f25aec7505f60524945` |
| `CODE-IDENTITY.json` | `8b64fce65c9079257172acde4700324d07662e7f884ba6e881c4a145fafee8b7` |
| `REVIEW-MANIFEST.json` | `a84ec0519b2f34e29db2826da12e66723df4f1e1b5e25571dcc35232dadbf6f0` |
| Complete code diff | `ea443c31506aa09607bb987878ac0e0f3138d13c91bd346043f1dd9c05279167` |
| Corrected `runtime.cpp` | `56325578d8839ce15b683d49b5c382096aa98e8bbd053e35feb5439d5ea3c3ca` |

All **306** current code/support hashes, **71** published candidate hashes and
**20** pinned local artifact hashes match. All **2,183** probe raw artifacts
match their retained sizes and hashes. The original rejected candidate's **55**
published inputs also match their preserved snapshot. The frozen code identity
is historical evidence, not a completion document to rewrite.

The actual claimed reviewer is run **5**, generation **3**, against revision **5**:

- Work `retained_vm`, output `xxh3_128:8955bb1ccbf3d51499caca0bcafacb43`.
- Worker W `xxh3_128:45b6e58e37fd6a82ec8c0bdc953df7e1`.
- Reviewer policy `xxh3_128:4476fe78ae35cdd44e23bf52e4fe12eb`.

The actual completed `authoring.c01.check.1` result names that output/W and has
three passing evidence rows: `host_debug`, `host_release`, and
`planar.boundary_v1`. The retained reviewer recovery Markdown names the exact
output/W and passing host checks; its acknowledged dispatch and current W match
the policy. Its outer `evidence` array is empty, so the actual three receipts
were inspected independently in the check result. This preserves the already
recorded packet-presentation limitation. No product verdict existed at this
review snapshot; this report authorizes the coordinator's next protocol step.

## R1 closure: complete new-binder allocation model

Read the repository instructions, implementation/protocol and roadmap opening,
v5 design, governing retained-VM/budget/EX-6/replay/check/review clauses, both
rounds/maps, both plan reviews and scoped profile amendment, the independent
parent review, and **both canonical DECISIONS/INVARIANTS pairs**. No unresolved
product or authority choice is needed for this correction.

The new accounting in `src/engine/runtime/runtime.cpp:6032` precedes
`history::prepare_stdlib`. Independent tracing through
`history/provenance_codec.cpp`, `history/tree_core.cpp`, `history/record.cpp`,
`canonical/canonical.cpp`, and the pinned libc++ string implementation supports
the complete reservation, beyond the runtime test's 3P lower bound:

| Allocation path | Coverage |
| --- | --- |
| Original/assembly file-record payload, independent encoded hash input, final owned payload | 3P; intermediate destruction gives no refund |
| Manifest's initial encode and those three subsequent constructions | 4M; exact schema length is `125 + separator bytes + sum(9 + path bytes)` |
| Initial canonical writer's growing string backing | Additional 3M; pinned geometric growth and alignment bound cumulative backing below 4M for this manifest |
| Manifest's separately owned separator value | Separate separator-byte charge |
| Path value/shared ownership, list capacity, root/map/key/scheme values and canonical traversal | `4096(n+16)`, with path size bounded before estimation |
| Tree nodes and vector growth, child maps, directory child/sort/alias/framing work | 2048 per counted component; charging each path occurrence overpays shared ancestors |
| Folded/name/validation/encoded-child strings and final owned paths | Eight transfers of `path bytes + 32`, supplementing component/object allowances |
| File-record framing, reserved file/view vectors, hex/provider identity | Explicit framing, structure-size and bounded 256-byte reservations |

The selected tree implementation first rejects paths over 1024 bytes or with
64 slashes; valid components are at most 128 bytes. These actual bounds support
the component/path allowances. Synthetic paths are exactly
`__planar__/assembly.lua` and `__planar__/assembly.canonical`. File-record
encoding's bounded initial framing/growth and directory encoding/alias scratch
fit within the separately paid framing/component allowances. The metadata
allowance includes construction and traversal objects, rather than treating
encoded M as their ownership budget. Estimation creates no owning graph,
container or string before reservation.

Checked provenance additions maintain P and M within 16 MiB, including P+M.
Count/path/depth limits bound `n+16`, `n+2`, component counts and `path+32` before
the dependent arithmetic. Every multiply is guarded before execution; numerical
capacity failure delegates to the existing safe one-over-ceiling `charge`,
preserving `native_bytes`, its maximum and terminal failure. There is no refund,
reset, ceiling increase or generic worldwide-accounting claim. Invalid structural
provenance remains distinct from numerical native exhaustion.

Comparison with the preserved rejected runtime proves that **all** C01 product
changes are inside this accounting block. The binder prefix and everything from
its `prepare_stdlib` call onward are byte-identical. The interface, private test
package, production/private ceilings and older load/profile callers remain
unchanged. This closes R1's complete scoped allocation requirement, not merely
its payload-copy example.

## Private fixture and quantitative proof

The private generator changes exactly two module/namespace identities per unit
and adds one checked-preconsumption insertion in the implementation after
`begin_invocation` and before the unchanged real binder call. It requires unique
anchors, reverses to exact source, decodes whole fixed-width **12-digit** ordinary
fixture IDs and calls the actual cumulative `charge`. There is no counter reset,
refund or public CLI/env/fault hook. Independent regeneration in the reviewer's
owned evidence directory matches current generated build bytes; a second run
changes neither bytes nor mtimes. The private target is excluded from default
`all` and reached through `planar_tests`.

The frozen tests/generator/CMake hashes still match the actual assertion-red
package. Its two cases compiled, selected nonzero tests and failed accounting
assertions against the old runtime. P=**463972**, exempt minimum=**10640**,
old bound minimum=**543816**, gap=**533176**, below **3P=1391916**. The old 2P
case incorrectly reached both callbacks. These are real assertion failures,
not setup/compile failures.

The fresh focused reviewer run reproduces corrected bound minimum **1638863**
and gap **1628223**; both new cases and the existing identity/exemption/owner
cases pass (**eight actual JUnit nodes**, zero failures/errors/skips). The gap
includes activation/status validation side cost and is not a pure allocator
measurement. At **2P=927944**, refusal is exact `resource_limit/native_bytes`,
with no binder/provider callback and consumed-owner refusal on the next call.
The generous control binds the exact independently accepted digest.

Inspected actual raw mutant sources, compile/link/object evidence and JUnit:
restoring initial one-pass accounting fails both cases; changing only payload
passes from three to two while retaining **all** metadata/scratch terms fails
the quantity assertion with gap **1164251**. The exact two-pass-only source
transformation was independently checked. Preparation-entry poison passes the
low/generous controls, while late reservation exposes poison at low room and
fails the diagnostic assertions. These demonstrate quantity and preparation
order separately; neither source-order checks nor no-provider observations
alone are mislabeled as allocator instrumentation.

## Combined behavior and evidence assessment

All twelve embedded execution/read branches forward the admitted VM into typed
local handlers. The runtime-free option module preserves all **78** field/type/
default slots across twelve states, unchanged CLI11 attachment and six complete
explicit intent adapters. Authoring's profile branch intentionally uses the
original closed owner and shared parsed state. There is no cast, shadow parser,
new authoring exposure or production fallback.

The VM derives the embedded provenance-tree digest from its own retained
originals/separator/assembly. Native services require that supplied identity;
the hardcoded production fallback is removed. Profiles still bind their
verified effective identity. Early saved-response/body-conflict preludes remain
before fresh derivation; bare status/request lookup, check-recover and unrelated
reads remain exempt. Consumed/failed owners are rejected before preparation and
terminal release remains intact. EX-6 equality/no-write refusal is preserved.

Independent current generated-byte verification matches **561** recorded
composition rows, including **14** semantic runtime compositions/**28** units.
Wiring passes in both configurations: **15** bootstraps/**180** family bindings,
with authoring's protected original profile routes. The exact **43-file** X08
inventory retains missing/extra/content controls. The private fixture is audited
separately from unmodified semantic compositions.

The C01 probe record correctly distinguishes **17** canonical groups: **15**
assertion kills, one configured-graph observation, and one isolated consumed-owner
mutant SIGSEGV. Healthy/restored owners pass. **19** structural carries have
verified unchanged input hashes, **10** parent native groups rerun, and two old
source-only quantity rows are superseded. A failed unused-variable compile is
retained as setup failure. The exploratory late duplicate-commit edit was masked
by early lookup on the CLI path and then assertion-killed by the existing direct
commit test; neither observation is overstated.

The raw final control has nine passing runtime JUnit nodes, five lifecycle
scenarios/**68** processes, two source guards, the direct-commit case and B2's
actual retained-callback control. Initial failed/interrupted gates, the earlier
path/harness correction, X08 expected-set repair, stale generations and
check-recover remain historical. Parent A1's canonical tables now form continuous
Markdown tables, with original snapshots retained.

Both actual complete host gates independently reparse as **1851/1851**, exact
unique inventory/JUnit names, zero failures/errors/skips/disabled. Each records
**251** linted sources, identical **802** before/after source hashes containing
all 306 candidate hashes, and the exact allowed glob probe followed by Ninja
idle. The completed actual check replay is byte-identical, leaves the ledger
hash unchanged and creates no new check attempts. This replay was executed by
the coordinator; this reviewer inspected its result and bytes.

## Reviewer checks and limits

Executed the existing Debug runtime binary with
`[binding_native_budget],[execution_binding]`, both configuration wiring scripts,
the field audit, `git diff --check`, private generation twice in owned evidence,
and independent hash/JUnit/source-freeze/mutant/carry audits. All final checks
pass. `review-verification/AUDIT.json`, `SOURCE-AND-FOCUSED.json`,
`PROBE-AUDIT.json`, `FIELDS.json` and `focused.xml` retain results. Two reviewer
index assumptions were corrected (SequenceMatcher split one source block into
two ranges; one evidence `observed` field is text), with no candidate change.
No broad suite was repeated unnecessarily and no native build was needed.

This is host macOS evidence. No Linux, full R19, quota both-groupings,
productivity-gain, OS-readonly or measured RSS guarantee follows. Preexisting
embedded `accept_load` and profile-load phase-4 reservation defects remain
explicit separately reviewed follow-ups; B1/D6 configure rewrite performance is
also separate. The approval closes the combined retained-VM repair and this
bounded new-binder correction for the exact identities above.
