# R13-C2 and integrated R13 independent review

Disposition: **approve**. No blocking product or contract finding remains in
C2 or the integrated R13 milestone. One nonblocking documentation correction is
recorded below; the reviewed candidate was not edited. This approval does not
implement or approve R14 acceptance, history activation or export.

## Scope and exact candidate

Fresh reviewer `/root/r13c2_reviewer` reviewed shared
`dev/planar-v2-vernacular`, HEAD
`3171c49e7b92d3abda01bfbb8bfb0c6b29d4c67a`. Read active repository instructions,
implementation/roadmap, v4 and ledger contract, R13/C/C2 briefs, A approval,
original B1 finding and corrective B1-C01 approval, B2 and C1 approvals, exact
candidate/diff, complete golden structures and raw evidence. The intentional
uncommitted rewrite and all prior evidence were preserved. Product files were
read-only; writes are confined to this report and
`tests/native/r13c2-review-evidence/`. No Planar tools/skills/queue, installed
binary, operator data, subagents, commits, installation, publication or merge.
Both build directories were exclusively held; build/test operations were direct
and serial. Both are released on delivery.

Candidate manifest `tests/native/r13c2-evidence/candidate.json` SHA256:
`20c8e91f43e7723d4c0b26b4c7c31404c116db00987445aa383d877f4d30123a`.
Thirteen-file candidate identity:
`37ad152cae2e7601a85761a6f23f9f8cc6213beafa9451ade67af05dbbba25e8`.
Candidate keys are repository-relative; exact snapshots use the same paths
below `candidate/`. Identity hashes compact sorted-key UTF-8 JSON of the files
map, with comma/colon separators and no terminal LF. All live files/snapshots
match and the baseline-relative diff independently reconstructs exactly.

Coder evidence manifest SHA256:
`3f8db25abbaa7f7a6416167f606bd1eed2d4c06cf014fc6832eb9609fcdd51c5`.
All 61 folder-relative entries match. All 11,863 repository-relative protected
inputs remain unchanged, including protected product/vendor/prior evidence and
completed round records. The frozen protection map explicitly excludes OS
`.DS_Store` metadata and active coordinator R13/C/C2 records. No protected drift
was found. Independent composition of the latest A/B1/C01/B2/C1/C2 manifests
verifies all 36 integrated candidate paths; approved component identities are
retained in the audit log. The final audit repeats every check after verification.

## C2 technical assessment

`src/stdlib/planar/acceptance.lua:3` invokes existing `validate_bundle` once
under `{status="not_captured"}`. That existing boundary already normalizes all
declarations and validates every job. The wrapper preserves its complete bundle,
dependencies and job models, sorts both normalized and semantic jobs by ID, and
returns the existing typed refusal unchanged. No schema, ownership, dependency,
selection or projection policy is copied into native code. The only existing
Lua source change is the fixed export in bootstrap. The ninth embedded original
participates in exact provenance and assembly identity; all earlier protected
Lua files remain unchanged.

At `src/engine/history/interpretation.cpp:161`, native preparation internally
prepares embedded stdlib provenance and calls only the fixed trusted export.
There is no caller stdlib digest argument, database access, fresh filesystem
read, clock/process capability or Lua execution under a transaction. The owning
result retains both complete canonical values/encodings and prepared provenance
for later transaction composition. Exact registry bytes/mode remain separately
available through C1, with parsed normalized registry retained in the bundle.
No database-local IDs enter portable output.

At `interpretation.cpp:57`, fixed envelope validation checks exact top-level,
bundle, semantic, versions and semantic-model envelope keys; normalized container
kinds; nested semantic field types; logical file paths; strict paired job ordering
and ID/file correspondence. The fixed ten-key versions map agrees exactly with
the ledger contract. `decode_interpretation` at line 141 uses bounded canonical
decoding and byte-exact equality with the separate versions value. It validates
digest spelling without comparing historical stdlib identity to today's code.
Historical decoding calls neither Lua nor current declaration policy; structurally
valid recorded policy values/extensions remain data. Supported recorded versions,
malformed data, resource limits, live validation and runtime failure retain
distinct categories. Canonical map/list and signed-32 semantics remain unchanged;
each stored encoding has the 16 MiB ceiling.

The complete Glaze JSON/canonical goldens were reviewed for workspace routing,
captured text, optional fields, all spec/registry definitions, both combined and
split jobs, complete dependencies/assignments/effective checks/graphs and versions.
Permanent tests independently compare whole values with existing normalize and
validate exports, so fixture capture cannot conceal dropped fields. They also
reverse routing/capture order and preserve absent optional values. The saved
fixture test actually interns both value records, commits/reopens and decodes
recorded bytes without Lua or capture. Independent saved-history probes strengthen
malformed framing, nested shape, paired ordering/correspondence, separate-version
equality and historical-policy tests.

## Integrated R13 obligations and boundaries

The previously approved component implementations remain exact under the latest
reviewed changes; the complete gates now exercise them together in both presets.

