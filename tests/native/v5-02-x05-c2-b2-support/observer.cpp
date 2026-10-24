// Private observer of the actual production projection bridge, with one-shot native failures.
#include "../../../src/engine/runtime/external_registration.t.cpp"
#include <cstdlib>
#include <new>
namespace {
std::array<std::array<std::size_t, 3>, 65536> before_identities{};
std::size_t                                   before_count = 0, bookkeeping_count = 0;
std::array<std::array<std::size_t, 3>, 65536> identities{};
std::size_t                                   identity_count = 0;
thread_local bool                             observe        = false;
thread_local std::size_t                      requests = 0, fault = 0;
std::size_t                                   calls = 0, scans = 0, counts = 0, copies = 0, nodes_at_scan = 0, nodes_at_copy = 0;
std::size_t                                   snapshot = 0, snapshot_count = 0, scan_charge = 0, copy_charge = 0;
} // namespace
void* operator new(std::size_t n) {
  if (observe) {
    ++requests;
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
extern "C" void c2_event(const char* name, std::size_t a, std::size_t b) {
  std::string_view stage(name);
  if (stage == "snapshot_begin") {
    identity_count    = 0;
    bookkeeping_count = 0;
  }
  if (stage == "bookkeeping")
    ++bookkeeping_count;
  if (stage == "before") {
    before_identities = identities;
    before_count      = identity_count;
  }
  if (stage == "registry" || stage == "tags" || stage == "backings" || stage == "sealed") {
    if (identity_count >= identities.size())
      std::abort();
    identities[identity_count++] = {stage == "registry" ? 0U : stage == "tags" ? 1U : stage == "backings" ? 2U : 3U, a, b};
  }
  if (stage == "call")
    ++calls;
  if (stage == "scan") {
    ++scans;
    nodes_at_scan = a;
    scan_charge   = b;
  }
  if (stage == "reserved")
    ++counts;
  if (stage == "copy") {
    ++copies;
    nodes_at_copy = a;
    copy_charge   = b;
  }
  if (stage == "snapshot") {
    snapshot       = a;
    snapshot_count = b;
  }
}
namespace {
void reset() {
  calls = scans = counts = copies = nodes_at_scan = nodes_at_copy = scan_charge = copy_charge = 0;
}
void sticky_projection(command_vm& vm, const registration_error& first) {
  auto same = [&](const registration_error& error) {
    CHECK(error.detail.code == first.detail.code);
    CHECK(error.detail.message == first.detail.message);
    CHECK(error.detail.limit == first.detail.limit);
    CHECK(error.detail.maximum == first.detail.maximum);
    CHECK(error.field == first.field);
  };
  auto retry = vm.invoke_projection("absent", projection_input_kind::history_snapshot_v1, {});
  REQUIRE_FALSE(retry);
  same(retry.error());
  auto identity = vm.resolve_projection_definition("absent");
  REQUIRE_FALSE(identity);
  same(identity.error());
  auto shape = vm.invoke_shape("absent", {});
  REQUIRE_FALSE(shape);
  same(shape.error());
  for (auto op : {&command_vm::render_profile, &command_vm::select_profile, &command_vm::validate_profile}) {
    auto r = (vm.*op)({});
    REQUIRE_FALSE(r);
    same(r.error());
  }
  auto moved = std::move(vm);
  vm         = std::move(moved);
  auto final = vm.resolve_shape_definition("absent");
  REQUIRE_FALSE(final);
  same(final.error());
}
} // namespace
TEST_CASE("C2 B2 actual protected scanner counts before AST and keeps borrowed roots through forced GC", "[c2-b2-observer]") {
  auto vm = command_vm::create(profile(
      {projection_callable("local text=input.payload..':borrowed';local id=string.sub(input.payload,1,1)..'section';return "
                           "{version=1,sections=engine.array{{id=id,text=text,mandatory=false}}}")}));
  REQUIRE(vm);
  reset();
  auto r = vm->invoke_projection("custom", projection_input_kind::role_context_v1, projection_input());
  REQUIRE(r);
  CHECK(calls == 1);
  CHECK(scans == 1);
  CHECK(counts == 1);
  CHECK(copies == 1);
  CHECK(nodes_at_scan == nodes_at_copy);
  CHECK(copy_charge > scan_charge);
  std::sort(identities.begin(), identities.begin() + identity_count);
  auto exact       = identities;
  auto exact_count = identity_count;
  auto first       = snapshot;
  auto n           = snapshot_count;
  reset();
  auto again = vm->invoke_projection("custom", projection_input_kind::role_context_v1, projection_input());
  REQUIRE(again);
  CHECK(calls == 1);
  CHECK(scans == 1);
  CHECK(counts == 1);
  CHECK(copies == 1);
  std::sort(identities.begin(), identities.begin() + identity_count);
  CHECK(identity_count == exact_count);
  CHECK(identities == exact);
  CHECK(snapshot == first);
  CHECK(snapshot_count == n);
  std::println("no AST visits before count; rooted GC stable; retained snapshot entries={}", n);
}
TEST_CASE("C2 B2 actual malformed result never reaches count or native AST", "[c2-b2-observer]") {
  auto vm = command_vm::create(profile({projection_callable("return {version=1,sections={}} ")}));
  REQUIRE(vm);
  reset();
  auto r = vm->invoke_projection("custom", projection_input_kind::role_context_v1, projection_input());
  REQUIRE_FALSE(r);
  CHECK(calls == 1);
  CHECK(scans == 1);
  CHECK(counts == 0);
  CHECK(copies == 0);
  sticky_projection(*vm, r.error());
}
TEST_CASE("C2 B2 every one shot native projection allocation failure retains first diagnostic", "[c2-b2-alloc]") {
  auto make = [] {
    std::string source;
    for (int i = 1; i < 10; ++i)
      source += projection_callable(projection_section(), "role_context_v1", "before" + std::to_string(i)) + ";";
    source += projection_callable(
        projection_section("id='long_section_identifier',text='long text beyond short string optimization',mandatory=true"));
    return command_vm::create(profile({source}));
  };
  auto vm = make();
  REQUIRE(vm);
  auto input = projection_input();
  requests   = 0;
  observe    = true;
  auto r     = vm->invoke_projection("custom", projection_input_kind::role_context_v1, input);
  observe    = false;
  REQUIRE(r);
  auto count = requests;
  REQUIRE(count > 10);
  std::println("sampled {} actual native requests", count);
  for (std::size_t i = 1; i <= count; ++i) {
    auto owner = make();
    REQUIRE(owner);
    requests    = 0;
    fault       = i;
    observe     = true;
    auto result = owner->invoke_projection("custom", projection_input_kind::role_context_v1, input);
    observe     = false;
    fault       = 0;
    REQUIRE_FALSE(result);
    CHECK(result.error().detail.code == "resource_limit");
    CHECK(result.error().field == "catalog.projections[10]");
    sticky_projection(*owner, result.error());
  }
}
TEST_CASE("C2 B2 controlled real cumulative registration quotas refuse and stay terminal", "[c2-b2-quota]") {
  auto vm = command_vm::create(profile({projection_callable(projection_section())}));
  REQUIRE(vm);
  auto r = vm->invoke_projection("custom", projection_input_kind::role_context_v1, projection_input());
  REQUIRE_FALSE(r);
  CHECK(r.error().detail.code == "resource_limit");
  sticky_projection(*vm, r.error());
  if (std::string_view(std::getenv("C2B2_MODE")) == "partial") {
    std::sort(identities.begin(), identities.begin() + identity_count);
    std::sort(before_identities.begin(), before_identities.begin() + before_count);
    CHECK(identity_count == before_count);
    CHECK(identities == before_identities);
    CHECK(bookkeeping_count == 1);
  }
  std::println("controlled remaining budget path exercised production refusal: {} {}", r.error().detail.message, r.error().field);
}

TEST_CASE("C2 B2 private unknown metatable refuses before count and copy", "[c2-b2-metatable]") {
  auto vm = command_vm::create(profile({projection_callable(projection_section())}));
  REQUIRE(vm);
  reset();
  auto r = vm->invoke_projection("custom", projection_input_kind::role_context_v1, projection_input());
  REQUIRE_FALSE(r);
  CHECK(r.error().detail.code == "internal");
  CHECK(counts == 0);
  CHECK(copies == 0);
  sticky_projection(*vm, r.error());
}
