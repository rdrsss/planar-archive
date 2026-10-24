// Copied-production descriptor, syscall and allocation probes for original root discovery.
#include <catch2/catch_test_macros.hpp>
#include <fcntl.h>
#include <sys/stat.h>
#include <unistd.h>
#include <cstdlib>
#include <new>
import std;
import planar.engine.capture.profile;
using namespace planar::engine;
extern "C" void d1_watch(bool);
extern "C" void d2_open_hook(void (*)(int));
extern "C" unsigned long d1_reads();
extern "C" unsigned long d1_enumeration();
namespace {
std::size_t fail_at = 0, allocation = 0;
struct temporary {
  std::filesystem::path path;
  temporary() {
    char name[] = "/tmp/planar-b2-private-XXXXXX";
    path = std::filesystem::canonical(::mkdtemp(name));
    std::filesystem::create_directory(path / "trusted");
    std::filesystem::create_directory(path / "workspace");
    std::ofstream(path / "trusted/profile.json") << R"({"version":1,"modules":[]})";
    std::ofstream(path / "workspace/file") << "original bytes";
  }
  ~temporary() {
    fail_at = 0; d1_watch(false); d2_open_hook(nullptr);
    std::filesystem::remove_all(path);
  }
  capture_admission admission() {
    auto a = capture_admission::create((path / "trusted").string());
    REQUIRE(a);
    REQUIRE(a->select_trusted("profile", (path / "trusted/profile.json").string()));
    REQUIRE(a->admit_root("root", (path / "workspace").string()));
    return std::move(*a);
  }
  profile_capture profile() {
    auto p = profile_capture::open((path / "trusted/profile.json").string());
    REQUIRE(p);
    REQUIRE(p->admit_root("root", (path / "workspace").string()));
    return std::move(*p);
  }
};
int descriptors() {
  int count = 0;
  for (int i = 0; i < 2048; ++i) if (::fcntl(i, F_GETFD) >= 0) ++count;
  return count;
}
}
void* operator new(std::size_t n) {
  if (fail_at && ++allocation == fail_at) { fail_at = 0; throw std::bad_alloc(); }
  if (auto p = std::malloc(n ? n : 1)) return p;
  throw std::bad_alloc();
}
void operator delete(void* p) noexcept { std::free(p); }
void* operator new[](std::size_t n) { return ::operator new(n); }
void operator delete[](void* p) noexcept { ::operator delete(p); }
TEST_CASE("D2 B2 metadata discovery has real zero-read and positive controls", "[d2-b2-private]") {
  temporary t; auto admission = t.admission(); auto root = admission.retain_root("root"); REQUIRE(root);
  d1_watch(true); auto first = root->discover("file"); d1_watch(false);
  REQUIRE(first); CHECK(d1_reads() == 0); CHECK(d1_enumeration() == 0);
  d1_watch(true); auto repeat = root->discover("file"); d1_watch(false);
  REQUIRE(repeat); CHECK(first->same_selection(*repeat)); CHECK(d1_reads() == 0); CHECK(d1_enumeration() == 0);
  native_capture_session session;
  REQUIRE(session.add_root("root", (t.path / "workspace").string(), std::move(*root)));
  d1_watch(true); auto read = session.read("root", "file"); d1_watch(false);
  REQUIRE(read); CHECK(d1_reads() > 0);
  d1_watch(true); auto tree = session.tree("root"); d1_watch(false);
  REQUIRE(tree); CHECK(d1_enumeration() > 0);
}
TEST_CASE("D2 B2 unsafe discoveries and actual leaf FD changes precede content", "[d2-b2-private]") {
  for (int route : {0,1,2,3}) {
    temporary t; auto owner = t.profile(); native_capture_session session;
    REQUIRE(owner.bind_root(session,"root",(t.path/"workspace").string(),"root"));
    if (route == 0) { std::filesystem::remove(t.path/"workspace/file"); std::filesystem::create_hard_link(t.path/"trusted/profile.json",t.path/"workspace/file"); }
    if (route == 1) { std::filesystem::remove(t.path/"workspace/file"); std::filesystem::create_symlink("../trusted/profile.json",t.path/"workspace/file"); }
    if (route == 2) d2_open_hook(+[](int fd) { ::fchmod(fd,0755); });
    if (route == 3) REQUIRE(::chmod((t.path/"workspace").c_str(),0700) == 0);
    d1_watch(true); auto result = session.read("root","file"); d1_watch(false);
    CHECK_FALSE(result); CHECK(d1_reads() == 0); CHECK(d1_enumeration() == 0); CHECK_FALSE(owner.snapshot());
  }
}
TEST_CASE("D2 B2 final membership never admits or reads a new member", "[d2-b2-private]") {
  for (bool added : {false,true}) {
    temporary t; auto owner = t.profile(); native_capture_session session;
    REQUIRE(owner.bind_root(session,"root",(t.path/"workspace").string(),"root"));
    REQUIRE(session.tree("root"));
    if (added) std::ofstream(t.path/"workspace/new") << "new file";
    else std::filesystem::remove(t.path/"workspace/file");
    d1_watch(true); auto result = session.finish(); d1_watch(false);
    CHECK_FALSE(result); CHECK(d1_reads() == 0); CHECK_FALSE(owner.snapshot());
  }
}
TEST_CASE("D2 B2 mixed explicit file authority survives root discovery cache and tree", "[d2-b2-private]") {
  temporary t; auto admission = t.admission();
  REQUIRE(admission.admit_file("explicit",(t.path/"workspace/file").string(),admission_file_role::declaration));
  auto explicit_file = admission.retain_file("explicit"); REQUIRE(explicit_file);
  auto root = admission.retain_root("root"); REQUIRE(root);
  auto discovered = root->discover("file"); REQUIRE(discovered);
  CHECK(explicit_file->same_selection(*discovered));
  native_capture_session session;
  REQUIRE(session.add_root("root",(t.path/"workspace").string(),std::move(*root)));
  REQUIRE(session.read("root","file",std::move(*explicit_file)));
  REQUIRE(session.tree("root"));
  std::ofstream(t.path/"workspace/file",std::ios::trunc) << "changed";
  d1_watch(true); auto result = session.read("root","file"); d1_watch(false);
  CHECK_FALSE(result); CHECK(d1_reads() == 0); CHECK_FALSE(admission.healthy());
}
TEST_CASE("D2 B2 allocation refusal poisons discovery without descriptor leaks", "[d2-b2-private]") {
  std::size_t failures = 0; bool success = false;
  for (std::size_t n = 1; n < 200 && !success; ++n) {
    int baseline = descriptors();
    {
      temporary t; auto admission = t.admission(); auto root = admission.retain_root("root"); REQUIRE(root);
      allocation = 0; fail_at = n; auto result = root->discover("file"); fail_at = 0;
      success = bool(result);
      if (!success) { ++failures; CHECK_FALSE(admission.healthy()); CHECK_FALSE(root->discover("file")); }
    }
    CHECK(descriptors() == baseline);
  }
  CHECK(success); CHECK(failures > 0); std::cout << "discovery_failed_positions=" << failures << '\n';
}
TEST_CASE("D2 B2 allocation refusal covers native read publication and root registration", "[d2-b2-private]") {
  for (bool binding : {false,true}) {
    std::size_t failures = 0; bool success = false;
    for (std::size_t n = 1; n < 250 && !success; ++n) {
      int baseline = descriptors();
      {
        temporary t; auto admission = t.admission(); auto root = admission.retain_root("root"); REQUIRE(root);
        native_capture_session session;
        std::string directory = (t.path/"workspace").string();
        if (!binding) REQUIRE(session.add_root("root",directory,std::move(*root)));
        allocation = 0; fail_at = n;
        bool ok = binding ? bool(session.add_root("root",std::move(directory),std::move(*root))) : bool(session.read("root","file"));
        fail_at = 0; success = ok;
        if (!success) {
          ++failures; CHECK_FALSE(admission.healthy()); CHECK_FALSE(session.finish());
          CHECK_FALSE(session.add_root("retry",(t.path/"workspace").string()));
        }
      }
      CHECK(descriptors() == baseline);
    }
    CHECK(success); CHECK(failures > 0); std::cout << "binding=" << binding << " failed_positions=" << failures << '\n';
  }
}
TEST_CASE("B2 reviewer same-original explicit receipt preserves authority and ordinary root rejects trusted alias", "[b2-review-positive]") {
  {
    temporary t; auto owner=t.admission();
    REQUIRE(owner.admit_file("explicit",(t.path/"workspace/file").string(),admission_file_role::declaration));
    auto selected=owner.retain_file("explicit"); REQUIRE(selected);
    auto root=owner.retain_root("root"); REQUIRE(root);
    auto discovered=root->discover("file"); REQUIRE(discovered);
    REQUIRE(discovered->same_selection(*selected));
    native_capture_session session;
    REQUIRE(session.add_root("root",(t.path/"workspace").string(),std::move(*root)));
    d1_watch(true); auto result=session.read("root","file",std::move(*selected));d1_watch(false);
    REQUIRE(result);CHECK(result->bytes=="original bytes");CHECK(d1_reads()>0);
    auto seal=session.finish_for_acceptance();REQUIRE(seal);CHECK(seal->verify());
  }
  {
    temporary t; auto owner=t.admission();auto root=owner.retain_root("root");REQUIRE(root);
    native_capture_session session;
    REQUIRE(session.add_root("root",(t.path/"workspace").string(),std::move(*root)));
    std::filesystem::remove(t.path/"workspace/file");
    std::filesystem::create_hard_link(t.path/"trusted/profile.json",t.path/"workspace/file");
    d1_watch(true);auto result=session.read("root","file");d1_watch(false);
    CHECK_FALSE(result);CHECK(d1_reads()==0);CHECK_FALSE(owner.healthy());
  }
}
TEST_CASE("B2 reviewer foreign retained receipt cannot bypass original rooted trusted alias policy", "[b2-review-regression]") {
  for(bool bounded:{false,true}) {
    temporary t;
    std::filesystem::remove(t.path/"workspace/file");
    std::filesystem::create_hard_link(t.path/"trusted/profile.json",t.path/"workspace/file");
    auto owner=t.admission();auto root=owner.retain_root("root");REQUIRE(root);
    native_capture_session session;
    REQUIRE(session.add_root("root",(t.path/"workspace").string(),std::move(*root)));
    std::filesystem::create_directory(t.path/"other-trusted");
    auto foreign=capture_admission::create((t.path/"other-trusted").string());REQUIRE(foreign);
    REQUIRE(foreign->admit_file("foreign",(t.path/"workspace/file").string(),admission_file_role::declaration));
    auto selected=foreign->retain_file("foreign");REQUIRE(selected);
    d1_watch(true);
    auto result=bounded?session.read_bounded("root","file",std::move(*selected),1048576,1048576):session.read("root","file",std::move(*selected));
    d1_watch(false);
    auto reads=d1_reads();
    std::cout<<"foreign_root_bypass bounded="<<bounded<<" success="<<bool(result)<<" reads="<<reads;
    if(result)std::cout<<" bytes="<<result->bytes;
    std::cout<<'\n';
    CHECK_FALSE(result);CHECK(reads==0);CHECK_FALSE(owner.healthy());CHECK_FALSE(foreign->healthy());
    if(result) {
      CHECK(result->bytes==R"({"version":1,"modules":[]})");
      auto seal=session.finish_for_acceptance();
      std::cout<<"foreign_root_bypass acceptance_seal="<<bool(seal);
      if(seal)std::cout<<" verify="<<bool(seal->verify());
      std::cout<<'\n';
    }
  }
}

