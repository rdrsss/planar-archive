# D01-B production allocation and lifetime derivation

This proof applies to the pinned Glaze/libc++ types/options recorded in
`../v5-02-x05-b3-b-d01-b-evidence/pinned-identities.json`. It extends the independently
approved [D01-A parser proof](../v5-02-x05-b3-b-d01-a-support/source-and-conversion.md).
The prototype is no longer the production mechanism: `codec_decode_reserved` uses
its own decode-only node. Ordinary codec_decode retains its original node,
reader, conversion, signature/defaults and diagnostics, with its complete early
and catch adaptation now protected by an outer allocation boundary. Encode's
validator, reservation, graph construction, writer and error behavior are unchanged.

## Parser/checker request boundary

`planar_codec_detail::allocator<T>::allocate` checks count*sizeof(T), calls the
request's erased native adapter synchronously, then enters std::allocator<T>.
The adapter calls the optional supplied hook and throws a moved owned codec_error
on typed denial. No allocation is attempted after denial. No deallocation refunds
a charge. There are no event arrays, mutable global state, default allocator
routing, arenas, new quotas or input-size estimates in product code.

The text/sequence wrappers bind before delegating to unchanged Glaze readers.
Their base move assignment propagates their allocator, releasing prior storage
with its old allocator first. Empty alternatives and object pair keys cannot grow
before their own wrapper reader binds. Allocator copies/rebinds retain the request
pointer, including unordered_set hash nodes/bucket arrays. The exact seven variant
alternatives/order, node-before-whitespace policy, depth guards, scalar/schema
checks, duplicate checking, furthest parse offset and strict validation retain
D01-A's reviewed call graph and ordering. The options are unchanged.

Glaze core/read.hpp:135–213 selects unpadded string_view input, no copied input.
json/read.hpp:887–1345 owns string growth/escape work, 2301–2688 arrays/pair ranges,
3868–4860 the speculative variant and its rewind (non-auto-deducible branch4801).
The only dynamic owners on these instantiated paths are supplied strings/vectors.
The native duplicate set explicitly uses the same allocator. Numeric parsing uses
scalar/fixed stack state (including fast_float fallback), not an owned spelling.
Strict validate_json at json/read.hpp:5217–5222 uses skip.hpp/parse.hpp pointer and
bounded stack operations and no owning destination; it has no heap workspace for
these options. No copied parser/JSON grammar or new escaping logic is introduced.

The allocation_state precedes the parsed graph in the call scope. All partial
variants, duplicate sets and parsed nodes unwind before that state dies, including
callback denial/throw and actual upstream allocation failure. No returned value
contains a parser allocator or request pointer. Nested independent calls have
their own local state; the API forbids mutating input/context through a reentrant
callback. Hooks must return/throw through ordinary C++, not Lua longjmp.

## Allocation-free immutable sizing and construction

After parse/schema/strict validation, `decode_workspace::measure` walks only the
privately owned parsed graph using its fixed129 frames (root depth0 through scalar
at depth128). It uses checked additions/multiplications and validates copied UTF-8
without constructing persistent values. The same workspace performs iterative
construction; no detached plan or externally mutable parsed aliases exist. The
conversion hook runs once after sizing, before ANY persistent node/container/key/
string allocation. Earlier parser/checker requests remain cumulatively charged,
even when their temporary storage has already been freed.

The byte components are derived directly from these pinned allocation recipes:

* Each visited value allocates one complete
  `std::__shared_ptr_emplace<value,std::allocator<value>>`. shared_ptr.h:650–669
  selects this exact control block and __allocation_guard allocates one rebound
  instance. The complete sizeof includes its value, control state and allocator;
  no guessed shared_ptr overhead or separately counted inline value is added.
* Each object pair allocates one complete
  `std::__tree_node<std::__value_type<std::string,value_ptr>,void*>`.
  __tree:923–934 rebinds to that type; __construct_node at2042–2046 allocates one.
  The sizeof includes links, color, key object and shared_ptr. emplace's piecewise
  key construction copies directly into that node, without an intermediate key.
  Tree rebalancing uses no allocation. Duplicate keys have already refused.
* Each list reserves its exact final size before append, then moves child
  shared_ptrs. vector.h:1016–1026 reserve constructs a split buffer of the requested
  count; ordinary allocator allocate_at_least returns that count. Account exactly
  entries*sizeof(value_ptr), with no geometric growth or duplicated child graph.
* Each string value starts empty, reserves its final length, then assigns bytes;
  each map key uses direct pointer/length construction. string:2591 __init and
  reserve at3351 both select __allocate_long_buffer/__align_allocation_size.
  Empty/SSO lengths need zero heap; their string objects are already in the value
  or tree node. For long char strings string:2280–2307 rounds n+1 to
  max(default-new-alignment,sizeof(void*)) and applies its exceptional min-cap+1
  adjustment with the actual layout/endian factor. The production recipe mirrors
  this arithmetic with checked operations. __allocate_at_least on std::allocator
  returns exactly this count. Assignment after reserve cannot grow; construction
  has no amortized assignment-growth step.
* `workspace_bytes` is sizeof(the actual decode_workspace), including129 frame
  source pointers/indices/mutable construction owners, depth, measured summary
  and completed shared_ptr. It is a logical native charge for fixed traversal
  ownership metadata, not a compiler stack-frame or independent heap/RSS claim.
  The walk has only bounded scalar/reference locals; valid_text uses scalar state.

The measured byte sum is checked node+map+list+string+workspace. Actual persistent
allocation sums equal total_bytes-workspace_bytes in the private probes across
0..1024 string/key lengths, all SSO/alignment transitions there, long and1MiB
values, empty/mixed/map/list trees and100000-node/depth128 boundaries. These
observations corroborate the source derivation, rather than estimating capacity
from a successful corpus. Every actual parser and conversion allocation ordinal
of the mixed fixture is separately failed, with all earlier owners released.

Construction retains only ordinary std allocators and returned const shared
ownership. All temporary mutable owners disappear when the workspace unwinds;
returned graphs and moved supplied errors survive request-state destruction.

## Refusals and protection

Typed hook errors move unchanged, including long strings, code/limit/maximum and
supplied offset; no fresh diagnostic replaces them. bad_alloc and length_error
from callbacks or actual allocations map to existing input_bytes resource refusal;
unexpected callback exceptions map to redacted internal refusal. Both decode
entry points wrap early guards, parsing, construction AND inner catch diagnostic
construction in the outer typed allocation boundary. Baseline isolated controls
reproduce actual ordinary early/schema diagnostic allocation escapes; production
controls verify resource refusal after the same one-shot faults.

Callback exception controls are separate from actual one-shot global-new faults
at parser, conversion and diagnostic allocations. The observer only verifies
requests and cleanup in an isolated executable; it never routes resource state.
C++ exception-runtime storage is separate, and final fallback diagnostic creation
can allocate. No persistent exhausted-heap or process RSS guarantee is claimed.
Runtime first-error ownership, whole-invocation512MiB consumption and request423
remain fresh B3-B integration responsibilities after independent codec review.
