# Probes for V5-02-R15-A2-C01 (docs evidence finisher)

## Correction (C02, 2026-11-07)

This block was added by the C02 record sync. The table, `mutants.tsv`, the rows and the logs below are unchanged. Two statements of the Result section were wrong or incomplete and are replaced there; their original text is quoted here.

Quoted, original Result paragraph (2026-11-06): "Mutants run: 157 (148 rows or variants of the invariant tables I01 to I148 plus the 9 extra prober-stage variants X01 to X09 replayed from their saved diffs). KILLED 156, SURVIVED 1, NOT RUN 2 (I83 and I106, workflow rows with no mutant that a checker can observe; I93 is absent from the plan). Every mutant has a non-empty diff (`changed_lines` in `mutants.tsv`)."

Quoted, original sentence of the "Earlier attempts kept" paragraph: "those first attempts were mutant-fidelity defects of the finisher, not survivors of the checker".

What C02 found, by replaying the saved first-attempt diffs (`raw/<id>.attempt-1.log`) against the checker as it stood at C01 (the pre-fix, attempt-2 checker; its copy is `tests/native/v5-02-r15-a2-c02-evidence/probes/checker-attempt-2/`):

| First attempt | Verdict against the pre-fix checker | Against the tightened (attempt-3) checker |
| --- | --- | --- |
| I07 | SURVIVED (CHK-A2-011 passed) | KILLED by CHK-A2-011 |
| I12 | SURVIVED (CHK-A2-016 passed) | KILLED by CHK-A2-016 |
| I18 | KILLED-OTHER (only CHK-A2-017 failed; CHK-A2-022 passed) | KILLED by CHK-A2-022 (with 017) |
| I21 | SURVIVED (CHK-A2-025 passed) | KILLED by CHK-A2-025 |
| I60 | SURVIVED (CHK-A2-084 passed) | KILLED by CHK-A2-084 |
| I63 | EMPTY (the saved diff is empty, so no mutant existed) | EMPTY |
| I69 | SURVIVED (CHK-A2-098 passed) | KILLED by CHK-A2-098 |

So the sentence calling these first attempts "mutant-fidelity defects" was wrong: I07, I12, I21, I60 and I69 were valid edits that the checker did not detect (survivors of the checker, not of the finisher), I18 was detected only by another check (CHK-A2-017), and I63 attempt 1 was an empty diff. The first attempts of I29, I47 and I74 were not replayed in C02 (the C02 probe scope, C02-D12); the sentence for them stands unverified. The retargeted I63 mutant below is a reword (`Services:` to `Service lists:` and `the service lists of` to `those of`), so its KILLED verdict shows only that CHK-A2-092 pins the bytes of the literal; the meaning-changing mutants I63-m1 to I63-m4 are in the C02 table and are KILLED by the unchanged CHK-A2-092.

Counts of this table, recomputed by awk over `mutants.tsv`: 159 data rows; verdict KILLED=156, SURVIVED=1 (X09), NOT RUN=2 (I83, I106). These are verdicts of the final mutant of each row. With the replayed first attempts the pre-fix checker survivors are the X09 variant and the five first attempts I07, I12, I21, I60 and I69 (6 SURVIVED), plus the KILLED-OTHER I18 and the EMPTY I63 attempt 1.

The C02 probe table, with the true verdicts of every first attempt on both checkers, is `tests/native/v5-02-r15-a2-c02-evidence/probes/PROBES.prober-p3.md` (with `mutants.prober-p3.tsv`, 80 rows). The `PROBES.md` and `mutants.tsv` of that folder are the later 32-row attempt-3 rerun, not this table.


Finisher run, 2026-11-06, on the final bytes. Observer: `contract_checks.py` (a byte copy in the scratch folder, run with `python3 -I -B` from the repository root), except I79 (observer `link_check.py`) and I81 (observer `scripts/redact-local-paths.py check`). No product build is involved.