TEST_CASE("C01 exact pre-cache selection and cached routes preserve incoming authority", "[d2-b2-c01-private]") {
  for (bool bounded : {false,true}) {
    temporary t; auto owner=t.admission();
    REQUIRE(owner.admit_file("explicit",(t.path/"workspace/file").string(),admission_file_role::declaration));
    auto selected=owner.retain_file("explicit"); REQUIRE(selected);
    auto repeated=owner.retain_file("explicit"); REQUIRE(repeated);
    auto root=owner.retain_root("root"); REQUIRE(root);
    native_capture_session session;
    REQUIRE(session.add_root("root",(t.path/"workspace").string(),std::move(*root)));
    d1_watch(true);
    auto first=bounded?session.read_bounded("root","file",std::move(*selected),100,100):session.read("root","file",std::move(*selected));
    d1_watch(false); REQUIRE(first); CHECK(d1_reads()>0);
    REQUIRE(session.read("root","file",std::move(*repeated)));
    REQUIRE(session.read("root","file")); REQUIRE(session.tree("root"));
    auto seal=session.finish_for_acceptance(); REQUIRE(seal); REQUIRE(seal->verify()); REQUIRE(seal->verify());
    CHECK(owner.healthy());
  }
}
TEST_CASE("C01 wrong incoming selections refuse before data or enumeration", "[d2-b2-c01-private]") {
  for (bool bounded : {false,true}) for(int route : {0,1,2,3,4}) {
    temporary t;
    if(route==3) std::filesystem::create_hard_link(t.path/"workspace/file",t.path/"workspace/other");
    else std::ofstream(t.path/"workspace/other")<<"other bytes";
    auto owner=t.admission();
    std::filesystem::create_directory(t.path/"other-trusted");
    auto foreign=capture_admission::create((t.path/"other-trusted").string()); REQUIRE(foreign);
    auto& incoming=route<2?*foreign:owner;
    REQUIRE(incoming.admit_file("selected",(t.path/(route==0||route==4?"workspace/file":"workspace/other")).string(),admission_file_role::declaration));
    if(route==4) REQUIRE(owner.admit_file("alternate",(t.path/"workspace/file").string(),admission_file_role::declaration));
    auto selected=incoming.retain_file(route==4?"alternate":"selected"); REQUIRE(selected);
    auto root=owner.retain_root("root"); REQUIRE(root);
    native_capture_session session;
    REQUIRE(session.add_root("root",(t.path/"workspace").string(),std::move(*root)));
    d1_watch(true);
    auto result=bounded?session.read_bounded("root","file",std::move(*selected),100,100):session.read("root","file",std::move(*selected));
    d1_watch(false); CHECK_FALSE(result);CHECK(d1_reads()==0);CHECK(d1_enumeration()==0);
    CHECK_FALSE(owner.healthy());CHECK_FALSE(incoming.healthy());CHECK_FALSE(session.finish_for_acceptance());
    CHECK_FALSE(session.read("root","file"));
  }
}
TEST_CASE("C01 explicit first capture allocation refusal poisons both owners and closes descriptors", "[d2-b2-c01-private]") {
  for(bool foreign_receipt:{false,true}) {
    std::size_t failures=0;bool reached=false;
    for(std::size_t n=1;n<250&&!reached;++n) {
      auto baseline=descriptors();
      {
        temporary t;auto owner=t.admission();
        std::filesystem::create_directory(t.path/"other-trusted");
        auto foreign=capture_admission::create((t.path/"other-trusted").string());REQUIRE(foreign);
        auto& incoming=foreign_receipt?*foreign:owner;
        REQUIRE(incoming.admit_file("explicit",(t.path/"workspace/file").string(),admission_file_role::declaration));
        auto selected=incoming.retain_file("explicit");REQUIRE(selected);
        auto root=owner.retain_root("root");REQUIRE(root);
        native_capture_session session;
        REQUIRE(session.add_root("root",(t.path/"workspace").string(),std::move(*root)));
        allocation=0;fail_at=n;
        auto result=session.read("root","file",std::move(*selected));
        auto positions=allocation;fail_at=0;
        reached=foreign_receipt?positions<n:bool(result);
        if(!result) {
          ++failures;CHECK_FALSE(owner.healthy());CHECK_FALSE(incoming.healthy());CHECK_FALSE(session.finish());
          CHECK_FALSE(session.add_root("retry",(t.path/"workspace").string()));
        }
      }
      CHECK(descriptors()==baseline);
    }
    CHECK(reached);CHECK(failures>0);
    std::cout<<"explicit_foreign="<<foreign_receipt<<" failed_positions="<<failures<<'\n';
  }
}

