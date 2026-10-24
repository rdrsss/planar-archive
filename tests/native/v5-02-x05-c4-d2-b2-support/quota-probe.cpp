// Controlled copied-production count ceiling and private numeric-key namespace regression.
#include <catch2/catch_test_macros.hpp>
#include <unistd.h>
import std;
import planar.engine.capture.profile;
using namespace planar::engine;
namespace {
struct temporary {
  std::filesystem::path path;
  temporary() {
    char p[]="/tmp/planar-b2-quota-XXXXXX";
    path=std::filesystem::canonical(::mkdtemp(p));
    std::filesystem::create_directory(path/"trusted");
    std::filesystem::create_directory(path/"workspace");
    std::ofstream(path/"trusted/profile") << "trusted";
    std::ofstream(path/"workspace/file") << "original";
    std::ofstream(path/"workspace/new") << "new";
    std::ofstream(path/"caller") << "caller";
  }
  ~temporary() { std::filesystem::remove_all(path); }
  capture_admission admitted(std::string_view root) {
    auto a=capture_admission::create((path/"trusted").string()); REQUIRE(a);
    REQUIRE(a->select_trusted("profile",(path/"trusted/profile").string()));
    REQUIRE(a->admit_root(root,(path/"workspace").string()));
    return std::move(*a);
  }
};
}
TEST_CASE("D2 B2 full file count reuses an explicitly admitted original selection", "[d2-b2-quota]") {
  temporary t; auto a=t.admitted("quota");
  REQUIRE(a.admit_file("explicit",(t.path/"workspace/file").string(),admission_file_role::declaration));
  auto original=a.retain_file("explicit"); REQUIRE(original);
  auto root=a.retain_root("quota"); REQUIRE(root);
  // Copied implementation sets files=max_files at this exact issuance boundary.
  auto reused=root->discover("file"); REQUIRE(reused);
  CHECK(reused->same_selection(*original));
  REQUIRE(root->discover("file"));
  auto fresh=root->discover("new"); REQUIRE_FALSE(fresh);
  CHECK(fresh.error().code=="resource_limit"); CHECK_FALSE(a.healthy());
}
TEST_CASE("D2 B2 private discovery ordinal may equal a caller numeric key", "[d2-b2-quota]") {
  temporary t; auto a=t.admitted("root");
  // Four selections exist after this admission; next private ordinal is exactly4.
  REQUIRE(a.admit_file("4",(t.path/"caller").string(),admission_file_role::observation));
  auto caller=a.retain_file("4"); REQUIRE(caller);
  auto root=a.retain_root("root"); REQUIRE(root);
  auto file=root->discover("file"); REQUIRE(file);
  CHECK_FALSE(file->same_selection(*caller));
  auto repeated=root->discover("file"); REQUIRE(repeated);
  CHECK(file->same_selection(*repeated)); CHECK(a.healthy());
}
