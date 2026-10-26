# V5-02-X06-C-C03 gate report (final run)

This report supersedes the earlier 10:36 gate report. The tree changed afterwards (acceptance.t.cpp gained three tests), so every command was re-run.
The earlier raw logs `raw/1-*.log` to `raw/7-*.log` are kept untouched; this run's logs are `raw/01-*` to `raw/08-*`.

## Candidate identity

HEAD `caaf0396ae90a7f7d2d511b9a9a6e185e4362d52` plus the uncommitted X06-C, C01, C02 and C03 working tree (not committed). Source: `raw/07-identity.txt`.

| Row (pathspec) | sha256 |
|---|---|
| `git diff` (tracked, all paths; untracked excluded) | `099386a6c3eea867c13c2e6fe9bb1081f1bfab95a6d6b8183268b24ef4e1acf2` |
| `git diff HEAD -- src` (tracked only) | `daced2fd0c1c6ae9f50237373a30c15f5b32c31a234f894b431edcb93e981ee8` |
| `git diff HEAD -- src` then the bytes of the five untracked sources in the order acceptance_seam.cpp, acceptance_seam.cppm, profile_acceptance.cpp, profile_acceptance.cppm, profile_acceptance.t.cpp | `5a9214f17d39dfe65fdef554c3efc658d4f54c7a5ca2fb388e3a3fcbccc1a1fb` |

Per-file hashes are in `raw/07-identity.txt`. Only `acceptance.t.cpp` differs from the earlier gate report (`bc8c0e6e...`, was `00b1f2b5...`).
The prober's evidence (`probes/PROBES.md`) records candidate hash `3d9c87aa...` for `git diff HEAD -- src`, which is an earlier tree than this one,
and the coders' red snapshot is `df3828ed...`, earlier still. So the probes and the red logs do not cover the three newest tests directly (see Limitations).

## Commands (raw logs in `raw/`, nothing piped, exit codes read from files)

| # | Command | Exit | Result |
|---|---|---|---|
| 1 | `cmake --build build/rewrite-debug --target planar_tests` | 0 | `ninja: no work to do.` (`raw/01-build.log`) |
| 2 | `ctest --test-dir build/rewrite-debug --no-tests=error --output-on-failure` | 0 | "100% tests passed, 0 tests failed out of 1038"; labels unit 1031, acceptance 7; total 371.56 s real; 0 skipped, 0 Not Run (the only "skip" substrings are test names); `#531` `X06 B3 C a file replaced at any allocation inside prepare_load is never published as verified` Passed 15.51 s (`raw/02-ctest.log`) |
| 3 | `ctest --test-dir build/rewrite-debug -N` | 0 | Total Tests: 1038 (`raw/03-ctestN.log`, `inventory.txt`) |
| 4 | `python3 scripts/rewrite-check.py lint --build-dir build/rewrite-debug` | 0 | clang-format `--Werror` and Doxygen: "Formatting and Doxygen checked 141 active source files" (`raw/04-lint.log`) |
| 5 | `cmake --build build/rewrite-debug` (first) | 0 | `ninja: no work to do.` (`raw/05-build.log`) |
| 6 | `cmake --build build/rewrite-debug` (second, no-change check) | 0 | `[0/2] Re-checking globbed directories...` then `ninja: no work to do.` Zero steps: pass (`raw/06-nochange.log`) |
| 7 | `python3 scripts/redact-local-paths.py check --repo-path $PWD --home $HOME <files>` on the 5 untracked sources, the 7 modified files and `docs/vernacular/rounds/V5-02-X06-C*.md` | 0 | clean (`raw/08-redact-check.log`). Not run without file arguments. The evidence folder was not included. |

Command 1 and 5 found nothing to rebuild because the tree had been built by the preceding coder and prober runs, so this gate did not itself compile the candidate from scratch
(first `ninja` of a changed tree is the one that proves it; the test binaries in command 2 were up to date with the sources). Command 6 is meaningful because command 5 succeeded first.
All criteria met: passed.

## Inventory

Current: 1038 names, saved as `inventory.txt` (ctest -N names, ctest order, `Test #n:` prefix removed; all start with `replacement.`).
Compared with (a) `tests/native/v5-02-x06-b3-c-c02-evidence/inventory.txt` (1003 names, most recent committed-lineage list), (b) `tests/native/v5-02-x06-c-c01-evidence/inventory.txt` (1031),
(c) `tests/native/v5-02-x06-c-c02-evidence/inventory.txt` (1036) and (d) the earlier C03 gate inventory (1035).

