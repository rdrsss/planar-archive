# V5-02-X05-C4-B3-C01 independent combined review

Disposition: **approve** for the combined C4-B3+C01 scope. **C4B3-R1 is closed.**
No blocking findings remain. This approves the internal retained history handler
and bounded missing-service correction; it does not close X05-C or the mandatory
external-profile acceptance gate.

The review used the shared `dev/planar-v2-vernacular` checkout at HEAD
`3171c49e7b92d3abda01bfbb8bfb0c6b29d4c67a`. Writes were limited to this report
and `tests/native/v5-02-x05-c4-b3-c01-review-evidence/`. Debug was exclusively
owned and serialized. Product, author artifacts, prior evidence and installed
release remained read-only. No Planar tools, skills, roles, delegation, commits,
installation, publication, merge, installed binaries or operator data were used.

## Authority and reviewed identity

The review considered AGENTS.md's CLAUDE.md target, implementation/roadmap and v5
baseline, the complete B3 and corrective authorization, the original independent
B3 finding, and the exact external-profile contracts. B3's explicit clarification
requires known missing-method lookup to return nil and poison both pure and
history invocations, including lookup without a call. Semantic errors may omit a
history read, while original receipts must still pass the final seal.

| Corrective author identity | SHA256 |
| --- | --- |
| `frozen-candidate.json` | `97302a8a54a7bb1ea8352618819adb0e067a2cf9be5c84cd46c420c2fd36cf2d` |
| `artifact-manifest.json` | `f1ff95c407cae2e260f439e4ed5e416106e475efe64fb476a9c0b54400ffda2d` |
| `finalization.json` | `e6463907ea43e2f85edb4852a309ddcd03aeb64a2726aeac3e67f98751076cb8` |
| `REPORT.md` | `63dcb2bf0f5be95ba55b57e4ea7bf19057dbd00fe085617c8effa78e105113db` |

The path-keyed corrective candidate has 17 entries. Its schema 1 artifact
manifest has 134 entries rooted at the author evidence directory, excluding
itself and finalization. Schema, root, safe paths, exact membership, numeric modes,
byte counts and hashes were checked before verification and again afterward.
The original B3 candidate identity remains
`65c517516840505ee603ece2f28899830f19a0c036864aa37a852c126973eb7d`;
its 166-entry artifact manifest remains
`8cd9069f84100de6afd887214c05bd601afc9409890e1da331bc552298e9b8af`.
All original B3 artifacts and all nine original candidate entries not superseded
by C01 match exactly, including runtime.cppm and runtime.t.cpp.

The review's `reviewed-candidate.json` merges those nine preserved B3 entries
with C01's 17 current entries, recording the exact combined 26-entry snapshot:
`0eebf0495c4f69c0692ee44c2dedcf0692f9bec532f8eaeb276aff0b604fd97a`.
Prior red evidence and failed author attempts remain intact. They are historical
or negative controls, not additional green runs.

## C4B3-R1 closure

`runtime.cpp:1385` implements one missing-service predicate. Both
`frozen_rawget` and `extension_context_index` call it after their raw lookup.
It requires the exact active context identity, a nil result, and one of the three
known service names. Thus read_history/read_role/project absence has identical
sticky refusal through ordinary indexing and supported proxy-aware rawget.
It returns nil rather than installing denied callables. Arbitrary tables,
including frozen tables carrying those names as data, retain ordinary behavior.

`execute_shape` installs that identity only after obtaining the actual context
proxy. It retrieves the proxy's private registry backing directly and uses that
same table for method installation and the index closure. History has exactly
one actual read_history method through indexing, rawget, next and pairs; pure
contexts enumerate none. Iteration does not attempt or poison missing services.
Captured lookup/iterator aliases continue to use the same native closures.
Numeric traversal/empty array behavior and unrelated keys remain unchanged.

The proxy is strongly rooted by the protected input stack and the ordinary
registry backing map; its object address cannot be collected and reused while
active. Execute's existing RAII cleanup clears the complete bridge, including
identity, before clearing input and releasing transient Lua roots. The protected
setup and lookup frames have no nontrivial local owner crossing Lua longjmp.
Actual allocation refusal immediately after identity installation and during
service result push verifies cleared owner/identity/input/push-memo fields.

The reviewer independently reconstructed the original five reviewer cases using
preserved pre-C01 B3 production. The same two missing-service cases fail with
five failed assertions, exit 42, while the three related controls pass. The exact
same five cases pass on current production. Raw evidence is in
`private/baseline-reviewer.xml` and `private/reviewer-private.xml`.
The production matrix additionally covers pure summary/semantic tails, ordinary
and raw aliases, lookup without call, caught nil-call, exact one/zero-method
enumeration, available raw history in both formats and ordinary/frozen data keys.
These results close C4B3-R1 without granting additional authority.

