# V5-02-X07-R1 independent review (whole X07 candidate, including the ledger sidecar repair)

Disposition: **request-changes (source, tests and records)**.

I reviewed the whole uncommitted X07 candidate against HEAD `7a81b0bc`: waves A and B, the gate-repair edits and the
Dispatch 2 sidecar repair in `src/engine/ledger/storage.cpp`. Most of the cutover holds up. The inventory, the closed
diagnostics and the public surface are as recorded, and the CAS acceptance test is now stable. The sidecar repair,
though, does not do what Dispatch 2 asked for, and I could show the original defect still occurs with a real
concurrent writer. The repair adds a 1 s grace poll in front of the same pre-lock check. It does not move the check
under the held lock. So a live writer that keeps its rollback journal for more than about 1 s still makes a concurrent
`load` or `history` fail with exit 4, "Explicit ledger recovery is required". That is well inside the contract's 5000 ms
busy wait. Fixing this needs a change to source and tests, so the round cannot be fixed in records alone.

The round record is candid. Its "Final candidate (R1)" section already reports criteria 1 and 2 of Dispatch 2 as not
met and asks for reviewer judgment on the grace. My judgment is that the grace is not acceptable as it stands
(finding R1 below).

What holds:

- **Candidate identity.** I re-ran identity.py on `src tests/native`. HEAD, the tracked-only hash, the combined hash and
  all 29 per-file hashes are byte-identical to the round record's identity table, the gate report and PROBES.md. No
  candidate file changed after the second gate run.
- **CAS stability (Dispatch 2 criterion 1).** I ran `x07.two_process_cas_one_winner` serially, 25 times in
  `build/rewrite-release` and 20 times in `build/rewrite-debug`, after confirming both trees were current (`ninja: no
  work to do`). Results: 25 of 25 and 20 of 20 passed. The logs are `review-r1/cas-release-25.log` and `cas-debug-20.log`.
  This closes the evidence gap the record reports for criterion 1. The test passes because of the grace, which R1
  finds insufficient, so it stays a weak guard. It runs 8 short loads, and the record says M01 does not kill it.
- **Inventory (criterion 3, original criteria 2 and 4).** `ctest -N` gives 1213 names in each configuration, and the
  two sorted lists are byte-identical. All 1114 X07-DOC baseline names are present in both, with 0 removed and 99
  added (`review-r1/inventory-check.txt`). Edits to existing tests only add assertions: format2 checks in
  `bootstrap_init.py` and `bootstrap_load.py` (D7-11, clauses 24 and 25) and the expected public-section text in
  `acceptance.t.cpp` for the factory and `run_known`. No assertion was removed or weakened.
- **Diagnostics (criterion 4).** The sidecar message "Ledger sidecar requires separately audited recovery" is unchanged,
  and the new byte-exact test pins it. I extracted every message-like string literal from the added non-test product
  lines and the four new product files, 61 in all, and looked each one up in a HEAD extract of `src` and `docs`
  (`review-r1/message-literals.py`, `.log`). Only one is new: "Expected explicit profile selector", the CLI11 validator
  return in `envelope.cpp:9`. CLI11 only tests whether that string is empty. Every `CLI::ParseError` is caught in
  `envelope.cpp:72` and `bootstrap.cpp:225/284/324` and mapped to the fixed usage response, so the string never reaches
  output. That follows the existing validator pattern ("Expected explicit value"). It is not a public diagnostic.
- **Public surface (criterion 5).** The sidecar repair adds no field, method or header change, and `storage.cppm` is
  untouched. The round's additions to `profile_invocation` are what the addendum designed: two `open_with_profile`
  overloads and `run_known`, plus the private `open_admitted`. The X06 D surface test pins them, and M14 kills an
  extra method. The new exported cmd helpers all have product callers in `bootstrap.cpp` or `invocation.cpp`. Two of
  them are exported for the reader-count tests: `prepare_profile_invocation` and `invocation_snapshot`. That is the
  recorded decision DC42 (a cmd helper, not a product-type field).
- **Known-limit note.** The gate-repair "known limit" (that the CAS test is race-prone under load) is now marked
  SUPERSEDED in the record (line 111). Dispatch 2 and the Final section call it a defect. That reconciliation is
  correct. R1 shows the defect is narrowed, not removed.
