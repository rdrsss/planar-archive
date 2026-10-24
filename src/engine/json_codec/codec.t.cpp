// Glaze-backed structural codec regression and boundary tests.
#include <catch2/catch_test_macros.hpp>
import std;
import planar.engine.json_codec;
namespace {
using namespace planar::engine;
std::shared_ptr<value> node(value_kind kind) {
  auto v  = std::make_shared<value>();
  v->kind = kind;
  return v;
}
value_ptr text(std::string s) {
  auto v    = node(value_kind::string);
  v->string = std::move(s);
  return v;
}
} // namespace
TEST_CASE("codec Glaze syntax and schema boundaries", "[codec]") {
  for (auto input : {"null", "1.0", "1e0", "1E+0", "2147483648", "-2147483649", "true false", "[1,]", "{\"a\":1,\"a\":2}",
                     "{\"a\":1,\"\\u0061\":2}", "\"\\u0000\"", "\"\\uD800\"", "\"\\uDC00\"", "\"\\x00\"", "{\"\\u0000\":1}", "01",
                     "+1", "", "  "}) {
    INFO(input);
    CHECK_FALSE(codec_decode(input));
  }
  for (auto input : {"-2147483648", "2147483647", "-0", "{}", "[]", "true", "\"é/\\n\"", "\"\\uD83D\\uDE00\""}) {
    INFO(input);
    CHECK(codec_decode(input));
  }
  CHECK_FALSE(codec_decode(std::string("\"\xff\"", 3)));
  CHECK_FALSE(codec_decode(std::string("\"\x01\"", 3)));
  CHECK_FALSE(codec_decode(std::string("\"a\0b\"", 5)));
}
TEST_CASE("codec decode exact resource ceilings", "[codec]") {
  std::string input(128, '[');
  input += std::string(128, ']');
  CHECK(codec_decode(input));
  input      = '[' + input + ']';
  auto depth = codec_decode(input);
  REQUIRE_FALSE(depth);
  CHECK(depth.error().detail.code == "resource_limit");
  CHECK(depth.error().detail.limit == "json_depth");
  CHECK(depth.error().detail.maximum == 128);
  CHECK(codec_decode("[true]", {6, 1, 2}));
  CHECK_FALSE(codec_decode("[true]", {6, 1, 1}));
  CHECK_FALSE(codec_decode("[true]", {5, 1, 2}));
  input = "[";
  for (int i = 0; i < 99999; ++i) {
    if (i)
      input += ',';
    input += '0';
  }
  input += ']';
  CHECK(codec_decode(input));
  input.insert(1, "0,");
  auto nodes = codec_decode(input);
  REQUIRE_FALSE(nodes);
  CHECK(nodes.error().detail.code == "resource_limit");
  CHECK(nodes.error().detail.limit == "json_nodes");
  CHECK(nodes.error().detail.maximum == 100000);
  input = '"' + std::string(1048574, 'a') + '"';
  CHECK(codec_decode(input));
  input.insert(1, "a");
  CHECK_FALSE(codec_decode(input));
  input                    = '{' + std::string(1048576, ' ');
  auto oversized_malformed = codec_decode(input);
  REQUIRE_FALSE(oversized_malformed);
  CHECK(oversized_malformed.error().detail.code == "resource_limit");
  CHECK(oversized_malformed.error().detail.limit == "input_bytes");
  CHECK(oversized_malformed.error().detail.maximum == 1048576);
  CHECK(oversized_malformed.error().offset == 0);
}
TEST_CASE("codec strict Glaze object separators and offsets", "[codec][strict-object]") {
  for (auto [input, offset] :
       std::initializer_list<std::pair<std::string_view, std::size_t>>{{"{\"a\":1,}", 7},
                                                                       {"{\"a\":1 \"b\":2}", 7},
                                                                       {"{\"outer\":{\"a\":1,}}", 16},
                                                                       {"{\"outer\":{\"a\":1 \"b\":2}}", 16},
                                                                       {"[{\"a\":1,}]", 8},
                                                                       {"[{\"a\":1 \"b\":2}]", 8}}) {
    INFO(input);
    auto decoded = codec_decode(input);
    CHECK_FALSE(decoded);
    if (!decoded) {
      CHECK(decoded.error().detail.code == "syntax");
      CHECK(decoded.error().offset == offset);
    }
  }
}
TEST_CASE("codec strict validation preserves object values and decoded duplicate policy", "[codec]") {
  auto decoded = codec_decode(" \t{ \"a\" : 1 ,\r\n \"b\" : { \"c\" : [ true , {} ] } } \n");
  REQUIRE(decoded);
  auto encoded = codec_encode(*decoded);
  REQUIRE(encoded);
  CHECK(*encoded == "{\"a\":1,\"b\":{\"c\":[true,{}]}}");
  for (auto input : {"{\"a\":1,\"\\u0061\":2}", "{\"outer\":{\"a\":1,\"\\u0061\":2}}"}) {
    INFO(input);
    auto duplicate = codec_decode(input);
    REQUIRE_FALSE(duplicate);
    CHECK(duplicate.error().detail.code == "schema");
  }
}
TEST_CASE("codec encode Glaze escaping and unsigned UTF8 key order", "[codec]") {
  auto v       = node(value_kind::record);
  v->fields    = {{"é", text("é/\\\"")}, {"z", text("z")}, {"a", text(std::string("\0\t\n\r\x1f", 5))}};
  auto encoded = codec_encode(v);
  REQUIRE(encoded);
  CHECK(*encoded == "{\"a\":\"\\u0000\\t\\n\\r\\u001F\",\"z\":\"z\",\"é\":\"é/\\\\\\\"\"}");
  std::string controls;
  for (int c = 0; c < 32; ++c)
    controls += static_cast<char>(c);
  auto all = codec_encode(text(controls));
  REQUIRE(all);
  CHECK(all->find('\\') != std::string::npos);
  CHECK_FALSE(codec_encode(text(std::string("\xff", 1))));
}
TEST_CASE("codec encode refuses cycles tags and exact bounds", "[codec]") {
  CHECK_FALSE(codec_encode(node(value_kind::job)));
  CHECK_FALSE(codec_encode(nullptr));
  auto cycle = node(value_kind::list);
  cycle->items.push_back(cycle);
  CHECK_FALSE(codec_encode(cycle));
  cycle->items.clear();
  auto list = node(value_kind::list);
  list->items.push_back(text("x"));
  CHECK(codec_encode(list, {5, 1, 2}));
  CHECK_FALSE(codec_encode(list, {4, 1, 2}));
  CHECK_FALSE(codec_encode(list, {5, 0, 2}));
  CHECK_FALSE(codec_encode(list, {5, 1, 1}));
  value_ptr nested = node(value_kind::list);
  for (int i = 1; i < 256; ++i) {
    auto outer = node(value_kind::list);
    outer->items.push_back(nested);
    nested = outer;
  }
  CHECK(codec_encode(nested));
  auto outer = node(value_kind::list);
  outer->items.push_back(nested);
  CHECK_FALSE(codec_encode(outer));
  auto exact = codec_encode(text(std::string(16777214, 'a')));
  REQUIRE(exact);
  CHECK(exact->size() == 16777216);
  CHECK_FALSE(codec_encode(text(std::string(16777215, 'a'))));
  CHECK_FALSE(codec_encode(text(std::string(3000000, '\0'))));
}

