# V5-02-X05-C4-A independent review

Disposition: **approve**. No required changes or unresolved product choices were
found in this bounded zero-capability execution candidate. This approves C4-A,
not C4 read services, public profile invocation, CLI admission or a complete gate.

The reviewer worked in the shared `dev/planar-v2-vernacular` checkout at HEAD
`3171c49e7b92d3abda01bfbb8bfb0c6b29d4c67a`. Ownership was limited to this report
and `tests/native/v5-02-x05-c4-a-review-evidence/`. Product, author evidence,
historical records, installed release and operator data were not modified.
No Planar tools/skills/roles/queue, subagents, commits, installation, publication
or merge were used. Debug verification was serialized under exclusive review
ownership; `build/rewrite-release` remained absent.

## Exact reviewed identity

All 231 workspace-relative candidate entries and all 235 evidence-relative author
artifact entries (including directories) were checked for membership, modes and
exact bytes or symlink targets. The independent protection baseline matches the
author's original 63,055 protected entries plus 8,248 release/absent-release
entries, with no discrepancy. The final comparison preserves all 71,303 entries.
`AGENTS.md` remains the `CLAUDE.md` symlink.

| Author artifact | SHA256 |
| --- | --- |
| `frozen-candidate.json` | `3e11f8a9e9e15792e0d975ddb0689f8b69fc0a0aff47a1d5af729f179c9b847b` |
| `candidate.diff` | `1f1a493f0e3a41a4c1cdadb4e1a052301399803bd0d36a876588573ef0a0b309` |
| `artifact-manifest.json` | `fe8f191b3752724f0d914595e5e0c5c0cad5780adfcea82afc6e686c2981bec2` |
| `REPORT.md` | `d07b4e4da7fcf15bcda385cecd743d67754eece18e42315196cdbf3416d8702c` |
| `finalization.json` | `bb7672a41ec04c7fca1a07b344be90d114e6b39651f8d803a5bd95c8449e614d` |

Product/test hashes are `538132c2efc9dca0d99d558f25e456bf9ff0188b953695591a412ff3817c961d`
for `runtime.cpp`, `e7e63035b0b217456e8db6dab5b45a1bc0d6a52ca531707fc591897a85ad0cea`
for `runtime.cppm`, and `4947093f0774a15707699424d5ab736576fe070c929075dc0127c9a0a3023923`
for `external_registration.t.cpp`. The approval binds these identities and the
frozen manifest, rather than an unspecified dirty-tree diff against the legacy HEAD.

## Source and contract assessment

The review used the repository instructions, implementation plan, roadmap, v5
design, complete C4 brief and amendments, X-REG/X-PROJ/X-LIMIT and render-contract
error schema at lines617–644. The exact bounded implementation agrees with them:

- `runtime.cpp:4292` executes only the original captured owner and its privately
  admitted command/scalars. The retained registry F is selected from original
  `external_entries`; command counting starts after all five shipped entries,
  filters out shapes/projections and preserves registration order across modules.
  Fallback staging, selected index, attempted/executed state, private admission,
  first error and move behavior are consistent with C3-B2 and the C4 amendments.
- The capabilities check precedes handler entry. Generic profile `invoke` remains
  closed; the new method exposes no service/provider, userdata, host parser or
  output publication. Original receipt verification occurs before F and after
  response conversion/reserved encoding, for both permitted response branches.
- At `runtime.cpp:4360–4419`, scalar construction reserves native owners, map/key
  capacity and copied bytes before allocation. Existing validation/transport
  performs its separate cumulative visits. The one `begin_invocation` transition
  precedes F; no native/node/audit reset is added. The new shared boolean occupies
  existing pinned-target padding, consistent with the author's positive controls
  for both embedded/profile owners and complete-owner sizeof/alignof evidence.
- `execute_shape` at `runtime.cpp:1638` freezes invocation/context together and
  calls the exact F with two arguments. The new branch defaults off for existing
  users. Protected Lua frames retain only trivial indices/borrowed views; native
  RAII remains outside `lua_pcall`. Temporary backing/tag roots survive raw scan,
  conversion and encoding, then the shared cleanup removes their entries.
- `runtime.cpp:2089–2334` performs raw, closed-schema admission before owned copy.
  Exact known keys/types, UTF-8/no NUL, exact command/version and the summary byte
  ceiling are checked. Inner E admits the fifteen existing codes, optional logical
  file/positive signed32 line and only code-specific details. Supported versions,
  sorted ownership paths (including duplicates), traversal labels, counts, IDs,
  option/operation strings and budget section records use the established rules.
  Optional details do not become mandatory, and no message/field/list quota is
  invented. Sparse/mixed lists and wrong record/list kinds refuse.
