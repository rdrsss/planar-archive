# V5-02-X06-A3 independent review

Disposition: **approve** for the exact recorded-history integration candidate.
No required changes or unresolved product choices. This approval opens verified
saved profile history and exact saved replay; fresh profile acceptance remains
refused.

Shared checkout `dev/planar-v2-vernacular`, starting/current HEAD
`3171c49e7b92d3abda01bfbb8bfb0c6b29d4c67a`, intentional dirty rewrite.
Reviewer writes are limited to this report and
`tests/native/v5-02-x06-a3-review-evidence/`; verification used exclusive serialized
Debug. Product/test sources and all author/prior evidence remained read-only.
No delegation, Planar operational tools/skills/roles/queue, installed binary,
commit, installation, publication, merge or operator-data action occurred.
No others' edits were reverted. Release/install-release were never invoked.

## Exact candidate and evidence

Review considered the active guide, implementation/roadmap and v5 trust/history
direction, the entire A3 round including accepted search refinements and frozen
record, X06 implementation path, exact X-STORE, approved A1/A2 reviews, and the
underlying tree/record/interpretation/provenance and acceptance boundaries.

| Identity | SHA256 |
| --- | --- |
| Candidate JSON, all 13 files | `5c785b5b0be91886b90191816bcf23a32e41915484063c1eca52d2298f7cd0b5` |
| Exact reconstructed source/reference diff | `b5c6d1c42ffb86e5268813ed4dec227f97fa173f3c5479f44e5fca1a7762e34e` |
| Author report | `2e34dccd57f886089c785a8985d5ec5e89505c677f3c21f472e03fa0b3fc419f` |
| Author artifact manifest | `6c6f946aaf43ae3bc4a5926e0b8621f3452c6a88521aaf327083debe05d76b0c` |

The independent [candidate audit](../../../tests/native/v5-02-x06-a3-review-evidence/candidate-audit.json)
checks every current candidate hash, all 74 author evidence entries and three
support files with exact membership/size/hash, original preedit copies against
the original full baseline, and byte-for-byte diff reconstruction. It independently
reconciles the author's 94,061-member baseline/final protection. The review's
94,149-member full-hash baseline matches every author-final protected entry; the
additional 88 entries are the author evidence/support subtrees.

The ledger implementation changes only its import and two authorized integrity
functions. The view adapter and required production/generated-variant CMake
edges are scoped. Schemas, format APIs, request codecs, A1/A2 pure modules,
prepared acceptance, complete transaction implementation, load/runtime, shipped
Lua, source export and exact external-profile contract retain their original
bytes. Nine historical documentation bodies remain exact. All 519 candidate
local Markdown file links resolve. AGENTS.md still links to CLAUDE.md;
compile_commands.json retains its original symlink identity/target.

The preserved [author regression](../../../tests/native/v5-02-x06-a3-evidence/red-tests.xml)
fails the valid effective-v2 success fixture at `open_existing` under the original
readers. Its copied source hashes equal the original full baseline. The earlier
request-v2/codec1 variant compile error and the later large-fixture manifest-order
failure are preserved separately as fixture failures. This review inspected these
artifacts; it did not rebuild old sources or claim a second red reproduction.
The 711-case author gate uses the frozen product/CMake bytes. The sole later test
change adds three existing negative fixture modes; its exact diff and affected
970-assertion rerun are preserved. This review independently ran that final case.

## Source assessment

[ledger/format.cpp](../../../src/engine/ledger/format.cpp), lines 771–822, retains
source reconstruction and the exact one-file registry validation before bounded
interpretation/versions classification. The classifier selects only the fixed
embedded16 MiB/4096 or effective32 MiB/8192 profile. Actual referenced directory
records are reconstructed through tree_core, then the approved helper repeats
classification and verifies complete raw provenance, fixed interpretation shape,
workspace and recomputed digest. This applies even when semantics are safely
unsupported. No caller limits, claimed digest or mutable prepared aggregate
substitutes for the recorded tree. Original storage failures survive; ordinary
malformed/resource/tree/correspondence refusals use existing corruption. Unknown
effective manifests use existing unsupported_state, while old embedded unknown
manifests retain corruption.

Lines 847–1000 retain the event/revision/actor/time/reference/request/head/result
checks and their ordering. Each accepted revision records its independently
verified family alongside active references. Noops already require exact active
references, so family reuse avoids a redundant tree reconstruction without losing
correspondence. Only after existing request checks, request_v1 and request_v2
embedded intent require embedded provenance, while profile request_v2 requires
effective provenance. Presence selects the family; the private host selector path
is never compared with portable profile/input.json. Both accepted and noop
mismatches refuse. Schema/codec restrictions remain independent. No new format1
profile-history validity or embedded-tree/v2 combination is inferred.

