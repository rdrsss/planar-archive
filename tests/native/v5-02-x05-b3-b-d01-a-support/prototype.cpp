// Test-only request-local Glaze destination and exact-codec comparison.
#include <catch2/catch_test_macros.hpp>
#include <cstdlib>
#include <glaze/glaze.hpp>
#include <new>
#include <unordered_set>
import std;
import planar.engine.json_codec;
// Process-local observer/fault injection only; reservation never reads this state.
// It is disabled during concurrent calls. No resource/context is routed globally.
namespace observation {
struct event {
  std::size_t bytes;
  int         kind;
};
std::array<event, 262144> events{};
std::size_t               requests = 0, fail_at = 0;
bool                      enabled = false, routed = false, adapting = false;
void                      request(std::size_t bytes) {
  if (!enabled)
    return;
  if (requests >= events.size())
    std::abort();
  events[requests++] = {bytes, routed ? 1 : adapting ? 2 : 0};
  if (requests == fail_at)
    throw std::bad_alloc();
}
void start(std::size_t fail = 0) {
  requests = 0;
  fail_at  = fail;
  enabled  = true;
}
struct phase {
  bool& flag;
  bool  previous;
  bool  active;
  explicit phase(bool& f) : flag(f), previous(f), active(enabled) {
    if (active)
      flag = true;
  }
  ~phase() {
    if (active)
      flag = previous;
  }
};
} // namespace observation
void* operator new(std::size_t n) {
  observation::request(n);
  if (auto p = std::malloc(n ? n : 1))
    return p;
  throw std::bad_alloc();
}
void operator delete(void* p) noexcept {
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
  if (posix_memalign(&p, static_cast<std::size_t>(a), n ? n : 1) == 0)
    return p;
  throw std::bad_alloc();
}
void operator delete(void* p, std::align_val_t) noexcept {
  std::free(p);
}
void* operator new[](std::size_t n, std::align_val_t a) {
  return ::operator new(n, a);
}
void operator delete[](void* p, std::align_val_t a) noexcept {
  ::operator delete(p, a);
}
namespace probe {
using namespace planar::engine;
struct refusal {
  std::string_view code, limit;
  std::size_t      maximum  = 0;
  const char*      position = nullptr;
};
struct hook_refusal {
  codec_error error;
};
struct event {
  std::size_t bytes, ordinal;
  void*       address   = nullptr;
  bool        allocated = false, freed = false;
};
// Caller-owned state remains alive until every parser/checker owner is destroyed.
// The callback is synchronous and nonowning; it must return or throw through C++.
struct allocation_state {
  std::array<event, 262144> events{};
  std::size_t               requests = 0, allocations = 0, frees = 0, live_bytes = 0, total_bytes = 0;
  std::size_t               deny = 0, fail_upstream = 0;
  using charge_function                                    = std::expected<void, codec_error> (*)(void*, std::size_t);
  void*                                   callback_context = nullptr;
  charge_function                         callback         = default_charge;
  static std::expected<void, codec_error> default_charge(void* context, std::size_t) {
    auto& state = *static_cast<allocation_state*>(context);
    if (state.requests == state.deny)
      return std::unexpected(std::move(state.supplied));
    return {};
  }
  codec_error supplied{{"resource_limit", "owned first diagnostic with a deliberately long message", "native_conversion", 123456},
                       73};
  std::expected<void, codec_error> charge(std::size_t bytes) {
    if (requests == events.size())
      std::abort(); // Test observer exhaustion is not a codec quota.
    events[requests] = {bytes, requests + 1};
    ++requests;
    return callback(callback_context ? callback_context : this, bytes);
  }
};
template <class T> struct allocator {
  using value_type                             = T;
  using propagate_on_container_move_assignment = std::true_type;
  allocation_state* state                      = nullptr;
  allocator()                                  = default;
  explicit allocator(allocation_state* s) : state(s) {
  }
  template <class U> allocator(const allocator<U>& other) : state(other.state) {
  }
  T* allocate(std::size_t count) {
    if (!state)
      throw std::logic_error("unbound parser allocator");
    if (count > std::numeric_limits<std::size_t>::max() / sizeof(T))
      throw std::bad_array_new_length();
    auto charged = state->charge(count * sizeof(T));
    if (!charged)
      throw hook_refusal{std::move(charged.error())};
    // Controlled upstream failure is after the charge, at the actual allocation adapter.
    if (state->requests == state->fail_upstream)
      throw std::bad_alloc();
    observation::phase phase(observation::routed);
    T*                 p                         = std::allocator<T>{}.allocate(count);
    state->events[state->requests - 1].allocated = true;
    state->events[state->requests - 1].address   = p;
    ++state->allocations;
    state->live_bytes += count * sizeof(T);
    state->total_bytes += count * sizeof(T);
    return p;
  }
  void deallocate(T* p, std::size_t count) noexcept {
    bool found = false;
    for (std::size_t i = state->requests; i > 0; --i) {
      auto& e = state->events[i - 1];
      if (e.address == p && e.allocated && !e.freed) {
        if (e.bytes != count * sizeof(T))
          std::abort();
        e.freed = true;
        found   = true;
        break;
      }
    }
    if (!found)
      std::abort();
    ++state->frees;
    state->live_bytes -= count * sizeof(T);
    std::allocator<T>{}.deallocate(p, count);
  }
  template <class U> bool operator==(const allocator<U>& other) const noexcept {
    return state == other.state;
  }
};
struct text : std::basic_string<char, std::char_traits<char>, allocator<char>> {
  using base = std::basic_string<char, std::char_traits<char>, allocator<char>>;
  using base::base;
  void bind(allocation_state* state) {
    static_cast<base&>(*this) = base(allocator<char>{state});
  }
};
template <class T> struct sequence : std::vector<T, allocator<T>> {
  using base = std::vector<T, allocator<T>>;
  using base::base;
  void bind(allocation_state* state) {
    static_cast<base&>(*this) = base(allocator<T>{state});
  }
};
struct node {
  using array   = sequence<node>;
  using object  = sequence<std::pair<text, node>>;
  using variant = std::variant<std::nullptr_t, bool, std::int64_t, double, text, array, object>;
  variant data;
};
struct context : glz::context {
  std::size_t       nodes            = 0, max_nodes, max_depth;
  const char*       failure_position = nullptr;
  allocation_state* allocations;
};
struct options : glz::opts {
  bool validate_trailing_whitespace = true;
  bool validate_utf8                = true;
  bool escape_control_characters    = true;
};
constexpr options opts{};
} // namespace probe
namespace glz {
template <> struct from<JSON, probe::text> {
  template <auto Opts> static void op(auto& value, auto& ctx, auto& it, auto end) {
    value.bind(ctx.allocations);
    from<JSON, probe::text::base>::template op<Opts>(static_cast<probe::text::base&>(value), ctx, it, end);
  }
};
template <class T> struct from<JSON, probe::sequence<T>> {
  template <auto Opts> static void op(auto& value, auto& ctx, auto& it, auto end) {
    value.bind(ctx.allocations);
    using base = typename probe::sequence<T>::base;
    from<JSON, base>::template op<Opts>(static_cast<base&>(value), ctx, it, end);
  }
};
template <> struct from<JSON, probe::node> {
  template <auto Opts> static void op(auto& value, auto& ctx, auto& it, auto end) {
    using namespace probe;
    if (++ctx.nodes > ctx.max_nodes)
      throw refusal{"resource_limit", "json_nodes", ctx.max_nodes, it};
    if (skip_ws<Opts>(ctx, it, end))
      return;
    if (it == end)
      return;
    if ((*it == '{' || *it == '[') && ctx.depth >= ctx.max_depth)
      throw refusal{"resource_limit", "json_depth", ctx.max_depth, it};
    const char* start = it;
    parse<JSON>::op<Opts>(value.data, ctx, it, end);
    if (bool(ctx.error) && ctx.error != error_code::end_reached) {
      const auto position = std::max(start, it);
      if (!ctx.failure_position || position > ctx.failure_position)
        ctx.failure_position = position;
      return;
    }
    if (std::holds_alternative<std::int64_t>(value.data)) {
      const std::string_view spelling(start, it - start);
      if (spelling.find_first_of(".eE") != spelling.npos)
        throw refusal{"schema", {}, 0, start};
      auto number = std::get<std::int64_t>(value.data);
      if (number < INT32_MIN || number > INT32_MAX)
        throw refusal{"schema", {}, 0, start};
    }
    if (std::holds_alternative<double>(value.data) || std::holds_alternative<std::nullptr_t>(value.data))
      throw refusal{"schema", {}, 0, start};
    if (auto s = std::get_if<probe::text>(&value.data); s && s->find('\0') != s->npos)
      throw refusal{"schema", {}, 0, start};
    if (auto object = std::get_if<node::object>(&value.data)) {
      using keys_type = std::unordered_set<std::string_view, std::hash<std::string_view>, std::equal_to<std::string_view>,
                                           allocator<std::string_view>>;
      keys_type keys(0, std::hash<std::string_view>{}, std::equal_to<std::string_view>{},
                     allocator<std::string_view>{ctx.allocations});
      for (const auto& [key, ignored] : *object)
        if (key.find('\0') != key.npos || !keys.insert(std::string_view(key)).second)
          throw refusal{"schema", {}, 0, start};
    }
  }
};
} // namespace glz
namespace probe {
codec_error error(std::string_view code, std::string_view limit = {}, std::size_t maximum = 0, std::size_t offset = 0) {
  observation::phase phase(observation::adapting);
  return {{std::string(code), "JSON codec refused input", std::string(limit), maximum}, offset};
}
// No parsed owner escapes the synchronous request. Conversion is a D01-B duty.
struct result {
  std::optional<codec_error> error;
  bool                       equal = true;
};
bool equal(const node& source, const value_ptr& expected) {
  return std::visit(
      [&](const auto& v) -> bool {
        using T = std::decay_t<decltype(v)>;
        if constexpr (std::same_as<T, bool>)
          return expected->kind == value_kind::boolean && expected->boolean == v;
        else if constexpr (std::same_as<T, std::int64_t>)
          return expected->kind == value_kind::integer && expected->integer == v;
        else if constexpr (std::same_as<T, text>)
          return expected->kind == value_kind::string && expected->string == std::string_view(v) && valid_text(v);
        else if constexpr (std::same_as<T, node::array>) {
          if (expected->kind != value_kind::list || v.size() != expected->items.size())
            return false;
          for (std::size_t i = 0; i < v.size(); ++i)
            if (!equal(v[i], expected->items[i]))
              return false;
          return true;
        } else if constexpr (std::same_as<T, node::object>) {
          if (expected->kind != value_kind::record || v.size() != expected->fields.size())
            return false;
          for (const auto& [key, child] : v) {
            auto found = std::ranges::find_if(expected->fields, [&](const auto& f) { return f.first == std::string_view(key); });
            if (found == expected->fields.end() || !valid_text(key) || !equal(child, found->second))
              return false;
          }
          return true;
        } else
          return false;
      },
      source.data);
}
result decode(std::string_view input, allocation_state& allocations, codec_limits limits = {1048576, 128, 100000},
              const value_ptr* expected = nullptr) {
  limits.bytes = std::min<std::size_t>(limits.bytes, 1048576);
  limits.nodes = std::min<std::size_t>(limits.nodes, 100000);
  limits.depth = std::min<std::size_t>(limits.depth, 128);
  try {
    try {
      if (input.size() > limits.bytes)
        return {error("resource_limit", "input_bytes", limits.bytes), {}};
      node    parsed;
      context ctx{};
      ctx.max_nodes   = limits.nodes;
      ctx.max_depth   = limits.depth;
      ctx.allocations = &allocations;
      auto status     = glz::read<opts>(parsed, input, ctx);
      if (status.ec == glz::error_code::exceeded_max_recursive_depth)
        return {error("resource_limit", "json_depth", limits.depth, status.count), {}};
      if (status)
        return {error("syntax", {}, 0, ctx.failure_position ? ctx.failure_position - input.data() : status.count), {}};
      auto syntax = glz::validate_json(input);
      if (syntax.ec == glz::error_code::exceeded_max_recursive_depth)
        return {error("resource_limit", "json_depth", limits.depth, syntax.count), {}};
      if (syntax)
        return {error("syntax", {}, 0, syntax.count), {}};
      result r;
      if (expected)
        r.equal = equal(parsed, *expected);
      return r;
    } catch (hook_refusal& r) {
      return {std::move(r.error), {}};
    } catch (const refusal& r) {
      return {error(r.code, r.limit, r.maximum, r.position ? r.position - input.data() : 0), {}};
    } catch (const std::bad_alloc&) {
      return {error("resource_limit", "input_bytes", limits.bytes), {}};
    }
  } catch (const std::bad_alloc&) {
    return {error("resource_limit", "input_bytes", limits.bytes), {}};
  }
}
void compare(std::string_view input, codec_limits limits = {1048576, 128, 100000}) {
  auto state    = std::make_unique<allocation_state>();
  auto baseline = codec_decode(input, limits);
  observation::start();
  auto actual          = decode(input, *state, limits, baseline ? &*baseline : nullptr);
  observation::enabled = false;
  std::size_t routed   = 0;
  for (std::size_t i = 0; i < observation::requests; ++i) {
    CHECK(observation::events[i].kind != 0);
    if (observation::events[i].kind == 1)
      ++routed;
  }
  CHECK(routed == state->allocations);
  if (baseline)
    CHECK(actual.equal);
  REQUIRE(bool(baseline) == !actual.error);
  if (!baseline) {
    CHECK(actual.error->detail.code == baseline.error().detail.code);
    CHECK(actual.error->detail.limit == baseline.error().detail.limit);
    CHECK(actual.error->detail.maximum == baseline.error().detail.maximum);
    CHECK(actual.error->offset == baseline.error().offset);
  }
  CHECK(state->live_bytes == 0);
  CHECK(state->allocations == state->frees);
  for (std::size_t i = 0; i < state->requests; ++i) {
    CHECK(state->events[i].allocated);
    CHECK(state->events[i].freed);
  }
}
} // namespace probe
TEST_CASE("D01-A wrappers preserve exact codec diagnostic ordering", "[d01-a]") {
  using namespace probe;
  for (auto input : {"true",
                     "false",
                     "0",
                     "-0",
                     "-2147483648",
                     "2147483647",
                     "null",
                     "1.0",
                     "1e0",
                     "2147483648",
                     "-2147483649",
                     "{}",
                     "[]",
                     "[[],{},true,1]",
                     "{\"a\":1,\"b\":[true]}",
                     "{\"a\":1,\"a\":2}",
                     "{\"a\":1,\"\\u0061\":2}",
                     "{\"a\":1 \"b\":2}",
                     "{,}",
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
                     "\"\\\"\\\\\\/\\b\\f\\n\\r\\t\\u0061\\uD83D\\uDE00\""}) {
    INFO(input);
    compare(input);
  }
  compare(std::string("\"\xff\"", 3));
  compare(std::string("\"\x01\"", 3));
  compare(std::string("\"a\0b\"", 5));
  for (std::size_t n : {0, 1, 2, 127, 128, 129}) {
    std::string input(n, '[');
    input += std::string(n, ']');
    compare(input);
  }
  compare("[true]", {6, 1, 2});
  compare("[true]", {6, 1, 1});
  compare("[true]", {5, 1, 2});
  compare("{\"a\":1,\"a\":null}");
  compare("{\"a\":1,\"a\":2} garbage");
}
TEST_CASE("D01-A long partial and boundary values retain exact errors", "[d01-a]") {
  using namespace probe;
  for (std::size_t n : {22, 23, 24, 255, 256, 257, 4095, 4096, 65536, 1048574}) {
    auto input = '"' + std::string(n, 'a') + '"';
    compare(input);
    input.pop_back();
    compare(input);
    input += '\\';
    compare(input);
    input += 'u';
    compare(input);
  }
  compare('"' + std::string(1048575, 'a') + '"');
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
  std::string object = "{";
  for (int i = 0; i < 500; ++i) {
    if (i)
      object += ',';
    object += std::format("\"long-key-{}-abcdefghijklmno\":\"long-value-abcdefghijklmno\"", i);
  }
  object += '}';
  compare(object);
  object.pop_back();
  compare(object);
  object += ",\"late\":";
  compare(object);
}
TEST_CASE("D01-A every actual request is preceded by charge and unwinds", "[d01-a]") {
  using namespace probe;
  std::string input =
      "{\"long-object-key-abcdefghijklmnopqrstuvwxyz\":[\"abcdefghijklmnopqrstuvwxyz\",{},[1,2,3],true],\"another-long-object-"
      "key-abcdefghijklmnopqrstuvwxyz\":{\"nested-key-abcdefghijklmnopqrstuvwxyz\":\"nested-value-abcdefghijklmnopqrstuvwxyz\"}}";
  auto observed = std::make_unique<allocation_state>();
  auto first    = decode(input, *observed);
  INFO((first.error ? first.error->detail.code + ":" + first.error->detail.limit + ":" + std::to_string(first.error->offset)
                    : "ok"));
  REQUIRE_FALSE(first.error);
  std::cout << "mixed allocation sizes=[";
  for (std::size_t i = 0; i < observed->requests; ++i) {
    if (i)
      std::cout << ',';
    std::cout << observed->events[i].bytes;
  }
  std::cout << "] total=" << observed->total_bytes << '\n';
  REQUIRE(observed->requests > 10);
  CHECK(observed->requests == observed->allocations);
  CHECK(observed->allocations == observed->frees);
  CHECK(observed->live_bytes == 0);
  for (std::size_t i = 1; i <= observed->requests; ++i) {
    INFO(i);
    auto denied  = std::make_unique<allocation_state>();
    denied->deny = i;
    auto r       = decode(input, *denied);
    REQUIRE(r.error);
    CHECK(r.error->detail.limit == "native_conversion");
    CHECK(r.error->detail.maximum == 123456);
    CHECK(r.error->offset == 73);
    CHECK(r.error->detail.message == "owned first diagnostic with a deliberately long message");
    CHECK(denied->requests == i);
    CHECK(denied->allocations == i - 1);
    CHECK_FALSE(denied->events[i - 1].allocated);
    CHECK(denied->allocations == denied->frees);
    CHECK(denied->live_bytes == 0);
    auto failed           = std::make_unique<allocation_state>();
    failed->fail_upstream = i;
    r                     = decode(input, *failed);
    REQUIRE(r.error);
    CHECK(r.error->detail.limit == "input_bytes");
    CHECK(failed->requests == i);
    CHECK(failed->allocations == i - 1);
    CHECK(failed->allocations == failed->frees);
    CHECK(failed->live_bytes == 0);
  }
}
TEST_CASE("D01-A contexts remain independent across concurrent calls", "[d01-a]") {
  using namespace probe;
  auto work = [](bool deny) {
    for (int i = 0; i < 20; ++i) {
      auto state  = std::make_unique<allocation_state>();
      state->deny = deny ? 2 : 0;
      auto r      = decode("[\"abcdefghijklmnopqrstuvwxyz\",{\"abcdefghijklmnopqrstuvwxyz\":true}]", *state);
      if (bool(r.error) != deny || state->live_bytes || state->allocations != state->frees) {
        std::fprintf(stderr, "concurrent deny=%d err=%s offset=%zu live=%zu allocations=%zu frees=%zu\n", deny,
                     r.error ? r.error->detail.code.c_str() : "ok", r.error ? r.error->offset : 0, state->live_bytes,
                     state->allocations, state->frees);
        return false;
      }
    }
    return true;
  };
  auto a = std::async(std::launch::async, work, false);
  auto b = std::async(std::launch::async, work, true);
  CHECK(a.get());
  CHECK(b.get());
}

