# V5-02-R15-A1 independent review (execution contract, part 1)

Disposition: **request-changes, not records only**. Most of the contract is sound and close to ready. The format3 SQL
runs as written. Its format2 lines are byte-exact. The trigger set, the arrow-rule foreign keys, the two explicit
indexes, the codec CHECK and the check-request exception agree with the cross-row invariants. The reconcile precedence
matches v5 lines 374 to 385 row for row. Takeover fences only and carries the lineage. The nine E16 A codes are exact,
and every reused existing message is byte-identical to its owner. The part 2 placeholders hold no normative text. The
pointer amendments are insertions only, `CLAUDE.md` gains exactly one entry, `AGENTS.md` is still a symlink, and no
`src` or `tests` byte changed.

Six findings block approval. Four are contract-text defects that R15-B or R15-C would otherwise have to resolve by
making a design choice (R1 to R4). One is a false statement about the design in normative text (R5). One is the
verification the round added for itself and never ran (R6). R1 to R5 need edits to `execution-contract.md`, and R6 needs
the evidence checker repaired or the prober addition withdrawn. That is why this is not a records-only request. The
minor findings R7 to R12 should go into the same corrective.

The scope was the uncommitted diff on HEAD `d536922b` (six tracked files, 31 insertions and 7 deletions) plus the
untracked `docs/vernacular/execution-contract.md` (1078 lines) and the round record. I read them against
execution-design.md EX-1 to EX-8 (lines 62 to 336), the R15 section (lines 549 to 720), R17-1 to R17-7 (lines 836 to
910) and the decisions table (lines 1366 to 1393), ledger-contract.md (the format1 SQL at lines 188 to 247, the event
vocabulary at lines 312 to 325, the command surface at lines 441 to 450, the error paragraph at lines 474 to 487 and the
format evolution section from line 489), external-profile-contract.md (lines 256 to 268, 506 to 541 and 543 to 573, and
the format table near line 652), load-command-contract.md (lines 101 to 107, 219 to 228 and 258 to 266),
authoring-contract.md (lines 500 to 506), declarations.md (lines 31 to 34, 217 to 221, 274 to 277 and 349 to 352),
render-contract.md (lines 214 to 221, 269 to 272 and 403 to 407), v5-alignment.md (lines 128 to 146),
x07-addendum.md (lines 58 to 67), remaining-steps-design.md (lines 66 to 68) and v5 (lines 59, 61, 110, 241, 276,
362 to 388, 497, 507 and 559 to 566). I also read the cited source lines `src/cmd/planar/response.cpp:216-222`,
`src/stdlib/planar/selection.lua:93-98` and `src/stdlib/planar/observations.lua:50-59`. I ran no script under
`tests/native/`.

## Exact candidate

identity.py (the workflow script) over the candidate documents, run from the repository root and pasted verbatim:

| identity | value |
|---|---|
| HEAD | `d536922b59bf9118b91e4188a26d770f9745d600` |
| sha256(git diff --binary HEAD -- docs/vernacular/execution-contract.md CLAUDE.md docs/vernacular/ledger-contract.md docs/vernacular/external-profile-contract.md docs/vernacular/acceptance-matrix.md docs/vernacular/v5-alignment.md docs/vernacular/implementation.md), tracked only | `eb2deed97a9fa03401a5625c75d0ccbd4d869c314457b5b634fa474253f27302` |
| combined sha256 (diff bytes, then each untracked path NUL bytes, sorted), pathspec docs/vernacular/execution-contract.md CLAUDE.md docs/vernacular/ledger-contract.md docs/vernacular/external-profile-contract.md docs/vernacular/acceptance-matrix.md docs/vernacular/v5-alignment.md docs/vernacular/implementation.md, evidence excluded | `b7e34425d33cc06c10492bf3c5a6162e2d8681477af64fb0510d3cbb0b591bb7` |

