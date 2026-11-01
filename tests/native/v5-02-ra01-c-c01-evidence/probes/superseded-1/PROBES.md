# V5-02-RA01-C-C01 mutation probes

Prober run of 2026-11-01. No earlier probes/ existed for this round, so nothing was moved to superseded-<n>/ (the previous lineage round, V5-02-RA01-B-W2-C02, probed other files; orchestrator.lua and authoring_orchestrator.t.cpp changed this round, and its X08 row C02-15c was re-run rather than carried). Only tests/native/v5-02-ra01-c-c01-evidence/probes/ was written in the checkout.

Scratch: warm copy `probe-warm` (outside the checkout, debug preset, kept) synced with `rsync -a --delete --exclude build/ --exclude .git/ --exclude 'tests/native/*-evidence/'`, incremental `planar_tests` build. Baseline before the first mutant: `ctest -R "RA01-C-C01|RA01-B2 orchestrator prompt carries|X08 C01 role preparation outputs-off"` 3/3. Driver `raw/driver/drv.py` (mutant definitions inside): exact-anchor patch (anchor must occur once, else patch-failed, not a kill), build of the test executable holding the named test (`planar_command_tests`, `planar_runtime_tests` for I8), `ctest -R '^replacement\.<name>$'`, byte-copy restore with sha256 verified (`restored_verified=true` in every row of `results.jsonl`), rebuild after restore. Each `raw/<id>.log` holds the unified diff, its sha256, original/mutated sha256, build and test commands, exit codes, failing test and SECTION names and failing lines (authoring_orchestrator.t.cpp:750-752 wrapper shape, :765 etc.). Outputs: `results.jsonl`, `raw/driver/run.out` (the I1a row ran first alone and is only in results.jsonl; run.out is the 31 later rows). Platform macOS Debug, non-root.

## Candidate identity (identity.py, verbatim; `raw/identity-before.txt` equals `raw/identity-after.txt`)

| identity | value |
|---|---|
| HEAD | `93053c42a4c68fd3ecc8911edab6464e3d1307f3` |
| sha256(git diff --binary HEAD -- src tests/native), tracked only | `ed6098cfa8266c6935eb9a5e48940ca5e96c09595b4fc24440ec15b483ec8480` |
| combined sha256 (diff bytes, then each untracked path NUL bytes, sorted), pathspec src tests/native, evidence excluded | `ed6098cfa8266c6935eb9a5e48940ca5e96c09595b4fc24440ec15b483ec8480` |

| file | state | sha256 |
|---|---|---|
| `src/cmd/planar/authoring_orchestrator.t.cpp` | modified | `379d82d765414932a174d3cc24a629d585c8824e4cfd681a3066112aea142a3d` |
| `src/stdlib/planar/orchestrator.lua` | modified | `e7d9f570ef70b014e6b3facac55709c59ea8736958bb5ef7d5147eb227bc87a1` |


The warm copy's orchestrator.lua and authoring_orchestrator.t.cpp were byte-identical to these before the run (cmp) and were restored byte-identical after every mutant.

## Mutant results (run; every killed row is killed by the new test unless noted)

