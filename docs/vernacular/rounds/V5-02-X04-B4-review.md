# V5-02-X04-B4 independent review

Verdict: **approve**. No blocking findings. This closes the integrated **pure
X04-B** verification gate for the profile decoder/effective tree budget, catalog
and effective provenance composition. The mandatory external-profile gate remains
open.

Reviewed the shared `dev/planar-v2-vernacular` checkout at
`3171c49e7b92d3abda01bfbb8bfb0c6b29d4c67a`, CLAUDE.md, the complete B4 brief and
search disposition, implementation/roadmap/v5 direction, exact X-REG/X-LIMIT/
X-STORE contracts, and independent B1/B2/B3 approvals. Reviewer writes are limited
to this report and `tests/native/v5-02-x04-b4-review-evidence/`. Product sources,
tests, author evidence and candidate documentation/support were read-only. No
Planar tools/skills/queue, installed binary, operator data, subagent, commit,
installation, publication or merge was used.

## Candidate and evidence integrity

| Identity | SHA256 |
| --- | --- |
| Author candidate manifest | `d6978ce61bb48d202476f9799ab5bb7fe84cf496e35d09eab8477b87805bc557` |
| Author owned diff | `d52cf977d48a3a5e0a55726c9778d89f7c460deb4a139e562915cb95b4e19706` |
| Author artifact manifest | `b5bedf8c5f6e280afa99b996c162139199e8182ab727016b5bfa946b975a356f` |
| Tested source freeze | `36f4559149fed381c2c092b962a75e6d50b435735ebfdd390eb2309d50b2dd38` |

Independent checks match all **9 candidate paths** to live and final-owned copies,
all **245 author artifacts**, all **143 archived source copies**, **18,310
protected final filesystem entries**, and **726 Debug / 723 RelWithDebInfo build
identities**. The build identities cover executables, object/module files, cache,
compile commands and build graph. Both per-pass manifests and saved build
configuration copies match. The reviewer checks these identities before and after
each independent command and again at finalization.

Recomputed baseline-to-final changes are exactly the nine assigned paths: seven
existing status documents, the new evidence map and one verification support
script. Independently reconstructed the complete owned diff from archived
originals and current bytes. Compared the tested freeze with final filesystem:
only the eight declared Markdown files differ. Product, tests, fixtures, support
and configuration remain unchanged through both author gates and finalization.
The complete manifests exclude only root `.git/`, root `build/` and the assigned
evidence tree; nested historical build paths remain covered. The independent
comparison additionally excludes reviewer-owned output/report. `AGENTS.md` still
points to `CLAUDE.md`. All 295 relative documentation links checked resolve after
URL decoding.

## Gate evidence

Parsed actual testcase elements and exact names, rather than Catch2 assertion
counts. Each fresh inventory contains **520 distinct names**, exactly preserving
the approved B3 inventory. In both configurations the JUnit cases, full CTest
passed-result lines, immediate LastTest names and 520 `Test Passed.` entries
agree. Every JUnit case has status `run`, with **zero failures, errors, skips or
disabled cases**. Cost data retains all 520 names; it is supporting history rather
than an additional execution count.

| Configuration | Full suite | Raw elapsed time | Additional gate |
| --- | --- | --- | --- |
| Debug | 520/520 | 234.18 seconds | vectors, source audits, pinned lint/Doxygen, no-work |
| Release preset, RelWithDebInfo | 520/520 | 115.63 seconds | vectors, source audits, pinned lint/Doxygen, no-work |

The 28 recorded author commands have successful exits, ordered UTC intervals and
exactly one full-suite invocation per configuration. The verifier checks complete
filesystem equality before and after every command, and immediately copies
LastTest/CTestCostData after the full suite. The source archive and freeze bind
the tested candidate. Raw logs confirm all four vector checks, four inherited
runtime/CLI audits, formatting/Doxygen for 113 active sources and final
`ninja: no work to do.` in each configuration. Saved caches identify the pinned
LLVM 23.1.4 toolchain and the distinct Debug/RelWithDebInfo build types.

Independently reproduced current-source hashes, generated hashes, exact forward
substitutions with declared occurrence counts and exact reverse source recovery
for all five real storage/export families in both builds: r12c2, r13b1, r14c3b2,
r14f1 and v5-02-x03-b2-b-c01. These are inherited production storage/interner
variants. No pure callback fixture is represented as collision-safe storage proof.

## Composition and claim boundaries

The 31 existing pure cases consist of four strict ordered profile decoder, four
effective tree budget, eleven catalog and twelve effective provenance cases.
B3 already composes these APIs through preparation and standalone verification,
including exact independent manifest/tree bytes and portable record-v1 digest.
No redundant wrapper test or new product behavior is needed for B4.

The inspected module boundaries remain pure: profile/value decoding, exact
catalog structural sealing and provenance/canonical/tree composition introduce
no runtime/Lua, filesystem, public CLI or acceptance dispatch. The catalog checks
explicit source owner ordinals, mandatory shipped signatures and external static
capabilities. Provenance reconstructs ordered profile modules, old embedded
originals/assembly, catalog owners, normalized modes and complete membership.
The independent vector generator checks twelve established canonical known-byte
and four portable record byte/digest controls before its five effective outputs.
It does not use product serialization as its oracle.

The catalog-only 1 MiB ceiling and combined canonical manifest 1 MiB ceiling are
separate checks; B3 exercises exact combined-limit success and +1 refusal while
the catalog itself still fits. B1 checks the exact outer 32 MiB/8192-file tree
bounds and expanded occurrence charges. Valid complete B3 inputs have at most
4162 files and remain below 32 MiB under component limits. These ceilings do not
prove capacity/RSS or whole-invocation shared accounting.

B2/B3 historical failed fixtures and the B2 diagnostic regression remain preserved
and explained by their independent approvals. B4 does not claim to rerun those
historical failing candidates. Its current full suites now execute all 520 cases
successfully in both configurations. Standalone historical verification proves
structural correspondence, with internally consistent changed bytes producing a
new digest. Later storage must compare its expected recorded digest; historical
callability or semantic honesty is not inferred.

## Independent commands and remaining limits

Reviewer commands, UTC intervals, exits and output are retained under
`tests/native/v5-02-x04-b4-review-evidence/`:

1. Serialized `all planar_tests` builds in Debug and release both report no work.
2. Fresh CTest inventories in both builds match all 520 author names exactly.
3. Canonical, record-v1, request-v2 and effective-provenance `--check` commands
   pass independently, including the established oracle controls.
4. Final candidate, author artifact, full filesystem and both build identity
   checks pass. No source changed during verification.

The reviewer did not duplicate either full suite or the already sufficient
113-source lint run. Two reviewer-only audit assumptions required correction
before any product command: whole-patch reconstruction initially used sorted
manifest order rather than diff order, and a CTest result expression initially
mislocated number padding. The original runners, reproduced errors and the
intermediate padding attempt are preserved with `runner-correction.md`. No
candidate or author evidence was changed, and no product command failed.

This approval is limited to the exact candidate identities above and pure X04-B.
Live external registration/Lua, public profile CLI, effective provenance storage/
profile acceptance, interpretation-v2/history dispatch, shared invocation budgets,
current-policy/callability/semantic honesty, public format2 cutover, X08,
execution/RA01 and Linux remain open. Public init still creates format1. Approval
confers no installation, publication or merge authority. Edits affecting this
approved scope require renewed review.
