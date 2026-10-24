# R12-B independent review

Disposition: **request-changes**. Two bounded correctness/resource findings need
fresh corrective search, implementation and review before R12-B approval.
The candidate and original evidence remain unchanged; no fixes were made during
this review.

## Identity and authority

Fresh reviewer `/root/r12b_reviewer`, ordinary gpt-6.1-sol with high reasoning,
reviewed the shared `dev/planar-v2-vernacular` checkout at starting HEAD
`3171c49e7b92d3abda01bfbb8bfb0c6b29d4c67a`. The large existing rewrite is
intentional. Reviewer writes were limited to this report and
`tests/native/r12b-review-evidence/`; other work was preserved. No legacy tools,
installed databases, default database paths, commits, installation, publication
or merge were used. Debug build access was exclusive and serialized.

The seven candidate files are frozen by
`tests/native/r12b-evidence/candidate-manifest.json`, SHA256
`0180eea28ea3d0efd991290b4a6d3c358a40aca5556f8068cd29cfcbf114039d`.
The original 24-entry evidence manifest has SHA256
`fe1a8995cf44a6dba60ff31c4bb62f6bcd0c73d688f21241d5a068a4dd524122`.
All entries matched independently before and after review probes. The three
approved SQLite primitive source/test files also match the protected baseline.
See `identity-before.json`, `identity-after.json` and `primitive-identity.json`
in the reviewer evidence directory. Reviewer `manifest.json` freezes this report
and every reviewer artifact except the manifest itself.

Review basis: `CLAUDE.md`, implementation plan, roadmap, v4 persistence and
acceptance sections, normative `ledger-contract.md`, R12/R12-B briefs, original
handoff/raw logs, complete candidate implementation/interface/tests/schema
fixture, affected product references and underlying approved SQLite bindings.
The R12-B boundary supports only initialized empty state; full record/history
semantics and recognized hot-journal recovery are deliberately later work.

## Findings

### R12-B-F01 — P1: refuse existing destination sidecars before publishing init

`src/engine/ledger/format.cpp:565` checks only the target filename. Initialization
then verifies the staging pathname at line 603 and publishes at line 613 without
checking the destination's `-journal`, `-wal` or `-shm` siblings. Consequently,
creating an orphan sidecar first and calling `initialize_new(target, ...)`
returns success and creates the target, while immediately calling
`open_existing(target)` returns `recovery_required`. The target and unrelated
sidecar now occupy the same SQLite namespace despite successful initialization.
This violates successful initialization's usable initialized-state expectation
and risks coupling a new database to unrelated recovery artifacts in later work.

The independent probe reproduces this for all three suffixes with each of regular
file, symlink and directory sidecars: **nine RED cases**. Every init returns
success; every immediate open refuses; all pre-existing bytes/links/directories
are preserved, but a new ledger is published. Raw before/after snapshots and
native responses are in `adversarial.json`; the smaller first reproduction is
`sidecar-publication.json`.

Required correction: conservatively refuse existing destination-sidecar entries,
without following links, deleting entries or publishing a target; check the
publication boundary consistently with the cooperative directory ownership
contract. Preserve no-replace publication and uncertainty behavior. Add native
regressions for every suffix and regular/link/directory forms, proving the whole
original directory snapshot remains unchanged and no target exists. This is not
a request to implement recovery in B.

### R12-B-F02 — P2: use byte bounds before copying persisted metadata text

`src/engine/ledger/format.cpp:373` selects `length(identity)`,
`length(workspace_id)` and `length(created_at)` as guards, then line 386 copies
the complete text through the primitive. SQLite `length(TEXT)` counts characters
only through the first NUL. A 32-character valid identity followed by NUL and a
large suffix still satisfies the exact schema's `CHECK(length(identity)=32)` and
the opener's length guard. The complete suffix is allocated before native
`valid_identity` rejects it. Thus the advertised small metadata guard does not
bound the owned copy. The same guard pattern at lines 319–333 for schema text
needs auditing for byte-count correctness.

