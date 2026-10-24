# V5-02-X05-C4-B1 independent production prerequisite review

Disposition: **approve**. No required changes were found in the exact bounded
SQLite primitive candidate. This approves the lower native read gate and narrow
backup exception-safety correction, not a profile reader or capability service.

The review used the shared `dev/planar-v2-vernacular` checkout at HEAD
`3171c49e7b92d3abda01bfbb8bfb0c6b29d4c67a`. Ownership was limited to this report
and `tests/native/v5-02-x05-c4-b1-review-evidence/`, with exclusive serialized Debug.
Product/configuration, author artifacts, earlier history, installed release and
operator data remained read-only. No Planar tools, skills, roles, queues,
delegation, commits, installation, publication or merge were used.

## Reviewed identity

The review read the repository instructions, implementation/roadmap and v5
baseline, exact X-CAP contract, complete C4-B1 authorization including the
original-main amendment, and approved C4-B0 feasibility/review. The current
bounded authorization controls over historical future-slice wording.

| Author identity | SHA256 |
| --- | --- |
| `frozen-candidate.json` | `99ce8983f9d28d475ddd49f05fc906d1915353de5072707fc1bff54493fc31bc` |
| `artifact-manifest.json` | `1126d8104d1e2b32044800d41d92bb6ef77d1c476b30ca429a69369df5e8cb8f` |
| `REPORT.md` | `f35d01a7f02e2466d2cb3cc47071f07c23c81e0e554b5fb32318fce99d58af72` |
| `finalization.json` | `537294d41a6ea61b34ac2d79da3fe3fca0bdfbc2480fbd20b2929fb9fdebabc7` |

All 16 candidate entries and 177 author artifact entries were independently
verified for exact membership, modes, bytes and symlink targets. The combined
author/support snapshot has 188 entries. The independent protection snapshot
contains 72,338 entries, including all candidate product/reference files, vendor,
prior evidence and installed release. The coordinator-owned round is excluded;
no change to it is approved here. `AGENTS.md` remains the `CLAUDE.md` symlink and
`build/rewrite-release` remains absent. Final protection checks confirm no drift.

The author's authorized cleanup is correctly bounded: its retained pre-cleanup
identity, exact removed-file inventory and finalization preserve the tested
product/test/reference bytes. The current candidate changes only the report and
cleanup support after the final tests. No missing removed binary is treated as
fresh executable evidence; this review rebuilt the private probes independently.

## Source assessment

`sqlite.cppm:8` introduces typed native outcomes and a trusted owned verifier;
`sqlite.cppm:63` keeps gated opening distinct from ordinary opening. The trailing
default status preserves existing three-field error aggregates. The lower module
imports no capture/runtime/format module and accepts no Lua assertion or service
capability. Rich original capture diagnostics remain adapter-owned.

`sqlite.cpp:28` creates a stable explicitly nondefault VFS with a fixed bounded
address-derived name, checked file-size arithmetic and aligned inner file storage.
Every wrapper retains its context directly. Forwarded VFS callbacks receive the
real base VFS pointer; file callbacks receive the real inner file. Versioned
methods and optional null methods follow the base. No application routing map,
TLS, mutable active-owner route, default replacement, private unixFile access,
immutable URI, database copy or new quota appears. Test observers temporarily
replace the default only within their serial verification process and restore it.

The main `xRead` and actual `xFetch` paths call the original native verifier,
then require inner HAS_MOVED success/unmoved before delegating. Unsupported,
error and moved outcomes fail closed. Gate refusal preserves read buffers and
nulls the fetch mapping. First typed status/result-code publication allocates
nothing, catches verifier allocation/length/other exceptions inside the noexcept
C boundary, rejects unknown statuses and never retries the verifier after refusal.
HAS_MOVED remains the cooperative inode/path supporting link established in B0;
it does not prove hostile substitution or change-and-revert protection.

Main writes/truncation, main journal/WAL opens and main shared-memory mapping/lock
refuse before their delegates. Temp read/write/truncate/close forwarding survives
terminal main refusal. Bound-only ATTACH/DETACH denial and one-main enforcement
close the original-main restriction; ordinary ungated ATTACH/DETACH still works.
The VFS does not authenticate which receipt the caller supplied: original receipt,
held descriptor and raw-header binding belong to the following adapter round.

`sqlite.cpp:337` retains the gate in sqlite_state. Statements finalize before
releasing state; transaction abandonment rolls back before releasing it. Moves
preserve that graph, including connection move assignment while an older
statement retains the replaced owner. Closing the final state precedes context
unregister and guard destruction. All supported public raw-resource creation paths
were inspected: failed prepare leaves no returned statement, tail refusal finalizes
both statements before error allocation, successful wrapper construction copies
only shared ownership, and no public raw handle/BLOB API escapes. The new small
backup owner finishes on every post-init throw and explicit finish happens once.
The fatal last-state close check is therefore an internal ownership invariant,
not a supported BUSY-close policy. Pinned register/unregister at
`vendor/sqlite/b9c7/sqlite3.c:27699` and `:27727` have no ordinary BUSY/retry branch
once initialization succeeds; no leaking retirement queue is justified.
Concurrent shutdown is unsupported.

