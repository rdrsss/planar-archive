# X05-B4-A pure custom-shape rendering candidate

Candidate for fresh independent review; no self-approval or full B/release gate.
Starting HEAD: `3171c49e7b92d3abda01bfbb8bfb0c6b29d4c67a`, shared
`dev/planar-v2-vernacular`. Coordinator owns the round. No commits, Planar tools,
installation, publishing, merging or operator data were used.

## Exact candidate and baseline

`baseline.json` explicitly identifies a post-edit reconstruction, not a pre-edit
snapshot. Inverse edits reconstruct original render/bootstrap; the new test tail
is removed; unchanged prose was copied before edits. All nine resulting SHA256
identities match the independently approved B3-C `frozen-source.json` exactly.
`baseline/` retains those verified original bytes. `candidate.diff` is incremental
against that baseline; `candidate-manifest.json` binds final owned bytes.

Product/test/support/current prose were frozen before the final gate. Finalization
rehashes every frozen file and all prior unowned frozen source/support files.
This includes unchanged native runtime/interface, selection, schema, semantic,
observations, presentation, codec, vendor/build configuration and prior support.
All fixture render bytes, including six shipped goldens, are also frozen. Historical
evidence and support are unmodified. No private compiler command was required or
executed: direct production `run_trusted` tests call the actual helper. No private
observer, frozen-native transport or same-call native pipeline proof is claimed.

## Behavior and exact private ABI

`render_profile_material({request,model,selected})` is a product-owned structural
helper. `request` contains the existing original render fields, including optional
budget; `model` is a finalized semantic model; `selected` is a finalized selection
value (without its outer `{value:...}` wrapper). The caller must provide authentic
corresponding results. Logical request/job/role/workspace/spec/shape ID/revision/
definition checks are defensive checks, not authentication of arbitrary tables.

The helper normalizes original render intent and calls the shared rendering body.
It does not validate bundles, run selection or F, mutate supplied model/selection,
execute projections, add public flags or claim native same-call ownership. Old
`render` retains its normalization, selection, validation and observation flow.

Custom rendering returns exactly version1 `render_external`, packet, manifest and
three-field shape identity. The manifest adds the same identity, versions remain
amended W.versions plus `projection_v2`, and packet stays structuralversion1.
Scoped obligations copy all normalized fields, add unverified status and sort with
old prerequisites. The mandatory `shape_policy` follows dependencies and uses the
existing presentation/Glaze helpers, labels, fences and control escaping. It uses
model guidance and scoped W obligations. Mandatory budget accounting and optional
removal order remain shared with old rendering. No completion/evidence is invented.

## Independent expectations and focused evidence

The new policy Markdown and complete prerequisite JSON are authored from X-RENDER
and the selected fixture declarations, before invoking rendering. Tests independently
assert exact envelope fields, shape identity, versions, section order/mandatory
flags/byte totals, prerequisite records, exact guidance escaping/fences, and
boundary budget behavior. They do not call output generation to create expectations.
These are scoped policy goldens and structural assertions, not a new full-envelope
self-generated golden.

Ten new production helper cases cover custom worker/reviewer structure, repeatability
and caller snapshot stability; all shipped roles through the helper; mismatched job,
role and shape identity; exact mandatory byte boundary/one-byte refusal; empty,
single and grouped obligations; guidance versus W identity and control/backtick
fencing; observation error priority; optional omission order; scoped nonincident
obligations; worker/reviewer recovery checkpoints. Native semantics/selection may
supply setup through separate operations. This is pure helper composition only.
Snapshots demonstrate unchanged caller structures, not a proof of B4-B's future
same-owner immutable Lua transport. Source inspection confirms the helper only
reads supplied nested structures and builds fresh merged prerequisite lists.

The first guidance test assumed semantic job order and altered the other job.
`development-render02.xml` retains that test setup failure. Corrected setup changes
each job's guidance explicitly; `development-render03.xml` and the expanded ten-case
`development-render04.xml` pass. No product fix was needed for that test correction.

## Frozen Debug gate

`final-gate/commands.json` records exact argv, status and elapsed time. The completed
frozen gate passes inventory665 preserving every prior655 name, focused399/399
(the prior389 plus ten new), supplemental11/11, and direct helper10/10, all with
zero failures/errors/skips. Pinned formatting/Doxygen checks118 active C++ sources. Focused coverage includes all six established quota
render goldens. The six established vector/oracle checks and six applicable exact
source audits run unchanged, together with pinned formatting/Doxygen and a no-work
build. `checks.json`, `finalization.json` and `artifact-manifest.json` bind raw logs,
XML and hashes after the gate; no full665-case or release run is claimed.

B4-B retains native same-call authentication/composition, immutable transport,
retained render root, accounting and lifecycle proof. B4-C retains the full integrated
Debug/RelWithDebInfo gate after native rendering approval. External projections,
public profile admission, X06–X08, execution/RA01 and Linux remain open. Public init
still creates format1. Debug is released after finalization; no edits then remain.
