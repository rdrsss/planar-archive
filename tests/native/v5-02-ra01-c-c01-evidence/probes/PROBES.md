# V5-02-RA01-C-C01 mutation probes (focused re-probe)

Focused re-probe of 2026-11-01 after the survivor repair (new test T2 `RA01-C-C01 orchestrator markdown bytes keep the golden title authority and joins`, appended at authoring_orchestrator.t.cpp:895-913; the diff between the earlier probed test file and the current one is that single appended test, so T1 and every other test body are unchanged and their rows are carried). Previous PROBES.md, raw/ and results.jsonl moved to `superseded-1/`. Only tests/native/v5-02-ra01-c-c01-evidence/probes/ was written in the checkout.

Scratch: warm copy `probe-warm` (outside the checkout, debug preset, kept), re-synced with rsync (orchestrator.lua and authoring_orchestrator.t.cpp cmp-identical to the checkout). Baseline `ctest -R 'RA01-C-C01|RA01-B2 orchestrator prompt carries'` 3/3 passed before the first mutant. Driver `raw/driver/drv.py` (mutants inside; `raw/driver/run.out` is the machine output): exact-anchor patch (anchor once, else patch-failed), build `planar_command_tests`, `ctest -R '^replacement\.<name>$'`, byte-copy restore with sha256 verified (`restored_verified=true` in all 9 rows), rebuild after restore. Scratch orchestrator.lua sha256 after the run equals the checkout (`e7d9f570...87a1`).

## Candidate identity (identity.py, verbatim)

| identity | value |
|---|---|
| HEAD | `93053c42a4c68fd3ecc8911edab6464e3d1307f3` |
| sha256(git diff --binary HEAD -- src tests/native), tracked only | `b17fda64e25a9683559256259c5c88b19563798acdfcdcbb22da1b82ec97b843` |
| combined sha256 (diff bytes, then each untracked path NUL bytes, sorted), pathspec src tests/native, evidence excluded | `b17fda64e25a9683559256259c5c88b19563798acdfcdcbb22da1b82ec97b843` |

| file | state | sha256 |
|---|---|---|
| `src/cmd/planar/authoring_orchestrator.t.cpp` | modified | `38d79424ab104ac3be416588bcc58d33715976fd781a6128e173b51c6a06a46f` |
| `src/stdlib/planar/orchestrator.lua` | modified | `e7d9f570ef70b014e6b3facac55709c59ea8736958bb5ef7d5147eb227bc87a1` |


## Rows run in this re-probe (I6 and extra byte-golden mutants; all killed by T2, none by a different test)

| id | invariant | mutant | diff sha256 (16) | result | killing test | failing lines | run or carried |
|---|---|---|---|---|---|---|---|
| I6join | I6 | section join LF to LF LF | `1fa305e4b1e35679` | killed | T2 | 903,906,911 | run (`raw/I6join.log`) |
| I6title | I6 | title text gets `!` | `d1dc28851f11c977` | killed | T2 | 903 | run (`raw/I6title.log`) |
| I6text | I6 | one authority sentence reworded | `d797d16279830fbd` | killed | T2 | 906 | run (`raw/I6text.log`) |
| I6authority | I6x | see raw/I6authority.log | `a02a2707224ccad2` | killed | RA01-C-C01 orchestrator markdown bytes keep the golden title authority and joins | failing CHECK lines 906 | run (`raw/I6authority.log`) |
| I6roleshead | I6x | see raw/I6roleshead.log | `9a732b85c610f1ae` | killed | RA01-C-C01 orchestrator markdown bytes keep the golden title authority and joins | failing CHECK lines 907 | run (`raw/I6roleshead.log`) |
| I6actionshead | I6x | see raw/I6actionshead.log | `241f59c187270126` | killed | RA01-C-C01 orchestrator markdown bytes keep the golden title authority and joins | failing CHECK lines 909 | run (`raw/I6actionshead.log`) |
| I6prescrhead | I6x | see raw/I6prescrhead.log | `6fc0ce77daca6718` | killed | RA01-C-C01 orchestrator markdown bytes keep the golden title authority and joins | failing CHECK lines 908 | run (`raw/I6prescrhead.log`) |
| I6graphhead | I6x | see raw/I6graphhead.log | `3a6308c10fae0d29` | killed | RA01-C-C01 orchestrator markdown bytes keep the golden title authority and joins | failing CHECK lines 906 | run (`raw/I6graphhead.log`) |
| I6headtext | I6x | see raw/I6headtext.log | `0d32cb2bc56aa578` | killed | RA01-C-C01 orchestrator markdown bytes keep the golden title authority and joins | failing CHECK lines 903,906,907,908,909 | run (`raw/I6headtext.log`) |

