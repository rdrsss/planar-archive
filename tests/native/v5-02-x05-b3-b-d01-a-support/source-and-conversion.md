# D01-A pinned mechanism and D01-B conversion proposal

This is test-only feasibility, not a production API or vendor amendment.

## Complete parser/checker interception argument

The concrete pinned graph is the same seven ordered variant alternatives as
`src/engine/json_codec/codec.cpp:6`: null, boolean, int64, double, string,
vector of nodes, vector of key/node pairs. The prototype replaces only owning
strings/vectors with wrappers around standard containers using an explicit
stateful allocator. Its `from<JSON,...>` specializations bind the request pointer
before delegating to the unchanged generic reader. They contain no JSON grammar
or escape decoding. The node reader reproduces the current structural/schema
policy and furthest-error position. The separate strict `validate_json` pass
remains after structural/schema checking.

The template call graph is finite for these types and options:

* `core/read.hpp:135–213` reads a `string_view`, so `use_padded` is false. There is
  no owned input copy or buffer resize. `context.hpp:137–150` has empty standard
  `current_file` and `scratch` strings, but this graph never reads into either.
* `json/read.hpp:3868–4860` owns variant selection, ordered speculative attempts,
  error rewind, depth rewind and speculation limits. Both the original and new
  alternatives fail `variant_is_auto_deducible` (two numeric alternatives).
  The exact ordered fallback at 4801 constructs empty alternatives then calls
  `parse` on them. `std::variant` stores alternatives inline; it allocates no
  separate ownership. The wrappers bind even when an alternative was default
  constructed during rewind; they also bind default-constructed pair keys.
* `json/read.hpp:887–1345` owns string scanning, resize/append/assign, SWAR tails,
  escape decoding and UTF-8 checks. Its only dynamic owner is its supplied string.
  Every allocation therefore reaches `allocator<char>::allocate`, including
  intermediate padded capacity and failure after partial decoding.
* `json/read.hpp:2301–2497` owns arrays and vector growth; 2499–2688 owns the
  concatenate range-of-pairs object reader. Each uses the destination vector's
  allocator. Its `emplace_back` constructs inline default nodes/pairs; those
  subobjects allocate only when their own readers run, after binding. The object
  reader does not use the map-reader scratch key branches at 3410 onward.