TEST_CASE("D01-A actual diagnostic allocations and moved first refusals stay typed", "[d01-a]") {
  using namespace probe;
  static_assert(std::is_nothrow_move_constructible_v<codec_error>);
  for (const std::string& input :
       {std::string("null"), std::string("[1,"), std::string("{\"a\":1,\"a\":2}"), std::string("[true]")}) {
    auto observed = std::make_unique<allocation_state>();
    observation::start();
    auto normal          = decode(input, *observed, {1048576, 1, 1});
    observation::enabled = false;
    REQUIRE(normal.error);
    auto                     requests = observation::requests;
    std::vector<std::size_t> adapter_ordinals;
    for (std::size_t i = 0; i < requests; ++i)
      if (observation::events[i].kind == 2)
        adapter_ordinals.push_back(i + 1);
    std::cout << "diagnostic input=" << input << " actual ordinals=";
    for (auto ordinal : adapter_ordinals)
      std::cout << ordinal << ',';
    std::cout << '\n';
    REQUIRE_FALSE(adapter_ordinals.empty());
    for (auto ordinal : adapter_ordinals) {
      auto state = std::make_unique<allocation_state>();
      observation::start(ordinal);
      auto result          = decode(input, *state, {1048576, 1, 1});
      observation::enabled = false;
      REQUIRE(result.error);
      CHECK(result.error->detail.code == "resource_limit");
      CHECK(result.error->detail.limit == "input_bytes");
      CHECK(result.error->detail.maximum == 1048576);
      CHECK(state->live_bytes == 0);
      CHECK(state->allocations == state->frees);
    }
  }
  auto state  = std::make_unique<allocation_state>();
  state->deny = 1;
  observation::start(1);
  auto r               = decode("[true]", *state);
  observation::enabled = false;
  REQUIRE(r.error);
  CHECK(r.error->detail.limit == "native_conversion");
  CHECK(r.error->offset == 73);
  CHECK(observation::requests == 0); // Moving the original owned diagnostic makes zero allocation requests.
  CHECK(state->allocations == 0);
  CHECK(state->live_bytes == 0);
}
TEST_CASE("D01-A partial values duplicate workspaces and rewind requests all unwind", "[d01-a]") {
  using namespace probe;
  std::vector<std::string> inputs = {"[\"abcdefghijklmnopqrstuvwxyz\",",
                                     "{\"abcdefghijklmnopqrstuvwxyz\":true,\"abcdefghijklmnopqrstuvwxyz\":false}",
                                     "[1e1000,\"abcdefghijklmnopqrstuvwxyz\"]",
                                     "{\"abcdefghijklmnopqrstuvwxyz\":{\"second-abcdefghijklmnopqrstuvwxyz\":[1,2,3]},"
                                     "\"unfinished-abcdefghijklmnopqrstuvwxyz\":\"long-truncated-abcdefghijklmnopqrstuvwxyz"};
  for (const auto& input : inputs) {
    auto observed = std::make_unique<allocation_state>();
    auto baseline = decode(input, *observed);
    REQUIRE(baseline.error);
    REQUIRE(observed->requests > 0);
    for (std::size_t ordinal = 1; ordinal <= observed->requests; ++ordinal) {
      auto state  = std::make_unique<allocation_state>();
      state->deny = ordinal;
      auto r      = decode(input, *state);
      REQUIRE(r.error);
      CHECK(r.error->detail.limit == "native_conversion");
      CHECK(r.error->offset == 73);
      CHECK(state->requests == ordinal);
      CHECK(state->allocations == ordinal - 1);
      CHECK(state->live_bytes == 0);
      CHECK(state->allocations == state->frees);
    }
  }
}