| R13 obligation | Assessment and retained evidence |
| --- | --- |
| Exact record framing and bounded canonical decode | A codec remains exact; ten independent record vectors and existing hand-byte/upstream/domain vectors pass. Record tests retain malformed claims, modes/names/IDs, proper truncations and exact bounds. |
| Complete immutable trees | B2/C1 shared construction/digest and reconstruction remain exact. Thirteen tree cases cover empty root, exact binary/CRLF/empty/executable bytes, sorted topology, rename/delete/same-length edits independent of timestamps, old complete roots, owned lifetime, shared expansion and profile/path/depth/file/directory/payload bounds. |
| Exact collision refusal and rollback | All four record kinds, local directory child-ID equality, pending-prefix rollback and both ambiguous-bucket orders remain active. Original B1 verification-only equality-mutation RED is preserved; fresh normal variant generation redirects both digest sites and reverses exactly. No product hash override or weakened comparison. |
| Stable verified storage ownership | B1-C01's real read-fault reader invalidation regression remains active, with original RED preserved. Writers retain typed refusal, whole-prefix rollback, explicit commit and caller ownership. Populated revision/request history remains refused. |
| Exact separate registry/stdlib provenance | C1 remains exact. Both presets independently match nine original byte arrays/portable paths and ordered LF assembly. Pure portable roots and stored reconstruction share the approved tree recipe; original boundaries/order/separators remain recoverable. |
| Stored interpretation and historical reconstruction | C2 preserves complete trusted outputs, separate exact versions and supported historical decoding without current Lua/policy or database IDs. Native record storage roundtrip and independent historical corruption probes pass. |
| Existing renderer and engineering gate | Both complete gates include the unchanged CLI acceptance case (247 source-built invocations), renderer/scoped identity tests, fault/collision helpers, active formatting/Doxygen and zero-work builds. |

This completes the R13 record/tree/provenance foundation, including A27 and tree
parts of A28/A29. It does not establish revision chronology/no-op/revert, CAS,
request replay, capture recheck, accepted-history events or safe filesystem export;
those remain R14. Active role/run/dispatch/check/review state and full v4 Cut 2
remain later reviewed work. No broader acceptance or historical-policy execution
claim is inferred from generic value/request records.

## Independent verification

Exact argv, raw logs, registry snapshots, JUnit, source and reproducible probes
are retained in [review evidence](../../../tests/native/r13c2-review-evidence/README.md).

| Check | Result |
| --- | --- |
| `make test-all` | Exit 0; 226/226 passed, zero skips; 50-source formatting/Doxygen; zero-work no-change build. |
| `make test-all PRESET=release` | Exit 0; 226/226 passed, zero skips; 50-source formatting/Doxygen; zero-work no-change build. |
| Separate CTest `replacement.interpretation` | 10/10 passed, zero failures/skips; retained JUnit. Each complete gate also includes all 109 cases in the R13 focused selection. |
| Independent saved-history harness | Three cases, 13,703 passing assertions; all 13,114 proper interpretation and 449 proper versions truncations, nested corruption, duplicate/reversed paired jobs, mismatches, old policy/digest and exact separate versions. |
| Both vector generators `--check` | 12 hand-byte, 20 upstream-hash, 41 canonical/domain and ten independent record vectors pass. |
| Independent final identity/protection/generation/embedding audit | Exact candidate13/evidence61/protected11863/integrated36; both configured variants and all nine originals/chunk match fresh generation in both presets. |
| Supplementary probe formatting and counts/JUnit/raw-log verification | Pass; all selected counts nonzero, zero skips, all 226 passing statuses per gate and no-work endings independently confirmed. |

Coder intermediate unused-helper compilation, absent goldens, normalized registry
set-order assertion and diagnostic-free Doxygen SIGBUS/retry are preserved and
accurately distinguished from final successes. No product test failure is hidden.
Own read-only inspection initially needed replacement UTF-8 decoding for binary
diagnostics. Own log checker initially expected older CTest summary spelling;
its original failure/source is preserved and its corrected form retains all
count/status/skip/lint/no-work checks. Neither issue changed product/test behavior.

Executed platform is macOS arm64 with the pinned toolchain. Linux execution,
physical disk exhaustion, persistent device faults and hostile filesystem races
are not established. Existing configured VFS/capacity/process-death checks do not
imply stronger hardware durability or exhaustive allocation-failure coverage.
No unresolved product choice or full-R13 implementation gap was found.

## Nonblocking editorial note

`docs/testing.md:48` still says the current registry has 216 tests (215 native
and one Python case). It now has 226 (225 native and one Python case); C2's added
coverage description is accurate, but that inherited summary was not refreshed.
This is a low-priority documentation correction, not a product/gate failure.
Coordinator acknowledged a fresh narrow documentation correction loop after this
approval. The current candidate was left exact.

Independent evidence manifest: 44 folder-relative entries; SHA256:
`024201db5b6a1b8a6248fc1135ced319c46ad6900717b47454f4f69e28a7a543`.
This report is outside that manifest and receives a separate delivery hash.
