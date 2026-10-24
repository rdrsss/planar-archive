# X05-A2 repeatable sealing foundation candidate

Status: candidate pending fresh independent review; no self-approval. Shared
checkout `dev/planar-v2-vernacular`, HEAD
`3171c49e7b92d3abda01bfbb8bfb0c6b29d4c67a`. The coordinator owns the round and
its authorization. Existing dirty rewrite sources and historical evidence were
preserved. No legacy Planar tools/roles/skills/queue, installed binaries, operator
data, commit, installation, publication or merge were used.

## Bounded production behavior

The private `seal_new_roots` protected-call seam accepts exact retained roots as
stack arguments. Native root indices do not introduce a synthetic graph depth.
Initial `seal_environment` retains the global-first/callable root ordering and
one-time registration removal. Repeated sealing never assigns into frozen globals.
The public runtime interface is byte-identical to baseline. The existing invocation
tail differs only in refusing an owner whose sealing failed.

A strong private registry set holds only successfully sealed table/Lua-closure
identities. It is distinct from each boundary's DFS/prototype memo. After native
C-root/identity checks, the guard can treat previously successful identities as
immutable atoms. Raw-empty proxy tables are never an independent trust signal.
The freezer skips prior successful identities, preserving their existing backing
map, proxy contents and all aliases. It still audits every newly retained root
before any mutation, including newly reachable helper writers and native closures.
Direct C callable roots and exact-identity-unknown stateful natives still refuse.

New identities are accumulated privately. After all roots freeze, the helper
copies the prior successful set into the local memo and replaces the existing
registry slot in one nonallocating publication. Copy work charges existing
conversion visits; Lua allocations use the existing VM allocator. A failed private
union leaves the old successful set unchanged. The owner is poisoned before
reservation/audit and cleared only after publication. Reservation refusal, audit
refusal/exception, partial freezing, Lua allocation failure and union failure all
leave it terminal. A poisoned entry refuses before root processing; no later
boundary can reinterpret a partially mutated graph as safe.

`pairs` now captures and returns the exact stable closure installed as global
`next`; it does not create another C closure on each call. The pristine native
snapshot sees global `next` directly and still never traverses C upvalues. There
is no per-call enrollment or C-pointer whitelist. The existing reviewed engine
callback enrollment and hidden proxy write/length callbacks remain unchanged.

The protected frame still reserves 2048 relative slots before root pushes/native
RAII. Guard preflight and traversal push checks remain nonallocating, and all
native RAII unwinds before any later Lua allocation or longjmp. Each audit shares
the existing checked 512 MiB cumulative native allocation accounting, with no
refund or reset. Static inspection totals accumulate separately from executed Lua
instructions/converted nodes. Existing depth256, stack and embedded invocation
limits remain. This foundation does not claim the future cumulative external
registration 64 MiB/10-million-instruction integration.

## Verification and composition

Attempt01 builds Debug `all` plus `planar_tests`, registers 555 unique names and
passes the exact broad focused selector: **140/140**, zero failures/errors/skips
or disabled cases. All 552 baseline names and all 520 last full-gate names remain.
The focused set includes 3 new A2 cases, 17 existing A1 guard cases, 15 historical
A0 cases, 32 command VM cases and 73 affected runtime/interpretation/load/
acceptance/CLI/workspace cases. Existing packet/command goldens are unchanged.

Attempt01 then fails in new test support from an unterminated Python string;
product/test/documentation bytes were unchanged by that correction. Attempt02
compares full source manifests, allows differences only in the two A2 support
scripts and composes the exact original JUnit/LastTest/command with rerun probes
and final checks. `focused-reuse.json` names the original command and XML hash;
no second 140-case run is claimed. It also serially configures/builds the same
product source again. Both source freezes remain available, including the failed
probe output. No full555, RelWithDebInfo/release or Linux gate is claimed.

Ten new reversible production scenarios compile a source copy containing the
actual private production helper and call it after the normal embedded boundary:

- Success executes three protected boundaries with newly source-compiled roots.
  The second root retains frozen globals, a shared API table, fresh nested tables,
  aliases and the exact `pairs` iterator. The third retains the second closure.
  Three later invocations preserve contents/aliases, reject writes through prior
  aliases and execute invocation-local nested scratch. A weak-value/full-GC probe
  drops the explicit closure reference and proves the successful identity set
  keeps it alive. Exact iterator identity and ordinary `next`/`pairs`/`rawget`
  behavior are checked before freezing and after later boundaries/GC.
- Newly retained scalar writers, a new opaque context using the same digest C
  pointer as a trusted callback, and a direct native callable root refuse with
  existing internal bridge diagnostics. None is enrolled from module data.
