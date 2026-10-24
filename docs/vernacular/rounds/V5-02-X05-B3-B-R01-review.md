# V5-02-X05-B3-B plus R01 independent review

Disposition: **approve**, covering the combined original B3-B native semantic
composition and R01 accounting/lifetime completion. No blocking finding remains.
This closes the bounded native integration slice; B3-C work identity and B4
rendering remain required before a full B gate.

Review used shared `dev/planar-v2-vernacular` at HEAD
`3171c49e7b92d3abda01bfbb8bfb0c6b29d4c67a`, preserving the intentional dirty
rewrite and others' work. Active instructions, implementation/roadmap/v5,
exact X-REG/X-SHAPE/X-LIMIT contracts, original B3-B checkpoint, approved B3-A
phase ABI, B2-C+C01+C02 bridge and D01-B decoder review informed this review.
Product, candidate support/current prose, vendor and historical artifacts stayed
read-only. Writes are confined to this report, new
`tests/native/v5-02-x05-b3-b-r01-review-evidence/`, and serialized ordinary Debug
verification outputs. No fixes, Planar tools/legacy operational skills/roles,
installed binaries, operator data, commits, installation, publication or merge
were used. Debug ownership was exclusive; release artifacts were read-only.

## Native semantic composition

[Setup](../../../src/engine/runtime/runtime.cpp#L1674) retains the exact three
original `shape_prepare/check/finish` functions before discarding the export
table. [Sealing](../../../src/engine/runtime/runtime.cpp#L1505) explicitly includes
these roots with original globals, five shipped handlers and the separate
external environment in the original shared closure audit/freeze. This precedes
external module execution and retains exact enrolled native identities. External
F receives constructor-only engine access and cannot select the phase exports.

[validate_profile](../../../src/engine/runtime/runtime.cpp#L2862) derives shape
context from the final native descriptors. Caller validation cannot inject
context, phase/export selectors, checked results, definitions or callbacks.
Prepare completes all-job mandatory/base-cycle checks before any F. Native then
walks normalized requests in order, resolves the exact selected module's final
definition before real retained F, and checks its complete response and cumulative
obligation count before the next F. Semantic refusal/malformed data stops later
dispatch; complete additive cycles are evaluated at finish, preserving existing
fail-fast priority. Finish receives matching native-owned job/index/ID and
same-call definition metadata and forms the specified selected_shape.

Protected top-level prepare/F/check/finish calls use the same profile Lua state.
The C++ orchestration retains immutable native values between calls, rather than
Lua stack indexes. [Transient cleanup](../../../src/engine/runtime/runtime.cpp#L1414)
occurs after response conversion, preserves original registry membership and
does not publish invocation proxies into the persistent sealed set. No public
VM recursion or provider interface is involved. Generic profile `invoke` remains
closed before provider effects. W, selection, render, CLI, history and acceptance
remain outside this slice.

Native context/definition/map/list/text staging is charged before allocation.
The inherited 512-byte visit and separate map/text/vector allowances cover the
small structural staging; protected transport/conversion passes remain cumulative.
`begin_invocation` resets instructions once, without resetting native work or
nodes between phases, jobs or repeated operations. New state flags default false
and reserved-codec routing is scoped to the semantic operation; the ordinary
embedded decoder/encoder branches and public invocation behavior remain exact.

## Decoder hooks, phase error data and failure ownership

The [semantic decode branch](../../../src/engine/runtime/runtime.cpp#L586)
consumes both approved reserved decoder hooks synchronously: every actual
parser/checker request charges before allocation, and complete immutable
conversion storage reserves before construction. Freed/abandoned alternatives
receive no refund. The subsequent inherited validation charge deliberately
conservatively overlaps codec ownership. No input multiplier, new quota or
postallocation reservation substitutes for the approved codec accounting.
Reserved encoding remains active for phase JSON and complete F return transport.

The [error graph staging](../../../src/engine/runtime/runtime.cpp#L468) correction
also charges ordinary phase error data before construction: exact make_shared
control/value blocks, direct in-place map tree nodes and copied long-string
buffers. There are no initializer-list owning graph copies in this branch.
String reserve-from-empty and assignment follow the same pinned libc++ capacity
recipe as D01-B, with separate checked payload/alignment charges. Successful
encoded text separately charges its control/value wrapper before construction;
the already reserved output buffer moves without another payload charge.

The authorized global-fragment `<map>/<memory>` exception is acceptable for this
pinned toolchain: it exposes exactly the hidden libc++ allocation type identities,
while product standard-library operations retain `import std`. Source inspection
of pinned `shared_ptr.h`, `__tree` and `string` confirms the complete allocated
types and alignment/endian rules; [source identities](../../../tests/native/v5-02-x05-b3-b-r01-review-evidence/accounting-source-identities.json)
bind that inspection. There is no arbitrary size fallback. Clean production and
isolated private compilation pass. This is an explicit pinned-ABI dependency,
not a portable allocator-overhead or RSS claim; Linux acceptance remains open.

The semantic scope unwinds on success, ordinary semantic refusal and native
failure. Codec adapters keep ownership outside allocating Lua pushes/errors;
expected-return hook callbacks call no Lua and cannot longjmp across C++ owners.
Partial native error graphs unwind before Lua error raising. Sticky conversion,
native-memory and protected-call state prevents caught resource failures from
becoming ordinary successful data. The shared native `shape_failure` preserves
the complete first code/message/limit/maximum/logical field through later APIs,
response-copy failures and retries. Ordinary caught Lua errors and semantic
refusals remain recoverable. These guarantees cover controlled one-shot native
refusal with later allocation permitted, not persistent exhausted-host-heap
recovery or C++ exception-runtime allocation.

## Independent verification

The [recorded Debug gate](../../../tests/native/v5-02-x05-b3-b-r01-review-evidence/gate/commands.json)
passes **376/376** focused cases in **190.69 seconds**, **11/11** supplemental
history/codec cases, **11/11** production semantic cases (**283 assertions**) and
**18/18** codec cases (**413 assertions**). All have zero failures/errors/skips/
disabled cases. The inventory is **642**, preserving all **641** prior names;
this is not a full 642-case run. Five vector families, six exact source audits,
pinned formatting/Doxygen (**118 active C++ sources**) and the final no-work
`all planar_tests` build pass. [Raw-record verification](../../../tests/native/v5-02-x05-b3-b-r01-review-evidence/results.json)
checks author/reviewer selections, immediately saved LastTest records and XML.

Three additional [production semantic cases](../../../tests/native/v5-02-x05-b3-b-r01-review-evidence/adversarial.cpp)
pass **96 assertions**. They bind two distinct native final definitions across
opposite module/catalog order, preserve retained outputs through recoverable
schema failure/repeats/VM destruction, retain later refusal priority over an
earlier additive cycle, and exercise caught ordinary F errors with private phase
exports and frozen globals. They use the real production library and semantic
operation.

The independent [actual native allocation sweep](../../../tests/native/v5-02-x05-b3-b-r01-review-evidence/allocation/allocation.xml)
fails **every one of 5,618 observed allocation ordinals** during a complete real
semantic operation, rather than relying on the author's 137 samples. All **5,618**
controlled one-shot refusals return typed resource errors and preserve all five
first diagnostic fields on retry: **one case / 50,575 assertions**, zero failures.
This covers native preparation, parser/conversion, F transport, checked results,
definition staging, finish and fallible error adaptation without replacing the
codec or trusted semantic functions.

The expanded reversible [observer](../../../tests/native/v5-02-x05-b3-b-r01-review-evidence/probes.py)
runs all **16 modes / 383 assertions** and compares complete original **sealed-set
membership** in addition to tag/backing key-value membership and all **16 exact
retained reference bindings** in the mixed two-module fixture. Counts alone cannot
pass: tag/backing identities/types/values and reference number/type/object all
match at constructor/phase/F/final cleanup boundaries. Its 47 tag and 267 backing
entries remain exact. Successful real operations stabilize at **428947 live Lua
bytes after GC**, unchanged through operation25; baseline bookkeeping/warmup is
separately reported. Partial snapshot publication fails with ready=false before
input proxy publication, preserves original membership/references and closes at
zero live Lua accounting.

Controlled parser/conversion/node/Lua and error-graph node/map/string/wrapper
refusals prevent the denied action and later F. Concrete staging requests match
six112-byte blocks, five72-byte map nodes and two32-byte buffers; the successful
encoded wrapper requests112 bytes. Denied staging storage performs zero denied
data allocations; exact one-node remaining capacity allocates one node then
refuses the next. Real repeated operations reach the unchanged100M instruction
ceiling at operation48 and cumulative native ceiling at operation112, without
phase/repeat resets or refunds. The observer adds only reversible observation
and resource controls; actual F and semantic functions remain unchanged.

Historical request423 evidence remains truthful. The original development XML
contains the real red failure; the preserved original checkpoint already includes
its sticky correction. Author checkpoint/current423 controls both pass and do
not establish that ordinal423 still denotes the original decoder allocation.
The exhaustive current sweep supersedes ordinal sampling for current behavior.
The interrupted gate and initial selection preflight are preserved as incomplete,
not green evidence. One reviewer raw-XML audit ran before the sweep closed its
JUnit and saw an incomplete XML document; after runner completion the same audit
verified all45 records. No candidate change was needed.

## Candidate identity and release

[Scope checks](../../../tests/native/v5-02-x05-b3-b-r01-review-evidence/scope-final.json)
verify all **21** candidate members, **370** author artifacts, **2094** frozen
source/vendor/support/current-prose entries, and the174-entry baseline outside
the eight authorized existing paths. Both combined-original and R01 diffs
reconstruct exactly. All **5479** additionally protected source/vendor/release/
checkpoint/approved-decoder evidence entries remain unchanged. AGENTS remains
the CLAUDE.md symlink. All four private compilations recursively expand response
files and isolate object/dependency/possible PCM outputs, preserving exact
membership and hashes of all **748** shared compiled artifacts. They never replace
production libraries.

| Reviewed identity | SHA256 |
| --- | --- |
| Candidate manifest | `506381976c972a05d2bdffb56e72341868252bbf7e87c3cc9692476f5ee8ad90` |
| Combined candidate diff | `334f680a9b997023fa784d749c34863b545d921c62a57d53dde65b8cf8896f3f` |
| R01 diff | `7ae92ef1f632b4c119d5b53372a58a3291253bc0a8367d76476e7b3af598fc15` |
| Author artifact manifest | `cb8bdd69503c75140d0bc2b65e0175692bb47ef4a6ce9b2fa382ee324456f200` |
| Author report | `3b4f70433c5409a86f6fef0b22cf31fe31550ad4b4fa36ea2c77500920c9acb8` |

[Finalization](../../../tests/native/v5-02-x05-b3-b-r01-review-evidence/finalization.json)
binds this report, reviewer artifact manifest and final source/candidate rehash,
then releases Debug with no further reviewer edits. The coordinator must verify
this exact candidate is still current before recording approval or advancing.
No full B, release or Linux gate is claimed; latest full approved gate remains
A4's582 per configuration. B3-C W, B4 rendering, X06–X08, public profile admission/
cutover, mandatory external-profile acceptance, execution/RA01 and Linux remain
open. Public init creates format1.
