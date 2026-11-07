# RED-CHECK V5-02-R15-A2-C02, attempt 3 (package P2, role test)

Base commit: `cdcfe829c466bf6338815ff6f5b6ea90ffddc570`. Placeholders: `<repo>` is the checkout root, `<scratch>` the
session scratch directory. This file and every other attempt-3 file are NEW. No existing file of `red/` was written:
`RED.md`, the unsuffixed `red-run.txt/.exit`, `stub-run.txt/.exit`, `snapshot-identity.txt`, `tests.txt` (the attempt-2 results)
and all `.attempt-1.` files are byte-equal to the before-inventory (`tree-verify.attempt-3.txt`, section (i): 923 files
equal, 0 differ, 0 missing). Nothing was deleted. Nothing was built; no mutant was applied (mutants belong to the prober).

## Checker identity (C02-D26, scoped exception)

Verbatim, as pasted at the top of `tree-verify.attempt-3.txt`:

```
$ cd <scratch>/checks-copy && shasum -a 256 contract_checks.py fixtures/*
cc248dd7a059049ec0e685666a06a619edabdf9b48d57c46d7ae451108e83092  contract_checks.py
a7d13d526610ddb9b89182feefad46e443fa980a17eb3956b0159cc864eddd9c  fixtures/absent.bad.txt
8075da1108f6738991be8e411d5d6ef9d86c57cccd877b634ba7da525dd45520  fixtures/absent.good.txt
0ee819c6db5fa0ba3d781f004dc9e494f7e8a9b01cd5cb0e45d889c5f5509d32  fixtures/allow.bad.txt
7c2edda7d3e254ab6294547bfcf10d0649cbac3449b8fed5cd7e3e56a79a5e61  fixtures/allow.good.txt
bdebab40f09db28c01a8b8b8c1cab4a528f497e659c36f6a73ecc2a9f7b45d6e  fixtures/allowed.bad.txt
04cdb3171caebd608221eee93ffcf7fbd7b2536b380a1660b5ced6354edd8610  fixtures/allowed.good.txt
9a11326b3bd05a0dc565887c17971e75c320418d91139373be75279f6c113fcb  fixtures/codes.bad.txt
2967f72c075306981b6152846b96a0553617eaa305956fea803385f3bed31889  fixtures/codes.good.txt
91d3863d9199673799bec1330a0c9fbe9e753a95813e7f8202d0ff5c85378dff  fixtures/exempt.bad.txt
87377775d816a88f0c6b1fe9e8d5a01ca5fce77d52ec3d6768498c5232c1a7b3  fixtures/exempt.good.txt
430268624e4902c334a18b722fbc0ae04c0bb7244620db972f390f6f607c30f7  fixtures/expectjob.bad.txt
8e9fe96a2e769990afa06a85a081185acba05cd7200569a5b7fb7aa91d64f545  fixtures/expectjob.good.txt
2060bb5092c84eb6757d78f012a495e208dbb55c5b412bcfa5c9cc751ac69dc3  fixtures/frozen.bad.txt
4c011d3915632a267b978b8afed07bb7aee24aa13c380f850d2123a4e6493446  fixtures/frozen.good.txt
140397e8609d4345c4b44d3da4709db626f6c5599889fc400f5cf188c9db7eb1  fixtures/headings.bad.txt
b605083c6d57e372f209ec89c83c3fce8ae2c5943ddb6b48f8fdadc55fde1a7d  fixtures/headings.good.txt
377c71835aad20ffdd4b5073b715b3fd59aebc31fb14f7006ed03235effe81c3  fixtures/lines.bad.txt
4066801f80cecb34db3a6cb29dfc8200e62b1496d4dcf50896ddb66865d2580e  fixtures/lines.good.txt
6d99d11ec522f5d8d54745143ed8175a88134eeb58c6bba10f054af7ac611bab  fixtures/link.bad.txt
a7d881777fe8471f6d0fae280ac6f88b40a2847426888940164938a434bbd752  fixtures/link.good.txt
787d02f7ba3f6c469b1859d438980745531f063b7e9c9df1f50f2fa39eb9df0f  fixtures/local.bad.txt
e250f227146148284a888751a120893795b3dc1ec212f04b6691176edd606445  fixtures/local.good.txt
a8cbf84b83145e9070ab56286977cb953abbdd8acda29bb844a88ced9707bac5  fixtures/messages_cells.bad.txt
6fb4deeefc57d358a29b49bd214c4a8c1fed26dec96841bfd46926e85515f30c  fixtures/messages_cells.good.txt
8dc0f2f21ef0f193e813d21a498e16fc100548d8095b8770997651e0c4a4fb9a  fixtures/messages_then.bad.txt
efff62a33a74e620a680231debd1a97f24c55136603e6d48ef3ae4b0995749b9  fixtures/messages_then.good.txt
0fee243dc9e72667dd928525167b6f5de0edc8d214366215934a35b3c30cfb77  fixtures/messages.bad.txt
6dd4a66cda47d7d18658e6e4fe9b87e805b781bb827f5a24083ed942d1740025  fixtures/messages.good.txt
7ccfb1d21261c84e49fc32571901f3e6e74ecabca377261dca5b5bd9d0f8c10d  fixtures/msg_table.bad.txt
e431302351bb23f81cb4c3b053601cee4a86cc8e197d16f6a88ac04720f64733  fixtures/msg_table.good.txt
a8f36fb6d0e1edbdfe488228c46118464ec49edba30588db5f42ff3939f6c346  fixtures/names.bad.txt
d47a82b1c2be0b8764be8083ce8f86549e51401eecbf554857e60e482f3161eb  fixtures/names.good.txt
4c8c3dc4b45942be1d44b2f665b4a98a46effee4e5bb8bea8374c94615f3f8d8  fixtures/one_change.bad.txt
2060bb5092c84eb6757d78f012a495e208dbb55c5b412bcfa5c9cc751ac69dc3  fixtures/one_change.good.txt
6ac1d2f2b7a96c8f52e2de04787189950e58a86767bd042052324565a6dbbf38  fixtures/ordered.bad.txt
7e17bbb5afd9acf459f00333da7b8b931f5ea0059eb21c9771d22927bd1ded87  fixtures/ordered.good.txt
b82607e5f2b55004617ad95e888551c8629537667d97c1b4fb8eeeab7dc33cf9  fixtures/paragraphs.bad.txt
20a6ff454f81cccf462c98e4599b16965355e1fe4c3f214563b122edd45b7486  fixtures/paragraphs.good.txt
05d917fec20815e252ed1ce707cfeb5f4ed1cae76f92d6bec7db383717c90455  fixtures/present.bad.txt
c3ed797d5dc2676bafb066d689207d649bd202e24c41cae129741daf1cb64279  fixtures/present.good.txt
0e4c4b09e05cc49a3dbf0b648233fa4325ec4738bc92ceb3aeb9c3b44ad97ab3  fixtures/raw.bad.txt
95d9178605f8536ed1f9befaf2e898401f0c02ff1647a4005e5fe470df8e07c8  fixtures/raw.good.txt
77c245b6a806174d04399357b95597158afa58deeae2eca3b33f808287e44f38  fixtures/removed.bad.txt
842717ba36b12972e2f504987b1536ebd665b7e9fe1ccdf284c12536ece47e03  fixtures/removed.good.txt
ac61db1fc41e128d2d02ad2cfea114bd032bcccdda63845f4af0116b3e2d8eac  fixtures/rows.bad.txt
7369ff973efa967251e2c99add906f4f4964ce6481cd16014b19a3b4d670837b  fixtures/rows.good.txt
ddb4b4d095ec7851fb894d0b1a7441b8490aeb3b4b9461f826f0f2ca3a484306  fixtures/tags.bad.txt
b933bcc058de456b00a86f0eb90dc66ec0e8f373365ea1f40c8183bb9b9873fd  fixtures/tags.good.txt
```

