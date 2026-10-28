// Native-only acceptance ownership, snapshot binding, canonical provenance and exact request intent.
#include <catch2/catch_test_macros.hpp>
#include <catch2/generators/catch_generators.hpp>
#include <sqlite3.h>
#include <sys/stat.h>
#include <unistd.h>
import std;
import planar.engine.acceptance.prepared;
import planar.engine.canonical;
import planar.engine.ledger.format;
import planar.engine.ledger.acceptance;
import planar.engine.history.read;
import planar.engine.source_export;
import planar.engine.json_codec;
using namespace planar::engine;
using namespace planar::engine::history;
using namespace planar::engine::acceptance;
using namespace planar::engine::ledger;
namespace {
struct temporary {
  std::filesystem::path path;
  int                   format;
  explicit temporary(int selected_format = 1) : format(selected_format) {
    char pattern[] = "/tmp/planar-prepared-XXXXXX";
    path           = std::filesystem::canonical(::mkdtemp(pattern));
    put("workspace/a.lua", std::string("a\0b\r\n", 5));
    put("workspace/b.lua", "second");
    put("registry.json", std::string("r\0z", 3));
    REQUIRE(::chmod((path / "workspace/b.lua").c_str(), 0755) == 0);
  }
  ~temporary() {
    std::filesystem::remove_all(path);
  }
  void put(std::string name, std::string_view bytes) {
    auto file = path / name;
    std::filesystem::create_directories(file.parent_path());
    std::ofstream stream(file, std::ios::binary);
    stream.write(bytes.data(), bytes.size());
  }
  acceptance_capture capture(int registry = 1) {
    native_capture_session session;
    REQUIRE(session.add_root("workspace", path / "workspace"));
    REQUIRE(session.tree("workspace"));
    if (registry == 1) {
      REQUIRE(session.add_input("registry", path / "registry.json"));
      REQUIRE(session.read("registry", "input.json", false));
    } else if (registry == 2) {
      REQUIRE(session.add_root("registry", path / "other"));
      REQUIRE(session.tree("registry"));
    }
    auto result = session.finish_for_acceptance();
    REQUIRE(result);
    return std::move(*result);
  }
};
std::string read_bytes(const std::filesystem::path& file) {
  std::ifstream stream(file, std::ios::binary);
  REQUIRE(stream.good());
  return {std::istreambuf_iterator<char>(stream), {}};
}
std::string hex(const record_digest& digest) {
  std::string out;
  for (auto byte : digest)
    out += std::format("{:02x}", byte);
  return out;
}
struct inputs {
  prepared_provenance     stdlib;
  recorded_interpretation interpretation;
  inputs() {
    const std::array originals{original_file_view{"part.lua", "return {}"}};
    auto             p = prepare_stdlib(originals, "\n", "return {}\n");
    REQUIRE(p);
    stdlib       = std::move(*p);
    auto root    = std::filesystem::path(PLANAR_PREPARED_GOLDENS);
    auto decoded = decode_interpretation(read_bytes(root / "interpretation.canonical"), read_bytes(root / "versions.canonical"));
    REQUIRE(decoded);
    auto p2 = encode_interpretation(decoded->interpretation->fields.at("bundle"), decoded->interpretation->fields.at("semantic"),
                                    hex(stdlib.digest));
    REQUIRE(p2);
    interpretation = std::move(*p2);
  }
  auto prepare(acceptance_capture cap, std::span<const std::string_view> paths) {
    return prepared_acceptance::create(std::move(cap), paths, stdlib.files, interpretation.encoded,
                                       interpretation.versions_encoded);
  }
};
const std::array paths{std::string_view("b.lua"), std::string_view("a.lua")};

} // namespace
namespace {
request_intent intent(temporary& t, std::string_view key, std::optional<std::int64_t> head, std::string_view actor = "tester") {
  auto result = (t.format == 2 ? request_intent::create_v2(key, actor, head, (t.path / "workspace/a.lua").string(),
                                                           (t.path / "registry.json").string())
                               : request_intent::create(key, actor, head, (t.path / "workspace/a.lua").string(),
                                                        (t.path / "registry.json").string()));
  REQUIRE(result);
  return *result;
}
prepared_acceptance prepare(temporary& t, inputs& i) {
  auto result = i.prepare(t.capture(), paths);
  REQUIRE(result);
  return std::move(*result);
}
load_result accepted(temporary& t, const request_intent& request, const prepared_acceptance& candidate) {
  const auto identity = [&] {
    auto metadata = open_existing(t.path / "ledger");
    REQUIRE(metadata);
    return metadata->metadata().identity;
  }();
  completion_receipt receipt;
  auto               result = complete_acceptance(t.path / "ledger", request, candidate, identity, receipt);
  CHECK(receipt.phase == completion_phase::committed);
  INFO((result ? "accepted" : result.error().message));
  REQUIRE(result);
  CHECK_FALSE(result->replayed);
  CHECK(receipt.saved_result == result->result);
  auto decoded = decode_load_result(result->result);
  REQUIRE(decoded);
  auto opened = open_existing(t.path / "ledger");
  REQUIRE(opened);
  auto saved = opened->read_request(decoded->request_id);
  REQUIRE(saved);
  REQUIRE(*saved);
  CHECK((**saved).result == result->result);
  return *decoded;
}
void refused(const auto& result, format_error_code code) {
  REQUIRE_FALSE(result);
  CHECK(result.error().code == code);
}
void initial_state(temporary& t) {
  auto file = open_existing(t.path / "ledger");
  REQUIRE(file);
  auto head = file->read_head();
  REQUIRE(head);
  CHECK_FALSE(head->revision);
  CHECK(head->last_sequence == 1);
  CHECK_FALSE(*file->read_request("first"));
}
} // namespace
TEST_CASE("atomic acceptance initial noop change and revert retain chronology", "[atomic_acceptance]") {
  const int format = GENERATE(1, 2);
  INFO(std::format("format={}", format));
  temporary t(format);
  inputs    i;
  REQUIRE((format == 2 ? initialize_new_v2 : initialize_new)(t.path / "ledger", "quota"));
  auto candidate = prepare(t, i);
  auto first     = accepted(t, intent(t, "first", {}), candidate);
  CHECK(first.revision == 1);
  CHECK(first.event == 2);
  CHECK_FALSE(first.parent);
  CHECK(first.outcome == load_outcome::accepted);
  auto noop = accepted(t, intent(t, "noop", 1), candidate);
  CHECK(noop.revision == 1);
  CHECK(noop.event == 3);
  CHECK_FALSE(noop.parent);
  CHECK(noop.outcome == load_outcome::noop);
  t.put("workspace/a.lua", "modified");
  auto changed = prepare(t, i);
  auto second  = accepted(t, intent(t, "second", 1), changed);
  CHECK(second.revision == 2);
  CHECK(second.parent == 1);
  t.put("workspace/a.lua", std::string("a\0b\r\n", 5));
  auto reverted = prepare(t, i);
  auto third    = accepted(t, intent(t, "third", 2), reverted);
  CHECK(third.revision == 3);
  CHECK(third.parent == 2);
  auto noop2 = accepted(t, intent(t, "noop2", 3), reverted);
  CHECK(noop2.revision == 3);
  CHECK(noop2.parent == 2);
  auto file = open_existing(t.path / "ledger");
  REQUIRE(file);
  auto a = *file->read_revision(1), b = *file->read_revision(3);
  REQUIRE(a);
  REQUIRE(b);
  CHECK(a->references == b->references);
  auto event = *file->read_event(2);
  REQUIRE(event);
  CHECK(event->occurred_at == a->accepted_at);
}
TEST_CASE("atomic replay precedes later head missing capture and moved input inspection", "[atomic_acceptance]") {
  const int format = GENERATE(1, 2);
  INFO(std::format("format={}", format));
  temporary t(format);
  inputs    i;
  REQUIRE((format == 2 ? initialize_new_v2 : initialize_new)(t.path / "ledger", "quota"));
  auto candidate = prepare(t, i);
  auto request   = intent(t, "first", {});
  auto result    = complete_acceptance(t.path / "ledger", request, candidate);
  REQUIRE(result);
  t.put("workspace/a.lua", "later");
  auto later = prepare(t, i);
  accepted(t, intent(t, "second", 1), later);
  std::filesystem::remove_all(t.path / "workspace");
  std::filesystem::remove(t.path / "registry.json");
  auto replay = complete_acceptance(t.path / "ledger", request, candidate);
  REQUIRE(replay);
  CHECK(replay->replayed);
  CHECK(replay->result == result->result);
  auto moved = std::move(candidate);
  replay     = complete_acceptance(t.path / "ledger", request, candidate);
  REQUIRE(replay);
  CHECK(replay->replayed);
  CHECK(replay->result == result->result);
  refused(complete_acceptance(t.path / "ledger", intent(t, "first", {}, "changed actor"), candidate),
          format_error_code::request_conflict);
  refused(complete_acceptance(t.path / "ledger", intent(t, "first", 2), candidate), format_error_code::request_conflict);
  refused(complete_acceptance(t.path / "ledger", intent(t, "stale", {}), candidate), format_error_code::revision_conflict);
}
TEST_CASE("atomic unknown request CAS precedes prepared inspection and leaves no rows", "[atomic_acceptance]") {
  const int format = GENERATE(1, 2);
  INFO(std::format("format={}", format));
  temporary t(format);
  inputs    i;
  REQUIRE((format == 2 ? initialize_new_v2 : initialize_new)(t.path / "ledger", "quota"));
  auto candidate = prepare(t, i);
  auto moved     = std::move(candidate);
  refused(complete_acceptance(t.path / "ledger", intent(t, "first", 1), candidate), format_error_code::revision_conflict);
  initial_state(t);
  refused(complete_acceptance(t.path / "ledger", intent(t, "first", {}), candidate), format_error_code::invalid_argument);
  initial_state(t);
}
TEST_CASE("atomic workspace mismatch refuses before record writes", "[atomic_acceptance]") {
  const int format = GENERATE(1, 2);
  INFO(std::format("format={}", format));
  temporary t(format);
  inputs    i;
  REQUIRE((format == 2 ? initialize_new_v2 : initialize_new)(t.path / "ledger", "other"));
  auto candidate = prepare(t, i);
  refused(complete_acceptance(t.path / "ledger", intent(t, "first", {}), candidate), format_error_code::invalid_argument);
  initial_state(t);
}
TEST_CASE("atomic final native receipt refusal rolls back all tentative history", "[atomic_acceptance]") {
  const int format = GENERATE(1, 2);
  INFO(std::format("format={}", format));
  temporary t(format);
  inputs    i;
  REQUIRE((format == 2 ? initialize_new_v2 : initialize_new)(t.path / "ledger", "quota"));
  auto candidate = prepare(t, i);
  t.put("workspace/a.lua", "receipt changed");
  refused(complete_acceptance(t.path / "ledger", intent(t, "first", {}), candidate), format_error_code::capture_changed);
  initial_state(t);
  auto fresh = prepare(t, i);
  CHECK(accepted(t, intent(t, "first", {}), fresh).revision == 1);
}
namespace {
// SQLite's real per-connection hooks inject interrupt/commit failures without product seams.
struct fault_probe {
  std::string target;
  bool        hit     = false;
  bool        pending = false;
};
fault_probe* active_probe = nullptr;
int          trace_statement(unsigned, void* context, void* raw_statement, void*) {
  auto* db = static_cast<sqlite3*>(context);
  if (!active_probe || sqlite3_db_readonly(db, "main") != 0)
    return 0;
  auto&                  probe = *active_probe;
  const std::string_view sql(sqlite3_sql(static_cast<sqlite3_stmt*>(raw_statement)));
  if (sql.starts_with("UPDATE current_state SET"))
    probe.pending = true;
  if (sql.starts_with(probe.target) ||
      (probe.target == "pending_verify" && probe.pending && sql.starts_with("PRAGMA application_id"))) {
    probe.hit = true;
    sqlite3_interrupt(db);
  }
  return 0;
}
int refuse_commit(void*) {
  if (active_probe && active_probe->target == "commit" && active_probe->pending) {
    active_probe->hit = true;
    return 1;
  }
  return 0;
}
int attach_probe(sqlite3* db, char**, const sqlite3_api_routines*) {
  sqlite3_trace_v2(db, SQLITE_TRACE_STMT, trace_statement, db);
  sqlite3_commit_hook(db, refuse_commit, nullptr);
  return SQLITE_OK;
}
struct installed_probe {
  fault_probe probe;
  explicit installed_probe(std::string target) : probe{std::move(target)} {
    REQUIRE(active_probe == nullptr);
    active_probe = &probe;
    REQUIRE(sqlite3_auto_extension(reinterpret_cast<void (*)()>(attach_probe)) == SQLITE_OK);
  }
  ~installed_probe() {
    sqlite3_cancel_auto_extension(reinterpret_cast<void (*)()>(attach_probe));
    active_probe = nullptr;
  }
};
} // namespace
TEST_CASE("atomic SQLite mutation verification and commit failures roll back every prefix", "[atomic_acceptance]") {
  const int format = GENERATE(1, 2);
  INFO(std::format("format={}", format));
  for (auto target : {"INSERT INTO records", "INSERT INTO tree_edges", "INSERT INTO revisions", "INSERT INTO events",
                      "INSERT INTO requests", "UPDATE current_state SET", "pending_verify", "commit"}) {
    DYNAMIC_SECTION(target) {
      temporary t(format);
      inputs    i;
      REQUIRE((format == 2 ? initialize_new_v2 : initialize_new)(t.path / "ledger", "quota"));
      auto       candidate = prepare(t, i);
      const auto before    = read_bytes(t.path / "ledger");
      {
        installed_probe    probe(target);
        completion_receipt receipt;
        auto               identity = open_existing(t.path / "ledger")->metadata().identity;
        auto               result = complete_acceptance(t.path / "ledger", intent(t, "first", {}), candidate, identity, receipt);
        REQUIRE(probe.probe.hit);
        CHECK_FALSE(result);
        if (std::string_view(target) == "commit") {
          // SQLite's commit-hook refusal automatically rolls back before returning. Autocommit alone cannot
          // classify a failed COMMIT, so the native API conservatively retains uncertainty and exact bytes.
          CHECK(receipt.phase == completion_phase::commit_uncertain);
          REQUIRE_FALSE(receipt.saved_result.empty());
          CHECK(decode_load_result(receipt.saved_result));
        } else {
          CHECK(receipt.phase == completion_phase::not_completed);
          CHECK(receipt.saved_result.empty());
        }
      }
      CHECK(read_bytes(t.path / "ledger") == before);
      initial_state(t);
      CHECK(accepted(t, intent(t, "first", {}), candidate).revision == 1);
    }
  }
}
TEST_CASE("atomic concurrent identical requests replay and competing heads refuse within finite retry", "[atomic_acceptance]") {
  for (int race : {0, 1, 2}) {
    const int format = GENERATE(1, 2);
    INFO(std::format("format={}", format));
    const bool same_request = race != 0;
    DYNAMIC_SECTION(race) {
      temporary t(format);
      inputs    i;
      auto      metadata = (format == 2 ? initialize_new_v2 : initialize_new)(t.path / "ledger", "quota");
      REQUIRE(metadata);
      auto a = prepare(t, i), b = prepare(t, i);
      auto ia = intent(t, "first", {}), ib = intent(t, same_request ? "first" : "second", {}, race == 2 ? "other" : "tester");
      std::optional<std::expected<acceptance_result, format_error>> ra, rb;
      std::barrier                                                  ready(2);
      completion_receipt                                            ca, cb;
      auto work = [&](const auto& request, const auto& candidate, auto& result, auto& receipt) {
        ready.arrive_and_wait();
        for (int attempt = 0; attempt < 20; ++attempt) {
          result = complete_acceptance(t.path / "ledger", request, candidate, metadata->identity, receipt);
          if (*result ||
              (result->error().code != format_error_code::busy && result->error().code != format_error_code::recovery_required))
            break;
          std::this_thread::sleep_for(std::chrono::milliseconds(10));
        }
      };
      std::thread first([&] { work(ia, a, ra, ca); });
      std::thread second([&] { work(ib, b, rb, cb); });
      first.join();
      second.join();
      REQUIRE(ra);
      REQUIRE(rb);
      if (race == 1) {
        REQUIRE(*ra);
        REQUIRE(*rb);
        CHECK((**ra).result == (**rb).result);
        CHECK((**ra).replayed != (**rb).replayed);
      } else {
        REQUIRE(bool(*ra) != bool(*rb));
        CHECK((!*ra ? ra->error() : rb->error()).code ==
              (race == 2 ? format_error_code::request_conflict : format_error_code::revision_conflict));
      }
      for (const auto& [result, receipt] : {std::pair{&ra, &ca}, std::pair{&rb, &cb}}) {
        if (**result) {
          CHECK(receipt->phase == ((***result).replayed ? completion_phase::replayed : completion_phase::committed));
          CHECK(receipt->saved_result == (***result).result);
        } else {
          CHECK(receipt->phase == completion_phase::not_completed);
          CHECK(receipt->saved_result.empty());
        }
      }
      auto file = open_existing(t.path / "ledger");
      REQUIRE(file);
      auto head = file->read_head();
      REQUIRE(head);
      CHECK(head->revision == 1);
      CHECK(head->last_sequence == 2);
    }
  }
}
namespace {
struct raw_ledger {
  sqlite3* db = nullptr;
  explicit raw_ledger(const std::filesystem::path& path) {
    REQUIRE(sqlite3_open_v2(path.c_str(), &db, SQLITE_OPEN_READWRITE, nullptr) == SQLITE_OK);
  }
  ~raw_ledger() {
    sqlite3_close(db);
  }
  void sql(std::string_view text) {
    REQUIRE(sqlite3_exec(db, std::string(text).c_str(), nullptr, nullptr, nullptr) == SQLITE_OK);
  }
  void blobs(std::string_view sql, std::span<const std::string_view> values) {
    sqlite3_stmt* row = nullptr;
    REQUIRE(sqlite3_prepare_v2(db, sql.data(), int(sql.size()), &row, nullptr) == SQLITE_OK);
    for (std::size_t n = 0; n < values.size(); ++n)
      REQUIRE(sqlite3_bind_blob(row, int(n + 1), values[n].data(), int(values[n].size()), SQLITE_TRANSIENT) == SQLITE_OK);
    CHECK(sqlite3_step(row) == SQLITE_DONE);
    CHECK(sqlite3_finalize(row) == SQLITE_OK);
  }
};
} // namespace
TEST_CASE("atomic valid maximum record ID refuses allocation without writes", "[atomic_acceptance]") {
  const int format = GENERATE(1, 2);
  INFO(std::format("format={}", format));
  temporary t(format);
  inputs    i;
  REQUIRE((format == 2 ? initialize_new_v2 : initialize_new)(t.path / "ledger", "quota"));
  {
    raw_ledger db(t.path / "ledger");
    auto       encoded = encode_record(file_record{420, "last record"});
    auto       digest  = digest_record(file_record{420, "last record"});
    REQUIRE(encoded);
    REQUIRE(digest);
    const std::array<std::string_view, 2> values{std::string_view(reinterpret_cast<const char*>(digest->data()), digest->size()),
                                                 *encoded};
    db.blobs("INSERT INTO records VALUES(9223372036854775807,1,1,?1,?2)", values);
  }
  initial_state(t); // A gap in record IDs is valid; the full opener verifies this fixture.
  const auto before    = read_bytes(t.path / "ledger");
  auto       candidate = prepare(t, i);
  refused(complete_acceptance(t.path / "ledger", intent(t, "first", {}), candidate), format_error_code::unsupported_state);
  CHECK(read_bytes(t.path / "ledger") == before);
  initial_state(t);
}
TEST_CASE("atomic valid maximum revision permits noop but refuses changed allocation", "[atomic_acceptance]") {
  const int format = GENERATE(1, 2);
  INFO(std::format("format={}", format));
  temporary t(format);
  inputs    i;
  REQUIRE((format == 2 ? initialize_new_v2 : initialize_new)(t.path / "ledger", "quota"));
  auto candidate = prepare(t, i);
  auto request   = intent(t, "first", {});
  auto first     = complete_acceptance(t.path / "ledger", request, candidate);
  REQUIRE(first);
  t.put("workspace/a.lua", "second revision");
  candidate = prepare(t, i);
  first     = complete_acceptance(t.path / "ledger", intent(t, "second", 1), candidate);
  REQUIRE(first);
  history_event event;
  {
    auto file = open_existing(t.path / "ledger");
    REQUIRE(file);
    auto saved = file->read_event(3);
    REQUIRE(saved);
    REQUIRE(*saved);
    auto decoded = decode_history_event((**saved).payload);
    REQUIRE(decoded);
    event = *decoded;
  }
  constexpr auto maximum = std::numeric_limits<std::int64_t>::max();
  event.after            = maximum;
  auto event_bytes       = encode_history_event(event);
  auto result            = decode_load_result(first->result);
  REQUIRE(event_bytes);
  REQUIRE(result);
  result->revision  = maximum;
  auto result_bytes = encode_load_result(*result);
  REQUIRE(result_bytes);
  // Rewrite the second valid chronological revision to an approved sparse positive ID,
  // restoring every immutable trigger and all references before the full opener checks it.
  {
    raw_ledger db(t.path / "ledger");
    db.sql("PRAGMA foreign_keys=OFF");
    db.sql("BEGIN IMMEDIATE");
    for (auto table : {"revisions", "events", "requests"})
      db.sql(std::format("DROP TRIGGER {}_no_update", table));
    db.sql("UPDATE revisions SET id=9223372036854775807 WHERE id=2");
    const std::array<std::string_view, 1> payload{*event_bytes}, response{*result_bytes};
    db.blobs("UPDATE events SET revision_id=9223372036854775807,payload=?1 WHERE seq=3", payload);
    db.blobs("UPDATE requests SET revision_id=9223372036854775807,result=?1 WHERE request_id='second'", response);
    db.sql("UPDATE current_state SET active_revision_id=9223372036854775807");
    for (auto table : {"revisions", "events", "requests"})
      db.sql(std::format("CREATE TRIGGER {}_no_update BEFORE UPDATE ON {} BEGIN SELECT RAISE(ABORT,'immutable ledger row'); END",
                         table, table));
    db.sql("COMMIT");
  }
  auto noop = accepted(t, intent(t, "maximum_noop", maximum), candidate);
  CHECK(noop.revision == maximum);
  CHECK(noop.outcome == load_outcome::noop);
  CHECK(noop.parent == 1);
  const auto before = read_bytes(t.path / "ledger");
  t.put("workspace/a.lua", "changed at maximum");
  auto changed = prepare(t, i);
  refused(complete_acceptance(t.path / "ledger", intent(t, "exhausted", maximum), changed), format_error_code::unsupported_state);
  CHECK(read_bytes(t.path / "ledger") == before);
  auto file = open_existing(t.path / "ledger");
  REQUIRE(file);
  auto head = file->read_head();
  REQUIRE(head);
  CHECK(head->revision == maximum);
  CHECK(head->last_sequence == 4);
  CHECK_FALSE(*file->read_request("exhausted"));
}

