# R14-E1 independent review

Disposition: **approve**. No blocking findings in the frozen init CLI scope.

Reviewed in the shared checkout on `dev/planar-v2-vernacular`, starting revision
`3171c49e7b92d3abda01bfbb8bfb0c6b29d4c67a`. Review changed only this report and
`tests/native/r14e1-review-evidence/`; no product fix, commit, installed binary,
operator state, Planar tooling, installation or publication was used. Other work
was preserved. The debug directory is released; no review process remains running.

## Exact candidate and evidence

Candidate identity:
`d9fd285dcc331c80a752f93aa3cb749bf95e3307d2159b0c30cf48e6bd9c75d9`.
The independently verified recipe is SHA256 of the compact sorted JSON map of
twelve owned path-to-content hashes, without LF. The actual
`tests/native/r14e1-evidence/candidate-files.json` schema has
`identity_recipe`, `candidate_identity` and `files`; its byte SHA256 is
`01baa127d64345eb29efb01f82f40babe045fadad70330fa8a259c0573f639e0`.
Every frozen file equals its current checkout counterpart and declared digest.

All 45 files listed by the coder evidence manifest match their hashes, and the
manifest covers the exact evidence directory membership apart from itself.
Its byte SHA256 is
`aec6110ce902dfce1c95b9208506a34d125aa46300c7c3e2eb43353892a1914a`.
The report's full-suite and final assertion-correction chronology is accurately
qualified: the full 312-case run preceded a test-only cleanup-prefix correction;
the final init and focused command runs exercise the frozen corrected script.

Independently compared all 4,836 protected file hashes against `before.json`:
native storage/types/schema, capture, Lua, vendor, prior helpers and prior evidence
are unchanged. Only permitted additions exist in audited source/test/tooling roots;
the `AGENTS.md` symlink is intact. All three configured generated variants
(`r12c2`, `r13b1`, `r14c3b2`) equal fresh generation from current product bytes and
retain exact reversal; r13b1 has no verification mutation. Root CMake reconstructs
its exact recorded baseline hash by removing only the three-line additive init
test registration.

`baseline-recovery.json` accurately distinguishes three saved original file copies
from six hash-only original owned files. This review additionally reconstructed
the original root CMake bytes exactly. A complete original round diff cannot be
recreated from the remaining five hash-only files alone; approval identifies the
exact final snapshot and uses direct full-file inspection, protected baselines and
behavioral verification. It does not claim those missing original copies exist.

## Contract and implementation assessment

- [Init bindings and dispatch](../../../src/cmd/planar/handlers/init/command.cpp)
  use CLI11 required scalar options, explicit nonempty values and duplicate
  refusal. [Bootstrap](../../../src/cmd/planar/bootstrap.cpp) keeps both option
  owners alive through parsing/dispatch, counts selected commands and all
  library-parsed option bindings, and dispatches exactly one handler. The existing
  long-equals guard remains the sole token preflight. Root-only fixed help and
  render syntax/semantics are preserved; unsupported load/history commands refuse.
- `init_command` line 29 passes the unchanged explicit target directly to
  `initialize_new`. There is no existing-file resolver, path canonicalization,
  default path, HOME lookup, SQL, scaffold or overwrite path in the handler.
  Lines 32–40 Glaze-encode the exact six-field envelope from returned metadata,
  with `active_revision:"none"`, `event:"1"` and one LF, only after native
  commit/publication returns success.
- [Shared response mapping](../../../src/cmd/planar/response.cpp) lines 36–112
  depends on `ledger.types`, ignores native message bytes, and matches all 17
  categories in R14-E1's authoritative table. Foreign/unsupported, invalid
  arguments and unsafe paths remain exit 3; corrupt/busy/full and I/O are exit 4.
  I/O details identify `ledger`. Publication ambiguity explicitly says to inspect
  the explicit target before retrying; recovery-required requires explicit
  recovery. The mapper performs no cleanup/retry. The status extension at lines
  114–124 adds only the three new storage categories to existing render policy.
- Dependency wiring keeps the response helper below both handlers; command code
  introduces no native format/storage policy changes or handler/bootstrap cycle.
  CLI, architecture and testing references describe the bounded behavior and its
  publication/output caveats. No load membership or future history/export behavior
  is implemented or presumed.

## Independent verification

Commands ran serially against the debug directory. Raw command output, JUnit,
probe observations, scripts and hashes are under
`tests/native/r14e1-review-evidence/`.

1. `python3 tests/native/r14e1-review-evidence/audit.py`: passed candidate/evidence
   hashing, protected files, root build delta, exact generation/reversal, inventory
   and frozen full-run audit. Configured inventory is exactly 312 unique cases;
   all prior 309 are retained and only the two new bootstrap cases plus init CLI
   are added. Frozen `full-LastTest.log` independently contains 312 passed records,
   zero failed/skipped records.
2. `cmake --build build/rewrite-debug --target all planar_tests`: passed with
   `ninja: no work to do.` No source build overlapped another command.
3. `ctest --test-dir build/rewrite-debug -R 'replacement\.(cli$|init_cli|help is|missing explicit|legacy commands|render handler|CLI11 rejects|init handler|ledger error)' --output-on-failure --output-junit <repo>/tests/native/r14e1-review-evidence/command.xml`:
   **9 matched, 9 passed, zero skips**. Includes all seven bootstrap cases, actual
   init CLI **60 processes**, and unchanged render CLI **247 processes**. The typed
   adapter case exercises every mapping and uncertainty/recovery guidance.
4. `python3 tests/native/r14e1-review-evidence/probes.py`: **55 additional actual
   binary processes, all passed, zero skips**. Independent exact-byte root help,
   success and complete error envelopes; required/duplicate/unknown/equals/empty/
   option-shaped values; positionals/terminators; multiple commands and unsupported
   commands; invalid workspace/path shapes; missing/non-directory/symlink parents,
   including aliases followed by `..`; existing regular/empty/directory/linked/
   dangling targets; relative targets and valid one/64-byte workspace IDs. A real
   unwritable parent produces exit 4 with empty stdout and exactly the redacted
   `io` envelope with operation `ledger`. Successful output matches read-only
   ledger metadata, empty record/revision/request/tree tables, initial current
   state and exact typed event. Repeated init preserves bytes; refused files,
   directory membership and links remain intact; no staging residue or changes to
   scratch HOME/PLANAR_DB sentinels occur.

Reviewed the coder's hash-verified raw full gate (312/zero skips), 76 active-file
format/Doxygen checks, seven helper checks, 12 hand/20 upstream/41 domain vectors,
ten record vectors and no-work output. The focused independent checks suffice
for this handler integration; the whole 312-case gate was not rerun.

## Limits carried forward

Coverage is macOS Apple Silicon debug. Linux and integrated R14 release remain
outstanding. Native publication fault tests are preserved and passed in the
recorded full run; this review reuses that evidence, and does not claim CLI fault
injection, power-loss testing or a new process-crash experiment. Publication-
uncertain/recovery messages are checked through the typed adapter, while actual
CLI I/O failure is checked through an OS permission refusal. Allocation and output
write limits retain the documented existing process contract. Future load
conflict/collision structured details and pending membership choices are outside
init's approval.
