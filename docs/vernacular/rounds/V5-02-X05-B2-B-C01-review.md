# V5-02-X05-B2-B+C01 independent review

Disposition: **approve**. C01 closes **B2B-R1** across the shared encode error
boundary. The combined B2-B+C01 codec reservation foundation is independently
approved. No additional blocking capacity or ordinary-behavior finding remains.
This does not close B2 or the B milestone.

The review used shared `dev/planar-v2-vernacular`, HEAD
`3171c49e7b92d3abda01bfbb8bfb0c6b29d4c67a`, preserving the intentional dirty
rewrite and others' work. Candidate and current reference files were read-only.
Writes are confined to this report, new
`tests/native/v5-02-x05-b2-b-c01-review-evidence/`, and ordinary verification
outputs in the exclusively owned, serialized Debug build directory. No Planar
tools, legacy skills/roles/queue, installed binaries, operator data, commits,
installation, publication or merge were used. The assigned C01 round and its
recorded search disposition supply the implementation/review authority.

## R1 closure and complete encode boundary

The [shared encoder](../../../src/engine/json_codec/codec.cpp#L335) retains the
original validation, capacity derivation, hook, graph builder, Glaze writer and
inner exception adapters. Its new outer `bad_alloc`/`length_error` handlers now
protect allocation inside every inner adapter, including the refusal handler,
callback exception handlers and unexpected-exception redaction. Ordinary encode
continues to delegate to this implementation. Limit clamping before the boundary
performs no allocation. The final resource diagnostic uses the effective output
byte maximum and zero offset.

Normal validation and callback diagnostic precedence is unchanged. A supplied
typed callback refusal moves its owned diagnostic directly, including long
message/limit strings and the original maximum. Successful graph/output
allocation failures remain typed. Diagnostic construction after writer refusal
also stays protected. Decode, interfaces, reservation math, JSON grammar,
serialization, runtime behavior and limits are unchanged by C01.

The final fallback diagnostic can itself allocate. Approval covers controlled
one-shot allocation refusal with subsequent requests permitted; it does not
promise recovery from persistent heap exhaustion or extend the finding to decode.
The inherited ordinary encode gap and the newly promised reserved/callback
boundary are both corrected, without claiming the former was a new regression.

The unchanged original review probe passes independently against current
production libraries: all seven actual diagnostic-allocation controls, all eight
copy/string/output allocation ordinals, 513 string-capacity fixtures, invalid
input without hook invocation, moved long first refusal and null-hook ownership.
The two phase observer cases and two supplemental normal/callback controls also
pass. The preserved author red evidence still records two failed original cases,
seven failed escape assertions and exit 42; the original review's evidence and
source remain unchanged. The supplemental author's first red runner mismatch
and its failure outputs also remain preserved.

Two additional [reviewer cases](../../../tests/native/v5-02-x05-b2-b-c01-review-evidence/extended.cpp)
pass **188 assertions**. Eighteen actual one-shot controls cover ordinary and
reserved validation refusals for null, out-of-int32 integer, malformed string,
malformed key, cycle, unsupported tag, depth, nodes and output bytes. All return
exact typed output-resource diagnostics without invoking the hook. Two further
controls first observe the real postwriter diagnostic allocation (ordinal 2),
then deny that request for ordinary/reserved output whose escaped form exceeds
its byte ceiling. Both retain the exact resource diagnostic without escape.
These extend coverage beyond R1's original null/callback cases. The first
reviewer-only compile failed on an unused inherited helper under `-Werror`;
`extended-probes/` preserves its exact source, runner and log. Removing that
unused helper produced the passing `extended-probes-02/` result; no product
source changed and no failed product test was discarded.

## Combined reservation foundation

The [shared validator and reservation](../../../src/engine/json_codec/codec.cpp#L151)
use checked addition/multiplication and bounded stack integer formatting. The
fixed active path contains containers only: depths 0–255 fit, scalar leaves at
256 need no extra slot, and a 257th container refuses before indexing. Membership
is per active path, so cycles refuse while repeated aliases are measured and
copied repeatedly. Existing nodes, bytes, UTF-8, int32 and error-order policies
remain shared between ordinary and reserved entry points.

Graph allowance covers the root, exact array slots and key/node pairs, with
embedded string/container objects already present. Each copied vector reserves
its full entry count before filling. Pinned libc++ string allocation rounding
and exceptional capacity slack are covered separately for every key/string and
the fixed output buffer. Sorting moves existing objects; Glaze writes into the
fixed span. The output string moves into the result and is counted once.
Workspace is an explicit conservative logical allowance whose moved slots can
overlap graph objects; it is not independent heap storage, full stack size or RSS.
Supported hard caps bound all arithmetic on this 64-bit target; no artificial
larger limits or public test knobs were introduced.

Successful validation/reservation performs no heap allocation before the native
hook, and a typed hook refusal allocates no graph/output. Independent phase and
capacity probes corroborate the source derivation, including aggregate requests
within graph/string/output allowance even without workspace. Input stability
through the synchronous trusted native hook is an explicit C++ precondition;
there is no detached plan or claim of safety for callers violating it. No runtime
consumer, raw-return conversion charging or invocation integration is delivered.

## Independent verification and identities

The [recorded gate commands](../../../tests/native/v5-02-x05-b2-b-c01-review-evidence/final-gate/commands.json)
configure Debug and build `all planar_tests`. The exact prior B2-B focused
selection passes **346/346** in **188.23 seconds**, and the separate direct
history/codec selection passes **11/11**. Both have zero failures, errors, skips
or disabled cases. Inventory is **612**, preserving all prior 606, 600 and 582
names; inventory is not a full test run. All **nine** retained private cases and
**two** new reviewer cases pass against production codec libraries. Independent
shape-definition bytes, four common canonical/source vector checks, six exact
source/variant audits, pinned formatting/Doxygen across **118** active C++
sources and the no-work `all planar_tests` build pass.

[Scope verification](../../../tests/native/v5-02-x05-b2-b-c01-review-evidence/scope-diff.json)
reconstructs the exact C01 diff. Compared with B2-B's 384-member scoped freeze,
the 387-member freeze changes only `codec.cpp` and six authorized current status
documents, adds three C01 support files, and removes none. The interface and
registered codec tests remain byte-identical to B2-B. All **14** combined
candidate members and **126** C01 artifacts match their manifests; all **90**
prior author and **74** prior review artifacts remain unchanged. This is scoped
verification, not a whole-tree claim. AGENTS remains a symlink to CLAUDE.

| Reviewed author identity | SHA256 |
| --- | --- |
| Combined candidate manifest | `4231de550042932b7ae98d249f845708a795b4bf59c99294351cc972249c22b8` |
| C01 owned diff | `0394161beab7a0a780adf1a4129cf1bd35f849d91d97b21a015a4cf7a925fe1c` |
| C01 artifact manifest | `2425f942948717e711e16234c705c01b892bbb82da7a2f1889689da3316e483b` |
| Author report | `4a05ff4c1dd972d3481bd8c0b5f1774692e81a94b6d7fa04d1c4fc09a37524c0` |

Reviewer finalization binds this report, retained artifacts, exact candidate and
final unchanged source identities. Debug ownership is released after final
hashing; no reviewer edits follow release. No full 612-case, release or Linux
gate was run. The latest full gate remains A4's 582 per configuration. Complete
16 MiB/+1 codec transport tests do not prove shape semantic acceptance. B2-C
retained F dispatch, immutable F input, raw-return conversion and invocation
integration, B3 selected identity/W, B4 render, X06–X08, execution/RA01 and Linux
remain open. Public init continues to create format1.