The independent probe changes identity to its valid 32-byte prefix, NUL and
32 MiB of suffix without disabling its CHECK constraint. SQLite reports character
length 32 and actual BLOB byte length **33,554,465**. Opening correctly returns
`corrupt` and preserves bytes, but maximum resident memory grows to
**74,039,296 bytes**, compared with **9,027,584 bytes** for a normal initialized
ledger using the same executable. The input can grow toward SQLite's much larger
text limit; native validation occurs after the avoidable unbounded copy. This
probe establishes the missing byte guard, not a measured OOM or a total SQLite
memory cap.

Required correction: validate actual persisted byte lengths before the wrapper
copies text, including embedded-NUL cases. Inspect pinned SQLite `octet_length`
and equivalent byte-length mechanisms; audit all schema/metadata guard columns.
Do not solve this with handwritten SQL/JSON parsers. Add meaningful NUL-suffixed
and multibyte regressions with bounded-copy evidence. Raw lengths/results are in
`adversarial.json`, and `/usr/bin/time -l` output is in
`nul-metadata-memory.log` and `baseline-memory.log`.

## Verification and positive review evidence

The reviewer executed directly:

```sh
cmake --build build/rewrite-debug --target all planar_tests
ctest --test-dir build/rewrite-debug -R 'replacement.ledger' --output-on-failure --output-junit <repo>/tests/native/r12b-review-evidence/focused.xml
python3 tests/native/r12b-review-evidence/adversarial.py
```

The first build reported `ninja: no work to do.` Focused CTest selected **26**,
passed **26**, failed **0**, skipped **0** (13 primitive and 13 format cases).
`focused-counts.json`, `focused.xml` and `focused.log` retain raw evidence.
The independent native probe was compiled against the exact current module maps
and static libraries; `compile.log` and `link.log` contain exact commands and
results. `probe.cpp`, binary/object and `adversarial.py` are retained. Every probe
used an explicit temporary root, which was removed after observation.

The adversarial script records **24 checks: 15 pass, 9 RED**. Passing checks cover
0600 publication with no stage residue; read-only and writable reopen without
mutation; retained snapshot preventing a competing writer commit; distinct OS
random identities; unchanged refusals for symlink, directory, FIFO and missing
targets; extra event, changed JSON payload, changed event time, missing metadata
and bad format invariants; and corruption refusal for the NUL-suffixed metadata
case. The last case still exposes F02's missing allocation bound.

Source/test review confirms the exact seven STRICT tables, one explicit index,
13 immutable triggers and expected automatic indexes; full stored-SQL comparison
against an independent SQLite-parsed normative fixture; application/user/encoding
and DELETE policy checks; FK/synchronous/trusted-schema session enforcement;
integrity/FK validation; canonical Glaze initialization JSON; fixed `planar:init`
actor; metadata/event/current projection in one transaction; opaque metadata-only
API and retained read snapshot; read-only verification before writable transition;
original-component/no-follow traversal and retained inode checks; exclusive owned
staging and database precreation; commit/close/file-and-directory sync; atomic
no-replace hard-link publication; owned cleanup; and no published-target removal
on uncertainty. The original corrected malformed-btree classification RED/GREEN
evidence remains intact. No new finding was raised in those reviewed portions.

Original coder formatting/Doxygen evidence was inspected: 35 active C++ files
passed, and the original no-change build did no work. The independently repeated
build also did no work. A second lint run and broad whole-registry rerun were not
needed to establish these two findings; no candidate source changed.

## Limits and next step

Executed platform: macOS Apple Silicon debug. This review does not establish
Linux execution, release/full-registry integration, power-loss behavior or
adversarial filesystem-swap resistance. Cooperative directory ownership remains
a precondition. R12-C still owns recognized hot-journal recovery, backup/restore,
VFS/disk-full faults, subprocess termination/restart, broader concurrency and
integrated gates. R13/R14 own record/accepted-history semantics; explicit
unsupported populated state is correct for B and is not a review failure.

Start a fresh corrective round with new search, coder and reviewer agents,
preserving both findings, this candidate and all raw evidence. Recheck hashes
before dependent work advances. This report does not approve R12-B or complete
R12.
