// Test-only post-native-success refusal; actual native export and unchanged history fixture.
#include <catch2/catch_test_macros.hpp>
#include <sys/wait.h>
#include <unistd.h>
import std;
import planar.test.export_command;
namespace {
std::string bytes(const std::filesystem::path& p) {
  std::ifstream in(p, std::ios::binary);
  return {std::istreambuf_iterator<char>(in), {}};
}
struct scratch {
  std::filesystem::path root;
  scratch() {
    char name[] = "/tmp/planar-export-response-XXXXXX";
    if (!::mkdtemp(name))
      throw std::runtime_error("scratch root");
    root     = std::filesystem::canonical(name);
    auto pid = ::fork();
    if (pid == 0) {
      ::execl(PLANAR_HISTORY_FIXTURE, PLANAR_HISTORY_FIXTURE, root.c_str(), "history", nullptr);
      ::_exit(99);
    }
    int status = 0;
    if (pid < 0 || ::waitpid(pid, &status, 0) != pid || !WIFEXITED(status) || WEXITSTATUS(status) != 0)
      throw std::runtime_error("scratch fixture");
  }
  ~scratch() {
    std::error_code ec;
    std::filesystem::remove_all(root, ec);
  }
};
} // namespace
TEST_CASE("export postpublication encoding refusal returns prebuilt uncertainty", "[export][response_fault]") {
  scratch    s;
  const auto before = bytes(s.root / "ledger");
  for (const auto* mode : {"encoding", "allocation"}) {
    auto target = s.root / mode;
    auto result = planar::cmd::handlers_fault::execute(
        planar::engine::command_export_options{(s.root / "ledger").string(), target.string(), 1});
    CHECK(result.status == 4);
    CHECK(result.output.empty());
    CHECK(result.stdout_error.empty());
    auto error = planar::engine::codec_decode(result.error);
    REQUIRE(error);
    auto fields = (*error)->fields.at("error");
    CHECK(fields->fields.at("code")->string == "io");
    CHECK(fields->fields.at("message")->string == "Export may be complete; inspect the explicit target before retrying");
    CHECK(fields->fields.at("details")->fields.at("operation")->string == "export");
    CHECK(bytes(target / "a.lua") == "original");
    CHECK(std::distance(std::filesystem::directory_iterator(target), std::filesystem::directory_iterator{}) == 1);
    CHECK(bytes(s.root / "ledger") == before);
  }
}

TEST_CASE("export service facts allocation failure retains adapter publication", "[export][response_fault]") {
  scratch    s;
  const auto before = bytes(s.root / "ledger");
  auto       target = s.root / "service-facts";
  auto       result = planar::cmd::handlers_fault::execute(
      planar::engine::command_export_options{(s.root / "ledger").string(), target.string(), 1});
  CHECK(result.status == 4);
  CHECK(result.output.empty());
  CHECK(result.stdout_error.empty());
  auto error = planar::engine::codec_decode(result.error);
  REQUIRE(error);
  auto fields = (*error)->fields.at("error");
  CHECK(fields->fields.at("code")->string == "io");
  CHECK(fields->fields.at("message")->string == "Export may be complete; inspect the explicit target before retrying");
  CHECK(fields->fields.at("details")->fields.at("operation")->string == "export");
  CHECK(bytes(target / "a.lua") == "original");
  CHECK(std::distance(std::filesystem::directory_iterator(target), std::filesystem::directory_iterator{}) == 1);
  CHECK(bytes(s.root / "ledger") == before);
}
