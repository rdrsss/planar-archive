# V5-02-X05-C4-C1 independent review

Disposition: **approve** for the protected nested projection prerequisite.
No required changes or unresolved product choices remain in this scope. This
approval does not close C4-C2 or grant a projection service.

The shared checkout is `dev/planar-v2-vernacular` at HEAD
`3171c49e7b92d3abda01bfbb8bfb0c6b29d4c67a`. I owned only this report,
`tests/native/v5-02-x05-c4-c1-review-evidence/`, and exclusive serialized Debug
verification. Product, author artifacts, prior evidence and installed release
were read-only. I did not revert others, delegate, invoke Planar tools/skills/
roles/queues, commit, install, publish, merge or access operator data.

## Authority and exact candidate

The review considered the active guide, implementation/roadmap and v5 baseline,
the complete C4-C1 authorization including its verification-seam clarification,
X-PROJ/X-LIMIT, and prior C2-B2 and combined B3+C01 independent reviews. The
clarification explicitly permits private preselection before handler entry;
active-invocation ID resolution remains C4-C2 work.

| Author identity | SHA256 |
| --- | --- |
| Frozen candidate, 20 mode/hash entries | `76f717420b4e18b4d517a794e4b645a211dd301679d2b0a9ccab08f1d9a13339` |
| Artifact manifest, 217 entries | `ea4672c81969c2218aacaaad64b405a5cff790e84376f583fb0da7ec447d4400` |
| Finalization | `6aa4c44be3efee01d7befccd0b2efa5f3fd8946826d9be61f4d2607cc4ef39b0` |
| Author REPORT | `d050d20c1af7a4a10afe4e9f225e64601ec539035b4420cc24946986687762d7` |

Exact current candidate entries and complete author artifact membership, numeric
modes, byte counts and hashes were verified before and after execution. Raw XML,
command exits, expected red evidence, inventory preservation and private source
transformations were independently examined. The review's `reviewed-candidate.json`
is an exact copy of the frozen candidate. `author-identity-audit.json` and
`raw-evidence-audit.json` retain these checks. Frozen prose still says review
pending; this report provides the independent disposition without altering it.

## Boundary assessment

At [runtime.cpp:1824](../../../src/engine/runtime/runtime.cpp), the native
`projection_call_frame` moves the outer input into its own owner and preserves
Lua stack top, push memo, alias mode, retained callable and extension mode. Its
constructor installs a fresh nested input/memo and ordinary one-argument
projection mode. Its destructor restores these fields after returned statuses
and C++ exceptions. The original history bridge/context identity and invocation
owner remain untouched. `call_retained_projection` charges the fixed frame and
scanner before use and does not begin an invocation, reset/refund counters,
release temporary roots or clear the outer stack.

The helper runs the existing `execute_shape` under `lua_pcall`. That protected
trampoline owns only trivial locals during allocating Lua operations; the saved
native owner remains outside it. `lua_checkstack` reports refusal without raising,
zero-upvalue C-function pushes are nonallocating, and the scanner retains its
protected POD-only traversal. Inner results remain rooted at `base+1` through
raw admission, exact borrowed-view Glaze counting and native copying. Conversion
protects allocating tag anchoring separately; native conversion owners unwind
before the frame restores the outer stack. No new Lua longjmp crosses an owning
C++ local. Outer invocation cleanup remains the sole transient-root cleanup.

At [runtime.cpp:3990](../../../src/engine/runtime/runtime.cpp), ordinary
`invoke_projection` uses this same helper after its existing definition/kind/
transport checks and invocation transition. There is one unchanged raw scanner
and one count/reservation/copy path. Existing top-level location, terminal-error
and cleanup ownership remain. Completed `LUA_ERRMEM` now explicitly sets the
existing sticky flag; a successful emergency allocation retry does not. The
current resource flags and cumulative instruction/native/node charges remain
shared with the outer owner.

