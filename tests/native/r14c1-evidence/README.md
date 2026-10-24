# R14-C1 typed historical read evidence

The shared checkout is used directly. Product/test edits are confined to
`format.cppm`, `format.cpp`, additive cases in `history_verification.t.cpp`, and
architecture/testing references. Engine CMake is included in the owned boundary
but remains byte-identical. `baseline/` was captured before edits. No schema,
shared type/error vocabulary, SQLite wrapper, pure codec, generator, Lua, CLI,
vendor or prior evidence file was changed. No Planar tool, operator database,
installation, publishing or commit was used.

`protection-before.json` captures nonowned source/evidence before edits.
Coordinator-owned planning/status records under `docs/vernacular/` were excluded
because the coordinator updates them concurrently. `historical-protection.json`
additionally checks inherited, previously frozen historical rounds and reference
contracts/designs against B2's unchanged protection evidence. The remaining
excluded planning files are implementation/roadmap and R14 round/status records.
Git internals, build directories and `.DS_Store` metadata are excluded. The audit
checks 12,363 protected hashes, unexpected nonplanning files, the preserved AGENTS
symlink, exact schema/init payload, exact original B2 test prefix and unchanged
engine CMake. Both namespace variants regenerate exactly and reverse identically.

The five methods use the existing opaque verified owner and snapshot. Revision,
event and request misses are successful optional misses; `read_record` retains
its original missing-record failure. Every actual failed read and invalid
argument poisons all old/new reads while preserving cached metadata. Returned
DTOs own their strings. Pages are ascending/exclusive, cursor 0..signed64max,
explicit limit 1..100; next_after is present only when the same pinned snapshot
contains more events. No history-wide vector/cache, raw handle or write API was
added. Request body bytes are the exact stored record_v1 encoding (up to 64MiB),
not reconstructed serialization. Native private body bytes contain host arguments
and are deliberately excluded from any future public history rendering.

Eight additive cases cover initialized ledgers; exact saved rows/body/results
for B2's initial revision1/event2, change revision3/event3, no-op event4 and revert
revision8/event5; state-at-event via pure after-state plus revision; successful
misses; malformed IDs/keys/pages and sticky invalidation; moves and owned result
lifetimes; 115-event bounded pagination; raw unsupported historical
interpretation; and DELETE snapshot commit blocking/release/fresh reopen.
The scoped VFS is a test-only additive copy of the existing snapshot test seam.
It injects actual SQLITE_IOERR_READ during an 8MiB private request read, observes
exactly one hit, verifies all reads refuse, and proves a fresh owner can read the
unchanged valid ledger. Fixture publication remains test-only SQL, not product
acceptance or a receipt-verified writer.

The baseline API cannot compile the additive tests (`baseline-api-absent.txt`,
nonzero exit), demonstrating missing feature surface rather than a previous bug.
Candidate files were saved, baseline format files substituted, and the candidate
restored in a finally block before a full rebuild. The initial empty-ledger test
used a macOS temporary path containing `/var`'s symlink and failed at initialization;
canonicalizing that test-owned root corrected the fixture. Both failure logs are
retained. Preliminary focused and CLI runs accidentally overlapped; their logs
are retained under `first-overlap`. The final CLI and focused runs were repeated
sequentially, with no overlapping build or test process, before the final gate.

Final evidence: 151 focused cases including every prior 143 case and eight new
cases, zero skips; CLI247 real processes; both vector checks; formatting and
Doxygen on 64 active C++ files; exact generated/protected audits; and no-change
all-plus-tests build with zero Ninja steps. The final focused run alone is the
full current focused result; the earlier six-case targeted run is preliminary.
JUnit paths in the first targeted runs were relative to the CTest build directory;
their XML was copied here under its corresponding preliminary name.

`candidate.json` contains the embedded identity derived from its sorted file hash
map. Its own SHA256 is a distinct manifest-file hash. `candidate/` and
`candidate.diff` freeze exact owned contents and changes from baseline.
`evidence.json` hashes raw round-local evidence. The coder returns this candidate
for fresh independent review and does not approve it. Verification is macOS debug;
release/Linux and integrated R14 acceptance/history CLI gates remain outstanding.
