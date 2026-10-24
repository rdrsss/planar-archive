# V5-02-X06-A2 independent review

Disposition: **approve**, combined pure recorded-interpretation helper and the
single scoped v1 canonical-resource adapter. No required changes or unresolved
product choices. This approval does not enable actual history/storage/export
dispatch or profile acceptance.

Shared checkout: `dev/planar-v2-vernacular`, starting/current HEAD
`3171c49e7b92d3abda01bfbb8bfb0c6b29d4c67a`, intentional dirty rewrite.
Reviewer writes are limited to this report and
`tests/native/v5-02-x06-a2-review-evidence/`; verification used exclusive,
serialized Debug. Product/test sources, author evidence and prior rounds stayed
read-only. No delegation, Planar operational tools/skills/roles/queue, installed
binary, commit, installation, publication, merge or operator-data action occurred.
No others' edits were reverted. Release and install-release were never invoked.

## Exact candidate and scope

Review considered the active guide, implementation/roadmap and v5 trust/history
direction, complete A2 brief including its dependency split and narrow adapter
amendment, approved A1 review, X06 implementation path, exact X-STORE and the
underlying canonical/tree/provenance/v1/v2 boundaries.

| Identity | SHA256 |
| --- | --- |
| Candidate JSON, all 14 files | `831d831bdcf93f64a023648451358b0500c3845f82f54522e4387df9bb82a5a1` |
| Exact reconstructed source/reference diff | `89aaa6f1118c854c0efe6b94370b736d0a9132edd925ac0d51caf198910a0771` |
| Author report | `80304e3fd4b2bc095a0a18af0e555e6dc68ff0854ef09e25ad3115a33ad7cde6` |
| Author artifact manifest | `356dfeb6ea54223b6baf580972f044b818d4322050e78e6395975b751dd2d17d` |

The independent [candidate audit](../../../tests/native/v5-02-x06-a2-review-evidence/candidate-audit.json)
checks every current candidate hash, all 52 author evidence entries and three
support files with exact membership/size/hash, original preedit copies against
the original baseline, and byte-for-byte reconstruction of the diff. It
independently reconciles the author's 93,932-member baseline and 93,962-member
final protection. The review's 94,026-member baseline matches every author-final
protected entry; its extra 64 entries are the author evidence/support subtrees.

CMake appends only the new pure module/test registration. Eight status-document
historical bodies and the X06 document's historical body remain exact; all 517
candidate local Markdown file links resolve. The existing A1 codec, provenance,
runtime, shipped Lua, actual readers, acceptance, storage, CLI and exact contract
remain unchanged. The old v1 interface/tests also remain unchanged; the only old
codec implementation edit is the precisely authorized resource adapter.
AGENTS.md still links to CLAUDE.md; compile_commands.json preserves its original
symlink identity and target.

## Source assessment

[recorded_interpretation.cpp](../../../src/engine/history/recorded_interpretation.cpp),
lines 34–39, preflights both encoded sizes before canonical decoding, with fixed
16 MiB/depth256/four-million-node limits. Only field presence selects the effective
family: `extensions`, or embedded/separate-V `provenance`/`registration`.
Non-record containers are checked safely. Numeric version alone does not change
the family, preserving old v1-shaped future-version behavior. Classification
is a non-authoritative hint, repeated internally by verification; its decoded
values die before provenance work. No caller-built digest, limits, profile or
prepared aggregate can bypass verification.

Lines 73–94 admit count before allocating file views, then use the fixed
embedded16 MiB/4096 or effective32 MiB/8192 tree profile to validate raw topology
and bounds. Complete provenance verification follows. Unknown effective manifests
have a deliberately narrow preflight at lines 41–62: exact known envelope
keys/kinds, bounded module entries with portable external paths and normalized
modes, and exact catalog container keys/kinds precede version/scheme unsupported
classification. This does not claim contextual validation of unknown catalog or
manifest semantics. Unknown manifests return refusal with no exportable result;
known manifests immediately enter the unchanged approved full verifier.

Lines 125–153 preserve the pinned order: canonical classification, raw topology,
provenance, fixed decoder, workspace/digest. Supported v2 still calls the approved
mandatory-rawtree A1 decoder, with its independent verification and selected-shape
correspondence. Lower malformed structure and separate-V mismatch precede
interpretation unsupported; provenance refusal precedes both. On a lower
unsupported-version result, bounded redecoding extracts only already fixed-shape-
checked workspace/digest for exact comparison against expected workspace and the
independently recomputed complete tree digest. A matching unsupported result has
no semantic payload. Both public operations retain resource categories and catch
native bad_alloc. Repeated bounded decoding/provenance verification is explicit,
without a shared-budget or RSS claim.