TEST_CASE("D01-A empty short escaped and mutated inputs retain library behavior", "[d01-a]") {
  using namespace probe;
  using original = std::variant<std::nullptr_t, bool, std::int64_t, double, std::string, std::vector<int>,
                                std::vector<std::pair<std::string, int>>>;
  static_assert(glz::variant_is_auto_deducible<node::variant>() == glz::variant_is_auto_deducible<original>());
  static_assert(!glz::variant_is_auto_deducible<node::variant>()); // Both use the same ordered speculative reader.
  static_assert(glz::str_t<text> && glz::array_t<node::array> && glz::json_pair_range_object<node::object>);
  for (std::size_t n = 0; n <= 64; ++n) {
    compare('"' + std::string(n, 'a') + '"');
    compare("[\"" + std::string(n, 'a') + "\\uD83D\\uDE00\",\"\",{},[]]");
    compare("{\"" + std::string(n, 'a') + "\":true}");
  }
  const std::string seed = "{\"z\":[1,true,\"abcdefghijklmnopqrstuvwxyz\\n\\u0061\"],\"a\":{\"x\":[]}}";
  for (std::size_t i = 0; i < seed.size(); ++i) {
    auto input = seed;
    input.erase(i, 1);
    compare(input);
    input    = seed;
    input[i] = '!';
    compare(input);
    input = seed.substr(0, i);
    compare(input);
  }
}
TEST_CASE("D01-A moved callback error outlives the request allocation owner", "[d01-a]") {
  using namespace probe;
  result r;
  {
    auto state                     = std::make_unique<allocation_state>();
    state->deny                    = 1;
    state->supplied.detail.message = std::string(4096, 'm');
    state->supplied.detail.limit   = std::string(1024, 'l');
    observation::start(1);
    r                    = decode("[true]", *state);
    observation::enabled = false;
    REQUIRE(r.error);
    CHECK(observation::requests == 0);
    CHECK(state->live_bytes == 0);
  }
  CHECK(r.error->detail.message == std::string(4096, 'm'));
  CHECK(r.error->detail.limit == std::string(1024, 'l'));
  CHECK(r.error->detail.maximum == 123456);
  CHECK(r.error->offset == 73);
}
