// Production codec allocation phase observation; no runtime or allocator-policy integration.
#include <catch2/catch_test_macros.hpp>
#include <cstdlib>
#include <new>
import std;
import planar.engine.json_codec;
namespace {
thread_local std::size_t requested_bytes = 0, maximum_request = 0;
thread_local bool        observe  = false;
thread_local std::size_t requests = 0, before_hook = 0, after_hook = 0;
thread_local bool        passed_hook = false;
} // namespace
void* operator new(std::size_t n) {
  if (observe) {
    ++requests;
    requested_bytes += n;
    maximum_request = std::max(maximum_request, n);
    if (passed_hook)
      ++after_hook;
    else
      ++before_hook;
  }
  if (auto p = std::malloc(n ? n : 1))
    return p;
  throw std::bad_alloc{};
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
using namespace planar::engine;
namespace {
struct charge {
  codec_reservation                      measured{};
  bool                                   refuse = false;
  std::size_t                            calls  = 0;
  diagnostic                             first{"resource_limit", "retained diagnostic", "native_bytes", 536870912};
  static std::expected<void, diagnostic> reserve(void* context, const codec_reservation& measured) {
    auto& self = *static_cast<charge*>(context);
    ++self.calls;
    self.measured = measured;
    passed_hook   = true;
    if (self.refuse)
      return std::unexpected(std::move(self.first));
    return {};
  }
};
void start() {
  requests = before_hook = after_hook = requested_bytes = maximum_request = 0;
  passed_hook                                                             = false;
  observe                                                                 = true;
}
} // namespace
TEST_CASE("B2B successful validation allocates only after the charge hook", "[b2b]") {
  auto scalar  = std::make_shared<value>();
  scalar->kind = value_kind::string;
  scalar->string.assign(4096, 'x');
  auto inner  = std::make_shared<value>();
  inner->kind = value_kind::record;
  inner->fields.emplace(std::string(128, 'k'), scalar);
  auto root   = std::make_shared<value>();
  root->kind  = value_kind::list;
  root->items = {inner, inner};
  charge c;
  start();
  auto result = codec_encode_reserved(root, {&c, charge::reserve});
  observe     = false;
  REQUIRE(result);
  CHECK(c.calls == 1);
  CHECK(before_hook == 0);
  CHECK(after_hook > 0);
  CHECK(requested_bytes <= c.measured.total_bytes);
  CHECK(maximum_request <= c.measured.output_storage_bytes);
  const auto admitted_requests = requests;
  charge     refusal;
  refusal.refuse = true;
  start();
  auto denied = codec_encode_reserved(root, {&refusal, charge::reserve});
  observe     = false;
  REQUIRE_FALSE(denied);
  CHECK(refusal.calls == 1);
  CHECK(before_hook == 0);
  CHECK(after_hook == 0);
  CHECK(requests == 0);
  CHECK(denied.error().detail.message == "retained diagnostic");
  std::cout << "admitted allocation requests=" << admitted_requests << "; refused=" << requests << '\n';
}
TEST_CASE("B2B integer sizing and maximum valid path require no prehook allocations", "[b2b]") {
  auto number     = std::make_shared<value>();
  number->kind    = value_kind::integer;
  number->integer = -2147483648;
  value_ptr root  = number;
  for (int i = 0; i < 256; ++i) {
    auto outer  = std::make_shared<value>();
    outer->kind = value_kind::list;
    outer->items.push_back(root);
    root = outer;
  }
  charge c;
  start();
  auto result = codec_encode_reserved(root, {&c, charge::reserve});
  observe     = false;
  REQUIRE(result);
  CHECK(c.calls == 1);
  CHECK(before_hook == 0);
  CHECK(after_hook > 0);
}
