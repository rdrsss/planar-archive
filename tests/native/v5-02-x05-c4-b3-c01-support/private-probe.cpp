// Private actual-owner probes: counters, receipts, allocation refusal and cleanup.
#include <catch2/catch_test_macros.hpp>
#include <cstdlib>
#include <fcntl.h>
#include <new>
#include <sqlite3.h>
#include <unistd.h>
import std;
import planar.engine.runtime;
using namespace planar::engine;
using namespace planar::engine::ledger;
static bool fail_next = false;
[[maybe_unused]] static std::size_t failed_size = 0;
void* operator new(std::size_t n) {
  if (std::exchange(fail_next, false)) { failed_size=n; throw std::bad_alloc(); }
  if (auto* p=std::malloc(n?n:1)) return p;
  throw std::bad_alloc();
}
void operator delete(void* p) noexcept { std::free(p); }
void* operator new[](std::size_t n) { return ::operator new(n); }
void operator delete[](void* p) noexcept { ::operator delete(p); }
extern "C" { bool c4_fail_lua = false; int c4_failed_lua = 0; std::size_t c4_lua_allocations = 0; }
static std::function<void(int)> stage;
extern "C" void c4_stage(int n) { if (stage) stage(n); }
namespace {
#include "observer.inc"
constexpr auto success="return {ok=true,value={version=1,kind='extension',command=inv.command,data={type='history',value=h}}}";
[[maybe_unused]] constexpr auto semantic="return {ok=false,error={code='schema',message='intentional',field='input'}}";
struct fixture {
 std::filesystem::path root;
 fixture(std::string body="local h=ctx.read_history({}); "+std::string(success)) {
  char pattern[]="/tmp/planar-c4b3-probe-XXXXXX";REQUIRE(::mkdtemp(pattern));root=std::filesystem::canonical(pattern);
  std::filesystem::create_directory(root/"trusted");
  std::ofstream(root/"trusted/profile.json")<<R"({"version":1,"modules":["module.lua"]})";
  std::ofstream(root/"trusted/module.lua")<<R"(
registration.add({name='inspect',family='extension_read_v1',handler_id='custom.inspect',help='inspect',response_kind='extension_v1',capabilities=engine.array{'history_read_v1'},options=engine.array{
{name='--ledger',type='text',required=true,help='ledger'},
{name='--revision',type='history_id',required=false,help='revision'},
{name='--at-event',type='history_id',required=false,help='event'},
{name='--after-event',type='history_id',required=false,help='cursor'},
{name='--request-id',type='text',required=false,help='request'},
{name='--limit',type='decimal',required=false,help='limit'}
}},function(inv,ctx) )"<<body<<" end)";
  REQUIRE(initialize_new(root/"ledger","demo"));
 }
 ~fixture(){stage={};fail_next=false;c4_fail_lua=false;std::filesystem::remove_all(root);}
 extension_invocation owner(std::vector<extension_parsed_option> options={}){
  auto capture=profile_capture::open((root/"trusted/profile.json").string());REQUIRE(capture);
  auto result=extension_invocation::create(std::move(*capture));REQUIRE(result);
  auto path=(root/"ledger").string();options.insert(options.begin(),{"ledger",true,path});REQUIRE(result->admit("inspect",options,{}));return std::move(*result);
 }
 void mutate(bool ledger=false){std::ofstream(root/(ledger?"ledger":"trusted/module.lua"),std::ios::app)<<"changed";}
};
void failure(const auto& result,std::string_view code){REQUIRE_FALSE(result);REQUIRE(std::holds_alternative<registration_error>(result.error()));CHECK(std::get<registration_error>(result.error()).detail.code==code);}
}
TEST_CASE("C4B3 baseline regression and actual lazy read success","[c4b3-red]") {
 fixture f;auto owner=f.owner();observed_vfs observer;
 auto layout=owner.private_probe(); std::ofstream("BASE_LAYOUT.json")<<"{\"command\":"<<layout[0]<<",\"vm\":"<<layout[1]<<",\"invocation\":"<<layout[2]<<",\"bridge\":"<<layout[3]<<",\"native_bytes\":"<<layout[4]<<"}\n";
 auto result=owner.execute();REQUIRE(result);CHECK(observer.reads>0);CHECK(observer.opens==observer.closes);
}