TEST_CASE("codec diagnostics retain safe offsets and resource details", "[codec]") {
  auto numeric = codec_decode("[0,1e0]");
  REQUIRE_FALSE(numeric);
  CHECK(numeric.error().detail.code == "schema");
  CHECK(numeric.error().offset == 3);
  auto syntax = codec_decode("[0,?]");
  REQUIRE_FALSE(syntax);
  CHECK(syntax.error().detail.code == "syntax");
  CHECK(syntax.error().offset == 3);
  auto nodes = codec_decode("[0,1]", {100, 10, 2});
  REQUIRE_FALSE(nodes);
  CHECK(nodes.error().detail.limit == "json_nodes");
  CHECK(nodes.error().detail.maximum == 2);
  CHECK(nodes.error().offset == 3);
  CHECK_FALSE(codec_decode(std::string("true\0false", 10)));
  auto empty_object = codec_decode("{}");
  auto empty_array  = codec_decode("[]");
  REQUIRE(empty_object);
  REQUIRE(empty_array);
  CHECK((*empty_object)->kind == value_kind::record);
  CHECK((*empty_array)->kind == value_kind::list);
  auto encoded = codec_encode(*empty_object);
  REQUIRE(encoded);
  CHECK(*encoded == "{}");
}

TEST_CASE("codec encoder enforces four million visits before expansion allocation", "[codec]") {
  auto zero     = node(value_kind::integer);
  auto repeated = node(value_kind::list);
  repeated->items.assign(3999999, zero);
  std::size_t    calls = 0;
  codec_reserver hook{&calls, [](void* context, const codec_reservation& measured) -> std::expected<void, diagnostic> {
                        ++*static_cast<std::size_t*>(context);
                        CHECK(measured.visits == 4000000);
                        CHECK(measured.array_entries == 3999999);
                        return {};
                      }};
  auto           exact = codec_encode_reserved(repeated, hook);
  REQUIRE(exact);
  CHECK(exact->size() == 7999999);
  repeated->items.push_back(zero);
  CHECK(calls == 1);
  auto excessive = codec_encode_reserved(repeated, hook);
  CHECK(calls == 1);
  REQUIRE_FALSE(excessive);
  CHECK(excessive.error().detail.limit == "json_nodes");
  CHECK(excessive.error().detail.maximum == 4000000);
}

