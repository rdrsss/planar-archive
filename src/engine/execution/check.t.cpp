// R17 native checks exercise real child execution, exact result parsing and stable digest observations.
#include <catch2/catch_test_macros.hpp>
#include <fcntl.h>
#include <unistd.h>
import std;
import planar.engine.execution.check;
import planar.engine.json_codec;
import planar.engine.hash;
using namespace planar::engine;
using namespace planar::engine::execution;
namespace {
struct temporary {
  std::filesystem::path path;
  temporary() {
    char p[] = "/tmp/planar-check-test-XXXXXX";
    path     = ::mkdtemp(p);
    std::filesystem::create_directory(path / "root");
    std::filesystem::create_directory(path / "base");
    std::filesystem::create_directory(path / "scratch");
  }
  ~temporary() {
    std::error_code ec;
    std::filesystem::remove_all(path, ec);
  }
};
value_ptr
plan(std::string command = "printf '{\"version\":1,\"matched\":2,\"passed\":2,\"failed\":0}' > \"$PLANAR_CHECK_RESULT\"") {
  auto argv    = std::make_shared<value>();
  argv->kind   = value_kind::string;
  argv->string = std::move(command);
  auto encoded = codec_encode(argv);
  REQUIRE(encoded);
  auto result = codec_decode(
      R"({"root":"source","base":"initial","registry":{"id":"tools","revision":"v1"},"units":[{"work":"unit","paths":["a"]}],"owned":["a"],"excluded":[],"frozen":[],"producers":[],"facets":[],"contracts":[],"checks":[{"work":"unit","check":"test","toolchain":"shell","command":["/bin/sh","-c"],"argv":[)" +
      *encoded +
      R"(],"environment":{},"timeout_ms":1000,"max_output_bytes":4096,"selection":"nonempty","inputs":["own_output"],"coverage":"test","vector":["unit"]}]})");
  REQUIRE(result);
  return *result;
}
} // namespace
TEST_CASE("R17-C strict result classification preserves precedence and wide counts") {
  process::run_result r{};
  r.how = process::termination::exited;
  REQUIRE(classify(r, "nonempty", R"({"version":1,"matched":4294967296,"passed":4294967296,"failed":0})").outcome == "pass");
  REQUIRE(classify(r, "nonempty", R"({"version":1,"matched":0,"passed":0,"failed":0})").reason == "zero_selected");
  REQUIRE(classify(r, "nonempty", R"({"version":1,"matched":1,"matched":2,"passed":1,"failed":0})").reason == "malformed_result");
  REQUIRE(classify(r, "nonempty", R"({"version":1,"matched":1,"passed":0,"failed":1})").reason == "malformed_result");
  r.exit_code = 7;
  REQUIRE(classify(r, "nonempty", "broken").reason == "exit_status");
  r.how = process::termination::timed_out;
  REQUIRE(classify(r, "nonempty", "broken").reason == "timeout");
}
TEST_CASE("R17-C digest capture matches file domain and rejects links") {
  temporary t;
  std::ofstream(t.path / "root" / "a") << "contents";
  int fd = ::open((t.path / "root").c_str(), O_RDONLY | O_DIRECTORY);
  REQUIRE(fd >= 0);
  auto captured = capture::capture_tree_digest(fd, {});
  REQUIRE(captured);
  REQUIRE(captured->retained.empty());
  REQUIRE(captured->entries.size() == 1);
  REQUIRE(capture::digest_text(captured->entries[0].digest) == *file_digest("contents"));
  std::filesystem::create_symlink("a", t.path / "root" / "b");
  auto bad = capture::capture_tree_digest(fd, {});
  ::close(fd);
  REQUIRE_FALSE(bad);
  REQUIRE(bad.error().code == "path_denied");
}
TEST_CASE("R17-C real check produces required boundary and child evidence") {
  temporary t;
  std::ofstream(t.path / "root" / "a") << "new";
  std::ofstream(t.path / "base" / "a") << "old";
  auto prepared = prepare_check(plan(), t.path / "root", t.path / "base", t.path / "scratch");
  REQUIRE(prepared);
  auto r = run_checks(*prepared, 1);
  REQUIRE(r);
  REQUIRE(r->recapture_matches);
  REQUIRE(r->evidence.size() == 2);
  for (const auto& e : r->evidence)
    REQUIRE(e.outcome == "pass");
  REQUIRE(std::filesystem::is_empty(t.path / "scratch"));
}
TEST_CASE("R17-C child mutation makes all evidence inconclusive") {
  temporary t;
  std::ofstream(t.path / "root" / "a") << "initial";
  auto prepared = prepare_check(plan("printf changed > a"), t.path / "root", t.path / "base", t.path / "scratch");
  REQUIRE(prepared);
  auto r = run_checks(*prepared, 2);
  REQUIRE(r);
  REQUIRE_FALSE(r->recapture_matches);
  for (const auto& e : r->evidence)
    REQUIRE(e.reason == "input_changed");
}
TEST_CASE("R17-C scratch overlap refused before any child or reservation") {
  temporary t;
  auto      prepared = prepare_check(plan(), t.path / "root", t.path / "base", t.path / "root");
  REQUIRE_FALSE(prepared);
  REQUIRE(prepared.error().code == "path_denied");
}
