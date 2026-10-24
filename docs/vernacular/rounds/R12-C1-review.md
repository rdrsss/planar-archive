# R12-C1 independent review

Disposition: **request-changes**. Frozen finding **R12-C1-F01 (P1)** blocks this
candidate. No product fixes were made; a fresh corrective search/coder/reviewer
round is required before advancing this scope.

## Finding: rollback journals can authorize external file deletion

`src/engine/ledger/format.cpp:748` (`hot_header`) validates the first 28 journal
bytes and dimensions but does not reject SQLite super-journal references at the
end of the file. `recover_quiescent` copies the journal unchanged and hands it to
SQLite at line 867, then hands the original pair to SQLite at line 887. SQLite's
journal format can contain an external super-journal pathname. That pathname
escapes the captured main/journal pair and owned scratch boundary.

Independent reproduction against the frozen current candidate:

1. Initialize a temporary ledger using the current native format API.
2. In a separate process using the pinned SQLite, begin an uncommitted large
   insert, flush dirty pages with `sqlite3_db_cacheflush`, report readiness, and
   kill that process with SIGKILL. The genuine journal is 14,848 bytes.
3. Append the documented SQLite super-journal trailer, naming a separate file
   `external-mj000000900`. Give that external file the original ledger's journal
   pathname followed by NUL. All files are under a reviewer-owned temporary root;
   no concurrent writer or directory swap occurs during recovery.
4. Call `recover_quiescent` on the original ledger.

**Observed:** recovery returns success with the original metadata, the ledger
reopens successfully, and `external-mj000000900` has been deleted. Only the ledger
remains. Thus a journal outside the supported single-database recovery boundary
can make Planar delete another explicitly unselected file.

The relevant pinned SQLite code is `readSuperJournal` at
`vendor/sqlite/b9c7/sqlite3.c:60938` and `pager_delsuper` at line 62217. Its filename
shape and `bSeen` guards limit arbitrary deletion but do not prevent this case.
Scratch recovery reads the external super-journal and finds the still-existing
original journal, retaining the external file. Original recovery then recognizes
its own journal in that file, sets `bSeen`, and deletes the external file. The
caller provided neither that file nor a captured/validated set of participants.

**Required correction:** reject unsupported super-journal references before any
SQLite writable recovery of scratch or original. Preserve the original pair and
all unrelated files unchanged on that refusal. Add a regression using this
reproduction, including a valid trailer and external file; also cover unavailable
or unsafe external references as appropriate. Supporting multi-database recovery
would require a separate contract and is unnecessary for this initialized-only
slice. This finding concerns authority over external paths, not authentication of
journal checksums or a demand to prove arbitrary malicious-tamper resistance.
It belongs to C1's recognized-recovery/refusal boundary, not the broad C2 fault
matrix.

Raw reproduction: `tests/native/r12c1-review-evidence/superjournal-probe.py`,
`superjournal-run.log`, and `superjournal-result.json`. The Python harness exits
zero because it records the observation successfully; `external_deleted: true`
and recovery return code zero are the failing product outcome, not a passing
safety expectation. The probe was independently compiled and linked against the
current module map and static libraries. Commands are recorded by `compile.py`,
`compile.log`, and `link.log`. A reviewer-only initial compile missed `stdout`;
`compile-initial.log` preserves that failure, followed by a successful probe
compile using a flushed C++ stream.

## Review boundary and evidence

Fresh independent reviewer `/root/r12c1_reviewer` reviewed the shared
`dev/planar-v2-vernacular` checkout at HEAD
`3171c49e7b92d3abda01bfbb8bfb0c6b29d4c67a`. Ownership was limited to this report
and new `tests/native/r12c1-review-evidence/`. Existing edits were preserved. No
legacy Planar/queue/skills, installed binary/data, commits, installation,
publication or merge were used. The debug directory was exclusively held;
verification commands were direct and serialized. All probe databases used
explicit temporary paths and were removed afterward.

Read the active instructions, implementation/roadmap, relevant v4 and normative
ledger contracts, R12-C brief, A/B corrective reviews, complete candidate diff,
current implementations/interfaces/tests, snapshots, audits, and raw build,
functional, lint and no-work evidence. Reviewed backup init/step/finish lifetime,
new-target staging and no-replace publication, ownership checks, snapshot
verification and recovery's scratch/original transition. The initial corruption
fixture correction is legitimate: rollback repairs the covered page, while the
corrected unjournaled event page remains corrupt and must refuse unchanged.
The redundant genuine-journal fixture is accurately labeled as reconstructed.

Executed:

```sh
cmake --build build/rewrite-debug --target all planar_tests
ctest --test-dir build/rewrite-debug -R 'replacement.ledger' --output-on-failure --output-junit <repo>/tests/native/r12c1-review-evidence/focused.xml
python3 tests/native/r12c1-review-evidence/compile.py
python3 tests/native/r12c1-review-evidence/superjournal-probe.py
python3 tests/native/r12c1-review-evidence/audit.py identity-before.json
python3 tests/native/r12c1-review-evidence/audit.py identity-after.json
```

The build reported `ninja: no work to do.` Focused CTest selected **35**, passed
**35**, failed **0**, skipped **0**. The independent super-journal reproduction
then demonstrated the blocking unsafe outcome above. Frozen coder lint evidence
shows formatting/Doxygen passed for **35** active sources. No product edits
occurred during review.

Candidate manifest SHA256:
`56fdd9f733c5fff1c99e1b9cbb1dfb7d3391535f147b383262250ff46d9a4699`.
Implementation-evidence manifest SHA256:
`4d53bfbc2e8cfffd860b56d7f72da40c422435998c134f157f1b541824d68865`.
Both independent audits verify all **9** current/candidate entries, **33** evidence
entries and all baseline snapshots. Rehashing the **3,345** protected baseline
files finds exactly the eight permitted C1 edits, no missing files and no
unexpected changes. Independent section comparisons confirm the schema,
complete verifier and ordinary opener are unchanged. `manifest.json` freezes
this report and all reviewer artifacts except itself.

Executed platform: macOS Apple Silicon debug. Broad injected VFS/syscall faults,
full-storage, concurrency/commit-boundary termination, backup-finish errors and
full debug/release/renderer gates remain explicit C2 obligations, not missing C1
claims. No additional fault campaign or product modification follows this
request-changes review. Debug directory released on completion.
