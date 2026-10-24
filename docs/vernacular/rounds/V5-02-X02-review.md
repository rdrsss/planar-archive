# V5-02-X02 independent contract review

Disposition: **request-changes**. Three exact-contract issues need a fresh
corrective round before presenting P1/P2 as request-ready. This is a docs-only
review; it selects neither policy and authorizes no implementation.

Reviewed the shared `dev/planar-v2-vernacular` checkout at HEAD
`3171c49e7b92d3abda01bfbb8bfb0c6b29d4c67a`, CLAUDE.md, implementation/roadmap,
X02 scope/freeze, both candidate documents, approved X01 spike/review and relevant
parser source, accepted V5-02 A/B/C/proposal, v5 line 571, and relevant exact
declaration/render/command/load/ledger contracts and native/Lua seams. Writes are
confined to this report and `tests/native/v5-02-x02-review-evidence/`. The checkout
is shared; other work and the intentional rewrite were preserved.

## Requested changes

1. **R1 — History extension decimal ABI contradicts the advertised history
   capability.** [X-REG](../external-profile-contract.md#x-reg--registration-and-immutable-ownership)
   lines 209–213 admits every external decimal only through 2147483647 and
   exposes it as an integer, whereas X-PROJ line 342 requires the history options
   to retain INT64_MAX and ID-string conversion. An extension history command
   with `--revision 2147483648` cannot satisfy both. The current
   [command contract](../command-contract.md#invocation-values-and-finite-native-service-protocol) lines
   173–178 deliberately distinguishes ID strings from signed-32 budget/limit
   integers; the actual history CLI parses signed-64 IDs
   (`src/cmd/planar/handlers/history/command.cpp:7`, `:47`). Define an exact
   native-owned capability-specific exception or a distinct descriptor type,
   including absent values and Lua representation, without relaxing arbitrary
   extension integers. Pin 2147483647/2147483648/INT64_MAX/overflow and selector
   conflict cases in X05-C/X07.

2. **R2 — The overlap timing is incomplete for external read verbs.** X-CLI
   lines 99–104 executes external registration before the final parser can know
   an extension's descriptor and native path slots. X-CAP lines 151–174 says
   supplied roots/files are checked before external evaluation using known CLI
   slots, but never specifies the mandatory initial check when slots become
   known only after that registration. For example, an external role-read verb
   may supply a workspace/source/observation overlapping its profile directory;
   its tail is intentionally opaque before the catalog exists. The current
   sequencing cannot establish separation from those inputs before registration.
   State the exact phase and error precedence for these late-known slots, require
   initial checks before handler/read service use and receipt retention/rechecks,
   and state explicitly whether registration may precede them. If P2 A is meant
   to promise separation before *any* external evaluation for these verbs, supply
   a compatible pre-evaluation admission mechanism; do not implement a shadow
   argv parser. Carry this timing consequence into P2 and X04/X05-C/X07 acceptance
   so the operator can assess the actual separation guarantee.

3. **R3 — Projection omission order has two incompatible definitions.** X-PROJ
   lines 325–327 removes optional custom sections in reverse result order and
   uses the current omission notice/count algorithm, but lines 365–366 requires
   omitted IDs in original order. For optional sections `[a,b]`, removing both
   produces `[b,a]` under the former and `[a,b]` under the latter. The current
   algorithm appends IDs as removed and uses that same order in the notice
   (`src/stdlib/planar/render.lua:160`, `:183`). Choose a single exact ordering
   for response IDs, notice bytes and metadata; explicitly distinguish removal
   priority from reporting order if they differ. Add a multiple-omission byte
   vector, including joins and the omission notice, to X05-C.

## Assessment and limits

The draft keeps explicit selection, ordered modules, no discovery/overrides or
job authority, fixed help and the approved X01 optional positional probe. Saved
load replay precedes external capture/Lua and CAS. It includes actual custom
shape validation/obligations within current roles, selected definition identity,
pure projections and closed read-only extension services rather than silently
deferring shapes. Old shipped W recipes remain separate from the new custom
domain. Request-v2 framing, effective provenance, versioned interpretation,
format1/2 coexistence, new-target init2, no conversion, recorded reads without
semantic Lua, native receipts and transaction lifetime have concrete proposed
boundaries. The implementation path covers these through bounded fresh-agent
rounds and an atomic public admission gate. These observations are design review,
not executable proof of sandbox sealing, codecs, confinement or race behavior.

P1 is a genuine product-scope decision: current worker/reviewer/additive semantics
versus new schema/role/state vocabulary. Recommendation A supplies useful custom
shapes for current render/load while execution/review-loop work stays future.
P2 is a genuine trust/separation decision; its recommendation is consistent with
the proposal's physical-overlap direction, but R2 must make its phase guarantee
concrete before operator disposition. The format2 cutover and byte/interface
mechanics do not need an invented additional public format-policy question.
Neither P1 nor P2 is selected by this review.

Independently verified all eight candidate entries, all 18 author-evidence entries
and all 14,714 baseline hashes. No unexpected changes/additions were found.
Command exact ABI text from `## Catalog` onward is unchanged; other protected
contracts, source/tests/CMake, historical evidence and goldens retain baseline
bytes. All 27 new-document links/anchors pass. AGENTS.md remains a symlink to
CLAUDE.md. The author's 30 checks are textual assertions, not schema execution.

The repository-relative flat candidate manifest SHA256 is
`67e3a6fa174d3f7a526e52d1717cb21f617655ebad65b8788d19b2ca264867bf`;
owned.diff is
`400139e1f9686a00109cfea78c8530ce2639a36883e8e9461f567dc697e29f9b`;
the 18-entry evidence-relative author manifest is
`6444e7ff1fbcb02ba80a0682af6fca573d5a52d9d5d0bf67e524743c5fa7aabd`.
Every identity matched. Independent verification and source-basis hashes are in
the review evidence directory; its manifest uses repository-relative paths and
excludes itself.

No builds, tests, runtime/schema execution, lint, CTest inventory, Planar
tools/skills/queue, installed binaries, operator data, commits, installation,
publication or merge occurred. The 405 inventory and prior R14 402/402 gates
remain prior evidence. Linux, execution, RA01 and the mandatory external-profile
runtime gate remain open. Candidate documents were not edited. Reviewer ownership
is released; debug/release were untouched and remain idle.