TEST_CASE("atomic conflict retains locked heads after rollback and later head advance", "[atomic_acceptance]") {
  const int format = GENERATE(1, 2);
  INFO(std::format("format={}", format));
  temporary t(format);
  inputs    i;
  REQUIRE((format == 2 ? initialize_new_v2 : initialize_new)(t.path / "ledger", "quota"));
  auto candidate = prepare(t, i);
  auto missing   = complete_acceptance(t.path / "ledger", intent(t, "private-key", 1, "private-body"), candidate);
  REQUIRE_FALSE(missing);
  REQUIRE(missing.error().detail);
  auto empty = std::get<types::revision_conflict_detail>(*missing.error().detail);
  CHECK(empty.expected == 1);
  CHECK_FALSE(empty.actual);
  accepted(t, intent(t, "first", {}), candidate);
  const auto before = read_bytes(t.path / "ledger");
  auto       stale  = complete_acceptance(t.path / "ledger", intent(t, "private-key", {}, "private-body"), candidate);
  REQUIRE_FALSE(stale);
  CHECK(stale.error().code == format_error_code::revision_conflict);
  REQUIRE(stale.error().detail);
  auto retained = stale.error();
  CHECK(read_bytes(t.path / "ledger") == before);
  t.put("workspace/a.lua", "later head");
  auto later = prepare(t, i);
  accepted(t, intent(t, "second", 1), later);
  auto detail = std::get<types::revision_conflict_detail>(*retained.detail);
  CHECK_FALSE(detail.expected);
  CHECK(detail.actual == 1);
  CHECK(std::get<types::revision_conflict_detail>(*missing.error().detail).actual == std::nullopt);
  auto conflict = complete_acceptance(t.path / "ledger", intent(t, "first", 2, "private-body"), later);
  REQUIRE_FALSE(conflict);
  CHECK(conflict.error().code == format_error_code::request_conflict);
  CHECK_FALSE(conflict.error().detail);
}