| file | state | sha256 |
|---|---|---|
| `CLAUDE.md` | modified | `7cdf4eadca490e6d3f6c6c697cd373c2c8a683f41612de02f98dce8b69b72163` |
| `docs/vernacular/acceptance-matrix.md` | modified | `6370cb09d99eaa33bf068f787e6ed040c3a81d160d6d2cbba47b3944e7178be5` |
| `docs/vernacular/external-profile-contract.md` | modified | `e190e2d1d88ad0a40cab3bd4bb31bbe826774a58f729224d083abcac877577da` |
| `docs/vernacular/implementation.md` | modified | `486a80a78d247e45badd73b2b96184bb952317d1229dbdba94d93cdd734e35fb` |
| `docs/vernacular/ledger-contract.md` | modified | `3e595653feeaa63f824f1f0bbf929a4923f10854dbcdd3e35725b8ff6223ceaf` |
| `docs/vernacular/v5-alignment.md` | modified | `23c6b7f86b0cbad42c26d455e810c7e7788e3c096a752c7f31a484f8041166fc` |
| `docs/vernacular/execution-contract.md` | untracked | `fd4c9134001eb960b9ca4f4357a29d3ff3d29589608d42d87848c6db77145ad1` |

This matches section (g) of the round record. identity.py over `src tests` is the sha256 of empty input for both the
tracked and the combined value, with no file rows (`review/identity-src-tests.txt`), so acceptance criterion 9 holds.

## Blocking findings

### V5-02-R15-A1-R1: the run token exposure list leaves out `status --request-id`

Contract lines 534 to 536 (Run token subsection) say the token "is shown only in the claim and takeover results, and in
`history --request-id` of those requests on format3", and that it is "never shown in `status`". Design R15-9
(execution-design.md:642-651) says that `status` *other than request lookup* never shows the token, and that request
lookup of a claim or takeover request does. R15-7 (lines 623 to 630) says `status --request-id` returns EX-2's saved result,
and that saved result carries the token. The design's A09 test (lines 670 to 673) recovers the lost token through
`status --request-id`. OP2's own rationale is that `history --request-id` returns the token "exactly as
`status --request-id` does".

As written, the contract forbids the A09 recovery path, narrows R15-9, and contradicts OP2. Invariant I54 in
`plan/INVARIANTS.md` encodes the same omission, so CHK-I54 passes on the defect. Fix: add `status --request-id` of a
claim or takeover request to the shown list, and change "never shown in `status`" to "never shown in `status` other
than its request lookup".

### V5-02-R15-A1-R2: the event-level reason names a transitions row that the SQL forbids

EX-4 (line 675) and D43 define the event-level `reason` as "the `reason` of the row with `ordinal` 0 in `transitions`".
The SQL has `transitions.ordinal INTEGER NOT NULL CHECK(ordinal>0)` (line 298), and convention C5 says `ordinal` is
above 0. Executing the block shows that an `ordinal` 0 insert fails (`review/sql_rows.txt`, "ordinal 0"). Under the
text as written, every event-level reason would be the empty string. R15-B and R15-C would have to choose between
zero-based and one-based ordinals, or reread the rule as "the first row". The invariant table contains the same
contradiction: I57 pins "the ordinal-0 transition reason" while I67's mutant list includes "drop `ordinal>0`".
Fix: pick one. Either state "the row with the lowest `ordinal` (ordinal 1)", or make the CHECK `ordinal>=0`, and
align C5, D43, I57 and I67.

### V5-02-R15-A1-R3: OP1 claims to change the approved "Flags cannot be required" rule

The OP1 row of the decisions table (line 1076) ends: "Flags cannot be required (`external-profile-contract.md:266-267`);
this changes that rule for this one option." Option A makes `--confirm-stopped` a required **scalar** option whose only
accepted value is `true`. It is no longer a flag, so the flag rule needs no change. The round record frames option A
this way at lines 81 to 87, and so does its own section (c). The sentence as written records an amendment to an approved
clause of the external-profile contract. That amendment has no pointer amendment, and it contradicts the header's
"narrows no clause" and "amends none ... except through the pointer amendments". Fix: say instead that the option is a
required scalar, so the external-profile flag rule is unchanged.

