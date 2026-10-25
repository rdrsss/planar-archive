# V5-02-X06-B3-B-C02 gate report (macOS Debug)

Gate runner output. No source file was edited by the gate runner. Raw logs and exit-code files are under `raw/gate-run/` (`NN.log`, `NN.exit`); exit codes were written to files, never read through a pipe. An earlier gate report for an older state of the tree was moved to `raw/REPORT.previous-stale.md`; it described 984 tests and is superseded by this one, because the tree changed afterwards (see "Candidate identity").

## Candidate identity

- HEAD: `0cc29923383b39508e31cbf0cb62651185963480` on `dev/planar-v2-vernacular`; tree uncommitted.
- `git diff HEAD` sha256: `2b214ec3eb1e140e38ec1511d77188fa64842b5d56e4de792083b0faa33825d9` (`raw/gate-run/diff.sha256`). The earlier coder gate recorded `848de5c1fbca...`, so the candidate is not the one that gate saw.
- Candidate file hashes (first 12), identical at start and end of this gate (`raw/gate-run/files.start.sha256`, `files.end.sha256`):
  `runtime.cpp` `ab70f5441bea` (was `1ff83396d768`), `runtime.cppm` `331bd1b629df` (was `91f866dc8ff6`), `runtime.t.cpp` `162997debb60` (was `5fbcbade1f32`), `profile.cpp` `91ac81f3d466`, `profile.cppm` `7bb805634449`, `profile.t.cpp` `d94976e9c9ea` (the last three unchanged).
- Reason for the change: the round record's section "Repair after mutation probes" added `owner_charge` to `profile_invocation_observation` in `runtime.cppm`/`runtime.cpp` and one new test. The round brief says runtime.cppm and runtime.cpp were unchanged and that C02 adds no observation fields (N-c). See "Deliberate differences".

## Commands

| # | Command | Exit | Result |
|---|---|---|---|
| 1 | `cmake --build build/rewrite-debug --target planar_tests` | 0 | `ninja: no work to do.` (already built from this tree) |
| 2 | `ctest --test-dir build/rewrite-debug --no-tests=error -j 4 --timeout 300` | 0 | `100% tests passed out of 985`; unit 978, acceptance 7; zero failed. No skipped, disabled or not-run entries (the four grep hits for "skip" are test names). |
| 3 | `ctest --test-dir build/rewrite-debug -N` | 0 | `Total Tests: 985` |
| 4 | `python3 scripts/rewrite-check.py lint --build-dir build/rewrite-debug` | 0 | `Formatting and Doxygen checked 136 active source files` |
| 5 | `cmake --build build/rewrite-debug` | 0 | Did work: 4 steps including a dyndep regeneration and relinking `bin/planar`; success. |
| 6 | `cmake --build build/rewrite-debug` | 0 | `ninja: no work to do.` Zero steps, after the successful build in 5. |

All criteria are met: nonzero matched tests, zero failures, zero skips, second build did no work. 39 tests match `X06 B3` in the inventory.

## Inventory

Compared against `tests/native/v5-02-x06-b3-b-c01-evidence/inventory.txt` (982 names; the C01 review inventory is the same size; both are gitignored, not committed). Current list: `inventory.txt` (985 names, sorted, test numbers stripped). Diffs: `new-names.txt`, `removed-names.txt`.

Kept 981, removed 1, new 4 (982 - 1 + 4 = 985). The brief said "982 kept"; strictly one name is gone through the rename the round record allows.

- Removed (C01-introduced rename): `replacement.X06 B3 B captured profile content is reverified before source binding, after capture and after sealing`.
- New:
  - `replacement.X06 B3 B prepare_load reverifies captured profile content exactly three times` (rename target)
  - `replacement.X06 B3 B prepare_load reads the declaration through the watched interposer`
  - `replacement.X06 B3 B exhaustion depends on route consumption because the instruction budget is one cumulative counter`
  - `replacement.X06 B3 B owner allocation charge is observable and covers the owner's own storage` (added by the post-probe repair)

## (a) Acceptance criteria to tests

