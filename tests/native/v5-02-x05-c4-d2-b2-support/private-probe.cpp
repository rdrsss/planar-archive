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
