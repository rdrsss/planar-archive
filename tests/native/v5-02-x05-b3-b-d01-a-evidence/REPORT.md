# X05-B3-B-D01-A test-only decode feasibility candidate

Result: **feasible for the exact pinned parser/checker instantiation; independent
review pending.** The request-local allocator-aware destination binds its allocator
from the Glaze context before each generic string/vector reader runs. Every actual
parser and duplicate-check allocation reaches the synchronous nonowning native
charge callback first. Glaze retains all JSON grammar, escape handling and variant
selection/rewind. No product, vendor, runtime, Lua, codec API or CMake bytes change.
This does not deliver reserved decode, immutable conversion or request423 closure.

The shared checkout is dev/planar-v2-vernacular at
3171c49e7b92d3abda01bfbb8bfb0c6b29d4c67a, preserving the dirty rewrite and B3-B
checkpoint. Ownership is only the new D01-A support/evidence directories and the
five explicitly allowed current prose files. Others' work was not reverted.
No Planar tools/skills/roles/queue, installed binary, operator data, commit,
installation, publication or merge was used. The coordinator owns the round.

## Mechanism and proof boundary

[Source and conversion derivation](../v5-02-x05-b3-b-d01-a-support/source-and-conversion.md)
provides the complete instantiated source call graph and version assumptions.
The prototype uses ordinary standard containers with explicit stateful allocators,
plus wrapper `from<JSON,...>` specializations that bind before delegating. Empty
variant alternatives and pair keys may be default constructed without a resource;
they cannot grow until the wrapper installs the call's allocator. Allocator copies
and rebinds preserve the pointer, move assignment propagates it, and old storage
is released with its stored allocator. Unbound actual allocation is a hard test
failure. No global/default PMR routing, alternate JSON parser, input-size capacity
multiplier, preparse guessed reservation or new product quota is used.

The seven alternatives and their order remain exact. Both original and prototype
use the same non-auto-deducible pinned speculative variant reader. Node/depth
checks, int32/null/noninteger/NUL schema refusals, duplicate checking, furthest
library position and later strict `validate_json` retain current ordering. Strict
validation uses no heap workspace for this string-view input and concrete options.
Duplicate nodes/buckets use the same explicit allocator. All per-call parse owners
are destroyed before the request state; no persistent output borrows that state.

The process-local global-new observer/fault injector is isolated test verification,
explicitly authorized by the coordinator. It never routes a request resource or
implements reservation. During observation each actual heap request is classified
as routed parser/checker, diagnostic adaptation, or an escape. The corpus reports
no escapes. Concurrent tests disable observation/fault injection and its phase
guards make no shared writes while disabled. Fixed event arrays are test observer
storage allocated before decoding, not codec capacity or a new runtime ceiling.
The source derivation supplies coverage beyond the finite corpus.

A supplied typed callback refusal moves its owned first diagnostic without new
allocations. Real one-shot global-new failures separately exercise ordinary
error-string construction and adaptation inside handlers; the outer boundary
returns typed resource refusal. The controlled upstream allocator-adapter throws
are separate tests, not claims of natural host exhaustion. C++ exception runtime
storage and final fallback allocation are not a process RSS or persistent exhausted
heap guarantee. No Lua longjmp occurs; future callbacks must return/throw through
an ordinary C++ boundary.

Immutable conversion remains a D01-B proposal: allocation-free checked sizing
using an explicit bounded traversal workspace, exact pinned shared control-block
and tree-node types, exact list reserves and derived standard-string capacity
rounding. Persistent returned native values stay on their ordinary allocator.
D01-B must independently implement and verify those recipes and every conversion
allocation/error boundary before native B3-B consumption. D01-A does not claim
production ABI, arbitrary Glaze option/type coverage or cross-toolchain portability.

## Verification

[Final private commands](final-prototype/commands.json) recursively expand compile
response files and move object, dependency and potential PCM output paths into the
private attempt directory. Existing pinned LLVM/libc++, Glaze and Catch2 are used;
only read-only module/library inputs come from shared Debug. Shared object/PCM/
library membership and hashes remain identical around the final private build.
No private compile mutates shared production artifacts.