* Null/bool/int64/double readers hold only scalars and call pinned numeric parsing
  (`from_chars`/Glaze's fast-float helpers). There is no owning string, vector,
  locale stream or dynamic workspace in these instantiated numeric paths.
* Native duplicate checking uses `unordered_set<string_view,...,allocator<...>>`.
  Allocator converting constructors preserve the same request pointer for hash
  nodes and bucket arrays. `insert` performs its actual rehash/node requests
  through that allocator. Borrowed keys remain valid until the set is destroyed;
  its source object is not mutated during checking. No copied key strings exist.
* `json/read.hpp:5217–5222` constructs a stack context and `glz::skip`, then runs
  strict `validate_json`. `json/skip.hpp` and `util/parse.hpp` perform pointer,
  scalar and bounded recursive stack operations. No destination ownership,
  hash storage, `current_file` or `scratch` growth occurs on this path. It has
  zero heap workspace to route, including malformed syntax.

The allocator's sole allocation edge checks multiplication, calls the synchronous
nonowning function-pointer hook, and only then invokes `std::allocator<T>`.
Request denial throws the moved owned typed diagnostic; the upstream allocation
is never entered. Copies/rebinds copy the pointer; container move assignment
propagates it (`propagate_on_container_move_assignment=true`). Wrapper binding
moves an empty correctly bound base into the old base. Old allocations are
released with their old stored allocator before the new pointer is installed.
Empty default allocator construction, inline variants and pair construction do
not allocate under pinned libc++. All copies/moves stay inside the request;
none of these containers or allocators is returned. Destruction runs while the
request owner is alive, including partial parse and variant rewind failures.
An unbound actual allocation throws a distinct logic failure rather than silently
using a default resource. The tests would fail on such an escape.

The request log is preallocated test instrumentation, with a deliberately generous
262144-event capacity; exhaustion aborts the test harness. It is not a codec quota,
reservation formula or proposed runtime workspace. No multiplier based on input
bytes estimates parser capacity. Every real request is charged as it occurs,
including abandoned attempts and freed buffers; there is no refund/reset.

The process-local global-new observer is separate verification instrumentation,
authorized by the coordinator, as in earlier allocation probes. It records sizes
and distinguishes allocator-routed, diagnostic-adaptation and escaped requests;
it never selects a request context/resource or implements the reservation. All
observed parser/checker requests in the corpus are routed. Concurrent tests run
with observation/fault injection disabled, and the observer phase guards avoid
writing shared flags when disabled. Source inspection above, rather than corpus
coverage alone, supplies the complete interception argument for this pinned
instantiation. C++ exception runtime storage is separate from container allocation
and is not an RSS or globally exhausted-heap claim.

The test-only upstream throw is a controlled allocation-adapter fault, not natural
host exhaustion. Separate global-new one-shot failures hit actual diagnostic
string allocations. The outer boundary protects adaptation inside inner handlers.
A supplied typed refusal moves all owned strings through the result with zero new
requests; its strings remain valid after request destruction. Final fallback can
allocate, so only one-shot failures with later requests allowed are demonstrated.

## D01-B immutable conversion proposal (not implemented)

Keep ordinary returned `value`, `shared_ptr`, `map`, vector and string ownership
on the normal allocator. Do not return parser wrappers or allocate persistent
values from a call-local arena. After parse/schema/duplicate/strict validation,
perform an allocation-free bounded iterative walk over the retained parsed graph.
A fixed `array<frame,129>` (root depth0 through scalar depth128) can hold source
pointer/index/visit state; take the reservation as `sizeof(the actual workspace
struct)` rather than guessed slot counts. Preserve the current conversion order
and UTF-8 policy. Use checked additions/multiplications for every count and sum.

Derive heap storage from exact pinned recipes:

1. One `make_shared<value>` per visited node. In pinned libc++, shared_ptr.h:660–669
   selects `__shared_ptr_emplace<value,allocator<value>>`, and 650–656 allocates
   exactly one such control block. Account its complete `sizeof`, including value
   and control/allocator storage, once. D01-B must tie any internal-type alias or
   local checked layout identity to this pinned source and corroborate the actual
   allocation size; do not guess a shared-pointer overhead constant.
2. Reserve each destination list's exact final size before filling, then move
   returned `value_ptr`s. Account `count*sizeof(value_ptr)` once, including spare
   capacity only if pinned `reserve` actually requests more (the present libc++
   exact `allocator_traits::allocate_at_least` path returns the requested count).
   There is no copied child graph beyond one node per parsed occurrence.
3. Each distinct object entry allocates one red-black tree node. Pinned
   `__tree:923–934` rebinds to `__tree_node<__value_type<string,value_ptr>,void*>`;
   2042–2046 allocates one node. Account that exact complete size, including the
   embedded key and shared pointer, once. A sorted map needs no hash buckets or
   sorting buffer. Duplicate checking has already completed and its actual
   temporary allocations were charged separately.
4. Keys and string values copy from parsed bytes into initially empty standard
   strings. Use pinned string's `__align_allocation_size` (string:2280–2307):
   align `n+1` to the standard-allocator alignment, with its explicit exceptional
   short/long adjustment. The previously reviewed `n + alignment + 2` per-string
   bound is a derived conservative alternative. Inline strings need zero heap;
   their objects are already included in the value/tree node. Do not double-count
   those objects. Reserve/copy once from empty to avoid geometric growth.
5. Add the explicit fixed walk workspace separately as a logical native workspace
   charge; it is stack metadata, not an independent heap/RSS claim. `valid_text`
   needs only scalar local state. No unbounded traversal container is required.

Call the conversion reservation hook after this checked allocation-free sizing
walk and before `make_shared` or any persistent container/string allocation.
The parser remains owned until conversion completes, so overlapping lifetimes
are counted naturally: cumulative parser/checker requests plus the conversion
reservation. A resource refusal receives no refund. Keep any callback/context
out of returned ownership; later deallocation must require no dead call state.

D01-B must independently verify these exact native-type sizes/recipes and every
conversion allocation ordinal, including capacity rounding and all error
adapters. Its API must keep ordinary callers/errors/limits unchanged, define
captured input stability and callback lifetime/reentrancy rules, and use nested
allocation protection for early/parse/conversion/callback diagnostics. D01-A
neither implements that API nor proves runtime consumption, Lua error priority,
whole-invocation accounting or B3-B request423 closure.
