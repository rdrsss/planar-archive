# R14-D8 independent acceptance-closure review

Disposition: **approve**. No blocking findings or requested changes. The reviewed
candidate closes the applicable R14 acceptance portions under the existing exact
contracts and the recovery limits below. Mandatory external-profile V5-02-X
remains next; this is not full execution, RA01, Linux or universal crash-recovery
acceptance.

Reviewed the shared `dev/planar-v2-vernacular` checkout at HEAD
`3171c49e7b92d3abda01bfbb8bfb0c6b29d4c67a`, against the rewrite instructions,
implementation/roadmap/v5 direction, exact load amendment, ledger failure contract,
R12-C recovery preconditions, D8 brief and boundary disposition, D7-B approval,
acceptance evidence table and raw author evidence. Writes were confined to this
report and `tests/native/r14-d8-review-evidence/`. No implementation, other work,
prior evidence or goldens were changed. No Planar tools/skills/queue, installed
binary, operator data, commit, installation, publication or merge was used.

## Exact candidate and preservation

The 12-file repository-relative `candidate.json` SHA256 is
`439abd02ca0ee1c7532e8a7b9a43982dde3762fdb10663031106fcd17b6bd0e8`;
`owned.diff` is
`92810148c8b3ad6b6f3db15dcabb2d79705f973b9cc9782a637cfce76aaf8a61`.
All current inputs matched their frozen snapshots. Independently reconstructed
the diff byte for byte against eight baseline copies, each matching the
contemporaneous full baseline hash inventory. Source copies were captured before
edits; five documentation copies were reconstructed after append-only edits and
verified against those contemporaneous hashes. These are distinct provenance
claims, both sufficient for the exact diff reviewed.

Independently checked all 6,806 baseline file hashes and the current nonignored
file inventory. Only the eight allowed preexisting paths changed, including the
coordinator-owned round record; additions are assigned candidate/evidence and
review outputs. Product C++/Lua, schemas/codecs, W recipes, previous evidence,
fixture goldens and generators remain unchanged. `AGENTS.md` remains the symlink
`CLAUDE.md`. The load contract body beginning at `Fixed catalog admission and
typed intent` remains byte-identical to the D3-approved snapshot, SHA256
`9360af4d6d38983f36e01e0019fd723c7ac01b0a23f6e8d3db1a5cfb4eea2634`.

The ten compiled-output/cache hashes in repository-relative `artifacts.json`
matched, manifest SHA256
`fc1abc7e69b01e45d323b0f1e4b9cdad2ab80f99b2e9ac30f2443276087ba9d3`.
The 97 evidence-relative artifacts in `evidence-sha256.json` matched, manifest
SHA256 `4a42ce5ddb9a0a5640cc236ef87f47f3d725a041febb035dff961becee0c8011`.
These manifests have different bases and purposes. `review.py` and
`identity-audit.json` preserve the independent checks; final recheck still matched.

## Coverage and contract assessment

`tests/acceptance/bootstrap_load.py:46` exercises the public six-stage trajectory.
Exact accepted/noop results have revisions 1,1,2,3,4,5, parents none,none,1,2,3,4
and events 2–7; every stdout equals the native stored request bytes plus LF.
Comment and prose changes retain W, semantic change moves W, and exact restore
returns original W/render/source identity while allocating a fresh chronological
revision. Registry and stdlib references remain stable. Prose changes rendering;
no unsupported equality between all authored-byte edits and render bytes is imposed.

At line 80, original revision/event/request data are retained, then the complete
workspace/registry/source fixture is removed. Both public and changed-current-load-
policy binaries reproduce those historical data, replay the original saved load
response at an advanced head, and export exactly the original declared file set,
bytes and executable/nonexecutable modes. The unrelated sibling is excluded.
This composes the earlier independent native/read/export and exact-source variant
proofs; it does not execute recorded policy or depend on current source trees.

The new native disk-full case at
`tests/native/r14-d8-support/acceptance_fault.t.cpp:363` drives real
`complete_acceptance`, not a symbolic storage transaction. Its bounded enumeration
hits every selected actual journal/main-database write, then reaches an unfaulted
successful endpoint. Both reviewed full runs and both independent runs show
**38 journal and 15 database SQLITE_FULL injections**. Each failed acceptance
verifies no request/revision/event/head prefix and unchanged counts in all seven
tables, then succeeds with the same key and replays exact saved bytes without
duplicates. The VFS is scoped/restored; inputs, ledgers and children are temporary.
The only build delta adds this source to the existing test-only executable at
`src/engine/CMakeLists.txt:357`; there is no runtime hook or new default-all target.

The five process-death boundaries at line 399 are distinct safety outcomes:

- Before first journal write and first journal sync, the main database retains
  its exact original bytes. Ordinary open and same-key retry refuse
  `recovery_required`; explicit `recover_quiescent` refuses `corrupt` with
  `Recovery requires a valid hot journal header`. Every file's bytes remain
  unchanged through each refusal. No sidecar deletion or raw-SQLite opening of
  the original is used to manufacture recovery.
