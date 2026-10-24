# V5-02-X05-C4-D2-B2+C01 independent review

Disposition: **approve** for the combined internal original-root discovery and
C01 correspondence correction. **B2-R1 is closed.** No required changes or
unresolved product choices remain in this bounded scope.

The shared checkout is `dev/planar-v2-vernacular`, HEAD
`3171c49e7b92d3abda01bfbb8bfb0c6b29d4c67a`, with the intentional dirty rewrite.
Reviewer ownership was limited to this report,
`tests/native/v5-02-x05-c4-d2-b2-c01-review-evidence/`, and exclusive Debug.
Product, author/prior evidence, vendor/tooling/fixtures and installed release were
read-only. No product edit, delegation, Planar tool/skill/role/queue, commit,
installation, publication, merge or operator-data access occurred. `AGENTS.md`
remains the `CLAUDE.md` symlink.

## Authority and exact candidate

The review considered the active guide, implementation/roadmap and v5 direction,
the complete C01 brief, predecessor B2 brief/review including its quota and
namespace corrections, exact X-CAP/P2 A and X-LIMIT, and approved D1, D2-A,
D2-B1 and current C4-C2 boundaries. Scope includes original rooted discovery,
its high-level declaration/reference forwarder and the B2-R1 correction.

| Author artifact under `tests/native/v5-02-x05-c4-d2-b2-c01-evidence/` | SHA256 |
| --- | --- |
| Frozen combined candidate, 28 entries | `ca756141175a1b3772a946e1246a13f04204b4306e86be8ecfb46c190a725781` |
| Artifact manifest, 129 entries | `4f3606796d9c6f9456db9e4e8372f115f707847eddb360972bff6dcd5bdc88a3` |
| Finalization | `cd6be0808562ae9f3b661802f0d56adeabd7b0532caa27ce9ef6dcc5d2a0ddc8` |
| Author REPORT | `b1b010088684e06e7ee17a6a7ee82f58eff88cbe67fe3aa2da927e7be4fa4380` |
| Combined B2+C01 diff | `27a12797a5a9c8c28b4ac81e3bdae95971e1aa43017d3004652a85173781132e` |

`identity-audit.json` independently verifies every current candidate entry and all
129 manifested author artifacts, including modes, bytes and hashes.
`reviewed-candidate.json` preserves the exact candidate. `raw-evidence-audit.json`
reconstructs the combined diff from B2's actual preedit sources and verifies all
combined source copies. It also confirms C01's preedit native source is the exact
B2 candidate. C01 changes production only in one native block, +11/-3 lines, with
no additional admission API or layout change. Original B2 candidate/artifact/
finalization/report identities and complete predecessor evidence remain protected.
Frozen author prose keeps its pending-review wording; this report supplies approval.

## Boundary assessment and B2-R1 closure

At `capture/admission.cpp:546`, the root token retains the original shared
admission and immutable supplied-root index. Discovery validates portable paths,
walks original metadata descriptors with no-follow opens, verifies all original
relationships and selected aliases, and performs no content read or enumeration.
The root/path index returns the original selection for repeats. Explicit supplied
files retain their original immutable selection before native caching; only a new
selection consumes the file-count ceiling. Private discovered ordinal keys remain
bounded and excluded from caller-slot lookup, in either admission ordering.
Root/file wrapper moves and destruction do not replace original authority.

At `capture/native.cpp:235`, every rooted read first verifies the actual retained
root FD against original identity/mode and all original receipt edges. On an
initial explicit retained read, `discover(path)` now runs and its result must
satisfy exact `same_selection` with the incoming token before native content
opening/allocation/read/publication. Shared owner or equal leaf inode alone cannot
satisfy that correspondence. Foreign same-leaf tokens, foreign other-leaf tokens,
same-owner other paths/hardlinks and alternate same-file selection slots refuse.
The incoming original token remains the stored authority after success; the
temporary comparison token never substitutes for it. Actual native leaf-FD
verification still follows discovery and precedes content.

At `native.cpp:212` and `native.cpp:781`, stored original authority governs all
cached/plain/retained/bounded/tree routes. Retained repeats require the same exact
selection; borrowed reads refuse without upgrading or replacing it. Initial tree
capture uses original discovery. `verify()` compares final tree membership before
rereading any file, so a new member is neither admitted nor read. Cached reads,
tree revisits, unread roots, seal and repeated/moved acceptance verification do
not rediscover or issue replacement authority. Standalone and never-bound capture
policies retain their approved behavior.

Operation guards and outer retained-read guards poison the bound root, every
other bound owner/session and incoming file authority before error adaptation,
including discovery, correspondence, content and publication refusal. There is
no unbound retry. Original receipt lifetime covers partial publication and failure;
temporary descriptors use RAII. A newly admitted original metadata leaf can remain
retained after foreign correspondence fails, until its owner dies; this is
intentional terminal retained state, not a descriptor leak.