TEST_CASE("atomic identity bound receipt refuses replacement before exact replay", "[atomic_acceptance]") {
  const int format = GENERATE(1, 2);
  INFO(std::format("format={}", format));
  temporary  t(format);
  inputs     i;
  const auto metadata = (format == 2 ? initialize_new_v2 : initialize_new)(t.path / "ledger", "quota");
  REQUIRE(metadata);
  auto candidate = prepare(t, i);
  auto request   = intent(t, "first", {});
  auto original  = complete_acceptance(t.path / "ledger", request, candidate);
  REQUIRE(original);
  // A real replacement ledger contains an equal request and valid saved response. Identity wins over replay.
  std::filesystem::rename(t.path / "ledger", t.path / "original");
  auto replacement = (format == 2 ? initialize_new_v2 : initialize_new)(t.path / "ledger", "quota");
  REQUIRE(replacement);
  REQUIRE(replacement->identity != metadata->identity);
  REQUIRE(complete_acceptance(t.path / "ledger", request, candidate));
  std::filesystem::remove_all(t.path / "workspace");
  completion_receipt receipt;
  refused(complete_acceptance(t.path / "ledger", request, candidate, metadata->identity, receipt),
          format_error_code::unsafe_path);
  CHECK(receipt.phase == completion_phase::not_completed);
  CHECK(receipt.saved_result.empty());
  for (auto wrong : {std::string{}, std::string(32, '0')}) {
    completion_receipt invalid;
    refused(complete_acceptance(t.path / "ledger", request, candidate, wrong, invalid), format_error_code::unsafe_path);
    CHECK(invalid.phase == completion_phase::not_completed);
    CHECK(invalid.saved_result.empty());
  }
}
TEST_CASE("atomic identity bound receipt late replay owns bytes before stale CAS and unusable candidate", "[atomic_acceptance]") {
  const int format = GENERATE(1, 2);
  INFO(std::format("format={}", format));
  temporary  t(format);
  inputs     i;
  const auto metadata = (format == 2 ? initialize_new_v2 : initialize_new)(t.path / "ledger", "quota");
  REQUIRE(metadata);
  auto candidate = prepare(t, i);
  auto request   = intent(t, "first", {});
  auto first     = complete_acceptance(t.path / "ledger", request, candidate);
  REQUIRE(first);
  t.put("workspace/a.lua", "advanced head");
  auto changed = prepare(t, i);
  REQUIRE(complete_acceptance(t.path / "ledger", intent(t, "second", 1), changed));
  auto retained = std::move(candidate);
  std::filesystem::remove_all(t.path / "workspace");
  std::filesystem::remove(t.path / "registry.json");
  completion_receipt receipt;
  auto               replay = complete_acceptance(t.path / "ledger", request, candidate, metadata->identity, receipt);
  REQUIRE(replay);
  CHECK(replay->replayed);
  CHECK(receipt.phase == completion_phase::replayed);
  CHECK(receipt.saved_result == first->result);
  CHECK(replay->result == receipt.saved_result);
  // Reusing an authoritative owner must not erase earlier completion evidence.
  refused(complete_acceptance(t.path / "ledger", request, candidate, metadata->identity, receipt),
          format_error_code::invalid_argument);
  CHECK(receipt.phase == completion_phase::replayed);
  CHECK(receipt.saved_result == first->result);
  for (bool differing_request : {false, true}) {
    completion_receipt refused_receipt;
    auto               refusal = complete_acceptance(
        t.path / "ledger", intent(t, differing_request ? "first" : "unknown", {}, differing_request ? "other" : "tester"),
        candidate, metadata->identity, refused_receipt);
    refused(refusal, differing_request ? format_error_code::request_conflict : format_error_code::revision_conflict);
    CHECK(refused_receipt.phase == completion_phase::not_completed);
    CHECK(refused_receipt.saved_result.empty());
  }
}
TEST_CASE("atomic identity bound receipt capture and unavailable candidate refusals retain noncompletion",
          "[atomic_acceptance]") {
  const int format = GENERATE(1, 2);
  INFO(std::format("format={}", format));
  temporary  t(format);
  inputs     i;
  const auto metadata = (format == 2 ? initialize_new_v2 : initialize_new)(t.path / "ledger", "quota");
  REQUIRE(metadata);
  auto candidate = prepare(t, i);
  t.put("workspace/a.lua", "changed after preparation");
  completion_receipt capture_receipt;
  refused(complete_acceptance(t.path / "ledger", intent(t, "first", {}), candidate, metadata->identity, capture_receipt),
          format_error_code::capture_changed);
  CHECK(capture_receipt.phase == completion_phase::not_completed);
  CHECK(capture_receipt.saved_result.empty());
  initial_state(t);
  auto               owner = std::move(candidate);
  completion_receipt unavailable_receipt;
  refused(complete_acceptance(t.path / "ledger", intent(t, "first", {}), candidate, metadata->identity, unavailable_receipt),
          format_error_code::invalid_argument);
  CHECK(unavailable_receipt.phase == completion_phase::not_completed);
  CHECK(unavailable_receipt.saved_result.empty());
  initial_state(t);
}

