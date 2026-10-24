# V5-02-X05-C1 independent review

Disposition: **approve**. No blocking findings. This approves the bounded pure
assembly slice; it does not close X05-C or native projection admission.

Review used the shared `dev/planar-v2-vernacular` checkout at
`3171c49e7b92d3abda01bfbb8bfb0c6b29d4c67a`. The intentional dirty rewrite and
others' work were preserved. Writes are confined to this report,
`tests/native/v5-02-x05-c1-review-evidence/`, and ordinary serialized Debug
verification outputs. Product/tests/support/current prose, author/historical
artifacts and the entire release tree remained read-only. No Planar tools,
skills, roles, queues, subagents, fixes, commits, installation, publication,
merge, installed binaries or operator data were used. Debug verification was
exclusive after checking for active build/test processes.

The review considered active instructions, implementation/roadmap/v5 direction,
the C disposition and C1 brief, X-PROJ and existing render contracts, B4-C approval,
the candidate diff, author report/support and raw results. Prior approved shape
work is a prerequisite, not independently reapproved by this review.

## Source and contract assessment

[`assemble_sections`](../../../src/stdlib/planar/render.lua) (lines 4–59) copies
base/custom bookkeeping, retains prepared base text and mandatory identity,
heads only custom bodies, and appends them in result order. It removes custom
optionals in reverse result order before checkpoint/job_notes/workspace_notes/
rationale, skips mandatory entries, and regenerates notice and complete joins
after every removal. Metadata remains in original order with notice last;
omitted IDs retain actual removal order. Included sizes count only actual joins,
and omitted sizes count standalone headed bytes. Exhaustion uses the existing
`budget_exceeded` field/message/details without truncation. These match
[X-PROJ](../external-profile-contract.md) (lines 363–375, 439–462).

The [private adapter](../../../src/stdlib/planar/render.lua) (lines 63–71) emits
exact `{id,bytes,included}` metadata. Ordinary rendering passes no custom records
(lines 226–229), preserves `{id,mandatory,included,bytes}`, and reuses the already
tagged omitted array in its manifest (line 245). The narrow
[bootstrap export](../../../src/stdlib/planar/bootstrap.lua) (line 33) introduces
no command or provider route. Native retained roots remain the existing six
profile phases in `runtime.cpp:1772–1782`; there is no native/CMake/inventory,
codec, hash, presentation, selection or admission change.

Authentic prepared base, already admitted custom records and a validated budget
are explicit private ABI preconditions, not properties established by this
helper. Native closed structure, UTF-8/NUL, section/count/total limits and
reserved-ID collisions remain C2 duties. No malformed caller-table or oversized
F-response admission claim is inferred. The source and affected prose correctly
preserve this boundary.

## Raw author evidence

The independent [raw audit](../../../tests/native/v5-02-x05-c1-review-evidence/raw-audit.json)
checks inventories, selected names, JUnit case membership/statuses, immediate
LastTest logs and command exit records. All 676 approved names are preserved in
682 unique current names. The author executed **416/416** focused cases and
**11/11** supplemental cases, with zero failures/errors/skips and all statuses
`run`. Direct assembly is **6/6**, production render **46/46**, old-body differential
**46/46**, and actual-freezer assembly **6/6**. All 21 outer and eight private
commands exit zero. The six vectors, six source/runtime audits, pinned lint over
118 active sources and final no-work build pass in the saved raw records; those
commands are inspected evidence, not claimed as independent reruns.

The actual archived renderer is SHA256
`803287f7bd091c2bea270021c4bbda94204f62648bc884f0691bb320455fbe51`.
The [scope audit](../../../tests/native/v5-02-x05-c1-review-evidence/scope-audit.json)
independently reconstructs the scoped diff and verifies that the private
comparison embeds that complete archived body with only the five recorded
function renames. The comparison runs authentic current semantic/selection
material through both render bodies and records **242 exact complete
Glaze-encoded response comparisons**, including existing goldens, shipped/custom
shapes, observations, recovery, budgets, manifests and errors. This isolates
renderer behavior; complete old/current effective-source identities are not
claimed equal.

