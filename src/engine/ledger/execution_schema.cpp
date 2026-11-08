// Format3 execution schema implementation: exact stored-SQL creation, the liveness-aware opener and the schema verifier.
// The 69 stored statements of execution-contract.md EX-1 are private to this file (D30, G10) and are compared byte for
// byte against sqlite_master. Every failure is mapped to the closed execution_error_code vocabulary without message text.
module;
#include <cerrno>
#include <fcntl.h>
#include <sqlite3.h>
#include <sys/random.h>
#include <sys/stat.h>
#include <unistd.h>
module planar.engine.ledger.execution_schema;

import std;
import planar.engine.ledger.sqlite;
import planar.engine.ledger.storage;
import planar.engine.json_codec;

namespace planar::engine::ledger {

namespace {

constexpr std::int64_t     format3_application_id = 1347178034;
constexpr std::int64_t     format3_user_version   = 3;
constexpr std::int64_t     format3_format         = 3;
constexpr std::string_view init_actor             = "planar:init";
constexpr std::string_view sqlite_magic{"SQLite format 3\0", 16};

// The 69 stored statements of execution-contract.md EX-1 in creation order: 24 tables, 2 explicit indexes and 43 triggers.
// Each entry is the stored sql, which excludes the terminating semicolon of the contract text.
constexpr std::array<std::string_view, 69> contract_statements = {
    R"SQL(CREATE TABLE ledger_meta (
  singleton INTEGER PRIMARY KEY CHECK(singleton=1),
  format INTEGER NOT NULL CHECK(format=3),
  identity TEXT NOT NULL CHECK(length(identity)=32),
  workspace_id TEXT NOT NULL,
  created_at TEXT NOT NULL
) STRICT)SQL",
    R"SQL(CREATE TABLE records (
  id INTEGER PRIMARY KEY CHECK(id>0),
  kind INTEGER NOT NULL CHECK(kind BETWEEN 1 AND 4),
  codec INTEGER NOT NULL CHECK((kind=4 AND codec IN (2,3)) OR (kind BETWEEN 1 AND 3 AND codec=1)),
  digest BLOB NOT NULL CHECK(length(digest)=16),
  encoded BLOB NOT NULL CHECK(length(encoded)<=67108864)
) STRICT)SQL",
    R"SQL(CREATE INDEX records_digest ON records(codec,kind,digest))SQL",
    R"SQL(CREATE TABLE tree_edges (
  parent_id INTEGER NOT NULL REFERENCES records(id),
  ordinal INTEGER NOT NULL CHECK(ordinal>=0),
  name TEXT NOT NULL,
  child_id INTEGER NOT NULL REFERENCES records(id),
  PRIMARY KEY(parent_id,ordinal), UNIQUE(parent_id,name),
  CHECK(parent_id>child_id)
) STRICT)SQL",
    R"SQL(CREATE TABLE revisions (
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
) STRICT)SQL",
    R"SQL(CREATE TABLE events (
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
) STRICT)SQL",
    R"SQL(CREATE TABLE current_state (
  singleton INTEGER PRIMARY KEY CHECK(singleton=1),
  active_revision_id INTEGER REFERENCES revisions(id),
  last_seq INTEGER NOT NULL REFERENCES events(seq),
  active_job TEXT CHECK(active_job IS NULL OR length(active_job) BETWEEN 1 AND 64),
  active_activation_id INTEGER CHECK(active_activation_id IS NULL OR active_activation_id>0)
) STRICT)SQL",
    R"SQL(CREATE TABLE requests (
  request_id TEXT PRIMARY KEY,
  operation TEXT NOT NULL CHECK(operation IN ('load','activate','claim','takeover','refresh','checkpoint','release','check','check-recover','verdict')),
  body_id INTEGER NOT NULL REFERENCES records(id),
  event_seq INTEGER NOT NULL UNIQUE REFERENCES events(seq),
  revision_id INTEGER NOT NULL REFERENCES revisions(id),
  outcome TEXT NOT NULL CHECK(outcome IN ('accepted','noop','activated','claimed','taken_over','refreshed','recorded','released','interrupted')),
  result BLOB NOT NULL CHECK(length(result)<=1048576),
  attachment_id INTEGER REFERENCES records(id)
) STRICT)SQL",
    R"SQL(CREATE TABLE job_activations (
  id INTEGER PRIMARY KEY CHECK(id>0),
  job TEXT NOT NULL CHECK(length(job) BETWEEN 1 AND 64),
  revision_id INTEGER NOT NULL CHECK(revision_id>0),
  event_seq INTEGER NOT NULL CHECK(event_seq>0)
) STRICT)SQL",
    R"SQL(CREATE TABLE generations (
  id INTEGER PRIMARY KEY CHECK(id>0),
  job TEXT NOT NULL CHECK(length(job) BETWEEN 1 AND 64),
  role TEXT NOT NULL CHECK(length(role) BETWEEN 1 AND 64),
  ordinal INTEGER NOT NULL CHECK(ordinal>0),
  structure_id INTEGER NOT NULL REFERENCES records(id),
  opened_seq INTEGER NOT NULL CHECK(opened_seq>0),
  reason TEXT NOT NULL CHECK(reason IN ('activated','structural_change','takeover','recreated'))
) STRICT)SQL",
    R"SQL(CREATE TABLE runs (
  id INTEGER PRIMARY KEY CHECK(id>0),
  generation_id INTEGER NOT NULL CHECK(generation_id>0),
  request_id TEXT NOT NULL,
  claimed_seq INTEGER NOT NULL CHECK(claimed_seq>0),
  secret BLOB NOT NULL CHECK(length(secret)=16),
  provenance_id INTEGER REFERENCES records(id)
) STRICT)SQL",
    R"SQL(CREATE TABLE dispatches (
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
) STRICT)SQL",
    R"SQL(CREATE TABLE checkpoints (
  id INTEGER PRIMARY KEY CHECK(id>0),
  run_id INTEGER NOT NULL CHECK(run_id>0),
  seq INTEGER NOT NULL CHECK(seq>0),
  body_id INTEGER NOT NULL REFERENCES records(id)
) STRICT)SQL",
    R"SQL(CREATE TABLE reservations (
  id INTEGER PRIMARY KEY CHECK(id>0),
  request_id TEXT NOT NULL UNIQUE,
  run_id INTEGER NOT NULL CHECK(run_id>0),
  opened_seq INTEGER NOT NULL CHECK(opened_seq>0),
  body_id INTEGER NOT NULL CHECK(body_id>0)
) STRICT)SQL",
    R"SQL(CREATE TABLE evidence (
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
) STRICT)SQL",
    R"SQL(CREATE TABLE completions (
  id INTEGER PRIMARY KEY CHECK(id>0),
  run_id INTEGER NOT NULL CHECK(run_id>0),
  seq INTEGER NOT NULL CHECK(seq>0),
  dispatch_id INTEGER NOT NULL CHECK(dispatch_id>0),
  work_fp BLOB NOT NULL CHECK(length(work_fp)=16),
  observation_id INTEGER NOT NULL REFERENCES records(id)
) STRICT)SQL",
    R"SQL(CREATE TABLE publications (
  id INTEGER PRIMARY KEY CHECK(id>0),
  completion_id INTEGER NOT NULL CHECK(completion_id>0),
  work TEXT NOT NULL CHECK(length(work) BETWEEN 1 AND 64),
  facet TEXT NOT NULL,
  semantic BLOB NOT NULL CHECK(length(semantic)=16),
  payload_id INTEGER NOT NULL REFERENCES records(id)
) STRICT)SQL",
    R"SQL(CREATE TABLE verdicts (
  id INTEGER PRIMARY KEY CHECK(id>0),
  request_id TEXT NOT NULL UNIQUE,
  run_id INTEGER NOT NULL CHECK(run_id>0),
  seq INTEGER NOT NULL CHECK(seq>0),
  body_id INTEGER NOT NULL REFERENCES records(id),
  applicability TEXT NOT NULL CHECK(applicability IN ('current','historical')),
  decision TEXT NOT NULL CHECK(decision IN ('approve','reject'))
) STRICT)SQL",
    R"SQL(CREATE TABLE repairs (
  verdict_id INTEGER NOT NULL CHECK(verdict_id>0),
  job TEXT NOT NULL CHECK(length(job) BETWEEN 1 AND 64),
  role TEXT NOT NULL CHECK(length(role) BETWEEN 1 AND 64),
  generation_id INTEGER NOT NULL CHECK(generation_id>0),
  revision INTEGER NOT NULL CHECK(revision>0)
) STRICT)SQL",
    R"SQL(CREATE TABLE blockers (
  id INTEGER PRIMARY KEY CHECK(id>0),
  seq INTEGER NOT NULL CHECK(seq>0),
  job TEXT NOT NULL CHECK(length(job) BETWEEN 1 AND 64),
  kind TEXT NOT NULL CHECK(kind IN ('unowned_rejection','max_rounds')),
  verdict_id INTEGER NOT NULL CHECK(verdict_id>0)
) STRICT)SQL",
    R"SQL(CREATE TABLE role_state (
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
) STRICT)SQL",
    R"SQL(CREATE TABLE run_state (
  run_id INTEGER PRIMARY KEY CHECK(run_id>0),
  job TEXT NOT NULL CHECK(length(job) BETWEEN 1 AND 64),
  role TEXT NOT NULL CHECK(length(role) BETWEEN 1 AND 64),
  generation_id INTEGER NOT NULL CHECK(generation_id>0),
  state TEXT NOT NULL CHECK(state IN ('active','released','interrupted','completed','fenced')),
  last_seq INTEGER NOT NULL CHECK(last_seq>0)
) STRICT)SQL",
    R"SQL(CREATE UNIQUE INDEX run_state_one_active ON run_state(job, role) WHERE state='active')SQL",
    R"SQL(CREATE TABLE reservation_state (
  reservation_id INTEGER PRIMARY KEY CHECK(reservation_id>0),
  state TEXT NOT NULL CHECK(state IN ('open','completed','interrupted')),
  last_seq INTEGER NOT NULL CHECK(last_seq>0)
) STRICT)SQL",
    R"SQL(CREATE TABLE blocker_state (
  blocker_id INTEGER PRIMARY KEY CHECK(blocker_id>0),
  state TEXT NOT NULL CHECK(state IN ('open','cleared')),
  cleared_seq INTEGER CHECK(cleared_seq IS NULL OR cleared_seq>0),
  last_seq INTEGER NOT NULL CHECK(last_seq>0),
  CHECK((state='open' AND cleared_seq IS NULL) OR (state='cleared' AND cleared_seq IS NOT NULL))
) STRICT)SQL",
    R"SQL(CREATE TABLE transitions (
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
) STRICT)SQL",
    R"SQL(CREATE TRIGGER ledger_meta_no_update BEFORE UPDATE ON ledger_meta BEGIN SELECT RAISE(ABORT,'immutable ledger row'); END)SQL",
    R"SQL(CREATE TRIGGER ledger_meta_no_delete BEFORE DELETE ON ledger_meta BEGIN SELECT RAISE(ABORT,'immutable ledger row'); END)SQL",
    R"SQL(CREATE TRIGGER records_no_update BEFORE UPDATE ON records BEGIN SELECT RAISE(ABORT,'immutable ledger row'); END)SQL",
    R"SQL(CREATE TRIGGER records_no_delete BEFORE DELETE ON records BEGIN SELECT RAISE(ABORT,'immutable ledger row'); END)SQL",
    R"SQL(CREATE TRIGGER tree_edges_no_update BEFORE UPDATE ON tree_edges BEGIN SELECT RAISE(ABORT,'immutable ledger row'); END)SQL",
    R"SQL(CREATE TRIGGER tree_edges_no_delete BEFORE DELETE ON tree_edges BEGIN SELECT RAISE(ABORT,'immutable ledger row'); END)SQL",
    R"SQL(CREATE TRIGGER revisions_no_update BEFORE UPDATE ON revisions BEGIN SELECT RAISE(ABORT,'immutable ledger row'); END)SQL",
    R"SQL(CREATE TRIGGER revisions_no_delete BEFORE DELETE ON revisions BEGIN SELECT RAISE(ABORT,'immutable ledger row'); END)SQL",
    R"SQL(CREATE TRIGGER events_no_update BEFORE UPDATE ON events BEGIN SELECT RAISE(ABORT,'immutable ledger row'); END)SQL",
    R"SQL(CREATE TRIGGER events_no_delete BEFORE DELETE ON events BEGIN SELECT RAISE(ABORT,'immutable ledger row'); END)SQL",
    R"SQL(CREATE TRIGGER current_state_no_delete BEFORE DELETE ON current_state BEGIN SELECT RAISE(ABORT,'immutable ledger row'); END)SQL",
    R"SQL(CREATE TRIGGER requests_no_update BEFORE UPDATE ON requests BEGIN SELECT RAISE(ABORT,'immutable ledger row'); END)SQL",
    R"SQL(CREATE TRIGGER requests_no_delete BEFORE DELETE ON requests BEGIN SELECT RAISE(ABORT,'immutable ledger row'); END)SQL",
    R"SQL(CREATE TRIGGER job_activations_no_update BEFORE UPDATE ON job_activations BEGIN SELECT RAISE(ABORT,'immutable ledger row'); END)SQL",
    R"SQL(CREATE TRIGGER job_activations_no_delete BEFORE DELETE ON job_activations BEGIN SELECT RAISE(ABORT,'immutable ledger row'); END)SQL",
    R"SQL(CREATE TRIGGER generations_no_update BEFORE UPDATE ON generations BEGIN SELECT RAISE(ABORT,'immutable ledger row'); END)SQL",
    R"SQL(CREATE TRIGGER generations_no_delete BEFORE DELETE ON generations BEGIN SELECT RAISE(ABORT,'immutable ledger row'); END)SQL",
    R"SQL(CREATE TRIGGER runs_no_update BEFORE UPDATE ON runs BEGIN SELECT RAISE(ABORT,'immutable ledger row'); END)SQL",
    R"SQL(CREATE TRIGGER runs_no_delete BEFORE DELETE ON runs BEGIN SELECT RAISE(ABORT,'immutable ledger row'); END)SQL",
    R"SQL(CREATE TRIGGER dispatches_no_update BEFORE UPDATE ON dispatches BEGIN SELECT RAISE(ABORT,'immutable ledger row'); END)SQL",
    R"SQL(CREATE TRIGGER dispatches_no_delete BEFORE DELETE ON dispatches BEGIN SELECT RAISE(ABORT,'immutable ledger row'); END)SQL",
    R"SQL(CREATE TRIGGER checkpoints_no_update BEFORE UPDATE ON checkpoints BEGIN SELECT RAISE(ABORT,'immutable ledger row'); END)SQL",
    R"SQL(CREATE TRIGGER checkpoints_no_delete BEFORE DELETE ON checkpoints BEGIN SELECT RAISE(ABORT,'immutable ledger row'); END)SQL",
    R"SQL(CREATE TRIGGER reservations_no_update BEFORE UPDATE ON reservations BEGIN SELECT RAISE(ABORT,'immutable ledger row'); END)SQL",
    R"SQL(CREATE TRIGGER reservations_no_delete BEFORE DELETE ON reservations BEGIN SELECT RAISE(ABORT,'immutable ledger row'); END)SQL",
    R"SQL(CREATE TRIGGER evidence_no_update BEFORE UPDATE ON evidence BEGIN SELECT RAISE(ABORT,'immutable ledger row'); END)SQL",
    R"SQL(CREATE TRIGGER evidence_no_delete BEFORE DELETE ON evidence BEGIN SELECT RAISE(ABORT,'immutable ledger row'); END)SQL",
    R"SQL(CREATE TRIGGER completions_no_update BEFORE UPDATE ON completions BEGIN SELECT RAISE(ABORT,'immutable ledger row'); END)SQL",
    R"SQL(CREATE TRIGGER completions_no_delete BEFORE DELETE ON completions BEGIN SELECT RAISE(ABORT,'immutable ledger row'); END)SQL",
    R"SQL(CREATE TRIGGER publications_no_update BEFORE UPDATE ON publications BEGIN SELECT RAISE(ABORT,'immutable ledger row'); END)SQL",
    R"SQL(CREATE TRIGGER publications_no_delete BEFORE DELETE ON publications BEGIN SELECT RAISE(ABORT,'immutable ledger row'); END)SQL",
    R"SQL(CREATE TRIGGER verdicts_no_update BEFORE UPDATE ON verdicts BEGIN SELECT RAISE(ABORT,'immutable ledger row'); END)SQL",
    R"SQL(CREATE TRIGGER verdicts_no_delete BEFORE DELETE ON verdicts BEGIN SELECT RAISE(ABORT,'immutable ledger row'); END)SQL",
    R"SQL(CREATE TRIGGER repairs_no_update BEFORE UPDATE ON repairs BEGIN SELECT RAISE(ABORT,'immutable ledger row'); END)SQL",
    R"SQL(CREATE TRIGGER repairs_no_delete BEFORE DELETE ON repairs BEGIN SELECT RAISE(ABORT,'immutable ledger row'); END)SQL",
    R"SQL(CREATE TRIGGER blockers_no_update BEFORE UPDATE ON blockers BEGIN SELECT RAISE(ABORT,'immutable ledger row'); END)SQL",
    R"SQL(CREATE TRIGGER blockers_no_delete BEFORE DELETE ON blockers BEGIN SELECT RAISE(ABORT,'immutable ledger row'); END)SQL",
    R"SQL(CREATE TRIGGER role_state_no_delete BEFORE DELETE ON role_state BEGIN SELECT RAISE(ABORT,'immutable ledger row'); END)SQL",
    R"SQL(CREATE TRIGGER run_state_no_delete BEFORE DELETE ON run_state BEGIN SELECT RAISE(ABORT,'immutable ledger row'); END)SQL",
    R"SQL(CREATE TRIGGER reservation_state_no_delete BEFORE DELETE ON reservation_state BEGIN SELECT RAISE(ABORT,'immutable ledger row'); END)SQL",
    R"SQL(CREATE TRIGGER blocker_state_no_delete BEFORE DELETE ON blocker_state BEGIN SELECT RAISE(ABORT,'immutable ledger row'); END)SQL",
    R"SQL(CREATE TRIGGER transitions_no_update BEFORE UPDATE ON transitions BEGIN SELECT RAISE(ABORT,'immutable ledger row'); END)SQL",
    R"SQL(CREATE TRIGGER transitions_no_delete BEFORE DELETE ON transitions BEGIN SELECT RAISE(ABORT,'immutable ledger row'); END)SQL"};

