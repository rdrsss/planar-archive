# V5-02-X05-B4-B+C01 independent review

Disposition: **approve**. The combined native B4-B+C01 candidate closes **B4B-R1**.
No other blocking finding remains in this scope. B4-C's full Debug/RelWithDebInfo
gate remains separate and has not run in this review.

## Scope and candidate identity

Reviewed the [B4-B authorization](V5-02-X05-B4-B.md),
[C01 authorization](V5-02-X05-B4-B-C01.md), active repository guide,
implementation/roadmap/v5 direction, exact external-profile shape/render/limit
contracts, [original independent review](V5-02-X05-B4-B-review.md), both author
reports, incremental and combined diffs, allocation accounts, raw original R1
reproduction, and preserved failure/recovery evidence. The shared checkout is
`dev/planar-v2-vernacular` at `3171c49e7b92d3abda01bfbb8bfb0c6b29d4c67a`.
Product, production tests, author support/evidence and historical artifacts were
read-only. Reviewer ownership is this report and
`tests/native/v5-02-x05-b4-b-c01-review-evidence/`, with exclusive serialized Debug
verification. No unrelated edits were reverted. No Planar tools/skills/roles,
subagents, implementation fixes, commits, installation, publication, merge,
provider or operator-data operations were used. Release remained read-only.

| Reviewed identity | SHA256 |
| --- | --- |
| Candidate manifest, 210 entries | `7f9ee160363fba3af1d652f328ffab596a2cad38cf6b5b98062f20c8c46e55a4` |
| C01 incremental diff | `5c0671642ddbec600eebd587c10c9c3e9e3a5739d7ba99f620380bd93611ea0c` |
| Combined B4-B+C01 diff | `f74a729e8af032465b453358d9b1495013c81a0eaefe0a8178d6d1c21aae5746` |
| Author artifact manifest, 276 entries | `471a1c7a3044b0e22f3817394e7198e9f566ef3792c1e0f7e95bdcc92a3d481d` |
| C01 author report | `a0253cb2dfd3b2f48b6d614a492edb050b89292dc5de14de659787e88a7463f6` |

The independent [starting audit](../../../tests/native/v5-02-x05-b4-b-c01-review-evidence/scope-start.json)
reconstructs both exact diffs from their actual preserved pre-edit baselines and
checks all 210 candidate, 276 author-artifact, 210 frozen, 22385 author-protected
and 203 prior unowned entries with zero mismatches. A separate reviewer baseline
protects 22592 historical/release files. Complete release membership is exactly
3463 files, matching the author's original protected membership.

## B4B-R1 closure and combined source review

