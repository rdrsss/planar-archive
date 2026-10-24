# X03-B2-A coder handoff

Candidate for fresh independent review; no self-approval. Shared checkout HEAD
3171c49e7b92d3abda01bfbb8bfb0c6b29d4c67a, dev/planar-v2-vernacular.
The candidate changes only the assigned ledger types/format/tests, narrow engine
wiring, owned status documentation and this round's support/evidence roots.
No current semantic Lua, prepared state, request/record codec, acceptance logic,
CLI or export policy changes. Existing history/export consumers need no edits.
No Planar tools, installed binary, operator database, commit, install, publish or
merge was used. All verification commands have finished; debug ownership is
released at handoff. Source/evidence will receive no further coder edits.

The stored payload variant adds request_v2 independently of immutable record_v1.
Codec2 is accepted only for request kind4 in exact format2; bounded decoding,
complete-frame digest, SQL/record kind agreement and directory-edge validation
are required. Digest bucket comparison retains exactly(codec,kind,digest), with
complete encoded-byte equality in each bucket. V1 read/digest/byte behavior is
preserved. Format2 directories reject request children of either codec; the v1
generic record graph behavior is explicitly preserved. Unknown SQL codec is
unsupported_state; an inconsistent codec2 frame version is corrupt, matching the
approved pure decoder's invalid_record classification. No request bytes, host
arguments or private digest appear in added failure messages/details.

The existing full chronological verifier checks v2 request actor/head against the
event and preserves all event/result/revision/reference correspondence checks.
Embedded format2 history/read/export work with unchanged v1 interpretation and
provenance. Profile requests can decode structurally as orphan stored records;
any accepted profile request refuses unsupported_state, requiring X06. The test
fixtures initialize a new native format2 target, then explicitly copy relational
rows through test-only SQL and replace selected request frames/digests. This is
neither native format2 acceptance proof nor a product conversion API. All format2
write/recovery guards remain, before writable handles. Public init remains v1.
B2-B retains interner/collision/CAS/replay/crash/full/recovery obligations; X06
and the full external-profile runtime gate remain open.

Baseline was frozen before product edits. baseline-full-manifest.json is the
first git-visible inventory (7,273 files); it omits ignored historical evidence.
baseline-complete-filesystem.json was then captured, still before product edits,
with 14,980 regular files and 2 symlinks. It is the authoritative complete baseline,
excluding only .git/, build/, and this new evidence/support root. Both original
manifests remain unchanged; no post-hoc baseline reconstruction occurred. Baseline
HEAD/status/diff and assigned owned snapshots are retained. final-source-manifest
and final-source/ freeze the exact source/docs before final focused tests.
candidate-manifest identifies complete owned snapshots and support, relative to
the repository root; artifact-manifest uses this evidence directory as path base.
owned.diff is against baseline-owned bytes, avoiding the unrelated rewrite diff.
The ownership audit reports no deleted/unexpected changed baseline files, stable
symlinks and unchanged final source hashes since verification.

The pre-fix regression adds the valid embedded codec2 row test, then fails at
open_existing: 1 matched test, 1 failure (regression-before.log). Product baseline
bytes are preserved in baseline-owned; the first regression's newly added test
source was not separately snapshotted before execution. Its command/build output
and original failing location are retained. Final tests include the same native
open/read check plus typed-value assertions. The first broad build failed only
on a test namespace typo. The first focused run was185/187 passing; two assertions
incorrectly expected unshared fixture request IDs and unsupported_state for a
pure malformed header. Raw failures are retained in focused.log/xml/LastTest.
Final corrected source was frozen before final verification.

Final serialized debug all+planar_tests build passes. final-focused.log/xml and
final-focused-LastTest.log retain188 passing storage/history/export/record/request/
prepared/acceptance/publication checks with zero skips,61.95 seconds. New tests
cover mixed codecs including accepted/noop/revert history, exact v1/v2 body
inequality, public history privacy, exact source export, backup/restore into new
targets, malformed/trailing/version/tag/length/digest data, actor/head/event/result/
reference mismatches, illegal kind/codec pairs including v1 rejection, private
profile acceptance refusal, structural profile-only decode and both request-child
codecs. Refusals preserve scratch ledger bytes and do not create a journal.
Old format1 storage/history and transaction tests are included; their transaction
fault coverage does not prove format2 acceptance.

Final inventory421 retains all416 baseline names and adds5; the saved final JUnit
and LastTest precede inventory. Independent record-v1/request-v2 vector oracles
verify10/2 vectors. Pinned formatting --Werror and Doxygen pass98 active sources.
no-work.log reports no work for all+planar_tests. test-summary and command result
JSON retain exact argv, timestamps/status and matched names. This is a focused
macOS debug slice; no release/full milestone/Linux/public profile or hardware
power-loss claim is made.