Scratch clone: `<scratch>/pw`, made with `rsync -a --exclude=build --exclude=.git` of `docs/` and `tests/native/v5-02-r15-a2-evidence/` only (the checker reads nothing else; the whole working tree is about 41 GB, nearly all of it `build/` and vendor sources, so it was not copied; this is a stated limit). The checker and link check run from `<scratch>/pw/checker/` and `<scratch>/pw/linkchk/`, copies of `tests/native/v5-02-r15-a2-evidence/checks/`. Each mutant is a line or text edit of one scratch file, applied only if the replace count matched, run with `--repo <scratch>/pw --base <scratch>/pw/tests/native/v5-02-r15-a2-evidence/base --git-repo <repo> --base-commit cdcfe829c466bf6338815ff6f5b6ea90ffddc570`, then restored by byte copy (sha256 re-verified in the engine; the next mutant starts from the restored bytes). A mutant counts as KILLED only if the exit code is not 0 and a `FAIL` line names the expected check. Guard rows are probed first (the log order in the run was I129, I132, I134 to I136, I17, I19, I71, I80, I82, I100, then the rest).

Positive control: unmutated scratch clone, `raw/baseline.attempt-2.log`, `SUMMARY pass=123 fail=0`, exit 0 (the prober-stage baseline is kept as `raw/baseline.attempt-1.log`).

Identity (workflow `identity.py` output, verbatim; the pathspec includes the checker folder, which is gitignored and so not covered by the diff hashes):

| identity | value |
|---|---|
| HEAD | `cdcfe829c466bf6338815ff6f5b6ea90ffddc570` |
| sha256(git diff --binary HEAD -- docs/vernacular/command-contract.md docs/vernacular/execution-contract.md docs/vernacular/execution-design.md docs/vernacular/external-profile-contract.md docs/vernacular/ledger-contract.md docs/vernacular/load-command-contract.md docs/vernacular/render-contract.md tests/native/v5-02-r15-a2-evidence/checks), tracked only | `312fabfc4520d7670aa085a70508672b0232a514b32d3d8ceaed426c47b7c08f` |
| combined sha256 (diff bytes, then each untracked path NUL bytes, sorted), pathspec docs/vernacular/command-contract.md docs/vernacular/execution-contract.md docs/vernacular/execution-design.md docs/vernacular/external-profile-contract.md docs/vernacular/ledger-contract.md docs/vernacular/load-command-contract.md docs/vernacular/render-contract.md tests/native/v5-02-r15-a2-evidence/checks, evidence excluded | `312fabfc4520d7670aa085a70508672b0232a514b32d3d8ceaed426c47b7c08f` |

| file | state | sha256 |
|---|---|---|
| `docs/vernacular/command-contract.md` | modified | `34ae9d4010f8e13a4183ce452d18395be9eb7f0f95d54eb3bc7aad34d48d4e8a` |
| `docs/vernacular/execution-contract.md` | modified | `38c0d3f70afe6412dc7b7ffe90270bb956562752493c690f0b740714f9176d75` |
| `docs/vernacular/execution-design.md` | modified | `1a5134a0815bb6a18bcb069b96eed94bd28ce247db88b5e4cf54c798c612cb87` |
| `docs/vernacular/external-profile-contract.md` | modified | `86fcbefbf070293b4da7f7efbab1dd7b6065b6d02abc0df7e4bcfa5199f0c493` |
| `docs/vernacular/ledger-contract.md` | modified | `46d8a0eef9f5a65822efbeea1a6e5b61dbd14f3b0adf3e0c01ade94112f4a84f` |
| `docs/vernacular/load-command-contract.md` | modified | `af99144c596184c3ec90a60f3f773fff8509401e14f76502f49b59684e70ee22` |
| `docs/vernacular/render-contract.md` | modified | `22a702eb57177f0c4a3d9b52ca3eb43cf9533a09d63c8586b3c3292d1e2ca874` |

## Result