| id | invariant | mutant | diff sha256 (16) | result | killing test | failing SECTIONs | run or carried |
|---|---|---|---|---|---|---|---|
| I1a | I1 | drop version | `cb5180b1d71060bf` | killed | RA01-C-C01 orchestrator projection wrapper is versioned and mandatory | source pins, wrapper shape | run (`raw/I1a.log`) |
| I1b | I1 | bare return sections | `b52527bef3f25638` | killed | RA01-C-C01 orchestrator projection wrapper is versioned and mandatory | every section mandatory, sections order and count, source pins, wrapper shape | run (`raw/I1b.log`) |
| I1c | I1 | extra key | `034231f18b382de2` | killed | RA01-C-C01 orchestrator projection wrapper is versioned and mandatory | source pins, wrapper shape | run (`raw/I1c.log`) |
| I1d | I1 | wrapper moved to consumer, project bare | `aa73581dbec3229a` | killed | RA01-C-C01 orchestrator projection wrapper is versioned and mandatory | every section mandatory, sections order and count, source pins, wrapper shape | run (`raw/I1d.log`) |
| I2a | I2 | version=2 | `59bdddfba1d3b48a` | killed | RA01-C-C01 orchestrator projection wrapper is versioned and mandatory | source pins, wrapper shape | run (`raw/I2a.log`) |
| I2b | I2 | version="1" | `f22565ef3579987d` | killed | RA01-C-C01 orchestrator projection wrapper is versioned and mandatory | source pins, wrapper shape | run (`raw/I2b.log`) |
| I2c | I2 | version=1.0 | `a786a1c86ca1aa9e` | killed | RA01-C-C01 orchestrator projection wrapper is versioned and mandatory | source pins, wrapper shape | run (`raw/I2c.log`) |
| I3a | I3 | drop mandatory from helper | `01685a7be4c05461` | killed | RA01-C-C01 orchestrator projection wrapper is versioned and mandatory | every section mandatory, source pins | run (`raw/I3a.log`) |
| I3b | I3 | helper mandatory=false | `6e7ead3c01a34ce8` | killed | RA01-C-C01 orchestrator projection wrapper is versioned and mandatory | every section mandatory, source pins | run (`raw/I3b.log`) |
| I3c1 | I3 | roles section .mandatory=false | `866766a0d0aa5a2d` | killed | RA01-C-C01 orchestrator projection wrapper is versioned and mandatory | every section mandatory, source pins | run (`raw/I3c1.log`) |
| I3c2 | I3 | roles ["mandatory"]=false | `e50cb090666e466f` | killed | RA01-C-C01 orchestrator projection wrapper is versioned and mandatory | every section mandatory, source pins | run (`raw/I3c2.log`) |
| I3c3 | I3 | roles string-built key=false | `93cb559d49053ca4` | killed | RA01-C-C01 orchestrator projection wrapper is versioned and mandatory | every section mandatory | run (`raw/I3c3.log`) |
| I3d | I3 | flag moved to wrapper | `c8b7b922d0791dd5` | killed | RA01-C-C01 orchestrator projection wrapper is versioned and mandatory | every section mandatory, source pins, wrapper shape | run (`raw/I3d.log`) |
| I3e | I3 | helper table.insert(sections,1,..) | `2601585d7c0db144` | killed | RA01-C-C01 orchestrator projection wrapper is versioned and mandatory | sections order and count, source pins | run (`raw/I3e.log`) |
| I3f | I3 | mandatory="true" | `42246017795898d1` | killed | RA01-C-C01 orchestrator projection wrapper is versioned and mandatory | every section mandatory, source pins | run (`raw/I3f.log`) |
| I4swap | I4 | swap sections 2,3 at runtime | `eca8d21247e644fd` | killed | RA01-C-C01 orchestrator projection wrapper is versioned and mandatory | sections order and count | run (`raw/I4swap.log`) |
| I4del | I4 | delete actions section | `1a7fe9c0300944d0` | killed | RA01-C-C01 orchestrator projection wrapper is versioned and mandatory | every section mandatory, sections order and count, source pins | run (`raw/I4del.log`) |
| I4add | I4 | add sixth section | `4b8604a1b90b8367` | killed | RA01-C-C01 orchestrator projection wrapper is versioned and mandatory | sections order and count, source pins | run (`raw/I4add.log`) |
| I5 | I5 | ipairs(project(c)) bypass | `fd003610602d4229` | killed | RA01-C-C01 orchestrator projection wrapper is versioned and mandatory | source pins | run (`raw/I5.log`) |
| I5b | I5b | same bypass, existing test | `fd003610602d4229` | killed | RA01-B2 orchestrator prompt carries five sections in order | - | run (`raw/I5b.log`) |
| I6join | I6 | join \n -> \n\n | `1fa305e4b1e35679` | survived | - | - | run (`raw/I6join.log`) |
| I6title | I6 | title text | `d1dc28851f11c977` | survived | - | - | run (`raw/I6title.log`) |
| I6text | I6 | section text | `d797d16279830fbd` | survived | - | - | run (`raw/I6text.log`) |
| I8 | I8 | 64B comment appended to semantic.lua | `73af78070e9a1d18` | killed | X08 C01 role preparation outputs-off boundary pins the request-record conversion charge | - | run (`raw/I8.log`) |
| X1heading | ext | heading marker '## ' -> '# ' | `d2191b159954b943` | killed | RA01-C-C01 orchestrator projection wrapper is versioned and mandatory | every section mandatory, source pins | run (`raw/X1heading.log`) |
| X2skipfirst | ext | consumer skips first section | `e32626ad49f4348b` | killed | RA01-B2 orchestrator prompt carries five sections in order | - | run (`raw/X2skipfirst.log`) |
| X3dropkey | ext | sections[1].mandatory=nil | `97a7b1fe39912b45` | killed | RA01-C-C01 orchestrator projection wrapper is versioned and mandatory | every section mandatory, source pins | run (`raw/X3dropkey.log`) |
| X4arraypart | ext | array part in wrapper | `69abf82b08420870` | killed | RA01-C-C01 orchestrator projection wrapper is versioned and mandatory | source pins, wrapper shape | run (`raw/X4arraypart.log`) |
| X5versionstr | ext | version=true | `47f26d08c1ffa6a3` | killed | RA01-C-C01 orchestrator projection wrapper is versioned and mandatory | source pins, wrapper shape | run (`raw/X5versionstr.log`) |
| X6nosections | ext | wrapper without sections | `15075563de02419e` | killed | RA01-C-C01 orchestrator projection wrapper is versioned and mandatory | every section mandatory, sections order and count, source pins, wrapper shape | run (`raw/X6nosections.log`) |
| X7dupid | ext | sections[5].id duplicated | `bf65a6464014ef2a` | killed | RA01-C-C01 orchestrator projection wrapper is versioned and mandatory | sections order and count | run (`raw/X7dupid.log`) |
| PF | R1 | whole file restored to HEAD (pre-fix) | `14d584b23d83661e` | killed | RA01-C-C01 orchestrator projection wrapper is versioned and mandatory | every section mandatory, sections order and count, source pins, wrapper shape | run (`raw/PF.log`) |

