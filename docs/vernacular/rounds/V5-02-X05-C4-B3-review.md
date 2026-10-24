# V5-02-X05-C4-B3 independent retained-history review

Disposition: **request-changes**. C4B3-R1 remains open: the supported `rawget`
lookup path bypasses sticky missing-capability refusal. The candidate must not
advance as approved. No product fix was made during review.

The review used the shared `dev/planar-v2-vernacular` checkout at HEAD
`3171c49e7b92d3abda01bfbb8bfb0c6b29d4c67a`. Ownership was limited to this report
and `tests/native/v5-02-x05-c4-b3-review-evidence/`, with exclusive serialized
Debug. Product, author artifacts, prior evidence and installed release were
read-only. No Planar tools, skills, roles, delegation, commits, installation,
publication, merge, installed binaries or operator data were used.

## Reviewed identity and authority

The review considered AGENTS.md's CLAUDE.md target, implementation/roadmap and
v5 baseline, the exact external-profile contract, the approved B2 authorization
and review, and the complete B3 authorization including its missing-capability
and correspondence clarification. That explicit clarification controls the
missing-service behavior in both pure and history contexts.

| Author identity | SHA256 |
| --- | --- |
| `frozen-candidate.json` | `65c517516840505ee603ece2f28899830f19a0c036864aa37a852c126973eb7d` |
| `artifact-manifest.json` | `8cd9069f84100de6afd887214c05bd601afc9409890e1da331bc552298e9b8af` |
| `finalization.json` | `215ce1baac3b992e9aa6f5ac9ffcbc367c41066322450c9c91f15f16c2dc820b` |
| `REPORT.md` | `e42cf175003dbdc28a2afea1f06cb11b970fe77ccc7e1974c96ce1a3145c8507` |

The path-keyed candidate has 18 entries. The schema 1 artifact manifest is rooted
at the author evidence directory and has 166 entries; it excludes itself and the
finalization record. The reviewer checked the schema, root, safe paths, exact
membership, modes, byte counts and hashes before running verification. All 18
candidate entries match the current source/support/prose. The six deliberately
retained author compiler products remain unchanged; the author's 888 dependency
copies remain absent. Earlier attempts and red evidence were preserved.

The reviewer protection snapshot covers 80,646 entries outside Debug, Git
metadata, the coordinator-owned round and reviewer-owned paths. Final comparison
finds zero membership, mode, hash or symlink-target changes. AGENTS.md remains
the CLAUDE.md symlink; rewrite-release is absent and install-release is unchanged.

## Required finding

### C4B3-R1 — `rawget` bypasses missing-service poisoning

Location: `src/engine/runtime/runtime.cpp:1381` (`frozen_rawget`) and
`:1695` (`extension_context_index`), with context installation at`:1767`.

The new context `__index` marks `extension_history.refused` when a known service
is absent. The supported global `rawget` is the existing proxy-aware
`frozen_rawget`: it calls `read_backing` and then `lua_rawget` directly. It never
passes through `extension_context_index`, so an absent known service returns nil
without marking the invocation. A handler can catch the resulting nil-call error
and release a response successfully:

```lua
pcall(function() rawget(ctx, 'read_role')({}) end)
return {ok=false,error={code='schema',message='intentional',field='input'}}
```

A real history-only captured handler accepts that inner semantic response for
both absent `read_role` and `project`. A real zero-capability captured handler
similarly accepts its summary after caught calls through each absent
`read_history`, `read_role` and `project`. All five attempts have zero SQLite
opens/reads. This grants no service authority, but violates the approved rule
that a missing-capability attempt poisons the invocation even when caught.

Independent regression source is
`tests/native/v5-02-x05-c4-b3-review-evidence/private-probe.cpp:113` and `:160`;
raw results are `private/reviewer-private.xml`. Two cases fail with five failed
assertions, exit 42, because `execute()` returns success. The other three reviewer
cases pass. The ordinary 425-case selection and reconstructed author probes do
not cover this alternate lookup path.

Required correction: apply the same sticky denial to known missing-service
lookups through the supported proxy-aware `rawget` on the actual invocation
context, preserving nil results, actual-method enumeration and ordinary-table
behavior. Do not add denied callable methods or hidden authority. Add regressions
for pure and history contexts, caught nil-call tails and lookup without a call,
with healthy actual-method and unrelated-table controls. Preserve first native
error priority, cleanup, accounting and existing direct-lookup behavior. Any new
inline bookkeeping must be charged and verified. Use a fresh corrective round;
retain this failing candidate and its raw evidence.

## Other source assessment

