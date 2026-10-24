# V5-02-X04-B1 independent review

Disposition: **approve** for the pure profile decoder extraction and distinct
effective-provenance tree budget. No blocking findings. This closes B1 only;
catalog validation, effective provenance assembly and the mandatory external-profile
gate remain open.

Reviewed in the shared `dev/planar-v2-vernacular` checkout at HEAD
`3171c49e7b92d3abda01bfbb8bfb0c6b29d4c67a`, against CLAUDE.md, the B1 brief,
parent X04-B search disposition, implementation/roadmap/v5 direction, X-CAP,
X-LIMIT, X-STORE and the independent A2/A3 approvals. Reviewer ownership is only
this report and `tests/native/v5-02-x04-b1-review-evidence/`. Product, tests,
author evidence and current-status documents were read-only. No Planar tool,
skill, queue, installed binary, operator data, subagent, commit, installation,
publication or merge was used.

## Candidate and evidence integrity

| Identity | SHA256 |
| --- | --- |
| Author candidate manifest | `efcb368aeb321450de2686ca18daad87074c044f694bbae9a2a5b0bb7f7aa4ab` |
| Author owned diff | `2998f8318d3e0234752cada944342ac90271999d1eda98b2ff756c968b0123a5` |
| Author artifact manifest | `2b73a7c1a1d85ec303d63bfdd7afdb2a4a80e99ca6bac0cfcb741a4d13b1776f` |

Independent checks match all **20 candidate paths** to live and final-owned
copies, all **368 author artifacts**, both attempt manifests (**163** and **150**
entries), **138/139** archived source copies, all **16,285 protected final
filesystem entries**, and **716 debug build identities**. The build identities
include executables, objects, module files, cache, compile commands and build
graph. These checks run before and after every independent product command and
again at reviewer finalization.

The recomputed baseline-to-final change list equals the author's **19 assigned
changes**. `protect.py` was already in the baseline; it is also bound by the
20-path candidate manifest. Root-only `.git/` and `build/` exclusions preserve
nested historical paths. AGENTS.md remains the CLAUDE.md symlink. Product and
tests are unchanged between the passing attempt01 freeze and final attempt02;
only seven owned current-status Markdown files and final-gate support were
added/changed between those gates. Later finalization support is separately
bound by the final candidate and artifact manifests.

## Boundary assessment

The new `capture/profile_codec.cppm:3` exports the ordered memory-only
`decode_profile` API and existing `profile_error`; `capture/profile.cppm`
re-exports that type through the codec. Independent extraction checks establish
byte identity for the entire Glaze adapter section, private error/path helpers,
error type, and decoder body after its sole symbol rename. Imports are only
`std` and `planar.engine.value`, with the existing Glaze dependency now owned by
the pure codec target. No host/native capture, filesystem or runtime dependency
enters that target.

The unchanged decoder retains the 65536-byte input cap, strict UTF-8/JSON
validation, decoded duplicate-key checks, arbitrary-sized integer version
classification, fixed decoding scratch, pre-growth node/array checks, complete
path and case-alias policy, ordered results and logical diagnostics. The capture
call at `capture/profile.cpp:64` remains after bounded profile read and before
module root setup, all module selections and all module reads. Existing native
receipt/admission/capture code and capture tests are unchanged.

The only algorithm changes in `history/tree_core.cpp` are the selected file
budget and additional profile values. `budgets` at line 12 preserves source
64 MiB/4096 files/8192 directories and original provenance
16 MiB/4096 files/1 + 4096 * 63 directories. Effective provenance is separately
32 MiB/8192 files/1 + 8192 * 63 directories. Existing enum values stay stable;
unknown values still refuse through the unchanged invalid-profile path.
Construction at line 116 and reconstruction at line 88 both use the selected
file limit. Construction completes structural validation before any interner
call. Reconstruction counts every expanded occurrence before adding traversal
entries and accumulates each expanded payload's bytes. Shared record IDs do not
reduce these charges.

The 16 MiB per-file, depth64, path1024, segment128 and normalized-mode limits
remain unchanged, as do topology checks and exact record/canonical hashes.
Reconstruction retains its documented requirement for codec-validated records;
per-record payload/mode/segment invariants belong to that existing boundary.
No codec, ledger integration or storage format changes are introduced.

The effective directory allowance follows directly from at most8192 files and
at most63 non-root ancestors per file, plus root. Shared ancestors only lower
the count. A separate directory-overflow fixture satisfying the other valid-tree
bounds is unreachable. Existing `history/tree.t.cpp:338` covers source
directories8192/8193 and `:398` covers the induced provenance maximum with shared
expanded occurrences; both execute in the author's focused run. This satisfies
the brief without a redundant large effective-directory fixture.

## Verification

Author raw inventory, JUnit and LastTest agree on **177 distinct focused cases**,
all passing with zero failures/errors/skips. The 497-name inventory preserves
all489 independently approved A3 names and adds exactly four direct decoder and
four effective-tree cases. The new tests cover ordered in-memory decoding,
schema/version/path/logical-field/resource refusals, files8192/8193, bytes32 MiB/+1,
old source/provenance byte/file boundaries, build/digest/read parity, lower
path/depth/per-file limits, shared expansions and zero interner calls on refused
inputs. Author command timing records are serialized and successful.

The author also retains passing canonical (12 hand-byte,20 upstream-hash,
41 domain),10 record-v1 and2 request-v2 vectors, four inherited runtime/CLI
audits,107-source pinned formatting/Doxygen and no-work builds. Attempt02's
documentation-only final gate preserves the product/tests and inventory.
Independently recomputed forward and reverse substitutions for all five
generated storage/export families match their source/generated hashes and bytes.

Independent serialized commands, UTC times, exits and raw output are retained
under `tests/native/v5-02-x04-b1-review-evidence/`:

1. `cmake --build build/rewrite-debug --target all planar_tests` reports
   `ninja: no work to do.`.
2. A fresh `ctest --show-only=json-v1` inventory exactly matches all497 names.
3. `ctest -R '^replacement\.(profile codec|tree core effective provenance)'`
   passes **8/8**, all eight newly added cases, in1.10 seconds with zero
   failures/errors/skips. JUnit and immediate LastTest/CTestCostData are saved.
4. `scripts/rewrite-check.py lint --build-dir build/rewrite-debug` independently
   passes pinned formatting/Doxygen for107 active sources.
5. The final serialized `all planar_tests` build again reports no work.

Three reviewer-only pre-product audit assumptions needed correction: an artifact
count was accidentally appended to a digest, finalization support was incorrectly
expected in the earlier tested freeze, and the module-selection audit initially
used the wrong existing symbol name. The initial runners/probe and reproduced
stderr are retained with `audit-corrections.md`. All occurred before any product
command. No candidate/product failure was hidden or repaired; the corrected
independent verification passes.

## Limits carried forward

This is focused macOS ARM64 Debug evidence. No full497-case suite, release or
Linux run was required or performed here. The independently approved A3 full489
Debug and RelWithDebInfo gates remain the latest full milestone evidence.

B2 catalog-v2 structural validation, B3 effective provenance assembly/verification,
live registration and Lua execution, public profile CLI, interpretation-v2/profile
acceptance and historical integration, whole-invocation shared budgets and the
mandatory external-profile gate remain open. Public init still creates format1.
Execution/RA01 and Linux acceptance remain future work. This approval authorizes
no installation, publication or merge.
