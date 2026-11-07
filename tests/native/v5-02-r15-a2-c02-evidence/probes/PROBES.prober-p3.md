# Probes for V5-02-R15-A2-C02 (mutation prober, package P3)

Prober run, 2026-11-07. Observer: the attempt-3 `contract_checks.py` (package P1 output) and the pre-fix attempt-2 copy (`probes/checker-attempt-2/`), each a byte copy in its own scratch folder, run as `python3 -I -B <scratch>/chkN/contract_checks.py --repo <scratch>/pw --base <scratch>/pw/tests/native/v5-02-r15-a2-evidence/base --git-repo <repo> --base-commit cdcfe829c466bf6338815ff6f5b6ea90ffddc570` (N = 2 or 3; `perl -e 'alarm 600; exec @ARGV'` as the timeout, since `timeout` is not installed). No product build is involved. P3 is the only agent that applied mutants; nothing was applied in the checkout.

Method (same as `tests/native/v5-02-r15-a2-c01-evidence/probes/PROBES.md`, extended by C02-D12, C02-D14 and C02-D26):

- Scratch clone `<scratch>/pw`: `rsync -a --exclude=build --exclude=.git` of `docs/` and `tests/native/v5-02-r15-a2-evidence/` only (the checker reads nothing else; stated limit: the rest of the working tree, mostly `build/`, was not copied). `<scratch>/chk2` is a `cmp`-identical copy of `probes/checker-attempt-2/`; `<scratch>/chk3` is a `diff -r`-identical copy of the checkout `checks/` folder.
- For each mutant the exact OLD to NEW replace of `plan/INVARIANTS.md` was applied to the named scratch file only if the occurrence count was exactly 1 (counted inside the cited line where the row limits the replace to a line, otherwise in the whole file; both counts are in each log). The unified diff and its sha256 are in the log; the run is chained after the replace; the file is restored by byte copy and the restored sha256 is compared with the original (`match=True` in every log).
- Verdicts per C02-D14: KILLED (exit not 0 and a `FAIL` line names the row's check), KILLED-OTHER (exit not 0, only other checks fail), SURVIVED (exit 0, or the named check passes), EMPTY (empty diff), PASS (unmutated control), NOT RUN (no `SUMMARY` line or the saved diff could not be applied). An observed verdict or failing set that differs from the expected one is listed under Deviations and is never relabelled.
- Raw logs are `probes/raw/<id>.<attempt2|attempt3>.log` (not staged); `mutants.tsv` has the C01 columns (`verdict` is the observed verdict) plus `checker`, `attempt`, `expected_verdict`, `observed_verdict`, `expected_failing_set`, `set_check` and `deviation`.

Limits:

- The regression replay (section 4) covers the rows of the C01 table whose expected killer or killing check, or one of whose other failing checks, is CHK-A2-000, 011, 016, 022, 025, 084, 092, 098 or 160 (the changed checks and the extraction helpers they use), not all 157 C01 mutants. Rows are replayed from the saved diffs by line number and old-line match; a diff that no longer matches the attempt-3 checker bytes is NOT RUN.
- Section D mutants (K80 to K83) make the named helper permissive in the scratch attempt-3 checker (whole body replaced by one `return` line, signature kept); the plan's helper names exist in the attempt-3 checker and were used unchanged. The attempt-2 column is n/a (the helpers do not exist there).
- The workflow rows A01 to A26 of `plan/INVARIANTS.md` section E have no mutant applied by the prober.
- The attempt-3 checker hashes below are those of the checker present when the runs started; P1's own reported hashes were not part of this brief, so equality with them is asserted by P4 and P6 (A23) and not by this file. The checkout checker was compared with `diff -r` against the scratch copy after the last run: identical.

## Positive controls

| checker | log | result |
| --- | --- | --- |
| attempt-3 | `raw/baseline.attempt3.log` | `SUMMARY pass=123 fail=0`, exit 0, 0 Traceback |
| attempt-2 | `raw/baseline.attempt2.log` | `SUMMARY pass=123 fail=0`, exit 0, 0 Traceback |

K22 (the unmutated control that reads CHK-A2-022 and CHK-A2-160 on the widened extraction) PASSes both checks on both checkers; see the table.

## Checker identity (C02-D26)

Verbatim `shasum -a 256 contract_checks.py fixtures/*` run in each scratch checker folder (the folder name is the first line of each block):

```
chk2
6283fdd174af7b4601991d3f9f8bf22c05c3dc6bd67b31b57a9824860ea52272  contract_checks.py
a7d13d526610ddb9b89182feefad46e443fa980a17eb3956b0159cc864eddd9c  fixtures/absent.bad.txt
8075da1108f6738991be8e411d5d6ef9d86c57cccd877b634ba7da525dd45520  fixtures/absent.good.txt
0ee819c6db5fa0ba3d781f004dc9e494f7e8a9b01cd5cb0e45d889c5f5509d32  fixtures/allow.bad.txt
7c2edda7d3e254ab6294547bfcf10d0649cbac3449b8fed5cd7e3e56a79a5e61  fixtures/allow.good.txt
bdebab40f09db28c01a8b8b8c1cab4a528f497e659c36f6a73ecc2a9f7b45d6e  fixtures/allowed.bad.txt
04cdb3171caebd608221eee93ffcf7fbd7b2536b380a1660b5ced6354edd8610  fixtures/allowed.good.txt
9a11326b3bd05a0dc565887c17971e75c320418d91139373be75279f6c113fcb  fixtures/codes.bad.txt
2967f72c075306981b6152846b96a0553617eaa305956fea803385f3bed31889  fixtures/codes.good.txt
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
chk3
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

## Identity (workflow `identity.py` output, verbatim, seven contract paths)

| identity | value |
|---|---|
| HEAD | `cdcfe829c466bf6338815ff6f5b6ea90ffddc570` |
| sha256(git diff --binary HEAD -- docs/vernacular/command-contract.md docs/vernacular/execution-contract.md docs/vernacular/execution-design.md docs/vernacular/external-profile-contract.md docs/vernacular/ledger-contract.md docs/vernacular/load-command-contract.md docs/vernacular/render-contract.md), tracked only | `312fabfc4520d7670aa085a70508672b0232a514b32d3d8ceaed426c47b7c08f` |
| combined sha256 (diff bytes, then each untracked path NUL bytes, sorted), pathspec docs/vernacular/command-contract.md docs/vernacular/execution-contract.md docs/vernacular/execution-design.md docs/vernacular/external-profile-contract.md docs/vernacular/ledger-contract.md docs/vernacular/load-command-contract.md docs/vernacular/render-contract.md, evidence excluded | `312fabfc4520d7670aa085a70508672b0232a514b32d3d8ceaed426c47b7c08f` |

| file | state | sha256 |
|---|---|---|
| `docs/vernacular/command-contract.md` | modified | `34ae9d4010f8e13a4183ce452d18395be9eb7f0f95d54eb3bc7aad34d48d4e8a` |
| `docs/vernacular/execution-contract.md` | modified | `38c0d3f70afe6412dc7b7ffe90270bb956562752493c690f0b740714f9176d75` |
| `docs/vernacular/execution-design.md` | modified | `1a5134a0815bb6a18bcb069b96eed94bd28ce247db88b5e4cf54c798c612cb87` |
| `docs/vernacular/external-profile-contract.md` | modified | `86fcbefbf070293b4da7f7efbab1dd7b6065b6d02abc0df7e4bcfa5199f0c493` |
| `docs/vernacular/ledger-contract.md` | modified | `46d8a0eef9f5a65822efbeea1a6e5b61dbd14f3b0adf3e0c01ade94112f4a84f` |
| `docs/vernacular/load-command-contract.md` | modified | `af99144c596184c3ec90a60f3f773fff8509401e14f76502f49b59684e70ee22` |
| `docs/vernacular/render-contract.md` | modified | `22a702eb57177f0c4a3d9b52ca3eb43cf9533a09d63c8586b3c3292d1e2ca874` |

The seven contract hashes equal the C01 Identity per-file values, so the mutants were applied to the same contract bytes as C01.

## Result

Rows of sections A, B and C: 7 + 19 + 4 = 30, each run against both checkers (60 verdicts, of which 2 are EMPTY by construction for I63-a1 and 2 are the K22 control). Section D: 4 rows (attempt-3 only). Regression replay: 16 rows (attempt-3 only). `mutants.tsv` has 80 data rows (30 x 2 + 4 + 16).

| verdict | attempt-2 checker (30 rows) | attempt-3 checker (30 rows) |
| --- | --- | --- |
| KILLED | 6 | 28 |
| KILLED-OTHER | 6 | 0 |
| SURVIVED | 16 | 0 |
| EMPTY | 1 | 1 |
| PASS | 1 | 1 |
| NOT RUN | 0 | 0 |
| total | 30 | 30 |

Attempt-2 (pre-fix) checker: the 16 SURVIVED and 6 KILLED-OTHER rows are the red-first evidence for the new checker code; 6 rows are guards KILLED by the existing checks (K06, K07, I63-m1 to m4). Attempt-3 checker: every killable row of sections A to C is KILLED by the named check; no survivor.

Section D (selftest observability, attempt-3 checker only): 4 of 4 KILLED by CHK-A2-000 (full run and `--selftest` both exit 1 and fail only CHK-A2-000).

Regression replay (attempt-3 checker): 16 rows, KILLED 14, NOT RUN 2 (I129 and I132, see Deviations).

## 1. Sections A, B and C: every row, expected and observed verdict per checker

"Failing" lists the checks that FAIL (numbers are CHK-A2-NNN). "Set" compares the failing set with the plan's stated set where the plan states one (attempt-2 KILLED-OTHER rows: exact; attempt-3 rows with "still failing" checks: all must fail).

| Row | Named check | Checker | Expected | Observed | Failing | Set | Log |
| --- | --- | --- | --- | --- | --- | --- | --- |
| I07-a1 | CHK-A2-011 | attempt-2 | SURVIVED | SURVIVED | - | - | `raw/I07-a1.attempt2.log` |
| I07-a1 | CHK-A2-011 | attempt-3 | KILLED | KILLED | 011 | - | `raw/I07-a1.attempt3.log` |
| I12-a1 | CHK-A2-016 | attempt-2 | SURVIVED | SURVIVED | - | - | `raw/I12-a1.attempt2.log` |
| I12-a1 | CHK-A2-016 | attempt-3 | KILLED | KILLED | 016 | - | `raw/I12-a1.attempt3.log` |
| I18-a1 | CHK-A2-022 | attempt-2 | KILLED-OTHER | KILLED-OTHER | 017 | MATCH | `raw/I18-a1.attempt2.log` |
| I18-a1 | CHK-A2-022 | attempt-3 | KILLED | KILLED | 017,022 | - | `raw/I18-a1.attempt3.log` |
| I21-a1 | CHK-A2-025 | attempt-2 | SURVIVED | SURVIVED | - | - | `raw/I21-a1.attempt2.log` |
| I21-a1 | CHK-A2-025 | attempt-3 | KILLED | KILLED | 025 | - | `raw/I21-a1.attempt3.log` |
| I60-a1 | CHK-A2-084 | attempt-2 | SURVIVED | SURVIVED | - | - | `raw/I60-a1.attempt2.log` |
| I60-a1 | CHK-A2-084 | attempt-3 | KILLED | KILLED | 084 | - | `raw/I60-a1.attempt3.log` |
| I69-a1 | CHK-A2-098 | attempt-2 | SURVIVED | SURVIVED | - | - | `raw/I69-a1.attempt2.log` |
| I69-a1 | CHK-A2-098 | attempt-3 | KILLED | KILLED | 098 | - | `raw/I69-a1.attempt3.log` |
| I63-a1 | CHK-A2-092 | attempt-2 | EMPTY | EMPTY | - | - | `raw/I63-a1.attempt2.log` |
| I63-a1 | CHK-A2-092 | attempt-3 | EMPTY | EMPTY | - | - | `raw/I63-a1.attempt3.log` |
| K01 | CHK-A2-016 | attempt-2 | SURVIVED | SURVIVED | - | - | `raw/K01.attempt2.log` |
| K01 | CHK-A2-016 | attempt-3 | KILLED | KILLED | 016 | - | `raw/K01.attempt3.log` |
| K02 | CHK-A2-016 | attempt-2 | SURVIVED | SURVIVED | - | - | `raw/K02.attempt2.log` |
| K02 | CHK-A2-016 | attempt-3 | KILLED | KILLED | 016 | - | `raw/K02.attempt3.log` |
| K03 | CHK-A2-016 | attempt-2 | SURVIVED | SURVIVED | - | - | `raw/K03.attempt2.log` |
| K03 | CHK-A2-016 | attempt-3 | KILLED | KILLED | 016 | - | `raw/K03.attempt3.log` |
| K04 | CHK-A2-016 | attempt-2 | SURVIVED | SURVIVED | - | - | `raw/K04.attempt2.log` |
| K04 | CHK-A2-016 | attempt-3 | KILLED | KILLED | 016 | - | `raw/K04.attempt3.log` |
| K05 | CHK-A2-016 | attempt-2 | SURVIVED | SURVIVED | - | - | `raw/K05.attempt2.log` |
| K05 | CHK-A2-016 | attempt-3 | KILLED | KILLED | 016 | - | `raw/K05.attempt3.log` |
| K06 | CHK-A2-016 | attempt-2 | KILLED | KILLED | 016 | - | `raw/K06.attempt2.log` |
| K06 | CHK-A2-016 | attempt-3 | KILLED | KILLED | 016 | - | `raw/K06.attempt3.log` |
| K07 | CHK-A2-016 | attempt-2 | KILLED | KILLED | 016 | - | `raw/K07.attempt2.log` |
| K07 | CHK-A2-016 | attempt-3 | KILLED | KILLED | 016 | - | `raw/K07.attempt3.log` |
| K10 | CHK-A2-011 | attempt-2 | SURVIVED | SURVIVED | - | - | `raw/K10.attempt2.log` |
| K10 | CHK-A2-011 | attempt-3 | KILLED | KILLED | 011 | - | `raw/K10.attempt3.log` |
| K11 | CHK-A2-011 | attempt-2 | SURVIVED | SURVIVED | - | - | `raw/K11.attempt2.log` |
| K11 | CHK-A2-011 | attempt-3 | KILLED | KILLED | 011 | - | `raw/K11.attempt3.log` |
| K12 | CHK-A2-011 | attempt-2 | SURVIVED | SURVIVED | - | - | `raw/K12.attempt2.log` |
| K12 | CHK-A2-011 | attempt-3 | KILLED | KILLED | 011 | - | `raw/K12.attempt3.log` |
| K13 | CHK-A2-011 | attempt-2 | KILLED-OTHER | KILLED-OTHER | 137 | DIFFERS | `raw/K13.attempt2.log` |
| K13 | CHK-A2-011 | attempt-3 | KILLED | KILLED | 011,137 | DIFFERS | `raw/K13.attempt3.log` |
| K20 | CHK-A2-022 | attempt-2 | KILLED-OTHER | KILLED-OTHER | 017 | MATCH | `raw/K20.attempt2.log` |
| K20 | CHK-A2-022 | attempt-3 | KILLED | KILLED | 017,022 | MATCH | `raw/K20.attempt3.log` |
| K21 | CHK-A2-022 | attempt-2 | KILLED-OTHER | KILLED-OTHER | 134 | MATCH | `raw/K21.attempt2.log` |
| K21 | CHK-A2-022 | attempt-3 | KILLED | KILLED | 022,134 | MATCH | `raw/K21.attempt3.log` |
| K22 | CHK-A2-022 | attempt-2 | PASS | PASS | - | - | `raw/K22.attempt2.log` |
| K22 | CHK-A2-022 | attempt-3 | PASS | PASS | - | - | `raw/K22.attempt3.log` |
| K30 | CHK-A2-025 | attempt-2 | KILLED-OTHER | KILLED-OTHER | 141 | MATCH | `raw/K30.attempt2.log` |
| K30 | CHK-A2-025 | attempt-3 | KILLED | KILLED | 025,141 | MATCH | `raw/K30.attempt3.log` |
| K31 | CHK-A2-025 | attempt-2 | KILLED-OTHER | KILLED-OTHER | 141 | MATCH | `raw/K31.attempt2.log` |
| K31 | CHK-A2-025 | attempt-3 | KILLED | KILLED | 025,141 | MATCH | `raw/K31.attempt3.log` |
| K40 | CHK-A2-084 | attempt-2 | SURVIVED | SURVIVED | - | - | `raw/K40.attempt2.log` |
| K40 | CHK-A2-084 | attempt-3 | KILLED | KILLED | 084 | - | `raw/K40.attempt3.log` |
| K50 | CHK-A2-098 | attempt-2 | SURVIVED | SURVIVED | - | - | `raw/K50.attempt2.log` |
| K50 | CHK-A2-098 | attempt-3 | KILLED | KILLED | 098 | - | `raw/K50.attempt3.log` |
| K51 | CHK-A2-098 | attempt-2 | SURVIVED | SURVIVED | - | - | `raw/K51.attempt2.log` |
| K51 | CHK-A2-098 | attempt-3 | KILLED | KILLED | 098 | - | `raw/K51.attempt3.log` |
| I63-m1 | CHK-A2-092 | attempt-2 | KILLED | KILLED | 092 | - | `raw/I63-m1.attempt2.log` |
| I63-m1 | CHK-A2-092 | attempt-3 | KILLED | KILLED | 092 | - | `raw/I63-m1.attempt3.log` |
| I63-m2 | CHK-A2-092 | attempt-2 | KILLED | KILLED | 092 | - | `raw/I63-m2.attempt2.log` |
| I63-m2 | CHK-A2-092 | attempt-3 | KILLED | KILLED | 092 | - | `raw/I63-m2.attempt3.log` |
| I63-m3 | CHK-A2-092 | attempt-2 | KILLED | KILLED | 092 | - | `raw/I63-m3.attempt2.log` |
| I63-m3 | CHK-A2-092 | attempt-3 | KILLED | KILLED | 092 | - | `raw/I63-m3.attempt3.log` |
| I63-m4 | CHK-A2-092 | attempt-2 | KILLED | KILLED | 092,131 | - | `raw/I63-m4.attempt2.log` |
| I63-m4 | CHK-A2-092 | attempt-3 | KILLED | KILLED | 092,131 | - | `raw/I63-m4.attempt3.log` |

## 2. Section D: checker-side mutants (attempt-3 checker)

| Row | Helper made permissive | Expected | Observed | Failing (full run) | `--selftest` | Log |
| --- | --- | --- | --- | --- | --- | --- |
| K80 | prim_exempt_reads (body `return`) | KILLED | KILLED | CHK-A2-000 | selftest exit 1 fails CHK-A2-000 | `raw/K80.attempt3.log` |
| K81 | prim_expect_job_spelling (body `return`) | KILLED | KILLED | CHK-A2-000 | selftest exit 1 fails CHK-A2-000 | `raw/K81.attempt3.log` |
| K82 | then_messages (body `return set()`) | KILLED | KILLED | CHK-A2-000 | selftest exit 1 fails CHK-A2-000 | `raw/K82.attempt3.log` |
| K83 | table_message_cells (body `return set()`) | KILLED | KILLED | CHK-A2-000 | selftest exit 1 fails CHK-A2-000 | `raw/K83.attempt3.log` |

The four helper names are those of the plan; each was found in the attempt-3 checker under that name. attempt-2 column: n/a.

## 3. Before and after for the seven first attempts

The first attempts (C01 `probes/raw/<id>.attempt-1.log`) were reported in C01 as "mutant-fidelity defects of the finisher". Replayed here with the same OLD/NEW on the pre-fix checker they are survivors of it (I18: killed only by another check; I63: no mutant exists), and the attempt-3 checker kills each.

| Row | Pre-fix (attempt-2) checker | Attempt-3 checker |
| --- | --- | --- |
| I07 | SURVIVED | KILLED by 011 |
| I12 | SURVIVED | KILLED by 016 |
| I18 | KILLED-OTHER (only 017) | KILLED by 017,022 |
| I21 | SURVIVED | KILLED by 025 |
| I60 | SURVIVED | KILLED by 084 |
| I63 | EMPTY (empty diff, no mutant) | EMPTY (empty diff, no mutant) |
| I69 | SURVIVED | KILLED by 098 |

## 4. Regression replay of C01 rows against the attempt-3 checker

Saved diffs of `tests/native/v5-02-r15-a2-c01-evidence/probes/raw/<id>.log` applied to the scratch attempt-3 checker folder and clone; expected: the killer recorded by C01 still kills. "C01 set" is the killing check plus the other failing checks recorded in C01.

| Row | Expected killer | Observed | C01 set | Attempt-3 set | Note |
| --- | --- | --- | --- | --- | --- |
| I06 | 010 | KILLED | 010,011 | 010,011 | same set |
| I07 | 011 | KILLED | 010,011 | 010,011 | same set |
| I12 | 016 | KILLED | 016 | 016 | same set |
| I18 | 022 | KILLED | 017,019,022 | 017,019,022 | same set |
| I21 | 025 | KILLED | 025 | 025 | same set |
| I60 | 084 | KILLED | 084 | 084 | same set |
| I63 | 092 | KILLED | 092 | 092 | same set |
| I69 | 098 | KILLED | 098 | 098 | same set |
| I82 | 000 | KILLED | 000 | 000 | same set |
| I98 | 131 | KILLED | 092,131 | 092,131 | same set |
| I100 | 133 | KILLED | 022,084,133,160 | 022,084,133 | failing set c01=CHK-A2-022,CHK-A2-084,CHK-A2-133,CHK-A2-160 observed=CHK-A2-022,CHK-A2-084,CHK-A2-133 |
| I129 | 160 | NOT RUN | 000,022,160 | - | verdict expected KILLED observed NOT RUN (saved diff does not apply: hunk at CHK:639 does not match current bytes) |
| I130 | 022 | KILLED | 017,022 | 017,022 | same set |
| I131 | 022 | KILLED | 021,022,084,133,160 | 021,022,084,160,133 | same set |
| I132 | 000 | NOT RUN | 000,022,160 | - | verdict expected KILLED observed NOT RUN (saved diff does not apply: hunk at CHK:639 does not match current bytes) |
| X01 | 151 | KILLED | 011,151 | 011,151 | same set |

## Survivors

Survivors and KILLED-OTHER rows of the PRE-FIX (attempt-2) checker. They are survivors of the named check, not prober defects; each is red-first evidence for the attempt-3 branch that now kills it.

| Row | Pre-fix verdict | Named check | Cause | Other killers (observed) |
| --- | --- | --- | --- | --- |
| I07-a1 | SURVIVED | 011 | the pre-fix checker does not fail the named check on this edit (observed); the plan assigns the new branch to P1 | none |
| I12-a1 | SURVIVED | 016 | the pre-fix checker does not fail the named check on this edit (observed); the plan assigns the new branch to P1 | none |
| I18-a1 | KILLED-OTHER | 022 | the named check CHK-A2-022 passes; only CHK-A2-017 (a pinned literal) fails | 017 |
| I21-a1 | SURVIVED | 025 | the pre-fix checker does not fail the named check on this edit (observed); the plan assigns the new branch to P1 | none |
| I60-a1 | SURVIVED | 084 | the pre-fix checker does not fail the named check on this edit (observed); the plan assigns the new branch to P1 | none |
| I69-a1 | SURVIVED | 098 | the pre-fix checker does not fail the named check on this edit (observed); the plan assigns the new branch to P1 | none |
| K01 | SURVIVED | 016 | the pre-fix checker does not fail the named check on this edit (observed); the plan assigns the new branch to P1 | none |
| K02 | SURVIVED | 016 | the pre-fix checker does not fail the named check on this edit (observed); the plan assigns the new branch to P1 | none |
| K03 | SURVIVED | 016 | the pre-fix checker does not fail the named check on this edit (observed); the plan assigns the new branch to P1 | none |
| K04 | SURVIVED | 016 | the pre-fix checker does not fail the named check on this edit (observed); the plan assigns the new branch to P1 | none |
| K05 | SURVIVED | 016 | the pre-fix checker does not fail the named check on this edit (observed); the plan assigns the new branch to P1 | none |
| K10 | SURVIVED | 011 | the pre-fix checker does not fail the named check on this edit (observed); the plan assigns the new branch to P1 | none |
| K11 | SURVIVED | 011 | the pre-fix checker does not fail the named check on this edit (observed); the plan assigns the new branch to P1 | none |
| K12 | SURVIVED | 011 | the pre-fix checker does not fail the named check on this edit (observed); the plan assigns the new branch to P1 | none |
| K13 | KILLED-OTHER | 011 | the named check CHK-A2-011 passes; only CHK-A2-137 (a pinned literal) fails | 137 |
| K20 | KILLED-OTHER | 022 | the named check CHK-A2-022 passes; only CHK-A2-017 (a pinned literal) fails | 017 |
| K21 | KILLED-OTHER | 022 | the named check CHK-A2-022 passes; only CHK-A2-134 (a pinned row) fails | 134 |
| K30 | KILLED-OTHER | 025 | the named check CHK-A2-025 passes; only CHK-A2-141 (a pinned literal) fails | 141 |
| K31 | KILLED-OTHER | 025 | the named check CHK-A2-025 passes; only CHK-A2-141 (a pinned literal) fails | 141 |
| K40 | SURVIVED | 084 | the pre-fix checker does not fail the named check on this edit (observed); the plan assigns the new branch to P1 | none |
| K50 | SURVIVED | 098 | the pre-fix checker does not fail the named check on this edit (observed); the plan assigns the new branch to P1 | none |
| K51 | SURVIVED | 098 | the pre-fix checker does not fail the named check on this edit (observed); the plan assigns the new branch to P1 | none |

Attempt-3 checker: no SURVIVED and no KILLED-OTHER row among the killable rows of sections A to C and D. I63-a1 is EMPTY (cause: the saved `I63.attempt-1.log` diff is empty; the planned mutant "delete the `preview` services statement" had no target because `command-contract.md:57` points to P2-1 for all five families) on both checkers; it is not a kill and not a survivor. C01 re-read: DIFF SHA256 is that of zero bytes.

## Deviations

A deviation is an observed verdict or failing set that differs from the expected one. None is relabelled.

| Row | Where | Deviation |
| --- | --- | --- |
| K13 | attempt-2 | failing set DIFFERS observed=CHK-A2-137 |
| K13 | attempt-3 | failing set DIFFERS missing=CHK-A2-019 observed=CHK-A2-011,CHK-A2-137 |
| I100 | attempt-3 replay | failing set c01=CHK-A2-022,CHK-A2-084,CHK-A2-133,CHK-A2-160 observed=CHK-A2-022,CHK-A2-084,CHK-A2-133 |
| I129 | attempt-3 replay | verdict expected KILLED observed NOT RUN (saved diff does not apply: hunk at CHK:639 does not match current bytes) |
| I132 | attempt-3 replay | verdict expected KILLED observed NOT RUN (saved diff does not apply: hunk at CHK:639 does not match current bytes) |

Reading of the deviations:

- K13: the verdicts match (attempt-2 KILLED-OTHER, attempt-3 KILLED by CHK-A2-011), but the plan expects CHK-A2-019 to fail as well. CHK-A2-019 does not fail on this mutant on either checker; only CHK-A2-137 fails besides CHK-A2-011. The plan's statement that 019 pins the step (5) literal is not borne out. The attempt-2 named check (011) still does not fail, so K13 is valid red-first evidence; no re-dispatch of P1 is needed for this (the verdict of the named check is as expected), and the plan table should read "137" instead of "019 and 137".
- I100: verdict KILLED as in C01; CHK-A2-160 no longer fails together with CHK-A2-133 when `base/authoring-contract.md` is deleted (cause not investigated; P3 does not read checker code for causes). The killer CHK-A2-133 is unchanged. Recorded, not a weakened kill.
- I129 and I132: mutants of the attempt-2 checker source (line 639, the old extraction loop). the code at that line differs in the attempt-3 checker, so the saved diff no longer matches its bytes and cannot be replayed; the two rows are NOT RUN against attempt-3 (stated limit: no equivalent mutation of the new extraction code was replayed beyond K82 and K83).