### V5-02-R15-A1-R4: `role_state.run_id` and `drifted` are unconstrained for roles that are not running

The design gives `role_state` a nullable `run_id` and treats drift as "a flag on a running role" (EX-1 table; EX-4).
The contract states the coupling in one direction only. Invariant 12 says a role is `running` if and only if one
active run exists and `run_id` equals it. Nothing says what `run_id` and `drifted` are for `blocked`, `ready`, `done`,
`stale` or `retired`. The same gap applies to `transitions.after_drifted` when `after_state` is not `running`. The SQL
accepts `('j','r',1,'ready',1,5,1,1)`, a ready role with drift set and run 5 (`review/sql_rows.txt`). `transitions`
carries no run column, so invariant 6 ("`role_state` equals the replay of all `transitions`") cannot reconstruct
`run_id` either.

The precondition `role_state(target) == (state, drifted, run)` compares these fields, so R15-B must decide whether a
done role keeps its last run id. Fix: state that for every state other than `running`, `run_id` is NULL and `drifted`
is 0, and that `after_drifted` is 0 unless `after_state` is `running`. Preferably add the matching table CHECKs. Also
say how replay derives `run_id`, from `run_state` and `events.run_id`.

### V5-02-R15-A1-R5: convention C6 misstates the design for `evidence.reason`

