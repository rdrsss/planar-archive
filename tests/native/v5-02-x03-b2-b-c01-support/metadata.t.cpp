// Native-only acceptance ownership, snapshot binding, canonical provenance and exact request intent.
#include <catch2/catch_test_macros.hpp>
#include <catch2/generators/catch_generators.hpp>
#include <sqlite3.h>
#include <sys/stat.h>
#include <unistd.h>
import std;
import planar.engine.acceptance.prepared;
import planar.engine.canonical;
import planar.test.metadata_format;
import planar.test.metadata_storage;
import planar.test.metadata_acceptance;
import planar.test.metadata_fixture;
import planar.engine.json_codec;
using namespace planar::engine;
using namespace planar::engine::history;
using namespace planar::engine::acceptance;
using namespace planar::engine::ledger;
namespace tested  = planar::engine::metadata_test;
namespace fixture = planar::test::metadata_fixture;
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
struct selection {
  selection(fixture::boundary boundary, bool different) {
    fixture::selected = different ? boundary : fixture::boundary::none;
    fixture::supplied = 0;
  }
  ~selection() {
    fixture::selected = fixture::boundary::none;
  }
};
auto initialize(temporary& t) {
  return (t.format == 2 ? tested::initialize_new_v2 : tested::initialize_new)(t.path / "ledger", "quota");
}
// SQL is observation only: verify every table's row count after the API returns.
auto counts(temporary& t) {
  sqlite3* db = nullptr;
  REQUIRE(sqlite3_open_v2((t.path / "ledger").c_str(), &db, SQLITE_OPEN_READONLY, nullptr) == SQLITE_OK);
  std::array<std::int64_t, 7> result{};
  unsigned                    index = 0;
  for (auto name : {"ledger_meta", "records", "tree_edges", "revisions", "events", "requests", "current_state"}) {
    sqlite3_stmt* stmt = nullptr;
    REQUIRE(sqlite3_prepare_v2(db, std::format("SELECT count(*) FROM {}", name).c_str(), -1, &stmt, nullptr) == SQLITE_OK);
    REQUIRE(sqlite3_step(stmt) == SQLITE_ROW);
    result[index++] = sqlite3_column_int64(stmt, 0);
    REQUIRE(sqlite3_finalize(stmt) == SQLITE_OK);
  }
  REQUIRE(sqlite3_close(db) == SQLITE_OK);
  return result;
}
void unchanged(temporary& t, const auto& before, std::string_view bytes, const tested::ledger_metadata& info) {
  CHECK(counts(t) == before);
  CHECK(read_bytes(t.path / "ledger") == bytes);
  auto reader = tested::open_existing(t.path / "ledger");
  REQUIRE(reader);
  CHECK(reader->metadata().identity == info.identity);
  CHECK(reader->metadata().workspace_id == info.workspace_id);
  CHECK(reader->metadata().created_at == info.created_at);
  CHECK(reader->metadata().format == info.format);
  auto head = reader->read_head();
  REQUIRE(head);
  CHECK_FALSE(head->revision);
  CHECK(head->last_sequence == 1);
  auto saved = reader->read_request("first");
  REQUIRE(saved);
  CHECK_FALSE(*saved);
}
void refusal(const auto& result, std::string_view message) {
  REQUIRE_FALSE(result);
  CHECK(result.error().code == tested::format_error_code::unsafe_path);
  CHECK(result.error().message == message);
}
} // namespace
TEST_CASE("metadata continuity locked begin includes selected format", "[metadata_continuity]") {
  for (int format : {1, 2}) {
    for (bool different : {false, true}) {
      DYNAMIC_SECTION("format=" << format << " supplied_difference=" << different) {
        temporary t(format);
        auto      info = initialize(t);
        REQUIRE(info);
        auto      before = counts(t);
        auto      bytes  = read_bytes(t.path / "ledger");
        selection selected(fixture::boundary::begin, different);
        auto      writer = tested::begin_records(t.path / "ledger");
        CHECK(fixture::supplied == unsigned(different));
        if (different) {
          refusal(writer, "Ledger changed before write transaction");
        } else {
          REQUIRE(writer);
          REQUIRE(writer->intern(file_record{420, "control commits"}));
          REQUIRE(writer->commit());
          CHECK(counts(t)[1] == before[1] + 1);
        }
        if (different)
          unchanged(t, before, bytes, *info);
      }
    }
  }
}
TEST_CASE("metadata continuity record precommit includes selected format", "[metadata_continuity]") {
  for (int format : {1, 2}) {
    for (bool different : {false, true}) {
      DYNAMIC_SECTION("format=" << format << " supplied_difference=" << different) {
        temporary t(format);
        auto      info = initialize(t);
        REQUIRE(info);
        auto before = counts(t);
        auto bytes  = read_bytes(t.path / "ledger");
        auto writer = tested::begin_records(t.path / "ledger");
        REQUIRE(writer);
        auto record = writer->intern(file_record{420, "tentative prefix"});
        REQUIRE(record);
        selection selected(fixture::boundary::writer_commit, different);
        auto      committed = writer->commit();
        CHECK(fixture::supplied == unsigned(different));
        if (different)
          refusal(committed, "Ledger metadata changed before commit");
        else {
          REQUIRE(committed);
          CHECK(counts(t)[1] == before[1] + 1);
          auto reader = tested::open_existing(t.path / "ledger");
          REQUIRE(reader);
          REQUIRE(reader->read_record(record->id));
        }
        auto again = writer->commit();
        REQUIRE_FALSE(again);
        CHECK(again.error().code == tested::format_error_code::invalid_argument);
        auto reused = writer->intern(file_record{420, "unavailable"});
        REQUIRE_FALSE(reused);
        CHECK(reused.error().code == tested::format_error_code::invalid_argument);
        if (different)
          unchanged(t, before, bytes, *info);
      }
    }
  }
}
TEST_CASE("metadata continuity acceptance precommit includes selected format", "[metadata_continuity]") {
  for (int format : {1, 2}) {
    for (bool different : {false, true}) {
      DYNAMIC_SECTION("format=" << format << " supplied_difference=" << different) {
        temporary t(format);
        inputs    i;
        auto      info = initialize(t);
        REQUIRE(info);
        auto before    = counts(t);
        auto bytes     = read_bytes(t.path / "ledger");
        auto candidate = i.prepare(t.capture(), paths);
        REQUIRE(candidate);
        auto request = format == 2 ? request_intent::create_v2("first", "tester", {}, (t.path / "workspace/a.lua").string(),
                                                               (t.path / "registry.json").string())
                                   : request_intent::create("first", "tester", {}, (t.path / "workspace/a.lua").string(),
                                                            (t.path / "registry.json").string());
        REQUIRE(request);
        completion_receipt receipt;
        selection          selected(fixture::boundary::acceptance_commit, different);
        auto accepted = tested::complete_acceptance(t.path / "ledger", *request, *candidate, info->identity, receipt);
        CHECK(fixture::supplied == unsigned(different));
        if (different) {
          refusal(accepted, "Ledger metadata changed before acceptance commit");
          CHECK(receipt.phase == completion_phase::not_completed);
          CHECK(receipt.saved_result.empty());
          unchanged(t, before, bytes, *info);
          // The refused request has no saved replay; unchanged real metadata permits a fresh retry.
          fixture::selected = fixture::boundary::none;
          accepted          = tested::complete_acceptance(t.path / "ledger", *request, *candidate, info->identity, receipt);
          REQUIRE(accepted);
          CHECK_FALSE(accepted->replayed);
        } else
          REQUIRE(accepted);
        CHECK(receipt.phase == completion_phase::committed);
        CHECK(receipt.saved_result == accepted->result);
        CHECK(counts(t)[3] == 1);
        CHECK(counts(t)[4] == 2);
        CHECK(counts(t)[5] == 1);
        fixture::selected = fixture::boundary::none;
        completion_receipt replay_receipt;
        auto replay = tested::complete_acceptance(t.path / "ledger", *request, *candidate, info->identity, replay_receipt);
        REQUIRE(replay);
        CHECK(replay->replayed);
        CHECK(replay->result == accepted->result);
        CHECK(replay_receipt.phase == completion_phase::replayed);
      }
    }
  }
}
