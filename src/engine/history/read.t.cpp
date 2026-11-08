// Recorded-history public projection and snapshot composition without live policy.
#include <catch2/catch_test_macros.hpp>
#include <unistd.h>
import std;
import planar.engine.history.read;
import planar.engine.json_codec;
import planar.engine.ledger.execution_schema;
using namespace planar::engine;
using namespace planar::engine::history;
using namespace planar::engine::ledger;
TEST_CASE("history metadata preserves native IDs above int32 exactly", "[history_read]") {
  revision_row row{9223372036854775807LL,
                   2147483648LL,
                   {2147483649LL, 2147483650LL, 2147483651LL, 2147483652LL, 2147483653LL},
                   "actor",
                   "time"};
  auto         value = revision_metadata(row);
  CHECK(value->fields.size() == 9);
  CHECK(value->fields.at("id")->string == "9223372036854775807");
  CHECK(value->fields.at("parent")->string == "2147483648");
  CHECK(value->fields.at("versions")->string == "2147483653");
  REQUIRE(codec_encode(value));
}
TEST_CASE("history summary keeps structured payload and absent IDs without null", "[history_read]") {
  auto payload = encode_history_event({{}, {}, event_reason::initialized, {}});
  REQUIRE(payload);
  auto summary = event_summary({1, "ledger_initialized", 1, {}, {}, "planar:init", "time", *payload});
  REQUIRE(summary);
  CHECK((*summary)->fields.size() == 8);
  CHECK((*summary)->fields.at("revision")->string == "none");
  CHECK((*summary)->fields.at("request_id")->string == "none");
  CHECK((*summary)->fields.at("payload")->kind == value_kind::record);
  CHECK_FALSE(event_summary({1, "ledger_initialized", 1, {}, {}, "actor", "time", "private malformed"}));
}
TEST_CASE("history initialized views use one read owner and typed misses", "[history_read]") {
  char pattern[] = "/tmp/planar-history-read-XXXXXX";
  REQUIRE(::mkdtemp(pattern));
  auto root = std::filesystem::canonical(pattern);
  struct cleanup {
    std::filesystem::path root;
    ~cleanup() {
      std::filesystem::remove_all(root);
    }
  } guard{root};
  REQUIRE(initialize_new(root / "ledger", "quota"));
  auto file = open_existing(root / "ledger");
  REQUIRE(file);
  auto page = read_history(*file, {});
  REQUIRE(page);
  CHECK((*page)->fields.at("active_revision")->string == "none");
  CHECK((*page)->fields.at("data")->fields.at("events")->items.size() == 1);
  CHECK_FALSE((*page)->fields.at("data")->fields.contains("next_after"));
  auto state = read_history(*file, {history_selector::state, 1, 50, {}});
  REQUIRE(state);
  CHECK((*state)->fields.at("data")->fields.at("revision")->string == "none");
  auto missing = read_history(*file, {history_selector::revision, 1, 50, {}});
  REQUIRE_FALSE(missing);
  CHECK(missing.error().code == history_read_code::revision_not_found);
  missing = read_history(*file, {history_selector::request, 0, 50, "missing"});
  REQUIRE_FALSE(missing);
  CHECK(missing.error().code == history_read_code::request_not_found);
  missing = read_history(*file, {history_selector::state, 2, 50, {}});
  REQUIRE_FALSE(missing);
  CHECK(missing.error().code == history_read_code::event_not_found);
  CHECK(read_history(*file, {}));
}

