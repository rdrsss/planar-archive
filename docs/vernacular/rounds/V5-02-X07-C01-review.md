# V5-02-X07-C01 independent review (whole X07 candidate: waves A and B, the R1 sidecar repair and C01)

Disposition: **approve**. The four findings below are minor, records-only and non-blocking. They can be fixed in
the commit step.

I reviewed the whole uncommitted X07 candidate against HEAD `7a81b0bc`. That covers waves A and B, the R1 sidecar
repair and this corrective round. Since the R1 review, only three files in `src tests/native` have changed:
`src/engine/ledger/storage.cpp`, `src/engine/ledger/storage.t.cpp` and `src/cmd/planar/invocation.cppm`. Every other
per-file hash equals the R1 review's identity table. So I relied on the R1 review for the unchanged parts and read the
C01 changes in full.

All seven X07-R1 findings are closed. The 1 s grace is gone, and `native::no_sidecars` and `format.cpp` are again
byte-identical to HEAD. Whether a `-journal` refuses now depends only on whether another process holds SQLite's
RESERVED byte. I checked this with the real release binary against a real competing writer. A 3.0 s or 4.5 s writer
makes `load` wait and then succeed. `history` succeeds at once. A genuine hot journal left by a SIGKILLed writer, or a
plain leftover journal, still refuses at once with the same bytes, and the journal is left untouched.

What I checked and found sound:

- **Candidate identity.** I ran identity.py from the repository root on `src tests/native`
  (`tests/native/v5-02-x07-c01-evidence/review/identity.txt`). HEAD, the tracked hash
  `ebe1f392f94739f782ad9fd3c70b1989f72953d0772a945123323d2bf26e5e62`, the combined hash
  `76698bd5421463aa69c2fc1819196576ea6950dbe486b84eff567af426982500` and all 30 per-file hashes equal the round
  record's "Final candidate identity" table, the gate (`raw/h01-identity.log`) and the prober
  (`probes/raw/00-identity.log`). The nine candidate files outside that pathspec (`docs/cli-reference.md`,
  `docs/architecture.md`, `docs/testing.md` and the six `tests/acceptance/*.py`) match the record's sha256 values.
- **R1, no grace and HEAD behaviour in format.cpp.** `git diff HEAD` is empty for `format.cpp`, `sqlite.cpp`,
  `acceptance_seam.cpp` and every `.cppm` under `src/engine/ledger`. The diff of `storage.cpp` against HEAD leaves
  `native::no_sidecars` (`:235-244`) unchanged. It adds three helpers in an anonymous namespace (`sidecar_present`,
  `reserved_probe`, `no_orphan_sidecars`, `:1132-1228`), swaps the two `open_read` checks to the liveness-aware helper
  (`:1255`, `:1279`), deletes the pre-BEGIN check and keeps the plain post-lock check (`:1469`). No `sleep_for`,
  `this_thread` or retry loop remains in `storage.cpp` or `format.cpp`. So the four `format.cpp` call sites (`:235`,
  `:282`, `:536`, `:555`) behave as on HEAD.
- **R1, the decision depends on the lock.** The probe is a non-blocking `F_SETLK F_RDLCK` on `PENDING_BYTE+1`. The
  vendored SQLite defines `RESERVED_BYTE` as `PENDING_BYTE+1` (`vendor/sqlite/b9c7/sqlite3.c:16727-16731`), and a
  writer in RESERVED or above holds a write lock on that byte. A failure with EAGAIN or EACCES therefore means a live
  writer, and any other errno is the shared `io` failure. The journal is checked while the probe holds the byte, so a
  writer cannot create or delete its journal in between. `-wal` and `-shm` refuse whether or not a writer is live. That
  matches HEAD's refusal set, since the ledger is `journal_mode=DELETE` and a live Planar writer never creates them. On
  macOS APFS, SQLite's autolock finder falls through to POSIX locks (`sqlite3.c:46174-46218`), so the byte layout
  applies there too.
