# V5-02-R15-A1-C04 close review (execution contract, part 1, fourth corrective)

Decision: **approve**. This is the single closing review of the R15-A1 lineage (R15-A1 with correctives C01 to
C04), in the form used for RA01-C-C02. C03-R1 to C03-R4 (blocking and medium) are closed. C03-R5 to C03-R8 are
closed or reduced to wording. I found no false claim of substance, no missing or failing FIN log, no redaction
failure in a file to be staged, and no `src` or `tests` change. The remaining points are wording and process
advisories, carried below. None of them blocks the commit.

Scope: the uncommitted candidate on HEAD `d536922b`. That covers the seven candidate documents, whose contract text
was accepted by the C01 review and has not changed since apart from the OP2 tag (C02). It also covers the round
record [V5-02-R15-A1](V5-02-R15-A1.md), with weight on 'V5-02-R15-A1-C04 outcome' and the staging list, the
[C03 review](V5-02-R15-A1-C03-review.md), the C04 evidence folder `tests/native/v5-02-r15-a1-c04-evidence/` and the
C01 evidence folder (`green/`, `README.md`, `red/RED.md`).

I ran no script in place under `tests/native/` and no `identity.py`. Every script ran as a `cmp`-identical copy in
the scratchpad, against the checkout read-only or against a scratch clone outside it.

## Closure of C03-R1 to C03-R8

| Finding | Verdict | Basis (my own runs) |
| --- | --- | --- |
| C03-R1 FIN evidence incomplete and stale | closed | All six C04 FIN logs exist and are newer than every input they describe (file times below). My checker runs pass in default and audit mode. The never-written GREEN2 names (`audit-verify-c02.txt`, `redaction-final-c02.txt`, `record-asserts-final-c03.txt`) are dropped and replaced by C04 names, and the record says so. The three C03 FIN1 logs stay older by design, as a stated limit. |
| C03-R2 `CLAUDE.md` redaction gap | closed | `green/redaction-c02.txt` has `exit 0 CLAUDE.md`. I re-applied C4-m1 (drop that line) in a scratch clone and it is KILLED: exit 1, `FAIL CHK-C02-evidence-logs green/redaction-c02.txt lacks "exit 0 CLAUDE.md"`. |
| C03-R3 owned files claim absent files | closed | 'Owned files (C03)' no longer names the three files. They appear only as "was not written" in 'Unstaged paths (C03)' and in historical plan text. |
| C03-R4 overwritten first-attempt logs | closed as a carried limit | 'Lost first-attempt probe logs (C03-R4)' names all six, gives the reason for each and states the attempt-log rule for later rounds. |
| C03-R5 permissive clause 9 | closed | The skip of table rows and indented lines is removed. I re-applied C4-m2 (table row) and C4-m3 (indented line) and both are KILLED with `the record quotes a result of audit-final`. |
| C03-R6 `raw/red-checks` description | closed | `ls raw/red-checks \| wc -l` gives 72. `red/RED.md:83` and the closure row now say 71 logs plus SUMMARY.txt, with 34 FAIL logs (29 default and 5 audit-only). |
| C03-R7 message count | closed | 'Checker amendments (C02)' now says 21 removed and 35 added against the pre-C03 C02 checker. The C02-R7 closure row keeps its old "36" sentence, followed by the correction (advisory A4). |
| C03-R8 hash pin and table intro | closed | The pin of `raw/ex_checks-c02-preedit-c03.py` is removed from 'C03 candidate identity' (`:1642` now says "not hashed here"). The intro states that cells were reworded. |

## What I verified

- **File order.** Times from `stat -f '%Sm' -t '%H:%M:%S'`:
  - inputs: record 19:20:00, C01 `README.md` and `red/RED.md` 19:18:56, checker and refreshed frozen copies
    19:10:14, C04 `probes/PROBES.md` 19:22:20, roadmap 19:06:53;
  - FIN logs: `checks.txt` 19:24:08, `linkcheck-c02.txt` 19:24:09, `redaction-c02.txt` 19:24:14,
    `audit-final.txt` 19:24:18, `redaction-final-c04.txt` 19:24:23, `record-asserts-final-c04.txt` 19:24:25.

  Every FIN log is newer than every input, and the logs are in the order of 'Final results (C04)'.
- **Checker, re-run by me.** I copied `green/ex_checks-c02.py` (`cmp` identical) and `green/frozen/` to
  `<scratchpad>/close/`. Then I ran
  `python3 -I -B <scratchpad>/close/ex_checks-c02.py --repo . --doc docs/vernacular/execution-contract.md --evidence-dir tests/native/v5-02-r15-a1-c01-evidence --frozen-dir <scratchpad>/close/frozen`
  from the repository root.
  - Default mode: exit 0, 113 PASS, 0 FAIL.
  - `--audit`: exit 0, 122 PASS, 0 FAIL.

  Both counts equal the FIN logs (`checks.txt` 113 PASS, `audit-final.txt` 122 PASS, each with exit code 0).