TEST_CASE("atomic identity bound receipt busy commit proves rollback and preserves proposed saved bytes", "[atomic_acceptance]") {
  const int format = GENERATE(1, 2);
  INFO(std::format("format={}", format));
  temporary  t(format);
  inputs     i;
  const auto metadata = (format == 2 ? initialize_new_v2 : initialize_new)(t.path / "ledger", "quota");
  REQUIRE(metadata);
  auto               candidate = prepare(t, i);
  auto               request   = intent(t, "first", {});
  completion_receipt receipt;
  {
    raw_ledger reader(t.path / "ledger");
    reader.sql("BEGIN");
    reader.sql("SELECT count(*) FROM events");
    auto result = complete_acceptance(t.path / "ledger", request, candidate, metadata->identity, receipt);
    refused(result, format_error_code::busy);
    CHECK(receipt.phase == completion_phase::not_completed);
    REQUIRE_FALSE(receipt.saved_result.empty());
    CHECK(decode_load_result(receipt.saved_result));
  }
  initial_state(t);
  completion_receipt retry_receipt;
  auto               retry = complete_acceptance(t.path / "ledger", request, candidate, metadata->identity, retry_receipt);
  REQUIRE(retry);
  CHECK(retry_receipt.phase == completion_phase::committed);
  CHECK(retry_receipt.saved_result == retry->result);
  CHECK(retry_receipt.saved_result == receipt.saved_result);
}

TEST_CASE("atomic format request mismatch and profile refusal precede CAS and unusable candidate",
          "[atomic_acceptance][request_v2]") {
  const int format = GENERATE(1, 2);
  temporary t(format);
  inputs    i;
  REQUIRE((format == 2 ? initialize_new_v2 : initialize_new)(t.path / "ledger", "quota"));
  auto       candidate   = prepare(t, i);
  auto       first       = accepted(t, intent(t, "first", {}), candidate);
  auto       unavailable = std::move(candidate);
  const auto before      = read_bytes(t.path / "ledger");
  auto       mismatched  = format == 1 ? request_intent::create_v2("first", "tester", {}, (t.path / "workspace/a.lua").string(),
                                                                   (t.path / "registry.json").string())
                                       : request_intent::create("first", "tester", {}, (t.path / "workspace/a.lua").string(),
                                                                (t.path / "registry.json").string());
  REQUIRE(mismatched);
  refused(complete_acceptance(t.path / "ledger", *mismatched, candidate), format_error_code::request_conflict);
  auto unknown = format == 1 ? request_intent::create_v2("unknown", "tester", {}, "/workspace", "/registry")
                             : request_intent::create("unknown", "tester", {}, "/workspace", "/registry");
  REQUIRE(unknown);
  refused(complete_acceptance(t.path / "ledger", *unknown, candidate), format_error_code::unsupported_version);
  auto profile = request_intent::create_v2("profile", "tester", {}, "/workspace", "/registry", "/private/profile");
  REQUIRE(profile);
  refused(complete_acceptance(t.path / "ledger", *profile, candidate),
          format == 1 ? format_error_code::unsupported_version : format_error_code::unsupported_state);
  CHECK(read_bytes(t.path / "ledger") == before);
  CHECK_FALSE(std::filesystem::exists(t.path / "ledger-journal"));
}

