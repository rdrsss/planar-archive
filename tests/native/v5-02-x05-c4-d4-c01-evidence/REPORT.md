# X05-C4-D4-C01 evidence correspondence correction

Status: evidence-only correction submitted for fresh independent review of the
combined unchanged D4 product and corrected evidence. D4 remains request-changes
for D4-R1; the independent review found no product defect. This report does not
approve the candidate. Shared checkout `dev/planar-v2-vernacular`, HEAD
`3171c49e7b92d3abda01bfbb8bfb0c6b29d4c67a`, intentional dirty rewrite.

## Superseding correction

This report supersedes the evidence-correspondence claims in the preserved
[original author report](../v5-02-x05-c4-d4-evidence/REPORT.md), specifically
“`preedit-*` preserves actual pre-edit product/reference bytes” and its claim
that the red tests ran against original pre-edit runtime/Lua and failed because
role+projection was still closed. Those exact-preedit, red-source and closed-pair
causal claims are retracted as unsupported. The
[independent D4 review](../../../docs/vernacular/rounds/V5-02-X05-C4-D4-review.md)
and its [raw audit](../v5-02-x05-c4-d4-review-evidence/raw-evidence-audit.json)
identify the discrepancy. The coordinator already recorded the corresponding
[round retraction](../../../docs/vernacular/rounds/V5-02-X05-C4-D4.md#coordinator-disposition-request-changes--d4-r1).

Three author `preedit-*` files (runtime.cpp, render.lua and bootstrap.lua) contain
D4 implementation changes and differ from the contemporaneous original D4
`baseline.json`; the test preedit copy matches that baseline. The original
`candidate.diff` omits substantive D3-to-D4 changes. All those copies, that diff,
red XML, build logs, frozen candidates and manifests remain exactly preserved.
No actor or intent is inferred; the record does not establish how the discrepancy
arose.

The retained [red XML](../v5-02-x05-c4-d4-evidence/red.xml) establishes four executed
cases and two failures, with no errors or skips. Its timestamp is
2026-10-24T10:40:26Z. It does not establish the exact runtime/Lua source bytes
executed or the claimed closed-pair cause. The two failing cases are:

- `C4 D4 role projection uses original role and exact shipped headed base`
- `C4 D4 pair stays lazy and performed role DATA remains valid after project`

The suite's `tests="350"` counts assertions; it is not 350 executed cases.
[red-outcomes.json](red-outcomes.json) binds the original XML and checked counts.
No historical red execution was reconstructed or newly run.

## Subsequently sourced baseline and complete diff

The four files under [subsequently-sourced-d3-baseline](subsequently-sourced-d3-baseline/)
were copied during C01 from the independently approved D3 `candidate-source`
snapshot. They are subsequently sourced comparison inputs, not files captured
at the original red execution. Every copy's exact bytes and mode match both its
D3 frozen-candidate entry and the original D4 baseline entry. The D3 manifest
SHA256 is `d554bccb9b1f2d65418abebf934addb6d55b4f2bf8bb8918bddb00b8ff55efee`.
The complete D3 artifact manifest also verifies unchanged.

[subsequently-sourced-D3-to-D4.diff](subsequently-sourced-D3-to-D4.diff) contains
all changes in runtime.cpp, external_registration.t.cpp, render.lua and
bootstrap.lua, generated from those verified D3 inputs to the original frozen D4
candidate-source. [coverage.json](coverage.json) gives each source path, exact
mode/bytes/SHA256, baseline correspondence, per-file diff hash and current source
identity. Each final frozen D4 product/test file equals the current checkout;
all four remain mode0644 and unchanged. Diff labels expressly identify the
subsequently sourced D3 and frozen D4 sides. This comparison demonstrates complete
source-diff coverage; it supplies no missing historical runtime identity.

Original D4 identities remain:

- candidate20: `875be93d599299846396174e059bdc1f543ad5fb029bb341d23b1ca4648bfed0`
- artifact153: `8c046afeb672623c2e03b0fde3994c1aadad005dc8c29410a3300905f03f8bd0`
- author REPORT: `6e1deed30d1990dd0c7df4f9ef48bc94b2764ecb868756db551d186f7854b482`
- review artifact107: `3249a70cec9cd44c4e3a561aabbd5ef8e15d0ae0e3df50eb02da7fd6a7da981d`

The eight authorized current-status openings now record the independent evidence
finding and pending combined review. They intentionally differ from the eight
historical D4 candidate documentation entries. This is not zero drift of the old
whole candidate: its four product/test files and eight support files remain exact,
and all153 author artifacts and107 review artifacts verify unchanged.
[identities-final.json](identities-final.json) lists the eight expected status-only
changes explicitly. [status-before.json](status-before.json) preserves their
pre-correction identities. Older stage records remain intact.

## Verification and release

Executed evidence verification:
`python3 tests/native/v5-02-x05-c4-d4-c01-evidence/verify.py baseline`, then
`python3 tests/native/v5-02-x05-c4-d4-c01-evidence/verify.py verify`.
The first invocation captured protection before any status edits, then stopped on
a checker path assumption about D3 support files. The checker was scoped to the
four required D3 source files and complete D3 artifact manifest; identity and
status baselines completed before edits without replacing the original protection
capture. [verification-attempt1.json](verification-attempt1.json) records this
checker correction. [verify.py](verify.py) and [verification-results.json](verification-results.json)
retain the final checks and result.

Checks pass for four baseline correspondences, full four-file diff coverage,
frozen/current D4 product identity, original manifests and red counts.
[protected-before.json](protected-before.json) covers76457 entries;
[protection-result.json](protection-result.json) reports zero unauthorized changes.
The exact C01 round is the only round exemption. The D4 round, D3 round and all
prior artifacts are protected; AGENTS.md remains the CLAUDE.md symlink. The eight
owned status documents and new C01 evidence directory are the explicit edit scope.
The capture protects the established source/vendor/build-support/fixtures/docs/
tests and release/install trees; no Debug build ownership was taken.

No product or test code, build files, prior helpers or artifacts changed. No
builds or product tests were run; identity-only verification is appropriate to
this evidence correction. The independent D4 result remains536/536 focused Debug
and16 private cases/947 assertions, with the reviewer-described limits; those are
prior independent results, not C01 reruns. Role+projection is implemented but not
yet independently approved. Latest approved full676/configuration remains
historical. No new full/release/Linux, public profile CLI, profile-ledger
acceptance, X06 shared-budget/RSS or external-profile completion gate is claimed.

[frozen-candidate.json](frozen-candidate.json) binds the corrected status documents,
report, diff, coverage, verifier and unchanged product identity.
[artifact-manifest.json](artifact-manifest.json) binds the new evidence; it excludes
itself and finalization to avoid a hash cycle. [finalization.json](finalization.json)
binds those manifests and the report with exact hashes. Sealing and manifest
self-check use `python3 tests/native/v5-02-x05-c4-d4-c01-evidence/verify.py seal`. Files are released to the
coordinator and fresh independent reviewer; no self-approval, delegation, Planar
operations, commits, installation, publication, merge or operator-data access.
