# V5-02-R15-A2-C03 close review (lineage R15-A2, C01, C02, C03 finish)

Decision: **approve**. C02-R1 and C02-R2 are closed. The staging list follows C02-D23, `plan/DECISIONS.md` is redacted
and listed, the records and README describe the final list, and the probe pointers name the 80-row two-checker table.
The contract checker passes from a scratch copy, three re-applied probe mutants give the recorded verdicts on both
checkers, no `src` or non-evidence `tests` byte changed, and no staged file pins the hash of a record, review, README,
plan file, probe report, REPORT or the roadmap. The lineage is committed with this review (see "Commit").

Scope: the uncommitted candidate on HEAD `cdcfe829`. That is the seven contract documents, the roadmap, the records
[V5-02-R15-A2](V5-02-R15-A2.md), [C01](V5-02-R15-A2-C01.md) and [C02](V5-02-R15-A2-C02.md) (with its "C03 finish"
section), the three earlier reviews, and the evidence folders `tests/native/v5-02-r15-a2-evidence/`,
`tests/native/v5-02-r15-a2-c01-evidence/` and `tests/native/v5-02-r15-a2-c02-evidence/`. I ran no script in place
under `tests/native/` and did not run any `identity.py` there. All scratch work was under the session scratchpad
(`<scratch>/a2close/`).

## What I verified (commands run by me, results)

| Check | Command (from the repository root) | Result |
| --- | --- | --- |
| C02-R1, list shape | `wc -l`, `cut -f2 \| sort \| uniq -c`, `awk -F'\t' 'NF!=2'` over `green/staging-list.txt` | 195 lines; 180 FORCE-ADD, 8 TRACKED-MODIFIED, 6 NEW, 1 CHECKED-BY-COORDINATOR; 0 lines without exactly two TAB fields; 0 duplicate paths |
| C02-R1, forbidden paths | `grep -cE 'raw/\|superseded\|/work/'` over the list | 0. The only `inventory/` paths are the staged-form `before-files.txt` and `before-git-status.txt` (C02-D21, C02-D28); the full-hash `work/` inventory is not listed |
| C02-R1, paths exist | `test -e` on every first field | 0 missing |
| C02-R1, decisions file | list contains `tests/native/v5-02-r15-a2-c02-evidence/plan/DECISIONS.md`; `python3 scripts/redact-local-paths.py --repo-path <repo> --home <home> check` on it | listed, exit 0 |
| C02-R1, lineage files | the three records, the three review files and the seven contract documents plus the roadmap appear once each | yes (6 NEW, 8 TRACKED-MODIFIED) |
| C02-R1, per-file redaction | the same `check` once per listed path (195 runs) | 195 of 195 exit 0, including `green/redaction.txt` itself |
| C02-R2, pointers | `sed -n` on `V5-02-R15-A2.md` :480-486 and :508-514, the R15-A2 evidence README :16-31, the C01 `probes/PROBES.md` :25-29 | each names `probes/PROBES.prober-p3.md` with `mutants.prober-p3.tsv` as the 80-row two-checker table and `PROBES.md` with `mutants.tsv` as the 32-row attempt-3 rerun; the record and README state the 195-line list. Row counts by `grep -c .`: `mutants.prober-p3.tsv` 81 lines (80 rows plus header), `mutants.tsv` 33 (32 plus header) |
| Contract checker | `cp -R tests/native/v5-02-r15-a2-evidence/checks <scratch>/a2close/chk` (`diff -r` identical), then `python3 -I -B <scratch>/a2close/chk/contract_checks.py --repo <repo> --base <repo>/tests/native/v5-02-r15-a2-evidence/base --git-repo <repo> --base-commit cdcfe829c466bf6338815ff6f5b6ea90ffddc570` | exit 0, 123 PASS, 0 FAIL, 0 Traceback, `SUMMARY pass=123 fail=0` |
| Self-test | `python3 -I -B <scratch>/a2close/chk/contract_checks.py --selftest` | exit 0, `SUMMARY pass=1 fail=0` |
| Link check | `python3 -I -B <scratch>/a2close/chk/link_check.py --repo <repo> --git-repo <repo> --base-commit cdcfe829...` | exit 0, `PASS LINK-A2-001 anchors_resolve`. A naive scan of mine over the listed non-snapshot `.md` files found 428 relative links; its 3 "misses" are false positives (two inside backticked mutant or CLAUDE.md text, one URL-encoded link to the existing v4 design file, unchanged from HEAD) |
| Probe spot-checks | scratch clone `<scratch>/a2close/pw` (`rsync` of `docs/` and `tests/native/v5-02-r15-a2-evidence/`), checkers `chk2` (`diff -r` identical to `probes/checker-attempt-2/`) and `chk` (attempt 3); exact OLD to NEW of plan `INVARIANTS.md`, one occurrence each, file restored byte for byte after each | I12-a1 (`preview` added to the exempt reads): attempt 2 exit 0 `pass=123 fail=0` (SURVIVED), attempt 3 exit 1, only `FAIL CHK-A2-016` (KILLED). I63-m2 (`event` to `request`): both checkers exit 1, only `FAIL CHK-A2-092` (KILLED). K11 (`null` as the no-active-job value): attempt 2 `pass=123 fail=0` (SURVIVED), attempt 3 only `FAIL CHK-A2-011` (KILLED). All six match `mutants.prober-p3.tsv` |
| No `src` or `tests` change | `git status --short -- src tests \| grep -v -- '-evidence/'`; `git diff --name-only HEAD -- src tests`; `git status --short --ignored -- src` | empty, 0, empty |
| Record rule (hash audit) | own scan: forbidden set = sha256 of every `docs/vernacular/rounds/*.md`, the roadmap now and at HEAD, and every README, PROBES*, REPORT*, RED* and `plan/**` file of every `tests/native/*-evidence/` folder (958 distinct hashes); every 64-hex token, either case, of every listed file | 1856 tokens in 25 files, 0 equal to a forbidden hash. Agrees with `green/hash-audit.txt` (1856 tokens, 25 files, 0 hits, positive control found) |

