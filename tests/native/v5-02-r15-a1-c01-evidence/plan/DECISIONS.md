# R15-A1-C01 decisions table and scope map (search stage, revision 4, 2026-11-01)

Round: V5-02-R15-A1-C01 (documentation corrective, plus checker and invariant repair in the evidence area). Base: the
uncommitted R15-A1 candidate on HEAD `d536922b`. Authoritative plan; `INVARIANTS.md` in this folder is its companion.
Revisions 1 to 3 are in `superseded-1/`, `superseded-2/` and `superseded-3/`. Revision 4 closes plan-review gaps 1 to 11 of
revision 3: (1) wave order, the green core run now happens after R1 (V1 in wave 4, R1 in wave 3); (2) results reporting, a SYNC package
(wave 9) after FINAL copies the final results into the record and REPORT, and a CLOSE package (wave 11) re-runs the audit on the final
record; (3) every mutant of C31 and C61 is probed (PROBER2), plus the final-results mutants (PROBER3); (4) the prober overlay
includes the whole old evidence tree `tests/native/v5-02-r15-a1-evidence/`; (5) C23 is expectRed true; (6) C16 has an exact
predicate (C.23); (7) D54, D67 and D68 carry the exact tag and are checked (C21, C42, C.22), and the `:672` cite is removed from the
relabel scope (no pending wording there); (8) D100 and C.4 say native-writer enforced and reader-verified only, no SQL CHECK, and
state that R17-3 lists no `pass` reason; (9) C36 names a real line; (10) C59 invariant and test agree; (11) the original-checker run has a
scratchpad copy (C.24). Revision 3 closed plan-review gaps 1 to 13 of revision 2 (run_id
replay restricted to the target role, before_drifted NULL wording, 35/105/99 counts, D100 phrasing, contiguous ordinals from 1, full
D81 splitting verbs, redaction of committed evidence, correction notes in the old plan files, exact PB text C.20, record-rule
basis, D102 tag, R6 baseline cite, standalone clone for the prober). Revision 2 closed plan-review gaps 1 to 11: D100 fallback (no evidence CHECK), no record
hash in evidence, evidence-audit rows moved to an audit phase, full D81 prober, full relabel (including the old plan files
and the record's copied invariant table), corrected expectRed values, exact text for R4/C5/R8/R12/D97-D103, D102 fixed as
operator-decided, D104 moved out of the contract, script locations, the R6 closure row, and an owner for REPORT.md.
The earlier plan (`tests/native/v5-02-r15-a1-evidence/plan/`, revision 4, D1 to D96) keeps its D-numbers; this file adds D97 to
D104 plus amendments to D43, D55, D56, D54, D67, D68 and OP1 to OP3.

Tags: **C** contract-decided (cited), **D** decided in `docs/vernacular/execution-design.md` (EX or R section, or the E1 to E17
decisions table, accepted 2026-10-28), **Q** decided by operator decision Q1..Q18 / Q-INIT / Q-G35 in
`remaining-steps-design.md`, **NEW-eng** a new engineering choice made inside closed text (none changes product behaviour or an
approved clause), **DELEGATED** an operator choice accepted under the standing delegation of 2026-10-31 (option A), recorded as
delegated and never as a live answer, **NEW-op** a new product or authority choice (the author must halt).

Result of the search: no row is NEW-op. There is no operator question to raise. D100 applies the dispatch's own fallback
(HALT-E): the evidence reason set is reader-verified only, no SQL CHECK, so R17 changes no stored format3 SQL.

Test strategy note. This is a docs round: no TEST_CASE, no product build, no source change. "Test" means a named scratch
check `CHK-*` in the repaired `ex_checks.py`. Only the mutation-prober packages (PROBER, PROBER2) apply mutants, always in a
scratch copy outside the checkout; coders and repair coders never mutate, stash, revert or experiment with checkout files. D81 is
resolved as RUN (D104), over every mutant of the I1 to I70 rows (amended text) plus the C and G rows, including every mutant of C31 and C61
(PROBER2) and of the final-results clauses (PROBER3).

### Script and checker execution rules (binding, answers dispatch :567-569)

1. Coders and reviewers run only the workflow `identity.py` and the scratchpad scripts. Scratchpad scripts:
   `<scratchpad>/ex_checks.py` (canonical), `<scratchpad>/ex_checks_orig.py` (byte copy of the pre-edit checker, C.24), `<scratchpad>/sql_c01.py`, `<scratchpad>/linkcheck.py`. The copies under
   `tests/native/v5-02-r15-a1-c01-evidence/` (`red/ex_checks.py`, `green/sql_c01.py`, `green/linkcheck.py`) are evidence copies:
   byte-identical to the scratchpad files (proved by `cmp`, no hash written), and never executed from `tests/native/`.
2. `ex_checks.py` has two phases. Default run = core checks (every registered id except the six audit ids). `--audit` = core plus
   the six audit checks (CHK-C01-sql-evidence-recorded, -green-evidence, -prober-evidence, -record-closure, -red-evidence,
   -record-d81), which audit evidence files written by other packages. A default run prints the line
   `audit checks not run: 6 (use --audit)` and counts them neither as PASS nor FAIL. Options: `--repo <root>`,
   `--evidence-dir <dir>` (default `<repo>/tests/native/v5-02-r15-a1-c01-evidence`), `--frozen-dir <dir>` (default
   `<scratchpad>/frozen`), `--audit`. The checker derives counts from its registry; it hard-codes none (expected today: 105
   registered ids = 70 old + 35 CHK-C01 [C1 to C33, C35 extractor-comments, C61 record-d81], 6 audit, 99 core).
3. The prober scratch tree is a STANDALONE clone, never a worktree of the shared repository (a worktree would register itself in the
   shared `.git`): `git clone --no-hardlinks <repo root> <scratchpad>/probe-tree && git -C <scratchpad>/probe-tree checkout --detach
   d536922b`, then the candidate working-tree files are overlaid by `cp` (every modified or untracked file of `docs/vernacular/`,
   `CLAUDE.md`, the `AGENTS.md` symlink, the whole C01 evidence folder AND the whole old evidence tree
   `tests/native/v5-02-r15-a1-evidence/` including `plan/`, `README.md`, `REPORT.md`, `review/`, `raw/`, `red/`, `green/`), because C18,
   C27, C28, G4 and G8 call `git diff HEAD`, and because both evidence trees are gitignored, so a git clone does not contain them while
   C22 reads and mutates the old `plan/DECISIONS.md` and `plan/INVARIANTS.md` and C52 scans the original README. The overlay is
   made by `cp -R` of each tree into the same relative path of the scratch clone, and the baseline run must show that every file
   those checks read exists in the scratch tree. A pristine copy of the complete overlaid tree (both evidence trees included) is
   kept for `cmp` restore. The checker is run with `--repo <scratch>`, `--evidence-dir <scratch
   evidence>` and `--frozen-dir <scratchpad>/frozen`. Never use `git stash`, `git checkout`, `git worktree` or edits in the real
   checkout. The scratch trees live in the scratchpad and need no cleanup; raw logs live under `probes/raw` and are never deleted.
4. Record rule (acceptance 7): no hash of `docs/vernacular/rounds/*.md`, `roadmap.md`, any REPORT or review file is written
   anywhere. `identity.py` is run only with explicit pathspecs that leave those out:
   `IDSPEC_DOCS` = `docs/vernacular/execution-contract.md docs/vernacular/v5-alignment.md docs/vernacular/ledger-contract.md
   docs/vernacular/external-profile-contract.md docs/vernacular/acceptance-matrix.md docs/vernacular/implementation.md CLAUDE.md`
   and `IDSPEC_SRC` = `src tests` . The roadmap, the round record (until R1 edits it) and the review file are proved frozen by
   byte comparison (`cmp`) against copies made by T1 in `<scratchpad>/frozen/` (`roadmap.md`, `V5-02-R15-A1.md`,
   `V5-02-R15-A1-review.md`); only the `cmp` verdict (identical or differs) is written to evidence.
   Basis of the record rule (acceptance 7 and `red/red-run.txt`, `raw/baseline-identity.txt`, the identity outputs): the workflow
   mandates `identity.py` candidate identity; lesson 5 requires a hash of the pre-fix snapshot; the R15-A1 review's "Record rule"
   paragraph treats candidate-document identity (per-file sha256 of the IDSPEC_DOCS contract documents and `CLAUDE.md`) as compliant.
   What is excluded is only the hash of a record, review, REPORT or the roadmap, because the record would contain its own hash.
   Evidence text files carry command lines with `<scratchpad>` and `<repo root>` placeholders, never host paths (C.19).
5. Original-checker run (C33, raw/orig-checker-run.txt). No script is executed from under `tests/native/`. Before T1 edits anything:
   `cp <scratchpad>/ex_checks.py <scratchpad>/ex_checks_orig.py` and `cp <scratchpad>/ex_checks.py <repo root>/tests/native/v5-02-r15-a1-c01-evidence/raw/ex_checks-r15a1-orig.py`, then `cmp` the two copies (verdict only, no hash); then run, unpiped,
   `python3 -I <scratchpad>/ex_checks_orig.py --repo <repo root>` and save its exit code and full output (13 FAIL lines) to
   `raw/orig-checker-run.txt`. The original checker has no `--evidence-dir`, `--frozen-dir` or `--audit` option and needs none.
6. Wave order (revision 4, binding, see C.25): T1 (1); PA, PB (2); R1 (3); V1 (4); PROBER (5); R2 (6); PROBER2 (7); FINAL (8);
   SYNC (9); PROBER3 (10); CLOSE (11). The green core run (V1) runs only after PA, PB and R1, because C22 and C43 pass only on the
   relabelled record and old plan files; the red run (T1) runs before them. C30 (`green/checks.txt`) depends on waves 1 to 4; C32
   depends on R1 (closure table), V1 and T1 (the files its R6 row cites), R2 (the `probes/PROBES.md` cite) and, for its final-results
   clause, FINAL and SYNC; C61 depends on R1, R2, PROBER and PROBER2, and for its verdict clause on SYNC. Packages that write an
   audited file run the audit AFTER the file exists and never audit a file they are about to write in the same run.

## A. Exact scope with file:line citations (candidate as it stands at plan time; locate by text before editing)

All lines are in the uncommitted candidate. A package must re-locate each passage with `grep -n` before editing, because
earlier edits in the same wave shift lines.

| Finding | File:line | What is there now | Change |
| --- | --- | --- | --- |
| R1 | `docs/vernacular/execution-contract.md:534-535` (Run token, second bullet) | "The token is shown only in the claim and takeover results, and in `history --request-id` ... It is never shown in `status`, in a history event expansion ..." | text C.6 |
| R1 | `execution-contract.md:510-515` (Disposition paragraph) | reports disposition only | add sentence C.12 |
| R1 | `execution-contract.md:1013` (A09 acceptance item) | "recovered by request lookup" | text C.13 |
| R2 | `execution-contract.md:675` (Event payload paragraph) | "the row with `ordinal` 0" (contradicts `CHECK(ordinal>0)` at :298) | text C.11 |
| R2 | `execution-contract.md:66-67` (C5) | counts; ordinal above 0 | append C.11b |
| R2 | `execution-contract.md:1040` (D43) | "comes from ordinal 0" | row C.14 |
| R3 | `execution-contract.md:1076` (OP1 row) | "Flags cannot be required (...:266-267); this changes that rule for this one option." | row C.15 |
| R4 | `execution-contract.md:262-272` (`role_state` SQL) | no coupling of `run_id`/`drifted` to `state` | SQL C.1 (D97) |
| R4 | `execution-contract.md:296-308` (`transitions` SQL) | `after_drifted` unconstrained | SQL C.2 (D98) |
| R4 | `execution-contract.md:433` (invariant 6), `:441` (invariant 12), `:645-649` (EX-4 States) | replay and coupling text | appends C.9, C.10, C.10b (D99). No bullet 21 |
| R5 | `execution-contract.md:68-69` (C6) | "`evidence.reason` has no value list in the design" (false) | replace with C.4 (D100 fallback) |
| R5 | `execution-contract.md:201-216` (`evidence` SQL) | `reason TEXT NOT NULL,` no CHECK | NO SQL change (D100 fallback); keep `-- Reserved for R17` label |
| R6 | `ex_checks.py` `sql_statements` (~:347-358) | a statement preceded by a `-- Reserved for ...` line fails `TABLE_RE.match`, so `f3()` drops every reserved table | strip leading `--` lines from each completed statement |
| R6 | `ex_checks.py` `_i8`, `_i39`, `_i42`, `_i46`, `_i54`, `_i57`, `_i60`, `_i64`, `_i66`, `_i69`, `_i33`, `_i32`, `_i59`, `_i48`, `_i67` | see INVARIANTS C39-C57 | repair or amend, names kept |
| R7 | `execution-contract.md:5-7` (header) | "Part 1 holds the header, the common rules and clauses EX-1 to EX-3 ... follow" | text C.7 |
| R8 | `execution-contract.md:860` (note under the role table) | "The before cells of M5, M7 and M8 are every non-running, non-retired state, and M13 and M14 apply to the same before states for each cause." | text C.16 |
| R9 | `execution-contract.md:1047` (D56), `:1046` (D55) | D56 `operator-decided`, basis "EX-8"; D55 lists "internal" | rows C.14 |
| R10 | `docs/vernacular/v5-alignment.md:95-101` (E2), `:102-107` (E5) | both point to "EX-2 for the request" | PB brief |
| R11 | `execution-contract.md:467` (cutover bullet 1) | "at the same explicit target as format2" | "at a new explicit target" |
| R12 | `execution-contract.md:797` (EX-8 Check output row, "Existing site" cell) | R17 outcome shown as an existing site | cell C.17 |
| Relabel (contract) | `execution-contract.md:517, 856, 865, 1025, 1076-1078` (the only lines with pending wording; `:672` has none and is NOT in scope) | "operator-pending, recommended option applied", "Operator question OP2 is pending" | C.8, C.15, C.18 |
| Relabel (contract, HALT-1) | `execution-contract.md:1045` (D54), `:1051` (D67), `:1052` (D68) | tag cell `operator-decided (delegated, standing delegation of 2026-10-31, option A)` | tag cell becomes exactly `operator-decided (delegated 2026-10-31, option A)` (C.22); checked by C21 and C42 |
| Relabel (record) | `docs/vernacular/rounds/V5-02-R15-A1.md:76-78, 127, 207-213, 345-348, 360-361, 408-409, 435` and the copied invariant table rows I32 (:408), I69 (:445); the HALT-1 section (b) and the HALT-1 bullet (:347) | operator-pending wording; HALT-1 text lacks the exact tag | R1 (see B package brief); HALT-1 section and bullet state `operator-decided (delegated 2026-10-31, option A)` for D54, D67, D68 (checked by C43) |
| Relabel (old plan) | `tests/native/v5-02-r15-a1-evidence/plan/DECISIONS.md:10, 103, 171-177` (OP-PENDING tag definition, D26, OP rows), `:273` (derivation note) and `INVARIANTS.md:57` (I32), `:94` (I69) | operator-pending wording; I69/derivation note encode the R8 and M8/M10 defects | R1 relabels and appends C01 correction notes (the dispatch owns these files) |
| Record invariant table | record rows I32 (:408), I33, I54, I57, I60, I67, I69 (:445) | encode R1/R2/R3/R6 defects | R1 appends a correction note to each row pointing to INVARIANTS C39-C56 (append-only) and relabels I32 and I69 |
| Old plan correction notes | `tests/native/v5-02-r15-a1-evidence/plan/INVARIANTS.md` rows I33, I54, I57, I60, I67, I69 and `.../DECISIONS.md` rows D43, D55, D56 | encode the defects of R1, R2, R3, R6, R9 | R1 appends, append-only, a note beginning `Corrected by C01:` with the pointers in C.21 (checked by C22) |

Files that must stay byte-identical to the pre-C01 candidate: `CLAUDE.md`, `AGENTS.md`, `docs/vernacular/ledger-contract.md`,
`external-profile-contract.md`, `acceptance-matrix.md`, `implementation.md`, `roadmap.md`, every file under `src/` and `tests/native/`
outside the evidence folders (the relabel needs no edit in them: grep finds no pending wording).

Owned-file list for the round record (lesson 4). Edited: `docs/vernacular/execution-contract.md`, `docs/vernacular/v5-alignment.md`,
`docs/vernacular/rounds/V5-02-R15-A1.md`, `docs/vernacular/rounds/V5-02-R15-A1-review.md` (reviewer only, appended),
`tests/native/v5-02-r15-a1-evidence/plan/DECISIONS.md` and `INVARIANTS.md` (relabel and correction notes only), scratchpad
`ex_checks.py`, `sql_c01.py`, `linkcheck.py`, `frozen/*` copies, and under `tests/native/v5-02-r15-a1-c01-evidence/`: `plan/`,
`red/`, `green/`, `probes/`, `raw/`, `README.md`, `REPORT.md`. The record's owned list must add the C01 evidence folder (R1 does).
Raw logs under `v5-02-r15-a1-evidence/` are never deleted or edited.

## B. Decisions

| # | Choice | Tag | Basis |
| --- | --- | --- | --- |
| D97 | `role_state` couples `run_id` and `drifted` to `state` by one table-level CHECK: `CHECK((state='running' AND run_id IS NOT NULL) OR (state<>'running' AND run_id IS NULL AND drifted=0))`. A running role may have `drifted` 0 or 1; every other state has `run_id` NULL and `drifted` 0 | NEW-eng | EX-1 invariant 12; `execution-design.md:109`; v5 `:376`; review R4 |
| D98 | `transitions` gets `CHECK(after_state='running' OR after_drifted=0)` and `CHECK(before_state IS NULL OR before_state='running' OR before_drifted=0)` | NEW-eng | D97; EX-3 before-state check |
| D99 | Replay derives `role_state.run_id`: NULL unless the role's last `transitions` row has `after_state` `running`; then `events.run_id` of the carrying event only when that event is `run_claimed` or `run_taken_over` AND its `events.role` (and `events.job`) is that role, that is the operation's target; every other running row (including a derived transition of another role that the same event carries, whose `after_state` may be `running` or `running+drift` by M23) takes the run_id derived for that role's previous `transitions` row (whose `after_state` is also `running`); cross-checked against the one `active` `run_state` row of the current generation. `transitions` has no `run_id` column, so the derivation is by role, not by column | NEW-eng | EX-1 invariants 6, 12, 15; review R4 |
| D100 | `evidence.reason` stays `TEXT NOT NULL` with no CHECK. It is native-writer enforced and reader-verified only, with no SQL CHECK. The set is closed by R17 (execution design EX-1 table: "closed enum, R17"), but R17-3 (`execution-design.md:879-881`, "`pass`; `fail` with reason ...") lists no `reason` value for `pass`, so the set that the writer and reader enforce is not fully defined in part 1; R17 states the `pass` reason value, with no change to the stored format3 SQL. The dispatch allows a CHECK only if the values are fully listed including the pass value | NEW-eng (fallback of dispatch R5 rule, HALT-E applied) | review R5; `execution-design.md:102, 879-881`; record dispatch R5 |
| D101 | The event-level `reason` is the `reason` of the `transitions` row with the lowest `ordinal`, which is ordinal 1 (`CHECK(ordinal>0)` kept), or the empty string when the event has no transition. The `transitions` rows of one event have ordinals contiguous from 1 (native writer and reader), so the lowest ordinal is always ordinal 1. Amends D43 | NEW-eng | review R2; `execution-design.md:231-234` |
| D102 | `status --request-id` of a claim or takeover request returns the saved result and therefore shows the run token; "never shown in `status`" becomes "never shown in `status` other than its request lookup" | operator-decided | E11 A; R15-9; R15-7 |
| D103 | OP1 option A is a required scalar option, an ordinary option of the separate-token grammar, so the external-profile "cannot be required" rule is unchanged | C | `external-profile-contract.md:266-267`; round record choice 1; review R3 |
| D104 | D81 resolved as RUN: the mutation-prober packages (PROBER, PROBER2, PROBER3) run every mutant of rows I1 to I70 (amended text of C39-C57) plus every C and G row mutant, including all mutants of C31 and C61, in a scratch tree outside the checkout, one raw log per mutant id, survivors listed per mutant. NOT in the contract Decisions table; it lives in this plan and the round record only | NEW-eng (process) | dispatch (a) item 4; acceptance 3; review R6 |
| D43 (amended) | Payload v2 with `after` shaped like `before`; `active_job` is the empty string, never `none`; the event reason comes from ordinal 1, the lowest ordinal | NEW-eng | EX-4 event payload; D101 |
| D55 (amended) | Exit mapping: usage 2; semantic, conflict, resource and format 3; I/O, corruption, busy and disk-full 4 | C | `ledger-contract.md:480-481` |
| D56 (amended) | EX-8 has ten rows, checked before allocation; the numbers are new engineering except the E15 row | NEW-eng | EX-8; the E15 row is D78 |
| D54, D67, D68 (relabelled) | tag cell exactly `operator-decided (delegated 2026-10-31, option A)` in the contract (C.22; checked by C21 and C42); in the record the HALT-1 section and bullet carry the same text (C43) | DELEGATED | E16 A; HALT-1 |
| OP1, OP2, OP3 (relabelled) | tag text `operator-decided (delegated 2026-10-31, option A)` in contract, record, old plan files and checks | DELEGATED | standing delegation of 2026-10-31; E1 A; R17-6 |

Contract Decisions-table rows to add: D97, D98, D99, D100, D101, D102, D103 (exact cells in C.14). D104 is absent from the contract.

### Design-choice inventory (Q18 A)

| Dimension | Choice | Tag | Cite |
| --- | --- | --- | --- |
| Module placement and visibility | None: contract text only; destination stays `execution-contract.md` | D | E14 A (`execution-design.md:562`) |
| Threat model | Unchanged: the token secret guards accidental reuse; `status --request-id` adds no reader capability because `history --request-id` already exposes it | D | E11 A; R15-9 |
| Enforcement placement | `role_state`/`transitions` CHECKs in the format3 SQL (native-enforced, reader-verifiable); `run_id` replay derivation reader-side; `evidence.reason` native-writer enforced and reader-verified only, no SQL CHECK (D100) | NEW-eng | D97 to D100 |
| Ordering | Event reason by ordinal 1; no new ordering of commit steps | NEW-eng | D101 |
| Diagnostic mapping against closed tables | No new code, message or changed byte | C | external-profile message tables |
| Budget composition | No change to EX-8 numbers; only the R12 label | C | EX-8 |
| Test strategy for hard-to-observe properties | Text properties by `CHK-*` reading text; SQL properties by executing the extracted SQL in sqlite3 with foreign keys on and probing good and bad rows; the prober proves each check can fail; evidence files are audited in a separate audit phase after they exist | NEW-eng | D104; lessons 1, 5 |
| Platform handling | None (docs). Python `sqlite3` needs SQLite 3.37+ for STRICT | C | review `sql_shape.txt` |

## C. Exact text and SQL (so a smaller model makes no wording choice)

C.1 `role_state` (after `PRIMARY KEY(job,role)`, add a comma and the line):
`  CHECK((state='running' AND run_id IS NOT NULL) OR (state<>'running' AND run_id IS NULL AND drifted=0))`

C.2 `transitions` (after the existing `PRIMARY KEY(event_seq,ordinal),` line and before the existing coupling CHECK):
`  CHECK(after_state='running' OR after_drifted=0),`
`  CHECK(before_state IS NULL OR before_state='running' OR before_drifted=0),`
The existing `CHECK((before_state IS NULL AND before_drifted IS NULL) OR ...)` stays last and unchanged.

C.3 `evidence` SQL: no change at all (D100 fallback). `reason TEXT NOT NULL,` and `output_truncated ... CHECK(output_truncated IN (0,1))`
(last column, no trailing comma) stay byte-identical; the `-- Reserved for R17` label stays.

C.4 C6 replacement (the whole bullet, written as three wrapped lines):
"- C6. Closed enums are `TEXT NOT NULL CHECK(c IN (...))` with the listed values. `evidence.reason` is native-writer enforced and reader-verified only, with no SQL CHECK. The set is closed by R17 (execution design EX-1 table: 'closed enum, R17'), but R17-3 lists no `reason` value for a `pass` outcome, so R17 states that value; no stored format3 SQL changes."
Required phrases (checked by C11, C12, C13): `native-writer enforced and reader-verified only` and `no SQL CHECK` in both C6 and the D100 row; `R17-3 lists no` followed by `reason` and `pass` in C6 and the D100 row. The former phrase "that the native writer enforces and the reader verifies" is NOT used anywhere.

C.5 Counts to record (unchanged by C01): 24 STRICT tables, 43 triggers (19 immutable tables times two, plus 5 mutable tables times one), 2 explicit indexes
(`records_digest`, `run_state_one_active`). PRAGMA `foreign_key_check` clean, `user_version=3` unchanged.

C.6 Run token second bullet: "The token is shown only in the claim and takeover results, in `status --request-id` of those requests, and in `history --request-id` of those requests on format3 (operator decision E11 A, R15-9, R15-7). It is never shown in `status` other than its request lookup, in a history event expansion, in render output, in any packet other than the claim or takeover packet, in logs or in error details."

C.7 Header (:5-7): replace "Part 1 holds the header, the common rules and clauses EX-1 to EX-3. The reconcile rules, the execution bounds, the messages, the state machines and the Part 2 placeholders for R15-A2 follow in this same file." with: "Part 1 holds the header, the common rules, clauses EX-1 to EX-8, the state machines, the Part 2 placeholders for R15-A2, the messages, the pointer amendments, the traceability table, the acceptance checklist and the decisions table. The Part 2 placeholders reserve the content that R15-A2 writes in this same file."

C.8 Decisions preamble (:1025), whole paragraph: "The tag set is closed: `contract-decided`, `operator-decided` and `new-engineering`. OP1, OP2, OP3, D54, D67 and D68 are operator-decided under the standing delegation of 2026-10-31, option A (`operator-decided (delegated 2026-10-31, option A)`), recorded as delegated, not as live answers; the operator may overrule them."

C.9 Invariant 6 (:433), append after "(native writer, by delta, and reader, by replay)." : " For a role whose last `transitions` row has `after_state` `running`, `role_state.run_id` is the `run_id` of the `events` row that carries that row when that event's type is `run_claimed` or `run_taken_over` and its `events.role` (and `events.job`) is that role, that is the target of the operation. Every other running row, including a derived transition of another role that the same `run_claimed` or `run_taken_over` event carries, takes the `run_id` derived for that role's previous `transitions` row, whose `after_state` is also `running`. For every other last `after_state` the `run_id` is NULL. The reader cross-checks the result against the one `run_state` row with state `active` of the role's current generation (invariant 12)."

C.10 Invariant 12 (:441), append at its end: " Every role state other than `running` has `run_id` NULL and `drifted` 0, a `running` role has a non-NULL `run_id`, `before_drifted` of a `transitions` row is NULL when `before_state` is NULL and 0 when `before_state` is a non-running state, and `after_drifted` is 0 unless `after_state` is `running`."
C.10b EX-4 States paragraph (:645-649), append after "...distinct from `done`." : " For every state other than `running`, `drifted` is 0 and `run_id` is NULL; a `running` role has a non-NULL `run_id`, and `after_drifted` is 0 unless `after_state` is `running`."

C.11 EX-4 Event payload (:675): replace "the `reason` of the row with `ordinal` 0 in `transitions` for that event" with "the `reason` of the row with the lowest `ordinal` in `transitions` for that event, which is ordinal 1 (the SQL has `CHECK(ordinal>0)`)".
C.11b C5 (:66-67), append after "...and `ordinal` and `revision` are above 0.": " The `transitions` rows of one event have `ordinal` values contiguous from 1, which the native writer enforces and the reader verifies, so the event-level reason of an event is the `reason` of its row with ordinal 1 ([EX-4](#ex-4-facts-derived-state-and-reconcile-rules))." This is a sentence in C5, not a new invariant bullet.

C.12 Disposition paragraph (:510-515), append after "...(R15-A2)." : " The saved result that `status --request-id` returns for a claim or takeover request carries the run token (R15-7, R15-9)."

C.13 A09 (:1013): replace "A lost response, recovered by request lookup, returns the run token;" with "A lost response, recovered by request lookup (`status --request-id` or `history --request-id`), returns the run token;".

C.14 Decisions table rows (4 cells each, tag cell and basis cell as shown), replacing D43, D55, D56 and appending D97-D103 after D96 and before OP1:
`| D43 | Payload v2 with `after` shaped like `before`; `active_job` is the empty string, never `none`; the event reason comes from ordinal 1, the lowest ordinal | new-engineering | EX-4 event payload; D101 |`
`| D55 | Exit mapping: usage 2; semantic, conflict, resource and format 3; I/O, corruption, busy and disk-full 4 | contract-decided | ledger-contract.md:480-481 |`
`| D56 | EX-8 has ten rows, checked before allocation; the numbers are new engineering except the E15 row | new-engineering | EX-8; the E15 row is D78 |`
`| D97 | `role_state` couples `run_id` and `drifted` to `state` by one table-level CHECK: a running role has a non-NULL `run_id`, every other state has `run_id` NULL and `drifted` 0 | new-engineering | EX-1 invariant 12; EX-4 States |`
`| D98 | `transitions` has two table-level CHECKs: `after_drifted` and `before_drifted` are 0 unless the matching state is `running` | new-engineering | D97; EX-3 before-state check |`
`| D99 | Replay derives `role_state.run_id` from `events.run_id` of a `run_claimed` or `run_taken_over` event only for the role that event's `events.role` names, the operation's target; every other running row takes the run_id derived for the role's previous `transitions` row; the result is cross-checked against the active `run_state` row | new-engineering | EX-1 invariants 6 and 12 |`
`| D100 | `evidence.reason` stays `TEXT NOT NULL` with no CHECK; it is native-writer enforced and reader-verified only, with no SQL CHECK; R17-3 lists no `reason` value for `pass`, which R17 states, so R17 changes no stored format3 SQL | new-engineering | Execution design EX-1 table (closed enum, R17); R17-3 lists no `reason` value for `pass` |`
`| D101 | The event-level reason is the reason of the `transitions` row with ordinal 1, the lowest ordinal, or the empty string when the event has no transition; the ordinals of one event are contiguous from 1 | new-engineering | EX-4 event payload; C5 |`
`| D102 | `status --request-id` of a claim or takeover request returns the saved result and so shows the run token; the token is never shown in `status` other than its request lookup | operator-decided | E11 A; R15-9; R15-7 |`
`| D103 | OP1 is a required scalar option of the separate-token grammar, so the external-profile rule that flags cannot be required is unchanged | contract-decided | external-profile-contract.md:266-267 |`

C.15 OP1 row: "`| OP1 | `--confirm-stopped` becomes a required scalar option whose only accepted value is `true`; any other value is `usage`. A required scalar option is an ordinary option of the separate-token grammar, so the external-profile rule that flags cannot be required (`external-profile-contract.md:266-267`) is unchanged. No other verb grammar is decided in part 1 | operator-decided (delegated 2026-10-31, option A) | E1 A |`". OP2 and OP3: the tag cell becomes `operator-decided (delegated 2026-10-31, option A)`, nothing else changes.

C.16 R8 note (:860): "- The before cell of M5 is every non-running, non-retired state; the before cells of M7 and M8 are every non-retired state, including `running` and `running+drift`; M13 has the same before states as M5, and M14 is the running counterpart of M13, for each cause. A running role with a new blocker stays running and has no row (EX-4)."

C.17 EX-8 Check output row, "Existing site" cell: "None yet. R17 fact (R17-2 and R17-3): over `max_output_bytes`, a recorded `fail` outcome with reason `output_limit`".

C.18 Other relabel cells: :517 "`history --request-id` on format3. Operator question OP2 is pending. This paragraph applies its recommended option." becomes "`history --request-id` on format3. Decision OP2 (operator-decided, delegated 2026-10-31, option A) applies."; and :856 and :865 replace "operator-pending, recommended option applied" with "operator-decided (delegated 2026-10-31, option A)" (rewrite a surrounding sentence only when it would no longer read; no 'pending' wording may remain). `:672` has no pending wording (it names OP3 only) and is left untouched.

C.19 Evidence-folder text rules: no sha256 of records, roadmap, review, REPORT; host paths redacted (commands written with `<scratchpad>` and `<repo root>` placeholders). Enforced by G1: V1 and FINAL run `redact-local-paths.py ... check` on every committed-evidence text file, one file per invocation, and record each exit code.

C.20 `v5-alignment.md` exact replacement sentences (PB; the E2 and E5 bullets are uncommitted candidate lines, so the diff against HEAD stays insertion-only). Keep each bullet's opening, the line 110, 497 and 241 cites, `2026-10-28` and `operator answer A` unchanged. E2, replace the last sentence (from "The contract is" to the end of the bullet) with:
"The contract is [execution contract EX-4](execution-contract.md#ex-4-facts-derived-state-and-reconcile-rules) for activation and re-activation and [EX-5](execution-contract.md#ex-5-role-structure-lineage-and-work_exec_v1) for the generations and fencing that activation causes."
E5, replace the last sentence (from "The contract is" to the end of the bullet) with:
"The contract is [execution contract EX-5](execution-contract.md#ex-5-role-structure-lineage-and-work_exec_v1) for the fingerprint."
The word `EX-2` and the anchor `#ex-2-request-framing-and-replay` must not occur in either bullet afterwards.

## D. Hazards for the packages

1. `ex_checks.py` `auto_row` builds default rows. After D97 to D98 `role_state` rows with `state: 'running'` need `run_id: 1` (probes near `:2326-2337`, I48 and I47), and `transitions` rows with `after_state: 'running'` need no change (`after_drifted` defaults 0). `evidence` rows need NO override (D100 fallback: no CHECK).
2. Checks that pass today and must keep passing under the new text: I10, I18, I28, I31, I32 (amended), I48, I51 (header keeps "Status", "Authority", "v1 bod", "design", "source"), I52, I56, I57 (amended), I70.
3. Do not rename a heading in `execution-contract.md`: I1 pins 16 H2 headings and I42 pins 15 anchors.
4. Do not touch any diagnostic line (messages table, EX-7, EX-8 rows other than the R12 cell).
5. The old review's SQL rows (`review/sql_rows.txt` in the old evidence folder) give the rows the old SQL wrongly accepted; the new checks reuse them.
6. M8's cause cell reads "same as M7" and does not itself say "new generation": the repaired I60/I69 (C55/C56) must resolve a "same as Mx" cause one hop to row Mx before testing for "new generation". M10's cause says "new empty generation".
7. The record paragraph that quotes the dispatch relabel rule (it begins "Under the standing delegation of 2026-10-31 these are accepted at their recommended options" and contains the old phrase twice, at about :520-521) is quoted dispatch text: R1 must leave it byte-identical; CHK-C01-relabel-record excludes exactly that paragraph (first line to the next blank line) and nothing else. R1 must not write the old phrase anywhere else (describe it as "the earlier pending wording").
8. identity.py output with a pathspec lists a sha256 per modified or untracked file: never include `docs/vernacular/rounds/`, `roadmap.md` or evidence folders in a pathspec (Script rules item 4).

## E. Threat model and platform

- Threat model: none new. Ordering: unchanged. Platform: macOS and Linux unaffected (docs).

## F. HALT list

- HALT-A: any change to an existing diagnostic message byte, any added code or message.
- HALT-B: any change to an approved contract file other than `execution-contract.md` and `v5-alignment.md`.
- HALT-C: any new event type, transition reason, state, outcome value or precondition. C01 adds SQL CHECKs over existing states only.
- HALT-D: a packet finds a passage that contradicts a decision here: report it, do not choose.
- HALT-E: already applied for D100 (fallback). If the plan reviewer rejects D101, state `CHECK(ordinal>=0)` is NOT chosen; report instead.

## C (continued)

C.21 Exact correction notes R1 appends, as a trailing sentence at the end of the row (append-only, the row text before it is unchanged). Placement: append the note to the end of the row's second cell (the invariant or choice text), before that cell's closing ` |`, so the table stays valid. Old `INVARIANTS.md`: I33 `Corrected by C01: see C41.`; I54 `Corrected by C01: see C39.`; I57 `Corrected by C01: see C40.`; I60 `Corrected by C01: see C55.`; I67 `Corrected by C01: see C48.`; I69 `Corrected by C01: see C56 and C55.` Old `DECISIONS.md`: D43 `Corrected by C01: see amended D43 and D101 in execution-contract.md, and C40.`; D55 `Corrected by C01: see amended D55 in execution-contract.md (exit 4 reads I/O, corruption, busy and disk-full), and C17.`; D56 `Corrected by C01: see amended D56 in execution-contract.md (new-engineering, basis EX-8 and D78), and C16.` The same wording, with the record's own row ids, goes on the record's copied invariant-table rows (I32 gets `Corrected by C01: see C42.`).

C.22 D54, D67, D68 tag cells (execution-contract.md at :1045, :1051, :1052, rows re-located by text): the third cell of each row is replaced by exactly
`operator-decided (delegated 2026-10-31, option A)` (it was `operator-decided (delegated, standing delegation of 2026-10-31, option A)`).
The first, second and fourth cells are unchanged. OP1, OP2, OP3 use the same exact tag (C.15). C21 and C42 require the exact tag on all six rows;
C23 requires the word "delegated" on them and in the preamble, and refuses a live-answer phrase.

C.23 C16 predicate (exact). Let a Decisions-table row be "operator-decided" when its third cell starts with `operator-decided`. Then the row is
valid when (its fourth cell matches the regex `\bE\d{1,2}\b|\bQ(-[A-Za-z0-9]+|\d+)\b`) OR (its third cell contains the word `delegated`). Every
operator-decided row of the final contract must be valid, and D56 must be tagged `new-engineering` with a fourth cell naming `EX-8` and `D78`. Checked
against the final text: D7 (Q14), D10 (Q13; XD5), D11 (Q13), D49 (E5 A), D50 (E3 A), D78 (E15 A), D102 (`E11 A; R15-9; R15-7`, matches E11), D54, D67, D68
(delegated and E16), OP1 (delegated and E1 A), OP2 (delegated; `round record choice 2; E1 A; E11 A`), OP3 (delegated; basis `R17-6; A12; M41` has no E-id, valid by
the tag). On the pre-C01 candidate the only invalid row is D56 (tag `operator-decided`, basis `EX-8`, no match), which is why C16 is red. The
predicate is applied per row, not per basis component. The checker also enumerates the operator-decided rows (non-empty set, at least ten) so the
check cannot pass on an empty table.

C.24 Original-checker run (fix of gap 11): see script rule 5. Commands, written with placeholders in evidence: `cp <scratchpad>/ex_checks.py
<scratchpad>/ex_checks_orig.py`; `cp <scratchpad>/ex_checks.py <repo root>/tests/native/v5-02-r15-a1-c01-evidence/raw/ex_checks-r15a1-orig.py`;
`cmp <scratchpad>/ex_checks_orig.py <repo root>/tests/native/v5-02-r15-a1-c01-evidence/raw/ex_checks-r15a1-orig.py` (silent on identity);
`python3 -I <scratchpad>/ex_checks_orig.py --repo <repo root>` -> `raw/orig-checker-run.txt` (13 FAIL lines).

C.25 Wave table (revision 4).

| wave | packages | notes |
| --- | --- | --- |
| 1 | T1 | repaired checker; red runs on the pre-C01 candidate (C1-C4, C6-C23, C39-C43, C48, C57 FAIL) |
| 2 | PA, PB | contract and alignment text |
| 3 | R1 | record relabel and C01 section (with the D81 PLACEHOLDER), README, the two old plan files |
| 4 | V1 | green core run (`green/checks.txt` exit 0, zero FAIL), `sql-c01.txt`, link, redaction, identity, scope, frozen |
| 5 | PROBER | every mutant except the pending ones (C31 m1, m2; C61 m1, m2; C32 m3, m4; C61 m3); baseline audit FAIL set exactly C31 and C61 |
| 6 | R2 | D81 resolution, R6 cite of `probes/PROBES.md`, REPORT.md |
| 7 | PROBER2 | C31-m1, C31-m2, C61-m1, C61-m2 (baseline: audit all PASS) |
| 8 | FINAL | six `-final` files; the audit run executes before `checks-final.txt` exists (expected all PASS) |
| 9 | SYNC | record C01 "Final results" subsection and REPORT.md final results (C.26) |
| 10 | PROBER3 | C32-m3, C32-m4, C61-m3 against the post-SYNC tree (baseline audit all PASS) |
| 11 | CLOSE | six `-closing` files; last audit run on the final record, exit 0, all PASS |

C.26 Final results text (SYNC writes it in the record's C01 section as a subsection titled `Final results`, and a matching section in REPORT.md). Lines, no hash of any record, review, REPORT or roadmap file:
`- Closing probe <id>: <KILLED|SURVIVED>` for each of C31-m1, C31-m2, C61-m1, C61-m2, copied from the `closing probes` section of `probes/PROBES.md`;
`- Probes C32-m3, C32-m4 and C61-m3 run by PROBER3 after this edit; verdicts in probes/PROBES.md`;
`- Final audit run: tests/native/v5-02-r15-a1-c01-evidence/green/checks-final.txt, exit <n>, PASS <p>, FAIL <f>` with n, p and f copied from that file;
one line each for `green/identity-final.txt`, `green/redaction-final.txt`, `green/linkcheck-final.txt`, `green/scope-final.txt` and `green/frozen-final.txt` naming the path and the one-line outcome read from it;
`- Closing audit run after the last record edit: tests/native/v5-02-r15-a1-c01-evidence/green/checks-closing.txt (exit code and counts are in that file)`.
The record cannot contain the closing run's own result; the pointer is the report. Counts are those of the registry (105 registered, 6 audit, 99 core).