| Check | Result |
| --- | --- |
| Final standalone prototype | **8/8 cases, 21280 assertions**, zero failures/errors/skips |
| Existing production JSON codec baseline | **14/14 cases, 219 assertions**, zero failures/errors/skips |
| Product inventory | **637**, preserving all prior approved **627** names; inventory only |
| Pinned support clang-format | Pass |
| Ordinary `all planar_tests` no-change build | `ninja: no work to do.` |

The eight private cases compare actual decoded graph content without allocating
comparison workspace, and compare exact code/limit/maximum/offset against the
unchanged source-built codec. They cover scalar/null/numeric boundaries, nested
and empty containers, all JSON escapes and Unicode surrogate cases, malformed
UTF-8, short/long/1 MiB strings, incomplete escape/large partial input, duplicate
and escaped-duplicate keys, loose object separators, whitespace/trailing data,
100000/+1 node visits and depth128/+1. The byte ceiling and tighter caller limits
retain exact precedence. Short strings/keys 0..64 and deterministic byte-deletion,
replacement and truncation mutations exercise library branches and rewind.

The mixed nested fixture observes **19 actual routed allocation requests totaling
1240 bytes**, with exact sizes/ordinals in the final XML. Every ordinal is denied
at the hook and independently failed at the upstream adapter. Denial prevents
that actual allocation; all prior partial allocations are freed, as checked by
pointer/size accounting and zero live bytes. Additional malformed/duplicate/
variant-rewind fixtures deny every observed ordinal and release all partial
owners. Each normal compared fixture observes every actual parser/checker request
and detects any allocation outside routing/identified error adaptation.

Actual diagnostic failures hit ordinal1 for `null` and ordinal2 after an earlier
parser allocation for truncated, object and node-limit cases. A moved long first
refusal has zero allocation requests even with the observer primed to fail the
next request; 4096-byte message and 1024-byte limit remain owned after request
state destruction. Two concurrent contexts independently repeat success/refusal
20 times each, with observation disabled. All checks are bounded feasibility,
not a full product, milestone, release, Linux or runtime integration gate.

## Preserved development attempts

Every attempt retains its exact prototype source, command list and raw logs.
Attempt01 failed compilation because unqualified `text` resolved to Glaze's type.
Attempt02 compiled but two cases failed because the harness passed zero codec
limits as its defaults; its comparison successes are not used as evidence.
Attempts03/04 failed compilation on a diagnostic INFO expression's precedence.
Attempt05 corrected the defaults and passed four initial cases. Attempt06 added
semantic/actual-allocation observation and failed two harness assertions that
incorrectly required semantic equality on refused results. Attempts07/08 failed
`-Werror` for a copied range-loop variable. Attempt09 corrected those harness
issues and passed six cases; attempt10 added the last independent lifetime and
short/escape/mutation controls and passed eight. The final formatted candidate
adds strict per-pointer deallocation size matching and passes the same eight.
No failed product test or speculative mechanism was silently discarded.

## Freeze and remaining work

`protected-baseline.json` captures initial product/vendor/build inputs, and
`checkpoint-baseline.json` binds the entire immutable B3-B evidence checkpoint.
Finalization verifies source/vendor/build compiled identities and checkpoint
membership/hashes. Ordinary verification changes only `.ninja_log` and
`Testing/Temporary/LastTest.log`; those operational logs are explicitly reported,
not mistaken for compiled candidate changes. AGENTS remains the CLAUDE symlink.
The candidate/diff/artifact manifests and finalization bind this report and exact
support/current prose. Debug is released only after final rehash; no later edits
are permitted by this coder.

B3-B remains WIP. Last approved product inventory is627; the latest full gate
remains A4's582 per configuration. D01-B production decode reservation, resumed
B3-B resource/lifetime/focused gate, B3-C W, B4 render, X06–X08, public profile
admission/cutover, mandatory external-profile acceptance, execution/RA01 and Linux
remain open. Public init creates format1.
