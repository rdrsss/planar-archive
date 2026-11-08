# V5-02-R15-B review (native format3 storage and commit path)

Decision: **request-changes** (not records only).

Most of the slice is solid. The 69 stored statements match the EX-1 SQL block byte for byte, in the product and in the
test's own independent copy. Public `init` is untouched and still creates format2. Nothing outside the owned list
changed: no CLI, Lua, `runtime.cpp`, `acceptance*`, handler or help file. The `storage` edit adds one forwarding
function and nothing else. The 1439 baseline names are all still present, 171 names were added and none removed. The
four new test executables pass when I run them directly (7, 22, 23 and 119 cases).

I cannot approve it for three reasons:

- The commit path does not do the EX-3 step 6 identity recheck (R1). This needs a product change and a test.
- The headline acceptance probe, the permissive "write the complete vector" lost-update mutant, is killed for the
  wrong reason (R2). It breaks every claim through a CHECK constraint, not through the lost-update assertions.
- The macOS Debug gate is not green and part of the run 5 gate output lives outside the evidence folder (R3).

R4 is a records defect.

Scope: the uncommitted candidate on HEAD `031ea4fc`, with the [round record](V5-02-R15-B.md) and
`tests/native/v5-02-r15-b-evidence/` (`plan/`, `red/`, `probes/`, `raw/`, `REPORT.md`, inventories). I ran no script
under `tests/native/`. I applied no mutants.

## Candidate identity (workflow `identity.py`, run by me with pathspec `src`, verbatim)

| identity | value |
|---|---|
| HEAD | `031ea4fce1bada519663a223f36ac8869e3b34f6` |
| sha256(git diff --binary HEAD -- src), tracked only | `36a8d7a4fdc5adc523563a1887e2da3bd1b124498847a5d2fc8d4dbd6d7837c9` |
| combined sha256 (diff bytes, then each untracked path NUL bytes, sorted), pathspec src, evidence excluded | `ca0fb9499f394c5b76adb1ffde62beded5c5b604382974018266eeb64c46b9d2` |

| file | state | sha256 |
|---|---|---|
| `src/engine/CMakeLists.txt` | modified | `231e57ebbaea763c074b10226ea7fab4c2e1c350d6c7b2817b46f92687155a2f` |
| `src/engine/ledger/storage.cpp` | modified | `faac978f2d926f7871813999aff9af447d63eab7bdd092643138dd71c5c3115c` |
| `src/engine/ledger/storage.cppm` | modified | `ef5ff6788363dd36a09eea686acc415f1e57460ef8854c7a11e1c58fc645f5cc` |
| `src/engine/history/execution_codec.cpp` | untracked | `9c016ce950681856a227347aa38c84e095a485c8221753bdaac048e74e5221fa` |
| `src/engine/history/execution_codec.cppm` | untracked | `49d48384282827f17179372b74949621b2fcde55142568e16a985eec8e56e124` |
| `src/engine/history/execution_codec.t.cpp` | untracked | `c00a4ef5e552ee7f410789ebc90cd4648db8a5bd62192557cd54cfcdd8f10faf` |
| `src/engine/history/request_v3.cpp` | untracked | `3c2c1e6af6b1043ddcbf75db68dae8c774063bdcc0b51a250f58d866cbbd7a3d` |
| `src/engine/history/request_v3.cppm` | untracked | `c1262eb00efc4c11498cbe73b260633721ae888a5a1380d945de9cb26e75e0e3` |
| `src/engine/history/request_v3.t.cpp` | untracked | `daf7f3bbe3563f31466b5855cc65550b1800289d881b7888270121559c3c296c` |
| `src/engine/ledger/execution.cpp` | untracked | `5b401ea917dfef20218878abf5decaa52c5ca21205cbd74854f15316a982af84` |
| `src/engine/ledger/execution.cppm` | untracked | `0658494ded05c16b8a6245c8486eecc5e4ebe974319d0e37dcacb74d6f0eb468` |
| `src/engine/ledger/execution.t.cpp` | untracked | `d7111c47276d34e24f0583b6edfb83f4004a251688134f628b8761915ce7c3d5` |
| `src/engine/ledger/execution_schema.cpp` | untracked | `896f3d791a214c7de2f86211c7270a049d70ac8853ad36fd04111e15fef8fbbf` |
| `src/engine/ledger/execution_schema.cppm` | untracked | `05c659c3470c091a80b0608ec5343b331292dd22be9fd437de6a35efc782801e` |
| `src/engine/ledger/execution_schema.t.cpp` | untracked | `af161384373002f74ba654190d079525af7862deb49f6e7883465a48a4917f9a` |