struct implicit_index {
  std::string_view name;
  std::string_view table;
};

// Implicit autoindexes of PRIMARY KEY and UNIQUE constraints. sqlite_master keeps their stored sql NULL.
constexpr std::array<implicit_index, 8> implicit_indexes = {{
    {"sqlite_autoindex_tree_edges_1", "tree_edges"},
    {"sqlite_autoindex_tree_edges_2", "tree_edges"},
    {"sqlite_autoindex_requests_1", "requests"},
    {"sqlite_autoindex_requests_2", "requests"},
    {"sqlite_autoindex_reservations_1", "reservations"},
    {"sqlite_autoindex_verdicts_1", "verdicts"},
    {"sqlite_autoindex_role_state_1", "role_state"},
    {"sqlite_autoindex_transitions_1", "transitions"},
}};

// Pragmas that mark a format3 ledger, run inside the initialization transaction.
constexpr std::array<std::string_view, 2> format3_markers = {
    "PRAGMA application_id=1347178034",
    "PRAGMA user_version=3",
};

execution_error code_error(execution_error_code code) {
  execution_error error{};
  error.code = code;
  return error;
}

execution_error from_sqlite(const sqlite_error& error) {
  if (error.code == SQLITE_BUSY || error.code == SQLITE_LOCKED)
    return code_error(execution_error_code::ledger_busy);
  if (error.code == SQLITE_FULL)
    return code_error(execution_error_code::storage_full);
  if (error.code == SQLITE_CORRUPT || error.code == SQLITE_NOTADB)
    return code_error(execution_error_code::ledger_corrupt);
  return code_error(execution_error_code::io);
}

