# R14-D7-A independent review

Disposition: **approve**. No blocking findings or requested changes for the
frozen native D7-A slice. This approval does not close D7-B or full R14.

Reviewed the shared checkout on `dev/planar-v2-vernacular`, starting HEAD
`3171c49e7b92d3abda01bfbb8bfb0c6b29d4c67a`, under the repository rewrite guide,
implementation plan, roadmap, v5 direction, D7-A scope/SQLite refinement and the
exact load atomic-completion contract. Review ownership was limited to this
report and `tests/native/r14-d7-a-review-evidence/`. No implementation, prior
evidence or unrelated rewrite files were changed or reverted. No Planar tools,
queue, installed binary, operator data, commit, install, publish or merge was used.

## Exact candidate and preservation

The 12-file repository-relative candidate manifest has SHA256
`5f66c57ff3224d4a79ea33f991adc45ca4b05998f1447e0228782f12308eb8c3`;
`owned.diff` has SHA256
`39a63c3366fc9757b2ba59f8c9ec24510780929425058558928177ce9f5808fc`.
The 48-artifact evidence-relative manifest has SHA256
`22806bdbabe149e38d9bfed9f4d84c28ce9122ca5ec4b62a4c183091fd2ebdd3`.
All current candidate files, their frozen snapshots, ten contemporaneous owned
baseline snapshots and all manifested artifacts matched. Independently rebuilt
the unified diff from the frozen baseline/candidate and matched it byte for byte.
Rechecked current candidate hashes after verification.

Verified the retained D6 17-file and C01 13-file candidate snapshots, their 67
artifacts each and handoff identities. The explicitly historical 14,145-path
protection basis has no unexpected changes outside D7-A ownership and recorded
coordinator exceptions; the changed exception is `R14-D6-C01.md`. This is not a
complete contemporaneous D7-A starting-tree inventory or exhaustive new-path
audit. `AGENTS.md` remains the symlink to `CLAUDE.md`. The protected load-contract
body remains SHA256
`a842901180c15691cda24a0ddafcc708dcb88add85717b0080b2a1313e6f4ab8`.
These checks are recorded in the review evidence's `identity-audit.json`.

## Implementation assessment

`format.cpp:1438` preserves the three-argument API and its single returned saved
response owner. The overload shares one implementation with an optional expected
identity; the public receipt overload always engages that identity check, including
for an empty supplied identity. The writable opener and locked full verification
remain authoritative. `format.cpp:1465` compares the retained prelude identity
before the explicit locked request-ID lookup, so a replacement ledger cannot
return an equal request's saved response in place of the original ledger.

`format.cpp:1479` compares exact saved request bytes and handles replay before
head/CAS or prepared-candidate access. For the identity-bound call it moves the
saved result into the receipt, publishes `replayed`, then constructs the separate
return copy. That copy may allocate without losing the authoritative receipt.
Unknown-request CAS and original typed conflict details retain their prior order.
Receipt reuse refuses without erasing existing bytes or phase.

`format.cpp:1608` retains final native filesystem identity, complete pending-history
verification and `candidate.verify()` under the write lock. No new Lua, provider
callback, projection, comparator or diagnostic encoding runs there. The existing
native SQLite test hooks remain verification-only. Schema, result codecs and
public catalog are unchanged; the shared completion leaf is reused through a
downward CMake dependency, including exact-source format variants.

`format.cpp:1631` copies fresh accepted/noop response bytes into receipt storage
before COMMIT. It then publishes `commit_uncertain` immediately before commit,
and `committed` only after confirmed success. Phase assignments and saved-string
move/ownership operations do not allocate. The local evidence guard publishes
`not_completed` when the SQLite primitive has confirmed noncommit, including if
subsequent error construction/conversion throws. The receipt outlives local
transaction and return owners; no postoperation cleanup clears its evidence.

`sqlite.cpp:294` adds a narrow observation overload while retaining `commit()`.
It rejects inactive/stale/poisoned owners before issuing COMMIT and preserves
generation checks. A failed COMMIT captures its original extended code and
SQLite-owned diagnostic before rollback. It observes whether SQLite still owns
an active transaction, attempts rollback, and publishes confirmed noncommit only
when that active transaction was successfully rolled back. This publication
precedes construction of the C++ error string. Automatic autocommit after failed
COMMIT remains uncertain. The original COMMIT diagnostic survives rollback;
SQLite-owned diagnostic storage is freed during unwinding. Failed rollback still
poisons the connection, and existing RAII abandonment/generation behavior remains.

## Independent verification and raw evidence

All build/test operations were direct and serialized in the exclusively assigned
debug build directory. `commands.json` records exact argv, UTC times and exits.

- `cmake --build build/rewrite-debug --target all planar_tests` passed and
  reported `ninja: no work to do.`.
- `ctest --test-dir build/rewrite-debug -R 'replacement\.(atomic |ledger sqlite )'
  --output-on-failure --output-junit <absolute review-evidence/focused.xml>`
  passed **39/39**, zero failures and zero skips, in 7.96 seconds. JUnit and
  `focused-LastTest.log` were archived before inventory.
- Subsequent `ctest --test-dir build/rewrite-debug --show-only=json-v1` returned
  **396** tests, exactly the coder's inventory. All 391 baseline names remain.

This selection exercised identity replacement containing an equal request,
empty/wrong identity refusal, replay with advanced head and moved/unavailable
candidate/deleted captures, fresh accepted/noop chronology, capture failure,
request/CAS precedence and typed details, transaction-prefix faults, commit-hook
refusal, concurrency with equal/different request bodies and competing heads,
and exact-source collision/allocator/final-capture variants.

The real reader-induced busy COMMIT retains proposed exact saved bytes, confirms
rollback, reports `not_completed` and permits retry. Deferred foreign-key failure
preserves `SQLITE_CONSTRAINT_FOREIGNKEY` and its original message across confirmed
rollback. Commit-hook automatic rollback conservatively retains `commit_uncertain`
and saved bytes. Independently parsed all nine enumerated VFS write/sync COMMIT
failures from LastTest: each returned failure with `confirmed_noncommit=false`;
four unhit sentinel commits succeeded. Existing old-state/reopen/retry checks
passed. The observations and zero-skip JUnit check are in `verification.json`.

Reviewed the frozen coder's final 188/188 zero-skip JUnit/LastTest and six CLI
suites, pinned formatting/Doxygen result for 93 active C++ files, unchanged
12/20/41 vectors, historical seven-family plus fixed-harness exact-source audits,
six D6 variants, C01 variant and no-work result. The initial generated-namespace
compile failure is retained and was followed by successful final builds/tests;
the final evidence corresponds to the frozen source. This independent review
did not repeat the full milestone gate.

## Limits and handoff

Verification is macOS ARM64 debug. Exception/allocation retention was reviewed
from ownership, no-throw publication and guard ordering; no deterministic C++
allocator-fault injection was performed or claimed. Release/Linux, hardware
power loss, public load/provider/transport integration and complete R14 acceptance
remain outside D7-A and retain their later gates. No stronger full-tree
preservation proof is claimed than the historical basis described above.

Debug is released idle after the archived focused run and inventory. Release was
untouched. Approval applies only while the exact reviewed candidate remains
current; D7-B can proceed after the coordinator rechecks it. Review artifacts use
an explicit evidence-relative manifest; `handoff-identities.json` uses explicit
repository-relative paths for the report and review manifest.