The private adapter uses actual retained native history and the captured same-VM
projection F. It checks identity/kind before F and retains the existing resolver's
owned definition before handler entry. It preserves the production resolver's
`invoked` refusal. Adapter bookkeeping retains first projection failure/location
before raising into Lua; original native history failures retain priority in the
outer execute boundary. This is verification of the internal foundation, not
production service admission or authenticated call-time ID resolution.

Repeated nested calls retain the first returned Lua result through the second
call and forced GC, while the original history/invocation/context stay intact.
Owned conversion allocation failure and raw scanner limits restore the same
outer state as success. Existing native receipt rechecks still suppress results
after original module or ledger mutation. History-only, pure/zero-capability,
missing-service and forbidden mixed history/projection behavior remain covered.

## Independent verification

Exact argv/cwd/status/timing, copied sources, raw XML/logs, dependency manifests
and cleanup records are under the reviewer evidence directory. Python used
`python3 -B`; private compilation recursively expands response files, disables
implicit modules/module maps and redirects compiled dependencies into owned
copies.

| Check | Result |
| --- | --- |
| Independent focused macOS Debug gate | 430/430 in 173.29 seconds; zero failures/errors/skips/disabled |
| Runtime / format / SQLite / history-read / CLI | 305 / 82 / 33 / 3 / 7 |
| Inventory | 791; all 790 prior names preserved |
| Final private copied-production cases | 12/12: nine author cases and three fresh reviewer cases |
| Compiled baseline/candidate layout | 1/1 each; persistent 1016/1424/488 and initial charge 17,916,913 unchanged; frame 56 bytes |
| Pre-edit nested success control | Expected exit42; one failing case reproduced |
| Six common vectors and six historical/current audits | Pass |
| Pinned formatting/Doxygen | 118 active sources pass |
| Build and post-private no-work build | Pass; Ninja reports no work |
| Shared/copied compiled dependencies | 888 members; exact membership/modes/bytes/hashes preserved |

The fresh reviewer cases independently cover two nested calls in one actual
handler for both ledger formats, an outer result retained and changed as scratch
across forced GC, native allocation failure specifically during owned conversion,
and caught 65-section/1 MiB+1 text refusals. They verify restored input/memo/
callable/mode/context/owner/stack/baseline/history-result identities, cumulative
native/node increases, terminal retries and original projection location.
Author replays additionally exercise malformed/throwing/multiple-return F,
unknown/wrong-kind refusal before F, codec allocation failure, completed Lua
memory refusal, successful emergency retry, real instruction exhaustion, original
typed history failure priority and final original receipt mutation suppression.
These controlled allocator refusals are not natural host exhaustion claims.

One initial reviewer fixture mistakenly called removed `string.rep`; that branch
correctly returned protocol `internal` before scanner admission. The attempt and
its exact sources/XML/commands are preserved under `attempt1-private*`. The
review fixture alone was corrected to bounded Lua concatenation and all final
cases passed. No product change was required.

## Protection and limits

The fresh baseline protects 73,480 entries outside reviewer ownership and the
coordinator's round file, including all author evidence, historical records,
product and installed release. Final comparison reports zero membership, mode,
hash or symlink-target changes. AGENTS.md remains the CLAUDE.md symlink;
rewrite-release remains absent. Only verified reviewer-owned disposable compiler
products and dependency copies were removed; complete removal membership, modes,
bytes and hashes remain recorded. Sources, commands, logs and XML are retained.
The artifact manifest and finalization bind this report, candidate identity,
verification, protection and cleanup.

Approval applies while this exact candidate remains current. Exclusive Debug
and reviewer edits are released at final handoff. This is focused macOS Debug
approval; the latest complete approved gate remains 676 per configuration.
No full, release, Linux, public profile/service, whole-invocation X06 accounting,
external-profile acceptance, cutover or execution/RA01 completion is claimed.
C4-C2 still owns real context.project, active-invocation definition resolution,
service ordering and assembly. Public init continues to create format1.
