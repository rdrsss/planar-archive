// Isolated actual-allocation observation and faults; never resource routing.
#include <catch2/catch_test_macros.hpp>
#include <cstdlib>
#include <new>
import std;
import planar.engine.json_codec;
namespace observation {
struct event {
  std::size_t bytes;
  void*       address;
  bool        freed;
};
std::array<event, 300000> events{};
std::size_t               count = 0, fail = 0;
bool                      enabled = false;
void                      request(std::size_t n) {
  if (!enabled)
    return;
  if (count == events.size())
    std::abort();
  events[count++] = {n, nullptr, false};
  if (count == fail)
    throw std::bad_alloc{};
}
void allocated(void* p) {
  if (enabled)
    events[count - 1].address = p;
}
void freed(void* p) {
  if (!enabled || !p)
    return;
  for (std::size_t i = count; i; --i)
    if (events[i - 1].address == p && !events[i - 1].freed) {
      events[i - 1].freed = true;
      return;
    }
}
void start(std::size_t f = 0) {
  count   = 0;
  fail    = f;
  enabled = true;
}
} // namespace observation
void* operator new(std::size_t n) {
  observation::request(n);
  if (auto p = std::malloc(n ? n : 1)) {
    observation::allocated(p);
    return p;
  }
  throw std::bad_alloc{};
}
void operator delete(void* p) noexcept {
  observation::freed(p);
  std::free(p);
}
void* operator new[](std::size_t n) {
  return ::operator new(n);
}
void operator delete[](void* p) noexcept {
  ::operator delete(p);
}
void* operator new(std::size_t n, std::align_val_t a) {
  observation::request(n);
  void* p = nullptr;
  if (!posix_memalign(&p, static_cast<std::size_t>(a), n ? n : 1)) {
    observation::allocated(p);
    return p;
  }
  throw std::bad_alloc{};
}
void operator delete(void* p, std::align_val_t) noexcept {
  observation::freed(p);
  std::free(p);
}
void* operator new[](std::size_t n, std::align_val_t a) {
  return ::operator new(n, a);
}
void operator delete[](void* p, std::align_val_t a) noexcept {
  ::operator delete(p, a);
}
namespace {
using namespace planar::engine;
struct state {
  std::array<std::size_t, 300000> sizes{};
  std::size_t                     requests = 0, deny = 0, conversion_start = 0, conversions = 0;
  bool                            deny_conversion = false;
  codec_decode_reservation        measured{};
  codec_error                     supplied{{"resource_limit", std::string(4096, 'm'), std::string(1024, 'l'), 123456}, 73};
  static std::expected<void, codec_error> allocate(void* p, std::size_t bytes) {
    auto& s = *static_cast<state*>(p);
    if (s.requests == s.sizes.size())
      std::abort();
    s.sizes[s.requests++] = bytes;
    if (s.requests == s.deny)
      return std::unexpected(std::move(s.supplied));
    return {};
  }
  static std::expected<void, codec_error> convert(void* p, const codec_decode_reservation& m) {
    auto& s = *static_cast<state*>(p);
    ++s.conversions;
    s.conversion_start = observation::count;
    s.measured         = m;
    if (s.deny_conversion)
      return std::unexpected(std::move(s.supplied));
    return {};
  }
  codec_decode_reserver hook() {
    return {this, allocate, convert};
  }
};
bool equal(const value_ptr& a, const value_ptr& b) {
  if (a->kind != b->kind)
    return false;
  if (a->kind == value_kind::boolean)
    return a->boolean == b->boolean;
  if (a->kind == value_kind::integer)
    return a->integer == b->integer;
  if (a->kind == value_kind::string)
    return a->string == b->string;
  if (a->items.size() != b->items.size() || a->fields.size() != b->fields.size())
    return false;
  for (std::size_t i = 0; i < a->items.size(); ++i)
    if (!equal(a->items[i], b->items[i]))
      return false;
  auto i = a->fields.begin(), j = b->fields.begin();
  for (; i != a->fields.end(); ++i, ++j)
    if (i->first != j->first || !equal(i->second, j->second))
      return false;
  return true;
}
void check_cleanup() {
  for (std::size_t i = 0; i < observation::count; ++i)
    CHECK((!observation::events[i].address || observation::events[i].freed));
}
void compare(std::string_view input, codec_limits limits = {1048576, 128, 100000}) {
  auto baseline = codec_decode(input, limits);
  auto s        = std::make_unique<state>();
  observation::start();
  auto       actual    = codec_decode_reserved(input, s->hook(), limits);
  const auto count     = observation::count;
  observation::enabled = false;
  REQUIRE(bool(actual) == bool(baseline));
  if (actual) {
    CHECK(equal(*actual, *baseline));
    CHECK(s->conversions == 1);
    REQUIRE(s->conversion_start == s->requests);
    std::size_t conversion_bytes = 0;
    for (std::size_t i = 0; i < s->requests; ++i)
      CHECK(observation::events[i].bytes == s->sizes[i]);
    for (std::size_t i = s->conversion_start; i < count; ++i)
      conversion_bytes += observation::events[i].bytes;
    CHECK(conversion_bytes == s->measured.total_bytes - s->measured.workspace_bytes);
  } else {
    CHECK(s->conversions == 0);
    CHECK(actual.error().detail.code == baseline.error().detail.code);
    CHECK(actual.error().detail.message == baseline.error().detail.message);
    CHECK(actual.error().detail.limit == baseline.error().detail.limit);
    CHECK(actual.error().detail.maximum == baseline.error().detail.maximum);
    CHECK(actual.error().offset == baseline.error().offset);
    REQUIRE(count >= s->requests);
    for (std::size_t i = 0; i < s->requests; ++i)
      CHECK(observation::events[i].bytes == s->sizes[i]);
  }
  observation::enabled = true;
  actual               = std::unexpected(codec_error{});
  observation::enabled = false;
  check_cleanup();
}
} // namespace
TEST_CASE("D01-B actual requests equal derived conversion capacities and parser charges", "[d01-b]") {
  for (std::size_t n = 0; n <= 1024; ++n) {
    compare('"' + std::string(n, 'a') + '"');
    compare("{\"" + std::string(n, 'k') + "\":\"" + std::string(n, 'v') + "\",\"\":[true,{},[]]}");
  }
  for (auto n : {4095, 4096, 65536, 1048574})
    compare('"' + std::string(n, 'a') + '"');
  for (auto n : {0, 1, 2, 127, 128, 129})
    compare(std::string(n, '[') + std::string(n, ']'));
  std::string input = "[";
  for (int i = 0; i < 99999; ++i) {
    if (i)
      input += ',';
    input += '0';
  }
  input += ']';
  compare(input);
  input.insert(1, "0,");
  compare(input);
}
TEST_CASE("D01-B ordinary differential covers malformed escapes duplicates limits and mutations", "[d01-b]") {
  for (auto input : {"true",
                     "false",
                     "-0",
                     "-2147483648",
                     "2147483647",
                     "null",
                     "1.0",
                     "1e0",
                     "1E+0",
                     "2147483648",
                     "-2147483649",
                     "{}",
                     "[]",
                     "{\"a\":1,\"a\":2}",
                     "{\"a\":1,\"\\u0061\":2}",
                     "{\"a\":1 \"b\":2}",
                     "{\"a\":1,}",
                     "[1,]",
                     "[1,",
                     "[",
                     "{",
                     "\"abc",
                     "true false",
                     "  true \n",
                     "",
                     "  ",
                     "01",
                     "+1",
                     "\"\\u0000\"",
                     "{\"\\u0000\":1}",
                     "\"\\uD800\"",
                     "\"\\uDC00\"",
                     "\"\\x00\"",
                     "\"\\\"\\\\\\/\\b\\f\\n\\r\\t\\u0061\\uD83D\\uDE00\""})
    compare(input);
  compare(std::string("\"\xff\"", 3));
  compare(std::string("\"\x01\"", 3));
  compare(std::string("\"a\0b\"", 5));
  for (auto n : {0, 1, 22, 23, 24, 255, 256, 257, 65536, 1048574}) {
    std::string input = '"' + std::string(n, 'a') + '"';
    input.pop_back();
    compare(input);
    input += '\\';
    compare(input);
    input += 'u';
    compare(input);
  }
  compare('"' + std::string(1048575, 'a') + '"');
  for (std::size_t n = 0; n <= 8; ++n)
    for (std::size_t d = 0; d <= 3; ++d)
      for (std::size_t nodes = 0; nodes <= 4; ++nodes)
        compare("[true]", {n, d, nodes});
  const std::string seed = "{\"z\":[1,true,\"abcdefghijklmnopqrstuvwxyz\\n\\u0061\"],\"a\":{\"x\":[]}}";
  for (std::size_t i = 0; i < seed.size(); ++i) {
    auto s = seed;
    s.erase(i, 1);
    compare(s);
    s    = seed;
    s[i] = '!';
    compare(s);
    compare(seed.substr(0, i));
  }
}
TEST_CASE("D01-B all actual parser and conversion allocation ordinals fail safely and unwind", "[d01-b]") {
  const std::vector<std::string> inputs = {
      "{\"long-key-abcdefghijklmnopqrstuvwxyz\":[\"abcdefghijklmnopqrstuvwxyz\",{},[1,2,3],true],\"\":{\"nested-key-"
      "abcdefghijklmnopqrstuvwxyz\":\"nested-value-abcdefghijklmnopqrstuvwxyz\"}}",
      "[\"abcdefghijklmnopqrstuvwxyz\",", "{\"abcdefghijklmnopqrstuvwxyz\":true,\"abcdefghijklmnopqrstuvwxyz\":false}",
      "[1e1000,\"abcdefghijklmnopqrstuvwxyz\"]"};
  for (const auto& input : inputs) {
    auto baseline = std::make_unique<state>();
    observation::start();
    auto       normal    = codec_decode_reserved(input, baseline->hook());
    const auto observed  = observation::count;
    observation::enabled = false;
    std::cout << "fixture=" << input << " parser=" << baseline->requests
              << " conversion=" << (normal ? observed - baseline->conversion_start : 0) << " total=" << observed << '\n';
    for (std::size_t i = 1; i <= observed; ++i) {
      auto s = std::make_unique<state>();
      observation::start(i);
      auto result          = codec_decode_reserved(input, s->hook());
      observation::enabled = false;
      REQUIRE_FALSE(result);
      CHECK(result.error().detail.code == "resource_limit");
      CHECK(result.error().detail.limit == "input_bytes");
      observation::enabled = true;
      result               = std::unexpected(codec_error{});
      observation::enabled = false;
      check_cleanup();
    }
    for (std::size_t i = 1; i <= baseline->requests; ++i) {
      auto s  = std::make_unique<state>();
      s->deny = i;
      observation::start();
      auto result          = codec_decode_reserved(input, s->hook());
      observation::enabled = false;
      REQUIRE_FALSE(result);
      CHECK(s->requests == i);
      CHECK(observation::count == i - 1);
      CHECK(result.error().offset == 73);
      CHECK(result.error().detail.message == std::string(4096, 'm'));
      for (std::size_t j = 0; j < observation::count; ++j)
        CHECK(observation::events[j].freed);
    }
  }
}
TEST_CASE("D01-B conversion denial and moved long error require no propagation allocation", "[d01-b]") {
  std::expected<value_ptr, codec_error> retained;
  {
    auto s             = std::make_unique<state>();
    s->deny_conversion = true;
    // A scalar has no parser heap allocation. Denial precedes make_shared.
    observation::start(1);
    retained             = codec_decode_reserved("true", s->hook());
    observation::enabled = false;
    REQUIRE_FALSE(retained);
    CHECK(observation::count == 0);
    CHECK(s->conversions == 1);
  }
  CHECK(retained.error().detail.message == std::string(4096, 'm'));
  CHECK(retained.error().detail.limit == std::string(1024, 'l'));
  CHECK(retained.error().offset == 73);
  value_ptr output;
  {
    auto s = std::make_unique<state>();
    auto result =
        codec_decode_reserved("{\"\":true,\"long-abcdefghijklmnopqrstuvwxyz\":[\"abcdefghijklmnopqrstuvwxyz\"]}", s->hook());
    REQUIRE(result);
    output = std::move(*result);
  }
  CHECK(output->fields.at("")->boolean);
  CHECK(output->fields.at("long-abcdefghijklmnopqrstuvwxyz")->items[0]->string == "abcdefghijklmnopqrstuvwxyz");
}
TEST_CASE("D01-B actual diagnostic allocation refusals cover early guards and callback adapters", "[d01-b]") {
  for (bool reserved : {false, true})
    for (const std::string& input :
         {std::string("null"), std::string("[true]"), std::string("[1,"), std::string("{\"a\":1,\"a\":2}")}) {
      auto decode = [&]() {
        return reserved ? codec_decode_reserved(input, {}, {input == "[true]" ? 5u : 1048576u, 128, 100000})
                        : codec_decode(input, {input == "[true]" ? 5u : 1048576u, 128, 100000});
      };
      observation::start();
      auto       normal    = decode();
      const auto n         = observation::count;
      observation::enabled = false;
      REQUIRE_FALSE(normal);
      for (std::size_t i = 1; i <= n; ++i) {
        observation::start(i);
        auto result          = decode();
        observation::enabled = false;
        REQUIRE_FALSE(result);
        CHECK(result.error().detail.code == "resource_limit");
        CHECK(result.error().detail.limit == "input_bytes");
      }
    }
  for (bool conversion : {false, true})
    for (int type : {1, 2, 3}) {
      auto callback = +[](void* p, std::size_t) -> std::expected<void, codec_error> {
        if (*static_cast<int*>(p) == 1)
          throw std::bad_alloc{};
        if (*static_cast<int*>(p) == 2)
          throw std::length_error("private");
        throw std::runtime_error("private");
      };
      codec_decode_reserver h{&type, callback, nullptr};
      if (conversion) {
        h.allocate = nullptr;
        h.convert  = +[](void* p, const codec_decode_reservation&) -> std::expected<void, codec_error> {
          if (*static_cast<int*>(p) == 1)
            throw std::bad_alloc{};
          if (*static_cast<int*>(p) == 2)
            throw std::length_error("private");
          throw std::runtime_error("private");
        };
      }
      // Exception objects themselves can allocate. Observe them, then fault each
      // actual request including catch-diagnostic construction independently.
      observation::start();
      auto normal          = codec_decode_reserved("[true]", h);
      auto n               = observation::count;
      observation::enabled = false;
      REQUIRE_FALSE(normal);
      CHECK(normal.error().detail.code == (type == 3 ? "internal" : "resource_limit"));
      for (std::size_t i = 1; i <= n; ++i) {
        observation::start(i);
        auto result          = codec_decode_reserved("[true]", h);
        observation::enabled = false;
        REQUIRE_FALSE(result);
        CHECK(result.error().detail.code == "resource_limit");
        CHECK(result.error().detail.limit == "input_bytes");
      }
    }
}
