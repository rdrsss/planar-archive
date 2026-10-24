# X05-C5 integrated projection and read-service verification

Status: verification candidate submitted for fresh independent review. C5 does
not approve itself. Shared `dev/planar-v2-vernacular`, HEAD
`3171c49e7b92d3abda01bfbb8bfb0c6b29d4c67a`, intentional dirty rewrite.
D3 and combined D4+C01 are independently approved. The latest independently
approved full gate remains historical 676/configuration pending C5 review.

## Fresh gates

| Actual configuration | Full unfiltered CTest | Time | Private cases | Private assertions |
| --- | --- | --- | --- | --- |
| Debug | 876/876 | 283.98 s | 10/10 | 709 |
| Fresh release preset, RelWithDebInfo | 876/876 | 132.68 s | 10/10 | 709 |

Both have zero failures/errors/skips/disabled and all full-suite JUnit statuses
`run`. Each has 869 unit cases and seven acceptance suites. The exact 876 unique
inventory names equal the frozen current D4 inventory, JUnit, immediate
LastTest.log and fresh CTestCostData rows. All historical B4-C 676 names remain
present. Later D3/D4 rename records are retained separately; no unchanged-name
claim is made for those intermediate renamed cases.

Exact commands, durations and exits are in [Debug commands](debug/commands.json)
and [release commands](release/commands.json). Each config runs configure,
`all planar_tests`, inventory and exactly one full unfiltered CTest with an
absolute JUnit path; immediate raw logs are copied before any later check.
No wrapper or full-suite rerun was used. The preexisting Debug cost cache was
preserved and only its generated build-local cache was removed after concrete
stale/missing-name reconciliation; [the record](debug/cost-cache-removal.json)
identifies the discrepancy. No other Testing history was removed.

Six common frozen-source vector families pass once in Debug: selection/W,
shape-definition, canonical, record-v1, request-v2 and effective provenance.
Each actual config passes five applicable historical source/generated/CLI audits,
the retained D2-B2-C01 native audit, current D4 runtime audit, and actual generated
runtime/interface and five storage/export reversals. Generated compile identities
bind actual config paths and commands. Full CTest executes existing CLI goldens
and generated variant tests. Pinned formatting/Doxygen checks all 118 active
C++ sources per config. Each final `all planar_tests` build reports literal
`ninja: no work to do.`

## Preserved integrity-tooling deviations

The initial configure-only support attempt was interrupted during redundant
post-configure hashing of 33 GiB of historical evidence. Configure succeeded;
no build or full CTest had run. [support-attempt1](support-attempt1/) preserves
that driver and raw configure record. The immutable baseline/preedit copies were
never replaced. The optimized guard always enumerates complete membership,
modes and link targets, and reuses a digest only with unchanged device, inode,
ctime, mtime, size and mode/type. Fresh processes fully hash at gate boundaries
and finalization. [integrity-method.json](integrity-method.json) states the exact
method and cooperative stable-source limitation; no continuous-change claim.

Debug then preserved a failure in the historical B2-A source audit:
[raw failure](debug/b2-a-audit.log). Its old delimiter extends from shape lookup
to `create_impl`, now spanning unrelated projection functions and detecting their
`function_reference`. The exact lookup body has no such access. The existing
current D4 audit uses the precise next projection-definition boundary and retains
every original B2-A invariant. [Applicability and assertion mapping](audit-applicability.json)
records native-only lookup, no budget resets, no callable/provider/filesystem
access, exact hash/source/owner bytes, no ordinal, and the generic profile guard.
The old helper was never edited. [Frozen pre-failure driver](gate-at-obsolete-audit.py)
and the original [support freeze](support-freeze.json) remain preserved.
[Debug continuation](continue-debug.py) ran only remaining checks using the
successful full results. The [second support freeze](support-freeze-v2.json)
omits only the obsolete helper invocation for release. Current guarantees and
actual config variant audits remain required. This is no product failure or fix.

The first finalization link checker stopped on an existing URL-encoded historical
Markdown path. [The retained parser attempt](finalization-attempt1.json) records
the failure; decoding the path fixes the checker without changing the link or
repeating product tests. A second [ordering attempt](finalization-attempt2.json)
preserved the checker stopping before its own linked diff/manifests were created;
those exact scheduled targets are now checked after sealing. Full cryptographic
protection passed on both attempts.