The native request model is bounded and documented: one shared state/control
block, one fixed context, fixed name and SQLite-owned per-file wrapper plus
underlying szOsFile. It introduces no growing application cache. This is separate
from the VM/capture accounting; X06 whole-invocation composition remains open.

## Independent evidence

All Python invocations used `python3 -B`. Exact commands, sources, expanded
arguments, logs, JUnit and dependency identities are retained in the reviewer
evidence directory.

| Check | Result |
| --- | --- |
| Focused production CTest gate | 116/116, zero failures/errors/skips, 55.18 seconds |
| SQLite/reliability; format/snapshot/history/acceptance; history-read | 33; 80; 3 registered cases |
| Author private probe reconstruction | 6/6 green, zero failures/errors/skips |
| Reviewer adversarial cases | 6/6 green, zero failures/errors/skips |
| Backup original-body regression | Expected one failing case, exit42; real unfinished backup/close BUSY |
| Pinned formatting/Doxygen | 118 active sources pass |
| Focused no-change build | Ninja reports no work |
| CTest inventory | 767 before/after, exact names unchanged |
| Shared/copied compiled dependencies | 888 members, exact membership/modes/hashes unchanged |
| Final author/protected snapshot | 188 / 72,338 entries unchanged |

The focused production cases independently exercise healthy first-header reads,
zero underlying reads on first-header refusal, typed/throwing/unknown outcomes,
late path replacement, non-null real mmap, sidecar restrictions, partial open,
retained statement/transaction ownership, original ungated behavior and ordinary
backup semantics. The author's original 756-name inventory is preserved in its
767-name candidate inventory. CTest registered-case counting avoids the author's
initial fork-child JUnit record-count mistake.

Private reconstruction checks the exact original backup body against the retained
preimage before instrumentation. Real pinned backup init/finish wrappers only
count activity. A real long prior SQLite error makes std::string allocate after
backup step; failing that actual allocation reproduces zero finishes and actual
source close BUSY in the original body. The corrected body finishes exactly once
on the throw and permits a subsequent real backup. No natural host exhaustion is
claimed. The remaining private cases exercise actual native context allocation
refusal, 24 SQLite allocator-failure positions, real WAL with a positive ungated
control, direct production callbacks/temp I/O, and actual profile_capture original
receipt/held-descriptor refusal before reads. Receipt integration here is test-only.

Reviewer cases add missing inner xFileControl refusal before the header, a real
non-null direct mapping followed by resource refusal and untouched outputs, second
main-open denial with working temp journals, move-assignment/retained-statement
close sequencing, ungated ATTACH/DETACH controls, and sticky refusal across a
successful already-prepared constant query. Allocation interception confirms
`gate_status()` itself makes no allocation. The initial reviewer cached-query
probe mistakenly prepared SQL after refusal before priming SQLite's schema; it
correctly refused that schema read. That raw failed attempt is retained. Preparing
the constant query before refusal demonstrates the intended cached-operation
boundary; no product edit was needed.

Private builds copied all 888 object/PCM/archive/dylib inputs, including explicit
standard modules and pinned LLVM dylibs. Response files were recursively expanded,
outputs redirected and implicit module/map compilation disabled. Shared and copied
membership/modes/hashes were checked after all probes. `otool -L` records the
copied admission syscall dylib and macOS system libc++/libSystem runtime loads;
those OS libraries remain platform prerequisites. After hash auditing, only the
reviewer's 895 disposable generated files (1,303,204,168 bytes) were removed.
Raw sources, logs, XML and exact removed identities remain. No shared build inputs
or author files were removed.

## Limits and successor requirements

Sticky gate refusal closes future delegated main reads/fetches; it does **not**
poison every SQLite operation. Already-prepared or cached operations may still
succeed. Any later native read adapter **must inspect retained gate_status even
after SQLite success before treating a result as authoritative**, prioritize the
adapter-owned original receipt failure, and perform the required original
pre-service/result/output seals. It must finish/close the native read transaction
before resuming external Lua. SQLite success alone is insufficient. Current
API/reference prose claims callback gating and retained status, not full command
poisoning. This handoff is mandatory for later service integration.

Production profile-receipt/raw-header/format/history adaptation, opaque-slot and
capability/once/order/poison orchestration, actual handler/read-result equality,
X06 recorded interpretation/history/acceptance and shared budgets, X07 public
cutover, X08 mandatory acceptance, execution/RA01 and Linux remain open.
Public init creates format1. No full suite, release execution or Linux execution
was performed; Linux support here is source portability review of public SQLite
interfaces and standard C++ only. The latest complete approved gate remains
676 per configuration.

All verification processes have completed. This reviewer releases exclusive Debug
and all owned paths to the coordinator; no subsequent edits/builds will follow
final handoff. Reviewer finalization binds this report and evidence.