- At `runtime.cpp:4429–4459`, malformed protocol and native resource failures
  remain distinct. The inherited depth/node/native conversion charges precede
  copies; reserved Glaze complete-result encoding supplies the unchanged16MiB
  encoded boundary. The original Lua response and proxy backings remain rooted.
  Native first failures publish before fallible diagnostic return copies and
  survive retries/moves. Permitted E remains response data and consumes execution.

No evidence is interpreted as proving semantic honesty of trusted text or as
completing whole-invocation capture/VM budgeting. Those limits remain explicit.

## Raw author evidence and chronology

The author raw JUnit contains422 initial focused cases and47 final exact-source
cases, each with zero failures/errors/skips. The initial422 gate precedes the
isolated summary-overlimit classification correction. `tested-to-final.diff`
contains only the new raw-summary status, its execute adaptation and the correct
regression assertion; it does not change shared helpers. The real red records
`internal` versus required `resource_limit`; final47 and private probes cover the
corrected bytes. This review does not claim a current-byte422 run.

The initial preselection exploration asserted the wrong `profile` fallback. Its
failure is not counted as contract-correct regression proof. The disclosed later
reconstruction removes only fallback staging, fails the final `catalog.commands`
assertion once, and has a matching exact-product green. The historical source
support's whitespace correction and the later layout-only support delta remain
preserved rather than overwritten.

Author private raw evidence has four production probe cases;106 summary and98 E
one-shot native allocation positions; diagnostic-copy request80 with single and
double faults; two post-F owned-copy receipt mutations with unchanged controls;
watched native read controls versus zero reads on closed capability execution;
and separate forced-GC, node/native boundary and layout controls. The layout raw
output records command976/prior976, complete owner1384/prior1384 and state400
for both owner kinds. These are controlled/private Debug results, with no natural
host-exhaustion, Linux ABI or release-rerun claim. Six vectors, six source audits,
pinned lint/Doxygen over118 sources and no-work evidence were inspected; no
unnecessary full or release rerun was performed.

## Independent verification

Reviewer commands and raw JUnit/logs are under
`tests/native/v5-02-x05-c4-a-review-evidence/`:

| Independent check | Result |
| --- | --- |
| Exact current focused execution/projection/admission selection |47/47 in8.88s; zero failures/errors/skips |
| Inventory |756 names; all743 prior names preserved and13 execution names added |
| Ordinary all + planar_tests build | No work |
| Reviewer-authored production adversarial cases |5/5; zero failures/errors/skips |
| Reviewer controlled full-GC case |1/1; zero failures/errors/skips |
| Compiled dependency protection |888 original and copied members/modes/hashes unchanged |
| Final candidate/author evidence/protected comparison |231/235/71,303 unchanged |

The five independent tests exercise retained nested E proxies and owned response
survival after VM destruction; all fifteen codes with absent details plus empty
tagged lists; sparse/mixed/mistyped detail containers and records;1MiB escaped
summary and multibyte byte boundaries; a4MiB control-character semantic message
whose JSON expansion exceeds16MiB; and shape/projection registrations interleaved
before commands without shifting the original complete command index. An
unavailable `engine.md` call also confirms closed external constructor exposure;
its refusal is not attributed to the result scanner. The GC variant adds only
`lua_gc(LUA_GCCOLLECT)` immediately before raw response scanning and reruns the
retained nested E case after its healthy production control.

Private compilation copied all888 shared object/PCM/archive/dylib dependencies,
including every explicit link/module input and pinned external dylib targets.
Response files were expanded, object/PCM/dependency outputs redirected and implicit
modules/maps disabled. Complete shared membership and all copied identities were
checked afterward before deleting disposable compiled copies. Raw generated
sources, commands, identities and XML remain. Two reviewer setup attempts failed
before any compilation: alias identity handling for a pinned dylib, then replacing
its read-only owned copy. Their scripts/manifests and failure records remain;
resolving the alias and unlinking the owned destination corrected the helper.
They were neither product failures nor failed test cases.

## Handoff and limits

C4-A is approved at the exact identity above. Actual read service protocols,
opaque-slot/history ABI, retained read/project correspondence, service order/once/
poison orchestration, lower capture before-read binding, output publication seals,
X06 interpretation/history/acceptance and shared capture budgets, X07 CLI cutover,
X08 mandatory acceptance, execution/RA01 and Linux remain open. Public init still
creates format1; the latest independently approved complete gate remains676 per
configuration. Neither this review nor the focused756-name inventory expands it.

All reviewer processes have finished, disposable compiled copies are removed and
exclusive Debug plus all owned paths are released to the coordinator. No further
reviewer edits or builds will follow final handoff. The reviewer artifact manifest
and finalization bind this report and the exact verification evidence.