Supported results own canonical values and exact bytes, retaining no raw-file
views. This proves recorded structural correspondence only. Self-consistent
alternative historical bytes, binary/non-callable modules, or modified ordinary
result aggregates confer no callability, semantic honesty, same-VM selection or
acceptance authority. The module has no ledger-format argument; future readers
can use its fixed v1/v2 dispatch across either schema without changing writer
restrictions. Actual reader/export adapters remain a separate A3 round.

The single edit in [interpretation_codec.cpp](../../../src/engine/history/interpretation_codec.cpp),
lines 150–154, separates failed embedded-V canonical encoding from exact byte
mismatch and forwards failures through existing `codec_failure`. The original
shape, equality, unsupported ordering and accepted bytes remain unchanged. Its
preedit source matches original SHA256
`08c995a8b62ee2c4f26a42d4c4d5f685572563db023f3f3d5ced73c226682497`.
Preserved first and diagnostic runs show 18 failed allocation assertions; the
diagnostic first failure is 1883/1904. The preserved direct old-decoder red run
shows 18 failures, first 880/900. This review inspected these exact artifacts and
snapshots; it did not rebuild the pre-correction candidate or claim a second red
reproduction. Direct and enclosing final sweeps independently pass. The lower
API still has original raw bad_alloc sites; the new enclosing APIs catch them.

## Independent verification

| Executed check | Result |
| --- | --- |
| CTest exact absolute focused list, JUnit output | **106/106**, zero failures/errors/skips/disabled, 61.01 seconds. |
| Private isolated `[review]` checks against unchanged candidate libraries | **6 cases / 32,790 assertions**, zero failures/errors/skips, 21.438 seconds. |
| Current JSON inventory versus approved A1 inventory | **910 names**, all **892** preserved, exactly 18 additions. |
| Serialized final `all planar_tests` build | `ninja: no work to do.` |
| Exact source/reference, native link/symbol and artifact audits | Pass. |

The private cases cover every version-vector key changed to a future value in
both families; independently refused digest mismatch; raw order invariance and
duplicate/prefix topology; malformed marker confusion; unknown-manifest path,
mode, catalog-kind/extra-key and module-count precedence; independent result
ownership after source destruction and mutation of another result; and exhaustive
one-shot allocation failures on unsupported, wrong-workspace, unknown-manifest
and malformed-manifest branches. The focused gate also reruns all 18 new cases
with 14,193 assertions, old v1/v2, tree/provenance/catalog/canonical/record/hash
coverage, valid embedded exact16 MiB and4096-file boundaries, and fixed effective
outer ceilings. Synthetic missing-manifest maxima do not prove valid effective
32 MiB/8192-file inputs; stricter component bounds still apply.

The [private dependency proof](../../../tests/native/v5-02-x06-a2-review-evidence/private-proof.json)
records recursive original response expansion, explicit existing PCMs and disabled
implicit modules. All 803 captured dependencies, including all 754 actual compile
dependencies and Ninja journals, retain exact mode/hash/mtime and other recorded
stat identity through compilation and execution. Raw source, argv, dependency
lists and XML are retained. Loader output records existing pinned libc++; no
copied-runtime claim is made. Only the owned disposable object/executable were
removed after proof, with hashes/modes/sizes preserved. No private verification
failure occurred.

Author formatting/Doxygen over 124 active sources, four existing independent
vector checks, source audit and no-work evidence were audited without redundant
reruns. Controlled allocation sweeps do not establish natural host exhaustion or
whole-process RSS limits.

## Protection and limits

The [protection audit](../../../tests/native/v5-02-x06-a2-review-evidence/protection-audit.json)
compares independent full-content baseline/final hashes including root dotfiles,
symlink identities/targets, candidate/evidence, every prior round and all build
trees. Only `.git`, this review report/evidence and the exact coordinator A2 round
are exempt; only Debug verification outputs may change. Protected directory
membership and original mode/device/inode remain exact; ordinary ancestor
directory timestamp changes are recorded separately. All author and prior A1
evidence, release and install-release remain protected. Current candidate hashes
are rechecked after verification. The review artifact manifest binds this report
and all review artifacts without a hash cycle.

No full 910-case configuration gate, release/Linux, actual reader dispatch,
profile preparation/transactions, shared whole-invocation budget, public CLI or
mandatory external-profile completion is approved here. Latest complete approved
macOS gate remains C5's876/configuration. Public init creates format1; later X06,
X07, X08, execution/RA01 and Linux remain open.

**Debug and all reviewer files are released. No build/test process remains.**