Every value matches the record's "Final candidate identity" table, `raw/run5/identity.txt` and
`probes/raw/identity.txt`. The Linux container's context manifest for run 5 lists the same per-file hashes (in the
gate's out-directory, see R3). So the gated, probed and reviewed candidates are the same bytes.

## Findings

### V5-02-R15-B-R1 (blocking, product and test): the locked commit does not recheck the ledger identity

EX-3 step 6 says: "The commit runs in `BEGIN IMMEDIATE`. It rechecks identity, format and schema, then relooks up the
request ID". DECISIONS D12 step 5 and D30 say `stage_execution` "re-verifies identity/format/schema with
`verify_format3_schema`" after `BEGIN IMMEDIATE`.

The candidate checks format and schema only:

- `verify_format3_schema` checks that `ledger_meta.identity` is 32 lowercase hex. It never compares it with anything.
- `snapshot.identity` is set in `build_snapshot` (`execution.cpp:1544`), and no code reads it. No line in
  `execution.cpp` compares it with `database.metadata().identity`.
- The file-identity check (`native::identity_matches`) in `open_format3` only ties the read-only open and the
  writable open inside one `stage_execution` call. It does not tie the snapshot read to the commit.

Failure scenario: a handler reads a snapshot of ledger A and builds a change. Before the commit, the file at that path
is replaced by another format3 ledger B whose head, active job, role states and fact versions happen to match. For
example, B can be a ledger set up by the same init/load/activate sequence; only its identity differs. The change is
then committed into B, and the saved result carries B's identity.

The format2 path refuses this case explicitly. `complete_acceptance(...,expected_ledger_identity,...)` compares the
prelude identity with the writable transaction before request relookup and refuses with `unsafe_path` "Ledger identity
differs from prelude" (`acceptance_seam.cpp:28-29`; load-command-contract "never acceptance into a replacement
ledger"). The doc comment on `stage_execution`'s `snapshot` parameter makes a similar claim that the code does not keep:
"its equality with the stored state is checked before the lock".

Required:

- Compare `snapshot.identity` with the locked `ledger_meta` identity right after `BEGIN IMMEDIATE` and the schema
  recheck, before the request relookup. On a mismatch, refuse with the existing native category that maps to the
  unsafe-path refusal (for example `path_denied`). Add no new message.
- Correct the doc comment.
- Add an invariant row with a red-first behavioural test. The test commits a change prepared on ledger A into a
  same-shaped ledger B at the same path, and asserts the refusal, no effect on B, and that A's same-ID retry still
  works.
- Add a mutant that drops the comparison.

### V5-02-R15-B-R2 (blocking, evidence): the permissive lost-update mutant is killed by a CHECK failure, not by the guard

The acceptance criterion says that "the permissive 'write the complete vector' mutant is killed" (row K6). The K6
mutant (`probes/superseded-1/raw/K6.log`, diff `922cf9b1...`) adds a loop after the delta write:
`UPDATE role_state SET state = ?3, drifted = ?4, run_id = ?5` for every `change.vector` entry, with
`optional_int_arg(entry.run)`. The vector entry of the claim target has no run, because
`execution_vector_entry::run` is "absent for a run created by this change" (`execution.cppm:182`). So the mutant writes
`running` with `run_id` NULL for the target, and the `role_state` table CHECK
`(state='running' AND run_id IS NOT NULL) OR ...` refuses it.

The log confirms this. The failing assertion is `first.has_value()`, on the **first** claim. That assertion comes
before any stale entry exists. Any test that commits one claim would fail under this mutant. So the probe does not show
that the test, or the delta write, catches a stale vector overwriting a concurrent claim.

I expect a faithful mutant to be killed: the second claim's stale `builder = ready` entry would either fail the
`running` assertion or be refused by the pre-COMMIT verifier (D47), which fails `second.has_value()`. But that has not
been shown, and this is the round's headline acceptance probe.

Required:

- The prober re-runs K6 with a faithful permissive mutant, keeping the first log. The mutant should write every vector
  entry after the delta, substituting the allocated run id for the target entry (or leaving the target entry out).
- The record states which assertion kills it. If the kill comes from the verifier rather than the delta write, the row
  should say so.
- If the faithful mutant survives, it stays a survivor and the acceptance row is not met.

### V5-02-R15-B-R3 (blocking, gate evidence): macOS Debug is not green, and run 5's Linux output is outside the evidence folder