- **X1 survivor.** I confirm the test coder's reasoning that X1 is an equivalent mutant. `classify_prefix` refuses when
  `selector_option->count() != 1` (`envelope.cpp:49`). In the vendored CLI11, `Option::count()` returns
  `results_.size()` (`vendor/cli11/9d6a/include/CLI/Option.hpp:388`), which counts every occurrence whatever the
  multi-option policy. So with `TakeLast`, a duplicate selector is still a usage error with the same bytes.
- **Gate logs.** I read the raw logs rather than the summaries. `g2-03` has 260 of 260 passed, `g2-04` and `g2-07` have
  1213 of 1213 passed with no "Skipped" or "Not Run" lines, `g2-07-exit.txt` is 0, lint checked 147 files, `g2-10` is
  "no work to do", and `g2-14` is empty. #531 took 0.09 s in Debug.

## Exact candidate

I reviewed in the shared checkout on `dev/planar-v2-vernacular`. The candidate is uncommitted. I ran the identity tool
from the repository root with pathspec `src tests/native` (`tests/native/v5-02-x07-evidence/review-r1/identity.txt`):

| identity | value |
|---|---|
| HEAD | `7a81b0bcb8c85c3d8f75a6b9ea1dc9a1c66f19a4` |
| sha256(git diff --binary HEAD -- src tests/native), tracked only | `a96814439fc55317332d89beb73b88f5f28bf46d73c2299ec4d83b3663658f6b` |
| combined sha256 (diff bytes, then each untracked path NUL bytes, sorted), pathspec src tests/native, evidence excluded | `0568aea2a156a3bbacbc2eae9c711a527a93fc7cb062fa3b9a539aaf6168870a` |

The per-file table equals the record's table row for row; for example, `src/engine/ledger/storage.cpp` is
`5a521ee1822a134dcf528c7f1d759197c17d34f6af3d400a9b73ee810d0b3afd` and `src/engine/ledger/storage.t.cpp` is
`ce1dea1b61fc8dd80c1123f2582ee15b19cde9269d2aeda0e2a66954b1ba674b`. Docs and `tests/acceptance/` are outside this
pathspec; I reviewed them by diff.

## Findings

### X07-R1-R1 (required, source): the sidecar race is narrowed, not fixed, and the grace conflicts with the ledger contract

Dispatch 2 required sidecars to be checked "only once this connection holds the lock". It also said that "a sidecar
observed while another connection may hold the lock must not refuse as `recovery_required`". The repair instead keeps
all four pre-lock checks: `open_read` at `storage.cpp:1170` and `:1194`, and `begin_write` at `:1376` before
`db.begin()`. It changes `no_sidecars` (`:235-256`) to poll up to 200 times at 5 ms and then refuse with
`recovery_required`. The real ordering defect is still there, with a 1 s threshold.

I reproduced the defect without mutating anything. Using the candidate release binary in a scratch HOME, a second
SQLite connection runs `BEGIN IMMEDIATE`, modifies a page so that `<ledger>-journal` exists while it holds RESERVED,
waits H seconds and rolls back. Meanwhile the candidate runs a plain `load` (the write path) or `history` (the read
path). Scripts and output are in `review-r1/live-journal-{load,history}.py` and `review-r1/live-journal.log`:

| writer holds journal | `load` | `history` |
|---|---|---|
| 0.3 s | exit 0 after 0.35 s | exit 0 after 0.32 s |
| 3.0 s | exit 4 after 1.36 s, `io` "Explicit ledger recovery is required" | exit 4 after 1.38 s, same |
| 4.5 s | exit 4 after 1.36 s, same | not run |

In each run, the same command succeeds once the writer releases. Under the contract
(`ledger-contract.md:166`, "SQLite busy wait | 5000 ms; timeout is a typed busy failure, no silent retry loop"), a
3.0 s or 4.5 s writer is lock contention. The loser should wait and then succeed, or report `revision_conflict` or
`ledger_busy`. It should never report crash recovery. A Planar winner can also hold its journal for more than 1 s:
interning a large captured source set (up to 64 MiB, `:158`) inside the write transaction, or a slow disk, is enough.
The grace is also the kind of "silent retry loop" the busy row excludes, and its timeout is reported as
`recovery_required`, not as a typed busy failure.

The change also alters `format.cpp` without the justification Dispatch 2 asked for. `no_sidecars` is shared, so init
publication (`format.cpp:235`, `:282`) and the explicit recovery path (`:536`, `:555`) now each wait up to 1 s before
refusing. The record mentions only the slower refusal tests.

