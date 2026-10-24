# V5-02-X06-A independent review

Disposition: **approve**, documentation only. No required changes or unresolved
product choices. The constness precision below bounds one informal source
description; it does not authorize a new API or behavior. X06-A1 may begin as a
fresh scoped round after the coordinator checks the approved candidate identity.

Shared checkout: `dev/planar-v2-vernacular`, HEAD
`3171c49e7b92d3abda01bfbb8bfb0c6b29d4c67a`, intentional dirty rewrite.
Ownership is only this report and `tests/native/v5-02-x06-a-review-evidence/`.
Product/build files, author evidence and prior rounds remained read-only. No
other work was reverted. No delegation, Planar tools/skills/roles/queues, product
build/test/probe, installed binary, commit, installation, publication, merge or
operator-data operation occurred. No build directory was acquired.

## Exact candidate and independent checks

Review considered the active guide, implementation/roadmap and v5 trust/history
direction, the complete X06-A brief/refinement/handoff, exact X-SHAPE/X-CAP/
X-LIMIT/X-STORE, ledger/load replay and recovery contracts, C4-C2 tagged-definition
precedent, C5 approval, the actual preedit bytes/diff and current native boundaries.

| Author identity | SHA256 |
| --- | --- |
| Combined manifest, ten candidate documents and twenty evidence entries | `d74ffa8f5286f3a30dcf5c88bf18b3315a4efad9482a4547e19ce69ce293ae25` |
| Exact reconstructed documentation diff | `39404077c4a44c532dadd4cfe032fe3f505352feb2318a4c952c0b046dc8a69e` |
| Author REPORT | `95403f30b657a5920c48d71dc68dbb148f1c0f3ac3c9c8be80809e78ceabc7d7` |

The independent [candidate audit](../../../tests/native/v5-02-x06-a-review-evidence/candidate-audit.json)
checks every current document and artifact hash/size, exact author artifact
membership and byte-for-byte diff reconstruction. The eight status documents
change only their openings; all later historical bodies remain byte-identical.
The external-profile contract changes only the scoped X-STORE notation; its
earlier policies and later format/replay rules remain exact. All **525** local
Markdown file links resolve after URL decoding. AGENTS.md remains the symlink
to CLAUDE.md. The preserved initial URI-decoding checker failure is a tool issue,
not a broken target or a claimed product pass.

The independent [scope audit](../../../tests/native/v5-02-x06-a-review-evidence/scope-audit.json)
compares the author's **93,736-member** preedit inventory and **93,737-member**
final inventory. Exactly the ten authorized documents differ, including the new
path. A fresh full-hash **93,762-member** reviewer baseline matches every author
final entry; its 25 additional entries are the complete author evidence subtree.
Material root dotfiles, symlink identities/targets, all build trees, C5 and all
prior rounds/evidence are covered. Only .git, this review report/evidence and the
exact coordinator X06-A round are exempt from reviewer protection.

## Contract and source assessment

