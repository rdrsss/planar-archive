# V5-02-X06-C independent review (native profile transactions)

Disposition: **request-changes** (not records-only).

The transaction work is sound. There is one shared body (`complete_acceptance_impl`). Replay
and conflict come before format, CAS and candidate inspection. Native receipts are checked after
the rows are staged and after `verify_before_commit`, as the last step before COMMIT. Receipt
phases are unchanged, and every embedded test passes unchanged. Every gate result I re-checked
matches the raw logs. All 1003 baseline names are kept and 25 are added. No existing diagnostic
message changed.

The round still cannot be approved, for two reasons. First, the private seam the adapter goes
through is exported to every importer of `planar.engine.ledger.acceptance`, including the CLI.
That seam takes caller-built trees, values and a caller-supplied receipt callback. This breaks
criterion 9 and the dispatch rule that nothing accepts caller-built data as authority (R1).
Second, criterion 1's "built-in and custom job mix" is not tested, because the fixture turns
every job custom (R2). Both need source or test changes. Three record items also need
correcting (R3 to R5).

I reviewed in the shared checkout on `dev/planar-v2-vernacular` at HEAD
`caaf0396ae90a7f7d2d511b9a9a6e185e4362d52`, against the uncommitted candidate. I wrote only this
file and new files under `tests/native/v5-02-x06-c-evidence/review/`. I applied no mutants and
edited no source, test, record or existing evidence. I ran two `cmake --build` calls on
`build/rewrite-debug` (`planar_tests`, then plain). Both reported `ninja: no work to do.` and
neither built anything. I also ran `ctest -N` and one focused ctest, one at a time, with no other
ninja or ctest process running. I did not commit, push, merge, install or publish, and I used no
old Planar tool, queue, skill, role or installed binary.

## Exact candidate

- `git diff HEAD -- src` sha256 is `e13ef3b13f6ee5303ea3aa3ae480ee3f0fbf82e941053ca49292c8916057076b`.
  This equals the round record's Final record, the gate `REPORT.md` and `PROBES.md` run 2
  (`review/raw/identity.txt`).
- All seven per-file sha256 values in the record's identity table equal the working tree, and so
  does the `runtime.cpp` reference hash `a82d1abf…7c3c`. `src/engine/runtime/*` and
  `src/cmd/*` are unmodified.
- `git status --short` lists exactly the four modified tracked files and the four untracked files
  (three `profile_acceptance.*` files and the round record). Evidence directories are gitignored
  (`.gitignore:37`), as in earlier rounds.

## Independent verification

| Check | Result (raw file under `tests/native/v5-02-x06-c-evidence/`) |
| --- | --- |
| Gate raw exits | All seven `raw/*.exit` files read 0. `raw/ctest.log` reads "100% tests passed out of 1028". Its only "skip" text is in the names of tests #560 and #1000. Test #531 took 15.32 s, against 15.28 s in the B3-C-C02 baseline log, so it is not slowed. |
| No-work build | My `cmake --build build/rewrite-debug --target planar_tests` and plain `cmake --build build/rewrite-debug` both exit 0 with `ninja: no work to do.` (`review/raw/build-*.log`, `.exit`). |
| Inventory | `ctest -N` exits 0 with "Total Tests: 1028" (`review/raw/ctest-N.*`). Compared as sets with `tests/native/v5-02-x06-b3-c-c02-evidence/inventory.txt` (1003 names): 0 removed, 25 new (`review/raw/removed-vs-baseline.txt`, `new-vs-baseline.txt`). My new list equals the gate's `raw/new-names.txt`. |
| Focused run | `ctest -R '^replacement\.(X06 C\|atomic )' --no-tests=error`: 46 of 46 passed, exit 0, including all 25 X06 C names (`review/raw/ctest-x06c-atomic.*`). |
| Diagnostics | The only literal removed from tracked product code, "Unavailable prepared acceptance", comes back byte-identical in both adapters. "Capture changed before acceptance commit" and "Unsupported profile acceptance" are unchanged. One new literal appears: "Prepared profile differs from request intent" (`profile_acceptance.cpp`), which is in no contract and was not in the tree at HEAD (`review/raw/diagnostic-literals.txt`). See R3. |
| Dependency audit (criterion 11) | `runtime.cppm`/`runtime.cpp` import `ledger.completion` only from the ledger. `planar_runtime` DEPENDS lists no ledger acceptance or storage target. `ledger.acceptance` imports storage, completion, prepared and tree_core, never runtime. The new `planar_ledger_profile_acceptance` depends on runtime, acceptance, completion and storage_types, and only its own test target uses it. There is no cycle. The generated CLI variants do not import the new module, and the full build plus `planar_tests` succeed (`review/raw/dependency-audit.txt`). Criterion 11 is met. The record marked it "partly met"; this audit closes it. |
| Order in the shared body | Read in `acceptance.cpp`. The in-use receipt check comes before `begin_write`, then the identity check, then the request lookup, where replay publishes the owner and then the phase, or a conflict refuses. Then come the format check, the embedded profile refusal, the head read with CAS, `values()`, interning, `append_acceptance_rows` and `verify_before_commit`. Only then does `candidate.verify()` run, followed by the uncertain phase and COMMIT. This matches criteria 2 to 5 and 7. |
| Red-first | I read `red/RED.md` and the raw logs t13, t16, t18, t22 and t26. Each is a behavioural assertion failure against the stub, with its line and expansion, and none is a compile failure. 23 rows are red. t11, t24 and t25 are guards that passed by design. |
| Probes | I read `probes/PROBES.md` and the raw diff of X27. The 53 runs were made in a scratch copy, each restored by hash. All 19 table rows I1a to I5d are killed, with the listed test among the failures. The survivors X12, X18 and X20 are argued equivalent, and I accept that argument. X25 and X26 are killed only by the existing runtime test. |

