# V5-02-R15-A2-C01 review (corrective for R15-A2, execution contract part 2)

Decision: **request-changes**. The contract findings of the R15-A2 review are closed. R4 (preview manifest and
run line), R5 (`dispatch_v1` and the generation wording), R6 (the EX-4 statement) and all five R7 wording items are
fixed in the text, agree with EX-4, P2-1 and D104, and have checker literals that the probes kill. The checker
repair (R2) and the red narrative (R3) are also sound. The round cannot be approved yet, because the finisher's
mutation run reports first-attempt mutants that survived as "mutant-fidelity defects of the finisher". At least one
of them is the R15-A2 plan's own mutant for row I12, and it survives all 123 checks. So CHK-A2-016 does not detect
the invariant it is named for, and the record's "156 KILLED, 1 SURVIVED" is wrong. Two smaller evidence gaps and a
set of stale record statements remain. The main fix is a checker change plus a finisher re-run, so this is not a
records-only change.

Scope: the uncommitted candidate on HEAD `cdcfe829c466bf6338815ff6f5b6ea90ffddc570`. That is the seven contract
documents, the round records [V5-02-R15-A2](V5-02-R15-A2.md) and [V5-02-R15-A2-C01](V5-02-R15-A2-C01.md), the R15-A2
evidence folder `tests/native/v5-02-r15-a2-evidence/` (`checks/`, `red/`, `plan/SUPERSEDED-BY-C01.md`, `README.md`)
and the C01 evidence folder `tests/native/v5-02-r15-a2-c01-evidence/` (`plan/`, `probes/`, `green/`). I read the
bytes of the contracts, the checker, the red logs, `PROBES.md`, `mutants.tsv` and the per-mutant logs. I applied no
mutant. I ran no script in place under `tests/native/`. The checker and the link check ran only as `cmp`-identical
copies in my scratchpad, read-only against the checkout.

## Candidate identity (workflow `identity.py`, run by me, verbatim)

Command, from the repository root: `python3 <identity.py> docs/vernacular/command-contract.md docs/vernacular/execution-contract.md docs/vernacular/execution-design.md docs/vernacular/external-profile-contract.md docs/vernacular/ledger-contract.md docs/vernacular/load-command-contract.md docs/vernacular/render-contract.md` (exit 0).

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

This equals the C01 record's Identity section and the identity table in `probes/PROBES.md` byte for byte. The
reviewed contract text is the text the finisher probed.

## Closure of the R15-A2 findings