execution_error from_format(const format_error& error) {
  switch (error.code) {
  case format_error_code::busy:
    return code_error(execution_error_code::ledger_busy);
  case format_error_code::storage_full:
    return code_error(execution_error_code::storage_full);
  case format_error_code::corrupt:
    return code_error(execution_error_code::ledger_corrupt);
  case format_error_code::recovery_required:
    return code_error(execution_error_code::recovery_required);
  case format_error_code::unsafe_path:
  case format_error_code::invalid_argument:
    return code_error(execution_error_code::path_denied);
  case format_error_code::not_found:
    return code_error(execution_error_code::ledger_not_found);
  case format_error_code::foreign_ledger:
    return code_error(execution_error_code::foreign_ledger);
  case format_error_code::unsupported_version:
    return code_error(execution_error_code::unsupported_ledger_version);
  default:
    return code_error(execution_error_code::io);
  }
}

// Failure while creating a new target exclusively: an existing target is never replaced.
execution_error create_failure(int error) {
  if (error == EEXIST)
    return code_error(execution_error_code::ledger_exists);
  if (error == ENOENT || error == ENOTDIR)
    return code_error(execution_error_code::path_denied);
  if (error == ENOSPC)
    return code_error(execution_error_code::storage_full);
  return code_error(execution_error_code::io);
}

