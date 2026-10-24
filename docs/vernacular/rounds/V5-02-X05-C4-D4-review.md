# V5-02-X05-C4-D4 independent review

Disposition: **request-changes**, for D4-R1, an evidence correspondence correction.
No product defect was found in the reviewed role+projection implementation. The
candidate must not be described as independently approved until the required
correction receives fresh independent review.

The shared checkout is `dev/planar-v2-vernacular`, HEAD
`3171c49e7b92d3abda01bfbb8bfb0c6b29d4c67a`, with the intentional dirty rewrite.
I owned only this report, `tests/native/v5-02-x05-c4-d4-review-evidence/`, and
exclusive serialized Debug verification. Product, author/prior evidence and
installed release were read-only. I did not delegate, revert others, invoke Planar
tools/skills/roles/queues, commit, install, publish, merge or access operator data.

## Required correction: D4-R1

**P2 — The claimed original pre-edit source and before/after diff do not match
the recorded starting snapshot; the exact source used for the historical red run
is unproved.**

The author's `REPORT.md` states that `preedit-*` preserves actual pre-edit product
bytes, and that `red.xml` runs the new cases against the original pre-edit runtime
and Lua, failing because the role+projection pair was still closed. The round
also describes this as an exact preserved pre-edit boundary. The retained artifacts
do not establish those claims:

| Source | Recorded original baseline SHA256 | Present `preedit-*` SHA256 |
| --- | --- | --- |
| runtime.cpp | `1742fba034d9ff7f967365990b27e93a2a315f407318c9d2162b08092deeca8e` | `421e44e3727872a78e111f16bdc814b4abf11ad178e5501528fba498a8fc0046` |
| render.lua | `54802e3acce016e745a13f517c7f7d79e7d09ea13909b760ef3b8a7a47567ace` | `826433de51a3377e743a767645bb0fb139304fad1ed07d6e02fe79106516625b` |
| bootstrap.lua | `093e95058acceb264cc39fa9019a8b2e6934bcabf97ca490142eaeca8521013c` | `3d0f18fb8bdb41d390c826cf109c320d800e569c47cd3098e77646e51333d504` |

All four approved D3 `candidate-source` files, including the unchanged pre-edit
test file, match the original D4 `baseline.json` hashes exactly. The present three
pre-edit product copies already contain D4 implementation: independent role/
projection capability recognition, private phase9/helper and bootstrap export.
The bootstrap copy even equals the final candidate. Consequently the author's
`candidate.diff` omits the substantive D3-to-D4 implementation. This discrepancy
also exists in the preserved first frozen candidate. It is not a final bookkeeping
change.

`red.xml` does preserve four executed cases and two failures at
2026-10-24T10:40:26Z. Its two failures only establish unsuccessful results. The
available `red-build2.log` recompiles the tests, with no runtime recompilation.
There is no contemporaneous red source/command identity manifest. Current pre-edit
file mtimes near10:38:33Z already accompany the D4 prototype bytes. The record
cannot establish whether those copies were overwritten or were initially copied
after product edits, nor establish the runtime/Lua identity actually executed by
the red run. No actor or intent is inferred.

Required correction in a fresh corrective round:

- Preserve the present copies, diffs, failures and manifests as historical evidence.
- Add an explicitly labelled original baseline/diff from the independently approved
  D3 snapshots, checked against the contemporaneous D4 baseline hashes. Do not
  represent reconstructed or subsequently sourced files as original captured copies.
- Correct author/round claims to distinguish the genuine retained red outcomes
  from their unproved source identity and causal interpretation. Retract the
  claim that this run proves execution of the exact original closed-pair source.
- Bind the correction and truthful limitations with new exact manifests and
  independent review. No product change or repeated full gate is requested by
  this finding alone.

The reviewer-owned `review-D3-to-D4.diff` and `raw-evidence-audit.json` provide the
verified comparison and precise discrepancy without modifying author evidence.
This is feature-enablement evidence, not a demand to recreate a historical bug
regression. No historical baseline execution was manufactured during this review.

## Exact candidate and boundary assessment

The review read the active guide, implementation/roadmap and v5 direction, full D4
brief/amendments, exact X-PROJ/X-LIMIT contracts and approved C4-C2/D3 reviews.
The original author candidate20 has SHA256
`875be93d599299846396174e059bdc1f543ad5fb029bb341d23b1ca4648bfed0`;
artifact153 manifest SHA256
`8c046afeb672623c2e03b0fde3994c1aadad005dc8c29410a3300905f03f8bd0`;
REPORT SHA256
`6e1deed30d1990dd0c7df4f9ef48bc94b2764ecb868756db551d186f7854b482`.
Independent before/final audits check every candidate/artifact entry and identity.

