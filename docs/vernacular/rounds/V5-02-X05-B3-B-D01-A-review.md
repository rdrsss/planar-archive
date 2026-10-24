# V5-02-X05-B3-B-D01-A independent feasibility review

Disposition: **approve**, limited to **test-only decode feasibility for the exact
pinned parser/checker instantiation**. No blocking finding remains in D01-A.
D01-B may proceed as a separately implemented and reviewed production round;
this review does not approve a production API, immutable conversion reservation,
native runtime integration, B3-B accounting or request423 closure.

Review used shared `dev/planar-v2-vernacular`, HEAD
`3171c49e7b92d3abda01bfbb8bfb0c6b29d4c67a`. The active guide, implementation,
roadmap, v5, exact external-profile limits, D01 search disposition and D01-A
observer clarification, incomplete B3-B checkpoint, and approved B2-B+C01 encode
boundary were read. The durable D01 search outcome is the disposition in the D01
round record; no separate search report is required or claimed. Candidate,
product, vendor and earlier evidence remained read-only. Writes are confined to
this report and new `tests/native/v5-02-x05-b3-b-d01-a-review-evidence/`, with
ordinary serialized Debug verification under exclusive ownership. Others' work
was not reverted. No Planar tools, legacy operational skills/roles/queue,
installed binaries, operator data, commits, installation, publication or merge
were used.

## Interception and lifetime proof