Lines refer to `TEST_CASE` in `src/engine/runtime/runtime.t.cpp` at hash `162997debb60`. Names omit the `replacement.` prefix.

| Criterion / finding | Test | Line |
|---|---|---|
| 1 R2: `d9b54d0579c3` killed; light variant succeeds; phases under 100M | X06 B3 B exhaustion depends on route consumption because the instruction budget is one cumulative counter | 1986 |
| 2 R3: zero-declaration-read watches only the declaration | X06 B3 A into_extension adopts the same VM and receipts without recapture (1500); X06 B3 B prepare_load succeeds through phase 2 for custom and mixed jobs | 1500, 1628 |
| 2 R3: capture-change variant watches declaration alone | X06 B3 B capture change before prepare_load refuses capture_changed with zero declaration reads | 1837 |
| 2 R3: positive control | X06 B3 B prepare_load reads the declaration through the watched interposer | 1666 |
| 2 R3: `52b29026261a` killed or renamed; masking reason | X06 B3 B prepare_load reverifies captured profile content exactly three times (renamed; masking reason in round record) | 1976 |
| 3 R1: red-first and killing-mutant record per C01/C02 test | section (b) | n/a |
| 3 re-run of C01 probe set | `probes/results.md`, `probes/results.tsv` (pre-repair candidate; see limitations) | n/a |
| 4 names preserved, new names listed | Inventory section | n/a |
| 4 diagnostics byte-identical, no `assert()`, no new public stage surface | Not verified by this gate beyond build, test and lint. The diff adds a public observation field (see (c)); no diagnostic text check was run here | n/a |
| 5 N-a | Decision record only; no test reaches it | n/a |
| 5 N-b | X06 B3 B owner allocation charge is observable and covers the owner's own storage; also X06 B3 B owner implementation allocation is charged identically by both factories | 1615, 1603 |
| 5 N-c | Not met literally: `owner_charge` is a new observation field (see (c)) | n/a |
| 5 N-e | Record edits; zero-custom-job success variant: X06 B3 B success releases nested temporaries to the exact pre invocation membership (`{false,false}` case) and the phase-2 success test | 1903, 1628 |
| 6 Gate | Commands table | n/a |

Other B3-B tests: 1578, 1675, 1691, 1702, 1720, 1742, 1766, 1787, 1808, 1872, 1922, 1956.

## (b) Red-first

Command as read from `probes/probe2.py` and `probes/run-probes.sh` (I did not re-run them): each mutant is applied to a scratch copy outside the checkout by exact-once replacement, then `cmake --build <scratch>/build --target planar_runtime_tests[,planar_capture_native_tests]`, then `perl -e 'alarm 1200; exec @ARGV' ctest --test-dir <scratch build> -R '<regex>'` with regex `X06 B3` (plus `D2 B2 C01 first retained reads` for native mutants). Exact flags and scratch path are in those scripts. Failing line numbers below are scratch-copy lines of the pre-repair test file and are about 13 lower than the current line for tests after 1615.

Pre-fix snapshot: the C01 pre-fix `runtime.cpp` `7783183ff6f3` and `runtime.cppm` `f30b48ea86be` (probe `snapshot-c01-prefix`) failed to build against the C02 tests (build exit 1). That is a compile failure and is not offered as red-first. No other pre-fix behavioural snapshot is in the evidence. Each test's red is therefore its killing mutant (a real red: the mutant builds and the named test fails; restored bytes equal the original in every row).

