# Computed native quota packets

The current R10-P01 `projection_v2` packets capture the unchanged R03 workspace,
registry and source snapshots with
`build/rewrite-debug/bin/planar`. Each `.json` is the complete actual CLI envelope
in canonical JSON plus LF; each matching `.md` is its exact packet Markdown.
`reviewer-resolved.json` is the original symbolic reviewer observation with only
`$O_COMMAND` and `$O_LIBRARY` replaced by computed current unit outputs. The
original `expected/`, `manifests/*.symbolic.json` and `observations/` remain
unchanged. These previews establish no check result, completion or approval.

`tests/acceptance/bootstrap_cli.py /absolute/path/to/built/planar` reproduces the
six invocations in fresh processes and compares exact bytes. It captures the
fixture in a temporary sibling layout, computes reviewer targets first, and
supplies `observations/empty.json`, the resolved reviewer input, or the original
recovery checkpoint as appropriate. No symbolic template is passed as a valid
input. The explicit `--write-goldens` developer option writes these files; normal
acceptance refuses drift and never updates them.

## Comparison with R03 authored expectations

| Packet | Original Markdown bytes | Native Markdown bytes |
| --- | ---: | ---: |
| combined-builder | 12951 | 12496 |
| combined-reviewer | 13761 | 13311 |
| split-library_worker | 8013 | 7542 |
| split-cli_worker | 10958 | 10543 |
| split-reviewer | 13780 | 13332 |
| split-cli_worker-recovery | 11512 | 11121 |

The executable comparison checks exact goal, selected-unit summary, scoped
constraint/prescription, milestone acceptance, expected contract and contract
review-note text against both the original authored packet and actual output.
It checks every selected ownership boundary and complete selected current/base
source text (including subscribed facet files) in both. It also checks each complete check/toolchain field, exact argv/input/environment
arrays and reviewer grants. All six comparisons pass.
The comparison deliberately retains original prose expectations; changing a
native snapshot cannot satisfy a missing source-prose or ownership check.

Presentation differences retain R08-A selection and escaping (including the
[R08-A correction](../../../../docs/vernacular/rounds/R08-A-C01-review.md))
and apply the R10-P01 readable presentation:

- All symbolic identities become computed native identities. Full envelopes
  retain exact normalized material W, retained-file hex/modes/provenance,
  captured trees, exact target vectors and section byte accounting. Recovery
  preserves the CLI worker's W and role output, while its fragment changes.
- Checks have separate labeled toolchain, timeout, output cap and selection
  fields; exact argv/input arrays and complete coverage prose are fenced. Each
  toolchain appears once with exact command argv/environment and full description.
  Small arrays and assertions/preferences still use Glaze canonical JSON with
  sorted keys and explicit string-control escaping labels. All exact structured
  definitions remain unchanged in the manifest.
- Source text remains verbatim inside dynamic fences. Source labels include
  logical root/path, normalized mode and byte length; computed content identities
  remain in the manifest.
  General rendering supports visible control escaping and binary hex; these
  unmodified quota source files are text and need no escaping. Native sections
  state added/modified/unchanged/deleted status from captured pairs, and give
  complete selected contents, including subscribed upstream facet contents.
- Dependency instructions come from normalized policy: producer/consumer roles,
  start/complete phase, available/unverified quality, contract/facet descriptors,
  internal obligations and exact output-vector instructions. Each dependency is
  shown once; facet paths/availability, pipeline and question prerequisites remain
  explicit without repeated policy JSON. Grouping changes
  worker grants and mapping while retaining identical per-unit output targets.
  Reviewer packets describe target boundaries and grant no source write paths.
- Boundary prose lists frozen/excluded declarations without the R03 editorial
  parentheticals about their absence. Captured inventories demonstrate absence;
  the declarations still apply. The renderer's repeated authority labels and
  explicit empty-collection notices replace R03 hand-written explanatory prose.
- Reviewer assertions contain the exact resolved target/vector identities and
  remain labeled `driver assertion, unverified`. The recovery checkpoint remains
  a session note with unavailable history and no active run. No authored
  inconclusive assertion becomes a successful receipt or approval.

The acceptance oracle uses the independent contract encoder from
`tests/vectors/generate.py` and a scratch shared library compiled from pinned
upstream XXH3 C. It recomputes every file, W, fragment, contract, base,
observation, unit-output and worker role-output digest. It separately checks
canonical JSON, every included section's UTF-8 byte count and joins, all file
hex bytes, cross-job unit-target equality, reviewer target equality and recovery
stability. This is native debug macOS integration evidence; it is not the R10
operator evaluation or a claim of release/Linux verification.

## N03-B current Glaze wire format

The six current envelopes now use Glaze short C0 escapes and uppercase control
hexadecimal, with sorted UTF-8 keys and no extra whitespace. The native-before
copies and `golden-identity-diff.json` in `tests/native/n03b-evidence/` preserve
exact previous/current SHA256 and byte counts. All six parsed envelopes are
identical: W, per-unit outputs, worker role outputs, observations, Markdown and
render identities are unchanged. Only outer JSON string spelling changed here;
these fixture descriptors contain no C0 requiring a Markdown spelling change.
Other descriptor inputs with controls may change Markdown bytes and therefore
render identity, while binary canonical_v1 and output identity remain unchanged.
The original R03 inputs/expectations and frozen R10 evidence remain untouched.

## R10-P01 scoped presentation

Authority retains one full work fingerprint and registry provenance. Worker unit
and role output IDs remain visible; reviewers see the complete ordered target
vector once in outputs. Global observation/base and file/facet content digests
remain in the manifest rather than the Markdown. Output/checkpoint sections state
capture status and show the driver base label as fenced data. Driver assertions
retain exact targets/vectors and remain unverified. An unrelated command edit
now leaves the library worker fragment stable while global observation changes.

`tests/native/r10p01-evidence/before/` retains all six exact prechange envelopes
and Markdown. `golden-comparison.json` records each before/after size, complete
policy/coverage counts and exact invariance: only manifest `versions`, `render`
and `sections` differ. The helper uses actual CLI output; no source declaration,
original R03 expectation, symbolic input or historical evidence is regenerated.
This change claims neither check execution nor persistence/history.
