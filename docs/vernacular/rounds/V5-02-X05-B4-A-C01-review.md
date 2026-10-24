# V5-02-X05-B4-A+C01 independent review

Disposition: **approve**. No blocking finding. The combined pure rendering
candidate is approved, and C01 closes **B4A-R1**. Native composition and the full
integrated B gate remain later work.

This fresh review used shared `dev/planar-v2-vernacular`, HEAD
`3171c49e7b92d3abda01bfbb8bfb0c6b29d4c67a`, under the active guide,
implementation, roadmap, v5 direction, exact X-SHAPE/external-render and original
render contracts, B4-A/C01 authorization, original B4-A review and approved B3-C.
The checkout was shared; others' work was preserved. Writes are confined to this
report, `tests/native/v5-02-x05-b4-a-c01-review-evidence/` and serialized ordinary
Debug verification outputs. Product, author evidence, historical evidence, vendor
and release outputs remained read-only. No fixes, Planar tools/legacy
skills/roles/queues, installed binaries, operator data, commits, installation,
publication or merge were used. Debug ownership was exclusive.

## B4A-R1 closure

The [corrected helper](../../../src/stdlib/planar/render.lua#L239) checks object
model/bundle, array containers, equal cardinality, object rows/values, valid IDs,
positional equality and uniqueness across the complete bundle/semantic mapping.
Only after that loop does it read the selected shape. This preserves the order
already used by semantic generation, selection and observations, and prevents a
matching shape definition from disguising another job's semantic record. It
neither falls back to ID lookup nor mutates/revalidates the supplied material.
Existing request/C/W job, role, workspace, spec and shape identity checks remain.
Every newly rejected correspondence uses the exact existing
`schema` / `render.selected` / `Finalized render material does not match request`
diagnostic.

The author's actual pre-edit baseline preserves the original B4-A product bytes.
Its [red production regression](../../../tests/native/v5-02-x05-b4-a-c01-evidence/red.xml)
records the required refusal failing with unchanged production helper; the
original review's wrong-guidance result and failed assertion remain immutable.
The regression process exit was not independently captured by the author, and
the XML is the red evidence. The fresh review reran the original independent
swapped-job reproducer successfully against the correction.

The [independent correspondence run](../../../tests/native/v5-02-x05-b4-a-c01-review-evidence/correspondence.xml)
passes **2 cases / 491 assertions**, covering wrong/swapped/missing/duplicate and
malformed selected/unselected IDs, containers, rows and wrapped values, equal and
unequal array lengths, exact refusal and nonmutation. The fresh
[reviewer controls](../../../tests/native/v5-02-x05-b4-a-c01-review-evidence/adversarial.cpp)
add native semantic/selection setup with three jobs and the selected job in the
middle: mismatching/reordered trailing or unselected records and matching
duplicate unselected IDs on both sides still refuse. Matching malformed IDs
(boolean, 65-byte, leading underscore, punctuation, whitespace, NUL and non-ASCII)
refuse; valid one-byte/64-byte IDs and a healthy single-job mapping render and
repeat exactly. A single-job malformed observation still precedes mandatory
budget refusal. These fresh cases join the prior independent context/shape
identity and captured worker/reviewer/driver-assertion controls: **5 cases / 364
assertions**, zero failures/errors/skips.

The initial reviewer three-job fixture omitted the extra declaration from its
retained byte inventory. Existing selection correctly refused `selection.files`
before rendering. The initial source/XML/commands are preserved; only the
reviewer fixture inventory was corrected. This was not a product finding.

## Combined pure rendering contract

Review of the full [shared body](../../../src/stdlib/planar/render.lua#L1), against
the approved B3-C old body, confirms the original render normalization,
selection, validation and observation ordering. Custom rendering returns exactly
the five-field `render_external` envelope and three-field outer/manifest shape
identity. Packet remains structural version1. Manifest versions copy amended
W.versions and append only `projection_v2`, with no duplicated shape version.
Mandatory `shape_policy` follows dependencies and uses the exact approved labels,
Glaze JSON, dynamic fences and display escaping. Scoped shape obligations copy
every normalized field, add `unverified` and merge/sort with original
prerequisites by ID. Empty/singleton arrays remain tagged. No completion or
acceptance claim, ordinary source-enum extension, external projection or public
profile flag is introduced.

Input model/selection are read only. Prerequisite arrays, shape identity and
version lists are freshly constructed; existing records are never written.
The body retains exact byte accounting, mandatory policy inclusion, mandatory
overflow diagnostics, optional removal order and observation-before-budget
priority. The existing exact custom-policy expectations and six shipped render
goldens pass independently.

The NEW owned [reversible native probe](../../../tests/native/v5-02-x05-b4-a-c01-review-evidence/transformations.json)
uses unchanged production `freeze_graph` and frozen read helpers for supplied
model/selection, with direct-write controls required to fail. It runs all 12
production helper cases and all five independent cases on deeply frozen inputs.
It also embeds the exact approved B3-C old render bytes, SHA256
`4466309435ed41a0db44b7dacc407d5980803e0a81d23d5bbb126e65c27ae364`, and compares
complete envelope/error bytes for **150** shipped combinations across five
job/role pairs, capture on/off, absent/malformed/recovery observations and five
budgets. The [combined run](../../../tests/native/v5-02-x05-b4-a-c01-review-evidence/variant.xml)
passes **18 cases / 1672 assertions**, zero failures/errors/skips. The owned
native copy reverses exactly to production source. This establishes pure helper
compatibility and nonmutation in the probe; native authenticated same-call
composition, immutable transport, retained render root, cumulative full-render
accounting and lifecycle remain B4-B.

## Verification and immutable candidate

The [independent Debug gate](../../../tests/native/v5-02-x05-b4-a-c01-review-evidence/gate/commands.json)
passes **401/401** focused cases, **11/11** supplemental history/codec cases and
**12 production helper cases / 824 assertions**, with zero failures/errors/skips
or disabled selected cases. Inventory **667** preserves all prior **665** names.
Six established vector families, six applicable exact source audits, pinned
formatting/Doxygen for **118** active C++ sources and no-work builds pass. This
is not a full 667-case, release or milestone gate.

All **15** candidate entries, **85** author artifact entries, **15** frozen source
entries and **23116** protected entries match. Artifact paths were resolved from
the repository root. Independent diff reconstruction from the eight actual
baseline files and new members exactly matches the incremental `candidate.diff`.
Both pre-edit product files match the preserved original B4-A candidate. The
combined review also checks bootstrap and the old-body/expectation bytes protected
outside the incremental C01 manifest.

| Reviewed author identity | SHA256 |
| --- | --- |
| Candidate manifest | `c2079360e46668aa3b4bda634bcbeaeca895ced1893fac63340cd91026feaff0` |
| Incremental candidate diff | `34e6442a62a2e742b92a42216b89cdee69deb751c3df7e2c4d8db96b3ee2a048` |
| Artifact manifest | `08afead3b8f159862013c393485e5b6ca0e511012be6c416f37394662863ef1b` |
| Author report | `ef9d076b9c15a983639f0db7d49e9ea9b503c35e6510b4044e7ce358e341c784` |

Private compilations recursively expand response files and direct object,
dependency and possible PCM outputs into the owned evidence directory. Each
successful compile retains exact shared object/PCM/library membership and hashes;
no shared library is replaced. Finalization rehashes candidate, author and
protected/historical entries, plus the complete **3463-file** release snapshot.
AGENTS remains the CLAUDE.md symlink. The reviewer artifact manifest and
`finalization.json` bind this report and the raw evidence, release Debug and
permit no later reviewer edits.

Approval closes combined B4-A+C01 and B4A-R1 only. B4-B native composition and
B4-C full Debug/RelWithDebInfo remain required. X06–X08, external projections,
public cutover, mandatory external-profile acceptance, execution/RA01 and Linux
remain open. Public init creates format1; latest full approved gate remains
A4's 582 per configuration.