bool valid_identity(std::string_view text) {
  return text.size() == 32 && std::ranges::all_of(text, [](char c) { return (c >= '0' && c <= '9') || (c >= 'a' && c <= 'f'); });
}

bool valid_workspace(std::string_view text) {
  if (text.empty() || text.size() > 64 || text.front() < 'a' || text.front() > 'z')
    return false;
  return std::ranges::all_of(text, [](char c) { return (c >= 'a' && c <= 'z') || (c >= '0' && c <= '9') || c == '_'; });
}

// Fixed-width UTC RFC3339 timestamp with milliseconds, as native::timestamp writes it.
bool valid_time(std::string_view text) {
  constexpr std::string_view shape = "dddd-dd-ddTdd:dd:dd.dddZ";
  if (text.size() != shape.size())
    return false;
  for (std::size_t i = 0; i < shape.size(); ++i) {
    if (shape[i] == 'd') {
      if (text[i] < '0' || text[i] > '9')
        return false;
    } else if (text[i] != shape[i]) {
      return false;
    }
  }
  return true;
}

std::expected<std::string, execution_error> random_identity() {
  std::array<unsigned char, 16> bytes{};
  if (::getentropy(bytes.data(), bytes.size()) != 0)
    return std::unexpected(code_error(execution_error_code::io));
  constexpr std::string_view digits = "0123456789abcdef";
  std::string                text;
  for (auto byte : bytes) {
    text += digits[byte >> 4];
    text += digits[byte & 15];
  }
  return text;
}