Final mutants (one per row, from `mutants.tsv`, restated at C02): 159 data rows. 157 were run (148 rows or variants of the invariant tables I01 to I148 plus the 9 extra prober-stage variants X01 to X09 replayed from their saved diffs). Verdict counts (awk over the verdict column): KILLED=156, SURVIVED=1 (X09), NOT RUN=2 (I83 and I106, workflow rows with no mutant that a checker can observe; I93 is absent from the plan). Every one of the 157 mutants has a non-empty diff (`changed_lines` in `mutants.tsv`). These verdicts belong to the final mutant of each row; the first attempts that the retargeting replaced are reported in the Correction (C02) block above and in the C02 table.

Survivor: X09 (a prober-stage extra variant of the run section line 1047, not a row of the invariant table) is not killed by its hypothesised check CHK-A2-042; the prober stage recorded the same outcome and that the variant is killed by CHK-A2-146 instead (see `probes/PROBES.prober-stage.attempt-1.md`). It is reported, not fixed here.

Earlier attempts kept: the first prober stage (C01 rows) logs are `raw/<id>.attempt-1.log`, and `PROBES.prober-stage.attempt-1.md` is the earlier table. For the finisher's first crafted mutants of I07, I12, I18, I21, I29, I47, I60, I63, I69 and I74 (whose results are in the Correction (C02) block above) the first-attempt log is `raw/<id>.attempt-1.log` and the retargeted mutant is `raw/<id>.log`.

Rows I01 to I111 mutants were written from the row text of `tests/native/v5-02-r15-a2-evidence/plan/INVARIANTS.md` (row I67 as retargeted by `SUPERSEDED-BY-C01.md`, so I67 deletes `database-free` and I67b deletes `not a W field`); a row whose text does not give an exact edit was given the smallest edit that removes or alters the literal the row names. The C01 rows I112 to I148 and X01 to X09 are replays of the prober-stage diffs on the final bytes.

## Table