// ---- R15-C P4: format3 read variant of the history projection. ----
namespace {
struct format3_root {
  std::filesystem::path root;
  format3_root() {
    char pattern[] = "/tmp/planar-history-format3-XXXXXX";
    REQUIRE(::mkdtemp(pattern));
    root = std::filesystem::canonical(pattern);
  }
  ~format3_root() {
    std::error_code ignored;
    std::filesystem::remove_all(root, ignored);
  }
};
void insert(connection& db, std::string_view sql) {
  auto statement = db.prepare(sql);
  REQUIRE(statement);
  auto step = statement->step();
  INFO(sql);
  REQUIRE(step);
}
std::set<std::string> key_set(const value_ptr& node) {
  std::set<std::string> out;
  for (const auto& [name, _] : node->fields)
    out.insert(name);
  return out;
}
} // namespace
TEST_CASE("history reads a fresh format3 ledger with the format3 event summary and state", "[history_read][format3]") {
  format3_root s;
  const auto   path = s.root / "ledger";
  REQUIRE(initialize_format3(path, "quota"));
  auto file = open_existing(path);
  REQUIRE(file);
  CHECK(file->metadata().format == ledger_format::v3);
  CHECK(file->metadata().workspace_id == "quota");
  auto page = read_history(*file, {});
  REQUIRE(page);
  const auto& events = (*page)->fields.at("data")->fields.at("events");
  REQUIRE(events->items.size() == 1);
  const auto& event = events->items.front();
  CHECK(key_set(event) == std::set<std::string>{"sequence", "type", "version", "revision", "request_id", "actor",
                                                "occurred_at", "payload", "job", "role", "generation_id", "run_id"});
  CHECK(event->fields.at("job")->string.empty());
  CHECK(event->fields.at("generation_id")->string == "none");
  auto state = read_history(*file, {history_selector::state, 1, 50, {}});
  REQUIRE(state);
  const auto& replayed = (*state)->fields.at("data")->fields.at("state");
  CHECK(key_set(replayed) == std::set<std::string>{"active_revision", "active_job", "activation", "roles", "runs"});
  CHECK(replayed->fields.at("activation")->string == "none");
  auto missing = read_history(*file, {history_selector::request, 0, 50, "missing"});
  REQUIRE_FALSE(missing);
  CHECK(missing.error().code == history_read_code::request_not_found);
  missing = read_history(*file, {history_selector::state, 2, 50, {}});
  REQUIRE_FALSE(missing);
  CHECK(missing.error().code == history_read_code::event_not_found);
  missing = read_history(*file, {history_selector::revision, 1, 50, {}});
  REQUIRE_FALSE(missing);
  CHECK(missing.error().code == history_read_code::revision_not_found);
}
TEST_CASE("storage read variant refuses writable format3 access and forged format3 markers", "[history_read][format3]") {
  format3_root s;
  const auto   path = s.root / "ledger";
  REQUIRE(initialize_format3(path, "quota"));
  auto writable = open_existing(path, ledger_access::read_write);
  REQUIRE_FALSE(writable);
  CHECK(writable.error().code == format_error_code::unsupported_version);
  // A format2 file whose marker says 3 lacks the execution tables and stays an unsupported version.
  const auto forged = s.root / "forged";
  REQUIRE(initialize_new_v2(forged, "quota"));
  {
    auto db = connection::open(forged, open_mode::read_write);
    REQUIRE(db);
    insert(*db, "PRAGMA user_version=3");
  }
  auto opened = open_existing(forged);
  REQUIRE_FALSE(opened);
  CHECK(opened.error().code == format_error_code::unsupported_version);
}
TEST_CASE("format3 at-event replay takes the last transition per role and the run of a running role",
          "[history_read][format3]") {
  format3_root s;
  const auto   path = s.root / "ledger";
  REQUIRE(initialize_format3(path, "quota"));
  {
    auto db = connection::open(path, open_mode::read_write);
    REQUIRE(db);
    insert(*db, "INSERT INTO records(id,kind,codec,digest,encoded) VALUES(1,3,1,zeroblob(16),x'00')");
    insert(*db, "INSERT INTO job_activations(id,job,revision_id,event_seq) VALUES(1,'j',1,2)");
    insert(*db, "INSERT INTO generations(id,job,role,ordinal,structure_id,opened_seq,reason) VALUES"
                "(1,'j','a',1,1,2,'activated'),(2,'j','b',1,1,2,'activated'),(3,'j','a',2,1,5,'structural_change'),"
                "(4,'other','x',1,1,2,'activated')");
    insert(*db, "INSERT INTO runs(id,generation_id,request_id,claimed_seq,secret,provenance_id) VALUES"
                "(7,1,'claim',3,zeroblob(16),NULL)");
    insert(*db, "INSERT INTO transitions(event_seq,ordinal,job,role,generation_id,before_state,after_state,before_drifted,"
                "after_drifted,reason) VALUES"
                "(2,1,'j','a',1,NULL,'ready',NULL,0,'activated'),(2,2,'j','b',2,NULL,'blocked',NULL,0,'activated'),"
                "(2,3,'other','x',4,NULL,'ready',NULL,0,'activated'),(3,1,'j','a',1,'ready','running',0,0,'claimed'),"
                "(5,1,'j','a',3,'running','ready',0,0,'structural_change'),"
                "(5,2,'j','b',2,'blocked','ready',0,0,'prerequisite_available')");
  }
  auto file = open_existing(path);
  REQUIRE(file);
  auto at = [&](std::uint64_t seq, std::string_view job) {
    auto replayed = file->read_execution_state_at(seq, job);
    INFO("seq " << seq << " refused: " << (replayed ? std::string() : replayed.error().message));
    REQUIRE(replayed);
    return *replayed;
  };
  auto before = at(1, "j");
  CHECK_FALSE(before.activation);
  CHECK(before.roles.empty());
  auto activated = at(2, "j");
  REQUIRE(activated.activation);
  CHECK(*activated.activation == 1);
  REQUIRE(activated.roles.size() == 2);
  CHECK(activated.roles[0].role == "a");
  CHECK(activated.roles[0].state == "ready");
  CHECK_FALSE(activated.roles[0].run);
  CHECK(activated.roles[1].role == "b");
  CHECK(activated.roles[1].state == "blocked");
  CHECK(activated.runs.empty());
  for (std::uint64_t seq : {3u, 4u}) {
    auto running = at(seq, "j");
    REQUIRE(running.roles.size() == 2);
    CHECK(running.roles[0].state == "running");
    REQUIRE(running.roles[0].run);
    CHECK(running.roles[0].run->id == 7);
    CHECK(running.roles[0].run->generation == 1);
    REQUIRE(running.runs.size() == 1);
    CHECK(running.runs[0].id == 7);
    CHECK(running.runs[0].role == "a");
  }
  auto later = at(5, "j");
  REQUIRE(later.roles.size() == 2);
  CHECK(later.roles[0].generation == 2);
  CHECK(later.roles[0].state == "ready");
  CHECK_FALSE(later.roles[0].run);
  CHECK(later.roles[1].state == "ready");
  CHECK(later.runs.empty());
  auto none = at(5, "");
  CHECK(none.roles.empty());
  REQUIRE(none.activation);
  // The replay is format3 only.
  REQUIRE(initialize_new_v2(s.root / "two", "quota"));
  auto two = open_existing(s.root / "two");
  REQUIRE(two);
  auto refused = two->read_execution_state_at(1, "j");
  REQUIRE_FALSE(refused);
  CHECK(refused.error().code == format_error_code::invalid_argument);
}