The [X-STORE clarification](../external-profile-contract.md#x-store--byte-codecs-exact-provenance-and-supported-formats)
preserves the existing identity. At
[runtime.cpp](../../../src/engine/runtime/runtime.cpp), lines 3566–3610,
`resolve_shape_definition_impl` hashes the validated descriptor, logical
`{tier,module}` owner and lowercase hex of the exact original selected-module
bytes under `shape_definition_v1`. It excludes capture ordinal. At
[hash.cpp](../../../src/engine/hash/hash.cpp), lines 16–39, the canonical typed
domain hash emits ASCII `xxh3_128:` plus the 32 lowercase canonical hash digits.
The independent construction and fixed tagged vector in
[external_registration.t.cpp](../../../src/engine/runtime/external_registration.t.cpp),
lines 69–105, match this recipe. These tests were inspected, not rerun.

Therefore S.selected_shape.definition and E.selected_shapes.definition retaining
the same **41-byte** tagged H value is a notation clarification, not a second
identity or a new policy. The approved C4-C2
[evidence](../../../tests/native/v5-02-x05-c4-c2-evidence/REPORT.md), lines 23–29,
already records that tagged transport precedent. The effective record-tree
digest remains distinct: the current acceptance preparation formats its 16 digest
bytes as raw 32 lowercase hex digits, and v1 interpretation checks that spelling.
No hash domain/recipe, W recipe, declaration, SQL, record framing or version
constant changed.

The [new path](../x06-implementation.md) correctly separates self-structure from
cross-record authenticity. Neither an asserted digest nor a structurally valid
catalog proves correspondence to the referenced raw tree. The future verifier
must internally verify raw files, independently decode the verified raw
manifest/catalog bytes and recompute digest/shape identities. The existing
[effective verifier](../../../src/engine/history/effective_provenance.cpp),
lines 189–258, performs native tree/component/membership reconstruction and
returns an owned `prepared_provenance`; that public aggregate exposes writable
files and digest. It is not an unforgeable acceptance capability.

**Constness precision:**
[value.cppm](../../../src/engine/value/value.cppm), lines 16–25, defines
`value_ptr` as `std::shared_ptr<const value>`, recursively used in fields/items.
[validated_catalog](../../../src/engine/history/effective_catalog.cppm) exposes
a const-facing graph and canonical bytes, and its implementation, lines 303–331,
severs caller aliases by encoding/decoding. Its ordinary getter is not mutable.
However, [canonical decoding](../../../src/engine/canonical/canonical.cpp),
lines 127–192, allocates nonconst `value` objects before exposing const pointers;
an explicit native `const_pointer_cast` can mutate those actual objects. The
path's informal “not deeply const” wording must be read as absence of an
unforgeable physically immutable capability, not denial of the recursive const
API. Raw-tree verification is required for cross-record authenticity regardless
of this cast possibility. A1 must not weaken existing const-facing APIs or trust
the catalog graph as evidence of a particular tree or VM. This precision does
not change the specified A1 behavior and is not an unresolved product choice.

The remaining structural plan matches X-SHAPE/X-STORE: exact B/S sorted job
correspondence, selected-shape fields, E catalog/selection correspondence,
normalized obligations with original phase/quality/work/role rules, guidance and
count limits, exact version substitutions and separate-V byte equality. It does
not invoke current declaration schema policy, shape functions or historical Lua,
and claims no proof of callability or semantic honesty. The 16 MiB, depth256 and
four-million-node codec expectations and lower effective-tree/component bounds
match existing limits; v2 error ordering remains a future reviewed test duty.

The proposed API names are explicitly unimplemented. The first slice leaves
[interpretation_codec.cpp](../../../src/engine/history/interpretation_codec.cpp),
including its malformed/separate-V checks before unsupported-version
classification at lines 140–158, and the
[history reader](../../../src/engine/history/read.cpp) dispatch untouched. The
existing prepared/load paths still verify embedded provenance and encode/decode
v1; the path does not claim fresh profile acceptance exists.

Later rows preserve the required dependency boundaries: reader dispatch and raw
source export; authentic same-VM private preparation; format2 profile acceptance
with exact early/locked replay, conflict-before-CAS, rollback/uncertain-commit/
same-ID recovery and original receipt seals; then genuine whole-invocation
ownership/accounting. Known/late input phases remain explicit. There is no Lua
under lock, mutable caller aggregate as authority, budget reset/refund or
promotion of today's separate metrics to RSS/shared-budget guarantees.

## Verification limits and release

The review executed only its own documentation/identity/protection audit:

```
python3 tests/native/v5-02-x06-a-review-evidence/audit.py baseline
python3 tests/native/v5-02-x06-a-review-evidence/audit.py candidate
python3 tests/native/v5-02-x06-a-review-evidence/audit.py final
python3 tests/native/v5-02-x06-a-review-evidence/source_audit.py
```

The [final protection audit](../../../tests/native/v5-02-x06-a-review-evidence/protection-audit.json)
records a fresh full-content hash inventory with **zero drift** over all 93,762
protected reviewer entries. There was no product test selection, full gate,
formatting/Doxygen or no-work build in this documentation-only round.
The review's first report-link check ran before the final protection artifact
existed; its ordering failure is preserved in `initial-review-link-check.txt`.
The corrected check runs after final publication and verifies all report links.
The candidate audit also repeats after final protection, with the same hashes.
[C5 approval](V5-02-X05-C5-review.md) remains the latest complete macOS gate:
876/876 per Debug and RelWithDebInfo configuration, plus its separately bounded
private evidence. Those historical results were not rerun or expanded here.

The review finalization binds this report and the review artifact manifest
without a hash cycle. **All reviewer files are released; no build/test process
or build ownership remains.** Public init still creates format1. X06 behavior,
X07 public flags/cutover, X08 mandatory external-profile completion,
execution/RA01 and Linux remain open. This approval closes only the X06-A
documentation clarification and bounded path.