## Combined retained-history assessment

The retained handler binds synchronously to its original moved invocation owner;
no caller provider or supplied facts can substitute for it. Only zero capabilities
or exactly history_read_v1 reaches execution. Role, projection and other
combinations remain refused before handler entry. The opaque ledger userdata and
exact history-ID strings/presence, integer general decimals and native default
limit remain intact. Manual prepare_history cannot authorize later handler success
without the actual once-only service sequence.

History is lazy. The callback invokes only original prepare_history with its
private admitted selector and path. Its native format file, snapshot/transaction,
connection and descriptor adapter unwind completely before Lua result allocation
or error propagation. Independent copied-production observations show zero Lua
allocator requests inside the native read and a successful real writer transaction
before Lua resumes. A semantic error without service has zero SQLite opens/reads;
receipt verification still runs before release.

Failed service, invalid arguments, repeated calls and missing known methods remain
sticky despite catches. The original typed native error is retained before
fallible diagnostic copying and outranks later Lua errors or responses. Private
actual native/Lua allocation probes preserve that priority and cleanup. A new
reviewer case also confirms an original foreign-ledger format failure wins over
a later missing-method lookup. Another confirms missing lookup prevents a later
otherwise valid history call from opening or reading SQLite.

Successful history returns are compared directly against privately retained
native facts before owned response conversion. Exact deep copies are allowed;
nested integer-to-float changes, payload extras, sequence changes, extra array
membership, retagged containers and cycles refuse. The complete response retains
the reserved Glaze 16 MiB ceiling. Independent original module/ledger receipt
mutations before/during read and after handler entry suppress results, including
semantic errors with and without service. Caught second calls perform no second
native read.

The pinned compiled comparison independently reproduces bridge32→40,
command_state1008→1016 and VM1416→1424; invocation implementation remains488.
Initial cumulative native charge rises from17852407 to17852415, exactly8 bytes.
The existing sizeof(bridge) charge accounts this pointer before use. No new quota,
refund/reset, process RSS claim or X06 whole-invocation sharing is inferred.

## Independent verification and protection

Exact argv/cwd/status/timing, sources, raw logs, XML, inventories and comparison
results are retained in the reviewer evidence directory. Python used `python3 -B`.

| Check | Result |
| --- | --- |
| Focused production Debug gate | 429/429, 173.33 seconds; zero failures/errors/skips/disabled |
| Runtime / format / SQLite / history-read / CLI cases | 304 / 82 / 33 / 3 / 7 |
| Inventory | 790; all 786 B3 names preserved |
| Current private copied-production probes | 16/16 green: 1 lazy-success, 7 boundary, 5 prior-reviewer, 3 fresh-reviewer |
| Pre-C01 original reviewer reproduction | Expected exit42; 2 failing cases, 5 failed assertions |
| Fresh reviewer controls | Iteration-supplied actual service calls; missing lookup blocks later read; original native failure priority |
| Six common contract vectors | Pass |
| Six historical/current source and generated-variant audits | Pass |
| Pinned formatting/Doxygen | 118 active sources pass |
| Build before/after gate and after private verification | Ninja reports no work |
| Compiled layout and cumulative native charge | Exact8-byte increase |
| Shared/copied compiled dependency inputs | 888 members; unchanged membership/modes/hashes |
| Protected baseline outside owned paths/Debug/Git/coordinator round | 80,868 entries; zero changes |

Private builds recursively expand response files and copy all object, PCM,
archive and dylib inputs, referenced standard-library PCMs and relevant LLVM
libc++ dylibs. Implicit modules/module-map builds are disabled. Complete shared
and copied membership/modes/hashes are verified before cleanup. Only the
reviewer's 888 dependency copies and six disposable compiler products are
removed, with full removal inventory preserved (894 files,1384743344 bytes).
Sources, commands, XML, logs and layout evidence remain. macOS system runtime
libraries remain platform prerequisites.

The independent protection snapshot includes prior evidence and install-release.
Final comparison finds zero membership, mode, hash or symlink-target changes.
AGENTS.md remains the CLAUDE.md symlink; rewrite-release remains absent. The
artifact manifest and finalization bind this report, final identities, protection
and cleanup. Debug and reviewer-owned paths are released at final handoff; no
further edits, builds or tests follow.

This is focused macOS Debug approval. The latest complete approved gate remains
676 per configuration; no new full, release or Linux gate is claimed. Public init
creates format1. Role/projection services and other combinations, X06 recorded
profile interpretation/history acceptance/shared accounting, X07 public cutover,
X08 mandatory external-profile acceptance, execution/RA01 and Linux remain open.
Original cooperative filesystem stability limitations remain unchanged.