Required: make the decision depend on the lock, not on time. One option is the prescribed order: before the lock, a
sidecar is not a refusal by itself; under the held lock, after `BEGIN IMMEDIATE` on the write path or inside the read
transaction on the read path, a sidecar refuses with the unchanged message. Another option, if the coordinator finds
that SQLite would roll back a hot journal when it takes SHARED before Planar can refuse, is a liveness test of the
kind SQLite itself uses. For example, check whether another process holds the RESERVED lock byte. A held lock means
contention, governed by the 5000 ms busy wait. A sidecar with no lock holder means recovery. Either way:

- Keep `format.cpp` behaviour unchanged, or justify each changed call site in the record.
- Keep every message byte-identical.

If the coordinator wants to keep a time-based grace, that is a choice against the busy-wait row. It needs an operator
decision and a contract amendment, not a reviewer waiver.

### X07-R1-R2 (required, tests and evidence): no real concurrent-writer regression, and no red-first evidence

Dispatch 2 criterion 2 asks for a test with "two connections or two processes; one holds a write transaction with a
live `-journal`". None of the four new tests (`storage.t.cpp:185-233`) has a concurrent SQLite writer. The "transient
live writer journal" test writes a one-byte plain file and deletes it from a thread after 150 ms. It passes with the
grace and would also pass with a correct lock-ordered fix, but it cannot detect the R1 case, a writer holding its
journal longer than the grace. The record and the gate report also confirm there is no behavioural red-first evidence
for any of the four tests: no command, no pre-fix snapshot hash and no failing line. Lesson 5 requires all three. The
M01 kill reproduces the pre-fix behaviour, but it is mutation evidence, as the record says.

Required: add a ledger-level test where a second real connection (or process) holds `BEGIN IMMEDIATE` plus a page write
for longer than any grace and shorter than 5000 ms, for example 2 to 3 s. `begin_write` (and the read open) must then
succeed or reach CAS, never `recovery_required`. Record that test red on its assertion against the pre-fix snapshot
(HEAD `storage.cpp` plus the new test), with the exact ctest command and the identity.py hash of that snapshot. With
the grace still in place, this test should fail as well. My `live-journal.log` reproduction shows the same behaviour
through the CLI.

### X07-R1-R3 (required, records): the R1 plan and red records are missing, and the named mutant does not apply

The review brief points to `tests/native/v5-02-x07-r1-evidence/plan/INVARIANTS.md` and `red/RED.md`. Neither exists:
the directory holds only `REPORT.md`, `inventory*`, `raw/` and `probes/`. The X07 invariant table
(`v5-02-x07-evidence/plan/INVARIANTS.md`, I01 to I125) has no row for any Dispatch 2 criterion, and a grep for
`sidecar` or `journal` there finds nothing. The only rows are the prober's M01 to M15 in PROBES.md, which are not
tied to a plan.

The named R1 mutant, "sidecar check moved back before the lock", cannot be applied because the check was never moved.
The prober applied a different mutant instead: M01 removes the grace. Survivors M09, M10 and M12 follow directly from
keeping redundant pre-lock checks.

Required, once R1 and R2 are done: write an R1 invariant table with one row per Dispatch 2 criterion and the closure of
the original CAS defect. Each row needs its real mutant (the sidecar check placed before the lock or made
lock-independent; a persistent sidecar under the held lock accepted; the message changed) and a named killing test.
Write `red/RED.md` for the new tests. Re-run the prober on the new rows.

### X07-R1-R4 (required, docs): `docs/cli-reference.md` documents an envelope syntax the binary refuses

`docs/cli-reference.md:263` says "The with-profile envelope is `planar --stdlib-profile PROFILE COMMAND ...`". The
contract form is `planar with-profile --stdlib-profile FILE COMMAND [OPTIONS]`
(`external-profile-contract.md:45-46`). `envelope.cpp` registers the `with-profile` subcommand, and every X07 test
uses it. I ran the documented form with the candidate binary, and it is a usage error, exit 2 (`review-r1/cli-syntax.log`).
The same page says "The prefix is exactly three tokens", which only works if `with-profile` is counted. Clause 32
binds this page, and the record notes that the docs content was never audited. Required: correct the synopsis and add
the selected-help form `planar with-profile --stdlib-profile FILE --help`.

### X07-R1-R5 (minor, non-blocking, source): stale contract note on `execute_profile_load`