- **Record assertions.** I ran a scratch copy of `record_asserts_c04.py` with `--phase final` on a scratch clone
  (HEAD plus the working-tree `docs/`, `CLAUDE.md` and the R15-A1 evidence trees, copied with `rsync -a` so file
  times are kept). Result: exit 0, 7 PASS, including `ASSERT-C03-order` ("10 of 10 files present, mtime chain holds")
  and `ASSERT-C03-record-rule`. This matches `green/record-asserts-final-c04.txt`.
- **Staging list existence and redaction.** I took every bullet of `### Staging list (C02)`, the three FIN files and
  the roadmap: 75 paths. All 75 exist. None is under `raw/`, none is `superseded-*` or `inventory.txt`, and none is
  the `green/frozen/` directory.
  - `python3 scripts/redact-local-paths.py --repo-path <repo> --home $HOME check <file>`, one file per invocation,
    exits 0 for all 75.
  - The positive control (a scratch file holding the repository path) exits 1.
  - After writing this review and the closing edits, I re-ran the same check on this file, the record and the
    roadmap (see 'Closing edits').
- **Probe spot-check.** On the scratch clone I first ran the control: `--audit CHK-C02-evidence-logs` gives exit 0
  and PASS. Then I applied C4-m1, C4-m2 and C4-m3, one at a time with the clone restored between them. Each gives
  exit 1 and one `FAIL CHK-C02-evidence-logs` naming the mutation. After restoring the clone, the control passes
  again.
- **No `src` or `tests` change.**
  - `git diff --stat HEAD -- src tests` is empty, and `git status --short src tests` is empty.
  - `git diff --binary HEAD -- src tests | shasum -a 256` gives the empty-input hash `e3b0c442...`.
  - No untracked file under `src` or `tests` is visible to Git. The evidence trees are ignored by `.gitignore:37`.
- **Candidate identity.** I compared the per-file sha256 of the seven documents and the tracked-only diff hash
  (`f4808815...`) with the C03 review's block, and they are equal value for value. `readlink AGENTS.md` prints
  `CLAUDE.md`.
- **Record rule.** I took the sha256 of every `docs/vernacular/rounds/*.md`, `docs/vernacular/roadmap.md` (live and
  at HEAD) and every `tests/native/*-evidence/**/REPORT*`: 563 hash sources. I compared them in memory with every
  64-hex token in the 75 staged files, and there are 0 hits. No staged file pins a hash of a round record, a review,
  a REPORT or the roadmap.

## Carried advisories (not blocking)

- **A1.** `green/record-asserts-final-c04.txt` ran in non-final mode (the log prints `mode: non-final, phase:
  final`). With `--final`, `ASSERT-C03-paths` reports 42 unresolved tokens. All of them are mentions of the
  never-written GREEN2 names (`audit-verify-c02.txt`, `redaction-final-c02.txt`, `record-asserts-final-c03.txt`) in
  historical plan quotes and invariant-table cells. The log states its mode honestly, so this is not a false claim.
  A later round should not reuse the FIN-only exemption for files that will never exist.
- **A2.** `record_asserts_c04.py` is the C03 script with two patches (`record_asserts_c04.diff`), one of which
  replaces the order chain. This is stated in the record. The unmodified script fails its final phase on this
  record, and its output is kept.
- **A3.** I did not reproduce the red-first claim that C4-m1 to C4-m3 survive on the reconstructed C03 checker. I
  confirmed the kills on the C04 checker only. The `*-on-c03-checker.log` files exist.
- **A4.** The C02-R7 closure row keeps "36 added" before its C04 correction, and line 3 of the record still reads
  "pending independent review of C04". The approval note appended to the record supersedes line 3. Both are wording
  only.
- **A5.** The closing edits (the roadmap status and the approval note in the record) come after the FIN logs. A
  re-run of the checker after this commit fails `CHK-I45-diff-scope` against the frozen roadmap copy, by
  construction. The FIN logs describe the candidate as reviewed. The closing edits are status and approval text
  only, the same as in the RA01-C-C02 close.
- **A6.** The six lost first-attempt probe logs (C03-R4) cannot be recovered. Later rounds keep
  `<id>.attempt-<n>.log` before a re-run.
- **A7.** The three C03 FIN1 logs (`record-asserts-c03.txt`, `reprobe-c03.txt`, `identity-compare-c03.txt`)
  describe the C03 record state. They were not regenerated because the identity tool is not in the repository.

## Closing edits

After this review, the roadmap opening and Next work were updated: R15-A1 is approved and committed, and R15-A2 is
next. A short approval note was appended to the round record. The redaction check was re-run on the roadmap, the
record and this review: exit 0 each.

## Limits

- I read the C04 outcome, the staging and unstaged lists, the closure rows and the evidence logs. I did not re-read
  the whole 1900-line record or the contract text, which the C01 review accepted.
- I sampled three probe kills. I did not re-run the 161 D81 mutants of C03.
- No product build or test ran, because this round changes documentation and evidence only.
