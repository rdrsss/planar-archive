// Controlled native allocator, result-phase receipt mutation and watched inode reads.
#include <cstdlib>
#include <fcntl.h>
#include <new>
#include <sys/stat.h>
namespace {
thread_local bool watching_new = false;
thread_local std::size_t requests = 0, fault_one = 0, fault_two = 0;
thread_local const char* mutation_path = nullptr;
thread_local bool mutated = false;
}
void* operator new(std::size_t bytes) {
  if (watching_new) {
    ++requests;
    if (mutation_path && !mutated && bytes >= 1048576 && bytes <= 1048608) {
      int descriptor = ::open(mutation_path, O_WRONLY | O_TRUNC);
      if (descriptor >= 0) {
        mutated = ::write(descriptor, "changed", 7) == 7;
        ::close(descriptor);
      }
    }
    if (requests == fault_one || requests == fault_two)
      throw std::bad_alloc{};
  }
  if (auto result = std::malloc(bytes ? bytes : 1))
    return result;
  throw std::bad_alloc{};
}
void operator delete(void* pointer) noexcept { std::free(pointer); }
void* operator new[](std::size_t bytes) { return ::operator new(bytes); }
void operator delete[](void* pointer) noexcept { ::operator delete(pointer); }
extern "C" void planar_admission_watch(std::uint64_t, std::uint64_t);
extern "C" std::uint64_t planar_admission_read_attempts();
TEST_CASE("C4 A private actual native allocation refusals cover both response branches", "[c4-a-private]") {
  for (auto body : {extension_summary(), std::string("return {ok=false,error={code='schema',message='semantic',field=''}}")}) {
    extension_fixture f("");
    f.put("trusted/module.lua", extension_execution_source(body));
    auto healthy = f.owner();
    REQUIRE(healthy.admit("inspect", {}, {}));
    requests = 0;
    watching_new = true;
    auto result = healthy.execute();
    watching_new = false;
    REQUIRE(result);
    auto count = requests;
    REQUIRE(count > 10);
    for (std::size_t position=1;position<=count;++position) {
      auto owner = f.owner();
      REQUIRE(owner.admit("inspect", {}, {}));
      requests=0;fault_one=position;watching_new=true;
      auto refusal = owner.execute();
      watching_new=false;fault_one=0;
      INFO(position);
      REQUIRE_FALSE(refusal);
      CHECK((refusal.error().detail.code=="resource_limit" || refusal.error().detail.code=="capture_changed"));
      CHECK((refusal.error().field=="catalog.commands" || refusal.error().field=="catalog.commands[6]" || refusal.error().field=="profile"));
      auto moved=std::move(owner);
      auto retry=moved.execute();
      REQUIRE_FALSE(retry);
      extension_same_error(retry.error(),refusal.error());
      extension_closed(moved,refusal.error());
    }
    std::println("actual one-shot execution allocation positions: {}",count);
  }
}
TEST_CASE("C4 A private published protocol error dominates diagnostic copy refusal", "[c4-a-private]") {
  extension_fixture f("");
  f.put("trusted/module.lua",extension_execution_source("return nil"));
  auto healthy=f.owner();REQUIRE(healthy.admit("inspect",{},{}));
  requests=0;watching_new=true;
  auto original=healthy.execute();
  watching_new=false;
  REQUIRE_FALSE(original);
  auto count=requests;
  REQUIRE(count>5);
  // The final request copies the already-published diagnostic. A second
  // fault on its immediate retry exercises the outer allocation adapter.
  for (bool twice : {false,true}) {
    auto owner=f.owner();REQUIRE(owner.admit("inspect",{},{}));
    requests=0;fault_one=count;fault_two=twice ? count+1 : 0;watching_new=true;
    auto refusal=owner.execute();
    watching_new=false;fault_one=fault_two=0;
    REQUIRE_FALSE(refusal);
    extension_same_error(refusal.error(),original.error());
    extension_closed(owner,original.error());
  }
  std::println("protocol error allocation requests: {}",count);
}
TEST_CASE("C4 A private original receipt mutation during owned result copy suppresses either branch", "[c4-a-private]") {
  for (bool semantic : {false,true}) {
    extension_fixture f("");
    std::string body="local t='a';for i=1,20 do t=t..t end; ";
    body+=semantic ? "return {ok=false,error={code='schema',message=t,field=''}}" : extension_summary("t");
    f.put("trusted/module.lua",extension_execution_source(body));
    auto healthy=f.owner();REQUIRE(healthy.admit("inspect",{},{}));
    REQUIRE(healthy.execute());
    auto owner=f.owner();REQUIRE(owner.admit("inspect",{},{}));
    auto file=(f.path/"trusted/module.lua").string();
    mutated=false;mutation_path=file.c_str();watching_new=true;
    auto refusal=owner.execute();
    watching_new=false;mutation_path=nullptr;
    REQUIRE(mutated);
    REQUIRE_FALSE(refusal);
    CHECK(refusal.error().detail.code=="capture_changed");
    auto retry=owner.execute();REQUIRE_FALSE(retry);
    extension_same_error(retry.error(),refusal.error());
  }
}
TEST_CASE("C4 A private closed capability execution performs zero watched late reads", "[c4-a-private]") {
  extension_fixture f;
  auto source=extension_protocol_source("role_context_read_v1");
  source.replace(source.find("function() error("),std::string::npos,"function() while true do end end)");
  f.put("trusted/module.lua",source);
  for (auto selected : {"workspaces/demo/workspace.lua","trusted/registry.json","observations.json"}) {
    auto owner=f.owner();REQUIRE(owner.admit("inspect",f.role(true),f.cwd));
    auto path=f.path/selected;struct stat metadata{};
    REQUIRE(::stat(path.c_str(),&metadata)==0);
    planar_admission_watch(metadata.st_dev,metadata.st_ino);
    auto refusal=owner.execute();REQUIRE_FALSE(refusal);
    CHECK(refusal.error().detail.code=="internal");
    CHECK(planar_admission_read_attempts()==0);
    int descriptor=::open(path.c_str(),O_RDONLY|O_NOFOLLOW);REQUIRE(descriptor>=0);
    char byte;CHECK(::read(descriptor,&byte,1)==1);::close(descriptor);
    CHECK(planar_admission_read_attempts()==1);
    planar_admission_watch(0,0);
  }
}
TEST_CASE("C4 A private controlled inherited conversion node refusal", "[c4-a-node-variant]") {
  extension_execution_refused(extension_summary(),"resource_limit");
}
TEST_CASE("C4 A private controlled inherited native accounting refusal", "[c4-a-native-variant]") {
  extension_execution_refused(extension_summary(),"resource_limit");
}

TEST_CASE("C4 A private both owner kinds preserve shared layout", "[c4-a-layout-variant]") {
  auto embedded = command_vm::create();
  REQUIRE(embedded);
  extension_fixture f("");
  f.put("trusted/module.lua",extension_execution_source(extension_summary()));
  auto owner=f.owner();REQUIRE(owner.admit("inspect",{},{}));REQUIRE(owner.execute());
}
