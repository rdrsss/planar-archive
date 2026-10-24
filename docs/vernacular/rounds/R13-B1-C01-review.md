# R13-B1-C01 independent review

Disposition: **approve**. Original B1 finding F1 is resolved. No blocking or
requested-change finding remains in the full corrected B1 candidate. This
approves typed record storage and the reader correction, not B2 trees, R13-C
provenance, the integrated R13 milestone or R14 acceptance.

## Scope and exact candidate

Fresh reviewer `/root/r13b1c01_reviewer` reviewed shared
`dev/planar-v2-vernacular` at HEAD
`3171c49e7b92d3abda01bfbb8bfb0c6b29d4c67a`. Read the active instructions,
implementation plan/roadmap, v4 and ledger contract, R13/B/B1/C01 briefs,
original B1 review and R13-A approval. Reviewed the full eleven-file candidate,
both baseline and correction diffs, original fault reproduction, permanent
regression, stored RED/GREEN and collision evidence, generated variants and
affected references. Product files were read-only; writes were limited to this
report and `tests/native/r13b1c01-review-evidence/`. No unrelated edits were
reverted. No Planar tools/skills/queue, operator data, installed binary, commit,
installation, publication or merge was used.

Candidate manifest `tests/native/r13b1c01-evidence/candidate.json` SHA256:
`6021da40ecddb1511aa8cf8bbedf57cc10ff046efb94b203c475312bebabef15`.
Full candidate identity:
`53542e7dd4e6dff86c8f0c45beb4c3b13c9c30b8ab6d172dc17263f8b714c7b4`.
Candidate keys are repository-relative and snapshots are under that evidence
directory's `candidate/`. Identity is SHA256 of the compact sorted-key files
map, UTF-8 without terminal LF. All eleven live files and snapshots match.

Coder evidence manifest SHA256:
`578697c6cbc7055c20737da3ed884f539d57b205c23096fbb07496a6217a673c`.
All 48 folder-relative entries match. The 50 original B1 evidence entries,
16 original reviewer evidence entries and all 1,834 protected files also match.
Both full-candidate and correction diffs were independently reconstructed from
their distinct preserved baselines and compare byte-for-byte. Final audit
rechecked these identities after verification.

## Technical assessment

At `src/engine/ledger/format.cpp:614`, the reader checks its private sticky
`readable` state before calling `load_record`, preserves the original expected
result, and clears validity on every failed read. Later calls refuse before any
SQLite operation. The owner retains immutable cached metadata and its existing
RAII resources. Default moves transfer the same state; moved-from readers refuse
and move assignment releases the destination's former snapshot. No primitive,
schema, retry, replacement snapshot or writer behavior changed. Thread confinement
is documented; mutation behind the const read method does not imply concurrent
read safety. The approved conservative policy includes invalid IDs and not-found.
Those refusals need not release the retained lock immediately, as documented.

The permanent VFS regression retains an actual single reader IOERR_READ hit,
the original typed `io` refusal, and an independently successful record-9 commit
while the failed reader remains alive. It then rejects new-record exposure,
checks cached metadata and invalidity after moves, rejects old-record reads,
and proves an explicit fresh reader sees the new record. The writer case still
requires a real fault and verifies pending-prefix rollback and permanent writer
refusal. The ordinary failure test separately exercises zero/overflow IDs and
missing records, original refusal categories, move assignment and explicit reopen.
These checks were strengthened from the preserved reproduction; none of the
fault-hit, typed-error, independent-commit or rollback checks was weakened.

The preserved before-fix run has two cases and 47 assertions, with exactly one
failure: the original reader exposes newly committed record 9. GREEN has two
cases and 59 passing assertions. The change from `not_found` to unavailable in
the earlier sequential invalid-read test follows the explicitly adopted sticky
policy; the new fresh-owner loop still checks the original `not_found` result.

The complete B1 review confirms independent write ownership, BEGIN IMMEDIATE
and revalidation, guarded MAX+1 allocation, explicit single commit, and complete
rollback after failed operations/commit or abandonment. Bounded streaming passes
validate every record encoding/hash/kind, exact ordered edges, older child IDs,
child kind/digest and all bucket candidates without accumulating total history.
Record-only backup/recovery uses that validation; revision/request history stays
unsupported. Exact directory equality includes local IDs despite portable hashes
omitting them. Generated collision tests import the original codec/types and
redirect both digest sites only in the test module. Original collision mutation
evidence remains sensitive to removal of equality across all four kinds and
local directory IDs; ambiguous buckets are checked in both orders. No product
hash override or new dependency cycle was introduced.

## Independent verification

Commands, raw logs, JUnit and a reproducible independent audit are retained in
[review evidence](../../../tests/native/r13b1c01-review-evidence/README.md).
Direct debug checks ran serially with exclusive build-directory ownership.

| Check | Result |
| --- | --- |
| `cmake --build --preset debug --target all planar_tests` | Exit 0; no work, including the final repeated build. |
| `planar_ledger_format_tests '[snapshot]'` | Two cases, 59 passing assertions, zero skips; real faults and independent commit recorded. |
| Focused CTest `replacement.(ledger\|canonical\|hash\|record)` | 79 passed, zero failures/skips; independent JUnit audited. |
| Existing vector generator `--check` | 12 hand-byte, 20 upstream-hash, 41 canonical/domain vectors pass. |
| Record vector generator `--check` | Ten independent vectors pass. |
| Active formatting/Doxygen | 41 source files pass. |
| Supplementary helper formatting/Doxygen | Two files pass, zero warnings. |
| Independent protection/generation audit | All manifests/protected files match; exact reversible fresh C2/B1 generation matches configured variants; normal collision variant and both digest sites verified. |

These are macOS Apple Silicon debug checks. Complete integrated debug/release
gates and Linux execution remain unestablished here; aggregate expanded-tree
limits and complete reconstruction belong to B2, provenance to C, and accepted
history/export to R14. Debug is released on delivery. No product choice remains
open within this reviewed candidate.

The independent evidence manifest has 18 folder-relative entries; its SHA256 is
`0b7856d0374ffbe1f9b9fcfae73f5af2b7d685398f3e6a773025252a77845385`.
The report is outside that manifest and receives a separate delivery hash.
