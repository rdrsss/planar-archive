# V5-02-RA01-C-C02 gate report (gate runner)

The gate runner edited no source file. It wrote only under `tests/native/v5-02-ra01-c-c02-evidence/`: this report,
`inventory.txt` and new `raw/gate-*.log` files. The coders' earlier report is preserved unchanged as `raw/coder-REPORT.md`.
This round is evidence-only, so the gate is the source-identity check, the redaction checks with a positive control, and lint.
No build or full ctest was run, because the source tree is byte-identical to the frozen C01 candidate (the round record
requires a full code gate only if a src byte differs).

## Candidate identity

Output of the workflow `identity.py src`, run from the repository root (raw/gate-identity-src.log, exit 0):

| identity | value |
|---|---|
| HEAD | `93053c42a4c68fd3ecc8911edab6464e3d1307f3` |
| sha256(git diff --binary HEAD -- src), tracked only | `6b23117bfb40136186fabf5515d97e0ac1da99c0b69b6d00584033cab0f4aeda` |
| combined sha256 (diff bytes, then each untracked path NUL bytes, sorted), pathspec src | `6b23117bfb40136186fabf5515d97e0ac1da99c0b69b6d00584033cab0f4aeda` |

| file | state | sha256 |
|---|---|---|
| `src/cmd/planar/authoring_orchestrator.t.cpp` | modified | `38d79424ab104ac3be416588bcc58d33715976fd781a6128e173b51c6a06a46f` |
| `src/stdlib/planar/orchestrator.lua` | modified | `e7d9f570ef70b014e6b3facac55709c59ea8736958bb5ef7d5147eb227bc87a1` |

All three values equal the round record's frozen values. `git diff HEAD -- src | shasum -a 256` is the same
`6b23117b...4aeda`. sha256 of `git diff HEAD` over the whole tracked tree at gate time (includes record and evidence edits):
`3de22b96ad88a37c223521d13a1e3f0fb3e8e4e76f8622268e6b9c9c87490f50`.

## Commands

| # | Command | Exit | Result |
|---|---|---|---|
| 1 | `python3 <workflow>/scripts/identity.py src` | 0 | three frozen hashes match (raw/gate-identity-src.log) |
| 2 | `python3 scripts/redact-local-paths.py --repo-path <repo> --home $HOME check <one argument per committable file>` | 0 | no output, no leak. Files: the 11 files from `git ls-files -m -o --exclude-standard` (excluding build/), listed in raw/gate-committable.txt, plus the 42-file list and 37-file staged list of the coders (raw/c02-redact-files.txt, c02-redact-staged.txt) re-run through the coders' wrapper (see below) (raw/gate-redact-check.log) |
| 3 | same script, `check` on a scratch file outside the repository containing the repository path | 1 | printed "local path in <scratch>/ctl.txt", as required (raw/gate-redact-control.log) |
| 4 | `python3 scripts/rewrite-check.py lint --build-dir build/rewrite-debug` | 0 | "Formatting and Doxygen checked 171 active source files" (raw/gate-lint.log) |
| 5 | `ctest --test-dir build/rewrite-debug -N` | 0 | Total Tests: 1439 (raw/gate-ctest-N.log). Listing only, no test was executed |
| 6 | `ninja -C build/rewrite-debug -n` (dry run) | 0 | see Zero work |

Matched test counts and skips: none were executed by this gate, so there are no failures and no skips to report. The
registered count of 1439 equals the 1439 passed, zero skipped, zero failed of the C01 gate run 2 (macOS Debug and
RelWithDebInfo and Linux), whose source is byte-identical to this candidate.

Redaction: raw/gate-redact-check.log is 9 bytes (the heading "final" only) and records no command, file count or exit code; treat it as unrecorded. The recorded checks are raw/c02-redact-check.log and review/redaction-check.txt.

## Test inventory

`inventory.txt`: 1439 names from `ctest -N` of build/rewrite-debug, sorted in the C locale. Compared with the most recent
committed inventory, `tests/native/v5-02-ra01-c-c01-evidence/inventory.txt` (1439 names; the committed RA01-C list is
`tests/native/v5-02-ra01-c-evidence/inventory.txt`): `diff` reports no difference. New names: none. Removed names: none.
(C01 itself added two names to the 1437 of RA01-C: the two `RA01-C-C01 orchestrator ...` tests.)

## Zero work

`ninja -C build/rewrite-debug -n` reported two steps, "Re-checking globbed directories" and "Re-running CMake" (raw/gate-ninja-n.log).
This is not a valid no-change check. It is a dry run, and no build was run in this gate. The round requires a no-work build
check only as part of a code gate, which is not triggered. The last valid zero-work results are the C01 gate run 2 logs
(`ninja: no work to do.` twice per configuration). The CMake re-run on this dry run is unexplained by this gate (likely a
touched glob input or an older generate). It is reported as a limitation, not claimed as a pass.