- **R1, contract.** Lock waiting is only SQLite's 5000 ms busy handler (`connection::open` defaults to `busy_ms = 5000`,
  `sqlite.cppm:57`), with the existing typed `busy` mapping, which satisfies `ledger-contract.md:166`. Planar never asks
  SQLite to roll back a journal. A genuine leftover is refused before any SQLite connection opens. The leftover
  journal's bytes are unchanged in the storage test and in my CLI run. The residual windows are recorded in the "Option
  A" limit and D10: a writer that dies after being excused as live can have its journal rolled back by SQLite. That is
  within `ledger-contract.md:493-497`, and format2 inherits those recovery rules exactly
  (`external-profile-contract.md:571`). HEAD had the same kind of stat-then-open window. The HALT condition was not
  triggered.
- **R1 at CLI level (this closes the record's open C01-I30).** I copied the R1 reviewer's scripts unchanged and ran them
  against the current `build/rewrite-release/bin/planar`. I first confirmed `ninja: no work to do` in both trees. Each
  run used a scratch `HOME` and `PLANAR_DB`, with a Python `sqlite3` writer holding `BEGIN IMMEDIATE` plus a page write:

  | writer holds journal | `load` | `history` |
  |---|---|---|
  | 0.3 s | exit 0 after 0.39 s | exit 0 after 0.01 s |
  | 3.0 s | exit 0 after 3.06 s | exit 0 after 0.01 s |
  | 4.5 s | exit 0 after 4.63 s | exit 0 after 0.01 s |

  No run reported `recovery_required` (`review/live-journal.log`, `review/live-journal-history.log`). The first
  history attempts in `live-journal.log` failed with a Python usage error because I passed the wrong number of
  arguments. I kept that log and re-ran history correctly in its own log. Under R1, the 3.0 s and 4.5 s rows were exit
  4 after about 1.36 s.
- **R1, genuine leftovers still refuse.** `review/leftover-journal.py` covers two cases. In the first, a writer process
  runs `BEGIN IMMEDIATE` plus a page write and is then SIGKILLed, leaving an 8720-byte hot journal with no lock
  holder. In the second, the leftover is a one-byte plain journal. In both, `history` and `load` exit 4 within 0.02 s
  with exactly
  `{"error":{"code":"io","details":{"operation":"ledger"},"field":"","message":"Explicit ledger recovery is required"},"kind":"error","version":1}`.
  Those are the same bytes the R1 review captured, and the journal sha256 is unchanged (`review/leftover-journal.log`).
- **R2, a real concurrent writer that can fail.** The kept name `storage begin_write waits out a transient live writer
  journal` now forks a child that uses the raw SQLite C API. The child holds `BEGIN IMMEDIATE` plus `CREATE TABLE` for
  2.5 s, and the parent confirms the journal exists before measuring. The test asserts success, an elapsed time from
  1.5 s up to 5 s, a usable `read_head()` and a clean child exit. No `std::thread` or remover thread remains. The read
  path is covered by `storage live writer journal does not refuse the read path` and `storage writable open tolerates
  a live writer journal`.

  I checked the red evidence independently. I reconstructed the red snapshot in a scratch clone: HEAD, then `git apply
  red/snapshot/candidate.diff`, then the saved untracked files. Running identity.py on it gives a tracked hash equal to
  RED.md's `candidate.diff` sha256 `6f7898d8...4920`, and `storage.cpp` `5a521ee1...3afd`, which is exactly the
  grace-era file the R1 review hashed (`review/red-snapshot-identity.txt`). Against the final checkout, the snapshot
  differs only in `storage.cpp` (the fix), `invocation.cppm` (the R5 comment) and `storage.t.cpp`, where the final file
  only adds the two repair tests and one helper class. So the red-time test bodies are the final ones. The red logs
  fail on assertions, not on compilation: `REQUIRE( tx )` at :372 (write path), `REQUIRE( snap )` at :472 (read path),
  and timing checks at :429 and :547.
- **R6.** `storage begin_write surfaces a non-ENOENT sidecar stat failure` asserts `format_error_code::io` and exactly
  "Ledger storage operation failed", and has the NAME_MAX comment. A new read-path variant pins the same thing for
  `open_read` and for the native helper.
- **R3, plans and probes.** DECISIONS.md D1 to D20 cover every finding. The round record's invariant table has 34 rows
  and covers R1 to R7 and the Dispatch 2 criteria. I read `superseded-1/raw/mutants.json`. Every mutant is a real edit
  that breaks its row: the grace re-added, the live branch dropped, a plain check at `:1255` or `:1279`, a pre-BEGIN
  check reinserted, the post-lock check removed, the probe moved to a SHARED byte, F_WRLCK or a widened GETLK, the
  unlock omitted, errno mappings, and message periods. C01-I15c really patches both stat sites through its `extra`
  edit. In the three re-probe logs I read, each kill names its test and ends with a restore whose sha256 matches. M09,
  M10 and M12 are killed (by C01-I13, C01-I07 and C01-I14), not merely recorded as equivalent: the pre-BEGIN check was
  removed. X1 is correctly recorded as equivalent in the X07 record (`envelope.cpp:49-50` refuses `count() != 1`, and
  `Option::count()` is `results_.size()` at `vendor/cli11/9d6a/include/CLI/Option.hpp:388`). As a behaviour check,
  `with-profile --stdlib-profile a.json --stdlib-profile b.json history` exits 2 with the fixed usage bytes.
- **R4.** The `docs/cli-reference.md` synopsis is `planar with-profile --stdlib-profile FILE COMMAND [OPTIONS]`. Both
  selected-help orders are listed, matching `external-profile-contract.md:45-47`. The release binary parses all three
  forms; with the profile file absent, each reaches profile capture (exit 4, "Cannot open confined input"). The old
  documented form is still usage exit 2 (`review/cli-syntax.log`).
- **R5.** The `@note` at `invocation.cppm:56-58` now names the early lookup, the factory, `prepare_load` and
  `complete_acceptance`, plus the four `X07-ANCHOR` comments that exist in `invocation.cpp:507-523`. It also notes the
  completion-uncertain fallback after commit (`if (committed()) return fallback;`). The note is accurate.
- **R7.** I re-ran the D20 compared-set check: 43 paths, all found in `V5-02-X07.md`, 0 missing. The record also
  states that `sqlite.cpp` was not touched.
- **CAS stability.** I ran `x07.two_process_cas_one_winner` serially, one `ctest` per run: 25 of 25 passed in
  `build/rewrite-release` and 20 of 20 in `build/rewrite-debug` (`review/cas-release-25.log`,
  `review/cas-debug-20.log`). This closes the record's note that the Debug loop was not logged.
- **New-test stability.** I ran the 29 storage and sidecar tests (all `replacement.storage ` tests plus the four carried
  format and publication sidecar tests) three times in each configuration. All 29 passed every time
  (`review/storage-{release,debug}-{1,2,3}.log`).
- **Inventory.** `ctest -N` gives 1227 names in each tree, and the two sorted lists are byte-identical. All 1114
  X07-DOC baseline names and all 1213 R1 names are present. Exactly the 14 storage names listed in the record are new
  (`review/inventory-check.txt`). No test was renamed or removed. The two edited bodies kept their names, and the
  plain-file body was replaced as R2 required.
- **Diagnostics and public surface.** C01 adds no message. The helper's constant equals the native literal byte for
  byte, and mutants E01, I03b, I11 and M07 kill any drift. The only removed message line in `git diff HEAD -- src`,
  "Cannot obtain invocation cwd", is a wave-A move that the R1 review accepted; it still exists in `invocation.cpp` and
  `response.cpp`. C01 changes no `.cppm` except comment lines in `invocation.cppm`, and adds no exported symbol.
- **Gate logs.** I read the second gate run's raw logs rather than the summary. Every `h*.exit` is 0. `h04` and `h07`
  each show "100% tests passed out of 1227". Their only matches for skip-like words are a test name containing
  "skipped". Lint checked 147 files after one recorded Doxygen SIGBUS retry. `h12` and `h13` are both "no work to do".
  The redaction command named all 45 files and produced no output.

## Exact candidate

The candidate is in the shared checkout on `dev/planar-v2-vernacular` and is uncommitted. Output of identity.py
(`review/identity.txt`), pathspec `src tests/native`:

| identity | value |
|---|---|
| HEAD | `7a81b0bcb8c85c3d8f75a6b9ea1dc9a1c66f19a4` |
| sha256(git diff --binary HEAD -- src tests/native), tracked only | `ebe1f392f94739f782ad9fd3c70b1989f72953d0772a945123323d2bf26e5e62` |
| combined sha256, pathspec src tests/native, evidence excluded | `76698bd5421463aa69c2fc1819196576ea6950dbe486b84eff567af426982500` |

For example, `src/engine/ledger/storage.cpp` is `1f3b3a68117a472952500d934d3895830daffa1eb10277120a6cc582fe9fee92`,
`src/engine/ledger/storage.t.cpp` is `c97c8a29b13437353cef8edb33c35cb1c1fad77af4639eb342ff654f46727f36` and
`src/cmd/planar/invocation.cppm` is `1059db8e2f4b9fea534e48fc2d14d105fb472f002e73e8a9c94a44dca532553a`.

## Findings

### X07-C01-R1 (minor, records): RED.md does not quote identity.py output

The dispatch and row C01-I18 ask for the identity.py hash of the pre-fix snapshot, and C01-I18 also asks for the
storage.cpp sha256. RED.md quotes the sha256 of `candidate.diff` and of `SHA256SUMS`, but no identity.py table and no
`storage.cpp` hash. The saved snapshot also leaves out `tests/native/v5-02-x07-support/generate.py`, although the
RED.md header says it covers the untracked support files. That file is byte-identical in R1 and C01 (`66ee281d...`), so
the snapshot can still be rebuilt.

I rebuilt it and confirmed the evidence is sound. identity.py's tracked hash equals the `candidate.diff` hash, and the
combined hash is `b7d55b830de7327b1330d7469e343b5880c94f4ee9492efc5d2cddf4beb58b08`
(`review/red-snapshot-identity.txt`). Fix: add that identity.py output and the `storage.cpp` sha256 (`5a521ee1...`) to
RED.md, or point to the review file.

### X07-C01-R2 (minor, records): stale plan and probe files

Row C01-I21 in INVARIANTS.md still says 12 new names and 1225 tests, and the file has no rows for C01-I33 or C01-I34.
PROBES.md lines 25-64 carry the revision-1 identity, where `storage.t.cpp` is `3e0240a4...`, and its "Skipped" section
still says the C01-I31 grep "fails". The round record states all of this and says the record governs. Its table is
complete, and both C01-I31 greps now match twice each. Fix: sync INVARIANTS.md (I21, I33, I34) and add a one-line
superseded note to the stale PROBES.md parts.

### X07-C01-R3 (minor, records): two unrecorded side effects of the RESERVED probe

1. SQLite checks for a RESERVED holder with `F_GETLK` and `F_WRLCK` on `RESERVED_BYTE` (`sqlite3.c:41894-41904`).
   That check reports a read lock as a holder. So while the probe holds its byte, which takes microseconds, another
   process's SQLite connection treats any `-journal` as live rather than hot. D5 records only the opposite effect, a
   spurious BUSY for a writer.
2. The "unlock is safe" argument (D11 and the `reserved_probe` comment) depends on the caller: this process must hold
   no RESERVED lock on that inode. Under POSIX lock-replacement rules, a probe on a ledger where this process already
   holds a write transaction would downgrade and then release that process's own RESERVED lock.

I traced every product caller. `format.cpp:90/99/125/268/330` and the X07 load path in `invocation.cpp:489-505` release
the read before `complete_acceptance`. None of them reaches the probe while holding a write transaction on the same
ledger. So this is not a defect today. Fix: add both points to D5, D10 and D11, or to the round record's limits, so a
later caller does not break the precondition without noticing.

### X07-C01-R4 (minor, records): close the record's open items at commit time

The round record lists two gaps as open: C01-I30, the CLI-level live-journal run, and the missing Debug CAS log. This
review supplies both: `review/live-journal*.log`, `review/leftover-journal.log` and `review/cas-debug-20.log`. The
record can cite them.

This review file, `docs/vernacular/rounds/V5-02-X07-C01-review.md`, is a new untracked path in the D20 compared set.
Add it to the X07 record's "Added by C01 and the review (R7)" list before the commit so the R7 check stays at zero
missing.

## Invariant table, red and probes

- **Invariant table.** The record's 34 rows map to findings as follows:
  - R1: rows I01 to I09, I12, I13, I26, I27 and I29.
  - R2: rows I04, I06, I17 and I18.
  - R3: rows I07, I13, I14 and I25.
  - R4: row I22.
  - R5: row I23.
  - R6: rows I15 and I33.
  - R7: row I24.
  - The Dispatch 2 criteria: rows I16, I19, I20 and I21.
  - I28 and I34 cover the probe's unlock.

  Every test-bearing row names a test that its mutant's log shows failing. The gate-only rows are backed by checks I
  repeated: I17, I21, I22, I23, I24, I26 and I31.
- **Red.** RED.md has eight red rows (I01, I03, I04/I27, I06/I07, I08, I13, I29), each failing on an assertion against
  a pre-fix snapshot whose identity I confirmed. The 12 rows marked expectRed=false passed as guards. The two repair
  tests (I33, I34) have no red rows. They guard code that was already correct, and their evidence is a kill in which
  they are the only failing test, which the record states.
- **Probes.** I read `mutants.json` in full. For the three re-probed rows I read the raw logs. For the carried rows I
  checked the PROBES.md table against the logs it cites. In total, 38 distinct mutants were killed and none survived.
  M14 and X2 to X4 are carried over because their files are byte-identical to the R1 probe. X1 is equivalent.

## Commands I ran (from the repository root, unpiped; logs under `tests/native/v5-02-x07-c01-evidence/review/`)

| Command | Exit | Result |
| --- | --- | --- |
| `python3 <workflow>/scripts/identity.py src tests/native` | 0 | equals the record (`identity.txt`) |
| identity.py in a scratch clone at HEAD plus `red/snapshot/` | 0 | tracked hash equals `candidate.diff` (`red-snapshot-identity.txt`) |
| `cmake --build build/rewrite-release`, `--target planar_tests` in both trees, `cmake --build build/rewrite-debug` | 0 | `ninja: no work to do` each time |
| `python3 -I review/live-journal-load.py <release planar> <repo> <scratch> H`, H = 0.3, 3.0, 4.5 | 0 | exit 0 each (`live-journal.log`) |
| `python3 -I review/live-journal-history.py <release planar> <scratch> H`, H = 0.3, 3.0, 4.5 | 0 | exit 0 each (`live-journal-history.log`) |
| `python3 -I review/leftover-journal.py <release planar> <repo> <scratch>` | 0 | exit 4 with the exact recovery bytes, journal unchanged (`leftover-journal.log`) |
| release binary with the documented, help and old envelope forms; duplicate selector | 4, 4, 4, 2; 2 | `cli-syntax.log`; duplicate selector is usage |
| 25 x `ctest --test-dir build/rewrite-release --no-tests=error -R '^x07\.two_process_cas_one_winner$' --output-on-failure` | 0 each | 25 of 25 (`cas-release-25.log`) |
| 20 x the same in `build/rewrite-debug` | 0 each | 20 of 20 (`cas-debug-20.log`) |
| 3 x per tree `ctest ... -R '^replacement\.(storage \|<four carried sidecar tests>)'` | 0 each | 29 of 29 each run (`storage-*.log`) |
| `ctest -N` in both trees, compared against the 1114 baseline and the 1213 R1 names | 0 | 1227 identical, 0 removed, 14 new (`inventory-check.txt`, `names-*.txt`) |
| D20 compared-set check against `V5-02-X07.md` | 0 | 43 of 43 found |

## Limits

- I applied no mutants; the workflow leaves that to the prober. I did not re-run the full Debug or RelWithDebInfo
  suites, lint or redaction. For those I read the second gate run's raw logs.
- Linux was not run. The probe depends on SQLite's POSIX lock layout, which the unix VFS shares on Linux, and D9 records
  the filesystems where SQLite uses no POSIX locks. The non-gating Docker probe is the coordinator's next step, and X08
  carries the real Linux gate.
- The residual windows of the "Option A" limit and D10 are races I did not reproduce. I accepted them on the contract
  text and on the comparison with HEAD. The operator decision cited for Option A is recorded only in the round
  records. I could not trace it independently, but the behaviour it allows is within `ledger-contract.md:493-497` in
  any case.
- I did not re-audit the wave A and B files, which are unchanged since the R1 review, beyond their identity hashes.
  The R1 review's limits for those files still apply.
