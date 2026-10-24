# V5-02-X03-B2-B-C01 independent review

Decision: **approve**. C01 closes B2B-R1. The combined B2-B native acceptance
candidate is approved to proceed to the subsequent integrated verification round.
No remaining blocking finding was identified. This review does not complete
X03-B, X06, public format2 admission or the mandatory external-profile gate.

Reviewed on the shared `<repo>` checkout,
`dev/planar-v2-vernacular`, HEAD `3171c49e7b92d3abda01bfbb8bfb0c6b29d4c67a`.
Reviewer ownership was limited to this report and
`tests/native/v5-02-x03-b2-b-c01-review-evidence/`. Product, tests, author evidence,
prior review and status documents were read-only. No subagents, Planar tools,
skills, queue, installed binary, operator data, commit, installation, publication
or merge were used. Debug build/test commands were serialized; coordinator
confirmed source/status/round stability throughout review.

## R1 correction and combined candidate

Read the active guide, implementation plan, roadmap, v5 direction, C01 round,
original B2-B review, X-STORE, bounded implementation path, author handoff,
exact baseline diff, support code and raw verification evidence. X-STORE requires
the selected identity/format recheck inside the transaction.

The sole product delta is `src/engine/ledger/format.cpp`. Its existing private
`same_metadata` helper now appears before its callers at line 19 and still compares
all four `ledger_metadata` fields: identity, workspace, creation time and format.
The three incomplete predicates now reuse it: locked `begin_records` at line 1371,
`record_writer::commit` at line 1529, and acceptance precommit at line 1732.
An independent textual reconstruction removes the unchanged helper from old/new
source, substitutes exactly those three predicates, and obtains identical bytes.
There is no other product delta. Existing refusal codes/messages, sidecar and file
identity checks, verification order, transaction ownership and commit logic remain
unchanged. The already complete writable-open and recovery checks retain their
behavior.

A mismatch at locked entry destroys the local transaction before returning a
writer. Writer precommit moves ownership out of `_state` before checking metadata,
so refusal both rolls back tentative records and invalidates the writer. Acceptance
precommit refuses while only its local RAII transaction owner exists, before
candidate receipt recheck, saved-result publication or commit phase changes.
The tests confirm the existing `unsafe_path` messages, complete rollback and
receipt behavior for both supported formats.

The inherited B2-B review identified only R1. The current C01 baseline and full
filesystem audits preserve that exact candidate and its evidence; this review
also reexamined immutable request-v2 construction, typed full-byte codec-aware
interning, request privacy and the acceptance/replay ordering. Complete locked
metadata equality now binds fresh request admission to the selected format.
Expected ledger identity still precedes saved request replay; exact saved body
replay/conflict precedes fresh format/profile refusal, CAS and prepared-candidate
access. Receipt/recovery authority, codec collision refusal and format1 behavior
retain the previously reviewed implementation. Public init remains format1;
fresh format2 requires request-v2 embedded intent; profile acceptance and profile
accepted history remain unsupported pending X06.

## Regression quality

The CMake addition is verification-only: the fixture/module are excluded from
default `all`, and their test executable is attached to `planar_tests` by the
existing helper. No public test hook or new product equality API was introduced.

The reversible variant has seven unique implementation substitutions and two
unique interface substitutions. Three implementation insertions supply the
otherwise identical alternate supported format immediately after real SQL
verification at the selected boundaries. The other substitutions isolate module,
namespace and fixture imports. Production comparisons, SQL verification,
transaction and commit code are retained. Independent forward/reverse replay
matches both generated files exactly for the frozen red and current green source;
all five current storage/export variant families reverse exactly as well.

The three test cases each execute formats1/2 with unchanged controls and supplied
format differences. Controls actually commit. Refusal checks compare all seven
table counts, original database bytes, all metadata, initial head and absence of
the request. Writer precommit tests tentative interning plus subsequent commit and
intern refusal. Acceptance precommit checks `not_completed` with empty saved bytes,
then same-key fresh retry and exact saved replay with authoritative receipt phases.

The red snapshot is exact pre-fix product source; its test, fixture, generator and
CMake bytes equal green. Preserved snapshot/manifest timestamps precede red
execution. Raw red output contains exactly six missing-refusal assertions, one
per boundary/format, with unchanged controls passing; its three CTest cases fail
with zero errors/skips. Green has the same tests passing. These checks establish
complete metadata comparison and rollback plumbing under controlled verified
metadata. They do not establish an external format-change reproduction, outside
writer interaction, filesystem timing or format conversion, and none is claimed.

