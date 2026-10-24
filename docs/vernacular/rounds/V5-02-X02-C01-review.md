# V5-02-X02-C01 independent contract review

Disposition: **approve**. R1–R3 are resolved, with no blocking findings. The X02
candidate is ready for the coordinator to present its concrete P1/P2 choices to
the operator. Neither choice is selected by this review. Approval is documentation
design approval, not implementation authorization or external-profile acceptance.

Reviewed the shared `dev/planar-v2-vernacular` checkout at HEAD
`3171c49e7b92d3abda01bfbb8bfb0c6b29d4c67a`, CLAUDE.md, the X02/C01 briefs and
original X02 review, both full candidate documents and their exact correction
diff, relevant implementation/roadmap/v5 direction, accepted V5-02 A/B/C, X01
review/spike limits, and the existing command/history/render source bases.
Writes are confined to this report and
`tests/native/v5-02-x02-c01-review-evidence/`. The checkout is shared; no other
agent's files or intentional rewrite changes were reverted or edited.

## Findings disposition

1. **R1 resolved — Exact capability-bound history ID ABI.**
   [X-REG](../external-profile-contract.md#x-reg--registration-and-immutable-ownership)
   lines 243–259 now distinguishes general INT32 `decimal` from `history_id`.
   The latter is optional and permitted only on revision/at-event/after-event for
   `history_read_v1`; a required, renamed, mistyped or wrong-capability descriptor
   is an internal protocol refusal. Tokens are canonical positive decimal through
   INT64_MAX, retained as exact Lua strings with absent `"0"`/present false.
   The [X-PROJ](../external-profile-contract.md#x-proj--pure-projections-and-native-read-capabilities)
   capability table and lines 398–425 give consistent selector exclusions,
   after-event+limit paging, limit 1..100/default 50, absence and request-key rules.
   They require Lua type/presence proof at 2147483647, 2147483648 and INT64_MAX,
   plus overflow/lexical/conflict failures, without widening ordinary decimals.
   This matches the current history parser's signed-64 boundary and exclusions
   (`src/cmd/planar/handlers/history/command.cpp:7–60`) and current invocation
   string/presence/default behavior (`src/engine/runtime/runtime.cpp:1228–1253`,
   `:1303–1319`). It does not change shipped descriptor signatures in the v1
   command contract. X05-C/X07-B/E09 carry the corresponding future evidence.

2. **R2 resolved — Path knowledge phases and P2 consequence are explicit.**
   [X-CLI](../external-profile-contract.md#x-cli--envelope-and-admission-order)
   steps 4–8 and
   [X-CAP](../external-profile-contract.md#x-cap--strict-capture-and-trust)
   lines 180–207 agree: early saved-load equal/conflict lookup precedes profile
   capture; on a miss, known shipped native slots are checked after capture and
   before external evaluation. Opaque extension slots are learned only through
   pure registration/sealing and successful final CLI11 parsing. Every supplied
   late slot must then pass initial checks before handler entry or read-service
   use, even if the handler would skip the service. No shadow argv scan or
   consumption of newly supplied untrusted input contents is introduced.
   Profile/registration failure precedes final extension syntax; final syntax and
   selector conflicts precede late overlap; overlap precedes handler/service.
   Initial receipts, including late receipts, are retained, relationships/aliases
   are rechecked before result authorization and applicable load commit, and no
   Lua runs under the transaction lock. P2 now states plainly that recommendation
   A cannot guarantee separation from late-known inputs before registration.
   The same consequence and tests recur in X04-A/X05-C/X07-A/X07-B and
   E02/E03/E13. This is an honest, implementable proposed phase boundary rather
   than evidence that the future loader already enforces it.

3. **R3 resolved — One removal/report order and exact byte vector.**
   X-PROJ lines 373–380 and 442–467 consistently remove optional custom sections
   in reverse projection result order before existing priorities, append omitted
   IDs in actual removal order, use that order in the notice, and preserve original
   section metadata order with notice last. Independent arithmetic extracted the
   six quoted strings from the candidate and reproduced the 87→94→59-byte
   sequence at budget 60, omitted IDs `["b","a"]`, and metadata byte counts
   11/37/37/48. Omitted entries have no joining LF; the included notice includes
   its joining LF. This agrees with the existing assembly, notice and removal
   behavior at `src/stdlib/planar/render.lua:160–187`. X05-C/E08 require the
   multiple-removal vector as later executable proof; this review ran byte
   arithmetic only.

## Policy readiness and retained limits

Cross-section review found no residual R1–R3 contradiction or new blocking issue.
The inherited X02 design still includes genuine custom shapes within the
recommended existing-role boundary, immutable pure projections and closed native
read capabilities; it preserves explicit ordered profile authority and the
read/projection-only limit on new verbs. The proposed request/provenance/
interpretation versioning, format1/2 coexistence, early/locked replay and recorded
reads without semantic Lua remain separate from current v1 authority and runtime
claims. Public admission stays atomic in X07, with full external acceptance in
X08 before RA01/dependent jobs. No completed R14, Linux or execution evidence is
inferred from this prose.

P1 remains the genuine choice between useful custom shapes within current
worker/reviewer/job fields and opening new schema/role/state vocabulary. P2 remains
the trust/separation choice between physical directory-tree refusal and
driver-protected colocation, now with its registration timing consequence visible.
The two recommendations are concrete enough to present. Neither is selected,
and no additional product-policy choice is needed by these corrections. Record
the operator's disposition and any resulting exact amendments under the stated
fresh-round review discipline before dependent implementation.

## Independent verification and identity

Executed `python3 tests/native/v5-02-x02-c01-review-evidence/verify.py`.
Both current candidate documents equal their frozen candidate-owned copies;
the owned diff regenerates byte-exactly. All 18 correction author-evidence entries,
all 18 original X02 author-evidence entries and all five author source-basis
entries match. Checked all 14,740 baseline regular-file hashes: only the two owned
candidate documents differ, with no unexpected additions or changes outside the
excluded evidence roots. Thus protected contracts, original review/evidence,
source/tests/CMake, v5 input and goldens retain baseline bytes. AGENTS.md remains
a symlink to CLAUDE.md. All 31 candidate local links/anchors resolve.

The repository-relative two-entry candidate manifest SHA256 is
`be85178f268816d082df3d9c1ecee09398a74e1b60cb6a7c7a059a79a9e9155f`;
`owned.diff` is
`abe6c97cb57c6c1721f896e22b8b32813eedc9d425aa5c008329ac672bc410d7`;
the evidence-relative 18-entry author manifest is
`17fff9ffdd99e8ed0201e41c2994fe81a16be5ee4dacb3d68359c54c4c64487d`.
Every identity matched. Independent outputs, source-basis hashes and the
reproducible checker are in the review evidence directory; its repository-relative
manifest excludes itself and this separately hashed report.

No product build/test, CTest, lint, executable schema/parser/Lua verification,
Planar tools/skills/queue, installed binary, operator data, commit, installation,
publication or merge occurred. The author's 57 assertions are textual checks,
not runtime evidence; this review does not promote them to runtime proof. The
405 inventory and R14 402/402 gates remain prior evidence. Native sealing,
confinement, codecs, races, Linux, execution, RA01 and mandatory external-profile
runtime acceptance remain unproved by this round. Candidate documents were not
edited. Reviewer ownership is released; both build directories were untouched
and remain idle.
