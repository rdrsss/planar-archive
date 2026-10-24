# R14-A1 independent review

Disposition: **approve**, with one nonblocking documentation note. No product,
contract or safety finding remains for this behavior-preserving extraction.
This approves A1 only; accepted-history support and the R14 milestone remain
unimplemented/unreviewed.

## Scope and exact candidate

Fresh reviewer `/root/r14a1_reviewer` reviewed shared
`dev/planar-v2-vernacular`, HEAD
`3171c49e7b92d3abda01bfbb8bfb0c6b29d4c67a`. Read active CLAUDE/AGENTS instructions,
implementation/roadmap, v4, ledger contract, R14/A/A1 briefs, R13 milestone and
D01 approvals, exact candidate/diff and raw evidence. The intentional rewrite
and other participants' changes were preserved. Product/candidate and prior
evidence were read-only; owned writes are this report and
`tests/native/r14a1-review-evidence/`. No Planar tools/skills/queue, operator data,
installed binary, subagents, commits, installation, publication or merge.
Debug was held exclusively with direct serial commands and is released on delivery.
Release was not used.

Ten-file identity:
`f8fc71887c97f775d40556e1a6f15177759620f348f42d7f3a188621315305be`.
Candidate manifest SHA256:
`e40f36b7af5ecaa5c7b9efbea3afc2004393818b188772180c26417ff999b5db`.
All live files and repository-relative `candidate/` snapshots match the manifest;
identity is SHA256 of sorted compact JSON of its file map, without terminal LF.
The baseline-relative diff independently reconstructs exactly against the
recorded approved historical snapshots. Coder evidence manifest SHA256:
`e8aa6480046f096de33da0ce86e3320fac4a9c125243a32b01cef84c020dfe14`.
All sixty folder-relative entries match. All 4,081 local and 11,867 inherited
repository-relative protected hashes match, with the recorded coordinator
planning/Finder exclusions. Earlier baseline/XML-path audit corrections remain
preserved and explained; no historical evidence was overwritten.

## Technical assessment

`ledger.types` contains only the unchanged error enum/struct and record DTOs,
with a distinct nested namespace. `ledger.format` re-exports fully qualified
aliases, retaining owner/access/metadata policy. Four independent compile-time
checks prove exact type identity through the old names. Format implementation,
schema, SQLite, record/canonical codecs, provenance, interpretation, Lua, CLI and
generator sources remain protected and unchanged.

`tree_core` owns one shared construction/trie, budget, reconstruction and digest
implementation. An independent complete-body comparison against approved source
allows only whitespace, leaf qualification and the explicit typed reader
signature/invocation. There is no copied alternate validation path. The four new
core cases exercise valid binary/mode reconstruction and digest parity, invalid
input before callback effects, original storage failures/exceptions, identity
mismatch and nonroot empty directories. All existing bounds, malformed-tree,
rollback and reader tests still execute through this same core.

Callbacks are per-call synchronous `std::function` inputs, never retained;
algorithm state is local. Readers must provide codec-validated records, including
kind/variant agreement, as the interface documents. Core owns no storage cleanup,
SQL, raw handles, transaction, commit, global callback or retry. Each writer
adapter creates `abort_guard` before any conversion/allocation and marks success
only after the core returns success. Read adapters call `read_record` on the
same original owner, retaining sticky reader failures and writer invalidation.
The unchanged allocation/abort probe confirms pending prior records roll back
on allocation exceptions and invalid profiles. No populated-history support is
enabled: the original verifier still refuses revision/request rows.

The independently captured pure-core link includes core, inert types, record,
canonical/value, xxhash and test/runtime standard-library support only. It has
no format, SQLite, runtime, embedded source or Lua dependency. Both fault/collision
variants regenerate byte-for-byte with their configured outputs; exact reversible
namespace substitutions preserve fully qualified leaf aliases and collision
verification mutation remains disabled.

## Independent verification

All direct commands and raw logs are under
`tests/native/r14a1-review-evidence/`; commands/probe manifests include argv,
working directory and exit status. On macOS/arm64 debug:

- `cmake --build --preset debug --target all planar_tests`: pass. Final repeated
  build reports exactly `ninja: no work to do.`
- Focused `replacement.(ledger|canonical|hash|record|tree|provenance|interpretation)`:
  113 passed, all 109 prior case names plus four new core cases, zero skips.
- Raw bootstrap CLI acceptance: 247 real processes, 60 host-path checks, six
  native goldens, zero skips. Both canonical and record vector checks pass.
- Active pinned formatter/Doxygen: 54 files pass; two generated-helper sources
  pass their existing supplementary lint.
- Independent pure-core probe: three cases, 541 assertions pass. Covers all 14
  typed ledger errors with long embedded-NUL messages, failures after different
  pending record counts, read failure after traversal begins, synchronous callback
  lifetime/no retention, reentrant digest calls, output lifetime and original
  custom exceptions from build and read.
- Existing independent allocation/abort probe: one case, 46 assertions pass,
  plus four compile-time alias checks. Temporary object/executable hashes are
  captured before their removal.
- Final audit repeats candidate/evidence/protected hashes, complete algorithm
  parity, exact reversible generations and pure link checks successfully.

No full debug/release milestone gate or Linux execution was required/run in A1.
The registry snapshot is inventory, not a claim that all 230 cases were executed.
Integrated R14 retains those broader gates.

## Nonblocking note

`docs/testing.md:48` still describes the current registry as 226 tests/225 native.
The new independent `registry.json` contains 230 tests/229 native plus one Python
case, reflecting the four new core cases. The later A1 paragraph correctly states
109 prior plus four new focused cases. This is a low-priority current-summary
correction; it does not invalidate product behavior or focused verification.
Preserve historical milestone totals. No candidate edit was made during review.
