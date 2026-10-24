# R12-B-C01 corrective candidate

Implementation handoff, awaiting independent review. No self-approval.
Shared checkout `dev/planar-v2-vernacular`, HEAD
`3171c49e7b92d3abda01bfbb8bfb0c6b29d4c67a`. Only the two format implementation/test
files and affected `docs/testing.md` changed from the full seven-file B candidate.
Existing rewrite work and all historical evidence remain intact. All databases
used explicit canonical temporary roots; no installed binary/operator DB, Planar
operational tool, queue, commit, installation or publication was used.

## Corrections and regression evidence

F01: `initialize_new` calls the existing retained-parent, no-follow `no_sidecars`
check before stage creation and immediately before atomic no-replace `linkat`.
Every extant `-journal`, `-wal`, `-shm` entry refuses conservatively. Owned cleanup
and post-publication uncertainty handling are unchanged. The new colocated
regression covers regular files, links, directories for all three suffixes;
compares the entire directory's contents/link targets/types and checks no target.
`red-tests.log` shows all nine original cases incorrectly publish (27 failed
assertions); final focused XML/log shows they pass. The separate malformed text
refusal case already passed before correction: refusal alone is not F02 evidence.
Multibyte counts in that test were refined after the first RED run to use each
field's character maximum, and formatting changed; no refusal-only RED is claimed.

F02: schema name/type/tbl_name/sql guards and metadata identity/workspace_id/
created_at guards now use pinned SQLite `octet_length` before wrapper text copies.
The normative DDL `CHECK(length(identity)=32)` and BLOB length guards are unchanged;
there is no schema revision. Other text copies are SQLite-generated PRAGMA results
(encoding/journal policy/integrity) rather than persisted metadata/schema payload;
event validation compares values in SQL and copies no event text. SQLite's pinned
implementation: vendor/sqlite/b9c7/sqlite3.c:134083 (byte length implementation),
137249 (BYTELEN registration), 118235 (direct-column byte-length optimization to
avoid unnecessary payload loading). Native byte guards execute before all seven
persisted TEXT copies.

`allocation-probe.cpp` is a standalone test-only executable replacing ordinary
C++ new/delete; it throws on a request >1 MiB only while opening. No product hook
or global was introduced. `probe.py` initializes and corrupts scratch fixtures
before arming the budget, restores the exact immutable trigger, records SQLite
character and byte lengths, then verifies refusal leaves the file hash unchanged.
For NUL tails in all three fields, `red-allocation.json` reports native requests
2,097,200 / 2,097,184 / 2,097,184 bytes and RED. `green-allocation.json` reports
all six NUL/multibyte cases `corrupt`, unchanged, largest request 768 bytes.
The multibyte cases have character lengths 32/64/24 and byte lengths 64/128/48.
Their refusal was already correct before this fix. This budget proves the native
copy regression; it does not cap SQLite C allocations, total memory, or RSS.
Original review 32 MiB/RSS proof remains under r12b-review-evidence unchanged.

`allocation-probe-red` retains the before-fix executable; `allocation-probe`
and its object retain the corrected version. `compile.log`, `link.log` and
`green-link.log` contain exact compiler/linker commands using the current module
map and static libraries from build/rewrite-debug. Recompile with compile.log's
first command and link with green-link.log's first command in that build directory,
then run `python3 tests/native/r12bc01-evidence/probe.py` from repository root.
The historical review probe initializes fixtures only and was not modified.

## Verification and identity

Direct serialized debug commands (raw logs alongside):

```
cmake --build build/rewrite-debug --target all planar_tests
ctest --test-dir build/rewrite-debug -R 'replacement.ledger' --output-on-failure --output-junit <repo>/tests/native/r12bc01-evidence/focused.xml
python3 scripts/rewrite-check.py lint --build-dir build/rewrite-debug
cmake --build build/rewrite-debug --target all planar_tests
```

Focused: 28 selected, 28 passed, zero failed/skipped (13 primitive + 15 format).
Pinned formatting and Doxygen: 35 active files passed. Final no-change build:
`ninja: no work to do.` Executed macOS Apple Silicon debug only. No broader
integrated/release gate was claimed. Publication-boundary sidecar recheck is
source-inspected; no deterministic filesystem interposition seam was added for
this narrow correction. Cooperative ownership remains required. Recovery,
backup/restore, VFS/disk-full/death injection and integrated gates remain R12-C.

`before-manifest.json` verified the original seven-file B candidate before edits;
`before-candidate/` preserves those exact bytes. `candidate/` and
`candidate-manifest.json` freeze the corrected full seven-file B candidate.
`correction.diff` compares the complete before/corrected snapshots.
`primitive-identity.json` records untouched approved primitive source/test hashes.
`historical-evidence-identity.json` records all untouched original/reviewer B
artifacts. `evidence-manifest.json` hashes every new artifact except itself.
