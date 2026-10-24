# R14-B2 candidate evidence

The shared checkout is used directly. The coder owns the files in R14-B2 plus
coordinator-authorized comment-only changes to `format.cppm`. The protection
snapshot was taken before edits. Coordinator-owned round/status/roadmap records
are excluded because they are updated concurrently as work records. Build trees
and Git internals are excluded. New `.DS_Store` filesystem view metadata is
excluded from the unexpected-file inventory. Historical non-R14 round records
are additionally checked against the unchanged B1 protection snapshot. `format.cppm` was initially protected, then its
comment-only scope was explicitly approved; its exact original is retained in
`baseline/` and must be excluded from final protected-file comparisons.

`baseline/src/engine/ledger/format.cpp` is copied unchanged from the reviewed
R13-B2 candidate. Its SHA256 equals B1's pre-B2 protection snapshot:
`2fac3706afa400a1fd01ec15b1920ef15965c513338c961e2aea74a70e854d98`.
The baseline build runs the new positive-history workflow and demonstrates the
old intentional nonempty-history refusal. The candidate source is restored
before verification. This is a feature acceptance regression, not a claim that
the old intentional refusal was a bug.

Fixture preparation uses the reviewed R13-C2 interpretation canonical goldens,
real quota workspace files and registry bytes, and the actual embedded trusted
stdlib through a test-only higher-wrapper link. It updates the envelope and
separate versions with that exact stdlib tree digest, preserving all other golden
fields. The fixture source change appends a Lua comment to the existing captured
`workspace.lua`, changing exact source bytes while preserving normalized meaning.
Its independent relationships are init event 1; acceptance revision 1/event 2;
change revision 3/parent 1/event 3; no-op revision 3/parent 1/event 4; revert
revision 8/parent 3/event 5 reusing the original roots. Request bodies retain
before heads none,1,3,3. Saved result parents are none,1,1,3. Backwards timestamps
and gapped revision IDs deliberately exercise contextual clock and chronology
rules. Publication SQL exists only inside the test fixture; there is no product
acceptance writer. Invalid variants use fresh scratch roots and restore exact
schema triggers after mutations. Read/RW/acquire/backup refusals check byte and
directory preservation.

The old negative revision-history case remains and now means an orphan revision
with no accepted event, which is corrupt after replacing blanket refusal. No
other old negative assertions are removed. Existing exact-source POSIX fault and
forced-digest variants remain reversible, with pure-validator dependency closures;
the digest variant's fixtures remain record-only.

Evidence logs with numbered or `first` names preserve iteration failures,
including fixture setup errors and missing test includes. Only final named gate
logs below describe the frozen candidate. Debug/macOS is the verification scope;
no release/Linux durability claim, installation, commits or publication is made.