The scratch copy and the checkout `checks/` copy have equal hash lists (`cmp` exit 0, `tree-verify.attempt-3.txt`, section (0)).

## Attempt 3 (C02)

Step A: six NEW byte copies of the unsuffixed attempt-2 files were made first: `red-run.attempt-2.txt/.exit`,
`stub-run.attempt-2.txt/.exit`, `snapshot-identity.attempt-2.txt`, `tests.attempt-2.txt`. All six `cmp` exits are 0
(`tree-verify.attempt-3.txt`, section (c)).

Construction (as `RED.md` section 2): the attempt-3 checker and fixtures were copied to `<scratch>/checks-copy`, the ten
`base/` files to `<scratch>/redtree/base` and `<scratch>/redtree/docs/vernacular`, and the stub tree was built by
`python3 -I <scratch>/make-stub-tree.py <scratch>/redtree/base <scratch>/stubtree` run from a scratch copy of the script.

Commands (output `red/red-run.attempt-3.txt/.exit` and `red/stub-run.attempt-3.txt/.exit`):

```
cd <scratch>/checks-copy && python3 -I contract_checks.py --repo <scratch>/redtree --base <scratch>/redtree/base --git-repo <repo> --base-commit cdcfe829c466bf6338815ff6f5b6ea90ffddc570
cd <scratch>/checks-copy && python3 -I contract_checks.py --repo <scratch>/stubtree --base <scratch>/stubtree/base --git-repo <repo> --base-commit cdcfe829c466bf6338815ff6f5b6ea90ffddc570
```

