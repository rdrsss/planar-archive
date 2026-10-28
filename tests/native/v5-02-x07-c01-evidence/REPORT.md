# V5-02-X07-C01 gate report (second gate run)

Gate runner output only; no source file was edited. This run supersedes the earlier gate report (17:12 to 17:30), which is kept at
`raw/prev-gate-REPORT.md` with its logs `raw/g*`; nothing was deleted. The new logs are `raw/h01` to `raw/h14` (`*.cmd`, `*.exit`, `*.log`),
driver `raw/run-gate2.sh`, file list `raw/h14-files.txt`. Nothing was piped through tail or head; exit codes were written directly.
The earlier gate ran before `src/engine/ledger/storage.t.cpp` was last edited (17:52), so its identity and counts are stale.

## Candidate identity

Output of `identity.py src tests/native docs` (raw/h01-identity.log, per-file hashes there):

- HEAD `7a81b0bcb8c85c3d8f75a6b9ea1dc9a1c66f19a4`
- sha256 of `git diff --binary HEAD -- src tests/native docs`, tracked only: `75f5a2929a621939c6b82884dde5e54e38614fa393534901db11f9ea6104302f`
- combined sha256 (diff bytes plus each untracked path and bytes, evidence excluded): `c729d8e2b52bb5ec1dee689d79e232b5973310e6527ecd601573df6ea46334af`
- `git diff --quiet HEAD -- src/engine/ledger/format.cpp` exits 0 (format.cpp has no diff).

## Commands and results

| # | command | exit | result |
|---|---|---|---|
| 1 | `cmake --build build/rewrite-debug --target planar_tests` | 0 | `ninja: no work to do.` (raw/h02) |
| 2 | `ctest --test-dir build/rewrite-debug --no-tests=error -R 'x07\|x06' --output-on-failure` | 0 | 100% passed, 19 of 19, 0 failed, 0 skipped (raw/h03) |
| 3 | `ctest --test-dir build/rewrite-debug --no-tests=error --output-on-failure` | 0 | 100% passed, 1227 of 1227, 0 failed, 0 skipped, 636.45 s (raw/h04) |
| 4 | `cmake --preset release` | 0 | configured (raw/h05) |
| 5 | `cmake --build build/rewrite-release --target planar_tests` | 0 | built, 635 steps, last `[634/635]` (raw/h06) |
| 6 | `ctest --test-dir build/rewrite-release --no-tests=error --output-on-failure` | 0 | 100% passed, 1227 of 1227, 0 failed, 0 skipped, 238.19 s (raw/h07) |
| 7 | 20 x `ctest ... -R '^x07\.two_process_cas_one_winner$'` in build/rewrite-release | 0 | 20 of 20 runs printed "100% tests passed", no "fail at" line (raw/h08-cas20.log) |
| 8 | `ctest --test-dir build/rewrite-debug -N` | 0 | Total Tests: 1227 (raw/h09) |
| 9 | `ctest --test-dir build/rewrite-release -N` | 0 | Total Tests: 1227 (raw/h10) |
| 10 | `python3 scripts/rewrite-check.py lint --build-dir build/rewrite-debug` | 0 | clang-format clean; "Formatting and Doxygen checked 147 active source files". Doxygen died from a signal on attempt 1 (the known 1.18.0 SIGBUS flake documented in docs/toolchain-parity.md) and succeeded on attempt 2 of 3 (raw/h11) |
| 11 | `cmake --build build/rewrite-debug` (first) | 0 | `ninja: no work to do.` (raw/h12) |
| 12 | `cmake --build build/rewrite-debug` (second) | 0 | `ninja: no work to do.`, zero steps (raw/h13) |
| 13 | `python3 scripts/redact-local-paths.py check --repo-path "$PWD" --home "$HOME" <45 files>` | 0 | no output, clean (raw/h14; list raw/h14-files.txt = tracked diff plus untracked files, gitignored evidence excluded) |

The focus filter matched 19 tests; zero would have been a failure. No skips appear in any log. Every command met its criterion; passed is true.

Zero-work result: the Debug `planar_tests` build (command 1) already had no work, and `all` was a no-op twice. The no-change check is therefore valid but did
not follow a build that did work in Debug; the Release build (command 5) did work in a different directory.

## Inventory

1227 registered tests in Debug and in RelWithDebInfo; the sorted name lists are byte-identical (raw/names-debug2.txt, raw/names-release2.txt; `cmp` equal).
The Debug `ctest -N` listing is saved as `inventory.txt`; sorted names are in raw/h-names.txt.

Compared against `tests/native/v5-02-x07-r1-evidence/inventory-names.txt` (1213 names, the latest list on disk, gitignored evidence; the 1114-name baseline
is a subset, no separate file exists): removed or renamed 0 (raw/h-removed.txt empty). New: exactly 14 (raw/h-new-vs-prev13.txt), 1213 + 14 = 1227, each with
the `replacement.` prefix:

1. `storage begin_write refuses a sidecar that appears while waiting for the writer lock`
2. `storage killed writer leaves a genuine hot journal that still refuses`
3. `storage leftover journal refuses immediately on read and write paths`
4. `storage live writer does not excuse a wal or shm sidecar`
5. `storage live writer journal does not refuse the read path`
6. `storage no_sidecars ignores writer liveness`
7. `storage no_sidecars refuses a leftover sidecar without delay`
8. `storage open_read and native no_sidecars surface a non-ENOENT sidecar stat failure`
9. `storage probe fcntl failure surfaces as io`
10. `storage probe lock is gone before the gated open reads`
11. `storage probe releases the writer lock on the no-journal path`
12. `storage SHARED-only reader does not excuse a leftover journal`
13. `storage writable open refuses a sidecar created after the read check`
14. `storage writable open tolerates a live writer journal`

Items 8 and 10 are new since the earlier gate (which counted 12 and 1225). plan/INVARIANTS.md I21 still says 12 new names and 1225; it is stale.
The existing `storage begin_write surfaces a non-ENOENT sidecar stat failure` name is still present (not renamed).

## (a) Acceptance criteria and findings to tests

All storage tests are in `src/engine/ledger/storage.t.cpp`; lines are the current TEST_CASE lines.

| criterion / finding | test names and file:line | evidence |
|---|---|---|
| AC1 / R1: no grace; format.cpp identical to HEAD | `storage no_sidecars refuses a leftover sidecar without delay` :476; `storage leftover journal refuses immediately on read and write paths` :508; carried `ledger format initialization refuses every existing destination sidecar` format.t.cpp:392; `ledger format sidecars require recovery without writable fallback` format.t.cpp:328; `ledger publication late sidecar refuses and preserves arriving sidecar` publication_fault.t.cpp:75; `ledger complete accepted history survives explicit quiescent hot journal recovery` history_verification.t.cpp:547 | format.cpp no diff vs HEAD (exit 0 above); all pass in both configurations |
| AC2 / R1, R2: real concurrent writer, write and read path | write `storage begin_write waits out a transient live writer journal` :404 (forked child holds BEGIN IMMEDIATE about 2.5 s); read `storage live writer journal does not refuse the read path` :527; writable transition `storage writable open tolerates a live writer journal` :553; mid-wait `storage begin_write refuses a sidecar that appears while waiting for the writer lock` :596 | red-first in (b) |
| AC3 / R1, R6: leftover refusals unchanged; non-ENOENT test asserts `io` and exact message | `storage begin_write still refuses a persistent journal` :420; `storage begin_write sidecar refusal message is byte exact` :431; `storage killed writer leaves a genuine hot journal that still refuses` :564; `storage live writer does not excuse a wal or shm sidecar` :580; `storage SHARED-only reader does not excuse a leftover journal` :538; non-ENOENT: `storage begin_write surfaces a non-ENOENT sidecar stat failure` :443 and new `storage open_read and native no_sidecars surface a non-ENOENT sidecar stat failure` :456 (both assert `io` and "Ledger storage operation failed"; the :456 test covers open_read read_only and read_write and native `no_sidecars`, with a NAME_MAX comment) | pass in both configurations |
| R3 survivors M09, M10, M12 | M09 `begin_write refuses a sidecar that appears while waiting for the writer lock` :596; M10 (pre-BEGIN check removed) killed by `begin_write waits out a transient live writer journal` :404; M12 `writable open refuses a sidecar created after the read check` :614; probe lock `storage probe releases the writer lock on the no-journal path` :626, `storage probe lock is gone before the gated open reads` :656, `storage probe fcntl failure surfaces as io` :666 | tables in plan/INVARIANTS.md and DECISIONS.md. Mutant kills were not verified by this gate; probes/PROBES.md holds the prober's results |
| AC4: CAS test 20 consecutive runs | `x07.two_process_cas_one_winner` (tests/acceptance/x07_replay.py:107, registered src/cmd/planar/CMakeLists.txt:232) | 20 of 20 in Release (raw/h08); passed in both full runs. A Debug 20-run loop was not in the assigned commands and was not run |
| AC5 / R3: invariant and decision tables, X1 equivalent | plan/INVARIANTS.md (C01-I01 to I32), plan/DECISIONS.md | tables exist; X1 recorded in docs/vernacular/rounds/V5-02-X07.md (earlier gate cited :309 and :377, not re-checked) |
| AC6 / R4 docs | doc check | docs/cli-reference.md:263 reads `planar with-profile --stdlib-profile FILE COMMAND [OPTIONS]`; :264-265 give both help orders |
| AC6 / R5 stale note | comment check | "later wave" has no match in src/cmd/planar/invocation.cppm; lint passes |
| AC6 / R7 owned-file list | record check | not re-checked by this gate beyond the earlier report (lines 72, 73, 227 to 232 of V5-02-X07.md) |
| AC7: names preserved, identical in both configs, diagnostics byte-identical, no new public surface | inventory above; `X06 D runtime public surface and resource messages stay closed` acceptance.t.cpp:1161 | 0 removed, 14 new, identical configs; that test passes |