The local [finish helper](../../../src/engine/runtime/runtime.cpp#L3096) routes
normalization (3122), preparation (3141), checked-shape (3164), finalization
(3184), selection (3188) and final render (3190) ordinary envelopes through the
same existing reserved complete-response encoder. The reservation callback and
resource/protocol mapping are the original final-render implementation. The
helper returns the original `value_ptr`; it neither copies a graph nor runs a
phase again. Validation-only/selection-only calls return before encoding.
Native unexpected failures remain on the existing typed paths, including retained
shape failure and exception adaptation. Resource failure becomes terminal before
fallible outward copying, preserving the first typed diagnostic through retries
and moves. The codec's existing default output ceiling remains 16777216 bytes;
its [validation, reservation and writer](../../../src/engine/json_codec/codec.cpp#L727)
remain unchanged. All native charges remain cumulative without refunds or resets.

The combined B4-B composition preserves original full-render then budget-stripped
selection schema priority before semantics/F. The six retained trusted phase
roots [join the original audit/freeze](../../../src/engine/runtime/runtime.cpp#L1617)
before external evaluation; their fixed references are retained at bootstrap.
Native-owned normalized intent, finalized semantic model and selection are passed
through sequential protected calls to the approved pure renderer. Each custom
job runs F once. Native inputs/results survive Lua cleanup, and the helper receives
authentic same-call material. Generic profile invocation still refuses before
provider effects. The two added inline reference slots receive the checked
8-byte initial charge; the pinned private layout assertions remain valid.
Shared Lua normalization merely extracts the existing exact schemas; no shape,
codec, hash, presentation, provider, CLI or history policy is broadened by C01.

The original [raw R1 log](../../../tests/native/v5-02-x05-b4-b-review-evidence/ceiling-final/ceiling.log)
was inspected. Independent replay uses the exact C01 baseline runtime in an
isolated object and the preserved [original-request reproducer](../../../tests/native/v5-02-x05-b4-b-c01-review-evidence/adversarial.cpp#L141).
The baseline again returns ordinary schema data with a 16777224-byte field; the
unchanged codec refuses that exact graph with output_bytes/16777216. The
[baseline log and XML](../../../tests/native/v5-02-x05-b4-b-c01-review-evidence/public-controls/baseline-ceiling.log)
preserve expected failing exit 42. The same original request on the current
runtime passes the required typed native-refusal assertion. No model, selected
material, context, response or runtime output is substituted.

Two additional independent original-input controls pass: a 3 MiB control-character
unknown key causes JSON escaping to exceed the encoded ceiling and retains exact
terminal diagnostics after moves/retries; a nearby ASCII unknown key returns an
ordinary schema error of **16776798 encoded bytes**, below the ceiling. A small
escaped-key error remains byte-identical to ordinary embedded rendering. The
[escaped control](../../../tests/native/v5-02-x05-b4-b-c01-review-evidence/public-controls/escaped.xml)
passes 35 assertions and the [near-limit control](../../../tests/native/v5-02-x05-b4-b-c01-review-evidence/public-controls/near.log)
passes 16. The original oversized regression passes 14 assertions on current code.

## Independent verification and evidence limits

The independent [serialized Debug gate](../../../tests/native/v5-02-x05-b4-b-c01-review-evidence/gate/commands.json)
passes **410/410** focused cases and **11/11** supplemental history/codec cases,
with zero failures/errors/skips. Inventory is 676 and preserves all 674 previous
names. The profile-render subset passes **21 cases / 1256 assertions**. Six vector
families, six established exact source audits, pinned formatting/Doxygen over
118 active C++ files and a no-work build pass. Product/test/support/prose bytes
remain frozen throughout the review.

The independently compiled production-linked [public controls](../../../tests/native/v5-02-x05-b4-b-c01-review-evidence/public-controls/commands.json)
pass **2 cases / 741 assertions**: 150 complete shipped-output comparisons across
five job/role pairs, capture on/off, three observation states and five budgets;
caller internal-material injection refusal; two genuine module owners in both
orders; exact definition/W/guidance correspondence; repeated rendering, input
immutability and retained output after owner destruction.

The current reversible [lifetime and boundary controls](../../../tests/native/v5-02-x05-b4-b-c01-review-evidence/lifetime-boundaries/commands.json)
pass all **22 retained transport modes / 516 assertions**, green preflight
(**32 assertions**), genuine phase/pointer/pass coverage (**121 assertions**) and
per-phase final-reservation refusal (**249 assertions**). The preflight-bypass red
control fails as expected with exit 42. Each of eight genuine scenarios traverses
the complete writer once and returns the exact observed response pointer; phase/F
counts preserve zero F on schema/base errors, one on checked refusal, and one per
custom job on later paths. Validation/selection-only operations do not invoke the
final writer. The reservation-refusal controls retain first field/code/message/
limit/maximum through moves and render/validation/selection retries.

Exact retained refs, all three registry table identities and complete key/value/
type/identity membership survive GC and cleanup. Twenty-five successful renders
run exactly 50 F calls, preserving all 23 retained identities and cumulative
counters. The observer requires non-dummy tables with zero array storage and exact
stable live heap after subtracting only measured pinned registry hash Node
capacity; owner destruction requires zero live Lua allocator bytes. This preserves
the independently reviewed explanation for the historical 12288-byte tags-table
512-to-1024 capacity growth, rather than introducing a heap tolerance. The
historical support failures remain intact and are not product findings.

Exactly 22 of the historical 23 modes are rerun: the old after-render substituted
oversized-response mode relied on the relocated insertion seam and is replaced
by the genuine original-request red/green regression and the independent escaped/
near-limit controls. The historical exhaustive 11073 actual refusal review remains
protected, inspected evidence; no exhaustive rerun is claimed here.

Current [native allocation controls](../../../tests/native/v5-02-x05-b4-b-c01-review-evidence/allocation-controls/commands.json)
pass **2 cases / 4104 assertions**: 11073 observed success requests, 138 sampled
actual one-shot refusals, plus 128 actual last-request refusals over eight genuine
phase scenarios. Each final-16 injected request is reached, returns a typed
resource failure, and preserves exact terminal diagnostics through moves/retries.
These are controlled allocation refusals, not natural host exhaustion or RSS
measurements. The final 16 requests belong to each whole operation; they are not
all asserted to be exclusively writer allocations.

All private compilations expand response files, isolate object/PCM/dependency
outputs and verify complete shared compiled membership and hashes unchanged
(888 entries). The exact original baseline replay is verification-only; no shared
runtime source or build object is replaced. Support copies and their provenance
are recorded in the review evidence.

## Final integrity and ownership release

The [final checks](../../../tests/native/v5-02-x05-b4-b-c01-review-evidence/final-checks.json)
repeat candidate/artifact/protected hash checks and complete protected membership
checks with zero mismatches, preserving all 3463 release files and the AGENTS →
CLAUDE.md symlink. The final Debug all/planar_tests build again reports
`ninja: no work to do.` The [review artifact manifest](../../../tests/native/v5-02-x05-b4-b-c01-review-evidence/artifact-manifest.json)
binds this report and raw evidence; [finalization](../../../tests/native/v5-02-x05-b4-b-c01-review-evidence/finalization.json)
records its identity and release. No build/test process remains. Exclusive Debug
ownership and all reviewer edits are released; no later reviewer edits are permitted.

This approves combined B4-B+C01 and closes B4B-R1 only. It is focused macOS ARM64
Debug verification, not a full 676-case or release/Linux gate. B4-C remains the
next complete Debug/RelWithDebInfo gate. Public profile admission, X06–X08,
mandatory external-profile acceptance, public cutover, execution/RA01 and Linux
remain open. Public init creates format1; latest full approval remains A4's
582 per configuration.
