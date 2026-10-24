# C3-B2 ownership, path staging and native accounting

`extension_invocation` consumes the actual original `profile_capture`, borrows its
immutable snapshot and internally constructs `command_vm`. Failed/moved captures
refuse through snapshot before registration. No arbitrary VM, prepared result,
mutable catalog, selector handle or service object is accepted or exposed.
The wrapper privately retains capture, VM, prepared scalars and selector strings,
normalized spelling/intent pairs, workspace parent and first failure. Catalog
borrowing returns empty after failure/move. One admission attempt is allowed;
subsequent admission poisons a success as protocol. Verification before successful
admission refuses terminally; later verification only rechecks receipts and grants
no execution/output authority. Move transfers the exact owner and first failure.

Creation before a VM exists maps allocation refusal to the existing external
resource diagnostic at profile and publishes no partial owner. After VM creation,
`sizeof(implementation)` is charged before its one `make_unique` allocation.
VM creation retains its existing copied-snapshot/registration charges. The original
capture's bounded metrics remain separate; no capture allocation is claimed as VM
accounting. Shared whole-invocation capture/VM accounting remains open for X06.

C3-A preparation charges all privately retained command/options/selector vectors
and strings by its existing rules. B2 moves that result without copying and charges
`pending_count * sizeof(prepared_host_path)` before one reserve; there are at most
six kinds, and push_back only moves already charged strings. The workspace parent
copy is separately charged `sizeof(string) + parent_length + 1` before assignment.
No cwd/environment snapshot copy is owned: caller-snapshotted views are borrowed
only synchronously; helpers' copied paths and temporary buffers are reserved below.
Root/base IDs remain in private prepared scalar strings; only the first '=' RHS
passes to path preparation. Native constant receipt keys allocate under the
capture subsystem's own inherited accounting.

## Conservative helper reservation

The proof is tied to pinned LLVM23.1.4's libc++ implementations, archived beside
this document: the exact upstream `llvmorg-23.1.4/libcxx/src/filesystem/path.cpp`,
local `__filesystem/path.h`, `__vector/vector.h` and `string`. The source URL is
https://raw.githubusercontent.com/llvm/llvm-project/llvmorg-23.1.4/libcxx/src/filesystem/path.cpp .
The candidate manifest binds modes/hashes of all four snapshots. These are evidence
of third-party implementation, not copied product normalization logic.

For one lexical helper pass over an upper byte length B, put N=B+1. Every path
component consumes at least a byte (root/trailing components are covered by +1).
`lexically_normal`'s component stack contains `pair<string_view, PathPartKind>`,
where PathPartKind is unsigned char. The local pinned layout equals the product's
`sizeof(pair<string_view,unsigned char>)` (24 bytes on this macOS ARM64 gate).
The stack reserves32 entries, then vector `__recommend` doubles capacity. Its
sum of allocation capacities is strictly below `4 * max(32,N)`; popping '..'
components adds no allocation. A conservative reservation for all component
allocations is therefore `4*max(32,N)*sizeof(pair<string_view,unsigned char>)`.
No semantic component parser or shadow lexical normalizer is introduced.

The separate `16*N + 8*sizeof(string)` allowance covers helper string/path
staging. The same-char path constructor copies a byte string; no transcoding
buffer exists on supported POSIX platforms. `lexically_normal` reserves its output
once for `Parts.size + NewPathSize + trailing`, bounded by2N; same-char appends
use that reservation. `.string()` makes at most one additional byte-string copy.
Pinned string capacity alignment rounds by at most16 bytes; eight string-wrapper
allowances (192 bytes locally) cover constant rounding/SSO slack for these bounded
passes. A simple path copy and final `.string()` each need <=2N plus constant
slack; normalized output needs <=4N plus slack. Relative spelling construction
copies cwd and appends at most one separator and argument; its at most three
allocations total <=6N plus slack. These total <=14N plus slack, below16N.
For workspace expansion, the same path/normalization/copy work uses <=8N and
at most three base-string appends use <=6N; the same16N allowance applies.
This counts all transient requests conservatively, not merely peak live storage.

Workspace expansion reserves one pass with B at least selector_length+16 plus
relevant environment_length (the16 covers separator and '/workspace.lua'). For
an explicit workspace selector no environment bytes enter the bound. Relative
host preparation reserves one pass for cwd normalization and another for the
full joined spelling upper bound argument_length+cwd_length+1; absolute paths
reserve only the full-spelling pass. The normalized intent cannot exceed these
byte bounds plus the covered constant. Unneeded cwd/env snapshots are ignored.
All reservations precede the corresponding helper calls/allocations; destruction
and failures do not refund cumulative native work.

The private reservation helper checks each separate byte length against an
upper value derived from the existing512MiB ceiling before additions or products.
An impossible reservation refuses via `charge`, retaining the original native_bytes
message/maximum. It does not invent a public path quota or count process RSS.
A valid8MiB cwd fixture therefore reaches actual native reservation refusal before
allocating helper normalization buffers; separate64MiB path_bytes bounds remain
owned by host_path and can refuse first during pure preflight.

## Validation and precedence

The authorized host_path amendment exports existing workspace-directory validation
and extracts shared path/relevant-cwd text, size and POSIX absolute checks into a
pure preflight. Success allocates nothing. Existing snapshot/expansion/preparation
APIs delegate to shared checks; lexical cwd validation remains before checked
joined-length overflow in prepare_host_path. The wrapper invokes these pure
checks before native reservations, so malformed oversized relevant text retains
usage/path_denied and valid oversized text retains path_bytes; irrelevant snapshots
remain ignored. Full lexical and checked-join work stays in existing helpers.
No ambient cwd/environment reads are added.

Bare workspace environment normalization is the approved C name rule: normalize
the supplied environment lexically, then append name/workspace.lua. Explicit paths
and the expanded output retain subsequent physical spelling; metadata admission
never uses lexical intent. Every slot is fully prepared before any metadata
admission. Workspace parent and selected file are both admitted. Source/base are
roots; registry, observations and ledger use their exact native file roles.
Original trusted and late receipts are verified after all admissions, including
capability-free commands with no late slots. No late content is read.

All failures are terminal for this wrapper. Existing C3-A shared failure publication
wins before wrapper adapters; moving its first error into the wrapper avoids losing
catalog.commands attribution if a return-copy allocation fails. Private two-fault
probes distinguish publication from later error adaptation and compare a private
pre-hardening implementation. Native error code/message/limit/maximum/field are
retained exactly; selectors never appear in diagnostics. There is no instruction
reset, service dispatch, Lua ABI materialization or early load replay here.
