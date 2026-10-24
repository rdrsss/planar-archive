// Full production resolver allocation requests and first-terminal controlled refusals.
#include "../../../src/engine/runtime/external_registration.t.cpp"
#include <cstdlib>
#include <new>
namespace {
thread_local std::size_t fault = 0, requests = 0, allocated = 0;
thread_local bool        observe  = false;
std::size_t              reserved = 0, visited = 0, actions = 0;
} // namespace
void* operator new(std::size_t n) {
  if (observe) {
    ++requests;
    allocated += n;
    if (fault && --fault == 0)
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
extern "C" void c2_plan(std::size_t bytes, std::size_t nodes) {
  reserved = bytes;
  visited  = nodes;
  ++actions;
}
namespace {
void same_error(const registration_error& a, const registration_error& b) {
  CHECK(a.detail.code == b.detail.code);
  CHECK(a.detail.message == b.detail.message);
  CHECK(a.detail.limit == b.detail.limit);
  CHECK(a.detail.maximum == b.detail.maximum);
  CHECK(a.field == b.field);
}
void sticky(command_vm& vm, const registration_error& first) {
  auto p = vm.resolve_projection_definition("missing");
  REQUIRE_FALSE(p);
  same_error(p.error(), first);
  auto s = vm.resolve_shape_definition("custom");
  REQUIRE_FALSE(s);
  same_error(s.error(), first);
  auto f = vm.invoke_shape("custom", {});
  REQUIRE_FALSE(f);
  same_error(f.error(), first);
  for (auto call : {&command_vm::validate_profile, &command_vm::select_profile, &command_vm::render_profile}) {
    auto result = (vm.*call)({});
    REQUIRE_FALSE(result);
    same_error(result.error(), first);
  }
  auto moved = std::move(vm);
  vm         = std::move(moved);
  auto p2    = vm.resolve_projection_definition("custom");
  REQUIRE_FALSE(p2);
  same_error(p2.error(), first);
}
} // namespace
TEST_CASE("C2 full construction hash return allocations fit independent prospective charge", "[c2-allocation]") {
  for (auto size : {0, 10, 22, 23, 255, 256, 257, 4096, 16384, 65536, 1048000}) {
    auto        source = projection_source("\n--" + std::string(size, 'x'));
    std::string name = "custom", revision = "final", help = "projection";
    if (size == 16384) {
      name     = std::string(64, 'p');
      revision = std::string(64, 'r');
      help     = std::string(4096, 'h');
      source += "\nD.id='" + name + "';D.revision='" + revision + "';D.help='" + help + "'";
    }
    std::string path = "m1.lua";
    if (size == 1048000)
      for (int i = 0; i < 10; ++i)
        path = std::string(80, 'p') + "/" + path;
    auto vm = command_vm::create(named_profile({{path, source}}));
    REQUIRE(vm);
    requests = allocated = actions = reserved = visited = 0;
    observe                                             = true;
    auto result                                         = vm->resolve_projection_definition(name);
    observe                                             = false;
    REQUIRE(result);
    REQUIRE(actions == 1);
    CHECK(visited == 25);
    const auto module = 9 + path.size(), hex = 2 * source.size();
    // Independent flat schema arithmetic: 5 key/value string pairs, 3 wrapper
    // strings, 2 maps; existing 512 slots and conservative duplicated D scratch.
    const std::size_t descriptor = 9 + (9 + 3 + 9 + 13) + (9 + 4 + 9 + help.size()) + (9 + 2 + 9 + name.size()) +
                                   (9 + 5 + 9 + 15) + (9 + 8 + 9 + revision.size());
    const auto        wrapper    = 18 + (5 * 9 + 37) + (3 * 9 + 8 + module + hex);
    const auto        copy       = [](std::size_t n) { return 2 * n + 16; };
    const auto expected = 25 * 512 + 5 * 128 + 11 * 512 + 4 * descriptor + 4 * 512 + copy(6) + copy(12) + copy(24) + 4 * 78 + 64 +
                          48 + 4 * wrapper + (2 * 37 + 5 * 16) + 2 * copy(hex) + 2 * copy(module) + copy(8) + copy(name.size()) +
                          copy(revision.size());
    CHECK(reserved == expected);
    CHECK(allocated <= reserved);
    std::println("source={} path={} requests={} allocated={} reserved={} visits={}", source.size(), path.size(), requests,
                 allocated, reserved, visited);
  }
}
TEST_CASE("C2 every actual one shot allocation failure preserves first error", "[c2-allocation]") {
  auto make = [] {
    return command_vm::create(profile({projection_source(";D.id='" + std::string(64, 'p') + "';D.revision='" +
                                                         std::string(64, 'r') + "';D.help='" + std::string(4096, 'h') + "'"),
                                       shape_source()}));
  };
  auto vm = make();
  REQUIRE(vm);
  auto selected_id = std::string(64, 'p');
  requests = allocated = 0;
  observe              = true;
  auto result          = vm->resolve_projection_definition(selected_id);
  observe              = false;
  REQUIRE(result);
  auto count = requests;
  REQUIRE(count > 20);
  std::println("sampling all {} resolver allocation requests", count);
  for (std::size_t n = 1; n <= count; ++n) {
    auto sample = make();
    REQUIRE(sample);
    requests     = 0;
    fault        = n;
    observe      = true;
    auto refusal = sample->resolve_projection_definition(selected_id);
    observe      = false;
    fault        = 0;
    INFO(n);
    REQUIRE_FALSE(refusal);
    CHECK(refusal.error().detail.code == "resource_limit");
    CHECK(refusal.error().field == "catalog.projections");
    sticky(*sample, refusal.error());
  }
}
TEST_CASE("C2 reservation refusal precedes all prospective allocations and preserves owner", "[c2-refusal]") {
  auto vm = command_vm::create(profile({projection_source(), shape_source()}));
  REQUIRE(vm);
  actions     = 0;
  auto result = vm->resolve_projection_definition("custom");
  REQUIRE_FALSE(result);
  CHECK(actions == 0);
  CHECK(result.error().detail.code == "resource_limit");
  CHECK(result.error().field == "catalog.projections");
  sticky(*vm, result.error());
}