| Test | Line | Mutant (sha256 12) | Failing line(s) / count |
|---|---|---|---|
| exhaustion depends on route consumption | 1986 | `reset-f-instr` `d9b54d0579c3`; `double-charge-route` `8984956b837d`; `reset-a-instr` `85b4f5383237`; `reset-e-instr` `8d241449497f` | 2002, 2008 (1 of 36); 1992 (1 of 36); 2008 (2 of 36 each) |
| reads the declaration through the watched interposer | 1666 | `control-watches-unread-file` `d0a6abbc2777` (test-side) | 1660 (1 of 36) |
| reverifies captured profile content exactly three times | 1976 | `reverify-pre` `62086754001d`, `-mid` `1565223e7db6`, `-final` `c631ebfa0a99`, `-mid-final` `13e47edc9d15`, `-all` `82950fd52247`, `-all-and-source` `643d3f3384f1` | 1971 (1 of 36 each). `reverify-moved` `52b29026261a` survives (equivalent mutant; hence the rename) |
| capture change, zero declaration reads | 1837 | `decl-read-in-prepare` `c8510e825f05`; `reverify-pre-and-open-bound-protocol` `706865d50730`; `reverify-pre-and-source-error-mapping` `ae349ca122a9` | 1854 (2 of 36); 1850 and 1971 (3 of 36 each) |
| phase-2 success, zero declaration/registry reads | 1628 | `decl-read-in-observe` `dc34e446a7d3` | 1640 (1 of 36) |
| declaration leaf replaced before prepare_load | 1720 | `decl-read-in-prepare`, `reverify-pre-and-*` | 1724 / 1850 |
| cumulative budget spans route, validation, preparation | 1872 | `reset-a-instr`, `reset-a-bytes` `f1e064072a0f`, `reset-e-bytes` `692c3165df60`, `reset-f-bytes` `cad9bc2eb5ce` | 1871 / 1877 / 1878 (2 of 36 each) |
| one frame cumulative counters never reset | 1787 | the four `reset-*-bytes` mutants | 1785 (2 of 36 each) |
| success releases to exact membership | 1903 | `release-success` `dfa8ec077558`, `release-both` `55172c300d4b`, `baseline-false` `a7e7c894c411` | 1901-1906; 1900 |
| failure paths release; cumulative exhaustion cleans nested roots | 1922, 1808 | `release-failure` `b13512efc6c7`, `release-both` | 1931, 1932; 1810, 1811 |
| opaque consumed owners; final intent equals bound intent | 1766, 1742 | `consumed` `539aa2f2a2a0` | 1746; 1469 (3 of 36) |
| every job validated exactly once | 1702 | `everyjob` `332d4145ede0` | 1695 (12 of 36) |
| known-load owner refuses into_extension | 1578 | `n3` `a72dda1d37a2` | 1582 (3 of 36) |
| owner allocation charge observable (new test, 1615) | 1615 | Only in-place probes on a backup, recorded in the round record: charge removed from both factories fails it (1 of 39); from the opaque factory only fails it plus two factory-charge tests (3 of 39). Round record gives restored hash `1a402d5995ff`. No raw log or command is in the evidence folder | Missing: exact command, mutant hashes and raw logs |

Gaps, stated and not filled:
- No killing-mutant row for the tests at 1675, 1691 and 1956 (`everyjob` fails 12 tests but not these by name).
- The probe set was run on the pre-repair candidate. It was not re-run on the current candidate (`runtime.cpp` and `runtime.cppm` differ), so criterion 3's "re-run on the final candidate" is not evidenced for this tree.
- The repair probes ran in place on the shared checkout (with a backup), which the brief's hard rule forbids.
- `n1-both` `9b2c43a0e71d` was recorded as surviving before the repair; the repair claims to kill it, evidenced only by the round-record text.

## (c) Deliberate differences

- Test rename `...reverified before source binding, after capture and after sealing` to `...reverifies captured profile content exactly three times`: permitted by round criterion 4 (C01-introduced name).
- New public field `profile_invocation_observation::owner_charge` plus owner-charge accounting in `runtime.cpp`: brief N-b asks to detect a charge removed from both factories "if feasible", but N-c says to add no further observation-only fields beyond what R2 and R3 strictly need. R2 and R3 do not need this field, so the addition goes beyond N-c and rests on N-b only. This is a reviewer decision. Whether any diagnostic text changed was not checked by this gate.

## Limitations

macOS Debug only; no RelWithDebInfo, Linux, RSS or whole-invocation budget claim. Probe results are the coders' and were not re-run. Evidence folders are gitignored. The inventory base (C01 evidence) is uncommitted.
