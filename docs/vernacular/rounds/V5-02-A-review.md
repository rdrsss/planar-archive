# V5-02-A independent exact-contract review

Verdict: **approve**. No blocking findings or unresolved product choices.
This approves the frozen documentation contract for bounded implementation;
it claims no new supported command, workspace-name support or runtime safety result.

Reviewed in the shared checkout on `dev/planar-v2-vernacular`, HEAD
`3171c49e7b92d3abda01bfbb8bfb0c6b29d4c67a`. Existing rewrite changes were preserved.
Reviewer writes are confined to this report and
`tests/native/v5-02-a-review-evidence/`. No implementation fixes, Planar tooling,
builds, product tests, installed binaries, operator data or commits were used.
The [round brief](V5-02-A.md) and [accepted A/B/C decision](V5-02.md#operator-decision)
are the review scope and authority; accepted choices need no repeated approval.

## Exact candidate

The eight documents in the coder's `candidate/` snapshot match current files.
The independently reconstructed baseline-to-candidate diff matches `owned.diff`
byte for byte. Identities:

- Coder manifest SHA256:
  `32d5e5a2f695d34afb62e6fadeaf353c485432d6b5b6a1c8706a4152405d5352`.
- Owned diff SHA256:
  `4f53299a5263140987c7dd9680c64933bdeadc9c773e9b3d4787003db204beae`.
- New command-contract.md SHA256:
  `fa7a6c22930073c84ec346a9b041a4b02b48eecd59dd9327e5ee0016862cf80a`.

All 23 coder manifest entries were checked, and its manifest covers every file
in that evidence directory except itself. See the review's
[independent audit](../../../tests/native/v5-02-a-review-evidence/audit.json),
[audit program](../../../tests/native/v5-02-a-review-evidence/audit.py) and
[source hashes](../../../tests/native/v5-02-a-review-evidence/source-evidence.json).

## Technical assessment

- [Catalog and structural ABI](../command-contract.md#catalog-and-structural-abi)
  specify exact descriptor fields, option signatures, bounds, four mandatory
  family/handler/response/capability pairings, atomic rejection and native-owned
  callable references. The same-VM binding is sealed with owner and exact bundle
  digest. Jobs receive none of the registry or service authority. This is an
  implementable retained-runtime interface, rather than deferred ABI design.
- [Lifecycle](../command-contract.md#bootstrap-errors-and-retained-lifetime)
  preserves the CLI11-only parse and option lifetime demonstrated by
  `src/cmd/planar/bootstrap.cpp` and the four family `command.cpp` files. Static
  signature probing does not execute a second argv parser. Sole root help and
  caller parse failures precede registry evaluation; a valid invocation cannot
  fall through a malformed shipped registry. Registration is pure, per-invocation
  references are retained until encoding, and both registration and cumulative
  execution budgets are terminal even after protected-call catches. This extends
  the existing bounded conversion/protected-call mechanisms in
  `src/engine/runtime/runtime.cpp` without claiming they already implement it.
- [Finite services](../command-contract.md#invocation-values-and-finite-native-service-protocol)
  give each family concrete call order, exact argument/result shapes and native
  failure ownership. Render's two-service split matches current workspace route,
  capture, stability verification and Lua render stages in
  `src/cmd/planar/handlers/render/command.cpp` and
  `src/stdlib/planar/schema.lua`. Registered Lua performs real route/validation/
  selection/projection work. Init/history/export retain native storage facts and
  exact response checks while executing their registered Lua handler. The required
  replaced-handler test establishes consequential semantic dispatch. Native
  handle/family/order/argument checks prevent local Lua option mutation from
  expanding authority. Database IDs stay decimal strings through INT64_MAX;
  budget/limit integers stay within the existing signed-32 structural range.
- The service contract forbids Lua execution under native write locks and freezes
  service arguments before entry. History's current recorded-version decoder in
  `src/engine/history/read.cpp` is retained without semantic reinterpretation.
  Export explicitly covers failure after native publication but before Lua
  projection/encoding completes, preserving the existing preallocated uncertainty
  and stdout diagnostics in `handlers/export/command.cpp` and safe cleanup tokens.
- [Load/provenance](../command-contract.md#load-admission-and-exact-provenance)
  leaves load unregistered. Its later ABI must perform saved-request lookup before
  capture or candidate policy, then retain the native transaction relookup already
  present in `src/engine/ledger/format.cpp`. Replay needs no handler invocation,
  declaration/registry/profile read or physical resolution. New embedded module
  bytes enter existing original/assembly provenance from `cmake/embed_lua.cmake`;
  request_v1, interpretation_v1, versions_v1, record_v1 and W recipes keep their
  shapes and meanings. Old evidence/goldens cannot be rewritten to hide changes.
- [Workspace rules](../command-contract.md#workspace-selector-and-environment)
  fix the accepted name/file grammar, exact env behavior, cwd/env snapshots,
  diagnostic categories, 64 MiB checked path bounds and no HOME/discovery fallback.
  Preserved render spelling resolves parents physically as current
  `src/engine/capture/host_path.cpp` does; load request/capture uses its explicit
  lexical target. The potentially different `symlink/..` results are stated
  directly, with no weaker named capture policy or replay filesystem dependency.
- [Sequencing](../command-contract.md#required-implementation-and-verification-sequence)
  assigns B1 retained registry/runtime, B2 CLI family/service composition, C naming,
  remaining R14, mandatory external X, then RA01/execution. External selection
  policy remains accepted while the envelope spike and exact new intent/provenance/
  interpretation/coexistence codecs require their own later review. No speculative
  external codec is introduced here. Focused and integrated code gates are named.

The seven existing-document amendments are scoped to those bridge/naming and
status/sequence changes. Unaffected render/ledger schemas and exact behavior
remain authoritative. V5 and the accepted frozen proposal are unchanged.

## Independent verification and limitations

Executed `python3 tests/native/v5-02-a-review-evidence/audit.py`: pass. It verified
23 manifest entries, eight current candidate snapshots, the exact reconstructed
diff, 5,601 unchanged protected files, 175 resolving local links/anchors, the
AGENTS.md-to-CLAUDE.md symlink, `git diff --check` for the eight owned paths and
added-line whitespace. There were no unexplained protection omissions across
current source/vendor/fixture/script/CMake files, tests or historical rounds.
The current coordinator round and new coder/reviewer evidence are explicit
exclusions; every prior test/evidence file and historical round is represented.
This independently checks the expanded inventory's coverage and unchanged bytes.
The coder/coordinator record supplies its pre-first-edit timing; hashes alone
cannot prove wall-clock authorship order.

No product tests were selected or run, and no test count is claimed. This is the
authorized docs-only gate. Future B1/B2/C/R14 acceptance still must prove the
specified runtime, parser, capability, resource, capture, publication and replay
properties against actual code with the recorded platform/toolchain limits.

The coordinator may proceed directly to fresh B1 search/coder/reviewer dispatch
after confirming this exact candidate is still current. Scope-affecting edits
require renewed review.