Per-check logs (the same commands plus `--only NNN`, one invocation per check): `red/raw/checker.attempt-3/` and
`red/raw/stub.attempt-3/`, 123 logs each, same format as the attempt-2 logs. Their result lines equal the full-run lines (0 mismatches).

| run | output | exit | summary line | Traceback lines | PASS names |
| --- | --- | --- | --- | --- | --- |
| red (base bytes), attempt 3 | `red/red-run.attempt-3.txt` | 1 | `SUMMARY pass=8 fail=115` | 0 | 000 004 021 023 100 116 133 160 |
| stub (headings present), attempt 3 | `red/stub-run.attempt-3.txt` | 1 | `SUMMARY pass=10 fail=113` | 0 | 000 001 003 004 021 023 100 116 133 160 |

The FAIL and PASS name lists equal the attempt-2 lists on both trees (`comm -3` prints nothing, section (e)), so no count needed a stated
reason. `CHK-A2-000` PASSes on both; the scratch `--selftest` prints `SUMMARY pass=1 fail=0`.
`tests.attempt-3.txt` has the 108 lines (106 rows) and the columns of `tests.attempt-2.txt`; its cells come from the attempt-3 logs by script.
All 106 outcome cells equal the attempt-2 outcomes. Two reason cells differ (section (f)):

- `CHK-A2-025` (stub): the first missing literal reported is now the D152 retired-role sentence, not the `status_v1` shape literal that attempt 2 reported first.
  This is a change of the checker's check order for a check that was tightened; the outcome is still FAIL.
- `CHK-A2-044` (stub): the attempt-2 log and the attempt-3 log carry the same text. The attempt-2 `tests.txt` cell (file time 21:05) differs from the
  attempt-2 logs (22:32), so that cell was stale in the attempt-2 table; it is not a change of the checker. `tests.attempt-2.txt` is kept byte-equal to `tests.txt` as required.

### The six tightened checks and the unchanged ones (FAIL lines copied from the logs, section (g))

