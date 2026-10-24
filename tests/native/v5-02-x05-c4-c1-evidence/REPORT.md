# X05-C4-C1 protected nested projection candidate

Status: candidate awaiting fresh independent review. Shared
`dev/planar-v2-vernacular`, HEAD `3171c49e7b92d3abda01bfbb8bfb0c6b29d4c67a`.
This is the authorized prerequisite, without a public projection capability,
`context.project`, assembly retention or active-service dispatch. C4-C2 remains
open. The latest complete approved gate remains 676/configuration. No full,
release, Linux, X06 shared accounting, external-profile gate or cutover is claimed.
Public init still creates format1.

## Implementation

`runtime.cpp` factors the existing projection call into one internal
`call_retained_projection`. Ordinary `invoke_projection` retains its original
selection, exact definition resolution, kind/transport guards, invocation
transition, diagnostic adaptation and final cleanup, and uses this same helper.
There is one raw scanner and one codec count/reservation/copy path.

A 56-byte compiled native stack frame saves/moves the outer input and saves its
alias memo, alias mode, retained callable, extension mode and Lua stack top. It
restores all of them on returned status and native exception unwinding. It does
not touch the original history bridge, context identity, registration field or
invocation owner. The already-active handler has established the temporary-root
baseline; nested execution retains it. `execute_shape` supplies one immutable
input and calls the exact retained F reference under `lua_pcall`, using its fresh
alias memo. Results remain rooted through raw scanning, codec counting and owned
conversion. Outer cleanup releases all temporary backings and tags.

There is no invocation transition, quota reset/refund, registry cleanup or stack
clear inside this helper. New fixed stack bookkeeping is charged before use with
the existing scanner reservation. Counters, Lua heap and sticky resource flags
belong to the same owner. Completed `LUA_ERRMEM` sets native sticky memory failure;
a single null allocation followed by successful Lua emergency retry does not.
Existing top-level terminal error/location handling remains responsible for
adapting helper failures. Persistent layouts remain command1016/VM1424/invocation488,
and the private fixture's initial cumulative native charge remains17,916,913.
This logical native charge is not process RSS or a heap allocation measurement.

The protected Lua execution/scanning frames have trivial local state. Native
saved owners live outside those frames and unwind only after `lua_pcall` returns.
`lua_checkstack` reports allocation refusal without raising; raw conversion uses
reserved stack capacity and protects its allocating tag-anchor work separately.
Conversion's active-identity guards unwind on native failures. No Lua allocation
or `lua_error` crosses the call frame's owning C++ locals.

## Verification adapter and limits

Private reversible production copies inject a clearly named `project_probe`
method into the real retained history context. It is a verification adapter,
not product service admission. It reads only the original invocation's
`authenticated_history`, and verifies equality to the original history bridge
result. It selects the actual captured same-VM projection entry/function and
checks its matching history kind. The existing exact native definition resolver
runs before handler entry, and its owned identity is retained in private adapter
bookkeeping. The production resolver's `invoked` refusal is unchanged.
Consequently this proves nested callable lifecycle for a preselected projection;
it does not prove call-time `context.project` ID resolution. C4-C2 must implement
that distinct authenticated boundary with its own errors and accounting.

The private adapter retains first projection failure/location before raising a
Lua error and invokes actual product helper code. It exercises the original
history read, actual retained handler and captured projection; there is no caller
provider, replacement facts, second VM or nested native transaction. It forces
complete Lua GC in the protected raw scanner while inner and outer inputs/results
are rooted. Native state snapshots verify outer input, memo, flags, callable,
context/owner/result identity, stack top and baseline state before/after nesting.
Native/node/instruction counters increase without resetting; the successful
handler and F both execute enough instructions to observe cumulative hook counts.

The pre-edit original top-level invocation fails the same private nested-success
case (expected exit42, one failing case). The candidate resumes that handler in
both ledger formats and returns the original exact history response. Ordinary
production coverage adds an immutable aliased-input/sequential projection case.
All existing top-level projection, history-only, zero-capability and forbidden
history+projection capability cases remain in the focused gate.

Private cases additionally cover malformed/throwing/multiple-return F, unknown
and wrong-kind identity before F, actual one-shot native allocation refusal,
actual completed Lua memory refusal despite a catch, successful emergency retry,
actual100-million-instruction exhaustion, original typed native history failure
priority and final original module/ledger receipt mutation suppression. These
are controlled allocator refusals, not natural host exhaustion claims.

## Frozen final verification

Exact argv/cwd/status/timing and raw XML are retained in `gate-results.json` and
`private/commands.json`; the frozen candidate hashes product, tests, support and
current prose before the final gate. `verification-summary.json` and raw outputs
are authoritative. The final sequence runs:

- Focused macOS Debug430/430: runtime305, format82, SQLite33, native history3,
  CLI7; zero failures/errors/skips/disabled.
- Inventory791, preserving every790 baseline name.
- Private nested9/9 plus baseline/candidate layout1/1 each, zero green failures,
  and the preserved expected pre-edit nonnestable failure.
- Six common vectors, historical/generated variant audits and the current
  source boundary audit; pinned formatting/Doxygen over118 active sources.
- Build and final no-work build, preserving all shared compiler products during
  private instrumentation.

Private builds recursively expand response files, copy all888 compiled object,
PCM, archive and dylib inputs, include referenced external standard-library PCMs
and LLVM libc++ dylibs, and disable implicit modules/module maps. Exact shared
and copied membership, numeric modes, bytes and hashes are verified. macOS system
runtime libraries remain platform prerequisites. Only verified owned disposable
compiler products, binaries and dependency copies are removed, with the full
removal inventory retained. Sources, commands, logs, XML and manifests remain.
Python uses `python3 -B`.

Development attempts are preserved. The first adapter driver encountered a
formatted signature mismatch; another missed the one-shot probe variable
declaration. Initial integration attempted definition resolution after handler
entry and correctly hit the existing refusal. The adapter was corrected to
preselect before entry, without changing that product guard. Failed source and
command snapshots/XML/logs are historical attempts, not green evidence.

The baseline protects73,240 entries outside owned scope/Debug and the coordinator
round; final protection checks exact membership, modes, hashes and symlink
identity, including installed release. `AGENTS.md` remains the `CLAUDE.md`
symlink; rewrite-release remains absent. The final artifact manifest and
finalization bind this report, frozen candidate, raw evidence, protection and
cleanup. Debug is released for fresh independent review at final handoff.
No self-approval or commits/install/publish/merge/Planar tools/operator data.