The acceptance criterion requires the macOS Debug, macOS RelWithDebInfo and Linux aarch64 gates to be green, with a
no-change build that does no work.

- Run 5 macOS Debug exited 1 (`raw/run5/2-gate-debug.exit`): 1609/1610, with #1030 timing out at 30.05 s under a load of
  about 74. The single rerun passed in 15.19 s (`2b-1030-rerun.log`).
- Because ctest failed first, the Debug gate's formatting, Doxygen and no-change steps did not run.
- The record classifies this correctly as "partly met". It is still not the agreed gate.

Separately, `raw/run5/6-linux-gate.log` contains only the scratchpad out-directory path. I read the container outputs
there:

- `debug-ctest.log` and `release-ctest.log` each show `100% tests passed out of 1610`.
- All ten container exits are 0.
- Both no-change logs show `ninja: no work to do.`
- `manifest-compare.txt` shows the host and context manifests are equal.

These files exist only in the session scratchpad, which is temporary. Earlier runs copied them into `raw/linux-gate-out/`.

R1 forces a new product candidate, so the next gate must include:

- a clean macOS Debug gate, including its lint and no-change steps;
- a copy of the Linux container outputs under the evidence folder (for example `raw/run6/linux-gate-out/`).

### V5-02-R15-B-R4 (required, records): unfilled template and committable host paths

- `REPORT.md:62`: under "New test names since run 4 (repair pass) with file:line and the probe mutant each is meant to
  kill", the table body is the literal placeholder `{rows}`. Fill it with the 19 names, their `file:line` and their
  mutants, or remove the section and point to the record's X1-X19 table.
- `red/rows.txt` contains absolute paths under the home directory. `scripts/redact-local-paths.py check` exits 1 on it.
  If it is staged, it must be redacted first, as the orchestration procedure requires.
- `probes/PROBES.md` names the absolute session-scratchpad path. The redaction script does not flag it, so it would be
  committed as is. Replace it with a placeholder such as `<scratchpad>/probe-warm`.

## Advisories (not blocking)

- **A1, `execution_state_bytes` overclaims.** Its doc comment says it computes "the same measure the reader and writer
  enforce", and "R15-C reuses this single definition". The body calls `measure_state(snapshot, {})`, which measures
  every dispatch at the active revision. The reader passes the real per-dispatch revisions (`execution.cpp:1671`).
  The two disagree whenever a running role's dispatch revision differs in decimal width from the active revision.
  Either export a measure that matches the reader, or narrow the comment and record the limit for R15-C. This function
  and `execution_snapshot::state_bytes` are used only by tests in this round. D30 permits this because R15-C will use
  them.
- **A2, red-first rests on shared fixture lines.** About 117 of the 150 red failures fail at a shared fixture or helper
  assertion: 90 at `execution.t.cpp:742`, 14 at `execution_schema.t.cpp:451` and 9 at `execution.t.cpp:1762`, all
  red-time line numbers. An example is `REQUIRE(initialize_format3(...).has_value())` in the K9 log. These are
  assertion failures against stubs, not compile failures, and earlier reviews accepted that form. But they show only
  that each test runs and fails. Whether a test catches its own clause rests entirely on the probes.
- **A3, survivors on acceptance-relevant clauses.** Some of the 16 row survivors sit on clauses the acceptance criteria
  name:
  - K9: the activate `role_state` member, masked by `check_locked_before`.
  - K32 and K58: the NULL-first check and the no-row member, under limit L23.
  - K38: abort versus commit, an equivalent mutant.
  - V8, V9, V15, V17, V29, V33a: cross-row checks masked by other checks.

  The record correctly says the masking reasons are unconfirmed hypotheses. R15-E should confirm or kill them. V3 has
  no test (L5).
- **A4, L20 narrows the native exact-set rule.** EX-3 (load and activate) says "a role with no `role_state` row asserts
  that no row exists", and D151 requires the exact set. Native accepts these no-row members but never requires them.
  The locked NULL-first check covers the effect, but R15-C must pin the handler side, as the record says.
- **A5, Q-PREC option A.** Under this option the P2-1/D114 lock-time `fenced{run_state}` mapping for a takeover can never
  be reached (L15), and `generation_current` becomes an equivalent-mutant site (L21). The choice is recorded as
  delegated. The operator may want to see this consequence when R15-D wires the public mapping.
- **A6, reserved-table labels.** The EX-1 `-- Reserved for R16/R17/R18` labels exist only in the contract text. SQLite
  cannot store them. The product statement array has no matching per-table comment. A one-line comment per reserved
  table would keep the label next to the code.

## What I verified