TEST_CASE("atomic native format2 acceptance history export and backup restore survive removed inputs",
          "[atomic_acceptance][format2]") {
  temporary t(2);
  inputs    i;
  REQUIRE(initialize_new_v2(t.path / "ledger", "quota"));
  auto       candidate = prepare(t, i);
  const auto request   = intent(t, "native-v2", {});
  const auto result    = accepted(t, request, candidate);
  {
    auto file = open_existing(t.path / "ledger");
    REQUIRE(file);
    auto saved = file->read_request("native-v2");
    REQUIRE(saved);
    REQUIRE(*saved);
    auto body = file->read_record((**saved).body_reference.id);
    REQUIRE(body);
    CHECK(std::holds_alternative<request_v2>(body->value));
    CHECK((**saved).body_encoded == (**request.data()).encoded);
  }
  std::filesystem::remove_all(t.path / "workspace");
  std::filesystem::remove(t.path / "registry.json");
  REQUIRE(backup_new(t.path / "ledger", t.path / "backup"));
  REQUIRE(restore_new(t.path / "backup", t.path / "restored"));
  CHECK_FALSE(restore_new(t.path / "backup", t.path / "restored"));
  for (auto name : {"ledger", "backup", "restored"}) {
    auto file = open_existing(t.path / name);
    REQUIRE(file);
    CHECK(file->metadata().format == ledger_format::v2);
    CHECK(file->metadata().identity == result.ledger);
    for (auto selector :
         {history_selector::page, history_selector::revision, history_selector::request, history_selector::state}) {
      auto history = read_history(*file, {selector, 1, 50, "native-v2"});
      REQUIRE(history);
      auto json = codec_encode(*history);
      REQUIRE(json);
      CHECK(json->find(t.path.string()) == std::string::npos);
      CHECK(json->find("registry.json") == std::string::npos);
    }
    auto exported = source_export::export_source(t.path / name, 1, t.path / (std::string(name) + "-export"));
    REQUIRE(exported);
    CHECK(read_bytes(t.path / (std::string(name) + "-export/a.lua")) == std::string("a\0b\r\n", 5));
    CHECK(read_bytes(t.path / (std::string(name) + "-export/b.lua")) == "second");
    CHECK_FALSE(source_export::export_source(t.path / name, 1, t.path / (std::string(name) + "-export")));
    auto replay = complete_acceptance(t.path / name, request, candidate);
    REQUIRE(replay);
    CHECK(replay->replayed);
    CHECK(replay->result == *encode_load_result(result));
  }
}

namespace {
template <class... A>
concept embedded_completes = requires(A&&... a) { ::planar::engine::ledger::complete_acceptance(std::forward<A>(a)...); };
} // namespace

namespace {
// Whitespace-insensitive view of a C++ unit: line comments removed, then every whitespace character dropped, so spaced or
// split spellings of an import or export cannot evade the checks below.
std::string squashed(const std::string& text) {
  std::string out;
  bool        comment = false;
  for (std::size_t i = 0; i < text.size(); ++i) {
    const char c = text[i];
    if (comment) {
      comment = c != '\n';
      continue;
    }
    if (c == '/' && i + 1 < text.size() && text[i + 1] == '/') {
      comment = true;
      continue;
    }
    if (!std::isspace(static_cast<unsigned char>(c)))
      out += c;
  }
  return out;
}
bool contains(const std::string& text, std::string_view needle) {
  return text.find(needle) != std::string::npos;
}
} // namespace

TEST_CASE("X06 C C01 seam is unreachable from a plain acceptance importer", "[atomic_acceptance][x06c][c01]") {
  // A qualified name that this TU cannot see is a hard compile error, so the invisibility is checked on the interface unit
  // this TU imports, whitespace-insensitively: the seam is imported privately and none of its names are exported.
  const auto interface_text = read_bytes(std::filesystem::path(__FILE__).parent_path() / "acceptance.cppm");
  REQUIRE_FALSE(interface_text.empty());
  const auto code = squashed(interface_text);
  CHECK(contains(code, "import:seam;"));
  CHECK_FALSE(contains(code, "exportimport:seam"));
  CHECK_FALSE(contains(code, "exportimportplanar.engine.ledger.acceptance_seam"));
  CHECK_FALSE(contains(code, "planar.engine.ledger.acceptance_seam"));
  for (const char* name : {"native_candidate", "candidate_values", "complete_native_candidate", "seam_detail"}) {
    INFO(name);
    CHECK_FALSE(contains(code, name));
  }
  // Only the two in-tree adapters (and the seam itself and these tests) may mention the seam at all: no other target,
  // such as a command entry point, reaches it even by linking.
  const auto                  source_root = std::filesystem::path(__FILE__).parent_path().parent_path().parent_path();
  const std::set<std::string> allowed{"engine/ledger/acceptance_seam.cppm",   "engine/ledger/acceptance_seam.cpp",
                                      "engine/ledger/acceptance.cppm",        "engine/ledger/acceptance.cpp",
                                      "engine/ledger/acceptance.t.cpp",       "engine/ledger/profile_acceptance.cppm",
                                      "engine/ledger/profile_acceptance.cpp", "engine/ledger/profile_acceptance_seam.t.cpp"};
  // The allowed set counts eight units.
  CHECK(allowed.size() == 8);
  std::set<std::string> mentioning;
  std::size_t           scanned = 0;
  for (const auto& entry : std::filesystem::recursive_directory_iterator(source_root)) {
    const auto ext = entry.path().extension().string();
    if (!entry.is_regular_file() || (ext != ".cpp" && ext != ".cppm"))
      continue;
    ++scanned;
    const auto text = squashed(read_bytes(entry.path()));
    if (contains(text, "acceptance_seam") || contains(text, ":seam") || contains(text, "native_candidate") ||
        contains(text, "complete_native_candidate") || contains(text, "candidate_values") || contains(text, "seam_detail"))
      mentioning.insert(std::filesystem::relative(entry.path(), source_root).generic_string());
  }
  CHECK(scanned > 100);
  for (const auto& file : mentioning) {
    INFO(file);
    CHECK(allowed.contains(file));
  }
  CHECK(mentioning == allowed);
  // Embedded entry points stay callable on the opaque prepared_acceptance.
  STATIC_REQUIRE(embedded_completes<const std::filesystem::path&, const request_intent&, const prepared_acceptance&>);
  STATIC_REQUIRE(embedded_completes<const std::filesystem::path&, const request_intent&, const prepared_acceptance&,
                                    std::string_view, completion_receipt&>);
}

namespace {
std::filesystem::path c03_source_root() {
  return std::filesystem::path(__FILE__).parent_path().parent_path().parent_path();
}
std::string c03_squashed_unit(const char* relative) {
  return squashed(read_bytes(c03_source_root() / relative));
}
} // namespace

TEST_CASE("X06 C C03 seam is a non-exported partition of the acceptance module", "[atomic_acceptance][x06c][c03]") {
  const auto seam_interface = c03_squashed_unit("engine/ledger/acceptance_seam.cppm");
  CHECK(contains(seam_interface, "exportmoduleplanar.engine.ledger.acceptance:seam;"));
  CHECK_FALSE(contains(seam_interface, "exportmoduleplanar.engine.ledger.acceptance_seam;"));
  const auto primary = c03_squashed_unit("engine/ledger/acceptance.cppm");
  CHECK(contains(primary, "exportmoduleplanar.engine.ledger.acceptance;"));
  CHECK(contains(primary, "import:seam;"));
  CHECK_FALSE(contains(primary, "exportimport:seam"));
  const auto seam_impl = c03_squashed_unit("engine/ledger/acceptance_seam.cpp");
  CHECK(contains(seam_impl, "moduleplanar.engine.ledger.acceptance;"));
  CHECK_FALSE(contains(seam_impl, "moduleplanar.engine.ledger.acceptance_seam;"));
  CHECK(contains(c03_squashed_unit("engine/ledger/acceptance.cpp"), "moduleplanar.engine.ledger.acceptance;"));
}

