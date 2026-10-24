# R13-B1 independent review

Disposition: **request-changes**. One blocking finding: a verified reader can
silently switch to a newer snapshot after a storage read error. Preserve this
candidate and finding for a fresh corrective search/coder/reviewer round.
No product fixes were made during review.

## Candidate and authority

Fresh reviewer `/root/r13b1_reviewer` reviewed the shared
`dev/planar-v2-vernacular` checkout at HEAD
`3171c49e7b92d3abda01bfbb8bfb0c6b29d4c67a`. Read the active instructions,
implementation/roadmap, v4 and ledger contract, R13-B/B1 briefs, approved R13-A
review, relevant R12 boundaries, all candidate files/diff, raw test logs and
exact generation/protection evidence. Writes are limited to this report and
`tests/native/r13b1-review-evidence/`. No product edits, unrelated reverts,
Planar tools, legacy queue/skills, operator data, installed binaries, commits,
installation, publication or merge. Debug was exclusively held for direct
serialized checks and is released upon delivery.

The ten-file candidate identity is
`60f930eba1be7213dfb0c571e9c7f9bfcfc7f87522af3b6a2223bf296eb9c68a`;
`tests/native/r13b1-evidence/candidate.json` SHA256 is
`a654e1fe4444074f8b59ac82e031f5fbd9b148bf43fc5bb94de71fc9377e71e7`.
Fifty-entry `evidence.json` SHA256 is
`d52c516e07573c326960b4a0ddd53b08d04c19ddc9668f44e73b100367d9c4a0`.
All candidate files/snapshots, coder evidence entries and 1,834 protected files
match before and after this review.

## Finding F1 — High: read error releases the verified snapshot without invalidating its owner

At `src/engine/ledger/format.cpp:614`, `format_file::read_record` returns
`load_record(_state->db, id)` directly. The protected SQLite primitive's
`failed` path (`src/engine/ledger/sqlite.cpp:56`) deliberately rolls back any
active transaction on prepare/step errors, leaving the connection usable after
a successful rollback. The format owner retains its state and does not detect
that its pinned verified transaction has ended. A subsequent `read_record`
therefore operates in an implicit fresh snapshot without repeating format,
record-set or initialized-history verification.

This violates the opaque owner's documented pinned-snapshot guarantee and
R13-B's same-snapshot read requirement. It can expose records committed after
the original verification; a caller cannot safely compose multiple reads from
one owner as a consistent verified ledger view after recovering from an I/O
failure. The defect requires a real storage read error, not hostile file swaps.

Independent scratch-only reproduction links the unchanged candidate libraries:

1. Commit eight distinct 1 MiB file records, then open a verified reader.
2. Inject exactly one real `SQLITE_IOERR_READ` through the pinned SQLite VFS
   while the reader accesses an evicted early record. Require the fault hit and
   the public `io` refusal; then disable the injection.
3. Keep the original reader alive. An independent record writer commits new
   record 9 successfully, demonstrating the original read lock has ended.
4. Read record 9 through the original owner: it succeeds and returns the newly
   committed payload, despite record 9 being absent from its verified snapshot.

Evidence source: `tests/native/r13b1-review-evidence/snapshot.t.cpp:141`;
raw log: `snapshot-red.log`; exact build/link commands: `probe-build.log`.
The final run has two cases, one pass / one failure, 47 assertions, 46 pass /
one failure, Catch2 exit 42. The only failure is the assertion rejecting the
reader's exposure of new record 9. A paired writer case passes: its real read
error invalidates the writer and rolls back pending record 9.

Required correction: after a storage failure that can end the pinned reader
transaction, make that owner unusable until an explicit safe reopen, or otherwise
prove and preserve the original verified snapshot. Add this regression without
relaxing fault-hit, typed-failure, independent-commit or no-new-record checks.
Ordinary invalid-ID/not-found refusals need not destroy an intact snapshot.
The primitive's intentional rollback policy remains protected.

## Other assessment and verification

No additional blocking finding was identified in the assigned B1 scope.
Independent writers own their connection/transaction; caller readers are not
converted into writers. BEGIN IMMEDIATE and in-transaction revalidation precede
insertion; guarded MAX+1 allocation and failure/abandonment/failed-commit paths
prevent successful partial commits. Complete validation streams one bounded
record/bucket candidate at a time, checks lengths before owned BLOB/name copies,
uses the approved codec/digest, requires ordered encoded-edge equality and
strictly older references, and refuses non-directory edges and ambiguous
buckets. No whole-history inventory is accumulated in C++ memory. This does not
establish B2 expanded-tree aggregate limits or R14 semantics.

The generated collision helper uses original record types and encoder, redirects
both production digest call sites in the renamed verification variant, and has
no product override. Exact regeneration/reversal of B1 and the existing C2
publication variant passes. Mutation RED removes only intern byte equality;
its missing collision refusal and leaked-prefix assertions fail across all
four kinds, and its local-child-ID case fails at the direct refusal assertion.
The independent ambiguous-bucket validator still passes under that mutation.
The corrected harness issues documented by the coder do not relax product
refusals; changing malformed stored records from unsupported to corrupt is the
intended transition to actual record validation.

| Independent check | Result |
| --- | --- |
| Candidate/evidence/protection audit, before and after | Ten current files/snapshots, fifty evidence entries, 1,834 protected files match. |
| `cmake --build build/rewrite-debug --target all planar_tests` | Exit 0; `ninja: no work to do.` |
| Focused ledger/canonical/hash/record CTest selection | 76 pass, zero failures/skips, independently retained JUnit. |
| Coder protection and both generated-variant audit | Exit 0; exact reversible regeneration, normal state restored. |
| Independent reader/writer real-read-fault probe | Reader regression fails as described; writer rollback/invalidation passes. |

Raw commands, logs, source and reproducible compilation are retained in the
[review evidence](../../../tests/native/r13b1-review-evidence/README.md).
An initial supplemental writer test assumed an evicted record that was cached;
the preserved initial source/log shows its missed-fault assertion. The final
writer test checks successive old records until an actual fault hits, retains
all safety assertions, and passes. The reader bug reproduces in both versions.

These are macOS Apple Silicon debug checks. Coder vector/lint evidence was
reviewed; unchanged vector generators and full lint were not redundantly rerun.
Full integrated R13 debug/release gates, Linux execution, B2 tree behavior,
provenance and R14 acceptance remain unapproved future work. The request for
changes prevents advancing dependent B2 implementation until a fresh corrective
round is independently approved.
