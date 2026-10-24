# V5-02-X05-C3-B1 independent review

Disposition: **approve**. No required changes or unresolved product choices were found in the bounded metadata-only physical-parent traversal candidate. This approves C3-B1 only. Runtime orchestration, handler/provider admission and read services remain later work.

The shared checkout is `dev/planar-v2-vernacular`, HEAD `3171c49e7b92d3abda01bfbb8bfb0c6b29d4c67a`. I owned only this report, `tests/native/v5-02-x05-c3-b1-review-evidence/`, and exclusive serialized Debug verification. Product, author evidence/support, historical records and release were read-only. I did not revert others, use Planar tools/skills/roles/queues, delegate, commit, install, publish, merge or access operator data.

## Scope and exact identity

I reviewed the active guide, implementation plan, roadmap, v5 direction, [C3-B1 authorization](V5-02-X05-C3-B.md), X-CAP/P2 A, and the existing host-path preparation and native before-read binding contracts. Only `capture/admission.cppm`, `admission.cpp`, `admission.t.cpp`, `profile.t.cpp`, architecture/testing prose and bounded round support are in the implementation diff. No runtime wrapper or service/CLI change is present.

The following author files identify the exact reviewed candidate. Every candidate mode/hash, candidate-source copy, actual-preedit diff and all author artifact modes/sizes/hashes were checked independently before verification and afterward:

| Artifact under `tests/native/v5-02-x05-c3-b1-evidence/` | SHA256 |
| --- | --- |
| `frozen-candidate.json`, 225 entries | `b4bbbe38d6d6d20bb361ca895ac2bee7771da46899647217d58acde0f60e85b6` |
| `candidate.diff` | `1c7a9583b785f8502574f7d1438ea17124a1778322b3037310a30111805669d4` |
| `artifact-manifest.json`, 46 entries | `8c289dc4e13750138be2c0f9d82e574bf36a5a040e852b0fd9cde07460fce10e` |

[identities-after-private.json](../../../tests/native/v5-02-x05-c3-b1-review-evidence/identities-after-private.json) records the final comparisons. Pending-review wording in the frozen author prose is left untouched; this report supplies the independent disposition.

## Boundary assessment

- [admission.cpp:139](../../../src/engine/capture/admission.cpp) walks original absolute spelling component by component. It opens and checks every non-dot intermediate directory before a later parent step; no canonicalization cancels a symlink, missing component or regular file. Existing no-follow metadata flags, regular-file checks and redacted diagnostics remain. A final dot or parent cannot turn a file selector into a directory; repeated/trailing separators preserve directory requirements.
- [admission.cpp:199](../../../src/engine/capture/admission.cpp) retains the actual upward descriptor/name edge, compares its identity/mode with the expected active ancestor, then pops only the active final ancestry. Filesystem-root parents compare against the retained root and leave that chain intact. Retained visited branches therefore do not fabricate final nesting.
- [admission.cpp:220](../../../src/engine/capture/admission.cpp) applies both containment directions to the final chain. Observation/declaration exclusion, registry colocation, trusted selected-file aliases, target overlap and absence policy remain unchanged. [The receipt check](../../../src/engine/capture/admission.cpp:247) still verifies every retained descriptor and name edge, including exited directories and upward edges; replacement/movement poisons the owner.
- New root-chain storage charges precede insertion. Every parent component uses the existing prospective temporary/receipt check and cumulative charge before copying/opening, and receipt retention charges before allocation. Parent popping allocates no chain element. Existing 64 MiB/100000-receipt limits remain; no new quota or reset is introduced. The accounting document describes this inherited conservative metric rather than claiming process RSS measurement.
- Metadata admission still contains no content reads. `verify_file` checks the original complete receipts before comparing the impending read descriptor. Existing positive-control read interposition cases pass. Independent binding and mutation-before-read probes also pass without changing the native read service.
- [profile.t.cpp:631](../../../src/engine/capture/profile.t.cpp) exercises late parent-spelled admissions on the original captured owner: original immutable bytes survive later profile mutation, while verification refuses. Its following case retains exited late-slot ancestors and sticky poisoning. Existing moved/failed-owner checks pass. No recapture replaces the original receipts.