| check | base run | stub run | cause category |
| --- | --- | --- | --- |
| `CHK-A2-011 expect_job_spelling` | FAIL, `heading ### P2-1 Verbs and services occurs 0 times` | FAIL, `P2-1 activate option row: missing exact row or line` | missing heading (base); missing exact row (stub) |
| `CHK-A2-016 exempt_reads` | FAIL, same heading | FAIL, `P2-1 exempt reads: missing literal` | missing heading; missing literal |
| `CHK-A2-022 no_new_message` | FAIL, `heading ## Part 2: ... occurs 0 times` | FAIL, `P2-1: missing literal: No twentieth message` | missing heading; missing literal |
| `CHK-A2-025 status_v1_schema` | FAIL, same heading as 011 | FAIL, `P2-1 status_v1: missing literal: A role that status_v1 lists as retired ...` | missing heading; missing literal |
| `CHK-A2-084 orch_ra7_pointer` | FAIL, `heading ### P2-6 Orchestrator live form occurs 0 times` | FAIL, `P2-6 RA-7 pointer: missing literal` | missing heading; missing literal |
| `CHK-A2-098 lg_selector_row` | FAIL, `ledger-contract pointer: missing literal: the command-surface row text ...` | FAIL, same | missing literal on both |
| `CHK-A2-092 cc_services_rows` (unchanged) | FAIL, `command-contract services: missing literal` | FAIL, same | keeps its attempt-2 verdict |
| `CHK-A2-000 selftest_positive_negative` | PASS | PASS | the selftest does not read contract text |

Every FAIL is a missing heading, row or literal, with no Traceback and no `internal error`.

### Limit for unchanged claims

The tightened branches (exact-list comparison in 016, spelling in 011, widened extraction in 022, retired-role clause in 025,
RA-7 sentences in 084, selector sentences in 098) are NOT shown to be reached by the base or the stub run: both stop on a missing heading
or an earlier missing literal, so the attempt-3 red set equals the attempt-2 red set and shows no new cause. These runs show only that the six
checks still go red without the contract text and do not crash. That each tightened check fails under its mutant, and that the attempt-2 checker lets
the same mutants survive, is shown only by the prober's runs (`probes/PROBES.md`, `probes/mutants.tsv`) and is not claimed here. No kill is claimed here.
`CHK-A2-092` (row I63) is unchanged and keeps its attempt-2 verdict; no new claim is made for it.

## Citation key for RED.md

`RED.md` is not edited (C02-D10, C02-D25). The A22 observer was run read-only on it; its exact output is in `tree-verify.attempt-3.txt`, section (j).
The unsuffixed names (`red-run.txt/.exit`, `stub-run.txt/.exit`, `snapshot-identity.txt`, `tests.txt`) denote the attempt-2 results, byte-equal to the
`.attempt-2.` copies. Figures of attempt 1 (`pass=7 fail=99`, `pass=9 fail=97`) denote the `.attempt-1.` files. Attempt 3 has the same figures as attempt 2
(`pass=8 fail=115`, `pass=10 fail=113`) in the `.attempt-3.` files.

