# V5-02-X05-A3 independent review

Disposition: **request-changes**. Three scoped findings remain: original-index diagnostics can identify the wrong descriptor, a pure provenance resource refusal becomes `internal`, and the final logical buffer reservation needs a complete coverage derivation. A3 is not approved. Preserve this candidate and evidence; use a fresh corrective search/coder/reviewer round before dependent implementation.

The shared checkout is `dev/planar-v2-vernacular`, HEAD `3171c49e7b92d3abda01bfbb8bfb0c6b29d4c67a`. Review owns only this report and `tests/native/v5-02-x05-a3-review-evidence/`. Product, current reference documents, coordinator records, author and historical evidence stayed read-only; other work was preserved. No legacy Planar tools/roles/skills/queue, installed binaries, operator data, commit, installation, publication or merge were used. The reviewer held sole serialized build ownership, read current CLAUDE.md, implementation/roadmap/v5 direction, the complete A3 authorization and environment ordering, exact X-CAP/X-REG/X-LIMIT/X-STORE and B/C boundaries, and approved A1/A2 reports/reviews before inspecting the candidate and raw evidence.

## Required corrections

### A3-R1 — Preserve the offending original index before catalog sorting

[Runtime catalog composition](../../../src/engine/runtime/runtime.cpp):1886 sorts every list by `text_field(descriptor, name/id)` before the pure validator checks those fields. [Final descriptor collection](../../../src/engine/runtime/runtime.cpp):1954 updates `registration_field` while converting every entry, then calls catalog validation only after the loop. A missing or non-string sort key in an earlier entry can throw before the canonical-index-to-original-index mapping runs. The catch retains the last converted entry's field.

Two independent tests link the **unmodified frozen production runtime**:

- A first shape descriptor containing valid `abi/revision/stages/help` but missing `id`, followed by one completely valid shape, returns `internal / Invalid external profile protocol` at `catalog.shapes[2]`; the sole malformed descriptor is original `catalog.shapes[1]`.
- A first projection descriptor with `id=1` and otherwise valid fields, followed by one completely valid projection, returns the same protocol failure at `catalog.projections[2]`; the sole malformed descriptor is original `catalog.projections[1]`.

[Exact source, compile/link commands and outputs](../../../tests/native/v5-02-x05-a3-review-evidence/adversarial/commands.json), [shape output](../../../tests/native/v5-02-x05-a3-review-evidence/adversarial/missing-shape-id.log), [projection output](../../../tests/native/v5-02-x05-a3-review-evidence/adversarial/wrong-projection-id.log). Both detectors exit1 as expected. Each fixture has exactly one malformed entry; no precedence between unrelated faults is assumed. This violates X-LIMIT's original-order logical fields and the A3 acceptance criteria.

Corrective work must retain each entry's native logical field through all pre-validation transformations, or validate required sort-key structure while the original field is current. Audit command-name sorting as well: it shares this composition path, although this review reproduced shapes and projections only. Handler IDs are checked by the pure validator under its per-entry field and then remapped; option-name sorting occurs while the containing command field is current. These paths need coverage, but the same stale-field defect is not independently claimed for them. Capability sorting already checks string kinds before sorting. Preserve exact diagnostic messages/categories and do not add a new malformed-versus-duplicate precedence rule.

An independent control with valid shape `z` first and bad-ABI shape `a` second correctly returns original `catalog.shapes[2]` after canonical reordering. The existing remapping works when execution reaches it; the correction must cover earlier failures too.

### A3-R2 — Preserve typed resource refusal from initial provenance preparation

[Runtime](../../../src/engine/runtime/runtime.cpp):2105–2107 unconditionally throws `external_protocol()` when initial `prepare_stdlib` fails. Its `provenance_error_code::resource_limit` must propagate as resource exhaustion. The later `prepare_effective_stdlib` adapter already distinguishes that category.

[Pure preparation](../../../src/engine/history/provenance_codec.cpp):86 maps failed canonical metadata encoding to a typed provenance resource refusal. [Canonical encoding](../../../src/engine/canonical/canonical.cpp):205 catches `std::bad_alloc` and returns a resource diagnostic. Therefore an allocation exception caught by this pure helper bypasses the constructor's outer bad_alloc catch and becomes `internal / Invalid external profile protocol` at `catalog`.

The independent [allocation probe](../../../tests/native/v5-02-x05-a3-review-evidence/allocation-mapping/commands.json) links the unchanged production runtime against a reversible canonical implementation copy. The sole insertion throws an actual `std::bad_alloc` inside canonical_encode's existing protected try only for `ordered_concat_v1` metadata. Its existing catch executes; the runtime then returns `internal`, and the resource detector exits1. [Exact reversible delta](../../../tests/native/v5-02-x05-a3-review-evidence/allocation-mapping/delta.json), [output](../../../tests/native/v5-02-x05-a3-review-evidence/allocation-mapping/allocation-resource-detector.log). This is controlled exception injection, not real host-memory exhaustion. A separate [typed-result control](../../../tests/native/v5-02-x05-a3-review-evidence/resource-mapping/commands.json) injects the helper's typed resource result directly and reproduces the same adapter defect.

