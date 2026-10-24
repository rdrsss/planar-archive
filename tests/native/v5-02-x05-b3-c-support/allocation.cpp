// Actual one-shot native allocation refusal samples across the production semantic operation.
#include "../../../src/engine/runtime/external_registration.t.cpp"
#include <cstdlib>
#include <new>
namespace {
thread_local std::size_t fault_countdown = 0, requests = 0;
thread_local bool        observe = false;
} // namespace
void* operator new(std::size_t n) {
  if (observe) {
    ++requests;
    if (fault_countdown && --fault_countdown == 0)
      throw std::bad_alloc{};
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

TEST_CASE("R01 actual native allocation samples preserve typed first terminal error", "[r01-allocation]") {
  semantic_fixture f;
  auto             input    = selection_request(semantic_validation(f.bundle));
  auto             make     = [] { return command_vm::create(profile({semantic_shape()})); };
  auto             baseline = make();
  REQUIRE(baseline);
  requests    = 0;
  observe     = true;
  auto result = baseline->select_profile(input);
  observe     = false;
  REQUIRE(result);
  REQUIRE((*result)->fields.contains("value"));
  auto count = requests;
  REQUIRE(count > 100);
  std::set<std::size_t> samples;
  for (std::size_t n = 1; n <= 32; ++n)
    samples.insert(n);
  for (std::size_t n = 1; n <= 40; ++n)
    samples.insert(1 + (count - 1) * n / 40);
  for (std::size_t n = count - 15; n <= count; ++n)
    samples.insert(n);
  for (std::size_t n = 400; n <= 450; ++n)
    samples.insert(n);
  std::println("{} allocation requests; {} actual one-shot sampled refusals", count, samples.size());
  for (auto fault : samples) {
    auto vm = make();
    REQUIRE(vm);
    fault_countdown = fault;
    requests        = 0;
    observe         = true;
    auto refusal    = vm->select_profile(input);
    observe         = false;
    fault_countdown = 0;
    INFO(fault);
    REQUIRE_FALSE(refusal);
    CHECK(refusal.error().detail.code == "resource_limit");
    auto retry = vm->select_profile({});
    REQUIRE_FALSE(retry);
    CHECK(retry.error().detail.code == refusal.error().detail.code);
    CHECK(retry.error().detail.message == refusal.error().detail.message);
    CHECK(retry.error().detail.limit == refusal.error().detail.limit);
    CHECK(retry.error().detail.maximum == refusal.error().detail.maximum);
    CHECK(retry.error().field == refusal.error().field);
  }
}
