// Request intent validation: explicit normalized arguments, codec2 selectors and moved-from refusal.
#include <catch2/catch_test_macros.hpp>
import std;
import planar.engine.acceptance.intent;
using namespace planar::engine;
using namespace planar::engine::acceptance;
namespace {
bool refused(const std::expected<request_intent, preparation_error>& result, preparation_error_code code) {
  return !result && result.error().code == code;
}
} // namespace
TEST_CASE("intent create returns exact immutable legacy data", "[intent]") {
  auto intent = request_intent::create("req-1", "actor", 7, "/work/space", "/work/registry");
  REQUIRE(intent);
  auto data = intent->data();
  REQUIRE(data);
  CHECK((*data)->request_id == "req-1");
  CHECK((*data)->fields.operation == "load");
  CHECK((*data)->fields.actor == "actor");
  CHECK((*data)->fields.expected_head == "7");
  CHECK((*data)->fields.workspace_argument == "/work/space");
  CHECK((*data)->fields.registry_argument == "/work/registry");
  CHECK((*data)->expected_head == std::optional<std::int64_t>{7});
  CHECK_FALSE((*data)->encoded.empty());
  CHECK_FALSE((*data)->version2.has_value());
  auto initial = request_intent::create("req-2", "actor", std::nullopt, "/w", "/r");
  REQUIRE(initial);
  CHECK((*initial->data())->fields.expected_head == "none");
}
TEST_CASE("intent create refuses invalid arguments", "[intent]") {
  constexpr auto bad = preparation_error_code::invalid_request;
  CHECK(refused(request_intent::create("", "a", 1, "/w", "/r"), bad));
  CHECK(refused(request_intent::create("bad id", "a", 1, "/w", "/r"), bad));
  CHECK(refused(request_intent::create("bad/id", "a", 1, "/w", "/r"), bad));
  CHECK(request_intent::create(std::string(128, 'a'), "a", 1, "/w", "/r"));
  CHECK(refused(request_intent::create(std::string(129, 'a'), "a", 1, "/w", "/r"), preparation_error_code::resource_limit));
  CHECK(refused(request_intent::create("id", "", 1, "/w", "/r"), bad));
  CHECK(refused(request_intent::create("id", "a", 0, "/w", "/r"), bad));
  CHECK(refused(request_intent::create("id", "a", -3, "/w", "/r"), bad));
  CHECK(refused(request_intent::create("id", "a", 1, "w", "/r"), bad));
  CHECK(refused(request_intent::create("id", "a", 1, "/w/../x", "/r"), bad));
  CHECK(refused(request_intent::create("id", "a", 1, "/w//x", "/r"), bad));
  CHECK(refused(request_intent::create("id", "a", 1, "/w", "r"), bad));
  CHECK(refused(request_intent::create("id", "a", 1, "/w", "/r/./x"), bad));
  CHECK(refused(request_intent::create("id", "a", 1, "/w", ""), bad));
}
TEST_CASE("intent create_v2 carries the optional profile selector", "[intent]") {
  auto embedded = request_intent::create_v2("id", "actor", 2, "/w", "/r");
  REQUIRE(embedded);
  auto edata = embedded->data();
  REQUIRE(edata);
  REQUIRE((*edata)->version2.has_value());
  CHECK_FALSE((*edata)->version2->profile_argument.has_value());
  auto expected = history::encode_request_v2(*(*edata)->version2);
  REQUIRE(expected);
  CHECK((*edata)->encoded == *expected);

  auto profile = request_intent::create_v2("id", "actor", 2, "/w", "/r", std::string_view{"/p/profile"});
  REQUIRE(profile);
  auto pdata = profile->data();
  REQUIRE(pdata);
  REQUIRE((*pdata)->version2.has_value());
  CHECK((*pdata)->version2->profile_argument == std::optional<std::string>{"/p/profile"});
  CHECK((*pdata)->version2->workspace_argument == "/w");
  auto encoded = history::encode_request_v2(*(*pdata)->version2);
  REQUIRE(encoded);
  CHECK((*pdata)->encoded == *encoded);
  CHECK((*pdata)->encoded != (*edata)->encoded);

  constexpr auto bad = preparation_error_code::invalid_request;
  CHECK(refused(request_intent::create_v2("id", "actor", 2, "/w", "/r", std::string_view{"p/profile"}), bad));
  CHECK(refused(request_intent::create_v2("id", "actor", 2, "/w", "/r", std::string_view{"/p/../profile"}), bad));
  CHECK(refused(request_intent::create_v2("id", "actor", 2, "/w", "/r", std::string_view{""}), bad));
}
TEST_CASE("intent moved-from data refuses", "[intent]") {
  auto a = request_intent::create("id", "actor", 1, "/w", "/r");
  REQUIRE(a);
  auto b     = std::move(*a);
  auto moved = a->data();
  REQUIRE_FALSE(moved);
  CHECK(moved.error().code == preparation_error_code::moved_from);
  CHECK(b.data());
}