## (a) Acceptance criteria and findings to test names

This round changes no source or test. Its criteria are evidence and record properties. The findings it closes concern the
C01 tests, which are unchanged.

| Criterion or finding | Test or check, file:line | Result |
|---|---|---|
| Acceptance: no src byte differs from the C01 frozen candidate | identity.py hashes above | met |
| Acceptance: redaction check exits 0 on every committable file, positive control exits 1 | commands 2 and 3 | met |
| Acceptance: lint passes | command 4 | met |
| R1 (redaction of red-first.log, plan/DECISIONS.md, with a non-vacuous check) | not a test; coder evidence `raw/c02-redact-check.log`, `raw/c02-redact-control.log` | met (see coder-REPORT.md I2 to I6) |
| A1 (inventory-debug.txt label) | not a test; coder-REPORT.md I10 | met |
| A2 (red-time identity limit) | not a test; recorded in C01 REPORT and as limit below | recorded |
| A3 (matrix A07 and acceptance rows) | not a test; coder-REPORT.md I12, I13 | met |
| A4 (probes pointers) | not a test; coder-REPORT.md I14 | met |
| C01 finding RA01-C-F1 (projection wrapper versioned and mandatory), unchanged | `RA01-C-C01 orchestrator projection wrapper is versioned and mandatory`, src/cmd/planar/authoring_orchestrator.t.cpp:740 | registered in inventory, passed in C01 gate run 2 |
| C01 markdown bytes pin | `RA01-C-C01 orchestrator markdown bytes keep the golden title authority and joins`, authoring_orchestrator.t.cpp:898 | same |
| C01 consumer wiring guard | `RA01-B2 orchestrator prompt carries five sections in order`, authoring_orchestrator.t.cpp:420 | same |
| C01 boundary guard | `X08 C01 role preparation outputs-off boundary pins the request-record conversion charge`, src/engine/runtime/external_registration.t.cpp:4589 | same |

## (b) Red-first

No new regression test exists in this round, so there is no red-first run and none is owed. For the C01 tests (unchanged):
the red evidence is `tests/native/v5-02-ra01-c-c01-evidence/red/RED.md`, `red-first.log` and `red/raw/`. Red-time identity from
RED.md: HEAD `93053c42...`, sha256 of `red/snapshot/candidate.diff` `c2a1fda122a9895be56b24811efbd76148c2efdbe235f38050d8b0f987b09f74`,
sha256 of `red/snapshot/SHA256SUMS` `f914784422ae3b12751c8c411551300987458fd199a3014596b27d07f2a88b36`. Command from RED.md:
`ctest --test-dir build/rewrite-debug -R '<exact name>' --no-tests=error --output-on-failure`.

| Test | Outcome at red time | Failing lines | Real red or guard |
|---|---|---|---|
| `RA01-C-C01 orchestrator projection wrapper is versioned and mandatory` | failed, 28 of 79 assertions | authoring_orchestrator.t.cpp:750, 751, 752, 755, 759, 761, 765, 767, 778, 810, 861-873, 890-892 | real red |
| `RA01-C-C01 orchestrator markdown bytes keep the golden title authority and joins` | no red-first run exists (the test postdates the red snapshot) | none | missing; probed later, 9 of 9 I6 mutants killed (C01 probes/PROBES.md) |
| `RA01-B2 orchestrator prompt carries five sections in order` | passed pre-fix | none | guard that already passed |
| `X08 C01 role preparation outputs-off boundary pins the request-record conversion charge` | passed pre-fix | none | guard that already passed |

Missing: the red test file hash exists only in abbreviated form (`379d82d7...2a3d`) in the review, and the C01 red-time
identity was recorded in prose (accepted by the reviewer as non-blocking, finding A2).

## (c) Deliberate behaviour differences

None. No product or test byte changed in this round. The only behavioural change in the C01 candidate (`project()` returns
`{version=1,sections}` with `mandatory=true`) is required by authoring-contract.md:422 and was reviewed and accepted in C01.
No diagnostic message changed.

## Files edited by this gate runner

`tests/native/v5-02-ra01-c-c02-evidence/REPORT.md`, `tests/native/v5-02-ra01-c-c02-evidence/inventory.txt`, and new files
`raw/coder-REPORT.md`, `raw/gate-identity-src.log`, `raw/gate-committable.txt`, `raw/gate-redact-check.log`,
`raw/gate-redact-control.log`, `raw/gate-lint.log`, `raw/gate-ctest-N.log`, `raw/gate-ninja-n.log` in the same folder.
The round-level edited files are listed in the round record.

## Limitations

- No ctest execution and no build in this gate; passes rest on the C01 gate run 2 results for an identical source tree.
- The ninja dry run is not a no-change check (see Zero work).
- The raw logs keep absolute paths (lint trace) and are never staged.
- A2 red-time identity limit as above.