| RED.md line | text in RED.md (cut) | file it truly denotes |
| --- | --- | --- |
| :6 | "`snapshot-identity.txt` rewritten after an attempt-1 copy" | `snapshot-identity.txt` is the attempt-2 file (= `snapshot-identity.attempt-2.txt`); the attempt-1 copy is `snapshot-identity.attempt-1.txt` |
| :15 | "re-run trees are verified in `red/tree-verify.attempt-2.txt`" | attempt-2 tree check (already suffixed); the attempt-3 check is `tree-verify.attempt-3.txt` |
| :19 | "`red/snapshot-identity.txt`. The earlier file is kept as `red/snapshot-identity.attempt-1.txt`" | `snapshot-identity.txt` = attempt 2; `.attempt-1.` = attempt 1 |
| :42 | "output `red/red-run.txt`, exit `red/red-run.exit`" | attempt-2 base run (= `red-run.attempt-2.txt/.exit`) |
| :48 | "output `red/stub-run.txt`, exit `red/stub-run.exit`" | attempt-2 stub run (= `stub-run.attempt-2.txt/.exit`) |
| :55 | "`red/raw/checker.attempt-2/` ... `red/raw/stub.attempt-2/`" | attempt-2 per-check logs (already suffixed) |
| :59 | table header "run / output / exit / summary line / Traceback lines" (no attempt label) | the table is the attempt-2 results (rows :61 and :62) |
| :61 | "`red/red-run.txt` ... `SUMMARY pass=8 fail=115`" | attempt-2 base run (`red-run.attempt-2.txt`); attempt 3: `red-run.attempt-3.txt` |
| :62 | "`red/stub-run.txt` ... `SUMMARY pass=10 fail=113`" | attempt-2 stub run (`stub-run.attempt-2.txt`); attempt 3: `stub-run.attempt-3.txt` |
| :64 | "All 123 checks ran in each tree. Each per-check log ..." | attempt-2 logs (`raw/checker.attempt-2/`, `raw/stub.attempt-2/`) |
| :68 | "`red/red-run.attempt-1.txt` and `.exit` (byte-copies of the earlier `red-run.txt` and `.exit` ...)" | attempt-1 files; "the earlier `red-run.txt`" means the file as it was before the attempt-2 rewrite, now `red-run.attempt-1.txt` |
| :69 | "`SUMMARY pass=7 fail=99`" | `red-run.attempt-1.txt` |
| :70 | "`red/stub-run.attempt-1.txt` and `.exit` (`SUMMARY pass=9 fail=97`)" | `stub-run.attempt-1.txt` |
| :83 | table header "red run (`red-run.txt`) / stub run (`stub-run.txt`)" (no attempt label) | the attempt-2 runs (`red-run.attempt-2.txt`, `stub-run.attempt-2.txt`); the counts under it are attempt-2 counts |
| :107 | "PASS lines of the red run (8): ..." | `red-run.attempt-2.txt` (attempt 3 has the same eight) |
| :125 | "Computed from `red/stub-run.txt` (10 PASS lines: `000 001 003 004 021 023 100 116 133 160`)" | `stub-run.attempt-2.txt`; the "10 PASS lines" statement is the attempt-2 figure (attempt 3: `stub-run.attempt-3.txt`, the same ten) |
| :137 | "The previous stub run (attempt 1) also passed these two checks" | `stub-run.attempt-1.txt` |
| :142 | "The red-run FAIL set (115)" | `red-run.attempt-2.txt` |
| :158-159 | "Kept, not overwritten: `red-run.attempt-1.txt` ... `snapshot-identity.attempt-1.txt`" | attempt-1 files (already suffixed) |
| :161-167 | "attempt-1 `CHK-A2-022` extraction defect ... The attempt-2 runs use the repaired checker" | attempt-1 and attempt-2 labels, correct as written |
| :171-179 | "New folders: `red/raw/checker.attempt-2/` ..." | attempt-2 logs (already suffixed) |
| :187 | "Section (b) of `tree-verify.attempt-2.txt`" | attempt-2 tree check (already suffixed) |

The list is the union of the three observer parts of A22 (section (j) of `tree-verify.attempt-3.txt`, plus the broader read aid (3b)) and a manual
read of the table headers. Observer part (1) matched lines :6, :19, :61, :62, :68, :125; part (2) matched :137; the manual read added :59, :83, :107, :142
and the already-suffixed lines.

## Files written by this package

All under `tests/native/v5-02-r15-a2-evidence/red/`: `red-run.attempt-2.txt`, `red-run.attempt-2.exit`, `stub-run.attempt-2.txt`,
`stub-run.attempt-2.exit`, `snapshot-identity.attempt-2.txt`, `tests.attempt-2.txt`, `red-run.attempt-3.txt`, `red-run.attempt-3.exit`,
`stub-run.attempt-3.txt`, `stub-run.attempt-3.exit`, `snapshot-identity.attempt-3.txt`, `tests.attempt-3.txt`, `tree-verify.attempt-3.txt`,
`RED.attempt-3.md`, `raw/checker.attempt-3/` (123 logs), `raw/stub.attempt-3/` (123 logs).

A re-dispatch (C02-D26) writes `*.attempt-4.*` and `RED.attempt-4.md` and marks attempt 3 VOID.