Runtime now recognizes exact role read and projection independently while retaining
zero, role-only, history-only and history+projection protocols. The live original
context, once/order/sticky refusal and same-VM selected-F resolver remain in use.
F receives the original immutable authenticated role with matching input kind;
caller replacement input is unavailable. Raw correspondence still precedes owned
response conversion. Exact role DATA remains valid after projection, and semantic
E may omit both services.

The approved D3 private `{request,model,selected}` basis supplies the new phase9.
That root joins the initial trusted audit/freeze; existing indices0–8 retain their
meaning and capacity derives from the actual phase array. No recapture, selection,
shape F or new VM occurs at project time. The extracted `role_base_material`
retains the original section body bytes and mandatory flags. Ordinary rendering
and the private projection helper share it and the complete profile consistency
checks. Observation assertions revalidate from retained bytes. Effective budget
is request override or selected job budget, at most1MiB; history remains fixed1MiB.
The existing scanner reserves all18 IDs and the assembler retains reverse custom
omissions, then checkpoint/job/workspace/rationale priorities, exact notice bytes,
joins and metadata.

The approved borrowed frame restores outer stack/input/memo/aliases/callable and
execution identity without resetting counters or releasing outer roots. Helper
and assembly failures remain outside selected-F attribution; native F/scanner/
conversion errors retain original projection ordinals. Owned semantic E and first
native errors survive catches, moves/retries and later failures. Original role
and profile receipts verify before release on both handler branches.

Compiled command/VM/invocation/frame layouts remain1056/1464/600/56 bytes. The
extra four-byte phase slot fits existing aggregate padding. The aligned conservative
root/function reservation grows24to32 bytes, charging8 more cumulative native bytes;
physical owner layout does not grow. Nested frames and transport/codec work remain
charged. These are separate bounded metrics, without X06 shared-budget/RSS claims.

## Independent verification

Final successful checks have zero failures/errors/skips/disabled cases. Complete
argv/results are in `commands.json` and `private/commands.json`; copied sources,
XML, dependency/journal/loader proofs and cleanup inventories are retained.

- Final-source focused macOS Debug:536/536; current inventory876.
- Six vectors, seven source/generated-variant audits and pinned formatting/Doxygen
  over118 active sources pass.
- Private controls:16/16, 947 assertions, including ten author replays and six fresh cases.
- Initial and final all+planar_tests builds: `ninja: no work to do.`

The six fresh private cases supplement ten author replays. They check exact1MiB
success and one-byte mandatory overflow including optional-removal notice bytes;
exact copied role/projection DATA versus an extra raw key; retained observation
checkpoint bytes and zero content reads during base generation; original projection
ordinal10 through actual native allocation refusal and moves/retries; once-per-job
shape F and private consistency refusal; and public/mismatched-owner guards inside
nested F/helper/assembly execution. Replays cover aliases/GC/frame/counters,
resource/native/Lua failures, receipt mutation on both branches, lazy services and
semantic-first errors.

Two failed reviewer attempts are preserved. Attempt1 has13/16 passing cases and
three fixture errors: total-base overflow may correctly remove optional base;
curated `string.find` is absent; and the all-job fixture contains two custom jobs,
so it correctly calls F twice. Attempt2 has15/16 passing cases: helper-only zero-IO
passed, but the fixture incorrectly extended it through final mandatory receipt
verification, which rereads the captured observations. Corrections change only
reviewer fixtures, retain helper-only zero-IO, and account for exact mandatory
base plus omission notice. Product bytes remain unchanged throughout.

Each private run recursively expands response files, disables implicit modules/maps
and redirects all888 enumerated object/PCM/archive/dylib dependencies to owned
copies. Complete shared/copied membership, modes, bytes, hashes and mtimes plus
both Ninja journals are checked before removal. Actual dyld logs establish copied
admission-helper loading. System libc++ install name remains recorded; copied
libc++ execution is not claimed.

## Preservation and limits

Final protection compares76348 entries with zero unauthorized drift; all20 author
candidate and153 artifact entries remain exact. Each private run verified and
removed its893 disposable dependency/product files; no owned compiled products,
dependency tree or live build/test/private process remains. Debug and reviewer
files are released at handoff. Exact review manifests bind this report, source
identity, raw results, preservation and cleanup.
The exact coordinator D4 round is the only non-review ownership exemption; D3,
author and prior evidence are protected. No full/release/Linux, public profile CLI,
profile-ledger acceptance, X06 shared-budget or mandatory external-profile completion
gate is claimed. Latest full approval remains676/configuration. Public init
continues to create format1. D4-R1 remains open for a fresh corrective round.
