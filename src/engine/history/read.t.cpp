// Recorded-history public projection and snapshot composition without live policy.
#include <catch2/catch_test_macros.hpp>
#include <unistd.h>
import std;
import planar.engine.history.read;
import planar.engine.json_codec;
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