- Removed vs 1003 list: none. Removed vs C01 1031 list: none (so all 28 X06-C and C01 names are present).
- Removed vs C02 (5, the withdrawn passkey tests): `X06 C C02 candidate_values requires the passkey`, `X06 C C02 complete_native_candidate requires the passkey`,
  `X06 C C02 native_candidate callables and constructor require the passkey`, `X06 C C02 passkey grants are confined to the adapters and the test probe`, `X06 C C02 seam key has no public constructor`.
  Reason: the operator decision of 2026-10-26 withdraws the passkey; the round record allows removing or replacing them.
- New vs C02 (7, all C03): `seam is a non-exported partition of the acceptance module`, `profile adapter is a partition re-exported by the acceptance interface`,
  `no passkey grant or test hook vocabulary remains under src`, `only the adapters call complete_native_candidate`, `shared body refusal message bytes are pinned`,
  `workspace mismatch message bytes are pinned`, `runtime product sources import only ledger completion from the ledger` (each prefixed `X06 C C03 `).
- New since the earlier C03 gate (3): the last three above (`shared body refusal message bytes are pinned`, `workspace mismatch message bytes are pinned`, `runtime product sources import only ledger completion from the ledger`).
- Arithmetic: 1036 - 5 + 7 = 1038. No other name differs.

## Acceptance criteria to tests

Test names are `replacement.<name>` in ctest. Lines are TEST_CASE starts in the current tree (`acceptance.t.cpp` = src/engine/ledger/acceptance.t.cpp; `profile_acceptance.t.cpp` likewise).
There is no separate list of C02 findings beyond R1-R4 in the round record.

| Criterion / finding | Gate evidence | Test (file:line) or status |
|---|---|---|
| 1 seam is a non-exported partition, profile partition re-exported, consumers keep the import, no passkey/friend/hook | source-scan tests, all pass | `X06 C C03 seam is a non-exported partition of the acceptance module` acceptance.t.cpp:825; `X06 C C03 profile adapter is a partition re-exported by the acceptance interface` acceptance.t.cpp:839; `X06 C C03 no passkey grant or test hook vocabulary remains under src` acceptance.t.cpp:849 |
| 2 negative-compile probes and control | not run by the gate; prober deliverable exists at `probes/raw/negative-compile/negative.log` and the I6 section of `probes/PROBES.md`; I did not re-verify it | prober evidence (earlier candidate hash) |
| 3 seam tested only through adapters | no direct seam unit was added (no new file in status beyond the partition files); seam behaviour tests use the adapters | `X06 C seam interns an effective stdlib closure beyond the original tree limits` profile_acceptance.t.cpp:759; `X06 C C01 seam interns the registry tree with the provenance limits` profile_acceptance.t.cpp:863; `atomic native format2 acceptance history export and backup restore survive removed inputs` acceptance.t.cpp:689 |
| 4 source scan second check, allowed set counted correctly | scan tests pass | `X06 C C01 seam is unreachable from a plain acceptance importer` acceptance.t.cpp:770; `X06 C C03 only the adapters call complete_native_candidate` acceptance.t.cpp:886. That the record states the count correctly is not gate-checkable |
| 5 inventory | section above | 1003 + 28 names present; 5 removed with reason; 7 new listed |
| 6 invariant rows / mutants | gate does not run mutants. Prober evidence `probes/PROBES.md`: 88 rows, 66 killed, 15 survived, 7 build-failed. C14 `X06 C profile completion refuses a same id intent whose body differs from the bound intent` profile_acceptance.t.cpp:350 (killed by I13); X27 `X06 C no Lua runs while the profile write transaction is open` profile_acceptance.t.cpp:563 (killed by I10) | prober evidence, earlier candidate |
| 6a prober finding F1 (diagnostic message strings in the shared body asserted by no test: D2 to D7, D9, E1) | the new pinning tests were added after that probe | `X06 C C03 shared body refusal message bytes are pinned` acceptance.t.cpp:918; `X06 C C03 workspace mismatch message bytes are pinned` acceptance.t.cpp:971. Whether they kill D2 to D7 and D9 is NOT shown: no re-probe of these tests exists in the evidence |
| 6b prober finding F2 (no automated layering gate, CY2 survives) | new test | `X06 C C03 runtime product sources import only ledger completion from the ledger` acceptance.t.cpp:983. Not re-probed |
| 7 dependency audit, generated variants build | generated variants `planar_atomic_acceptance_seam_test` / `planar_metadata_acceptance_seam_test` are part of `planar_tests` and passed in the 1038 run; configure and build had no cycle. Written audit: `probes/raw/dependency-audit.txt` (runtime imports only `planar.engine.ledger.completion`; src/cmd imports only `planar.engine.ledger.acceptance`) | prober evidence |
| 8 diagnostics byte-identical, #531 not slower | every existing diagnostic test passes; no diagnostic test was removed; `#531` 15.51 s in this run (record range about 15.3 to 16.5 s) | `X06 B3 C a file replaced at any allocation inside prepare_load is never published as verified` (#531) |
| 9 records corrected, hashes refreshed | coordinator work, not checked here; the identity table above is fresh for this tree | n/a |
| 10 red-first behavioural | next section | `red/RED.md` |

