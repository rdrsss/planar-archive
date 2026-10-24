# V5-02 independent proposal review

Verdict: **approve** — suitable for the operator's A/B/C contract decisions.
No blocking findings. This approves the frozen proposal, not those product
choices, exact-contract amendments, implementation, extension safety or R14
completion. No supported API changes in this round.

## Reviewed candidate and scope

Reviewed only the final 393-line
[proposal](../v5-command-workspace-proposal.md), SHA256
`c16cf5d4aefa7b947fdb176c93ce8753a9ffd83ecd0db6f1559c0f9fa55b8b66`.
The [candidate diff](../../../tests/native/v5-02-evidence/candidate.diff) is
`dae9cdafdd746222113b28abd6ce330c36d9b3fe7e35ec4e552e60f3c8d8fd5f`;
its added hunk reconstructs the current proposal exactly. The coder's
[manifest](../../../tests/native/v5-02-evidence/manifest.json) is
`60cc071f6b49ff2b2049f8883d74cc12cff7d7de531e847e910e33d8f8916a76`.
The retained draft-01 was checked for evidence integrity, not reviewed as the
candidate. Starting/current HEAD is `3171c49e7b92d3abda01bfbb8bfb0c6b29d4c67a`
on `dev/planar-v2-vernacular`.

This shared checkout contains the pre-existing intentional rewrite. The reviewer
owns only this report and `tests/native/v5-02-review-evidence/`; no candidate,
product, prior evidence or coordinator records were edited or reverted. No
Planar tools/skills, queues, builds, product tests, installed binaries, operator
data or commits were used.

## Substantive assessment

- **Authority and sequencing:** The proposal preserves declaration/render/ledger
  v1 and the approved R14-D2 checkpoint. Sections 1, 5 and 7 explicitly require
  reviewed amendments before dependent code. Embedded-only R14 is a recommended
  stage; the external gate remains a required delivery obligation. The three
  A/B/C choices each state a concrete recommendation and alternative. Approval
  does not resolve any of them.
- **Lua and native boundary:** Sections 2–4 make Lua registration consequential:
  descriptors resolve actual semantic callables, normalization/validation and
  projection policy remain Lua-owned, and native families retain CLI11 option
  state and typed orchestration. Descriptor/callable/module/bundle sealing in
  one retained invocation-local VM prevents a later global-name lookup from
  replacing the reviewed handler. Unsupported families/capabilities refuse;
  external verbs initially receive read/projection services only. Job-tier code
  receives neither registration nor capability objects. This is compatible with
  the current fixed embedded exports in `src/engine/runtime/runtime.cpp`, while
  correctly identifying the necessary new registry rather than claiming it exists.
- **Native invariants:** The proposal retains opaque capture receipts, native
  canonical encoding and exact collision refusal, structural verification,
  native CAS/commit, and frozen Lua output before a write transaction. There is
  no generic SQL or unchecked commit binding. This fits
  `src/engine/acceptance/prepared.cppm` and `prepared.cpp`'s receipt ownership and
  provenance/interpretation checks, and
  `src/engine/ledger/format.cpp::complete_acceptance`'s replay-before-CAS and
  candidate inspection. No Lua runs under the native write lock.
- **Parsing and transport:** Sections 2–4 preserve the current bootstrap/family
  lifetime pattern, CLI11 coercion/metadata checks, separate-token syntax and
  native help/exit policy. Responses remain typed and Glaze-encoded. Vendored
  CLI11 documents `prefix_command()` and `remaining_for_passthrough()`; the latter
  returns the reverse-order vector expected by its vector parser. This supports
  the proposed two-phase envelope as a plausible design, not proof that the
  exact parser works. Section 3 explicitly blocks adoption on a focused spike
  for ordering, duplicates, missing values, terminators and profile-looking
  option values, with design review required on failure. No shadow parser is
  authorized.
- **Extensions and identity:** Explicit driver selection, deterministic module
  order, collision/override refusal, captured exact profile/module bytes,
  bounded registration/execution, logical diagnostics and trust limits are
  concrete. The complete sealed bundle and ABI enter retained provenance.
  Sections 5 and 7 recognize that current request framing,
  `interpretation_v1` and embedded ordered-concatenation provenance cannot simply
  accept extensions. New codecs/tags, format reader support, mixed-version
  conflict and scope-specific identity recipes must be reviewed before the
  external gate. Keeping raw bundle provenance separate from work freshness
  agrees with the existing materiality contract.
- **Replay and history:** The profile selector is lexical request intent, not
  captured bytes. The shipped-signature probe enables saved-request lookup
  before opening/evaluating external code; identical intent can replay after
  profile/modules disappear, and different selectors conflict. The transaction
  repeats request equality before CAS. Historical reads/export use retained
  versions/trees and never execute today's or historical extensions. These
  preserve ledger v1's original-result replay and immutable-history semantics.
- **Workspace and init:** Section 6 concretely distinguishes names, explicit
  files, unset/empty/invalid environment and no discovery/defaults. Name and
  explicit spelling share intent only at the same lexical target. Its explicit
  render/load `symlink/..` distinction agrees with
  `src/engine/capture/host_path.cpp`: render physically resolves parents, while
  load intent first normalizes lexically. Native captured traversal and alias
  rules remain in force. Existing explicit-ledger init remains current;
  scaffold/prompt behavior, exclusive publication and cleanup are correctly
  assigned to RA01 with no-overwrite requirements and no implied directory
  transaction guarantee.

## Verification and limits

Executed read-only source/document inspection plus
`python3 tests/native/v5-02-review-evidence/verify.py`. The independently retained
[verification](../../../tests/native/v5-02-review-evidence/verification.json)
records:

- All 10 coder-manifest entries match their exact hashes, including final
  proposal/snapshot/diff and frozen draft evidence.
- The proposal has 393 lines; its snapshot and diff-added bytes match exactly.
- All 20 local links resolve; every linked heading fragment exists.
- All 12,021 protected inputs match the coder's before snapshot, with zero
  changes. The coordinator-owned round was not part of that snapshot; the
  reviewer's newly owned files are outside it.
- V5 remains `c3d37e0e28407a9066eb51c515982ef6c93d82795eb56f97dea4a780b66fe08e`;
  `AGENTS.md` remains the symlink to `CLAUDE.md`.

This is a documentation/design review. Builds and product tests were not run;
no runtime count or platform result is claimed. Exact command-catalog/family ABI,
profile capture protocol, parser spike, codec coexistence, selected-definition
identity and RA01 publication remain mandatory scoped gates already identified
by the proposal. They are appropriately deferred from this operator-choice
proposal and must not be inferred from this approval.
