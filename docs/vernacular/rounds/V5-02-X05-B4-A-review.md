# V5-02-X05-B4-A independent review

Disposition: **request-changes**. B4A-R1 remains open. The pure rendering contract
is otherwise supported by focused and independent evidence; B4-A is not approved
until a fresh corrective round fixes and reviews the job correspondence check.

The review used shared `dev/planar-v2-vernacular`, starting HEAD
`3171c49e7b92d3abda01bfbb8bfb0c6b29d4c67a`, under the active guide, implementation,
roadmap, v5 direction, exact X-SHAPE/X-RENDER, original render/presentation contract,
B4 split/authorization and approved B3-C. The checkout was shared; others' work
was preserved. Writes are confined to this report, the owned
`tests/native/v5-02-x05-b4-a-review-evidence/` and serialized ordinary Debug
verification outputs. Product, author support/evidence, historical evidence,
vendor and release outputs remained read-only. No fixes, Planar tools/legacy
skills/roles/queues, installed binaries, operator data, commits, installation,
publication or merge were used. Debug ownership was exclusive.

## B4A-R1 — Reject a semantic job record belonging to another job

[The helper](../../../src/stdlib/planar/render.lua#L237) finds the normalized
bundle job by request ID, then takes `model.jobs[i]` by position. Its
correspondence predicate checks the request against C/W and the selected shape's
ID/revision/definition, but never checks `job_model.id`. Two jobs using the same
shape therefore satisfy every current check even when the semantic job records
are reversed. The other job's guidance is silently attached to the selected job.
The observations helper also relies on that positional semantic job mapping.

The [production reproducer](../../../tests/native/v5-02-x05-b4-a-review-evidence/adversarial.cpp#L100)
uses actual `validate_profile`/`select_profile` results as separate pure-helper
setup. Both `combined` and `split` use the same native shape definition; F returns
`input.job.id` as guidance. Rendering `split/library_worker` normally displays
`split`. Reversing only the two `model.jobs` records still successfully renders
`split/library_worker`, now displaying `combined`, with unchanged matching shape
identity. The [actual output](../../../tests/native/v5-02-x05-b4-a-review-evidence/correspondence-result.json)
and [expected-refusal failure](../../../tests/native/v5-02-x05-b4-a-review-evidence/correspondence.xml)
are retained. Both explicit guidance checks pass; the required `render.selected`
refusal fails. This is a structural defensive-check gap, not a claim that the
future native authenticated pipeline currently permits caller substitution.

The round explicitly requires defensive matching-job correspondence and forbids
silently attaching another job's shape metadata. Require the chosen semantic job
record to identify the requested/normalized job before using its shape or passing
it to observations. Preserve the existing typed `render.selected` refusal and
input nonmutation. Add a regression with distinct per-job guidance but identical
shape ID/revision/definition, covering swapped records and a mismatching semantic
job ID, alongside ordinary correctly aligned results. Native same-call
authentication remains B4-B; it does not replace this explicit B4-A requirement.

## Rendering and input ownership

The shared body preserves original normalization, selection, validation and
observation ordering for shipped render. Custom output matches the exact
five-field `render_external` envelope and three-field outer/manifest shape
identity. Manifest versions copy amended W.versions and append only
`projection_v2`; packet stays structural version1. Mandatory `shape_policy`
follows dependencies and uses the exact approved labels, existing Glaze JSON,
dynamic fences and display escaping. Shape prerequisite records copy every
normalized obligation field, add `unverified`, and merge/sort by ID with existing
prerequisites. Empty/singleton arrays retain tags. No completion or acceptance
claim, ordinary source-enum change, external projection or public flag is added.

The helper reads model/selection; new prerequisite arrays/shape records/version
lists are fresh. The old prerequisites are read and reused without mutation.
Budget calculation, section byte totals, mandatory refusal, omission order and
observation error priority remain shared with old rendering.

Two [independent production cases](../../../tests/native/v5-02-x05-b4-a-review-evidence/adversarial-final.xml)
pass **159 assertions**. They exercise eleven C/W/model shape correspondence
refusals, correctly aligned retry, captured worker/reviewer rendering, mixed
original/custom prerequisites with complete records and lexical order, real
role-visible driver findings and recovery notes, repeated exact output and caller
snapshots. The first reviewer fixture incorrectly assumed the upstream worker
had original prerequisites and that upstream findings were invisible to a
consumer. The original failed XML/source is retained; only the reviewer fixture
was corrected to the existing scoped dependency rules.

The [isolated reversible probe](../../../tests/native/v5-02-x05-b4-a-review-evidence/transformations.json)
uses the unchanged production `freeze_graph` on supplied model/selection before
calling the actual helper. It installs the existing frozen read helpers and
lets the private test converter consult the backing map. Protected direct write
controls fail on each frozen root. All ten author helper cases and the two
independent production cases still pass with deeply frozen inputs. This proves
pure helper compatibility/nonmutation in that probe; it does not prove B4-B's
future native pipeline, audited retained render root, transport or accounting.

The same private probe also embeds the exact approved B3-C old render body and
compares complete envelopes against the candidate shared body for **150** shipped
combinations: five valid job/role pairs, capture on/off, absent/malformed/recovery
observations and five byte budgets. Error envelopes are compared too. The
[combined private run](../../../tests/native/v5-02-x05-b4-a-review-evidence/variant.xml)
passes **13 cases / 976 assertions**, zero failures/errors/skips. The initial
private driver compile exceeded the compiler's required literal length; its log
is retained. The final private compile disables that warning for the owned
112638-byte probe source only. Product build flags remain unchanged.

## Focused verification and candidate identity

The [independent Debug gate](../../../tests/native/v5-02-x05-b4-a-review-evidence/commands.json)
passes **399/399** focused cases, **11/11** supplemental history/codec cases and
**10** direct helper cases / **333 assertions**, with zero failures/errors/skips.
Inventory **665** preserves every prior **655** name. Six established vector
families, six applicable exact source audits, pinned formatting/Doxygen for
**118** active sources and no-work builds pass. The focused cases retain all six
shipped render goldens byte-exact. This is not a full 665-case or release gate.

All **15** candidate members and **66** author artifact entries match their
manifests. Independent diff reconstruction from the nine baseline files and new
members exactly matches `candidate.diff`. Every reconstructed baseline file
matches the independently approved B3-C frozen SHA256. The baseline is truthfully
labeled a post-edit inverse reconstruction, not an invented pre-edit capture.

| Reviewed author identity | SHA256 |
| --- | --- |
| Candidate manifest | `86f2ee5a919d3f2a6f14cf8b71dfe4ad66adbc1ed194ae29fe72e6ea0bd633a7` |
| Candidate diff | `753408bc7641c721f4019a71c22e82940f38b8ef141bd3b3a8cba03f1ed07588` |
| Artifact manifest | `125d42a25234407f7e923f66f0a40f93153f684dc6d0d90e8c84588719671adb` |
| Author report | `c98c6324003afd2d60feb82a50eef152d2376b66e75e19b3088fcc04d567977d` |

Private compilations recursively expand response files and redirect object,
dependency and possible PCM outputs into the owned evidence directory. Before
and after each successful compile, exact shared compiled membership/hashes are
unchanged; no shared library is replaced. Final scope verification rehashes the
**5398** protected source/vendor/fixture/release/author entries, all frozen-source
and protected-source members and the candidate identities. AGENTS remains the
CLAUDE symlink. [Finalization](../../../tests/native/v5-02-x05-b4-a-review-evidence/finalization.json)
binds this report/artifacts and releases Debug with no later reviewer edits.

B4A-R1 requires a fresh corrective search/coder/reviewer round. B4-B native
composition/immutability/accounting/lifecycle and B4-C full integrated
Debug/RelWithDebInfo remain required. X06–X08, external projections, public
cutover, execution/RA01 and Linux remain open. Public init creates format1;
latest full approved gate remains A4's 582 per build.
