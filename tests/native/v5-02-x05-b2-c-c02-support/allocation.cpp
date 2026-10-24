// Independent review controls against the unchanged production native API.
#include <catch2/catch_test_macros.hpp>
#include <cstdlib>
#include <new>
import std;
import planar.engine.runtime;
import planar.engine.hash;
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
using namespace planar::engine;
namespace {
profile_snapshot captured(std::string source, std::string path = "selected.lua") {
  auto p     = std::make_shared<profile_closure>();
  p->profile = {"profile", "profile.json", "0644", "{\"version\":1,\"modules\":[\"" + path + "\"]}"};
  p->modules.push_back({"modules", std::move(path), "0644", std::move(source)});
  return p;
}
std::string source(std::string id = "custom") {
  return "local D={id='" + id +
         "',abi='shape_v1',revision='revision',stages='optional',help='shape'}; registration.shape(D,function(input) return "
         "engine.object{alias=input.job,scratch=engine.array{1,2}} end)";
}
} // namespace

value_ptr input() {
  auto n         = std::make_shared<value>();
  n->kind        = value_kind::integer;
  n->integer     = 1;
  auto empty     = std::make_shared<value>();
  empty->kind    = value_kind::record;
  auto result    = std::make_shared<value>();
  result->kind   = value_kind::record;
  result->fields = {{"version", n}, {"job", empty}, {"spec", empty}, {"assignments", empty}, {"reviewer", empty}};
  return result;
}
TEST_CASE("B2 C every actual native allocation boundary returns typed failure and terminal retry", "[b2c-allocation]") {
  auto argument = input();
  auto baseline = command_vm::create(captured(source()));
  REQUIRE(baseline);
  requests        = 0;
  fault_countdown = 0;
  observe         = true;
  auto success    = baseline->invoke_shape("custom", argument);
  observe         = false;
  REQUIRE(success);
  auto count = requests;
  REQUIRE(count > 50);
  std::println("observed native allocations tested individually: {}", count);
  for (std::size_t fault = 1; fault <= count; ++fault) {
    auto vm = command_vm::create(captured(source()));
    REQUIRE(vm);
    requests                                                                 = 0;
    fault_countdown                                                          = fault;
    observe                                                                  = true;
    bool                                                             escaped = false;
    std::optional<std::expected<shape_response, registration_error>> result;
    try {
      result.emplace(vm->invoke_shape("custom", argument));
    } catch (...) {
      escaped = true;
    }
    observe         = false;
    fault_countdown = 0;
    INFO(fault);
    CHECK_FALSE(escaped);
    REQUIRE(result);
    REQUIRE_FALSE(*result);
    CHECK(result->error().detail.code == "resource_limit");
    auto retry = vm->invoke_shape("custom", argument);
    REQUIRE_FALSE(retry);
    CHECK(retry.error().detail.code == "resource_limit");
  }
}
TEST_CASE("B2 C embedded and moved diagnostic allocations stay typed", "[b2c-allocation]") {
  for (int mode : {0, 1}) {
    std::optional<command_vm> vm;
    if (mode == 0) {
      auto owner = command_vm::create();
      REQUIRE(owner);
      vm.emplace(std::move(*owner));
    } else {
      auto owner = command_vm::create(captured(source()));
      REQUIRE(owner);
      vm.emplace(std::move(*owner));
    }
    std::optional<command_vm> moved;
    if (mode == 1)
      moved.emplace(std::move(*vm));
    auto argument                                                            = input();
    requests                                                                 = 0;
    fault_countdown                                                          = 1;
    observe                                                                  = true;
    bool                                                             escaped = false;
    std::optional<std::expected<shape_response, registration_error>> result;
    try {
      result.emplace(vm->invoke_shape("custom", argument));
    } catch (...) {
      escaped = true;
    }
    observe         = false;
    fault_countdown = 0;
    CHECK_FALSE(escaped);
    REQUIRE(result);
    REQUIRE_FALSE(*result);
    CHECK(result->error().detail.code == "resource_limit");
  }
}

TEST_CASE("C02 terminal lookup ownership and later copies preserve first diagnostic under one shot refusal", "[c02-allocation]") {
  auto make = [] { return command_vm::create(captured(source())); };
  auto count_vm = make(); REQUIRE(count_vm);
  std::size_t successful = 0;
  std::optional<registration_error> first;
  for (; successful < 100000; ++successful) {
    requests=0;observe=true;
    auto d = count_vm->resolve_shape_definition("custom");
    observe=false;
    if (!d) { first = std::move(d.error()); break; }
  }
  auto boundary_count=requests;
  REQUIRE(first); REQUIRE(successful > 0); REQUIRE(boundary_count>0);
  auto same = [&](const registration_error& e) {
    CHECK(e.detail.code == first->detail.code); CHECK(e.detail.message == first->detail.message);
    CHECK(e.detail.limit == first->detail.limit); CHECK(e.detail.maximum == first->detail.maximum); CHECK(e.field == first->field);
  };
  for (int operation : {0,1,2,3}) {
    requests=0; fault_countdown=0; observe=true;
    if (operation==0) { auto r=count_vm->resolve_shape_definition("missing"); observe=false; REQUIRE_FALSE(r); }
    else { auto r=count_vm->invoke_shape(operation==1 ? "missing" : "custom", {}); observe=false; REQUIRE_FALSE(r); }
    auto count=requests; REQUIRE(count>0);
    for(std::size_t fault=1;fault<=count;++fault) {
      requests=0;fault_countdown=fault;observe=true;
      if(operation==0) { auto r=count_vm->resolve_shape_definition("missing");observe=false;REQUIRE_FALSE(r);same(r.error()); }
      else { auto r=count_vm->invoke_shape(operation==1 ? "missing" : "custom", {});observe=false;REQUIRE_FALSE(r);same(r.error()); }
      fault_countdown=0;
    }
  }
  // Repeat the real last lookup, denying each actual allocation request once.
  // A diagnostic allocation before ownership may yield the generic resource error;
  // after ownership the original native-limit diagnostic must survive adaptation.
  for(std::size_t fault=1;fault<=boundary_count;++fault) {
    auto vm=make();REQUIRE(vm);
    for(std::size_t i=0;i<successful;++i) REQUIRE(vm->resolve_shape_definition("custom"));
    requests=0;fault_countdown=fault;observe=true;
    auto r=vm->resolve_shape_definition("custom"); observe=false; CHECK(fault_countdown==0); fault_countdown=0;
    REQUIRE_FALSE(r);CHECK(r.error().detail.code=="resource_limit");
    auto retry=vm->invoke_shape("missing",{}); REQUIRE_FALSE(retry);
    CHECK(retry.error().detail.code==r.error().detail.code);CHECK(retry.error().detail.message==r.error().detail.message);
    CHECK(retry.error().detail.limit==r.error().detail.limit);CHECK(retry.error().detail.maximum==r.error().detail.maximum);CHECK(retry.error().field==r.error().field);
    std::println("C02 first-boundary fault={} actual_requests={} message={} limit={} field={}",fault,requests,r.error().detail.message,r.error().detail.limit,r.error().field);
  }
}