The bridge retains the original invocation owner only during `execute()` and
calls the B2 helper lazily from the actual retained handler. There is no eager
ledger read on the semantic no-service path. `prepare_history()` uses only its
private admitted selector/path and original moved capture. All native format,
transaction, connection and duplicate descriptor owners unwind before the
callback pushes/freezes Lua values or raises a Lua error. The callback carries
only trivial locals across Lua allocation/longjmp; a native exception is handled
before entering the Lua error path. The copied production observer confirms no
Lua allocator requests during the native helper and a real writer transaction
can commit before Lua resumes.

Invocation transport substitutes the admitted ledger slot with native opaque
userdata before freezing. History IDs remain exact strings including absent
`"0"`, while limits/general decimals remain Lua integers with native presence
and default distinctions. Only an empty capability set or exactly
`history_read_v1` executes; role/projection and combined capability admission
remain closed. Existing pure summary and semantic-error protocols remain.

A successful read retains authenticated native facts privately. The raw scanner
compares complete membership, kinds and scalars before owned response conversion;
owned encoding retains the reserved Glaze 16 MiB bound. The independent deep-copy
probe accepts a recursively copied record/list graph, then refuses a nested
integer changed to a Lua float, nested payload extras, changed event sequence,
extra array membership, array-to-record retagging and a cycle. Semantic error
responses may omit service execution; both semantic branches still reach original
receipt verification before release.

Native registration/format/history errors are retained before fallible copying
and outrank caught/later Lua errors and semantic responses. The reconstructed
actual allocation probes preserve the original foreign-ledger error through a
failed diagnostic copy. Failed service/invalid argument/direct missing-method
and repeated calls remain sticky on their tested paths. The independent repeated
service probe confirms no second native read, balanced reader closure and cleared
bridge, input and push-memo fields. Manual preparation cannot authorize a later
handler success without actual service sequencing.

The pinned compiled layout independently reproduces command_state 976→1008 and
VM 1384→1416, with invocation implementation 488 unchanged. Initial cumulative
native accounting increases by exactly 32 bytes. No quota reset or full X06
shared-invocation accounting is inferred.

## Independent verification

Exact argv/cwd/timing, raw logs, copied production sources, JUnit and dependency
inventories are retained in the reviewer evidence directory. Python used
`python3 -B`. The independent production gate ran against the exact final
candidate, with a no-work build before and after.

| Check | Result |
| --- | --- |
| Focused production Debug gate | 425/425; zero failures/errors/skips/disabled; 172.61 seconds |
| Runtime; format; SQLite; history-read; CLI cases | 300; 82; 33; 3; 7 |
| Inventory | 786; all 773 original names preserved |
| Reconstructed author private probes | 7/7 green |
| Exact pre-B3 retained-handler regression | One expected failing case, exit 42 |
| Additional reviewer cases | 3 pass; 2 fail with 5 assertions, C4B3-R1 |
| Six common contract vectors | Pass |
| Historical/current source and generated-variant audits | Pass |
| Pinned formatting/Doxygen | 118 active sources pass |
| Build before/after focused gate | Ninja reports no work |
| Shared/copied compiled dependencies | 888 members; exact membership/modes/hashes unchanged |
| Compiled layout and initial native charge | Exact 32-byte increase |

The three passing reviewer cases exercise deep history correspondence, a caught
second service call without a second native read, and final receipt mutation
for semantic errors both with and without service execution. The last case
mutates both original module and admitted ledger receipts after the handler;
all four combinations return `capture_changed`, and no-service controls retain
zero native content reads.

Private builds recursively expanded response files and copied every object,
PCM, archive and dylib input, including referenced standard-library PCMs and
relevant LLVM libc++ dylibs. Implicit module/module-map builds were disabled.
All original and copied compiled inputs were compared before cleanup; complete
removal membership/modes/hashes are retained. Only reviewer-owned disposable
copies and compiler products were removed after that proof. Sources, commands,
logs, XML and failure evidence remain. macOS system runtime libraries remain
platform prerequisites.

## Limits and handoff

This review is focused macOS Debug evidence. It claims no new complete suite,
release or Linux gate. The latest complete approved gate remains 676 per
configuration; public init creates format1. Role/projection services, X06
recorded-profile interpretation/history acceptance/shared accounting, X07 public
cutover, X08 mandatory external-profile acceptance, execution/RA01 and Linux
remain open. Original cooperative filesystem stability limits remain unchanged.

C4B3-R1 requires a fresh corrective round and fresh independent review before
this candidate can advance. Debug and reviewer ownership are released after the
final protection/identity checks and evidence finalization. No further edits or
builds follow that handoff.