## Exact candidate and preservation

Author names below resolve under `tests/native/v5-02-x03-b2-b-c01-evidence/`.

| Frozen identity | SHA256 |
| --- | --- |
| `candidate-manifest.json` | `6497c9c720369653b60f38b180dae32465ab7468ccdab3685ac10827522490a6` |
| `final-source-manifest.json` | `717ddb977f5c07563c674d6397d6023bf66b7a65caf6c4be0fe0b80866b2747f` |
| `available-baseline.diff` | `0277ce09ca89d41206998615e752d13950c070b64034e510417c65f6580b7f04` |
| `artifact-manifest.json` | `3f30f8f69b3c3782dd739bacd064ab620b92b2990c7fba803c1795c84a121d40` |

Initial, final and handoff audits verify all 12 candidate members against working
files and exact snapshots, all 90 C01 author artifacts, and all 119 inherited
B2-B author artifacts against their own manifest. The full frozen final tree
matches all 15,288 files and two separately recorded symlinks. Exclusions apply
only at root `.git/` and `build/`, plus the C01 author/review evidence and this
report; nested historical build files and old candidate/review/evidence remain
covered. AGENTS.md remains a symlink to CLAUDE.md.

The 15,285-file pre-edit baseline has no removed file. Exactly nine existing files
changed and three support files were added, matching the candidate map. Every
changed existing file has a matching exact archived baseline copy. Independent
unified-diff reconstruction reproduces `available-baseline.diff` byte for byte.
The status-only additions retain pending integrated verification and product limits.

## Independent verification

Review evidence records exact argv, cwd, timestamps and exit status, raw logs,
JUnit and copied LastTest before inventory.

| Check | Independent result |
| --- | --- |
| Debug `all planar_tests` build | Pass; no work after normal Ninja glob recheck |
| Focused CTest `ledger\|history\|export\|record\|request\|prepared\|acceptance\|publication\|atomic\|metadata continuity` | 209/209 pass, zero failures/errors/skips; 81.85 s |
| Record-v1 / request-v2 vector oracles | 10 + 2 verified |
| Pinned active formatting / Doxygen | Pass, 98 active sources |
| Pinned support C++ formatting | Pass, five files |
| CTest inventory | 430 names; every prior 427 retained, three metadata cases added |
| Final debug `all planar_tests` build | No work |
| Exact source/artifact/diff/variant audits | Pass |

Raw independent focused LastTest establishes the full inherited fault matrix for
each format; this is not inferred from unchanged CTest names:

| Format | Journal SQLITE_FULL | Database SQLITE_FULL | Unique process boundaries |
| --- | --- | --- | --- |
| 1 | 38 faults, ordinals 0–37; success 38 | 15 faults, ordinals 0–14; success 15 | 0,1,2,3,4 |
| 2 | 38 faults, ordinals 0–37; success 38 | 15 faults, ordinals 0–14; success 15 | 0,1,2,3,4 |

All faulted ordinals report positive hits; retained assertions require
`storage_full`, initial history/row counts, same-key retry and exact replay.
Boundaries0/1 preserve main/non-hot sidecars and refuse ordinary open, explicit
recovery and retry; 2/3 use explicit hot-journal recovery then retry; 4 replays the
committed result. Repeated fork output is counted by unique boundaries.

Two initial reviewer audit attempts failed because the script assumed map-shaped
artifact entries and unique counts for inherited multi-substitution variants.
Both scripts/logs are retained. The corrected audit honors exact recorded counts
and enforces uniqueness for C01. No product/test change resulted. The expected
author red failure is preserved; all independent product verification passed.

Review `artifact-manifest.json` freezes 42 other review artifacts and excludes
itself; SHA256 is
`f8d8ab0586ab220c6813bc444bae75fb9e918428278c06c0b79c18e72bde8dc3`.
Reports/manifests are finalized before ownership release.

This remains macOS ARM64 focused debug verification. Full integrated debug/release
verification is the next required round. Linux, hardware power loss, public
format2 init/load admission, profile acceptance and effective interpretation/
provenance remain unestablished. Approval closes R1 and permits that next round;
it does not close the full X03-B or external-profile gate.
