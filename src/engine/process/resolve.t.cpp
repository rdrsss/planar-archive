// R17-B resolve: R17-2 rule 1 executable resolution against an explicit environment.
#include <catch2/catch_test_macros.hpp>
#include <stdlib.h>
#include <unistd.h>
import std;
import planar.engine.process.resolve;
using namespace planar::engine::process;
namespace {
using environment_t = std::vector<std::pair<std::string, std::string>>;
struct temporary {
  std::filesystem::path path;
  temporary() {
    char pattern[] = "/tmp/planar-r17b-resolve-XXXXXX";
    path           = std::filesystem::canonical(::mkdtemp(pattern));
  }
  ~temporary() {
    std::error_code ignored;
    std::filesystem::remove_all(path, ignored);
  }
  temporary(const temporary&)            = delete;
  temporary& operator=(const temporary&) = delete;
};
// Writes a shell script with the given permission bits, creating parent directories.
std::string script(const std::filesystem::path& file, std::filesystem::perms permissions) {
  std::filesystem::create_directories(file.parent_path());
  std::ofstream(file) << "#!/bin/sh\nexit 0\n";
  std::filesystem::permissions(file, permissions);
  return file.string();
}
constexpr auto executable     = std::filesystem::perms::owner_all;
constexpr auto not_executable = std::filesystem::perms::owner_read | std::filesystem::perms::owner_write;
// Restores the process cwd; ctest runs each case in its own process.
struct cwd_guard {
  std::filesystem::path saved = std::filesystem::current_path();
  explicit cwd_guard(const std::filesystem::path& target) {
    std::filesystem::current_path(target);
  }
  ~cwd_guard() {
    std::error_code ignored;
    std::filesystem::current_path(saved, ignored);
  }
};
void missing(const std::expected<std::string, resolve_outcome>& result) {
  REQUIRE_FALSE(result.has_value());
  CHECK(result.error() == resolve_outcome::missing);
}
} // namespace

TEST_CASE("R17-B resolve absolute executable path is found", "[process]") {
  temporary  root;
  const auto tool   = script(root.path / "tool", executable);
  const auto result = resolve_executable(tool, {});
  REQUIRE(result.has_value());
  CHECK(*result == tool);
}

TEST_CASE("R17-B resolve absolute symlink to executable keeps the link spelling", "[process]") {
  temporary root;
  script(root.path / "real", executable);
  std::filesystem::create_symlink(root.path / "real", root.path / "link");
  const auto result = resolve_executable((root.path / "link").string(), {});
  REQUIRE(result.has_value());
  CHECK(*result == (root.path / "link").string());
}

TEST_CASE("R17-B resolve absolute non-executable regular file is missing", "[process]") {
  temporary root;
  missing(resolve_executable(script(root.path / "tool", not_executable), {}));
}

TEST_CASE("R17-B resolve absolute directory is missing", "[process]") {
  temporary root;
  std::filesystem::create_directory(root.path / "dir");
  missing(resolve_executable((root.path / "dir").string(), {}));
}

TEST_CASE("R17-B resolve absolute nonexistent and dangling symlink are missing", "[process]") {
  temporary root;
  missing(resolve_executable((root.path / "absent").string(), {}));
  std::filesystem::create_symlink(root.path / "absent", root.path / "dangling");
  missing(resolve_executable((root.path / "dangling").string(), {}));
}

TEST_CASE("R17-B resolve relative path with a slash is missing even when it exists", "[process]") {
  temporary root;
  script(root.path / "x", executable);
  script(root.path / "bin/x", executable);
  cwd_guard           guard(root.path);
  const environment_t environment{{"PATH", (root.path / "bin").string()}};
  missing(resolve_executable("./x", environment));
  missing(resolve_executable("bin/x", environment));
}

TEST_CASE("R17-B resolve PATH search picks the first matching entry", "[process]") {
  temporary root;
  script(root.path / "a/tool", executable);
  const auto second = script(root.path / "b/tool", executable);
  std::filesystem::create_directory(root.path / "empty");
  const environment_t environment{
      {"HOME", "/nowhere"},
      {"PATH", std::format("{}:{}:{}", (root.path / "empty").string(), (root.path / "b").string(), (root.path / "a").string())},
      {"TERM", "dumb"}};
  const auto result = resolve_executable("tool", environment);
  REQUIRE(result.has_value());
  CHECK(*result == second);
}

TEST_CASE("R17-B resolve PATH entry with a trailing slash joins once", "[process]") {
  temporary  root;
  const auto tool   = script(root.path / "a/tool", executable);
  const auto result = resolve_executable("tool", {{"PATH", (root.path / "a").string() + "/"}});
  REQUIRE(result.has_value());
  CHECK(*result == tool);
}

TEST_CASE("R17-B resolve skips empty and relative PATH entries", "[process]") {
  temporary root;
  script(root.path / "tool", executable);
  script(root.path / "rel/tool", executable);
  const auto          absolute = script(root.path / "abs/tool", executable);
  cwd_guard           guard(root.path);
  const environment_t environment{{"PATH", std::format("::.:rel:{}:", (root.path / "abs").string())}};
  const auto          result = resolve_executable("tool", environment);
  REQUIRE(result.has_value());
  CHECK(*result == absolute);
  // Without an absolute entry, the cwd-relative matches are never taken.
  missing(resolve_executable("tool", {{"PATH", ":.:rel:"}}));
}

TEST_CASE("R17-B resolve skips non-executable and directory candidates for a later executable", "[process]") {
  temporary root;
  script(root.path / "a/tool", not_executable);
  std::filesystem::create_directories(root.path / "b/tool");
  const auto          good = script(root.path / "c/tool", executable);
  const environment_t environment{
      {"PATH", std::format("{}:{}:{}", (root.path / "a").string(), (root.path / "b").string(), (root.path / "c").string())}};
  const auto result = resolve_executable("tool", environment);
  REQUIRE(result.has_value());
  CHECK(*result == good);
}

TEST_CASE("R17-B resolve no match in PATH is missing", "[process]") {
  temporary root;
  std::filesystem::create_directory(root.path / "a");
  missing(resolve_executable("tool", {{"PATH", (root.path / "a").string()}}));
}

TEST_CASE("R17-B resolve without a PATH variable is missing", "[process]") {
  temporary root;
  script(root.path / "tool", executable);
  cwd_guard guard(root.path);
  missing(resolve_executable("tool", {}));
  missing(resolve_executable("tool", {{"HOME", root.path.string()}, {"Path", root.path.string()}}));
}

TEST_CASE("R17-B resolve ignores the host PATH", "[process]") {
  temporary root;
  script(root.path / "only-in-host-path", executable);
  REQUIRE(::setenv("PATH", (root.path.string() + ":/bin:/usr/bin").c_str(), 1) == 0);
  missing(resolve_executable("only-in-host-path", {}));
  missing(resolve_executable("sh", {{"HOME", "/"}}));
  // An explicit PATH wins over the host value.
  std::filesystem::create_directory(root.path / "other");
  missing(resolve_executable("only-in-host-path", {{"PATH", (root.path / "other").string()}}));
}

TEST_CASE("R17-B resolve empty command0 is missing", "[process]") {
  temporary root;
  script(root.path / "tool", executable);
  missing(resolve_executable("", {{"PATH", root.path.string()}}));
}

TEST_CASE("R17-B resolve command0 with an embedded NUL is missing", "[process]") {
  temporary root;
  script(root.path / "tool", executable);
  missing(resolve_executable(std::string_view("tool\0x", 6), {{"PATH", root.path.string()}}));
}
