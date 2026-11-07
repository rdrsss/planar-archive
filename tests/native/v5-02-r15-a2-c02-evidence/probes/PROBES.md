# Probes for V5-02-R15-A2-C02 (docs evidence finisher, 2026-11-06)

Observer: the checkout checker `tests/native/v5-02-r15-a2-evidence/checks/contract_checks.py`, run from a `diff -r` identical scratch copy (`<scratch>/chk`) with `python3 -I -B`, `--repo <scratch>/pw`, `--base <scratch>/pw/tests/native/v5-02-r15-a2-evidence/base`, `--git-repo <repo>`, `--base-commit cdcfe829c466bf6338815ff6f5b6ea90ffddc570`, from the repository root, under `perl -e 'alarm 600; exec @ARGV'` (no `timeout` here). No product build is involved. Only the attempt-3 (current) checker was run by this finisher; the earlier prober's attempt-2 and attempt-3 results are kept in `PROBES.prober-p3.md`, `mutants.prober-p3.tsv` and the `*.attempt2.log` / `*.attempt3.log` files.

Method: the scratch clone `<scratch>/pw` holds `docs/` and `tests/native/v5-02-r15-a2-evidence/` only (the checker reads nothing else; stated limit: the rest of the working tree, about 41 GB mostly `build/`, was not copied). Each mutant is the exact OLD to NEW text of `plan/INVARIANTS.md`, replaced on the named line of one scratch contract file only when the count of OLD in that line is exactly 1; the run is chained after the replace; the file is restored by byte copy and the restored sha256 equals the original (`RESTORED_MATCH True` in every contract-mutant log). Checker-side mutants K80 to K83 replace the body of one helper by a permissive `return` (docstring and signature kept) in `<scratch>/chk-mut`. Nothing was applied in the checkout.

Verdicts: KILLED = exit not 0 and a `FAIL` line names the row's expected check; KILLED-OTHER = exit not 0 but only other checks fail (counted as a survivor of the named check); SURVIVED = exit 0 or the named check passes; EMPTY/NOT RUN as in the C02 table. Every first run is kept: each mutant ran once, so there are no re-runs and no `.attempt-N.log` files from this finisher.

Positive control: `green/checks.txt` (123 PASS, 0 FAIL, exit 0) is the unmutated run of the same scratch checker over the unchanged contract text.

## Result

32 mutants run: 32 KILLED, 0 KILLED-OTHER, 0 SURVIVED, 0 EMPTY, 0 NOT RUN. There are no survivors. "Other failing checks" lists checks besides the expected killer that also failed (informational).

| row | file:line | verdict | killing check | other failing checks | log |
| --- | --- | --- | --- | --- | --- |
| I07-a1 | EC:921 | KILLED | CHK-A2-011 | - | `probes/raw/I07-a1.log` |
| I12-a1 | EC:987 | KILLED | CHK-A2-016 | - | `probes/raw/I12-a1.log` |
| I18-a1 | EC:972 | KILLED | CHK-A2-022 | CHK-A2-017 | `probes/raw/I18-a1.log` |
| I21-a1 | EC:983 | KILLED | CHK-A2-025 | - | `probes/raw/I21-a1.log` |
| I60-a1 | EC:1154 | KILLED | CHK-A2-084 | - | `probes/raw/I60-a1.log` |
| I69-a1 | LG:454 | KILLED | CHK-A2-098 | - | `probes/raw/I69-a1.log` |
| K01 | EC:987 | KILLED | CHK-A2-016 | - | `probes/raw/K01.log` |
| K02 | EC:987 | KILLED | CHK-A2-016 | - | `probes/raw/K02.log` |
| K03 | EC:1426 | KILLED | CHK-A2-016 | - | `probes/raw/K03.log` |
| K04 | EC:1426 | KILLED | CHK-A2-016 | - | `probes/raw/K04.log` |
| K05 | EC:1426 | KILLED | CHK-A2-016 | - | `probes/raw/K05.log` |
| K06 | EC:987 | KILLED | CHK-A2-016 | - | `probes/raw/K06.log` |
| K07 | EC:987 | KILLED | CHK-A2-016 | - | `probes/raw/K07.log` |
| K10 | EC:921 | KILLED | CHK-A2-011 | - | `probes/raw/K10.log` |
| K11 | EC:921 | KILLED | CHK-A2-011 | - | `probes/raw/K11.log` |
| K12 | EC:1422 | KILLED | CHK-A2-011 | - | `probes/raw/K12.log` |
| K13 | EC:970 | KILLED | CHK-A2-011 | CHK-A2-137 | `probes/raw/K13.log` |
| K20 | EC:972 | KILLED | CHK-A2-022 | CHK-A2-017 | `probes/raw/K20.log` |
| K21 | EC:1124 | KILLED | CHK-A2-022 | CHK-A2-134 | `probes/raw/K21.log` |
| K30 | EC:983 | KILLED | CHK-A2-025 | CHK-A2-141 | `probes/raw/K30.log` |
| K31 | EC:983 | KILLED | CHK-A2-025 | CHK-A2-141 | `probes/raw/K31.log` |
| K40 | EC:1154 | KILLED | CHK-A2-084 | - | `probes/raw/K40.log` |
| K50 | LG:454 | KILLED | CHK-A2-098 | - | `probes/raw/K50.log` |
| K51 | LG:454 | KILLED | CHK-A2-098 | - | `probes/raw/K51.log` |
| I63-m1 | CC:57 | KILLED | CHK-A2-092 | - | `probes/raw/I63-m1.log` |
| I63-m2 | CC:57 | KILLED | CHK-A2-092 | - | `probes/raw/I63-m2.log` |
| I63-m3 | CC:57 | KILLED | CHK-A2-092 | - | `probes/raw/I63-m3.log` |
| I63-m4 | CC:57 | KILLED | CHK-A2-092 | CHK-A2-131 | `probes/raw/I63-m4.log` |
| K80 | CHK helper | KILLED | CHK-A2-000 | - | `probes/raw/K80.log` |
| K81 | CHK helper | KILLED | CHK-A2-000 | - | `probes/raw/K81.log` |
| K82 | CHK helper | KILLED | CHK-A2-000 | - | `probes/raw/K82.log` |
| K83 | CHK helper | KILLED | CHK-A2-000 | - | `probes/raw/K83.log` |

## Acceptance readings

- Acceptance 1: I07-a1 (lowercase `none`, also K10 to K13) is KILLED by CHK-A2-011; I12-a1 (extra exempt member `preview`) and K01 (`init` re-added) are KILLED by CHK-A2-016; I18-a1 (the "then `X`" form) is KILLED by CHK-A2-022 (CHK-A2-017 also fails); the first-attempt mutants I21-a1, I60-a1 and I69-a1 are KILLED by CHK-A2-025, CHK-A2-084 and CHK-A2-098.
- Acceptance 3: I63 has four meaning-changing mutants (service name `read_execution` to `read_ledger`, selector `event` to `request`, deletion of the "five families have the service lists of P2-1" clause, `limit:50` to `limit:1`), all KILLED by CHK-A2-092 (I63-m4 also fails CHK-A2-131).
- I63-a1 (the empty first-attempt diff) was not re-run: it has no mutant; its EMPTY verdict stays as recorded in `PROBES.prober-p3.md`.
- Workflow rows A01 to A26 have no mutant; they are checked by the files named in `plan/INVARIANTS.md` section E.
