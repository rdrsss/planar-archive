// Private copies only: deterministic allocation, GC, resource and mutation controls.
#include <cstdlib>
#include <new>
static bool d3_fail_next=false;
void* operator new(std::size_t n) {
 if(std::exchange(d3_fail_next,false))throw std::bad_alloc();
 if(auto* p=std::malloc(n?n:1))return p;
 throw std::bad_alloc();
}
void operator delete(void* p) noexcept {std::free(p);}
void* operator new[](std::size_t n){return ::operator new(n);}
void operator delete[](void* p) noexcept {::operator delete(p);}
extern "C" { bool d3_fail_lua=false; bool d3_one_shot=false; int d3_lua_refusals=0; }
static std::function<void(int)> d3_phase;
extern "C" void d3_stage(int phase) { if(d3_phase)d3_phase(phase); }
struct d3_private_cleanup {
 ~d3_private_cleanup(){d3_phase={};d3_fail_next=false;d3_fail_lua=false;d3_one_shot=false;}
};
TEST_CASE("D3 private nested phases preserve aliases stack counters original basis and GC cleanup", "[d3-private]") {
 d3_private_cleanup cleanup;
 extension_fixture f;d3_inputs(f,true);
 d3_handler(f,"local role=ctx.read_role({}); "+d3_return(),semantic_shape("local t={};for i=1,2000 do t[i]={i} end;"));
 auto owner=f.owner();REQUIRE(owner.admit("inspect",d3_options(f),f.cwd));
 auto created=owner.private_probe();CHECK(created[19]==9);
 std::array<std::size_t,23> before{},after{};bool started=false,finished=false;
 d3_phase=[&](int phase){if(phase==1){before=owner.private_probe();started=true;}if(phase==4){after=owner.private_probe();finished=true;}};
 auto result=owner.execute();REQUIRE(result);CHECK(started);CHECK(finished);
 CHECK(after[4]>before[4]);CHECK(after[5]>before[5]);CHECK(after[6]>before[6]);
 for(auto index:{8,9,10,11,12,13,14,15})CHECK(after[index]==before[index]);
 CHECK(after[20]!=0);CHECK(after[21]!=0);CHECK(after[22]==1);
 auto ended=owner.private_probe();for(auto index:{8,9,10,13,14,15,17})CHECK(ended[index]==0);
 CHECK(ended[4]>=after[4]);CHECK(ended[6]>=after[6]);
 std::ofstream out("D3-LAYOUT.json");out<<"["<<created[0]<<","<<created[1]<<","<<created[2]<<","<<created[3]<<"]\n";
}
TEST_CASE("D3 private post discovery and post semantic mutation suppress authentic role release", "[d3-private]") {
 for(int target:{2,4}) {
  d3_private_cleanup cleanup;extension_fixture f;d3_inputs(f);
  d3_handler(f,"local role=ctx.read_role({}); "+d3_return());
  auto owner=f.owner();REQUIRE(owner.admit("inspect",d3_options(f),f.cwd));bool mutated=false;
  d3_phase=[&](int phase){if(phase==target){std::ofstream(f.path/"workspaces/demo/text/shared.md",std::ios::app)<<"changed after original read";mutated=true;}};
  auto result=owner.execute();CHECK(mutated);REQUIRE_FALSE(result);
  auto error=std::get_if<registration_error>(&result.error());REQUIRE(error);CHECK(error->detail.code=="capture_changed");
  auto ended=owner.private_probe();for(auto index:{8,9,10,13,14,15,17})CHECK(ended[index]==0);
 }
}
TEST_CASE("D3 private native allocation node native byte instruction and completed Lua refusal stay terminal", "[d3-private]") {
 for(int fault:{0,1,2,3,4}) {
  d3_private_cleanup cleanup;extension_fixture f;d3_inputs(f,true);
  d3_handler(f,"pcall(function() ctx.read_role({}) end); return {ok=false,error={code='schema',message='caught later',field=''}}",semantic_shape());
  auto owner=f.owner();REQUIRE(owner.admit("inspect",d3_options(f),f.cwd));bool armed=false;
  d3_phase=[&](int phase){if(phase==3){armed=true;if(fault<3)owner.private_limit(fault);if(fault==3)d3_fail_next=true;if(fault==4)d3_fail_lua=true;}};
  auto result=owner.execute();d3_fail_lua=false;CHECK(armed);REQUIRE_FALSE(result);
  auto error=std::get_if<registration_error>(&result.error());REQUIRE(error);CHECK(error->detail.code=="resource_limit");
  auto ended=owner.private_probe();for(auto index:{8,9,10,13,14,15,17})CHECK(ended[index]==0);
  if(fault==4){CHECK(ended[18]==1);CHECK(d3_lua_refusals>0);}
 }
}
TEST_CASE("D3 private semantic first error survives later native allocation failure", "[d3-private]") {
 d3_private_cleanup cleanup;extension_fixture f;d3_inputs(f,true);
 d3_handler(f,"pcall(function() ctx.read_role({}) end); error('later')",callable_shape("return {ok=false,error={code='ownership',message='first',field='job.roles'}}"));
 auto owner=f.owner();REQUIRE(owner.admit("inspect",d3_options(f),f.cwd));bool armed=false;
 d3_phase=[&](int phase){if(phase==4){armed=true;d3_fail_next=true;}};
 auto result=owner.execute();CHECK(armed);REQUIRE_FALSE(result);auto error=std::get_if<assembly_error>(&result.error());REQUIRE(error);
 CHECK(error->error->fields.at("message")->string=="first");
}
TEST_CASE("D3 private selected custom shape allocation failure retains selected catalog location", "[d3-private]") {
 d3_private_cleanup cleanup;extension_fixture f;d3_inputs(f,true);
 d3_handler(f,"pcall(function() ctx.read_role({}) end); error('later')",semantic_shape());
 auto owner=f.owner();REQUIRE(owner.admit("inspect",d3_options(f),f.cwd));bool armed=false;
 d3_phase=[&](int phase){if(phase==5){armed=true;d3_fail_next=true;}};
 auto result=owner.execute();CHECK(armed);REQUIRE_FALSE(result);auto error=std::get_if<registration_error>(&result.error());REQUIRE(error);
 CHECK(error->detail.code=="resource_limit");CHECK(error->field=="catalog.shapes[1]");
}
