# X05-A4 integrated registration verification candidate

Status: pending fresh independent review. This verification-only round adds no
product code, test cases, fixtures or generated goldens. A1/A2 and combined
A3+C01–C05 are independently approved; the C05 review closes R1–R7. The
integrated gate does not close the mandatory external-profile acceptance gate.

The shared `dev/planar-v2-vernacular` checkout starts at
`3171c49e7b92d3abda01bfbb8bfb0c6b29d4c67a`, preserving the intentional dirty
rewrite tree. Commands run directly and serialize both build directories. No
Planar tool, queue, installed binary, operator data, commit, installation,
publication or merge is used. `AGENTS.md` remains the CLAUDE.md symlink.

The full filesystem freeze excludes only root `.git/`, root `build/`, and the new
A4 evidence directory. Existing nested historical build paths remain included.
Before and after every gate command, complete membership, SHA256, mode and
symlink equality is required. Both configurations test identical product/test/
configuration/support bytes. Current documentation is finalized only afterward;
post-gate stability restricts those changes to the authorized current references.

Observed results: **Debug 582/582 in 218.30 seconds** and **RelWithDebInfo 582/582
in 111.96 seconds**, each with zero failures/errors/skips/disabled and all statuses
`run`. Each suite executes 575 unit and seven acceptance cases. Both exact fresh
inventories match the JUnit and immediate LastTest names; all 520 previous and
582 starting names survive. The 62 added X05 names comprise 15 A0, 20 A1, three A2
and 24 A3 cases; `registration-test-groups.json` records their exact set.

Debug's 14 and RelWithDebInfo's 10 explicit commands all pass in `attempt01/`.
Pinned formatting/Doxygen checks 117 active C++ files per configuration, both
final builds report `ninja: no work to do.`, and every complete source-stability
check passes. There were no failed gate attempts or repeated full executions.
Debug and release use the same 24,021-entry tested-source freeze. The release
preset is actually RelWithDebInfo, not CMake Release. Both gates run macOS ARM64.

Each full CTest command has no filter. The immediate JUnit, LastTest.log and
CTestCostData.txt are retained before later commands; exact inventory/JUnit/
LastTest names, unique registration, status run, zero failures/errors/skips/
disabled cases and baseline 520/current 582 name preservation are checked.
Cost data supports the record but is not a separate execution count.

Four source-only vector generators run once on unchanged common inputs. Four
configuration-dependent inherited runtime/CLI audits, five storage/export
variant manifest forward/reverse checks, A0 runtime/interface exact reversible
generation and shipped Lua byte checks, pinned formatting/Doxygen and no-work
all+planar_tests builds run per configuration. No helper silently repeats a
suite, and the earlier focused 209 and approved private runners are not rerun.

The full registered suites include the production registration/refusal cases.
Actual retained alternative dispatch, repeated-root sealing, controlled native
and Lua allocation refusals, cleanup and corrected location/resource probes are
retained independently approved Debug evidence in A1/A2/C05; they are not claimed
as RelWithDebInfo private-probe reruns. Profile-owner invocation still refuses
before provider effects. Shipped Lua and reserved shape/projection behavior are
unchanged. Native cumulative conversion/audit charges and conservative pure-helper
reservations remain logical charged metrics, not process RSS accounting.

X05-B/C own actual custom shape/projection semantics and builtin adapters. X06
owns recorded interpretation/history/storage acceptance, receipt orchestration
and whole-invocation budget sharing; X07 owns atomic public admission/cutover;
X08 owns mandatory external-profile acceptance. Execution/RA01 and Linux remain
open. Public init still creates format1. Fresh independent review is required
before advancing from this integrated X05-A candidate.

The current reference reconciliation records the actual X04-B4 independent
approval and combined A3 through C05 approval, including A0+C01's test-only
approval in alignment/acceptance references. Historical round reports and raw
evidence are immutable. The eight current status documents plus new evidence
map are the only post-gate source differences; no product, tests, support,
configuration, vendor, fixtures or historical evidence changes. The exact
`owned.diff`, `final-owned/`, `candidate-manifest.json`, `final-filesystem.json`,
`artifact-manifest.json` and `finalization.json` bind the final candidate and
release hashes. Per-configuration pass manifests and build identities bind the
measured gates. Source archive copies preserve all first-party product/test
sources, build configuration/tooling and verification support at the tested freeze.
