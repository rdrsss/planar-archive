# R14-D3 independent documentation review

Disposition: **approve**. No blocking findings or unresolved product choices.
Approval applies only to the frozen nine-document candidate identified below.
Dependent implementation still requires the separately scoped D4–D8 reviews.

## Scope and candidate identity

Fresh independent reviewer in the shared checkout on
`dev/planar-v2-vernacular`, HEAD
`3171c49e7b92d3abda01bfbb8bfb0c6b29d4c67a`. The intentional pre-existing rewrite
was preserved. Reviewer owns only this report and
`tests/native/r14-d3-review-evidence/`; no candidate fix, source edit, prior evidence
rewrite, product build, installed binary, operator database, Planar tool/skill,
queue, commit, install, publication or merge was used.

Reviewed current CLAUDE/AGENTS instructions, implementation plan, roadmap, v5
direction/alignment/acceptance map, R14-D3 brief, full baseline-relative candidate
diff, approved command/workspace and ledger contracts, C approval, author evidence,
and the current native capture/preparation/transaction, codec, retained runtime,
shipped Lua and response composition APIs. Author audit.py was inspected but not
executed: it writes frozen author evidence. An independent read-only verifier
writes only reviewer output.

SHA256 identities, rechecked against current documents and snapshots:

| Frozen object | SHA256 |
| --- | --- |
| wrapped candidate.json; repository-relative nine-file map | `ffe90a6e6db40b5d47ba72697bf85d7705bb08a41a67e85a51b93a70d12edb38` |
| owned.diff; exact baseline-relative candidate | `37508941e36017d8f9727d20aa9ce25822c19dba1e66e486cf15c6e749af1c7f` |
| wrapped artifacts.json; evidence-relative 30-file map | `42f78b71fd4bf328cf64cb7fbffaa0c53f9dd26bdf178b3f6e1797b8ee2afd1f` |

## Contract assessment

- [Fixed admission and intent](../load-command-contract.md#fixed-catalog-admission-and-typed-intent)
  defines one fifth mandatory pairing, six required separate-token text options,
  exact presence and opaque slots, canonical signed-64 expected revision, and the
  approved `--expect-revision` spelling. D7 is the single public switch; D6 may use
  test-only bundles without breaking shipped four-entry validation.
- [Native prelude](../load-command-contract.md#native-prelude-and-early-replay)
  frames lexical normalized intent without requiring inputs to exist, compares
  exact request body bytes and returns saved bytes before handler/current Lua,
  capture, provenance or CAS. It releases the read owner on a miss. This composes
  the immutable [request intent](../../../src/engine/acceptance/prepared.cppm#L24)
  and verified request reads rather than introducing request_v2 or current-policy
  replay. Changed environment matters only when expansion changes the target.
- [Three services](../load-command-contract.md#finite-fresh-load-service-protocol)
  preserve same-bundle trusted routing/interpretation and native retained capture.
  Existing [interpret_bundle](../../../src/stdlib/planar/acceptance.lua#L3) validates
  every job with source `not_captured`; no selected role or checkout is needed.
  The closure is native-owned, includes references, and retains selected ancestors
  and registry receipts. Native provenance/codecs/preparation own source members,
  versions and encoded records; Lua supplies no generic mutation/callback or saved
  response. This is implementable composition with new bounded adapters, not a
  claim that those adapters already exist.
- [Atomic completion](../load-command-contract.md#atomic-completion-and-saved-byte-transport)
  preserves late exact replay/conflict before locked CAS and candidate access,
  noop/restore chronology, atomic event/request/head changes and final receipt
  reads under the write lock. Current
  [complete_acceptance](../../../src/engine/ledger/format.cpp#L1436) provides those
  ordering sites; current
  [acceptance capture](../../../src/engine/capture/native.cpp#L479) clears its
  instrumentation callback before ownership transfer and performs native final
  verification. The amendment neither moves Lua under lock nor prohibits the
  required native filesystem reads. Cooperative race limits are explicit.
- The proposed overload explicitly carries the early ledger identity into the
  writable transaction, checks it before relookup, and retains the old API.
  Its native completion receipt distinguishes known commit, replay, uncertainty
  and confirmed noncompletion before allocating post-operation values. Saved-byte
  ownership, preallocated fallbacks and exact handler-result equivalence cover
  first-result allocation, Lua/encoding failures and stdout short writes. Replay
  is not described as fresh mutation; recovery retries original intent with the
  same ID or queries history. Implementers must prove the receipt across exception
  and COMMIT failure paths; the text accurately marks this as a proposed addition.
- [Typed diagnostics](../load-command-contract.md#typed-authoritative-diagnostics)
  use finite owned native variants. The current
  [format_error](../../../src/engine/ledger/types.cppm#L27) lacks details, while
  [intern_record](../../../src/engine/ledger/format.cpp#L1317) and the locked CAS
  have authoritative values to add them. No CLI head reread or message parsing is
  needed. Expected/actual remain canonical strings; file/directory/value collisions
  expose only stable kind/digest. Request collisions omit digest and request
  conflicts omit details. Malformed variant/code combinations refuse internally.
  Existing unrelated generic mappings remain intact.
- [Bounds, precedence and D4–D8 matrix](../load-command-contract.md#bounds-and-failure-precedence)
  retain existing lower limits and cumulative budgets, explicit native-error
  precedence and exact saved JSON plus LF transport. The matrix assigns failing
  regression evidence, all-job/receipt/provenance cases, retained-callable abuse,
  early/late replay races, CAS, crash/disk-full/busy/collision rollback and
  postcommit recovery. D7/D8 require full serialized debug/release gates. Historical
  reads/export without current policy and unchanged W/render/storage shapes are
  preserved. No partial public load or full-R14 claim is authorized by this review.

The scoped status/reference amendments agree with approved C and preserve required
external-profile X after full R14 and before dependent jobs/RA01. User v5 is
unchanged. Existing 370/370 macOS ARM64/LLVM23.1.4 evidence is correctly identified
as the approved baseline; load and Linux evidence remain pending.

## Independent verification and limitations

`python3 tests/native/r14-d3-review-evidence/verify.py` passed: nine current
documents match candidate snapshots, all 30 author artifacts and eight baseline
snapshots match their manifests, and the diff reproduces exactly. All 212 local
links/anchors and all nine whitespace scans pass. All 6,124 protected ordinary
file hashes match; AGENTS remains a symlink to CLAUDE.md. User v5 SHA256 remains
`c3d37e0e28407a9066eb51c515982ef6c93d82795eb56f97dea4a780b66fe08e`.
The author's 11/11 traceability assertions were inspected as documentation checks,
not accepted as runtime tests. Reviewer semantic assessment above is independent
of those string assertions.

Raw independent results and the reproducible verifier are in
[review evidence](../../../tests/native/r14-d3-review-evidence/verification.json).
This docs-only review ran no product builds/tests and claimed no build directory.
It approves the design amendment, not future implementation or full R14 acceptance.
Coordinator must recheck the exact candidate identity before advancement.
