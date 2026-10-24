# R14-D7-A coder handoff

Status: implementation candidate requiring independent review; no self-approval.
Shared checkout at HEAD 3171c49e7b92d3abda01bfbb8bfb0c6b29d4c67a.
Debug is released idle; release was untouched and remains idle.

## Scope and behavior

The identity-bound complete_acceptance overload checks retained prelude identity
before the locked request lookup, preserves exact late replay before CAS/candidate
inspection, and writes the existing shared completion receipt. Replay moves owned
saved bytes and publishes replayed before independent response-copy allocation.
Fresh accepted/noop receipts own saved bytes before COMMIT, publish uncertainty
immediately before COMMIT and committed only after confirmed success. A fresh
receipt is required for each attempt; refusal to reuse preserves earlier evidence.
The existing three-argument API keeps its original single response owner.

The narrow SQLite commit overload publishes a no-allocation confirmed-noncommit
observation before constructing C++ errors. An active transaction followed by
successful explicit rollback proves noncommit. Failed COMMIT followed by autocommit
alone remains uncertain. SQLite-owned diagnostic bytes preserve the original
COMMIT message across rollback. An outer stack guard reflects confirmed rollback
even if error allocation throws. Existing transaction generation, poisoning and
RAII ownership remain intact. No storage schema/codec, Lua, runtime admission,
public CLI or completion leaf changed. CMake adds the shared leaf dependency to
the production format module and its existing exact-source native variants.

## Verification and observed outcomes

Direct all/planar_tests builds and focused CTest execution were serialized. The
final run passed 188/188 entries with six CLI suites, zero failures and zero skips.
The final inventory contains 396 names and preserves all 391 baseline names; five
new cases cover identity replacement, late replay, capture/unavailable candidate,
busy-COMMIT receipts and primitive SQLite commit evidence. Existing accepted/noop,
mutation/verification fault and concurrent request/body/head tests inspect receipts.

Real busy COMMIT and deferred foreign-key constraint failures leave active
transactions: successful explicit rollback publishes confirmed noncommit. Busy
acceptance retains the proposed exact result bytes and permits a successful retry.
Commit-hook refusal automatically rolls back and conservatively leaves the receipt
uncertain with saved bytes. Nine enumerated VFS write/sync COMMIT failures report
false confirmed-noncommit evidence; existing state/reopen/retry checks pass.

Pinned formatter/Doxygen passed for 93 active C++ files. Unchanged vectors passed
12 hand byte, 20 upstream hash and 41 canonical/domain checks. All seven historical
generated variant families plus the fixed runtime harness, six D6 variants and C01
variant passed exact-source/original/assembly audits. Final build performed no work.
Raw command argv, UTC timing, exits, logs, JUnit and LastTest are retained here.
JUnit/LastTest were archived before final inventory. The intermediate 187-case run
passed; its relative JUnit output was copied from the build directory. Initial
build failure exposed unqualified shared types in generated namespaces; its raw
log is preserved. Subsequent builds passed. This feature added an absent API;
no baseline failing runtime-regression run is claimed.

## Candidate and preservation basis

Ten contemporaneous owned-file baseline snapshots support the exact candidate
diff; the frozen candidate includes those ten files and two new support scripts.
Candidate SHA256 is 5f66c57ff3224d4a79ea33f991adc45ca4b05998f1447e0228782f12308eb8c3.
Diff SHA256 is 39a63c3366fc9757b2ba59f8c9ec24510780929425058558928177ce9f5808fc.

Historical C01 baseline plus approved frozen candidate protects 14,145 paths.
All paths outside assigned ownership and recorded coordinator exceptions match;
the changed coordinator exception is R14-D6-C01.md. All 17 D6/13 C01 frozen
candidate files and each round's 67 manifested artifacts were verified. The exact
load contract body retains a842901180c15691cda24a0ddafcc708dcb88add85717b0080b2a1313e6f4ab8;
AGENTS.md remains a symlink to CLAUDE.md. This is an explicitly historical
protection basis: no full contemporaneous D7-A tree inventory or exhaustive new-path
audit is claimed. Current candidate and artifact hashes were rechecked.

## Limits

Verification is macOS ARM64 debug only. Allocation/exception retention follows
owned storage and no-throw publication/guard ordering; no deterministic C++
allocator-failure injection is claimed. No release/Linux, hardware power loss,
full R14 milestone, public provider/CLI or transport completion result is claimed.
Public integration remains D7-B. No installed binary/operator data, Planar tools,
queue, commit, installation, publication or merge was used. Only assigned source,
reference docs and new D7-A support/evidence files were written.