[history/read.cpp](../../../src/engine/history/read.cpp), lines 41–87, reads the
actual stdlib root and files through the existing pinned format_file/read_tree
API, preserving typed storage errors and applying the same helper with recorded
workspace. Only a supported owned decoded result becomes semantic projection.
Safely unsupported interpretation yields existing unsupported_interpretation,
while raw page/state/request reads and unchanged source export remain available
for supported provenance. Unknown provenance is refused by format integrity
before an owner is published. There is no fallback to current or historical Lua.
The new history_tree edge stays above ledger_format and tree_core; no cycle is
introduced. The actual compiler dyndep closure and native-symbol audit show no
Lua/runtime/live-profile dependency. The existing writer's native-capture edge
and pure profile_codec are retained and are not historical execution.

The unchanged complete_acceptance implementation, lines 1621–1675, still performs
verified open/identity and exact saved request lookup before format restrictions,
CAS or candidate access. Exact saved accepted/noop profile requests can therefore
return their original bytes even with moved-from preparation and stale current
head. Identity-bound completion retains its original receipt behavior. A fresh
profile ID still refuses unsupported_state before candidate inspection; saved
body conflicts retain request_conflict. SQL fixture publication confers no fresh
writer/preparation authority and supplies no migration tool.

## Independent verification

| Executed check | Result |
| --- | --- |
| Exact absolute CTest focused list and JUnit | **40/40**, zero failures/errors/skips/disabled, **31.72 seconds**. |
| Isolated private `[review]` integration checks | **4 cases / 572 assertions**, zero failures/errors/skips, **2.186 seconds**. |
| Current JSON inventory versus A2 | **918 names**, all **910** preserved, exactly eight additions. |
| Serialized final `cmake --build --preset debug --target all planar_tests` | `ninja: no work to do.` |
| Exact source/reference, artifact, dependency/symbol and local-link audits | Pass. |

The focused set includes all eight added cases, historical relational/chronology
and noop corruption, both embedded formats/mixed request codecs, unsupported
interpretations, exact source export and its failure/race controls. It exercises
the valid4100-file effective closure, both selector mismatch directions on
accepted/noop events, request-v1/effective mismatch, actual raw-tree and
workspace/digest/catalog/selection forgeries, unknown manifests, exact saved
accepted/noop replay and fresh profile refusal after original file removal.
It is deliberately a focused rerun, not a complete918-case configuration gate.

The [private checks](../../../tests/native/v5-02-x06-a3-review-evidence/private-probe.cpp)
use the unchanged SQL fixture builder, adding independent assertions and SQL
histories. They verify profile→embedded→profile transitions with accepted revisions
and intervening noops, then reject a correct-head wrong-family noop selector;
pinned profile revision projection remains byte-identical while a concurrent
corrupt replacement is staged and blocked at COMMIT, with fresh open refusing
after release; jointly forged interpretation/separate-V digest claims use exact
valid record encodings but still refuse under unknown semantics; and every raw
stored effective file path/byte/mode equals the independent vector input after
original source/runtime deletion. Exports of accepted/reverted revisions preserve
their exact differing source bytes and exclude provenance directories.

The [private dependency proof](../../../tests/native/v5-02-x06-a3-review-evidence/private-proof.json)
records recursive original response expansion, explicit existing PCMs and disabled
implicit modules. All 898 captured dependencies, including all 775 actual compile
dependencies and Ninja journals, retain exact mode/hash/mtime and other recorded
stat identity through compilation/execution. Raw source, argv, dependencies and
XML remain. Loader output records existing pinned libc++; no copied-runtime
claim is made. Only owned disposable object/executable outputs were removed after
proof, retaining their hashes/modes/sizes. No private verification failure occurred.

Author formatting/Doxygen over124 active sources, four unchanged independent
vector checks, source/runtime variants and711/711 focused evidence were audited
without redundant broad reruns. No complete configuration, release or Linux gate
is claimed here; no process RSS or natural host-exhaustion guarantee follows.

## Protection and limits

The [protection audit](../../../tests/native/v5-02-x06-a3-review-evidence/protection-audit.json)
compares independent full-content baseline/final hashes including root dotfiles,
symlink identities/targets, candidate/evidence, every prior round and all build
trees. Only `.git`, this report/evidence and the exact coordinator A3 round are
exempt. Only Debug verification outputs may change. Protected directory membership
and mode/device/inode remain exact; ordinary ancestor directory timestamp changes
are recorded separately. Every candidate, author/prior evidence, release and
install-release entry remains protected. Candidate hashes are rechecked after
verification. The review artifact manifest binds this report and all review
artifacts without a hash cycle.

Later X06 same-VM preparation, fresh profile transactions and shared invocation
budgets remain open, as do X07, X08 mandatory external-profile completion,
execution/RA01 and Linux. Public init creates format1. Latest complete approved
macOS gate remains C5's876/configuration.

**Debug and all reviewer files are released. No build/test process remains.**