namespace {
struct reservation_budget {
  codec_reservation                      measured{};
  std::size_t                            calls     = 0;
  std::size_t                            remaining = std::numeric_limits<std::size_t>::max();
  int                                    failure   = 0;
  static std::expected<void, diagnostic> reserve(void* context, const codec_reservation& measured) {
    auto& self = *static_cast<reservation_budget*>(context);
    ++self.calls;
    self.measured = measured;
    if (self.failure == 1)
      throw std::bad_alloc{};
    if (self.failure == 2)
      throw std::length_error("private");
    if (self.failure == 3)
      throw std::runtime_error("private");
    if (self.failure == 4 || measured.total_bytes > self.remaining)
      return std::unexpected(diagnostic{"resource_limit", "first native refusal", "native_bytes", 536870912});
    self.remaining -= measured.total_bytes;
    return {};
  }
  codec_reserver hook() {
    return {this, reserve};
  }
};
} // namespace
TEST_CASE("codec reservation measures repeated copies and deterministic capacity", "[codec][reservation]") {
  auto shared    = text("xyz");
  auto list      = node(value_kind::list);
  list->items    = {shared, shared};
  auto record    = node(value_kind::record);
  record->fields = {{"ab", list}, {"c", shared}};
  reservation_budget budget;
  auto               result = codec_encode_reserved(record, budget.hook());
  REQUIRE(result);
  CHECK(*result == "{\"ab\":[\"xyz\",\"xyz\"],\"c\":\"xyz\"}");
  auto ordinary = codec_encode(record);
  REQUIRE(ordinary);
  CHECK(*ordinary == *result);
  const auto m = budget.measured;
  CHECK(budget.calls == 1);
  CHECK(m.visits == 5);
  CHECK(m.array_entries == 2);
  CHECK(m.object_entries == 2);
  CHECK(m.value_strings == 3);
  CHECK(m.key_bytes == 3);
  CHECK(m.value_bytes == 9);
  CHECK(m.minimum_bytes == result->size());
  const auto string_slack = (m.string_bytes - 12) / 5;
  CHECK(string_slack >= 1);
  CHECK(m.string_bytes == 12 + 5 * string_slack);
  CHECK(m.output_storage_bytes == m.output_capacity + string_slack);
  CHECK(m.output_capacity == 6 * m.minimum_bytes + 1024);
  CHECK(m.total_bytes == m.graph_bytes + m.string_bytes + m.workspace_bytes + m.output_storage_bytes);
  reservation_budget again;
  REQUIRE(codec_encode_reserved(record, again.hook()));
  CHECK(again.measured.total_bytes == m.total_bytes);
  CHECK(again.measured.graph_bytes == m.graph_bytes);
  CHECK(again.measured.workspace_bytes == m.workspace_bytes);
  auto unreserved = codec_encode_reserved(record, {});
  REQUIRE(unreserved);
  CHECK(*unreserved == *result);
}
TEST_CASE("codec reservation cumulative budget exact fit and sticky typed refusal", "[codec][reservation]") {
  auto               input = text(std::string(100, 'a'));
  reservation_budget first;
  REQUIRE(codec_encode_reserved(input, first.hook()));
  reservation_budget exact;
  exact.remaining = first.measured.total_bytes;
  REQUIRE(codec_encode_reserved(input, exact.hook()));
  CHECK(exact.remaining == 0);
  auto refused = codec_encode_reserved(input, exact.hook());
  REQUIRE_FALSE(refused);
  CHECK(refused.error().detail.code == "resource_limit");
  CHECK(refused.error().detail.message == "first native refusal");
  CHECK(refused.error().detail.limit == "native_bytes");
  CHECK(refused.error().detail.maximum == 536870912);
  CHECK(refused.error().offset == 0);
  reservation_budget short_budget;
  short_budget.remaining = first.measured.total_bytes - 1;
  auto short_result      = codec_encode_reserved(input, short_budget.hook());
  REQUIRE_FALSE(short_result);
  CHECK(short_result.error().detail.message == refused.error().detail.message);
  CHECK(short_budget.remaining == first.measured.total_bytes - 1);
}
TEST_CASE("codec reservation callbacks have a safe typed exception boundary", "[codec][reservation]") {
  for (int failure : {1, 2, 3, 4}) {
    reservation_budget budget;
    budget.failure = failure;
    auto result    = codec_encode_reserved(text("x"), budget.hook());
    REQUIRE_FALSE(result);
    CHECK(budget.calls == 1);
    CHECK(result.error().detail.code == (failure == 3 ? "internal" : "resource_limit"));
    CHECK(result.error().detail.message.find("private") == std::string::npos);
    if (failure < 3) {
      CHECK(result.error().detail.limit == "output_bytes");
      CHECK(result.error().detail.maximum == 16777216);
    }
  }
}
TEST_CASE("codec validation refusal precedes reservation and preserves ordinary errors", "[codec][reservation]") {
  auto cycle = node(value_kind::list);
  cycle->items.push_back(cycle);
  auto integer     = node(value_kind::integer);
  integer->integer = 2147483648;
  for (value_ptr input :
       {value_ptr{}, value_ptr(cycle), value_ptr(integer), text(std::string("\xff", 1)), value_ptr(node(value_kind::job))}) {
    reservation_budget budget;
    budget.failure = 3;
    auto result    = codec_encode_reserved(input, budget.hook());
    auto ordinary  = codec_encode(input);
    REQUIRE_FALSE(result);
    REQUIRE_FALSE(ordinary);
    CHECK(budget.calls == 0);
    CHECK(result.error().detail.code == ordinary.error().detail.code);
    CHECK(result.error().detail.limit == ordinary.error().detail.limit);
  }
  cycle->items.clear();
  reservation_budget budget;
  auto               result = codec_encode_reserved(nullptr, budget.hook(), {0, 0, 0});
  REQUIRE_FALSE(result);
  CHECK(result.error().detail.limit == "json_nodes");
  CHECK(budget.calls == 0);
}
TEST_CASE("codec reservation preserves depth leaf semantics and repeated container aliases", "[codec][reservation]") {
  value_ptr nested = text("x");
  for (int i = 0; i < 256; ++i) {
    auto outer = node(value_kind::list);
    outer->items.push_back(nested);
    nested = outer;
  }
  reservation_budget budget;
  REQUIRE(codec_encode_reserved(nested, budget.hook()));
  CHECK(budget.measured.visits == 257);
  CHECK(budget.measured.array_entries == 256);
  auto outer   = node(value_kind::list);
  outer->items = {nested};
  reservation_budget excessive;
  auto               result = codec_encode_reserved(outer, excessive.hook());
  REQUIRE_FALSE(result);
  CHECK(result.error().detail.limit == "json_depth");
  CHECK(excessive.calls == 0);
  auto empty   = node(value_kind::record);
  outer->items = {empty, empty};
  reservation_budget aliases;
  REQUIRE(codec_encode_reserved(outer, aliases.hook()));
  CHECK(aliases.measured.visits == 3);
}
TEST_CASE("codec reservation retains complete transport and escape expansion ceilings", "[codec][reservation]") {
  reservation_budget exact;
  auto               result = codec_encode_reserved(text(std::string(16777214, 'a')), exact.hook());
  REQUIRE(result);
  CHECK(result->size() == 16777216);
  CHECK(exact.calls == 1);
  reservation_budget excessive;
  CHECK_FALSE(codec_encode_reserved(text(std::string(16777215, 'a')), excessive.hook()));
  CHECK(excessive.calls == 0);
  reservation_budget escaped;
  auto               expansion = codec_encode_reserved(text(std::string(3000000, '\0')), escaped.hook());
  REQUIRE_FALSE(expansion);
  CHECK(escaped.calls == 1);
  CHECK(escaped.measured.minimum_bytes == 3000002);
  CHECK(expansion.error().detail.limit == "output_bytes");
}

