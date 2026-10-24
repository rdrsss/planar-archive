# R12-B implementation evidence

Native format/init slice only; not approval and not completion of R12. Shared
checkout `dev/planar-v2-vernacular`, starting HEAD
`3171c49e7b92d3abda01bfbb8bfb0c6b29d4c67a`. No commits, installed binaries,
operator data, legacy tools or CLI changes were used. Coordinator owns round
records; this directory records implementation evidence for fresh review.

Candidate ownership: `src/engine/ledger/format.cppm`, `format.cpp`, `format.t.cpp`,
`src/engine/CMakeLists.txt`, `docs/architecture.md`, `docs/testing.md`, and the
independent `expected-schema.sql` fixture here. The SQLite primitive, vendor
pins, renderer, Lua, CLI, old fixtures and historical evidence were preserved.
The source fixture is copied from the normative contract, including all thirteen
trigger definitions; tests let SQLite parse it in a separate scratch database
and compare full schema metadata/stored SQL against the initialized ledger.

The format module exposes metadata and opaque ownership only. Initialization
records fixed actor `planar:init`, random identity and timestamp in one committed
transaction. Read-only verification precedes writable opening; both owners retain
a consistent read snapshot. Initialization uses no-follow original-component
traversal, retained directory/file identity, exclusive 0700 sibling staging,
exclusive 0600 database precreation, SQLite commit/close, file/directory sync,
POSIX atomic no-replace `linkat`, parent sync, and owned-stage cleanup. Failures
after publication return uncertainty without deleting the published target.
Populated history is unsupported until R13/R14 validators exist. Any journal,
WAL or SHM sibling refuses before SQLite opens the file; audited recovery, backup
and detailed storage fault testing remain R12-C. Cooperative ownership is a
precondition; no hostile path-swap or power-loss immunity is claimed.

Executed directly and serially in `build/rewrite-debug` on macOS Apple Silicon:

```sh
cmake --preset debug
cmake --build build/rewrite-debug --target all planar_tests
ctest --test-dir build/rewrite-debug -R 'replacement.ledger' --output-on-failure --output-junit <repo>/tests/native/r12b-evidence/focused.xml
python3 scripts/rewrite-check.py lint --build-dir build/rewrite-debug
cmake --build build/rewrite-debug --target all planar_tests
ctest --test-dir build/rewrite-debug --show-only=json-v1
```

Final results: **26 selected tests, 26 passed, zero skipped**: thirteen primitive
and thirteen format cases. `focused.xml`, `focused.log` and
`focused-counts.json` record final results. Final registry inspection has 150
tests (149 native plus one Python CLI acceptance); this focused round did not
execute the whole registry. `lint-final.log` passes pinned formatting and Doxygen
for all 35 active C++ files. `no-work-final.log` says `ninja: no work to do.`
Full debug/release integration and Linux execution remain later gates.

Intermediate logs are preserved and superseded: first build missed macOS's
`sys/random.h` declaration for `getentropy`; `build-02.log` passed after adding
it. Initial 25-case suite passed. Adding the malformed-btree regression exposed
SQLITE_CORRUPT during session setup being labeled generic I/O;
`corrupt-classification-red.{log,xml}` preserves that failure (unchanged-byte and
unchanged-directory checks still passed). Format-owned error wrapping now maps
SQLITE_CORRUPT/NOTADB to corruption, without changing the primitive or weakening
the test. `build-correction.log` and `build-final-02.log` record the correction
and independent schema fixture build. The final 26-case run is green.
`lint.log` records the known upstream signal-only Doxygen retry, succeeding on
attempt two; `lint-final.log` passed without a retry.

The protection baseline hashes 44,749 existing files across `src`, `vendor`,
`fixtures`, `build` and prior `tests/native` evidence before implementation.
`protected-audit.json` reports 44,738 unchanged, no missing or unexpected changes;
the eleven changes are owned `src/engine/CMakeLists.txt` and ten generated debug
build files. All previous primitive source and native evidence bytes match.
Round records under `docs/vernacular` are coordinator-owned and were not edited
by this implementation. Two authorized product references were updated.

`candidate-manifest.json` hashes the seven final candidate files, with copies
under `candidate/`; `evidence-manifest.json` hashes all other frozen evidence
(excluding itself and candidate copies). Verify those identities before review.
No whole-tree Git diff is offered as this round's identity because the approved
rewrite is a large pre-existing uncommitted change.
