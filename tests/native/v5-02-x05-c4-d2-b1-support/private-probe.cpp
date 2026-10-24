// Actual syscall and allocation controls over copied production dependencies only.
#include <catch2/catch_test_macros.hpp>
#include <dirent.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <sys/uio.h>
#include <unistd.h>
#include <cstdlib>
#include <new>
import std;
import planar.engine.capture.profile;
using namespace planar::engine;
extern "C" void d1_watch(bool);
extern "C" void d2_open_hook(void (*)(int));
extern "C" unsigned long d2_metadata();
extern "C" void d2_metadata_reset();
extern "C" unsigned long d1_reads();
extern "C" unsigned long d1_enumeration();
namespace {
std::size_t allocation = 0, fail_at = 0, observed = 0, first_allocation = 0;
bool observe = false;
std::size_t failed_size = 0;
struct temporary {
  std::filesystem::path path;
  temporary() {
    char p[] = "/tmp/planar-d1-private-XXXXXX";
    path = std::filesystem::canonical(::mkdtemp(p));
    std::filesystem::create_directories(path / "trusted");
    std::filesystem::create_directories(path / "workspace");
    std::ofstream(path / "trusted/profile.json") << R"({"version":1,"modules":[]})";
    std::ofstream(path / "workspace/file") << std::string(256,'x');
  }
  ~temporary() { d2_open_hook(nullptr); d1_watch(false); fail_at = 0; std::filesystem::remove_all(path); }
  profile_capture capture() {
    auto p = profile_capture::open((path / "trusted/profile.json").string());
    REQUIRE(p);
    REQUIRE(p->admit_root("root", (path / "workspace").string()));
    REQUIRE(p->admit_file("file", (path / "workspace/file").string(), admission_file_role::declaration));
    return std::move(*p);
  }
};
int descriptors() {
  int count = 0;
  for (int fd = 0; fd < 2048; ++fd)
    if (::fcntl(fd, F_GETFD) >= 0) ++count;
  return count;
}
}
void* operator new(std::size_t n) {
  if (observe && ++observed == 1) first_allocation = n;
  if (fail_at && ++allocation == fail_at) { failed_size=n; fail_at = 0; throw std::bad_alloc(); }
  if (auto p = std::malloc(n ? n : 1)) return p;
  throw std::bad_alloc();
}
void operator delete(void* p) noexcept { std::free(p); }
void* operator new[](std::size_t n) { return ::operator new(n); }
void operator delete[](void* p) noexcept { ::operator delete(p); }