value_ptr make_integer(std::int64_t number) {
  auto node     = std::make_shared<value>();
  node->kind    = value_kind::integer;
  node->integer = number;
  return node;
}

value_ptr make_string(std::string_view text) {
  auto node    = std::make_shared<value>();
  node->kind   = value_kind::string;
  node->string = std::string(text);
  return node;
}

value_ptr make_record(std::map<std::string, value_ptr> fields) {
  auto node    = std::make_shared<value>();
  node->kind   = value_kind::record;
  node->fields = std::move(fields);
  return node;
}

// Canonical payload of the ledger_initialized event: {version:1,before,after,reason:'initialized',references:{}}, with
// the same state shape as the format2 initialization payload.
std::expected<std::string, execution_error> initial_payload() {
  const auto state   = make_record({{"active_revision", make_string("none")}});
  const auto root    = make_record({{"version", make_integer(1)},
                                    {"before", state},
                                    {"after", state},
                                    {"reason", make_string("initialized")},
                                    {"references", make_record({})}});
  auto       encoded = codec_encode(root);
  if (!encoded)
    return std::unexpected(code_error(execution_error_code::io));
  return *encoded;
}

std::expected<void, execution_error> run_statement(connection& db, std::string_view sql) {
  auto executed = native::execute(db, sql);
  if (!executed)
    return std::unexpected(from_format(executed.error()));
  return {};
}

std::expected<void, execution_error> run_prepared(statement& prepared) {
  auto step = prepared.step();
  if (!step)
    return std::unexpected(from_sqlite(step.error()));
  if (*step != step_result::done)
    return std::unexpected(code_error(execution_error_code::io));
  return {};
}

// Read one integer result through the storage helper, which requires exactly one row of a nonzero value.
std::expected<std::int64_t, execution_error> integer_value(connection& db, std::string_view sql) {
  auto value = native::scalar(db, sql);
  if (!value)
    return std::unexpected(from_format(value.error()));
  return *value;
}

// Read one text result of a single-row query and require that the query has no second row.
std::expected<std::string, execution_error> text_value(connection& db, std::string_view sql) {
  auto query = db.prepare(sql);
  if (!query)
    return std::unexpected(from_sqlite(query.error()));
  auto step = query->step();
  if (!step)
    return std::unexpected(from_sqlite(step.error()));
  if (*step != step_result::row)
    return std::unexpected(code_error(execution_error_code::ledger_corrupt));
  auto text = query->text(0);
  if (!text)
    return std::unexpected(from_sqlite(text.error()));
  if (!text->has_value())
    return std::unexpected(code_error(execution_error_code::ledger_corrupt));
  auto done = query->step();
  if (!done)
    return std::unexpected(from_sqlite(done.error()));
  if (*done != step_result::done)
    return std::unexpected(code_error(execution_error_code::ledger_corrupt));
  return **text;
}

// One stored object as sqlite_master lists it.
struct listed_object {
  std::string                type;
  std::string                name;
  std::string                table;
  std::optional<std::string> sql;
};

// The expected identity of one stored object, derived from its statement text.
struct expected_object {
  std::string                     type;
  std::string                     table;
  std::optional<std::string_view> sql;
};

// Object type, name and owning table of one stored statement; the statement shapes are fixed by the contract.
struct statement_identity {
  std::string type;
  std::string name;
  std::string table;
};

statement_identity identify(std::string_view sql_text) {
  std::vector<std::string> words;
  std::string              word;
  for (char c : sql_text) {
    if (c == ' ' || c == '\n' || c == '(') {
      if (!word.empty())
        words.push_back(word);
      word.clear();
    } else {
      word += c;
    }
  }
  if (!word.empty())
    words.push_back(word);
  statement_identity out;
  for (std::size_t i = 0; i + 1 < words.size(); ++i) {
    if (words[i] == "TABLE") {
      out.type  = "table";
      out.name  = words[i + 1];
      out.table = out.name;
      return out;
    }
    if (words[i] == "INDEX" || words[i] == "TRIGGER") {
      out.type = words[i] == "INDEX" ? "index" : "trigger";
      out.name = words[i + 1];
      for (std::size_t j = i + 2; j + 1 < words.size(); ++j) {
        if (words[j] == "ON") {
          out.table = words[j + 1];
          break;
        }
      }
      return out;
    }
  }
  return out;
}