The `@note` at `invocation.cppm:56` on `execute_profile_load` says "Preparation and commit are a later wave and
currently refuse with the redacted protocol diagnostic". Wave B implemented both: `invocation.cpp:513` calls
`prepare_load`, and `:521` calls `complete_acceptance`. Fix it with the R1 source change.

### X07-R1-R6 (minor, non-blocking, tests): the non-ENOENT stat test has a loose oracle

"storage begin_write surfaces a non-ENOENT sidecar stat failure" only asserts `code != recovery_required`. Any other
failure on the path would pass. Asserting `format_error_code::io` and the exact "Ledger storage operation failed"
message would pin the behaviour this test is named for. The test relies on a 250-byte name pushing the `-journal` name
past `NAME_MAX` (255 on macOS and Linux). A comment saying so would help.

### X07-R1-R7 (minor, non-blocking, records): owned-file list and survivor disposition

The Dispatch owned-file list does not name `tests/native/r14-d7-b-support/generate.py`,
`tests/native/v5-02-b2-support/generate.py` or `src/engine/runtime/external_registration.t.cpp`. They appear only
under "Changed files by purpose" and in the identity table. Add them to the owned list (lesson 4). Also record X1 as an
equivalent mutant, with the code reasoning confirmed above, so it is not carried as an open survivor.

## Invariant table, red and probes

- **Invariant table.** The original X07 table (118 test-bearing rows) is unchanged by this repair. As R3 says, the
  repair has no rows of its own. Only 4 of the 118 X07 rows were probed (X1 to X4). The record states this, and I treat
  it as a recorded limit of the original round rather than a new finding.
- **Red.** The wave tests' `red/RED.md` (96 names, 79 red on assertion, 17 guards) was produced in the earlier lineage,
  and I did not re-audit it here. The sidecar tests have no red (R2).
- **Probes.** I read `probes/PROBES.md` and the raw logs for M01, M07, M08, M09, M10, M12 and X1. Each log ends with the
  restore line and sha256 match, kills name the failing test, and M10 is shown surviving at "100% tests passed out of
  15". The M01 mutant (`attempt == 0` forces an immediate refusal) is equivalent to the pre-fix behaviour, and it kills
  only the plain-file transient test. As the prober noted, the M01 to M06 kills were carried from the previous pass
  because `storage.cpp` was unchanged.

## Commands I ran (from the repository root, unpiped; logs under `tests/native/v5-02-x07-evidence/review-r1/`)

| Command | Exit | Result |
| --- | --- | --- |
| `python3 <workflow>/scripts/identity.py src tests/native` | 0 | equals the record (`identity.txt`) |
| `cmake --build build/rewrite-release --target planar_tests`, same for `build/rewrite-debug` | 0, 0 | `ninja: no work to do` in both trees |
| 25 x `ctest --test-dir build/rewrite-release --no-tests=error -R '^x07\.two_process_cas_one_winner$' --output-on-failure` | 0 each | 25 of 25 passed, 1 test matched each run (`cas-release-25.log`) |
| 20 x the same in `build/rewrite-debug` | 0 each | 20 of 20 passed (`cas-debug-20.log`) |
| `python3 -I review-r1/live-journal-history.py` and `live-journal-load.py` with holds 0.3, 3.0, 4.5 s | 0 | table in R1 (`live-journal.log`) |
| `ctest -N` in both trees, compared against `v5-02-x07-doc-evidence/inventory.txt` | 0 | 1213 and 1213, identical, 1114 preserved (`inventory-check.txt`, `names-*.txt`) |
| `python3 -I review-r1/message-literals.py` over the added product lines and a HEAD extract | 0 | 61 checked, 1 internal validator string (`message-literals.log`) |
| candidate binary with the documented and the contract envelope forms | 2, 4 | `cli-syntax.log` |

## Limits

- I applied no mutants; the workflow leaves that to the prober. I did not re-run the full Debug or RelWithDebInfo
  suites, lint or redaction. I relied on the raw g2 logs, which I read directly.
- I did not audit all 2162 added lines of `bootstrap.t.cpp` or the 413 of `external_registration.t.cpp` row by row
  against the addendum's E and O tables. I spot-checked the public-surface, message and inventory properties above. The
  Linux Docker probe, the negative-compile probes and the catch-all diff were not re-run in this lineage, as the
  record states.
- My live-journal reproduction uses Python's `sqlite3` module as the competing writer. I did not stage a Planar load that
  holds its write transaction for more than 1 s. The refusing code path (`open_read` at `storage.cpp:1170`) is the same
  whichever process owns the journal.