Map the pure resource category before mapping other provenance failures to protocol. Check equivalent new helper adapters and retain tests for exceptions both thrown through and caught within pure helpers. The author constructor/callback green/red controls pass, but they do not cover this typed-result path.

### A3-R3 — Complete the final canonical/payload reservation derivation

[Runtime](../../../src/engine/runtime/runtime.cpp):2081 reserves four bounded 1 MiB canonical buffers before pure effective preparation. The report assigns these to effective/catalog/seal buffers and small embedded metadata. The following distinct logical payload constructions must be reconciled explicitly:

1. `decode_catalog` retains its input canonical catalog bytes in a new owned string ([effective_catalog.cpp](../../../src/engine/history/effective_catalog.cpp):323).
2. Effective preparation creates the canonical combined manifest string ([effective_provenance.cpp](../../../src/engine/history/effective_provenance.cpp):166).
3. Final tree digest constructs a `file_record` payload copy of `__planar__/effective.bin` ([tree_core.cpp](../../../src/engine/history/tree_core.cpp):180).
4. Digesting that file record constructs the encoded record buffer ([record.cpp](../../../src/engine/history/record.cpp):256).
5. Effective preparation retains another owned effective.bin payload in `result.files` ([effective_provenance.cpp](../../../src/engine/history/effective_provenance.cpp):183).
6. Runtime concatenates catalog bytes and digest into a separate `c.seal` string ([runtime.cpp](../../../src/engine/runtime/runtime.cpp):2122).

The three final digest/ownership passes over effective.bin are not charged by the explicit eleven embedded or three profile/module payload multipliers; those loops cover their named source payloads. The complete catalog structural charge has a separate stated purpose: the decoded graph and its string payloads. File/path reservations likewise have stated node/path/metadata purposes. The present four-slot recipe does not establish how all six byte-buffer constructions, plus embedded metadata, fit without counting the same reservation twice.

This finding is a **missing logical-capacity coverage proof**, not a demonstrated 512 MiB overrun, RSS measurement, or demand for allocator headers/rounded capacity accounting. Corrective work should enumerate all pure-helper payload/graph passes, reserve their bounds before entry with checked arithmetic, or demonstrate explicit conservative coverage from a documented reservation without double use. Add a bounded high-catalog-byte fixture/probe so the argument is checked beyond the small successful catalog. Do not introduce a new public quota or change the approved accounting metric.

The source trace does confirm `4*2+3=11` embedded payload passes and `1*2+1=3` profile/module passes. Their multiplications are bounded by native component checks and checked cumulative `charge()`. PMR audit requests still charge before allocation with no refund. The finding concerns the remaining canonical synthetic payloads, rather than those correct multipliers.

## Reviewed production boundaries

The native profile constructor redecodes bounded JSON, checks exact root/path/mode and ordered module correspondence, byte limits and source text, then privately copies the captured closure. Physical receipt verification remains a documented later caller obligation; there is no runtime filesystem reload or substitution. Immutable native catalog values and canonical bytes do not expose Lua handles.

Setup constructs the private external environment from pristine curated globals before shipped evaluation. Its `_G` points to itself, its engine has exactly kind/object/array, and its API has exactly add/shape/projection. Exact external API native identities enroll before any source; shipped helper enrollment stays immediate after native installation. Original globals/handlers and the safe external environment audit/freeze together. Embedded handlers retain their full original engine; external closures have no global alias to it. An independent exact-API/ambient-capability probe passes.

Captured source-only chunks execute in declared order in one VM under native logical names, with checked native `_ENV` binding and actual zero-result checking. Active module ordinals assign owners; saved API aliases/pcall during active initialization remain valid. There is no invented immediate-Lua-caller or F-source-equals-owner restriction. Strong D/F references survive until module-end final descriptor conversion/copy and all-root audit/seal before the next chunk. The repeated successful-identity set, private backings, stable iterator and terminal seal-failure rules retain approved A1/A2 behavior. Registration-only invocation refuses before provider work.

The exact pure catalog enforces the three descriptor kinds, native owners, mandatory five commands, reserved identities and external static capabilities. Real shipped shape/projection adapters and B/C semantics remain open as authorized; no dummy ABI or shipped Lua change was introduced. Exact original embedded bytes, captured profile whitespace, ordered module bytes/modes and canonical catalog feed effective provenance. Private retained callable dispatch verifies the effective tree and digest without substituting captured inputs.