std::map<std::string, expected_object, std::less<>> expected_inventory() {
  std::map<std::string, expected_object, std::less<>> out;
  for (auto sql_text : contract_statements) {
    const auto id = identify(sql_text);
    out.emplace(id.name, expected_object{id.type, id.table, sql_text});
  }
  for (const auto& index : implicit_indexes)
    out.emplace(std::string(index.name), expected_object{"index", std::string(index.table), std::nullopt});
  return out;
}

std::expected<listed_object, execution_error> read_object(const statement& row) {
  auto type  = row.text(0);
  auto name  = row.text(1);
  auto table = row.text(2);
  auto sql   = row.text(3);
  if (!type)
    return std::unexpected(from_sqlite(type.error()));
  if (!name)
    return std::unexpected(from_sqlite(name.error()));
  if (!table)
    return std::unexpected(from_sqlite(table.error()));
  if (!sql)
    return std::unexpected(from_sqlite(sql.error()));
  if (!type->has_value() || !name->has_value() || !table->has_value())
    return std::unexpected(code_error(execution_error_code::ledger_corrupt));
  return listed_object{**type, **name, **table, *sql};
}

// Compare the complete sqlite_master listing with the expected inventory: every listed object must match its expected
// type, owning table and stored sql byte for byte, and every expected object must be listed exactly once.
std::expected<void, execution_error> verify_inventory(connection& db) {
  const auto wanted_objects = expected_inventory();
  auto       query          = db.prepare("SELECT type,name,tbl_name,sql FROM sqlite_master");
  if (!query)
    return std::unexpected(from_sqlite(query.error()));
  std::size_t matched = 0;
  while (true) {
    auto step = query->step();
    if (!step)
      return std::unexpected(from_sqlite(step.error()));
    if (*step == step_result::done)
      break;
    auto listed = read_object(*query);
    if (!listed)
      return std::unexpected(listed.error());
    const auto found = wanted_objects.find(listed->name);
    if (found == wanted_objects.end())
      return std::unexpected(code_error(execution_error_code::ledger_corrupt));
    const auto& wanted = found->second;
    if (wanted.type != listed->type || wanted.table != listed->table)
      return std::unexpected(code_error(execution_error_code::ledger_corrupt));
    if (wanted.sql.has_value() != listed->sql.has_value())
      return std::unexpected(code_error(execution_error_code::ledger_corrupt));
    if (wanted.sql && *wanted.sql != *listed->sql)
      return std::unexpected(code_error(execution_error_code::ledger_corrupt));
    ++matched;
  }
  if (matched != wanted_objects.size())
    return std::unexpected(code_error(execution_error_code::ledger_corrupt));
  return {};
}

// Verify the single ledger_meta row: singleton 1, format 3, identity, workspace identifier and creation time grammars.
std::expected<execution_metadata, execution_error> verify_metadata(connection& db) {
  auto rows = integer_value(db, "SELECT count(*) FROM ledger_meta");
  if (!rows)
    return std::unexpected(rows.error());
  if (*rows != 1)
    return std::unexpected(code_error(execution_error_code::ledger_corrupt));
  auto query = db.prepare("SELECT singleton,format,identity,workspace_id,created_at FROM ledger_meta");
  if (!query)
    return std::unexpected(from_sqlite(query.error()));
  auto step = query->step();
  if (!step)
    return std::unexpected(from_sqlite(step.error()));
  if (*step != step_result::row)
    return std::unexpected(code_error(execution_error_code::ledger_corrupt));
  auto singleton = query->integer(0);
  auto format    = query->integer(1);
  auto identity  = query->text(2);
  auto workspace = query->text(3);
  auto created   = query->text(4);
  if (!singleton)
    return std::unexpected(from_sqlite(singleton.error()));
  if (!format)
    return std::unexpected(from_sqlite(format.error()));
  if (!identity)
    return std::unexpected(from_sqlite(identity.error()));
  if (!workspace)
    return std::unexpected(from_sqlite(workspace.error()));
  if (!created)
    return std::unexpected(from_sqlite(created.error()));
  if (!singleton->has_value() || **singleton != 1 || !format->has_value() || **format != format3_format)
    return std::unexpected(code_error(execution_error_code::ledger_corrupt));
  if (!identity->has_value() || !workspace->has_value() || !created->has_value())
    return std::unexpected(code_error(execution_error_code::ledger_corrupt));
  if (!valid_identity(**identity) || !valid_workspace(**workspace) || !valid_time(**created))
    return std::unexpected(code_error(execution_error_code::ledger_corrupt));
  return execution_metadata{**identity, **workspace, **created};
}

