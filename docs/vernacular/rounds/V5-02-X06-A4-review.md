# V5-02-X06-A4 independent architecture review

Disposition: **request-changes**. One required architectural correction,
**X06A4-R1**, prevents approval of this exact documentation candidate. No product
implementation or build/test gate is reviewed or approved. The correction needs
a fresh search/coder/reviewer round; this reviewer has not edited the candidate.

Shared checkout `dev/planar-v2-vernacular`, starting/current HEAD
`3171c49e7b92d3abda01bfbb8bfb0c6b29d4c67a`, intentional dirty rewrite.
Reviewer ownership is only this report and
`tests/native/v5-02-x06-a4-review-evidence/`. All build trees, product/test/config
sources, candidate documents, author support/evidence and prior rounds remained
read-only. No builds, product tests, installed binary, Planar operational
tools/skills/roles/queue, delegation, commit, installation, publication, merge or
operator-data action occurred. No others' work was reverted.

## Required change: X06A4-R1 — Admit the known selected workspace file before registration

Severity: **high**. The known-load construction protocol at
[architecture lines 183–186](../profile-preparation-architecture.md#constructor-parse-and-consumer-protocol)
explicitly admits the workspace **root**, registry file and ledger selection
before external registration, but omits the known selected workspace declaration
file. Preparation step1 at lines226–231 first binds the retained root and discovers
its files after registration and final parsing. The source-capture description
at lines77–83 likewise lists only root authority for the workspace.

Root containment does not prove selected-file nonaliasing. For example, let
`trusted/modules/a.lua` and `workspace/workspace.lua` be hard links to the same
regular file, with disjoint trusted and workspace directories. Trusted capture
and supplied-root admission can succeed: the directories neither equal nor nest,
and root admission does not open/enumerate the workspace leaf. The proposed
sequence then evaluates the external module before discovering the selected
workspace file's inode alias. A later refusal before declaration evaluation
cannot restore the required pre-registration order. Missing/replaced selected
workspace files have the same initial-admission timing problem.

The approved [X-CAP contract](../external-profile-contract.md#x-cap--strict-capture-and-trust),
lines150–155, explicitly rejects aliases with declaration files; lines179–182
require every known supplied native root/file slot to be checked before external
Lua. Lines194–204 require original initial receipts to survive later seals.
This is a known supplied selector, not a newly discovered reference.

The source already distinguishes the two authorities:

- [capture/admission.cpp](../../../src/engine/capture/admission.cpp), lines225–245,
  checks containment for supplied roots and compares regular-file identities only
  for selected files. `admit_root`, lines321–327, cannot provide the omitted leaf
  selection; `admit_file`, lines338–353, creates it separately.
- [runtime/runtime.cpp](../../../src/engine/runtime/runtime.cpp), lines4807–4809,
  admits an extension workspace's root **and** its explicit declaration file with
  `admission_file_role::declaration`. Lines5071–5076 then bind the original root
  and explicitly read with the original retained file receipt before `load`.
  This preserves the existing original-root/selected-file correspondence rule.
- [admission.t.cpp](../../../src/engine/capture/admission.t.cpp), lines74–80,
  contains existing selected hard-link coverage for every supplied-file role.
  It was inspected, not executed in this documentation round.

Required correction: specify metadata-only admission of both the known workspace
root and selected workspace declaration file **before** external registration;
retain both original capabilities through final parsing, preparation and commit.
The source-capture bound path must consume/verify the original file authority
alongside its original root before first content access, using the existing
`same_selection` correspondence behavior rather than replacing that original
selection with discovery. Add this ownership/phase to the factory, preparation,
source-capture prerequisite and future acceptance matrix consistently. Future
code evidence must prove the disjoint-directory hard-link case refuses before
external evaluation and original file/root replacement remains terminal without
unbound retry. No new product choice or exact-contract amendment is needed.

## Remaining architecture assessment

No additional required finding was identified in the bounded static review.
These observations do not approve the candidate while R1 remains open.

The proposed downward extraction addresses a real dependency:
[runtime.cppm](../../../src/engine/runtime/runtime.cppm) imports history.read;
[read.cppm](../../../src/engine/history/read.cppm) imports ledger.format; and
[format.cppm](../../../src/engine/ledger/format.cppm) currently imports embedded
prepared acceptance. Moving completion upward while extracting inert storage
vocabulary and storage mechanics downward avoids a runtime/format acceptance
cycle. The document explicitly forbids a format compatibility re-export and
stages the embedded-only extraction before runtime prepared types and the later
profile overload. The reviewed lower codecs/types do not require runtime.
The [source assessment](../../../tests/native/v5-02-x06-a4-review-evidence/source-assessment.json)
records inspected source hashes/imports and a representative proposed acyclic core
graph. This is a static design check, not a compiled or exhaustive target proof.

The private runtime constructor/adoption path is coherent within one named
module: no foreign-module friendship or caller-built aggregate mints the prepared
profile. Known and opaque modes cannot upgrade one another. The original
capture/VM persists through final parsing and private extension adoption. Public
snapshot/codec/shape APIs retain structural authority only. Native source closure,
registry and effective provenance derive from private originals; canonical-owned
v2 results sever aliases. Both receipt owners and dormant VM/accounting lifetime
remain with the prepared object. R1 must add the omitted initial file authority
to that lifetime.

Current runtime phase0/all-F/phase1/phase2 already supplies the all-job semantic
path and returns before selection when no selection exists
([runtime.cpp](../../../src/engine/runtime/runtime.cpp), lines4529–4580).
The candidate correctly requires a private active-frame refactor rather than
calling the existing public validation path after routing. It retains phase8,
all-job F, rooted conversion, first failure and cumulative counters without
claiming current separate metadata/content/declaration/trusted-VM metrics are
already a shared budget. Exact common-budget composition remains a later gate.

The higher common transaction consumer can use concrete fixed native verifiers
without executing Lua or a caller callback. The proposed finite storage seam
keeps SQL/connection ownership below semantics. Extraction must preserve the
actual current entry and precommit boundaries, exact byte comparison and
allocation/commit evidence, as the document requires:
[format.cpp](../../../src/engine/ledger/format.cpp), lines1407–1430 and1621–1833.
Early lookup remains outside the factory; exact late replay precedes candidate
inspection and CAS; profile family/bound intent is checked only after late miss
and CAS. The existing explicit fresh-profile refusal remains through extraction.
Receipt publication before COMMIT and confirmed-noncommit handling remain
explicit. Lower storage authority is accurately scoped separately from semantic
attestation against ordinary native API clients.

The follow-up rounds have bounded ownership, independently reviewed extraction
before profile writing, adversarial origin/alias/move/failure/replay/transaction
checks and explicit X06/X07/X08 boundaries. The API notation is correctly labeled
uncompiled. Documentation-only verification cannot prove implementation,
allocation accounting, generated runtime variants or transaction races; those
remain required future evidence.

## Exact candidate, checks and protection

Review considered the active guide, implementation/roadmap and v5 trust/history
direction, entire A4 brief including the final documentation-only amendment,
approved A1/A2/A3, X06 path, exact X-CAP/X-SHAPE/X-LIMIT/X-STORE and the underlying
runtime/native capture/preparation/storage/CLI sources cited above.

| Identity | SHA256 |
| --- | --- |
| Candidate JSON, ten files | `eec7fe7ff64bcdc46f52993491e3ddfcaaa82938be52b71fecff108a3121deb5` |
| Exact preedit-relative diff | `fcc7366fa05d666c7874ca55dbd3ef852838f190509f18a86c16b4b609921fb7` |
| Author report | `e9e98b2c295817feb211fecf7c7baf1106cae6d151cd60f76b74d801aaf0a664` |
| Author artifact manifest | `10be3227d35e8d9a4ebc2692ab32c73b75c1804210c17ac409740a6d34fce17d` |

The independent [candidate audit](../../../tests/native/v5-02-x06-a4-review-evidence/candidate-audit.json)
verifies all ten current candidate hashes, all17 author evidence entries and
three support files with exact membership/size/hash, original preedit copies
against the author baseline, byte-for-byte diff reconstruction, nine unchanged
historical document bodies and535 URI-decoded local file links. The first audit
attempt reconstructed files in JSON key order; its order-only assertion was
corrected to the frozen diff's file order before the successful audit. No
candidate or author artifact was changed.

The author's full-content94187/94188 baseline/final manifests independently
reconcile with only ten authorized documentation changes/new-file addition.
The review's full-hash94214-member baseline matches every author-final protected
entry; its26 additional entries are the author evidence/support subtrees.
The independent [protection audit](../../../tests/native/v5-02-x06-a4-review-evidence/protection-audit.json)
compares one full-content baseline and one full-content final pass, including root
dotfiles, link identity/target, every build tree, all candidate/author artifacts
and prior A1/A2/A3 evidence. Only `.git`, this report/evidence and the exact
coordinator A4 round are exempt. Directory identity/mode/membership remains
protected; permitted ancestor directory timestamp changes are recorded separately.
AGENTS.md remains the original symlink to CLAUDE.md; compile_commands.json retains
its original symlink. No build tree has a mutable exemption. Final bookkeeping
rechecks the frozen candidate/artifact hashes without repeating the full scan.
The review artifact manifest binds this report and review artifacts without a
hash cycle.

No product build or test was run. Current inventory remains918; the latest
complete independently approved macOS gate remains876/configuration. Public init
creates format1. A1/A2/A3 approval is unchanged; fresh profile acceptance,
preparation/transactions/shared budgets, X07/X08, execution/RA01 and Linux remain
open. R1 requires fresh corrective review before dependent product work.

**All reviewer files are released. Every build tree remained protected and no
build/test process was started.**