## Cross-configuration private proof

C5 adapts D4's ten controlled cases using each actual configuration's compiler
and linker commands. [Release preflight](release/compiler-mode-preflight.json)
proves effective `-O2 -DNDEBUG` for interface/runtime/probe, compares Debug,
and rejects Debug paths or an `-O0` override. Final verification compares actual
private compile flags to those originals, including disabled implicit modules.

Each config copies 888 compiled/module/archive/helper/runtime dependencies;
recursive response-file expansion matches the actual interface/runtime/test/link
argv. Independent reconciliation records 133 direct actual dependency members
and three response files per config, covered by that copied superset:
[Debug closure](debug/private/dependency-closure.json),
[release closure](release/private/dependency-closure.json).
Shared and copied full membership, modes, SHA256, sizes and mtimes stay exact;
Ninja `.ninja_log`/`.ninja_deps` hashes and mtimes stay unchanged. No Debug object
is used in release. Actual loader output proves the copied admission syscall
helper, without claiming the copied libc++ was selected. Original dynamic install
names and that limitation are recorded in each loader proof.

Controls exercise same-owner role/projection/base/assembly frames, GC, retained
outer aliases/roots and cumulative counters; late original role-file mutations
on result/error branches; lazy IO; sticky first semantic/refusal/resource errors;
selected catalog attribution; native/Lua allocation, node and instruction limits;
and assembly fallback. Assembly's instruction test explicitly pre-exhausts the
retained instruction counter at entry; selected F's instruction case actually
executes its loop under a lowered remaining allowance. These are distinct
controlled tests, not natural host exhaustion or RSS measurements. Ordinary full
native tests supply the wider profile/root/standalone receipt matrix and decimal
ordinal/ID boundary controls. Unrelated historical private Debug probes were not
rerun in release and remain historical.

Cleanup verified each owned disposable entry before removal: 893 files per
config (1,460,326,228 Debug bytes; 1,312,133,952 release bytes). Sources, commands,
XML, layout, loader/dependency/journal proofs and removal inventories remain.
No shared build products or prior evidence were removed.

## Source, prose and release

[baseline.json](baseline.json) captures 76,510 entries before edits, including
product/tests/vendor/fixtures/build configuration/tooling, prior rounds/artifacts
and protected `build/install-release`. [Release precreation](release-precreation.json)
records that `build/rewrite-release` was absent before its authorized creation.
The exact C5 coordinator round is the only round exemption; D4/C01 and prior
records are protected. AGENTS.md remains the CLAUDE.md symlink.

[tested-source-manifest.json](tested-source-manifest.json) binds 628 preserved
active source/configuration/tooling/helper/vector inputs. Both per-config
boundary checks and [final protection](final-protection.json) verify unchanged
product/test/vendor/fixture/tool identities and prior artifacts. Source-only
identity checks after tests are distinct from [post-test prose](post-test-prose.json):
the eight owned status openings and new projection evidence map were finalized
after both gates. Their exact final prose was not the prose tested by CTest.
Pre-edit bytes are preserved once; [candidate.diff](candidate.diff) reconstructs
the owned status/support/map changes.

[frozen-candidate.json](frozen-candidate.json),
[artifact-manifest.json](artifact-manifest.json) and
[finalization.json](finalization.json) bind final reviewable files, raw artifacts
and tested-source identity without a hash cycle. [Verification summary](verification-summary.json)
records exact results; [the reference map](../../../docs/vernacular/x05-projection-evidence.md)
connects internal closure obligations and remaining work. Files and both build
directories are released for fresh independent review. No product/test/vendor/
build-config/prior-helper edits, delegation, Planar operations, commits,
installation, publishing, merging or operator-data access occurred.

X06 recorded interpretation/history/acceptance and original-receipt/shared-budget
orchestration, X07 public profile flags/cutover, X08 mandatory external-profile
completion, execution/RA01 and Linux remain open. No full-process RSS guarantee,
public profile CLI or external-profile completion gate is claimed. Public init
continues to create format1. D4's corrected historical red-source limitation and
original failed review remain preserved; C5 supplies no reconstructed historical
red execution or causal claim.
