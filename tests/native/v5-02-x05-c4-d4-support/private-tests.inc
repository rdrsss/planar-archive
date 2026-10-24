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
 auto created=owner.private_probe();CHECK(created[19]==10);
 std::array<std::size_t,23> before{},after{};bool started=false,finished=false;
 d3_phase=[&](int phase){if(phase==1){before=owner.private_probe();started=true;}if(phase==4){after=owner.private_probe();finished=true;}};
 auto result=owner.execute();REQUIRE(result);CHECK(started);CHECK(finished);
 CHECK(after[4]>before[4]);CHECK(after[5]>before[5]);CHECK(after[6]>before[6]);
 for(auto index:{8,9,10,11,12,13,14,15})CHECK(after[index]==before[index]);
 CHECK(after[20]!=0);CHECK(after[21]!=0);CHECK(after[22]==1);
 auto ended=owner.private_probe();for(auto index:{8,9,10,13,14,15,17})CHECK(ended[index]==0);
 CHECK(ended[4]>=after[4]);CHECK(ended[6]>=after[6]);
 std::ofstream out("D4-LAYOUT.json");out<<"["<<created[0]<<","<<created[1]<<","<<created[2]<<","<<created[3]<<"]\n";
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
  d3_phase=[&](int phase){if(phase==3){armed=true;if(fault<3)owner.private_limit(fault==1 && phase==10 ? 3 : fault);if(fault==3)d3_fail_next=true;if(fault==4)d3_fail_lua=true;}};
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
TEST_CASE("D4 private projection base assembly frames preserve outer aliases cumulative counters and original basis", "[d3-private]") {
 d3_private_cleanup cleanup;extension_fixture f;d3_inputs(f,true);
 d4_handler(f,"local options=inv.options;local read=ctx.read_role;local role=read({});local p=ctx.project({id='custom'});assert(options==inv.options and read==ctx.read_role and role.context.job=='split'); "+std::string(d4_return),projection_section(),semantic_shape());
 auto owner=f.owner();REQUIRE(owner.admit("inspect",d3_options(f),f.cwd));
 std::array<std::array<std::size_t,23>,12> probes{};
 d3_phase=[&](int phase){probes[phase]=owner.private_probe();};REQUIRE(owner.execute());
 for(auto pair: {std::pair{6,7},std::pair{8,9},std::pair{10,11}}) {
  const auto& a=probes[pair.first];const auto& b=probes[pair.second];
  CHECK(b[4]>a[4]);CHECK(b[5]>a[5]);CHECK(b[6]>=a[6]);
  for(auto index:{8,9,10,11,12,13,14,15,20,21,22})CHECK(a[index]==b[index]);
  CHECK(a[20]!=0);CHECK(a[21]!=0);CHECK(a[13]!=0);
 }
 auto ended=owner.private_probe();for(auto index:{8,9,10,13,14,15,17})CHECK(ended[index]==0);
}
TEST_CASE("D4 private mutation after role or projection suppresses both handler result branches", "[d3-private]") {
 for(int phase:{4,7,9,11})for(bool semantic:{false,true}) {
  d3_private_cleanup cleanup;extension_fixture f;d3_inputs(f);
  d4_handler(f,"ctx.read_role({});local p=ctx.project({id='custom'}); "+std::string(semantic?c01_semantic:d4_return));
  auto owner=f.owner();REQUIRE(owner.admit("inspect",d3_options(f),f.cwd));bool armed=false;
  d3_phase=[&](int point){if(point==phase){armed=true;std::ofstream(f.path/"workspaces/demo/text/shared.md",std::ios::app)<<"late mutation";}};
  auto result=owner.execute();CHECK(armed);REQUIRE_FALSE(result);auto error=std::get_if<registration_error>(&result.error());REQUIRE(error);CHECK(error->detail.code=="capture_changed");
 }
}
TEST_CASE("D4 private selected F base helper assembly resource failures preserve their own attribution", "[d3-private]") {
 for(int phase:{6,8,10})for(int fault:{0,1,2,3,4}) {
  d3_private_cleanup cleanup;extension_fixture f;d3_inputs(f);
  d4_handler(f,"ctx.read_role({});pcall(function()ctx.project({id='custom'})end);error('later handler')", "local n=0;for i=1,5000 do n=n+i end;"+projection_section());
  auto owner=f.owner();REQUIRE(owner.admit("inspect",d3_options(f),f.cwd));bool armed=false;
  d3_phase=[&](int point){if(point==phase){armed=true;if(fault<3)owner.private_limit(fault==1 && phase==10 ? 3 : fault);if(fault==3)d3_fail_next=true;if(fault==4)d3_fail_lua=true;}};
  auto result=owner.execute();d3_fail_lua=false;CHECK(armed);REQUIRE_FALSE(result);auto error=std::get_if<registration_error>(&result.error());REQUIRE(error);
  CHECK(error->detail.code=="resource_limit");CHECK(error->field==(phase==6?"catalog.projections[1]":"catalog.commands[6]"));
  auto ended=owner.private_probe();for(auto index:{8,9,10,13,14,15,17})CHECK(ended[index]==0);
 }
}
TEST_CASE("D4 private lazy pair performs no role IO or unused projection", "[d3-private]") {
 for(bool read:{false,true}) {
  d3_private_cleanup cleanup;extension_fixture f;d3_inputs(f);
  d4_handler(f,read?"local role=ctx.read_role({});"+d3_return():std::string(c01_semantic),"error('unused projection F')");
  d3_watch watched(f.path/"workspaces/demo/workspace.lua");auto owner=f.owner();REQUIRE(owner.admit("inspect",d3_options(f),f.cwd));
  REQUIRE(owner.execute());CHECK((planar_admission_read_attempts()>0)==read);
 }
}
TEST_CASE("D4 private assembly semantic error survives later allocation refusal", "[d3-private]") {
 d3_private_cleanup cleanup;extension_fixture f;d3_inputs(f);
 d4_handler(f,"ctx.read_role({});pcall(function()ctx.project({id='custom'})end);error('later')");
 auto options=d3_options(f);options.push_back({"budget-bytes",true,"1"});auto owner=f.owner();REQUIRE(owner.admit("inspect",options,f.cwd));bool armed=false;
 d3_phase=[&](int phase){if(phase==11){armed=true;d3_fail_next=true;}};
 auto result=owner.execute();CHECK(armed);REQUIRE_FALSE(result);auto error=std::get_if<assembly_error>(&result.error());REQUIRE(error);
 CHECK(error->error->fields.at("code")->string=="budget_exceeded");CHECK(error->error->fields.at("field")->string=="render.budget_bytes");
}