TEST_CASE("X06 C C03 profile adapter is a partition re-exported by the acceptance interface", "[atomic_acceptance][x06c][c03]") {
  const auto profile = c03_squashed_unit("engine/ledger/profile_acceptance.cppm");
  CHECK(contains(profile, "exportmoduleplanar.engine.ledger.acceptance:profile;"));
  CHECK(contains(profile, "import:seam;"));
  CHECK_FALSE(contains(profile, "exportimport:seam"));
  CHECK_FALSE(contains(profile, "planar.engine.ledger.acceptance_seam"));
  CHECK(contains(c03_squashed_unit("engine/ledger/acceptance.cppm"), "exportimport:profile;"));
  CHECK(contains(c03_squashed_unit("engine/ledger/profile_acceptance.cpp"), "moduleplanar.engine.ledger.acceptance;"));
}

TEST_CASE("X06 C C04 seam entities live in a nested non-exported internal namespace", "[atomic_acceptance][x06c][c04]") {
  const auto seam = c03_squashed_unit("engine/ledger/acceptance_seam.cppm");
  CHECK(contains(seam, "namespaceseam_detail{"));
  CHECK_FALSE(contains(seam, "exportnamespaceseam_detail"));
  CHECK_FALSE(contains(seam, "namespaceplanar::engine::ledger::seam_detail"));
  const auto open = seam.find("namespaceseam_detail{");
  REQUIRE(open != std::string::npos);
  for (const char* name : {"structcandidate_values{", "classnative_candidate{", "complete_native_candidate("}) {
    INFO(name);
    const auto first = seam.find(name);
    REQUIRE(first != std::string::npos);
    CHECK(first > open);
  }
  // Nothing declared inside the nested namespace may be exported: an exported nested entity would be reachable through
  // the primary interface's import of the partition.
  CHECK_FALSE(contains(seam.substr(open), "export"));
  const auto impl      = c03_squashed_unit("engine/ledger/acceptance_seam.cpp");
  const auto impl_open = impl.find("namespaceseam_detail{");
  CHECK(impl_open != std::string::npos);
  const auto impl_call = impl.find("complete_native_candidate(");
  REQUIRE(impl_call != std::string::npos);
  if (impl_open != std::string::npos)
    CHECK(impl_call > impl_open);
}

TEST_CASE("X06 C C04 acceptance_result stays outside the internal seam namespace", "[atomic_acceptance][x06c][c04]") {
  const auto seam = c03_squashed_unit("engine/ledger/acceptance_seam.cppm");
  CHECK(contains(seam, "namespaceplanar::engine::ledger{exportstructacceptance_result{"));
  CHECK(contains(seam, "boolreplayed;};namespaceseam_detail{"));
  CHECK_FALSE(contains(seam, "exportnamespaceseam_detail"));
  const auto primary = c03_squashed_unit("engine/ledger/acceptance.cppm");
  CHECK_FALSE(contains(primary, "seam_detail"));
  for (const char* name : {"native_candidate", "candidate_values", "complete_native_candidate"}) {
    INFO(name);
    CHECK_FALSE(contains(primary, name));
  }
}

TEST_CASE("X06 C C04 direct seam tests live in their own implementation unit and the adapter tests import normally",
          "[atomic_acceptance][x06c][c04]") {
  const std::string first_name  = "X06 C C01 seam interns the registry tree with the provenance limits";
  const std::string second_name = "X06 C seam interns an effective stdlib closure beyond the original tree limits";
  const auto        root        = c03_source_root();
  const auto        seam_raw    = read_bytes(root / "engine/ledger/profile_acceptance_seam.t.cpp");
  const auto        plain_raw   = read_bytes(root / "engine/ledger/profile_acceptance.t.cpp");
  REQUIRE_FALSE(seam_raw.empty());
  REQUIRE_FALSE(plain_raw.empty());
  const auto seam_unit  = squashed(seam_raw);
  const auto plain_unit = squashed(plain_raw);
  CHECK(contains(seam_unit, "moduleplanar.engine.ledger.acceptance;"));
  CHECK(contains(seam_unit, "import:seam;"));
  CHECK(contains(seam_unit, "usingnamespaceplanar::engine::ledger::seam_detail;"));
  CHECK(contains(seam_raw, first_name));
  CHECK(contains(seam_raw, second_name));
  CHECK(contains(plain_unit, "importplanar.engine.ledger.acceptance;"));
  CHECK_FALSE(contains(plain_unit, "moduleplanar.engine.ledger.acceptance;"));
  CHECK_FALSE(contains(plain_unit, "import:seam;"));
  CHECK_FALSE(contains(plain_raw, first_name));
  CHECK_FALSE(contains(plain_raw, second_name));
  for (const char* name : {"native_candidate", "complete_native_candidate", "candidate_values"}) {
    INFO(name);
    CHECK_FALSE(contains(plain_raw, name));
  }
}

TEST_CASE("X06 C C03 no passkey grant or test hook vocabulary remains under src", "[atomic_acceptance][x06c][c03]") {
  const std::string tokens[]      = {std::string("seam_") + "key", std::string("seam_") + "grant", std::string("test_") + "probe",
                                     std::string("make_") + "key"};
  const std::string adjacent_word = std::string("fri") + "end";
  const std::string hook_macro    = std::string("PLANAR_") + "TEST";
  const std::set<std::string> eight{"engine/ledger/acceptance.cppm",        "engine/ledger/acceptance.cpp",
                                    "engine/ledger/acceptance_seam.cppm",   "engine/ledger/acceptance_seam.cpp",
                                    "engine/ledger/acceptance.t.cpp",       "engine/ledger/profile_acceptance.cppm",
                                    "engine/ledger/profile_acceptance.cpp", "engine/ledger/profile_acceptance_seam.t.cpp"};
  const auto                  root = c03_source_root();
  std::set<std::string>       offending;
  std::size_t                 scanned = 0;
  for (const auto& entry : std::filesystem::recursive_directory_iterator(root)) {
    const auto ext = entry.path().extension().string();
    if (!entry.is_regular_file() || (ext != ".cpp" && ext != ".cppm"))
      continue;
    ++scanned;
    const auto raw = read_bytes(entry.path());
    const auto rel = std::filesystem::relative(entry.path(), root).generic_string();
    for (const auto& token : tokens)
      if (contains(raw, token))
        offending.insert(rel + " has " + token);
    if (eight.contains(rel)) {
      if (contains(raw, adjacent_word))
        offending.insert(rel + " has " + adjacent_word);
      if (contains(raw, hook_macro))
        offending.insert(rel + " has " + hook_macro);
    }
  }
  CHECK(scanned > 100);
  std::string names;
  for (const auto& n : offending)
    names += n + "; ";
  INFO(names);
  CHECK(offending.empty());
}

TEST_CASE("X06 C C03 only the adapters call complete_native_candidate", "[atomic_acceptance][x06c][c03]") {
  const std::string                  call = std::string("complete_native_") + "candidate(";
  const auto                         root = c03_source_root();
  std::map<std::string, std::size_t> counts;
  for (const auto& entry : std::filesystem::recursive_directory_iterator(root)) {
    const auto ext = entry.path().extension().string();
    if (!entry.is_regular_file() || (ext != ".cpp" && ext != ".cppm"))
      continue;
    const auto  text = squashed(read_bytes(entry.path()));
    std::size_t n    = 0;
    for (auto at = text.find(call); at != std::string::npos; at = text.find(call, at + call.size()))
      ++n;
    if (n != 0)
      counts[std::filesystem::relative(entry.path(), root).generic_string()] = n;
  }
  CHECK(counts["engine/ledger/acceptance.cpp"] == 2);
  CHECK(counts["engine/ledger/profile_acceptance.cpp"] == 1);
  CHECK(counts["engine/ledger/acceptance_seam.cppm"] == 1);
  CHECK(counts["engine/ledger/acceptance_seam.cpp"] == 1);
  CHECK(counts["engine/ledger/profile_acceptance_seam.t.cpp"] >= 1);
  counts.erase("engine/ledger/acceptance.cpp");
  counts.erase("engine/ledger/profile_acceptance.cpp");
  counts.erase("engine/ledger/acceptance_seam.cppm");
  counts.erase("engine/ledger/acceptance_seam.cpp");
  counts.erase("engine/ledger/profile_acceptance_seam.t.cpp");
  counts.erase("engine/ledger/acceptance.t.cpp"); // this file names the call in its own pins
  std::string others;
  for (const auto& [file, n] : counts)
    others += file + "; ";
  INFO(others);
  CHECK(counts.empty());
}

