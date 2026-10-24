# R12-C1-C01 independent review

Disposition: **approve** for the complete corrected C1 candidate identified
below, including backup/restore and explicit initialized-ledger recovery.
**R12-C1-F01 is resolved.** No further findings were identified. This approval
permits advancement to C2; it does not complete R12 or approve the unexecuted C2
fault and integrated gates.

## Footer correction

`src/engine/ledger/format.cpp:779` reads exactly eight bytes at EOF into a fixed
array. The preceding regular-file and `st_size > 512` validation makes the offset
nonnegative; no pathname length controls allocation, copying or file access.
A short/failed read, including an interrupted read, refuses with the typed `io`
error. Exact journal magic returns typed `corrupt` with a diagnostic containing
no captured pathname. The guard does not parse or open any encoded external path.
Malformed trailers and coincidental matching footers are intentionally refused,
as documented in `docs/architecture.md:181` and `docs/testing.md:187`.

The ordering is correct: `recover_quiescent` calls `hot_header` at
`format.cpp:823`, returns its failure immediately, and only afterward creates
scratch at line 832. The first writable SQLite recovery is at line 872; original
recovery follows at line 892. Thus footer refusal precedes both recoveries and
scratch creation, preserving the original pair and unrelated entries.

Pinned SQLite `vendor/sqlite/b9c7/sqlite3.c:60938` (`readSuperJournal`) requires
exact `aJournalMagic` at `szJ-8` before returning a super-journal name. Length and
checksum checks further narrow recognition; they cannot admit a reference
without that footer. Its playback caller at line 62526 uses that result before
external existence checks. The later `pager_delsuper` path at line 62214 is
reachable only with that recognized name and can also inspect other participant
journals. Refusing the outer journal's necessary EOF marker therefore closes
that entire unsupported authority path under the documented whole-operation
quiescence precondition. There is no new journal parser or vendor change.

The existing regression at `src/engine/ledger/format.t.cpp:603` exercises the
frozen real external-file case, absent/symlink/directory references, malformed
length/checksum/NUL-name fields and a magic-only suffix. Every variant checks
typed refusal and the recursive snapshot of main/journal/external entries.
The preserved pre-fix `red-focused.log` records **one failing case, 18 failed
assertions out of 153**, including successful recovery and deletion in the
external variant. The independent corrected suite passes the same regression.
No additional reproduction tooling was created or run during this review.

## Full C1 assessment

Reviewed the original nine-file C1 baseline, frozen C1 candidate, complete
corrected C1 diff, correction delta, current interfaces/implementations/tests,
prior finding/source analysis and raw evidence, against the active instructions,
implementation/roadmap, relevant v4 provisions and normative ledger contract.
The review covers the full C1 candidate rather than relying on the prior
request-changes review as approval of its remaining scope.

`connection::online_backup_to` owns SQLite backup init/step/finish, checks owner
validity and an idle distinct destination, preserves step/finish failure codes,
and always finishes an initialized handle on its ordinary return paths. It
accepts the retained source read transaction and adds no retry loop.
`backup_new`/`restore_new` validate the source before target creation, use SQLite
online backup into owned staging, close and sync it, then verify full supported
state and preserved metadata before no-replace publication. Early/late sidecar
refusal and ownership checks remain. Published targets survive sync/cleanup
uncertainty; no existing destination is overwritten.

Explicit recovery retains no-follow regular main/journal descriptors, refuses
WAL/SHM, checks recognized headers/dimensions and the new footer boundary, then
captures retained input bytes separately from the working scratch pair. SQLite
recovers scratch; exact schema/integrity/foreign-key/initialized-state checks,
original-size check and journal consumption precede original writable recovery.
Original bytes and identities are rechecked, and verification/metadata checks
repeat after original recovery. The ordinary opener, SQL schema, and complete
schema/state verifier (including byte guards) are independently confirmed
byte-identical to the original C1 baseline. Only initialized state is supported;
caller-enforced quiescence and cooperative directory ownership remain explicit
preconditions, with no external-writer coordination or journal-authentication
claim.

Passing existing cases at `format.t.cpp:493`, `512`, `534`, `567`, `588` and `662`
cover backup/restore success, unchanged destination/source refusals, real killed
dirty-page recovery, redundant genuine-journal recovery and malformed/deceptive
pair refusal. The redundant case is accurately labeled as reconstructing the
prior main bytes beside a genuine journal. The earlier corruption-fixture
correction remains sound: journal-covered corruption is repaired by rollback;
the corrected unjournaled event-page corruption must and does refuse unchanged.

## Independent verification and identity

Fresh reviewer `/root/r12c1c01_reviewer` used the shared
`dev/planar-v2-vernacular` checkout at HEAD
`3171c49e7b92d3abda01bfbb8bfb0c6b29d4c67a`. Product files were read-only; writes
were limited to this report and new `tests/native/r12c1c01-review-evidence/`.
Other edits and frozen evidence were preserved. No Planar operational tools,
legacy skills/queue, installed binaries/data, commits, installation, publication
or merge were used. Debug was held exclusively for direct serialized checks.

Executed commands (stdout/stderr retained in reviewer evidence):

```sh
python3 tests/native/r12c1c01-review-evidence/audit.py identity-before.json
cmake --build build/rewrite-debug --target all planar_tests
ctest --test-dir build/rewrite-debug -R 'replacement.ledger' --output-on-failure --output-junit <repo>/tests/native/r12c1c01-review-evidence/focused.xml
python3 scripts/rewrite-check.py lint --build-dir build/rewrite-debug
cmake --build build/rewrite-debug --target all planar_tests
python3 tests/native/r12c1c01-review-evidence/audit.py identity-after.json
```

Both builds exit zero with `ninja: no work to do.` CTest exits zero, selects
**36**, passes **36**, fails **0**, skips **0**, independently counted from JUnit
in `focused-counts.json`. Formatting/Doxygen exits zero and checks **35** active
C++ sources. Existing RED and coder GREEN/lint/no-work logs were also inspected.

Full corrected nine-file candidate manifest SHA256:
`2a90623073df705d33b4c4db0cd37cd933f31e8bae48e9456e0cec5bba81202f`.
Correction implementation-evidence manifest SHA256:
`cc0a1878354709f85dad48a5a4c45b6cb972d7f2051d8267f5e9e0670ee8094a`.
Both use repository-relative paths. Before/after audits verify all **9** current
files and candidate snapshots, **35** evidence entries, all original and
correction-baseline snapshots, and **54** protected historical evidence files.
Across the **3,345** original baseline entries, exactly the eight permitted C1
files differ, with no missing files or unexpected baseline changes. Exactly four
files differ from the correction baseline: recovery implementation/test and the
two affected references. The CMake candidate entry remains unchanged. Reviewer
`manifest.json` freezes this report and reviewer artifacts except itself.

Executed platform: macOS Apple Silicon debug. Broad syscall/VFS and backup-finish
faults, actual SQLite full storage, publication uncertainty, competing processes,
comprehensive commit-boundary termination, full debug/release and renderer gates
remain explicit C2 obligations. Linux execution is unclaimed. These limits do
not reopen C1's now-verified refusal boundary. Debug directory released on
completion; no verification process remains active.
