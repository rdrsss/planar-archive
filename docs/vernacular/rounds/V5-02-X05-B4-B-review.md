# V5-02-X05-B4-B independent review

Disposition: **request-changes**. **B4B-R1** blocks approval: ordinary render
error envelopes can bypass the complete-response 16 MiB check. B4-B and the
dependent B4-C gate remain open. No implementation correction was made.

## Scope and candidate

Reviewed the [round authorization](V5-02-X05-B4-B.md), active repository guide,
implementation/roadmap/v5 direction, exact external-profile render/shape/limit
contracts, approved B4-A+C01 helper correspondence, B3-C native selection and
the author's [report](../../../tests/native/v5-02-x05-b4-b-evidence/REPORT.md),
incremental diff, allocation account and preserved failed/recovery evidence.
This is the shared `dev/planar-v2-vernacular` checkout at
`3171c49e7b92d3abda01bfbb8bfb0c6b29d4c67a`. Candidate, author artifacts and
historical evidence were read-only. Reviewer ownership is this report and
`tests/native/v5-02-x05-b4-b-review-evidence/` only. No other edits were reverted;
no Planar tools/skills/roles, subagents, commits, installation, publication,
merge, provider or operator-data operations were used.

The original independent review session ended before publishing its report.
This continuation preserved its raw files, waited for its already-running
exhaustive allocation process to finish, and completed only bounded remaining
review work. There was no concurrent build or support edit during that process.

| Reviewed author identity | SHA256 |
| --- | --- |
| Candidate manifest, 211 entries | `2e6b3e9beac951f9f7269fdbe0441ff0c1009ed8893df8c48c37857bcb160a2b` |
| Owned candidate diff | `abb009633070b7cdca1a18e349e233a9f3ade8bd32957b869f721126ae9ed446` |
| Author artifact manifest, 734 entries | `d586eade82564b24a533d3257a5d8669b1ee448a21af2acd268ce326c454eb80` |
| Author report | `8a06f4af852baa36a808feaa1dd4f3d7c11a08ed0170920e3944733c3e1b08ba` |

## B4B-R1 — Complete response bound excludes early ordinary errors