## Raw evidence and independent checks

Author raw evidence supports the claimed 113/113 focused Debug gate, zero failures/errors/skips/disabled, inventory723 preserving716 plus seven new names, pinned formatting/Doxygen over118 sources, and no-work builds. The real one-case red regression was built against the pre-fix product and failed at ordinary `trusted/../workspace` admission. The report explicitly excludes the stale exploratory run made before an asynchronous rebuild finished; I do not count it as candidate verification.

The raw 717-name capture is correctly described as post-red-build. Independently comparing names establishes exact equality of its derived716 baseline with the immutable C3-A inventory, and exactly the one new red-regression name in717. This is evidence of preservation, not a claim that716 was captured before editing.

| Independent check | Result |
| --- | --- |
| `cmake --build build/rewrite-debug --target all planar_tests` | Pass; no work |
| Current CTest JSON inventory | 723 exact names; all716 prior names preserved, seven additions |
| Complete admission/profile/profile_codec/host_path/native/capture binary selection | 113/113 cases; zero failures/errors/skips/disabled |
| Pinned rewrite lint | Formatting/Doxygen pass over118 active sources |
| Final ordinary build | Pass; no work |
| Four new private adversarial cases | 4/4 cases, 68 assertions; zero failures/errors/skips |
| All shared/copied compiled dependencies | 889 objects/PCMs/archives/dylibs, 1,233,475,924 bytes; exact membership/modes/hashes unchanged |

The private [fixture](../../../tests/native/v5-02-x05-c3-b1-review-evidence/adversarial.cpp) covers repeated exits through distinct branches with live declaration descriptor binding; mutation of an exited branch before native content capture; trusted-root parent spelling and both final containment directions; missing/dot/parent/file intermediate target typing and absent-target symlink appearance. [focused.xml](../../../tests/native/v5-02-x05-c3-b1-review-evidence/focused.xml) and [adversarial.xml](../../../tests/native/v5-02-x05-c3-b1-review-evidence/adversarial.xml) retain the actual results. Catch2's private JUnit suite `tests=68` counts assertions; there are exactly four testcase elements.

Private compilation copied **all** shared objects, PCMs, archives and dylibs before compiling, expanded the response/module mappings, redirected object/executable paths and all explicit module inputs to owned copies, and disabled implicit modules/maps. Copied and shared membership/modes/hashes were checked after execution. Commands and dependency manifests are retained in the review evidence. Disposable copied dependencies and the private object/executable were removed after successful verification; source, commands, manifests and results remain reproducible.

Two initial reviewer harness failures are preserved: an unused module-cache option was rejected under `-Werror`, and a fixture used `input.lua` instead of the documented standalone logical name `input.json`. Only the review harness changed. Its corrected four-case run passes; neither failure identifies a product defect. The first private runner also overwrote the common focused command journal; `focused-command-record.json` explicitly reconstructs those argv from the executed runner. Raw focused stdout/XML were unaffected. Subsequent private commands use a separate journal.

## Preservation and limits

All62,673 author-protected historical/source entries and all4,238 release entries match their recorded complete membership/modes/bytes/symlink identities before and after review. The candidate and46 author artifacts match separately. Coordinator-owned round files and this review's prefix are excluded from the author's historical snapshot. `AGENTS.md` remains the `CLAUDE.md` symlink.

Approval applies while the exact reviewed candidate remains current. No full suite, release run, Linux execution, runtime host-slot orchestration, public profile admission, handler/provider/read-service ordering or complete external-profile acceptance is inferred. X06–X08, public cutover, execution/RA01 and the mandatory external-profile gate remain open; public init creates format1.

Exclusive Debug and all review-owned edits are released at final handoff. No build/test/private process remains, and no later edits are planned. `review-manifest.json` binds this report and review evidence; `review-final.json` binds that manifest and final release.