namespace {
struct decode_budget {
  codec_decode_reservation                measured{};
  std::size_t                             requests = 0, bytes = 0, conversions = 0, deny = 0;
  bool                                    deny_conversion = false;
  codec_error                             supplied{{"resource_limit", "first decode refusal", "native_bytes", 536870912}, 73};
  static std::expected<void, codec_error> allocate(void* context, std::size_t bytes) {
    auto& s = *static_cast<decode_budget*>(context);
    ++s.requests;
    s.bytes += bytes;
    if (s.requests == s.deny)
      return std::unexpected(std::move(s.supplied));
    return {};
  }
  static std::expected<void, codec_error> convert(void* context, const codec_decode_reservation& m) {
    auto& s = *static_cast<decode_budget*>(context);
    ++s.conversions;
    s.measured = m;
    if (s.deny_conversion)
      return std::unexpected(std::move(s.supplied));
    return {};
  }
  codec_decode_reserver hook() {
    return {this, allocate, convert};
  }
};
} // namespace
TEST_CASE("codec reserved decode preserves graphs and exact ordinary refusals", "[codec][decode-reservation]") {
  for (auto input : {"true",
                     "-2147483648",
                     "2147483647",
                     "{}",
                     "[]",
                     "{\"\":0,\"x\":[true,{}]}",
                     "null",
                     "1.0",
                     "1e0",
                     "2147483648",
                     "[1,]",
                     "{\"a\":1,}",
                     "{\"a\":1 \"b\":2}",
                     "{\"a\":1,\"\\u0061\":2}",
                     "\"\\u0000\"",
                     "\"\\uD800\"",
                     "[",
                     "",
                     "  ",
                     "true false"}) {
    INFO(input);
    decode_budget budget;
    auto          actual   = codec_decode_reserved(input, budget.hook());
    auto          ordinary = codec_decode(input);
    REQUIRE(bool(actual) == bool(ordinary));
    if (actual) {
      CHECK(*codec_encode(*actual) == *codec_encode(*ordinary));
      CHECK(budget.conversions == 1);
    } else {
      CHECK(actual.error().detail.code == ordinary.error().detail.code);
      CHECK(actual.error().detail.message == ordinary.error().detail.message);
      CHECK(actual.error().detail.limit == ordinary.error().detail.limit);
      CHECK(actual.error().detail.maximum == ordinary.error().detail.maximum);
      CHECK(actual.error().offset == ordinary.error().offset);
      CHECK(budget.conversions == 0);
    }
  }
}
TEST_CASE("codec reserved decode charges exact output components and preserves owned first refusal",
          "[codec][decode-reservation]") {
  const std::string input = "{\"ab\":[\"xyz\",true],\"c\":7}";
  decode_budget     first;
  auto              r = codec_decode_reserved(input, first.hook());
  REQUIRE(r);
  const auto& m = first.measured;
  CHECK(m.visits == 5);
  CHECK(m.list_slots == 2);
  CHECK(m.map_entries == 2);
  CHECK(m.string_bytes == 0);
  CHECK(m.list_bytes == 2 * sizeof(value_ptr));
  CHECK(m.total_bytes == m.node_bytes + m.list_bytes + m.map_bytes + m.string_bytes + m.workspace_bytes);
  CHECK(first.requests > 0);
  for (std::size_t i = 1; i <= first.requests; ++i) {
    decode_budget denied;
    denied.deny  = i;
    auto refusal = codec_decode_reserved(input, denied.hook());
    REQUIRE_FALSE(refusal);
    CHECK(denied.requests == i);
    CHECK(denied.conversions == 0);
    CHECK(refusal.error().detail.message == "first decode refusal");
    CHECK(refusal.error().offset == 73);
  }
  decode_budget denied;
  denied.deny_conversion = true;
  auto refusal           = codec_decode_reserved(input, denied.hook());
  REQUIRE_FALSE(refusal);
  CHECK(denied.conversions == 1);
  CHECK(refusal.error().detail.maximum == 536870912);
  CHECK(refusal.error().offset == 73);
  value_ptr returned;
  {
    decode_budget temporary;
    auto          output = codec_decode_reserved(input, temporary.hook());
    REQUIRE(output);
    returned = std::move(*output);
  }
  CHECK(*codec_encode(returned) == *codec_encode(*r));
}
TEST_CASE("codec reserved decode callback exceptions and early guards remain typed", "[codec][decode-reservation]") {
  for (bool conversion : {false, true})
    for (int failure : {1, 2, 3}) {
      codec_decode_reserver hook;
      hook.context  = &failure;
      auto callback = +[](void* p, std::size_t) -> std::expected<void, codec_error> {
        if (*static_cast<int*>(p) == 1)
          throw std::bad_alloc{};
        if (*static_cast<int*>(p) == 2)
          throw std::length_error("private");
        throw std::runtime_error("private");
      };
      if (conversion)
        hook.convert = +[](void* p, const codec_decode_reservation&) -> std::expected<void, codec_error> {
          if (*static_cast<int*>(p) == 1)
            throw std::bad_alloc{};
          if (*static_cast<int*>(p) == 2)
            throw std::length_error("private");
          throw std::runtime_error("private");
        };
      else
        hook.allocate = callback;
      auto r = codec_decode_reserved("[true]", hook);
      REQUIRE_FALSE(r);
      CHECK(r.error().detail.code == (failure == 3 ? "internal" : "resource_limit"));
      CHECK(r.error().detail.message.find("private") == std::string::npos);
    }
  decode_budget budget;
  auto          oversized = codec_decode_reserved("[true]", budget.hook(), {5, 1, 2});
  REQUIRE_FALSE(oversized);
  CHECK(budget.requests == 0);
  CHECK(budget.conversions == 0);
  CHECK(oversized.error().detail.limit == "input_bytes");
  auto nodes = codec_decode_reserved("[true]", budget.hook(), {6, 1, 1});
  REQUIRE_FALSE(nodes);
  CHECK(nodes.error().detail.limit == "json_nodes");
  CHECK(nodes.error().offset == 1);
}
TEST_CASE("codec reserved decode supports independent nested concurrent and cumulative contexts", "[codec][decode-reservation]") {
  struct nested {
    std::size_t                             calls = 0;
    static std::expected<void, codec_error> allocate(void* p, std::size_t) {
      ++static_cast<nested*>(p)->calls;
      decode_budget separate;
      auto          r = codec_decode_reserved("{\"x\":[true]}", separate.hook());
      if (!r)
        return std::unexpected(std::move(r.error()));
      return {};
    }
  } state;
  REQUIRE(codec_decode_reserved("[\"abcdefghijklmnopqrstuvwxyz\"]", {&state, nested::allocate, nullptr}));
  CHECK(state.calls > 0);
  auto run = [](bool deny) {
    decode_budget state;
    state.deny = deny ? 1 : 0;
    auto a     = codec_decode_reserved("[true]", state.hook());
    if (bool(a) == deny)
      return false;
    if (deny)
      return state.requests == 1;
    const auto n = state.requests;
    auto       b = codec_decode_reserved("[true]", state.hook());
    return b && state.requests == 2 * n && state.conversions == 2;
  };
  auto a = std::async(std::launch::async, run, false);
  auto b = std::async(std::launch::async, run, true);
  CHECK(a.get());
  CHECK(b.get());
}