extern "C" void c01_arm(std::size_t, bool);
extern "C" std::size_t c01_issues();
TEST_CASE("C01 temporary comparison issuance is cumulative and cache seals never discover", "[c01-controlled]") {
  for(bool indexed:{false,true}) for(bool enough:{false,true}) {
    temporary t;auto owner=t.admission();
    REQUIRE(owner.admit_file("explicit",(t.path/"workspace/file").string(),admission_file_role::declaration));
    auto selected=owner.retain_file("explicit");REQUIRE(selected);
    auto repeated=owner.retain_file("explicit");REQUIRE(repeated);
    auto root=owner.retain_root("root");REQUIRE(root);
    if(indexed) REQUIRE(root->discover("file"));
    native_capture_session session;
    REQUIRE(session.add_root("root",(t.path/"workspace").string(),std::move(*root)));
    auto issuance=sizeof(retained_file_receipt)+sizeof(std::optional<retained_file_receipt>)+256+8;
    auto before=c01_issues();
    // Exact repeated issuance succeeds; one byte less refuses. A fresh path also
    // stages the original descriptor/index and therefore needs more than issuance.
    c01_arm(enough?issuance:issuance-1,true);
    d1_watch(true);auto result=session.read("root","file",std::move(*selected));d1_watch(false);
    CHECK(c01_issues()==before+1);
    if(indexed&&enough) {
      REQUIRE(result);
      REQUIRE(session.read("root","file",std::move(*repeated)));
      REQUIRE(session.read("root","file"));REQUIRE(session.tree("root"));
      auto seal=session.finish_for_acceptance();REQUIRE(seal);REQUIRE(seal->verify());REQUIRE(seal->verify());
      CHECK(c01_issues()==before+1);CHECK(owner.healthy());
    } else {
      REQUIRE_FALSE(result);CHECK(result.error().code=="resource_limit");CHECK(d1_reads()==0);
      CHECK_FALSE(owner.healthy());CHECK_FALSE(session.finish());
    }
  }
}
TEST_CASE("C01 first explicit reuse at full file count keeps original private selection", "[c01-controlled]") {
  temporary t;auto owner=t.admission();
  REQUIRE(owner.admit_file("explicit",(t.path/"workspace/file").string(),admission_file_role::declaration));
  auto selected=owner.retain_file("explicit");REQUIRE(selected);
  auto root=owner.retain_root("root");REQUIRE(root);
  native_capture_session session;
  REQUIRE(session.add_root("root",(t.path/"workspace").string(),std::move(*root)));
  c01_arm(1048576,true);
  REQUIRE(session.read("root","file",std::move(*selected)));
  auto seal=session.finish_for_acceptance();REQUIRE(seal);CHECK(seal->verify());
}
