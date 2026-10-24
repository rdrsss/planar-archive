# V5-02-X05-A4 independent review

Disposition: **approve**. No blocking findings. This closes the integrated
**X05-A registration verification gate** for the independently approved A1/A2
and combined A3 through C05 implementation. It does not close the mandatory
external-profile acceptance gate or implement X05-B/C.

Review used the shared `dev/planar-v2-vernacular` checkout at
`3171c49e7b92d3abda01bfbb8bfb0c6b29d4c67a`, preserving the intentional dirty
rewrite tree and others' work. Writes are confined to this report and
`tests/native/v5-02-x05-a4-review-evidence/`. Candidate product, tests, support,
current documentation, coordinator records and all author/historical evidence
remained read-only. No legacy Planar tool/skill/role/queue, installed binary,
operator data, commit, installation, publication or merge was used. Review held
sole serialized ownership of both build directories after checking for lingering
build processes. This is a fresh review after the earlier reviewer disappeared;
no completed prior A4 review was reused.

The review read the active CLAUDE.md/AGENTS symlink instructions, A4 authorization,
implementation/roadmap/v5 direction, exact external-profile contract and bounded
implementation path, A1/A2 approvals, combined A3–C05 final approval, previous
integrated gate procedure, A4 evidence map and raw author/support records.

## Full gates and independent verification

The [raw gate audit](../../../tests/native/v5-02-x05-a4-review-evidence/raw-gate-audit.json)
independently parses the original inventories, JUnit, LastTest.log, full console
logs and CTestCostData.txt. Debug passes **582/582 in 218.30 seconds**;
release-preset **RelWithDebInfo passes 582/582 in 111.96 seconds**. Both have zero
failures, errors, skips or disabled cases, every JUnit status `run`, 575 unit and
seven acceptance cases. Names are unique and exactly equal across each fresh
inventory, JUnit and immediate LastTest log. All current 582 and previous 520
names survive. Both cost files contain the same 582 names and an empty failed-test
section; their historical run counters are supporting evidence, not another
execution count.

The author's 14 Debug and ten RelWithDebInfo command records each contain one
unfiltered full-suite invocation, with absolute JUnit destinations and zero exit
statuses. The inspected driver copies LastTest and cost data immediately after
CTest returns, before later commands/checks. Both configurations use the same
24,021-entry tested-source freeze, with full membership/byte/mode/symlink equality
required before and after every gate command. Saved caches confirm actual Debug
and RelWithDebInfo configurations. Per-pass manifests and final saved build
configuration copies match their recorded identities.

The four common-input vector generators pass once against unchanged inputs:
12 hand byte, 20 upstream hash and 41 canonical/domain checks; ten record-v1;
two request-v2; and effective provenance's 12 canonical/four portable-record
controls plus five fixed artifacts. These recorded vector commands were inspected,
not claimed as independently rerun.

[Independent commands](../../../tests/native/v5-02-x05-a4-review-evidence/commands.json)
obtain fresh inventories of 582 names in both build directories, rerun all four
B2/D6/D6-C01/D7-B runtime/CLI audits per configuration, run pinned formatting/
Doxygen for **117 active sources** per configuration, and require `all planar_tests`
builds to report `ninja: no work to do.`. All pass. No full suite, focused209 or
private fault runner was unnecessarily repeated during review.

The independent [variant audit](../../../tests/native/v5-02-x05-a4-review-evidence/variant-audit.json)
checks all five current generated storage/export families per configuration:
source/generated SHA256, exact counted substitutions, complete forward equality
and reverse reconstruction. The A0 generated runtime/interface copies likewise
match source hashes and reverse exactly, with exact shipped commands Lua bytes.
The full registered suites execute the actual generated tests; source audits do
not add a new collision or dispatch proof.

The reviewer-only console parser initially rejected CTest's padded `Test   #1`
columns. Its spacing rule was corrected and the unchanged raw records passed;
[retained failure](../../../tests/native/v5-02-x05-a4-review-evidence/parser-spacing-failure.log).
This was a review parser error, not a candidate test failure or suite rerun.

## Scope, documentation and composition

The [scope audit](../../../tests/native/v5-02-x05-a4-review-evidence/scope-audit.json)
reconstructs the complete owned diff exactly from archived originals and current
bytes. Original existing documents match the tested freeze. The candidate consists
of the eight authorized current status documents, new registration evidence map
and new verification helper. The helper was already frozen before both gates.
Only the eight current documents and new map differ after the gates; no product,
test, fixture, golden, support, configuration, vendor or historical evidence bytes
changed. The source archive independently matches **147** frozen first-party
product/test, build/tooling and A4 support entries. All **385** checked relative
file links resolve. AGENTS.md remains a symlink to CLAUDE.md.

Current references correctly reconcile X04-B4 approval, test-only A0+C01 approval
and combined A3–C05 approval, while retaining A4's pending-review status in the
reviewed candidate. Historical reports retain their original contemporaneous
status. The evidence map ties actual registered production cases to captured
identity/order, source-only execution, frozen authority, lexical sealing,
resource stickiness and original diagnostic attribution.

The inspected `command_vm::invoke` profile-owner guard precedes invocation
preparation/provider use; the registered `X05 A3 profile owners refuse invocation
before provider effects` test checks zero service calls. Actual alternative
retained-callable execution, repeated-root sealing, allocation refusal, cleanup
and corrected diagnostic probes remain separately approved Debug evidence from
A1/A2/C05. Neither the candidate nor this review claims they were private
RelWithDebInfo reruns. Conservative cumulative native charge recipes remain
logical accounting, not process RSS measurements.

## Identity, release and remaining work

[Before](../../../tests/native/v5-02-x05-a4-review-evidence/before.json) and
[final release](../../../tests/native/v5-02-x05-a4-review-evidence/final-source.json)
independently verify complete membership and identities for **24,022 source
entries**, **244 author artifacts**, **935 Debug** and **932 RelWithDebInfo build
identities**, with zero mismatches. Reviewer exclusions add only this report and
its evidence root to the author's root-only .git/build and A4-evidence exclusions.
The comparison includes nested historical build paths. Build identities bind
executables, objects/modules, libraries, cache, compile commands and build graph.

| Author identity | SHA256 |
| --- | --- |
| Tested source freeze | `2b25473281360b2527c52d027dedcc52daac633948b6953543e7b48cb08b8608` |
| Final filesystem | `070c6eefb5f036dda7278b25174bf5bb3e78cf913082cfd250fe8b37979aa76b` |
| Candidate manifest | `4dc82171c97f66d764f97fcc1145ccba81e8ea8decc8a15f969614d2e90b820e` |
| Owned diff | `81d9cd1ed957f05303cf7f50e056259127ee39c40d3029c9595f7a300028eb22` |
| Artifact manifest | `ca79b698e7b8a2a6023f11166078289a6653abfe6a3ff11857525b0d9d6e6e52` |
| Author report | `1a8e90be9594c54395cc89c472336a9284c07e54625028649089725c9e51d57f` |

Review [finalization](../../../tests/native/v5-02-x05-a4-review-evidence/finalization.json)
binds this report and its own artifact manifest. Candidate and author evidence
remain current at final release. Both build directories are released after final
hashing; no further reviewer edits follow release.

These gates ran on macOS ARM64. X05-B/C actual shape/projection semantics and
builtin adapters, X06 recorded history/interpretation/storage acceptance and
receipt/whole-invocation orchestration, X07 atomic public admission/cutover,
X08 mandatory external-profile acceptance, execution/RA01 and Linux remain open.
Public init still creates format1. Coordinator advancement requires the usual
current-candidate identity check against this approved snapshot.