- Before first database write and first database sync, recognized hot journals
  recover explicitly to the old whole acceptance. Same-key retry accepts once;
  another retry replays without duplicate records in the checked history tables.
- After returned completion, the committed whole state remains readable and
  retry replays exact native saved bytes.

The two early refusals conform to ledger-contract initialization/failure safety
(lines 494–499) and R12-C's explicit quiescent recognized-recovery preconditions
(lines 43–49). Those approved rules authorize conservative diagnosis, not repair
of arbitrary remnants. D8 preserves that boundary and changes no recovery
semantics. **No supported automated recovery/retry is established for the early
non-hot remnants.** Native process/VFS tests do not establish hardware power-loss
survival or public CLI disk-full/crash injection.

Reused proofs remain appropriate: C3-B2's all-four-kind targeted collision case
calls complete acceptance and compares unchanged database bytes/head/history;
existing acceptance tests cover mutation prefixes, commit-hook uncertainty,
busy-COMMIT rollback, final capture refusal, late request replay and competing
acceptance. The retained public suite covers all-job/capture rejection, intent
conflict, concurrent equality/CAS, postcompletion failure, lost response and
stdout recovery. D8 adds the missing real acceptance faults and public history
trajectory without restating those lower-layer proofs as public VFS coverage.

The evidence table accurately scopes A01–A08, A20, A23, A25–A29, A31 and the raw
A36 diagnostic boundary to R14. Existing render/schema/materiality/sandbox/byte
contracts and all historical goldens remain protected. Accepted-role/run state,
packets/cache reconstruction, execution reconciliation/checks/verdicts, authoring
prompts and external profiles retain their future owners. The table closes no
whole execution row merely because its storage portion is proven.

## Independent verification and full-gate adequacy

Both build directories were exclusively assigned during this review. Commands
were direct and sequential; argv, UTC times and exit codes are in `commands.json`.
Before and after checks, both `cmake --build build/rewrite-{debug,release}
--target all planar_tests` builds reported `ninja: no work to do.`.

The focused CTest expression `D8 complete|replacement.load_cli$` passed
**3/3 debug** in 10.90 seconds and **3/3 release** in 4.72 seconds, zero
failures/skips. Each public suite reports 163 checked invocations. Parsed raw
LastTest confirms all 53 fault positions and all five process outcomes above.
JUnit and LastTest were archived before inventory; both current inventories
match the frozen 402-name lists exactly.

The reviewer additionally ran `active-draft.py` against both final public binaries.
For each of invalid unselected job, missing reference and wrong constructor,
first accepted a valid revision, then attempted the invalid draft with a fresh
key. All seven table counts, main database bytes, current history, revision 1,
event 2 and original request history remained unchanged. Restoring valid inputs
reused the failed key successfully as a noop at revision 1/event 3. All six
scenario runs passed. An initial reviewer harness attempt used macOS's unresolved
`/var` temporary-parent spelling and received the expected path refusal; the
resolved-root harness and successful reruns are retained separately, not counted
as a product failure or passing test.

Independently parsed the author's final full JUnit and LastTest: **402/402 debug
and 402/402 release**, zero failures/skips, preserving all 400 D7-B names and
adding exactly the two native D8 cases. Inspected final lint output (95 active
files plus explicit new test-source formatting), 12/20/41 canonical/hash vectors,
ten independent record vectors and successful exact-source audits for all seven
historical families/fixed export, six D6 variants, C01 and D7 originals/separator/
assembly. Protected source/golden hashes and no-work current builds support
using those full gates; another full run adds no necessary coverage here.

The transient two logging-string substitutions during the debug full gate were
restored byte-identically; the intermediate hash is reconstructed, not a
contemporaneous snapshot. The already-built full-run binary and later rebuilt
object distinction is explicitly recorded. The first final no-work attempt
rebuilt the test object; the next genuinely did no work. This is a provenance
limitation, not an unreported full behavioral rerun. The independent debug run
above executes the final rebuilt native tests and final public workflow, removing
that ambiguity for the affected scope; unchanged product inputs and the complete
release gate need no broader repeat. Failed/zero-match exploratory attempts and
the original failed universal-recovery expectation remain retained.

## Limits and handoff

Verification is macOS ARM64 / pinned LLVM 23.1.4 in debug and RelWithDebInfo.
Linux, hardware power loss, universal recovery, public CLI VFS fault injection,
external-profile X, RA01 and R15–R19 execution remain open. Approval closes only
the applicable R14 matrix under the unchanged exact contracts and explicit
non-hot-journal limit. It does not authorize installation, publication or merge.

Debug and release were released idle after the final checks. Approval applies
while this exact candidate remains current. Review evidence has an explicit
review-directory-relative manifest; `handoff-identities.json` identifies the
report and evidence manifest with repository-relative paths.
