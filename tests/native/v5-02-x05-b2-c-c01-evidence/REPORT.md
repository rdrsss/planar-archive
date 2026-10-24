# X05-B2-C-C01 retained registry reference correction

Status: correction candidate pending fresh independent review of combined B2-C+C01.
No self-approval, full milestone, release or Linux gate is claimed.

Shared checkout: `dev/planar-v2-vernacular`, starting HEAD
`3171c49e7b92d3abda01bfbb8bfb0c6b29d4c67a`. The intentional dirty tree and prior
candidate/evidence remain preserved. Ownership covers runtime.cpp, new C01 support/
evidence and six current prose files. No other product/test/API, codec, Lua,
CMake, CLI, history or vendor files changed. The coordinator owns the round
record. No Planar tools, installed binaries, operator data, commits, installation,
publishing or merging were used. AGENTS remains a symlink to CLAUDE.md.

## Correction and lifetime

B2C-R1 is reproduced before the correction using the exact original reviewer
observer. `exact-red/registry.log` reports retained references 10/11 changing from
function (type 6) to table (type 5); the observer deliberately exits 42. Its runtime
copy reverses exactly to the preserved baseline runtime, SHA256
`123c785e971e9a81a4adf36d985f30b90d6dadc1c3fb9c7df3df7b273c45fc7f`.
The original review helper/evidence are unchanged. The owned exact-red helper
loads that preserved baseline for future reproduction.

The production correction uses two private immutable constexpr char objects as
address-identity keys with `lua_rawsetp`/`lua_rawgetp`. Pinned Lua is 5.5.0;
its `luaL_ref` allocates integer keys using its freelist or raw length. Light-userdata
keys for these two distinct private addresses cannot overlap those integers.
No reservation, renumbering or setup ordering changes are needed. Embedded setup
never accesses these snapshot keys. The snapshots remain strongly owned by each
VM registry until Lua closes. Their contents, first-publication timing, existing
ready bit and temporary membership cleanup algorithm are unchanged. Raw deletion,
alias lifetime through response conversion, protected frames, sticky diagnostics,
terminal owner behavior and cumulative accounting remain unchanged.

The source diff changes only the snapshot key declarations and their single
publication/read operations. No public test hooks are added.

## Regressions and verification

The exact original observer now exits 0 and reports all five shipped reference
identities/types preserved (`final-exact-green/`). The expanded owned observer
(`final-registry/`) captures all five shipped F references and all ten external
D/F references from five external entries across two modules: one command,
three shapes and one projection. This includes selected and nonselected entries.
Every boundary compares reference number, Lua type and exact `lua_topointer`
identity against the original post-registration baseline. It passes embedded-only
setup (five F), the first and 24 repeated actual selected-F calls with explicit GC,
ordinary execution failure cleanup, and controlled native codec reservation
refusal cleanup. Failure modes also check terminal retry. Each private owner
asserts zero live Lua accounting after destruction. Transform records prove exact
reversal to production; observations do not replace dispatch or open profile CLI.
The resource mode deliberately sets the existing charged counter at its ceiling;
it is controlled quota evidence, not natural host exhaustion or RSS accounting.

`development-registry/` preserves the earlier successful development observer.
The first development product build was successful; final verification below is
the authoritative fully logged gate. No failed development compile/run was hidden.
`exact-green/` preserves the first original-observer green run before the final
formatting pass; `final-exact-green/` binds the final frozen runtime.

## Frozen final gate

`final-gate/commands.json` records exact direct serialized Debug commands,
exit codes and timings. One pre-gate freeze covers 399 source/configuration/
fixture/support/current-prose entries; all match after the complete gate.

| Check | Result |
| --- | --- |
| Configure; `all planar_tests` build | Pass |
| Inventory | Exactly 619 names, identical to prior B2-C; prior 612/606/600/582 preserved |
| Focused selection | 353/353, zero failures/errors/skips/disabled |
| Supplemental history/codec | 11/11, zero failures/errors/skips/disabled |
| Original sticky callback regression | One passing case, 26 assertions; also included in focused selection |
| Exact original retained-reference observer | Red exit 42 before fix; green exit 0 on frozen source |
| Expanded retained-reference observer | Four modes pass; embedded setup, 25 successful calls/GC, execution failure, controlled resource failure |
| Production native allocation sweep and embedded/moved controls | Two cases, 482 assertions; all 67 observed allocation requests independently refused and typed |
| Reversible original lifetime/resource probes | Six modes pass: success, codec, instructions, Lua allocator, nodes, bridge diagnostic allocation |
| Independent shape-definition vector; four existing vector families | Pass |
| Historical, D6, D6-C01, D7-B, B1, B2-A exact-source audits | All six pass |
| Pinned formatting/Doxygen | Pass, 118 active sources |
| No-change build | `ninja: no work to do.` |
| Original B2-C author/reviewer artifacts | All 153 + 36 unchanged |

The original lifetime probe is adapted in owned C01 support solely to read the
new pointer-key snapshots; its exact reversible runtime substitutions are recorded
in `final-probes/transformations.json`. Historical support is immutable.
Across 100 successful actual calls, tags remain 42, backings 262, sealed identities
311 and post-GC live Lua bytes 396607. Instructions rise from 1000 to 43000,
nodes from 4176 to 29025, native charged bytes from 17837188 to 37187629.
No work counter is refunded/reset. All six modes also check zero live Lua
accounting at owner destruction. The bridge mode arms an actual one-shot native
allocation refusal during sticky error adaptation and preserves the first diagnostic.
The allocation sweep denies one observed request at a time and allows later
requests. These controls do not establish natural host exhaustion or process RSS.

## Handoff

Candidate diff/manifest bind the correction relative to its retained scoped
baseline, support, six current prose files and this report. The original B2-C
candidate/evidence remains separately bound and unchanged for combined review.
Artifact manifest binds raw logs and baseline/variant copies without self-hashing.
Finalization rechecks the same 399-entry freeze before and after hashing. Debug
ownership is released only after final hashes are reported; no edits follow release.

The correction leaves generic/public profile invocation closed, public init on
format1 and B1/B3/B4/X06–X08 semantic integration, selected W/render identity,
history/acceptance, public cutover and mandatory external-profile acceptance,
execution/RA01 and Linux open. The latest full independently approved gate remains
A4's 582 cases per configuration. No full 619-case or release run is claimed.