Extra mutants used in this re-probe: 6 of 20 (I6authority drops the Planar-does-not clause; I6roleshead, I6actionshead, I6prescrhead, I6graphhead change one heading text; I6headtext adds a blank line after every heading). Skipped: none needed. The earlier survivors I6join, I6title, I6text are now KILLED, so the I6 survivor finding in superseded-1 is closed by T2. I6 remains structural-plus-prefix (T2 pins title, authority text, joins and headings, not the full text of roles, graph, prescriptions and actions bodies, which are data-derived); that is the intended limit.

## Red-first for T2

T2 pins bytes the fix does not change, so it is a guard that passes on the pre-fix product (not red on PF) and is red only under the I6 mutants above (each killed by T2 alone, failing CHECK at the quoted lines). Mutants that make it fail: I6join, I6title, I6text, I6authority, I6*head, I6headtext. T1 red-first (PF row, whole file restored to HEAD, killed, four SECTIONs) is carried below.

## Rows carried from superseded-1 (T1, B2 and X08 test bodies and orchestrator.lua unchanged by the repair; source hashes below in identity are the new test-file hash only)

| id | invariant | mutant | diff sha256 (16) | result | killing test | failing SECTIONs | run or carried |
|---|---|---|---|---|---|---|---|
| I1a | I1 | drop version | `cb5180b1d71060bf` | killed | RA01-C-C01 orchestrator projection wrapper is versioned and mandatory | source pins, wrapper shape | carried from superseded-1 (`superseded-1/raw/I1a.log`) |
| I1b | I1 | bare return sections | `b52527bef3f25638` | killed | RA01-C-C01 orchestrator projection wrapper is versioned and mandatory | every section mandatory, sections order and count, source pins, wrapper shape | carried from superseded-1 (`superseded-1/raw/I1b.log`) |
| I1c | I1 | extra key | `034231f18b382de2` | killed | RA01-C-C01 orchestrator projection wrapper is versioned and mandatory | source pins, wrapper shape | carried from superseded-1 (`superseded-1/raw/I1c.log`) |
| I1d | I1 | wrapper moved to consumer, project bare | `aa73581dbec3229a` | killed | RA01-C-C01 orchestrator projection wrapper is versioned and mandatory | every section mandatory, sections order and count, source pins, wrapper shape | carried from superseded-1 (`superseded-1/raw/I1d.log`) |
| I2a | I2 | version=2 | `59bdddfba1d3b48a` | killed | RA01-C-C01 orchestrator projection wrapper is versioned and mandatory | source pins, wrapper shape | carried from superseded-1 (`superseded-1/raw/I2a.log`) |
| I2b | I2 | version="1" | `f22565ef3579987d` | killed | RA01-C-C01 orchestrator projection wrapper is versioned and mandatory | source pins, wrapper shape | carried from superseded-1 (`superseded-1/raw/I2b.log`) |
| I2c | I2 | version=1.0 | `a786a1c86ca1aa9e` | killed | RA01-C-C01 orchestrator projection wrapper is versioned and mandatory | source pins, wrapper shape | carried from superseded-1 (`superseded-1/raw/I2c.log`) |
| I3a | I3 | drop mandatory from helper | `01685a7be4c05461` | killed | RA01-C-C01 orchestrator projection wrapper is versioned and mandatory | every section mandatory, source pins | carried from superseded-1 (`superseded-1/raw/I3a.log`) |
| I3b | I3 | helper mandatory=false | `6e7ead3c01a34ce8` | killed | RA01-C-C01 orchestrator projection wrapper is versioned and mandatory | every section mandatory, source pins | carried from superseded-1 (`superseded-1/raw/I3b.log`) |
| I3c1 | I3 | roles section .mandatory=false | `866766a0d0aa5a2d` | killed | RA01-C-C01 orchestrator projection wrapper is versioned and mandatory | every section mandatory, source pins | carried from superseded-1 (`superseded-1/raw/I3c1.log`) |
| I3c2 | I3 | roles ["mandatory"]=false | `e50cb090666e466f` | killed | RA01-C-C01 orchestrator projection wrapper is versioned and mandatory | every section mandatory, source pins | carried from superseded-1 (`superseded-1/raw/I3c2.log`) |
| I3c3 | I3 | roles string-built key=false | `93cb559d49053ca4` | killed | RA01-C-C01 orchestrator projection wrapper is versioned and mandatory | every section mandatory | carried from superseded-1 (`superseded-1/raw/I3c3.log`) |
| I3d | I3 | flag moved to wrapper | `c8b7b922d0791dd5` | killed | RA01-C-C01 orchestrator projection wrapper is versioned and mandatory | every section mandatory, source pins, wrapper shape | carried from superseded-1 (`superseded-1/raw/I3d.log`) |
| I3e | I3 | helper table.insert(sections,1,..) | `2601585d7c0db144` | killed | RA01-C-C01 orchestrator projection wrapper is versioned and mandatory | sections order and count, source pins | carried from superseded-1 (`superseded-1/raw/I3e.log`) |
| I3f | I3 | mandatory="true" | `42246017795898d1` | killed | RA01-C-C01 orchestrator projection wrapper is versioned and mandatory | every section mandatory, source pins | carried from superseded-1 (`superseded-1/raw/I3f.log`) |
| I4swap | I4 | swap sections 2,3 at runtime | `eca8d21247e644fd` | killed | RA01-C-C01 orchestrator projection wrapper is versioned and mandatory | sections order and count | carried from superseded-1 (`superseded-1/raw/I4swap.log`) |
| I4del | I4 | delete actions section | `1a7fe9c0300944d0` | killed | RA01-C-C01 orchestrator projection wrapper is versioned and mandatory | every section mandatory, sections order and count, source pins | carried from superseded-1 (`superseded-1/raw/I4del.log`) |
| I4add | I4 | add sixth section | `4b8604a1b90b8367` | killed | RA01-C-C01 orchestrator projection wrapper is versioned and mandatory | sections order and count, source pins | carried from superseded-1 (`superseded-1/raw/I4add.log`) |
| I5 | I5 | ipairs(project(c)) bypass | `fd003610602d4229` | killed | RA01-C-C01 orchestrator projection wrapper is versioned and mandatory | source pins | carried from superseded-1 (`superseded-1/raw/I5.log`) |
| I5b | I5b | same bypass, existing test | `fd003610602d4229` | killed | RA01-B2 orchestrator prompt carries five sections in order | - | carried from superseded-1 (`superseded-1/raw/I5b.log`) |
| I8 | I8 | 64B comment appended to semantic.lua | `73af78070e9a1d18` | killed | X08 C01 role preparation outputs-off boundary pins the request-record conversion charge | - | carried from superseded-1 (`superseded-1/raw/I8.log`) |
| X1heading | ext | heading marker '## ' -> '# ' | `d2191b159954b943` | killed | RA01-C-C01 orchestrator projection wrapper is versioned and mandatory | every section mandatory, source pins | carried from superseded-1 (`superseded-1/raw/X1heading.log`) |
| X2skipfirst | ext | consumer skips first section | `e32626ad49f4348b` | killed | RA01-B2 orchestrator prompt carries five sections in order | - | carried from superseded-1 (`superseded-1/raw/X2skipfirst.log`) |
| X3dropkey | ext | sections[1].mandatory=nil | `97a7b1fe39912b45` | killed | RA01-C-C01 orchestrator projection wrapper is versioned and mandatory | every section mandatory, source pins | carried from superseded-1 (`superseded-1/raw/X3dropkey.log`) |
| X4arraypart | ext | array part in wrapper | `69abf82b08420870` | killed | RA01-C-C01 orchestrator projection wrapper is versioned and mandatory | source pins, wrapper shape | carried from superseded-1 (`superseded-1/raw/X4arraypart.log`) |
| X5versionstr | ext | version=true | `47f26d08c1ffa6a3` | killed | RA01-C-C01 orchestrator projection wrapper is versioned and mandatory | source pins, wrapper shape | carried from superseded-1 (`superseded-1/raw/X5versionstr.log`) |
| X6nosections | ext | wrapper without sections | `15075563de02419e` | killed | RA01-C-C01 orchestrator projection wrapper is versioned and mandatory | every section mandatory, sections order and count, source pins, wrapper shape | carried from superseded-1 (`superseded-1/raw/X6nosections.log`) |
| X7dupid | ext | sections[5].id duplicated | `bf65a6464014ef2a` | killed | RA01-C-C01 orchestrator projection wrapper is versioned and mandatory | sections order and count | carried from superseded-1 (`superseded-1/raw/X7dupid.log`) |
| PF | R1 | whole file restored to HEAD (pre-fix) | `14d584b23d83661e` | killed | RA01-C-C01 orchestrator projection wrapper is versioned and mandatory | every section mandatory, sections order and count, source pins, wrapper shape | carried from superseded-1 (`superseded-1/raw/PF.log`) |

I6join, I6title and I6text appear in superseded-1 as survivors against the B2 test; they are replaced by the rows run above.

## Invariant coverage summary

I1-I5b, I8, R1: carried, all killed (I5 behaviourally by the B2 test, by T1 `source pins` in T1). I6: now mutation-enforced by T2, 9 of 9 killed. I7, I9-I11, G1-G9: not mutation rows. No survivors.
