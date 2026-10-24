// Reversible native instrumentation, real original captures, real delegate counters and actual allocation refusal.
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
extern "C" int c4_header_reads;
static bool fail_next = false;
static std::size_t failed_size = 0;
void* operator new(std::size_t n) {
  if (std::exchange(fail_next, false)) { failed_size = n; throw std::bad_alloc(); }
  if (auto* p = std::malloc(n ? n : 1)) return p;
  throw std::bad_alloc();
}
void operator delete(void* p) noexcept { std::free(p); }
void* operator new[](std::size_t n) { return ::operator new(n); }
void operator delete[](void* p) noexcept { ::operator delete(p); }
static std::function<void(int)> stage;
extern "C" void c4_stage(int n) { if (stage) stage(n); }
namespace {
#include "observer.inc"
struct fixture {
  std::filesystem::path root;
  fixture(bool v2 = false) {
    char pattern[] = "/tmp/planar-c4b2-probe-XXXXXX";
    REQUIRE(::mkdtemp(pattern));root=std::filesystem::canonical(pattern);
    std::filesystem::create_directory(root/"trusted");
    std::ofstream(root/"trusted/profile.json") << R"({"version":1,"modules":["module.lua"]})";
    std::ofstream(root/"trusted/module.lua") << R"(
registration.add({name='inspect',family='extension_read_v1',handler_id='custom.inspect',help='inspect',response_kind='extension_v1',capabilities=engine.array{'history_read_v1'},options=engine.array{
{name='--ledger',type='text',required=true,help='ledger'},
{name='--revision',type='history_id',required=false,help='revision'},
{name='--at-event',type='history_id',required=false,help='event'},
{name='--after-event',type='history_id',required=false,help='cursor'},
{name='--request-id',type='text',required=false,help='request'},
{name='--limit',type='decimal',required=false,help='limit'}
}},function() error('Lua handler must never execute') end)
)";
    REQUIRE((v2?initialize_new_v2(root/"ledger","demo"):initialize_new(root/"ledger","demo")));
  }
  ~fixture() { stage={};fail_next=false;std::filesystem::remove_all(root); }
  extension_invocation owner(std::vector<extension_parsed_option> options={}) {
    auto capture=profile_capture::open((root/"trusted/profile.json").string());REQUIRE(capture);
    auto owner=extension_invocation::create(std::move(*capture));REQUIRE(owner);
    auto ledger_path=(root/"ledger").string();
    options.insert(options.begin(),{"ledger",true,ledger_path});
    auto admitted=owner->admit("inspect",options,{});if(!admitted)INFO(admitted.error().detail.code<<" "<<admitted.error().detail.message<<" "<<admitted.error().field);REQUIRE(admitted);return std::move(*owner);
  }
  void mutate(bool ledger=false) { std::ofstream(root/(ledger?"ledger":"trusted/module.lua"),std::ios::app)<<"changed"; }
};
struct guard final : format_read_guard {
  bool refuse=false;int checks=0,fail_at=-1,fd=-1;
  read_gate_status bind(int descriptor) override { fd=descriptor;return refuse?read_gate_status::refused:read_gate_status::ready; }
  read_gate_status verify() override {++checks;if(fd>=0)CHECK(::fcntl(fd,F_GETFD)>=0);return checks==fail_at?read_gate_status::refused:read_gate_status::ready;}
};
void registration_failure(const std::expected<void,std::variant<registration_error,format_error,history::history_read_error>>& result,std::string_view code) {
 REQUIRE_FALSE(result);REQUIRE(std::holds_alternative<registration_error>(result.error()));
 CHECK(std::get<registration_error>(result.error()).detail.code==code);
}
}
TEST_CASE("C4B2 pre-header binding sensitivity and positive actual reads", "[c4b2-header]") {
  fixture f;observed_vfs observer;
  auto g=std::make_shared<guard>();g->refuse=true;c4_header_reads=0;
  auto refused=open_existing_gated(f.root/"ledger",g);REQUIRE_FALSE(refused);
  CHECK(c4_header_reads==0);CHECK(observer.reads==0);
  g=std::make_shared<guard>();c4_header_reads=0;
  auto good=open_existing_gated(f.root/"ledger",g);REQUIRE(good);
  CHECK(c4_header_reads>0);CHECK(observer.reads>0);CHECK(g->checks>0);
}
TEST_CASE("C4B2 cached native success inspects sticky gate before authorization", "[c4b2-native]") {
 fixture f;observed_vfs observer;auto g=std::make_shared<guard>();
 auto file=open_existing_gated(f.root/"ledger",g);REQUIRE(file);REQUIRE(file->read_head());
 auto before=observer.reads;g->fail_at=g->checks+2;
 auto result=file->read_head();REQUIRE_FALSE(result);
 CHECK(observer.reads==before);CHECK(file->gate_status()==read_gate_status::refused);
 auto calls=g->checks;CHECK_FALSE(file->read_events_after(0,50));CHECK(g->checks==calls);
}
TEST_CASE("C4B2 actual original owner refuses before raw and first SQLite header reads", "[c4b2-native]") {
 for (int when : {1,2}) {
  fixture f;auto owner=f.owner();observed_vfs observer;int verifies=0;
  c4_header_reads=0;
  stage=[&](int n){if((when==1&&n==1)||(when==2&&n==3&&++verifies==2))f.mutate();};
  auto result=owner.prepare_history();stage={};registration_failure(result,"capture_changed");
  CHECK(c4_header_reads==(when==1?0:1));CHECK(observer.reads==0);
  auto retry=owner.prepare_history();registration_failure(retry,"capture_changed");
 }
}
TEST_CASE("C4B2 after-read mutations outrank success and typed missing selectors with closed SQLite", "[c4b2-native]") {
 for(bool ledger : {false,true})for(bool miss : {false,true}) {
  fixture f;auto owner=f.owner(miss?std::vector<extension_parsed_option>{{"revision",true,"2"}}:std::vector<extension_parsed_option>{});
  observed_vfs observer;
  stage=[&](int n){if(n==4)f.mutate(ledger);};
  auto result=owner.prepare_history();stage={};registration_failure(result,"capture_changed");
  CHECK(observer.reads>0);CHECK(observer.opens==observer.closes);
 }
}
TEST_CASE("C4B2 helper retains authenticated selector identity and releases locks before return", "[c4b2-native]") {
 for(bool v2 : {false,true})for(bool at : {false,true}) {
  fixture f(v2);auto owner=f.owner(at?std::vector<extension_parsed_option>{{"at-event",true,"1"}}:std::vector<extension_parsed_option>{{"after-event",true,"1"},{"limit",true,"1"}});
  observed_vfs observer;REQUIRE(owner.prepare_history());
  CHECK(observer.opens==observer.closes);
  auto retained=owner.private_history();REQUIRE(retained);
  CHECK(retained->fields.at("last_event")->string=="1");
  CHECK(retained->fields.at("ledger")->string.size()==32);
  CHECK(retained->fields.at("data")->fields.at("selector")->string==(at?"state":"page"));
  if(!at)CHECK(retained->fields.at("data")->fields.at("events")->items.empty());
  auto writer=connection::open(f.root/"ledger",open_mode::read_write,0);REQUIRE(writer);
  auto tx=writer->begin();REQUIRE(tx);REQUIRE(tx->commit());
 }
}
TEST_CASE("C4B2 actual allocation failures preserve native resource and original failure priority", "[c4b2-native]") {
 for(int when : {10,2,4}) {
  fixture f;auto owner=f.owner();observed_vfs observer;bool injected=false;int verifies=0;failed_size=0;
  stage=[&](int n){
   if(when==4&&n==3&&++verifies==2)f.mutate();
   if(!injected&&n==when){injected=true;fail_next=true;}
  };
  auto result=owner.prepare_history();stage={};fail_next=false;
  REQUIRE(injected);CHECK(failed_size>0);
  registration_failure(result,when==4?"capture_changed":"resource_limit");
  CHECK(observer.opens==observer.closes);
  auto retry=owner.prepare_history();registration_failure(retry,when==4?"capture_changed":"resource_limit");
 }
}
TEST_CASE("C4B2 preexisting registration refusal survives actual diagnostic copy allocation failure", "[c4b2-native][c4b2-first-error]") {
 fixture f;auto owner=f.owner();auto original=owner.execute();REQUIRE_FALSE(original);
 auto code=original.error().detail.code;failed_size=0;fail_next=true;
 auto result=owner.prepare_history();fail_next=false;
 REQUIRE(failed_size>0);registration_failure(result,code);
 auto retry=owner.prepare_history();registration_failure(retry,code);
}