## Invariant coverage

- I1 (wrapper keys, rawlen 0): I1a-I1d and X4, X5, X6 killed; I1a/I1c/I2* are killed by SECTION `wrapper shape` (behavioural) and the source pins.
- I2 (integer version 1): I2a, I2b, I2c killed by `wrapper shape`.
- I3 (every section mandatory): (a)-(f) killed; (c) string-built key (I3c3) is killed by the behavioural SECTION `every section mandatory` alone (the supplement does not see it), as the invariant table requires; (e) killed by `sections order and count` and the source pins.
- I4 (five ids in order): I4swap killed only behaviourally (`sections order and count`); I4del and I4add killed; X7 duplicate id killed behaviourally.
- I5 (consumer iterates the wrapper): I5 is killed in the new test only by SECTION `source pins` (the behavioural sections cannot see the consumer; that is the stated D22 limit). The behavioural kill is the existing test (I5b): the bypass `ipairs(project(c))` yields nothing and `RA01-B2 orchestrator prompt carries five sections in order` fails. X2 (consumer skips the first section) is also killed only by that existing test.
- I8 (boundary): a 64-byte comment appended to semantic.lua is killed by `X08 C01 role preparation outputs-off boundary pins the request-record conversion charge` (carried-row C02-15c re-run). The boundary files are byte-identical to HEAD per REPORT.md; the pin test needs no re-pin.
- I7, I9, I10, I11 and G1-G9: not mutation rows (inventory, document and gate checks); not probed. R1: see the red-first table.

## SURVIVORS (finding)

I6join, I6title, I6text survived. Mutating the section join (`\n` to `\n\n`), the prompt title, and one sentence of the authority section all pass the named test `RA01-B2 orchestrator prompt carries five sections in order`, and pass the broad fallback (`ctest -R 'RA01|orchestrator|X08'`, 170 tests, `raw/I6join.log`). For I6join the full Debug suite was also run on the mutant (`raw/I6join-full.log`): 1437 of 1438 pass; the one failure, `ledger format initializes exact metadata schema and Glaze event` (format.t.cpp:115), also fails on the unmutated scratch baseline (`raw/FINAL-BASELINE.log`; unrelated to this round and specific to the scratch copy, whose absolute path differs from the checkout), so no test pins the join. `grep -rlE "Planar orchestrator prompt|actions marked as available|Orchestrator authority" src docs/vernacular/*.md tests` (evidence folders excluded) finds only orchestrator.lua itself. Conclusion: invariant I6 ("output bytes unchanged, existing goldens pass untouched") is true of the diff, but the repository has no byte golden for the orchestrator markdown: only the structural assertions of the B2 test (five sections, markers, envelope, trailing LF) guard it, so I6 is not mutation-enforced. This is not caused by this round (the wrapper change is covered by the I1-I5b rows); it is reported as a pre-existing coverage gap. Heading-marker, section-count, order and drop-section changes ARE caught (X1, I4*, X2).

## Red-first table (test added in this round lineage, one row)

| test | mutant that makes it fail | result | per-SECTION failures |
|---|---|---|---|
| `RA01-C-C01 orchestrator projection wrapper is versioned and mandatory` | PF: orchestrator.lua replaced by `git show HEAD:src/stdlib/planar/orchestrator.lua` (the pre-fix product, diff sha256 `14d584b23d83661e...`, `raw/PF.log`) | killed | wrapper shape, sections order and count, every section mandatory, source pins: all four fail (behavioural, no load error) |
| same | I1b (bare `return sections`), I2a, I3b, I4swap, I5 | each killed (see table) | one or more SECTIONs as listed |

## Extra exploratory mutants

Used 8 of 20 (X1-X7 plus PF counted separately as the red-first row; X1 heading marker, X2 consumer skips first, X3 delete one mandatory key, X4 array part, X5 version=true, X6 no sections, X7 duplicate id). Skipped (not needed, covered by an equivalent row): one-section removal of the flag by metatable, section text changes (I6 survivors show they are unpinned), consumer-side version check (no invariant requires it).

## Notes

- Scratch build state: `ledger format initializes exact metadata schema` fails on the unmutated scratch copy (path-length dependent); it is unrelated and passes in the checkout gate (REPORT.md 1438/1438).
- Final state of the scratch: orchestrator.lua sha256 `e7d9f570ef70b014e6b3facac55709c59ea8736958bb5ef7d5147eb227bc87a1` (equal to the checkout), planar_tests rebuilt.
