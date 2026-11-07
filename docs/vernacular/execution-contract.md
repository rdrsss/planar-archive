# Execution contract v1 (parts 1 and 2)

Status: exact contract text for round [V5-02-R15-A1](rounds/V5-02-R15-A1.md), documentation only. It specifies
behaviour; it ships nothing. Until the R15-B and R15-C rounds implement it, the current commands remain `render`,
`init`, `history`, `export` and `load`, and the approved contracts govern them unchanged. Part 1 holds the header,
the common rules, clauses EX-1 to EX-8, the state machines, the Part 2 section for R15-A2 (below), the messages, the
pointer amendments, the traceability table, the acceptance checklist and the decisions table. The Part 2 section
is filled by R15-A2 and is normative; Part 1 amendments A-1 to A-13 are listed in the section "Part 1 amendments (R15-A2)".

Authority: the accepted decisions E1 to E17 of the [execution design](execution-design.md#decisions-table) (operator
answers of 2026-10-28), the operator decisions Q1 to Q18, Q-INIT A and Q-G35 A of the
[remaining-steps design](remaining-steps-design.md#operator-decisions-2026-10-27), and the approved contracts
[declarations](declarations.md), [render](render-contract.md), [ledger](ledger-contract.md),
[command and workspace](command-contract.md), [authoring](authoring-contract.md),
[load](load-command-contract.md) and [external profile](external-profile-contract.md). This contract amends none of
their v1 bodies except through the pointer amendments listed in [Pointer amendments](#pointer-amendments). It
narrows no clause of any approved contract. It specifies no check, no verdict and no R16 to R19 behaviour. The
execution design is the source of this text, not its authority: where this contract and an approved contract
differ, the approved contract governs until a reviewed amendment says otherwise.

## Common rules

1. **Formats.** Format3 is the schema of new execution ledgers. Public `init` creates it only after the R15-B and
   R15-C cutover ([EX-1](#ex-1-format3)). Format1 and format2 ledgers stay readable under their existing contracts.
   Each execution operation (`activate`, `claim`, `takeover`, `refresh`, `checkpoint`, `release`, `check`,
   `check-recover` and `verdict`) refuses a format1 or format2 ledger with `unsupported_ledger_version` before any capture or Lua runs.
   Load keeps its existing format1 and format2 behaviour.
2. **Tables.** Every table is `STRICT`. Immutable tables carry the update and delete triggers of EX-1. The mutable
   projections `current_state`, `role_state`, `run_state`, `reservation_state` and `blocker_state` are written only
   by the commit protocol of [EX-3](#ex-3-execution-commit-protocol) and are never deleted.
3. **Identifiers.** Identifiers and event sequences allocate `MAX+1` under the write transaction, beginning at 1, as
   format2 does ([ledger-contract](ledger-contract.md), the native writer invariant list). Request IDs are text, as
   in format2, and are chosen by the driver before dispatch.
4. **Reserved tables.** Format3 is one schema, so it includes the tables that rounds R16 to R18 use. Each such table
   is labelled in the SQL with the round that defines its behaviour. This contract defines no behaviour for a
   reserved table.
5. **Messages.** Existing codes and messages keep their existing bytes. An execution operation reuses an existing
   code only where its meaning fits, and names the existing message ([Messages](#messages)). Every message that
   execution introduces is listed in [Messages](#messages) as an amendment to XD4 for execution only.
6. **Citations.** A design citation is a file and line located by its text when this contract was written. The
   execution design and the round record are sources for this text. Neither is an approval.

## EX-1 Format3

Format3 is the format2 schema with the `events`, `requests` and `current_state` tables modified, the execution
tables added, `ledger_meta.format` set to 3 and `PRAGMA user_version=3`. The application ID stays
`application_id = 1347178034`, and foreign keys stay ON ([ledger-contract](ledger-contract.md), the format paragraph).
The format2 SQL is the schema block of the ledger contract with the format2 changes of the
[external profile](external-profile-contract.md#x-store--byte-codecs-exact-provenance-and-supported-formats).
Format3 keeps every unmodified line of that SQL byte-exact. In `ledger_meta`, `records`, `tree_edges` and
`revisions` the only changes are the format value and the records codec CHECK. In `events`, `current_state` and
`requests` every unmodified column line is byte-exact and the changed lines (CHECK lists and added columns) are the
only differences.

Column conventions. These conventions apply to every column of the SQL below, and the SQL follows them.

- C1. A primary key id is `INTEGER PRIMARY KEY CHECK(id>0)`.
- C2. A sequence or other non-reference integer id (`seq`, `opened_seq`, `claimed_seq`, `last_seq`, `event_seq`,
  `ordinal`, `revision_id`, `generation_id`, `run_id`, `dispatch_id` and the other ids that carry no REFERENCES) is
  `INTEGER NOT NULL CHECK(c>0)`. Its nullable form is `INTEGER CHECK(c IS NULL OR c>0)`. A column that carries
  REFERENCES has no extra range CHECK, as in format2.
- C3. Booleans (`drifted`, `before_drifted`, `after_drifted`, `output_truncated`) are `INTEGER NOT NULL CHECK(c IN (0,1))`.
  `before_drifted` is NULL exactly when `before_state` is NULL.
- C4. Identifier text (`job`, `role`, `work`, `check`) is `TEXT NOT NULL CHECK(length(c) BETWEEN 1 AND 64)`, the
  length bound of the declaration ID grammar. The nullable form is used for `events.job`, `events.role` and
  `current_state.active_job`, and `events` carries `CHECK(role IS NULL OR job IS NOT NULL)`.
- C5. Counts are `INTEGER NOT NULL CHECK(c>=0)` (`duration_ms`, `output_bytes`). `exit_status` and `matched` are
  nullable `INTEGER` with no CHECK. `fact_version` is at least 1, and `ordinal` and `revision` are above 0. The `transitions` rows of one event have `ordinal` values contiguous from 1, which the native writer enforces and the reader verifies, so the event-level reason of an event is the `reason` of its row with ordinal 1 ([EX-4](#ex-4-facts-derived-state-and-reconcile-rules)).
- C6. Closed enums are `TEXT NOT NULL CHECK(c IN (...))` with the listed values. `evidence.reason` is native-writer enforced and reader-verified only, with no SQL CHECK. The set is closed by R17 (execution design EX-1 table: 'closed enum, R17'), but R17-3 lists no `reason` value for a `pass` outcome, so R17 states that value; no stored format3 SQL changes.
- C7. Fingerprints and secrets (`work_fp`, `render_fp`, `semantic`, `secret`) are `BLOB NOT NULL CHECK(length(c)=16)`.
  `payload` and `result` are `BLOB NOT NULL CHECK(length(c)<=1048576)`.

`check` is an SQL keyword. In the SQL, the column named `check` is written as the quoted identifier `"check"`, and
every CHECK expression that names it quotes it the same way. In prose it is named `check`.

The complete schema is the following block. Tables appear in creation order: the format2 order first (`ledger_meta`,
`records`, `tree_edges`, `revisions`, `events`, `current_state`, `requests`), then the execution tables. It includes
the two explicit indexes of the schema, and the explicit triggers, which are the stored SQL the schema verifier
compares. A `Reserved for` line directly above a table is that table's label.

```sql
CREATE TABLE ledger_meta (
  singleton INTEGER PRIMARY KEY CHECK(singleton=1),
  format INTEGER NOT NULL CHECK(format=3),
  identity TEXT NOT NULL CHECK(length(identity)=32),
  workspace_id TEXT NOT NULL,
  created_at TEXT NOT NULL
) STRICT;
CREATE TABLE records (
  id INTEGER PRIMARY KEY CHECK(id>0),
  kind INTEGER NOT NULL CHECK(kind BETWEEN 1 AND 4),
  codec INTEGER NOT NULL CHECK((kind=4 AND codec IN (2,3)) OR (kind BETWEEN 1 AND 3 AND codec=1)),
  digest BLOB NOT NULL CHECK(length(digest)=16),
  encoded BLOB NOT NULL CHECK(length(encoded)<=67108864)
) STRICT;
CREATE INDEX records_digest ON records(codec,kind,digest);
CREATE TABLE tree_edges (
  parent_id INTEGER NOT NULL REFERENCES records(id),
  ordinal INTEGER NOT NULL CHECK(ordinal>=0),
  name TEXT NOT NULL,
  child_id INTEGER NOT NULL REFERENCES records(id),
  PRIMARY KEY(parent_id,ordinal), UNIQUE(parent_id,name),
  CHECK(parent_id>child_id)
) STRICT;
CREATE TABLE revisions (
  id INTEGER PRIMARY KEY CHECK(id>0),
  parent_id INTEGER REFERENCES revisions(id),
  source_id INTEGER NOT NULL REFERENCES records(id),
  registry_id INTEGER NOT NULL REFERENCES records(id),
  stdlib_id INTEGER NOT NULL REFERENCES records(id),
  interpretation_id INTEGER NOT NULL REFERENCES records(id),
  versions_id INTEGER NOT NULL REFERENCES records(id),
  actor TEXT NOT NULL,
  accepted_at TEXT NOT NULL,
  CHECK(parent_id IS NULL OR parent_id<id)
) STRICT;
CREATE TABLE events (
  seq INTEGER PRIMARY KEY CHECK(seq>0),
  type TEXT NOT NULL CHECK(type IN ('ledger_initialized','declaration_accepted','load_noop','job_activated','run_claimed','run_taken_over','run_refreshed','checkpoint_recorded','run_released','run_interrupted','check_reserved','check_recorded','check_interrupted','verdict_recorded')),
  version INTEGER NOT NULL CHECK(version IN (1,2)),
  revision_id INTEGER REFERENCES revisions(id),
  request_id TEXT,
  actor TEXT NOT NULL,
  occurred_at TEXT NOT NULL,
  payload BLOB NOT NULL CHECK(length(payload)<=1048576),
  job TEXT CHECK(job IS NULL OR length(job) BETWEEN 1 AND 64),
  role TEXT CHECK(role IS NULL OR length(role) BETWEEN 1 AND 64),
  generation_id INTEGER CHECK(generation_id IS NULL OR generation_id>0),
  run_id INTEGER CHECK(run_id IS NULL OR run_id>0),
  CHECK(role IS NULL OR job IS NOT NULL)
) STRICT;
CREATE TABLE current_state (
  singleton INTEGER PRIMARY KEY CHECK(singleton=1),
  active_revision_id INTEGER REFERENCES revisions(id),
  last_seq INTEGER NOT NULL REFERENCES events(seq),
  active_job TEXT CHECK(active_job IS NULL OR length(active_job) BETWEEN 1 AND 64),
  active_activation_id INTEGER CHECK(active_activation_id IS NULL OR active_activation_id>0)
) STRICT;
CREATE TABLE requests (
  request_id TEXT PRIMARY KEY,
  operation TEXT NOT NULL CHECK(operation IN ('load','activate','claim','takeover','refresh','checkpoint','release','check','check-recover','verdict')),
  body_id INTEGER NOT NULL REFERENCES records(id),
  event_seq INTEGER NOT NULL UNIQUE REFERENCES events(seq),
  revision_id INTEGER NOT NULL REFERENCES revisions(id),
  outcome TEXT NOT NULL CHECK(outcome IN ('accepted','noop','activated','claimed','taken_over','refreshed','recorded','released','interrupted')),
  result BLOB NOT NULL CHECK(length(result)<=1048576),
  attachment_id INTEGER REFERENCES records(id)
) STRICT;
CREATE TABLE job_activations (
  id INTEGER PRIMARY KEY CHECK(id>0),
  job TEXT NOT NULL CHECK(length(job) BETWEEN 1 AND 64),
  revision_id INTEGER NOT NULL CHECK(revision_id>0),
  event_seq INTEGER NOT NULL CHECK(event_seq>0)
) STRICT;
CREATE TABLE generations (
  id INTEGER PRIMARY KEY CHECK(id>0),
  job TEXT NOT NULL CHECK(length(job) BETWEEN 1 AND 64),
  role TEXT NOT NULL CHECK(length(role) BETWEEN 1 AND 64),
  ordinal INTEGER NOT NULL CHECK(ordinal>0),
  structure_id INTEGER NOT NULL REFERENCES records(id),
  opened_seq INTEGER NOT NULL CHECK(opened_seq>0),
  reason TEXT NOT NULL CHECK(reason IN ('activated','structural_change','takeover','recreated'))
) STRICT;
CREATE TABLE runs (
  id INTEGER PRIMARY KEY CHECK(id>0),
  generation_id INTEGER NOT NULL CHECK(generation_id>0),
  request_id TEXT NOT NULL,
  claimed_seq INTEGER NOT NULL CHECK(claimed_seq>0),
  secret BLOB NOT NULL CHECK(length(secret)=16),
  provenance_id INTEGER REFERENCES records(id)
) STRICT;
CREATE TABLE dispatches (
  id INTEGER PRIMARY KEY CHECK(id>0),
  run_id INTEGER NOT NULL CHECK(run_id>0),
  seq INTEGER NOT NULL CHECK(seq>0),
  revision_id INTEGER NOT NULL CHECK(revision_id>0),
  kind TEXT NOT NULL CHECK(kind IN ('claim','takeover','refresh')),
  packet_id INTEGER NOT NULL REFERENCES records(id),
  manifest_id INTEGER NOT NULL REFERENCES records(id),
  work_id INTEGER NOT NULL REFERENCES records(id),
  work_fp BLOB NOT NULL CHECK(length(work_fp)=16),
  render_fp BLOB NOT NULL CHECK(length(render_fp)=16),
  checkpoint_id INTEGER REFERENCES records(id)
) STRICT;
-- Reserved for R16
CREATE TABLE checkpoints (
  id INTEGER PRIMARY KEY CHECK(id>0),
  run_id INTEGER NOT NULL CHECK(run_id>0),
  seq INTEGER NOT NULL CHECK(seq>0),
  body_id INTEGER NOT NULL REFERENCES records(id)
) STRICT;
-- Reserved for R17
CREATE TABLE reservations (
  id INTEGER PRIMARY KEY CHECK(id>0),
  request_id TEXT NOT NULL UNIQUE,
  run_id INTEGER NOT NULL CHECK(run_id>0),
  opened_seq INTEGER NOT NULL CHECK(opened_seq>0),
  body_id INTEGER NOT NULL CHECK(body_id>0)
) STRICT;
-- Reserved for R17
CREATE TABLE evidence (
  id INTEGER PRIMARY KEY CHECK(id>0),
  reservation_id INTEGER NOT NULL CHECK(reservation_id>0),
  seq INTEGER NOT NULL CHECK(seq>0),
  work TEXT NOT NULL CHECK(length(work) BETWEEN 1 AND 64),
  "check" TEXT NOT NULL CHECK(length("check") BETWEEN 1 AND 64),
  definition_id INTEGER NOT NULL REFERENCES records(id),
  vector_id INTEGER NOT NULL REFERENCES records(id),
  outcome TEXT NOT NULL CHECK(outcome IN ('pass','fail','inconclusive')),
  reason TEXT NOT NULL,
  matched INTEGER,
  exit_status INTEGER,
  duration_ms INTEGER NOT NULL CHECK(duration_ms>=0),
  output_id INTEGER REFERENCES records(id),
  output_bytes INTEGER NOT NULL CHECK(output_bytes>=0),
  output_truncated INTEGER NOT NULL CHECK(output_truncated IN (0,1))
) STRICT;
-- Reserved for R17
CREATE TABLE completions (
  id INTEGER PRIMARY KEY CHECK(id>0),
  run_id INTEGER NOT NULL CHECK(run_id>0),
  seq INTEGER NOT NULL CHECK(seq>0),
  dispatch_id INTEGER NOT NULL CHECK(dispatch_id>0),
  work_fp BLOB NOT NULL CHECK(length(work_fp)=16),
  observation_id INTEGER NOT NULL REFERENCES records(id)
) STRICT;
-- Reserved for R17
CREATE TABLE publications (
  id INTEGER PRIMARY KEY CHECK(id>0),
  completion_id INTEGER NOT NULL CHECK(completion_id>0),
  work TEXT NOT NULL CHECK(length(work) BETWEEN 1 AND 64),
  facet TEXT NOT NULL,
  semantic BLOB NOT NULL CHECK(length(semantic)=16),
  payload_id INTEGER NOT NULL REFERENCES records(id)
) STRICT;
-- Reserved for R18
CREATE TABLE verdicts (
  id INTEGER PRIMARY KEY CHECK(id>0),
  request_id TEXT NOT NULL UNIQUE,
  run_id INTEGER NOT NULL CHECK(run_id>0),
  seq INTEGER NOT NULL CHECK(seq>0),
  body_id INTEGER NOT NULL REFERENCES records(id),
  applicability TEXT NOT NULL CHECK(applicability IN ('current','historical')),
  decision TEXT NOT NULL CHECK(decision IN ('approve','reject'))
) STRICT;
-- Reserved for R18
CREATE TABLE repairs (
  verdict_id INTEGER NOT NULL CHECK(verdict_id>0),
  job TEXT NOT NULL CHECK(length(job) BETWEEN 1 AND 64),
  role TEXT NOT NULL CHECK(length(role) BETWEEN 1 AND 64),
  generation_id INTEGER NOT NULL CHECK(generation_id>0),
  revision INTEGER NOT NULL CHECK(revision>0)
) STRICT;
-- Reserved for R18
CREATE TABLE blockers (
  id INTEGER PRIMARY KEY CHECK(id>0),
  seq INTEGER NOT NULL CHECK(seq>0),
  job TEXT NOT NULL CHECK(length(job) BETWEEN 1 AND 64),
  kind TEXT NOT NULL CHECK(kind IN ('unowned_rejection','max_rounds')),
  verdict_id INTEGER NOT NULL CHECK(verdict_id>0)
) STRICT;
CREATE TABLE role_state (
  job TEXT NOT NULL CHECK(length(job) BETWEEN 1 AND 64),
  role TEXT NOT NULL CHECK(length(role) BETWEEN 1 AND 64),
  generation_id INTEGER NOT NULL CHECK(generation_id>0),
  state TEXT NOT NULL CHECK(state IN ('blocked','ready','running','done','stale','retired')),
  drifted INTEGER NOT NULL CHECK(drifted IN (0,1)),
  run_id INTEGER CHECK(run_id IS NULL OR run_id>0),
  fact_version INTEGER NOT NULL CHECK(fact_version>=1),
  last_seq INTEGER NOT NULL CHECK(last_seq>0),
  PRIMARY KEY(job,role),
  CHECK((state='running' AND run_id IS NOT NULL) OR (state<>'running' AND run_id IS NULL AND drifted=0))
) STRICT;
CREATE TABLE run_state (
  run_id INTEGER PRIMARY KEY CHECK(run_id>0),
  job TEXT NOT NULL CHECK(length(job) BETWEEN 1 AND 64),
  role TEXT NOT NULL CHECK(length(role) BETWEEN 1 AND 64),
  generation_id INTEGER NOT NULL CHECK(generation_id>0),
  state TEXT NOT NULL CHECK(state IN ('active','released','interrupted','completed','fenced')),
  last_seq INTEGER NOT NULL CHECK(last_seq>0)
) STRICT;
CREATE UNIQUE INDEX run_state_one_active ON run_state(job, role) WHERE state='active';
-- Reserved for R17
CREATE TABLE reservation_state (
  reservation_id INTEGER PRIMARY KEY CHECK(reservation_id>0),
  state TEXT NOT NULL CHECK(state IN ('open','completed','interrupted')),
  last_seq INTEGER NOT NULL CHECK(last_seq>0)
) STRICT;
-- Reserved for R18
CREATE TABLE blocker_state (
  blocker_id INTEGER PRIMARY KEY CHECK(blocker_id>0),
  state TEXT NOT NULL CHECK(state IN ('open','cleared')),
  cleared_seq INTEGER CHECK(cleared_seq IS NULL OR cleared_seq>0),
  last_seq INTEGER NOT NULL CHECK(last_seq>0),
  CHECK((state='open' AND cleared_seq IS NULL) OR (state='cleared' AND cleared_seq IS NOT NULL))
) STRICT;
CREATE TABLE transitions (
  event_seq INTEGER NOT NULL CHECK(event_seq>0),
  ordinal INTEGER NOT NULL CHECK(ordinal>0),
  job TEXT NOT NULL CHECK(length(job) BETWEEN 1 AND 64),
  role TEXT NOT NULL CHECK(length(role) BETWEEN 1 AND 64),
  generation_id INTEGER NOT NULL CHECK(generation_id>0),
  before_state TEXT CHECK(before_state IS NULL OR before_state IN ('blocked','ready','running','done','stale','retired')),
  after_state TEXT NOT NULL CHECK(after_state IN ('blocked','ready','running','done','stale','retired')),
  before_drifted INTEGER CHECK(before_drifted IS NULL OR before_drifted IN (0,1)),
  after_drifted INTEGER NOT NULL CHECK(after_drifted IN (0,1)),
  reason TEXT NOT NULL CHECK(reason IN ('activated','prerequisite_available','prerequisite_unavailable','claimed','taken_over','fenced','released','interrupted','completed','work_changed','evidence_inapplicable','repair_admitted','structural_change','retired','recreated','drift_cleared')),
  PRIMARY KEY(event_seq,ordinal),
  CHECK(after_state='running' OR after_drifted=0),
  CHECK(before_state IS NULL OR before_state='running' OR before_drifted=0),
  CHECK((before_state IS NULL AND before_drifted IS NULL) OR (before_state IS NOT NULL AND before_drifted IS NOT NULL))
) STRICT;
CREATE TRIGGER ledger_meta_no_update BEFORE UPDATE ON ledger_meta BEGIN SELECT RAISE(ABORT,'immutable ledger row'); END;
CREATE TRIGGER ledger_meta_no_delete BEFORE DELETE ON ledger_meta BEGIN SELECT RAISE(ABORT,'immutable ledger row'); END;
CREATE TRIGGER records_no_update BEFORE UPDATE ON records BEGIN SELECT RAISE(ABORT,'immutable ledger row'); END;
CREATE TRIGGER records_no_delete BEFORE DELETE ON records BEGIN SELECT RAISE(ABORT,'immutable ledger row'); END;
CREATE TRIGGER tree_edges_no_update BEFORE UPDATE ON tree_edges BEGIN SELECT RAISE(ABORT,'immutable ledger row'); END;
CREATE TRIGGER tree_edges_no_delete BEFORE DELETE ON tree_edges BEGIN SELECT RAISE(ABORT,'immutable ledger row'); END;
CREATE TRIGGER revisions_no_update BEFORE UPDATE ON revisions BEGIN SELECT RAISE(ABORT,'immutable ledger row'); END;
CREATE TRIGGER revisions_no_delete BEFORE DELETE ON revisions BEGIN SELECT RAISE(ABORT,'immutable ledger row'); END;
CREATE TRIGGER events_no_update BEFORE UPDATE ON events BEGIN SELECT RAISE(ABORT,'immutable ledger row'); END;
CREATE TRIGGER events_no_delete BEFORE DELETE ON events BEGIN SELECT RAISE(ABORT,'immutable ledger row'); END;
CREATE TRIGGER current_state_no_delete BEFORE DELETE ON current_state BEGIN SELECT RAISE(ABORT,'immutable ledger row'); END;
CREATE TRIGGER requests_no_update BEFORE UPDATE ON requests BEGIN SELECT RAISE(ABORT,'immutable ledger row'); END;
CREATE TRIGGER requests_no_delete BEFORE DELETE ON requests BEGIN SELECT RAISE(ABORT,'immutable ledger row'); END;
CREATE TRIGGER job_activations_no_update BEFORE UPDATE ON job_activations BEGIN SELECT RAISE(ABORT,'immutable ledger row'); END;
CREATE TRIGGER job_activations_no_delete BEFORE DELETE ON job_activations BEGIN SELECT RAISE(ABORT,'immutable ledger row'); END;
CREATE TRIGGER generations_no_update BEFORE UPDATE ON generations BEGIN SELECT RAISE(ABORT,'immutable ledger row'); END;
CREATE TRIGGER generations_no_delete BEFORE DELETE ON generations BEGIN SELECT RAISE(ABORT,'immutable ledger row'); END;
CREATE TRIGGER runs_no_update BEFORE UPDATE ON runs BEGIN SELECT RAISE(ABORT,'immutable ledger row'); END;
CREATE TRIGGER runs_no_delete BEFORE DELETE ON runs BEGIN SELECT RAISE(ABORT,'immutable ledger row'); END;
CREATE TRIGGER dispatches_no_update BEFORE UPDATE ON dispatches BEGIN SELECT RAISE(ABORT,'immutable ledger row'); END;
CREATE TRIGGER dispatches_no_delete BEFORE DELETE ON dispatches BEGIN SELECT RAISE(ABORT,'immutable ledger row'); END;
CREATE TRIGGER checkpoints_no_update BEFORE UPDATE ON checkpoints BEGIN SELECT RAISE(ABORT,'immutable ledger row'); END;
CREATE TRIGGER checkpoints_no_delete BEFORE DELETE ON checkpoints BEGIN SELECT RAISE(ABORT,'immutable ledger row'); END;
CREATE TRIGGER reservations_no_update BEFORE UPDATE ON reservations BEGIN SELECT RAISE(ABORT,'immutable ledger row'); END;
CREATE TRIGGER reservations_no_delete BEFORE DELETE ON reservations BEGIN SELECT RAISE(ABORT,'immutable ledger row'); END;
CREATE TRIGGER evidence_no_update BEFORE UPDATE ON evidence BEGIN SELECT RAISE(ABORT,'immutable ledger row'); END;
CREATE TRIGGER evidence_no_delete BEFORE DELETE ON evidence BEGIN SELECT RAISE(ABORT,'immutable ledger row'); END;
CREATE TRIGGER completions_no_update BEFORE UPDATE ON completions BEGIN SELECT RAISE(ABORT,'immutable ledger row'); END;
CREATE TRIGGER completions_no_delete BEFORE DELETE ON completions BEGIN SELECT RAISE(ABORT,'immutable ledger row'); END;
CREATE TRIGGER publications_no_update BEFORE UPDATE ON publications BEGIN SELECT RAISE(ABORT,'immutable ledger row'); END;
CREATE TRIGGER publications_no_delete BEFORE DELETE ON publications BEGIN SELECT RAISE(ABORT,'immutable ledger row'); END;
CREATE TRIGGER verdicts_no_update BEFORE UPDATE ON verdicts BEGIN SELECT RAISE(ABORT,'immutable ledger row'); END;
CREATE TRIGGER verdicts_no_delete BEFORE DELETE ON verdicts BEGIN SELECT RAISE(ABORT,'immutable ledger row'); END;
CREATE TRIGGER repairs_no_update BEFORE UPDATE ON repairs BEGIN SELECT RAISE(ABORT,'immutable ledger row'); END;
CREATE TRIGGER repairs_no_delete BEFORE DELETE ON repairs BEGIN SELECT RAISE(ABORT,'immutable ledger row'); END;
CREATE TRIGGER blockers_no_update BEFORE UPDATE ON blockers BEGIN SELECT RAISE(ABORT,'immutable ledger row'); END;
CREATE TRIGGER blockers_no_delete BEFORE DELETE ON blockers BEGIN SELECT RAISE(ABORT,'immutable ledger row'); END;
CREATE TRIGGER role_state_no_delete BEFORE DELETE ON role_state BEGIN SELECT RAISE(ABORT,'immutable ledger row'); END;
CREATE TRIGGER run_state_no_delete BEFORE DELETE ON run_state BEGIN SELECT RAISE(ABORT,'immutable ledger row'); END;
CREATE TRIGGER reservation_state_no_delete BEFORE DELETE ON reservation_state BEGIN SELECT RAISE(ABORT,'immutable ledger row'); END;
CREATE TRIGGER blocker_state_no_delete BEFORE DELETE ON blocker_state BEGIN SELECT RAISE(ABORT,'immutable ledger row'); END;
CREATE TRIGGER transitions_no_update BEFORE UPDATE ON transitions BEGIN SELECT RAISE(ABORT,'immutable ledger row'); END;
CREATE TRIGGER transitions_no_delete BEFORE DELETE ON transitions BEGIN SELECT RAISE(ABORT,'immutable ledger row'); END;
```

The immutable tables are `ledger_meta`, `records`, `tree_edges`, `revisions`, `events`, `requests`,
`job_activations`, `generations`, `runs`, `dispatches`, `checkpoints`, `reservations`, `evidence`, `completions`,
`publications`, `verdicts`, `repairs`, `blockers` and `transitions`. Each has both triggers. The mutable tables
`current_state`, `role_state`, `run_state`, `reservation_state` and `blocker_state` have only the no-delete trigger.
After the block runs, `sqlite_master` holds exactly these triggers, the two indexes above and the tables. Each
trigger has the form `CREATE TRIGGER <table>_no_update BEFORE UPDATE ON <table> BEGIN SELECT RAISE(ABORT,'immutable ledger row'); END;`,
or the `_no_delete` form with `BEFORE DELETE`. That is the literal expansion of the format2 pattern
([ledger-contract](ledger-contract.md), the trigger paragraph).

The explicit indexes of format3 are exactly `records_digest`, byte-exact as format2 has it, and
`run_state_one_active`. The partial unique index keeps at most one `active` run for each (job, role). No other
explicit index exists. The implicit indexes of `UNIQUE` and primary key constraints are not index statements.

Event types. `events.type` has exactly fourteen values. The three format2 types are `ledger_initialized`,
`declaration_accepted` and `load_noop`. The eleven added types are `job_activated`, `run_claimed`,
`run_taken_over`, `run_refreshed`, `checkpoint_recorded`, `run_released`, `run_interrupted`, `check_reserved`,
`check_recorded`, `check_interrupted` and `verdict_recorded`. `events.version` is 1 for `ledger_initialized` and
2 for every other type, as [EX-4](#ex-4-facts-derived-state-and-reconcile-rules) fixes.

Transition reasons. `transitions.reason` has exactly the sixteen values in its CHECK. The reasons and their
meanings are fixed by [EX-4](#ex-4-facts-derived-state-and-reconcile-rules) and the state machines.

Request outcomes. `requests.outcome` has the nine values of its CHECK, and each operation pairs with a closed set
of them. A reader verifies the pairing, and the native writer writes only a pairing listed here.

| Operation | Outcome values |
| --- | --- |
| `load` | `accepted`, `noop` |
| `activate` | `activated` |
| `claim` | `claimed` |
| `takeover` | `taken_over` |
| `refresh` | `refreshed` |
| `checkpoint` | `recorded` |
| `release` | `released`, `interrupted` |
| `check` | `recorded`, `interrupted` |
| `check-recover` | `interrupted` |
| `verdict` | `recorded` |

No check request row is written at reserve time (R17-1, step 1), so the nine outcome values have no `reserved` value.
The `check` request row is written only at the end, with the final result, or with an interruption result written by
`check-recover` (R17-7). R17-A's choice for a check whose run is fenced is not pre-decided by this contract.

Reference rule. A column carries REFERENCES if and only if its cell in the execution design's EX-1 table has an
arrow to another table. The author makes no other choice. Every other ID or sequence column is reader-verified and
carries no REFERENCES. The reader-verified columns are `events.job`, `events.role`, `events.generation_id`,
`events.run_id`, `events.request_id` (as in format2), `current_state.active_activation_id`,
`job_activations.revision_id`, `job_activations.event_seq`, `generations.opened_seq`, `runs.generation_id`,
`runs.request_id`, `runs.claimed_seq`, `dispatches.run_id`, `dispatches.revision_id`, `checkpoints.run_id`,
`reservations.request_id`, `reservations.run_id`, `reservations.body_id`, `evidence.reservation_id`,
`completions.run_id`, `completions.dispatch_id`, `publications.completion_id`, `verdicts.request_id`,
`verdicts.run_id`, `repairs.verdict_id`, `repairs.generation_id`, `blockers.verdict_id`, `role_state.generation_id`,
`role_state.run_id`, `run_state.run_id`, `run_state.generation_id`, `transitions.event_seq`,
`transitions.generation_id`, and the projection identifiers `reservation_state.reservation_id` and
`blocker_state.blocker_id`.

Two reader-verified groups need a reason. `events.generation_id` and `events.run_id` carry no REFERENCES because
the rows they name are opened by the same commit that writes the event (`generations.opened_seq` and
`runs.claimed_seq`), so a foreign key in each direction would form a cycle. `reservations.request_id` carries no
REFERENCES because a reservation commits before its request row exists: the reserve commit writes the reservation
and the `check_reserved` event and no request row (R17-1, step 1). Foreign keys are ON
([ledger-contract](ledger-contract.md), the format paragraph), and a foreign key that is not deferred is checked when
its statement ends, so a foreign key on that column would fail the reserve commit.

Cross-row invariants. The native writer enforces each invariant below before commit, and the reader verifies it
before trusting referenced state. Each bullet names which side enforces it. Bullets 1 to 9 are the nine cross-row
invariants of the execution design. Bullets 10 to 20 are the invariants this contract adds to them, and the format2
invariant that continues in format3.

1. Request and event bijection (native writer and reader). Each request row names exactly one event, and that
   event names the request through `events.request_id`. The one exception is the check operation. Its request row
   is written only at the end and references its `check_reserved` event (R17-1). The later `check_recorded` or
   `check_interrupted` event names the reservation, and through it the request, and never a second request row
   (R17-1, R17-7).
2. At most one active run per (job, role) (native writer, enforced by the `run_state_one_active` index).
3. An active run's generation is the role's current generation (native writer and reader).
4. A dispatch's run is active at the dispatch's sequence (reader).
5. Every `transitions` row's `before_state` equals the previous `after_state` of that role (native writer and reader),
   except for the first transition of a (job, role), which bullet 16 governs.
6. `role_state` equals the replay of all `transitions` rows (native writer, by delta, and reader, by replay). For a role whose last `transitions` row has `after_state` `running`, `role_state.run_id` is the `run_id` of the `events` row that carries that row when that event's type is `run_claimed` or `run_taken_over` and its `events.role` (and `events.job`) is that role, that is the target of the operation. Every other running row, including a derived transition of another role that the same `run_claimed` or `run_taken_over` event carries, takes the `run_id` derived for that role's previous `transitions` row, whose `after_state` is also `running`. For every other last `after_state` the `run_id` is NULL. The reader cross-checks the result against the one `run_state` row with state `active` of the role's current generation (invariant 12).
7. A completed reservation has exactly one terminal request (reader).
8. Repair revisions per role and generation are contiguous from 1 (native writer and reader).
9. `current_state` equals the final event's after-state (native writer and reader).
10. `generations.ordinal` is contiguous from 1 for each (job, role) (native writer and reader).
11. `current_state.active_activation_id` equals the `id` of the latest `job_activations` row, and is NULL when no
    activation exists (native writer and reader). `job_activations` is created after `current_state` in the creation
    order, so this column carries no REFERENCES.
12. Coupling: a role is `running` if and only if exactly one `run_state` row with state `active` belongs to the
    role's current generation, and `role_state.run_id` equals that run's `run_id` (native writer and reader). Every role state other than `running` has `run_id` NULL and `drifted` 0, a `running` role has a non-NULL `run_id`, `before_drifted` of a `transitions` row is NULL when `before_state` is NULL and 0 when `before_state` is a non-running state, and `after_drifted` is 0 unless `after_state` is `running`.
13. Outcome pairing: each request row's outcome is a member of the set its operation has in the table above
    (native writer and reader).
14. Reader-verified columns equal the rows they name, as the reference rule lists them (reader).
15. `run_state` is derived, not written by a run transition table. Replaying `events` and `transitions` through the
    run transition table of the [State machines](#state-machines) section reproduces every `run_state` row
    (reader). The SQL has no run transition table.
16. NULL-first transition (native writer). The first transition of a (job, role) has `before_state` and
    `before_drifted` both SQL NULL, and the locked before-state check for it reads "no `role_state` row exists" as
    [EX-3](#ex-3-execution-commit-protocol) states. The same wording applies to `run_state` at claim, where the
    check reads "no `run_state` row exists for the new run id".
17. Non-active-job rows (native writer and reader). Every `role_state` row of a job other than
    `current_state.active_job` is `retired`, and when `active_job` is NULL every `role_state` row is `retired`.
18. `fact_version` replay (native writer, by delta, and reader, by replay). `role_state.fact_version` equals the
    number of committed changes that contain a bumping event type and a `transitions` row for that role. The
    bumping rule is in [EX-3](#ex-3-execution-commit-protocol).
19. Historical reads replay rows (reader). `history --at-event` reconstructs state by replaying the rows above and
    never runs Lua ([ledger-contract](ledger-contract.md), the history paragraph that says historical inspection
    neither executes Lua nor checks).
20. Format2 invariants continue (native writer and reader). Each format2 cross-row invariant of
    [ledger-contract](ledger-contract.md) holds for format3 with the typed columns of format3. Bullets 1 and 13
    restate the request and outcome rules for the execution operations.

Cutover. The following rules apply to the R15 cutover.

- Public `init` creates format3 only, at a new explicit target, with the unchanged CLI signature and
  response bytes and no format selector. The [external profile](external-profile-contract.md#x-store--byte-codecs-exact-provenance-and-supported-formats)
  format3 paragraph supersedes, by insertion, the sentence "New init creates format2 only" in the same section.
  That sentence is not edited.
- Native test fixtures keep explicit format1 and format2 creation for reader checks. Each existing test that pins
  "init creates format2" keeps its name and changes only its asserted format (XD5).
- Each execution operation refuses a format1 or format2 ledger with `unsupported_ledger_version` before any capture
  or Lua runs. The existing message of that code applies, as [Messages](#messages) names it.
- An older binary that opens a format3 ledger refuses it unchanged. The ledger contract already requires a binary to
  refuse a future or unsupported database unchanged ([ledger-contract](ledger-contract.md), the initialization and
  format evolution section).

## EX-2 Request framing and replay

Execution operations other than load use `request_v3`. The framing is the ASCII bytes `PLANAR-REQUEST`, then byte
03, then `bytes(operation) || bytes(actor) || bytes(parameters) || selector_tag || [bytes(profile_argument)]`. The
`bytes` form is the unsigned big-endian u64 byte length followed by the bytes, with no terminator, as
`request_v2` defines it ([external profile](external-profile-contract.md#x-store--byte-codecs-exact-provenance-and-supported-formats),
the `request_v2` framing paragraph). `parameters` is the canonical_v1 encoding of the operation's exact parameter
map. R15-A2 lists that map for each verb. `selector_tag` is byte 00 for embedded and byte 01 for profile, as in
`request_v2`, and `profile_argument` is present only with the profile selector.

- The framing is stored as a record of kind 4 with codec 3, and format2 stores its `request_v2` framing the same
  way as kind 4 with codec 2.
- Host-path arguments enter the parameter map as lexically normalized absolute private strings, as load normalizes
  them. These are the root, base-root, scratch and file-selector arguments. The ledger path is excluded.
- File contents never enter the body. A retry after a file has changed replays the original result.
- Load on format3 keeps `request_v2`, including the profile argument and the `effective_stdlib_v1` path, unchanged.

Replay and conflict.

- A request with the same ID and an equal body returns the saved response bytes.
- The same ID with an unequal body is `request_conflict`, as the existing message of that code names it.
- A body of another request version under the same ID also conflicts. A `request_v2` framing and a `request_v3`
  framing of one ID never replay each other ([external profile](external-profile-contract.md#x-store--byte-codecs-exact-provenance-and-supported-formats)).
- A failed operation reserves nothing. It writes no event and no request row, and its ID stays unused.
- A reused ID never launches a session again. Replaying a historical claim is not permission to launch its
  session again ([planar-design-spec-v5.md](planar-design-spec-v5.md), line 370).

Packet responses. A response that carries a packet is assembled natively from the saved `result` and the immutable
`attachment_id` record that the request row names. Replay returns the same bytes, and Markdown is not stored in the
1 MiB `result` column.

Disposition. For claim and takeover requests, `status --request-id` reports one disposition from the closed
vocabulary `current`, `completed`, `released`, `interrupted` and `fenced`. The vocabulary follows the v5 wording
that a looked-up request reports whether its reservation is current, completed, released or fenced
([planar-design-spec-v5.md](planar-design-spec-v5.md), line 370), with `interrupted` added by the run state
machine. The disposition is the state of the request's run: `current` is the run state `active`, and the other
four names are the run states of the same spelling. The `status` verb is defined in [P2-1](#p2-1-verbs-and-services). The saved result that `status --request-id` returns for a claim or takeover request carries the run token (R15-7, R15-9).

`history --request-id` on format3. Decision OP2, tagged `operator-decided (delegated 2026-10-31, option A)`, applies.

- On format3, `history --request-id` also returns the execution results of its request. For claim and takeover
  requests the result includes the run token, as the [run token](#run-token-r15-9) subsection defines.
- The response keeps the existing request record shape of the ledger request lookup row: the saved result with the
  head and `last_seq`. It adds no `disposition` field. Disposition is reported by `status --request-id` only.
- Format1 and format2 `history --request-id` responses are unchanged.
- Any further response shape, including packet assembly and a per-operation result schema, is deferred to the
  [P2-5](#p2-5-history-event-selector-and-request-results).

### Run token (R15-9)

- Grammar: `<run id>.<generation ordinal>.<32 lowercase hex>`. The run id is `runs.id`. The generation ordinal is
  the ordinal of the run's generation. The 32 hex digits encode the secret.
- The secret is 16 bytes from the operating-system random source, stored in `runs.secret` as a 16-byte BLOB.
  The secret guards against accidental reuse. It does not guard against a reader of the ledger: anyone who can read
  the ledger and learn a request ID can recover the token from the saved result.
- The token is shown only in the claim and takeover results, in `status --request-id` of those requests, and in
  `history --request-id` of those requests on format3 (operator decision E11 A, R15-9, R15-7).
  It is never shown in `status` other than its request lookup, in a history event expansion, in render output,
  in any packet, in logs or in error details. The claim and takeover results carry it only in the field `run.token`.
- A token-bearing operation refuses a wrong secret, or a run that is not current, with `fenced`. The refusal is
  defined by [EX-7](#ex-7-error-codes-and-messages).

## EX-3 Execution commit protocol

Every execution mutation uses this order: activate, claim, takeover, refresh, checkpoint, release, the two check
commits, verdict, and format3 load reconciliation. No Lua runs under the lock, a replay is checked before any
compare-and-swap, and native code owns every invariant.

1. **Prelude.** CLI11 parses the request. The current directory and environment are snapshotted, host arguments
   are normalized, and the ledger is opened read-only and verified. For an operation of this protocol other than
   load, the ledger must be format3, otherwise `unsupported_ledger_version` refuses it ([EX-1](#ex-1-format3)). The exact request lookup runs: an equal replay
   returns the saved bytes, and an unequal body is `request_conflict`. The read snapshot closes. Pure embedded
   registration may precede it.
2. **Snapshot.** Native code reads one `execution_snapshot_v1` in a single read transaction at `last_seq = N`. It
   holds the active revision (recorded interpretation, versions and registry records), the active job, each role's
   generation, state, drifted flag and `fact_version`, the active run and its acknowledged dispatch (W value and
   digests), and the fields the operation needs from the open blockers and completions that its round defines.
   Run token secrets never enter the snapshot. The snapshot is bounded as [EX-8](#ex-8-execution-bounds) states.
3. **Capture.** Only operations that need captured declaration or root inputs capture. Capture runs outside any
   lock and keeps retained receipts. The rounds that own those operations define which operations capture.
4. **Trusted Lua.** The registered handler receives the snapshot, the typed options and any capture results. It
   returns one `execution_change_v1` value. The value holds the operation's typed fact records, the complete next
   role-state vector for the active job, the `transitions` that the vector implies, the rendered packet bytes when
   the operation produces a packet, and an `expected` precondition set.
5. **Native validation.** Native code checks the closed schema of the value, its references and bounds. It
   recomputes the digests natively: the render digest from the packet bytes and the W digest from the W value. It
   checks that each transition's before state equals the snapshot. It refuses fabricated facts, for example a
   completion without native evidence from this invocation. Lua's next-state vector is validated here and is never
   written wholesale.
6. **Locked commit.** The commit runs in `BEGIN IMMEDIATE`. It rechecks identity, format and schema, then relooks up
   the request ID; an equal replay returns the saved bytes before any precondition is compared. It verifies every
   precondition natively, verifies the retained capture receipts, appends the facts, events, `transitions` and
   request row, applies the delta write of the projections, and commits. A failed precondition rolls back everything
   and refuses with the conflict code of the next subsection.
7. **Output.** Native code writes the saved bytes followed by LF. The load post-commit rules apply
   ([load-command-contract](load-command-contract.md), the post-commit fallback text): a short stdout write discards
   the partial output and retries the original intent, and an uncertain commit never implies rollback and preserves
   replay state. Each operation's fallback message is listed in [Messages](#messages).

Precondition vocabulary. The closed vocabulary of preconditions, all checked under the lock, has exactly these eight members, in the
design's spelling:

- `active_revision == R`
- `active_job == J`
- `fact_version(job, role) == v` for every role of the active job
- `role_state(target) == (state, drifted, run)`
- `generation_current(target, G)`
- `run_active(run, G, secret)`
- `reservation_open(id)`
- `request_unused(id)`

No other precondition exists. Every operation uses at least `request_unused` and `active_revision`. The remaining
members for each other operation are defined by [P2-1](#p2-1-verbs-and-services), which defines the sets of `claim` and `takeover`; the sets of `refresh`, `checkpoint`, `release`, the two check commits and `verdict` are deferred to the R16 to R18 contract rounds (D150, amendment A-4). The sets for format3 load and `activate` are fixed
by the paragraph "Format3 load and activate preconditions" below.

Conflicts. A claim that loses a concurrent claim of the same role, because `role_state(target)` changed under it,
is refused `claim_conflict` with its state detail. A claim whose role is not `ready` or `stale` at the snapshot is
`claim_conflict` too, as R15-4 states. Every other failed precondition is `state_conflict`. Nothing changes, and a
retry with the same request ID is the recovery. The check completion commit is the only internal retry, and its
rule belongs to R17 ([R17-1](execution-design.md#r17-checks-boundaries-and-observed-completion), step 3). It is not
restated here.

Fact versions. `fact_version(job, role)` is a per-role counter in `role_state`. The bumping facts are exactly seven:
generation change or retirement, completion, publication, repair admission, verdict, blocker change and revision
acceptance. A committed change that contains at least one bumping fact increments `fact_version` by exactly 1 for
each role that has a `transitions` row in that change, and for no other role. A change with no bumping fact
increments nothing, even when it has `transitions` rows. Claims, refreshes, checkpoints, releases and reservations
are such changes, and they bump nothing. The first row of a (job, role) is written with `fact_version` 1. A `role_state` write, including a
`fact_version` increment, never happens for a role without a `transitions` row in the change. No exception to the
delta rule exists. The roles that each bumping fact increments are:

| Bumping fact | Events | Roles incremented |
| --- | --- | --- |
| Generation change or retirement | `job_activated`, `declaration_accepted` (non-noop), `run_taken_over` | The roles whose generation changed or retired, and every other role with a `transitions` row in the same change |
| Completion and publication | `check_recorded` (labelled R17) | Every role with a `transitions` row in the commit |
| Repair admission and verdict | `verdict_recorded` (labelled R18) | Every role with a `transitions` row in the commit |
| Revision acceptance | `declaration_accepted` (non-noop), guarded by `active_revision == R` | Every role with a `transitions` row in the change |
| Blocker change | Labelled R18 | Only roles with a `transitions` row in the change. A role without such a row gets no increment. R18 decides whether a blocker change writes a row for such a role or states an explicit amendment. This contract does not decide it |

`load_noop` bumps nothing. A replay of `role_state.fact_version` gives the number of committed changes that contain
a bumping event type and a `transitions` row for the role. The bumping event types are `declaration_accepted`
(non-noop), `job_activated` and `run_taken_over`, and the labelled `check_recorded` and `verdict_recorded` types of
R17 and R18. This rule is sufficient for the lost-update guard. Format3 load and `activate` assert the
`fact_version` of every role of the active job (see the paragraph below), so a bump on any one role refuses a
concurrent change that was prepared against the old counters.

Delta write. Native code writes `role_state` only for roles that have a `transitions` row in the change, and
`run_state` only for runs that the change adds or changes. Under the lock, native code checks each transitioned
role's locked `before_state` and `before_drifted` against the transition row. For the first transition of a
(job, role), it checks that no `role_state` row exists. At claim, for `run_state`, it checks that no `run_state` row
exists for the new run id. The same locked before-state check applies to `run_state` for every run row it changes.
Lua's complete next-state vector is validated against the snapshot at step 5 and is never written wholesale, so a
concurrent commit on another role cannot be overwritten by a stale vector.

Format3 load and activate preconditions. A format3 load and an `activate` assert, under the lock, `request_unused`,
`active_revision == R` and `active_job == J`, where `J` is the snapshot's active job or none (SQL NULL). They assert
for every role of the active job both `fact_version(job, role) == v` and
`role_state(job, role) == (state, drifted, run)`. A role with no `role_state` row asserts that no row exists. For
`activate`, they also assert the same two members for every role of the job being activated that already has a
`role_state` row. The reason is that claims do not bump `fact_version`, so the version member alone cannot see a
claim, while the `role_state` member can. The consequence is that a claim which commits between the snapshot and the
load or activation commit makes that load or activation refuse `state_conflict`, and a retry with the same request ID
succeeds. The closed vocabulary does not change: these are the existing members applied to more roles. The
precondition sets of the other operations are in [P2-1](#p2-1-verbs-and-services); the sets of the reserved operations are deferred to the R16 to R18 contract rounds (D150).

## EX-4 Facts, derived state and reconcile rules

Facts are the immutable rows of [EX-1](#ex-1-format3). The mutable projections `current_state`, `role_state`, `run_state`, `reservation_state` and `blocker_state` are written only by the [commit protocol](#ex-3-execution-commit-protocol), from those facts. A status read reports state from the projections and never needs Lua to do so. The reconcile rules below are the rules of the trusted reconcile service (execution design EX-4, `src/stdlib/planar/reconcile.lua`), and they are the normative content of this clause.

**States.** Role states are `blocked`, `ready`, `running`, `done`, `stale` and `retired` (`planar-design-spec-v5.md:61`). Drift is a flag on `running`: `running+drift` means `running` with `drifted` 1, and `running` alone means `drifted` 0. A run is an ownership record, not a state. `accepted` is derived by R18 and is distinct from `done`. For every state other than `running`, `drifted` is 0 and `run_id` is NULL; a `running` role has a non-NULL `run_id`, and `after_drifted` is 0 unless `after_state` is `running`.

**Reconcile precedence.** The after-state of a role is the first of the following six rules whose condition holds for the role's facts after the operation. The list is ordered and has exactly six rules.

1. `retired`: the role's generation is retired, because the role was removed or structurally replaced (v5 `planar-design-spec-v5.md:385`).
2. `running`, with the drift flag set when the current W differs from the acknowledged W: an active run of the current generation exists (v5 `planar-design-spec-v5.md:375` and `:376`).
3. `blocked`: a start prerequisite is absent, and previous completion evidence is preserved (v5 `planar-design-spec-v5.md:381`).
4. `done`: an applicable completion exists (v5 `planar-design-spec-v5.md:378`), or a blocker clears with unchanged valid completion evidence (v5 `planar-design-spec-v5.md:382`).
5. `stale`: an earlier completion in the lineage exists and does not apply (v5 `planar-design-spec-v5.md:380`), or a blocker clears with obsolete evidence or an admitted repair (v5 `planar-design-spec-v5.md:383`).
6. `ready`: the role has never completed and its start prerequisites are satisfied (v5 `planar-design-spec-v5.md:374`).

A running role with a new blocker stays `running` with the blocker recorded, and no transition row is written for it (v5 `planar-design-spec-v5.md:387`).

Prerequisites come from ledger facts and never from a checkout. A `contract` dependency is available when its captured contract text is present in the active revision. A `facet` dependency is available when an existing publication is present. A `done` quality is the producer role's applicable completion, `accepted` is the derived acceptance of R18, and unanswered blocking questions are declaration facts (`declarations.md:219-220`).

**Derivation.** Every after-state in the [role table](#state-machines) is the result of this precedence applied to the role's facts after the operation. The first transition of a (job, role) has a NULL before state (EX-1 invariant 16, and the rule of D19 in the Decisions table).

**Carrying events.** A derived transition is a transition of a role other than the operation's target, or a re-derived prerequisite, drift or applicability state. It is carried by the event of the operation whose committed fact caused it, and that event writes its `transitions` row. The R15 events that carry derived transitions are `declaration_accepted` (load), `job_activated` (activation), `run_claimed` (claim) and `run_taken_over` (takeover). A completion (`check_recorded`, a fact defined by R17) and a verdict (`verdict_recorded`, a fact defined by R18) also carry the derived transitions that the [State machines](#state-machines) section lists for them. R15-C `reconcile.lua` and R17-A must not read `declaration_accepted` as the only event of rows M17-M25, M38 and M40. This rule adds no event type and no transition reason.

**Event types.** `events.type` has fourteen values. The three format2 types are `ledger_initialized`, `declaration_accepted` and `load_noop`. The eleven added types are `job_activated`, `run_claimed`, `run_taken_over`, `run_refreshed`, `checkpoint_recorded`, `run_released`, `run_interrupted`, `check_reserved`, `check_recorded`, `check_interrupted` and `verdict_recorded`. A format3 load keeps its `declaration_accepted` event, or `load_noop` for a noop load, and attaches its `transitions` rows to that event. Activation uses `job_activated`. One operation writes one event. The check operation is the stated exception: it writes `check_reserved`, and later `check_recorded` or `check_interrupted` ([EX-1](#ex-1-format3) invariant 1).

**Transition reasons.** `transitions.reason` has sixteen closed values: `activated`, `prerequisite_available`, `prerequisite_unavailable`, `claimed`, `taken_over`, `fenced`, `released`, `interrupted`, `completed`, `work_changed`, `evidence_inapplicable`, `repair_admitted`, `structural_change`, `retired`, `recreated` and `drift_cleared`. Each state-machine row has exactly one of them. Their reachability by round is as follows.

- R15: `activated`, `prerequisite_available`, `prerequisite_unavailable`, `claimed`, `taken_over`, `fenced`, `structural_change`, `retired` and `recreated`. `work_changed` is reached by a load that moves W (rows M23, M24 and M40). `drift_cleared` is reached by a load that finds the current W equal to the acknowledged W again (row M25).
- R16: `released` and `interrupted`, from the release operation (rows M29 to M36, with events run_released or run_interrupted), and `drift_cleared`, from the refresh acknowledgement (row M28).
- R17: `completed` (row M37). `work_changed` is also reached by a completion that moves a consumer's W (rows M23, M24 and M40, carried by check_recorded). `evidence_inapplicable` is reached by rows M38 and M41, and M41 is governed by OP3 in the [Decisions table](#decisions-table).
- R18: `repair_admitted` (row M39).

**Event payload.** Every format3 event other than `ledger_initialized` writes payload version 2, including `declaration_accepted` and `load_noop`, which carry transitions. The payload of `ledger_initialized` is version 1: the format2 payload, with `references` equal to `{}`. The payload of version 2 is exactly `{version:2,operation,before:{active_revision,active_job},after:{active_revision,active_job},reason,references}`. `after` has the shape of `before`: `active_revision` is a decimal string or `"none"`, and `active_job` is the job ID string, or the empty string `""` when no job is active. `"none"` is not used for the job, because `none` is a legal job ID (`declarations.md:33`). `references` is an object whose values are record or row ID decimal strings, with the key sets of the table below. The event-level `reason` is the `reason` of the row with the lowest `ordinal` in `transitions` for that event, which is ordinal 1 (the SQL has `CHECK(ordinal>0)`), or the empty string when the event has no transitions. The event-level reason never names an operation.

| Event type | version | references | job | role | generation | run | Label |
| --- | --- | --- | --- | --- | --- | --- | --- |
| `ledger_initialized` | 1 (format2 payload) | none; `references` is `{}` | NULL | NULL | NULL | NULL | R15 |
| `declaration_accepted` | 2 | `source`, `registry`, `stdlib`, `interpretation`, `versions` (as format2) | NULL | NULL | NULL | NULL | R15 |
| `load_noop` | 2 | the same five keys | NULL | NULL | NULL | NULL | R15 |
| `job_activated` | 2 | `activation` (the `job_activations` id) and `revision` (the revisions id) | the activated job | NULL | NULL | NULL | R15 |
| `run_claimed` | 2 | `run`, `dispatch`, `packet`, `manifest`, `work`, and `provenance` only when `runs.provenance_id` is not NULL | the job | the role | the role's current generation | the new run | R15 |
| `run_taken_over` | 2 | `run` (the new run), `previous_run`, `dispatch`, `packet`, `manifest`, `work`, and `provenance` only when not NULL | the job | the role | the new generation | the new run | R15 |
| `run_refreshed` | 2 | reserved for R16; keys pinned by R16 | defined by R16 | defined by R16 | defined by R16 | defined by R16 | R16 (reserved) |
| `checkpoint_recorded` | 2 | reserved for R16; keys pinned by R16 | defined by R16 | defined by R16 | defined by R16 | defined by R16 | R16 (reserved) |
| `run_released` | 2 | reserved for R16; keys pinned by R16 | defined by R16 | defined by R16 | defined by R16 | defined by R16 | R16 (reserved) |
| `run_interrupted` | 2 | reserved for R16; keys pinned by R16 | defined by R16 | defined by R16 | defined by R16 | defined by R16 | R16 (reserved) |
| `check_reserved` | 2 | reserved for R17; keys pinned by R17 | defined by R17 | defined by R17 | defined by R17 | defined by R17 | R17 (reserved) |
| `check_recorded` | 2 | reserved for R17; keys pinned by R17 | defined by R17 | defined by R17 | defined by R17 | defined by R17 | R17 (reserved) |
| `check_interrupted` | 2 | reserved for R17; keys pinned by R17 | defined by R17 | defined by R17 | defined by R17 | defined by R17 | R17 (reserved) |
| `verdict_recorded` | 2 | reserved for R18; keys pinned by R18 | defined by R18 | defined by R18 | defined by R18 | defined by R18 | R18 (reserved) |

The table gives, per event type, the payload version (`version`), the key set of `references`, and the typed columns: `job`, `role`, `generation` and `run` are `events.job`, `events.role`, `events.generation_id` and `events.run_id`.

Typed columns are reader-verified to equal the rows they name. Per-role load facts live in `transitions`, not in the event columns.

**Explain.** The explain output of the status read reports the reasons of the changed roles together with the top-level W fields that changed, and it separates the three causes that v5 requires: a work or semantic change, output-vector applicability, and prerequisite availability (`v5-alignment.md:143-145`). Its shape is in [P2-1](#p2-1-verbs-and-services) (`role_detail.explain`).

**Format3 load reconciliation.** A format3 load runs the native prelude and early replay, then reconciles the active job under the candidate revision outside any lock. Under the lock it performs, in order, the compare-and-swap (CAS) of the expected head, the preconditions of [EX-3](#ex-3-execution-commit-protocol) (the set pinned by its paragraph "Format3 load and activate preconditions"), and then one commit of the revision, the `declaration_accepted` event, the `transitions` rows, the generations, the `current_state` update and the request row. Active runs keep their acknowledged revision until refresh (v5 `planar-design-spec-v5.md:497`). A noop load writes no transitions. Its revision equals the active revision exactly, its event is `load_noop`, and it bumps no `fact_version`.

**Absent active job (D94).** If the active job is absent from the new revision, its generations retire and the active job is cleared. Every role of that job that is not already retired gets a `transitions` row in the same `declaration_accepted` event, with reason `retired` (rows M13 and M14, cause (b)), and a role with an active run also fences that run (row M14). After the commit `current_state.active_job` is NULL, `active_activation_id` keeps its value, and the after payload `active_job` is the empty string. Afterwards every `role_state` row is `retired`, as [EX-1](#ex-1-format3) invariant 17 requires.

**Activation.** `activate` makes the named job the single active job of the ledger. It has these effects in one commit under the preconditions of EX-3: a `job_activations` row; `current_state.active_job` and `current_state.active_activation_id` set to that row; a new generation with reason `activated` for every role of the job; every active run of the previous job fenced (row N4); the roles of the previous job retired (rows M5 and M6); and the reconciled state vector. New generations are never reused. A ledger with no active job refuses every run verb with `no_active_job` ([EX-7](#ex-7-error-codes-and-messages)).

**Re-activation.** The literal reading of R15-2 applies: `activate` for the job that is already active is a full re-activation. Every role of that job gets a new generation with reason `activated`, every active run of the job is fenced (row N5), the previous generation of each role is retired as a generation, and new generations are never reused. Each role gets exactly one `transitions` row, from its old state to `blocked` or `ready` with reason `activated` (rows M7 and M8), and its run row goes from `active` to `fenced`, paired with `activated`. Re-activation is not a noop. No role takes the reason `retired` or `fenced` on this path, and no role's state becomes `retired`.

**Preconditions and fact versions.** The load and activation preconditions are the sets pinned by [EX-3](#ex-3-execution-commit-protocol). The `fact_version` counters that they compare change only as the paragraph "Fact versions" of EX-3 states: a committed change that contains a bumping fact increments `fact_version` by exactly one for each role that has a `transitions` row in that change. The `transitions` rows of the [State machines](#state-machines) section are written under that rule.

## EX-5 Role structure, lineage and work_exec_v1

**Role structure.** The structure value of a role is `role_structure_v1` = `{job, role, kind, work, targets, ownership, shape}`. `work` is the sorted unit IDs of a worker role, `targets` is the sorted unit IDs of a reviewer role, `ownership` is the sorted `{work, paths}` pairs, and `shape` is the job shape kind. Its digest is computed by `canonical_v1` and XXH3-128 (`render-contract.md:271`). The digest drives the generations:

- A changed structure digest opens a new generation for the role and fences its active run, if one exists (rows M9 to M12 and N6).
- A removed role retires: its generation retires and its active token is fenced (rows M13, M14 and N7).
- A role re-added under an old name gets a new generation that inherits nothing. No run token, completion, evidence or approval carries over (rows M15 and M16, reason `recreated`).

**Lineage.** A takeover generation (reason `takeover`) fences only. Its structure digest is unchanged, and it carries forward the previous generation's completions, evidence, publications, repair revision, carried findings and approvals. Only generations with reason `activated`, `structural_change` or `recreated` start empty, and a retirement ends a lineage. Wherever EX-4 and the [State machines](#state-machines) section say "in this generation" or "per role and generation", read it as "in this structural lineage": the chain of takeover generations since the last generation that started empty (v5 `planar-design-spec-v5.md:59` and `:370`).

**work_exec_v1.** The execution work fingerprint W is the versioned recipe `work_exec_v1`. It is computable from the ledger alone, so load, activate, status and claim compute it without a checkout (operator decision E5 A). The state-level rules are these: drift is the state of a running role whose current W differs from its acknowledged W ([EX-4](#ex-4-facts-derived-state-and-reconcile-rules)). The field recipe of `work_exec_v1` is in [P2-3](#p2-3-work_exec_v1). The render-only recipe `work_v1` is unchanged and remains what render emits (`render-contract.md:406`: later execution adds its fields through a versioned recipe). The narrowing of v5 `planar-design-spec-v5.md:241` that E5 records is in the [Pointer amendments](#pointer-amendments).

## EX-6 Interpretation binding

Every execution verb, and the `preview` and `recover` reads, interprets the active revision with the exact trusted code that accepted it. The invocation's effective stdlib digest, embedded or with-profile, must equal the active revision's recorded `stdlib_digest`. If it does not, the verb refuses with `interpretation_mismatch` before any mutation (operator decision E3 A). The interpretation check runs after early replay, so a saved result still replays across a binary or profile change, as load does (`load-command-contract.md:103-106`).

Reads that need no current Lua are exempt from the check, so an upgrade cannot block recovery (E3 A). Which verbs are exempt reads is a clause of [P2-1](#p2-1-verbs-and-services) (D108). A revision accepted under a profile requires `with-profile` with the same profile bytes. The factory family is closed at five (`x07-addendum.md:66`): each added family needs an architecture revision, an admission row and a contract amendment. A binary or profile change is absorbed by a fresh load, which creates a new revision reconciled normally.

The basis is v5 `planar-design-spec-v5.md:507`, which states that a changed stdlib, schema or normalizer version participates in revision interpretation, and `ledger-contract.md:104-105`, which states that historical interpretation is never silently regenerated under current Lua.

## EX-7 Error codes and messages

Execution keeps the closed error envelope and the exit policy of the ledger contract: usage 2; semantic, conflict, resource and format refusals 3; I/O, corruption, busy and disk-full 4 (`ledger-contract.md:480-481`). Existing codes are reused where their meaning fits, and the [Messages](#messages) section names the existing message of each reuse. The `field` of every code below is the empty string.

The nine codes below are new. Each is an E16 A amendment: operator decision E16 A (2026-10-28) adds the new codes as flagged amendments, because XD4 states that no new public code or message is added (`remaining-steps-design.md:67`). Each amendment is for execution only, and XD4 is amended for execution only by this clause. Each code is introduced by the round shown. No other new code exists.

| Code | Exit | Field | Details | Meaning | Introduced | Amendment |
| --- | --- | --- | --- | --- | --- | --- |
| `no_active_job` | 3 | `""` | none | The ledger has no active job, or the named job is not the active job. | R15 | E16 A amendment |
| `claim_conflict` | 3 | `""` | `{state}`, where state is one of `blocked`, `running`, `done`, `retired` | The role is not claimable, or a claim lost a concurrent claim of the same role. The details never carry another run's token. | R15 | E16 A amendment |
| `state_conflict` | 3 | `""` | none | A precondition other than the claim check changed between snapshot and commit. Nothing changed; retry with the same request ID. | R15 | E16 A amendment |
| `fenced` | 3 | `""` | `{run_state}`, where run_state is one of `active`, `released`, `interrupted`, `completed`, `fenced` | The token names a run that is not current, or its secret is wrong. | R15 | E16 A amendment |
| `interpretation_mismatch` | 3 | `""` | none | The invocation's stdlib digest does not equal the active revision's recorded digest ([EX-6](#ex-6-interpretation-binding)). | R15 | E16 A amendment |
| `history_unavailable` | 3 | `""` | none | A requested render digest was never dispatched for this role (v5 `planar-design-spec-v5.md:276`). | R16 | E16 A amendment |
| `drifted` | 3 | `""` | none | The acknowledged W is not current (A11). | R16 | E16 A amendment |
| `check_in_progress` | 3 | `""` | none | An open reservation exists for this run or request ID. | R17 | E16 A amendment |
| `unsupported_policy` | 3 | `""` | none | An external approval assertion was made while no assertion policy exists (operator decision Q16 A; acceptance row A24). | R18 | E16 A amendment |

The details are pinned as the table shows. `claim_conflict` and `fenced` carry the details listed, and the other seven codes carry no `details` key. `resource_limit` at a new execution site carries `{limit,maximum}` ([EX-8](#ex-8-execution-bounds)).

**Closed message set.** The execution messages are the nineteen strings of this clause: ten code messages and nine post-commit fallbacks. No other new message exists. Each message is a flagged amendment to XD4 for execution only. The decisions D54, D67 and D68 are delegated operator decisions under the standing delegation of 2026-10-31, option A, and they are recorded as delegated, not as live answers ([Decisions table](#decisions-table)).

Code messages. Each row is exit 3 with field `""`, and each code has exactly one fixed message with no substitution.

| Code | Message |
| --- | --- |
| `no_active_job` | `No active job matches the request` |
| `claim_conflict` | `Role state does not allow this operation` |
| `state_conflict` | `Execution state changed since the request was prepared` |
| `fenced` | `Run is not current` |
| `interpretation_mismatch` | `Interpretation does not match the active revision` |
| `history_unavailable` | `History needed for this request is unavailable` |
| `drifted` | `Role work changed since it was acknowledged` |
| `check_in_progress` | `A check is already in progress for this role` |
| `unsupported_policy` | `Execution policy is not supported` |
| `resource_limit`, at new execution native sites only | `Execution resource limit exceeded` |

The resource message applies to the new bounds of [EX-8](#ex-8-execution-bounds). The existing bounds keep their existing codes and messages, as the Messages section lists.

Post-commit fallbacks. Each of the nine execution operations has two fallbacks, and both share one message. Each has code `io`, exit 4 and field `""`. Variant 1 covers an uncertain commit or any other failure after the commit, and its `details.operation` is the lowercase request operation. Variant 2 covers a stdout write failure after the commit, and its `details.operation` is `stdout`. The EX-3 step 7 rules apply: a short write discards the partial stdout and retries the original intent, and an uncertain commit never implies rollback and preserves replay state.

| Operation | Code | Exit | Field | Message | Variant 1 `details` | Variant 2 `details` |
| --- | --- | --- | --- | --- | --- | --- |
| `activate` | `io` | 4 | `""` | `Activate may be complete; retry the same request ID or inspect request history` | `{operation:"activate"}` | `{operation:"stdout"}` |
| `claim` | `io` | 4 | `""` | `Claim may be complete; retry the same request ID or inspect request history` | `{operation:"claim"}` | `{operation:"stdout"}` |
| `takeover` | `io` | 4 | `""` | `Takeover may be complete; retry the same request ID or inspect request history` | `{operation:"takeover"}` | `{operation:"stdout"}` |
| `refresh` | `io` | 4 | `""` | `Refresh may be complete; retry the same request ID or inspect request history` | `{operation:"refresh"}` | `{operation:"stdout"}` |
| `checkpoint` | `io` | 4 | `""` | `Checkpoint may be complete; retry the same request ID or inspect request history` | `{operation:"checkpoint"}` | `{operation:"stdout"}` |
| `release` | `io` | 4 | `""` | `Release may be complete; retry the same request ID or inspect request history` | `{operation:"release"}` | `{operation:"stdout"}` |
| `check` | `io` | 4 | `""` | `Check may be complete; retry the same request ID or inspect request history` | `{operation:"check"}` | `{operation:"stdout"}` |
| `check-recover` | `io` | 4 | `""` | `Check-recover may be complete; retry the same request ID or inspect request history` | `{operation:"check-recover"}` | `{operation:"stdout"}` |
| `verdict` | `io` | 4 | `""` | `Verdict may be complete; retry the same request ID or inspect request history` | `{operation:"verdict"}` | `{operation:"stdout"}` |

Format3 load keeps its two existing fallbacks byte-exact: `details:{operation:"load"}` and `details:{operation:"stdout"}`, both with the message `Load may be complete; retry the same request ID or inspect request history` (`load-command-contract.md:221` and `:227`). The ten code messages and the nine fallback messages make the nineteen-message set, and no other new message exists.

## EX-8 Execution bounds

All bounds below are checked before allocation, with checked addition. The lower inherited bound wins, and nothing resets the invocation budget (operator decision Q2 A, `external-profile-contract.md:508-510`). A new bound refuses with `resource_limit`, exit 3, field `""`, `details:{limit,maximum}` and the message `Execution resource limit exceeded`. The message is scoped to new execution native sites only. An existing site keeps its code and message, as the last column of each row states. The numbers are the design numbers, and the ten rows are the complete bounds table.

| Boundary | Bound (design numbers) | Existing site (keeps code and message) | New site limit name and maximum | Round |
| --- | --- | --- | --- | --- |
| Execution snapshot | Recorded interpretation as stored, at most 16 MiB (`ledger-contract.md:162`), plus execution state at most 16 MiB canonical | The 16 MiB interpretation bound of `ledger-contract.md:162` as already enforced | `execution_snapshot_bytes`, 16777216 (execution-state part) | R15 |
| Dispatch packet | Role budget at most 1 MiB (`declarations.md:276`); manifest at most 16 MiB; response within the 16 MiB output ceiling (`external-profile-contract.md:523`) | A packet over the role budget refuses `budget_exceeded`; a response over the output ceiling keeps the existing output-ceiling behaviour | `manifest_bytes`, 16777216 | R15 |
| Checkpoint | The render checkpoint schema, at most 8 KiB canonical (`render-contract.md:220`) | The render checkpoint validator's existing message `Checkpoint canonical JSON too large` (`resource_limit`, `src/stdlib/planar/observations.lua:58`) | none | R16 |
| Provenance file (E6) | At most 64 KiB, depth 4, at most 32 settings, strings at most 1024 bytes | none | `provenance_bytes` 65536; `provenance_depth` 4; `provenance_settings` 32; `provenance_string_bytes` 1024 | R15 |
| Verdict file (R18) | At most 1 MiB; at most 256 findings; finding text at most 4096 bytes; summary at most 16 KiB | none | `verdict_bytes` 1048576; `verdict_findings` 256; `verdict_finding_bytes` 4096; `verdict_summary_bytes` 16384 | R18 |
| Observe directory (RA01) | At most 1024 entries; file at most 64 KiB; total at most 4 MiB; output at most 1 MiB canonical | `resource_limit` with the message `Capture limit exceeded` and the names `observation_entries` 1024, `observation_file_bytes` 65536, `observation_total_bytes` 4194304 and `observation_bytes` 1048576 (`authoring-contract.md:504`) | none | RA01 (existing) |
| Check output per check | The registry `max_output_bytes`, at most 16 MiB (`declarations.md:350-351`); per invocation at most 64 MiB retained | None yet. R17 fact (R17-2 and R17-3): over `max_output_bytes`, a recorded `fail` outcome with reason `output_limit` | `check_retained_bytes`, 67108864 | R17 |
| Facet payload per publication | At most 1 MiB per file and 4 MiB per facet | none | `facet_file_bytes` 1048576; `facet_bytes` 4194304 | R17 |
| Check observation (E15) | 1 GiB hashed bytes, 100,000 files, depth 64; the digest-only recheck is recorded as a limit. Tagged operator-decided (E15 A) and reserved for R17, with no behaviour in this part | none | `check_observation_bytes` 1073741824; `check_observation_files` 100000; `check_observation_depth` 64 | R17 |
| History event expansion | Whole response at most 16 MiB (`ledger-contract.md:447`) | The ledger's existing 16 MiB transport-ceiling refusal (`ledger-contract.md:447`) | none | existing |

## State machines

This section is normative. It holds the role table (rows M1 to M41), the run table (rows N1 to N11) and the rules that bind them. Every after-state is the result of the six-rule precedence of [EX-4](#ex-4-facts-derived-state-and-reconcile-rules) applied to the role's facts after the operation. Every row has exactly one of the sixteen reasons, and its Event cell lists every event that carries it.

No `stale` to `ready` row exists within one generation, because `ready` requires that the role never completed (v5 `planar-design-spec-v5.md:383`). The two rows where a stale role becomes `ready` are M8 and M10: each opens a new generation (re-activation or a structural change), and a new generation has never completed. R17-6 is not narrowed by this section: each restoration that R17-6 names has a row here, rows M40 and M41.

**Notation.** `running+drift` is `running` with `drifted` 1, and `running` alone is `running` with `drifted` 0. A comma list in a before cell is shorthand for one row per listed state, with the same cause, after-state, reason and events. A before cell of NULL is the first-ever transition of a (job, role). Structural, activation and removal rows write a transitions row even when the state name is unchanged, because the generation changes. Rows whose before or after state is `done` or `stale` are normative now, but reachable only once R17 facts exist, and R15 itself produces no completion.

Events in the Event cell are written out in full with their labels: `declaration_accepted` (R15), `job_activated` (R15), `run_claimed` (R15), `run_taken_over` (R15), `run_refreshed` (fact defined by R16), `run_released` (fact defined by R16), `run_interrupted` (fact defined by R16), `check_recorded` (fact defined by R17) and `verdict_recorded` (fact defined by R18). The label of a row is the round of its first event. An event of R16 to R18 appears in a row only as a labelled fact, and R15 implements only the R15 events.

**Role table.** The v5 column cites lines of `planar-design-spec-v5.md`.

| Row | Before | Cause | After | Reason | Event | v5 | Label |
| --- | --- | --- | --- | --- | --- | --- | --- |
| M1 | NULL | activate: role of the newly activated job, first-ever (job, role); new generation | blocked | activated | `job_activated` (R15) | :381 | R15 |
| M2 | NULL | activate: same as M1 | ready | activated | `job_activated` (R15) | :374 | R15 |
| M3 | retired | activate: role of the newly activated job that has a retired row; new generation | blocked | activated | `job_activated` (R15) | :381 | R15 |
| M4 | retired | activate: same as M3 | ready | activated | `job_activated` (R15) | :374 | R15 |
| M5 | blocked, ready, done, stale | activate another job: role of the previous job with no active run | retired | retired | `job_activated` (R15) | :385 | R15 |
| M6 | running, running+drift | activate another job: role of the previous job with an active run (run fenced) | retired | fenced | `job_activated` (R15) | :385 | R15 |
| M7 | blocked, ready, done, stale, running, running+drift | activate the job that is already active (re-activation): every role gets a new generation and each active run is fenced | blocked | activated | `job_activated` (R15) | :381 | R15 |
| M8 | blocked, ready, done, stale, running, running+drift | activate the job that is already active: same as M7 | ready | activated | `job_activated` (R15) | :374 | R15 |
| M9 | blocked, ready, done, stale | load: structural digest change, no active run; new empty generation | blocked | structural_change | `declaration_accepted` (R15) | :385, :381 | R15 |
| M10 | blocked, ready, done, stale | load: structural digest change, no active run; new empty generation (same as M9) | ready | structural_change | `declaration_accepted` (R15) | :385, :374 | R15 |
| M11 | running, running+drift | load: structural change of a role with an active run (run fenced); new empty generation | blocked | structural_change | `declaration_accepted` (R15) | :385, :381 | R15 |
| M12 | running, running+drift | load: structural change of a role with an active run (same as M11) | ready | structural_change | `declaration_accepted` (R15) | :385, :374 | R15 |
| M13 | blocked, ready, done, stale | load, two causes with the same row: (a) role removed from the active job, no active run; (b) the active job is absent from the new revision (R15-3), for every role of that job with no active run (D94). After the commit `current_state.active_job` is NULL | retired | retired | `declaration_accepted` (R15) | :385 | R15 |
| M14 | running, running+drift | load, two causes with the same row: (a) role removed from the active job, active run (run fenced); (b) the active job is absent from the new revision (R15-3), for every role of that job with an active run (run fenced; D94). After the commit `current_state.active_job` is NULL | retired | fenced | `declaration_accepted` (R15) | :385 | R15 |
| M15 | retired | load: role re-added under an old name; new generation inheriting nothing | blocked | recreated | `declaration_accepted` (R15) | :385, :381 | R15 |
| M16 | retired | load: role re-added under an old name (same as M15) | ready | recreated | `declaration_accepted` (R15) | :385, :374 | R15 |
| M17 | ready | a start prerequisite becomes unavailable (precedence rule 3): load; a producer completion stops applying (fact defined by R17); an accepted-quality prerequisite stops applying (fact defined by R18) | blocked | prerequisite_unavailable | `declaration_accepted` (R15), `check_recorded` (fact defined by R17), `verdict_recorded` (fact defined by R18) | :381 | R15 |
| M18 | done | the same cause as M17, with the completion evidence preserved | blocked | prerequisite_unavailable | `declaration_accepted` (R15), `check_recorded` (fact defined by R17), `verdict_recorded` (fact defined by R18) | :381 | R15 |
| M19 | stale | the same cause as M17 | blocked | prerequisite_unavailable | `declaration_accepted` (R15), `check_recorded` (fact defined by R17), `verdict_recorded` (fact defined by R18) | :381 | R15 |
| M20 | blocked | a prerequisite becomes available and the role never completed: load; a producer completion appears (fact defined by R17); acceptance appears (fact defined by R18) | ready | prerequisite_available | `declaration_accepted` (R15), `check_recorded` (fact defined by R17), `verdict_recorded` (fact defined by R18) | :374, :383 | R15 |
| M21 | blocked | a prerequisite becomes available and the completion evidence is still valid | done | prerequisite_available | `declaration_accepted` (R15), `check_recorded` (fact defined by R17), `verdict_recorded` (fact defined by R18) | :382 | R15 |
| M22 | blocked | a prerequisite becomes available and the evidence is obsolete or an earlier completion applies | stale | prerequisite_available | `declaration_accepted` (R15), `check_recorded` (fact defined by R17), `verdict_recorded` (fact defined by R18) | :383 | R15 |
| M23 | running | the current W differs from the acknowledged W: load; a producer completion moves a consumer's W (fact defined by R17) | running+drift | work_changed | `declaration_accepted` (R15), `check_recorded` (fact defined by R17) | :376 | R15 |
| M24 | done | the W moves: load; a producer completion moves a consumer's W (fact defined by R17), separate from evidence_inapplicable | stale | work_changed | `declaration_accepted` (R15), `check_recorded` (fact defined by R17) | :380 | R15 |
| M25 | running+drift | the current W again equals the acknowledged W: load; a producer completion restores a consumer's W (fact defined by R17) | running | drift_cleared | `declaration_accepted` (R15), `check_recorded` (fact defined by R17) | :376 | R15 |
| M26 | ready, stale | claim: the role must be ready or stale (R15-4) | running | claimed | `run_claimed` (R15) | :375 | R15 |
| M27 | running, running+drift | takeover: new generation and run, drift cleared, old run fenced | running | taken_over | `run_taken_over` (R15) | :370 | R15 |
| M28 | running+drift | refresh acknowledgement (fact defined by R16); the same run stays active | running | drift_cleared | `run_refreshed` (fact defined by R16) | :377 | R16 |
| M29 | running, running+drift | release (fact defined by R16) closes the run; the role was never completed | ready | released | `run_released` (fact defined by R16) | :384 | R16 |
| M30 | running, running+drift | release (fact defined by R16); an earlier completion no longer applies | stale | released | `run_released` (fact defined by R16) | :384 | R16 |
| M31 | running, running+drift | release (fact defined by R16); a start prerequisite is absent | blocked | released | `run_released` (fact defined by R16) | :384 | R16 |
| M32 | running, running+drift | release (fact defined by R16); applicable evidence is retained | done | released | `run_released` (fact defined by R16) | :384 | R16 |
| M33 | running, running+drift | release with reason interrupted (fact defined by R16); the role was never completed | ready | interrupted | `run_interrupted` (fact defined by R16) | :384 | R16 |
| M34 | running, running+drift | release with reason interrupted (fact defined by R16); an earlier completion no longer applies | stale | interrupted | `run_interrupted` (fact defined by R16) | :384 | R16 |
| M35 | running, running+drift | release with reason interrupted (fact defined by R16); a start prerequisite is absent | blocked | interrupted | `run_interrupted` (fact defined by R16) | :384 | R16 |
| M36 | running, running+drift | release with reason interrupted (fact defined by R16); applicable evidence is retained | done | interrupted | `run_interrupted` (fact defined by R16) | :384 | R16 |
| M37 | running | completion (fact defined by R17) | done | completed | `check_recorded` (fact defined by R17) | :378 | R17 |
| M38 | done | evidence inapplicable (fact defined by R17): output vector or integration evidence, R17-5 | stale | evidence_inapplicable | `check_recorded` (fact defined by R17) | :380 | R17 |
| M39 | done | repair admitted (fact defined by R18): W moves through the repair, R18-4 | stale | repair_admitted | `verdict_recorded` (fact defined by R18) | :383 | R18 |
| M40 | stale | W returns to a value for which an earlier completion in the lineage applies again (R17-6, precedence rule 4): load restores W, or a producer publication restores the role's start facets (fact defined by R17). Tagged NEW-eng (D91) | done | work_changed | `declaration_accepted` (R15), `check_recorded` (fact defined by R17) | :378 | R15 |
| M41 | stale | a producer's output vector is restored, so an earlier completion's output vector again equals the current outputs (fact defined by R17; R17-6, A12). Tagged operator-decided (delegated 2026-10-31, option A) (OP3, option A) | done | evidence_inapplicable | `check_recorded` (fact defined by R17) | :378 | R17 |

Notes on the role table.

- The before cell of M5 is every non-running, non-retired state; the before cells of M7 and M8 are every non-retired state, including `running` and `running+drift`; M13 has the same before states as M5, and M14 is the running counterpart of M13, for each cause. A running role with a new blocker stays running and has no row (EX-4).
- The after-state of M29 to M36 is the precedence result for the retained facts (v5 `planar-design-spec-v5.md:384`, R16-4: ready, stale, blocked or done).
- The only rows whose before state is `retired` are M3, M4, M15 and M16.
- All sixteen reasons appear in at least one row, and all six states appear as after-states.
- M13 and M14 name both causes and D94 (the absent active job). M40 is the W-return row (D91): the role is stale, W has changed, and the earlier completion applies again, so precedence rule 4 gives `done`.
- M41 is tagged operator-decided (delegated 2026-10-31, option A) (OP3). Option A is applied: `evidence_inapplicable` names the applicability fact class and is the reason for both directions of output-vector applicability. The explain output separates the causes by the changed W and vector fields, not by the reason name.

**Run table.** The paired role reason of each run row is the reason of the same event in the role table.

| Row | Before | Cause | After | Event | Paired role reason | Label |
| --- | --- | --- | --- | --- | --- | --- |
| N1 | none (no row) | claim creates the run | active | `run_claimed` (R15) | claimed (M26) | R15 |
| N2 | none (no row) | takeover: new run | active | `run_taken_over` (R15) | taken_over (M27) | R15 |
| N3 | active | takeover: old run | fenced | `run_taken_over` (R15) | taken_over (M27) | R15 |
| N4 | active | activate another job: the run of the previous job | fenced | `job_activated` (R15) | fenced (M6) | R15 |
| N5 | active | re-activation of the active job | fenced | `job_activated` (R15) | activated (M7, M8) | R15 |
| N6 | active | load: structural change of the role | fenced | `declaration_accepted` (R15) | structural_change (M11, M12) | R15 |
| N7 | active | load, two causes with the same row: (a) role removed; (b) the active job absent from the new revision (D94) | fenced | `declaration_accepted` (R15) | fenced (M14) | R15 |
| N8 | active | release | released | `run_released` (fact defined by R16) | released (M29 to M32) | R16 |
| N9 | active | release with reason interrupted | interrupted | `run_interrupted` (fact defined by R16) | interrupted (M33 to M36) | R16 |
| N10 | active | worker completion commit (R17-5) | completed | `check_recorded` (fact defined by R17) | completed (M37) | R17 |
| N11 | active | reviewer run completed by the verdict commit (R18-4) | completed | `verdict_recorded` (fact defined by R18) | fact defined by R18 (the design leaves the reviewer role transition open) | R18 |

- `released`, `interrupted`, `completed` and `fenced` have no outgoing row. They are terminal.
- `interrupted` is reached only by a release with the reason `interrupted`, carried by `run_interrupted`. `check-recover` appears in no run row: it closes a reservation as `interrupted` and changes no `run_state` (D58; R17-7).
- At most one `active` run exists for each (job, role). The partial unique index `run_state_one_active` enforces it ([EX-1](#ex-1-format3)).
- No run-transition table exists in SQL. `run_state` is reader-verified as derived from `events` and `transitions` through the rows above, and the history reconstruction at an event rebuilds it without Lua (EX-1 invariants 15 and 19).

## Part 2: execution verbs, projections and reads (R15-A2)

Part 2 is normative. It completes the verbs and reads of round R15: the `activate`, `status`, `preview`, `claim` and `takeover` verbs, the dispatch projection and `dispatch_manifest_v1`, the `work_exec_v1` recipe, the driver provenance input, and the format3 history and orchestrator shapes that R15 adds. It defines no R16 to R19 behaviour: `refresh`, `checkpoint`, `release`, `recover`, `check`, `check-recover` and `verdict` stay reserved names, and `--since` stays R16. Part 1 is fixed except the amendments A-1 to A-13 listed in the section "Part 1 amendments (R15-A2)". This part adds no code and no message.

### P2-1 Verbs and services

#### Common verb rules

- One command per verb. Reads are separate from mutations. Mutation verbs (`activate`, `claim`, `takeover`) require `--request-id` and `--actor` with the load grammar of `load-command-contract.md` and the ledger contract (request keys 1 to 128 bytes of `[A-Za-z0-9_.:-]`; actor nonempty text). Reads (`status`, `preview`) write nothing and have no request framing.
- All options are separate tokens (`--name value`), scalar values nonempty and not starting `--`, no defaults, aliases, positionals, repeats or short flags. A flag cannot be required.
- `--ledger`, `--workspace`, `--registry` and `--provenance` are opaque native host-path slots; no host path crosses to Lua. `--job`, `--role`, `--request-id`, `--actor`, `--expect-revision` and `--expect-job` are text. `--run-id` is a canonical positive decimal 1..9223372036854775807 that crosses into Lua as a canonical decimal string.
- Each execution mutation uses the EX-3 commit protocol and `request_v3` (EX-2). Early replay precedes the interpretation binding of EX-6.
- No twentieth message exists: Part 2 adds no code and no message text. Every refusal named below uses a code of the E16 set or an existing code with its existing message ([Messages](#messages)).
- The R16 to R19 verbs keep their E1 rows as reserved names and are unknown commands (`usage`) until their rounds.

#### Option maps

Columns are the verb, its kind, its required options, its optional options, its exclusions and rules, and its response kind.

| Verb | Kind | Required | Optional | Exclusions and rules | Response kind |
| --- | --- | --- | --- | --- | --- |
| `activate` | mutation | `--ledger`, `--job`, `--expect-revision`, `--expect-job`, `--request-id`, `--actor` | none | `--expect-revision` is `ID_OR_NONE` as load spells it (a canonical positive decimal revision ID or `none`); `--expect-job` is `NONE` or a job ID (D104; its help metavariable is `JOB_OR_NONE`) | `activate_v1` |
| `status` | read | `--ledger` | `--role`, `--request-id`, `--workspace`, `--registry` | `--role` excludes `--request-id` and nothing else (E1); `--request-id` may combine with `--workspace` and `--registry`; `--workspace` and `--registry` both or neither; any violation is `usage` | `status_v1` |
| `preview` | read | `--ledger`, `--role` | none | R16 adds `--since`; not in R15 | `preview_v1` |
| `claim` | mutation | `--ledger`, `--job`, `--role`, `--request-id`, `--actor` | `--provenance` | none | `claim_v1` |
| `takeover` | mutation | `--ledger`, `--job`, `--role`, `--run-id`, `--request-id`, `--actor` | `--provenance` | `--run-id` is the current active run id of the role, not a token | `claim_v1` |

#### Expectation spelling

`--expect-job` is `NONE` or a job ID (D104). The uppercase value `NONE` is the no-active-job expectation; any other value is a job ID. `none` is a legal job ID and stays an ordinary job ID, because `declarations.md:33` defines `id` as `[a-z][a-z0-9_]{0,63}`, which `none` matches. The help metavariable is `JOB_OR_NONE`, not `ID_OR_NONE`, so that it does not share the metavariable of `--expect-revision`, whose value spelling is the lowercase `none`. A flag is not used because flags cannot be required, and an empty string is not used because scalars are nonempty.

#### Parameter maps

Request parameter maps (`request_v3`, EX-2; keys sorted by canonical_v1; paths enter as normalized absolute private strings; file contents never enter):

| Operation | `parameters` map |
| --- | --- |
| `activate` | `{expect_job, expect_revision, job}`; `expect_job` is the text `NONE` or the job ID, `expect_revision` is the decimal string or `none` |
| `claim` | `{job, role}` plus `provenance_path` only when `--provenance` was supplied |
| `takeover` | `{job, role, run_id}` plus `provenance_path` only when `--provenance` was supplied; `run_id` is a decimal string |

#### Precondition sets

Every set contains `request_unused(id)` and `active_revision == R`. Members by operation:

| Operation | Members (all checked under the lock) |
| --- | --- |
| format3 load and `activate` | exactly as EX-3 "Format3 load and activate preconditions" (unchanged) |
| `claim` | `request_unused`, `active_revision == R`, `active_job == J`, `fact_version(job, role) == v` for every role of the active job, `role_state(target) == (state, drifted, run)`, `generation_current(target, G)` |
| `takeover` | the `claim` set with `role_state(target) == (running, drifted, run)` where `run` is the `--run-id`; no member uses a secret; `run_active` is not used |
| `refresh`, `checkpoint`, `release`, the `check` and `check-recover` commits, `verdict` | not defined in this part (D150): these operations are reserved and have no request body, facts or response here; their member sets are defined by the contract round of each operation (R16, R17, R18), using only the eight closed members |
| `status`, `preview` | none (reads; one read transaction) |

P2-1 defines the member sets of `claim` and `takeover` only, besides the unchanged load and `activate` sets. Part 1 EX-3 says in two places that the remaining members of other operations are defined by R15-A2; amendment A-4 re-points those sentences to this section and records that the sets of `refresh`, `checkpoint`, `release`, the two check commits and `verdict` are deferred to the R16 to R18 contract rounds, because the operations are reserved and nothing here can pin members before their lifecycle and event shapes exist.

#### Enforcement of the precondition sets

Enforcement (D151). Lua supplies `expected`; native enforces it. At EX-3 step 5 native validation requires `expected` to equal exactly the fixed set of the operation in the table above: the same member kinds, with none omitted, added or duplicated (for `claim` and `takeover`, the six members, with one `fact_version` member per role of the active job), and every value equal to the snapshot read by the same command. Native refuses a `claim` change whose target before-state in the snapshot is not `ready` or `stale`, and a `takeover` change whose named run is not the target's active run. A handler that sends any of these changes is refused with code `internal` (its existing message; a handler response mismatch, not a driver-visible conflict), because a correct handler returns `claim_conflict`, `fenced` or `state_conflict` before it builds the change. A permissive handler therefore cannot omit `fact_version` or `role_state`, or claim a `done` role, and have native commit it.

#### Mismatch mapping

Mismatch mapping (closed EX-7 details; D114).

- At the snapshot, `claim` on a role that is not `ready` or `stale` is `claim_conflict` with `{state}`, the role's state, which is one of `blocked`, `running`, `done`, `retired`.
- At the snapshot, `takeover` is never `claim_conflict`: a `--run-id` that names no run of that job and role is `state_conflict` (a snapshot-time site, before the lock, named in the Messages cell by amendment A-13; a same-ID retry cannot succeed because the run id names no run, which is a recorded limit); any other run id that is not the role's current active run (this includes every role that is not `running`) is `fenced` with `{run_state}` of the named run.
- Under the lock, for `claim`, a failed `role_state(target)` member is `claim_conflict` with `{state}` of the locked state only when the locked role is taken by another run (`running`) or is in another closed-set state (`blocked`, `done`, `retired`). When the locked state is `ready` or `stale` (for example a stale role turned ready by a concurrent structural load) it is `state_conflict`, as is every other failed member (`request_unused`, `active_revision`, `active_job`, `fact_version`, `generation_current`).
- Under the lock, for `takeover`, a failed `role_state(target)` member is `fenced` with the locked `run_state` of the named run when that run is no longer the role's current active run, else `state_conflict`; every other failed member is `state_conflict`.

`claim_conflict` never carries `ready` or `stale`, and `fenced` never carries a value outside its five. `reservation_open` and `run_active` are not used in R15. Claims of two different roles do not conflict because neither asserts the other's `role_state` and claims bump no `fact_version`.

#### Refusal order

Refusal order (D109) for a non-exempt verb (`activate`, `claim`, `takeover`, `preview`, `status --role`):

- (1) `usage`;
- (2) `unsupported_ledger_version` for a format1 or format2 ledger;
- (3) early request replay (equal body returns saved bytes, unequal `request_conflict`; mutations only; replay never reads `--provenance`);
- (4) `interpretation_mismatch` (EX-6; only when an active revision exists);
- (5) expectations (`activate` only): `--expect-revision` mismatch is `revision_conflict` with `{expected,actual}`; `--expect-job JOB` naming a job that is not the active job, or any job when none is active, is `no_active_job` (EX-7: "The ledger has no active job, or the named job is not the active job"); `--expect-job NONE` while a job is active is `state_conflict` at the snapshot, before the lock, a site named by amendment A-13;
- (6) `no_active_job` when no job is active or `--job` is not the active job (`claim`, `takeover`, and `preview` and `status --role` when none is active);
- (7) `reference` with message `Unknown selected job` and field `selection.job`, then `Unknown selected role` and field `selection.role` (`activate`: the job must exist in the active revision; `claim` and `takeover`: the role must exist in the job; `status --role` and `preview`: see the role references below);
- (8) role-state refusals (`claim_conflict`, `fenced` or `state_conflict` per the mismatch mapping above);
- (9) for `claim` and `takeover` with `--provenance`, the provenance input refusals of [P2-4](#p2-4-provenance-driver_provenance_v1) (the file is read by `read_provenance` after `read_execution`, so a refusal at steps (1) to (8) wins and the file is not read);
- (10) commit preconditions (`state_conflict`).

#### Driver-visible limits

Driver-visible limit (D149): a same-ID retry cannot succeed after the `state_conflict` of `activate` `--expect-job NONE` while a job is active, or after the `state_conflict` of a `takeover` `--run-id` that names no run; the first is deterministic until the active job changes and the second never changes, so the driver must change its expectation or run id, not retry. An `activate` `--expect-job JOB` mismatch is `no_active_job` for the same reason and carries the same limit.

#### Role references of the reads

Role references of the reads (D152). "The role exists in the job" means declared by the active revision's recorded interpretation for the active job. For `status --role R` and `preview --role R` with an active job, a role that is not declared and is not a `role_state` row of the job is `reference` (message `Unknown selected role`, field `selection.role`). A role that `status_v1` lists as `retired` but that the active revision no longer declares has no current W: `status --role R` returns `role_detail` with `current` the string `none`, `changed` `[]` and `explain` `{work_or_semantic:[],output_vector:[],prerequisite:[]}` (`acknowledged` and `reasons` as for any role), and `preview --role R` is `reference` with the same message and field.

#### Exempt reads

Exempt reads (D108, D142): `status` without `--role` (bare, `--request-id`, `--workspace` with `--registry`, or both of the last two), `history`, `export` and `orchestrator --ledger` skip step (4) only. `init` is not an exempt read: it is classified `not bound (no active revision exists; not an execution verb)`, because it creates a ledger and so is a mutation of the read/mutation split of E1, with no active revision to bind. Their order is: (1) `usage`; (2) `unsupported_ledger_version` for a format1 or format2 ledger, which applies to every form of `status` (including `status --role`) and to `orchestrator --ledger`, because they read execution tables that exist only in format3, and does not apply to `history` and `export`, which keep every ledger format (`history --event` is valid on all), nor to `init`, which creates a ledger and applies no format, replay or binding step; (3) for `status --role` only, the interpretation binding of step (4); (4) the verb's own refusals (`request_not_found` for `status --request-id`, `no_active_job` for `orchestrator --ledger` when no job is active). Exempt reads never replay and never write. `preview` is bound, not exempt: it follows the non-exempt order, so its format refusal is step (2), before the binding. On a format1 or format2 ledger no degenerate status shape exists.

#### Verb behaviour and responses

`activate`: effects as EX-4 "Activation" and "Re-activation". Response `activate_v1`: `{version:1,kind:"activate",ledger,request_id,revision,job,previous_job,activation,event,roles}`. `ledger` is the 32-lowercase-hex identity; `revision`, `activation` and `event` are canonical decimal strings; `previous_job` is the job ID or the empty string; `roles` is an array sorted by role ID of `{role,generation,state}` where `generation` is the new ordinal as a decimal string and `state` is the after-state (`blocked` or `ready`).

`claim`: role must be `ready` or `stale`, otherwise `claim_conflict` with `{state}` (details of EX-7: `blocked`, `running`, `done`, `retired`). Lua renders the dispatch packet ([P2-2](#p2-2-dispatch-projection-and-dispatch_manifest_v1)). One EX-3 commit creates the run (token secret from the OS random source), the dispatch with retained packet, manifest and W records (`dispatches` row, `kind` `claim`), the `running` transition and the request. Response `claim_v1`: `{version:1,kind:"claim",ledger,request_id,job,role,revision,run:{id,generation,token},dispatch:{id,work_fp,render_digest,bytes},packet,manifest}`. `work_fp` and `render_digest` are `xxh3_128:` digests; `bytes` is the packet Markdown byte length; `packet` is the packet value P and `manifest` is the manifest value M, both assembled natively. `run.id` and `dispatch.id` are decimal strings, `run.generation` is the generation ordinal as a decimal string, and `run.token` follows the [Run token](#run-token-r15-9) grammar. Key order is canonical (sorted).

`takeover`: requires `--run-id`, the id of the run the driver believes is current; the secret is not required. At step (8) the named run must be a run of that job and role, otherwise `state_conflict` (no run state exists, so none is invented). If the named run is not the role's current active run, which includes every role that is not `running` (`ready`, `stale`, `blocked`, `done`, `retired`: no run is current), the refusal is `fenced` with `{run_state}` of the named run. `claim_conflict` is never used by takeover (D114). Only a role that is `running` (with or without drift) whose current active run is the named run proceeds. One commit: old run `fenced`, new generation (reason `takeover`, lineage carried), new run, dispatch `kind` `takeover`, event `run_taken_over`. Response `claim_v1` with `kind:"claim"` and the same keys plus `previous_run` (decimal string of the fenced run id) present only for takeover.

`status`: response `status_v1`: `{version:1,kind:"status",ledger,active_revision,active_job,last_event,roles,drafts?,role_detail?,request?}`. `active_revision` is a decimal string or `none`; `active_job` is the job ID or the empty string; `last_event` is a decimal string; `roles` is the array of every `role_state` row of the active job (a retired role that the active revision no longer declares stays listed with state `retired`) sorted by role ID, each `{role,kind,generation,state,drifted,run}` with `kind` `worker` or `reviewer` taken from the `role_structure_v1` record of the role's current generation (`snapshot.roles[].generation.structure`), never from the interpretation, `generation` the ordinal decimal string, `drifted` a boolean, and `run` either the string `none` or `{id,generation}` (decimal strings, never a token or secret); `roles` is `[]` when no job is active. `--role R` adds `role_detail`: `{role,acknowledged,current,changed,explain,reasons}` where `acknowledged` is `none` or `{work,render,dispatch}` (digest texts and the dispatch id of the active run's latest dispatch), `current` is `{work}` (or the string `none` for a retired role the active revision no longer declares, D152), `changed` is the sorted array of top-level W field names whose values differ between the acknowledged W value and the current one (empty when equal or when no acknowledged W exists), `explain` is `{work_or_semantic,output_vector,prerequisite}` separating the three v5 causes (`work_or_semantic`: array of changed W field names; `output_vector`: array, always `[]` in R15 because no completion exists; `prerequisite`: array of `{id,phase,status}` of the prerequisites whose status is not `available`), and `reasons` is the array of `{event,ordinal,reason}` for the role's most recent `transitions` rows, oldest first, at most 16. `changed` and `explain` are computed from the decoded acknowledged W value `snapshot.roles[].run.dispatch.work` (see the services and protocol below) and the current W. With `--workspace` and `--registry`, `drafts` is added: `{status,changed,error?}` where `status` is `unchanged`, `changed` or `invalid`, `changed` is the sorted array of logical portable paths of captured declaration-closure files that differ from, are absent from, or are added to the active revision's source, and `error` (only for `invalid`) is the typed error object `{code,message,field}` that load would report; nothing is accepted. With `--request-id`, `request` is added: `{request_id,operation,outcome,result,disposition?}` where `result` is the assembled saved result (the claim and takeover result carries the run token; D118) and `disposition` is present only for claim and takeover with the EX-2 vocabulary; unknown request is `request_not_found`. `--request-id` may be combined with `--workspace` and `--registry` (`request` and `drafts` are then both present); only `--role` excludes `--request-id`. On a format1 or format2 ledger every form of `status` is `unsupported_ledger_version` (D142).

`preview`: no active job gives `no_active_job`; the role must exist in the job, meaning declared by the active revision's recorded interpretation (`reference`, message `Unknown selected role`, field `selection.role`, also for a `retired` role the revision no longer declares). Response `preview_v1`: `{version:1,kind:"preview",ledger,revision,job,role,generation,state,work_fp,render_digest,bytes,packet,manifest}`; it claims and retains nothing, writes no event and no request, and has no token. Its packet has `mode:"dispatch"` with the preview authority template. The `generation` of a preview is the role's current generation ordinal and a preview never creates a generation.

The run token is never in a packet, manifest, `--event` expansion, status other than lookup, preview or log. The claim and takeover results carry it only in the field `run.token`, and their `status --request-id` and `history --request-id` lookups carry it in that same field (D118).

#### Services and protocol

Each service is callable at most once, in the stated order; failure poisons further use; no Lua runs under the write lock; argument maps have exact keys.

| Family | Services in order | Notes |
| --- | --- | --- |
| `activate` | `read_execution({})`; `commit_execution({change:C})` | no capture |
| `status` | `read_execution({})`; then, only with `--request-id`, `read_request({request_id:TEXT})`; then, only with `--workspace` and `--registry`, `open_workspace({workspace:SLOT})` and `capture_drafts({route:R})` | no commit; the optional services combine and keep this order |
| `preview` | `read_execution({})` | no commit |
| `claim` | `read_execution({})`; `read_provenance({provenance:SLOT})` only when `--provenance` is present; `commit_execution({change:C})` | |
| `takeover` | the `claim` services | |
| format3 `load` | `open_workspace`, `capture_load`, `read_execution({})`, `accept_load({bundle:B,semantic:M,execution:CHANGE})` | format1 and format2 keep three services and `{bundle,semantic}` |

Results:

- `read_execution({})` returns `{version:1,ledger,last_seq,snapshot}`. `snapshot` is `execution_snapshot_v1` with exactly `active_revision` (decimal string or `none`), `active_job` (ID or empty string), `activation` (decimal string or `none`), `revision` (`none` or `{id,interpretation,versions,stdlib_digest,registry,source}` with the decoded recorded values), `roles` (every `role_state` row, sorted by job then role: `{job,role,kind,generation:{id,ordinal,structure},state,drifted,fact_version,run}`, where `structure` is the decoded `role_structure_v1` value and `kind` equals its `kind`; retired roles keep their rows; `run` is `none` or `{id,generation,dispatch:{id,work_fp,render_fp,revision,work}}`, where `work` is the decoded acknowledged `work_exec_v1` W value of that dispatch, the retained record `dispatches.work_id`, because EX-3 step 2 requires the active run and its acknowledged dispatch, W value and digests) and `publications` (array, `[]` in R15). Run secrets and tokens never appear. Bound: `execution_snapshot_bytes` 16777216 (EX-8).
- `read_request({request_id})` returns `{request_id,operation,outcome,result,disposition?}` (the assembled saved result).
- `read_provenance({provenance:SLOT})` returns the normalized `driver_provenance_v1` document after the bounds of [P2-4](#p2-4-provenance-driver_provenance_v1); a refusal is one of `io`, `path_denied`, `capture_changed`, `resource_limit` or `schema`, as mapped in P2-4.
- `capture_drafts({route:R})` returns `{outcome:"captured",bundle:B,changed:[paths]}` or `{outcome:"rejected",error:E,changed:[paths]}`; E is the typed error of the declaration-content classes (`lua_syntax`, `capability_denied`, `schema`, `reference`, `ownership`, `dependency_cycle`, `path_denied`, `unsupported_version`); `capture_changed`, `io`, `resource_limit`, `ledger_busy` and `internal` fail the command. Lua runs `interpret_bundle` on a captured bundle to separate `unchanged` and `changed` from `invalid`.
- `commit_execution({change:C})` returns the decoded saved response value after the lock is released. `C` is `execution_change_v1` with exactly `version` (1), `operation` (`load`, `activate`, `claim` or `takeover`), `snapshot_seq` (decimal string), `facts` (operation facts: activate `{job,generations}`; claim `{role,provenance?}`; takeover `{role,previous_run,provenance?}`; load `{generations,fenced_runs}`), `vector` (the complete next role-state vector of the active job, validated and never written wholesale), `transitions`, `packet` (claim, takeover only: `{markdown,manifest,work}`) and `expected`. `expected` is an array of objects with `kind` one of the eight closed members (`active_revision`, `active_job`, `fact_version`, `role_state`, `generation_current`, `run_active`, `reservation_open`, `request_unused`). Native validates `expected` as an exact fixed set and refuses a non-claimable claim target or a takeover whose run is not the active run (D151, see Enforcement of the precondition sets above), recomputes the render digest from the packet bytes and the W digest from the W value, allocates every id and the run secret, and the handler's response must equal the retained native result.
- Format3 `accept_load` result is the existing complete load success envelope; the extra key `execution` is `CHANGE` of the same shape with `operation:"load"`. This amends the `load-command-contract.md` services table for format3 only.

The exempt reads skip the interpretation binding step only (D108); their order and their format1 and format2 outcome are stated under Exempt reads. `status` without `--role` and `orchestrator --ledger` run with whatever embedded Lua the binary carries.

### P2-2 Dispatch projection and dispatch_manifest_v1

#### Packet mode and sections

- Packet value P keeps the render keys `version`, `mode`, `workspace`, `spec`, `job`, `role`, `kind`, `work`, `markdown`; `mode` is `"dispatch"` for claim, takeover and `preview`. Render's `mode:"preview"` is unchanged and render stays database-free. The `preview` verb builds its packet the same way. Its packet has `mode:"dispatch"` and the preview authority template below.
- Markdown follows the render presentation convention (LF, no BOM, final LF). Digest `render_fp` is `H("fragment_v1", exact_markdown_UTF8_bytes)`.
- Section order for dispatch: `authority`, `run`, `goal`, `acceptance`, `requirements`, `contracts`, `boundary`, `dependencies`, `shape_policy`, `checks`, `outputs`, `observations`, `preferences`, `rationale`, `workspace_notes`, `job_notes`, `checkpoint`, `omissions`, `recorded_history`, as the shipped worker and reviewer projections order them with `run` inserted second. `run` is mandatory and is never omitted by the budget algorithm.
- `dispatch_v1` is the identifier of the dispatch projection rules of this section: packet mode `dispatch`, the section order with `run` second, the two authority templates, the run-section bytes and the `dispatch_manifest_v1` recipe. A semantic change to any of them needs a new identifier. The identifier names these rules and carries no data.

#### Authority statements

Fixed authority statements (the `authority` section body, byte-exact templates; `{x}` marks substitution):

- dispatched (claim and takeover): `This packet was dispatched by Planar for run request {request_id}, generation {generation} of role {role}, under revision {revision} of job {job}. Checks have not run in this dispatch. Upstream facet content comes from recorded publications, and prerequisite status comes from ledger qualities, not from a checkout.`
- preview: `This packet is a preview of a dispatch for generation {generation} of role {role}, under revision {revision} of job {job}. No run exists and nothing was retained. Checks have not run in this dispatch. Upstream facet content comes from recorded publications, and prerequisite status comes from ledger qualities, not from a checkout.`

`{generation}` is the generation ordinal; `{revision}` is the revision id; both are decimal.

#### Run section

- The `run` section is framed as every generated section (`## run`, a blank line, the body, one structural LF; the render Markdown projection rule). Its body is these lines joined by single LFs with no blank line: first `run request: {request_id}` (dispatched) or `run request: (preview)` (preview); then the legal next actions of the role kind in E1 table order, worker `refresh`, `checkpoint`, `check`, `release`, reviewer `refresh`, `checkpoint`, `verdict`, `release`, one line `- {verb}: {purpose}` each; then the line `Actions not shipped in this build are unavailable.` The purposes are fixed byte-exact by this contract (D147), each at most 200 bytes, and `dispatch.lua` reproduces them:

  | verb | purpose (the text after `- {verb}: `) |
  | --- | --- |
  | `refresh` | `Re-render this work and acknowledge the current packet.` |
  | `checkpoint` | `Record observations of progress on this run.` |
  | `check` | `Run the checks of the job against the output of this role.` |
  | `release` | `End this run as released or interrupted.` |
  | `verdict` | `Record the verdict of the reviewer for this run.` |

The preview spelling `(preview)` contains characters outside the request ID grammar `[A-Za-z0-9_.:-]{1,128}`, so it never equals a legal request ID.

In R15 these verbs are reserved. The run section never contains a token or secret.

#### Projections

Projection IDs: the shipped-owned `worker` and `reviewer` projections produce dispatch packets from the internal input `dispatch_context_v1` (not an admissible external projection input). No new projection ID exists, and external projections do not apply to dispatch.

#### Run section identifier

The section ID `run` is a reserved built-in section ID (D135; pointer in [P2-7](#p2-7-command-render-load-ledger-and-external-profile-pointers)).

#### dispatch_manifest_v1 fields

Canonical_v1 value M, stored as a value record with codec ID `dispatch_manifest_v1` (`dispatches.manifest_id`). Bound `manifest_bytes` 16777216 (EX-8). Fields, exactly:

| Key | Value |
| --- | --- |
| `version` | 1 |
| `revision` | revisions id, decimal string |
| `job`, `role` | IDs |
| `generation` | the generation ordinal, decimal string |
| `run` | `{request,kind}` for a dispatch: `request` is the claim or takeover request ID and `kind` is `claim` or `takeover` (takeover adds `previous`, the decimal string of the fenced run id); for `preview` the value is `{kind:"preview"}` with no `request` key |
| `versions` | the seven `work_exec_v1` versions then `dispatch_v1` |
| `work` | `{digest}` (the `xxh3_128:` work fingerprint; the W value is the record `dispatches.work_id`) |
| `render` | `{digest,bytes,budget_bytes}` as render M |
| `sections` | ordered array of `{id,mandatory,included,bytes}`, includes `run` with `mandatory:true` |
| `omitted` | ordered array of optional section IDs |
| `prerequisites` | sorted array as render M, statuses from ledger qualities (`available`, `unavailable`, `unverified`, `answered`; `not_captured` never) |
| `registry` | `{id,revision,authority:"driver_selected_trusted_configuration"}` |
| `inputs` | `{source,interpretation,publications,repair}`: `source` and `interpretation` are record ids (decimal strings of `revisions.source_id` and `revisions.interpretation_id`), `publications` is a sorted array of `{work,facet,publication,semantic}` (publication id decimal string, semantic digest text; `[]` in R15), `repair` is the repair revision as a decimal string (`"0"` in R15) |

There is no `files` array, no `bytes_hex`, no `base`, `observation`, `outputs`, `role_output`, `targets`, `assertions`, token or secret key. The W value and the packet Markdown are separate records (`dispatches.work_id`, and file record mode 420 for `dispatches.packet_id`). Render-contract pointer: [P2-7](#p2-7-command-render-load-ledger-and-external-profile-pointers).

A preview manifest has the same 14 keys as a dispatch manifest. Its keys `version`, `revision`, `job`, `role`, `generation`, `versions`, `work`, `prerequisites`, `registry` and `inputs` are computed as for a dispatch, except that `generation` is the role's current generation ordinal, the same value as `generation` in the `roles` entries of `status_v1`, and a preview never creates a generation. Its keys `render`, `sections` and `omitted` are computed from the preview packet's own bytes, which use the preview authority template and the `run request: (preview)` line. Its `run` is `{kind:"preview"}`. The manifest is returned in `preview_v1` and never stored.

### P2-3 work_exec_v1

`work_exec_v1` is computed from the ledger alone. It is the execution-design EX-5 bullet "Execution W is a new versioned recipe `work_exec_v1`" made exact, and it cites that bullet rather than re-deciding it. The W value has exactly these 24 fields (listed in W's presentation order; canonical_v1 sorts the keys when W is encoded): `versions`, `workspace`, `spec`, `job`, `role`, `root`, `base`, `goal`, `units`, `milestones`, `constraints`, `prescriptions`, `contracts`, `dependencies`, `facets`, `pipeline_predecessors`, `questions`, `ownership`, `effective_checks`, `checks`, `toolchains`, `shape`, `start_facets`, `repair`.

- Fields other than `versions`, `base`, `start_facets` and `repair` have exactly the definitions of `render-contract.md` for W (units, milestones, constraints, prescriptions, contracts, dependencies, facets, pipeline predecessors, questions, ownership, effective checks, checks, toolchains, shape), taken from the active revision's recorded interpretation.
- `versions`: `["declarations_v1","selection_v1","dependencies_v1","exact_bytes_v1","canonical_v1","xxh3_128_v1","work_exec_v1"]`.
- `base`: always the declared form `{status:"declared",root:<source ID>,label:<base label>}`, as execution-design EX-5 fixes. Base content enters output identity, not W (E5 A).
- `start_facets`: array of `{dependency,semantic}` for start and facet dependencies consumed by selected units: `semantic` is the `xxh3_128:` text of the `publications.semantic` of the current publication of the facet's producer ("current" is defined by R17), or `{dependency,unavailable:true}` when no publication exists (always, in R15). Complete-only facet values, producer quality and outputs are excluded; units whose producer is selected are excluded as in render.
- `repair`: the role's repair revision in its structural lineage (Part 1 EX-5 Lineage) as a decimal string; `"0"` before any repair, which is always the case in R15 because R18 admits repairs. `repair` enters W: `render-contract.md` says "no repair revision or generation is fabricated. Later execution must add those through a versioned recipe", and `work_exec_v1` is that recipe for the repair revision.
- Generation is not a W field: the generation is the `generation` key of `dispatch_manifest_v1` and the `{generation}` of the authority statement, not a W field, so a takeover generation never moves W (execution-design EX-5 bullet, operator E5 A; the rule that takeover must not move W).
- Digest: `work_fp = H("work_exec_v1", W)` with `H` of `render-contract.md` (canonical_v1, XXH3-128, `xxh3_128:` text, 16 raw bytes in `dispatches.work_fp`). The W value is retained as a value record with codec ID `work_exec_v1`.
- Render emits `work_v1` unchanged. The render-contract pointer adds `work_exec_v1` and `dispatch_v1` to the interpretation identifier list and states that render never emits them.

### P2-4 Provenance (driver_provenance_v1)

`driver_provenance_v1` document (UTF-8 JSON file, Glaze strict parse, unknown or duplicate keys and trailing bytes are `schema`): `{version:1,driver:{name,version},trial:{id},view:{id},model:{requested,actual},session:{requested,actual},settings:[{name,value}]}`. Every key of the shape is required; a driver that knows nothing writes the unknown leaf.

Leaf rule: every leaf (`driver.name`, `driver.version`, `trial.id`, `view.id`, `model.requested`, `model.actual`, `session.requested`, `session.actual`, and each settings `value`) is a UTF-8 string of 1 to 1024 bytes without NUL, or exactly the object `{"status":"unknown"}` (one key `status`, value the string `unknown`). A settings `name` is a string leaf (never unknown); names are unique and normalized sorted by unsigned bytes.

Bounds (EX-8 row "Provenance file"): at most 65536 bytes (`provenance_bytes`), depth 4 (`provenance_depth`), at most 32 settings (`provenance_settings`), strings at most 1024 bytes (`provenance_string_bytes`); a violation is `resource_limit` with `details:{limit,maximum}` and the message `Execution resource limit exceeded`. The reader applies the same bounds to a stored record; a stored value that violates them is `ledger_corrupt`.

Stored record: value record, codec ID `driver_provenance_v1`, canonical_v1 value `{version:1,authority:"driver_assertion_unverified",document:<normalized document>}`; referenced by `runs.provenance_id` and the event reference key `provenance` (EX-4 table). Absence of the file stores no record: `runs.provenance_id` is NULL and every field reads as unknown; the history event expansion shows such a run with the all-unknown driver document ([P2-5](#p2-5-history-event-selector-and-request-results)).

Attachment points: `claim` and `takeover` (live in R15, option `--provenance`); `release` and `check` are specified as fields only (their rounds R16 and R17 pin the event reference key); `check-recover` and `verdict` attach none. Planar's native per-run provenance (ledger identity, revision, source digest, job, role, generation, base label, W and render digests, requested preferences `model` and `session` from the job) is derived from rows and reported by the [history event expansion](#p2-5-history-event-selector-and-request-results); it is never taken from the driver file. The token is never part of any provenance value.

#### Provenance input handling (D146)

`--provenance FILE` is an opaque native slot and an explicit single-file capture that uses the same refusals as the authoring `--diagnostic` rows (`authoring-contract.md` Messages table). The `read_provenance` service reads the file once. The request body holds only `provenance_path`, never contents, so an early replay never reads the file. Existing codes and messages only; no new message:

| Condition | Code | Message |
| --- | --- | --- |
| path missing, unresolvable, or the file cannot be opened | `io` | `Cannot resolve explicit input path` |
| the final path component is a symbolic link | `path_denied` | `Explicit input file is a symbolic link` |
| not a regular file (directory, FIFO, socket, device) | `io` | `Explicit input must be a regular file` |
| the file changed between the capture gate and the end of the read | `capture_changed` | `File changed during read` |
| a bound of EX-8 is exceeded | `resource_limit` | `Execution resource limit exceeded` with `details:{limit,maximum}` |
| JSON grammar or shape violation | `schema` | the existing message of the shared JSON parser or schema normalizer |

Path normalization refusals keep their existing messages. Check order, first failure wins: (1) the captured size exceeds 65536 bytes, `provenance_bytes`, before any parse; (2) JSON grammar (invalid JSON, duplicate keys, trailing bytes), `schema` with field `provenance`; (3) nesting depth above 4 (the root object counts 1, so a `settings[].value` object sits at depth 4), `provenance_depth`; (4) more than 32 `settings` entries, `provenance_settings`; (5) any string longer than 1024 bytes, `provenance_string_bytes`, so a 1025-byte leaf is `resource_limit` and never `schema`; (6) shape and leaf-rule violations (missing or unknown key, wrong type, empty string, NUL byte, an object other than `{"status":"unknown"}`, duplicate settings names), `schema`. The `schema` field is `provenance` alone for a grammar violation, otherwise `provenance.` followed by the keys from the document root joined by `.`, with array indexes as canonical decimals (for example `provenance.model.requested` and `provenance.settings.0.value`); a missing key names its own path, and an unknown key names the unknown key. The provenance refusals are step (9) of the refusal order in [P2-1](#p2-1-verbs-and-services), after the role-state refusals and before the commit preconditions.

### P2-5 History event selector and request results

`planar history --ledger PATH --event ID`: `--event` is a canonical positive decimal 1..9223372036854775807; it excludes `--revision`, `--request-id`, `--at-event`, `--after-event` and `--limit` (any combination is `usage`); it works on format1, format2 and format3 ledgers; an unknown event is `event_not_found`. Services: `read_history({selector:"event",id:ID,request_id:"",limit:50})`. `limit` carries the existing absent-limit value 50 that `commands.lua` passes whenever `--limit` is absent (`present.limit and options.limit or 50`), and the event selector ignores it; `id` crosses into Lua as a canonical decimal string, like `at-event`. The selector set of the history family becomes `page`, `revision`, `request`, `state`, `event`.

Format3 event summary (R15-10, D143). Format1 and format2 event summaries are unchanged. A format3 event summary, wherever it appears (`page` data, `state` data, `event` data), is the existing `{sequence,type,version,revision,request_id,actor,occurred_at,payload}` plus the four typed reference keys `job`, `role`, `generation_id` and `run_id`, being the `events` columns of the same names: `job` and `role` are IDs, and each is the empty string when the column is NULL (consistent with payload v2 `active_job`: `none` is a legal `id` per `declarations.md:33`, EX-4 does not spell an absent job `none`, because `none` is a legal job ID, and a job or role named `none` stays an ordinary ID, so an absent one is the empty string); `generation_id` and `run_id` are canonical decimal strings, and each is `"none"` when the column is NULL (R14-E2: absent database IDs use `none`; that rule covers numeric database IDs only). `version` is 1 for `ledger_initialized` and 2 for every other format3 event, and `payload` is the structured public payload, version 2 for format3 as EX-4 pins it (`{version:2,operation,before:{active_revision,active_job},after:{active_revision,active_job},reason,references}`; the `ledger_initialized` payload keeps version 1).

Format3 at-event state (R15-10, D143). `history --at-event ID` on a format3 ledger returns `{selector:"state",event,state,revision}` where `event` is the format3 event summary and `revision` is the revision metadata or `"none"`, as before. `state` is `{active_revision,active_job,activation,roles,runs}` replayed from rows with seq at most ID, without Lua (EX-1 invariant 19): `active_revision` as before; `active_job` is the job ID or the empty string; `activation` is the decimal id of the latest `job_activations` row with `event_seq` at most ID, or `"none"`; `roles` are the roles of the active job at that event, sorted by role ID, each `{role,generation,state,drifted,run}` taken from the last `transitions` row of that (job, role) with `event_seq` at most ID (`generation` is the generations ordinal as a decimal string; `state` and `drifted` are the row's after-values; `run` is `"none"` or `{id,generation}` of the run claimed or taken over at the event of the role's last transition into `running`); `runs` are the runs active at that event, sorted by id, each `{id,role,generation}` (decimal strings). Role `kind` is not part of the replay (it needs the interpretation), and no token or secret appears. Roles of other jobs are retired (EX-1 invariant 17) and omitted. Format1 and format2 `state` data stays exactly `{active_revision}`.

Response of `--event`: the existing history envelope `{version:1,kind:"history",ledger,active_revision,last_event,data}` with `data` = `{selector:"event",event,transitions,execution?}`: `event` is the event summary (the format3 extension above for format3); `transitions` is the array of the event's `transitions` rows ordered by ordinal, each `{ordinal,job,role,generation,before,after,reason}` (`generation` the ordinal as a decimal string; `before` is `none` or `{state,drifted}`; `after` is `{state,drifted}`), and is `[]` for format1 and format2 events and for events without transitions; `execution` is present only for `job_activated`, `run_claimed` and `run_taken_over` events:

- `job_activated`: `{activation:{id,job,revision}}`.
- `run_claimed`, `run_taken_over`: `{run:{id,generation,request_id},dispatch:{id,kind,work_fp,render_fp,revision},packet:{id,bytes,markdown},manifest:{id,value},work:{id,value},provenance:{native,driver}}`; `packet.markdown` is the exact stored UTF-8; `manifest.value` and `work.value` are the decoded canonical values; takeover adds `previous_run` (decimal string). `provenance` is always present for these two event types. `native` is `{ledger,revision,source,job,role,generation,base_label,work_fp,render_fp,preferences}`, derived from rows and the recorded interpretation and never from the driver file: `ledger` the 32-lowercase-hex identity, `revision` the decimal id, `source` `{id,digest}` of the revision's source record (`digest` its `xxh3_128:` text), `generation` the ordinal as a decimal string, `base_label` the label of the W `base`, `work_fp` and `render_fp` the digest texts, and `preferences` `{model,session}` the requested preferences of the job as strings, each `{"status":"unknown"}` when the job declares none. `driver` is `{id,authority,document}`: with a stored record (`runs.provenance_id` not NULL) `id` is the record id, `authority` is `driver_assertion_unverified` and `document` is the normalized `driver_provenance_v1` document; without one, `id` is `"none"`, `authority` is `"none"` and `document` is the all-unknown form `{version:1,driver:{name:U,version:U},trial:{id:U},view:{id:U},model:{requested:U,actual:U},session:{requested:U,actual:U},settings:[]}` with `U` the object `{"status":"unknown"}`, so the rule that a missing driver file records every field unknown is observable.

The expansion never contains the run token or `runs.secret`; the reader does not select the secret column. Records are decoded natively with no Lua, as `--at-event` is. The whole stdout including the final LF is at most 16777216 bytes; the existing refusal `resource_limit` with `details:{limit:"output_bytes",maximum:16777216}` and empty stdout applies and nothing is truncated. A referenced record that is missing is `ledger_corrupt`; an unsupported codec is `unsupported_interpretation`.

`history --request-id` on format3 (OP2, D126): the data keeps `{selector:"request",result}`, no `disposition`. `result` is the decoded saved result of the request: for `load` the existing result; for `activate` the `activate_v1` value of [P2-1](#p2-1-verbs-and-services); for `claim` and `takeover` the `claim_v1` value including `run.token`, with `packet` and `manifest` assembled natively from the request's `attachment_id` record and the dispatch's `manifest_id` record (the saved `result` column holds the value without `packet` and `manifest`, at most 1 MiB). Format1 and format2 responses are unchanged. The whole stdout including the final LF is at most 16777216 bytes: a format3 `claim` or `takeover` result assembles the packet (up to 1 MiB) and the manifest (up to 16 MiB), so it can exceed the ceiling, and the existing refusal `resource_limit` with `details:{limit:"output_bytes",maximum:16777216}` and empty stdout applies with no truncation. The records are decoded natively with no Lua. History pages (`--after-event`) and `--at-event` never contain the token.

### P2-6 Orchestrator live form

`planar orchestrator --ledger PATH` exists only where the orchestrator is compiled, which is the authoring bundle (D148); the production binary has no `orchestrator` verb and nothing in this section applies to it, except that the reservation of the section IDs `run` and `orchestrator_state` applies in every build, production included (D135, [P2-7](#p2-7-command-render-load-ledger-and-external-profile-pointers)), even though `orchestrator_state` is rendered only in the authoring bundle. Exactly one form is accepted. Database-free form: `--workspace`, `--registry`, `--job` together (RA-1), unchanged. Ledger form: `--ledger` alone; any other combination is `usage`. The descriptor's four options are all `required:false` and the presence rule is native. Services (ledger form): `read_execution({})`. Response kind `orchestrator_v1`, envelope kind `prompt` (`kind:"orchestrator"`), as RA-3.

On a format1 or format2 ledger the ledger form is `unsupported_ledger_version` (D142). No active job: `no_active_job`. Otherwise the context is `orchestrator_context_v1` of RA-7 with workspace, spec, job, shape, roles, graph and prescriptions taken from the active revision's recorded interpretation for the active job, plus one extra key `live` (absent in the database-free form): `live` = `{ledger,revision,last_event,activation,roles,runs,blockers}` where `roles` is the status role array of [P2-1](#p2-1-verbs-and-services), `runs` is the array of active runs sorted by role `{id,role,generation,request_id}` (decimal strings; no token, no secret), `blockers` is `[]` in R15, and `activation` is a decimal string. An additional mandatory section `orchestrator_state` follows `orchestrator_actions` in the ledger form only; it lists role states, runs, blockers and the legal next action per role state (`ready` or `stale`: `claim`; `running`: `status`, or `takeover` when the driver lost the session; `running` with drift: `status` and, from R16, `refresh`; `blocked`: wait for prerequisites; `done`: none; `retired`: none). RA-7's sentence "It states no execution state" applies to the database-free form only.

Where the orchestrator is compiled, `actions` labels: `available` exactly when the verb is in the invocation's sealed catalog. After R15 the future list is `refresh`, `checkpoint`, `release`, `recover`, `check`, `check-recover`, `verdict`; `activate`, `status`, `preview`, `claim` and `takeover` are `available` (they join the catalog at the R15 integrated gate); R16 flips `refresh`, `checkpoint`, `release`, `recover` (and `preview --since`), R17 flips `check` and `check-recover`, R18 flips `verdict`. `with-profile` is never an action. Pointer: `authoring-contract.md` RA-7 is the base definition; this contract extends it by insertion and `authoring-contract.md` is not edited by this round. `orchestrator --ledger` is an exempt read (D108).

### P2-7 Command, render, load, ledger and external-profile pointers

This section records the pointers that R15-A2 inserts into the five other approved contracts and into the [execution design](execution-design.md). Each pointer is an inserted paragraph labelled `Pointer (R15-A2):`. Each says that nothing is shipped until the R15 integrated gate, that the rows, tables and bytes beside it stay current until that gate, and that it narrows no approved clause. The constants below are the constants those pointers state.

- **Command contract** ([command-contract.md](command-contract.md)). Catalog counts at the R15 integrated gate, per build (D148): ten mandatory rows in the production build (the current five plus the five R15 rows) and sixteen in a build that compiles the authoring bundle (the ten plus the six RA01 rows). The RA01 rows and the `orchestrator` verb exist only in the authoring bundle. The ABI rows are:

  | name / family | handler_id | response_kind | capabilities |
  | --- | --- | --- | --- |
  | activate / activate | `shipped.activate.v1` | `activate_v1` | `execution_commit_v1` |
  | status / status | `shipped.status.v1` | `status_v1` | `execution_read_v1`, `draft_capture_v1` |
  | preview / preview | `shipped.preview.v1` | `preview_v1` | `execution_read_v1` |
  | claim / claim | `shipped.claim.v1` | `claim_v1` | `execution_commit_v1`, `provenance_read_v1` |
  | takeover / takeover | `shipped.takeover.v1` | `claim_v1` | `execution_commit_v1`, `provenance_read_v1` |

  Only where the orchestrator is compiled, `orchestrator` gains capability `execution_read_v1` beside `orchestrator_capture_v1`, and `orchestrator --ledger` exists only there. Option signatures: activate `--ledger!`, `--job!`, `--expect-revision!`, `--expect-job!`, `--request-id!`, `--actor!`; status `--ledger!`, `--role`, `--request-id`, `--workspace`, `--registry` (`--role` excludes `--request-id` and nothing else; `--workspace` and `--registry` both or neither); preview `--ledger!`, `--role!`; claim `--ledger!`, `--job!`, `--role!`, `--request-id!`, `--actor!`, `--provenance`; takeover `--ledger!`, `--job!`, `--role!`, `--request-id!`, `--actor!`, `--provenance` and decimal `--run-id!`; history gains decimal `--event` (excluding `--revision`, `--request-id`, `--at-event`, `--after-event` and `--limit`); orchestrator `--ledger`, `--workspace`, `--registry` and `--job`, all optional, with the native both-or-exactly-one-form rule. Services: the five families have the service lists of [P2-1](#p2-1-verbs-and-services); `orchestrator` gains `read_execution({})`; `read_history` gains the selector `event` and is called as `read_history({selector:"event",id:ID,request_id:"",limit:50})`, `limit` being the existing absent-limit value 50. The Lua boundary rule for decimal database IDs extends to `run-id` and `event`. Opaque slots gain `provenance`.

- **Root help** (D132, D148). Production build: the second line of the history block becomes the history continuation below, and the five lines follow immediately before the final `planar --help` line. No orchestrator line exists in the production build.

  ```text
    or --revision ID | --request-id ID | --at-event ID | --event ID
  planar activate --ledger PATH --job ID --expect-revision ID_OR_NONE --expect-job JOB_OR_NONE --request-id ID --actor TEXT
  planar status --ledger PATH [--role ID | --request-id ID] [--workspace FILE_OR_NAME --registry FILE]
  planar preview --ledger PATH --role ID
  planar claim --ledger PATH --job ID --role ID --request-id ID --actor TEXT [--provenance FILE]
  planar takeover --ledger PATH --job ID --role ID --run-id ID --request-id ID --actor TEXT [--provenance FILE]
  ```

  Authoring bundle: the production bytes above, with the six RA01 lines spliced immediately before `planar --help`, and the continuation `  or --ledger PATH` spliced directly after the `planar orchestrator` line. The relative order is the history block, the five R15 lines, the six RA01 lines, the continuation, then `planar --help`.

  ```text
  planar spec-prompt --workspace FILE_OR_NAME --registry FILE
  planar critique --workspace FILE_OR_NAME --registry FILE
  planar ingest --workspace FILE_OR_NAME --registry FILE
  planar explain-rejection --diagnostic FILE
  planar observe --from DIRECTORY
  planar orchestrator --workspace FILE_OR_NAME --registry FILE --job ID
    or --ledger PATH
  ```

  The `with-profile` help bytes and every other help byte are unchanged.

- **Render contract** ([render-contract.md](render-contract.md)). Execution adds packet mode `dispatch` (render's `mode:"preview"` is unchanged) and the identifiers `work_exec_v1` and `dispatch_v1`, which render never emits. `dispatch_v1` names the dispatch projection rules of [P2-2](#p2-2-dispatch-projection-and-dispatch_manifest_v1). `repair` enters W through `work_exec_v1`. The generation is the `generation` key of `dispatch_manifest_v1` and the `{generation}` of the authority statement, not a W field, so a takeover never moves W (execution-design EX-5 bullet, operator E5 A). Render stays database-free and emits `work_v1` unchanged.

- **Load command contract** ([load-command-contract.md](load-command-contract.md)). On format3 only, the services are `open_workspace`, `capture_load`, `read_execution({})` and `accept_load({bundle,semantic,execution})`; `read_execution({})` returns `{version:1,ledger,last_seq,snapshot}`; `accept_load` takes the extra key `execution`, which is `CHANGE` of `operation:"load"`. The capability stays `accept_load_v1`. Format1 and format2 keep three services and `{bundle,semantic}`. The production help lines are those of the root help above.

- **Ledger contract** ([ledger-contract.md](ledger-contract.md)). The command-surface row `planar history --ledger <path> --event <id>` returns the event with its transitions and, for execution events, the expanded records; the data selector `event` joins `page`, `revision`, `request` and `state`; the format3 event summary and the format3 `--at-event` state are those of [P2-5](#p2-5-history-event-selector-and-request-results). Format1 and format2 text is unchanged.

- **External profile contract** ([external-profile-contract.md](external-profile-contract.md)). The with-profile family list is ten: the five families of X08 plus `activate`, `status`, `preview`, `claim`, `takeover`. Their known slots are `ledger` and, per verb, `workspace`, `registry` and `provenance`. The six RA01 verbs stay embedded-only. The reserved section IDs are `run` and `orchestrator_state`, reserved in every build including production (D135). Default root help follows the command-contract pointer. The consequences are stated as valid today and refused after R15: the five command and family names, the five handler IDs and the two section IDs leave the external namespace; external command capacity falls from 59 to 54; the `history` probe signature gains `--event ID`; the with-profile completeness rule moves from five to ten. The message tables and the X-LIMIT table are unedited.

- **Execution design** ([execution-design.md](execution-design.md)). Only the F2 bullet is reworded (D139).

## Part 1 amendments (R15-A2)

Each amendment changes Part 1 text, or a Messages cell, that Part 2 requires. Locations are line numbers of this contract as they stood before R15-A2. The SQL block, the M and N tables, the EX-7 and EX-8 tables, and the decisions rows D7 to D103 and OP1 to OP3 are unchanged by R15-A2. Every amendment is listed here with its location, its change and its reason.

| ID | Location (lines before R15-A2) | Change | Reason |
| --- | --- | --- | --- |
| A-1 | Title (line 1) and Status paragraph (lines 3 to 9) | The title becomes `# Execution contract v1 (parts 1 and 2)`; the Status paragraph says that Part 2 is filled by R15-A2 and points to this section | the file now holds both parts |
| A-2 | Lines 518 and 527 to 528 (EX-2, the `status` verb and the `history --request-id` response shape) | The forward references to Part 2 are re-pointed: `status` to [P2-1](#p2-1-verbs-and-services), and the further response shape of `history --request-id` to [P2-5](#p2-5-history-event-selector-and-request-results) | the placeholders are replaced by P2-1 and P2-5 |
| A-3 | Line 540 (Run token, R15-9) | "in any packet other than the claim or takeover packet" becomes "in any packet, in logs or in error details. The claim and takeover results carry it only in the field `run.token`" | packets are retained records that `history --event` expands, so no packet carries the token (F-B); the claim and takeover results carry it in `run.token` only (D118) |
| A-4 | Lines 594 and 646 (EX-3, precondition sets of the other operations) | "defined by R15-A2 in Part 2" and "belong to R15-A2" are re-pointed to [P2-1](#p2-1-verbs-and-services), which defines the sets of `claim` and `takeover`; the sets of `refresh`, `checkpoint`, `release`, the two check commits and `verdict` are defined by the R16 to R18 contract rounds | the placeholders are replaced; the other operations are reserved and have no request body, facts or response in this part, so pinning their members now would precede their lifecycle and event shapes (D150) |
| A-5 | Line 702 (EX-4, explain) | "Its shape is in the Part 2 placeholders" becomes a link to [P2-1](#p2-1-verbs-and-services) (`role_detail.explain`) | the placeholders are replaced |
| A-6 | Line 724 (EX-5, `work_exec_v1`) | The forward reference to the placeholder becomes a link to [P2-3](#p2-3-work_exec_v1), and the sentence that the recipe is not written in this part is removed | the placeholders are replaced |
| A-7 | Line 730 (EX-6, exempt reads) | "Which verbs are exempt reads is a Part 2 clause" becomes a clause of [P2-1](#p2-1-verbs-and-services) (D108) | the placeholders are replaced |
| A-8 | Messages, `revision_conflict` row, "Execution sites" cell | "The format3 load compare-and-swap only" becomes "The format3 load compare-and-swap and the `--expect-revision` check of `activate`" | R15-A1 I66 scoped this cell to the load compare-and-swap because `activate` was not yet specified; P2-1 now specifies it and reuses the code and message (D111) |
| A-9 | Messages, the two `reference` rows, "Execution sites" cells | the cells name the field values `selection.job` and `selection.role` | P2-1 reuses these existing sites at the execution verbs (D110); the messages are unchanged |
| A-10 | Messages, `schema` row, "Execution sites" cell | the cell names the provenance file field `provenance` | P2-4 adds the provenance file as a `schema` site (D110) |
| A-11 | Pointer amendments, item 7 and the lead sentence (base line 966) | the former item 7 is replaced by an item stating that the command, render, load, ledger and external-profile pointers are made by R15-A2 as inserted paragraphs recorded in P2-7. The lead sentence of this section changes from "The amendments are insertions only" to "The amendments are insertions only, except the reworded F2 bullet of item 8". | the pointers are made by this round |
| A-12 | Traceability and Pointer amendments rows that linked the placeholder heading | re-pointed to the P2 anchors | the placeholder heading no longer exists |
| A-13 | Messages, `state_conflict` row of the execution refusal sites, "Refusal site" cell "Any other failed precondition at commit" | the cell becomes "Any other failed precondition at commit; also, at the snapshot before the lock, an `activate` `--expect-job NONE` mismatch (a job is active) and a `takeover` `--run-id` that names no run of the job and role" | P2-1 uses `state_conflict` at these two sites before the lock (D111, D114), and neither the frozen EX-7 meaning of `state_conflict` nor the original cell covers them; an `activate` `--expect-job JOB` mismatch is `no_active_job`, which EX-7 fits exactly, so it is not a site; EX-7 stays byte-identical and no code or message is added. Limit: a same-ID retry cannot succeed for either site, because the refusal stays deterministic until the active job changes and, for an unknown run id, never changes (D149) |

## Messages

This section maps each execution refusal site to an existing message or to one of the nineteen messages of [EX-7](#ex-7-error-codes-and-messages). A site uses an existing message by name and never respells it. No execution site introduces a twentieth message.

**Execution refusal sites.**

| Refusal site | Code | Message | Defined by |
| --- | --- | --- | --- |
| A run verb with no active job | `no_active_job` | `No active job matches the request` | [EX-7](#ex-7-error-codes-and-messages) |
| Claim of a role that is blocked, running, done or retired, or a lost concurrent claim of one role | `claim_conflict` | `Role state does not allow this operation` | [EX-3](#ex-3-execution-commit-protocol), [EX-7](#ex-7-error-codes-and-messages) |
| Any other failed precondition at commit; also, at the snapshot before the lock, an `activate` `--expect-job NONE` mismatch (a job is active) and a `takeover` `--run-id` that names no run of the job and role | `state_conflict` | `Execution state changed since the request was prepared` | [EX-3](#ex-3-execution-commit-protocol) |
| Token with a wrong secret, or a run that is not current | `fenced` | `Run is not current` | [Run token (R15-9)](#run-token-r15-9), [EX-7](#ex-7-error-codes-and-messages) |
| Stdlib digest differs from the active revision | `interpretation_mismatch` | `Interpretation does not match the active revision` | [EX-6](#ex-6-interpretation-binding) |
| Requested render digest never dispatched for the role (R16) | `history_unavailable` | `History needed for this request is unavailable` | [EX-7](#ex-7-error-codes-and-messages) |
| Acknowledged W not current (R16) | `drifted` | `Role work changed since it was acknowledged` | [EX-7](#ex-7-error-codes-and-messages) |
| Open reservation exists (R17) | `check_in_progress` | `A check is already in progress for this role` | [EX-7](#ex-7-error-codes-and-messages) |
| External approval assertion attempted (R18) | `unsupported_policy` | `Execution policy is not supported` | [EX-7](#ex-7-error-codes-and-messages) |
| New bound exhausted | `resource_limit` | `Execution resource limit exceeded` | [EX-8](#ex-8-execution-bounds) |
| Existing bound at an existing site | the existing code | the existing message | Reused existing codes, below |
| Format1 or format2 ledger | `unsupported_ledger_version` | `Ledger version or state is unsupported` | Reused existing codes, below |
| Uncertain commit, or stdout failure after commit | `io` | the operation's post-commit fallback | [EX-7](#ex-7-error-codes-and-messages) |

**Reused existing codes.** Each row gives one existing code with its existing message, byte-exact, its owner and the execution sites that use it. None of these strings is in the nineteen-message set of EX-7.

| Code | Existing message (byte-exact) | Owner | Execution sites |
| --- | --- | --- | --- |
| `unsupported_ledger_version` | `Ledger version or state is unsupported` | existing mapping, `src/cmd/planar/response.cpp:218` (ledger-contract.md lists the code) | Format1 or format2 refusal by each execution verb before capture or Lua ([EX-1](#ex-1-format3), cutover) |
| `request_conflict` | `Ledger request conflicts with its saved intent` | `load-command-contract.md:264`, row `reused request ID / unequal body` | The same request ID with an unequal body or a different request version ([EX-2](#ex-2-request-framing-and-replay)) |
| `revision_conflict` | `Ledger revision does not match the expected revision` | `load-command-contract.md:261`, row `locked head mismatch`, with details `{expected,actual}` | The format3 load compare-and-swap and the `--expect-revision` check of `activate` |
| `reference` | `Unknown selected job` | `authoring-contract.md:506`, row `Orchestrator job not listed`; the same string at `src/stdlib/planar/selection.lua:95` | An unknown job at an execution verb; field `selection.job` |
| `reference` | `Unknown selected role` | existing mapping, `src/stdlib/planar/selection.lua:97` | An unknown role at an execution verb; field `selection.role` |
| `schema` | The existing message of the shared JSON parser or schema normalizer for the failing bytes; no single string | `authoring-contract.md:505`, row `Diagnostic file grammar or envelope violation, observation file validation, ...` | Malformed verdict or observation input, and malformed `--provenance` input (field `provenance`) |
| `observation_target_mismatch` | `Wrong output vector` | existing mapping, `src/stdlib/planar/observations.lua:51` (render observation validator) | A wrong output vector in a check view (R17-2, step 2) |
| `resource_limit` | `Capture limit exceeded` | `authoring-contract.md:504`, row `Bounds`, for the existing observe bounds only | The `observation_*` bounds of EX-8; new bounds use `Execution resource limit exceeded` |
| `capture_changed` | `Captured declaration inputs changed` | `load-command-contract.md:265`, row `selected receipt changed`, for load capture | Load capture; for observe, the two messages `Directory membership or identity changed` and `File changed during read` at `authoring-contract.md:502-503` |
| `ledger_busy` | `Ledger is busy` | existing mapping, `src/cmd/planar/response.cpp:220` | A busy ledger at any execution commit |
| `storage_full` | `Ledger storage is full` | existing mapping, `src/cmd/planar/response.cpp:221` | A full disk at any execution commit |

The owner of each message that names a contract row is that row's text, and the owner of each existing mapping is the named source file. No row respells a message.

## Pointer amendments

Each amendment is recorded in the pointed document's own text. The amendments are insertions only, except the reworded F2 bullet of item 8, and no approved clause is narrowed.

1. [CLAUDE.md](../../CLAUDE.md): one inserted authority entry, `[execution](docs/vernacular/execution-contract.md),`, after the authoring entry of authority item 2 (E14 A). No other line changes, and `AGENTS.md` remains a symlink.
2. [ledger-contract](ledger-contract.md): three inserted pointer paragraphs: the format3 event types and payload version 2, which point to [EX-4](#ex-4-facts-derived-state-and-reconcile-rules), after the event vocabulary paragraph; the format3 request-lookup note, which points to [EX-2](#ex-2-request-framing-and-replay); and the format3 pointer to [EX-1](#ex-1-format3) in the format evolution section. Format1 and format2 text is unchanged.
3. [external-profile-contract](external-profile-contract.md): one inserted format3 paragraph in the format table region, pointing to [EX-1](#ex-1-format3) and [EX-2](#ex-2-request-framing-and-replay). It supersedes by insertion the sentence "New init creates format2 only", which is not edited (D82). It states that load on format3 keeps `request_v2`, including the profile and `effective_stdlib_v1` path. The message tables are untouched.
4. [acceptance-matrix](acceptance-matrix.md): the seven rows A09, A10, A20, A22, A23, A26 and A33 each gain an appended sentence that links the exact section of this contract. Their status words are byte-unchanged, and no other row changes.
5. [v5-alignment](v5-alignment.md): two inserted recorded entries, each citing this contract and the operator answer of 2026-10-28. The first is deviation E2 from `planar-design-spec-v5.md` lines 110 and 497 (operator answer A). The second is narrowing E5 of line 241 (operator answer A).
6. [implementation](implementation.md): one inserted bullet pointing to this contract. No paragraph is rewrapped.
7. Pointers made by R15-A2 (A-11). The command, render, load, ledger and external-profile pointers are made by R15-A2 as inserted paragraphs labelled `Pointer (R15-A2)` in [command-contract](command-contract.md), [render-contract](render-contract.md), [load-command-contract](load-command-contract.md), [ledger-contract](ledger-contract.md) and [external-profile-contract](external-profile-contract.md). Their content is recorded in P2-7, which links them: [P2-7](#p2-7-command-render-load-ledger-and-external-profile-pointers). Nothing is shipped by them until the R15 integrated gate.
8. [execution-design](execution-design.md): the F2 bullet (the capture-receipt claim at line 527) is reworded (D139) to say that the RA01 authoring and observe captures are covered by the named tests, and that the receipt-reuse claim is limited to what the render and load capture paths exercise. No other line of that file changes.
9. [authoring-contract](authoring-contract.md), RA-1 by reference (D144): the orchestrator options `--workspace`, `--registry` and `--job` become optional, with the native rule that exactly one form is accepted (the database-free options together, or `--ledger` alone) and the ledger form of [P2-6](#p2-6-orchestrator-live-form). Authority: RA-1 says "R15 later adds a ledger form by its own contract amendment".
10. [authoring-contract](authoring-contract.md), RA-7 by reference (D144): the section list gains `orchestrator_state` ([P2-6](#p2-6-orchestrator-live-form)), and the sentence "It states no execution state" applies to the database-free form only.
11. [authoring-contract](authoring-contract.md), RA-9 items 1, 4 and 6 by reference (D144), each valid until the R15 integrated gate: item 1 (the embedded probe holds only the five X08 families) reads ten families; item 4 (the with-profile catalog is five shipped rows, its completeness rule stays five and the embedded rule is eleven) reads ten, ten and sixteen (authoring bundle); item 6 (the with-profile catalog holds five shipped commands, so external capacity is unchanged) reads capacity 54.
12. [authoring-contract](authoring-contract.md), the closing paragraph of RA-9 and design RA-9 at `execution-design.md:499` (D134 option A, delegated 2026-10-31): the sentence "R15 widens the closed factory to admit these verbs under `with-profile` only by its own reviewed amendment" is superseded. R15 does not admit the six RA01 verbs under `with-profile`. A later round, provisionally named RA02 (the roadmap assigns the real ID after the R15 integrated gate), admits them under `with-profile` by its own reviewed amendment, decides which reads are exempt, and carries the shape-catalog narrowing-risk row (`remaining-steps-design.md:493-494`) in its invariant table. `orchestrator --ledger` is an exempt embedded read.
13. `authoring-contract.md`, `acceptance-matrix.md` and `v5-alignment.md` are not edited by this round. The by-reference amendments of items 9 to 12 are recorded here instead.

## Traceability

| Item | Contract section |
| --- | --- |
| EX-1 Format3 | [EX-1](#ex-1-format3) |
| EX-2 Request framing and replay | [EX-2](#ex-2-request-framing-and-replay) |
| EX-3 Execution commit protocol | [EX-3](#ex-3-execution-commit-protocol) |
| EX-4 Facts, derived state and reconcile rules | [EX-4](#ex-4-facts-derived-state-and-reconcile-rules) |
| EX-5 Role structure, lineage and work_exec_v1 | [EX-5](#ex-5-role-structure-lineage-and-work_exec_v1) |
| EX-6 Interpretation binding | [EX-6](#ex-6-interpretation-binding) |
| EX-7 Error codes and messages | [EX-7](#ex-7-error-codes-and-messages) |
| EX-8 Execution bounds | [EX-8](#ex-8-execution-bounds) |
| R15-1 Format3 and request_v3; init cutover | [EX-1](#ex-1-format3), [EX-2](#ex-2-request-framing-and-replay), [Common rules](#common-rules) |
| R15-2 Active job and activate | [EX-4](#ex-4-facts-derived-state-and-reconcile-rules) (activation and re-activation), [P2-1](#p2-1-verbs-and-services) (`activate`, `--expect-job`, D104, D111) |
| R15-3 Format3 load | [EX-4](#ex-4-facts-derived-state-and-reconcile-rules) (load reconciliation and absent active job), [EX-3](#ex-3-execution-commit-protocol), [P2-1](#p2-1-verbs-and-services) (format3 services, D136), [P2-7](#p2-7-command-render-load-ledger-and-external-profile-pointers) |
| R15-9 Run tokens | [Run token (R15-9)](#run-token-r15-9), [P2-1](#p2-1-verbs-and-services) (token exposure, D118), [Part 1 amendments (R15-A2)](#part-1-amendments-r15-a2) (A-3) |
| E1 One command per verb; reads separate | [Decisions table](#decisions-table) (OP1, OP2), [P2-1](#p2-1-verbs-and-services) (D105, D106) |
| E2 Active job | [Pointer amendments](#pointer-amendments), [EX-4](#ex-4-facts-derived-state-and-reconcile-rules), [P2-1](#p2-1-verbs-and-services) (D104) |
| E3 Exact stdlib and profile binding | [EX-6](#ex-6-interpretation-binding), [P2-1](#p2-1-verbs-and-services) (exempt reads, D108, D142), [P2-6](#p2-6-orchestrator-live-form) |
| E5 Ledger-only work fingerprint | [EX-5](#ex-5-role-structure-lineage-and-work_exec_v1), [Pointer amendments](#pointer-amendments), [P2-3](#p2-3-work_exec_v1) (D120) |
| E11 Run token secret | [Run token (R15-9)](#run-token-r15-9), [P2-1](#p2-1-verbs-and-services) (D118) |
| E14 New contract and authority list | [Pointer amendments](#pointer-amendments) |
| E16 New codes and execution message | [EX-7](#ex-7-error-codes-and-messages), [P2-1](#p2-1-verbs-and-services) (D114) |
| A09 One claim, lost response, request conflict | [Acceptance checklist](#acceptance-checklist), [EX-2](#ex-2-request-framing-and-replay), [EX-3](#ex-3-execution-commit-protocol) |
| A10 Takeover fences old mutations | [Acceptance checklist](#acceptance-checklist), [State machines](#state-machines) |
| A20 Drafts and invalid draft | [Acceptance checklist](#acceptance-checklist), [EX-3](#ex-3-execution-commit-protocol) |
| A22 Removed or recreated role | [EX-5](#ex-5-role-structure-lineage-and-work_exec_v1), [State machines](#state-machines) |
| A23 Format and storage failure, dispatch | [EX-1](#ex-1-format3), [EX-2](#ex-2-request-framing-and-replay) |
| A26 Reconciliation atomicity and races | [EX-3](#ex-3-execution-commit-protocol), [EX-4](#ex-4-facts-derived-state-and-reconcile-rules) |
| A33 Regrouping fences generations | [EX-5](#ex-5-role-structure-lineage-and-work_exec_v1), [State machines](#state-machines) |
| v5 line 110 (active jobs in workspace declarations) | [Pointer amendments](#pointer-amendments) (deviation E2) |
| v5 line 241 (applicable contracts and identities) | [Pointer amendments](#pointer-amendments) (narrowing E5), [EX-5](#ex-5-role-structure-lineage-and-work_exec_v1) |
| v5 lines 364-387 (state table and role states) | [EX-4](#ex-4-facts-derived-state-and-reconcile-rules), [State machines](#state-machines) |
| v5 line 497 (revision change and active runs) | [EX-4](#ex-4-facts-derived-state-and-reconcile-rules), [Pointer amendments](#pointer-amendments) |
| v5 line 507 (stdlib participates in interpretation) | [EX-6](#ex-6-interpretation-binding) |
| v5 lines 561-565 (engine transaction invariants) | [EX-1](#ex-1-format3) (cross-row invariants), [EX-3](#ex-3-execution-commit-protocol) |
| R15-4 Claim | [P2-1](#p2-1-verbs-and-services) (D107, D114; the `claim_v1` response), [Acceptance checklist](#acceptance-checklist) (the `claim` retry bytes, A23) |
| R15-5 Takeover | [P2-1](#p2-1-verbs-and-services) (D107, D114) |
| R15-6 Dispatch projection and dispatch_manifest_v1 | [P2-2](#p2-2-dispatch-projection-and-dispatch_manifest_v1), [P2-3](#p2-3-work_exec_v1) |
| R15-7 Status shape and drafts | [P2-1](#p2-1-verbs-and-services) (D115, D116) |
| R15-8 Driver provenance input | [P2-4](#p2-4-provenance-driver_provenance_v1) (D121, D122, D123, D146) |
| R15-10 Format3 history: event summary, at-event state and event selector | [P2-5](#p2-5-history-event-selector-and-request-results) (format3 event summary with payload v2 and typed reference columns, format3 `--at-event` state, `--event` selector; D124 to D127, D143) |
| R15-11 Orchestrator ledger form | [P2-6](#p2-6-orchestrator-live-form) (D128, D129, D130, D148) |
| A29 History `--event` selector | [P2-5](#p2-5-history-event-selector-and-request-results) |
| A34 Provenance unknown rule | [P2-4](#p2-4-provenance-driver_provenance_v1) |
| E6 Native and driver provenance | [P2-4](#p2-4-provenance-driver_provenance_v1), [P2-5](#p2-5-history-event-selector-and-request-results) |
| Part 1 amendments (R15-A2) | [Part 1 amendments (R15-A2)](#part-1-amendments-r15-a2) |

## Acceptance checklist

Each item below is checkable from this text, without a design choice.

- A09: two processes claim one role with a pause variant that forces both past the snapshot. Exactly one `claim` response is returned and the other is `claim_conflict` ([EX-3](#ex-3-execution-commit-protocol), [EX-7](#ex-7-error-codes-and-messages)). A lost response, recovered by request lookup (`status --request-id` or `history --request-id`), returns the run token; a retry returns identical bytes; and the same ID with a changed role conflicts ([EX-2](#ex-2-request-framing-and-replay)).
- A10: after a takeover, the old token on a later verb refuses `fenced` with no effect ([Run token (R15-9)](#run-token-r15-9), [EX-7](#ex-7-error-codes-and-messages), row N3).
- A20: an invalid draft in the workspace is reported invalid, while the active revision and the running roles are unchanged. A failed load activates nothing ([EX-3](#ex-3-execution-commit-protocol), step 6).
- A22 and A33: a regroup by load and by `activate` retires the old generations and fences the old tokens. A re-added role name gets a new generation and no inherited token ([EX-5](#ex-5-role-structure-lineage-and-work_exec_v1), rows M9 to M16).
- A26: a load during an active run with a structural change leaves revision, transitions and fencing all absent before the commit and all present after it. A concurrent claim between snapshot and commit makes the load refuse `state_conflict`, and a same-ID retry succeeds ([EX-3](#ex-3-execution-commit-protocol), "Format3 load and activate preconditions").
- A23: the dispatched packet is reconstructed from SQLite after a restart and is byte-equal to the claim response. A disk-full fault during a claim leaves no run ([EX-2](#ex-2-request-framing-and-replay), packet responses; [EX-3](#ex-3-execution-commit-protocol), step 6).
- A02, A31 and A32 are preserved: `work_exec_v1` moves only for applicable inputs ([EX-5](#ex-5-role-structure-lineage-and-work_exec_v1)), and the existing refusals of invalid assignments and cycles are unchanged.
- Lost-update guard: two claims of different roles forced to overlap with the pause variant both succeed, and both roles end `running` in `role_state` and in the replayed transitions. A permissive mutant that writes Lua's complete vector must be killed by this test ([EX-3](#ex-3-execution-commit-protocol), "Delta write").
- EX-6 binding: a variant binary with different embedded Lua refuses `claim` with `interpretation_mismatch` and no effect, and a same-ID retry of a request committed by the original binary still replays its saved bytes ([EX-6](#ex-6-interpretation-binding), [EX-2](#ex-2-request-framing-and-replay)).
- The status explain separation (A20/A22): a `status --role` read reports `role_detail.explain` with `work_or_semantic` (the changed W fields), `output_vector` (always `[]` in R15) and `prerequisite` (the prerequisites not `available`) as three separate arrays, so that no cause is reported in another cause's array ([P2-1](#p2-1-verbs-and-services)).
- The `claim` retry bytes (A23): a same-ID retry of a claim returns the saved bytes, and the packet and manifest of the saved result equal the packet and manifest of the first claim response byte for byte ([EX-2](#ex-2-request-framing-and-replay), [P2-1](#p2-1-verbs-and-services), [P2-2](#p2-2-dispatch-projection-and-dispatch_manifest_v1)).
- The history `--event` (A29): `history --event ID` returns the event summary, its transitions and, for `job_activated`, `run_claimed` and `run_taken_over`, the expansion of [P2-5](#p2-5-history-event-selector-and-request-results) on every ledger format, and an unknown event is `event_not_found`.
- The provenance unknown rule (A34): an absent driver file stores no record and every field reads unknown, and the event expansion of such a run shows the all-unknown driver document ([P2-4](#p2-4-provenance-driver_provenance_v1)).
- F3 (render-level insertion-order byte test) is an obligation of R15-B to R15-E (D140), carried from the RA01-C coverage gaps as a test-only item.
- F4 (mandatory-file identity test) is an obligation of R15-B to R15-E (D140), carried from the RA01-C coverage gaps as a test-only item.
- F5 (change between listing and first read) is an obligation of R15-B to R15-E (D140), carried from the RA01-C coverage gaps as a test-only item.
- F6 (probe set stays five) is an obligation of R15-B to R15-E (D140), carried from the RA01-C coverage gaps as a test-only item.
- F7 (envelope comparisons to literals) is an obligation of R15-B to R15-E (D140), carried from the RA01-C coverage gaps as a test-only item.
- F8 (no-ledger snapshot beyond the scratch root) is an obligation of R15-B to R15-E (D140), carried from the RA01-C coverage gaps as a test-only item.

## Decisions table

The tag set is closed: `contract-decided`, `operator-decided` and `new-engineering`. OP1, OP2, OP3, D54, D67 and D68 are operator-decided under the standing delegation of 2026-10-31, option A (`operator-decided (delegated 2026-10-31, option A)`), recorded as delegated, not as live answers; the operator may overrule them.

| ID | Choice | Tag | Basis |
| --- | --- | --- | --- |
| D7 | Render stays database-free; no claim, refresh or since options on render | operator-decided | Q14 A |
| D10 | Public init creates format3 only after R15-B and R15-C, with the same signature and bytes and no selector | operator-decided | Q13 A; XD5 |
| D11 | Execution verbs refuse format1 and format2 with `unsupported_ledger_version` before capture or Lua | operator-decided | Q13 A |
| D12 | An older binary refuses a format3 ledger unchanged | contract-decided | ledger-contract.md, format evolution |
| D13 | Column types, nullability, indexes and key choices not in the design EX-1 table are stated once in the SQL | new-engineering | execution design EX-1 table |
| D15 | `events`: a fourteen-value type enum, version 1 or 2, nullable typed columns, no REFERENCES on `generation_id` and `run_id` | new-engineering | EX-1 |
| D17 | `current_state` gains `active_job` and `active_activation_id`, with no REFERENCES on the activation column | new-engineering | EX-1 |
| D19 | NULL-first transition: `before_state` and `before_drifted` are NULL together; the first-ever check reads "no row exists"; a sentinel `none` state is rejected | new-engineering | EX-1 invariant 16; EX-3 |
| D23 | Replay: an equal body returns the saved bytes; an unequal body under the same ID is `request_conflict`; failed operations reserve nothing | contract-decided | v5 `planar-design-spec-v5.md:368-370`; ledger-contract.md |
| D34 | Per-verb precondition sets are fixed only for format3 load and `activate` (D83); the others are Part 2 | new-engineering | EX-3; Part 2 placeholders |
| D37 | One reason per cause, in the rows of the State machines section | new-engineering | EX-4; State machines |
| D43 | Payload v2 with `after` shaped like `before`; `active_job` is the empty string, never `none`; the event reason comes from ordinal 1, the lowest ordinal | new-engineering | EX-4 event payload; D101 |
| D44 | Sixteen closed reasons, split by reachability by round | new-engineering | EX-4 transition reasons |
| D49 | The narrowing of v5 line 241 by E5 is recorded by pointer | operator-decided | E5 A |
| D50 | Binding by the stdlib digest, after early replay | operator-decided | E3 A |
| D51 | Which verbs are exempt reads is Part 2; the rule and the class are in EX-6 | new-engineering | EX-6 |
| D54 | Each new code and each execution operation gets one fixed message or fallback; no open text | operator-decided (delegated 2026-10-31, option A) | E16 A |
| D55 | Exit mapping: usage 2; semantic, conflict, resource and format 3; I/O, corruption, busy and disk-full 4 | contract-decided | ledger-contract.md:480-481 |
| D56 | EX-8 has ten rows, checked before allocation; the numbers are new engineering except the E15 row | new-engineering | EX-8; the E15 row is D78 |
| D57 | Role state machine: rows M1 to M41, one reason per row, every carrying event listed | new-engineering | State machines |
| D58 | Run states and rows; `interrupted` only from release; check-recover changes no `run_state` | new-engineering | State machines, rows N1 to N11 |
| D60 | `run_state` is delta-written under the locked before-state check | new-engineering | EX-3, delta write |
| D67 | Fixed messages for the nine codes and the resource message | operator-decided (delegated 2026-10-31, option A) | E16 A |
| D68 | Post-commit fallbacks: nine operations, two variants, one message each | operator-decided (delegated 2026-10-31, option A) | E16 A; load-command-contract.md |
| D69 | Explicit `CREATE TRIGGER` statements in the format2 form | new-engineering | ledger-contract.md, trigger paragraph |
| D70 | REFERENCES by the arrow rule of the design EX-1 table; reader-verified columns carry none | new-engineering | EX-1, reference rule |
| D71 | `requests.outcome` is the nine-value union with per-operation pairing and no `reserved` value | new-engineering | EX-1, request outcomes |
| D72 | Cause-to-reason map, the authoritative rows of the State machines section | new-engineering | State machines |
| D73 | Payload version 2 for every format3 event except `ledger_initialized`, which is version 1 | new-engineering | EX-4 event payload |
| D74 | `run_state` is reader-verified as derived; no run-transition table exists in SQL | new-engineering | EX-1 invariant 15 |
| D75 | `history --request-id` on format3 keeps the existing record shape and adds no disposition field | new-engineering | EX-2; OP2 |
| D78 | The E15 bounds row: 1 GiB hashed bytes, 100,000 files, depth 64 | operator-decided | E15 A |
| D80 | Part 2 placeholders: seven `###` subsections with clause lists | new-engineering | Part 2 placeholders |
| D82 | The external-profile format3 row supersedes by insertion; load on format3 keeps `request_v2` | new-engineering | Pointer amendments, item 3 |
| D83 | Load and activate preconditions: request, revision, active job, and `fact_version` and `role_state` for every role | new-engineering | EX-3 |
| D85 | Exactly two explicit indexes: `records_digest` and `run_state_one_active` | new-engineering | EX-1 |
| D86 | EX-8 limit names, and the scope of the resource message | new-engineering | EX-8 |
| D87 | Payload v2 reference keys and typed-column population per event type | new-engineering | EX-4, event payload table |
| D88 | Error `details` and `field` for the nine new codes | new-engineering | EX-7 |
| D89 | Reader-verified invariants added to the nine design invariants | new-engineering | EX-1, cross-row invariants |
| D90 | Carrying events of derived transitions | new-engineering | EX-4, carrying events |
| D91 | `stale` to `done` rows M40 and M41 | new-engineering | State machines; OP3 |
| D92 | `fact_version` per bumping fact; the first row of a (job, role) is version 1 | new-engineering | EX-3, fact versions |
| D93 | Column conventions C1 to C7 and the projection column sets | new-engineering | EX-1, column conventions |
| D94 | Absent active job: rows M13 and M14; non-active-job rows are retired | new-engineering | EX-4, absent active job |
| D95 | Reused existing messages are named, never respelled | new-engineering | Messages |
| D96 | Exact v5 citations for the rows at lines 374 to 385 | new-engineering | EX-4; State machines |
| D97 | `role_state` couples `run_id` and `drifted` to `state` by one table-level CHECK: a running role has a non-NULL `run_id`, every other state has `run_id` NULL and `drifted` 0 | new-engineering | EX-1 invariant 12; EX-4 States |
| D98 | `transitions` has two table-level CHECKs: `after_drifted` and `before_drifted` are 0 unless the matching state is `running` | new-engineering | D97; EX-3 before-state check |
| D99 | Replay derives `role_state.run_id` from `events.run_id` of a `run_claimed` or `run_taken_over` event only for the role that event's `events.role` names, the operation's target; every other running row takes the run_id derived for the role's previous `transitions` row; the result is cross-checked against the active `run_state` row | new-engineering | EX-1 invariants 6 and 12 |
| D100 | `evidence.reason` stays `TEXT NOT NULL` with no CHECK; it is native-writer enforced and reader-verified only, with no SQL CHECK; R17-3 lists no `reason` value for `pass`, which R17 states, so R17 changes no stored format3 SQL | new-engineering | Execution design EX-1 table (closed enum, R17); R17-3 lists no `reason` value for `pass` |
| D101 | The event-level reason is the reason of the `transitions` row with ordinal 1, the lowest ordinal, or the empty string when the event has no transition; the ordinals of one event are contiguous from 1 | new-engineering | EX-4 event payload; C5 |
| D102 | `status --request-id` of a claim or takeover request returns the saved result and so shows the run token; the token is never shown in `status` other than its request lookup | operator-decided | E11 A; R15-9; R15-7 |
| D103 | OP1 is a required scalar option of the separate-token grammar, so the external-profile rule that flags cannot be required is unchanged | contract-decided | external-profile-contract.md:266-267 |
| OP1 | `--confirm-stopped` becomes a required scalar option whose only accepted value is `true`; any other value is `usage`. A required scalar option is an ordinary option of the separate-token grammar, so the external-profile rule that flags cannot be required (`external-profile-contract.md:266-267`) is unchanged. No other verb grammar is decided in part 1 | operator-decided (delegated 2026-10-31, option A) | E1 A |
| OP2 | `history --request-id` on format3 also returns execution results (tokens for claim and takeover); format1 and format2 are unchanged. The shape is the D75 shape, with no disposition field; packet assembly and the per-operation result schema are deferred to the Part 2 `history --event` placeholder | operator-decided (delegated 2026-10-31, option A) | round record choice 2; E1 A; E11 A |
| OP3 | Row M41 (stale to done when an output vector is restored) has no reason of the sixteen that fits. Options: (A) `evidence_inapplicable` names the applicability fact class and is used for both directions of output-vector applicability; the set stays sixteen and no SQL enum changes. (B) Add a seventeenth reason `evidence_applicable`, which amends the SQL enum, the reason list and every row that uses it. (C) Argue M41 unreachable: rejected, because R17-6 states that restoration is reachable and A12 restores `done` without rerun. Option A is applied. M40 is not part of OP3 | operator-decided (delegated 2026-10-31, option A) | R17-6; A12; M41 |
| D104 | `--expect-job` spelling: the uppercase value `NONE` means "no active job"; any other value is a job ID. `none` stays an ordinary job ID. The help metavariable is `JOB_OR_NONE` (root help `--expect-job JOB_OR_NONE`), not `ID_OR_NONE`, so that it does not share the metavariable of `--expect-revision`, whose spelling is the lowercase `none`; the spelling sentence stays in P2-1. Rejected: `none` (collides with the legal ID), a flag (flags cannot be required, external-profile-contract.md:266-267), empty string (scalars are nonempty). | operator-decided (delegated 2026-10-31, option A) | NEW-delegated; E2 A; design R15-2 and verb table (execution-design.md:393) only say "`none` or ID" |
| D105 | Option maps of the five verbs (section 4.1 below); all scalars separate tokens, nonempty, not starting `--`; `--run-id` is a canonical positive decimal 1..9223372036854775807; `--actor` and `--request-id` use the load grammar. | operator-decided | design-decided (E1 A; verb table execution-design.md:393-405) |
| D106 | `status`: `--role` excludes `--request-id` and nothing else; `--request-id` may combine with `--workspace` and `--registry` (a request lookup together with a draft report is legal, as `status_v1` already carries `drafts?` and `request?` together; the status services row allows `read_request` and the draft services together); `--workspace` and `--registry` are both-or-neither. | operator-decided | design-decided (execution-design.md verb table, E1 status row, operator-accepted: "`--role` excludes `--request-id`" and nothing else) |
| D107 | Precondition sets (section 4.1.3): activate keeps D83; claim and takeover use `request_unused`, `active_revision`, `active_job`, `fact_version` for every role of the active job, `role_state(target)` and `generation_current(target, G)`; takeover omits any secret (`run_active` is not used). Reads have no preconditions. Only the eight closed members. The mismatch mapping follows Part 1 EX-3 Conflicts and the closed EX-7 details (D114). The sets of the reserved operations are deferred (D150) and native enforces the fixed sets (D151). | new-engineering | design-decided (EX-3 vocabulary, R15-4, R15-5); NEW-engineering for the exact sets (D34) |
| D108 | Exempt reads (D51): `status` without `--role`, `history`, `export`, `orchestrator --ledger`. Bound (not exempt): `activate`, `claim`, `takeover`, `preview`, `status --role`. `init` is classified `not bound (no active revision exists; not an execution verb)`: it creates a ledger, so it is a mutation (the read/mutation split of E1 stays correct: it is not listed among the reads) with no active revision to bind, and it is neither an exempt read nor bound. Rule: a verb is exempt exactly when it neither interprets the active revision nor computes `work_exec_v1` under current Lua. Exemption removes only the interpretation binding step; the format outcome of the exempt reads is D142. | new-engineering | design-decided (E3 A: reads that need no current Lua are exempt); NEW-engineering for the list |
| D109 | Refusal order for a non-exempt execution verb: usage, format, early replay, interpretation binding, expectations, `no_active_job`, `reference` (unknown job then role), state refusals, provenance input (claim and takeover with `--provenance`, D146), then commit preconditions. The order for exempt reads is D142. | new-engineering | EX-3 step 1 and EX-6 (contract-decided) for the first four; NEW-engineering for the rest |
| D110 | `reference` field values at execution sites are `selection.job` and `selection.role` (the existing values at `selection.lua:95,97`); `schema` for a malformed provenance file has field `provenance` followed by the failing leaf path; no new code or message. | new-engineering | contract-decided for codes (Messages table); NEW-engineering for field values |
| D111 | `activate` expectations: `--expect-revision` mismatch is `revision_conflict` with `{expected,actual}` (existing code and message, reused where the meaning fits); `--expect-job` naming a job ID that is not the active job (or when no job is active) is `no_active_job`, because EX-7 freezes that code as "The ledger has no active job, or the named job is not the active job"; `--expect-job NONE` while a job is active is `state_conflict` at the snapshot, before the lock (a site added to the `state_conflict` Messages cell by amendment A-13, D149); the Messages row for `revision_conflict` names activate as a second site (recorded as amendment A-8 because R15-A1 I66 scoped that cell to the load compare-and-swap). | new-engineering | design-decided (R15-2 preconditions); NEW-engineering for the codes |
| D112 | The dispatch packet and manifest name the run by its claim or takeover request ID, never by the numeric run id, token or secret, because the run id is allocated under the lock (Common rule 3) and a predicted id would cause spurious `state_conflict` between claims of different roles. The numeric run id appears in the claim response and in later reads. | new-engineering | NEW-engineering (derived from Common rule 3 and the lost-update guard A-claims) |
| D113 | Service protocols (section 4.1.5): `read_execution({})`, `read_request({request_id})`, `read_provenance({provenance})`, `open_workspace`, `capture_drafts({route})`, `commit_execution({change})`, and for format3 load the order `open_workspace`, `capture_load`, `read_execution({})`, `accept_load({bundle,semantic,execution})`. `status` calls `read_execution`, then `read_request` (only with `--request-id`), then `open_workspace` and `capture_drafts` (only with `--workspace` and `--registry`). Each callable once, in order; family-scoped. | new-engineering | design-decided for `read_execution({})` and `accept_load(... execution:CHANGE)` (R15-3, execution-design.md:565-568); NEW-engineering for the rest |
| D114 | Role-state refusals use only the closed EX-7 details (`claim_conflict` `{state}` is one of `blocked`, `running`, `done`, `retired`; `fenced` `{run_state}` is one of `active`, `released`, `interrupted`, `completed`, `fenced`). `claim`: role not `ready`/`stale` at the snapshot is `claim_conflict` with its state. `takeover`: a `--run-id` that names no run of that job and role is `state_conflict` at the snapshot, before the lock (no run state exists, so none is invented; a site added to the Messages cell by amendment A-13, D149; a same-ID retry cannot succeed for an unknown run id, recorded as a driver-visible limit in P2-1); a run id that is not the role's current active run, including when the role is not `running` (no run is current), is `fenced` with `{run_state}` of the named run; `claim_conflict` is never used by takeover. Under the lock: for `claim`, a changed `role_state(target)` is `claim_conflict` only when the locked state shows the role taken by another run (`running`) or in another closed-set state (`blocked`, `done`, `retired`), else `state_conflict`; for `takeover`, a named run that is no longer current is `fenced` with its locked `run_state`, else `state_conflict`. No new message. | new-engineering | design-decided (R15-5 requires the current run id; Part 1 EX-3 Conflicts and EX-7 details); NEW-engineering for the mapping |
| D115 | `status` shape (section 4.1.4): full R15-7 field list; ids as decimal strings; `active_job` is the empty string when none; `generation` is the ordinal as a decimal string; `roles` lists every `role_state` row of the active job, including `retired` roles that the active revision no longer declares, sorted by role ID; `kind` is read from the `role_structure_v1` structure record of the role's current generation (`snapshot.roles[].generation.structure`), never from the interpretation, so a retired role keeps its kind; `role_detail` and `drafts` per R15-7 with exact keys; "role exists in the job" means declared by the active revision's recorded interpretation for the active job, and for a role that `status_v1` lists as `retired` but the active revision no longer declares `role_detail.current` is `none` (D152). | new-engineering | design-decided (R15-7); NEW-engineering for exact keys |
| D116 | Drafts: `status --workspace --registry` uses `open_workspace` and a status-only service `capture_drafts`; declaration-content errors come back as a value `outcome:"rejected"` so A20 can report `invalid`; capture instability, I/O, busy, resource errors fail the command. | new-engineering | design-decided (R15-7, A20); NEW-engineering for the service |
| D117 | Dispatch packet: `P.mode:"dispatch"` for both claim/takeover packets and the `preview` verb; two fixed authority templates (dispatched, preview); `run` section mandatory, second in order after `authority`; legal next actions by role kind with the purpose strings pinned byte-exact in 4.2.1 (D147); shipped `worker` and `reviewer` projections only, no new projection ID, no external projection for dispatch. | new-engineering | design-decided (R15-6); NEW-engineering for placement, templates and the external exclusion |
| D118 | Run token appears only in `run.token` of claim and takeover responses (and their `status --request-id` and `history --request-id` lookups); never in a packet, manifest, `--event` expansion, status other than lookup, preview or log. Wording of EX-2 line 540 is clarified. | operator-decided | design-decided (R15-9, E11 A) |
| D119 | `dispatch_manifest_v1` exact field list (section 4.2.3); versions entry `dispatch_v1`; records referenced by ID (packet file record, manifest value record codec `dispatch_manifest_v1`, W value record codec `work_exec_v1`); bound `manifest_bytes` 16777216. | new-engineering | design-decided (R15-6 field list; EX-8) ; NEW-engineering for exact value forms |
| D120 | `work_exec_v1` is the render W fields (render-contract.md:331-338) with the changes of the execution-design.md EX-5 bullet "Execution W is a new versioned recipe `work_exec_v1`": `base` is the declared form `{status:"declared",root,label}`; `start_facets` take their semantic from the current publication (the word of the design; "current" is defined by R17, and no publication exists in R15); `versions` gains `work_exec_v1` (seven entries); and the new field `repair` is a decimal string (`"0"` before any repair), kept per structural lineage (Part 1 EX-5 Lineage). Digest `H("work_exec_v1", W)`. render-contract.md says "no repair revision or generation is fabricated. Later execution must add those through a versioned recipe": `repair` enters W through `work_exec_v1`, while the generation is the `generation` key of `dispatch_manifest_v1` and the `{generation}` of the authority statement, not a W field, so a takeover (new generation, same W) never moves W (execution-design EX-5 bullet, operator E5 A). | operator-decided | design-decided (execution-design.md EX-5 bullet, operator E5 A, R15-6; Part 1 EX-5 Lineage) |
| D121 | `driver_provenance_v1` field list, leaf rule (string 1..1024 bytes or exactly `{"status":"unknown"}`), bounds from EX-8, stored value record codec `driver_provenance_v1` wrapping `authority:"driver_assertion_unverified"`; absent file stores nothing (`runs.provenance_id` NULL) and every field reads unknown. | new-engineering | design-decided (R15-8, E6 A, EX-8 row Provenance file); NEW-engineering for the record wrapper |
| D122 | Attachment points: claim and takeover (live in R15); release and check are fields-only, event reference key `provenance` pinned by R16 and R17. | operator-decided | design-decided (R15-8; basis: operator E6 A); scope boundary in round record |
| D123 | Native per-run provenance (ledger identity, revision, source digest, job, role, generation, base label, W and render digests, requested preferences) is derived from rows and always reported by the `history --event` expansion of `run_claimed` and `run_taken_over` as `provenance.native`, whether or not a driver file was given; it is not stored again. The driver part of the expansion is the stored document or, without one, the all-unknown form (D121). | operator-decided | design-decided (R15-8, marked NEW (operator: E6), basis E6 A: Planar records native provenance per run; a missing file records every field unknown) |
| D124 | `history --event ID`: decimal selector; excludes `--revision`, `--request-id`, `--at-event`, `--after-event`, `--limit`; valid on every ledger format; `data` selector `event`. | new-engineering | design-decided (R15-10, A29); NEW-engineering for exclusivity |
| D125 | `--event` expansion: `{selector:"event",event,transitions,execution?}`; records decoded natively without Lua; whole response at most 16777216 bytes with the existing `resource_limit` `{limit:"output_bytes",maximum:16777216}` refusal; the reader never selects `runs.secret`. | contract-decided | design-decided (R15-10, R15-9, ledger-contract.md:447, R14-E2) |
| D126 | `history --request-id` on format3: per-operation result decoded: `activate_v1` and `claim_v1` exact keys, packet and manifest assembled natively without Lua from `attachment_id` and `dispatches.manifest_id`; the whole stdout is bounded at 16777216 bytes by the existing `resource_limit` `{limit:"output_bytes",maximum:16777216}` refusal with no truncation (the packet is up to 1 MiB and the manifest up to 16 MiB, so the result can exceed the ceiling); no `disposition`; format1/2 unchanged. | new-engineering | design-decided (basis OP2 A, EX-2 packet responses); NEW-engineering for the exact keys (the standing delegation covers halt questions only, not these keys) |
| D127 | `status --request-id` response: the base status plus `request:{request_id,operation,outcome,result,disposition?}`; `disposition` only for claim and takeover (EX-2); unknown id `request_not_found`. | contract-decided | design-decided (R15-7, EX-2) |
| D128 | Orchestrator ledger form: option set `--ledger` alone; exactly one form; no active job gives `no_active_job`; on a format1 or format2 ledger `unsupported_ledger_version` (D142); context gains key `live` only in this form (database-free bytes unchanged apart from the action labels flipping). The orchestrator, its ledger form, the capability `execution_read_v1` on it and the label flips exist only in builds that compile the orchestrator, the authoring bundle (the production binary has no authoring path, D148). | new-engineering | design-decided (R15-11, RA-1 "R15 later adds a ledger form"); NEW-engineering for the no-job refusal |
| D129 | `live` content: `{ledger,revision,last_event,activation,roles,runs,blockers}`; `runs` are active runs `{id,role,generation,request_id}` with no token or secret; `blockers` is always `[]` in R15 (R18 defines rows). New mandatory section `orchestrator_state` in the ledger form only. | new-engineering | design-decided (R15-11); NEW-engineering for keys and section |
| D130 | Action labels: after R15 the `future` list is `refresh`, `checkpoint`, `release`, `recover`, `check`, `check-recover`, `verdict`; R16 flips the first four (and `preview --since`), R17 flips `check` and `check-recover`, R18 flips `verdict`; `available` still means "in the sealed catalog". The labels exist only where the orchestrator is compiled (D148). | contract-decided | design-decided (RA-7 future list; R16-7, R17-8, R18-8, R15-11) |
| D131 | Pointer amendments in the five other contracts are inserted pointer paragraphs only, each labelled "Pointer (R15-A2)" and stating that nothing is shipped until the R15 integrated gate; no current-state row, table or sentence of those contracts is edited, following the RA01-A pattern (the only existing line edited outside execution-contract.md is the `execution-design.md` F2 bullet). Every pointer says it narrows no approved clause. | contract-decided | contract-decided (round record scope; RA01-A pointer pattern) |
| D132 | Fixed root help after R15, per build (exact bytes, section 4.7.2), described in an inserted pointer (the current bytes stay in force until the R15 integrated gate). Production: the `bootstrap.cpp` bytes with the history continuation line gaining ` \| --event ID` and five lines (activate with `--expect-job JOB_OR_NONE`, status as one line, preview, claim, takeover) immediately before the final `planar --help\n`; no orchestrator line. Authoring bundle: the production bytes with the six RA01 lines spliced immediately before `planar --help\n` (so after the five R15 lines) and the continuation line `  or --ledger PATH\n` directly after the `planar orchestrator` line. `with-profile` help bytes unchanged. | new-engineering | design-decided (execution-design.md:384-391 requires both contracts amended); NEW-engineering for the exact text |
| D133 | Catalog after R15, per build: the production build has ten mandatory rows (the current five plus the five R15 rows) and the authoring bundle has sixteen (the ten plus the six RA01 rows; the RA01 rows and the `orchestrator` verb exist only there, D148). `orchestrator` (authoring bundle only) has descriptor options all optional, presence rule native. Capabilities: activate `execution_commit_v1`; status `execution_read_v1` and `draft_capture_v1`; preview `execution_read_v1`; claim and takeover `execution_commit_v1` and `provenance_read_v1`; orchestrator (authoring bundle only) gains `execution_read_v1`. Handler IDs `shipped.<verb>.v1`; response kinds `activate_v1`, `status_v1`, `preview_v1`, `claim_v1` (shared by claim and takeover). | new-engineering | design-decided for shape (verb table, RA-1 pattern); NEW-engineering for names |
| D134 | X-CLI family list under `with-profile`: the closed factory widens from five to ten families (adds `activate`, `status`, `preview`, `claim`, `takeover`; admission rows and the `command_options` variant change are R15-B and R15-C work with an x07-addendum amendment, not made here). The six RA01 verbs stay embedded-only: this is option A of the delegated D134 question, accepted under the standing delegation of 2026-10-31, and it is recorded as a by-reference amendment of the closing paragraph of `authoring-contract.md` RA-9 and of design RA-9 (`execution-design.md:499`), listed in the Pointer amendments next to D144 (neither file is edited beyond the F2 bullet). The sentence "R15 widens the closed factory to admit these verbs under `with-profile` only by its own reviewed amendment" is superseded by: R15 does not admit them; a later round, provisionally named RA02 here (the roadmap assigns the real ID after the R15 integrated gate), admits them under `with-profile` by its own reviewed amendment, decides which reads are exempt, and carries the shape-catalog narrowing-risk row (`remaining-steps-design.md:493-494`) in its invariant table. `orchestrator --ledger` is an exempt embedded read. The consequences for external profiles are stated in the pointer (D145). | operator-decided (delegated 2026-10-31, option A) | design-decided (E3 A: "new shipped families join the closed factory", execution-design.md:281-283); delegated option A of the D134 question (operator, standing delegation 2026-10-31); an authority choice that departs from the RA-9 closing sentence by recorded amendment |
| D135 | Reserved built-in section IDs gain `run` (dispatch packets) and `orchestrator_state` (ledger-form orchestrator). The reservation of both section IDs applies in every build, production included, even though `orchestrator_state` is rendered only in the authoring bundle: `runtime.cpp` reserves the RA01 `orchestrator_*` section IDs in every build today, so R15-D does not guess. A profile registering either is valid today and refused after R15 (same consequence statement as RA-9 item 6). | new-engineering | design-decided (R15-6 adds `run`); NEW-engineering for `orchestrator_state` |
| D136 | Load services table, format3 row: services in order `open_workspace`, `capture_load`, `read_execution({})`, `accept_load({bundle,semantic,execution})`; format1 and format2 keep three services and exact keys `{bundle,semantic}`. | new-engineering | NEW (engineering) per R15-3 (service shape), design-decided order |
| D137 | `execution_change_v1` and `execution_snapshot_v1` exact key lists (section 4.1.5), expectations as eight-member objects; run secrets never in the snapshot; the snapshot carries the decoded acknowledged W value (`work`) of each running role's dispatch because `status --role` needs it for `changed` and `explain` (Part 1 EX-3 step 2). | new-engineering | design-decided (EX-3 steps 2 and 4); NEW-engineering for keys |
| D138 | Part 1 amendments A-1 to A-13 (section 5) are wording, pointer and Messages-cell updates that Part 2 requires. A-3 tightens the token-exposure rule of EX-2 line 540 (no packet carries the token; claim and takeover results carry it only as `run.token`); A-13 widens the refusal-site cell of `state_conflict` in the Messages table to two snapshot-time sites (D149); A-8 widens the scope of the `revision_conflict` Messages cell that R15-A1 I66 recorded as the load compare-and-swap only. Every other amendment re-points a reference or adds a listed item. No other semantics of Part 1 change. | contract-decided | contract-decided (round record: scoped, recorded, reasoned) |
| D139 | F2 carried gap: the execution-design line 527 bullet is reworded to say that the RA01 authoring and observe captures are covered by the named tests, and the receipt-reuse claim is limited to what render and load capture paths exercise. | contract-decided | contract-decided (RA01-C row C-15q, `rounds/V5-02-RA01-C.md:102,213`) |
| D140 | F3 to F8 stay carried as test-only obligations in the acceptance checklist (named for R15-B to R15-E). | contract-decided | contract-decided (round record "Carried RA01-C coverage gaps") |
| D141 | Evidence for this docs round (checker, base snapshot from the recorded base commit `cdcfe829c466bf6338815ff6f5b6ea90ffddc570` via `git show`, never HEAD, red run on a scratch tree built only from the snapshot, prober baseline and per-mutant logs in `probes/` owned by the prober stage (unmutated baseline first; each kill log names the killing check), link check, redaction with control); hashes only from the workflow `identity.py`, pasted verbatim; no record or evidence auditor; no hashes of other records pinned. | contract-decided | contract-decided (round record "Agreed verification") |
| D142 | Exempt reads and ledger format. `status` in every form and `orchestrator --ledger` read execution tables that exist only in format3, so on a format1 or format2 ledger they are refused `unsupported_ledger_version` after `usage` (existing code and message; no replay, no binding step except `status --role`, which is bound). `history` and `export` keep every format (`history --event` is valid on all); `init` is not bound and not an exempt read (D108). `preview` (non-exempt) follows D109, with the format refusal second. Refusal order for exempt reads: usage, format, (interpretation binding for `status --role` only), the verb's own refusals. | operator-decided (delegated 2026-10-31, option A) | NEW-delegated (Common rule 1 lists only the mutations; the exempt reads need an explicit outcome) |
| D143 | Format3 history shapes (R15-10): the event summary gains the typed reference columns `job`, `role`, `generation_id`, `run_id` beside payload v2 (`job` and `role` are IDs or the empty string, consistent with payload v2 `active_job`; `generation_id` and `run_id` are decimal strings or `"none"`; `none` is a legal `id` per declarations.md:33 and EX-4 does not spell an absent job `none`, because `none` is a legal job ID, so an absent job or role is never spelled `none`); `--at-event` `state` on format3 carries role states and active runs of the active job as of that event, replayed from rows without Lua (EX-1 invariant 19). Format1 and format2 shapes are unchanged. | new-engineering | design-decided (R15-10; ledger-contract.md R14-E2 pins "absent IDs use none" for numeric database IDs only); NEW-engineering for key names |
| D144 | By-reference amendments of `authoring-contract.md` are recorded in the Pointer amendments section of the execution contract and the file is not edited: RA-9 closing paragraph (D134: R15 does not admit the six RA01 verbs under `with-profile`; a later round does), RA-1 (the orchestrator options become optional with a native all-or-none rule and the ledger form, authority: RA-1 "R15 later adds a ledger form by its own contract amendment") RA-7 (section list gains `orchestrator_state`; "It states no execution state" applies to the database-free form only) and RA-9 items 1, 4 and 6 (item 1: the embedded probe holds the five X08 families, ten after R15; item 4: the with-profile catalog of five shipped rows, completeness rule five, embedded rule eleven, become ten, ten and sixteen (authoring bundle); item 6: with-profile catalog of five shipped commands, external capacity unchanged, becomes capacity 54), each "valid until the R15 integrated gate". | contract-decided | contract-decided (RA-1 sentence; the contract's own rule that each amendment is recorded) |
| D145 | External-profile consequences of the ten-family list are stated as "valid today, refused after R15" in the XP pointer, following RA-9 item 6: the five command and family names, the five shipped handler IDs and the section IDs `run` and `orchestrator_state` (reserved in every build, production included) leave the external namespace; external command capacity falls by five (X-LIMIT counts 64 commands including shipped; the table is unedited); the `history` probe signature gains `--event ID`; the with-profile completeness rule moves from five to ten. | new-engineering | design-decided (E3 A) with RA-9 item 6 as the model; NEW-engineering for the exact statement |
| D146 | Provenance input handling (4.4.1): `--provenance` is an explicit single-file capture that follows the `--diagnostic` rows of the authoring Messages table (`io` `Cannot resolve explicit input path` for a missing, unresolvable or unreadable file; `path_denied` `Explicit input file is a symbolic link`; `io` `Explicit input must be a regular file`; `capture_changed` `File changed during read`). Checks run in the order file bytes, JSON grammar, depth, settings count, string bytes, then shape and leaf-rule checks, so a string leaf over 1024 bytes is `resource_limit` with `provenance_string_bytes`, never `schema`. The `schema` field is `provenance` or `provenance.` plus a dotted key path. Provenance refusals are step (9) of the refusal order, after the role-state refusals; an early replay never reads the file. | new-engineering | NEW-engineering (authoring-contract.md Messages `--diagnostic` rows; EX-8 row Provenance file; D109) |
| D147 | The `run` section body is pinned byte-exact (4.2.1): the `## run` framing follows the render rule "Every generated section begins `## <ID>`"; the five purpose strings for `refresh`, `checkpoint`, `check`, `release` and `verdict` are fixed by this contract and `dispatch.lua` reproduces them, because the retained packet bytes, `render_fp` and the claim retry bytes depend on them and the dispatch packet states the legal next actions as normative text. | new-engineering | NEW-engineering (render-contract.md leaves section headings and labels to the projection; this contract fixes the body) |
| D148 | Build variants of the R15 bytes and counts (4.6, 4.7.1, 4.7.2). Operator answer D2/D3 option B (2026-10-30): the production binary has no authoring path (`src/cmd/planar/CMakeLists.txt:267`: "The product targets never receive PLANAR_AUTHORING_BUNDLE"). Production: ten catalog rows; root help is the `bootstrap.cpp` bytes with the history continuation changed and the five R15 lines before `planar --help`; no `orchestrator` verb, so no `orchestrator --ledger`, ledger form, `orchestrator_state` section, orchestrator `execution_read_v1` capability or action-label flips. Authoring bundle: sixteen rows; the splice (`authoring::splice_root_help`) inserts the six RA01 lines immediately before `planar --help`, so the relative order is the five R15 lines, the six RA01 lines, the continuation `  or --ledger PATH`, then `planar --help`; the continuation is a seventh spliced line (R15-D work on `root_help_lines`). The section-ID reservation of `run` and `orchestrator_state` is not build-qualified: it applies in every build (D135). | operator-decided | design-decided (basis: operator answer D2/D3 option B, 2026-10-30); NEW-engineering for the R15 bytes and order |
| D149 | Amendment A-13: the `state_conflict` refusal-site cell of the Messages table ("Any other failed precondition at commit") gains the two snapshot-time sites that D111 and D114 use before the lock: the `activate` `--expect-job NONE` mismatch while a job is active (refusal step 5) and a `takeover` `--run-id` that names no run of the job and role. An `activate` `--expect-job JOB` mismatch is not a site: it is `no_active_job`, which EX-7 freezes as "The ledger has no active job, or the named job is not the active job", so a driver that follows EX-7 does not retry it as a conflict. The frozen EX-7 meaning of `state_conflict` ("changed between snapshot and commit ... retry with the same request ID") is not edited, and no code or message is added. Driver-visible limit stated in P2-1: a same-ID retry cannot succeed for these two sites (the refusal is deterministic until the active job changes, or forever for an unknown run id); the code is reused because EX-7 is closed and no existing code fits better for them. | new-engineering | NEW-engineering (derived from D111, D114 and the closed EX-7 set) |
| D150 | Precondition sets of the reserved operations are deferred: `refresh`, `checkpoint`, `release`, the `check` and `check-recover` commits and `verdict` have no request body, facts, response or event in Part 2, so P2-1 defines no member set for them; their sets are defined by the contract round of each operation (R16 for `refresh`, `checkpoint`, `release` and `recover`; R17 for the two check commits; R18 for `verdict`), using only the eight closed members. Part 1 EX-3 (lines 594 and 646) says the remaining members are defined by R15-A2; amendment A-4 re-points those sentences to P2-1 and records this deferral with its reason. Rejected: fields-only sets for reserved operations (they would pin members before the lifecycle and event shapes exist). | new-engineering | design-decided (EX-3 vocabulary; reserved verbs stay reserved, D140); NEW-engineering for the deferral |
| D151 | Native enforcement placement: native validation at EX-3 step 5 requires `expected` to equal exactly the fixed set of the operation in 4.1.3 (same eight-member kinds, no omitted, extra or duplicate member) with every value equal to the snapshot read by the same command; it refuses a `claim` change whose target before-state in the snapshot is not `ready` or `stale`, and a `takeover` change whose named run is not the target's active run. A handler that violates this is refused with code `internal` (its existing message in the Messages table; no new message) because a correct handler has already returned `claim_conflict`, `fenced` or `state_conflict` before building the change. Lua supplies `expected`; native enforces the set. | new-engineering | design-decided (EX-3 step 5: native verifies the given preconditions); NEW-engineering for the exact-set rule |
| D152 | `status --role R` and `preview --role R` reference behaviour: with an active job, a role not declared by the active revision's recorded interpretation for the active job and not listed in `role_state` is `reference` (message `Unknown selected role`, field `selection.role`); a role listed with state `retired` that the active revision no longer declares has no current W: `status --role` returns `role_detail` with `current:"none"`, `changed:[]` and `explain:{work_or_semantic:[],output_vector:[],prerequisite:[]}`, while `preview --role` is `reference` with the same message and field. "Role exists in the job" always means declared by the active revision's recorded interpretation. | new-engineering | design-decided (R15-7, R15-6); NEW-engineering for the retired case |
| D153 | Preview manifest: `run` is `{kind:"preview"}` with no `request` key; `version`, `revision`, `job`, `role`, `generation`, `versions`, `work`, `prerequisites`, `registry` and `inputs` are computed as for a dispatch, except that `generation` is the role's current generation ordinal (a preview never creates a generation); `render`, `sections` and `omitted` are computed from the preview packet's own bytes; the manifest is returned in `preview_v1` and never stored. | new-engineering | design-decided (R15-7, R15-6); NEW-engineering for the preview run value and the preview generation (C01, review R4) |
| D154 | Preview run-section line is `run request: (preview)`; its characters are outside the request ID grammar `[A-Za-z0-9_.:-]{1,128}`, so it never equals a legal request ID. | new-engineering | NEW-engineering (C01, review R4) |
| D155 | `dispatch_v1` names the dispatch projection rules of P2-2; the generation is the `generation` key of `dispatch_manifest_v1` and the `{generation}` of the authority statement, not a W field and not carried by any identifier. | new-engineering | design-decided (render-contract identifier rule; execution-design EX-5 bullet, operator E5 A); NEW-engineering for the definition (C01, review R5) |