## What is sound

- **Single implementation (criterion 8).** Both embedded overloads and the profile overload call
  `complete_acceptance_impl` through the `native_candidate` interface. The embedded wrapper reads
  the same fields as before: `stdlib_effective` is false, so it keeps the provenance limits, and
  the interpretation and versions bytes are the same. The shared-body mutants fail both test
  families: I1c fails 22 of 41 tests and I4a fails 8.
- **Receipt races (criterion 5).** The SQLite trace hook changes a closure file or the profile
  module at the staging `UPDATE current_state`, and separately changes a closure file inside the
  pre-commit `PRAGMA application_id` check. Both runs refuse `capture_changed` with no request
  row, and an unchanged control commits. Source and profile captures are each raced. I5c, I5d,
  X01 and X29 are killed.
- **No Lua under lock (criterion 6).** I checked that the evidence observes this rather than
  asserting it. `prepared_profile::observe()` reads the live counters of the retained, dormant VM
  (`state::instructions` and registrations). The test samples them at every writer-connection
  statement, for a commit, a replay and a refused run. X27 makes `verify_native_receipts` bump
  the registration counter, and the test kills it. `verify_native_receipts`
  (`runtime.cpp:5100`) calls only `stage.source.verify()` and `capture.verify()`.
- **Carried item 1, public error.** `response.cpp:150` maps every `capture_changed` to
  "Captured declaration inputs changed". The ledger-boundary message stays "Capture changed
  before acceptance commit", and the test pins that neither "Captured inputs changed." nor
  "Captured external profile changed" appears there (X02 is killed). `verify_native_receipts` and
  its runtime test are byte-identical.

## Findings

### X06C-R1 (required, source): the seam is exported to every importer of `ledger.acceptance` and accepts caller-built data and a receipt callback

`candidate_values`, `native_candidate` and `complete_native_candidate` are declared inside
`export namespace planar::engine::ledger` in `acceptance.cppm`. Every module that imports
`planar.engine.ledger.acceptance` can see them. Today those importers are
`src/cmd/planar/invocation.cpp`, `ledger/history_verification.t.cpp` and
`export/source_export.t.cpp`, besides the adapter itself. Through the seam, any such caller can:

- supply the source, registry and stdlib trees, the workspace id and the interpretation and
  versions bytes directly. The source tree is no longer tied to a native `acceptance_capture`, as
  it is with `prepared_acceptance::create`.
- supply `verify()`. This is a caller callback that replaces native receipt verification, and the
  ledger runs it inside the write transaction just before COMMIT. Criterion 6 then holds only for
  the one in-tree adapter, not structurally.
- return `selects_profile() == true` to turn off the embedded "Unsupported profile acceptance"
  refusal.

The round's own seam test shows this. `fake_candidate` in `acceptance.t.cpp` imports only
`ledger.acceptance`. It returns 4097 fabricated stdlib files with `verify()` returning true and
`selects_profile()` returning true, and it reaches `append_acceptance_rows` and
`verify_before_commit`. The only thing that stops it is the storage consistency check: the
family-versus-selector rule at `storage.cpp:938` and the re-derivation of the interpretation.
That check limits whether the data is internally consistent. It does not check where the data
came from.

This conflicts with:

- criterion 9: "no API constructs or accepts a prepared profile, `profile_acceptance_data` or a
  receipt from a caller; no raw handle or callback escapes; the new surface is not exported past
  what the adapter needs; compile and public surface audit shows no new construction path".
- the dispatch line "Nothing accepts caller-built data as authority".
- the dispatch's halt clause for a placement choice that alters storage authority.

The record marks criterion 9 "met, compile-time only". Its guard (t25 / X28) checks only the
`prepared_profile` overload's parameter types and never looks at the seam. No authority analysis
of the seam is recorded.

Required:

- Restrict the seam so that only the two in-tree adapters can reach it. For example:
  - move `native_candidate` and `complete_native_candidate` into a separate internal module that
    `ledger.acceptance` imports without re-exporting, with a CMake edge carried only by
    `planar_ledger_acceptance` and `planar_ledger_profile_acceptance`; and/or
  - add a passkey that only the two adapters can construct.
- Then add a compile-time surface check that a plain `import planar.engine.ledger.acceptance`
  (as the CLI does) cannot name `complete_native_candidate` or derive a usable candidate.