| Finding | Verdict | Basis |
| --- | --- | --- |
| R4 preview manifest `run` | closed | The P2-2 manifest row `run` (`execution-contract.md:1079`) gives `{kind:"preview"}` with no `request` key. The paragraph at `:1091` keeps 14 keys and says which are computed as for a dispatch, with `generation` the current ordinal, and which come from the preview packet's own bytes. The preview line is `run request: (preview)` (`:1047`). `(` and `)` are outside `[A-Za-z0-9_.:-]`, so it never equals a legal request ID (`:1057`, D154). `run request: none` occurs nowhere. CHK-A2-144 to 146 exist, FAIL on base and stub, PASS on the final text, and are killed by I112 to I114c. |
| R5 `dispatch_v1` | closed | One normative bullet defines it (`:1034`): packet mode, section order with `run` second, the two authority templates, run-section bytes and the `dispatch_manifest_v1` recipe, with a new identifier for any semantic change and "carries no data". P2-3 (`:1102`), P2-7 (`:1199`), D120 and both render pointers (`render-contract.md:253`, `:416`) say the generation is the manifest `generation` key and the authority `{generation}`, not a W field. `grep 'carried by'` over the seven documents finds only unrelated EX-4 prose and the negated D155 clause. CHK-A2-147 to 150 are killed by I115 to I118b, X02 and X05. |
| R6 EX-4 citation | closed | P2-5 (`:1137`) and D143 (`:1461`) now say "EX-4 does not spell an absent job `none`, because `none` is a legal job ID". This agrees with EX-4 (`:679`), P2-1 (`:921`) and D104. `EX-4 rejects` occurs nowhere. The R15-A2 record's reviewer note on D143 (`V5-02-R15-A2.md:162`) is corrected. CHK-A2-151 and 152 are killed by I119, I120 and X01. |
| R7-1 A-3 token sentence | closed | `:539-540` is a "never shown in" list ending "in logs or in error details", then a separate sentence that the claim and takeover results carry the token only in `run.token`. The A-3 row matches. |
| R7-2 load pointer | closed (placement accepted) | The not-shipped sentence moved before "and are:", so the colon now introduces the help block (`load-command-contract.md:21`). C01-D8 explains why it is not after the block: a following paragraph would lose the `Pointer (R15-A2):` label and the gate sentence. I accept this placement. |
| R7-3 F3 to F8 | closed | `:1352-1357` say "carried from the RA01-C coverage gaps". |
| R7-4 traceability labels | closed | `R15-4 Claim` cites the `claim_v1` response and the checklist retry-bytes item (A23, `:1349`). `R15-5 Takeover`. Both match `execution-design.md:597` and `:605`. |
| R7-5 lead sentence and A-2 | closed | A-11 lists the lead-sentence change with old and new text (`:1225`). A-2 reads "Lines 518 and 527 to 528" (`:1216`). |
| R2 CHK-A2-022 | closed (with a carried gap, see C01-R1) | `allowed_messages` pairs backticks per line and per table cell (`contract_checks.py:635-644`). Guard CHK-A2-160 PASSes on base and stub. I130 (new message) and I131 (`Unknown selected job` removed from the base copies) are both killed by CHK-A2-022, so both directions are shown. I129 and I132 (whole-file pairing restored) are killed by CHK-A2-160 and CHK-A2-000. |
| R3 red narrative and snapshot | closed | `red/RED.md` is the checker narrative and states the pin (base commit plus the 20 `cmp` exits of `tree-verify.txt`, and `tree-verify.attempt-2.txt` for the new trees). The ctest table is `red/superseded/RED-ctest-table.md`. `red/snapshot-identity.txt` labels `e3b0c442...` as the empty diff and pins the ten files by `git ls-tree` blob IDs and `shasum -a 256`. I checked that each `base/` file is `cmp`-equal to `git show cdcfe829:docs/vernacular/<file>` (10 exits 0) and that `git hash-object base/execution-contract.md` equals the listed blob `1a5398f9...`. Nothing is deleted: attempt-1 files are kept and `red-run.attempt-1.txt` is `cmp`-equal to `red/checker-run.txt`. |
| R1 red causes and probes | partly closed (see C01-R1, C01-R2) | Red attempt 2 (`pass=8 fail=115`, exit 1, 0 Traceback) and the headings-present stub run (`pass=10 fail=113`; 96 literal, 15 structure, 2 diff causes, 0 heading causes) are present and classified honestly in `RED.md` sections 4 to 6. The only expectRed checks passing on the stub are CHK-A2-001 and 003, with their reasons. The finisher probed I01 to I111. But first-attempt survivors were relabelled rather than reported (C01-R1), and one retargeted mutant does not break its invariant (C01-R2). |

The R15-A2 advisories are handled as the record says: A4 (record anchors) is fixed, and the finisher's link check
reports 145 links and 0 problems in the R15-A2 record. A2 is recorded as an open item, and A1 and A3 are carried to
R15-B/C.

## Findings

### V5-02-R15-A2-C01-R1 (blocking, checker and evidence): surviving mutants reported as finisher defects; CHK-A2-016 is hollow for its own row

`probes/PROBES.md` ("Earlier attempts kept") and the record's Final results say that the first-attempt mutants of
I07, I12, I18, I21, I29, I47, I60, I63, I69 and I74 "did not touch the literal the named check pins, so they
survived or had an empty diff", and that "those first attempts were mutant-fidelity defects of the finisher, not
survivors of the checker". The headline count is then "156 KILLED, 1 SURVIVED". For three rows the logs contradict
this.