namespace {
struct projection_budget {
  codec_projection_reservation measured{};
  std::size_t                  calls = 0, remaining = std::numeric_limits<std::size_t>::max();
  int                          failure = 0;
  diagnostic                   supplied{"resource_limit", "original projection charge refusal", "native_bytes", 536870912};
  static std::expected<void, diagnostic> reserve(void* p, const codec_projection_reservation& measured) {
    auto& s = *static_cast<projection_budget*>(p);
    ++s.calls;
    s.measured = measured;
    if (s.failure == 1)
      throw std::bad_alloc{};
    if (s.failure == 2)
      throw std::length_error("private projection detail");
    if (s.failure == 3)
      throw std::runtime_error("private projection detail");
    if (s.failure == 4 || measured.total_bytes > s.remaining)
      return std::unexpected(std::move(s.supplied));
    s.remaining -= measured.total_bytes;
    return {};
  }
  codec_projection_reserver hook() {
    return {this, reserve};
  }
};
value_ptr projection_graph(std::span<const projection_section_view> sections) {
  auto list = node(value_kind::list);
  for (const auto& section : sections) {
    auto record        = node(value_kind::record);
    auto mandatory     = node(value_kind::boolean);
    mandatory->boolean = section.mandatory;
    record->fields = {{"id", text(std::string(section.id))}, {"text", text(std::string(section.text))}, {"mandatory", mandatory}};
    list->items.push_back(record);
  }
  auto version     = node(value_kind::integer);
  version->integer = 1;
  auto root        = node(value_kind::record);
  root->fields     = {{"version", version}, {"sections", list}};
  return root;
}
void projection_parity(std::span<const projection_section_view> sections) {
  const auto counted = codec_projection_count(sections);
  const auto encoded = codec_encode(projection_graph(sections));
  REQUIRE(encoded);
  REQUIRE(counted);
  CHECK(*counted == encoded->size());
}
} // namespace
TEST_CASE("codec X05 C2 B1 projection count matches unchanged Glaze encoding", "[codec][projection-count]") {
  projection_parity({});
  std::string controls = "ASCII \\\" café 世界 😀";
  for (char c = 1; c < 32; ++c)
    controls += c;
  const std::array sections{projection_section_view{"a", controls, false}, projection_section_view{"b", {}, true}};
  projection_parity(sections);
  const std::string max_id(64, 'a'), escaped(1048576, '\1');
  const std::array  maximum{projection_section_view{max_id, escaped, true}};
  projection_parity(maximum);
  std::array<std::string, 64>             ids;
  std::array<projection_section_view, 64> many;
  for (std::size_t i = 0; i < ids.size(); ++i) {
    ids[i]  = "section_" + std::to_string(i);
    many[i] = {ids[i], i % 2 ? std::string_view(controls) : std::string_view{}, bool(i % 2)};
  }
  projection_parity(many);
}
TEST_CASE("codec X05 C2 B1 projection encoded 16 MiB exact boundaries", "[codec][projection-count]") {
  std::array<std::string, 16>             ids;
  std::array<projection_section_view, 16> sections;
  std::string                             full(1048576, 'a'), last = full;
  for (std::size_t i = 0; i < ids.size(); ++i) {
    ids[i]      = "section_" + std::to_string(i);
    sections[i] = {ids[i], full, false};
  }
  // Ask the unchanged codec to measure framing using empty text, then fill to
  // each exact transport boundary without duplicating JSON grammar in the test.
  for (auto& s : sections)
    s.text = {};
  const auto framing = codec_encode(projection_graph(sections));
  REQUIRE(framing);
  for (int delta : {-1, 0, 1}) {
    for (auto& s : sections)
      s.text = full;
    last.resize(1048576 - framing->size() + delta, 'a');
    sections.back().text = last;
    auto counted         = codec_projection_count(sections);
    auto ordinary        = codec_encode(projection_graph(sections));
    if (delta <= 0) {
      REQUIRE(counted);
      REQUIRE(ordinary);
      CHECK(*counted == std::size_t(16777216 + delta));
      CHECK(*counted == ordinary->size());
    } else {
      REQUIRE_FALSE(counted);
      REQUIRE_FALSE(ordinary);
      CHECK(counted.error().kind == codec_projection_failure::output_limit);
      CHECK(counted.error().detail.detail.code == "resource_limit");
    }
  }
}
TEST_CASE("codec X05 C2 B1 projection validates all source bounds before charging", "[codec][projection-count]") {
  const std::string oversized(1048577, 'a'), max_id(65, 'a');
  for (auto id : {std::string_view{}, std::string_view{"A"}, std::string_view{"0a"}, std::string_view{"_a"},
                  std::string_view{"a-b"}, std::string_view{"é"}, std::string_view(max_id)}) {
    projection_budget budget;
    const std::array  input{projection_section_view{id, "text", false}};
    auto              r = codec_projection_count(input, budget.hook());
    REQUIRE_FALSE(r);
    CHECK(r.error().kind == codec_projection_failure::invalid_input);
    CHECK(budget.calls == 0);
  }
  for (auto text : {std::string_view(oversized), std::string_view("a\0b", 3), std::string_view("\xff", 1),
                    std::string_view("\xc0\xaf", 2), std::string_view("\xed\xa0\x80", 3)}) {
    projection_budget budget;
    const std::array  input{projection_section_view{"a", text, false}};
    auto              r = codec_projection_count(input, budget.hook());
    REQUIRE_FALSE(r);
    CHECK(r.error().kind == codec_projection_failure::invalid_input);
    CHECK(budget.calls == 0);
  }
  std::array<projection_section_view, 65> too_many{};
  projection_budget                       budget;
  CHECK_FALSE(codec_projection_count(too_many, budget.hook()));
  CHECK(budget.calls == 0);
  const std::array duplicate{projection_section_view{"a", {}, false}, projection_section_view{"a", {}, true}};
  CHECK_FALSE(codec_projection_count(duplicate, budget.hook()));
  CHECK(budget.calls == 0);
  std::array<std::string, 17>             ids;
  std::array<projection_section_view, 17> aggregate;
  const std::string                       full(1048576, 'a');
  for (std::size_t i = 0; i < ids.size(); ++i) {
    ids[i]       = "a" + std::to_string(i);
    aggregate[i] = {ids[i], full, false};
  }
  CHECK_FALSE(codec_projection_count(aggregate, budget.hook()));
  CHECK(budget.calls == 0);
}
TEST_CASE("codec X05 C2 B1 projection exact precharge cumulative denial and diagnostic ownership", "[codec][projection-count]") {
  projection_budget first;
  REQUIRE(codec_projection_count({}, first.hook()));
  CHECK(first.calls == 1);
  CHECK(first.measured.scratch_bytes == 6325248);
  CHECK(first.measured.workspace_bytes > 0);
  CHECK(first.measured.total_bytes == first.measured.scratch_bytes + first.measured.workspace_bytes);
  projection_budget exact;
  exact.remaining = first.measured.total_bytes;
  REQUIRE(codec_projection_count({}, exact.hook()));
  CHECK(exact.remaining == 0);
  auto deny = codec_projection_count({}, exact.hook());
  REQUIRE_FALSE(deny);
  CHECK(deny.error().kind == codec_projection_failure::resource);
  CHECK(deny.error().detail.detail.message == "original projection charge refusal");
  CHECK(deny.error().detail.detail.limit == "native_bytes");
  CHECK(deny.error().detail.detail.maximum == 536870912);
  CHECK(exact.calls == 2);
  projection_budget short_budget;
  short_budget.remaining = first.measured.total_bytes - 1;
  CHECK_FALSE(codec_projection_count({}, short_budget.hook()));
  CHECK(short_budget.remaining == first.measured.total_bytes - 1);
  // Even encoded failure has consumed the complete charge; no refunds.
  projection_budget overflow;
  auto              refused = codec_projection_count({}, overflow.hook(), 0);
  REQUIRE_FALSE(refused);
  CHECK(refused.error().kind == codec_projection_failure::output_limit);
  CHECK(overflow.remaining == std::numeric_limits<std::size_t>::max() - first.measured.total_bytes);
}
TEST_CASE("codec X05 C2 B1 projection callback exceptions remain typed", "[codec][projection-count]") {
  for (int failure : {1, 2, 3, 4}) {
    projection_budget budget;
    budget.failure = failure;
    auto r         = codec_projection_count({}, budget.hook());
    REQUIRE_FALSE(r);
    CHECK(budget.calls == 1);
    CHECK(r.error().kind == (failure == 3 ? codec_projection_failure::internal : codec_projection_failure::resource));
    CHECK(r.error().detail.detail.code == (failure == 3 ? "internal" : "resource_limit"));
    CHECK(r.error().detail.detail.message.find("private") == std::string::npos);
  }
}
TEST_CASE("codec X05 C2 B1 projection independent nested concurrent and borrowed immutable calls", "[codec][projection-count]") {
  struct nested {
    std::size_t                            calls = 0;
    static std::expected<void, diagnostic> reserve(void* p, const codec_projection_reservation&) {
      ++static_cast<nested*>(p)->calls;
      projection_budget other;
      auto              r = codec_projection_count({}, other.hook());
      if (!r)
        return std::unexpected(std::move(r.error().detail.detail));
      return {};
    }
  } owner;
  std::string      id = "owned", body = "stable café";
  const auto       before = body;
  const std::array sections{projection_section_view{id, body, true}};
  auto             r = codec_projection_count(sections, {&owner, nested::reserve});
  REQUIRE(r);
  CHECK(owner.calls == 1);
  CHECK(body == before);
  CHECK(sections[0].text.data() == body.data());
  const auto run = [](bool deny) {
    projection_budget budget;
    budget.failure = deny ? 4 : 0;
    auto r         = codec_projection_count({}, budget.hook());
    return bool(r) != deny && budget.calls == 1;
  };
  auto a = std::async(std::launch::async, run, false);
  auto b = std::async(std::launch::async, run, true);
  CHECK(a.get());
  CHECK(b.get());
  id.clear();
  body.clear();
  CHECK(*r > 0); // Returned count retains neither borrowed strings nor callback owner.
}
TEST_CASE("codec X05 C2 B1 projection tighter output ceiling and semantic name separation", "[codec][projection-count]") {
  auto empty = codec_projection_count({});
  REQUIRE(empty);
  CHECK(codec_projection_count({}, {}, *empty));
  auto short_result = codec_projection_count({}, {}, *empty - 1);
  REQUIRE_FALSE(short_result);
  CHECK(short_result.error().kind == codec_projection_failure::output_limit);
  CHECK(short_result.error().detail.detail.maximum == *empty - 1);
  // Reserved-name policy belongs to native protocol admission, outside counting.
  const std::array input{projection_section_view{"recorded_history", "x", true}, projection_section_view{"goal", "y", false}};
  projection_parity(input);
  CHECK(*codec_projection_count({}, {}, std::numeric_limits<std::size_t>::max()) == *empty);
}