The [exact contract](../external-profile-contract.md#x-limit--cumulative-ceilings-and-errors)
requires output 16 MiB and cumulative allocation checks; the round explicitly
requires the complete result bound. The new reserved encoder is reached only
after phase 5 rendering. In
[runtime.cpp](../../../src/engine/runtime/runtime.cpp#L3095), normalization errors
return directly at line 3098. Preparation errors at line 3117, checked shape
errors at line 3140, finalization errors at line 3160 and selection errors at
line 3164 also return before the reserved complete-response encode at line 3169.
Their native conversion and ordinary error status do not enforce encoded output
size.

The independent [original-input reproducer](../../../tests/native/v5-02-x05-b4-b-review-evidence/adversarial-final.cpp#L129)
creates an actual captured profile and calls the exported native `render_profile`
operation with its ordinary original render request plus one unknown key of
**16,777,217** ASCII bytes. No model, context, selected material, response or
runtime result is substituted. Native input validation accepts the finite graph
under its retained cumulative limits. Existing
[schema normalization](../../../src/stdlib/planar/schema.lua#L79) constructs the
field as `render.` plus the key and the
[preflight](../../../src/stdlib/planar/render.lua#L220) returns that schema error.
The external shape callable contains a failure sentinel and is not reached.

`render_profile` succeeds with ordinary error data whose `field` alone is
**16,777,224** bytes. Calling unchanged `codec_encode` on that exact returned
graph refuses with `resource_limit`, `output_bytes`, maximum **16777216**.
The [final log](../../../tests/native/v5-02-x05-b4-b-review-evidence/ceiling-final/ceiling.log),
[failed assertion XML](../../../tests/native/v5-02-x05-b4-b-review-evidence/ceiling-final/ceiling.xml)
and [exact commands](../../../tests/native/v5-02-x05-b4-b-review-evidence/ceiling-final/commands.json)
retain the reproduction (expected failing test exit 42). The exact returned
field bytes and the remaining error graph are preserved separately in
`oversized-schema-field.bin` and `oversized-response-without-field.json`.
The earlier `public-controls/ceiling` failure independently records the same
product behavior.

Required correction: every ordinary envelope returned by the render operation,
including normalization, semantic/check/finalization and selection refusals,
must pass the same reserved complete-response encoding boundary. Preserve the
existing normalization/semantic/selection priority, zero F execution on schema
errors, cumulative accounting, and first typed terminal resource diagnostic on
retry/move. A fresh corrective round must retain this failing original-request
regression and cover the ordinary early-return paths. Do not relax the output
ceiling or replace the diagnostic merely to satisfy the probe.

The author's after-render oversized-result adapter still usefully proves the
last encoder's branch. Its substitution happens after genuine rendering and
does not exercise these earlier returns; its passing result cannot close R1.

## Other reviewed behavior and independent verification

The native composition otherwise follows the authorized structure: original
render and stricter selection normalization precede semantics/F; retained fixed
roots join the initial audit/freeze; native-owned phase results survive cleanup;
the same-call finalized model/selection feed the approved private helper. Custom
F dispatch is once per custom job, and generic profile invocation stays closed.
No second helper validation/F/selection or new provider/CLI surface was found.

The preserved independent [Debug gate](../../../tests/native/v5-02-x05-b4-b-review-evidence/gate/commands.json)
passes **408/408** focused cases, **11/11** supplemental history/codec cases and
**19 profile-render cases / 1123 assertions**, with zero failures/errors/skips.
Inventory 674 preserves the prior 667 names. Six vector families, six established
source audits, pinned formatting/Doxygen 118 and no-work builds pass. The final
private compilations expand response files, isolate object/PCM/dependency outputs
and verify shared compiled-file membership and hashes unchanged.

The independent [production-linked controls](../../../tests/native/v5-02-x05-b4-b-review-evidence/public-controls-final/commands.json)
pass **2 cases / 741 assertions**. They compare complete shipped render/error
bytes for **150** combinations over five job/role pairs, capture on/off,
absent/malformed/recovery observations and five budgets. They also check caller
internal-material injection refusal, two module owners in both orders, genuine
selected definition/W/guidance correspondence, repeated rendering, immutable
input and native response lifetime after owner destruction.

The already-running independent
[exhaustive allocation control](../../../tests/native/v5-02-x05-b4-b-review-evidence/exhaustive-allocation/commands.json)
completed successfully: **11073 observed requests, 11073 actual one-shot
refusals, 1 case / 110743 assertions**, zero failures/errors/skips. It exercises
real production rendering linked to the candidate and checks typed first
terminal failures and exact retry diagnostics. These are controlled native
allocation refusals, not natural host exhaustion or RSS measurements.

The strengthened reversible
[lifetime/transport controls](../../../tests/native/v5-02-x05-b4-b-review-evidence/lifetime-controls-corrected/commands.json)
pass all **23 modes** and the separate green preflight (**24 cases / 569
assertions**); the preflight bypass red control fails as expected with exit 42.
They check exact retained key/value/type/identity membership and all 20 callable/
environment/phase refs plus the three registry-table identities, cumulative
counters, cleanup, success/error/resource paths, final writer reservations,
zero live Lua bytes at destruction and authentic phase/F dispatch. Twenty-five
successive renders execute exactly 50 custom F calls and preserve exact 23
retained identities across collection. The reversible source transformation
and shared compiled hashes are recorded alongside the results.

The author's support-only recovery is substantiated by the preserved failed
support snapshot, measured tags hash 512→1024 increase and unchanged exact
memberships/identities. The pinned Node is 24 bytes, so its 512-node increase
accounts exactly for 12288 live bytes. The corrected observer subtracts only
measured Node-array storage. Pinned Lua's
[Limbox header](../../../vendor/lua/0b10/src/ltable.c#L48) stays present at all
observed capacities and remains in the exact residual; no header or arbitrary
heap allowance is subtracted. The independent strengthened run also requires
zero array slots and non-dummy tables. The author's 16 fresh-process capacity
runs remain audited author evidence; they are not claimed as 16 independent
reruns.

Two reviewer-support attempts are preserved: `lifetime-controls` failed to
compile because the new non-dummy assertion lacked `ltable.h`; the corrected
private copy adds only that include. `public-controls-final/ceiling` stopped at
an auxiliary request-hash assertion before the intended assertion (the request
hash has its own canonical bounds). `adversarial-final.cpp` removes that
auxiliary hash and reaches the actual response-bound failure in `ceiling-final`.
Neither attempt changes product bytes or supplies a successful product gate.

## Final identity and release

The initial [scope check](../../../tests/native/v5-02-x05-b4-b-review-evidence/scope-start.json)
independently reconstructed the exact owned diff. Final checks rehash all 211
candidate entries, 734 author artifacts, 211 frozen source entries, 21492 author
protected entries, seven protected unowned entries and 24430 reviewer-protected
entries, with zero mismatches. Complete release membership and hashes remain
exact for 3463 files. AGENTS remains the CLAUDE.md symlink. The final Debug
all/planar_tests build reports `ninja: no work to do.`

The [final checks](../../../tests/native/v5-02-x05-b4-b-review-evidence/final-checks.json),
[review artifact manifest](../../../tests/native/v5-02-x05-b4-b-review-evidence/artifact-manifest.json)
and [finalization](../../../tests/native/v5-02-x05-b4-b-review-evidence/finalization.json)
bind this report and preserved raw evidence. Debug ownership and all reviewer
edits are released; no later reviewer edits are permitted. This is focused
macOS Debug review, not a full 674-case gate, release gate, B4-C acceptance or
Linux validation. B4-B remains unapproved pending B4B-R1 correction and fresh
independent review. X06–X08, public cutover, mandatory external-profile
acceptance, execution/RA01 and Linux remain open. Public init creates format1;
latest full approved gate remains A4's 582 per configuration.