C6 (lines 68 and 69) says "`evidence.reason` has no value list in the design, so it is `TEXT NOT NULL` with no CHECK".
The design EX-1 table gives `reason` as "(closed enum, R17)". R17-3 (execution-design.md:879-883) lists that enum:
`exit_status`, `zero_selected`, `missing_executable`, `timeout`, `output_limit`, `malformed_result`, `boundary` and
`input_changed`. A `pass` row has no listed reason. The SQL accepts any text (`review/sql_rows.txt`, "evidence reason
arbitrary").

Format3 is meant to be one complete schema that R15-B writes once, and the opener verifies the exact stored SQL. A later
R17 CHECK would therefore be a schema change to format3. Fix: correct the sentence. Then either encode the R17-3 set,
with the `pass` value stated explicitly (for example the empty string), or state that the enum is native-writer and
reader-verified only, so that R17 changes no stored SQL. Whichever is chosen, record it in the decisions table.

### V5-02-R15-A1-R6: the verification the round added for itself did not run, and the invariant table is partly hollow

Section (a) item 4 of the round record (D81) replaced "no mutation prober" with a prober run. It never ran:
`probes/PROBES.md` does not exist and no prober output exists. On the final candidate, 13 of the 70 scratch checks fail
(`raw/ex_checks-final.txt`). By my reading of the bytes, every one of the 13 is a checker or invariant defect, not a
contract defect:

- I2, I5, I47, I53 and I67 stop extracting the SQL at the first `-- Reserved for R16` comment. The complete block runs
  in Python `sqlite3` 3.54.1 with foreign keys on. It yields the 24 STRICT tables in the stated order, 43 triggers equal
  to the expected `_no_update`/`_no_delete` set, and exactly the two explicit indexes (`review/sql_shape.txt`).
- I8 (line 475), I39 (v5-alignment.md, the E2 and E5 entries), I64 (line 796) and I66 (line 951, which mirrors
  authoring-contract.md:505) fail only on the checker's wording or parsing.
- I42 does not decode `%20`. My own link check resolves every link and anchor in the eight changed documents
  (`review/linkcheck_review.txt`). Its two reports are the `CLAUDE.md` entry quoted inside inline code, not links.
- I46 ran before the README existed. The README holds no hash.
- I60 and I69 demand that no `stale` to `ready` row exists. M8 and M10 are correct as written, because each opens a new
  empty generation. The contract states this at line 806, and it agrees with v5 :383.

The result is that five checks could never pass, and two invariant rows (I54 and I57) encode defects R1 and R2. The
invariant table is therefore not a reliable gate for this candidate, and no row has a kill result.

Fix, in the corrective: repair the extractor, and correct I54, I57, I60 and I69. Then either run the prober in a scratch
copy and record kills and survivors, or withdraw the D81 prober addition in the record with its reason. The original
dispatch planned a reviewer read only, and withdrawal must be stated, not implied.

## Minor findings (fix in the same corrective)

- **R7.** Header line 6 says "Part 1 holds the header, the common rules and clauses EX-1 to EX-3", and that the
  reconcile rules and the rest "follow". A reader can take EX-4 onward as part 2. Part 1 is EX-1 to EX-8 plus the state
  machines.
- **R8.** The role-table note at line 860 says the before cells of M5, M7 and M8 are "every non-running, non-retired
  state". M7 and M8 include `running` and `running+drift`.
- **R9.** D56 is tagged `operator-decided` with basis "EX-8", which is not an E or Q id. The design calls the EX-8
  numbers NEW engineering, apart from the E15 row (D78). D55 lists "internal" in the exit-4 class, but the ledger
  contract it cites (ledger-contract.md:480-481) says "I/O, corruption, disk-full and busy".
- **R10.** The v5-alignment.md E2 and E5 entries point to "EX-2 for the request". Activation is specified in EX-4
  (Activation and Re-activation), and E5 has nothing to do with request framing. The contract's own traceability maps E2
  to EX-4. Point E2 at EX-4 (and EX-5), and E5 at EX-5 only.
- **R11.** The cutover bullet (line 467) says "at the same explicit target as format2". The design (line 80) says "at a
  new explicit target". Use the design wording.
- **R12.** The EX-8 check-output row puts an R17 outcome (`fail` with `output_limit`) in the column headed "Existing
  site". No such site exists yet. Label it as an R17 fact.

## Checks that passed on the bytes

- **SQL.** The diff against the ledger-contract format1 block is exactly the format value, the codec CHECK, the
  `events` type and version CHECKs plus four typed columns and the role/job CHECK, the two `current_state` columns, and
  the `requests` `operation`, outcome and `attachment_id` lines. With the external-profile format2 delta applied, every
  unmodified format2 line is byte-exact.
- **Foreign keys.** They follow the design's arrows exactly. `reservations.request_id` has no foreign key, and a
  reservation without a request row inserts (R17-1 step 1).
- **Behaviour of the stored SQL.** An update of an immutable table and a delete of a mutable one raise
  `immutable ledger row`. A second `active` run for one (job, role) fails, and a `fenced` one succeeds. The
  `blocker_state` coupling CHECK holds in both directions. The outcome `reserved` is refused, and a request_v1 codec on
  kind 4 is refused.
- **EX-3.** The seven steps, the eight precondition members in the design spelling, "No other precondition exists", the
  bump set, `claim_conflict` against `state_conflict`, the delta write and the lost-update guard all match
  execution-design.md:150-202. The load and activate sets add `role_state` per role so that a claim is visible. This is
  consistent with A26 and R15-3.
- **EX-4 and the state machines.** The precedence order and the line cites match v5 :374, :375, :376, :378, :380,
  :381, :382, :383, :385 and :387. Each role and run row has a before state, a cause, an after state and one of the 16
  reasons. The run table has five states, `interrupted` comes only from release, and terminal states have no outgoing
  rows. R16 to R18 rows are labelled facts only.
- **EX-5 and EX-6.** Takeover fences only and carries completions, evidence, publications, repair revision and
  approvals forward (EX-5 Lineage). The interpretation binding runs after early replay, and the exempt-read class is
  present.
- **EX-7 and Messages.** There are nine codes, each exit 3 and each tagged as an E16 A amendment, with the introducing
  rounds as the design gives them. The text says "No other new code exists", and XD4 is amended for execution only. The
  reused messages `Ledger version or state is unsupported`, `Ledger request conflicts with its saved intent`,
  `Ledger revision does not match the expected revision`, `Unknown selected job`, `Unknown selected role`,
  `Wrong output vector`, `Capture limit exceeded`, `Captured declaration inputs changed`, `Ledger is busy`,
  `Ledger storage is full` and the load fallback are byte-equal to their owners. None of the 19 new strings occurs
  anywhere else under `src` or `docs`. No existing diagnostic line is removed or altered in any changed document.
- **Part 2 placeholders.** All seven say "Not yet written", list their R15-A2 clauses, and contain no `must`, `shall`
  or `refuses`.
- **Pointer amendments and records.** The pointers are insertions at the stated places. ledger-contract.md has three,
  with zero removed lines. external-profile-contract.md has one paragraph before "New init creates format2 only", with
  the message tables untouched. The seven acceptance-matrix rows each have the old text plus one appended sentence, with
  the status words unchanged. `git diff --numstat HEAD -- CLAUDE.md` is `1 0`, and `readlink AGENTS.md` prints
  `CLAUDE.md`.
- **Citations.** Every `file:line` citation in the contract was read at the cited line and supports its claim.
- **Redaction.** `check` exits 0 on each of the eight changed files, one argument per call, and the scratch positive
  control exits 1 (`review/redaction-check.txt`).

## Operator confirmations needed at commit

None of the following is a live operator answer. Each was applied as the recommended option under the standing rule of
2026-10-31, so each needs the operator's confirmation:

1. **OP1 (E1 A sub-choice 1).** `--confirm-stopped` becomes a required scalar whose only accepted value is `true`. The
   alternatives were B, `--stopped-run <run id>`, and C, keep the flag and refuse its absence as usage. Its wording must
   first be corrected under R3.
2. **OP2 (E1 A sub-choice 2).** On format3, `history --request-id` also returns execution results, including tokens
   for claim and takeover requests. Format1 and format2 are unchanged. The alternative was B, lookup through
   `status --request-id` only.
3. **OP3 (an extra halt raised by the author).** M41 reuses `evidence_inapplicable` for stale to done. The
   alternatives were B, add a 17th reason, and C, declare M41 unreachable (rejected).
4. **HALT-1 (D54, D67 and D68).** There are 19 new messages: nine code messages, `Execution resource limit exceeded`
   and nine post-commit fallbacks. Acceptance criterion 3 as dispatched said "the single new message". The design
   itself requires a message per new code and operation-specific fallbacks (EX-7, line 312; EX-3 step 7), so I accept
   the enumerated set. It is still a change to the round's acceptance text.

## Record rule

The record, the evidence README and this review pin no sha256 of another round record, a review file, a REPORT or the
roadmap. One point needs the coordinator's attention. Section (g) of the record, and this review, pin the per-file
hashes of the candidate contract documents, because the workflow's candidate-identity output requires them. Acceptance
criterion 10's second sentence says "Hashes may pin only `src` and committed evidence data files". Its explicit
prohibition list is not breached. The pre-fix snapshot hash in section (a) covers a clone that included this record's
own early copy, which standing lesson 5 requires. I treat both as compliant, with this tension noted.

## Evidence

New files under `tests/native/v5-02-r15-a1-evidence/review/`:

- `sql_shape.py` and `sql_shape.txt`: the table set, STRICT, triggers, indexes and foreign keys of the executed block.
- `sql_rows.py` and `sql_rows.txt`: the row-level CHECK and trigger probes. In this file `BAD` marks a row that the SQL
  accepted against the contract's intent (R4, R5).
- `linkcheck_review.py` and `linkcheck_review.txt`.
- `redaction-check.txt`.
- `identity-src-tests.txt`.

The executed SQL is lines 82 to 352 of the candidate, copied into the session scratchpad only.

## Limits

- I ran `check` only, not `redact`, on the changed files. I do not own those files, and `redact` may rewrite them.
  The gate's `raw/redact-*.txt` record redact as "0 files changed".
- No product build or test ran, because this is a documentation round. I relied on the gate's read-only
  `ctest -N` inventory (1439 names, unchanged) and did not repeat it.
- I did not run the scratch `ex_checks.py` or any prober (see R6). My judgement of the 13 failing checks comes from
  reading the contract bytes and from my own SQL execution.
- I did not re-derive the whole of `plan/DECISIONS.md`. I checked only the decisions the contract states.