The preserved development failure did expose an integration bug: retagging the
already typed omitted array caused `already_typed`. Reusing that array fixes the
fault; the final differential covers the manifest/error envelope. The author's
incorrect 26-byte test expectation and failed attempts remain recorded. The
reviewer's first raw-parser attempt incorrectly expected nine private commands;
[the retained record](../../../tests/native/v5-02-x05-c1-review-evidence/parser-attempt01.log)
explains the corrected eight-command assertion. Neither is concealed as a pass.

## Independent verification

The review's [ten serialized commands](../../../tests/native/v5-02-x05-c1-review-evidence/commands.json)
pass a Debug `all planar_tests` build, fresh 682-name inventory, existing
**46/46** render/profile-render cases, and a final build reporting
`ninja: no work to do.` No complete suite was unnecessarily rerun.

Three [independently authored boundary cases](../../../tests/native/v5-02-x05-c1-review-evidence/probe-extra.cpp)
add coverage for removal of the first base section and recomputation of the first
retained join; a mandatory base named checkpoint surviving full optional
exhaustion; and no retained base with a notice that fits exactly 52 bytes and
refuses at 51. Nonlexical custom IDs prove reverse result order. These cases plus
the six candidate assembly cases pass **9/9** through the production native
boundary, and **9/9** through a freshly compiled private setup using the actual
unmodified production `freeze_graph`. The latter checks root/base/custom write
refusal and two calls on the same frozen Lua input. Together with source inspection,
this supports nonmutation and exact byte accounting under C1's preconditions.

Private compile commands expand response files recursively and isolate every
object/PCM/dependency output under reviewer evidence. The author freezer setup
is copied read-only and freshly compiled; product sources are not changed.
Complete shared compiled membership and hashes remain identical across all
**888** objects/PCMs/archives. See [summary](../../../tests/native/v5-02-x05-c1-review-evidence/summary.json)
and [stability proof](../../../tests/native/v5-02-x05-c1-review-evidence/private-compiled-stability.json).

## Identity and release

[Starting](../../../tests/native/v5-02-x05-c1-review-evidence/before.json) and
[final](../../../tests/native/v5-02-x05-c1-review-evidence/final-source.json)
audits match all 229 candidate entries and 366 author artifacts. Complete source,
historical and author artifact membership/bytes/modes/symlinks across **30,295**
review-start entries remain identical, excluding only this report/evidence and
root `.git/`/`build/`. Independently comparing the actual 29,923-entry author
baseline with its 29,927-entry freeze finds exactly the nine authorized existing
edits and four new support files; the diff reconstructs exactly. AGENTS.md remains
the symlink to CLAUDE.md.

All **3,492 release files** remain byte-identical throughout review, including
all **940 approved B4-C release identities**. All 888 Debug compiled identities
also remain fixed. No release build/test was invoked.

| Author identity | SHA256 |
| --- | --- |
| Candidate manifest | `492e4d8cd34c8d23e40e9fb3b9709f1c02432e5b71ba165ce605900b34e6c88d` |
| Artifact manifest | `b483c5b4107fe05b7482e86513b2c3a2aaf5087715788f936b3ef6f92e40ead1` |
| Scoped diff | `5b381d9779e1215ec0fde2297e9333793946750697d2aecfc64dc0553694f1b0` |
| Author report | `7e93c911e40f5d83cdbf6420d0e8187606fe90de151678c9d99255d6f55b5cf3` |

Review [finalization](../../../tests/native/v5-02-x05-c1-review-evidence/finalization.json)
binds this report and reviewer artifact manifest. Debug and all reviewer edits
are released after final hashing, with no later edits. Coordinator advancement
requires checking that this exact approved candidate remains current.

Approval establishes pure assembly only. Native projection identity/admission/
retained F, read capabilities, initial/result receipts, whole-invocation
orchestration, profile CLI, X06–X08, execution/RA01 and Linux remain open. Public
init creates format1; the latest full approved gate remains B4-C's 676 cases per
configuration. This review makes no full C or release gate claim.