- **I12, CHK-A2-016 `exempt_reads`.** The plan mutant (R15-A2 `plan/INVARIANTS.md`, row I12) is "Move `preview` into
  the exempt list". `probes/raw/I12.attempt-1.log` does that: it inserts `` `preview`, `` at the head of the exempt-read
  list at `execution-contract.md:987`. The result is `exit 0`, `SUMMARY pass=123 fail=0`. The row's invariant
  ("Exempt reads are exactly: ...") is broken and the contract contradicts itself, yet no check fails. The reason is in
  the source: `CHK_A2_016_exempt_reads` (`contract_checks.py:947-953`) only tests that four substrings are present.
  An extra member, for example `` `activate`, ``, or `init` written back into the list (the row's second mutant), keeps
  every substring. The row's observer column promises "the exempt-read list, the bound list ...; absence of `init`
  from the exempt list". The check implements neither the exact list nor the absence. The retargeted `I12.log`
  rewrites "`preview` is bound, not exempt" instead, which only shows that the sentence literal is pinned.
- **I07, CHK-A2-011 `expect_job_spelling`.** `I07.attempt-1.log` changes "The uppercase value `NONE` is the
  no-active-job expectation" to "The lowercase value `none` is the no-active-job expectation" (`:921`). That reverses
  D104 and is a real break of the row's invariant ("no-active-job spelling is `NONE`"). It passes 123 of 123. The
  check (`:909-915`) pins `` `--expect-job` is `NONE` or a job ID `` but not the sentence that says which value means
  "no active job".
- **I18, CHK-A2-022 `no_new_message`.** `I18.attempt-1.log` adds `` `Role is not claimable` `` to refusal step (7) in
  the same shape the contract already uses for an existing message: "then `X` and field ..." (`:972` introduces
  `Unknown selected role` this way). CHK-A2-022 PASSes; only CHK-A2-017 fails, because that line's literal moved.
  `message_strings` (`:534-535`) only extracts a backtick string directly after the word `message` or `messages`.
  So the "then `X`" form is invisible, and so is every cell of a Part 2 Message column, for example the provenance
  input table at `:1122-1129`. The plan's observer column defines it this narrowly, so this is partly a stated design.
  But the invariant says "every quoted message string in P2-1 to P2-7", and the closed message table is a standing
  lesson. The record should either widen the extraction or state the limit. Calling this attempt a finisher defect
  hides it.

Three more first attempts survived all 123 checks with meaning changes off their own row: `I21.attempt-1.log` drops
the name `explain` from the D152 retired-role clause (`:983`, row I109 territory), `I60.attempt-1.log` drops "of RA-7"
from the P2-6 context sentence (`:1154`), and `I69.attempt-1.log` deletes "it is mutually exclusive with all other
selectors and paging" from the ledger pointer (`ledger-contract.md:454`). These are true finisher misses for their
rows, but they show unpinned normative clauses. I list them as optional strengthening. I07 and I12 are not optional.

Fix: (1) make CHK-A2-016 compare the exempt-read list in P2-1 (and the D108 lists) with the exact expected members,
and check that `init` is absent from it. Make CHK-A2-011 pin the "uppercase value `NONE` is the no-active-job
expectation" sentence. For CHK-A2-022, either extract message strings from the "then `X` and field" form and from
Message-column cells of Part 2 tables, or record the narrower observer as an accepted limit in the record and in the
plan supersession note. Keep every check name. (2) Have the change reviewed as a checker change. Re-run the red run
and the stub run as attempt 3, keeping attempt 2. (3) Re-run the finisher. Replay each first-attempt mutant above as
its own `mutants.tsv` entry with its true verdict, keep the earlier logs as `attempt-<n>`, and correct `PROBES.md`
and the record so that every survivor is counted as a survivor. A first attempt is a fidelity defect only when its
diff is empty or does not change meaning (I63 attempt 1, empty diff, is one; I29, I47 and I74 attempt 1, which other
checks killed, are fine as stated).

### V5-02-R15-A2-C01-R2 (blocking, evidence): the retargeted I63 mutant does not break the invariant

Row I63 (CHK-A2-092 `cc_services_rows`) states the command-contract pointer's services statements. The final
`probes/raw/I63.log` rewrites "Services: the five families have the service lists of [P2-1]" as "Service lists: the
five families have those of [P2-1]" (`command-contract.md:57`). That sentence means the same thing. The kill shows
only that the literal is pinned, not that the check detects a broken services statement. The plan mutant ("Delete
the `preview` services statement") has no target, because the pointer refers to P2-1 for all five families. That is
why attempt 1 had an empty diff.

Fix: in the finisher re-run, give I63 a meaning-changing mutant, for example delete "`read_history` gains the
selector `event`", change `read_execution({})` to another service name, or drop "the five families have the service
lists of [P2-1]". Record the deviation from the plan text in `PROBES.md`.

### V5-02-R15-A2-C01-R3 (blocking, evidence): redaction does not cover the R15-A2 evidence files that will be committed

`green/redaction.txt` checks 25 files: the 11 documents and 14 files under `tests/native/v5-02-r15-a2-c01-evidence/`.
The commit plan stages the R15-A2 evidence data with `git add -f` too (`V5-02-R15-A2.md:129`, C01 Agreed
verification: "never a file that fails redaction"). That includes `checks/contract_checks.py`, `checks/fixtures/*`,
`README.md`, `red/RED.md`, `red/snapshot-identity.txt`, the red and stub run logs and `plan/SUPERSEDED-BY-C01.md`.
None of these is in the log. No record names the exact staging list for that folder either.

As a diagnostic (not round evidence), I ran `python3 scripts/redact-local-paths.py --repo-path <repo> --home <home>
check <file>` once per non-`raw/` file of `tests/native/v5-02-r15-a2-evidence/`. Every file exits 0 except
`red/superseded/RED-pre-redcheck3.md` (exit 1), which the record already keeps uncommitted. So I expect no content
change, but the round evidence must show it.

Fix: in the finisher re-run, name the exact files of both evidence folders that will be staged, and run the
per-file redaction over all of them together with the documents and the control.

### V5-02-R15-A2-C01-R4 (blocking, records): the records describe the state before the finisher and name a file that no longer exists

The finisher appended "Final results" but left the earlier sections unchanged. These sections now contradict it or
the files on disk:

- C01 record: "Pending finisher outputs" (`:244-247`), Acceptance rows 7 ("Not met", `:318`) and 9 (`:320`), Open
  item 7 (`:330`), and Limits ("Not claimed: any green checker run ... none exists", `:368`). They also put `green/`
  under `tests/native/v5-02-r15-a2-evidence/`, but the finisher wrote it under `tests/native/v5-02-r15-a2-c01-evidence/`.
  They name `raw/redact-exits.txt`, `raw/redact-control.txt`, `green/identity.txt` and `green/FIN.md`, which do not
  exist (the redaction log is `green/redaction.txt`).
- C01 record: Files edited (`:151`), Results (`:229`, `:242`) and Limits cite `probes/raw/baseline.log` and "48 logs,
  one attempt each, no `mutants.tsv`". `probes/raw/baseline.log` no longer exists. The finisher renamed the
  prober-stage logs to `baseline.attempt-1.log` and `<id>.attempt-1.log` (their 22:52 times are kept, so nothing is
  lost), and `mutants.tsv` now exists. `PROBES.prober-stage.attempt-1.md` still points at `raw/<id>.log`, which now
  holds the finisher's replay.
- Files edited omits the finisher's own outputs and its edit of this record (the Final results section).
- R15-A2 record: Verification item 10 (`:455`), Open item 5 (`:466`), Acceptance rows 7 and V5 (`:484`, `:490`) and
  "Pending the docs evidence finisher" (`:505-508`) still say pending.
- R15-A2 evidence `README.md`: the `probes/`, `green/` and `raw/` layout rows and the "State at the pre-review record
  sync" section say the finisher outputs are not produced, and name files in this folder that the finisher wrote
  elsewhere.

Acceptance 9 ("never claims a file or result that does not exist") fails on the `baseline.log` citation, and the
rest is contradictory state. This is a records-only edit, but the record must be synced again after the re-run that
C01-R1 to C01-R3 need. Do the sync before the finisher, and let the finisher write only its own section, as now.

## Advisories (not blocking)

- **A1, wording.** The new sentence "The preview spelling `(preview)` contains characters outside the request ID
  grammar ..." (`execution-contract.md:1057`) now sits between the purpose table and "In R15 these verbs are
  reserved." (`:1059`), so "these verbs" no longer follows its antecedent. Move the preview sentence into the
  run-section bullet, right after "`run request: (preview)` (preview)", in the next round that edits this file.
- **A2, X09.** I accept the stated limit: CHK-A2-042 counts the `(preview)` literal at least once, and CHK-A2-146
  kills a one-site respelling.
- **A3, C01-D2 and C01-D18.** I accept both choices. The current ordinal is always defined after activation (row M1
  opens a generation for every role), it equals `status_v1` `roles[].generation`, and the record does not claim that
  `sections` always differ. The R16 `--since` item (open item 1) is the right place for the comparison rule.
- **A4, allowed-message breadth.** The repaired set is every backtick string on any line of ten base files, so any
  identifier or value spelled in backticks there counts as "existing". This was also true before the repair and does
  not reopen R2. A later round could limit the set to the Messages tables and message cells.
- **A5, CHK-A2-005.** The flag disagreement with plan row I134 (open item 4) is honestly recorded. I134 is killed
  either way.

## What I verified

- **Identity.** My `identity.py` output (above) equals the record and `PROBES.md`. The probe logs' `RESTORE sha256`
  lines refer to the same `execution-contract.md` bytes.
- **Green run (my diagnostic, not round evidence).** A `cmp`-identical scratch copy of `checks/` run as
  `python3 -I -B <scratch>/contract_checks.py --repo <repo> --base <repo>/tests/native/v5-02-r15-a2-evidence/base
  --git-repo <repo> --base-commit cdcfe829...` exits 0, 123 PASS lines, `SUMMARY pass=123 fail=0`. `--selftest` exits 0
  with `SUMMARY pass=1 fail=0`. The copied `link_check.py` exits 0 with `PASS LINK-A2-001`. These match
  `green/checks.txt` (exit 0, 123 PASS, 0 FAIL) and `green/linkcheck.txt` (exit 0, 11 documents, 0 problems,
  negative control 2 problems).
- **Probe evidence.** `mutants.tsv` has 157 rows. The verdicts and killing checks match `PROBES.md`. Every listed log
  exists, has a non-empty diff (the I63 attempt-1 empty diff is a kept earlier attempt) and ends with a restore line
  whose `match=True` holds. The positive control `baseline.attempt-2.log` is `pass=123 fail=0`. The C01 rows I112 to
  I148 and X01 to X08 are killed by the named check, X09 survives CHK-A2-042 as stated, and I131 removes the
  existing message from the base copies (the reverse direction). C01-R1 and C01-R2 are the exceptions.
- **Invariant table.** The record's rows I112 to I150 equal `tests/native/v5-02-r15-a2-c01-evidence/plan/INVARIANTS.md`
  in Invariant, Mutant and Test (39 of 39, compared by script). Each acceptance criterion and each closed finding has
  a row. The rows I137 to I150 are file-state or reviewer rows. I checked the facts they name: the attempt-1 copies,
  the two 123-log folders, `RED-A2-002_attempt2.log`, `tree-verify.attempt-2.txt` with 40 exits of 0, the absence of
  host paths in `RED.md`, and the plan supersession markers in the R15-A2 record (`:230`, `:306`, `:322`, `:344`).
- **Red evidence.** `red-run.txt` and `stub-run.txt` summaries, exits and Traceback counts are as stated. The FAIL set
  of the red run is the 115 expectRed checks and the PASS set is the eight guards.
- **Check names and messages.** All 106 check names of the attempt-1 run are among the 123 of attempt 2 (`comm -23`
  is empty), so nothing is removed or renamed. `external-profile-contract.md` has no removed line. The four EX-7
  Messages rows changed by R15-A2 keep their message strings byte for byte; only the site cells gained text.
- **Scope.** `git status` shows no `src` or `tests` change outside the evidence folders. `command-contract.md`,
  `ledger-contract.md`, `external-profile-contract.md` and `execution-design.md` have the same hashes as in the R15-A2
  review, so C01 did not touch them.
- **Hash pins.** The records pin only the seven contract documents (current, plus the historical R15-A2 values in
  the R15-A2 Review outcome) and the empty-diff value. No other record, review, REPORT or the roadmap is pinned.
  `red/snapshot-identity.txt` uses the scoped `base/` exception that C01-D14 states.
- **Host paths.** My redaction diagnostic (C01-R3) found no machine-local path in any file of the R15-A2 evidence
  folder except the known `red/superseded/RED-pre-redcheck3.md`.

## Limits

- I applied no mutant. The survivor statements in C01-R1 rest on the finisher's saved logs (diff, command, exit and
  `SUMMARY` lines), read together with the check source.
- My checker, link-check and redaction runs are diagnostics, not round evidence.
- The evidence folders are gitignored, so `identity.py` gives no hash for the checker. My green run used a copy that
  is `cmp`-equal to the checkout file at review time.
- No files were added under `tests/native/v5-02-r15-a2-c01-review-evidence/`. Every command above is short and stated
  inline.
