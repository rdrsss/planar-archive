# V5-02-RA01-C-C01 gate report (gate runner, second run)

Gate run on 2026-11-01. The runner edited no source file and wrote only under tests/native/v5-02-ra01-c-c01-evidence/. This run supersedes the first gate run: between the two, `src/cmd/planar/authoring_orchestrator.t.cpp` changed (a second new test, `RA01-C-C01 orchestrator markdown bytes keep the golden title authority and joins`, was added to close the I6 coverage gap the mutation prober reported), so every command was re-run. The first run's report is kept as raw/g1-REPORT.md and its logs keep the `gate-` prefix; this run's raw logs use `g2-` (and raw/linux2/). Raw logs contain absolute local paths and are not for staging.

## Candidate identity

Output of identity.py (pathspec `src tests/native :!tests/native/*-evidence`), verbatim, taken before the first command and again after the last (unchanged):

| identity | value |
|---|---|
| HEAD | `93053c42a4c68fd3ecc8911edab6464e3d1307f3` |
| sha256(git diff --binary HEAD -- src tests/native :!tests/native/*-evidence), tracked only | `b17fda64e25a9683559256259c5c88b19563798acdfcdcbb22da1b82ec97b843` |
| combined sha256 (diff bytes, then each untracked path NUL bytes, sorted), same pathspec | `b17fda64e25a9683559256259c5c88b19563798acdfcdcbb22da1b82ec97b843` |

| file | state | sha256 |
|---|---|---|
| `src/cmd/planar/authoring_orchestrator.t.cpp` | modified | `38d79424ab104ac3be416588bcc58d33715976fd781a6128e173b51c6a06a46f` |
| `src/stdlib/planar/orchestrator.lua` | modified | `e7d9f570ef70b014e6b3facac55709c59ea8736958bb5ef7d5147eb227bc87a1` |

sha256 of `git diff HEAD` over the whole tree (tracked, includes the records files): `274fb6e78d9c3df8904a497f238f7f4f701d7ee93d7757046e79301be3ebcfc0`.

Files edited by the round (git status): src/stdlib/planar/orchestrator.lua; src/cmd/planar/authoring_orchestrator.t.cpp; docs/vernacular/acceptance-matrix.md; tests/native/v5-02-ra01-c-evidence/REPORT.md; tests/native/v5-02-ra01-c-evidence/acceptance-rows.md; docs/vernacular/rounds/V5-02-RA01-C-C01.md (new); tests/native/v5-02-ra01-c-c01-evidence/ (new).

Boundary files: `git diff --quiet HEAD` over src/engine/runtime/runtime.cpp, src/stdlib/planar/commands.lua, schema.lua and semantic.lua: all four identical to HEAD, so `c02_off_boundary` needs no re-pin (hashes as in the first run, equal to tests/native/v5-02-ra01-b-w2-evidence/c02-pin.md lines 34-37).

## Commands

| # | Command | Exit | Result |
|---|---|---|---|
| 1 | `python3 scripts/rewrite-check.py lint --build-dir build/rewrite-debug` | 0 | "Formatting and Doxygen checked 171 active source files" (raw/g2-lint.log) |
| 2 | `cmake --build build/rewrite-debug --target all planar_tests` | 0 | built (raw/g2-build-debug.log) |
| 3 | `cmake --build build/rewrite-release --target all planar_tests` | 0 | built, 830 steps (raw/g2-build-release.log) |
| 4 | `ctest --test-dir build/rewrite-debug -N` | 0 | Total Tests: 1439 |
| 5 | `ctest --test-dir build/rewrite-release -N` | 0 | Total Tests: 1439 |
| 6 | `ctest --test-dir build/rewrite-debug --no-tests=error -j4 --output-on-failure` | 0 | 100% tests passed out of 1439; no failed, skipped or not-run line (raw/g2-ctest-debug.log) |
| 7 | `ctest --test-dir build/rewrite-release --no-tests=error -j4 --output-on-failure` | 0 | 100% tests passed out of 1439; no failed, skipped or not-run line (raw/g2-ctest-release.log) |
| 8, 9 | no-work build Debug, twice (same build command as 2) | 0, 0 | both `ninja: no work to do.` (raw/g2-nowork1-debug.log, g2-nowork2-debug.log), run after the successful build in 2 |
| 10, 11 | no-work build RelWithDebInfo (preset `release`), twice | 0, 0 | both `ninja: no work to do.` (raw/g2-nowork1-release.log, g2-nowork2-release.log), run after the successful build in 3 |
| 12 | `scripts/linux-gate.sh <scratch out-dir>` (Debug and RelWithDebInfo in one container build, no -t) | 0 | see Linux |
| 13 | `python3 scripts/redact-local-paths.py check <explicit files>` | see Redaction | vacuous gate-run check, pointer only: raw/c02-redact-check.log (corrected check), raw/c02-redact-control.log (control) |

## Linux

df before: 134742488 512-byte blocks free (about 64 GiB, floor 15 GiB); after about 53 GiB. One docker build, no tag; docker-build.exit 0; script-run `docker builder prune -f`, builder-prune.exit 0. All ten container exit files 0 (configure, build, ctest, inventory, nowork for debug and release). Debug and RelWithDebInfo each: "100% tests passed out of 1439"; each no-work step `ninja: no work to do.`. `x07.two_process_cas_one_winner` Passed in both (Test #1430; 1.00 s and 0.44 s). Both container sorted inventories are byte-identical to inventory.txt (cmp). Manifest compare: host before vs after equal; host vs container context equal. linux/arm64 only. Raw: raw/linux2/ (script logs and container logs). The script's own stdout is raw/g2-linux-gate.log.

## Test inventory

inventory.txt: 1439 names, sorted (C locale) from `ctest -N` Debug; Debug and RelWithDebInfo name lists are byte-identical (raw/g2-names-debug.txt, g2-names-release.txt). Compared against the most recent committed inventory, tests/native/v5-02-ra01-c-evidence/inventory.txt (1437 names at 93053c42): added exactly two names, none removed or renamed. inventory-debug.txt is the gate run 2 Debug list (sorted raw/g2-names-debug.txt; replaced at C02, old content kept in raw/c02-inventory-debug-old.txt).

- `replacement.RA01-C-C01 orchestrator projection wrapper is versioned and mandatory`
- `replacement.RA01-C-C01 orchestrator markdown bytes keep the golden title authority and joins`

1437 + 2 = 1439. (The round record says "1437 names kept, new listed"; the second added name is a deviation from the first gate run's 1438 and is explained above.)

## (a) Acceptance criteria and findings to test names

New tests in src/cmd/planar/authoring_orchestrator.t.cpp: T1 `RA01-C-C01 orchestrator projection wrapper is versioned and mandatory` at :740 (SECTIONs `wrapper shape` :741, `sections order and count` :770, `every section mandatory` :802, `source pins` :843); T2 `RA01-C-C01 orchestrator markdown bytes keep the golden title authority and joins` at :898. Existing consumer test `RA01-B2 orchestrator prompt carries five sections in order` at :416.

| Criterion (round record) | Test and location | Gate result |
|---|---|---|
| Req 1: `project()` returns `{version=1,sections}`, five sections `mandatory=true`, contract order | T1 :741 (version integer 1, exactly keys version and sections, rawlen 0), :770 (five ids in contract order), :802 (Lua boolean true on each, exactly keys id, text, mandatory); behavioural on the real orchestrator.lua bytes in a test-built Lua state (limit: not the embedded authoring stack, plan D22) | passed Debug, RelWithDebInfo, Linux x2 |
| Req 2: `orchestrator_command_impl` iterates `result.sections`, command bytes unchanged | T1 `source pins` :843 (supplement); behaviour by existing B2 test :416 (untouched; the prober showed the bypass mutant fails it); byte-level structure and joins by T2 :898; no existing golden modified | passed |
| Req 3: no public field, method or hook | no product file but orchestrator.lua changed (7 lines); T1 uses the C Lua API on the file-local `project` found by upvalue name; limit stated in the test comment and plan D22 | review item, no symbol added |
| Req 4: red first | section (b) | see (b) |
| Req 5: boundary files byte-identical | `X08 C01 role preparation outputs-off boundary pins the request-record conversion charge` (src/engine/runtime/external_registration.t.cpp) plus the per-file `git diff --quiet` above | identical, test passed |
| Req 6: records updated | docs/vernacular/acceptance-matrix.md (A07), acceptance-rows.md rows C-15r and RA01-C-F1, RA01-C REPORT.md note | edited; review item, no test |
| Acc: probe mutants killed (drop mandatory; one section false; drop version; change version; reorder; bypass in impl) | T1 SECTIONs per INVARIANTS I1-I5; bypass also killed by B2 test (I5b); results in probes/PROBES.md | not run by the gate runner; the prober reports all killed (drop mandatory I3a, one false I3c1-c3, drop version I1a, change version I2a-c, reorder I4swap, bypass I5 and I5b) |
| Acc: existing goldens untouched, orchestrator bytes unchanged | all 1437 prior names kept and passing; T2 :898 | passed |
| Acc: names kept, new listed, identical inventories | inventory section | met (1437 kept, 2 added, identical across Debug, RelWithDebInfo, Linux) |
| Finding RA01-C-F1 (row C-15r) closed | T1 (all four SECTIONs) | closed on gate evidence; the reviewer decides |
| Prober survivors I6join, I6title, I6text (orchestrator markdown bytes unpinned, pre-existing gap, not a round finding) | T2 :898 added after the probe run | gate: passes. NOT mutation-probed at gate time: the prober's I6 rows predate T2 and the probes folder held no re-run then. Since the gate, the re-probe is recorded: see probes/PROBES.md (T2 probed, 9 of 9 I6 mutants killed by T2, no survivors) |
| F2 to F8 | carried to R15 or later | not worked, per scope |

## (b) Red-first

Source: the coders' red/RED.md, red-first.log and red/raw/; the runner did not re-run it (the checkout holds the fixed product and the runner may not mutate or revert it).

Red-time identity (RED.md): HEAD 93053c42a4c68fd3ecc8911edab6464e3d1307f3; sha256 of red/snapshot/candidate.diff (`git diff HEAD -- src tests/native`) `c2a1fda122a9895be56b24811efbd76148c2efdbe235f38050d8b0f987b09f74`; sha256 of red/snapshot/SHA256SUMS `f914784422ae3b12751c8c411551300987458fd199a3014596b27d07f2a88b36`. Snapshot is T1 plus the pre-fix orchestrator.lua. Build: `cmake --build build/rewrite-debug --target planar_tests`, exit 0. Command: `ctest --test-dir build/rewrite-debug -R '<exact name>' --no-tests=error --output-on-failure`.

| Test | Result | Failing lines | Real red or guard |
|---|---|---|---|
| T1 `RA01-C-C01 orchestrator projection wrapper is versioned and mandatory` | failed, 28 of 79 assertions (red/raw/c01-wrapper.log, red-first.log) | authoring_orchestrator.t.cpp:750, 751, 752, 755, 759, 761, 765, 767 (wrapper shape); 778 (order); 810 (mandatory); 861-873, 890-892 (source pins) | real red, behavioural: runtime values from a successful load and call (`lua_type == 3` expands to `0 == 3`; `lua_rawlen == 0` expands to `5 == 0`), not a compile or load error |
| T2 `RA01-C-C01 orchestrator markdown bytes keep the golden title authority and joins` | no red-first evidence exists | none recorded | MISSING: T2 postdates the red snapshot and the evidence has no run of it. Because it pins bytes the fix does not change, it was expected to pass pre-fix (a guard); this is not demonstrated. The prober's I6 survivors show those bytes were unpinned before T2. Mutation evidence: see probes/PROBES.md (T2 probed, 9 of 9 I6 mutants killed by T2, no survivors) |
| `RA01-B2 orchestrator prompt carries five sections in order` | passed pre-fix (red/raw/b2-prompt.log) | none | guard that already passed (becomes the I5b kill) |
| `X08 C01 role preparation outputs-off boundary pins the request-record conversion charge` | passed pre-fix (red/raw/x08.log) | none | guard that already passed |

Also missing: no per-SECTION pass/fail log at red time (the prober's PF row, probes/PROBES.md, shows all four T1 SECTIONs failing on the pre-fix orchestrator.lua, diff sha256 `14d584b23d83661e...`). The red-time identity.py output was not pasted verbatim: RED.md states the hashes in prose, and the independent review reconstructed them (candidate.diff `c2a1fda1...f09f74`, SHA256SUMS `f9147844...a88b36`, red test file `379d82d7...2a3d`). The reconstruction is accepted non-blocking, citing docs/vernacular/rounds/V5-02-RA01-C-C01-review.md.

## (c) Deliberate behaviour differences

None in command output. The only behavioural change is internal: `project()` returns `{version=1,sections=...}` with `mandatory=true` on each section, required by authoring-contract.md:422; no contract amendment (round record, option (a)). No diagnostic message changed.

## Redaction

The gate-run redaction check (`python3 scripts/redact-local-paths.py check <explicit files>`, raw/g2-redact.log, empty) was vacuous: the command line was not saved, and the script silently drops any argument that is not an existing regular file, so an unsplit list passes. The cause is not proven; the log is kept. The round's redaction step ran on red-first.log and plan/DECISIONS.md, and the script updated tests/native/path-redaction.json. The corrected check's file count and exit code are in raw/c02-redact-check.log, and the positive control is in raw/c02-redact-control.log. The no-argument invocation (raw/g2-redact-noargs.log) scans the whole tree including ignored build output and is not meaningful as a gate.

## Limitations

- The runner ran no mutation probes, review or commit.
- T1 observes `project()` by running the real orchestrator.lua bytes in a test-built Lua state, not the embedded authoring stack; consumer wiring is pinned by source text plus the existing B2 behavioural test (plan D22).
- T2 has no red-first or mutation evidence at gate time (see (a), (b)); its later mutation probe is recorded in probes/PROBES.md (T2 probed, 9 of 9 I6 mutants killed by T2, no survivors).
- Debug and RelWithDebInfo were built incrementally (not clean builds).
- Linux covers linux/arm64 only.