// Verify the header-independent format3 markers, the storage policy and the exact schema on an open connection.
std::expected<execution_metadata, execution_error> verify_connection_schema(connection& db) {
  auto configured = native::settings(db, false);
  if (!configured)
    return std::unexpected(from_format(configured.error()));
  auto version = integer_value(db, "PRAGMA user_version");
  if (!version)
    return std::unexpected(version.error());
  if (*version != format3_user_version)
    return std::unexpected(code_error(execution_error_code::ledger_corrupt));
  auto application = integer_value(db, "PRAGMA application_id");
  if (!application)
    return std::unexpected(application.error());
  if (*application != format3_application_id)
    return std::unexpected(code_error(execution_error_code::ledger_corrupt));
  auto encoding = text_value(db, "PRAGMA encoding");
  if (!encoding)
    return std::unexpected(encoding.error());
  if (*encoding != "UTF-8")
    return std::unexpected(code_error(execution_error_code::ledger_corrupt));
  auto journal = text_value(db, "PRAGMA journal_mode");
  if (!journal)
    return std::unexpected(journal.error());
  if (*journal != "delete")
    return std::unexpected(code_error(execution_error_code::ledger_corrupt));
  auto inventory = verify_inventory(db);
  if (!inventory)
    return std::unexpected(inventory.error());
  return verify_metadata(db);
}

// Settings and a read transaction around the schema verification, so the verified state is one consistent snapshot.
std::expected<execution_metadata, execution_error> verify_opened(connection& db) {
  auto configured = native::settings(db);
  if (!configured)
    return std::unexpected(from_format(configured.error()));
  auto tx = db.begin(false);
  if (!tx)
    return std::unexpected(from_sqlite(tx.error()));
  return verify_connection_schema(db);
}

// Create the ledger inside one write transaction; the caller has already created the empty target exclusively.
std::expected<execution_metadata, execution_error> create_ledger(const native::location& where, const std::string& identity,
                                                                 std::string_view workspace_id) {
  const auto created_at = native::timestamp();
  auto       opened     = connection::open(where.path, open_mode::read_write);
  if (!opened)
    return std::unexpected(from_sqlite(opened.error()));
  connection db         = std::move(*opened);
  auto       configured = native::settings(db);
  if (!configured)
    return std::unexpected(from_format(configured.error()));
  // Persistent settings are confined to this exclusively owned new ledger, before the write transaction begins.
  auto mode = text_value(db, "PRAGMA journal_mode=DELETE");
  if (!mode)
    return std::unexpected(mode.error());
  if (*mode != "delete")
    return std::unexpected(code_error(execution_error_code::io));
  if (auto encoding = run_statement(db, "PRAGMA encoding='UTF-8'"); !encoding)
    return std::unexpected(encoding.error());
  auto payload = initial_payload();
  if (!payload)
    return std::unexpected(payload.error());
  auto tx = db.begin(true);
  if (!tx)
    return std::unexpected(from_sqlite(tx.error()));
  for (auto sql : format3_markers)
    if (auto marked = run_statement(db, sql); !marked)
      return std::unexpected(marked.error());
  for (auto sql : contract_statements)
    if (auto created = run_statement(db, sql); !created)
      return std::unexpected(created.error());

  auto meta = db.prepare("INSERT INTO ledger_meta(singleton,format,identity,workspace_id,created_at) VALUES(1,3,?1,?2,?3)");
  if (!meta)
    return std::unexpected(from_sqlite(meta.error()));
  if (auto bound = meta->bind_text(1, identity); !bound)
    return std::unexpected(from_sqlite(bound.error()));
  if (auto bound = meta->bind_text(2, workspace_id); !bound)
    return std::unexpected(from_sqlite(bound.error()));
  if (auto bound = meta->bind_text(3, created_at); !bound)
    return std::unexpected(from_sqlite(bound.error()));
  if (auto inserted = run_prepared(*meta); !inserted)
    return std::unexpected(inserted.error());

  auto event = db.prepare("INSERT INTO events(seq,type,version,revision_id,request_id,actor,occurred_at,payload,job,role,"
                          "generation_id,run_id) VALUES(1,'ledger_initialized',1,NULL,NULL,?1,?2,?3,NULL,NULL,NULL,NULL)");
  if (!event)
    return std::unexpected(from_sqlite(event.error()));
  if (auto bound = event->bind_text(1, init_actor); !bound)
    return std::unexpected(from_sqlite(bound.error()));
  if (auto bound = event->bind_text(2, created_at); !bound)
    return std::unexpected(from_sqlite(bound.error()));
  if (auto bound = event->bind_blob(3, *payload); !bound)
    return std::unexpected(from_sqlite(bound.error()));
  if (auto inserted = run_prepared(*event); !inserted)
    return std::unexpected(inserted.error());

  if (auto state =
          run_statement(db, "INSERT INTO current_state(singleton,active_revision_id,last_seq,active_job,active_activation_id) "
                            "VALUES(1,NULL,1,NULL,NULL)");
      !state)
    return std::unexpected(state.error());
  auto committed = tx->commit();
  if (!committed)
    return std::unexpected(from_sqlite(committed.error()));
  return execution_metadata{identity, std::string(workspace_id), created_at};
}

// Remove the target only when it is still the file this call created, so a concurrent replacement is never unlinked.
void remove_owned(const native::location& where, const struct stat& made) {
  struct stat named{};
  if (::fstatat(where.parent.fd, where.name.c_str(), &named, AT_SYMLINK_NOFOLLOW) == 0 && native::same(named, made))
    ::unlinkat(where.parent.fd, where.name.c_str(), 0);
}

