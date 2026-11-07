# Execution contract v1 (part 1)

Status: exact contract text for round [V5-02-R15-A1](rounds/V5-02-R15-A1.md), documentation only. It specifies
behaviour; it ships nothing. Until the R15-B and R15-C rounds implement it, the current commands remain `render`,
`init`, `history`, `export` and `load`, and the approved contracts govern them unchanged. Part 1 holds the header,
the common rules, clauses EX-1 to EX-8, the state machines, the Part 2 placeholders for R15-A2, the messages, the
pointer amendments, the traceability table, the acceptance checklist and the decisions table. The Part 2 placeholders
reserve the content that R15-A2 writes in this same file.

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
four names are the run states of the same spelling. The `status` verb is defined in Part 2 (R15-A2). The saved result that `status --request-id` returns for a claim or takeover request carries the run token (R15-7, R15-9).

`history --request-id` on format3. Decision OP2, tagged `operator-decided (delegated 2026-10-31, option A)`, applies.

- On format3, `history --request-id` also returns the execution results of its request. For claim and takeover
  requests the result includes the run token, as the [run token](#run-token-r15-9) subsection defines.
- The response keeps the existing request record shape of the ledger request lookup row: the saved result with the
  head and `last_seq`. It adds no `disposition` field. Disposition is reported by `status --request-id` only.
- Format1 and format2 `history --request-id` responses are unchanged.
- Any further response shape, including packet assembly and a per-operation result schema, is deferred to the
  placeholder `Part 2 (R15-A2): history --event selector`.

### Run token (R15-9)

- Grammar: `<run id>.<generation ordinal>.<32 lowercase hex>`. The run id is `runs.id`. The generation ordinal is
  the ordinal of the run's generation. The 32 hex digits encode the secret.
- The secret is 16 bytes from the operating-system random source, stored in `runs.secret` as a 16-byte BLOB.
  The secret guards against accidental reuse. It does not guard against a reader of the ledger: anyone who can read
  the ledger and learn a request ID can recover the token from the saved result.
- The token is shown only in the claim and takeover results, in `status --request-id` of those requests, and in
  `history --request-id` of those requests on format3 (operator decision E11 A, R15-9, R15-7).
  It is never shown in `status` other than its request lookup, in a history event expansion, in render output,
  in any packet other than the claim or takeover packet, in logs or in error details.
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
members for each other operation are defined by R15-A2 in Part 2. The sets for format3 load and `activate` are fixed
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
precondition sets of the other operations belong to R15-A2.

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

**Explain.** The explain output of the status read reports the reasons of the changed roles together with the top-level W fields that changed, and it separates the three causes that v5 requires: a work or semantic change, output-vector applicability, and prerequisite availability (`v5-alignment.md:143-145`). Its shape is in the [Part 2 placeholders](#part-2-placeholders-r15-a2).

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

**work_exec_v1.** The execution work fingerprint W is the versioned recipe `work_exec_v1`. It is computable from the ledger alone, so load, activate, status and claim compute it without a checkout (operator decision E5 A). The state-level rules are these: drift is the state of a running role whose current W differs from its acknowledged W ([EX-4](#ex-4-facts-derived-state-and-reconcile-rules)). The field recipe of `work_exec_v1` is not written in this part. It is the [Part 2 placeholder](#part-2-placeholders-r15-a2) for `work_exec_v1`. The render-only recipe `work_v1` is unchanged and remains what render emits (`render-contract.md:406`: later execution adds its fields through a versioned recipe). The narrowing of v5 `planar-design-spec-v5.md:241` that E5 records is in the [Pointer amendments](#pointer-amendments).

## EX-6 Interpretation binding

Every execution verb, and the `preview` and `recover` reads, interprets the active revision with the exact trusted code that accepted it. The invocation's effective stdlib digest, embedded or with-profile, must equal the active revision's recorded `stdlib_digest`. If it does not, the verb refuses with `interpretation_mismatch` before any mutation (operator decision E3 A). The interpretation check runs after early replay, so a saved result still replays across a binary or profile change, as load does (`load-command-contract.md:103-106`).

Reads that need no current Lua are exempt from the check, so an upgrade cannot block recovery (E3 A). Which verbs are exempt reads is a Part 2 clause. A revision accepted under a profile requires `with-profile` with the same profile bytes. The factory family is closed at five (`x07-addendum.md:66`): each added family needs an architecture revision, an admission row and a contract amendment. A binary or profile change is absorbed by a fresh load, which creates a new revision reconciled normally.

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

## Part 2 placeholders (R15-A2)

The seven subsections below reserve content for round R15-A2. They are not normative, and they define no behaviour in this part.

### Part 2 (R15-A2): verbs and services

Not yet written. Marked non-normative placeholder; R15-A2 clauses: per-verb parameter maps; precondition sets for the operations other than load and activate (D34); the spelling of the no-active-job expectation for `activate`, `--expect-job`, since none is a legal job ID (`declarations.md:33`); the field value of reused `reference` and `schema` sites (Messages); the service protocol of each verb.

### Part 2 (R15-A2): dispatch projection and dispatch_manifest_v1

Not yet written. Marked non-normative placeholder; R15-A2 clauses: the dispatch projection mode and its fixed authority statement; the mandatory run section and its legal next actions; the `dispatch_manifest_v1` field list and its record references; the worker and reviewer projection IDs; the render-contract pointer for the dispatch projection (R15-6).

### Part 2 (R15-A2): work_exec_v1 field recipe

Not yet written. Marked non-normative placeholder; R15-A2 clauses: the field list of `work_exec_v1` and its digest recipe; the start-facet semantic values taken from current publications; the repair revision field; the versions entry; the render-contract pointer for W (EX-5).

### Part 2 (R15-A2): provenance (driver_provenance_v1)

Not yet written. Marked non-normative placeholder; R15-A2 clauses: the `driver_provenance_v1` field list and its leaf rules; the unknown-status leaves; the stored value record labelled `driver_assertion_unverified`; the attachment points of claim, takeover, release and check; the provenance bounds of EX-8 as the reader applies them.

### Part 2 (R15-A2): history --event selector

Not yet written. Marked non-normative placeholder; R15-A2 clauses: the `history --event` selector and the expansion of referenced execution records (packet Markdown, manifest and transitions) within 16 MiB, decoded without Lua; the `history --request-id` execution-result shape, including packet assembly and the per-operation result schema; the ledger-contract command-surface pointer (the `history --event` selector row).

### Part 2 (R15-A2): orchestrator live form

Not yet written. Marked non-normative placeholder; R15-A2 clauses: the `orchestrator --ledger` live state (role states, runs without secrets, blockers); the orchestrator context fields; the action labels that become available in R15 and in later rounds; the pointer to the authoring contract's orchestrator projection (RA-7).

### Part 2 (R15-A2): command, render and load pointer amendments

Not yet written. Marked non-normative placeholder; R15-A2 clauses: the command-contract, render-contract and load-command-contract pointers; the external-profile X-CLI family list (`X-CLI`); the reserved section IDs, including the run section (R15-6); the load-command-contract services table amendment for format3 (R15-3).

## Messages

This section maps each execution refusal site to an existing message or to one of the nineteen messages of [EX-7](#ex-7-error-codes-and-messages). A site uses an existing message by name and never respells it. No execution site introduces a twentieth message.

**Execution refusal sites.**

| Refusal site | Code | Message | Defined by |
| --- | --- | --- | --- |
| A run verb with no active job | `no_active_job` | `No active job matches the request` | [EX-7](#ex-7-error-codes-and-messages) |
| Claim of a role that is blocked, running, done or retired, or a lost concurrent claim of one role | `claim_conflict` | `Role state does not allow this operation` | [EX-3](#ex-3-execution-commit-protocol), [EX-7](#ex-7-error-codes-and-messages) |
| Any other failed precondition at commit | `state_conflict` | `Execution state changed since the request was prepared` | [EX-3](#ex-3-execution-commit-protocol) |
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
| `revision_conflict` | `Ledger revision does not match the expected revision` | `load-command-contract.md:261`, row `locked head mismatch`, with details `{expected,actual}` | The format3 load compare-and-swap only |
| `reference` | `Unknown selected job` | `authoring-contract.md:506`, row `Orchestrator job not listed`; the same string at `src/stdlib/planar/selection.lua:95` | An unknown job at an execution verb |
| `reference` | `Unknown selected role` | existing mapping, `src/stdlib/planar/selection.lua:97` | An unknown role at an execution verb; the field value is an R15-A2 clause |
| `schema` | The existing message of the shared JSON parser or schema normalizer for the failing bytes; no single string | `authoring-contract.md:505`, row `Diagnostic file grammar or envelope violation, observation file validation, ...` | Malformed verdict, observation or provenance input |
| `observation_target_mismatch` | `Wrong output vector` | existing mapping, `src/stdlib/planar/observations.lua:51` (render observation validator) | A wrong output vector in a check view (R17-2, step 2) |
| `resource_limit` | `Capture limit exceeded` | `authoring-contract.md:504`, row `Bounds`, for the existing observe bounds only | The `observation_*` bounds of EX-8; new bounds use `Execution resource limit exceeded` |
| `capture_changed` | `Captured declaration inputs changed` | `load-command-contract.md:265`, row `selected receipt changed`, for load capture | Load capture; for observe, the two messages `Directory membership or identity changed` and `File changed during read` at `authoring-contract.md:502-503` |
| `ledger_busy` | `Ledger is busy` | existing mapping, `src/cmd/planar/response.cpp:220` | A busy ledger at any execution commit |
| `storage_full` | `Ledger storage is full` | existing mapping, `src/cmd/planar/response.cpp:221` | A full disk at any execution commit |

The owner of each message that names a contract row is that row's text, and the owner of each existing mapping is the named source file. No row respells a message.

## Pointer amendments

Each amendment is recorded in the pointed document's own text. The amendments are insertions only, and no approved clause is narrowed.

1. [CLAUDE.md](../../CLAUDE.md): one inserted authority entry, `[execution](docs/vernacular/execution-contract.md),`, after the authoring entry of authority item 2 (E14 A). No other line changes, and `AGENTS.md` remains a symlink.
2. [ledger-contract](ledger-contract.md): three inserted pointer paragraphs: the format3 event types and payload version 2, which point to [EX-4](#ex-4-facts-derived-state-and-reconcile-rules), after the event vocabulary paragraph; the format3 request-lookup note, which points to [EX-2](#ex-2-request-framing-and-replay); and the format3 pointer to [EX-1](#ex-1-format3) in the format evolution section. Format1 and format2 text is unchanged.
3. [external-profile-contract](external-profile-contract.md): one inserted format3 paragraph in the format table region, pointing to [EX-1](#ex-1-format3) and [EX-2](#ex-2-request-framing-and-replay). It supersedes by insertion the sentence "New init creates format2 only", which is not edited (D82). It states that load on format3 keeps `request_v2`, including the profile and `effective_stdlib_v1` path. The message tables are untouched.
4. [acceptance-matrix](acceptance-matrix.md): the seven rows A09, A10, A20, A22, A23, A26 and A33 each gain an appended sentence that links the exact section of this contract. Their status words are byte-unchanged, and no other row changes.
5. [v5-alignment](v5-alignment.md): two inserted recorded entries, each citing this contract and the operator answer of 2026-10-28. The first is deviation E2 from `planar-design-spec-v5.md` lines 110 and 497 (operator answer A). The second is narrowing E5 of line 241 (operator answer A).
6. [implementation](implementation.md): one inserted bullet pointing to this contract. No paragraph is rewrapped.
7. Not made by this round: the command-contract ABI rows, the render-contract dispatch projection and the load-command-contract services table for format3 (R15-2, R15-3 and R15-6). They appear only as the placeholder clauses of [Part 2](#part-2-placeholders-r15-a2), and the external-profile X-CLI family list and the reserved section IDs appear there too.

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
| R15-2 Active job and activate | [EX-4](#ex-4-facts-derived-state-and-reconcile-rules) (activation and re-activation) |
| R15-3 Format3 load | [EX-4](#ex-4-facts-derived-state-and-reconcile-rules) (load reconciliation and absent active job), [EX-3](#ex-3-execution-commit-protocol) |
| R15-9 Run tokens | [Run token (R15-9)](#run-token-r15-9) |
| E1 One command per verb; reads separate | [Decisions table](#decisions-table) (OP1, OP2) |
| E2 Active job | [Pointer amendments](#pointer-amendments), [EX-4](#ex-4-facts-derived-state-and-reconcile-rules) |
| E3 Exact stdlib and profile binding | [EX-6](#ex-6-interpretation-binding) |
| E5 Ledger-only work fingerprint | [EX-5](#ex-5-role-structure-lineage-and-work_exec_v1), [Pointer amendments](#pointer-amendments) |
| E11 Run token secret | [Run token (R15-9)](#run-token-r15-9) |
| E14 New contract and authority list | [Pointer amendments](#pointer-amendments) |
| E16 New codes and execution message | [EX-7](#ex-7-error-codes-and-messages) |
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
