# Independent R14-C1 review evidence

The reviewer used the shared checkout, preserved the frozen candidate and all
other work, and wrote only this directory and the independent review report.
The debug build directory was exclusively held during verification. All build,
CTest and reviewer-probe executions were serialized. No Planar CLI, legacy
binary, operator database, installation, publication or commit was used.

`identity.txt` records independent manifest, live/frozen candidate, raw evidence
and protected-file verification. `audit.txt` records the final unchanged hashes,
exact generated namespace variants, prior-case preservation and test counts.
`commands.json` records the final CLI/vector/lint/no-work commands. Additional
executed commands were:

```sh
cmake --build --preset debug --target all planar_tests
ctest --test-dir build/rewrite-debug -R 'replacement.(ledger|canonical|hash|record|tree|provenance|interpretation)' --output-on-failure --output-junit <repo>/tests/native/r14c1-review-evidence/focused.xml
python3 tests/native/r14c1-review-evidence/build_probes.py
tests/native/r14c1-review-evidence/probes '[review]' --reporter console --reporter JUnit::out=tests/native/r14c1-review-evidence/probes.xml
python3 tests/native/r14c1-review-evidence/checks.py
```

Build, focused and probe stdout/stderr were redirected to the corresponding
`build.txt`, `focused.txt` and `probes.txt`. Every final command exited zero.
`build_probes.py` records exact compiler/linker argv in `probe-commands.json`;
it reuses the frozen candidate's scratch fixture translation unit and existing
module maps/libraries without replacing source or build objects. Reviewer-only
object/executable outputs were removed after success; `probe-binaries.json`
records their hashes and sizes. Re-running the builder recreates them.

The three reviewer cases passed 731 assertions. They cover UINT64_MAX IDs and
cursors, SIZE_MAX limits, non-ASCII/whitespace request tokens, allowed punctuation,
poison propagation through move construction and assignment, moved-from read
refusals, metadata retained on live poisoned owners, exact owned body/result/
event bytes, and every cursor/limit edge of the five-event fixture. The fault
case forces one real SQLITE_IOERR_READ for each of head, revision, event,
request, page and existing record reads, checks typed I/O classification, then
checks every read refuses and metadata survives. A final fresh owner succeeds.
A test-only VFS PDB hint supplies the connection solely to
`sqlite3_db_release_memory` before arming xRead; this evicts unused cache pages
without ending the snapshot or altering database bytes. No product SQL handle
was exposed.

Two preliminary reviewer harness issues are retained under
`initial-harness-error-*` and `initial-cache-hit-*`, with explanatory Markdown.
The first called metadata on a moved-from owner contrary to its explicit API
precondition. The second attempted an I/O fault against a cached page and thus
did not trigger xRead. Both were corrected only in reviewer-owned probes before
the final successful run. Neither is a product finding or a skipped case.

Final evidence: 151 focused cases, every prior 143 case preserved, zero skips;
CLI247; canonical/hash and record vectors; formatting/Doxygen on 64 active C++
files; exact/reversible generated variants; 12,363 protected hashes unchanged;
and a no-change all-plus-tests build reporting zero Ninja work. The focused run
also independently reruns the 115-event pagination, exact SQL-byte comparison,
unsupported interpretation and DELETE snapshot busy/release/reopen cases.
Verification is macOS arm64 debug only. Release/Linux and the integrated R14
acceptance/history/export milestone remain outside this bounded approval.