- Existing native accounting exhaustion and injected upstream `bad_alloc` return
  existing typed resource-limit bridge diagnostics after normal C++ unwinding.
- Controlled partial-freeze and private-union Lua errors leave the owner poisoned
  and the prior successful-set identity unchanged. The union probe fails before
  registry publication, after some private copy work.
- Actual Lua allocator refusal during freezing returns `LUA_ERRMEM`; existing
  caller classification is resource-limit. The stack fixture lowers only the
  earlier backing reservation to20, performs protected supported GC and denies
  real growth **816 -> 32896 bytes**. The active repeat-helper reserve remains2048;
  it refuses with existing stack resource state before audit charges advance.
- Every failed second boundary is followed by an attempted boundary on the same
  state, which refuses at poisoned entry. It cannot bless or reuse the graph.

The success probe's cumulative native charge is **1,357,860 -> 1,522,284 ->
1,681,156 bytes** across normal embedded, second and third boundaries. These
fixture figures include the reviewed audit allocations and conversion accounting;
there is no per-boundary reset. Full transformations, reverse-equality checks,
original/copy source hashes, compile/link commands, objects, binaries, mode and
raw outputs are retained under `attempt02-production-probes/`.

The red control disables only the prior-identity freezer shortcut in the new
helper copy. A subsequent boundary then refreezes prior raw-empty proxies and
loses their backing contents; the same invocation detector fails. This is a
modified-helper control, not a claim that A1 had a repeatable API. A1's approved
one-shot API had no production repeated-boundary seam. Both the failing control
and fixed-helper output are retained; historical feasibility generators were
never rewritten.

## Preserved development failures

Development probe01 uses unavailable curated `string.gmatch`; its exact load
failure is preserved. Probe02's supposed reservation fixture instead reached a
real freezer allocation failure because the outer backing reserve prevented
stack shrinking; its output remains. Probe03 corrects that reserve and passes
all real scenarios but its red-control build fails the pinned self-assignment
warning. Probe04 fixes the red control and passes ten scenarios plus the control.
Probe05 adds explicit unsealed helper checks, strong closure retention across GC,
failed publication-set identity checks and cumulative-third-boundary output.
Attempt01 preserves the later support syntax error. Attempt02 supplies the final
composed evidence. No production detector was weakened and no historical test or
fixture changed.

## Scope and limits

Owned product edits are runtime.cpp and closure_guard's interface, implementation
and colocated tests. No CMake edits were needed. Six affected current reference/
status files and new A2 support/evidence complete the change. `AGENTS.md` remains
the CLAUDE.md symlink. Baseline/source scope audits preserve old tests, shipped Lua,
vendor, all historical support/evidence and unrelated changes. The source baseline
contains 19,329 entries, including the new baseline helper already created before
capture; final freezes contain 19,331. Only root .git/build and this A2 evidence
root are excluded; nested names, regular-file modes and symlinks are retained.

A3 must integrate captured ordered source chunks, native owner frames, exact
add/shape/projection descriptors and final catalog. It retains strong D/F refs,
validates/copies final D at module end before sealing D/F and preserves each
specified refusal classification. Complete module isolation/cleanup and cumulative
whole-registration gates remain required. A2 adds no external module execution,
public profile flags/constructor, profile storage/history dispatch, public cutover,
shape/projection semantics, execution/RA01 or Linux support claim. Latest full gate
remains 520 per configuration. The mandatory external-profile gate remains open;
public init creates format1.

## Final gate and identities

Attempt02 completes all 14 executed commands with exit zero, composing the
source-identical attempt01 140-case run as described above. Canonical, record-v1,
request-v2 and effective-provenance vectors pass. Historical B2, D6, D6-C01 and
D7-B source audits pass; historical A0 runtime/interface forward/reverse and exact
shipped-source assembly checks pass. Pinned formatting/Doxygen checks all **116**
active sources. The final all+planar_tests build reports `ninja: no work to do.`.
Full-tree checks before/after every final command and finalization verify all
19,331 source entries, modes and symlinks. Final build identities retain **747**
executable/object/PCM/cache/build-graph hashes.

- Source-freeze SHA256: `05caa65f7723a15331961db474d1f2bdde1c7680310d94643dfe79906f7e6d40`.
- Candidate owned.diff SHA256: `c08997657a2afc4f4bfc354f2e0b964dbf237a2dab669450d641a7845679c9a2`.
- Public runtime.cppm SHA256 (unchanged): `525fed5cf6fd2b5c30670a47baf709cf4052cf664edeb5d1f47cb6d783431f78`.

The artifact manifest excludes itself and finalization.json to avoid circular
hashes. Finalization binds the report and artifact manifest. This is not approval.
Serialized build ownership is released after final source/artifact hashing;
no further coder edits follow this handoff.
