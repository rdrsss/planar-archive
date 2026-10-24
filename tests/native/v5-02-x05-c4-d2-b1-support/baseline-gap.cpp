// Historical missing standalone retained binding: expected failure over the preserved pre-edit native implementation.
#include <catch2/catch_test_macros.hpp>
#include <sys/stat.h>
#include <unistd.h>
import std;
import planar.engine.capture.native;
using namespace planar::engine;
TEST_CASE("D2 B1 historical standalone unbound input lacks original first read authority", "[d2-b1-baseline]") {
  char pattern[]="/tmp/planar-d2-baseline-XXXXXX";
  auto path=std::filesystem::canonical(::mkdtemp(pattern));
  std::filesystem::create_directories(path/"trusted"); std::filesystem::create_directories(path/"workspace");
  std::ofstream(path/"workspace/file")<<"original content";
  {
    auto owner=capture_admission::create((path/"trusted").string()); REQUIRE(owner);
    REQUIRE(owner->admit_file("file",(path/"workspace/file").string(),admission_file_role::registry));
    native_capture_session session; REQUIRE(session.add_input("root",(path/"workspace/file").string()));
    REQUIRE(::chmod((path/"workspace/file").c_str(),0755)==0);
    CHECK_FALSE(session.read("root","input.json"));
  }
  std::filesystem::remove_all(path);
}