#ifndef C4_BASELINE
TEST_CASE("C4B3 actual no-read semantic path and closed transaction before Lua result", "[c4b3-private]") {
 fixture f(semantic);auto owner=f.owner();observed_vfs observer;
 REQUIRE(owner.execute());CHECK(observer.reads==0);CHECK(observer.opens==0);
 fixture yes;auto active=yes.owner();bool returned=false;std::size_t before=0;
 stage=[&](int n){if(n==2)before=c4_lua_allocations; if(n==3)CHECK(c4_lua_allocations==before); if(n==4){returned=true;CHECK(observer.opens==observer.closes);auto writer=connection::open(yes.root/"ledger",open_mode::read_write,0);REQUIRE(writer);auto tx=writer->begin();REQUIRE(tx);REQUIRE(tx->commit());}};
 REQUIRE(active.execute());stage={};CHECK(returned);CHECK(observer.reads>0);CHECK(observer.opens==observer.closes);
}
TEST_CASE("C4B3 before-service during-read and after-handler original mutation suppress facts", "[c4b3-private]") {
 for(int when:{1,2,3,5,7})for(bool ledger:{false,true}) {
  fixture f;auto owner=f.owner();observed_vfs observer;bool hit=false;
  stage=[&](int n){if(n==when&&!hit&&(when!=7||observer.reads>0)){hit=true;f.mutate(ledger);}};
  auto result=owner.execute();stage={};REQUIRE(hit);failure(result,"capture_changed");
  CHECK(observer.opens==observer.closes);if(when<=2)CHECK(observer.reads==0);else CHECK(observer.reads>0);
  failure(owner.execute(),"capture_changed");
 }
}
TEST_CASE("C4B3 real service allocation failure poisons caught handler and clears bridge", "[c4b3-private]") {
 for(int when:{2,3}) {
  fixture f("pcall(function() ctx.read_history({}) end); "+std::string(semantic));auto owner=f.owner();observed_vfs observer;
  failed_size=0;stage=[&](int n){if(n==when){stage={};fail_next=true;}};
  auto result=owner.execute();stage={};fail_next=false;REQUIRE(failed_size>0);failure(result,"resource_limit");CHECK(observer.opens==observer.closes);
  auto layout=owner.private_probe();CHECK(layout[5]==0);CHECK(layout[6]==0);CHECK(layout[7]==0);CHECK(layout[8]==0);failure(owner.execute(),"resource_limit");
 }
}
TEST_CASE("C4B3 actual Lua longjmp during service result push preserves cleanup", "[c4b3-private]") {
 fixture f("pcall(function() ctx.read_history({}) end); "+std::string(semantic));auto owner=f.owner();observed_vfs observer;
 c4_failed_lua=0;stage=[&](int n){if(n==4){c4_fail_lua=true;CHECK(observer.opens==observer.closes);}};
 auto result=owner.execute();stage={};c4_fail_lua=false;REQUIRE(c4_failed_lua>0);failure(result,"resource_limit");
 CHECK(observer.opens==observer.closes);auto layout=owner.private_probe();CHECK(layout[5]==0);CHECK(layout[6]==0);CHECK(layout[7]==0);CHECK(layout[8]==0);
 failure(owner.execute(),"resource_limit");
}
TEST_CASE("C4B3 original typed format failure survives actual diagnostic copy allocation failure", "[c4b3-private]") {
 fixture f("pcall(function() ctx.read_history({}) end); error('later')");
 std::ofstream(f.root/"ledger",std::ios::trunc)<<"bad";auto owner=f.owner();
 stage=[&](int n){if(n==6){fail_next=true;stage={};}};failed_size=0;
 auto result=owner.execute();stage={};fail_next=false;REQUIRE(failed_size>0);REQUIRE_FALSE(result);
 REQUIRE(std::holds_alternative<format_error>(result.error()));CHECK(std::get<format_error>(result.error()).code==format_error_code::foreign_ledger);
 CHECK(std::get<format_error>(result.error()).message=="Empty or truncated ledger header");
 auto retry=owner.execute();REQUIRE_FALSE(retry);CHECK(std::get<format_error>(retry.error()).message==std::get<format_error>(result.error()).message);
}
TEST_CASE("C4B3 pinned native layout and existing initial cumulative charge", "[c4b3-private]") {
 fixture f;auto owner=f.owner();auto layout=owner.private_probe();
 CHECK(layout[0]>layout[3]);CHECK(layout[1]>layout[0]);CHECK(layout[4]>=layout[3]+8);
 std::ofstream("LAYOUT.json")<<"{\"command\":"<<layout[0]<<",\"vm\":"<<layout[1]<<",\"invocation\":"<<layout[2]<<",\"bridge\":"<<layout[3]<<",\"native_bytes\":"<<layout[4]<<"}\n";
 REQUIRE(owner.execute());
}

#endif