| Row | Expected killer | Verdict | Killing check | Other failing checks | File:line | Log |
| --- | --- | --- | --- | --- | --- | --- |
| I01 | CHK-A2-001 | KILLED | CHK-A2-001 | - | EC line 894 | `raw/I01.log` |
| I02 | CHK-A2-002 | KILLED | CHK-A2-002 | - | EC line 894 | `raw/I02.log` |
| I03 | CHK-A2-003 | KILLED | CHK-A2-003 | CHK-A2-050,CHK-A2-051,CHK-A2-052,CHK-A2-053,CHK-A2-054,CHK-A2-127,CHK-A2-096,CHK-A2-149,CHK-A2-150 | EC line 1092 | `raw/I03.log` |
| I04 | CHK-A2-004 | KILLED | CHK-A2-004 | CHK-A2-005 | EC line 84 | `raw/I04.log` |
| I05 | CHK-A2-005 | KILLED | CHK-A2-005 | - | EC line 670 | `raw/I05.log` |
| I06 | CHK-A2-010 | KILLED | CHK-A2-010 | CHK-A2-011 | EC line 913 | `raw/I06.log` |
| I07 | CHK-A2-011 | KILLED | CHK-A2-011 | CHK-A2-010 | EC line 913 | `raw/I07.log` |
| I08 | CHK-A2-012 | KILLED | CHK-A2-012 | CHK-A2-010 | EC line 914 | `raw/I08.log` |
| I09 | CHK-A2-013 | KILLED | CHK-A2-013 | - | EC line 940 | `raw/I09.log` |
| I10 | CHK-A2-014 | KILLED | CHK-A2-014 | - | EC line 941 | `raw/I10.log` |
| I11 | CHK-A2-015 | KILLED | CHK-A2-015 | CHK-A2-013 | EC line 940 | `raw/I11.log` |
| I12 | CHK-A2-016 | KILLED | CHK-A2-016 | - | EC line 987 | `raw/I12.log` |
| I13 | CHK-A2-017 | KILLED | CHK-A2-017 | - | EC line 972 | `raw/I13.log` |
| I14 | CHK-A2-018 | KILLED | CHK-A2-018 | - | EC line 1009 | `raw/I14.log` |
| I15 | CHK-A2-019 | KILLED | CHK-A2-019 | CHK-A2-020 | EC line 971 | `raw/I15.log` |
| I16 | CHK-A2-020 | KILLED | CHK-A2-020 | - | EC line 971 | `raw/I16.log` |
| I17 | CHK-A2-021 | KILLED | CHK-A2-021 | CHK-A2-004,CHK-A2-005 | EC line 763 | `raw/I17.log` |
| I18 | CHK-A2-022 | KILLED | CHK-A2-022 | CHK-A2-017,CHK-A2-019 | EC line 972 | `raw/I18.log` |
| I19 | CHK-A2-023 | KILLED | CHK-A2-023 | CHK-A2-100 | XP line 558 | `raw/I19.log` |
| I20 | CHK-A2-024 | KILLED | CHK-A2-024 | - | EC line 991 | `raw/I20.log` |
| I21 | CHK-A2-025 | KILLED | CHK-A2-025 | - | EC line 997 | `raw/I21.log` |
| I22 | CHK-A2-026 | KILLED | CHK-A2-026 | - | EC line 999 | `raw/I22.log` |
| I23 | CHK-A2-027 | KILLED | CHK-A2-027 | - | EC line 993 | `raw/I23.log` |
| I24 | CHK-A2-028 | KILLED | CHK-A2-028 | - | EC line 995 | `raw/I24.log` |
| I25 | CHK-A2-029 | KILLED | CHK-A2-029 | - | EC line 931 | `raw/I25.log` |
| I26 | CHK-A2-030 | KILLED | CHK-A2-030 | - | EC line 1018 | `raw/I26.log` |
| I27 | CHK-A2-031 | KILLED | CHK-A2-031 | - | EC line 1022 | `raw/I27.log` |
| I28 | CHK-A2-032 | KILLED | CHK-A2-032 | CHK-A2-018 | EC line 1014 | `raw/I28.log` |
| I29 | CHK-A2-040 | KILLED | CHK-A2-040 | - | EC line 999 | `raw/I29.log` |
| I30 | CHK-A2-041 | KILLED | CHK-A2-041 | - | EC line 1041 | `raw/I30.log` |
| I31 | CHK-A2-042 | KILLED | CHK-A2-042 | CHK-A2-043 | EC line 1083 | `raw/I31.log` |
| I32 | CHK-A2-043 | KILLED | CHK-A2-043 | - | EC line 1087 | `raw/I32.log` |
| I33 | CHK-A2-044 | KILLED | CHK-A2-044 | CHK-A2-043,CHK-A2-144 | EC line 1079 | `raw/I33.log` |
| I34 | CHK-A2-045 | KILLED | CHK-A2-045 | CHK-A2-043,CHK-A2-144 | EC line 1084 | `raw/I34.log` |
| I35 | CHK-A2-046 | KILLED | CHK-A2-046 | - | EC line 1063 | `raw/I35.log` |
| I36 | CHK-A2-047 | KILLED | CHK-A2-047 | - | EC line 1071 | `raw/I36.log` |
| I37 | CHK-A2-050 | KILLED | CHK-A2-050 | - | EC line 1095 | `raw/I37.log` |
| I38 | CHK-A2-051 | KILLED | CHK-A2-051 | - | EC line 1103 | `raw/I38.log` |
| I39 | CHK-A2-052 | KILLED | CHK-A2-052 | - | EC line 1100 | `raw/I39.log` |
| I40 | CHK-A2-053 | KILLED | CHK-A2-053 | - | EC line 1099 | `raw/I40.log` |
| I41 | CHK-A2-054 | KILLED | CHK-A2-054 | - | EC line 1098 | `raw/I41.log` |
| I42 | CHK-A2-060 | KILLED | CHK-A2-060 | - | EC line 1108 | `raw/I42.log` |
| I43 | CHK-A2-061 | KILLED | CHK-A2-061 | - | EC line 1110 | `raw/I43.log` |
| I44 | CHK-A2-062 | KILLED | CHK-A2-062 | - | EC line 1114 | `raw/I44.log` |
| I45 | CHK-A2-063 | KILLED | CHK-A2-063 | - | EC line 1114 | `raw/I45.log` |
| I46 | CHK-A2-064 | KILLED | CHK-A2-064 | - | EC line 1116 | `raw/I46.log` |
| I47 | CHK-A2-065 | KILLED | CHK-A2-065 | - | EC line 1112 | `raw/I47.log` |
| I48 | CHK-A2-066 | KILLED | CHK-A2-066 | - | EC line 1144 | `raw/I48.log` |
| I49 | CHK-A2-070 | KILLED | CHK-A2-070 | - | EC line 1135 | `raw/I49.log` |
| I50 | CHK-A2-071 | KILLED | CHK-A2-071 | - | EC line 1141 | `raw/I50.log` |
| I51 | CHK-A2-072 | KILLED | CHK-A2-072 | - | EC line 1146 | `raw/I51.log` |
| I52 | CHK-A2-073 | KILLED | CHK-A2-073 | CHK-A2-071 | EC line 1144 | `raw/I52.log` |
| I53 | CHK-A2-074 | KILLED | CHK-A2-074 | - | EC line 1148 | `raw/I53.log` |
| I54 | CHK-A2-075 | KILLED | CHK-A2-075 | - | EC line 1141 | `raw/I54.log` |
| I55 | CHK-A2-076 | KILLED | CHK-A2-076 | - | EC line 1001 | `raw/I55.log` |
| I56 | CHK-A2-080 | KILLED | CHK-A2-080 | - | EC line 1152 | `raw/I56.log` |
| I57 | CHK-A2-081 | KILLED | CHK-A2-081 | - | EC line 1154 | `raw/I57.log` |
| I58 | CHK-A2-082 | KILLED | CHK-A2-082 | - | EC line 1156 | `raw/I58.log` |
| I59 | CHK-A2-083 | KILLED | CHK-A2-083 | - | EC line 1154 | `raw/I59.log` |
| I60 | CHK-A2-084 | KILLED | CHK-A2-084 | - | EC line 1156 | `raw/I60.log` |
| I61 | CHK-A2-090 | KILLED | CHK-A2-090 | CHK-A2-101 | CC line 41 | `raw/I61.log` |
| I62 | CHK-A2-091 | KILLED | CHK-A2-091 | - | CC line 52 | `raw/I62.log` |
| I63 | CHK-A2-092 | KILLED | CHK-A2-092 | - | CC line 57 | `raw/I63.log` |
| I64 | CHK-A2-093 | KILLED | CHK-A2-093 | - | CC line 55 | `raw/I64.log` |
| I65 | CHK-A2-094 | KILLED | CHK-A2-094 | CHK-A2-095 | CC line 62 | `raw/I65.log` |
| I66 | CHK-A2-095 | KILLED | CHK-A2-095 | - | XP line 53 | `raw/I66.log` |
| I67 | CHK-A2-096 | KILLED | CHK-A2-096 | - | RC line 416 | `raw/I67.log` |
| I67b | CHK-A2-096 | KILLED | CHK-A2-096 | CHK-A2-150 | RC line 416 | `raw/I67b.log` |
| I68 | CHK-A2-097 | KILLED | CHK-A2-097 | - | LC line 21 | `raw/I68.log` |
| I69 | CHK-A2-098 | KILLED | CHK-A2-098 | - | LG line 454 | `raw/I69.log` |
| I70 | CHK-A2-099 | KILLED | CHK-A2-099 | - | XP line 48 | `raw/I70.log` |
| I71 | CHK-A2-100 | KILLED | CHK-A2-100 | - | AM line 1 | `raw/I71.log` |
| I72 | CHK-A2-101 | KILLED | CHK-A2-101 | CHK-A2-020 | EC line 1167 | `raw/I72.log` |
| I73 | CHK-A2-110 | KILLED | CHK-A2-110 | - | EC line 1300 | `raw/I73.log` |
| I74 | CHK-A2-111 | KILLED | CHK-A2-111 | - | EC line 1441 | `raw/I74.log` |
| I75 | CHK-A2-112 | KILLED | CHK-A2-112 | CHK-A2-155 | EC line 1354 | `raw/I75.log` |
| I76 | CHK-A2-113 | KILLED | CHK-A2-113 | - | ED line 704 | `raw/I76.log` |
| I77 | CHK-A2-114 | KILLED | CHK-A2-114 | - | EC line 1225 | `raw/I77.log` |
| I78 | CHK-A2-115 | KILLED | CHK-A2-115 | - | EC line 912 | `raw/I78.log` |
| I79 | LINK-A2-001 | KILLED | LINK-A2-001 | - | EC line 1106 | `raw/I79.log` |
| I80 | CHK-A2-116 | KILLED | CHK-A2-116 | - | EC line 1160 | `raw/I80.log` |
| I81 | RED-A2-001 | KILLED | RED-A2-001 | - | scratch copy of execution-contract.md line 894 | `raw/I81.log` |
| I82 | CHK-A2-000 | KILLED | CHK-A2-000 | - | CHK line 393 | `raw/I82.log` |
| I84 | CHK-A2-117 | KILLED | CHK-A2-117 | - | EC line 960 | `raw/I84.log` |
| I85 | CHK-A2-118 | KILLED | CHK-A2-118 | - | EC line 957 | `raw/I85.log` |
| I86 | CHK-A2-119 | KILLED | CHK-A2-119 | - | EC line 995 | `raw/I86.log` |
| I87 | CHK-A2-120 | KILLED | CHK-A2-120 | - | EC line 1137 | `raw/I87.log` |
| I88 | CHK-A2-121 | KILLED | CHK-A2-121 | - | EC line 1139 | `raw/I88.log` |
| I89 | CHK-A2-122 | KILLED | CHK-A2-122 | - | EC line 1139 | `raw/I89.log` |
| I90 | CHK-A2-123 | KILLED | CHK-A2-123 | - | EC line 1144 | `raw/I90.log` |
| I91 | CHK-A2-124 | KILLED | CHK-A2-124 | - | EC line 987 | `raw/I91.log` |
| I92 | CHK-A2-125 | KILLED | CHK-A2-125 | CHK-A2-005 | EC line 1227 | `raw/I92.log` |
| I94 | CHK-A2-127 | KILLED | CHK-A2-127 | CHK-A2-043 | EC line 1087 | `raw/I94.log` |
| I95 | CHK-A2-128 | KILLED | CHK-A2-128 | - | XP line 61 | `raw/I95.log` |
| I96 | CHK-A2-129 | KILLED | CHK-A2-129 | - | EC line 1283 | `raw/I96.log` |
| I97 | CHK-A2-130 | KILLED | CHK-A2-130 | - | LG line 454 | `raw/I97.log` |
| I98 | CHK-A2-131 | KILLED | CHK-A2-131 | CHK-A2-092 | CC line 57 | `raw/I98.log` |
| I99 | CHK-A2-132 | KILLED | CHK-A2-132 | - | LC line 21 | `raw/I99.log` |
| I100 | CHK-A2-133 | KILLED | CHK-A2-133 | CHK-A2-022,CHK-A2-084,CHK-A2-160 | B:authoring-contract.md line 1 | `raw/I100.log` |
| I101 | CHK-A2-134 | KILLED | CHK-A2-134 | - | EC line 1131 | `raw/I101.log` |
| I102 | CHK-A2-135 | KILLED | CHK-A2-135 | - | EC line 1051 | `raw/I102.log` |
| I103 | CHK-A2-136 | KILLED | CHK-A2-136 | CHK-A2-101 | CC line 31 | `raw/I103.log` |
| I104 | CHK-A2-137 | KILLED | CHK-A2-137 | - | EC line 970 | `raw/I104.log` |
| I105 | CHK-A2-138 | KILLED | CHK-A2-138 | - | EC line 997 | `raw/I105.log` |
| I107 | CHK-A2-139 | KILLED | CHK-A2-139 | - | EC line 942 | `raw/I107.log` |
| I108 | CHK-A2-140 | KILLED | CHK-A2-140 | - | EC line 949 | `raw/I108.log` |
| I109 | CHK-A2-141 | KILLED | CHK-A2-141 | - | EC line 983 | `raw/I109.log` |
| I110 | CHK-A2-142 | KILLED | CHK-A2-142 | - | EC line 1148 | `raw/I110.log` |
| I111 | CHK-A2-143 | KILLED | CHK-A2-143 | - | XP line 48 | `raw/I111.log` |
| I112 | CHK-A2-144 | KILLED | CHK-A2-144 | CHK-A2-043,CHK-A2-044 | EC line 1079 | `raw/I112.log` |
| I113 | CHK-A2-145 | KILLED | CHK-A2-145 | - | EC line 1091 | `raw/I113.log` |
| I113b | CHK-A2-145 | KILLED | CHK-A2-145 | - | EC line 999 | `raw/I113b.log` |
| I113c | CHK-A2-145 | KILLED | CHK-A2-145 | - | EC line 1091 | `raw/I113c.log` |
| I114 | CHK-A2-146 | KILLED | CHK-A2-146 | CHK-A2-042,CHK-A2-145 | EC line 1047 | `raw/I114.log` |
| I114b | CHK-A2-146 | KILLED | CHK-A2-146 | CHK-A2-042,CHK-A2-145 | EC line 1047 | `raw/I114b.log` |
| I114c | CHK-A2-146 | KILLED | CHK-A2-146 | CHK-A2-042,CHK-A2-111,CHK-A2-145 | EC line 1047 | `raw/I114c.log` |
| I115 | CHK-A2-147 | KILLED | CHK-A2-147 | - | EC line 1034 | `raw/I115.log` |
| I116 | CHK-A2-148 | KILLED | CHK-A2-148 | - | RC line 253 | `raw/I116.log` |
| I117 | CHK-A2-149 | KILLED | CHK-A2-149 | CHK-A2-150 | EC line 1438 | `raw/I117.log` |
| I117b | CHK-A2-149 | KILLED | CHK-A2-149 | CHK-A2-096,CHK-A2-150 | RC line 416 | `raw/I117b.log` |
| I117c | CHK-A2-149 | KILLED | CHK-A2-149 | CHK-A2-096,CHK-A2-150 | EC line 1102 | `raw/I117c.log` |
| I118 | CHK-A2-150 | KILLED | CHK-A2-150 | CHK-A2-096,CHK-A2-149 | EC line 1102 | `raw/I118.log` |
| I118b | CHK-A2-150 | KILLED | CHK-A2-150 | - | EC line 1199 | `raw/I118b.log` |
| I119 | CHK-A2-151 | KILLED | CHK-A2-151 | - | EC line 1137 | `raw/I119.log` |
| I120 | CHK-A2-152 | KILLED | CHK-A2-152 | CHK-A2-151 | EC line 1461 | `raw/I120.log` |
| I121 | CHK-A2-153 | KILLED | CHK-A2-153 | CHK-A2-076 | EC line 540 | `raw/I121.log` |
| I122 | CHK-A2-154 | KILLED | CHK-A2-154 | - | LC line 21 | `raw/I122.log` |
| I123 | CHK-A2-155 | KILLED | CHK-A2-155 | - | EC line 1354 | `raw/I123.log` |
| I123b | CHK-A2-155 | KILLED | CHK-A2-155 | CHK-A2-112 | EC line 1357 | `raw/I123b.log` |
| I124 | CHK-A2-156 | KILLED | CHK-A2-156 | - | EC line 1323 | `raw/I124.log` |
| I125 | CHK-A2-157 | KILLED | CHK-A2-157 | - | EC line 1324 | `raw/I125.log` |
| I126 | CHK-A2-158 | KILLED | CHK-A2-158 | - | EC line 1225 | `raw/I126.log` |
| I126b | CHK-A2-158 | KILLED | CHK-A2-158 | - | EC line 1225 | `raw/I126b.log` |
| I127 | CHK-A2-159 | KILLED | CHK-A2-159 | - | EC line 1216 | `raw/I127.log` |
| I128 | CHK-A2-111 | KILLED | CHK-A2-111 | - | EC line 1472 | `raw/I128.log` |
| I129 | CHK-A2-160 | KILLED | CHK-A2-160 | CHK-A2-022,CHK-A2-000 | CHK line 639 | `raw/I129.log` |
| I130 | CHK-A2-022 | KILLED | CHK-A2-022 | CHK-A2-017 | EC line 972 | `raw/I130.log` |
| I131 | CHK-A2-022 | KILLED | CHK-A2-022 | CHK-A2-021,CHK-A2-084,CHK-A2-160,CHK-A2-133 | B:execution-contract.md line 953 | `raw/I131.log` |
| I132 | CHK-A2-000 | KILLED | CHK-A2-000 | CHK-A2-022,CHK-A2-160 | CHK line 639 | `raw/I132.log` |
| I133 | CHK-A2-043 | KILLED | CHK-A2-043 | CHK-A2-044,CHK-A2-144 | EC line 1079 | `raw/I133.log` |
| I134 | CHK-A2-005 | KILLED | CHK-A2-005 | - | EC line 659 | `raw/I134.log` |
| I135 | CHK-A2-100 | KILLED | CHK-A2-100 | - | LC line 5 | `raw/I135.log` |
| I136 | CHK-A2-021 | KILLED | CHK-A2-021 | CHK-A2-004,CHK-A2-005 | EC line 763 | `raw/I136.log` |
| I144 | CHK-A2-096 | KILLED | CHK-A2-096 | CHK-A2-150 | RC line 416 | `raw/I144.log` |
| I144b | CHK-A2-096 | KILLED | CHK-A2-096 | CHK-A2-150 | RC line 416 | `raw/I144b.log` |
| I148 | CHK-A2-111 | KILLED | CHK-A2-111 | - | EC line 1471 | `raw/I148.log` |
| I148b | CHK-A2-111 | KILLED | CHK-A2-111 | - | EC line 1471 | `raw/I148b.log` |
| I148c | CHK-A2-111 | KILLED | CHK-A2-111 | - | EC line 1473 | `raw/I148c.log` |
| X01 | CHK-A2-151 | KILLED | CHK-A2-151 | CHK-A2-011 | EC line 921 | `raw/X01.log` |
| X02 | CHK-A2-147 | KILLED | CHK-A2-147 | - | EC line 1034 | `raw/X02.log` |
| X03 | CHK-A2-145 | KILLED | CHK-A2-145 | - | EC line 1091 | `raw/X03.log` |
| X04 | CHK-A2-145 | KILLED | CHK-A2-145 | - | EC line 1091 | `raw/X04.log` |
| X05 | CHK-A2-148 | KILLED | CHK-A2-148 | - | EC line 1199 | `raw/X05.log` |
| X06 | CHK-A2-155 | KILLED | CHK-A2-155 | - | EC line 1356 | `raw/X06.log` |
| X07 | CHK-A2-156 | KILLED | CHK-A2-156 | - | EC line 1323 | `raw/X07.log` |
| X08 | CHK-A2-146 | KILLED | CHK-A2-146 | - | EC line 1057 | `raw/X08.log` |
| X09 | CHK-A2-042 | SURVIVED | - | CHK-A2-146 | EC line 1047 | `raw/X09.log` |
| I83 | RED-A2-002 | NOT RUN | - | - | - | workflow row |
| I106 | PRB-A2-001 | NOT RUN | - | - | - | satisfied by `raw/baseline.attempt-2.log` |