## Red-first (from `red/RED.md` and `red/raw/t1.log` to `t13.log`)

Command (RED.md): `ctest --test-dir build/rewrite-debug -R '^replacement\.<escaped name>$' --no-tests=error --output-on-failure`, after `cmake --build build/rewrite-debug --target planar_tests` (`red/raw/build.log`).
Snapshot hash: `sha256(git diff HEAD -- src) = df3828ed5b814375993e01c4deecb4c09680857267decd47b459638e99d49620`. This covers tracked changes under `src` only; RED.md does not state that untracked files
were included, and no copy of the pre-fix snapshot is kept, so criterion 10's pathspec and untracked-file requirement is NOT shown as met. Test numbers in the logs are red-time registration numbers.

| Log | Test | Outcome | Failing lines in acceptance.t.cpp at red time (current line) | Kind |
|---|---|---|---|---|
| t1 | X06 C C03 seam is a non-exported partition of the acceptance module | failed (assertion) | 827, 828, 831, 834, 835 (now at 825) | real red |
| t2 | X06 C C01 seam is unreachable from a plain acceptance importer | failed (assertion) | 776, 779, 809 (now at 770) | real red (existing test updated to the partition layout) |
| t3 | X06 C C03 profile adapter is a partition re-exported by the acceptance interface | failed (assertion) | 841, 842, 845, 846 (now at 839) | real red |
| t4 | X06 C C03 no passkey grant or test hook vocabulary remains under src | failed (assertion) | 883 (now at 849) | real red |
| t5 | X06 C C03 only the adapters call complete_native_candidate | passed | none | guard (already green; expectRed no) |
| t6 to t13 | guards: profile derived trees (profile_acceptance.t.cpp:240), C01 registry tree (863), stdlib closure (759), no Lua in transaction (563), atomic final receipt refusal rollback, atomic replay precedes head/capture inspection, same id intent body differs (350), native format2 history export and backup restore (acceptance.t.cpp:689) | passed | none | guards |

Missing from the evidence: red-first runs for `shared body refusal message bytes are pinned`, `workspace mismatch message bytes are pinned` and `runtime product sources import only ledger completion from the ledger`
(they postdate the red run; nothing under `red/` names them). Whether each is a real red or a guard that was already green is therefore unknown; they are mutation-justified only if the prober re-runs D2 to D7, D9 and CY2.
The record item C02-R4 (departure statement for C14 `expectRed=false`) is a coordinator statement, not in the coders' files. RED.md says I6 is compile-time only (no runtime test).

## Deliberate behaviour differences

None observed. Product behaviour and diagnostics are unchanged. The intended difference is structural: the seam is a non-exported partition instead of a passkey-protected module, authorized by the operator decision
of 2026-10-26 recorded in the round record (it supersedes the C02 passkey decision and ranks above the contract and architecture text). The profile partition makes `planar.engine.ledger.acceptance` import
`planar.engine.runtime`, which the round record flags as a known limit; the prober's dependency audit found no cycle.

## Limitations

- Builds 1 and 5 found no work (earlier runs built the tree); this gate did not itself compile the candidate from scratch.
- Probes and red logs were taken on earlier trees (hashes `3d9c87aa...` and `df3828ed...`); the three newest tests have no red-first or mutation evidence. The prober should re-run the F1 and F2 rows.
- Probe F4 notes earlier incidental flakiness in two variant tests; they passed in this run.
- Redact check covered the 12 code files and the X06-C round and review records, not this evidence folder.
- `make test-all` was not part of the agreed commands and was not run.