TEST_CASE("X06 C C03 shared body refusal message bytes are pinned", "[atomic_acceptance][x06c][c03]") {
  const int format = GENERATE(1, 2);
  INFO(std::format("format={}", format));
  const auto message = [](const auto& result, format_error_code code, std::string_view text) {
    REQUIRE_FALSE(result);
    CHECK(result.error().code == code);
    CHECK(result.error().message == text);
  };
  temporary  t(format);
  inputs     i;
  const auto metadata = (format == 2 ? initialize_new_v2 : initialize_new)(t.path / "ledger", "quota");
  REQUIRE(metadata);
  auto candidate = prepare(t, i);
  auto first     = accepted(t, intent(t, "first", {}), candidate);
  // Unknown request id at a stale head.
  message(complete_acceptance(t.path / "ledger", intent(t, "stale", {}), candidate), format_error_code::revision_conflict,
          "Expected revision differs from current head");
  // Same request id with different parameters.
  message(complete_acceptance(t.path / "ledger", intent(t, "first", {}, "changed actor"), candidate),
          format_error_code::request_conflict, "Request parameters differ from saved request");
  // Ledger identity differs from the prelude identity.
  completion_receipt identity_receipt;
  message(complete_acceptance(t.path / "ledger", intent(t, "first", {}), candidate, std::string(32, '0'), identity_receipt),
          format_error_code::unsafe_path, "Ledger identity differs from prelude");
  // Request format differs from the ledger format.
  auto mismatched = format == 1 ? request_intent::create_v2("unknown", "tester", {}, "/workspace", "/registry")
                                : request_intent::create("unknown", "tester", {}, "/workspace", "/registry");
  REQUIRE(mismatched);
  message(complete_acceptance(t.path / "ledger", *mismatched, candidate), format_error_code::unsupported_version,
          "Unsupported ledger format version");
  if (format == 2) {
    auto profile = request_intent::create_v2("profile", "tester", {}, "/workspace", "/registry", "/private/profile");
    REQUIRE(profile);
    message(complete_acceptance(t.path / "ledger", *profile, candidate), format_error_code::unsupported_state,
            "Unsupported profile acceptance");
  }
  // A used receipt, an unavailable intent and an unavailable candidate.
  auto               request = intent(t, "first", {});
  completion_receipt used;
  REQUIRE(complete_acceptance(t.path / "ledger", request, candidate, metadata->identity, used));
  REQUIRE(used.phase == completion_phase::replayed);
  message(complete_acceptance(t.path / "ledger", request, candidate, metadata->identity, used),
          format_error_code::invalid_argument, "Completion receipt is already in use");
  auto               gone_intent = intent(t, "other", 1);
  auto               owner       = std::move(gone_intent);
  completion_receipt fresh;
  message(complete_acceptance(t.path / "ledger", gone_intent, candidate, metadata->identity, fresh),
          format_error_code::invalid_argument, "Unavailable request intent");
  auto gone = std::move(candidate);
  message(complete_acceptance(t.path / "ledger", intent(t, "next", 1), candidate), format_error_code::invalid_argument,
          "Unavailable prepared acceptance");
}

TEST_CASE("X06 C C03 workspace mismatch message bytes are pinned", "[atomic_acceptance][x06c][c03]") {
  const int format = GENERATE(1, 2);
  temporary t(format);
  inputs    i;
  REQUIRE((format == 2 ? initialize_new_v2 : initialize_new)(t.path / "ledger", "other"));
  auto candidate = prepare(t, i);
  auto result    = complete_acceptance(t.path / "ledger", intent(t, "first", {}), candidate);
  REQUIRE_FALSE(result);
  CHECK(result.error().code == format_error_code::invalid_argument);
  CHECK(result.error().message == "Prepared workspace differs from ledger");
}

TEST_CASE("X06 C C03 runtime product sources import only ledger completion from the ledger", "[atomic_acceptance][x06c][c03]") {
  const auto root = c03_source_root() / "engine/runtime";
  REQUIRE(std::filesystem::is_directory(root));
  std::size_t           scanned = 0;
  std::set<std::string> offending;
  bool                  completion_seen = false;
  for (const auto& entry : std::filesystem::recursive_directory_iterator(root)) {
    const auto name = entry.path().filename().string();
    if (!entry.is_regular_file() || name.ends_with(".t.cpp") ||
        (entry.path().extension() != ".cpp" && entry.path().extension() != ".cppm"))
      continue;
    ++scanned;
    const auto        text   = squashed(read_bytes(entry.path()));
    const std::string prefix = "planar.engine.ledger.";
    for (auto at = text.find(prefix); at != std::string::npos; at = text.find(prefix, at + prefix.size())) {
      std::size_t end = at;
      while (end < text.size() && (std::isalnum(static_cast<unsigned char>(text[end])) || text[end] == '_' || text[end] == '.'))
        ++end;
      const auto module = text.substr(at, end - at);
      if (module == "planar.engine.ledger.completion")
        completion_seen = true;
      else
        offending.insert(name + " names " + module);
    }
  }
  CHECK(scanned > 0);
  CHECK(completion_seen);
  std::string names;
  for (const auto& n : offending)
    names += n + "; ";
  INFO(names);
  CHECK(offending.empty());
}

namespace {
std::string x06d_runtime_text(const char* name) {
  const auto text = read_bytes(c03_source_root() / "engine/runtime" / name);
  REQUIRE_FALSE(text.empty());
  return text;
}
// Ordered member names of a plain struct, read from its declaration (comments removed, one statement per semicolon).
std::vector<std::string> x06d_struct_members(const std::string& text, std::string_view name) {
  const auto head = text.find("struct " + std::string(name) + " {");
  REQUIRE(head != std::string::npos);
  const auto end = text.find("\n};", head);
  REQUIRE(end != std::string::npos);
  std::string body;
  const auto  raw     = text.substr(head, end - head);
  bool        comment = false;
  for (std::size_t i = raw.find('{') + 1; i < raw.size(); ++i) {
    if (!comment && raw[i] == '/' && i + 1 < raw.size() && raw[i + 1] == '/')
      comment = true;
    if (raw[i] == '\n')
      comment = false;
    if (!comment)
      body += raw[i];
  }
  std::vector<std::string> members;
  std::size_t              start = 0;
  for (auto semi = body.find(';'); semi != std::string::npos; semi = body.find(';', start)) {
    auto statement = body.substr(start, semi - start);
    start          = semi + 1;
    if (const auto cut = statement.find_first_of("={"); cut != std::string::npos)
      statement.resize(cut);
    while (!statement.empty() && std::isspace(static_cast<unsigned char>(statement.back())))
      statement.pop_back();
    auto at = statement.size();
    while (at > 0 && (std::isalnum(static_cast<unsigned char>(statement[at - 1])) || statement[at - 1] == '_'))
      --at;
    members.push_back(statement.substr(at));
  }
  return members;
}
// Squashed public section of a class plus its access-grant declarations.
struct x06d_surface {
  std::string              public_section;
  std::vector<std::string> grants;
};
x06d_surface x06d_class_surface(const std::string& squashed_text, std::string_view name) {
  const auto head = squashed_text.find("class" + std::string(name) + "{");
  REQUIRE(head != std::string::npos);
  const auto end = squashed_text.find("};", head);
  REQUIRE(end != std::string::npos);
  const auto section = squashed_text.substr(head, end - head + 2);
  const auto priv    = section.find("private:");
  REQUIRE(priv != std::string::npos);
  x06d_surface out;
  out.public_section       = section.substr(0, priv);
  const std::string marker = std::string("fri") + "endclass";
  for (auto at = section.find(marker); at != std::string::npos; at = section.find(marker, at + 1)) {
    const auto semi = section.find(';', at);
    out.grants.push_back(section.substr(at, semi - at));
  }
  return out;
}
} // namespace

TEST_CASE("X06 D composed ceiling constants keep their contract values", "[atomic_acceptance][x06d]") {
  // Mutant I14: any composed ceiling constant changed in runtime.cpp. Per-module and total capture limits stay in the
  // capture module and are not restated here.
  const auto code = squashed(x06d_runtime_text("runtime.cpp"));
  for (const char* pinned :
       {"constexprstd::size_tmemory_max=512*1024*1024;", "constexprstd::size_tinstruction_max=100000000;",
        "constexprstd::size_tnode_max=4000000;", "runtime.memory_limit=64*1024*1024;", "runtime.instruction_limit=10000000;"}) {
    INFO(pinned);
    const auto first = code.find(pinned);
    REQUIRE(first != std::string::npos);
    CHECK(code.find(pinned, first + 1) == std::string::npos);
  }
}