The [allocator edge](../../../tests/native/v5-02-x05-b3-b-d01-a-support/prototype.cpp#L118)
checks multiplication, synchronously calls the supplied nonowning callback, then
enters the ordinary allocator. A typed denial exits before the actual request.
Deallocation updates live tracking but does not refund charged requests or reset
cumulative totals. The fixed event arrays are preallocated test observation,
not a proposed product quota or a capacity multiplier.

The [string/vector wrappers](../../../tests/native/v5-02-x05-b3-b-d01-a-support/prototype.cpp#L169)
bind immediately before delegating unchanged Glaze readers. Empty default
variant alternatives and default pair keys require no allocation before their
reader binds them. The seven alternatives and order match the current codec;
both use the non-auto-deducible speculative variant branch at pinned
`json/read.hpp:4801`. Failed alternatives rewind the library iterator/depth and
destroy partial owners; their actual allocation charges remain consumed. Array
and pair-range object growth at `json/read.hpp:2301–2610` owns only the supplied
vectors and their inline children. String resize/assign/escape work at
`json/read.hpp:887–1343` owns only the supplied string. Customization contains no
JSON grammar or escape decoder.

Allocator copies and rebinds retain the request pointer. In pinned libc++, vector
move assignment (`__vector/vector.h:945`) releases old storage before propagating
the allocator; string move assignment (`string:2853`) does the same. Moving nested
nodes carries their own string/vector allocators. The independent populated-
destination checks verify rebinding across two owners and later moves with exact
old-owner cleanup. Within decode, the parsed graph and duplicate sets die while
the caller's allocation state is alive; no persistent output retains a borrowed
allocator. Returned moved diagnostics have ordinary owning strings and survive
request-state destruction.

[Duplicate checking](../../../tests/native/v5-02-x05-b3-b-d01-a-support/prototype.cpp#L246)
uses an explicitly allocated unordered set of borrowed views. Pinned
`__hash_table` routes nodes and bucket arrays through rebound allocators, including
rehash. Source object keys remain stable until that set is destroyed. Null/bool/
int64/double branches retain scalar state; int64 uses bounded `atoi`, double uses
Glaze's fast-float path. Its difficult-number fallback has fixed stack limb
storage (`util/fast_float.hpp:3000` onward), with no heap owner. No locale or
owned numeric spelling is introduced.

The input is a `string_view`, so `core/read.hpp:135` selects the unpadded input
path. Empty context strings are never grown by this concrete graph. Strict
`validate_json` at `json/read.hpp:5217` constructs a stack context/skip value;
`json/skip.hpp` and `util/parse.hpp` operate on pointers/scalars and bounded
recursive stack state for these nonstreaming options. There is no hidden strict-
validation heap workspace. Independent observation of strict validation alone
corroborates this for malformed separators, numbers, Unicode, large strings and
over-depth inputs. This source proof is specific to the recorded type/options/
Glaze/libc++ identities; corpus success alone is not generalized to other readers.

## Behavior, errors and independent verification

The copied structural policy retains node-before-whitespace checks, container
depth checks, scalar/schema policy, duplicate checking, furthest parse position
and later strict validation in the current order. Differential tests compare
actual successful graph content and exact refusal code, limit, maximum and
offset against the unchanged source-built codec, including scalar/numeric,
Unicode/escape, malformed/partial/duplicate, byte/node/depth and trailing-input
cases. Ordinary diagnostic text is the same constant. D01-A deliberately stops
before persistent conversion.

The authorized process-local global-new observer/fault injector only verifies
allocations; it never selects a request allocator. While disabled, concurrent
calls read its flags without shared writes; concurrent observation/fault injection
is not promised. Author controls cover two concurrent independent contexts.
Reviewer controls also nest an independent decode inside the synchronous callback
without global routing.

The [independent reviewer cases](../../../tests/native/v5-02-x05-b3-b-d01-a-review-evidence/extended.cpp)
add exact byte-budget prefix boundaries, repeated same-state cumulative charging,
populated-wrapper rebinding/moves, nested independent contexts, strict-pass heap
observation, numeric/Unicode differential cases and 1200 deterministic mixed
mutations. A separate case denies **actual global-new parser allocations** at
every observed ordinal of nested, duplicate, partial and long-string fixtures;
all return the typed input-resource diagnostic and free every earlier partial
allocation. These are real one-shot heap requests, distinct from the author's
controlled upstream-adapter throws. The author's moved first-refusal controls
still allocate nothing during error propagation, including long owned strings.
Final fallback construction and C++ exception-runtime storage do not establish a
persistent exhausted-heap or RSS guarantee.

[Exact private commands and raw results](../../../tests/native/v5-02-x05-b3-b-d01-a-review-evidence/attempt02/commands.json)
use pinned LLVM/libc++, Glaze and Catch2 with expanded response files; object,
dependency and potential PCM output paths remain in private review directories.
Read-only shared module/library inputs are the only shared compile inputs.
Attempt01 passed the initial five reviewer cases; attempt02 adds the actual heap
failure case. Both exact sources/commands/logs are preserved; no failed review
attempt or product failure was discarded.

| Independent final check | Result |
| --- | --- |
| Original author prototype | **8/8 cases, 21280 assertions** |
| New reviewer cases | **6/6 cases, 29158 assertions** |
| Combined private executable | **14/14 cases, 50438 assertions** |
| Existing production codec | **14/14 cases, 219 assertions** |
| Pinned applicable clang-format | Pass |
| Ordinary `all planar_tests` build | `ninja: no work to do.` |

All test selections have zero failures, errors and skips. No full product,
release or Linux suite was run. The 637-name author inventory includes B3-B WIP;
last approved inventory remains 627 and latest full approved gate remains A4's
582 per configuration.

## D01-B constraints and identities

The [immutable-conversion proposal](../../../tests/native/v5-02-x05-b3-b-d01-a-support/source-and-conversion.md#d01-b-immutable-conversion-proposal-not-implemented)
is a concrete path, not implemented evidence. D01-B must independently establish
checked allocation-free bounded sizing, actual pinned shared-control-block/tree-
node sizes, exact list reserves, standard-string capacity rounding and full
workspace accounting. Reserving/copying from empty must avoid unaccounted geometric
string/list growth. Persistent values must keep ordinary ownership and outlive the
request allocator. Corroborate every conversion allocation ordinal, successful
lifetime and all partial failures against the derived allowance.

The production boundary must retain ordinary callers and exact error/limit/
precedence/offset behavior, protect early and handler diagnostic construction,
move supplied first refusals without replacement, and define synchronous callback,
input stability, lifetime and reentrancy rules. Test callback exceptions and real
adapter allocation failures explicitly. No fixed observer arena, process-global
routing, guessed capacity multiplier, fresh quota, Lua longjmp or refund/reset may
become the implementation. Runtime sticky diagnostics and cumulative invocation
consumption remain resumed B3-B responsibilities after D01-B approval.

[Scope verification](../../../tests/native/v5-02-x05-b3-b-d01-a-review-evidence/scope-verification.json)
reconstructs the exact candidate diff and verifies all **10** candidate members
and **98** author artifacts. All **2907** review-protected source/vendor/author/
checkpoint/shared object-PCM-library entries remain unchanged. The author's
broader **5391**-entry baseline also matches except its two already recorded
operational logs (`.ninja_log`, `Testing/Temporary/LastTest.log`); this includes
shared executables. All **53** B3-B checkpoint files remain unchanged. Shared
compiled membership/hashes are fixed, and pinned source/library identities match.
AGENTS remains the symlink to CLAUDE.

| Reviewed author identity | SHA256 |
| --- | --- |
| Candidate manifest | `24c39ea1945bb7ed1d12446ca560d54a76ca5122a4a242712d6ae54a93796cbf` |
| Candidate diff | `667640d86da888595ca4e451f7a80a5392920eb1cdf7bdda6cc00eeefaab8161` |
| Artifact manifest | `f0f8a7417d4c64273b2e900ddaa06ef3aca62bb840d95652e92ec7eadf0ec1c9` |
| Author report | `dad67275e270df6a42fffd6cd52bcb41ca3f65ac28c85ec0189f426c86f7d9c0` |

Reviewer finalization binds this report and all review artifacts, rechecks protected
sources/build and author/checkpoint identities, and releases Debug with no later
reviewer edits. D01-B production decode/conversion, B3-B integration/request423,
B3-C W, B4 render, X06–X08, public profile admission/cutover, mandatory external-
profile acceptance, execution/RA01 and Linux remain open. Public init creates
format1.