## Closure

| Finding | Verdict |
| --- | --- |
| C02-R1 (staging list) | closed |
| C02-R2 (stale records, probe pointers) | closed |
| C01-R1 to C01-R4 | closed, as recorded in the C02 review; nothing reopened |

## Carried advisories (not blocking)

- **A1.** The coordinator's final-bytes check named by C02-D29 (`green/redaction-final-coordinator.txt`) was not
  written as a file. The per-file `check` over all 195 listed paths in this review, and over the three files edited at
  close (roadmap, C02 record, this review), stands in for it. The R15-A2 record (Acceptance row 7) and the README still
  say "not done yet", which stays literally true of that file.
- **A2.** P6's 191-line staging list and 190-file redaction log were overwritten before C03 and cannot be recovered.
  The C03 finish states this plainly; the 197-line list and its logs are kept, unstaged, as `*.c02.txt`.
- **A3.** `plan/DECISIONS.md` was redacted with a scratch script that uses the tool's `<home>` token, because
  `redact-local-paths.py redact` refuses files that already hold a token. The deviation is recorded; `check` exits 0.
- **A4.** `green/hash-audit-final.txt` (P6) audited the earlier list; the final-list audit is `green/hash-audit.txt`.
  The README names both. Readers should cite `hash-audit.txt` for the committed list.
- **A5.** Carried from earlier reviews without change: the CHK-A2-022 limit (prose without backticks,
  lowercase-initial backtick messages, Part 1 tables) and the NOT RUN replays I129 and I132.

## Commit

Staged exactly the 195 paths of `green/staging-list.txt` plus this review (the roadmap is already on the list), with
explicit paths and `git add -f` for gitignored evidence, and checked `git diff --cached --name-only` against that set.
Delegated decisions D104, D134 and D142 were taken at their recommended options under the 2026-10-31 standing
delegation.
