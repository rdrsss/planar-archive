# X05-A integrated registration evidence

Status: verification candidate pending fresh independent review. The [A1 review](rounds/V5-02-X05-A1-review.md),
[A2 review](rounds/V5-02-X05-A2-review.md), and [combined A3 through C05 review](rounds/V5-02-X05-A3-C05-review.md)
independently approve the production sealing and captured registration slices.
C05 closes R1–R7. A4 adds no product code, test cases, fixtures or golden changes.
The mandatory external-profile acceptance gate remains open.

## Registration composition

`command_vm::create(profile_snapshot)` structurally verifies and privately copies
captured profile/module bytes, modes and order. Source-only zero-result chunks
execute in the same bounded VM under native owner ordinals. Three exact APIs
retain descriptors and callables; final descriptors validate/copy before module-end
D/F audit and sealing. Catalog-v2 and effective provenance bind original captured
bytes. A single shared identity/prototype audit precedes freezing, and a persistent
native-owned successful set preserves prior immutable proxies and aliases.
Terminal failures preserve original entry locations without formatting inside Lua.
A completed restricted `lua_pcall` memory failure stays sticky even through catches;
a successful emergency allocation retry remains successful.

| Obligation | Full-suite production cases and retained evidence | Limit |
| --- | --- | --- |
| Exact captured identity and owner order | `X05 A3 validates structural snapshot before any module evaluation`, `ordered external descriptors finalize and retain native owners`, `real capture owns original source after filesystem changes`, `retained source byte and mode changes affect exact effective digest` | Native registration uses retained captured inputs; public profile CLI is absent. |
| Source-only execution and frozen authority | `X05 A3 source only chunks require actual zero results`, `external environment is pure frozen and self rooted`, `exact descriptor schemas and external capability restrictions`, `separate identity namespaces and all reserved IDs` | No read services run during registration; shipped shape/projection IDs remain reserved. |
| Lexical writers, aliases and exact native identities | Production A1 guard cases covering direct/inherited/shared writers, actual helper closures, masks, source-compiled payloads, C roots and shared graph depth; A2 prior successful identity cases; A3 retained writer/native-root refusal | Original embedded handlers keep their engine; no claim of semantic honesty or opaque C-upvalue inspection. |
| Module boundaries and cumulative failures | `X05 A3 instruction accounting does not restart for later chunks`, `resource ceilings remain sticky when protected calls catch`, C04 sticky-original-refusal and C05 nested ordinary/resource controls | Conversion/audit charges and conservative pure-helper reservations are cumulative logical metrics, not RSS. Whole-invocation orchestration remains future work. |
| Correct original diagnostic attribution and bounds | C01 malformed sort keys/high catalog/handler-option controls; C02 mixed-kind/module locations; C03 two-digit locations; C04 original descriptor remapping; C05 nested resource preservation | Approved Debug private runners cover actual allocation refusals and adapters; not every controlled fault is a natural exhaustion or registered release test. |
| Invocation boundary and retained alternatives | `X05 A3 profile owners refuse invocation before provider effects`; approved A1/A2/C05 reversible private production copies execute retained alternative callables and failure/cleanup probes | Profile-owner invocation still refuses. Actual public dispatch, builtin adapters and shape/projection semantics belong to B/C and later public cutover. |

Full CTest names have the additional `replacement.` prefix. Exact names and
results are in the fresh per-configuration inventories and JUnit. Detailed
approved private execution, repeated sealing and fault-probe sources/commands
remain in the [C05 independent review](rounds/V5-02-X05-A3-C05-review.md) and its
links to A1/A2. A4 does not rerun the focused 209 or those private runners and does
not claim private RelWithDebInfo fault-probe execution. Historical reports remain
unchanged.

## Observed integrated gate

Full Debug and release-preset **RelWithDebInfo** each execute one unfiltered CTest
suite: **582/582**, zero failures/errors/skips/disabled cases, every status `run`.
Exact fresh inventory/JUnit/LastTest names agree and preserve all current 582 and
prior 520 names. Immediate LastTest.log and CTestCostData.txt copies precede later
checks; cost data is supporting history rather than another test count.

Commands per configuration are `cmake --preset debug`/`release`, direct
`cmake --build build/rewrite-<configuration> --target all planar_tests`, fresh
`ctest --show-only=json-v1`, and `ctest --output-on-failure --output-junit` with
an absolute output path. No helper repeats a full suite.

Four common-input vector generators run once: 12 hand byte/20 upstream hash/41
canonical-domain checks, ten record-v1 and two request-v2 vectors, plus the effective
provenance generator's 12 established canonical/four portable-record controls and
five fixed artifacts. Both configurations separately pass the four inherited
B2/D6/D6-C01/D7-B exact runtime/CLI audits, all five storage/export generated-family
SHA256/substitution/forward/reverse checks, A0 runtime/interface exact reverse and
shipped commands source audit, pinned formatting/Doxygen for 117 active C++ sources,
and a final `all planar_tests` build reporting `ninja: no work to do.`.
The full suites execute the actual registered generated storage/export tests;
manifest checks support their source identity and add no new collision claim.

## Integrity and remaining work

[Raw evidence](../../tests/native/v5-02-x05-a4-evidence/REPORT.md) contains argv,
UTC times, statuses, actual counts, immediate CTest logs, exact inventories,
per-configuration build hashes and audits. A complete 24,021-entry source freeze
excludes only root `.git/`, root `build/` and the new A4 evidence root. Equality is
required before and after every gate command. Both gates preserve identical
product/test/configuration/support bytes. Only the eight authorized current
status documents and this new map change afterward; post-gate stability, exact
owned diff, final-owned copies, candidate/source/artifact manifests bind the
finished candidate. AGENTS.md remains the CLAUDE.md symlink.

These gates ran on macOS ARM64. X05-B/C actual shapes/projections and builtin
adapters, X06 recorded interpretation/history/storage acceptance and receipt/
whole-invocation orchestration, X07 atomic public cutover, X08 mandatory external-
profile acceptance, execution/RA01 and Linux remain open. Public init creates
format1. Independent A4 review and a current-candidate identity check are required
before advancing the integrated X05-A candidate.
