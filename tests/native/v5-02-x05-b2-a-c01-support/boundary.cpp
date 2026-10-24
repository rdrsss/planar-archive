// Independent review controls against the unchanged production native API.
#include <catch2/catch_test_macros.hpp>
#include <cstdlib>
#include <new>
import std;
import planar.engine.runtime;
import planar.engine.hash;
namespace {
thread_local std::size_t fault_countdown = 0, requests = 0;
thread_local bool observe = false;
}
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
void operator delete(void* p) noexcept { std::free(p); }
void* operator new[](std::size_t n) { return ::operator new(n); }
void operator delete[](void* p) noexcept { ::operator delete(p); }
using namespace planar::engine;
namespace {
profile_snapshot captured(std::string source, std::string path = "selected.lua") {
  auto p=std::make_shared<profile_closure>();
  p->profile={"profile","profile.json","0644","{\"version\":1,\"modules\":[\""+path+"\"]}"};
  p->modules.push_back({"modules",std::move(path),"0644",std::move(source)});
  return p;
}
std::string source(std::string id="custom") {
 return "local D={id='"+id+"',abi='shape_v1',revision='revision',stages='optional',help='shape'}; registration.shape(D,function() error('DO NOT EXECUTE') end)";
}
}

void resource(const std::expected<shape_definition,registration_error>& result) {
 REQUIRE_FALSE(result); CHECK(result.error().detail.code=="resource_limit");
 CHECK(result.error().detail.message=="External profile resource limit exceeded");
 CHECK(result.error().field=="catalog.shapes");
}
TEST_CASE("C01 ordinary protocol refusals remain recoverable and redacted", "[c01]") {
 auto vm=command_vm::create(captured(source()));REQUIRE(vm);
 for(auto id:{"unknown","pipeline"}) {auto bad=vm->resolve_shape_definition(id);REQUIRE_FALSE(bad);CHECK(bad.error().detail.code=="internal");CHECK(bad.error().detail.message=="Invalid external profile protocol");CHECK(bad.error().field=="catalog.shapes");REQUIRE(vm->resolve_shape_definition("custom"));}
 auto moved=std::move(*vm);auto bad=vm->resolve_shape_definition("custom");REQUIRE_FALSE(bad);CHECK(bad.error().detail.code=="internal");REQUIRE(moved.resolve_shape_definition("custom"));
 auto embedded=command_vm::create();REQUIRE(embedded);auto normal=embedded->resolve_shape_definition("pipeline");REQUIRE_FALSE(normal);CHECK(normal.error().detail.code=="internal");
}
TEST_CASE("C01 early allocation failure keeps owned state terminal and moved-to state live", "[c01]") {
 for(bool moved_from:{false,true}) {
 auto vm=moved_from ? command_vm::create(captured(source())) : std::expected<command_vm,registration_error>{std::move(*command_vm::create())};REQUIRE(vm);
 std::optional<command_vm> moved;if(moved_from)moved.emplace(std::move(*vm));
 requests=0;fault_countdown=1;observe=true;bool escaped=false;std::optional<std::expected<shape_definition,registration_error>> result;
 try{result.emplace(vm->resolve_shape_definition("custom"));}catch(...){escaped=true;}
 observe=false;fault_countdown=0;CHECK(requests>0);CHECK_FALSE(escaped);REQUIRE(result);resource(*result);
 auto again=vm->resolve_shape_definition("custom");REQUIRE_FALSE(again);CHECK(again.error().detail.code=="internal");
 if(moved)REQUIRE(moved->resolve_shape_definition("custom"));
 }
}
TEST_CASE("C01 quota-first diagnostic survives second actual allocation refusal", "[c01]") {
 auto vm=command_vm::create(captured(source()+"\n--"+std::string(524288,'x')));REQUIRE(vm);
 std::optional<registration_error> first;
 for(std::size_t i=0;i<200;++i){auto next=vm->resolve_shape_definition("custom");if(!next){first=std::move(next.error());break;}}
 REQUIRE(first);CHECK(first->detail.code=="resource_limit");CHECK(first->detail.limit=="native_bytes");CHECK(first->field=="catalog.shapes");
 requests=0;fault_countdown=1;observe=true;bool escaped=false;std::optional<std::expected<shape_definition,registration_error>> result;
 try{result.emplace(vm->resolve_shape_definition("custom"));}catch(...){escaped=true;}
 observe=false;fault_countdown=0;CHECK(requests>0);CHECK_FALSE(escaped);REQUIRE(result);REQUIRE_FALSE(*result);
 CHECK(result->error().detail.code==first->detail.code);CHECK(result->error().detail.message==first->detail.message);CHECK(result->error().detail.limit==first->detail.limit);CHECK(result->error().detail.maximum==first->detail.maximum);CHECK(result->error().field==first->field);
 auto retry=vm->resolve_shape_definition("custom");REQUIRE_FALSE(retry);CHECK(retry.error().detail.message==first->detail.message);CHECK(retry.error().detail.limit==first->detail.limit);CHECK(retry.error().detail.maximum==first->detail.maximum);
}