TEST_CASE("D2 A actual reads have positive controls and FD time refusal", "[d2-b1-private]") {
  for (int route : {0,1,2,3,4,5,6,7,8}) {
    temporary t; auto owner = t.capture(); native_capture_session session;
    REQUIRE(owner.bind_root(session,"root",(t.path/"workspace").string(),"root"));
    if (route == 0) d2_open_hook(+[](int fd) { ::fchmod(fd,0755); });
    if (route == 1) { std::filesystem::remove(t.path/"workspace/file"); std::filesystem::create_hard_link(t.path/"trusted/profile.json",t.path/"workspace/file"); }
    if (route > 1) REQUIRE(owner.read(session,"root","file","file"));
    std::optional<acceptance_capture> seal;
    if (route == 6) { auto s = session.finish_for_acceptance(); REQUIRE(s); seal.emplace(std::move(*s)); }
    if (route > 1 && route != 5 && route != 7 && route != 8) { std::ofstream(t.path/"workspace/file",std::ios::trunc) << "different bytes"; }
    if (route == 5) session.before_verification([&] { std::ofstream(t.path/"workspace/file",std::ios::trunc)<<"seal mutation"; });
    if (route == 7) { std::filesystem::rename(t.path/"workspace",t.path/"old-workspace");std::filesystem::create_directory(t.path/"workspace"); }
    if (route == 8) std::filesystem::resize_file(t.path/"workspace/file",16777217);
    d1_watch(true);
    bool ok = route < 2 ? bool(owner.read(session,"root","file","file")) :
      route == 2 ? bool(session.read("root","file")) :
      route == 3 ? bool(owner.read_bounded(session,"root","file","file",64,64)) :
      route == 4 ? bool(session.tree("root")) :
      route == 5 ? bool(session.finish()) : route == 6 ? bool(seal->verify()) : bool(session.read("root","file"));
    d1_watch(false); CHECK_FALSE(ok); CHECK(d1_reads()==0); CHECK_FALSE(owner.snapshot());
  }
  std::cout<<"retained_handle_bytes="<<sizeof(retained_file_receipt)<<" optional_handle_bytes="<<sizeof(std::optional<retained_file_receipt>)<<" admission_wrapper_bytes="<<sizeof(capture_admission)<<"\n";
  temporary t; auto owner=t.capture(); native_capture_session session;
  REQUIRE(owner.bind_root(session,"root",(t.path/"workspace").string(),"root"));
  d1_watch(true); auto file=owner.read(session,"root","file","file"); d1_watch(false);
  REQUIRE(file); CHECK(d1_reads()>0); CHECK(d1_enumeration()==0);
  d1_watch(true); auto cached=session.read("root","file"); d1_watch(false); REQUIRE(cached); CHECK(d1_reads()>0);
  d1_watch(true); auto sealed=session.finish_for_acceptance(); d1_watch(false); REQUIRE(sealed); CHECK(d1_reads()>0);
  d1_watch(true); auto verified=sealed->verify(); d1_watch(false); REQUIRE(verified); CHECK(d1_reads()>0);
}
TEST_CASE("D2 A no filesystem snapshot observes shared poison and surviving tokens release descriptors", "[d2-b1-private]") {
  temporary t; auto before=descriptors();
  {
    auto directory=(t.path/"trusted").string(); observed=0;observe=true;
    auto admission=capture_admission::create(directory); observe=false; REQUIRE(admission);
    std::cout<<"shared_state_first_allocation_bytes="<<first_allocation<<"\n";
    REQUIRE(admission->admit_file("file",(t.path/"workspace/file").string(),admission_file_role::registry));
    observed=0;observe=true; auto token=admission->retain_file("file");observe=false;
    REQUIRE(token);CHECK(observed==0);
  }
  CHECK(descriptors()==before);
  {
    native_capture_session session; REQUIRE(session.add_root("root",(t.path/"workspace").string()));
    {
      auto owner=t.capture(); REQUIRE(owner.read(session,"root","file","file"));
      d2_metadata_reset(); d1_watch(true); auto snapshot=owner.snapshot(); d1_watch(false);
      REQUIRE(snapshot); CHECK(d1_reads()==0); CHECK(d2_metadata()==0);
      std::ofstream(t.path/"workspace/file",std::ios::trunc)<<"changed";
      CHECK_FALSE(session.read("root","file"));
      d2_metadata_reset(); d1_watch(true); auto failed=owner.snapshot(); d1_watch(false);
      CHECK_FALSE(failed); CHECK(d1_reads()==0); CHECK(d2_metadata()==0);
    }
    CHECK_FALSE(session.finish());
  }
  CHECK(descriptors()==before);
  std::ofstream(t.path/"workspace/file",std::ios::trunc)<<"positive content";
  {
    native_capture_session session; REQUIRE(session.add_root("root",(t.path/"workspace").string()));
    { auto owner=t.capture(); REQUIRE(owner.read(session,"root","file","file")); }
    REQUIRE(session.read("root","file")); REQUIRE(session.finish());
  }
  CHECK(descriptors()==before);
}
TEST_CASE("D2 A actual allocation refusals poison before copy and publish no partial entry", "[d2-b1-private]") {
  temporary t; std::size_t failures=0, successes=0; bool before_content_failure=false, after_content_failure=false;
  for(std::size_t index=1;index<256;++index) {
    auto owner=t.capture(); auto original=*owner.snapshot(); native_capture_session session;
    REQUIRE(session.add_root("root",(t.path/"workspace").string())); auto before=descriptors();
    allocation=0; fail_at=index;
    d1_watch(true); auto read=owner.read(session,"root","file","file"); fail_at=0; d1_watch(false);
    if(read) { ++successes; break; }
    ++failures; before_content_failure |= d1_reads()==0; after_content_failure |= d1_reads()>0;
    std::cout<<"initial_allocation_failure_index="<<index<<" size="<<failed_size<<" reads="<<d1_reads()<<"\n";
    CHECK(read.error().detail.code=="resource_limit"); CHECK_FALSE(owner.snapshot());
    CHECK(descriptors()==before); CHECK(original->profile.bytes==R"({"version":1,"modules":[]})");
    auto sealed=session.finish(); REQUIRE(sealed); CHECK((*sealed)->empty());
  }
  CHECK(before_content_failure); CHECK(after_content_failure); CHECK(successes==1);
  std::cout<<"retained_initial_allocation_refusals="<<failures<<"\n";
  failures=0;successes=0;
  for(std::size_t index=1;index<256;++index) {
    auto owner=t.capture(); native_capture_session session; REQUIRE(session.add_root("root",(t.path/"workspace").string()));
    REQUIRE(owner.read(session,"root","file","file")); auto before=descriptors(); allocation=0;fail_at=index;
    d1_watch(true); auto read=session.read("root","file");fail_at=0;d1_watch(false);
    if(read) { ++successes;break; }
    ++failures; std::cout<<"cache_allocation_failure_index="<<index<<" size="<<failed_size<<" reads="<<d1_reads()<<"\n"; CHECK(read.error().code=="resource_limit"); CHECK_FALSE(owner.snapshot()); CHECK(descriptors()==before);
  }
  CHECK(failures>0);CHECK(successes==1); std::cout<<"retained_cache_allocation_refusals="<<failures<<"\n";
}
TEST_CASE("D2 B1 actual binding and unread seals are metadata only with positive read controls", "[d2-b1-private]") {
  temporary t; auto before=descriptors();
  {
    native_capture_session session;
    {
      auto owner=t.capture();
      d1_watch(true); auto bound=owner.bind_input(session,"input",(t.path/"workspace/file").string(),"file");d1_watch(false);
      REQUIRE(bound);CHECK(d1_reads()==0);CHECK(d1_enumeration()==0);
    }
    d1_watch(true);auto seal=session.finish_for_acceptance();d1_watch(false);
    REQUIRE(seal);CHECK(d1_reads()==0);CHECK(d1_enumeration()==0);CHECK((**seal->snapshot()).empty());
    d1_watch(true);auto verified=seal->verify();d1_watch(false);REQUIRE(verified);CHECK(d1_reads()==0);CHECK(d1_enumeration()==0);
    std::ofstream(t.path/"workspace/file",std::ios::trunc)<<"changed";
    d1_watch(true);auto changed=seal->verify();d1_watch(false);CHECK_FALSE(changed);CHECK(d1_reads()==0);CHECK(d1_enumeration()==0);
  }
  CHECK(descriptors()==before);
  {
    auto owner=t.capture();native_capture_session session;
    REQUIRE(owner.bind_input(session,"input",(t.path/"workspace/file").string(),"file"));
    d1_watch(true);auto read=session.read("input","input.json");d1_watch(false);REQUIRE(read);CHECK(d1_reads()>0);CHECK(d1_enumeration()==0);
    d1_watch(true);auto cached=session.read("input","input.json");d1_watch(false);REQUIRE(cached);CHECK(d1_reads()>0);
    d1_watch(true);auto seal=session.finish_for_acceptance();d1_watch(false);REQUIRE(seal);CHECK(d1_reads()>0);
    d1_watch(true);auto verified=seal->verify();d1_watch(false);REQUIRE(verified);CHECK(d1_reads()>0);
  }
  CHECK(descriptors()==before);
}
TEST_CASE("D2 B1 actual FD and changed input paths refuse before bytes or enumeration", "[d2-b1-private]") {
  for(int route=0;route<8;++route) {
    temporary t;auto owner=t.capture();native_capture_session session;
    if(route==0) {
      d2_open_hook(+[](int fd) { ::fchmod(fd,0755); });
      d1_watch(true);auto bound=owner.bind_input(session,"input",(t.path/"workspace/file").string(),"file");d1_watch(false);
      CHECK_FALSE(bound);
    } else {
      REQUIRE(owner.bind_input(session,"input",(t.path/"workspace/file").string(),"file"));
      if(route==1) d2_open_hook(+[](int fd) { ::fchmod(fd,0755); });
      if(route==2) { std::filesystem::remove(t.path/"workspace/file");std::filesystem::create_hard_link(t.path/"trusted/profile.json",t.path/"workspace/file"); }
      if(route==3) std::filesystem::resize_file(t.path/"workspace/file",16777217);
      if(route==4) { std::filesystem::rename(t.path/"workspace",t.path/"old-workspace");std::filesystem::create_directory(t.path/"workspace"); }
      if(route==5) session.before_verification([&] { std::ofstream(t.path/"workspace/file",std::ios::trunc)<<"changed"; });
      d1_watch(true);
      bool ok = route==5 ? bool(session.finish()) : route==6 ? bool(session.tree("input")) : route==7 ? bool(session.read("input","sibling")) : bool(session.read("input","input.json"));
      d1_watch(false);CHECK_FALSE(ok);
    }
    CHECK(d1_reads()==0);CHECK(d1_enumeration()==0);CHECK_FALSE(owner.snapshot());CHECK_FALSE(session.read("input","input.json"));
  }
}
TEST_CASE("D2 B1 actual binding and first read allocation failures are terminal and close descriptors", "[d2-b1-private]") {
  for(bool initial_read:{false,true}) {
    std::size_t failures=0,successes=0;bool before_content=false,after_content=false;
    for(std::size_t index=1;index<256;++index) {
      temporary t;auto owner=t.capture();native_capture_session session;
      if(initial_read) REQUIRE(owner.bind_input(session,"input",(t.path/"workspace/file").string(),"file"));
      auto before=descriptors();auto file=(t.path/"workspace/file").string();std::string logical="input";
      allocation=0;fail_at=index;d1_watch(true);
      bool ok;std::string code;
      if(initial_read) { auto result=session.read("input","input.json");fail_at=0;d1_watch(false);ok=bool(result);if(!result)code=result.error().code; }
      else { auto result=owner.bind_input(session,std::move(logical),std::move(file),"file");fail_at=0;d1_watch(false);ok=bool(result);if(!result)code=result.error().detail.code; }
      if(ok) { ++successes;break; }
      ++failures;before_content|=d1_reads()==0;after_content|=d1_reads()>0;
      std::cout<<"B1_"<<(initial_read?"first_read":"binding")<<"_allocation_failure="<<index<<" size="<<failed_size<<" reads="<<d1_reads()<<"\n";
      CHECK(code=="resource_limit");CHECK_FALSE(owner.snapshot());CHECK(descriptors()==before);
      if(initial_read) { CHECK_FALSE(session.read("input","input.json"));CHECK_FALSE(session.finish()); }
      else CHECK_FALSE(session.read("input","input.json"));
    }
    CHECK(failures>0);CHECK(successes==1);CHECK(before_content);if(initial_read)CHECK(after_content);
  }
}
TEST_CASE("D2 B1 unread sealing allocation failures retain original authority", "[d2-b1-private]") {
  std::size_t failures=0,successes=0;
  for(std::size_t index=1;index<256;++index) {
    temporary t;auto owner=t.capture();native_capture_session session;
    REQUIRE(owner.bind_input(session,"input",(t.path/"workspace/file").string(),"file"));auto before=descriptors();
    allocation=0;fail_at=index;d1_watch(true);auto sealed=session.finish_for_acceptance();fail_at=0;d1_watch(false);
    CHECK(d1_reads()==0);CHECK(d1_enumeration()==0);
    if(sealed) { ++successes;REQUIRE(sealed->verify());break; }
    ++failures;std::cout<<"B1_unread_seal_allocation_failure="<<index<<" size="<<failed_size<<"\n";
    CHECK(sealed.error().code=="resource_limit");CHECK_FALSE(owner.snapshot());CHECK(descriptors()==before);CHECK_FALSE(session.read("input","input.json"));
  }
  CHECK(failures>0);CHECK(successes==1);
}
