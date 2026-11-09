// Format3 execution schema checks: stored SQL, object inventory, immutability, initialization, opener refusals, and the
// storage sidecar export. Expected SQL text and the table, index and trigger names are independent literals copied from
// execution-contract.md EX-1; the product exports none of them. Every ledger lives under a per-test scratch root.
#include <catch2/catch_test_macros.hpp>
#include <fcntl.h>
#include <sqlite3.h>
#include <stdlib.h>
#include <unistd.h>
import std;
import planar.engine.ledger.execution_schema;
import planar.engine.ledger.format;
import planar.engine.ledger.sqlite;
import planar.engine.ledger.storage;

using namespace planar::engine::ledger;

namespace {

constexpr std::string_view workspace_id        = "test_workspace";
constexpr std::int64_t     format3_application = 1347178034;

struct scratch {
  std::filesystem::path root;
  scratch() {
    char  pattern[] = "/tmp/planar-execution-schema-XXXXXX";
    auto* made      = ::mkdtemp(pattern);
    REQUIRE(made);
    root = std::filesystem::canonical(made);
  }
  ~scratch() {
    std::filesystem::remove_all(root);
  }
};

// Verbatim copy of the 69 stored statements of execution-contract.md EX-1 (SQL block lines 82-355, Reserved comments
// removed), in contract creation order: 24 tables, 2 explicit indexes, then 43 triggers.
const std::array<std::string_view, 69> contract_statements = {
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

// Immutable tables: the names of the contract triggers `<table>_no_update` and `<table>_no_delete` (EX-1 lines 313-353).
constexpr std::array<std::string_view, 19> immutable_tables = {
    "ledger_meta",  "records",  "tree_edges", "revisions",   "events",       "requests", "job_activations",
    "generations",  "runs",     "dispatches", "checkpoints", "reservations", "evidence", "completions",
    "publications", "verdicts", "repairs",    "blockers",    "transitions"};

// Mutable projections: only the `<table>_no_delete` trigger exists for each (EX-1 lines 350-355).
constexpr std::array<std::string_view, 5> mutable_tables = {"current_state", "role_state", "run_state", "reservation_state",
                                                            "blocker_state"};

// The 24 created tables in creation order (EX-1 CREATE TABLE statements).
constexpr std::array<std::string_view, 24> contract_tables = {
    "ledger_meta",       "records",       "tree_edges", "revisions",  "events",      "current_state", "requests",
    "job_activations",   "generations",   "runs",       "dispatches", "checkpoints", "reservations",  "evidence",
    "completions",       "publications",  "verdicts",   "repairs",    "blockers",    "role_state",    "run_state",
    "reservation_state", "blocker_state", "transitions"};

// The two explicit indexes (EX-1: records_digest and run_state_one_active).
constexpr std::array<std::string_view, 2> explicit_indexes = {"records_digest", "run_state_one_active"};

// Implicit autoindexes created by PRIMARY KEY and UNIQUE constraints; their stored sql is NULL.
constexpr std::array<std::string_view, 8> implicit_indexes = {
    "sqlite_autoindex_tree_edges_1", "sqlite_autoindex_tree_edges_2",   "sqlite_autoindex_requests_1",
    "sqlite_autoindex_requests_2",   "sqlite_autoindex_reservations_1", "sqlite_autoindex_verdicts_1",
    "sqlite_autoindex_role_state_1", "sqlite_autoindex_transitions_1"};

struct insert_case {
  std::string_view table;
  std::string_view sql;
};

// One minimal row per immutable table so each UPDATE and DELETE reaches its trigger. ledger_meta and events already hold
// the initialization rows. Foreign keys are switched off for these raw rows; CHECK constraints still apply.
constexpr std::array<insert_case, 17> immutable_inserts = {{
    {"records", "INSERT INTO records(id,kind,codec,digest,encoded) VALUES(1,1,1,zeroblob(16),x'00')"},
    {"tree_edges", "INSERT INTO tree_edges(parent_id,ordinal,name,child_id) VALUES(2,0,'a',1)"},
    {"revisions",
     "INSERT INTO revisions(id,parent_id,source_id,registry_id,stdlib_id,interpretation_id,versions_id,actor,accepted_at) "
     "VALUES(1,NULL,1,1,1,1,1,'planar:test','2026-11-07T00:00:00.000Z')"},
    {"requests", "INSERT INTO requests(request_id,operation,body_id,event_seq,revision_id,outcome,result,attachment_id) "
                 "VALUES('r1','load',1,1,1,'accepted',x'00',NULL)"},
    {"job_activations", "INSERT INTO job_activations(id,job,revision_id,event_seq) VALUES(1,'j',1,1)"},
    {"generations",
     "INSERT INTO generations(id,job,role,ordinal,structure_id,opened_seq,reason) VALUES(1,'j','r',1,1,1,'activated')"},
    {"runs",
     "INSERT INTO runs(id,generation_id,request_id,claimed_seq,secret,provenance_id) VALUES(1,1,'r1',1,zeroblob(16),NULL)"},
    {"dispatches",
     "INSERT INTO dispatches(id,run_id,seq,revision_id,kind,packet_id,manifest_id,work_id,work_fp,render_fp,checkpoint_id) "
     "VALUES(1,1,1,1,'claim',1,1,1,zeroblob(16),zeroblob(16),NULL)"},
    {"checkpoints", "INSERT INTO checkpoints(id,run_id,seq,body_id) VALUES(1,1,1,1)"},
    {"reservations", "INSERT INTO reservations(id,request_id,run_id,opened_seq,body_id) VALUES(1,'r1',1,1,1)"},
    {"evidence",
     "INSERT INTO evidence(id,reservation_id,seq,work,\"check\",definition_id,vector_id,outcome,reason,matched,exit_status,"
     "duration_ms,output_id,output_bytes,output_truncated) VALUES(1,1,1,'w','c',1,1,'pass','ok',NULL,NULL,0,NULL,0,0)"},
    {"completions", "INSERT INTO completions(id,run_id,seq,dispatch_id,work_fp,observation_id) VALUES(1,1,1,1,zeroblob(16),1)"},
    {"publications",
     "INSERT INTO publications(id,completion_id,work,facet,semantic,payload_id) VALUES(1,1,'w','f',zeroblob(16),1)"},
    {"verdicts",
     "INSERT INTO verdicts(id,request_id,run_id,seq,body_id,applicability,decision) VALUES(1,'r1',1,1,1,'current','approve')"},
    {"repairs", "INSERT INTO repairs(verdict_id,job,role,generation_id,revision) VALUES(1,'j','r',1,1)"},
    {"blockers", "INSERT INTO blockers(id,seq,job,kind,verdict_id) VALUES(1,1,'j','max_rounds',1)"},
    {"transitions",
     "INSERT INTO "
     "transitions(event_seq,ordinal,job,role,generation_id,before_state,after_state,before_drifted,after_drifted,reason) "
     "VALUES(1,1,'j','r',1,NULL,'ready',NULL,0,'activated')"},
}};

// One minimal row for each mutable projection that starts empty. current_state already holds its initialization row.
constexpr std::array<insert_case, 4> mutable_inserts = {{
    {"role_state", "INSERT INTO role_state(job,role,generation_id,state,drifted,run_id,fact_version,last_seq) "
                   "VALUES('j','r',1,'ready',0,NULL,1,1)"},
    {"run_state", "INSERT INTO run_state(run_id,job,role,generation_id,state,last_seq) VALUES(1,'j','r',1,'released',1)"},
    {"reservation_state", "INSERT INTO reservation_state(reservation_id,state,last_seq) VALUES(1,'open',1)"},
    {"blocker_state", "INSERT INTO blocker_state(blocker_id,state,cleared_seq,last_seq) VALUES(1,'open',NULL,1)"},
}};

// Name of a CREATE TABLE, CREATE INDEX or CREATE TRIGGER statement: the word after the object keyword.
std::string object_name(std::string_view statement) {
  std::vector<std::string> words;
  std::string              word;
  for (char c : statement) {
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
  for (std::size_t i = 0; i + 1 < words.size(); ++i)
    if (words[i] == "TABLE" || words[i] == "INDEX" || words[i] == "TRIGGER")
      return words[i + 1];
  return {};
}

std::vector<std::string> sorted_names(const auto& names) {
  std::vector<std::string> out;
  for (auto name : names)
    out.emplace_back(name);
  std::ranges::sort(out);
  return out;
}

// Names of the verbatim statement copy whose text starts with keyword, for example "CREATE TRIGGER".
std::vector<std::string> names_of_statements(std::string_view keyword) {
  std::vector<std::string> out;
  for (auto statement : contract_statements)
    if (statement.starts_with(keyword))
      out.push_back(object_name(statement));
  return out;
}

std::string refusal_message(const std::expected<void, sqlite_error>& result) {
  return result ? std::string() : result.error().message;
}

std::string outcome_text(const std::expected<void, sqlite_error>& result) {
  return result ? std::string("accepted") : "refused: " + result.error().message;
}

std::string bytes(const std::filesystem::path& path) {
  std::ifstream in(path, std::ios::binary);
  return {std::istreambuf_iterator<char>(in), {}};
}

std::map<std::string, std::string> snapshot(const std::filesystem::path& root) {
  std::map<std::string, std::string> out;
  for (const auto& entry : std::filesystem::recursive_directory_iterator(root)) {
    const auto name = entry.path().lexically_relative(root).string();
    if (entry.is_symlink())
      out[name] = "link:" + std::filesystem::read_symlink(entry.path()).string();
    else if (entry.is_directory())
      out[name] = "directory";
    else
      out[name] = bytes(entry.path());
  }
  return out;
}

std::filesystem::path sidecar(const std::filesystem::path& path, std::string_view suffix) {
  return std::filesystem::path(path.string() + std::string(suffix));
}

std::filesystem::path initialized(const scratch& f, std::string_view name) {
  const auto path = f.root / name;
  auto       made = initialize_format3(path, workspace_id);
  REQUIRE(made);
  return path;
}

connection raw_connection(const std::filesystem::path& path, open_mode mode) {
  auto db = connection::open(path, mode);
  REQUIRE(db);
  return std::move(*db);
}

// Run one statement to completion and report its refusal, if any.
std::expected<void, sqlite_error> exec_sql(connection& db, std::string_view text) {
  auto statement = db.prepare(text);
  if (!statement)
    return std::unexpected(statement.error());
  while (true) {
    auto step = statement->step();
    if (!step)
      return std::unexpected(step.error());
    if (*step == step_result::done)
      return {};
  }
}

std::optional<std::int64_t> integer_at(connection& db, std::string_view text) {
  auto statement = db.prepare(text);
  REQUIRE(statement);
  REQUIRE(statement->step() == step_result::row);
  auto value = statement->integer(0);
  REQUIRE(value);
  return *value;
}

std::optional<std::string> text_at(connection& db, std::string_view text) {
  auto statement = db.prepare(text);
  REQUIRE(statement);
  REQUIRE(statement->step() == step_result::row);
  auto value = statement->text(0);
  REQUIRE(value);
  return *value;
}

std::optional<std::string> blob_at(connection& db, std::string_view text) {
  auto statement = db.prepare(text);
  REQUIRE(statement);
  REQUIRE(statement->step() == step_result::row);
  auto value = statement->blob(0);
  REQUIRE(value);
  return *value;
}

std::int64_t row_count(connection& db, std::string_view table) {
  auto count = integer_at(db, std::string("SELECT count(*) FROM ") + std::string(table));
  REQUIRE(count);
  return *count;
}

struct listed_object {
  std::string                type;
  std::string                name;
  std::string                table;
  std::optional<std::string> sql;
};

std::vector<listed_object> schema_rows(connection& db) {
  auto query = db.prepare("SELECT type,name,tbl_name,sql FROM sqlite_master ORDER BY rowid");
  REQUIRE(query);
  std::vector<listed_object> rows;
  while (true) {
    auto step = query->step();
    REQUIRE(step);
    if (*step == step_result::done)
      break;
    auto type  = query->text(0);
    auto name  = query->text(1);
    auto table = query->text(2);
    auto sql   = query->text(3);
    REQUIRE(type);
    REQUIRE(name);
    REQUIRE(table);
    REQUIRE(sql);
    rows.push_back(listed_object{**type, **name, **table, *sql});
  }
  return rows;
}

// Every ledger must verify first, so a later refusal is caused by the one corruption under test.
void require_opens(const std::filesystem::path& path) {
  for (auto access : {execution_access::read_only, execution_access::read_write}) {
    auto opened = open_format3(path, access);
    REQUIRE(opened);
  }
}

void require_refused(const std::filesystem::path& path, execution_error_code wanted) {
  for (auto access : {execution_access::read_only, execution_access::read_write}) {
    auto opened = open_format3(path, access);
    REQUIRE_FALSE(opened);
    CHECK(opened.error().code == wanted);
  }
}

// Replace one occurrence in a copy of stored SQL and return the edited copy and the first differing offset.
struct one_byte_edit {
  std::string edited;
  std::size_t first_difference;
  std::size_t differing_bytes;
};

one_byte_edit edit_once(const std::string& original, std::string_view old_text, std::string_view new_text) {
  const auto position = original.find(old_text);
  REQUIRE(position != std::string::npos);
  REQUIRE(original.find(old_text, position + 1) == std::string::npos);
  REQUIRE(old_text.size() == new_text.size());
  one_byte_edit result{original, 0, 0};
  result.edited.replace(position, old_text.size(), new_text);
  for (std::size_t i = 0; i < original.size(); ++i)
    if (original[i] != result.edited[i]) {
      if (result.differing_bytes == 0)
        result.first_difference = i;
      ++result.differing_bytes;
    }
  return result;
}

} // namespace

TEST_CASE("format3 schema stored SQL equals the contract text", "[ledger][execution_schema]") {
  scratch                  f;
  const auto               path = initialized(f, "ledger");
  auto                     db   = raw_connection(path, open_mode::read_only);
  std::vector<std::string> stored;
  for (const auto& row : schema_rows(db))
    if (row.sql)
      stored.push_back(*row.sql);
  REQUIRE(stored.size() == contract_statements.size());
  for (std::size_t i = 0; i < stored.size(); ++i) {
    INFO("statement " << i);
    CHECK(stored[i] == contract_statements[i]);
  }
}

TEST_CASE("format3 schema object inventory", "[ledger][execution_schema]") {
  scratch    f;
  const auto path = initialized(f, "ledger");
  auto       db   = raw_connection(path, open_mode::read_only);
  const auto rows = schema_rows(db);

  std::vector<std::string> tables, explicit_index, implicit_index, triggers, other;
  for (const auto& row : rows) {
    if (row.type == "table")
      tables.push_back(row.name);
    else if (row.type == "index" && row.sql)
      explicit_index.push_back(row.name);
    else if (row.type == "index" && !row.sql)
      implicit_index.push_back(row.name);
    else if (row.type == "trigger")
      triggers.push_back(row.name);
    else
      other.push_back(row.name);
  }

  // Independent literal expectations, cross-checked against the verbatim statement copy.
  CHECK(sorted_names(contract_tables) == sorted_names(names_of_statements("CREATE TABLE")));
  CHECK(names_of_statements("CREATE INDEX") == std::vector<std::string>{"records_digest"});
  CHECK(names_of_statements("CREATE UNIQUE INDEX") == std::vector<std::string>{"run_state_one_active"});
  REQUIRE(rows.size() == 24 + 2 + 8 + 43);
  CHECK(other.empty());
  CHECK(sorted_names(tables) == sorted_names(contract_tables));
  CHECK(sorted_names(explicit_index) == sorted_names(explicit_indexes));
  CHECK(sorted_names(implicit_index) == sorted_names(implicit_indexes));
  for (const auto& name : implicit_index)
    CHECK(name.starts_with("sqlite_autoindex_"));
  CHECK(triggers.size() == 43);
  CHECK(sorted_names(triggers) == sorted_names(names_of_statements("CREATE TRIGGER")));
}

TEST_CASE("format3 immutable tables refuse update and delete", "[ledger][execution_schema]") {
  scratch    f;
  const auto path = initialized(f, "ledger");
  auto       db   = raw_connection(path, open_mode::read_write);
  REQUIRE(exec_sql(db, "PRAGMA foreign_keys=OFF"));
  for (const auto& insert : immutable_inserts) {
    INFO(std::string(insert.table));
    REQUIRE(exec_sql(db, insert.sql));
  }
  for (auto table : immutable_tables) {
    INFO(std::string(table));
    REQUIRE(row_count(db, table) >= 1);
    const auto update = exec_sql(db, std::string("UPDATE ") + std::string(table) + " SET rowid=rowid");
    const auto remove = exec_sql(db, std::string("DELETE FROM ") + std::string(table));
    INFO(outcome_text(update));
    CHECK(refusal_message(update).find("immutable ledger row") != std::string::npos);
    INFO(outcome_text(remove));
    CHECK(refusal_message(remove).find("immutable ledger row") != std::string::npos);
  }
}

TEST_CASE("format3 mutable projections allow update and refuse delete", "[ledger][execution_schema]") {
  scratch    f;
  const auto path = initialized(f, "ledger");
  auto       db   = raw_connection(path, open_mode::read_write);
  REQUIRE(exec_sql(db, "PRAGMA foreign_keys=OFF"));
  for (const auto& insert : mutable_inserts)
    REQUIRE(exec_sql(db, insert.sql));
  for (auto table : mutable_tables) {
    INFO(std::string(table));
    REQUIRE(row_count(db, table) >= 1);
    const auto update = exec_sql(db, std::string("UPDATE ") + std::string(table) + " SET rowid=rowid");
    const auto remove = exec_sql(db, std::string("DELETE FROM ") + std::string(table));
    INFO(outcome_text(update));
    CHECK(update.has_value());
    INFO(outcome_text(remove));
    CHECK(refusal_message(remove).find("immutable ledger row") != std::string::npos);
  }
}

TEST_CASE("format3 initialization markers", "[ledger][execution_schema]") {
  scratch    f;
  const auto path = f.root / "ledger";
  auto       made = initialize_format3(path, workspace_id);
  REQUIRE(made);
  {
    auto db = raw_connection(path, open_mode::read_only);
    CHECK(integer_at(db, "PRAGMA user_version") == 3);
    CHECK(integer_at(db, "PRAGMA application_id") == format3_application);
    CHECK(integer_at(db, "SELECT singleton FROM ledger_meta") == 1);
    CHECK(integer_at(db, "SELECT format FROM ledger_meta") == 3);
    const auto identity = text_at(db, "SELECT identity FROM ledger_meta");
    REQUIRE(identity);
    CHECK(identity->size() == 32);
    CHECK(std::ranges::all_of(*identity, [](char c) { return (c >= '0' && c <= '9') || (c >= 'a' && c <= 'f'); }));
    CHECK(text_at(db, "SELECT workspace_id FROM ledger_meta") == std::string(workspace_id));
    const auto created = text_at(db, "SELECT created_at FROM ledger_meta");
    REQUIRE(created);
    CHECK(created->size() == 24);
    CHECK(made->identity == *identity);
    CHECK(made->workspace_id == std::string(workspace_id));
    CHECK(made->created_at == *created);
  }
  // The foreign key setting is per connection; the opener's own connection carries the product settings.
  auto opened = open_format3(path, execution_access::read_only);
  REQUIRE(opened);
  CHECK(integer_at(opened->sql(), "PRAGMA foreign_keys") == 1);
}

TEST_CASE("format3 initialization writes the initial event and state", "[ledger][execution_schema]") {
  scratch    f;
  const auto path   = f.root / "ledger";
  const auto oracle = f.root / "format2-oracle";
  REQUIRE(initialize_format3(path, workspace_id));
  REQUIRE(initialize_new_v2(oracle, workspace_id));
  std::string oracle_payload;
  std::string oracle_actor;
  {
    auto source    = raw_connection(oracle, open_mode::read_only);
    oracle_payload = blob_at(source, "SELECT payload FROM events WHERE seq=1").value_or("");
    oracle_actor   = text_at(source, "SELECT actor FROM events WHERE seq=1").value_or("");
  }
  auto db = raw_connection(path, open_mode::read_only);
  CHECK(row_count(db, "events") == 1);
  CHECK(integer_at(db, "SELECT seq FROM events") == 1);
  CHECK(text_at(db, "SELECT type FROM events") == std::string("ledger_initialized"));
  CHECK(integer_at(db, "SELECT version FROM events") == 1);
  CHECK(integer_at(db, "SELECT revision_id FROM events") == std::nullopt);
  CHECK(text_at(db, "SELECT request_id FROM events") == std::nullopt);
  CHECK(text_at(db, "SELECT job FROM events") == std::nullopt);
  CHECK(text_at(db, "SELECT role FROM events") == std::nullopt);
  CHECK(integer_at(db, "SELECT generation_id FROM events") == std::nullopt);
  CHECK(integer_at(db, "SELECT run_id FROM events") == std::nullopt);
  CHECK(text_at(db, "SELECT actor FROM events") == oracle_actor);
  // Version 1 keeps the format2 payload, whose references are {}.
  CHECK(blob_at(db, "SELECT payload FROM events WHERE seq=1") == oracle_payload);
  CHECK(oracle_payload.find("\"references\":{}") != std::string::npos);

  CHECK(row_count(db, "current_state") == 1);
  CHECK(integer_at(db, "SELECT singleton FROM current_state") == 1);
  CHECK(integer_at(db, "SELECT active_revision_id FROM current_state") == std::nullopt);
  CHECK(integer_at(db, "SELECT last_seq FROM current_state") == 1);
  CHECK(text_at(db, "SELECT active_job FROM current_state") == std::nullopt);
  CHECK(integer_at(db, "SELECT active_activation_id FROM current_state") == std::nullopt);

  for (auto table : contract_tables) {
    if (table == "ledger_meta" || table == "events" || table == "current_state")
      continue;
    INFO(std::string(table));
    CHECK(row_count(db, table) == 0);
  }
}

TEST_CASE("format3 initialization refuses an existing target", "[ledger][execution_schema]") {
  scratch    f;
  const auto path   = initialized(f, "ledger");
  const auto before = snapshot(f.root);
  const auto stored = bytes(path);
  auto       again  = initialize_format3(path, workspace_id);
  REQUIRE_FALSE(again);
  CHECK(again.error().code == execution_error_code::ledger_exists);
  CHECK(snapshot(f.root) == before);
  CHECK(bytes(path) == stored);

  auto missing_parent = initialize_format3(f.root / "missing" / "ledger", workspace_id);
  REQUIRE_FALSE(missing_parent);
  CHECK(missing_parent.error().code == execution_error_code::path_denied);
  CHECK(snapshot(f.root) == before);
}

TEST_CASE("format3 opener verifies a fresh ledger", "[ledger][execution_schema]") {
  scratch    f;
  const auto path = f.root / "ledger";
  auto       made = initialize_format3(path, workspace_id);
  REQUIRE(made);
  const auto stored = bytes(path);
  for (auto access : {execution_access::read_only, execution_access::read_write}) {
    const std::string mode = access == execution_access::read_only ? "read_only" : "read_write";
    INFO(mode);
    auto opened = open_format3(path, access);
    REQUIRE(opened);
    CHECK(opened->metadata().identity == made->identity);
    CHECK(opened->metadata().workspace_id == made->workspace_id);
    CHECK(opened->metadata().created_at == made->created_at);
    CHECK(bytes(path) == stored);
  }
  CHECK(bytes(path) == stored);
}

TEST_CASE("format3 opener refuses a one byte stored SQL difference", "[ledger][execution_schema]") {
  // Each edit changes exactly one byte of the stored sql of one object, with writable_schema, and keeps the text parseable.
  // Each differing byte lies past offset 40, so a comparison limited to the first 40 bytes cannot see it.
  struct edit_case {
    std::string_view object;
    std::string_view old_text;
    std::string_view new_text;
  };
  constexpr std::array<edit_case, 4> cases = {{
      {"events", "payload BLOB NOT NULL", "Payload BLOB NOT NULL"}, // table
      {"records_digest", "kind,digest)", "kind,Digest)"},           // explicit index
      {"events_no_update", "SELECT RAISE", "sELECT RAISE"},         // trigger
      {"checkpoints", "body_id INTEGER NOT NULL REFERENCES records(id)",
       "Body_id INTEGER NOT NULL REFERENCES records(id)"}, // reserved table
  }};
  for (const auto& edit : cases) {
    INFO(std::string(edit.object));
    scratch    f;
    const auto path = initialized(f, "ledger");
    require_opens(path);
    {
      auto db = raw_connection(path, open_mode::read_write);
      REQUIRE(exec_sql(db, "PRAGMA writable_schema=ON"));
      const auto original =
          text_at(db, std::string("SELECT sql FROM sqlite_master WHERE name='") + std::string(edit.object) + "'");
      REQUIRE(original);
      const auto changed = edit_once(*original, edit.old_text, edit.new_text);
      CHECK(changed.differing_bytes == 1);
      CHECK(changed.first_difference >= 40);
      auto update = db.prepare("UPDATE sqlite_master SET sql=?1 WHERE name=?2");
      REQUIRE(update);
      REQUIRE(update->bind_text(1, changed.edited));
      REQUIRE(update->bind_text(2, edit.object));
      REQUIRE(update->step() == step_result::done);
    }
    require_refused(path, execution_error_code::ledger_corrupt);
  }
}

TEST_CASE("format3 opener refuses a missing trigger", "[ledger][execution_schema]") {
  for (auto trigger : {"transitions_no_update", "current_state_no_delete"}) {
    INFO(trigger);
    scratch    f;
    const auto path = initialized(f, "ledger");
    require_opens(path);
    {
      auto db = raw_connection(path, open_mode::read_write);
      REQUIRE(exec_sql(db, std::string("DROP TRIGGER ") + trigger));
    }
    require_refused(path, execution_error_code::ledger_corrupt);
  }
}

TEST_CASE("format3 opener refuses an extra schema object", "[ledger][execution_schema]") {
  for (auto ddl :
       {"CREATE TABLE extra_table (id INTEGER PRIMARY KEY CHECK(id>0)) STRICT", "CREATE INDEX extra_index ON events(type)",
        "CREATE TRIGGER extra_trigger AFTER INSERT ON current_state BEGIN SELECT 1; END"}) {
    INFO(ddl);
    scratch    f;
    const auto path = initialized(f, "ledger");
    require_opens(path);
    {
      auto db = raw_connection(path, open_mode::read_write);
      REQUIRE(exec_sql(db, ddl));
    }
    require_refused(path, execution_error_code::ledger_corrupt);
  }
}

TEST_CASE("format3 opener refuses wrong format markers", "[ledger][execution_schema]") {
  {
    scratch    f;
    const auto path = initialized(f, "ledger");
    require_opens(path);
    {
      auto db = raw_connection(path, open_mode::read_write);
      REQUIRE(exec_sql(db, "PRAGMA user_version=4"));
    }
    require_refused(path, execution_error_code::unsupported_ledger_version);
  }
  {
    scratch    f;
    const auto path = initialized(f, "ledger");
    require_opens(path);
    {
      auto db = raw_connection(path, open_mode::read_write);
      // Keep user_version 3 and the schema text, but make ledger_meta.format 2. The triggers are dropped only for the
      // corrupting UPDATE, then recreated from their stored text, so the schema check cannot be the refusal.
      const auto no_update = text_at(db, "SELECT sql FROM sqlite_master WHERE name='ledger_meta_no_update'");
      const auto no_delete = text_at(db, "SELECT sql FROM sqlite_master WHERE name='ledger_meta_no_delete'");
      REQUIRE(no_update);
      REQUIRE(no_delete);
      REQUIRE(exec_sql(db, "PRAGMA ignore_check_constraints=ON"));
      REQUIRE(exec_sql(db, "DROP TRIGGER ledger_meta_no_update"));
      REQUIRE(exec_sql(db, "DROP TRIGGER ledger_meta_no_delete"));
      REQUIRE(exec_sql(db, "UPDATE ledger_meta SET format=2"));
      REQUIRE(exec_sql(db, *no_update));
      REQUIRE(exec_sql(db, *no_delete));
    }
    require_refused(path, execution_error_code::ledger_corrupt);
  }
}

TEST_CASE("format1 and format2 ledgers refuse execution open with unsupported ledger version", "[ledger][execution_schema]") {
  scratch    f;
  const auto v1 = f.root / "format1";
  const auto v2 = f.root / "format2";
  REQUIRE(initialize_new(v1, workspace_id));
  REQUIRE(initialize_new_v2(v2, workspace_id));
  for (const auto& path : {v1, v2}) {
    INFO(path.filename().string());
    const auto before = snapshot(f.root);
    require_refused(path, execution_error_code::unsupported_ledger_version);
    CHECK(snapshot(f.root) == before);
  }
}

TEST_CASE("format3 opener refuses a foreign application id", "[ledger][execution_schema]") {
  {
    scratch    f;
    const auto path = initialized(f, "ledger");
    require_opens(path);
    {
      auto db = raw_connection(path, open_mode::read_write);
      REQUIRE(exec_sql(db, "PRAGMA application_id=42"));
    }
    require_refused(path, execution_error_code::foreign_ledger);
  }
  {
    scratch    f;
    const auto path = f.root / "text";
    {
      std::string text;
      while (text.size() < 1024)
        text += "plain text, not a SQLite database\n";
      std::ofstream(path, std::ios::binary) << text;
    }
    const auto before = snapshot(f.root);
    require_refused(path, execution_error_code::foreign_ledger);
    CHECK(snapshot(f.root) == before);
  }
}

TEST_CASE("format3 opener refuses a missing ledger without creating it", "[ledger][execution_schema]") {
  scratch    f;
  const auto path   = f.root / "missing";
  const auto before = snapshot(f.root);
  require_refused(path, execution_error_code::ledger_not_found);
  CHECK(snapshot(f.root) == before);
}

TEST_CASE("format3 opener refuses journal sidecars", "[ledger][execution_schema]") {
  // Orphan sidecars only. A live writer's journal (excused while it holds RESERVED) cannot be produced in one process,
  // because fcntl locks never conflict within a process; that excusal is the R15-E work (L13) and is not tested here.
  for (auto suffix : {"-journal", "-wal", "-shm"}) {
    INFO(suffix);
    scratch    f;
    const auto path = initialized(f, "ledger");
    require_opens(path);
    const auto stored = bytes(path);
    std::ofstream(sidecar(path, suffix)) << "orphan sidecar";
    const auto before = snapshot(f.root);
    require_refused(path, execution_error_code::recovery_required);
    CHECK(bytes(path) == stored);
    CHECK(snapshot(f.root) == before);
  }
}

TEST_CASE("format3 records codec check admits request v3 only for requests", "[ledger][execution_schema]") {
  struct codec_case {
    int  kind;
    int  codec;
    bool admitted;
  };
  constexpr std::array<codec_case, 6> cases = {{
      {4, 3, true}, // request v3
      {4, 2, true}, // request v2
      {1, 1, true}, // ordinary record
      {4, 1, false},
      {3, 3, false},
      {1, 2, false},
  }};
  scratch                             f;
  const auto                          path = initialized(f, "ledger");
  auto                                db   = raw_connection(path, open_mode::read_write);
  REQUIRE(exec_sql(db, "PRAGMA foreign_keys=OFF"));
  int id = 1;
  for (const auto& test : cases) {
    const auto sql = std::format("INSERT INTO records(id,kind,codec,digest,encoded) VALUES({},{},{},zeroblob(16),x'00')", id++,
                                 test.kind, test.codec);
    INFO(sql);
    const auto result = exec_sql(db, sql);
    CHECK(result.has_value() == test.admitted);
    if (!result)
      CHECK(result.error().code == SQLITE_CONSTRAINT);
  }
}

TEST_CASE("format3 allows one active run per job and role", "[ledger][execution_schema]") {
  scratch    f;
  const auto path = initialized(f, "ledger");
  auto       db   = raw_connection(path, open_mode::read_write);
  REQUIRE(exec_sql(db, "INSERT INTO run_state(run_id,job,role,generation_id,state,last_seq) VALUES(1,'j','r',1,'active',1)"));
  const auto second =
      exec_sql(db, "INSERT INTO run_state(run_id,job,role,generation_id,state,last_seq) VALUES(2,'j','r',1,'active',2)");
  INFO(outcome_text(second));
  CHECK_FALSE(second.has_value());
  if (!second)
    CHECK(second.error().code == SQLITE_CONSTRAINT);
  REQUIRE(exec_sql(db, "INSERT INTO run_state(run_id,job,role,generation_id,state,last_seq) VALUES(3,'j','r',1,'released',3)"));
  REQUIRE(exec_sql(db, "INSERT INTO run_state(run_id,job,role,generation_id,state,last_seq) VALUES(4,'j','r',1,'fenced',4)"));
}

TEST_CASE("format3 role state and transition checks", "[ledger][execution_schema]") {
  scratch    f;
  const auto path = initialized(f, "ledger");
  auto       db   = raw_connection(path, open_mode::read_write);
  REQUIRE(exec_sql(db, "PRAGMA foreign_keys=OFF"));
  struct row_case {
    std::string_view sql;
    bool             admitted;
  };
  constexpr std::array<row_case, 13> cases = {{
      // role_state: running iff run_id, and drift only on running rows.
      {"INSERT INTO role_state(job,role,generation_id,state,drifted,run_id,fact_version,last_seq) "
       "VALUES('j','r1',1,'running',0,NULL,1,1)",
       false},
      {"INSERT INTO role_state(job,role,generation_id,state,drifted,run_id,fact_version,last_seq) "
       "VALUES('j','r2',1,'ready',0,5,1,1)",
       false},
      {"INSERT INTO role_state(job,role,generation_id,state,drifted,run_id,fact_version,last_seq) "
       "VALUES('j','r3',1,'ready',1,NULL,1,1)",
       false},
      {"INSERT INTO role_state(job,role,generation_id,state,drifted,run_id,fact_version,last_seq) "
       "VALUES('j','r4',1,'ready',0,NULL,1,1)",
       true},
      {"INSERT INTO role_state(job,role,generation_id,state,drifted,run_id,fact_version,last_seq) "
       "VALUES('j','r5',1,'running',0,7,1,1)",
       true},
      // transitions: NULL-first pairing and drift only on running states.
      {"INSERT INTO "
       "transitions(event_seq,ordinal,job,role,generation_id,before_state,after_state,before_drifted,after_drifted,reason) "
       "VALUES(1,1,'j','t1',1,NULL,'ready',0,0,'activated')",
       false},
      {"INSERT INTO "
       "transitions(event_seq,ordinal,job,role,generation_id,before_state,after_state,before_drifted,after_drifted,reason) "
       "VALUES(1,2,'j','t2',1,NULL,'ready',NULL,0,'activated')",
       true},
      {"INSERT INTO "
       "transitions(event_seq,ordinal,job,role,generation_id,before_state,after_state,before_drifted,after_drifted,reason) "
       "VALUES(1,3,'j','t3',1,NULL,'ready',NULL,1,'activated')",
       false},
      {"INSERT INTO "
       "transitions(event_seq,ordinal,job,role,generation_id,before_state,after_state,before_drifted,after_drifted,reason) "
       "VALUES(1,4,'j','t4',1,'ready','ready',1,0,'activated')",
       false},
      {"INSERT INTO "
       "transitions(event_seq,ordinal,job,role,generation_id,before_state,after_state,before_drifted,after_drifted,reason) "
       "VALUES(1,5,'j','t5',1,'ready','ready',NULL,0,'activated')",
       false},
      {"INSERT INTO "
       "transitions(event_seq,ordinal,job,role,generation_id,before_state,after_state,before_drifted,after_drifted,reason) "
       "VALUES(1,6,'j','t6',1,'ready','running',0,1,'claimed')",
       true},
      {"INSERT INTO "
       "transitions(event_seq,ordinal,job,role,generation_id,before_state,after_state,before_drifted,after_drifted,reason) "
       "VALUES(1,7,'j','t7',1,'running','ready',1,0,'released')",
       true},
      {"INSERT INTO "
       "transitions(event_seq,ordinal,job,role,generation_id,before_state,after_state,before_drifted,after_drifted,reason) "
       "VALUES(1,8,'j','t8',1,'running','ready',0,1,'released')",
       false},
  }};
  for (const auto& test : cases) {
    INFO(std::string(test.sql));
    const auto result = exec_sql(db, test.sql);
    CHECK(result.has_value() == test.admitted);
  }
}

TEST_CASE("existing opener still refuses a format3 ledger", "[ledger][format3][guard]") {
  // A format2 ledger whose user_version marker is 3 is the header an older binary sees for a format3 ledger. This guard
  // needs no format3 initialization, so it runs against the stub too.
  scratch    f;
  const auto path = f.root / "ledger";
  REQUIRE(initialize_new_v2(path, workspace_id));
  {
    auto db = raw_connection(path, open_mode::read_write);
    REQUIRE(exec_sql(db, "PRAGMA user_version=3"));
  }
  const auto stored = bytes(path);
  for (auto access : {ledger_access::read_only, ledger_access::read_write}) {
    auto opened = open_existing(path, access);
    REQUIRE_FALSE(opened);
    CHECK(opened.error().code == format_error_code::unsupported_version);
    CHECK(bytes(path) == stored);
  }
}

TEST_CASE("storage exported orphan sidecar check keeps the liveness rule", "[ledger][storage][guard]") {
  scratch    f;
  const auto path = f.root / "ledger";
  REQUIRE(initialize_new_v2(path, workspace_id));
  auto where = native::locate(path);
  REQUIRE(where);
  native::descriptor held(::open(path.c_str(), O_RDONLY | O_NOFOLLOW | O_CLOEXEC));
  REQUIRE(held.fd >= 0);
  CHECK(native::check_orphan_sidecars(*where, held.fd).has_value());
  for (auto suffix : {"-journal", "-wal", "-shm"}) {
    INFO(suffix);
    const auto orphan = sidecar(path, suffix);
    std::ofstream(orphan) << "orphan sidecar";
    const auto result = native::check_orphan_sidecars(*where, held.fd);
    CHECK_FALSE(result.has_value());
    if (!result)
      CHECK(result.error().code == format_error_code::recovery_required);
    std::filesystem::remove(orphan);
  }
}

TEST_CASE("storage header accepts supported version 3 and refuses unsupported version 4", "[ledger][storage][guard]") {
  // The raw header boundary supports versions 1, 2 and 3. Schema verification is separate: relabelling this
  // format2 fixture does not make it a complete format3 ledger. Unsupported version 4 must fail at the header itself.
  scratch    f;
  const auto path = f.root / "ledger";
  REQUIRE(initialize_new_v2(path, workspace_id));
  {
    native::descriptor held(::open(path.c_str(), O_RDONLY | O_NOFOLLOW | O_CLOEXEC));
    REQUIRE(held.fd >= 0);
    CHECK(native::header(held.fd).has_value());
  }
  {
    auto db = raw_connection(path, open_mode::read_write);
    REQUIRE(exec_sql(db, "PRAGMA user_version=3"));
  }
  {
    native::descriptor held(::open(path.c_str(), O_RDONLY | O_NOFOLLOW | O_CLOEXEC));
    REQUIRE(held.fd >= 0);
    REQUIRE(native::header(held.fd).has_value());
  }
  {
    auto db = raw_connection(path, open_mode::read_write);
    REQUIRE(exec_sql(db, "PRAGMA user_version=4"));
  }
  native::descriptor held(::open(path.c_str(), O_RDONLY | O_NOFOLLOW | O_CLOEXEC));
  REQUIRE(held.fd >= 0);
  auto checked = native::header(held.fd);
  REQUIRE_FALSE(checked.has_value());
  CHECK(checked.error().code == format_error_code::unsupported_version);
}

TEST_CASE("format3 opener refuses a truncated ledger file", "[ledger][execution_schema]") {
  scratch    f;
  const auto source  = initialized(f, "ledger");
  const auto content = bytes(source);
  REQUIRE(content.size() > 512);
  // A valid header followed by fewer than 512 bytes is a foreign ledger, whatever the header says.
  for (std::size_t length : {std::size_t{100}, std::size_t{200}, std::size_t{511}}) {
    INFO(length);
    const auto path = f.root / std::format("short-{}", length);
    {
      std::ofstream out(path, std::ios::binary);
      out.write(content.data(), static_cast<std::streamsize>(length));
    }
    require_refused(path, execution_error_code::foreign_ledger);
  }
}