- Rework the I1i seam test to fit, and record the authority analysis.
- If the operator instead wants the seam public, that needs a recorded amendment and is not this
  reviewer's call.

### X06C-R2 (required, test): criterion 1's "built-in and custom job mix" is not covered

Criterion 1 requires a real prepared profile "(built-in and custom job mix)". `profile_fixture`
rewrites `shape = "fanout_review"` to `shape = "custom"` in both `combined.lua` and `split.lua`.
The quota workspace has only those two jobs (`fixtures/render/quota/workspace/workspace.lua`), so
every completion in `profile_acceptance.t.cpp` has custom jobs only. No profile completion stores
a built-in job's classification. Fix it by leaving one job on the built-in shape, or by adding a
mixed-fixture case to the positive tests, and record the red or probe evidence for it.

### X06C-R3 (required, records): a new internal diagnostic and an unsourced "Decision B"

`profile_candidate::values()` refuses a mismatched bound intent with `invalid_argument` /
"Prepared profile differs from request intent". The text is new; it was not in the tree at HEAD.
Yet the record's "Changes versus the dispatch" says "no new diagnostic message". Rows I1g and X16
cite "Decision B" for this behaviour, but nothing in the record or the evidence says where that
decision came from.

X-LIMIT in `external-profile-contract.md` says new native sites use the closed message table.
The record must:

- name the source of Decision B;
- either show, in the same way as carried item 1, that this is a ledger-internal text that never
  reaches the public surface (`response.cpp:86-88` maps every `invalid_argument` to `schema` /
  "Invalid ledger arguments"), or reuse an existing message;
- correct the "no new diagnostic message" statement.

If the analysis holds, this is a records-only fix.

### X06C-R4 (required, records): carried item 1 contradicts the implemented moved-from order

Carried item 1, point 2, says the adapter "must refuse [a moved-from owner] before opening a
transaction as `invalid_argument` / 'Unavailable prepared acceptance'". The implementation does
something different. It refuses inside the transaction, after the replay lookup, the format check
and CAS, and before any row is written. The test is named accordingly ("moved-from profile
refuses after CAS before any row") and also checks that a stale head gives `revision_conflict`.

That order is the correct one. Criterion 2 requires a moved-from profile to replay, which needs
the locked lookup, and the embedded overload refuses at the same point. The record must
nonetheless reconcile the text, the acceptance table and criterion 7's wording, which is about a
moved-from or in-use receipt and not a moved-from profile. This is a records-only fix.

### X06C-R5 (minor): record and comment inaccuracies

- Row I3b describes the embedded refusal as "Unsupported profile acceptance" /
  `unsupported_version`. The code (`acceptance.cpp`, unchanged except for the
  `selects_profile()` guard) uses `unsupported_state`.
- The comment at the top of the no-Lua test ("no mutant is constructible … not a mutant killer")
  contradicts the record and `PROBES.md`, where X27 is killed by that test. Correct the comment
  when the source is touched for R1.
- The record has no search section with the placement justification or dependency audit that
  the dispatch asked for. The invariant table rows marked (r) were reconstructed. My dependency
  audit above covers criterion 11, so this is a records gap and not a test gap.

## Limits carried and observed

- The red-first snapshot hash (`fb54336b…`) covers tracked files only. The stubs and the new
  tests lived in untracked `profile_acceptance.*`, so the hash does not identify the code that was
  red. The test added after red-first (same-id intent, X16) and the repaired seam test have no red
  record. Their kills rest on the probes (X16, I1i). I accept this as recorded, not as a finding,
  because it cannot be reconstructed without a new red run. Future test-first rounds should hash
  untracked files too.
- Ledger tests alone do not tell the source receipt set from the profile set or pin their order
  (X25 and X26). Only the existing runtime test kills those mutants.
- The I9 guard is compile-time only. X28 is a build failure, not a runtime kill.
- The mutants ran against 41 ledger tests (13 runtime tests for X25 and X26), not the full suite.
- Carried and unchanged: P01 masked, N1 phase coverage unmeasured, macOS Debug only, Linux not
  run, fresh profile acceptance still refused at the public surface.
- The redact check (`raw/redact-check.*`, 22:21) predates the 22:59:49 test edit. It must be
  re-run at commit on the final file list, as the record already says.
- `docs/architecture.md` does not describe the new `ledger.profile_acceptance` module. It is
  outside this round's owned files, and the B3 rounds did not update it for `prepared_profile`
  either. Raise it when the module graph is next documented.

## Review evidence

The files are under `tests/native/v5-02-x06-c-evidence/review/raw/`:

- `identity.txt`: HEAD, diff and per-file hashes, status.
- `build-planar_tests.*` and `build-all.*`: the no-work builds.
- `ninja-dry-*`: the earlier dry runs.
- `ctest-N.*`, `inventory-now.txt`, `removed-vs-baseline.txt` and `new-vs-baseline.txt`: the
  inventory check.
- `ctest-x06c-atomic.*`: the focused run.
- `dependency-audit.txt`: the dependency audit.
- `diagnostic-literals.txt`: the diagnostic literal check.