#ifndef C4_BASELINE
TEST_CASE("Reviewer missing-capability rawget call stays poisoned despite pcall", "[c4b3-reviewer]") {
 for (auto method : {"read_role", "project"}) {
  fixture f("pcall(function() rawget(ctx,'"+std::string(method)+"')({}) end); "+std::string(semantic));
  auto owner=f.owner();observed_vfs observer;
  auto result=owner.execute();CHECK_FALSE(result);
  if(!result){REQUIRE(std::holds_alternative<registration_error>(result.error()));CHECK(std::get<registration_error>(result.error()).detail.code=="internal");}
  CHECK(observer.opens==0);CHECK(observer.reads==0);
 }
}
TEST_CASE("Reviewer raw correspondence distinguishes deep exact copies nested scalar and container changes", "[c4b3-reviewer]") {
 const std::string prefix=R"(local original=ctx.read_history({});
 local function copy(v)
  if type(v)~='table' then return v end
  local r={}; for k,x in pairs(v) do r[k]=copy(x) end
  if engine.kind(v)=='array' then return engine.array(r) end
  return r
 end
 local h=copy(original); )";
 for(auto mutation:{"", "h.data.events[1].version=1.0; ", "h.data.events[1].payload.extra='forged'; ",
                    "h.data.events[1].sequence='2'; ", "h.data.events.extra=true; ",
                    "h.data.events=engine.object(h.data.events); ", "h.data.events[1]=h; "}) {
  fixture f(prefix+mutation+success);auto owner=f.owner();observed_vfs observer;
  auto result=owner.execute();
  if(std::string_view(mutation).empty())REQUIRE(result);else failure(result,"internal");
  CHECK(observer.reads>0);CHECK(observer.opens==observer.closes);
 }
}
TEST_CASE("Reviewer caught duplicate service does not perform a second native read", "[c4b3-reviewer]") {
 fixture f("local h=ctx.read_history({}); pcall(function() ctx.read_history({}) end); "+std::string(semantic));
 auto owner=f.owner();observed_vfs observer;int reads_after_first=-1;
 stage=[&](int n){if(n==4)reads_after_first=observer.reads;};
 auto result=owner.execute();stage={};failure(result,"internal");
 REQUIRE(reads_after_first>0);CHECK(observer.reads==reads_after_first);CHECK(observer.opens==observer.closes);
 auto layout=owner.private_probe();CHECK(layout[5]==0);CHECK(layout[6]==0);CHECK(layout[7]==0);CHECK(layout[8]==0);
}
TEST_CASE("Reviewer semantic error final seal detects original receipt mutation with and without a read", "[c4b3-reviewer]") {
 for(bool read:{false,true}) for(bool ledger:{false,true}){
  fixture f((read?std::string("local h=ctx.read_history({}); "):std::string{})+semantic);auto owner=f.owner();observed_vfs observer;bool hit=false;
  stage=[&](int n){if(n==5){hit=true;f.mutate(ledger);}};
  auto result=owner.execute();stage={};REQUIRE(hit);failure(result,"capture_changed");
  if(read)CHECK(observer.reads>0);else {CHECK(observer.reads==0);CHECK(observer.opens==0);}
  CHECK(observer.opens==observer.closes);
 }
}
#endif

#ifndef C4_BASELINE
TEST_CASE("Reviewer missing-capability rawget call poisons pure summary", "[c4b3-reviewer]") {
 for(auto method:{"read_history","read_role","project"}) {
  fixture f("pcall(function() rawget(ctx,'"+std::string(method)+"')({}) end); return {ok=true,value={version=1,kind='extension',command=inv.command,data={type='summary',text='should refuse'}}}");
  auto path=f.root/"trusted/module.lua";std::ifstream input(path);std::string source(std::istreambuf_iterator<char>{input},{});
  auto cap=source.find("engine.array{'history_read_v1'}");REQUIRE(cap!=std::string::npos);source.replace(cap,std::string("engine.array{'history_read_v1'}").size(),"engine.array{}");
  auto opts=source.find("options=engine.array{");auto tail=source.find("}},function(inv,ctx)",opts);REQUIRE(tail!=std::string::npos);
  source.replace(opts,tail+2-opts,"options=engine.array{}} ");std::ofstream(path)<<source;
  auto capture=profile_capture::open((f.root/"trusted/profile.json").string());REQUIRE(capture);
  auto owner=extension_invocation::create(std::move(*capture));REQUIRE(owner);REQUIRE(owner->admit("inspect",{},{}));
  observed_vfs observer;auto result=owner->execute();CHECK_FALSE(result);
  if(!result){REQUIRE(std::holds_alternative<registration_error>(result.error()));CHECK(std::get<registration_error>(result.error()).detail.code=="internal");}
  CHECK(observer.opens==0);CHECK(observer.reads==0);
 }
}
#endif

TEST_CASE("C01 actual context installation allocation refusal clears exact identity", "[c4b3-private]") {
 fixture f(semantic); auto owner=f.owner(); observed_vfs observer; bool hit=false;
 stage=[&](int n){if(n==8){hit=true;c4_fail_lua=true;}};
 auto result=owner.execute();stage={};c4_fail_lua=false;REQUIRE(hit);REQUIRE(c4_failed_lua>0);failure(result,"resource_limit");
 auto layout=owner.private_probe();CHECK(layout[5]==0);CHECK(layout[6]==0);CHECK(layout[7]==0);CHECK(layout[8]==0);
 CHECK(observer.opens==0);CHECK(observer.reads==0);failure(owner.execute(),"resource_limit");
}
