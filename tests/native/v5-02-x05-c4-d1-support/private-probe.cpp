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
extern "C" unsigned long d1_reads();
extern "C" unsigned long d1_enumeration();
namespace {
std::size_t allocation = 0, fail_at = 0;
struct temporary {
  std::filesystem::path path;
  temporary() {
    char p[] = "/tmp/planar-d1-private-XXXXXX";
    path = std::filesystem::canonical(::mkdtemp(p));
    std::filesystem::create_directories(path / "trusted");
    std::filesystem::create_directories(path / "workspace");
    std::ofstream(path / "trusted/profile.json") << R"({"version":1,"modules":[]})";
    std::ofstream(path / "workspace/file") << "positive content";
  }
  ~temporary() { d1_watch(false); fail_at = 0; std::filesystem::remove_all(path); }
  profile_capture capture() {
    auto p = profile_capture::open((path / "trusted/profile.json").string());
    REQUIRE(p);
    REQUIRE(p->admit_root("root", (path / "workspace").string()));
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
  if (fail_at && ++allocation == fail_at) { fail_at = 0; throw std::bad_alloc(); }
  if (auto p = std::malloc(n ? n : 1)) return p;
  throw std::bad_alloc();
}
void operator delete(void* p) noexcept { std::free(p); }
void* operator new[](std::size_t n) { return ::operator new(n); }
void operator delete[](void* p) noexcept { ::operator delete(p); }
TEST_CASE("D1 actual binding has zero data reads and enumeration with positive controls", "[d1-private]") {
  temporary t;
  auto capture = t.capture();
  native_capture_session session;
  d1_watch(true);
  auto bound = capture.bind_root(session, "root", (t.path / "workspace").string(), "root");
  d1_watch(false);
  REQUIRE(bound);
  CHECK(d1_reads() == 0); CHECK(d1_enumeration() == 0);
  d1_watch(true);
  auto file = session.read("root", "file");
  d1_watch(false);
  REQUIRE(file); CHECK(d1_reads() > 0); CHECK(d1_enumeration() == 0);
  d1_watch(true);
  auto tree = session.tree("root");
  d1_watch(false);
  REQUIRE(tree); CHECK(d1_enumeration() > 0);
}
TEST_CASE("D1 failed actual binding reads nothing and releases temporary descriptors", "[d1-private]") {
  temporary t;
  auto capture = t.capture();
  native_capture_session session;
  auto before = descriptors();
  d1_watch(true);
  auto bound = capture.bind_root(session, "root", (t.path / "trusted").string(), "root");
  d1_watch(false);
  REQUIRE_FALSE(bound); CHECK(bound.error().detail.code == "capture_changed");
  CHECK(d1_reads() == 0); CHECK(d1_enumeration() == 0); CHECK(descriptors() == before);
  CHECK_FALSE(session.read("root", "profile.json"));
  CHECK_FALSE(capture.snapshot());
}
TEST_CASE("D1 actual allocation refusals are terminal without descriptor or publication leaks", "[d1-private]") {
  temporary t;
  std::size_t failures = 0, successes = 0;
  for (std::size_t index = 1; index < 64; ++index) {
    auto capture = t.capture();
    auto original = *capture.snapshot();
    native_capture_session session;
    auto logical = std::string("root");
    auto directory = (t.path / "workspace").string();
    auto before = descriptors();
    allocation = 0; fail_at = index;
    auto bound = capture.bind_root(session, std::move(logical), std::move(directory), "root");
    fail_at = 0;
    if (bound) { ++successes; CHECK(index > 1); break; }
    ++failures;
    CHECK(bound.error().detail.code == "resource_limit");
    CHECK(descriptors() == before);
    CHECK_FALSE(capture.snapshot()); CHECK_FALSE(capture.verify());
    CHECK(original->profile.bytes == R"({"version":1,"modules":[]})");
    CHECK_FALSE(session.read("root", "file"));
    REQUIRE(session.add_root("root", (t.path / "workspace").string()));
    CHECK(session.read("root", "file"));
  }
  CHECK(failures >= 2);
  CHECK(successes == 1);
  std::cout << "actual_allocation_refusals=" << failures << "\n";
}