// Raw header checks run before any SQLite open: magic, application id and the exact user version 3.
std::expected<void, execution_error> check_header(int fd) {
  struct stat st{};
  if (::fstat(fd, &st) != 0)
    return std::unexpected(code_error(execution_error_code::io));
  if (st.st_size < 512)
    return std::unexpected(code_error(execution_error_code::foreign_ledger));
  std::array<unsigned char, 100> bytes{};
  if (::pread(fd, bytes.data(), bytes.size(), 0) != static_cast<ssize_t>(bytes.size()))
    return std::unexpected(code_error(execution_error_code::foreign_ledger));
  if (std::string_view(reinterpret_cast<const char*>(bytes.data()), sqlite_magic.size()) != sqlite_magic)
    return std::unexpected(code_error(execution_error_code::foreign_ledger));
  auto u32 = [&](std::size_t i) {
    return (std::uint32_t(bytes[i]) << 24) | (std::uint32_t(bytes[i + 1]) << 16) | (std::uint32_t(bytes[i + 2]) << 8) |
           std::uint32_t(bytes[i + 3]);
  };
  if (u32(68) != format3_application_id)
    return std::unexpected(code_error(execution_error_code::foreign_ledger));
  if (u32(60) != format3_user_version)
    return std::unexpected(code_error(execution_error_code::unsupported_ledger_version));
  return {};
}

} // namespace

/// Private state of an open execution database: the verified metadata and the owning connection.
struct execution_database::impl {
  execution_metadata        metadata; ///< Metadata verified when the database was opened.
  std::optional<connection> database; ///< The owned connection, absent after it is moved out.
};

execution_database::execution_database(execution_database&&) noexcept            = default;
execution_database& execution_database::operator=(execution_database&&) noexcept = default;
execution_database::~execution_database()                                        = default;

execution_database::execution_database(std::unique_ptr<impl> value) noexcept : _impl(std::move(value)) {
}

const execution_metadata& execution_database::metadata() const {
  return _impl->metadata;
}

connection& execution_database::sql() {
  return _impl->database.value();
}

std::expected<execution_metadata, execution_error> initialize_format3(const std::filesystem::path& path,
                                                                      std::string_view             workspace_id) {
  if (!valid_workspace(workspace_id))
    return std::unexpected(code_error(execution_error_code::invalid_change));
  auto where = native::locate(path);
  if (!where)
    return std::unexpected(from_format(where.error()));
  auto identity = random_identity();
  if (!identity)
    return std::unexpected(identity.error());
  // Exclusive creation refuses an existing target without touching it; the descriptor is closed before SQLite opens it.
  struct stat made{};
  {
    native::descriptor created(
        ::openat(where->parent.fd, where->name.c_str(), O_RDWR | O_CREAT | O_EXCL | O_NOFOLLOW | O_CLOEXEC, 0600));
    if (created.fd < 0)
      return std::unexpected(create_failure(errno));
    if (::fstat(created.fd, &made) != 0 || !S_ISREG(made.st_mode))
      return std::unexpected(code_error(execution_error_code::io));
  }
  auto result = create_ledger(*where, *identity, workspace_id);
  if (!result) {
    remove_owned(*where, made);
    return std::unexpected(result.error());
  }
  return result;
}

std::expected<execution_database, execution_error> open_format3(const std::filesystem::path& path, execution_access access) {
  auto where = native::locate(path);
  if (!where)
    return std::unexpected(from_format(where.error()));
  native::descriptor held(::openat(where->parent.fd, where->name.c_str(), O_RDONLY | O_NOFOLLOW | O_CLOEXEC | O_NONBLOCK));
  if (held.fd < 0)
    return std::unexpected(
        code_error(errno == ENOENT ? execution_error_code::ledger_not_found : execution_error_code::path_denied));
  struct stat identity{};
  if (::fstat(held.fd, &identity) != 0 || !S_ISREG(identity.st_mode))
    return std::unexpected(code_error(execution_error_code::path_denied));
  if (auto header = check_header(held.fd); !header)
    return std::unexpected(header.error());
  if (auto sidecars = native::check_orphan_sidecars(*where, held.fd); !sidecars)
    return std::unexpected(from_format(sidecars.error()));
  if (!native::identity_matches(*where, identity))
    return std::unexpected(code_error(execution_error_code::path_denied));

  std::optional<connection> db;
  {
    auto read_only = connection::open(where->path, open_mode::read_only);
    if (!read_only)
      return std::unexpected(from_sqlite(read_only.error()));
    db.emplace(std::move(*read_only));
  }
  auto metadata = verify_opened(*db);
  if (!metadata)
    return std::unexpected(metadata.error());
  if (access == execution_access::read_write) {
    // Release the read-only connection, repeat the sidecar and identity checks, then verify the writable connection.
    db.reset();
    if (auto sidecars = native::check_orphan_sidecars(*where, held.fd); !sidecars)
      return std::unexpected(from_format(sidecars.error()));
    if (!native::identity_matches(*where, identity))
      return std::unexpected(code_error(execution_error_code::path_denied));
    auto writable = connection::open(where->path, open_mode::read_write);
    if (!writable)
      return std::unexpected(from_sqlite(writable.error()));
    db.emplace(std::move(*writable));
    metadata = verify_opened(*db);
    if (!metadata)
      return std::unexpected(metadata.error());
  }
  auto made      = std::make_unique<execution_database::impl>();
  made->metadata = std::move(*metadata);
  made->database.emplace(std::move(*db));
  return execution_database(std::move(made));
}

std::expected<execution_metadata, execution_error> verify_format3_schema(connection& db) {
  return verify_connection_schema(db);
}

} // namespace planar::engine::ledger
