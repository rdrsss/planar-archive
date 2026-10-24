# V5-02-X05-C4-D2-B2 independent review

Disposition: **request-changes**. Required finding **B2-R1** remains open.
The candidate is not independently approved.

The shared checkout is `dev/planar-v2-vernacular`, HEAD
`3171c49e7b92d3abda01bfbb8bfb0c6b29d4c67a`, with the intentional dirty rewrite.
Reviewer ownership was limited to this report,
`tests/native/v5-02-x05-c4-d2-b2-review-evidence/`, and exclusive serialized Debug.
Product, author/prior evidence, vendor/tooling/fixtures and installed release were
read-only. No product fix, delegation, Planar tooling/skills/roles/queues, commit,
installation, publication, merge or operator-data access occurred. `AGENTS.md`
remains the `CLAUDE.md` symlink.

## Exact candidate and authority

The review considered the active instructions, implementation/roadmap and v5
direction, complete D2-B2 round including the two author corrections, X-CAP/P2 A
and X-LIMIT, and approved D1, D2-A, D2-B1 and current C4-C2 boundaries. Scope is
internal rooted discovery and the narrow declaration/reference forwarder. Role
services, X06 accounting and public profile behavior are outside this review.

| Author artifact under `tests/native/v5-02-x05-c4-d2-b2-evidence/` | SHA256 |
| --- | --- |
| Frozen candidate, 33 entries | `f549d60c3936256264c77f291f216dcb4cf07941cb469c047998cff1635cc431` |
| Artifact manifest, 252 entries | `aab8cc4d783adc25b40aaf3e7cbfc104b3ef634453f95345559c322e783b9eaf` |
| Finalization | `81bfaeeb125159249f4f2c40ef5e44aea6a22c111c69b94325e18f2391dc71f2` |
| Author REPORT | `629d369ffd3c4f065910c4f398d7621dc352273dea0bc9e0d9e32d8d50813d79` |

`identity-audit.json` verifies these identities, every current candidate entry and
all 252 manifested author artifacts, including modes/bytes/hashes.
`reviewed-candidate.json` preserves the exact reviewed candidate. The actual
candidate diff against the preserved preedit files was inspected. Frozen author
prose retains its pending-review wording; this report provides the disposition.

## Required change B2-R1 — preserve original root authority for retained reads

At `src/engine/capture/native.cpp`, `implementation::read_file` verifies the bound
root, but calls its `discover(path)` only when no file authority is already
supplied. On an initial retained read, an incoming file token can therefore govern
the leaf without establishing its correspondence to the bound root's original
admission. `read_bounded` then stores that incoming token in the captured entry.
Actual-FD equality alone does not establish original-owner selection authority.

The existing reviewer regression and result are preserved in
[`private-probe.cpp`](../../../tests/native/v5-02-x05-c4-d2-b2-review-evidence/private-probe.cpp),
[`private/regression.xml`](../../../tests/native/v5-02-x05-c4-d2-b2-review-evidence/private/regression.xml)
and [`private/commands.json`](../../../tests/native/v5-02-x05-c4-d2-b2-review-evidence/private/commands.json).
The one failing case exits42 with eight failed assertions across both retained
read forms. The recorded output demonstrates two actual data reads, exact fixture
bytes returned, both owners remaining healthy, and successful acceptance sealing
and verification. This violates the original-root selected-alias boundary before
content and remains a release-blocking correctness issue for this slice.

The independent positive-control case passes: a pre-admitted file receipt from
the same original admission retains its selection and passes acceptance; ordinary
rooted discovery refuses the disallowed alias before data access. See
[`private/positive.xml`](../../../tests/native/v5-02-x05-c4-d2-b2-review-evidence/private/positive.xml).
These controls distinguish required same-original reuse from the unrelated-owner
case. The correction must enforce original-root correspondence before content,
retain legitimate original explicit selections, preserve owned cache authority,
and make refusals terminal without an unbound retry. Both retained overloads,
cached/tree routes and acceptance need coverage in a fresh corrective round.

No product correction was made by this reviewer. The earlier author file-count
reuse and public/private namespace corrections remain part of this candidate;
the new finding does not reopen the predecessor approvals.

## Independent verification already completed

All successful checks below have zero failures/errors/skips/disabled cases.
Ordinary argv/results are in `commands.json`; isolated commands and sources are
under `private/`.

| Check | Result |
| --- | --- |
| Native/profile/admission/codec/host-path/capture plus C3-B2 focused Debug | 184/184, 43.55 seconds |
| Inventory | 851 names; all 838 baseline names preserved, 13 additions |
| Author private replay | 6/6 cases pass |
| Fresh reviewer positive control | 1/1 case passes |
| Fresh reviewer required regression | Expected exit42; 1 failing case, 8 assertions |
| Exact vectors / historical and current audits | Six / seven pass |
| Pinned formatting/Doxygen | 118 active sources pass |
| Initial/final ordinary `all planar_tests` builds | Both `ninja: no work to do.` |

The author's supplied-file count correction moves the genuinely-new-selection
ceiling after original explicit-selection reuse. The public slot lookup correction
excludes private discovered selections while token lookup remains index-based.
The focused run includes the final ordinary namespace regression. Preserved author
red/green chronology remains intact; neither correction resolves B2-R1.

The first reviewer fixture did not reproduce B2-R1 because its setup altered an
already retained metadata receipt. The candidate correctly refused it with zero
data reads. That attempt's source, XML, argv, complete dependency/journal proof
and cleanup are preserved in `attempt1/`. Only reviewer fixture setup changed
before the recorded reproducing run; no product byte or expected requirement
changed. The initial successful refusal is not counted as reproducing evidence.

Private compilation recursively expanded response files, disabled implicit modules
and maps, copied all 888 enumerated object/PCM/archive/dylib inputs and redirected
explicit compiled dependencies. Complete shared/copied membership, modes, bytes,
hashes and mtimes plus both shared Ninja journals remained exact. Verified cleanup
removed only reviewer-owned disposable copies/products: 891 files per attempt.
Sources, XML, argv, dependency/journal checks and removal inventories remain.
Copied runtime execution is not proved or claimed; recorded `otool -L` identifies
the platform runtime install names. No additional probes or investigation followed
the coordinator's finalization-only instruction.

## Preservation, handoff and limits

The independent baseline protects product, complete author/prior evidence,
vendor/tooling/fixtures, both release paths and the prior D2-B1 round. Only the
reviewer's two owned paths and exact coordinator D2-B2 round were exempted.
`protection-after.json` records zero membership/mode/hash/symlink drift.
Rewrite-release remains absent; install-release was never invoked. Final manifests
bind this report, exact reviewed candidate and retained verification/cleanup.

Exclusive Debug and both reviewer-owned paths are released at handoff. No live
build/test/private process or disposable copied dependency tree remains, and no
further reviewer edits are planned. This request-changes disposition requires a
fresh corrective search/coder/reviewer round preserving B2-R1 and this candidate.
Latest full approval remains 676/configuration. No full/release/Linux,
role-service, public CLI, profile-ledger acceptance or X06 gate is claimed.
X06–X08, mandatory external-profile acceptance and execution/RA01 remain open;
public init continues to create format1.
