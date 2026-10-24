// Historical missing retained authority: expected failure over the preserved pre-edit native implementation.
#include <catch2/catch_test_macros.hpp>
#include <sys/stat.h>
#include <unistd.h>
import std;
import planar.engine.capture.native;
using namespace planar::engine;
TEST_CASE("D2 A historical borrowed cache does not retain original before read authority", "[d2-a-baseline]") {
  char pattern[]="/tmp/planar-d2-baseline-XXXXXX";
  auto path=std::filesystem::canonical(::mkdtemp(pattern));
  std::filesystem::create_directories(path/"trusted"); std::filesystem::create_directories(path/"workspace");
  std::ofstream(path/"workspace/file")<<"original content";
  {
    auto owner=capture_admission::create((path/"trusted").string()); REQUIRE(owner);
    REQUIRE(owner->admit_file("file",(path/"workspace/file").string(),admission_file_role::registry));
    native_capture_session session; REQUIRE(session.add_root("root",(path/"workspace").string()));
    REQUIRE(session.read("root","file",*owner,"file"));
    REQUIRE(::chmod((path/"workspace/file").c_str(),0755)==0);
    CHECK_FALSE(session.read("root","file"));
  }
  std::filesystem::remove_all(path);
}