The allocator and instruction counter remain cumulative across shipped and external chunks at 64 MiB/10 million. Sticky caught registration/resource failures, retained writer refusal, unknown exact native identity, GC, protected-frame/native unwind and cleanup probes pass. This does not resolve R2/R3 or prove whole-invocation caller receipt/budget orchestration.

## Independent verification and historical evidence

[Independent command record](../../../tests/native/v5-02-x05-a3-review-evidence/commands.json) builds Debug `all` plus `planar_tests`, inventories, executes the exact author-focused selector, recompiles/runs both production probe runners, runs pinned formatting/Doxygen, and requires a no-work build. All seven commands pass. Both builds report `ninja: no work to do.`. Inventory is 570 unique names preserving all 555 baseline names. Focused execution is **197/197**, zero failures/errors/skips/disabled cases, **82.54 seconds**, including all 15 A3 cases. [JUnit](../../../tests/native/v5-02-x05-a3-review-evidence/focused.xml), [matched names](../../../tests/native/v5-02-x05-a3-review-evidence/test-summary.json), [raw output](../../../tests/native/v5-02-x05-a3-review-evidence/focused.log). Pinned lint/Doxygen checks 117 active sources.

Both [author production scenarios](../../../tests/native/v5-02-x05-a3-review-evidence/production-probes/commands.json) and [supplemental reachable-native scenarios](../../../tests/native/v5-02-x05-a3-review-evidence/reachable-native/commands.json) independently recompile and pass eight scenarios plus the catch-all red control each. Review copies change only output roots, with exact reverse equality recorded. They cover actual same-VM retained dispatch through all three kinds after GC; final D/aliases; exact catalog/provenance verification; inactive saved callbacks; direct versus Lua-reachable unknown native identity sharing a C pointer with enrolled helpers; cumulative native exhaustion; constructor/callback unexpected and bad_alloc mapping; and zero tracked live Lua allocation after destruction. The red control intentionally fails its exception-category detector. These passing controls do not supersede the additional failing R1/R2 detectors.

Independent adversarial controls also pass canonical-order field mapping, caught protocol stickiness and exact API capabilities. R1/R2 failing detectors are preserved as review findings, not mislabeled successful product checks or fixture failures.

The author's final 15-command gate/raw logs were inspected: focused197, canonical/record-v1/request-v2/effective vectors, historical B2/D6/D6-C01/D7-B and A0 forward/reverse/exact shipped audits, lint and no-work pass. These vector and historical commands were inspected rather than claimed as independently rerun here. The initial compilation initializer failure and four development fixture failures remain preserved and consistent with the final changes; none explains the independently reproduced R1/R2 defects.

[Independent scope audit](../../../tests/native/v5-02-x05-a3-review-evidence/scope-audit.json) reconstructs the exact owned diff, matches original/candidate copies, and verifies that the previous invocation tail changes only by the profile-owner refusal. Old tests, shipped Lua, vendor and historical support/evidence remain unchanged. The approved scope includes narrow engine and copied-runtime CMake dependencies and new test wiring. AGENTS.md remains the CLAUDE.md symlink.

## Candidate identity and release

[Before](../../../tests/native/v5-02-x05-a3-review-evidence/before.json), [after focused verification](../../../tests/native/v5-02-x05-a3-review-evidence/after-tests.json), and final release audits independently rehash 19,663 candidate source entries with modes/symlinks, 176 author artifacts and 931 executable/object/PCM/cache/build identities: zero mismatches. Review exclusions add only this report and review evidence to the author's root-only .git/build and A3-evidence exclusions.

Author identities remain:

- Source and source-release manifests: `56cdefe4bae53ebccf8b3e28b436592acb15ea08aa16c5c5cadf7da125087130`.
- Owned diff: `0fad899cfdac93df1fa6438c9ecd6d63138af2fdfb66252d20360d30327bfe6b`.
- Author artifact manifest: `cae4e13b1473d031d8ba6a5a697557d95e429810d3e8e1fd1a9a5c8ec05ae170`.
- Author report: `13f26a81cbca4067f4f14f1b50c5da2201303adca6e4828b50df5d4944c560cf`.
- Build identities: `0d8d0ff27c4216cc4ceaa5cdf9884bf678371a2d7eff9c25bbb478d186e3c313`.

Review [finalization](../../../tests/native/v5-02-x05-a3-review-evidence/finalization.json) binds this report and review artifact manifest. Candidate and author evidence stay frozen through release. Serialized build ownership is released after final hashing, with no subsequent reviewer edits.

No full 570 suite, RelWithDebInfo/release or Linux gate is claimed. The latest full gate remains 520 per configuration. A3, the mandatory external-profile gate, B/C semantics/adapters, X06 interpretation/profile-history acceptance, public profile CLI/cutover, execution/RA01 and Linux remain open. Public init creates format1.
