# R12-A-C01 independent review

Disposition: **approve**. No remaining concrete finding in the full corrected
R12-A candidate or this correction.

Reviewed the frozen fourteen-file candidate on shared branch
`dev/planar-v2-vernacular`, HEAD `3171c49e7b92d3abda01bfbb8bfb0c6b29d4c67a`.
Product, reference and vendor files remained read-only. Reviewer ownership was
limited to this report and `tests/native/r12ac01-review-evidence/`. No fixes,
Planar operational tools, installed binaries/data, commits, installation,
publication or merge were used. The debug build directory was exclusively held
for direct serialized verification.

## Correction assessment

The prior P2 finding is resolved. `connection::open` validates original empty/NUL
input before using the error-code overload of `filesystem::absolute`. The
resulting absolute pathname cannot equal SQLite's bare `:memory:` sentinel or
begin with its URI prefix. No URI flag, canonicalization, lexical component
collapse or link resolution was added. Literal colon-prefixed names therefore
remain persistent native files under all supported spellings, and existing-file
modes still refuse absent files.

The additional native-parent check addresses SQLite's own dot-component
collapse: the OS must traverse the original unnormalized absolute parent to an
existing directory before opening. `missing/../target` and
`ordinary-file/../target` cannot silently become `target`; SQLite NOFOLLOW also
continues to reject symlink ancestors, including a symlink followed by `..`.
Valid `directory/../target` remains supported. Removed-cwd conversion returns a
typed SQLITE_CANTOPEN, while explicit absolute paths remain usable in that cwd.
The documentation correctly limits this primitive to trusted SQL and explicitly
excludes hostile filesystem races, exclusive publication and ledger validity.

Independently inspected the preserved red log: four cases matched, three failed
and one passed. The failures include both existing-mode sentinel bypasses,
missing persistence and the original `missing/../db` assertion. Comparing the
initial regression source to the final source confirms that every original
assertion remains; final changes only add missing-parent and valid-dot-traversal
coverage. Intermediate passing evidence is accurately identified as superseded.

## Full R12-A assessment

Reviewed the complete current interface/implementation, tests, dependency/build
wiring and affected references. Connection state is retained by statements and
transaction guards; statements finalize before retained state is released.
Move operations leave unavailable sources, and transaction move assignment
abandons the previous guard. Explicit-sized SQLITE_TRANSIENT binding owns bytes;
result access copies values, distinguishes NULL and empty text/BLOB, and refuses
wrong types or invalid row/index access. Extended error codes survive typed
failures. Prepare/bind/step failures abandon active transactions; failed commits
return the original failure and roll back; failed rollback poisons reuse.
Authorizer checks refuse manual transaction/savepoint control, and transaction
generations prevent stale guards from rolling back newer transactions.

Read the prior independent probe source, command and raw output for pending
INSERT RETURNING failed-commit rollback and destruction of a stale guard during
a newer transaction. Those implementation paths are unchanged by C01, and the
current focused suite independently covers deferred-constraint failed commit,
guard moves/lifetimes, typed rollback and retry. No additional transaction or
ownership defect was identified. The wrapper's single-caller-thread precondition
is explicit; serialized SQLite internals do not claim wrapper thread safety.

All seven restored vendor/cache files independently match Git HEAD byte for
byte. Offline CMake compiles only the pinned amalgamation with THREADSAFE=1,
DQS=0 and DEFAULT_FOREIGN_KEYS=1; no old opener, migration, environment discovery,
Lua or command integration returns. The module is an internal primitive and is
not a safe format-aware ledger API.

## Independent verification and integrity

Raw output and reproducible probe command/source are under
`tests/native/r12ac01-review-evidence/`.

```sh
ctest --test-dir build/rewrite-debug -R 'replacement.ledger' --output-on-failure --output-junit <repo>/tests/native/r12ac01-review-evidence/focused.xml
sh tests/native/r12ac01-review-evidence/probe-command.sh
cmake --build build/rewrite-debug --target all planar_tests
```

Focused CTest: **13 matched, 13 passed, zero skipped**. The independent compiled
probe exited zero. It tested four literal special/URI-looking filenames across
bare, relative and absolute reopens; absent existing-mode refusal; missing,
non-directory and symlink parent traversals in all three modes without target
creation; valid directory/../ traversal; and removed-cwd relative/absolute paths.
The final build reported `ninja: no work to do.` Independently inspected the
final raw formatting/Doxygen evidence: all 32 active source files passed.

Candidate manifest SHA256:
`0ed8a453a76dac87d59c06f91a839d9e1643a798ef92b20373a9fb82d866ec58`.
Implementation-evidence manifest SHA256:
`03d889fb946aa39f917c512929093d8f3b1ba8369374d3aa35c5caaedbb1504c`.
All fourteen candidate entries and twenty-three implementation-evidence entries
matched before and after verification. Independent protected-baseline audit
rehashes all 3,248 files: 3,246 unchanged, with only the two authorized reference
changes (`docs/architecture.md`, `docs/testing.md`). Original R12-A implementation
and review evidence remains unchanged.

Limits: macOS Apple Silicon/debug verification only. Full debug/release and
Linux gates, rollback-failure injection, backup/restore, disk exhaustion and
termination/restart remain later R12 work. This approval closes the corrected
R12-A primitive slice only; R12-B format/schema/publication and R12-C storage
fault/milestone gates remain required.
