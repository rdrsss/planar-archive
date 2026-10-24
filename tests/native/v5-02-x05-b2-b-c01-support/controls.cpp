// Independent actual-allocation and string-capacity boundary checks against the production codec.
#include <catch2/catch_test_macros.hpp>
#include <cstdlib>
#include <new>
import std;
import planar.engine.json_codec;
namespace {
thread_local bool observe = false, passed_hook = false;
thread_local std::size_t requests = 0, requested_bytes = 0, before_hook = 0, fail_at = 0;
thread_local std::array<std::size_t, 1024> sizes{};
}
void* operator new(std::size_t n) {
  if (observe) {
    ++requests;
    requested_bytes += n;
    if (requests <= sizes.size()) sizes[requests - 1] = n;
    if (!passed_hook) ++before_hook;
    if (requests == fail_at) throw std::bad_alloc{};
  }
  if (auto p = std::malloc(n ? n : 1)) return p;
  throw std::bad_alloc{};
}
void operator delete(void* p) noexcept { std::free(p); }
void* operator new[](std::size_t n) { return ::operator new(n); }
void operator delete[](void* p) noexcept { ::operator delete(p); }
namespace {
using namespace planar::engine;
struct charge {
  codec_reservation measured{};
  std::size_t calls = 0;
  bool refuse = false;
  std::exception_ptr exception;
  diagnostic first{"resource_limit", std::string(256, 'm'), std::string(128, 'l'), 536870912};
  static std::expected<void, diagnostic> reserve(void* context, const codec_reservation& measured) {
    auto& c = *static_cast<charge*>(context);
    c.measured = measured;
    ++c.calls;
    passed_hook = true;
    if (c.exception) std::rethrow_exception(c.exception);
    if (c.refuse) return std::unexpected(std::move(c.first));
    return {};
  }
  codec_reserver hook() { return {this, reserve}; }
};
void start(std::size_t failure = 0) {
  requests = requested_bytes = before_hook = 0;
  sizes.fill(0);
  fail_at = failure;
  passed_hook = false;
  observe = true;
}
std::shared_ptr<value> node(value_kind kind) {
  auto p = std::make_shared<value>(); p->kind = kind; return p;
}
value_ptr mixed() {
  auto text = node(value_kind::string); text->string.assign(4096, 'x');
  auto object = node(value_kind::record); object->fields.emplace(std::string(128, 'k'), text);
  auto root = node(value_kind::list); root->items = {object, object}; return root;
}
}

TEST_CASE("C01 normal encode diagnostics preserve validation precedence and callback adaptation", "[b2b-c01]") {
  for (bool reserved : {false, true}) {
    charge c;
    auto normal = reserved ? codec_encode_reserved(nullptr, c.hook()) : codec_encode(nullptr);
    REQUIRE_FALSE(normal);
    CHECK(normal.error().detail.code == "internal");
    CHECK(normal.error().detail.message == "JSON codec refused input");
    CHECK(normal.error().detail.limit.empty());
    CHECK(normal.error().detail.maximum == 0);
    auto zero = reserved ? codec_encode_reserved(nullptr, c.hook(), {0,0,0}) : codec_encode(nullptr, {0,0,0});
    REQUIRE_FALSE(zero);
    CHECK(zero.error().detail.code == "resource_limit");
    CHECK(zero.error().detail.limit == "json_nodes");
    CHECK(zero.error().detail.maximum == 0);
    CHECK(c.calls == 0);
  }
  auto input = mixed();
  std::array exceptions{std::make_exception_ptr(std::bad_alloc{}), std::make_exception_ptr(std::length_error("private")), std::make_exception_ptr(std::runtime_error("private"))};
  for (std::size_t i = 0; i < exceptions.size(); ++i) {
    charge c; c.exception = exceptions[i];
    auto result = codec_encode_reserved(input, c.hook(), {10000,256,4000000});
    REQUIRE_FALSE(result);
    CHECK(c.calls == 1);
    CHECK(result.error().detail.code == (i == 2 ? "internal" : "resource_limit"));
    CHECK(result.error().detail.message == "JSON codec refused input");
    CHECK(result.error().detail.limit == (i == 2 ? "" : "output_bytes"));
    CHECK(result.error().detail.maximum == (i == 2 ? 0 : 10000));
    CHECK(result.error().offset == 0);
  }
}
TEST_CASE("C01 all callback diagnostic allocation failures return exact typed output refusal", "[b2b-c01]") {
  auto input = mixed();
  std::array exceptions{std::make_exception_ptr(std::bad_alloc{}), std::make_exception_ptr(std::length_error("private")), std::make_exception_ptr(std::runtime_error("private"))};
  for (const auto& exception : exceptions) {
    charge c; c.exception = exception;
    std::optional<std::expected<std::string, codec_error>> result;
    bool escaped = false;
    start(1);
    try { result.emplace(codec_encode_reserved(input, c.hook(), {10000,256,4000000})); }
    catch (...) { escaped = true; }
    observe = false;
    CHECK_FALSE(escaped);
    CHECK(c.calls == 1);
    CHECK(before_hook == 0);
    REQUIRE(result);
    REQUIRE_FALSE(*result);
    CHECK((*result).error().detail.code == "resource_limit");
    CHECK((*result).error().detail.message == "JSON codec refused input");
    CHECK((*result).error().detail.limit == "output_bytes");
    CHECK((*result).error().detail.maximum == 10000);
    CHECK((*result).error().offset == 0);
  }
}