## (b) Red-first

Source: `red/RED.md` and `red/raw/NN.log` (coders). I did not re-run any red check. Pre-fix snapshot: HEAD 7a81b0bc plus the candidate with the 1 s grace.
Snapshot hashes (RED.md): `candidate.diff` sha256 `6f7898d814564c5f365ba9375f653503008d4e56139e0a75459ffeb2fe4e4920`; `red/snapshot/SHA256SUMS` sha256
`18e0e6070ca002913c9b1e7ce11ae375528b6394120cf9ebd376c98c4e91bbb0`. Command per test, from RED.md: `ctest --test-dir build/rewrite-debug -R '^replacement\.<escaped name>$' --no-tests=error --output-on-failure`
after `cmake --build build/rewrite-debug --target planar_tests`. Failing lines are as at red time (the test file was edited afterwards; current lines are in (a)).

| test | log | outcome | failing line (red time) |
|---|---|---|---|
| `storage no_sidecars refuses a leftover sidecar without delay` | 01 | real red | storage.t.cpp:429 |
| `storage leftover journal refuses immediately on read and write paths` | 03 | real red | :457 |
| `storage live writer journal does not refuse the read path` | 04 | real red | :472 |
| `storage begin_write waits out a transient live writer journal` | 06 | real red | :372 |
| `storage writable open tolerates a live writer journal` | 07 | real red | :498 |
| `storage begin_write refuses a sidecar that appears while waiting for the writer lock` | 12 | real red | :547 |
| `storage probe fcntl failure surfaces as io` | 19 | real red | :600 |
| `storage no_sidecars ignores writer liveness` | 02 | guard, already passed | none |
| `storage SHARED-only reader does not excuse a leftover journal` | 05 | guard, passed | none |
| `storage killed writer leaves a genuine hot journal that still refuses` | 08 | guard, passed | none |
| `storage begin_write still refuses a persistent journal` | 09 | carried guard, passed | none |
| `storage begin_write sidecar refusal message is byte exact` | 10 | carried guard, passed | none |
| `storage live writer does not excuse a wal or shm sidecar` | 11 | guard, passed | none |
| `storage writable open refuses a sidecar created after the read check` | 13 | guard, passed | none |
| `storage begin_write surfaces a non-ENOENT sidecar stat failure` | 14 | strengthened existing, guard, passed | none |
| `x07.two_process_cas_one_winner` | 15 | guard, passed | none |
| `X06 D runtime public surface and resource messages stay closed` | 16 | guard, passed | none |
| `ledger format initialization refuses every existing destination sidecar` | 17 | guard, passed | none |
| `storage probe releases the writer lock on the no-journal path` | 18 | guard, passed | none |
| `storage open_read and native no_sidecars surface a non-ENOENT sidecar stat failure` | none | MISSING | RED.md has no row and red/raw has no log for it |
| `storage probe lock is gone before the gated open reads` | none | MISSING | RED.md has no row and red/raw has no log for it |

Missing: red or guard evidence for the two tests added after RED.md was written (17:01); they were added at or before the 17:52 edit of storage.t.cpp, and
neither name appears in RED.md, plan/ or probes/PROBES.md. RED.md names `candidate.diff` and `SHA256SUMS` hashes, not an `identity.py` combined hash. No
red evidence exists for gate-check rows (I17, I18, I21 to I25, I30 to I32), which are not ctest tests.

## (c) Deliberate behaviour differences

- A plain `-journal` is excused when another process holds RESERVED or higher (live writer), where HEAD refuses on mere existence. Permitted by
  `ledger-contract.md` lines 485 to 498 (hot-journal rules) and the busy wait at line 166; leftover refusal with no lock holder is unchanged and byte-identical
  (plan/DECISIONS.md D3, D4, D5).
- `begin_write` no longer checks sidecars before BEGIN IMMEDIATE; it checks once under this connection's own lock (D3, I07).
- Recorded limits: D3 Option A, D9 (filesystems without POSIX locks), D10 (residual windows). The round record V5-02-X07-C01.md does not contain
  `SQLITE_READONLY_ROLLBACK` (grep count 0), so record item I31 is still open for the coordinator.

## Limitations

- Not run by this gate: the CLI-level live-journal load and history scripts (D18, I30), the mutation prober, a Debug 20-run CAS loop, the Docker Linux probe.
- The 1114-name baseline list is not a separate file; preservation was checked against the 1213-name list of the previous gate.
- INVARIANTS.md I21 (12 new names, 1225) is stale against the 14 new names and 1227 total.
- The Debug planar_tests build did no work, so the no-change check does not follow a Debug build that did work.
- Lint needed one Doxygen retry (known upstream flake).
- Redaction check was run with explicit files only.
