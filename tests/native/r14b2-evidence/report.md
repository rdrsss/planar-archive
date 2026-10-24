# R14-B2 implementation handoff

Candidate awaits independent review; this report does not approve it.

The private shared verifier now accepts fully validated history. It streams one
ordered event cursor and retains one head/parent/reference tuple plus counters.
Each accepted revision is checked once; source (64 MiB/4096 files/8192 dirs),
registry and stdlib (independently 16 MiB/4096 files), and canonical values
(individually 16 MiB) retain the reviewed fixed bounds. Each reconstructed tree
is released before the next tree. Stdlib verification itself retains its bounded
input plus verified output; interpretation decoding retains bounded values and
fallback metadata only. Nothing scales native retained state with total history.

The pass pins exact init, gap-free event sequence, before/after continuity,
initial revision 1 then increasing revisions with immediate prior parent,
accepted/no-op reason/type/reference agreement, revision actor/time, saved result,
request body operation/actor/expected head, one request per operation, one accepted
event per revision and final projection. Counts plus per-event reverse references
reject orphans and repeated acceptance; timestamps validate syntax but can regress.

Exact no-op comparison also handles distinct equivalent record IDs. It compares
five referenced records' complete encodings, never hashes alone. Equal directory
encoding fixes exact child identities; the complete-record prepass checks every
digest bucket member, rejecting equal portable directory digest with unequal
child-ID encoding as ambiguous. Thus duplicate root rows cannot bypass no-op
classification. No-op event references remain the existing revision's exact IDs.
This verification does not regenerate normalized values from source through Lua
or rederive the semantic capture closure with current policy. Exact closure
capture is the acceptance writer/preparation responsibility; this pass verifies
complete bounded stored topology and all fixed recorded relationships. The valid
change fixture appends a comment to captured workspace.lua while keeping the
normalized interpretation unchanged.

Referenced content is reconstructed using `tree_core`, with pure stdlib manifest
verification and interpretation decoding. Registry has exactly `input.json`.
Historical workspace and recorded stdlib digest must match. Unsupported policy
versions are admitted only after the interpretation decoder's canonical/shape/
separate-version equality checks; bounded canonical metadata extraction then
checks workspace/digest and all other deep content remains mandatory. Unknown
record/payload codecs refuse. Unknown event row versions are checked after exact
schema verification but before SQLite integrity_check, so a future version's
CHECK violation does not mask `unsupported_state`; event/result payload versions
use the same category. Outer format handling and exact schema are unchanged.
Malformed persisted values are corrupt. Prepare/bind/step failures in the shared
verification path retain SQLite busy/full/corrupt/I/O categories rather than
collapsing actual storage errors into corruption. Required lengths precede copies.

Every trust transition already calls this verifier: RO/RW open, write acquisition,
record precommit, backup/restore publication, and scratch/original quiescent
recovery. No public acceptance writer, CLI or current Lua execution is added.
Production dependencies include only pure tree/provenance/interpretation codecs;
the fixture alone links the higher embedded-provenance wrapper. Generated fault
and digest variants import the same pure dependencies. The forced-digest fixture
continues to exercise record-only state, without falsifying provenance hashes.

Verification completed directly and serially in the macOS debug directory:

- `cmake --build --preset debug --target all planar_tests`: passed.
- `ctest --test-dir build/rewrite-debug -R 'replacement.(ledger|canonical|hash|record|tree|provenance|interpretation)' --output-on-failure --output-junit <absolute focused-final.xml>`: 143 passed, zero skips; all 133 prior cases retained, with the one documented orphan-test rename/category update; ten new cases include multiple corrupt variants.
- After the final fixture comment adjustment, `ctest --test-dir build/rewrite-debug -R '^replacement.ledger (accepted history|complete accepted)' --output-on-failure --output-junit <absolute history-final.xml>` reran all ten changed history cases; zero skips. The 133 prior cases and the unchanged final production verifier had already passed the complete 143-case selection above.
- `ctest --test-dir build/rewrite-debug -R '^replacement.cli$' -V --output-junit <absolute cli.xml>`: one suite, 247 real CLI processes passed, zero skips.
- `python3 tests/vectors/generate.py --check`: 12 hand byte vectors, 20 upstream hashes, 41 canonical/domain vectors passed.
- `python3 tests/vectors/records/generate.py --check`: ten independent record vectors passed.
- `python3 scripts/rewrite-check.py lint --build-dir build/rewrite-debug`: pinned formatting and Doxygen passed for 64 active files; supplementary pinned formatting passed for four unchanged verification helpers.
- Reversible generators, exact configured outputs, schema/init identity, comment-only public API, protected files, proper product link and pure format dependency declarations: audited.
- Final `cmake --build --preset debug --target all planar_tests`: `ninja: no work to do.`

The new positive workflow fails the exact pre-B2 format's intentional history
refusal (one expected failure after 71 passing assertions), then passes the
candidate. The retained old orphan-history negative now returns corrupt rather
than blanket unsupported_state; no other prior negative assertion was removed.
The prior sticky-reader, rollback, storage-full/busy, exact POSIX fault and
collision cases all remain within the passing focused set. New history recovery
uses a real killed writer and a real hot DELETE journal. There is no new
phase-specific VFS injection for every deep validation query; existing typed
reader and storage fault tests plus the shared error propagation bound that claim.
Release/Linux verification remains for the integrated gate. No installed binary,
operator database, commits, installation, publication or merge was touched.
