# X05-B4-C integrated shape verification candidate

Status: verification and authorized documentation complete; pending fresh independent
review. This author does not approve the candidate or close X05-B. Work ran in the
shared `dev/planar-v2-vernacular` checkout at
`3171c49e7b92d3abda01bfbb8bfb0c6b29d4c67a`, preserving its intentional dirty rewrite
and every unrelated/historical file. Both build directories were exclusively owned
and serialized. No Planar tool/role/skill/queue, subagent, installed binary, operator
data, commit, installation, publication or merge was used.

## Measured results

| Configuration | Full execution | Counts | Complete checks |
| --- | --- | --- | --- |
| Debug | 676/676, 268.37 seconds | 669 unit + 7 acceptance; zero failures/errors/skips/disabled | Six common vectors, six config audits, five storage/export reversals, A0 runtime/interface/shipped Lua, pinned lint/Doxygen 118 sources, no-work |
| release preset (RelWithDebInfo) | 676/676, 126.24 seconds | 669 unit + 7 acceptance; zero failures/errors/skips/disabled | Six config audits, same inherited variant/A0 checks, pinned lint/Doxygen 118 sources, no-work |

Each configuration executes exactly one unfiltered CTest suite with an absolute
JUnit path. Fresh JSON inventories have 676 unique names. Inventory/JUnit/immediate
LastTest names match exactly, all cases have status `run`, and all current 676 and
prior approved A4 582 names remain. The 94 additions since A4 are enumerated in
`added-since-a4.json`; they were present before this verification-only round.
JUnit, LastTest.log and CTestCostData.txt are copied immediately when CTest returns,
before later commands. There are no duplicate full-suite wrapper executions.

`attempt01/debug/commands.json` records 18 external gate commands and
`attempt01/release/commands.json` records 12, with exact argv, cwd, UTC start/end,
duration and return code. These comprise configure, `all planar_tests` build,
JSON inventory, one full CTest, six vector families once in Debug, six config
source/runtime audits per build, pinned lint and no-work. Inline exact
forward/reverse manifest checks produce the five-storage/export and A0 JSON
artifacts. All recorded external gate commands return zero. Environment/tool
versions and exact build identities are retained separately.

Canonical checks report 12 hand byte vectors, 20 upstream hashes and 41 canonical/
domain vectors. Selection checks verify the independently authored W/full canonical
preimage and external domain/source definition. Shape-definition output records a
733-byte canonical construction and expected digest. Record-v1/request-v2/effective
provenance output and exact counts remain in their raw logs rather than being
presented as separate CTest cases. Six current audits are B2 variants, D6, D6-C01,
D7-B, B1 and B2-A. The full registered suites execute the real production/generated
storage tests; source-reversal audits supplement their exact provenance.

## Preserved failed author check and authorized continuation

The initial Debug driver completed its full suite successfully, then exited on an
overstrict scheduler-cache name assertion. Raw immediate CTestCostData has 677
unique rows: all 676 current test names and the obsolete historical row
`replacement.X05 B1 cumulative obligations retain the exact 100000 global ceiling`.
There are no missing or duplicate current names. This is historical scheduler
state, not a product test failure or an extra execution. The failed traceback is
preserved in `debug-driver.log`; its source remains byte-for-byte frozen.

The coordinator explicitly accepted the registered-name reconciliation and forbade
a full-suite rerun. `continuation-authorization.json` records that instruction,
original support hash and the independently frozen evidence-local continuation
hash. `continuation.py` resumes only remaining Debug checks, reusing the captured
configure/build/inventory/full logs without executing those commands again. It
retains the original command rows, records the reconciliation, then runs vectors,
audits/lint/no-work. `debug-continuation-driver.log` preserves the result.
For release, it preserves the pre-full scheduler cache as
`pre-full-CTestCostData.txt`, removes only that generated build-local cache, and
executes the release suite once. Fresh release cost data contains exact 676 names
with no stale rows. Both raw files and reconciliation JSON remain available.
There are no failed CTest executions and no concealed/retried suite attempts.

## Exact inputs, scope and finalization

Before prose edits, `before-docs/` preserves all eight actual status-document
baselines. Starting Git revision/status are saved. The original support was frozen
before commands; `attempt01/source-freeze.json` binds 29,306 file/symlink entries,
including modes, exact bytes and membership. Only root `.git/`, root `build/` and
new B4-C evidence are excluded. Historical nested build/evidence trees and vendor
inputs remain protected. Complete equality checks run before/after each command;
both gates test identical product, production-test, configuration and support
bytes. Source archive copies preserve live first-party product, tests, fixtures,
build/tooling and verification support; the full freeze binds remaining inputs.

After both gates, only the eight owned current status documents and the new
`docs/vernacular/x05-shape-evidence.md` map change. The narrow status reconciliation
records independently approved B2-C+C01+C02, B3-A, B3-B+R01 and B4-B+C01, including
B4B-R1 closure, while preserving all historical reports and raw artifacts. The
new current checkpoint explicitly awaits this gate's independent review. The
AGENTS.md -> CLAUDE.md symlink remains intact. No product, test, fixture, vendor,
configuration or original support bytes change.

`owned.diff` is the exact incremental prose diff from actual captured baselines;
`final-owned/` preserves final prose. `candidate-manifest.json` binds current live
candidate sources/support/tooling and owned prose. `final-filesystem.json` binds
full source membership; per-build pass manifests/build identities bind measured
outputs. `post-gate-source-stability.json` proves the only post-gate differences
are the nine allowed prose paths. Artifact/finalization manifests bind this report,
exact diff, candidate, full freeze and raw evidence. No further author edits or
build/test processes remain after finalization; both builds and all author edits
are released to the coordinator/reviewer.

## Coverage limits

The [shape evidence map](../../../docs/vernacular/x05-shape-evidence.md) relates
full-suite production cases to B1 policy, B2 exact definition/retained F/immutable
transport, B3 same-call semantics and W, and B4 complete native rendering/ordinary
error-envelope bounds. Earlier independently approved Debug private allocation,
retained-reference/GC, phase/writer/F and original-input adversarial controls remain
cited evidence; this gate neither reruns every private probe nor claims their
RelWithDebInfo execution. Controlled refusal tests do not establish natural host
exhaustion or RSS accounting.

Both full gates run on macOS ARM64. Generic profile command invocation/public
profile admission remain closed; public init creates format1. C projection/read
services, X06 recorded interpretation/history/storage acceptance and receipt/
whole-invocation orchestration, X07 public cutover, X08 mandatory external-profile
acceptance, execution/RA01 and Linux remain open. Fresh independent review and a
current-candidate identity check are required to close integrated X05-B.
