# R14-C3-B2 independent review

Disposition: **approve**. No blocking findings or unresolved product questions.

Reviewed in the shared checkout on `dev/planar-v2-vernacular`, starting Git revision
`3171c49e7b92d3abda01bfbb8bfb0c6b29d4c67a`. The reviewer changed only this report
and `tests/native/r14c3b2-review-evidence/`; no product, prior evidence, Planar tool,
operator data, commit, installation or publication was touched. Debug ownership
is released after this report and evidence manifest are frozen.

## Exact candidate and evidence

- Candidate identity: `320c1ceeceab03d8944e33f829954c869e62a34403d5c46693644432c73214cd`.
- Candidate JSON file SHA256: `b6d7638678af06f65c8fd366d8a479dc575058e704ff1779cd6753c04e81dcb4`.
- Raw report identity: `ce4cea9f43196062bacdd09252e4bd1461a6ba8223f4187148340b342904e1a7`.
- Raw report JSON file SHA256: `19c96d3b5b49beeac707aeb4fad59e7eb0dfe213c597ba9baee1df8d99357778`.

The independent audit recomputed both canonical manifest identities, verified all
10 current/frozen candidate members and all 30 raw evidence members, and verified
4,701 protected hashes plus the `AGENTS.md -> CLAUDE.md` symlink. Exact pre-round
format/types baselines were found by their recorded hashes and diffed; CMake and
reference changes were compared with the frozen B1 candidate. All three generated
variants independently matched configured files and reversed to exact production
source bytes. The final audit repeats these checks against the current candidate.

## Contract review

`format.cpp:1436` provides one synchronous completion path with opaque private
storage ownership. It uses `begin_records` and its validated writable opener,
`BEGIN IMMEDIATE`, identity/schema/history revalidation. Exact request body replay
or conflict occurs at `format.cpp:1450` before head CAS, prepared data or receipts.
Unknown requests enforce CAS at line 1469, then prepared ownership/workspace, then
interning. A moved-from candidate is harmless for successful replay.

The shared interner extraction retains bucket-wide exact encoded comparison,
child-reference checks, typed-failure writer invalidation, explicit abort, and the
prior equality-mutation substitution. Source/provenance use `tree_core`. No-op
compares complete stored encodings, including local child IDs; a revert creates a
new revision. Guarded signed maximum allocation covers all three ID classes.
No-op result parent remains the returned revision's parent. Event/revision actor,
time, references, request result and projection are verified together.

Owned response bytes exist before full pending-state verification. The final
sequence at `format.cpp:1588` is full verification, immutable metadata comparison,
direct `candidate.verify()`, SQLite commit, and response return. There is no SQL
between the native receipt check and commit, no optional callback/bypass, and no
response returned before successful commit. Local RAII releases the complete
transaction on every typed refusal and C++ exception. The production changes do
not add schema, SQLite primitive, prepared/capture, codec, Lua, loader, runtime or
CLI behavior. Dependency changes and an actual independent native-only link
exclude Lua, loader, runtime, embedded source and higher format-importing tree or
interpretation wrappers; production has no added failure hooks.

## Independent execution

All following commands succeeded, with nonzero selections and zero skips:

| Command or owned driver | Cases | Assertions |
| --- | ---: | ---: |
| `build/rewrite-debug/src/engine/planar_ledger_format_tests '[atomic_acceptance]'` | 9 | 670 |
| `build/rewrite-debug/src/engine/planar_atomic_format_test_tests` | 3 | 136 |
| `build/rewrite-debug/src/engine/planar_ledger_format_tests '[records],ledger writer explicit abort*'` | 12 | 670 |
| `build/rewrite-debug/src/engine/planar_record_format_test_tests` | 3 | 151 |
| `python3 tests/native/r14c3b2-review-evidence/build_verify.py` | 4 | 534 |

The final selected executions total 31 cases and 2,161 assertions. The owned driver
compiles the additional tests against the existing production module objects and
actually links the native dependency closure; exact commands are saved in
`commands.txt`. It does not alter CMake, generated product files or production
sources. Additional adversarial checks demonstrate:

- Allocation failure after each of six completed SQLite mutation stages. SQLite
  PROFILE callbacks only arm a later C++ allocation; they do not throw through C.
  Five stages propagate real `std::bad_alloc`; the records stage becomes an
  existing codec typed refusal. Every path restores exact database bytes, removes
  the journal, fully reopens the initial state and retries successfully.
- Four real races with the same request ID and different actors. Exactly one
  succeeds; the other returns `request_conflict`, and only one event/revision is
  published. Each contender owns separate native preparation and uses finite
  busy/live-journal retries.
- Registry, stdlib, recorded interpretation and normalized source-mode changes
  each produce a revision; a subsequent complete no-op preserves revision/parent.
- New workspace membership, registry edits and source-mode changes invalidate
  retained receipts, roll back exact bytes, and allow fresh retry.

The first owned harness run had two verification-design failures: it incorrectly
required the record codec's allocation refusal to escape as an exception, and it
attempted an unsupported extra semantic-envelope field. That raw run is retained
as `adversarial-first.log`. The final harness accepts the existing typed refusal
while requiring rollback, and changes a valid recorded portable path for the
interpretation-only case. No product change was made for either harness correction.
The owned C++ test also passes the pinned formatter with `--dry-run --Werror`.

`python3 tests/native/r14c3b2-review-evidence/audit.py` independently checked all
301 raw full-gate Passed lines, absence of skipped/not-run results, current
registration of the 151 history, 25 capture and 12 prepared baseline names, CLI247
stdout, exact receipt-omission mutation and failing regression. Coder vectors
(12 hand bytes, 20 upstream hashes, 41 domain/canonical and 10 record vectors),
71 active and six helper formatting/Doxygen evidence were inspected. The full
301-case gate was not redundantly rerun. A final direct
`cmake --build build/rewrite-debug --target all planar_tests` reported
`ninja: no work to do.`

## Limits and handoff

Approval is for this exact bounded macOS Apple Silicon debug candidate. Event
maximum coverage uses the exact private allocator helper, not an invented valid
maximum event ledger; record and sparse later maximum-revision fixtures pass the
real opener. New acceptance faults cover real statement interruption, commit-hook
refusal and C++ exception rollback. They do not establish acceptance VFS faults,
process-crash, device-full or power-loss behavior. Integrated R14, release and
Linux gates remain outstanding, along with later service/CLI work and its required
pre-capture replay lookup. Debug is available for the coordinator's next round.