Temporary comparison issuance and discovery staging retain existing cumulative
metadata charges with no refund. Controlled boundary evidence proves exact repeat
issuance succeeds, one byte less refuses before data, fresh discovery requires its
additional staging, and first explicit reuse works at the selected-file ceiling.
After initial capture with no remaining issuance allowance, cached/plain/retained/
tree/seal/acceptance routes succeed without another discovery entry. Native
content ceilings, admission metadata and file-count bounds remain distinct; this
review makes no process RSS or X06 shared-budget claim.

The narrow `capture_session::add_root` forwarder moves the same root token into
native capture. Existing declaration and text-reference reads use it automatically.
Its imports add no profile/runtime edge, callback provider, role semantics or public
CLI surface. Profile methods obtain authority only through private original admission;
late native poison remains visible through the existing terminal snapshot observer.

## Independent verification

Successful results below have zero failures/errors/skips/disabled cases. Exact
ordinary argv/results are in `commands.json`; copied-production commands/source/
XML are in `attempt1/private/` and `private/`.

| Check | Result |
| --- | --- |
| Focused native/profile/admission/codec/host-path/capture plus C3-B2 Debug | 187/187, 44.75 seconds |
| Inventory | 854; all 851 baseline names preserved, three additions |
| Copied-production private cases | 16 distinct cases pass: 13 replays and three fresh reviewer controls |
| Exact vectors / historical and current source audits | Six / seven pass |
| Pinned formatting/Doxygen | 118 active sources pass |
| Initial, gate-final and post-private ordinary builds | All `ninja: no work to do.` |

The three fresh controls pass 114 assertions. They verify exact pre-cache incoming
authority after admission-wrapper destruction, cache/tree and acceptance moves,
then zero-data refusal after mutation; terminal propagation to all owners in a
populated session after foreign correspondence refusal; and actual native leaf
mode mutation after exact discovery but before content, through both retained
forms. Zero-read/enumeration checks and descriptor counts cover the relevant
refusals, with actual-read positive controls. The original B2-R1 regression passes
through both forms with zero data reads and terminal original/incoming owners.
Replayed allocation and accounting sweeps retain their controlled-fault scope.

The first fresh three-case run failed only the populated-session descriptor
assertion while the original owner remained alive: original discovery had retained
one legitimate new metadata leaf. All authority/zero-data assertions passed.
`attempt1/explanation.txt` records the correction to check descriptor equality
after all owners/session leave scope; the corrected three-case run passes. Raw
failed XML, exact source/argv and complete dependency/cleanup proof are preserved.
Other successful private cases were not unnecessarily rerun. A separate raw-audit
parser initially assumed sorted manifest order matched combined patch order;
its preserved correction uses the recorded patch order and checks exact source-set
membership. Neither correction changes product or authority expectations.

Each isolated run recursively expands response files, disables implicit modules
and module maps, copies all 888 enumerated object/PCM/archive/dylib inputs and
redirects explicit dependencies. Complete shared/copied membership, modes, bytes,
hashes and mtimes plus both shared Ninja journals remain exact. Verified cleanup
removes only reviewer-owned disposable copies/products: 894 files for attempt1
and 892 for the final three-case run. Sources/logs/XML/commands/removal inventories
remain. `otool -L` records runtime install names; copied runtime load selection is
not established or claimed.

## Author evidence, preservation and handoff

The raw audit verifies the author's 187 focused and 13 private passes, 854/851
inventory, original regression-before-fix exit42 with eight failed assertions,
and pre-fix positive controls. Final regression is green. Pre-fix and final copied
input/journal checks and verified cleanup remain intact. The author's first lint
attempt records an empty-output Doxygen SIGBUS; the existing helper's second
attempt succeeds. Independent lint succeeds without that retry. This is an
explicit tooling limitation, not a source or test-result change.

The independent baseline protects 75643 entries, exempting only reviewer ownership
and the exact coordinator C01 round; B2's round/review and complete author/prior
evidence remain protected. Final membership/mode/hash/symlink comparison reports
zero drift. Rewrite-release remains absent and install-release unchanged. Review
artifact/finalization manifests bind this report, exact candidate, results and
protection/cleanup records.

Approval applies while this exact combined candidate remains current. Exclusive
Debug and both reviewer-owned paths are released at handoff. No live build/test/
private process or disposable dependency tree remains; no further reviewer edit
is planned. Latest full approval stays 676/configuration. This approval adds no
full/release/Linux, role-service, public CLI, profile-ledger acceptance or X06 gate.
X06–X08, mandatory external-profile acceptance and execution/RA01 remain open;
public init continues to create format1.
