// Independent fixed v1 wire vectors and pure typed event/result refusal checks.
#include <catch2/catch_test_macros.hpp>
import std;
import planar.engine.ledger.history_codec;
using namespace planar::engine::ledger;
namespace {
constexpr std::string_view init =
    R"({"after":{"active_revision":"none"},"before":{"active_revision":"none"},"reason":"initialized","references":{},"version":1})";
constexpr std::string_view initial =
    R"({"after":{"active_revision":"1"},"before":{"active_revision":"none"},"reason":"declaration_accepted","references":{"interpretation":"8","registry":"3","source":"2","stdlib":"5","versions":"9"},"version":1})";
constexpr std::string_view changed =
    R"({"after":{"active_revision":"7"},"before":{"active_revision":"1"},"reason":"declaration_accepted","references":{"interpretation":"18","registry":"13","source":"12","stdlib":"15","versions":"19"},"version":1})";
constexpr std::string_view noop =
    R"({"after":{"active_revision":"7"},"before":{"active_revision":"7"},"reason":"identical_declaration","references":{"interpretation":"18","registry":"13","source":"12","stdlib":"15","versions":"19"},"version":1})";
constexpr std::string_view revert =
    R"({"after":{"active_revision":"11"},"before":{"active_revision":"7"},"reason":"declaration_accepted","references":{"interpretation":"8","registry":"3","source":"2","stdlib":"5","versions":"9"},"version":1})";
constexpr std::string_view saved =
    R"({"event":"12","kind":"load","ledger":"0123456789abcdef0123456789abcdef","outcome":"noop","parent":"1","request_id":"Retry_42.a:b-c","revision":"7","version":1})";
std::string replace(std::string_view input, std::string_view from, std::string_view to) {
  std::string s(input);
  auto        pos = s.find(from);
  REQUIRE(pos != s.npos);
  s.replace(pos, from.size(), to);
  return s;
}
void bad_event(std::string_view s, history_codec_error expected = history_codec_error::malformed) {
  auto r = decode_history_event(s);
  REQUIRE_FALSE(r);
  CHECK(r.error() == expected);
}
void bad_result(std::string_view s, history_codec_error expected = history_codec_error::malformed) {
  auto r = decode_load_result(s);
  REQUIRE_FALSE(r);
  CHECK(r.error() == expected);
}
const event_references first{2, 3, 5, 8, 9}, second{12, 13, 15, 18, 19};
static_assert(std::is_enum_v<history_codec_error> && std::is_enum_v<event_reason> && std::is_enum_v<load_outcome>);
static_assert(std::is_same_v<decltype(history_event::before), std::optional<std::int64_t>>);
static_assert(std::is_same_v<decltype(load_result::parent), std::optional<std::int64_t>>);
} // namespace
TEST_CASE("ledger history codec initialization exact existing format payload", "[replacement][ledger][history_codec]") {
  const history_event e{{}, {}, event_reason::initialized, {}};
  auto                encoded = encode_history_event(e);
  REQUIRE(encoded);
  CHECK(*encoded == init);
  auto decoded = decode_history_event(init);
  REQUIRE(decoded);
  CHECK(*decoded == e);
}
TEST_CASE("ledger history codec accepted changed noop and reverted independent event wires",
          "[replacement][ledger][history_codec]") {
  const std::array cases{std::pair{initial, history_event{{}, 1, event_reason::declaration_accepted, first}},
                         std::pair{changed, history_event{1, 7, event_reason::declaration_accepted, second}},
                         std::pair{noop, history_event{7, 7, event_reason::identical_declaration, second}},
                         std::pair{revert, history_event{7, 11, event_reason::declaration_accepted, first}}};
  for (const auto& [wire, e] : cases) {
    auto encoded = encode_history_event(e);
    REQUIRE(encoded);
    CHECK(*encoded == wire);
    auto decoded = decode_history_event(wire);
    REQUIRE(decoded);
    CHECK(*decoded == e);
  }
}
TEST_CASE("ledger history codec saved load result keeps returned revision parent on noop",
          "[replacement][ledger][history_codec]") {
  const load_result r{"0123456789abcdef0123456789abcdef", "Retry_42.a:b-c", load_outcome::noop, 7, 1, 12};
  auto              encoded = encode_load_result(r);
  REQUIRE(encoded);
  CHECK(*encoded == saved);
  auto decoded = decode_load_result(saved);
  REQUIRE(decoded);
  CHECK(*decoded == r);
  const std::string initial_result =
      R"({"event":"2","kind":"load","ledger":"0123456789abcdef0123456789abcdef","outcome":"accepted","parent":"none","request_id":"a","revision":"1","version":1})";
  const load_result first_result{r.ledger, "a", load_outcome::accepted, 1, {}, 2};
  auto              first_encoded = encode_load_result(first_result);
  REQUIRE(first_encoded);
  CHECK(*first_encoded == initial_result);
  auto first_decoded = decode_load_result(initial_result);
  REQUIRE(first_decoded);
  CHECK(*first_decoded == first_result);
}
TEST_CASE("ledger history codec maximum signed IDs and nonconsecutive identities roundtrip",
          "[replacement][ledger][history_codec]") {
  constexpr auto    max = std::numeric_limits<std::int64_t>::max();
  const std::string wire =
      R"({"after":{"active_revision":"9223372036854775807"},"before":{"active_revision":"9223372036854775806"},"reason":"declaration_accepted","references":{"interpretation":"9223372036854775807","registry":"9223372036854775807","source":"9223372036854775807","stdlib":"9223372036854775807","versions":"9223372036854775807"},"version":1})";
  const history_event e{max - 1, max, event_reason::declaration_accepted, event_references{max, max, max, max, max}};
  auto                encoded = encode_history_event(e);
  REQUIRE(encoded);
  CHECK(*encoded == wire);
  auto decoded = decode_history_event(wire);
  REQUIRE(decoded);
  CHECK(*decoded == e);
  const std::string result_wire =
      R"({"event":"9223372036854775807","kind":"load","ledger":"0123456789abcdef0123456789abcdef","outcome":"accepted","parent":"9223372036854775806","request_id":"max","revision":"9223372036854775807","version":1})";
  const load_result r{"0123456789abcdef0123456789abcdef", "max", load_outcome::accepted, max, max - 1, max};
  auto              result = encode_load_result(r);
  REQUIRE(result);
  CHECK(*result == result_wire);
  auto back = decode_load_result(result_wire);
  REQUIRE(back);
  CHECK(*back == r);
}
TEST_CASE("ledger history codec event exact field sets and types refuse malformed schema",
          "[replacement][ledger][history_codec]") {
  for (auto wire : {replace(init, "\"version\":1", "\"version\":1,\"extra\":0"), replace(init, ",\"version\":1", ""),
                    replace(init, "\"version\":1", "\"version\":\"1\""), replace(init, "\"references\":{}", "\"references\":[]"),
                    replace(init, "\"active_revision\":\"none\"", "\"active_revision\":\"none\",\"extra\":false"),
                    replace(initial, "\"source\":\"2\",", ""), replace(initial, "\"source\":\"2\"", "\"source\":2"),
                    replace(initial, "\"versions\":\"9\"", "\"versions\":\"9\",\"extra\":\"1\""),
                    replace(init, "\"reason\":\"initialized\"", "\"reason\":\"unknown\"")})
    bad_event(wire);
  for (auto wire : {"", "null", "[]", "true", "{", "{\"version\":1}", "{\"version\":null}"})
    bad_event(wire);
}
TEST_CASE("ledger history codec result exact field sets and types refuse malformed schema",
          "[replacement][ledger][history_codec]") {
  for (auto field : {"event", "kind", "ledger", "outcome", "parent", "request_id", "revision", "version"}) {
    std::string wire(saved);
    auto        start = wire.find('"' + std::string(field) + '"');
    REQUIRE(start != wire.npos);
    auto end = wire.find(',', start);
    if (end == wire.npos) {
      wire.erase(start - 1, wire.size() - start);
    } else
      wire.erase(start, end - start + 1);
    bad_result(wire);
  }
  for (auto wire :
       {replace(saved, "\"version\":1", "\"version\":1,\"extra\":0"), replace(saved, "\"kind\":\"load\"", "\"kind\":\"other\""),
        replace(saved, "\"outcome\":\"noop\"", "\"outcome\":\"failed\""), replace(saved, "\"parent\":\"1\"", "\"parent\":null"),
        replace(saved, "\"revision\":\"7\"", "\"revision\":7"), replace(saved, "\"event\":\"12\"", "\"event\":true"),
        replace(saved, "\"ledger\":\"0123456789abcdef0123456789abcdef\"", "\"ledger\":{}")})
    bad_result(wire);
}
TEST_CASE("ledger history codec canonical decimal and optional head token refusals", "[replacement][ledger][history_codec]") {
  for (auto token : {"0", "01", "+1", "-1", "1.0", "1e1", "9223372036854775808", "99999999999999999999999999", "none", "", " 1",
                     "1 ", "NONE"}) {
    bad_event(replace(initial, "\"source\":\"2\"", std::string("\"source\":\"") + token + "\""));
    bad_result(replace(saved, "\"revision\":\"7\"", std::string("\"revision\":\"") + token + "\""));
  }
  for (auto token : {"0", "01", "+1", "-1", "None", "", "9223372036854775808"}) {
    bad_event(replace(init, "\"none\"", std::string("\"") + token + "\""));
    bad_result(replace(saved, "\"parent\":\"1\"", std::string("\"parent\":\"") + token + "\""));
  }
}
TEST_CASE("ledger history codec typed state and enum validation refuses invalid encoding",
          "[replacement][ledger][history_codec]") {
  for (const auto& e :
       std::array{history_event{1, {}, event_reason::initialized, {}}, history_event{{}, {}, event_reason::initialized, first},
                  history_event{{}, {}, event_reason::declaration_accepted, first},
                  history_event{7, 7, event_reason::declaration_accepted, first},
                  history_event{8, 7, event_reason::declaration_accepted, first},
                  history_event{{}, 7, event_reason::identical_declaration, first},
                  history_event{1, 7, event_reason::identical_declaration, first},
                  history_event{7, 7, event_reason::identical_declaration, {}},
                  history_event{0, 7, event_reason::declaration_accepted, first},
                  history_event{{}, 1, static_cast<event_reason>(99), first},
                  history_event{{}, 1, event_reason::declaration_accepted, event_references{1, 1, 1, -1, 1}}})
    CHECK_FALSE(encode_history_event(e));
  bad_event(replace(initial, "\"before\":{\"active_revision\":\"none\"}", "\"before\":{\"active_revision\":\"1\"}"));
  bad_event(replace(noop, "\"before\":{\"active_revision\":\"7\"}", "\"before\":{\"active_revision\":\"1\"}"));
  auto r = *decode_load_result(saved);
  for (auto parent : {std::int64_t{0}, std::int64_t{-1}, std::int64_t{7}, std::int64_t{8}}) {
    auto changed   = r;
    changed.parent = parent;
    CHECK_FALSE(encode_load_result(changed));
    bad_result(replace(saved, "\"parent\":\"1\"", std::string("\"parent\":\"") + std::to_string(parent) + "\""));
  }
  r.outcome = static_cast<load_outcome>(99);
  CHECK_FALSE(encode_load_result(r));
  r          = *decode_load_result(saved);
  r.revision = 0;
  CHECK_FALSE(encode_load_result(r));
  r       = *decode_load_result(saved);
  r.event = -1;
  CHECK_FALSE(encode_load_result(r));
}
TEST_CASE("ledger history codec ledger identity and bounded ASCII request grammar", "[replacement][ledger][history_codec]") {
  auto r = *decode_load_result(saved);
  for (auto ledger :
       {std::string(31, 'a'), std::string(33, 'a'), std::string(32, 'A'), std::string(32, 'g'), std::string(32, '\0')}) {
    auto changed   = r;
    changed.ledger = ledger;
    CHECK_FALSE(encode_load_result(changed));
  }
  for (auto request : {std::string{}, std::string(129, 'a'), std::string("with space"), std::string("a/b"), std::string("é"),
                       std::string("x\0y", 3), std::string("x\ny")}) {
    auto changed       = r;
    changed.request_id = request;
    CHECK_FALSE(encode_load_result(changed));
  }
  r.request_id = std::string(128, 'Z');
  auto encoded = encode_load_result(r);
  REQUIRE(encoded);
  auto decoded = decode_load_result(*encoded);
  REQUIRE(decoded);
  CHECK(*decoded == r);
  bad_result(replace(saved, "Retry_42.a:b-c", "a/b"));
  bad_result(replace(saved, "0123456789abcdef0123456789abcdef", "0123456789ABCDEF0123456789abcdef"));
}
TEST_CASE("ledger history codec duplicate escape whitespace ordering and trailing bytes refuse",
          "[replacement][ledger][history_codec]") {
  for (auto wire :
       {replace(init, "\"version\":1", "\"version\":1,\"version\":1"),
        replace(init, "\"references\":{}", "\"references\":{},\"references\":{}"),
        replace(init, "initialized", "initializ\\u0065d"), replace(init, "\"version\":1", "\"version\":1.0"),
        replace(init, "\"version\":1", "\"version\":1e0"), replace(init, "\"after\":", "\"after\" :"), std::string(init) + "\n",
        std::string(init) + "{}", std::string(init) + std::string(1, '\0'), std::string(" ") + std::string(init),
        replace(init, "\"after\":{\"active_revision\":\"none\"},\"before\":{\"active_revision\":\"none\"}",
                "\"before\":{\"active_revision\":\"none\"},\"after\":{\"active_revision\":\"none\"}"),
        replace(init, "\"active_revision\":\"none\"", "\"active_revision\":\"none\",\"active_revision\":\"none\"")})
    bad_event(wire);
  bad_event(replace(initial, "\"source\":\"2\"", "\"source\":\"2\",\"source\":\"2\""));
  bad_result(replace(saved, "\"event\":\"12\"", "\"event\":\"12\",\"event\":\"12\""));
  bad_result(replace(saved, "load", "lo\\u0061d"));
  bad_result(std::string(saved) + "\n");
  bad_result(replace(saved, "\"event\":", "\"event\" :"));
}
TEST_CASE("ledger history codec unsupported versions and redacted fixed byte ceiling failures",
          "[replacement][ledger][history_codec]") {
  for (auto v : {"0", "2", "-1", "2147483647"}) {
    bad_event(replace(init, "\"version\":1", std::string("\"version\":") + v), history_codec_error::unsupported_version);
    bad_result(replace(saved, "\"version\":1", std::string("\"version\":") + v), history_codec_error::unsupported_version);
  }
  // There is no valid fixed-schema v1 event/result near one MiB; no padding extension is permitted.
  const std::string at_limit(1048576, ' '), over_limit(1048577, ' ');
  bad_event(at_limit);
  bad_result(at_limit);
  bad_event(over_limit, history_codec_error::resource_limit);
  bad_result(over_limit, history_codec_error::resource_limit);
  bad_event("/private/operator/secret");
  bad_result("/private/operator/secret");
  static_assert(std::is_same_v<decltype(decode_history_event(init).error()), history_codec_error&&>);
}