TEST_CASE("X06 D runtime public surface and resource messages stay closed", "[atomic_acceptance][x06d]") {
  // Mutants I15 and I16: a new resource message, a budget or ceiling vocabulary export, a new observation member, a new
  // public constructor or a new access grant on the profile types.
  const auto interface_raw = x06d_runtime_text("runtime.cppm");
  {
    auto code = squashed(interface_raw);
    for (std::size_t at = code.find("budget_bytes"); at != std::string::npos; at = code.find("budget_bytes"))
      code.erase(at, std::string_view("budget_bytes").size()); // The pre-existing render option, not budget vocabulary.
    for (const char* word : {"budget", "ceiling", "capture_charge"}) {
      INFO(word);
      CHECK_FALSE(contains(code, word));
    }
  }
  CHECK(x06d_struct_members(interface_raw, "profile_invocation_observation") ==
        std::vector<std::string>{"live", "consumed", "known_load", "frame_begun", "instructions", "native_bytes", "owner_charge",
                                 "registrations", "tag_entries", "backing_entries", "temporary_entries"});
  CHECK(x06d_struct_members(interface_raw, "profile_load_stage_observation") ==
        std::vector<std::string>{"live", "frame_begun", "sealed", "bundle_retained", "model_retained", "jobs", "shape_dispatches",
                                 "instructions", "native_bytes", "registrations", "tag_entries", "backing_entries",
                                 "temporary_entries", "cleanup_ran", "peak_temporary_entries", "baseline_before_route",
                                 "instruction_marks", "native_byte_marks"});
  const auto squashed_interface = squashed(interface_raw);
  const auto prepared           = x06d_class_surface(squashed_interface, "prepared_profile");
  CHECK(prepared.public_section ==
        "classprepared_profile{public:std::expected<std::shared_ptr<constprofile_acceptance_data>,registration_error>data()"
        "const;std::shared_ptr<constacceptance::request_data>bound_intent()const;std::expected<void,diagnostic>verify_native_"
        "receipts()const;profile_load_stage_observationobserve()constnoexcept;prepared_profile(constprepared_profile&)=delete;"
        "prepared_profile&operator=(constprepared_profile&)=delete;prepared_profile(prepared_profile&&other)noexcept;"
        "prepared_profile&operator=(prepared_profile&&other)noexcept;~prepared_profile();");
  CHECK(prepared.grants == std::vector<std::string>{std::string("fri") + "endclassprofile_invocation"});
  const auto owner = x06d_class_surface(squashed_interface, "profile_invocation");
  CHECK(owner.public_section ==
        "classprofile_invocation{public:staticstd::expected<profile_invocation,registration_error>open_known_load(const"
        "acceptance::request_intent&request_intent,std::string_viewledger_path);staticstd::expected<profile_invocation,"
        "registration_error>open_opaque(std::string_viewprofile_selector);value_ptrcatalog()const;std::string_vieweffective_"
        "catalog()const;std::expected<prepared_profile,profile_preparation_error>prepare_load(constacceptance::request_intent&"
        "final_intent)&&;std::expected<extension_invocation,registration_error>into_extension()&&;profile_invocation_"
        "observationobserve()constnoexcept;profile_invocation(constprofile_invocation&)=delete;profile_invocation&operator=("
        "constprofile_invocation&)=delete;profile_invocation(profile_invocation&&other)noexcept;profile_invocation&operator=("
        "profile_invocation&&other)noexcept;~profile_invocation();");
  CHECK(owner.grants == std::vector<std::string>{std::string("fri") + "endclassprofile_load_stage",
                                                 std::string("fri") + "endclassprepared_profile"});
  const auto stage = x06d_class_surface(squashed_interface, "profile_load_stage");
  CHECK(stage.public_section ==
        "classprofile_load_stage{public:profile_load_stage_observationobserve()constnoexcept;profile_load_stage(const"
        "profile_load_stage&)=delete;profile_load_stage&operator=(constprofile_load_stage&)=delete;profile_load_stage("
        "profile_load_stage&&other)noexcept;profile_load_stage&operator=(profile_load_stage&&other)noexcept;~profile_load_"
        "stage();");
  CHECK(stage.grants == std::vector<std::string>{std::string("fri") + "endclassprofile_invocation",
                                                 std::string("fri") + "endclassprepared_profile"});
  // Every resource_limit message literal in the runtime unit is one of the closed seven; the capture charge adds none.
  const auto            code   = x06d_runtime_text("runtime.cpp");
  const std::string     marker = "{\"resource_limit\",";
  std::set<std::string> messages;
  for (auto at = code.find(marker); at != std::string::npos; at = code.find(marker, at + 1)) {
    auto quote = code.find_first_not_of(" \t\r\n", at + marker.size());
    REQUIRE(quote != std::string::npos);
    if (code[quote] != '"')
      continue;
    std::string message;
    for (auto i = quote + 1; i < code.size() && code[i] != '"'; ++i) {
      if (code[i] == '\\' && i + 1 < code.size())
        ++i;
      message += code[i];
    }
    messages.insert(message);
  }
  const std::set<std::string> pinned{"Conversion depth exceeded",
                                     "Conversion node count exceeded",
                                     "External profile resource limit exceeded",
                                     "Lua allocation exceeded",
                                     "Lua instruction limit exceeded",
                                     "Lua stack exceeded",
                                     "Native conversion allocation exceeded"};
  CHECK(messages == pinned);
}

TEST_CASE("X06 D C01 variant ceiling mechanism does not depend on runtime source text", "[atomic_acceptance][x06d][c01]") {
  // Mutants: the small-ceiling test variant is produced by rewriting runtime.cpp text in the build description (a
  // FATAL_ERROR on a pattern miss, a REGEX REPLACE over the runtime text), or the runtime loses its overridable
  // default-constant seam or one of its contract values.
  const auto build = read_bytes(c03_source_root() / "engine" / "CMakeLists.txt");
  REQUIRE_FALSE(build.empty());
  CHECK_FALSE(contains(build, "planar_budget_rewrite"));
  CHECK_FALSE(contains(build, "budget_bridge"));
  CHECK_FALSE(contains(build, "pattern did not match"));
  for (auto at = build.find("file(READ"); at != std::string::npos; at = build.find("file(READ", at + 1)) {
    const auto line = build.substr(at, build.find('\n', at) - at);
    INFO(line);
    CHECK_FALSE(contains(line, "runtime.cpp"));
  }
  for (auto at = build.find("FATAL_ERROR"); at != std::string::npos; at = build.find("FATAL_ERROR", at + 1)) {
    const auto before = build.substr(at > 300 ? at - 300 : 0, at > 300 ? 300 : at);
    CHECK_FALSE(contains(before, "runtime.cpp"));
    CHECK_FALSE(contains(before, "runtime/runtime"));
  }
  for (auto at = build.find("REGEX REPLACE"); at != std::string::npos; at = build.find("REGEX REPLACE", at + 1)) {
    const auto statement = build.substr(at, build.find(')', at) - at);
    INFO(statement);
    CHECK_FALSE(contains(statement, "runtime"));
    CHECK_FALSE(contains(statement, "budget"));
  }
  const auto        code   = squashed(x06d_runtime_text("runtime.cpp"));
  const std::string guard  = "#ifndefPLANAR_RUNTIME_";
  std::size_t       guards = 0;
  for (auto at = code.find(guard); at != std::string::npos; at = code.find(guard, at + 1))
    ++guards;
  CHECK(guards >= 4);
  std::vector<std::string> defaults;
  const std::string        define = "#definePLANAR_RUNTIME_";
  for (auto at = code.find(define); at != std::string::npos; at = code.find(define, at + 1)) {
    auto end = code.find("#endif", at);
    REQUIRE(end != std::string::npos);
    auto chunk = code.substr(at, end - at);
    while (!chunk.empty() && std::string_view("uUlLzZ()").find(chunk.back()) != std::string_view::npos)
      chunk.pop_back();
    defaults.push_back(chunk);
  }
  for (const char* value : {"512*1024*1024", "100000000", "4000000", "10000000"}) {
    INFO(value);
    CHECK(std::ranges::any_of(defaults, [&](const std::string& chunk) { return chunk.ends_with(value); }));
  }
}