- **Identity.** I ran the workflow `identity.py`; its output matches the record, run 5 and the probe identity.
- **Exact schema.** I extracted the contract's EX-1 SQL block (`execution-contract.md` 82-355), dropped the comment
  lines and split it at statement terminators. The 69 statements are byte-equal, in order, to the 69 raw-string
  literals in `execution_schema.cpp` and to the 69 in `execution_schema.t.cpp`. The S1 to S19 tests and probes cover
  the exact-SQL check, the one-byte difference, the missing trigger, the extra object, the wrong markers, the
  immutable and mutable triggers, and refusal of format1 and format2. All are KILLED.
- **Boundaries.** `git status` shows only the listed files.
  - `storage.cpp` and `storage.cppm` add one declaration and one forwarding `native::check_orphan_sidecars`, nothing
    else.
  - `CMakeLists.txt` registers four modules and test links only.
  - `scripts/linux-gate.Dockerfile` changes only the LLVM apt pin and its comment.
  - `docs/architecture.md` adds four module rows.
  - No module outside the four new ones imports them, and `initialize_format3` has no product caller.
  - The boundary pin rule is not triggered.
- **Diagnostics.** The new modules contain no message strings. Failures use the closed `execution_error_code` set,
  so no closed diagnostic byte changed.
- **Inventory.** Sorted comparison against `v5-02-ra01-c-c01-evidence/inventory.txt` (1439): zero removed, 171 added
  (7 + 22 + 23 + 119). Every test named in the invariant table and the X1-X19 table exists. No name starts with `-`.
- **Invariant table.** All 153 rows of `plan/INVARIANTS.md` appear in the record with the same invariant, mutant and
  test text. The only difference is the escaped pipe in V45. Every acceptance criterion has rows:
  - schema: S1-S19;
  - `request_v3`: Q1-Q7, K1-K5;
  - commit: K6-K15, K37, K38, K44;
  - fencing: K16-K27;
  - cross-row: V6-V47;
  - format1 and format2: S13, V5, G1, G2;
  - EX-8: Q6, C11, C14, V4, V30, V31, K40, K41;
  - F8: V1.

  D149 moves to R15-C under L10, as the acceptance clause allows. F3 to F7 are explicitly carried.
- **Probes.** I read `probes/PROBES.md`, `results.jsonl` and the driver. Mutants were applied by exact substitution in
  a scratch copy, restored with a sha256 assert and built with ninja. The stale-object incident is explained, and the
  baseline was taken after restoration. Survivors are recorded as survivors. I checked the K6 log in detail (R2) and
  sampled the failure excerpts of every K row. Most K kills are a refused commit (`result.has_value()`), which is
  legitimate where the pre-COMMIT verifier or the one-active-run index enforces the invariant.
- **Red-first.** `red/RED.md` gives the exact per-test `ctest` command, the red-time diff hash, `SHA256SUMS` and
  `red/snapshot-2/`. Attempt 1 failed to build and is honestly labelled as not behavioural.
- **Contract spot checks.**
  - Replay comes before the CAS and the preconditions. Request conflict is a byte comparison of the stored frame.
  - The load CAS compares the intent head with the locked head.
  - Preconditions are evaluated in vocabulary order, and the claim-conflict state list is closed (P2-1 line 957).
  - The locked before-state recheck runs, and fencing comes before the new run's insert.
  - The secret comes from `getentropy`.
  - Token authorization returns `fenced{state}` or `unknown_run`.
  - The hash-collision detail for a request has an empty digest.
- **Direct test run.** I ran the four new test executables in `build/rewrite-debug`, whose binaries are newer than every
  candidate source. All passed with 229, 1275, 1643 and 77101 assertions.
- **Redaction.** `scripts/redact-local-paths.py check` exits 0 on the round record, `REPORT.md`, `RED.md` and
  `PROBES.md`, and exits 1 on `red/rows.txt` (R4). Raw logs are gitignored and stay local.

## Limits

- I applied no mutants (only the prober does). R2's claim that a faithful mutant would be killed is my reasoning from
  the code, not an experiment.
- I did not rebuild or re-run the full gate. I ran only the four new test binaries in the existing Debug build.
- I read the commit path (`apply_locked`, `check_preconditions`, `check_locked_before`, `stage_execution`, the delta
  write and `authorize_run_token`) and sampled the verifier. I did not audit all 3363 lines of `execution.cpp` or all
  42 verifier rows against EX-1 and EX-4 line by line.
- The Linux container output I read is in the session scratchpad (R3). It will not survive the session unless it is
  copied.
